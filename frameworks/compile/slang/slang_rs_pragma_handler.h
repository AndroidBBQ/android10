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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_PRAGMA_HANDLER_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_PRAGMA_HANDLER_H_

#include <string>

#include "clang/Lex/Pragma.h"

namespace clang {
  class Token;
  class IdentifierInfo;
  class Preprocessor;
}

namespace slang {

  class RSContext;

class RSPragmaHandler : public clang::PragmaHandler {
 protected:
  RSContext *mContext;

  RSPragmaHandler(llvm::StringRef Name, RSContext *Context)
      : clang::PragmaHandler(Name),
        mContext(Context) {
  }
  RSContext *getContext() const {
    return this->mContext;
  }

  virtual void handleItem(const std::string &Item) { }
  virtual void handleInt(clang::Preprocessor &PP, clang::Token &Tok,
                         const int v) { }

  // Handle pragma like #pragma rs [name] ([item #1],[item #2],...,[item #i])
  void handleItemListPragma(clang::Preprocessor &PP,
                            clang::Token &FirstToken);

  // Handle pragma like #pragma rs [name]
  void handleNonParamPragma(clang::Preprocessor &PP,
                            clang::Token &FirstToken);

  // Handle pragma like #pragma rs [name] ("string literal")
  void handleOptionalStringLiteralParamPragma(clang::Preprocessor &PP,
                                              clang::Token &FirstToken);

  // Handle pragma like #pragma version (integer)
  void handleIntegerParamPragma(clang::Preprocessor &PP,
                                clang::Token &FirstToken);

 public:
  virtual void HandlePragma(clang::Preprocessor &PP,
                            clang::PragmaIntroducerKind Introducer,
                            clang::Token &FirstToken) = 0;
};

// Add handlers for the RS pragmas to the preprocessor.  These handlers
// validate the pragmas and, if valid, set fields of the RSContext.
void AddPragmaHandlers(clang::Preprocessor &PP, RSContext *RsContext);

}   // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_PRAGMA_HANDLER_H_  NOLINT
