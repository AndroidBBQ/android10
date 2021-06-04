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

#include "slang_rs_export_func.h"

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"

#include "slang_assert.h"
#include "slang_rs_context.h"

namespace slang {

namespace {

// Ensure that the exported function is actually valid
static bool ValidateFuncDecl(slang::RSContext *Context,
                             const clang::FunctionDecl *FD) {
  slangAssert(Context && FD);
  const clang::ASTContext &C = FD->getASTContext();
  if (FD->getReturnType().getCanonicalType() != C.VoidTy) {
    Context->ReportError(
        FD->getLocation(),
        "invokable non-static functions are required to return void");
    return false;
  }
  return true;
}

}  // namespace

RSExportFunc *RSExportFunc::Create(RSContext *Context,
                                   const clang::FunctionDecl *FD) {
  llvm::StringRef Name = FD->getName();
  RSExportFunc *F;

  slangAssert(!Name.empty() && "Function must have a name");

  if (!ValidateFuncDecl(Context, FD)) {
    return nullptr;
  }

  F = new RSExportFunc(Context, Name, FD);

  // Initialize mParamPacketType
  if (FD->getNumParams() <= 0) {
    F->mParamPacketType = nullptr;
  } else {
    clang::ASTContext &Ctx = Context->getASTContext();

    std::string Id = CreateDummyName("helper_func_param", F->getName());

    clang::RecordDecl *RD =
        clang::RecordDecl::Create(Ctx, clang::TTK_Struct,
                                  Ctx.getTranslationUnitDecl(),
                                  clang::SourceLocation(),
                                  clang::SourceLocation(),
                                  &Ctx.Idents.get(Id));

    for (unsigned i = 0; i < FD->getNumParams(); i++) {
      const clang::ParmVarDecl *PVD = FD->getParamDecl(i);
      llvm::StringRef ParamName = PVD->getName();

      if (PVD->hasDefaultArg())
        fprintf(stderr, "Note: parameter '%s' in function '%s' has default "
                        "value which is not supported\n",
                        ParamName.str().c_str(),
                        F->getName().c_str());

      clang::FieldDecl *FD =
          clang::FieldDecl::Create(Ctx,
                                   RD,
                                   clang::SourceLocation(),
                                   clang::SourceLocation(),
                                   PVD->getIdentifier(),
                                   PVD->getOriginalType(),
                                   nullptr,
                                   /* BitWidth = */ nullptr,
                                   /* Mutable = */ false,
                                   /* HasInit = */ clang::ICIS_NoInit);
      RD->addDecl(FD);
    }

    RD->completeDefinition();

    clang::QualType T = Ctx.getTagDeclType(RD);
    slangAssert(!T.isNull());

    RSExportType *ET =
      RSExportType::Create(Context, T.getTypePtr(), NotLegacyKernelArgument);

    if (ET == nullptr) {
      fprintf(stderr, "Failed to export the function %s. There's at least one "
                      "parameter whose type is not supported by the "
                      "reflection\n", F->getName().c_str());
      return nullptr;
    }

    slangAssert((ET->getClass() == RSExportType::ExportClassRecord) &&
           "Parameter packet must be a record");

    F->mParamPacketType = static_cast<RSExportRecordType *>(ET);
  }

  return F;
}

bool
RSExportFunc::checkParameterPacketType(llvm::StructType *ParamTy) const {
  if (ParamTy == nullptr)
    return !hasParam();
  else if (!hasParam())
    return false;

  slangAssert(mParamPacketType != nullptr);

  const RSExportRecordType *ERT = mParamPacketType;
  // must have same number of elements
  if (ERT->getFields().size() != ParamTy->getNumElements())
    return false;

  const llvm::StructLayout *ParamTySL =
      getRSContext()->getDataLayout().getStructLayout(ParamTy);

  unsigned Index = 0;
  for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
       FE = ERT->fields_end(); FI != FE; FI++, Index++) {
    const RSExportRecordType::Field *F = *FI;

    llvm::Type *T1 = F->getType()->getLLVMType();
    llvm::Type *T2 = ParamTy->getTypeAtIndex(Index);

    // Fast check
    if (T1 == T2)
      continue;

    // Check offset
    size_t T1Offset = F->getOffsetInParent();
    size_t T2Offset = ParamTySL->getElementOffset(Index);

    if (T1Offset != T2Offset)
      return false;

    // Check size
    size_t T1Size = F->getType()->getAllocSize();
    size_t T2Size = getRSContext()->getDataLayout().getTypeAllocSize(T2);

    if (T1Size != T2Size)
      return false;
  }

  return true;
}

}  // namespace slang
