/*
 * Copyright 2011-2012, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FOREACH_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FOREACH_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/AST/Decl.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_exportable.h"
#include "slang_rs_export_type.h"

namespace clang {
  class FunctionDecl;
}  // namespace clang

namespace slang {

// Base class for reflecting control-side forEach
class RSExportForEach : public RSExportable {
 public:

  typedef llvm::SmallVectorImpl<const clang::ParmVarDecl*> InVec;
  typedef llvm::SmallVectorImpl<const RSExportType*> InTypeVec;

  typedef InVec::const_iterator InIter;
  typedef InTypeVec::const_iterator InTypeIter;

 private:
  std::string mName;

  // For diagnostic purposes, we record the order in which we parse
  // foreach kernels.  Does not apply to a dummy root.
  unsigned mOrdinal;

  RSExportRecordType *mParamPacketType;
  llvm::SmallVector<const RSExportType*, 16> mInTypes;
  RSExportType *mOutType;
  size_t numParams;

  unsigned int mSignatureMetadata;

  llvm::SmallVector<const clang::ParmVarDecl*, 16> mIns;
  const clang::ParmVarDecl *mOut;
  const clang::ParmVarDecl *mUsrData;

  // Accumulator for metadata bits corresponding to special parameters.
  unsigned int mSpecialParameterSignatureMetadata;

  clang::QualType mResultType;  // return type (if present).
  bool mHasReturnType;  // does this kernel have a return type?
  bool mIsKernelStyle;  // is this a pass-by-value kernel?

  bool mDummyRoot;

  // TODO(all): Add support for LOD/face when we have them
  RSExportForEach(RSContext *Context, const llvm::StringRef &Name, clang::SourceLocation Loc)
    : RSExportable(Context, RSExportable::EX_FOREACH, Loc),
      mName(Name.data(), Name.size()), mOrdinal(~unsigned(0)),
      mParamPacketType(nullptr),
      mOutType(nullptr), numParams(0), mSignatureMetadata(0),
      mOut(nullptr), mUsrData(nullptr), mSpecialParameterSignatureMetadata(0),
      mResultType(clang::QualType()), mHasReturnType(false),
      mIsKernelStyle(false), mDummyRoot(false) {
  }

  bool validateAndConstructParams(RSContext *Context,
                                  const clang::FunctionDecl *FD);

  bool validateAndConstructOldStyleParams(RSContext *Context,
                                          const clang::FunctionDecl *FD);

  bool validateAndConstructKernelParams(RSContext *Context,
                                        const clang::FunctionDecl *FD);

  bool processSpecialParameters(RSContext *Context,
                                const clang::FunctionDecl *FD,
                                size_t *IndexOfFirstSpecialParameter);

  bool setSignatureMetadata(RSContext *Context,
                            const clang::FunctionDecl *FD);
 public:
  static RSExportForEach *Create(RSContext *Context,
                                 const clang::FunctionDecl *FD);

  static RSExportForEach *CreateDummyRoot(RSContext *Context);

  inline const std::string &getName() const {
    return mName;
  }

  inline unsigned getOrdinal() const {
    slangAssert(!mDummyRoot);
    return mOrdinal;
  }

  inline size_t getNumParameters() const {
    return numParams;
  }

  inline bool hasIns() const {
    return (!mIns.empty());
  }

  inline bool hasOut() const {
    return (mOut != nullptr);
  }

  inline bool hasUsrData() const {
    return (mUsrData != nullptr);
  }

  inline bool hasReturn() const {
    return mHasReturnType;
  }

  inline const InVec& getIns() const {
    return mIns;
  }

  inline const InTypeVec& getInTypes() const {
    return mInTypes;
  }

  inline const RSExportType *getOutType() const {
    return mOutType;
  }

  inline const RSExportRecordType *getParamPacketType() const {
    return mParamPacketType;
  }

  inline unsigned int getSignatureMetadata() const {
    return mSignatureMetadata;
  }

  inline bool isDummyRoot() const {
    return mDummyRoot;
  }

  // is this a pass-by-value kernel?
  inline bool isKernelStyle() const {
    return mIsKernelStyle;
  }

  typedef RSExportRecordType::const_field_iterator const_param_iterator;

  inline const_param_iterator params_begin() const {
    slangAssert((mParamPacketType != nullptr) &&
                "Get parameter from export foreach having no parameter!");
    return mParamPacketType->fields_begin();
  }

  inline const_param_iterator params_end() const {
    slangAssert((mParamPacketType != nullptr) &&
                "Get parameter from export foreach having no parameter!");
    return mParamPacketType->fields_end();
  }
  inline size_t params_count() const {
    return (mParamPacketType ? mParamPacketType->fields_size() : 0);
  }

  static bool isRSForEachFunc(unsigned int targetAPI,
                              const clang::FunctionDecl *FD);

  static unsigned getNumInputs(unsigned int targetAPI,
                              const clang::FunctionDecl *FD);
};  // RSExportForEach

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_FOREACH_H_  NOLINT
