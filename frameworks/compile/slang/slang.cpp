/*
 * Copyright 2010, The Android Open Source Project
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

#include "slang.h"

#include <stdlib.h>

#include <cstring>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"

#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/FileSystemOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"

#include "clang/Frontend/DependencyOutputOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/PCHContainerOperations.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"

#include "clang/Parse/ParseAST.h"

#include "clang/Sema/SemaDiagnostic.h"

#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "llvm/Bitcode/ReaderWriter.h"

// More force linking
#include "llvm/Linker/Linker.h"

// Force linking all passes/vmcore stuffs to libslang.so
#include "llvm/LinkAllIR.h"
#include "llvm/LinkAllPasses.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"

#include "os_sep.h"
#include "rs_cc_options.h"
#include "slang_assert.h"
#include "slang_backend.h"

#include "slang_rs_context.h"
#include "slang_rs_export_type.h"

#include "slang_rs_reflection.h"
#include "slang_rs_reflection_cpp.h"
#include "slang_rs_reflection_state.h"

namespace {

static const char *kRSTriple32 = "renderscript32-none-linux-gnueabi";
static const char *kRSTriple64 = "renderscript64-none-linux-gnueabi";

}  // namespace

namespace slang {


#define FS_SUFFIX  "fs"

#define RS_HEADER_SUFFIX  "rsh"

/* RS_HEADER_ENTRY(name) */
#define ENUM_RS_HEADER()  \
  RS_HEADER_ENTRY(rs_allocation_create) \
  RS_HEADER_ENTRY(rs_allocation_data) \
  RS_HEADER_ENTRY(rs_atomic) \
  RS_HEADER_ENTRY(rs_convert) \
  RS_HEADER_ENTRY(rs_core) \
  RS_HEADER_ENTRY(rs_debug) \
  RS_HEADER_ENTRY(rs_for_each) \
  RS_HEADER_ENTRY(rs_graphics) \
  RS_HEADER_ENTRY(rs_graphics_types) \
  RS_HEADER_ENTRY(rs_io) \
  RS_HEADER_ENTRY(rs_math) \
  RS_HEADER_ENTRY(rs_matrix) \
  RS_HEADER_ENTRY(rs_object_info) \
  RS_HEADER_ENTRY(rs_object_types) \
  RS_HEADER_ENTRY(rs_quaternion) \
  RS_HEADER_ENTRY(rs_time) \
  RS_HEADER_ENTRY(rs_value_types) \
  RS_HEADER_ENTRY(rs_vector_math) \


// The named of metadata node that pragma resides (should be synced with
// bcc.cpp)
const llvm::StringRef Slang::PragmaMetadataName = "#pragma";

static llvm::LLVMContext globalContext;

llvm::LLVMContext &getGlobalLLVMContext() { return globalContext; }

static inline std::unique_ptr<llvm::tool_output_file>
OpenOutputFile(const char *OutputFile,
               llvm::sys::fs::OpenFlags Flags,
               std::error_code &EC,
               clang::DiagnosticsEngine *DiagEngine) {
  slangAssert((OutputFile != nullptr) &&
              (DiagEngine != nullptr) && "Invalid parameter!");

  EC = llvm::sys::fs::create_directories(
      llvm::sys::path::parent_path(OutputFile));
  if (!EC) {
    return llvm::make_unique<llvm::tool_output_file>(OutputFile, EC, Flags);
  }

  // Report error here.
  DiagEngine->Report(clang::diag::err_fe_error_opening)
    << OutputFile << EC.message();

  return nullptr;
}

void Slang::createTarget(uint32_t BitWidth) {
  if (BitWidth == 64) {
    mTargetOpts->Triple = kRSTriple64;
  } else {
    mTargetOpts->Triple = kRSTriple32;
  }

  mTarget.reset(clang::TargetInfo::CreateTargetInfo(*mDiagEngine,
                                                    mTargetOpts));
}

void Slang::createFileManager() {
  mFileSysOpt.reset(new clang::FileSystemOptions());
  mFileMgr.reset(new clang::FileManager(*mFileSysOpt));
}

void Slang::createSourceManager() {
  mSourceMgr.reset(new clang::SourceManager(*mDiagEngine, *mFileMgr));
}

void Slang::createPreprocessor() {
  // Default only search header file in current dir
  clang::HeaderSearch *HeaderInfo = new clang::HeaderSearch(&getHeaderSearchOpts(),
                                                            *mSourceMgr,
                                                            *mDiagEngine,
                                                            LangOpts,
                                                            mTarget.get());

  mPP.reset(new clang::Preprocessor(&getPreprocessorOpts(),
                                    *mDiagEngine,
                                    LangOpts,
                                    *mSourceMgr,
                                    *HeaderInfo,
                                    *this,
                                    nullptr,
                                    /* OwnsHeaderSearch = */true));
  // Initialize the preprocessor
  mPP->Initialize(getTargetInfo());
  clang::FrontendOptions FEOpts;

  auto *Reader = mPCHContainerOperations->getReaderOrNull(
      getHeaderSearchOpts().ModuleFormat);
  clang::InitializePreprocessor(*mPP, getPreprocessorOpts(), *Reader, FEOpts);

  clang::ApplyHeaderSearchOptions(*HeaderInfo, getHeaderSearchOpts(), LangOpts,
      mPP->getTargetInfo().getTriple());

  mPragmas.clear();

  std::vector<clang::DirectoryLookup> SearchList;
  for (unsigned i = 0, e = mIncludePaths.size(); i != e; i++) {
    if (const clang::DirectoryEntry *DE =
            mFileMgr->getDirectory(mIncludePaths[i])) {
      SearchList.push_back(clang::DirectoryLookup(DE,
                                                  clang::SrcMgr::C_System,
                                                  false));
    }
  }

  HeaderInfo->SetSearchPaths(SearchList,
                             /* angledDirIdx = */1,
                             /* systemDixIdx = */1,
                             /* noCurDirSearch = */false);

  initPreprocessor();
}

void Slang::createASTContext() {
  mASTContext.reset(
      new clang::ASTContext(LangOpts, *mSourceMgr, mPP->getIdentifierTable(),
                            mPP->getSelectorTable(), mPP->getBuiltinInfo()));
  mASTContext->InitBuiltinTypes(getTargetInfo());
  initASTContext();
}

clang::ASTConsumer *
Slang::createBackend(const RSCCOptions &Opts, const clang::CodeGenOptions &CodeGenOpts,
                     llvm::raw_ostream *OS, OutputType OT) {
  auto *B = new Backend(mRSContext, &getDiagnostics(), Opts,
                        getHeaderSearchOpts(), getPreprocessorOpts(),
                        CodeGenOpts, getTargetOptions(), &mPragmas, OS, OT,
                        getSourceManager(), mAllowRSPrefix, mIsFilterscript);
  B->Initialize(getASTContext());
  return B;
}

Slang::Slang(uint32_t BitWidth, clang::DiagnosticsEngine *DiagEngine,
             DiagnosticBuffer *DiagClient)
    : mDiagEngine(DiagEngine), mDiagClient(DiagClient),
      mTargetOpts(new clang::TargetOptions()),
      mHSOpts(new clang::HeaderSearchOptions()),
      mPPOpts(new clang::PreprocessorOptions()),
      mPCHContainerOperations(std::make_shared<clang::PCHContainerOperations>()),
      mOT(OT_Default), mRSContext(nullptr), mAllowRSPrefix(false), mTargetAPI(0),
      mVerbose(false), mIsFilterscript(false) {
  // Please refer to include/clang/Basic/LangOptions.h to setup
  // the options.
  LangOpts.RTTI = 0;  // Turn off the RTTI information support
  LangOpts.LineComment = 1;
  LangOpts.C99 = 1;
  LangOpts.RenderScript = 1;
  LangOpts.LaxVectorConversions = 0;  // Do not bitcast vectors!
  LangOpts.CharIsSigned = 1;  // Signed char is our default.

  CodeGenOpts.OptimizationLevel = 3;

  // We must set StackRealignment, because the default is for the actual
  // Clang driver to pass this option (-mstackrealign) directly to cc1.
  // Since we don't use Clang's driver, we need to similarly supply it.
  // If StackRealignment is zero (i.e. the option wasn't set), then the
  // backend assumes that it can't adjust the stack in any way, which breaks
  // alignment for vector loads/stores.
  CodeGenOpts.StackRealignment = 1;

  createTarget(BitWidth);
  createFileManager();
  createSourceManager();
}

Slang::~Slang() {
  delete mRSContext;
  for (ReflectedDefinitionListTy::iterator I = ReflectedDefinitions.begin(),
                                           E = ReflectedDefinitions.end();
       I != E; I++) {
    delete I->getValue().first;
  }
}

clang::ModuleLoadResult Slang::loadModule(
    clang::SourceLocation ImportLoc,
    clang::ModuleIdPath Path,
    clang::Module::NameVisibilityKind Visibility,
    bool IsInclusionDirective) {
  slangAssert(0 && "Not implemented");
  return clang::ModuleLoadResult();
}

bool Slang::setInputSource(llvm::StringRef InputFile) {
  mInputFileName = InputFile.str();

  mSourceMgr->clearIDTables();

  const clang::FileEntry *File = mFileMgr->getFile(InputFile);
  if (File) {
    mSourceMgr->setMainFileID(mSourceMgr->createFileID(File,
        clang::SourceLocation(), clang::SrcMgr::C_User));
  }

  if (mSourceMgr->getMainFileID().isInvalid()) {
    mDiagEngine->Report(clang::diag::err_fe_error_reading) << InputFile;
    return false;
  }

  return true;
}

bool Slang::setOutput(const char *OutputFile) {
  std::error_code EC;
  std::unique_ptr<llvm::tool_output_file> OS;

  switch (mOT) {
    case OT_Dependency:
    case OT_Assembly:
    case OT_LLVMAssembly: {
      OS = OpenOutputFile(OutputFile, llvm::sys::fs::F_Text, EC, mDiagEngine);
      break;
    }
    case OT_Nothing: {
      break;
    }
    case OT_Object:
    case OT_Bitcode: {
      OS = OpenOutputFile(OutputFile, llvm::sys::fs::F_None, EC, mDiagEngine);
      break;
    }
    default: {
      llvm_unreachable("Unknown compiler output type");
    }
  }

  if (EC)
    return false;

  mOS = std::move(OS);

  mOutputFileName = OutputFile;

  return true;
}

bool Slang::setDepOutput(const char *OutputFile) {
  std::error_code EC;

  mDOS = OpenOutputFile(OutputFile, llvm::sys::fs::F_Text, EC, mDiagEngine);
  if (EC || (mDOS.get() == nullptr))
    return false;

  mDepOutputFileName = OutputFile;

  return true;
}

int Slang::generateDepFile(bool PhonyTarget) {
  if (mDiagEngine->hasErrorOccurred())
    return 1;
  if (mDOS.get() == nullptr)
    return 1;

  // Initialize options for generating dependency file
  clang::DependencyOutputOptions DepOpts;
  DepOpts.IncludeSystemHeaders = 1;
  if (PhonyTarget)
    DepOpts.UsePhonyTargets = 1;
  DepOpts.OutputFile = mDepOutputFileName;
  DepOpts.Targets = mAdditionalDepTargets;
  DepOpts.Targets.push_back(mDepTargetBCFileName);
  for (std::vector<std::string>::const_iterator
           I = mGeneratedFileNames.begin(), E = mGeneratedFileNames.end();
       I != E;
       I++) {
    DepOpts.Targets.push_back(*I);
  }
  mGeneratedFileNames.clear();

  // Per-compilation needed initialization
  createPreprocessor();
  clang::DependencyFileGenerator::CreateAndAttachToPreprocessor(*mPP.get(), DepOpts);

  // Inform the diagnostic client we are processing a source file
  mDiagClient->BeginSourceFile(LangOpts, mPP.get());

  // Go through the source file (no operations necessary)
  clang::Token Tok;
  mPP->EnterMainSourceFile();
  do {
    mPP->Lex(Tok);
  } while (Tok.isNot(clang::tok::eof));

  mPP->EndSourceFile();

  // Declare success if no error
  if (!mDiagEngine->hasErrorOccurred())
    mDOS->keep();

  // Clean up after compilation
  mPP.reset();
  mDOS.reset();

  return mDiagEngine->hasErrorOccurred() ? 1 : 0;
}

int Slang::compile(const RSCCOptions &Opts) {
  if (mDiagEngine->hasErrorOccurred())
    return 1;
  if (mOS.get() == nullptr)
    return 1;

  // Here is per-compilation needed initialization
  createPreprocessor();
  createASTContext();

  mBackend.reset(createBackend(Opts, CodeGenOpts, &mOS->os(), mOT));

  // Inform the diagnostic client we are processing a source file
  mDiagClient->BeginSourceFile(LangOpts, mPP.get());

  // The core of the slang compiler
  ParseAST(*mPP, mBackend.get(), *mASTContext);

  // Inform the diagnostic client we are done with previous source file
  mDiagClient->EndSourceFile();

  // Declare success if no error
  if (!mDiagEngine->hasErrorOccurred())
    mOS->keep();

  // The compilation ended, clear
  mBackend.reset();
  mOS.reset();

  return mDiagEngine->hasErrorOccurred() ? 1 : 0;
}

void Slang::setDebugMetadataEmission(bool EmitDebug) {
  if (EmitDebug)
    CodeGenOpts.setDebugInfo(clang::codegenoptions::FullDebugInfo);
  else
    CodeGenOpts.setDebugInfo(clang::codegenoptions::NoDebugInfo);
}

void Slang::setOptimizationLevel(llvm::CodeGenOpt::Level OptimizationLevel) {
  CodeGenOpts.OptimizationLevel = OptimizationLevel;
}

bool Slang::isFilterscript(const char *Filename) {
  const char *c = strrchr(Filename, '.');
  if (c && !strncmp(FS_SUFFIX, c + 1, strlen(FS_SUFFIX) + 1)) {
    return true;
  } else {
    return false;
  }
}

bool Slang::generateJavaBitcodeAccessor(const std::string &OutputPathBase,
                                          const std::string &PackageName,
                                          const std::string *LicenseNote) {
  RSSlangReflectUtils::BitCodeAccessorContext BCAccessorContext;

  BCAccessorContext.rsFileName = getInputFileName().c_str();
  BCAccessorContext.bc32FileName = mOutput32FileName.c_str();
  BCAccessorContext.bc64FileName = mOutputFileName.c_str();
  BCAccessorContext.reflectPath = OutputPathBase.c_str();
  BCAccessorContext.packageName = PackageName.c_str();
  BCAccessorContext.licenseNote = LicenseNote;
  BCAccessorContext.bcStorage = BCST_JAVA_CODE;   // Must be BCST_JAVA_CODE
  BCAccessorContext.verbose = false;

  return RSSlangReflectUtils::GenerateJavaBitCodeAccessor(BCAccessorContext);
}

bool Slang::checkODR(const char *CurInputFile) {
  for (RSContext::ExportableList::iterator I = mRSContext->exportable_begin(),
          E = mRSContext->exportable_end();
       I != E;
       I++) {
    RSExportable *RSE = *I;
    if (RSE->getKind() != RSExportable::EX_TYPE)
      continue;

    RSExportType *ET = static_cast<RSExportType *>(RSE);
    if (ET->getClass() != RSExportType::ExportClassRecord)
      continue;

    RSExportRecordType *ERT = static_cast<RSExportRecordType *>(ET);

    // Artificial record types (create by us not by user in the source) always
    // conforms the ODR.
    if (ERT->isArtificial())
      continue;

    // Key to lookup ERT in ReflectedDefinitions
    llvm::StringRef RDKey(ERT->getName());
    ReflectedDefinitionListTy::const_iterator RD =
        ReflectedDefinitions.find(RDKey);

    if (RD != ReflectedDefinitions.end()) {
      const RSExportRecordType *Reflected = RD->getValue().first;

      // See RSExportRecordType::matchODR for the logic
      if (!Reflected->matchODR(ERT, true)) {
        unsigned DiagID = mDiagEngine->getCustomDiagID(
            clang::DiagnosticsEngine::Error,
            "type '%0' in different translation unit (%1 v.s. %2) "
            "has incompatible type definition");
        getDiagnostics().Report(DiagID) << Reflected->getName()
                                        << getInputFileName()
                                        << RD->getValue().second;
        return false;
      }
    } else {
      llvm::StringMapEntry<ReflectedDefinitionTy> *ME =
          llvm::StringMapEntry<ReflectedDefinitionTy>::Create(RDKey);
      ME->setValue(std::make_pair(ERT, CurInputFile));

      if (!ReflectedDefinitions.insert(ME)) {
        slangAssert(false && "Type shouldn't be in map yet!");
      }

      // Take the ownership of ERT such that it won't be freed in ~RSContext().
      ERT->keep();
    }
  }
  return true;
}

void Slang::initPreprocessor() {
  clang::Preprocessor &PP = getPreprocessor();

  std::stringstream RSH;
  RSH << PP.getPredefines();
  RSH << "#define RS_VERSION " << mTargetAPI << "\n";
  RSH << "#include \"rs_core." RS_HEADER_SUFFIX "\"\n";
  PP.setPredefines(RSH.str());
}

void Slang::initASTContext() {
  mRSContext = new RSContext(getPreprocessor(),
                             getASTContext(),
                             getTargetInfo(),
                             &mPragmas,
                             mTargetAPI,
                             mVerbose);
}

bool Slang::IsRSHeaderFile(const char *File) {
#define RS_HEADER_ENTRY(name)  \
  if (::strcmp(File, #name "." RS_HEADER_SUFFIX) == 0)  \
    return true;
ENUM_RS_HEADER()
#undef RS_HEADER_ENTRY
  return false;
}

bool Slang::IsLocInRSHeaderFile(const clang::SourceLocation &Loc,
                                  const clang::SourceManager &SourceMgr) {
  clang::FullSourceLoc FSL(Loc, SourceMgr);
  clang::PresumedLoc PLoc = SourceMgr.getPresumedLoc(FSL);

  const char *Filename = PLoc.getFilename();
  if (!Filename) {
    return false;
  } else {
    return IsRSHeaderFile(llvm::sys::path::filename(Filename).data());
  }
}

bool Slang::compile(
    const std::list<std::pair<const char*, const char*> > &IOFiles64,
    const std::list<std::pair<const char*, const char*> > &IOFiles32,
    const std::list<std::pair<const char*, const char*> > &DepFiles,
    const RSCCOptions &Opts,
    clang::DiagnosticOptions &DiagOpts,
    ReflectionState *RState) {
  if (IOFiles32.empty())
    return true;

  if (Opts.mEmitDependency && (DepFiles.size() != IOFiles32.size())) {
    unsigned DiagID = mDiagEngine->getCustomDiagID(
        clang::DiagnosticsEngine::Error,
        "invalid parameter for output dependencies files.");
    getDiagnostics().Report(DiagID);
    return false;
  }

  if (Opts.mEmit3264 && (IOFiles64.size() != IOFiles32.size())) {
    slangAssert(false && "Should have equal number of 32/64-bit files");
    return false;
  }

  std::string RealPackageName;

  const char *InputFile, *Output64File, *Output32File, *BCOutputFile,
             *DepOutputFile;

  setIncludePaths(Opts.mIncludePaths);
  setOutputType(Opts.mOutputType);
  if (Opts.mEmitDependency) {
    setAdditionalDepTargets(Opts.mAdditionalDepTargets);
  }

  setDebugMetadataEmission(Opts.mDebugEmission);

  setOptimizationLevel(Opts.mOptimizationLevel);

  mAllowRSPrefix = Opts.mAllowRSPrefix;

  mTargetAPI = Opts.mTargetAPI;
  if (mTargetAPI != SLANG_DEVELOPMENT_TARGET_API &&
      (mTargetAPI < SLANG_MINIMUM_TARGET_API ||
       mTargetAPI > SLANG_MAXIMUM_TARGET_API)) {
    unsigned DiagID = mDiagEngine->getCustomDiagID(
        clang::DiagnosticsEngine::Error,
        "target API level '%0' is out of range ('%1' - '%2')");
    getDiagnostics().Report(DiagID) << mTargetAPI << SLANG_MINIMUM_TARGET_API
                                    << SLANG_MAXIMUM_TARGET_API;
    return false;
  }

  if (mTargetAPI >= SLANG_M_TARGET_API) {
    LangOpts.NativeHalfArgsAndReturns = 1;
    LangOpts.NativeHalfType = 1;
    LangOpts.HalfArgsAndReturns = 1;
  }

  mVerbose = Opts.mVerbose;

  // Skip generation of warnings a second time if we are doing more than just
  // a single pass over the input file.
  bool SuppressAllWarnings = (Opts.mOutputType != Slang::OT_Dependency);

  bool doReflection = true;
  if (Opts.mEmit3264 && (Opts.mBitWidth == 32)) {
    // Skip reflection on the 32-bit path if we are going to emit it on the
    // 64-bit path.
    doReflection = false;
  }

  std::list<std::pair<const char*, const char*> >::const_iterator
      IOFile64Iter = IOFiles64.begin(),
      IOFile32Iter = IOFiles32.begin(),
      DepFileIter = DepFiles.begin();

  ReflectionState::Tentative TentativeRState(RState);
  if (Opts.mEmit3264) {
    if (Opts.mBitWidth == 32)
      RState->openJava32(IOFiles32.size());
    else {
      slangAssert(Opts.mBitWidth == 64);
      RState->openJava64();
    }
  }

  for (unsigned i = 0, e = IOFiles32.size(); i != e; i++) {
    InputFile = IOFile64Iter->first;
    Output64File = IOFile64Iter->second;
    Output32File = IOFile32Iter->second;

    if (!setInputSource(InputFile))
      return false;

    if (!setOutput(Output64File))
      return false;

    // For use with 64-bit compilation/reflection. This only sets the filename of
    // the 32-bit bitcode file, and doesn't actually verify it already exists.
    mOutput32FileName = Output32File;

    mIsFilterscript = isFilterscript(InputFile);

    CodeGenOpts.MainFileName = mInputFileName;

    if (Slang::compile(Opts) > 0)
      return false;

    if (!Opts.mJavaReflectionPackageName.empty()) {
      mRSContext->setReflectJavaPackageName(Opts.mJavaReflectionPackageName);
    }
    const std::string &RealPackageName =
        mRSContext->getReflectJavaPackageName();

    if (Opts.mOutputType != Slang::OT_Dependency) {

      if (Opts.mBitcodeStorage == BCST_CPP_CODE) {
        if (doReflection) {
          const std::string &outputFileName = (Opts.mBitWidth == 64) ?
              mOutputFileName : mOutput32FileName;
          RSReflectionCpp R(mRSContext, Opts.mJavaReflectionPathBase,
                            getInputFileName(), outputFileName);
          if (!R.reflect()) {
            return false;
          }
        }
      } else {
        if (!Opts.mRSPackageName.empty()) {
          mRSContext->setRSPackageName(Opts.mRSPackageName);
        }

        std::vector<std::string> generatedFileNames;
        RSReflectionJava R(mRSContext, &generatedFileNames,
                           Opts.mJavaReflectionPathBase, getInputFileName(),
                           mOutputFileName,
                           Opts.mBitcodeStorage == BCST_JAVA_CODE,
                           RState);
        if (!R.reflect()) {
          // TODO Is this needed or will the error message have been printed
          // already? and why not for the C++ case?
          fprintf(stderr, "RSContext::reflectToJava : failed to do reflection "
                          "(%s)\n",
                  R.getLastError());
          return false;
        }

        if (doReflection) {
          for (std::vector<std::string>::const_iterator
                   I = generatedFileNames.begin(), E = generatedFileNames.end();
               I != E;
               I++) {
            std::string ReflectedName = RSSlangReflectUtils::ComputePackagedPath(
                Opts.mJavaReflectionPathBase.c_str(),
                (RealPackageName + OS_PATH_SEPARATOR_STR + *I).c_str());
            appendGeneratedFileName(ReflectedName + ".java");
          }

          if ((Opts.mOutputType == Slang::OT_Bitcode) &&
              (Opts.mBitcodeStorage == BCST_JAVA_CODE) &&
              !generateJavaBitcodeAccessor(Opts.mJavaReflectionPathBase,
                                           RealPackageName,
                                           mRSContext->getLicenseNote())) {
            return false;
          }
        }
      }
    }

    if (Opts.mEmitDependency) {
      BCOutputFile = DepFileIter->first;
      DepOutputFile = DepFileIter->second;

      setDepTargetBC(BCOutputFile);

      if (!setDepOutput(DepOutputFile))
        return false;

      if (SuppressAllWarnings) {
        getDiagnostics().setSuppressAllDiagnostics(true);
      }
      if (generateDepFile(Opts.mEmitPhonyDependency) > 0)
        return false;
      if (SuppressAllWarnings) {
        getDiagnostics().setSuppressAllDiagnostics(false);
      }

      DepFileIter++;
    }

    if (!checkODR(InputFile))
      return false;

    IOFile64Iter++;
    IOFile32Iter++;
  }

  if (Opts.mEmit3264) {
    if (Opts.mBitWidth == 32)
      RState->closeJava32();
    else {
      slangAssert(Opts.mBitWidth == 64);
      RState->closeJava64();
    }
  }
  TentativeRState.ok();

  return true;
}

}  // namespace slang
