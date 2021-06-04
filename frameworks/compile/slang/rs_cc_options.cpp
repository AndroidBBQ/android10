/*
 * Copyright 2014, The Android Open Source Project
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
#include "clang/Frontend/Utils.h"

#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/CommandLine.h"

#include "rs_cc_options.h"
#include "slang.h"
#include "slang_assert.h"

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

enum {
  OPT_INVALID = 0,  // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)                                             \
  OPT_##ID,
#include "RSCCOptions.inc"
  LastOption
#undef OPTION
#undef PREFIX
};

#define PREFIX(NAME, VALUE) const char *const NAME[] = VALUE;
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)
#include "RSCCOptions.inc"
#undef OPTION
#undef PREFIX

static const llvm::opt::OptTable::Info RSCCInfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR)                                              \
  {                                                                            \
    PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID, llvm::opt::Option::KIND##Class, \
        PARAM, FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS                      \
  }                                                                            \
  ,
#include "RSCCOptions.inc"
#undef OPTION
#undef PREFIX
};

namespace {

class RSCCOptTable : public llvm::opt::OptTable {
 public:
  RSCCOptTable()
      : OptTable(llvm::makeArrayRef(RSCCInfoTable)) {}
};
}

namespace slang {

llvm::opt::OptTable *createRSCCOptTable() { return new RSCCOptTable(); }

// This function is similar to
// clang/lib/Frontend/CompilerInvocation::CreateFromArgs.
bool ParseArguments(const llvm::ArrayRef<const char *> &ArgsIn,
                    llvm::SmallVectorImpl<const char *> &Inputs,
                    RSCCOptions &Opts, clang::DiagnosticOptions &DiagOpts,
                    llvm::StringSaver &StringSaver) {
  // We use a different diagnostic engine for argument parsing from the rest of
  // the work.  This mimics what's done in clang.  I believe it is so the
  // argument parsing errors are well formatted while the full errors can be
  // influenced by command line arguments.
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> ArgumentParseDiagOpts(
      new clang::DiagnosticOptions());
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIDs(
      new clang::DiagnosticIDs());
  DiagnosticBuffer DiagsBuffer;
  clang::DiagnosticsEngine DiagEngine(DiagIDs, &*ArgumentParseDiagOpts,
                                      &DiagsBuffer, false);

  // Populate a vector with the command line arguments, expanding command files
  // that have been included via the '@' argument.
  llvm::SmallVector<const char *, 256> ArgVector;
  // Skip over the command name, or we will mistakenly process it as a source file.
  ArgVector.append(ArgsIn.slice(1).begin(), ArgsIn.end());
  llvm::cl::ExpandResponseFiles(StringSaver, llvm::cl::TokenizeGNUCommandLine,
                                ArgVector, false);

  std::unique_ptr<llvm::opt::OptTable> OptParser(createRSCCOptTable());
  unsigned MissingArgIndex = 0;
  unsigned MissingArgCount = 0;
  llvm::opt::InputArgList Args =
      OptParser->ParseArgs(ArgVector, MissingArgIndex, MissingArgCount);

  // Check for missing argument error.
  if (MissingArgCount) {
    DiagEngine.Report(clang::diag::err_drv_missing_argument)
        << Args.getArgString(MissingArgIndex) << MissingArgCount;
  }

  // Issue errors on unknown arguments.
  for (llvm::opt::arg_iterator it = Args.filtered_begin(OPT_UNKNOWN),
                               ie = Args.filtered_end();
       it != ie; ++it) {
    DiagEngine.Report(clang::diag::err_drv_unknown_argument)
        << (*it)->getAsString(Args);
  }

  DiagOpts.IgnoreWarnings = Args.hasArg(OPT_w);
  DiagOpts.Warnings = Args.getAllArgValues(OPT_W);

  // Always turn off warnings for empty initializers, since we really want to
  // employ/encourage this extension for zero-initialization of structures.
  DiagOpts.Warnings.push_back("no-gnu-empty-initializer");

  for (llvm::opt::ArgList::const_iterator it = Args.begin(), ie = Args.end();
       it != ie; ++it) {
    const llvm::opt::Arg *A = *it;
    if (A->getOption().getKind() == llvm::opt::Option::InputClass)
      Inputs.push_back(A->getValue());
  }

  Opts.mIncludePaths = Args.getAllArgValues(OPT_I);

  Opts.mBitcodeOutputDir = Args.getLastArgValue(OPT_o);

  if (const llvm::opt::Arg *A = Args.getLastArg(OPT_M_Group)) {
    switch (A->getOption().getID()) {
    case OPT_M: {
      Opts.mEmitDependency = true;
      Opts.mOutputType = Slang::OT_Dependency;
      break;
    }
    case OPT_MD: {
      Opts.mEmitDependency = true;
      Opts.mOutputType = Slang::OT_Bitcode;
      break;
    }
    case OPT_MP: {
      Opts.mEmitDependency = true;
      Opts.mOutputType = Slang::OT_Bitcode;
      Opts.mEmitPhonyDependency = true;
      break;
    }
    default: { slangAssert(false && "Invalid option in M group!"); }
    }
  }

  if (const llvm::opt::Arg *A = Args.getLastArg(OPT_Output_Type_Group)) {
    switch (A->getOption().getID()) {
    case OPT_emit_asm: {
      Opts.mOutputType = Slang::OT_Assembly;
      break;
    }
    case OPT_emit_llvm: {
      Opts.mOutputType = Slang::OT_LLVMAssembly;
      break;
    }
    case OPT_emit_bc: {
      Opts.mOutputType = Slang::OT_Bitcode;
      break;
    }
    case OPT_emit_nothing: {
      Opts.mOutputType = Slang::OT_Nothing;
      break;
    }
    default: { slangAssert(false && "Invalid option in output type group!"); }
    }
  }

  if (Opts.mEmitDependency && ((Opts.mOutputType != Slang::OT_Bitcode) &&
                               (Opts.mOutputType != Slang::OT_Dependency)))
    DiagEngine.Report(clang::diag::err_drv_argument_not_allowed_with)
        << Args.getLastArg(OPT_M_Group)->getAsString(Args)
        << Args.getLastArg(OPT_Output_Type_Group)->getAsString(Args);

  Opts.mAllowRSPrefix = Args.hasArg(OPT_allow_rs_prefix);

  Opts.mJavaReflectionPathBase =
      Args.getLastArgValue(OPT_java_reflection_path_base);
  Opts.mJavaReflectionPackageName =
      Args.getLastArgValue(OPT_java_reflection_package_name);

  Opts.mRSPackageName = Args.getLastArgValue(OPT_rs_package_name);

  llvm::StringRef BitcodeStorageValue =
      Args.getLastArgValue(OPT_bitcode_storage);
  if (BitcodeStorageValue == "ar")
    Opts.mBitcodeStorage = BCST_APK_RESOURCE;
  else if (BitcodeStorageValue == "jc")
    Opts.mBitcodeStorage = BCST_JAVA_CODE;
  else if (!BitcodeStorageValue.empty())
    DiagEngine.Report(clang::diag::err_drv_invalid_value)
        << OptParser->getOptionName(OPT_bitcode_storage) << BitcodeStorageValue;

  llvm::opt::Arg *lastBitwidthArg = Args.getLastArg(OPT_m32, OPT_m64);
  if (Args.hasArg(OPT_reflect_cpp)) {
    Opts.mBitcodeStorage = BCST_CPP_CODE;
    // mJavaReflectionPathBase can be set for C++ reflected builds.
    // Set it to the standard mBitcodeOutputDir (via -o) by default.
    if (Opts.mJavaReflectionPathBase.empty()) {
      Opts.mJavaReflectionPathBase = Opts.mBitcodeOutputDir;
    }

    // Check for bitwidth arguments.
    if (lastBitwidthArg) {
      if (lastBitwidthArg->getOption().matches(OPT_m32)) {
        Opts.mBitWidth = 32;
      } else {
        Opts.mBitWidth = 64;
      }
    }
  } else if (lastBitwidthArg) {
      // -m32/-m64 are forbidden for non-C++ reflection paths for non-eng builds
      // (they would make it too easy for a developer to accidentally create and
      // release an APK that has 32-bit or 64-bit bitcode but not both).
#ifdef __ENABLE_INTERNAL_OPTIONS
      if (lastBitwidthArg->getOption().matches(OPT_m32)) {
        Opts.mBitWidth = 32;
      } else {
        Opts.mBitWidth = 64;
      }
      Opts.mEmit3264 = false;
#else
      DiagEngine.Report(
          DiagEngine.getCustomDiagID(clang::DiagnosticsEngine::Error,
                                     "cannot use -m32/-m64 without specifying "
                                     "C++ reflection (-reflect-c++)"));
#endif
  }

  Opts.mDependencyOutputDir =
      Args.getLastArgValue(OPT_output_dep_dir, Opts.mBitcodeOutputDir);
  Opts.mAdditionalDepTargets = Args.getAllArgValues(OPT_additional_dep_target);

  Opts.mShowHelp = Args.hasArg(OPT_help);
  Opts.mShowVersion = Args.hasArg(OPT_version);
  Opts.mDebugEmission = Args.hasArg(OPT_emit_g);
  Opts.mVerbose = Args.hasArg(OPT_verbose);
  Opts.mASTPrint = Args.hasArg(OPT_ast_print);

  // Delegate options

  std::vector<std::string> DelegatedStrings;
  for (int Opt : std::vector<unsigned>{OPT_debug, OPT_print_after_all, OPT_print_before_all}) {
    if (Args.hasArg(Opt)) {
      // TODO: Don't assume that the option begins with "-"; determine this programmatically instead.
      DelegatedStrings.push_back(std::string("-") + std::string(OptParser->getOptionName(Opt)));
      slangAssert(OptParser->getOptionKind(Opt) == llvm::opt::Option::FlagClass);
    }
  }
  if (DelegatedStrings.size()) {
    std::vector<const char *> DelegatedCStrs;
    DelegatedCStrs.push_back(*ArgVector.data()); // program name
    std::for_each(DelegatedStrings.cbegin(), DelegatedStrings.cend(),
                  [&DelegatedCStrs](const std::string &String) { DelegatedCStrs.push_back(String.c_str()); });
    llvm::cl::ParseCommandLineOptions(DelegatedCStrs.size(), DelegatedCStrs.data());
  }

  // If we are emitting both 32-bit and 64-bit bitcode, we must embed it.

  size_t OptLevel =
      clang::getLastArgIntValue(Args, OPT_optimization_level, 3, DiagEngine);

  Opts.mOptimizationLevel =
      OptLevel == 0 ? llvm::CodeGenOpt::None : llvm::CodeGenOpt::Aggressive;

  Opts.mTargetAPI =
      clang::getLastArgIntValue(Args, OPT_target_api, RS_VERSION, DiagEngine);

  if (Opts.mTargetAPI == 0) {
    Opts.mTargetAPI = UINT_MAX;
  } else if (Opts.mTargetAPI == SLANG_N_MR1_TARGET_API ||
             Opts.mTargetAPI == SLANG_O_TARGET_API ||
             Opts.mTargetAPI == SLANG_O_MR1_TARGET_API ||
             Opts.mTargetAPI == SLANG_P_TARGET_API) {
    // Bug: http://b/35767071
    // No new APIs for N_MR1, O, O_MR1 and P, convert to N.
    Opts.mTargetAPI = SLANG_N_TARGET_API;
  }

  if ((Opts.mTargetAPI < 21) || (Opts.mBitcodeStorage == BCST_CPP_CODE))
    Opts.mEmit3264 = false;
  if (Opts.mEmit3264)
    Opts.mBitcodeStorage = BCST_JAVA_CODE;

  if (DiagEngine.hasErrorOccurred()) {
    llvm::errs() << DiagsBuffer.str();
    return false;
  }

  return true;
}
}
