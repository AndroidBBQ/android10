/*
 * Copyright 2010-2012, The Android Open Source Project
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

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "llvm/Option/OptTable.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "os_sep.h"
#include "rs_cc_options.h"
#include "slang.h"
#include "slang_assert.h"
#include "slang_diagnostic_buffer.h"
#include "slang_rs_reflect_utils.h"
#include "slang_rs_reflection_state.h"

#include <list>
#include <set>
#include <string>

namespace {
class StringSet {
public:
  const char *save(const char *Str) {
    return Strings.save(Str);
  }

  StringSet() : Strings(A), A() {}

  llvm::StringSaver & getStringSaver() { return Strings; }

private:
  llvm::StringSaver Strings;
  llvm::BumpPtrAllocator A;
};
}

static const char *DetermineOutputFile(const std::string &OutputDir,
                                       const std::string &PathSuffix,
                                       const char *InputFile,
                                       slang::Slang::OutputType OutputType,
                                       StringSet *SavedStrings) {
  if (OutputType == slang::Slang::OT_Nothing)
    return "/dev/null";

  std::string OutputFile(OutputDir);

  // Append '/' to Opts.mBitcodeOutputDir if not presents
  if (!OutputFile.empty() &&
      (OutputFile[OutputFile.size() - 1]) != OS_PATH_SEPARATOR)
    OutputFile.append(1, OS_PATH_SEPARATOR);

  if (!PathSuffix.empty()) {
    OutputFile.append(PathSuffix);
    OutputFile.append(1, OS_PATH_SEPARATOR);
  }

  if (OutputType == slang::Slang::OT_Dependency) {
    // The build system wants the .d file name stem to be exactly the same as
    // the source .rs file, instead of the .bc file.
    OutputFile.append(slang::RSSlangReflectUtils::GetFileNameStem(InputFile));
  } else {
    OutputFile.append(
        slang::RSSlangReflectUtils::BCFileNameFromRSFileName(InputFile));
  }

  switch (OutputType) {
    case slang::Slang::OT_Dependency: {
      OutputFile.append(".d");
      break;
    }
    case slang::Slang::OT_Assembly: {
      OutputFile.append(".S");
      break;
    }
    case slang::Slang::OT_LLVMAssembly: {
      OutputFile.append(".ll");
      break;
    }
    case slang::Slang::OT_Object: {
      OutputFile.append(".o");
      break;
    }
    case slang::Slang::OT_Bitcode: {
      OutputFile.append(".bc");
      break;
    }
    case slang::Slang::OT_Nothing:
    default: {
      slangAssert(false && "Invalid output type!");
    }
  }

  return SavedStrings->save(OutputFile.c_str());
}

typedef std::list<std::pair<const char*, const char*> > NamePairList;

/*
 * Compile the Inputs.
 *
 * Returns 0 on success and nonzero on failure.
 *
 * IOFiles - list of (foo.rs, foo.bc) pairs of input/output files.
 * IOFiles32 - list of input/output pairs for 32-bit compilation.
 * Inputs - input filenames.
 * Opts - options controlling compilation.
 * DiagEngine - Clang diagnostic engine (for creating diagnostics).
 * DiagClient - Slang diagnostic consumer (collects and displays diagnostics).
 * SavedStrings - expanded strings copied from argv source input files.
 *
 * We populate IOFiles dynamically while working through the list of Inputs.
 * On any 64-bit compilation, we pass back in the 32-bit pairs of files as
 * IOFiles32. This allows the 64-bit compiler to later bundle up both the
 * 32-bit and 64-bit bitcode outputs to be included in the final reflected
 * source code that is emitted.
 */
static void makeFileList(NamePairList *IOFiles, NamePairList *DepFiles,
    const llvm::SmallVector<const char*, 16> &Inputs, slang::RSCCOptions &Opts,
    StringSet *SavedStrings) {
  std::string PathSuffix = "";
  // In our mixed 32/64-bit path, we need to suffix our files differently for
  // both 32-bit and 64-bit versions.
  if (Opts.mEmit3264) {
    if (Opts.mBitWidth == 64) {
      PathSuffix = "bc64";
    } else {
      PathSuffix = "bc32";
    }
  }

  for (int i = 0, e = Inputs.size(); i != e; i++) {
    const char *InputFile = Inputs[i];

    const char *BCOutputFile = DetermineOutputFile(Opts.mBitcodeOutputDir,
                                                   PathSuffix, InputFile,
                                                   Opts.mOutputType,
                                                   SavedStrings);
    const char *OutputFile = BCOutputFile;

    if (Opts.mEmitDependency) {
      // The dependency file is always emitted without a PathSuffix.
      // Collisions between 32-bit and 64-bit files don't make a difference,
      // because they share the same sources/dependencies.
      const char *DepOutputFile =
          DetermineOutputFile(Opts.mDependencyOutputDir, "", InputFile,
                              slang::Slang::OT_Dependency, SavedStrings);
      if (Opts.mOutputType == slang::Slang::OT_Dependency) {
        OutputFile = DepOutputFile;
      }

      DepFiles->push_back(std::make_pair(BCOutputFile, DepOutputFile));
    }

    IOFiles->push_back(std::make_pair(InputFile, OutputFile));
  }
}

#define str(s) #s
#define wrap_str(s) str(s)
static void llvm_rs_cc_VersionPrinter() {
  llvm::raw_ostream &OS = llvm::outs();
  OS << "llvm-rs-cc: Renderscript compiler\n"
     << "  (http://developer.android.com/guide/topics/renderscript)\n"
     << "  based on LLVM (http://llvm.org):\n";
  OS << "  Target APIs: " << SLANG_MINIMUM_TARGET_API << " - "
     << SLANG_MAXIMUM_TARGET_API;
  OS << "\n  Build type: " << wrap_str(TARGET_BUILD_VARIANT);
#ifndef __DISABLE_ASSERTS
  OS << " with assertions";
#endif
  OS << ".\n";
}
#undef wrap_str
#undef str

static void LLVMErrorHandler(void *UserData, const std::string &Message,
                             bool GenCrashDialog) {
  clang::DiagnosticsEngine *DiagEngine =
      static_cast<clang::DiagnosticsEngine *>(UserData);

  DiagEngine->Report(clang::diag::err_fe_error_backend) << Message;

  // Run the interrupt handlers to make sure any special cleanups get done, in
  // particular that we remove files registered with RemoveFileOnSignal.
  llvm::sys::RunInterruptHandlers();

  exit(1);
}

int main(int argc, const char **argv) {
  llvm::llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.
  LLVMInitializeARMTargetInfo();
  LLVMInitializeARMTarget();
  LLVMInitializeARMAsmPrinter();

  StringSet SavedStrings; // Keeps track of strings to be destroyed at the end.

  // Parse the command line arguments and respond to show help & version
  // commands.
  llvm::SmallVector<const char *, 16> Inputs;
  slang::RSCCOptions Opts;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts =
      new clang::DiagnosticOptions();
  if (!slang::ParseArguments(llvm::makeArrayRef(argv, argc), Inputs, Opts,
                             *DiagOpts, SavedStrings.getStringSaver())) {
    // Exits when there's any error occurred during parsing the arguments
    return 1;
  }
  if (Opts.mShowHelp) {
    std::unique_ptr<llvm::opt::OptTable> OptTbl(slang::createRSCCOptTable());
    const std::string Argv0 = llvm::sys::path::stem(argv[0]);
    OptTbl->PrintHelp(llvm::outs(), Argv0.c_str(),
                      "Renderscript source compiler");
    return 0;
  }
  if (Opts.mShowVersion) {
    llvm_rs_cc_VersionPrinter();
    return 0;
  }

  // Initialize the diagnostic objects
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIDs(
      new clang::DiagnosticIDs());
  slang::DiagnosticBuffer DiagsBuffer;
  clang::DiagnosticsEngine DiagEngine(DiagIDs, &*DiagOpts, &DiagsBuffer, false);
  clang::ProcessWarningOptions(DiagEngine, *DiagOpts);
  (void)DiagEngine.setSeverityForGroup(clang::diag::Flavor::WarningOrError,
                                       "implicit-function-declaration",
                                       clang::diag::Severity::Error);

  // Report error if no input file
  if (Inputs.empty()) {
    DiagEngine.Report(clang::diag::err_drv_no_input_files);
    llvm::errs() << DiagsBuffer.str();
    return 1;
  }

  llvm::install_fatal_error_handler(LLVMErrorHandler, &DiagEngine);

  slang::ReflectionState Reflection;

  // Compile the 32 bit version
  NamePairList IOFiles32;
  NamePairList DepFiles32;
  makeFileList(&IOFiles32, &DepFiles32, Inputs, Opts, &SavedStrings);

  int CompileFailed = 0;
  // Handle 32-bit case for Java and C++ reflection.
  // For Java, both 32bit and 64bit will be generated.
  // For C++, either 64bit or 32bit will be generated based on the target.
  if (Opts.mEmit3264 || Opts.mBitWidth == 32) {
      std::unique_ptr<slang::Slang> Compiler(
          new slang::Slang(32, &DiagEngine, &DiagsBuffer));
      CompileFailed =
          !Compiler->compile(IOFiles32, IOFiles32, DepFiles32, Opts, *DiagOpts, &Reflection);
  }

  // Handle the 64-bit case too!
  bool needEmit64 = Opts.mEmit3264 || Opts.mBitWidth == 64;
  if (needEmit64 && !CompileFailed) {
    Opts.mBitWidth = 64;
    NamePairList IOFiles64;
    NamePairList DepFiles64;
    makeFileList(&IOFiles64, &DepFiles64, Inputs, Opts, &SavedStrings);

    std::unique_ptr<slang::Slang> Compiler(
        new slang::Slang(64, &DiagEngine, &DiagsBuffer));
    CompileFailed =
        !Compiler->compile(IOFiles64, IOFiles32, DepFiles64, Opts, *DiagOpts, &Reflection);
  }

  llvm::errs() << DiagsBuffer.str();
  llvm::remove_fatal_error_handler();
  return CompileFailed;
}
