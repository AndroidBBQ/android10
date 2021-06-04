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

#include "slang_diagnostic_buffer.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/ADT/SmallString.h"

#include "slang_assert.h"

namespace slang {

DiagnosticBuffer::DiagnosticBuffer()
  : mSOS(new llvm::raw_string_ostream(mDiags)) {
}

DiagnosticBuffer::~DiagnosticBuffer() {
}

void DiagnosticBuffer::HandleDiagnostic(
    clang::DiagnosticsEngine::Level DiagLevel,
    clang::Diagnostic const &Info) {
  clang::SourceLocation const &SrcLoc = Info.getLocation();

  std::string Message;
  llvm::raw_string_ostream stream(Message);

  if (SrcLoc.isValid()) {
    SrcLoc.print(stream, Info.getSourceManager());
    stream << ": ";
  }

  switch (DiagLevel) {
    case clang::DiagnosticsEngine::Note: {
      stream << "note: ";
      break;
    }
    case clang::DiagnosticsEngine::Warning: {
      stream << "warning: ";
      break;
    }
    case clang::DiagnosticsEngine::Error: {
      stream << "error: ";
      break;
    }
    case clang::DiagnosticsEngine::Fatal: {
      stream << "fatal: ";
      break;
    }
    default: {
      slangAssert(0 && "Diagnostic not handled during diagnostic buffering!");
    }
  }
  // 100 is enough for storing general diagnosis Message
  llvm::SmallString<100> Buf;
  Info.FormatDiagnostic(Buf);
  stream << Buf.str() << '\n';
  stream.flush();

  if (mIncludedMessages.find(Message) == mIncludedMessages.end()) {
    mIncludedMessages.insert(Message);
    (*mSOS) << Message;
  }
}

}  // namespace slang
