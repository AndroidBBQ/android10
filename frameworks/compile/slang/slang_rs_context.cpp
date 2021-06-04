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

#include "slang_rs_context.h"

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/Type.h"

#include "clang/Basic/Linkage.h"
#include "clang/Basic/TargetInfo.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DataLayout.h"

#include "slang.h"
#include "slang_assert.h"
#include "slang_backend.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_func.h"
#include "slang_rs_export_reduce.h"
#include "slang_rs_export_type.h"
#include "slang_rs_export_var.h"
#include "slang_rs_exportable.h"
#include "slang_rs_pragma_handler.h"
#include "slang_rs_reflection.h"
#include "slang_rs_special_func.h"

namespace slang {

RSContext::RSContext(clang::Preprocessor &PP,
                     clang::ASTContext &Ctx,
                     const clang::TargetInfo &Target,
                     PragmaList *Pragmas,
                     unsigned int TargetAPI,
                     bool Verbose)
    : mPP(PP),
      mCtx(Ctx),
      mPragmas(Pragmas),
      mTargetAPI(TargetAPI),
      mVerbose(Verbose),
      mDataLayout(Target.getDataLayout()),
      mLLVMContext(slang::getGlobalLLVMContext()),
      mLicenseNote(nullptr),
      mRSPackageName("android.renderscript"),
      version(0),
      mMangleCtx(Ctx.createMangleContext()),
      mIs64Bit(Target.getPointerWidth(0) == 64),
      mNextSlot(1),
      mNextForEachOrdinal(0) {

  AddPragmaHandlers(PP, this);

  // Prepare target data
  // mDataLayout = Target.getDataLayout();

  // Reserve slot 0 for the root kernel.
  mExportForEach.push_back(nullptr);
  mFirstOldStyleKernel = mExportForEach.end();
}

bool RSContext::processExportVar(const clang::VarDecl *VD) {
  slangAssert(!VD->getName().empty() && "Variable name should not be empty");

  RSExportType *ET = RSExportType::CreateFromDecl(this, VD);
  if (!ET)
    return false;

  RSExportVar *EV = new RSExportVar(this, VD, ET);
  if (EV == nullptr)
    return false;
  else
    mExportVars.push_back(EV);

  return true;
}

int RSContext::getForEachSlotNumber(const clang::FunctionDecl* FD) {
  const clang::StringRef& funcName = FD->getName();
  return getForEachSlotNumber(funcName);
}

int RSContext::getForEachSlotNumber(const clang::StringRef& funcName) {
  auto it = mExportForEachMap.find(funcName);
  if (it == mExportForEachMap.end()) {
    return -1;
  }
  return it->second;
}

bool RSContext::processExportFunc(const clang::FunctionDecl *FD) {
  slangAssert(!FD->getName().empty() && "Function name should not be empty");

  if (!FD->isThisDeclarationADefinition()) {
    return true;
  }

  slangAssert(FD->getStorageClass() == clang::SC_None);

  // Specialized function
  if (RSSpecialFunc::isSpecialRSFunc(mTargetAPI, FD)) {
    // Do not reflect specialized functions like init, dtor, or graphics root.
    return RSSpecialFunc::validateSpecialFuncDecl(mTargetAPI, this, FD);
  }

  // Foreach kernel
  if (RSExportForEach::isRSForEachFunc(mTargetAPI, FD)) {
    RSExportForEach *EFE = RSExportForEach::Create(this, FD);
    if (EFE == nullptr) {
      return false;
    }

    // The root function should be at index 0 in the list
    if (FD->getName().equals("root")) {
      mExportForEach[0] = EFE;
      return true;
    }

    // New-style kernels with attribute "kernel" should come first in the list
    if (FD->hasAttr<clang::RenderScriptKernelAttr>()) {
      mFirstOldStyleKernel = mExportForEach.insert(mFirstOldStyleKernel, EFE) + 1;
      slangAssert((mTargetAPI < SLANG_FEATURE_SINGLE_SOURCE_API ||
                   getForEachSlotNumber(FD->getName()) ==
                   mFirstOldStyleKernel - mExportForEach.begin() - 1) &&
                  "Inconsistent slot number assignment");
      return true;
    }

    // Old-style kernels should appear in the end of the list
    mFirstOldStyleKernel = mExportForEach.insert(mFirstOldStyleKernel, EFE);
    return true;
  }

  // Invokable
  if (auto *EF = RSExportFunc::Create(this, FD)) {
    mExportFuncs.push_back(EF);
    return true;
  }

  return false;
}

bool RSContext::addForEach(const clang::FunctionDecl* FD) {
  const llvm::StringRef& funcName = FD->getName();

  if (funcName.equals("root")) {
    // The root kernel should always be in slot 0.
    mExportForEachMap.insert(std::make_pair(funcName, 0));
  } else {
    mExportForEachMap.insert(std::make_pair(funcName, mNextSlot++));
  }

  return true;
}

bool RSContext::processExportType(const llvm::StringRef &Name) {
  clang::TranslationUnitDecl *TUDecl = mCtx.getTranslationUnitDecl();

  slangAssert(TUDecl != nullptr && "Translation unit declaration (top-level "
                                   "declaration) is null object");

  const clang::IdentifierInfo *II = mPP.getIdentifierInfo(Name);
  if (II == nullptr)
    // TODO(zonr): alert identifier @Name mark as an exportable type cannot be
    //             found
    return false;

  clang::DeclContext::lookup_result R = TUDecl->lookup(II);
  RSExportType *ET = nullptr;

  for (clang::DeclContext::lookup_iterator I = R.begin(), E = R.end();
       I != E;
       I++) {
    clang::NamedDecl *const ND = *I;
    const clang::Type *T = nullptr;

    switch (ND->getKind()) {
      case clang::Decl::Typedef: {
        T = static_cast<const clang::TypedefDecl*>(
            ND)->getCanonicalDecl()->getUnderlyingType().getTypePtr();
        break;
      }
      case clang::Decl::Record: {
        T = static_cast<const clang::RecordDecl*>(ND)->getTypeForDecl();
        break;
      }
      default: {
        // unsupported, skip
        break;
      }
    }

    if (T != nullptr)
      ET = RSExportType::Create(this, T, NotLegacyKernelArgument);
  }

  return (ET != nullptr);
}

void RSContext::setAllocationType(const clang::TypeDecl* TD) {
  mAllocationType = mCtx.getTypeDeclType(TD);
}

void RSContext::setScriptCallType(const clang::TypeDecl* TD) {
  mScriptCallType = mCtx.getTypeDeclType(TD);
}

bool RSContext::processExports() {
  bool valid = true;

  if (getDiagnostics()->hasErrorOccurred()) {
    return false;
  }

  clang::TranslationUnitDecl *TUDecl = mCtx.getTranslationUnitDecl();
  for (auto I = TUDecl->decls_begin(), E = TUDecl->decls_end(); I != E; I++) {
    clang::Decl* D = *I;
    switch (D->getKind()) {
    case clang::Decl::Var: {
      clang::VarDecl* VD = llvm::dyn_cast<clang::VarDecl>(D);
      bool ShouldExportVariable = true;
      if (VD->getFormalLinkage() == clang::ExternalLinkage) {
        clang::QualType QT = VD->getTypeSourceInfo()->getType();
        if (QT.isConstQualified() && !VD->hasInit()) {
          if (Slang::IsLocInRSHeaderFile(VD->getLocation(),
                                         *getSourceManager())) {
            // We don't export variables internal to the runtime's
            // implementation.
            ShouldExportVariable = false;
          } else {
            clang::DiagnosticsEngine *DiagEngine = getDiagnostics();
            DiagEngine->Report(VD->getLocation(), DiagEngine->getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "invalid declaration of uninitialized constant variable '%0'"))
              << VD->getName();
            valid = false;
          }
        }
        if (valid && ShouldExportVariable && isSyntheticName(VD->getName()))
          ShouldExportVariable = false;
        if (valid && ShouldExportVariable && !processExportVar(VD)) {
          valid = false;
        }
      }
      break;
    }
    case clang::Decl::Function: {
      clang::FunctionDecl* FD = llvm::dyn_cast<clang::FunctionDecl>(D);
      if (FD->getFormalLinkage() == clang::ExternalLinkage) {
        if (!processExportFunc(FD)) {
          valid = false;
        }
      }
      break;
    }
    default:
      break;
    }
  }

  // Create a dummy root in slot 0 if a root kernel is not seen
  // and there exists a non-root kernel.
  if (valid && mExportForEach[0] == nullptr) {
    const size_t numExportedForEach = mExportForEach.size();
    if (numExportedForEach > 1) {
      mExportForEach[0] = RSExportForEach::CreateDummyRoot(this);
    } else {
      slangAssert(numExportedForEach == 1);
      mExportForEach.pop_back();
    }
  }

  // Finally, export type forcely set to be exported by user
  for (NeedExportTypeSet::const_iterator EI = mNeedExportTypes.begin(),
           EE = mNeedExportTypes.end();
       EI != EE;
       EI++) {
    if (!processExportType(EI->getKey())) {
      valid = false;
    }
  }

  return valid;
}

bool RSContext::processReducePragmas(Backend *BE) {
  // This is needed to ensure that the dummy variable is emitted into
  // the bitcode -- which in turn forces the function to be emitted
  // into the bitcode.  We couldn't do this at
  // markUsedByReducePragma() time because we had to wait until the
  // Backend is available.
  for (auto DummyVar : mUsedByReducePragmaDummyVars)
    BE->HandleTopLevelDecl(clang::DeclGroupRef(DummyVar));

  bool valid = true;
  for (auto I = export_reduce_begin(), E = export_reduce_end(); I != E; ++I) {
    if (! (*I)->analyzeTranslationUnit())
      valid = false;
  }
  return valid;
}

void RSContext::markUsedByReducePragma(clang::FunctionDecl *FD, CheckName Check) {
  if (mUsedByReducePragmaFns.find(FD) != mUsedByReducePragmaFns.end())
    return;  // already marked used

  if (Check == CheckNameYes) {
    // This is an inefficient linear search.  If this turns out to be a
    // problem in practice, then processReducePragmas() could build a
    // set or hash table or something similar containing all function
    // names mentioned in a reduce pragma and searchable in O(c) or
    // O(log(n)) time rather than the currently-implemented O(n) search.
    auto NameMatches = [this, FD]() {
      for (auto I = export_reduce_begin(), E = export_reduce_end(); I != E; ++I) {
        if ((*I)->matchName(FD->getName()))
          return true;
      }
      return false;
    };
    if (!NameMatches())
      return;
  }

  mUsedByReducePragmaFns.insert(FD);

  // This is needed to prevent clang from warning that the function is
  // unused (in the case where it is only referenced by #pragma rs
  // reduce).
  FD->setIsUsed();

  // Each constituent function "f" of a reduction kernel gets a dummy variable generated for it:
  //   void *.rs.reduce_fn.f = (void*)&f;
  // This is a trick to ensure that clang will not delete "f" as unused.

  // `-VarDecl 0x87cb558 <line:3:1, col:30> col:7 var 'void *' cinit
  //     `-CStyleCastExpr 0x87cb630 <col:19, col:26> 'void *' <BitCast>
  //       `-ImplicitCastExpr 0x87cb618 <col:26> 'void (*)(int *, float, double)' <FunctionToPointerDecay>
  //         `-DeclRefExpr 0x87cb5b8 <col:26> 'void (int *, float, double)' Function 0x8784e10 'foo' 'void (int *, float, double)

  const clang::QualType VoidPtrType = mCtx.getPointerType(mCtx.VoidTy);

  clang::DeclContext *const DC = FD->getDeclContext();
  const clang::SourceLocation Loc = FD->getLocation();

  clang::VarDecl *const VD = clang::VarDecl::Create(
      mCtx, DC, Loc, Loc,
      &mCtx.Idents.get(std::string(".rs.reduce_fn.") + FD->getNameAsString()),
      VoidPtrType,
      mCtx.getTrivialTypeSourceInfo(VoidPtrType),
      clang::SC_None);
  VD->setLexicalDeclContext(DC);
  DC->addDecl(VD);

  clang::DeclRefExpr *const DRE = clang::DeclRefExpr::Create(mCtx,
                                                             clang::NestedNameSpecifierLoc(),
                                                             Loc,
                                                             FD, false, Loc, FD->getType(),
                                                             clang::VK_RValue);
  clang::ImplicitCastExpr *const ICE = clang::ImplicitCastExpr::Create(mCtx, mCtx.getPointerType(FD->getType()),
                                                                       clang::CK_FunctionToPointerDecay, DRE,
                                                                       nullptr, clang::VK_RValue);
  clang::CStyleCastExpr *const CSCE = clang::CStyleCastExpr::Create(mCtx, VoidPtrType, clang::VK_RValue, clang::CK_BitCast,
                                                                    ICE, nullptr, nullptr,
                                                                    Loc, Loc);
  VD->setInit(CSCE);

  mUsedByReducePragmaDummyVars.push_back(VD);
}

bool RSContext::insertExportType(const llvm::StringRef &TypeName,
                                 RSExportType *ET) {
  ExportTypeMap::value_type *NewItem =
      ExportTypeMap::value_type::Create(TypeName,
                                        mExportTypes.getAllocator(),
                                        ET);

  if (mExportTypes.insert(NewItem)) {
    return true;
  } else {
    NewItem->Destroy(mExportTypes.getAllocator());
    return false;
  }
}

RSContext::~RSContext() {
  delete mLicenseNote;
  for (ExportableList::iterator I = mExportables.begin(),
          E = mExportables.end();
       I != E;
       I++) {
    if (!(*I)->isKeep())
      delete *I;
  }
}

}  // namespace slang
