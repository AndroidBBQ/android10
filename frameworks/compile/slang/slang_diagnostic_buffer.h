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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_DIAGNOSTIC_BUFFER_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_DIAGNOSTIC_BUFFER_H_

#include <set>
#include <string>

#include "clang/Basic/Diagnostic.h"

#include "llvm/Support/raw_ostream.h"

namespace llvm {
  class raw_string_ostream;
}

namespace slang {

// The diagnostics consumer instance (for reading the processed diagnostics)
class DiagnosticBuffer : public clang::DiagnosticConsumer {
private:
  // We keed track of the messages that have been already added to this
  // diagnostic buffer, to avoid duplicates.  This can happen because for a
  // given script we'll usually compile for both 32 and 64 bit targets.
  std::set<std::string> mIncludedMessages;
  std::string mDiags;
  std::unique_ptr<llvm::raw_string_ostream> mSOS;

public:
  DiagnosticBuffer();
  virtual ~DiagnosticBuffer();

  virtual void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
                                const clang::Diagnostic &Info) override;

  inline const std::string &str() const {
    mSOS->flush();
    return mDiags;
  }

  inline void reset() {
    mIncludedMessages.clear();
    mSOS.reset();
    mDiags.clear();
  }
};

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_DIAGNOSTIC_BUFFER_H_  NOLINT
