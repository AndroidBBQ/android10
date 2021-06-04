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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_H_

#include <cstdio>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "llvm/ADT/StringMap.h"

#include "slang_rs_reflect_utils.h"
#include "slang_version.h"

// Terrible workaround for TargetOptions.h not using llvm::RefCountedBase!
#include "llvm/ADT/IntrusiveRefCntPtr.h"
using llvm::RefCountedBase;

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "clang/Lex/ModuleLoader.h"

#include "llvm/ADT/StringRef.h"

#include "llvm/Target/TargetMachine.h"

#include "slang_diagnostic_buffer.h"
#include "slang_pragma_list.h"

namespace llvm {
  class tool_output_file;
}

namespace clang {
  class ASTConsumer;
  class ASTContext;
  class Backend;
  class CodeGenOptions;
  class Diagnostic;
  class DiagnosticsEngine;
  class FileManager;
  class FileSystemOptions;
  class HeaderSearchOptions;
  class LangOptions;
  class PCHContainerOperations;
  class Preprocessor;
  class PreprocessorOptions;
  class SourceManager;
  class TargetInfo;
}  // namespace clang

namespace slang {

class ReflectionState;
class RSCCOptions;
class RSContext;
class RSExportRecordType;

llvm::LLVMContext &getGlobalLLVMContext();

class Slang : public clang::ModuleLoader {
 public:
  enum OutputType {
    OT_Dependency,
    OT_Assembly,
    OT_LLVMAssembly,
    OT_Bitcode,
    OT_Nothing,
    OT_Object,

    OT_Default = OT_Bitcode
  };

 private:
  // Language options (define the language feature for compiler such as C99)
  clang::LangOptions LangOpts;
  // Code generation options for the compiler
  clang::CodeGenOptions CodeGenOpts;

  // Returns true if this is a Filterscript file.
  static bool isFilterscript(const char *Filename);

  // Diagnostics Engine (Producer and Diagnostics Reporter)
  clang::DiagnosticsEngine *mDiagEngine;

  // Diagnostics Consumer
  // NOTE: The ownership is taken by mDiagEngine after creation.
  DiagnosticBuffer *mDiagClient;

  // The target being compiled for
  std::shared_ptr<clang::TargetOptions> mTargetOpts;
  std::unique_ptr<clang::TargetInfo> mTarget;
  void createTarget(uint32_t BitWidth);

  // File manager (for prepocessor doing the job such as header file search)
  std::unique_ptr<clang::FileManager> mFileMgr;
  std::unique_ptr<clang::FileSystemOptions> mFileSysOpt;
  void createFileManager();

  // Source manager (responsible for the source code handling)
  std::unique_ptr<clang::SourceManager> mSourceMgr;
  void createSourceManager();

  // Preprocessor (source code preprocessor)
  std::unique_ptr<clang::Preprocessor> mPP;
  void createPreprocessor();

  // AST context (the context to hold long-lived AST nodes)
  std::unique_ptr<clang::ASTContext> mASTContext;
  void createASTContext();

  // AST consumer, responsible for code generation
  std::unique_ptr<clang::ASTConsumer> mBackend;

  // Options for includes
  llvm::IntrusiveRefCntPtr<clang::HeaderSearchOptions> mHSOpts;

  // Options for the preprocessor (but not header includes)
  llvm::IntrusiveRefCntPtr<clang::PreprocessorOptions> mPPOpts;

  // Module provider (probably not necessary, but keeps us more consistent
  // with regular Clang.
  std::shared_ptr<clang::PCHContainerOperations> mPCHContainerOperations;

  // File names
  std::string mInputFileName;
  std::string mOutputFileName;
  std::string mOutput32FileName;

  std::string mDepOutputFileName;
  std::string mDepTargetBCFileName;
  std::vector<std::string> mAdditionalDepTargets;

  OutputType mOT;

  // Output stream
  std::unique_ptr<llvm::tool_output_file> mOS;

  // Dependency output stream
  std::unique_ptr<llvm::tool_output_file> mDOS;

  std::vector<std::string> mIncludePaths;

  // Context for Renderscript
  RSContext *mRSContext;

  bool mAllowRSPrefix;

  unsigned int mTargetAPI;

  bool mVerbose;

  bool mIsFilterscript;

  // Collect generated filenames (without the .java) for dependency generation
  std::vector<std::string> mGeneratedFileNames;

  PragmaList mPragmas;

  // FIXME: Should be std::list<RSExportable *> here. But currently we only
  //        check ODR on record type.
  //
  // ReflectedDefinitions maps record type name to a pair:
  //  <its RSExportRecordType instance,
  //   the first file contains this record type definition>
  typedef std::pair<RSExportRecordType*, const char*> ReflectedDefinitionTy;
  typedef llvm::StringMap<ReflectedDefinitionTy> ReflectedDefinitionListTy;
  ReflectedDefinitionListTy ReflectedDefinitions;

  bool generateJavaBitcodeAccessor(const std::string &OutputPathBase,
                                   const std::string &PackageName,
                                   const std::string *LicenseNote);

  // CurInputFile is the pointer to a char array holding the input filename
  // and is valid before compile() ends.
  bool checkODR(const char *CurInputFile);

  clang::DiagnosticsEngine &getDiagnostics() { return *mDiagEngine; }
  clang::TargetInfo const &getTargetInfo() const { return *mTarget; }
  clang::FileManager &getFileManager() { return *mFileMgr; }
  clang::SourceManager &getSourceManager() { return *mSourceMgr; }
  clang::Preprocessor &getPreprocessor() { return *mPP; }
  clang::ASTContext &getASTContext() { return *mASTContext; }
  clang::HeaderSearchOptions &getHeaderSearchOpts() { return *mHSOpts; }
  clang::PreprocessorOptions &getPreprocessorOpts() { return *mPPOpts; }

  inline clang::TargetOptions const &getTargetOptions() const
    { return *mTargetOpts.get(); }

  void initPreprocessor();
  void initASTContext();

  clang::ASTConsumer *createBackend(const RSCCOptions &Opts,
                                    const clang::CodeGenOptions &CodeGenOpts,
                                    llvm::raw_ostream *OS,
                                    OutputType OT);

 public:
  static const llvm::StringRef PragmaMetadataName;

  static void GlobalInitialization();

  static bool IsRSHeaderFile(const char *File);
  // FIXME: Determine whether a location is in RS header (i.e., one of the RS
  //        built-in APIs) should only need its names (we need a "list" of RS
  //        built-in APIs).
  static bool IsLocInRSHeaderFile(const clang::SourceLocation &Loc,
                                  const clang::SourceManager &SourceMgr);

  Slang(uint32_t BitWidth, clang::DiagnosticsEngine *DiagEngine,
        DiagnosticBuffer *DiagClient);

  virtual ~Slang();

  bool setInputSource(llvm::StringRef InputFile);

  std::string const &getInputFileName() const { return mInputFileName; }

  void setIncludePaths(const std::vector<std::string> &IncludePaths) {
    mIncludePaths = IncludePaths;
  }

  void setOutputType(OutputType OT) { mOT = OT; }

  bool setOutput(const char *OutputFile);

  bool setDepOutput(const char *OutputFile);

  void setDepTargetBC(const char *TargetBCFile) {
    mDepTargetBCFileName = TargetBCFile;
  }

  void setAdditionalDepTargets(
      std::vector<std::string> const &AdditionalDepTargets) {
    mAdditionalDepTargets = AdditionalDepTargets;
  }

  void appendGeneratedFileName(std::string const &GeneratedFileName) {
    mGeneratedFileNames.push_back(GeneratedFileName);
  }

  int generateDepFile(bool PhonyTarget);

  int compile(const RSCCOptions &Opts);

  char const *getErrorMessage() { return mDiagClient->str().c_str(); }

  void setDebugMetadataEmission(bool EmitDebug);

  void setOptimizationLevel(llvm::CodeGenOpt::Level OptimizationLevel);

  // Compile bunch of RS files given in the llvm-rs-cc arguments. Return true if
  // all given input files are successfully compiled without errors.
  //
  // @IOFiles - List of pairs of <input file path, output file path>.
  //
  // @DepFiles - List of pairs of <output dep. file path, dependent bitcode
  //             target>. If @OutputDep is true, this parameter must be given
  //             with the same number of pairs given in @IOFiles.
  //
  // @Opts - Selection of options defined from invoking llvm-rs-cc
  //
  // @Reflection - Carries reflection information from 32-bit compile to 64-bit compile.
  bool
  compile(const std::list<std::pair<const char *, const char *>> &IOFiles64,
          const std::list<std::pair<const char *, const char *>> &IOFiles32,
          const std::list<std::pair<const char *, const char *>> &DepFiles,
          const RSCCOptions &Opts,
          clang::DiagnosticOptions &DiagOpts,
          ReflectionState *Reflection);

  clang::ModuleLoadResult loadModule(clang::SourceLocation ImportLoc,
                                     clang::ModuleIdPath Path,
                                     clang::Module::NameVisibilityKind VK,
                                     bool IsInclusionDirective) override;

  void makeModuleVisible(clang::Module *Mod,
                         clang::Module::NameVisibilityKind Visibility,
                         clang::SourceLocation ImportLoc) override {}

  clang::GlobalModuleIndex *
  loadGlobalModuleIndex(clang::SourceLocation TriggerLoc) override {
    // We don't support C++ modules for RenderScript.
    return nullptr;
  }

  bool lookupMissingImports(llvm::StringRef Name,
                            clang::SourceLocation TriggerLoc) override {
    // We don't support C++ modules for RenderScript.
    return false;
  }
};

} // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_H_  NOLINT
