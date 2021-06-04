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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_VAR_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_VAR_H_

#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"

#include "llvm/ADT/StringRef.h"

#include "slang_assert.h"
#include "slang_rs_exportable.h"

namespace slang {
  class RSContext;
  class RSExportType;

class RSExportVar : public RSExportable {
  friend class RSContext;
 private:
  std::string mName;
  const RSExportType *mET;
  bool mIsConst;
  bool mIsUnsigned;

  clang::Expr::EvalResult mInit;

  size_t mArraySize; // number of elements
  size_t mNumInits;
  llvm::SmallVector<clang::Expr::EvalResult, 0> mInitArray;

  RSExportVar(RSContext *Context,
              const clang::VarDecl *VD,
              const RSExportType *ET);

 public:
  inline const std::string &getName() const { return mName; }
  inline const RSExportType *getType() const { return mET; }
  inline bool isConst() const { return mIsConst; }
  inline bool isUnsigned() const { return mIsUnsigned; }

  inline const clang::APValue &getInit() const { return mInit.Val; }

  inline size_t getArraySize() const { return mArraySize; }
  inline size_t getNumInits() const { return mNumInits; }
  inline const clang::APValue &getInitArray(unsigned int i) const {
    slangAssert(i < mNumInits);
    return mInitArray[i].Val;
  }
};  // RSExportVar

}   // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_VAR_H_  NOLINT
