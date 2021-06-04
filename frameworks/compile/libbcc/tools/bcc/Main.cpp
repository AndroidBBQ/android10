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

#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <dlfcn.h>
#include <stdlib.h>

#include <log/log.h>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Config/config.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/PluginLoader.h>
#include <llvm/Support/raw_ostream.h>

#include <bcc/BCCContext.h>
#include <bcc/CompilerConfig.h>
#include <bcc/Config.h>
#include <bcc/Initialization.h>
#include <bcc/RSCompilerDriver.h>
#include <bcc/Source.h>

#ifdef __ANDROID__
#include <vndksupport/linker.h>
#endif

using namespace bcc;

#define STR2(a) #a
#define STR(a) STR2(a)

//===----------------------------------------------------------------------===//
// General Options
//===----------------------------------------------------------------------===//
namespace {

llvm::cl::list<std::string>
OptInputFilenames(llvm::cl::Positional, llvm::cl::OneOrMore,
                  llvm::cl::desc("<input bitcode files>"));

llvm::cl::list<std::string>
OptMergePlans("merge", llvm::cl::ZeroOrMore,
               llvm::cl::desc("Lists of kernels to merge (as source-and-slot "
                              "pairs) and names for the final merged kernels"));

llvm::cl::list<std::string>
OptInvokes("invoke", llvm::cl::ZeroOrMore,
           llvm::cl::desc("Invocable functions"));

llvm::cl::opt<std::string>
OptOutputFilename("o", llvm::cl::desc("Specify the output filename"),
                  llvm::cl::value_desc("filename"),
                  llvm::cl::init("bcc_output"));

llvm::cl::opt<std::string>
OptBCLibFilename("bclib", llvm::cl::desc("Specify the bclib filename"),
                 llvm::cl::value_desc("bclib"));

llvm::cl::opt<std::string>
OptBCLibRelaxedFilename("bclib_relaxed", llvm::cl::desc("Specify the bclib filename optimized for "
                                                        "relaxed precision floating point maths"),
                        llvm::cl::init(""),
                        llvm::cl::value_desc("bclib_relaxed"));

llvm::cl::opt<std::string>
OptOutputPath("output_path", llvm::cl::desc("Specify the output path"),
              llvm::cl::value_desc("output path"),
              llvm::cl::init("."));

llvm::cl::opt<bool>
OptEmitLLVM("emit-llvm",
            llvm::cl::desc("Emit an LLVM-IR version of the generated program"));

llvm::cl::opt<std::string>
OptTargetTriple("mtriple",
                llvm::cl::desc("Specify the target triple (default: "
                               DEFAULT_TARGET_TRIPLE_STRING ")"),
                llvm::cl::init(DEFAULT_TARGET_TRIPLE_STRING),
                llvm::cl::value_desc("triple"));

llvm::cl::alias OptTargetTripleC("C", llvm::cl::NotHidden,
                                 llvm::cl::desc("Alias for -mtriple"),
                                 llvm::cl::aliasopt(OptTargetTriple));

llvm::cl::opt<bool>
OptRSDebugContext("rs-debug-ctx",
    llvm::cl::desc("Enable build to work with a RenderScript debug context"));

llvm::cl::opt<bool>
OptRSGlobalInfo("rs-global-info",
    llvm::cl::desc("Embed information about global variables in the code"));

llvm::cl::opt<bool>
OptRSGlobalInfoSkipConstant("rs-global-info-skip-constant",
    llvm::cl::desc("Skip embedding information about constant global "
                   "variables in the code"));

llvm::cl::opt<std::string>
OptChecksum("build-checksum",
            llvm::cl::desc("Embed a checksum of this compiler invocation for"
                           " cache invalidation at a later time"),
            llvm::cl::value_desc("checksum"));

#ifdef __ANDROID__
llvm::cl::opt<std::string>
OptVendorPlugin("plugin", llvm::cl::ZeroOrMore,
    llvm::cl::value_desc("pluginfilename"),
    llvm::cl::desc("Load the specified vendor plugin. Use this instead of the -load option"));
#endif

//===----------------------------------------------------------------------===//
// Compiler Options
//===----------------------------------------------------------------------===//
llvm::cl::opt<bool>
OptPIC("fPIC", llvm::cl::desc("Generate fully relocatable, position independent"
                              " code"));

// If set, use buildForCompatLib to embed RS symbol information into the object
// file.  The information is stored in the .rs.info variable.  This option is
// to be used in tandem with -fPIC.
llvm::cl::opt<bool>
OptEmbedRSInfo("embedRSInfo",
    llvm::cl::desc("Embed RS Info into the object file instead of generating"
                   " a separate .o.info file"));

// RenderScript uses -O3 by default
llvm::cl::opt<char>
OptOptLevel("O", llvm::cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                                "(default: -O3)"),
            llvm::cl::Prefix, llvm::cl::ZeroOrMore, llvm::cl::init('3'));

// Override "bcc -version" since the LLVM version information is not correct on
// Android build.
void BCCVersionPrinter() {
  llvm::raw_ostream &os = llvm::outs();
  os << "libbcc (The Android Open Source Project, http://www.android.com/):\n"
     << "  Default target: " << DEFAULT_TARGET_TRIPLE_STRING << "\n\n"
     << "LLVM (http://llvm.org/):\n"
     << "  Version: " << PACKAGE_VERSION << "\n";
  return;
}

void extractSourcesAndSlots(const llvm::cl::list<std::string>& optList,
                            std::list<std::string>* batchNames,
                            std::list<std::list<std::pair<int, int>>>* sourcesAndSlots) {
  for (unsigned i = 0; i < optList.size(); ++i) {
    std::string plan = optList[i];
    unsigned found = plan.find(':');

    std::string name = plan.substr(0, found);
    std::cerr << "new kernel name: " << name << std::endl;
    batchNames->push_back(name);

    std::istringstream iss(plan.substr(found + 1));
    std::string s;
    std::list<std::pair<int, int>> planList;
    while (getline(iss, s, '.')) {
      found = s.find(',');
      std::string sourceStr = s.substr(0, found);
      std::string slotStr = s.substr(found + 1);

      std::cerr << "source " << sourceStr << ", slot " << slotStr << std::endl;

      int source = std::stoi(sourceStr);
      int slot = std::stoi(slotStr);
      planList.push_back(std::make_pair(source, slot));
    }

    sourcesAndSlots->push_back(planList);
  }
}

bool compileScriptGroup(BCCContext& Context, RSCompilerDriver& RSCD) {
  std::vector<bcc::Source*> sources;
  for (unsigned i = 0; i < OptInputFilenames.size(); ++i) {
    bcc::Source* source =
        bcc::Source::CreateFromFile(Context, OptInputFilenames[i]);
    if (!source) {
      llvm::errs() << "Error loading file '" << OptInputFilenames[i]<< "'\n";
      return false;
    }
    sources.push_back(source);
  }

  std::list<std::string> fusedKernelNames;
  std::list<std::list<std::pair<int, int>>> sourcesAndSlots;
  extractSourcesAndSlots(OptMergePlans, &fusedKernelNames, &sourcesAndSlots);

  std::list<std::string> invokeBatchNames;
  std::list<std::list<std::pair<int, int>>> invokeSourcesAndSlots;
  extractSourcesAndSlots(OptInvokes, &invokeBatchNames, &invokeSourcesAndSlots);

  std::string outputFilepath(OptOutputPath);
  outputFilepath.append("/");
  outputFilepath.append(OptOutputFilename);

  bool success = RSCD.buildScriptGroup(
    Context, outputFilepath.c_str(), OptBCLibFilename.c_str(),
    OptBCLibRelaxedFilename.c_str(), OptEmitLLVM, OptChecksum.c_str(),
    sources, sourcesAndSlots, fusedKernelNames,
    invokeSourcesAndSlots, invokeBatchNames);

  return success;
}

} // end anonymous namespace

static inline
bool ConfigCompiler(RSCompilerDriver &pRSCD) {
  Compiler *RSC = pRSCD.getCompiler();
  CompilerConfig *config = nullptr;

  config = new (std::nothrow) CompilerConfig(OptTargetTriple);
  if (config == nullptr) {
    llvm::errs() << "Out of memory when create the compiler configuration!\n";
    return false;
  }

  if (OptPIC) {
    config->setRelocationModel(llvm::Reloc::PIC_);

    // For x86_64, CodeModel needs to be small if PIC_ reloc is used.
    // Otherwise, we end up with TEXTRELs in the shared library.
    if (config->getTriple().find("x86_64") != std::string::npos) {
        config->setCodeModel(llvm::CodeModel::Small);
    }
  }
  switch (OptOptLevel) {
    case '0': config->setOptimizationLevel(llvm::CodeGenOpt::None); break;
    case '1': config->setOptimizationLevel(llvm::CodeGenOpt::Less); break;
    case '2': config->setOptimizationLevel(llvm::CodeGenOpt::Default); break;
    case '3':
    default: {
      config->setOptimizationLevel(llvm::CodeGenOpt::Aggressive);
      break;
    }
  }

  pRSCD.setConfig(config);
  Compiler::ErrorCode result = RSC->config(*config);

  if (OptRSDebugContext) {
    pRSCD.setDebugContext(true);
  }

  if (OptRSGlobalInfo) {
    pRSCD.setEmbedGlobalInfo(true);
  }

  if (OptRSGlobalInfoSkipConstant) {
    pRSCD.setEmbedGlobalInfoSkipConstant(true);
  }

  if (result != Compiler::kSuccess) {
    llvm::errs() << "Failed to configure the compiler! (detail: "
                 << Compiler::GetErrorString(result) << ")\n";
    return false;
  }

  return true;
}

int main(int argc, char **argv) {

  llvm::llvm_shutdown_obj Y;
  init::Initialize();
  llvm::cl::SetVersionPrinter(BCCVersionPrinter);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  BCCContext context;
  RSCompilerDriver RSCD;

  if (OptBCLibFilename.empty()) {
    ALOGE("Failed to compile bitcode, -bclib was not specified");
    return EXIT_FAILURE;
  }

#ifdef __ANDROID__
  if (!OptVendorPlugin.empty()) {
    // bcc is a system process and the vendor plugin is a vendor lib. Since the
    // vendor lib might have been compiled using the old versions of platform
    // libraries, they must not be directly loaded into the default namespace
    // but into the sphal namespace where old versions of platform libraries
    // (aka VNDK libs) are provided.
    void* handle = android_load_sphal_library(OptVendorPlugin.c_str(), RTLD_LAZY|RTLD_GLOBAL);
    if (handle == nullptr) {
      ALOGE("Failed to load vendor plugin %s", OptVendorPlugin.c_str());
      return EXIT_FAILURE;
    }
  }
#endif

  if (!ConfigCompiler(RSCD)) {
    ALOGE("Failed to configure compiler");
    return EXIT_FAILURE;
  }

  // Attempt to dynamically initialize the compiler driver if such a function
  // is present. It is only present if passed via "-load libFOO.so".
  RSCompilerDriverInit_t rscdi = (RSCompilerDriverInit_t)
      dlsym(RTLD_DEFAULT, STR(RS_COMPILER_DRIVER_INIT_FN));
  if (rscdi != nullptr) {
    rscdi(&RSCD);
  }

  if (OptMergePlans.size() > 0) {
    bool success = compileScriptGroup(context, RSCD);

    if (!success) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> mb_or_error =
      llvm::MemoryBuffer::getFile(OptInputFilenames[0].c_str());
  if (mb_or_error.getError()) {
    ALOGE("Failed to load bitcode from path %s! (%s)",
          OptInputFilenames[0].c_str(), mb_or_error.getError().message().c_str());
    return EXIT_FAILURE;
  }
  std::unique_ptr<llvm::MemoryBuffer> input_data = std::move(mb_or_error.get());

  const char *bitcode = input_data->getBufferStart();
  size_t bitcodeSize = input_data->getBufferSize();

  if (!OptEmbedRSInfo) {
    bool built = RSCD.build(context, OptOutputPath.c_str(),
                            OptOutputFilename.c_str(),
                            bitcode, bitcodeSize,
                            OptChecksum.c_str(), OptBCLibFilename.c_str(),
                            nullptr, OptEmitLLVM);

    if (!built) {
      return EXIT_FAILURE;
    }
  } else {
    // embedRSInfo is set.  Use buildForCompatLib to embed RS symbol information
    // into the .rs.info symbol.
    Source *source = Source::CreateFromBuffer(context, OptInputFilenames[0].c_str(),
                                              bitcode, bitcodeSize);

    // If the bitcode fails verification in the bitcode loader, the returned Source is set to NULL.
    if (!source) {
      ALOGE("Failed to load source from file %s", OptInputFilenames[0].c_str());
      return EXIT_FAILURE;
    }

    std::unique_ptr<Script> s(new (std::nothrow) Script(source));
    if (s == nullptr) {
      llvm::errs() << "Out of memory when creating script for file `"
                   << OptInputFilenames[0] << "'!\n";
      delete source;
      return EXIT_FAILURE;
    }

    s->setOptimizationLevel(RSCD.getConfig()->getOptimizationLevel());
    llvm::SmallString<80> output(OptOutputPath);
    llvm::sys::path::append(output, "/", OptOutputFilename);
    llvm::sys::path::replace_extension(output, ".o");

    if (!RSCD.buildForCompatLib(*s, output.c_str(), OptChecksum.c_str(),
                                OptBCLibFilename.c_str(), OptEmitLLVM)) {
      fprintf(stderr, "Failed to compile script!");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
