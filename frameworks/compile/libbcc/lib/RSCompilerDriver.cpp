/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bcc/RSCompilerDriver.h"

#include "Assert.h"
#include "FileMutex.h"
#include "Log.h"
#include "RSScriptGroupFusion.h"
#include "slang_version.h"

#include "bcc/BCCContext.h"
#include "bcc/Compiler.h"
#include "bcc/CompilerConfig.h"
#include "bcc/Config.h"
#include "bcc/Initialization.h"
#include "bcc/Script.h"
#include "bcc/Source.h"
#include "bcinfo/BitcodeWrapper.h"
#include "bcinfo/MetadataExtractor.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include <llvm/IR/Module.h>
#include "llvm/Linker/Linker.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <sstream>
#include <string>

using namespace bcc;

RSCompilerDriver::RSCompilerDriver() :
    mConfig(nullptr), mCompiler(), mDebugContext(false),
    mLinkRuntimeCallback(nullptr), mEnableGlobalMerge(true),
    mEmbedGlobalInfo(false), mEmbedGlobalInfoSkipConstant(false) {
  init::Initialize();
}

RSCompilerDriver::~RSCompilerDriver() {
  delete mConfig;
}


#if defined(PROVIDE_ARM_CODEGEN)
extern llvm::cl::opt<bool> EnableGlobalMerge;
#endif

bool RSCompilerDriver::setupConfig(const Script &pScript) {
  bool changed = false;

  const llvm::CodeGenOpt::Level script_opt_level = pScript.getOptimizationLevel();

#if defined(PROVIDE_ARM_CODEGEN)
  EnableGlobalMerge = mEnableGlobalMerge;
#endif

  if (mConfig != nullptr) {
    // Renderscript bitcode may have their optimization flag configuration
    // different than the previous run of RS compilation.
    if (mConfig->getOptimizationLevel() != script_opt_level) {
      mConfig->setOptimizationLevel(script_opt_level);
      changed = true;
    }
  } else {
    // Haven't run the compiler ever.
    mConfig = new (std::nothrow) CompilerConfig(DEFAULT_TARGET_TRIPLE_STRING);
    if (mConfig == nullptr) {
      // Return false since mConfig remains NULL and out-of-memory.
      return false;
    }
    mConfig->setOptimizationLevel(script_opt_level);
    changed = true;
  }

#if defined(PROVIDE_ARM_CODEGEN)
  bcinfo::MetadataExtractor me(&pScript.getSource().getModule());
  if (!me.extract()) {
    bccAssert("Could not extract RS pragma metadata for module!");
  }

  bool script_full_prec = (me.getRSFloatPrecision() == bcinfo::RS_FP_Full);
  if (mConfig->getFullPrecision() != script_full_prec) {
    mConfig->setFullPrecision(script_full_prec);
    changed = true;
  }
#endif

  return changed;
}

Compiler::ErrorCode RSCompilerDriver::compileScript(Script& pScript, const char* pScriptName,
                                                    const char* pOutputPath,
                                                    const char* pRuntimePath,
                                                    const char* pBuildChecksum,
                                                    bool pDumpIR) {
  // embed build checksum metadata into the source
  if (pBuildChecksum != nullptr && strlen(pBuildChecksum) > 0) {
    pScript.getSource().addBuildChecksumMetadata(pBuildChecksum);
  }

  // Verify that the only external functions in pScript are Renderscript
  // functions.  Fail if verification returns an error.
  if (mCompiler.screenGlobalFunctions(pScript) != Compiler::kSuccess) {
    return Compiler::kErrInvalidSource;
  }

  // For (32-bit) x86, translate GEPs on structs or arrays of structs to GEPs on
  // int8* with byte offsets.  This is to ensure that layout of structs with
  // 64-bit scalar fields matches frontend-generated code that adheres to ARM
  // data layout.
  //
  // The translation is done before RenderScript runtime library is linked
  // (during LinkRuntime below) to ensure that RenderScript-driver-provided
  // structs (like Allocation_t) don't get forced into using the ARM layout
  // rules.
  if (!pScript.isStructExplicitlyPaddedBySlang() &&
      (mCompiler.getTargetMachine().getTargetTriple().getArch() == llvm::Triple::x86)) {
    mCompiler.translateGEPs(pScript);
  }

  //===--------------------------------------------------------------------===//
  // Link RS script with Renderscript runtime.
  //===--------------------------------------------------------------------===//
  if (!pScript.LinkRuntime(pRuntimePath)) {
    ALOGE("Failed to link script '%s' with Renderscript runtime %s!",
          pScriptName, pRuntimePath);
    return Compiler::kErrInvalidSource;
  }

  {
    // FIXME(srhines): Windows compilation can't use locking like this, but
    // we also don't need to worry about concurrent writers of the same file.
#ifndef _WIN32
    //===------------------------------------------------------------------===//
    // Acquire the write lock for writing output object file.
    //===------------------------------------------------------------------===//
    FileMutex write_output_mutex(pOutputPath);

    if (write_output_mutex.hasError() || !write_output_mutex.lockMutex()) {
      ALOGE("Unable to acquire the lock for writing %s! (%s)",
            pOutputPath, write_output_mutex.getErrorMessage().c_str());
      return Compiler::kErrInvalidOutputFileState;
    }
#endif

    // Open the output file for write.
    std::error_code error;
    llvm::raw_fd_ostream out_stream(pOutputPath, error, llvm::sys::fs::F_RW);
    if (error) {
      ALOGE("Unable to open %s for write! (%s)", pOutputPath,
            error.message().c_str());
      return Compiler::kErrPrepareOutput;
    }

    // Setup the config to the compiler.
    bool compiler_need_reconfigure = setupConfig(pScript);

    if (mConfig == nullptr) {
      ALOGE("Failed to setup config for RS compiler to compile %s!",
            pOutputPath);
      return Compiler::kErrInvalidSource;
    }

    if (compiler_need_reconfigure) {
      Compiler::ErrorCode err = mCompiler.config(*mConfig);
      if (err != Compiler::kSuccess) {
        ALOGE("Failed to config the RS compiler for %s! (%s)",pOutputPath,
              Compiler::GetErrorString(err));
        return Compiler::kErrInvalidSource;
      }
    }

    std::unique_ptr<llvm::raw_fd_ostream> IRStream;
    if (pDumpIR) {
      std::string path(pOutputPath);
      path.append(".ll");
      IRStream.reset(new llvm::raw_fd_ostream(
          path.c_str(), error, llvm::sys::fs::F_RW | llvm::sys::fs::F_Text));
      if (error) {
        ALOGE("Unable to open %s for write! (%s)", path.c_str(),
              error.message().c_str());
        return Compiler::kErrPrepareOutput;
      }
    }

    // Run the compiler.
    Compiler::ErrorCode compile_result =
        mCompiler.compile(pScript, out_stream, IRStream.get());

    if (compile_result != Compiler::kSuccess) {
      ALOGE("Unable to compile the source to file %s! (%s)", pOutputPath,
            Compiler::GetErrorString(compile_result));
      return Compiler::kErrInvalidSource;
    }
  }

  return Compiler::kSuccess;
}

bool RSCompilerDriver::build(BCCContext &pContext,
                             const char *pCacheDir,
                             const char *pResName,
                             const char *pBitcode,
                             size_t pBitcodeSize,
                             const char *pBuildChecksum,
                             const char *pRuntimePath,
                             RSLinkRuntimeCallback pLinkRuntimeCallback,
                             bool pDumpIR) {
    //  android::StopWatch build_time("bcc: RSCompilerDriver::build time");
  //===--------------------------------------------------------------------===//
  // Check parameters.
  //===--------------------------------------------------------------------===//
  if ((pCacheDir == nullptr) || (pResName == nullptr)) {
    ALOGE("Invalid parameter passed to RSCompilerDriver::build()! (cache dir: "
          "%s, resource name: %s)", ((pCacheDir) ? pCacheDir : "(null)"),
                                    ((pResName) ? pResName : "(null)"));
    return false;
  }

  if ((pBitcode == nullptr) || (pBitcodeSize <= 0)) {
    ALOGE("No bitcode supplied! (bitcode: %p, size of bitcode: %u)",
          pBitcode, static_cast<unsigned>(pBitcodeSize));
    return false;
  }

  //===--------------------------------------------------------------------===//
  // Construct output path.
  // {pCacheDir}/{pResName}.o
  //===--------------------------------------------------------------------===//
  llvm::SmallString<80> output_path(pCacheDir);
  llvm::sys::path::append(output_path, pResName);
  llvm::sys::path::replace_extension(output_path, ".o");

  //===--------------------------------------------------------------------===//
  // Load the bitcode and create script.
  //===--------------------------------------------------------------------===//
  Source *source = Source::CreateFromBuffer(pContext, pResName,
                                            pBitcode, pBitcodeSize);
  if (source == nullptr) {
    return false;
  }

  Script script(source);
  script.setOptimizationLevel(getConfig()->getOptimizationLevel());
  if (pLinkRuntimeCallback) {
    setLinkRuntimeCallback(pLinkRuntimeCallback);
  }

  script.setLinkRuntimeCallback(getLinkRuntimeCallback());

  script.setEmbedGlobalInfo(mEmbedGlobalInfo);
  script.setEmbedGlobalInfoSkipConstant(mEmbedGlobalInfoSkipConstant);

  // Read optimization level from bitcode wrapper.
  bcinfo::BitcodeWrapper wrapper(pBitcode, pBitcodeSize);
  script.setOptimizationLevel(static_cast<llvm::CodeGenOpt::Level>(
                              wrapper.getOptimizationLevel()));

// Assertion-enabled builds can't compile legacy bitcode (due to the use of
// getName() with anonymous structure definitions).
#ifdef _DEBUG
  static const uint32_t kSlangMinimumFixedStructureNames = SlangVersion::M_RS_OBJECT;
  uint32_t version = wrapper.getCompilerVersion();
  if (version < kSlangMinimumFixedStructureNames) {
    ALOGE("Found invalid legacy bitcode compiled with a version %u llvm-rs-cc "
          "used with an assertion build", version);
    ALOGE("Please recompile this apk with a more recent llvm-rs-cc "
          "(at least %u)", kSlangMinimumFixedStructureNames);
    return false;
  }
#endif

  //===--------------------------------------------------------------------===//
  // Compile the script
  //===--------------------------------------------------------------------===//
  Compiler::ErrorCode status = compileScript(script, pResName,
                                             output_path.c_str(),
                                             pRuntimePath,
                                             pBuildChecksum,
                                             pDumpIR);

  return status == Compiler::kSuccess;
}

bool RSCompilerDriver::buildScriptGroup(
    BCCContext& Context, const char* pOutputFilepath, const char* pRuntimePath,
    const char* pRuntimeRelaxedPath, bool dumpIR, const char* buildChecksum,
    const std::vector<Source*>& sources,
    const std::list<std::list<std::pair<int, int>>>& toFuse,
    const std::list<std::string>& fused,
    const std::list<std::list<std::pair<int, int>>>& invokes,
    const std::list<std::string>& invokeBatchNames) {

  // Read and store metadata before linking the modules together
  std::vector<bcinfo::MetadataExtractor*> metadata;
  for (Source* source : sources) {
    if (!source->extractMetadata()) {
      ALOGE("Cannot extract metadata from module");
      return false;
    }
  }

  // ---------------------------------------------------------------------------
  // Link all input modules into a single module
  // ---------------------------------------------------------------------------

  llvm::LLVMContext& context = Context.getLLVMContext();
  llvm::Module module("Merged Script Group", context);

  unsigned wrapperCompilerVersion = 0, wrapperOptimizationLevel = 0;
  bool gotFirstSource = false;
  llvm::Linker linker(module);
  for (Source* source : sources) {
    unsigned sourceWrapperCompilerVersion, sourceWrapperOptimizationLevel;
    source->getWrapperInformation(&sourceWrapperCompilerVersion, &sourceWrapperOptimizationLevel);
    if (gotFirstSource) {
      if ((wrapperCompilerVersion != sourceWrapperCompilerVersion) ||
          (wrapperOptimizationLevel != sourceWrapperOptimizationLevel))
        ALOGE("ScriptGroup source files have inconsistent metadata");
        return false;
    } else {
      wrapperCompilerVersion = sourceWrapperCompilerVersion;
      wrapperOptimizationLevel = sourceWrapperOptimizationLevel;
      gotFirstSource = true;
    }
    std::unique_ptr<llvm::Module> sourceModule(&source->getModule());
    if (linker.linkInModule(std::move(sourceModule))) {
      ALOGE("Linking for module in source failed.");
      return false;
    }
    // source->getModule() is destroyed after linking.
    source->markModuleDestroyed();
    // linking copies metadata from source->getModule(), but we don't
    // want the wrapper metadata (we'll be reconstructing this when we
    // instantiate a Source instance from the new Module).
    llvm::NamedMDNode *const wrapperMDNode =
        module.getNamedMetadata(bcinfo::MetadataExtractor::kWrapperMetadataName);
    bccAssert(wrapperMDNode != nullptr);
    module.eraseNamedMetadata(wrapperMDNode);
  }

  // ---------------------------------------------------------------------------
  // Create fused kernels
  // ---------------------------------------------------------------------------

  auto inputIter = toFuse.begin();
  for (const std::string& nameOfFused : fused) {
    auto inputKernels = *inputIter++;
    std::vector<Source*> sourcesToFuse;
    std::vector<int> slots;

    for (auto p : inputKernels) {
      sourcesToFuse.push_back(sources[p.first]);
      slots.push_back(p.second);
    }

    if (!fuseKernels(Context, sourcesToFuse, slots, nameOfFused, &module)) {
      return false;
    }
  }

  // ---------------------------------------------------------------------------
  // Rename invokes
  // ---------------------------------------------------------------------------

  auto invokeIter = invokes.begin();
  for (const std::string& newName : invokeBatchNames) {
    auto inputInvoke = *invokeIter++;
    auto p = inputInvoke.front();
    Source* source = sources[p.first];
    int slot = p.second;

    if (!renameInvoke(Context, source, slot, newName, &module)) {
      return false;
    }
  }

  // ---------------------------------------------------------------------------
  // Compile the new module with fused kernels
  // ---------------------------------------------------------------------------

  const std::unique_ptr<Source> source(
      Source::CreateFromModule(Context, pOutputFilepath, module,
                               wrapperCompilerVersion, wrapperOptimizationLevel,
                               true));
  Script script(source.get());

  // Embed the info string directly in the ELF
  script.setEmbedInfo(true);
  // TODO jeanluc Should we override the config's optimization?
  // i.e., why not script.setOptimizationLevel(getConfig()->getOptimizationLevel)?
  script.setOptimizationLevel(llvm::CodeGenOpt::Level::Aggressive);
  script.setEmbedGlobalInfo(mEmbedGlobalInfo);
  script.setEmbedGlobalInfoSkipConstant(mEmbedGlobalInfoSkipConstant);

  llvm::SmallString<80> output_path(pOutputFilepath);
  llvm::sys::path::replace_extension(output_path, ".o");

  // Pick the right runtime lib
  const char* coreLibPath = pRuntimePath;
  if (strcmp(pRuntimeRelaxedPath, "")) {
      bcinfo::MetadataExtractor me(&module);
      me.extract();
      if (me.getRSFloatPrecision() == bcinfo::RS_FP_Relaxed) {
          coreLibPath = pRuntimeRelaxedPath;
      }
  }

  compileScript(script, pOutputFilepath, output_path.c_str(), coreLibPath,
                buildChecksum, dumpIR);

  return true;
}

bool RSCompilerDriver::buildForCompatLib(Script &pScript, const char *pOut,
                                         const char *pBuildChecksum,
                                         const char *pRuntimePath,
                                         bool pDumpIR) {
  // Embed the info string directly in the ELF, since this path is for an
  // offline (host) compilation.
  pScript.setEmbedInfo(true);

  pScript.setEmbedGlobalInfo(mEmbedGlobalInfo);
  pScript.setEmbedGlobalInfoSkipConstant(mEmbedGlobalInfoSkipConstant);
  pScript.setLinkRuntimeCallback(getLinkRuntimeCallback());

  Compiler::ErrorCode status = compileScript(pScript, pOut, pOut, pRuntimePath,
                                             pBuildChecksum, pDumpIR);
  if (status != Compiler::kSuccess) {
    return false;
  }

  return true;
}
