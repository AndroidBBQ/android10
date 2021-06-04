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

#ifndef _FRAMEWORKS_COMPILE_SLANG_RS_CC_OPTIONS_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_RS_CC_OPTIONS_H_

#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Option/OptTable.h"

#include "slang.h"
#include "slang_rs_reflect_utils.h"

#include <string>
#include <vector>

namespace llvm {
namespace cl {
class StringSaver;
}
namespace opt {
class OptTable;
}
}

namespace slang {

// Options for the RenderScript compiler llvm-rs-cc
class RSCCOptions {
 public:
  // User-defined include paths.
  std::vector<std::string> mIncludePaths;

  // The output directory for writing the bitcode files
  // (i.e. out/target/common/obj/APPS/.../src/renderscript/res/raw).
  std::string mBitcodeOutputDir;

  // Type of file to emit (bitcode, dependency, ...).
  slang::Slang::OutputType mOutputType;

  // Allow user-defined functions prefixed with 'rs'.
  bool mAllowRSPrefix;

  // 32-bit or 64-bit target
  uint32_t mBitWidth;

  // The path for storing reflected Java source files
  // (i.e. out/target/common/obj/APPS/.../src/renderscript/src).
  std::string mJavaReflectionPathBase;

  // Force package name. This may override the package name specified by a
  // #pragma in the .rs file.
  std::string mJavaReflectionPackageName;

  // Force the RS package name to use. This can override the default value of
  // "android.renderscript" used for the standard RS APIs.
  std::string mRSPackageName;

  // Where to store the generated bitcode (resource, Java source, C++ source).
  slang::BitCodeStorageType mBitcodeStorage;

  // Emit output dependency file for each input file.
  bool mEmitDependency;

  // Emit phony targets for each header dependency, which can avoid make errors
  // when the header gets deleted. See -MP option of cc.
  bool mEmitPhonyDependency;

  // The output directory for writing dependency files
  // (i.e. out/target/common/obj/APPS/.../src/renderscript).
  std::string mDependencyOutputDir;

  // User-defined files added to the dependencies (i.e. for adding fake
  // dependency files like RenderScript.stamp).
  std::vector<std::string> mAdditionalDepTargets;

  bool mShowHelp;     // Show the -help text.
  bool mShowVersion;  // Show the -version text.

  // The target API we are generating code for (see slang_version.h).
  unsigned int mTargetAPI;

  // Enable emission of debugging symbols.
  bool mDebugEmission;

  // The optimization level used in CodeGen, and encoded in emitted bitcode.
  llvm::CodeGenOpt::Level mOptimizationLevel;

  // Display verbose information about the compilation on stdout.
  bool mVerbose;

  // Display AST.
  bool mASTPrint;

  // Emit both 32-bit and 64-bit bitcode (embedded in the reflected sources).
  bool mEmit3264;

  RSCCOptions() {
    mOutputType = slang::Slang::OT_Bitcode;
    mBitWidth = 32;
    mBitcodeStorage = slang::BCST_APK_RESOURCE;
    mEmitDependency = 0;
    mEmitPhonyDependency = 0;
    mShowHelp = 0;
    mShowVersion = 0;
    mTargetAPI = RS_VERSION;
    mDebugEmission = 0;
    mOptimizationLevel = llvm::CodeGenOpt::Aggressive;
    mVerbose = false;
    mASTPrint = false;
    mEmit3264 = true;
  }
};

/* Return a valid OptTable (useful for dumping help information)
 */
llvm::opt::OptTable *createRSCCOptTable();

/* Parse ArgVector and return a list of Inputs (source files) and Opts
 * (options affecting the compilation of those source files).
 *
 * \param ArgVector - the input arguments to llvm-rs-cc
 * \param Inputs - returned list of actual input source filenames
 * \param Opts - returned options after command line has been processed
 * \param DiagEngine - input for issuing warnings/errors on arguments
 */

bool ParseArguments(const llvm::ArrayRef<const char *> &ArgsIn,
                    llvm::SmallVectorImpl<const char *> &Inputs,
                    RSCCOptions &Opts, clang::DiagnosticOptions &DiagOpts,
                    llvm::StringSaver &StringSaver);

} // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_RS_CC_OPTIONS_H_
