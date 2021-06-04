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

#include "slang_rs_export_foreach.h"

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/TypeLoc.h"

#include "llvm/IR/DerivedTypes.h"

#include "bcinfo/MetadataExtractor.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_export_type.h"
#include "slang_rs_special_func.h"
#include "slang_rs_special_kernel_param.h"
#include "slang_version.h"

namespace {

const size_t RS_KERNEL_INPUT_LIMIT = 8; // see frameworks/base/libs/rs/cpu_ref/rsCpuCoreRuntime.h

bool isRootRSFunc(const clang::FunctionDecl *FD) {
  if (!FD) {
    return false;
  }
  return FD->getName().equals("root");
}

} // end anonymous namespace

namespace slang {

// This function takes care of additional validation and construction of
// parameters related to forEach_* reflection.
bool RSExportForEach::validateAndConstructParams(
    RSContext *Context, const clang::FunctionDecl *FD) {
  slangAssert(Context && FD);
  bool valid = true;

  numParams = FD->getNumParams();

  if (Context->getTargetAPI() < SLANG_JB_TARGET_API) {
    // Before JellyBean, we allowed only one kernel per file.  It must be called "root".
    if (!isRootRSFunc(FD)) {
      Context->ReportError(FD->getLocation(),
                           "Non-root compute kernel %0() is "
                           "not supported in SDK levels %1-%2")
          << FD->getName() << SLANG_MINIMUM_TARGET_API
          << (SLANG_JB_TARGET_API - 1);
      return false;
    }
  }

  mResultType = FD->getReturnType().getCanonicalType();
  // Compute kernel functions are defined differently when the
  // "__attribute__((kernel))" is set.
  if (FD->hasAttr<clang::RenderScriptKernelAttr>()) {
    valid &= validateAndConstructKernelParams(Context, FD);
  } else {
    valid &= validateAndConstructOldStyleParams(Context, FD);
  }

  valid &= setSignatureMetadata(Context, FD);
  return valid;
}

bool RSExportForEach::validateAndConstructOldStyleParams(
    RSContext *Context, const clang::FunctionDecl *FD) {
  slangAssert(Context && FD);
  // If numParams is 0, we already marked this as a graphics root().
  slangAssert(numParams > 0);

  bool valid = true;

  // Compute kernel functions of this style are required to return a void type.
  clang::ASTContext &C = Context->getASTContext();
  if (mResultType != C.VoidTy) {
    Context->ReportError(FD->getLocation(),
                         "Compute kernel %0() is required to return a "
                         "void type")
        << FD->getName();
    valid = false;
  }

  // Validate remaining parameter types

  size_t IndexOfFirstSpecialParameter = numParams;
  valid &= processSpecialParameters(Context, FD, &IndexOfFirstSpecialParameter);

  // Validate the non-special parameters, which should all be found before the
  // first special parameter.
  for (size_t i = 0; i < IndexOfFirstSpecialParameter; i++) {
    const clang::ParmVarDecl *PVD = FD->getParamDecl(i);
    clang::QualType QT = PVD->getType().getCanonicalType();

    if (!QT->isPointerType()) {
      Context->ReportError(PVD->getLocation(),
                           "Compute kernel %0() cannot have non-pointer "
                           "parameters besides special parameters (%1). Parameter '%2' is "
                           "of type: '%3'")
          << FD->getName() << listSpecialKernelParameters(Context->getTargetAPI())
          << PVD->getName() << PVD->getType().getAsString();
      valid = false;
      continue;
    }

    // The only non-const pointer should be out.
    if (!QT->getPointeeType().isConstQualified()) {
      if (mOut == nullptr) {
        mOut = PVD;
      } else {
        Context->ReportError(PVD->getLocation(),
                             "Compute kernel %0() can only have one non-const "
                             "pointer parameter. Parameters '%1' and '%2' are "
                             "both non-const.")
            << FD->getName() << mOut->getName() << PVD->getName();
        valid = false;
      }
    } else {
      if (mIns.empty() && mOut == nullptr) {
        mIns.push_back(PVD);
      } else if (mUsrData == nullptr) {
        mUsrData = PVD;
      } else {
        Context->ReportError(
            PVD->getLocation(),
            "Unexpected parameter '%0' for compute kernel %1()")
            << PVD->getName() << FD->getName();
        valid = false;
      }
    }
  }

  if (mIns.empty() && !mOut) {
    Context->ReportError(FD->getLocation(),
                         "Compute kernel %0() must have at least one "
                         "parameter for in or out")
        << FD->getName();
    valid = false;
  }

  return valid;
}

bool RSExportForEach::validateAndConstructKernelParams(
    RSContext *Context, const clang::FunctionDecl *FD) {
  slangAssert(Context && FD);
  bool valid = true;
  clang::ASTContext &C = Context->getASTContext();

  if (Context->getTargetAPI() < SLANG_JB_MR1_TARGET_API) {
    Context->ReportError(FD->getLocation(),
                         "Compute kernel %0() targeting SDK levels "
                         "%1-%2 may not use pass-by-value with "
                         "__attribute__((kernel))")
        << FD->getName() << SLANG_MINIMUM_TARGET_API
        << (SLANG_JB_MR1_TARGET_API - 1);
    return false;
  }

  // Denote that we are indeed a pass-by-value kernel.
  mIsKernelStyle = true;
  mHasReturnType = (mResultType != C.VoidTy);

  if (mResultType->isPointerType()) {
    Context->ReportError(
        FD->getTypeSpecStartLoc(),
        "Compute kernel %0() cannot return a pointer type: '%1'")
        << FD->getName() << mResultType.getAsString();
    valid = false;
  }

  // Validate remaining parameter types

  size_t IndexOfFirstSpecialParameter = numParams;
  valid &= processSpecialParameters(Context, FD, &IndexOfFirstSpecialParameter);

  // Validate the non-special parameters, which should all be found before the
  // first special.
  for (size_t i = 0; i < IndexOfFirstSpecialParameter; i++) {
    const clang::ParmVarDecl *PVD = FD->getParamDecl(i);

    if (Context->getTargetAPI() >= SLANG_M_TARGET_API || i == 0) {
      if (i >= RS_KERNEL_INPUT_LIMIT) {
        Context->ReportError(PVD->getLocation(),
                             "Invalid parameter '%0' for compute kernel %1(). "
                             "Kernels targeting SDK levels %2+ may not use "
                             "more than %3 input parameters.") << PVD->getName() <<
                             FD->getName() << SLANG_M_TARGET_API <<
                             int(RS_KERNEL_INPUT_LIMIT);

      } else {
        mIns.push_back(PVD);
      }
    } else {
      Context->ReportError(PVD->getLocation(),
                           "Invalid parameter '%0' for compute kernel %1(). "
                           "Kernels targeting SDK levels %2-%3 may not use "
                           "multiple input parameters.") << PVD->getName() <<
                           FD->getName() << SLANG_MINIMUM_TARGET_API <<
                           (SLANG_M_TARGET_API - 1);
      valid = false;
    }
    clang::QualType QT = PVD->getType().getCanonicalType();
    if (QT->isPointerType()) {
      Context->ReportError(PVD->getLocation(),
                           "Compute kernel %0() cannot have "
                           "parameter '%1' of pointer type: '%2'")
          << FD->getName() << PVD->getName() << PVD->getType().getAsString();
      valid = false;
    }
  }

  // Check that we have at least one allocation to use for dimensions.
  if (valid && mIns.empty() && !mHasReturnType && Context->getTargetAPI() < SLANG_M_TARGET_API) {
    Context->ReportError(FD->getLocation(),
                         "Compute kernel %0() targeting SDK levels "
                         "%1-%2 must have at least one "
                         "input parameter or a non-void return "
                         "type")
        << FD->getName() << SLANG_MINIMUM_TARGET_API
        << (SLANG_M_TARGET_API - 1);
    valid = false;
  }

  return valid;
}

// Process the optional special parameters:
// - Sets *IndexOfFirstSpecialParameter to the index of the first special parameter, or
//     FD->getNumParams() if none are found.
// - Add bits to mSpecialParameterSignatureMetadata for the found special parameters.
// Returns true if no errors.
bool RSExportForEach::processSpecialParameters(
    RSContext *Context, const clang::FunctionDecl *FD,
    size_t *IndexOfFirstSpecialParameter) {
  auto DiagnosticCallback = [FD] {
    std::ostringstream DiagnosticDescription;
    DiagnosticDescription << "compute kernel " << FD->getName().str() << "()";
    return DiagnosticDescription.str();
  };
  return slang::processSpecialKernelParameters(Context,
                                               DiagnosticCallback,
                                               FD,
                                               IndexOfFirstSpecialParameter,
                                               &mSpecialParameterSignatureMetadata);
}

bool RSExportForEach::setSignatureMetadata(RSContext *Context,
                                           const clang::FunctionDecl *FD) {
  mSignatureMetadata = 0;
  bool valid = true;

  if (mIsKernelStyle) {
    slangAssert(mOut == nullptr);
    slangAssert(mUsrData == nullptr);
  } else {
    slangAssert(!mHasReturnType);
  }

  // Set up the bitwise metadata encoding for runtime argument passing.
  const bool HasOut = mOut || mHasReturnType;
  mSignatureMetadata |= (hasIns() ?       bcinfo::MD_SIG_In     : 0);
  mSignatureMetadata |= (HasOut ?         bcinfo::MD_SIG_Out    : 0);
  mSignatureMetadata |= (mUsrData ?       bcinfo::MD_SIG_Usr    : 0);
  mSignatureMetadata |= (mIsKernelStyle ? bcinfo::MD_SIG_Kernel : 0);  // pass-by-value
  mSignatureMetadata |= mSpecialParameterSignatureMetadata;

  if (Context->getTargetAPI() < SLANG_ICS_TARGET_API) {
    // APIs before ICS cannot skip between parameters. It is ok, however, for
    // them to omit further parameters (i.e. skipping X is ok if you skip Y).
    if (mSignatureMetadata != (bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out | bcinfo::MD_SIG_Usr |
                               bcinfo::MD_SIG_X | bcinfo::MD_SIG_Y) &&
        mSignatureMetadata != (bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out | bcinfo::MD_SIG_Usr |
                               bcinfo::MD_SIG_X) &&
        mSignatureMetadata != (bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out | bcinfo::MD_SIG_Usr) &&
        mSignatureMetadata != (bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out) &&
        mSignatureMetadata != (bcinfo::MD_SIG_In)) {
      Context->ReportError(FD->getLocation(),
                           "Compute kernel %0() targeting SDK levels "
                           "%1-%2 may not skip parameters")
          << FD->getName() << SLANG_MINIMUM_TARGET_API
          << (SLANG_ICS_TARGET_API - 1);
      valid = false;
    }
  }
  return valid;
}

RSExportForEach *RSExportForEach::Create(RSContext *Context,
                                         const clang::FunctionDecl *FD) {
  slangAssert(Context && FD);
  llvm::StringRef Name = FD->getName();
  RSExportForEach *FE;

  slangAssert(!Name.empty() && "Function must have a name");

  FE = new RSExportForEach(Context, Name, FD->getLocation());
  FE->mOrdinal = Context->getNextForEachOrdinal();

  if (!FE->validateAndConstructParams(Context, FD)) {
    return nullptr;
  }

  clang::ASTContext &Ctx = Context->getASTContext();

  std::string Id = CreateDummyName("helper_foreach_param", FE->getName());

  // Construct type information about usrData, inputs, and
  // outputs. Return null when there is an error exporting types.

  bool TypeExportError = false;

  // Extract the usrData parameter (if we have one)
  if (FE->mUsrData) {
    const clang::ParmVarDecl *PVD = FE->mUsrData;
    clang::QualType QT = PVD->getType().getCanonicalType();
    slangAssert(QT->isPointerType() &&
                QT->getPointeeType().isConstQualified());

    const clang::ASTContext &C = Context->getASTContext();
    if (QT->getPointeeType().getCanonicalType().getUnqualifiedType() ==
        C.VoidTy) {
      // In the case of using const void*, we can't reflect an appopriate
      // Java type, so we fall back to just reflecting the ain/aout parameters
      FE->mUsrData = nullptr;
    } else {
      clang::RecordDecl *RD =
          clang::RecordDecl::Create(Ctx, clang::TTK_Struct,
                                    Ctx.getTranslationUnitDecl(),
                                    clang::SourceLocation(),
                                    clang::SourceLocation(),
                                    &Ctx.Idents.get(Id));

      clang::FieldDecl *FD =
          clang::FieldDecl::Create(Ctx,
                                   RD,
                                   clang::SourceLocation(),
                                   clang::SourceLocation(),
                                   PVD->getIdentifier(),
                                   QT->getPointeeType(),
                                   nullptr,
                                   /* BitWidth = */ nullptr,
                                   /* Mutable = */ false,
                                   /* HasInit = */ clang::ICIS_NoInit);
      RD->addDecl(FD);
      RD->completeDefinition();

      // Create an export type iff we have a valid usrData type
      clang::QualType T = Ctx.getTagDeclType(RD);
      slangAssert(!T.isNull());

      RSExportType *ET =
          RSExportType::Create(Context, T.getTypePtr(), LegacyKernelArgument);

      if (ET) {
        slangAssert((ET->getClass() == RSExportType::ExportClassRecord) &&
                    "Parameter packet must be a record");

        FE->mParamPacketType = static_cast<RSExportRecordType *>(ET);
      } else {
        TypeExportError = true;
      }
    }
  }

  if (FE->hasIns()) {
    for (InIter BI = FE->mIns.begin(), EI = FE->mIns.end(); BI != EI; BI++) {
      const clang::Type *T = (*BI)->getType().getCanonicalType().getTypePtr();
      ExportKind EK = (FE->mIsKernelStyle ? NotLegacyKernelArgument :
                                            LegacyKernelArgument);
      RSExportType *InExportType = RSExportType::Create(Context, T, EK);

      // It is not an error if we don't export an input type for legacy
      // kernel arguments. This can happen in the case of a void pointer.
      // See ReflectionState::addForEachIn().
      if (FE->mIsKernelStyle && !InExportType) {
        TypeExportError = true;
      }

      FE->mInTypes.push_back(InExportType);
    }
  }

  if (FE->mIsKernelStyle && FE->mHasReturnType) {
    const clang::Type *ReturnType = FE->mResultType.getTypePtr();
    FE->mOutType = RSExportType::Create(Context, ReturnType,
                                        NotLegacyKernelArgument);
    TypeExportError |= !FE->mOutType;
  } else if (FE->mOut) {
    const clang::Type *OutType =
        FE->mOut->getType().getCanonicalType().getTypePtr();
    FE->mOutType = RSExportType::Create(Context, OutType, LegacyKernelArgument);
    // It is not an error if we don't export an output type.
    // This can happen in the case of a void pointer.
  }

  if (TypeExportError) {
    slangAssert(Context->getDiagnostics()->hasErrorOccurred() &&
                "Error exporting type but no diagnostic message issued!");
    return nullptr;
  }

  return FE;
}

RSExportForEach *RSExportForEach::CreateDummyRoot(RSContext *Context) {
  slangAssert(Context);
  llvm::StringRef Name = "root";
  RSExportForEach *FE = new RSExportForEach(Context, Name, clang::SourceLocation());
  FE->mDummyRoot = true;
  return FE;
}

bool RSExportForEach::isRSForEachFunc(unsigned int targetAPI,
                                      const clang::FunctionDecl *FD) {
  if (!FD) {
    return false;
  }

  // Anything tagged as a kernel("") is definitely used with ForEach.
  if (FD->hasAttr<clang::RenderScriptKernelAttr>()) {
    return true;
  }

  if (RSSpecialFunc::isGraphicsRootRSFunc(targetAPI, FD)) {
    return false;
  }

  // Check if first parameter is a pointer (which is required for ForEach).
  unsigned int numParams = FD->getNumParams();

  if (numParams > 0) {
    const clang::ParmVarDecl *PVD = FD->getParamDecl(0);
    clang::QualType QT = PVD->getType().getCanonicalType();

    if (QT->isPointerType()) {
      return true;
    }

    // Any non-graphics root() is automatically a ForEach candidate.
    // At this point, however, we know that it is not going to be a valid
    // compute root() function (due to not having a pointer parameter). We
    // still want to return true here, so that we can issue appropriate
    // diagnostics.
    if (isRootRSFunc(FD)) {
      return true;
    }
  }

  return false;
}

unsigned RSExportForEach::getNumInputs(unsigned int targetAPI,
                                       const clang::FunctionDecl *FD) {
  unsigned numInputs = 0;
  for (const clang::ParmVarDecl* param : FD->parameters()) {
    if (!isSpecialKernelParameter(param->getName())) {
      numInputs++;
    }
  }

  return numInputs;
}

}  // namespace slang
