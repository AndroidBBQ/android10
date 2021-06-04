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

#include "clang/AST/Attr.h"

#include "slang_rs_check_ast.h"

#include "slang_assert.h"
#include "slang.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_reduce.h"
#include "slang_rs_export_type.h"

namespace slang {

void RSCheckAST::VisitStmt(clang::Stmt *S) {
  // This function does the actual iteration through all sub-Stmt's within
  // a given Stmt. Note that this function is skipped by all of the other
  // Visit* functions if we have already found a higher-level match.
  for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
       I != E;
       I++) {
    if (clang::Stmt *Child = *I) {
      Visit(Child);
    }
  }
}

void RSCheckAST::WarnOnSetElementAt(clang::CallExpr *E) {
  clang::FunctionDecl *Decl;
  Decl = clang::dyn_cast_or_null<clang::FunctionDecl>(E->getCalleeDecl());

  if (!Decl || Decl->getNameAsString() != std::string("rsSetElementAt")) {
    return;
  }

  clang::Expr *Expr;
  clang::ImplicitCastExpr *ImplCast;
  Expr = E->getArg(1);
  ImplCast = clang::dyn_cast_or_null<clang::ImplicitCastExpr>(Expr);

  if (!ImplCast) {
    return;
  }

  const clang::Type *Ty;
  const clang::VectorType *VectorTy;
  const clang::BuiltinType *ElementTy;
  Ty = ImplCast->getSubExpr()->getType()->getPointeeType()
    ->getUnqualifiedDesugaredType();
  VectorTy = clang::dyn_cast_or_null<clang::VectorType>(Ty);

  if (VectorTy) {
    ElementTy = clang::dyn_cast_or_null<clang::BuiltinType>(
      VectorTy->getElementType()->getUnqualifiedDesugaredType());
  } else {
    ElementTy = clang::dyn_cast_or_null<clang::BuiltinType>(
      Ty->getUnqualifiedDesugaredType());
  }

  if (!ElementTy) {
    return;
  }

  // We only support vectors with 2, 3 or 4 elements.
  if (VectorTy) {
    switch (VectorTy->getNumElements()) {
    default:
      return;
    case 2:
    case 3:
    case 4:
      break;
    }
  }

  const char *Name;

  switch (ElementTy->getKind()) {
    case clang::BuiltinType::Float:
      Name = "float";
      break;
    case clang::BuiltinType::Double:
      Name = "double";
      break;
    case clang::BuiltinType::Char_S:
      Name = "char";
      break;
    case clang::BuiltinType::Short:
      Name = "short";
      break;
    case clang::BuiltinType::Int:
      Name = "int";
      break;
    case clang::BuiltinType::Long:
      Name = "long";
      break;
    case clang::BuiltinType::UChar:
      Name = "uchar";
      break;
    case clang::BuiltinType::UShort:
      Name = "ushort";
      break;
    case clang::BuiltinType::UInt:
      Name = "uint";
      break;
    case clang::BuiltinType::ULong:
      Name = "ulong";
      break;
    default:
      return;
  }

  clang::DiagnosticBuilder DiagBuilder =
      Context->ReportWarning(E->getLocStart(),
                             "untyped rsSetElementAt() can reduce performance. "
                             "Use rsSetElementAt_%0%1() instead.");
  DiagBuilder << Name;

  if (VectorTy) {
    DiagBuilder << VectorTy->getNumElements();
  } else {
    DiagBuilder << "";
  }
}

void RSCheckAST::VisitCallExpr(clang::CallExpr *E) {
  WarnOnSetElementAt(E);

  for (clang::CallExpr::arg_iterator AI = E->arg_begin(), AE = E->arg_end();
       AI != AE; ++AI) {
    Visit(*AI);
  }
}

void RSCheckAST::ValidateFunctionDecl(clang::FunctionDecl *FD) {
  if (!FD) {
    return;
  }

  // Validate that the kernel attribute is not used with static.
  if (FD->hasAttr<clang::RenderScriptKernelAttr>() &&
      FD->getStorageClass() == clang::SC_Static) {
    Context->ReportError(FD->getLocation(),
                         "Invalid use of attribute kernel with "
                         "static function declaration: %0")
      << FD->getName();
    mValid = false;
  }

  clang::QualType resultType = FD->getReturnType().getCanonicalType();
  bool isExtern = (FD->getFormalLinkage() == clang::ExternalLinkage);

  // We use FD as our NamedDecl in the case of a bad return type.
  if (!RSExportType::ValidateType(Context, C, resultType, FD,
                                  FD->getLocStart(), mTargetAPI,
                                  mIsFilterscript, isExtern)) {
    mValid = false;
  }

  size_t numParams = FD->getNumParams();
  for (size_t i = 0; i < numParams; i++) {
    clang::ParmVarDecl *PVD = FD->getParamDecl(i);
    clang::QualType QT = PVD->getType().getCanonicalType();
    if (!RSExportType::ValidateType(Context, C, QT, PVD, PVD->getLocStart(),
                                    mTargetAPI, mIsFilterscript, isExtern)) {
      mValid = false;
    }
  }

  bool saveKernel = mInKernel;
  mInKernel = RSExportForEach::isRSForEachFunc(mTargetAPI, FD);

  if (clang::Stmt *Body = FD->getBody()) {
    Visit(Body);
  }

  mInKernel = saveKernel;
}


void RSCheckAST::ValidateVarDecl(clang::VarDecl *VD) {
  if (!VD || RSContext::isSyntheticName(VD->getName())) {
    return;
  }

  clang::QualType QT = VD->getType();

  if (VD->getFormalLinkage() == clang::ExternalLinkage) {
    llvm::StringRef TypeName;
    const clang::Type *T = QT.getTypePtr();
    if (!RSExportType::NormalizeType(T, TypeName, Context, VD,
                                     NotLegacyKernelArgument)) {
      mValid = false;
    }
  }

  // We don't allow static (non-const) variables within kernels.
  if (mInKernel && VD->isStaticLocal()) {
    if (!QT.isConstQualified()) {
      Context->ReportError(
          VD->getLocation(),
          "Non-const static variables are not allowed in kernels: '%0'")
          << VD->getName();
      mValid = false;
    }
  }

  if (!RSExportType::ValidateVarDecl(Context, VD, mTargetAPI, mIsFilterscript)) {
    mValid = false;
  } else if (clang::Expr *Init = VD->getInit()) {
    // Only check the initializer if the decl is already ok.
    Visit(Init);
  }
}


void RSCheckAST::VisitDeclStmt(clang::DeclStmt *DS) {
  if (!Slang::IsLocInRSHeaderFile(DS->getLocStart(), mSM)) {
    for (clang::DeclStmt::decl_iterator I = DS->decl_begin(),
                                        E = DS->decl_end();
         I != E;
         ++I) {
      if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(*I)) {
        ValidateVarDecl(VD);
      } else if (clang::FunctionDecl *FD =
            llvm::dyn_cast<clang::FunctionDecl>(*I)) {
        ValidateFunctionDecl(FD);
      }
    }
  }
}


void RSCheckAST::VisitCastExpr(clang::CastExpr *CE) {
  if (CE->getCastKind() == clang::CK_BitCast) {
    clang::QualType QT = CE->getType();
    const clang::Type *T = QT.getTypePtr();
    if (T->isVectorType()) {
      if (llvm::isa<clang::ImplicitCastExpr>(CE)) {
        Context->ReportError(CE->getExprLoc(), "invalid implicit vector cast");
      } else {
        Context->ReportError(CE->getExprLoc(), "invalid vector cast");
      }
      mValid = false;
    }
  }
  Visit(CE->getSubExpr());
}


void RSCheckAST::VisitExpr(clang::Expr *E) {
  // This is where FS checks for code using pointer and/or 64-bit expressions
  // (i.e. things like casts).

  // First we skip implicit casts (things like function calls and explicit
  // array accesses rely heavily on them and they are valid.
  E = E->IgnoreImpCasts();

  // Expressions at this point in the checker are not externally visible.
  static const bool kIsExtern = false;

  if (mIsFilterscript &&
      !Slang::IsLocInRSHeaderFile(E->getExprLoc(), mSM) &&
      !RSExportType::ValidateType(Context, C, E->getType(), nullptr, E->getExprLoc(),
                                  mTargetAPI, mIsFilterscript, kIsExtern)) {
    mValid = false;
  } else {
    // Only visit sub-expressions if we haven't already seen a violation.
    VisitStmt(E);
  }
}


bool RSCheckAST::Validate() {
  clang::TranslationUnitDecl *TUDecl = C.getTranslationUnitDecl();
  for (clang::DeclContext::decl_iterator DI = TUDecl->decls_begin(),
          DE = TUDecl->decls_end();
       DI != DE;
       DI++) {

    // Following tests are not applicable to implicitly defined types
    if (DI->isImplicit())
      continue;

    if (!Slang::IsLocInRSHeaderFile(DI->getLocStart(), mSM)) {
      if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(*DI)) {
        ValidateVarDecl(VD);
      } else if (clang::FunctionDecl *FD =
            llvm::dyn_cast<clang::FunctionDecl>(*DI)) {
        ValidateFunctionDecl(FD);
      } else if (clang::Stmt *Body = (*DI)->getBody()) {
        Visit(Body);
      }
    }
  }

  return mValid;
}

}  // namespace slang
