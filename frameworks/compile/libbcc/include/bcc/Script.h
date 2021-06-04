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

#ifndef BCC_SCRIPT_H
#define BCC_SCRIPT_H

#include "slang_version.h"

#include <llvm/Support/CodeGen.h>
#include "bcc/Source.h"

namespace llvm {
class Module;
}

namespace bcc {

class Script;
class Source;
class CompilerConfig;

typedef llvm::Module *(*RSLinkRuntimeCallback)(bcc::Script *, llvm::Module *,
                                               llvm::Module *);

class Script {
private:
  // This is the source associated with this object and is going to be
  // compiled.
  // TODO(jeanluc) Verify that the lifetime is managed correctly.
  Source *mSource;

  llvm::CodeGenOpt::Level mOptimizationLevel;

  RSLinkRuntimeCallback mLinkRuntimeCallback;

  bool mEmbedInfo;

  // Specifies whether we should embed global variable information in the
  // code via special RS variables that can be examined later by the driver.
  bool mEmbedGlobalInfo;

  // Specifies whether we should skip constant (immutable) global variables
  // when potentially embedding information about globals.
  bool mEmbedGlobalInfoSkipConstant;

public:
  explicit Script(Source *pSource);

  ~Script() {}

  bool LinkRuntime(const char *rt_path);

  unsigned getCompilerVersion() const {
    return getSource().getCompilerVersion();
  }

  bool isStructExplicitlyPaddedBySlang() const {
    return getCompilerVersion() >= SlangVersion::N_STRUCT_EXPLICIT_PADDING;
  }

  void setOptimizationLevel(llvm::CodeGenOpt::Level pOptimizationLevel) {
    mOptimizationLevel = pOptimizationLevel;
  }

  llvm::CodeGenOpt::Level getOptimizationLevel() const {
    return mOptimizationLevel;
  }

  void setLinkRuntimeCallback(RSLinkRuntimeCallback fn) {
    mLinkRuntimeCallback = fn;
  }

  void setEmbedInfo(bool pEnable) { mEmbedInfo = pEnable; }

  bool getEmbedInfo() const { return mEmbedInfo; }

  // Set to true if we should embed global variable information in the code.
  void setEmbedGlobalInfo(bool pEnable) { mEmbedGlobalInfo = pEnable; }

  // Returns true if we should embed global variable information in the code.
  bool getEmbedGlobalInfo() const { return mEmbedGlobalInfo; }

  // Set to true if we should skip constant (immutable) global variables when
  // potentially embedding information about globals.
  void setEmbedGlobalInfoSkipConstant(bool pEnable) {
    mEmbedGlobalInfoSkipConstant = pEnable;
  }

  // Returns true if we should skip constant (immutable) global variables when
  // potentially embedding information about globals.
  inline bool getEmbedGlobalInfoSkipConstant() const {
    return mEmbedGlobalInfoSkipConstant;
  }

  // Merge (or link) another source into the current source associated with
  // this Script object. Return false on error.
  //
  // This is equivalent to the call to Script::merge(...) on mSource.
  bool mergeSource(Source &pSource);

  inline Source &getSource() { return *mSource; }
  inline const Source &getSource() const { return *mSource; }
};

} // end namespace bcc

#endif // BCC_SCRIPT_H
