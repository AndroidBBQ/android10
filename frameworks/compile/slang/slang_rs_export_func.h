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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FUNC_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FUNC_H_

#include <list>
#include <string>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/AST/Decl.h"

#include "slang_assert.h"
#include "slang_rs_export_type.h"
#include "slang_rs_exportable.h"

namespace llvm {
  class StructType;
}

namespace slang {

class RSContext;

class RSExportFunc : public RSExportable {
  friend class RSContext;

 private:
  std::string mName;
  std::string mMangledName;
  bool mShouldMangle;
  RSExportRecordType *mParamPacketType;

  RSExportFunc(RSContext *Context, const llvm::StringRef &Name,
               const clang::FunctionDecl *FD)
    : RSExportable(Context, RSExportable::EX_FUNC, FD->getLocation()),
      mName(Name.data(), Name.size()),
      mMangledName(),
      mShouldMangle(false),
      mParamPacketType(nullptr) {

    mShouldMangle = Context->getMangleContext().shouldMangleDeclName(FD);

    if (mShouldMangle) {
      llvm::raw_string_ostream BufStm(mMangledName);
      Context->getMangleContext().mangleName(FD, BufStm);
      BufStm.flush();
    }
  }

 public:
  static RSExportFunc *Create(RSContext *Context,
                              const clang::FunctionDecl *FD);

  typedef RSExportRecordType::const_field_iterator const_param_iterator;

  inline const_param_iterator params_begin() const {
    slangAssert((mParamPacketType != nullptr) &&
                "Get parameter from export function having no parameter!");
    return mParamPacketType->fields_begin();
  }
  inline const_param_iterator params_end() const {
    slangAssert((mParamPacketType != nullptr) &&
                "Get parameter from export function having no parameter!");
    return mParamPacketType->fields_end();
  }

  inline const std::string &getName(bool mangle = true) const {
    return (mShouldMangle && mangle) ? mMangledName : mName;
  }

  inline bool hasParam() const
    { return (mParamPacketType && !mParamPacketType->getFields().empty()); }
  inline size_t getNumParameters() const
    { return ((mParamPacketType) ? mParamPacketType->getFields().size() : 0); }

  inline const RSExportRecordType *getParamPacketType() const
    { return mParamPacketType; }

  // Check whether the given ParamsPacket type (in LLVM type) is "size
  // equivalent" to the one obtained from getParamPacketType(). If the @Params
  // is nullptr, means there must be no any parameters.
  bool checkParameterPacketType(llvm::StructType *ParamTy) const;
};  // RSExportFunc


}   // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FUNC_H_  NOLINT
