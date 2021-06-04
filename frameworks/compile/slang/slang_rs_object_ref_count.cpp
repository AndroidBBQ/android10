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

#include "slang_rs_object_ref_count.h"

#include "clang/AST/DeclGroup.h"
#include "clang/AST/Expr.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtVisitor.h"

#include "slang_assert.h"
#include "slang.h"
#include "slang_rs_ast_replace.h"
#include "slang_rs_export_type.h"

namespace slang {

/* Even though those two arrays are of size DataTypeMax, only entries that
 * correspond to object types will be set.
 */
clang::FunctionDecl *
RSObjectRefCount::RSSetObjectFD[DataTypeMax];
clang::FunctionDecl *
RSObjectRefCount::RSClearObjectFD[DataTypeMax];

void RSObjectRefCount::GetRSRefCountingFunctions(clang::ASTContext &C) {
  for (unsigned i = 0; i < DataTypeMax; i++) {
    RSSetObjectFD[i] = nullptr;
    RSClearObjectFD[i] = nullptr;
  }

  clang::TranslationUnitDecl *TUDecl = C.getTranslationUnitDecl();

  for (clang::DeclContext::decl_iterator I = TUDecl->decls_begin(),
          E = TUDecl->decls_end(); I != E; I++) {
    if ((I->getKind() >= clang::Decl::firstFunction) &&
        (I->getKind() <= clang::Decl::lastFunction)) {
      clang::FunctionDecl *FD = static_cast<clang::FunctionDecl*>(*I);

      // points to RSSetObjectFD or RSClearObjectFD
      clang::FunctionDecl **RSObjectFD;

      if (FD->getName() == "rsSetObject") {
        slangAssert((FD->getNumParams() == 2) &&
                    "Invalid rsSetObject function prototype (# params)");
        RSObjectFD = RSSetObjectFD;
      } else if (FD->getName() == "rsClearObject") {
        slangAssert((FD->getNumParams() == 1) &&
                    "Invalid rsClearObject function prototype (# params)");
        RSObjectFD = RSClearObjectFD;
      } else {
        continue;
      }

      const clang::ParmVarDecl *PVD = FD->getParamDecl(0);
      clang::QualType PVT = PVD->getOriginalType();
      // The first parameter must be a pointer like rs_allocation*
      slangAssert(PVT->isPointerType() &&
          "Invalid rs{Set,Clear}Object function prototype (pointer param)");

      // The rs object type passed to the FD
      clang::QualType RST = PVT->getPointeeType();
      DataType DT = RSExportPrimitiveType::GetRSSpecificType(RST.getTypePtr());
      slangAssert(RSExportPrimitiveType::IsRSObjectType(DT)
             && "must be RS object type");

      if (DT >= 0 && DT < DataTypeMax) {
          RSObjectFD[DT] = FD;
      } else {
          slangAssert(false && "incorrect type");
      }
    }
  }
}

namespace {

unsigned CountRSObjectTypes(const clang::Type *T);

clang::Stmt *CreateSingleRSSetObject(clang::ASTContext &C,
                                     clang::Expr *DstExpr,
                                     clang::Expr *SrcExpr,
                                     clang::SourceLocation StartLoc,
                                     clang::SourceLocation Loc);

// This function constructs a new CompoundStmt from the input StmtList.
clang::CompoundStmt* BuildCompoundStmt(clang::ASTContext &C,
      std::vector<clang::Stmt*> &StmtList, clang::SourceLocation Loc) {
  unsigned NewStmtCount = StmtList.size();
  unsigned CompoundStmtCount = 0;

  clang::Stmt **CompoundStmtList;
  CompoundStmtList = new clang::Stmt*[NewStmtCount];

  std::vector<clang::Stmt*>::const_iterator I = StmtList.begin();
  std::vector<clang::Stmt*>::const_iterator E = StmtList.end();
  for ( ; I != E; I++) {
    CompoundStmtList[CompoundStmtCount++] = *I;
  }
  slangAssert(CompoundStmtCount == NewStmtCount);

  clang::CompoundStmt *CS = new(C) clang::CompoundStmt(
      C, llvm::makeArrayRef(CompoundStmtList, CompoundStmtCount), Loc, Loc);

  delete [] CompoundStmtList;

  return CS;
}

void AppendAfterStmt(clang::ASTContext &C,
                     clang::CompoundStmt *CS,
                     clang::Stmt *S,
                     std::list<clang::Stmt*> &StmtList) {
  slangAssert(CS);
  clang::CompoundStmt::body_iterator bI = CS->body_begin();
  clang::CompoundStmt::body_iterator bE = CS->body_end();
  clang::Stmt **UpdatedStmtList =
      new clang::Stmt*[CS->size() + StmtList.size()];

  unsigned UpdatedStmtCount = 0;
  unsigned Once = 0;
  for ( ; bI != bE; bI++) {
    if (!S && ((*bI)->getStmtClass() == clang::Stmt::ReturnStmtClass)) {
      // If we come across a return here, we don't have anything we can
      // reasonably replace. We should have already inserted our destructor
      // code in the proper spot, so we just clean up and return.
      delete [] UpdatedStmtList;

      return;
    }

    UpdatedStmtList[UpdatedStmtCount++] = *bI;

    if ((*bI == S) && !Once) {
      Once++;
      std::list<clang::Stmt*>::const_iterator I = StmtList.begin();
      std::list<clang::Stmt*>::const_iterator E = StmtList.end();
      for ( ; I != E; I++) {
        UpdatedStmtList[UpdatedStmtCount++] = *I;
      }
    }
  }
  slangAssert(Once <= 1);

  // When S is nullptr, we are appending to the end of the CompoundStmt.
  if (!S) {
    slangAssert(Once == 0);
    std::list<clang::Stmt*>::const_iterator I = StmtList.begin();
    std::list<clang::Stmt*>::const_iterator E = StmtList.end();
    for ( ; I != E; I++) {
      UpdatedStmtList[UpdatedStmtCount++] = *I;
    }
  }

  CS->setStmts(C, llvm::makeArrayRef(UpdatedStmtList, UpdatedStmtCount));

  delete [] UpdatedStmtList;
}

// This class visits a compound statement and collects a list of all the exiting
// statements, such as any return statement in any sub-block, and any
// break/continue statement that would resume outside the current scope.
// We do not handle the case for goto statements that leave a local scope.
class DestructorVisitor : public clang::StmtVisitor<DestructorVisitor> {
 private:
  // The loop depth of the currently visited node.
  int mLoopDepth;

  // The switch statement depth of the currently visited node.
  // Note that this is tracked separately from the loop depth because
  // SwitchStmt-contained ContinueStmt's should have destructors for the
  // corresponding loop scope.
  int mSwitchDepth;

  // Output of the visitor: the statements that should be replaced by compound
  // statements, each of which contains rsClearObject() calls followed by the
  // original statement.
  std::vector<clang::Stmt*> mExitingStmts;

 public:
  DestructorVisitor() : mLoopDepth(0), mSwitchDepth(0) {}

  const std::vector<clang::Stmt*>& getExitingStmts() const {
    return mExitingStmts;
  }

  void VisitStmt(clang::Stmt *S);
  void VisitBreakStmt(clang::BreakStmt *BS);
  void VisitContinueStmt(clang::ContinueStmt *CS);
  void VisitDoStmt(clang::DoStmt *DS);
  void VisitForStmt(clang::ForStmt *FS);
  void VisitReturnStmt(clang::ReturnStmt *RS);
  void VisitSwitchStmt(clang::SwitchStmt *SS);
  void VisitWhileStmt(clang::WhileStmt *WS);
};

void DestructorVisitor::VisitStmt(clang::Stmt *S) {
  for (clang::Stmt* Child : S->children()) {
    if (Child) {
      Visit(Child);
    }
  }
}

void DestructorVisitor::VisitBreakStmt(clang::BreakStmt *BS) {
  VisitStmt(BS);
  if ((mLoopDepth == 0) && (mSwitchDepth == 0)) {
    mExitingStmts.push_back(BS);
  }
}

void DestructorVisitor::VisitContinueStmt(clang::ContinueStmt *CS) {
  VisitStmt(CS);
  if (mLoopDepth == 0) {
    // Switch statements can have nested continues.
    mExitingStmts.push_back(CS);
  }
}

void DestructorVisitor::VisitDoStmt(clang::DoStmt *DS) {
  mLoopDepth++;
  VisitStmt(DS);
  mLoopDepth--;
}

void DestructorVisitor::VisitForStmt(clang::ForStmt *FS) {
  mLoopDepth++;
  VisitStmt(FS);
  mLoopDepth--;
}

void DestructorVisitor::VisitReturnStmt(clang::ReturnStmt *RS) {
  mExitingStmts.push_back(RS);
}

void DestructorVisitor::VisitSwitchStmt(clang::SwitchStmt *SS) {
  mSwitchDepth++;
  VisitStmt(SS);
  mSwitchDepth--;
}

void DestructorVisitor::VisitWhileStmt(clang::WhileStmt *WS) {
  mLoopDepth++;
  VisitStmt(WS);
  mLoopDepth--;
}

clang::Expr *ClearSingleRSObject(clang::ASTContext &C,
                                 clang::Expr *RefRSVar,
                                 clang::SourceLocation Loc) {
  slangAssert(RefRSVar);
  const clang::Type *T = RefRSVar->getType().getTypePtr();
  slangAssert(!T->isArrayType() &&
              "Should not be destroying arrays with this function");

  clang::FunctionDecl *ClearObjectFD = RSObjectRefCount::GetRSClearObjectFD(T);
  slangAssert((ClearObjectFD != nullptr) &&
              "rsClearObject doesn't cover all RS object types");

  clang::QualType ClearObjectFDType = ClearObjectFD->getType();
  clang::QualType ClearObjectFDArgType =
      ClearObjectFD->getParamDecl(0)->getOriginalType();

  // Example destructor for "rs_font localFont;"
  //
  // (CallExpr 'void'
  //   (ImplicitCastExpr 'void (*)(rs_font *)' <FunctionToPointerDecay>
  //     (DeclRefExpr 'void (rs_font *)' FunctionDecl='rsClearObject'))
  //   (UnaryOperator 'rs_font *' prefix '&'
  //     (DeclRefExpr 'rs_font':'rs_font' Var='localFont')))

  // Get address of targeted RS object
  clang::Expr *AddrRefRSVar =
      new(C) clang::UnaryOperator(RefRSVar,
                                  clang::UO_AddrOf,
                                  ClearObjectFDArgType,
                                  clang::VK_RValue,
                                  clang::OK_Ordinary,
                                  Loc);

  clang::Expr *RefRSClearObjectFD =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 ClearObjectFD,
                                 false,
                                 ClearObjectFD->getLocation(),
                                 ClearObjectFDType,
                                 clang::VK_RValue,
                                 nullptr);

  clang::Expr *RSClearObjectFP =
      clang::ImplicitCastExpr::Create(C,
                                      C.getPointerType(ClearObjectFDType),
                                      clang::CK_FunctionToPointerDecay,
                                      RefRSClearObjectFD,
                                      nullptr,
                                      clang::VK_RValue);

  llvm::SmallVector<clang::Expr*, 1> ArgList;
  ArgList.push_back(AddrRefRSVar);

  clang::CallExpr *RSClearObjectCall =
      new(C) clang::CallExpr(C,
                             RSClearObjectFP,
                             ArgList,
                             ClearObjectFD->getCallResultType(),
                             clang::VK_RValue,
                             Loc);

  return RSClearObjectCall;
}

static int ArrayDim(const clang::Type *T) {
  if (!T || !T->isArrayType()) {
    return 0;
  }

  const clang::ConstantArrayType *CAT =
    static_cast<const clang::ConstantArrayType *>(T);
  return static_cast<int>(CAT->getSize().getSExtValue());
}

clang::Stmt *ClearStructRSObject(
    clang::ASTContext &C,
    clang::DeclContext *DC,
    clang::Expr *RefRSStruct,
    clang::SourceLocation StartLoc,
    clang::SourceLocation Loc);

clang::Stmt *ClearArrayRSObject(
    clang::ASTContext &C,
    clang::DeclContext *DC,
    clang::Expr *RefRSArr,
    clang::SourceLocation StartLoc,
    clang::SourceLocation Loc) {
  const clang::Type *BaseType = RefRSArr->getType().getTypePtr();
  slangAssert(BaseType->isArrayType());

  int NumArrayElements = ArrayDim(BaseType);
  // Actually extract out the base RS object type for use later
  BaseType = BaseType->getArrayElementTypeNoTypeQual();

  if (NumArrayElements <= 0) {
    return nullptr;
  }

  // Example destructor loop for "rs_font fontArr[10];"
  //
  // (ForStmt
  //   (DeclStmt
  //     (VarDecl used rsIntIter 'int' cinit
  //       (IntegerLiteral 'int' 0)))
  //   (BinaryOperator 'int' '<'
  //     (ImplicitCastExpr int LValueToRValue
  //       (DeclRefExpr 'int' Var='rsIntIter'))
  //     (IntegerLiteral 'int' 10)
  //   nullptr << CondVar >>
  //   (UnaryOperator 'int' postfix '++'
  //     (DeclRefExpr 'int' Var='rsIntIter'))
  //   (CallExpr 'void'
  //     (ImplicitCastExpr 'void (*)(rs_font *)' <FunctionToPointerDecay>
  //       (DeclRefExpr 'void (rs_font *)' FunctionDecl='rsClearObject'))
  //     (UnaryOperator 'rs_font *' prefix '&'
  //       (ArraySubscriptExpr 'rs_font':'rs_font'
  //         (ImplicitCastExpr 'rs_font *' <ArrayToPointerDecay>
  //           (DeclRefExpr 'rs_font [10]' Var='fontArr'))
  //         (DeclRefExpr 'int' Var='rsIntIter'))))))

  // Create helper variable for iterating through elements
  static unsigned sIterCounter = 0;
  std::stringstream UniqueIterName;
  UniqueIterName << "rsIntIter" << sIterCounter++;
  clang::IdentifierInfo *II = &C.Idents.get(UniqueIterName.str());
  clang::VarDecl *IIVD =
      clang::VarDecl::Create(C,
                             DC,
                             StartLoc,
                             Loc,
                             II,
                             C.IntTy,
                             C.getTrivialTypeSourceInfo(C.IntTy),
                             clang::SC_None);
  // Mark "rsIntIter" as used
  IIVD->markUsed(C);

  // Form the actual destructor loop
  // for (Init; Cond; Inc)
  //   RSClearObjectCall;

  // Init -> "int rsIntIter = 0"
  clang::Expr *Int0 = clang::IntegerLiteral::Create(C,
      llvm::APInt(C.getTypeSize(C.IntTy), 0), C.IntTy, Loc);
  IIVD->setInit(Int0);

  clang::Decl *IID = (clang::Decl *)IIVD;
  clang::DeclGroupRef DGR = clang::DeclGroupRef::Create(C, &IID, 1);
  clang::Stmt *Init = new(C) clang::DeclStmt(DGR, Loc, Loc);

  // Cond -> "rsIntIter < NumArrayElements"
  clang::DeclRefExpr *RefrsIntIterLValue =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 IIVD,
                                 false,
                                 Loc,
                                 C.IntTy,
                                 clang::VK_LValue,
                                 nullptr);

  clang::Expr *RefrsIntIterRValue =
      clang::ImplicitCastExpr::Create(C,
                                      RefrsIntIterLValue->getType(),
                                      clang::CK_LValueToRValue,
                                      RefrsIntIterLValue,
                                      nullptr,
                                      clang::VK_RValue);

  clang::Expr *NumArrayElementsExpr = clang::IntegerLiteral::Create(C,
      llvm::APInt(C.getTypeSize(C.IntTy), NumArrayElements), C.IntTy, Loc);

  clang::BinaryOperator *Cond =
      new(C) clang::BinaryOperator(RefrsIntIterRValue,
                                   NumArrayElementsExpr,
                                   clang::BO_LT,
                                   C.IntTy,
                                   clang::VK_RValue,
                                   clang::OK_Ordinary,
                                   Loc,
                                   false);

  // Inc -> "rsIntIter++"
  clang::UnaryOperator *Inc =
      new(C) clang::UnaryOperator(RefrsIntIterLValue,
                                  clang::UO_PostInc,
                                  C.IntTy,
                                  clang::VK_RValue,
                                  clang::OK_Ordinary,
                                  Loc);

  // Body -> "rsClearObject(&VD[rsIntIter]);"
  // Destructor loop operates on individual array elements

  clang::Expr *RefRSArrPtr =
      clang::ImplicitCastExpr::Create(C,
          C.getPointerType(BaseType->getCanonicalTypeInternal()),
          clang::CK_ArrayToPointerDecay,
          RefRSArr,
          nullptr,
          clang::VK_RValue);

  clang::Expr *RefRSArrPtrSubscript =
      new(C) clang::ArraySubscriptExpr(RefRSArrPtr,
                                       RefrsIntIterRValue,
                                       BaseType->getCanonicalTypeInternal(),
                                       clang::VK_RValue,
                                       clang::OK_Ordinary,
                                       Loc);

  DataType DT = RSExportPrimitiveType::GetRSSpecificType(BaseType);

  clang::Stmt *RSClearObjectCall = nullptr;
  if (BaseType->isArrayType()) {
    RSClearObjectCall =
        ClearArrayRSObject(C, DC, RefRSArrPtrSubscript, StartLoc, Loc);
  } else if (DT == DataTypeUnknown) {
    RSClearObjectCall =
        ClearStructRSObject(C, DC, RefRSArrPtrSubscript, StartLoc, Loc);
  } else {
    RSClearObjectCall = ClearSingleRSObject(C, RefRSArrPtrSubscript, Loc);
  }

  clang::ForStmt *DestructorLoop =
      new(C) clang::ForStmt(C,
                            Init,
                            Cond,
                            nullptr,  // no condVar
                            Inc,
                            RSClearObjectCall,
                            Loc,
                            Loc,
                            Loc);

  return DestructorLoop;
}

unsigned CountRSObjectTypes(const clang::Type *T) {
  slangAssert(T);
  unsigned RSObjectCount = 0;

  if (T->isArrayType()) {
    return CountRSObjectTypes(T->getArrayElementTypeNoTypeQual());
  }

  DataType DT = RSExportPrimitiveType::GetRSSpecificType(T);
  if (DT != DataTypeUnknown) {
    return (RSExportPrimitiveType::IsRSObjectType(DT) ? 1 : 0);
  }

  if (T->isUnionType()) {
    clang::RecordDecl *RD = T->getAsUnionType()->getDecl();
    RD = RD->getDefinition();
    for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
           FE = RD->field_end();
         FI != FE;
         FI++) {
      const clang::FieldDecl *FD = *FI;
      const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
      if (CountRSObjectTypes(FT)) {
        slangAssert(false && "can't have unions with RS object types!");
        return 0;
      }
    }
  }

  if (!T->isStructureType()) {
    return 0;
  }

  clang::RecordDecl *RD = T->getAsStructureType()->getDecl();
  RD = RD->getDefinition();
  for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
         FE = RD->field_end();
       FI != FE;
       FI++) {
    const clang::FieldDecl *FD = *FI;
    const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
    if (CountRSObjectTypes(FT)) {
      // Sub-structs should only count once (as should arrays, etc.)
      RSObjectCount++;
    }
  }

  return RSObjectCount;
}

clang::Stmt *ClearStructRSObject(
    clang::ASTContext &C,
    clang::DeclContext *DC,
    clang::Expr *RefRSStruct,
    clang::SourceLocation StartLoc,
    clang::SourceLocation Loc) {
  const clang::Type *BaseType = RefRSStruct->getType().getTypePtr();

  slangAssert(!BaseType->isArrayType());

  // Structs should show up as unknown primitive types
  slangAssert(RSExportPrimitiveType::GetRSSpecificType(BaseType) ==
              DataTypeUnknown);

  unsigned FieldsToDestroy = CountRSObjectTypes(BaseType);
  slangAssert(FieldsToDestroy != 0);

  unsigned StmtCount = 0;
  clang::Stmt **StmtArray = new clang::Stmt*[FieldsToDestroy];
  for (unsigned i = 0; i < FieldsToDestroy; i++) {
    StmtArray[i] = nullptr;
  }

  // Populate StmtArray by creating a destructor for each RS object field
  clang::RecordDecl *RD = BaseType->getAsStructureType()->getDecl();
  RD = RD->getDefinition();
  for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
         FE = RD->field_end();
       FI != FE;
       FI++) {
    // We just look through all field declarations to see if we find a
    // declaration for an RS object type (or an array of one).
    bool IsArrayType = false;
    clang::FieldDecl *FD = *FI;
    const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
    slangAssert(FT);
    const clang::Type *OrigType = FT;
    while (FT->isArrayType()) {
      FT = FT->getArrayElementTypeNoTypeQual();
      slangAssert(FT);
      IsArrayType = true;
    }

    // Pass a DeclarationNameInfo with a valid DeclName, since name equality
    // gets asserted during CodeGen.
    clang::DeclarationNameInfo FDDeclNameInfo(FD->getDeclName(),
                                              FD->getLocation());

    if (RSExportPrimitiveType::IsRSObjectType(FT)) {
      clang::DeclAccessPair FoundDecl =
          clang::DeclAccessPair::make(FD, clang::AS_none);
      clang::MemberExpr *RSObjectMember =
          clang::MemberExpr::Create(C,
                                    RefRSStruct,
                                    false,
                                    clang::SourceLocation(),
                                    clang::NestedNameSpecifierLoc(),
                                    clang::SourceLocation(),
                                    FD,
                                    FoundDecl,
                                    FDDeclNameInfo,
                                    nullptr,
                                    OrigType->getCanonicalTypeInternal(),
                                    clang::VK_RValue,
                                    clang::OK_Ordinary);

      slangAssert(StmtCount < FieldsToDestroy);

      if (IsArrayType) {
        StmtArray[StmtCount++] = ClearArrayRSObject(C,
                                                    DC,
                                                    RSObjectMember,
                                                    StartLoc,
                                                    Loc);
      } else {
        StmtArray[StmtCount++] = ClearSingleRSObject(C,
                                                     RSObjectMember,
                                                     Loc);
      }
    } else if (FT->isStructureType() && CountRSObjectTypes(FT)) {
      // In this case, we have a nested struct. We may not end up filling all
      // of the spaces in StmtArray (sub-structs should handle themselves
      // with separate compound statements).
      clang::DeclAccessPair FoundDecl =
          clang::DeclAccessPair::make(FD, clang::AS_none);
      clang::MemberExpr *RSObjectMember =
          clang::MemberExpr::Create(C,
                                    RefRSStruct,
                                    false,
                                    clang::SourceLocation(),
                                    clang::NestedNameSpecifierLoc(),
                                    clang::SourceLocation(),
                                    FD,
                                    FoundDecl,
                                    clang::DeclarationNameInfo(),
                                    nullptr,
                                    OrigType->getCanonicalTypeInternal(),
                                    clang::VK_RValue,
                                    clang::OK_Ordinary);

      if (IsArrayType) {
        StmtArray[StmtCount++] = ClearArrayRSObject(C,
                                                    DC,
                                                    RSObjectMember,
                                                    StartLoc,
                                                    Loc);
      } else {
        StmtArray[StmtCount++] = ClearStructRSObject(C,
                                                     DC,
                                                     RSObjectMember,
                                                     StartLoc,
                                                     Loc);
      }
    }
  }

  slangAssert(StmtCount > 0);
  clang::CompoundStmt *CS = new(C) clang::CompoundStmt(
      C, llvm::makeArrayRef(StmtArray, StmtCount), Loc, Loc);

  delete [] StmtArray;

  return CS;
}

clang::Stmt *CreateSingleRSSetObject(clang::ASTContext &C,
                                     clang::Expr *DstExpr,
                                     clang::Expr *SrcExpr,
                                     clang::SourceLocation StartLoc,
                                     clang::SourceLocation Loc) {
  const clang::Type *T = DstExpr->getType().getTypePtr();
  clang::FunctionDecl *SetObjectFD = RSObjectRefCount::GetRSSetObjectFD(T);
  slangAssert((SetObjectFD != nullptr) &&
              "rsSetObject doesn't cover all RS object types");

  clang::QualType SetObjectFDType = SetObjectFD->getType();
  clang::QualType SetObjectFDArgType[2];
  SetObjectFDArgType[0] = SetObjectFD->getParamDecl(0)->getOriginalType();
  SetObjectFDArgType[1] = SetObjectFD->getParamDecl(1)->getOriginalType();

  clang::Expr *RefRSSetObjectFD =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 SetObjectFD,
                                 false,
                                 Loc,
                                 SetObjectFDType,
                                 clang::VK_RValue,
                                 nullptr);

  clang::Expr *RSSetObjectFP =
      clang::ImplicitCastExpr::Create(C,
                                      C.getPointerType(SetObjectFDType),
                                      clang::CK_FunctionToPointerDecay,
                                      RefRSSetObjectFD,
                                      nullptr,
                                      clang::VK_RValue);

  llvm::SmallVector<clang::Expr*, 2> ArgList;
  ArgList.push_back(new(C) clang::UnaryOperator(DstExpr,
                                                clang::UO_AddrOf,
                                                SetObjectFDArgType[0],
                                                clang::VK_RValue,
                                                clang::OK_Ordinary,
                                                Loc));
  ArgList.push_back(SrcExpr);

  clang::CallExpr *RSSetObjectCall =
      new(C) clang::CallExpr(C,
                             RSSetObjectFP,
                             ArgList,
                             SetObjectFD->getCallResultType(),
                             clang::VK_RValue,
                             Loc);

  return RSSetObjectCall;
}

clang::Stmt *CreateStructRSSetObject(clang::ASTContext &C,
                                     clang::Expr *LHS,
                                     clang::Expr *RHS,
                                     clang::SourceLocation StartLoc,
                                     clang::SourceLocation Loc);

/*static clang::Stmt *CreateArrayRSSetObject(clang::ASTContext &C,
                                           clang::Expr *DstArr,
                                           clang::Expr *SrcArr,
                                           clang::SourceLocation StartLoc,
                                           clang::SourceLocation Loc) {
  clang::DeclContext *DC = nullptr;
  const clang::Type *BaseType = DstArr->getType().getTypePtr();
  slangAssert(BaseType->isArrayType());

  int NumArrayElements = ArrayDim(BaseType);
  // Actually extract out the base RS object type for use later
  BaseType = BaseType->getArrayElementTypeNoTypeQual();

  clang::Stmt *StmtArray[2] = {nullptr};
  int StmtCtr = 0;

  if (NumArrayElements <= 0) {
    return nullptr;
  }

  // Create helper variable for iterating through elements
  clang::IdentifierInfo& II = C.Idents.get("rsIntIter");
  clang::VarDecl *IIVD =
      clang::VarDecl::Create(C,
                             DC,
                             StartLoc,
                             Loc,
                             &II,
                             C.IntTy,
                             C.getTrivialTypeSourceInfo(C.IntTy),
                             clang::SC_None,
                             clang::SC_None);
  clang::Decl *IID = (clang::Decl *)IIVD;

  clang::DeclGroupRef DGR = clang::DeclGroupRef::Create(C, &IID, 1);
  StmtArray[StmtCtr++] = new(C) clang::DeclStmt(DGR, Loc, Loc);

  // Form the actual loop
  // for (Init; Cond; Inc)
  //   RSSetObjectCall;

  // Init -> "rsIntIter = 0"
  clang::DeclRefExpr *RefrsIntIter =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 IIVD,
                                 Loc,
                                 C.IntTy,
                                 clang::VK_RValue,
                                 nullptr);

  clang::Expr *Int0 = clang::IntegerLiteral::Create(C,
      llvm::APInt(C.getTypeSize(C.IntTy), 0), C.IntTy, Loc);

  clang::BinaryOperator *Init =
      new(C) clang::BinaryOperator(RefrsIntIter,
                                   Int0,
                                   clang::BO_Assign,
                                   C.IntTy,
                                   clang::VK_RValue,
                                   clang::OK_Ordinary,
                                   Loc);

  // Cond -> "rsIntIter < NumArrayElements"
  clang::Expr *NumArrayElementsExpr = clang::IntegerLiteral::Create(C,
      llvm::APInt(C.getTypeSize(C.IntTy), NumArrayElements), C.IntTy, Loc);

  clang::BinaryOperator *Cond =
      new(C) clang::BinaryOperator(RefrsIntIter,
                                   NumArrayElementsExpr,
                                   clang::BO_LT,
                                   C.IntTy,
                                   clang::VK_RValue,
                                   clang::OK_Ordinary,
                                   Loc);

  // Inc -> "rsIntIter++"
  clang::UnaryOperator *Inc =
      new(C) clang::UnaryOperator(RefrsIntIter,
                                  clang::UO_PostInc,
                                  C.IntTy,
                                  clang::VK_RValue,
                                  clang::OK_Ordinary,
                                  Loc);

  // Body -> "rsSetObject(&Dst[rsIntIter], Src[rsIntIter]);"
  // Loop operates on individual array elements

  clang::Expr *DstArrPtr =
      clang::ImplicitCastExpr::Create(C,
          C.getPointerType(BaseType->getCanonicalTypeInternal()),
          clang::CK_ArrayToPointerDecay,
          DstArr,
          nullptr,
          clang::VK_RValue);

  clang::Expr *DstArrPtrSubscript =
      new(C) clang::ArraySubscriptExpr(DstArrPtr,
                                       RefrsIntIter,
                                       BaseType->getCanonicalTypeInternal(),
                                       clang::VK_RValue,
                                       clang::OK_Ordinary,
                                       Loc);

  clang::Expr *SrcArrPtr =
      clang::ImplicitCastExpr::Create(C,
          C.getPointerType(BaseType->getCanonicalTypeInternal()),
          clang::CK_ArrayToPointerDecay,
          SrcArr,
          nullptr,
          clang::VK_RValue);

  clang::Expr *SrcArrPtrSubscript =
      new(C) clang::ArraySubscriptExpr(SrcArrPtr,
                                       RefrsIntIter,
                                       BaseType->getCanonicalTypeInternal(),
                                       clang::VK_RValue,
                                       clang::OK_Ordinary,
                                       Loc);

  DataType DT = RSExportPrimitiveType::GetRSSpecificType(BaseType);

  clang::Stmt *RSSetObjectCall = nullptr;
  if (BaseType->isArrayType()) {
    RSSetObjectCall = CreateArrayRSSetObject(C, DstArrPtrSubscript,
                                             SrcArrPtrSubscript,
                                             StartLoc, Loc);
  } else if (DT == DataTypeUnknown) {
    RSSetObjectCall = CreateStructRSSetObject(C, DstArrPtrSubscript,
                                              SrcArrPtrSubscript,
                                              StartLoc, Loc);
  } else {
    RSSetObjectCall = CreateSingleRSSetObject(C, DstArrPtrSubscript,
                                              SrcArrPtrSubscript,
                                              StartLoc, Loc);
  }

  clang::ForStmt *DestructorLoop =
      new(C) clang::ForStmt(C,
                            Init,
                            Cond,
                            nullptr,  // no condVar
                            Inc,
                            RSSetObjectCall,
                            Loc,
                            Loc,
                            Loc);

  StmtArray[StmtCtr++] = DestructorLoop;
  slangAssert(StmtCtr == 2);

  clang::CompoundStmt *CS =
      new(C) clang::CompoundStmt(C, StmtArray, StmtCtr, Loc, Loc);

  return CS;
} */

clang::Stmt *CreateStructRSSetObject(clang::ASTContext &C,
                                     clang::Expr *LHS,
                                     clang::Expr *RHS,
                                     clang::SourceLocation StartLoc,
                                     clang::SourceLocation Loc) {
  clang::QualType QT = LHS->getType();
  const clang::Type *T = QT.getTypePtr();
  slangAssert(T->isStructureType());
  slangAssert(!RSExportPrimitiveType::IsRSObjectType(T));

  // Keep an extra slot for the original copy (memcpy)
  unsigned FieldsToSet = CountRSObjectTypes(T) + 1;

  unsigned StmtCount = 0;
  clang::Stmt **StmtArray = new clang::Stmt*[FieldsToSet];
  for (unsigned i = 0; i < FieldsToSet; i++) {
    StmtArray[i] = nullptr;
  }

  clang::RecordDecl *RD = T->getAsStructureType()->getDecl();
  RD = RD->getDefinition();
  for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
         FE = RD->field_end();
       FI != FE;
       FI++) {
    bool IsArrayType = false;
    clang::FieldDecl *FD = *FI;
    const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
    const clang::Type *OrigType = FT;

    if (!CountRSObjectTypes(FT)) {
      // Skip to next if we don't have any viable RS object types
      continue;
    }

    clang::DeclAccessPair FoundDecl =
        clang::DeclAccessPair::make(FD, clang::AS_none);
    clang::MemberExpr *DstMember =
        clang::MemberExpr::Create(C,
                                  LHS,
                                  false,
                                  clang::SourceLocation(),
                                  clang::NestedNameSpecifierLoc(),
                                  clang::SourceLocation(),
                                  FD,
                                  FoundDecl,
                                  clang::DeclarationNameInfo(
                                      FD->getDeclName(),
                                      clang::SourceLocation()),
                                  nullptr,
                                  OrigType->getCanonicalTypeInternal(),
                                  clang::VK_RValue,
                                  clang::OK_Ordinary);

    clang::MemberExpr *SrcMember =
        clang::MemberExpr::Create(C,
                                  RHS,
                                  false,
                                  clang::SourceLocation(),
                                  clang::NestedNameSpecifierLoc(),
                                  clang::SourceLocation(),
                                  FD,
                                  FoundDecl,
                                  clang::DeclarationNameInfo(
                                      FD->getDeclName(),
                                      clang::SourceLocation()),
                                  nullptr,
                                  OrigType->getCanonicalTypeInternal(),
                                  clang::VK_RValue,
                                  clang::OK_Ordinary);

    if (FT->isArrayType()) {
      FT = FT->getArrayElementTypeNoTypeQual();
      IsArrayType = true;
    }

    DataType DT = RSExportPrimitiveType::GetRSSpecificType(FT);

    if (IsArrayType) {
      clang::DiagnosticsEngine &DiagEngine = C.getDiagnostics();
      DiagEngine.Report(
        clang::FullSourceLoc(Loc, C.getSourceManager()),
        DiagEngine.getCustomDiagID(
          clang::DiagnosticsEngine::Error,
          "Arrays of RS object types within structures cannot be copied"));
      // TODO(srhines): Support setting arrays of RS objects
      // StmtArray[StmtCount++] =
      //    CreateArrayRSSetObject(C, DstMember, SrcMember, StartLoc, Loc);
    } else if (DT == DataTypeUnknown) {
      StmtArray[StmtCount++] =
          CreateStructRSSetObject(C, DstMember, SrcMember, StartLoc, Loc);
    } else if (RSExportPrimitiveType::IsRSObjectType(DT)) {
      StmtArray[StmtCount++] =
          CreateSingleRSSetObject(C, DstMember, SrcMember, StartLoc, Loc);
    } else {
      slangAssert(false);
    }
  }

  slangAssert(StmtCount < FieldsToSet);

  // We still need to actually do the overall struct copy. For simplicity,
  // we just do a straight-up assignment (which will still preserve all
  // the proper RS object reference counts).
  clang::BinaryOperator *CopyStruct =
      new(C) clang::BinaryOperator(LHS, RHS, clang::BO_Assign, QT,
                                   clang::VK_RValue, clang::OK_Ordinary, Loc,
                                   false);
  StmtArray[StmtCount++] = CopyStruct;

  clang::CompoundStmt *CS = new(C) clang::CompoundStmt(
      C, llvm::makeArrayRef(StmtArray, StmtCount), Loc, Loc);

  delete [] StmtArray;

  return CS;
}

}  // namespace

void RSObjectRefCount::Scope::InsertStmt(const clang::ASTContext &C,
                                         clang::Stmt *NewStmt) {
  std::vector<clang::Stmt*> newBody;
  for (clang::Stmt* S1 : mCS->body()) {
    if (S1 == mCurrent) {
      newBody.push_back(NewStmt);
    }
    newBody.push_back(S1);
  }
  mCS->setStmts(C, newBody);
}

void RSObjectRefCount::Scope::ReplaceStmt(const clang::ASTContext &C,
                                          clang::Stmt *NewStmt) {
  std::vector<clang::Stmt*> newBody;
  for (clang::Stmt* S1 : mCS->body()) {
    if (S1 == mCurrent) {
      newBody.push_back(NewStmt);
    } else {
      newBody.push_back(S1);
    }
  }
  mCS->setStmts(C, newBody);
}

void RSObjectRefCount::Scope::ReplaceExpr(const clang::ASTContext& C,
                                          clang::Expr* OldExpr,
                                          clang::Expr* NewExpr) {
  RSASTReplace R(C);
  R.ReplaceStmt(mCurrent, OldExpr, NewExpr);
}

void RSObjectRefCount::Scope::ReplaceRSObjectAssignment(
    clang::BinaryOperator *AS) {

  clang::QualType QT = AS->getType();

  clang::ASTContext &C = RSObjectRefCount::GetRSSetObjectFD(
      DataTypeRSAllocation)->getASTContext();

  clang::SourceLocation Loc = AS->getExprLoc();
  clang::SourceLocation StartLoc = AS->getLHS()->getExprLoc();
  clang::Stmt *UpdatedStmt = nullptr;

  if (!RSExportPrimitiveType::IsRSObjectType(QT.getTypePtr())) {
    // By definition, this is a struct assignment if we get here
    UpdatedStmt =
        CreateStructRSSetObject(C, AS->getLHS(), AS->getRHS(), StartLoc, Loc);
  } else {
    UpdatedStmt =
        CreateSingleRSSetObject(C, AS->getLHS(), AS->getRHS(), StartLoc, Loc);
  }

  RSASTReplace R(C);
  R.ReplaceStmt(mCS, AS, UpdatedStmt);
}

void RSObjectRefCount::Scope::AppendRSObjectInit(
    clang::VarDecl *VD,
    clang::DeclStmt *DS,
    DataType DT,
    clang::Expr *InitExpr) {
  slangAssert(VD);

  if (!InitExpr) {
    return;
  }

  clang::ASTContext &C = RSObjectRefCount::GetRSSetObjectFD(
      DataTypeRSAllocation)->getASTContext();
  clang::SourceLocation Loc = RSObjectRefCount::GetRSSetObjectFD(
      DataTypeRSAllocation)->getLocation();
  clang::SourceLocation StartLoc = RSObjectRefCount::GetRSSetObjectFD(
      DataTypeRSAllocation)->getInnerLocStart();

  if (DT == DataTypeIsStruct) {
    const clang::Type *T = RSExportType::GetTypeOfDecl(VD);
    clang::DeclRefExpr *RefRSVar =
        clang::DeclRefExpr::Create(C,
                                   clang::NestedNameSpecifierLoc(),
                                   clang::SourceLocation(),
                                   VD,
                                   false,
                                   Loc,
                                   T->getCanonicalTypeInternal(),
                                   clang::VK_RValue,
                                   nullptr);

    clang::Stmt *RSSetObjectOps =
        CreateStructRSSetObject(C, RefRSVar, InitExpr, StartLoc, Loc);
    // Fix for b/37363420; consider:
    //
    // struct foo { rs_matrix m; };
    // void bar() {
    //   struct foo M = {...};
    // }
    //
    // slang modifies that declaration with initialization to a
    // declaration plus an assignment of the initialization values.
    //
    // void bar() {
    //   struct foo M = {};
    //   M = {...}; // by CreateStructRSSetObject() above
    // }
    //
    // the slang-generated statement (M = {...}) is a use of M, and we
    // need to mark M (clang::VarDecl *VD) as used.
    VD->markUsed(C);

    std::list<clang::Stmt*> StmtList;
    StmtList.push_back(RSSetObjectOps);
    AppendAfterStmt(C, mCS, DS, StmtList);
    return;
  }

  clang::FunctionDecl *SetObjectFD = RSObjectRefCount::GetRSSetObjectFD(DT);
  slangAssert((SetObjectFD != nullptr) &&
              "rsSetObject doesn't cover all RS object types");

  clang::QualType SetObjectFDType = SetObjectFD->getType();
  clang::QualType SetObjectFDArgType[2];
  SetObjectFDArgType[0] = SetObjectFD->getParamDecl(0)->getOriginalType();
  SetObjectFDArgType[1] = SetObjectFD->getParamDecl(1)->getOriginalType();

  clang::Expr *RefRSSetObjectFD =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 SetObjectFD,
                                 false,
                                 Loc,
                                 SetObjectFDType,
                                 clang::VK_RValue,
                                 nullptr);

  clang::Expr *RSSetObjectFP =
      clang::ImplicitCastExpr::Create(C,
                                      C.getPointerType(SetObjectFDType),
                                      clang::CK_FunctionToPointerDecay,
                                      RefRSSetObjectFD,
                                      nullptr,
                                      clang::VK_RValue);

  const clang::Type *T = RSExportType::GetTypeOfDecl(VD);
  clang::DeclRefExpr *RefRSVar =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 VD,
                                 false,
                                 Loc,
                                 T->getCanonicalTypeInternal(),
                                 clang::VK_RValue,
                                 nullptr);

  llvm::SmallVector<clang::Expr*, 2> ArgList;
  ArgList.push_back(new(C) clang::UnaryOperator(RefRSVar,
                                                clang::UO_AddrOf,
                                                SetObjectFDArgType[0],
                                                clang::VK_RValue,
                                                clang::OK_Ordinary,
                                                Loc));
  ArgList.push_back(InitExpr);

  clang::CallExpr *RSSetObjectCall =
      new(C) clang::CallExpr(C,
                             RSSetObjectFP,
                             ArgList,
                             SetObjectFD->getCallResultType(),
                             clang::VK_RValue,
                             Loc);

  std::list<clang::Stmt*> StmtList;
  StmtList.push_back(RSSetObjectCall);
  AppendAfterStmt(C, mCS, DS, StmtList);
}

void RSObjectRefCount::Scope::InsertLocalVarDestructors() {
  if (mRSO.empty()) {
    return;
  }

  clang::DeclContext* DC = mRSO.front()->getDeclContext();
  clang::ASTContext& C = DC->getParentASTContext();
  clang::SourceManager& SM = C.getSourceManager();

  const auto& OccursBefore = [&SM] (clang::SourceLocation L1, clang::SourceLocation L2)->bool {
    return SM.isBeforeInTranslationUnit(L1, L2);
  };
  typedef std::map<clang::SourceLocation, clang::Stmt*, decltype(OccursBefore)> DMap;

  DMap dtors(OccursBefore);

  // Create rsClearObject calls. Note the DMap entries are sorted by the SourceLocation.
  for (clang::VarDecl* VD : mRSO) {
    clang::SourceLocation Loc = VD->getSourceRange().getBegin();
    clang::Stmt* RSClearObjectCall = ClearRSObject(VD, DC);
    dtors.insert(std::make_pair(Loc, RSClearObjectCall));
  }

  DestructorVisitor Visitor;
  Visitor.Visit(mCS);

  // Replace each exiting statement with a block that contains the original statement
  // and added rsClearObject() calls before it.
  for (clang::Stmt* S : Visitor.getExitingStmts()) {

    const clang::SourceLocation currentLoc = S->getLocStart();

    DMap::iterator firstDtorIter = dtors.begin();
    DMap::iterator currentDtorIter = firstDtorIter;
    DMap::iterator lastDtorIter = dtors.end();

    while (currentDtorIter != lastDtorIter &&
           OccursBefore(currentDtorIter->first, currentLoc)) {
      currentDtorIter++;
    }

    if (currentDtorIter == firstDtorIter) {
      continue;
    }

    std::vector<clang::Stmt*> Stmts;

    // Insert rsClearObject() calls for all rsObjects declared before the current statement
    for(DMap::iterator it = firstDtorIter; it != currentDtorIter; it++) {
      Stmts.push_back(it->second);
    }
    Stmts.push_back(S);

    RSASTReplace R(C);
    clang::CompoundStmt* CS = BuildCompoundStmt(C, Stmts, S->getLocEnd());
    R.ReplaceStmt(mCS, S, CS);
  }

  std::list<clang::Stmt*> Stmts;
  for(auto LocCallPair : dtors) {
    Stmts.push_back(LocCallPair.second);
  }
  AppendAfterStmt(C, mCS, nullptr, Stmts);
}

clang::Stmt *RSObjectRefCount::Scope::ClearRSObject(
    clang::VarDecl *VD,
    clang::DeclContext *DC) {
  slangAssert(VD);
  clang::ASTContext &C = VD->getASTContext();
  clang::SourceLocation Loc = VD->getLocation();
  clang::SourceLocation StartLoc = VD->getInnerLocStart();
  const clang::Type *T = RSExportType::GetTypeOfDecl(VD);

  // Reference expr to target RS object variable
  clang::DeclRefExpr *RefRSVar =
      clang::DeclRefExpr::Create(C,
                                 clang::NestedNameSpecifierLoc(),
                                 clang::SourceLocation(),
                                 VD,
                                 false,
                                 Loc,
                                 T->getCanonicalTypeInternal(),
                                 clang::VK_RValue,
                                 nullptr);

  if (T->isArrayType()) {
    return ClearArrayRSObject(C, DC, RefRSVar, StartLoc, Loc);
  }

  DataType DT = RSExportPrimitiveType::GetRSSpecificType(T);

  if (DT == DataTypeUnknown ||
      DT == DataTypeIsStruct) {
    return ClearStructRSObject(C, DC, RefRSVar, StartLoc, Loc);
  }

  slangAssert((RSExportPrimitiveType::IsRSObjectType(DT)) &&
              "Should be RS object");

  return ClearSingleRSObject(C, RefRSVar, Loc);
}

bool RSObjectRefCount::InitializeRSObject(clang::VarDecl *VD,
                                          DataType *DT,
                                          clang::Expr **InitExpr) {
  slangAssert(VD && DT && InitExpr);
  const clang::Type *T = RSExportType::GetTypeOfDecl(VD);

  // Loop through array types to get to base type
  slangAssert(T);
  while (T->isArrayType()) {
    T = T->getArrayElementTypeNoTypeQual();
    slangAssert(T);
  }

  bool DataTypeIsStructWithRSObject = false;
  *DT = RSExportPrimitiveType::GetRSSpecificType(T);

  if (*DT == DataTypeUnknown) {
    if (RSExportPrimitiveType::IsStructureTypeWithRSObject(T)) {
      *DT = DataTypeIsStruct;
      DataTypeIsStructWithRSObject = true;
    } else {
      return false;
    }
  }

  bool DataTypeIsRSObject = false;
  if (DataTypeIsStructWithRSObject) {
    DataTypeIsRSObject = true;
  } else {
    DataTypeIsRSObject = RSExportPrimitiveType::IsRSObjectType(*DT);
  }
  *InitExpr = VD->getInit();

  if (!DataTypeIsRSObject && *InitExpr) {
    // If we already have an initializer for a matrix type, we are done.
    return DataTypeIsRSObject;
  }

  clang::Expr *ZeroInitializer =
      CreateEmptyInitListExpr(VD->getASTContext(), VD->getLocation());

  if (ZeroInitializer) {
    ZeroInitializer->setType(T->getCanonicalTypeInternal());
    VD->setInit(ZeroInitializer);
  }

  return DataTypeIsRSObject;
}

clang::Expr *RSObjectRefCount::CreateEmptyInitListExpr(
    clang::ASTContext &C,
    const clang::SourceLocation &Loc) {

  // We can cheaply construct a zero initializer by just creating an empty
  // initializer list. Clang supports this extension to C(99), and will create
  // any necessary constructs to zero out the entire variable.
  llvm::SmallVector<clang::Expr*, 1> EmptyInitList;
  return new(C) clang::InitListExpr(C, Loc, EmptyInitList, Loc);
}

clang::DeclRefExpr *RSObjectRefCount::CreateGuard(clang::ASTContext &C,
                                                  clang::DeclContext *DC,
                                                  clang::Expr *E,
                                                  const llvm::Twine &VarName,
                                                  std::vector<clang::Stmt*> &NewStmts) {
  clang::SourceLocation Loc = E->getLocStart();
  const clang::QualType Ty = E->getType();
  clang::VarDecl* TmpDecl = clang::VarDecl::Create(
      C,                                     // AST context
      DC,                                    // Decl context
      Loc,                                   // Start location
      Loc,                                   // Id location
      &C.Idents.get(VarName.str()),          // Id
      Ty,                                    // Type
      C.getTrivialTypeSourceInfo(Ty),        // Type info
      clang::SC_None                         // Storage class
  );
  const clang::Type *T = Ty.getTypePtr();
  clang::Expr *ZeroInitializer =
      RSObjectRefCount::CreateEmptyInitListExpr(C, Loc);
  ZeroInitializer->setType(T->getCanonicalTypeInternal());
  TmpDecl->setInit(ZeroInitializer);
  TmpDecl->markUsed(C);
  clang::Decl* Decls[] = { TmpDecl };
  const clang::DeclGroupRef DGR = clang::DeclGroupRef::Create(
      C, Decls, sizeof(Decls) / sizeof(*Decls));
  clang::DeclStmt* DS = new (C) clang::DeclStmt(DGR, Loc, Loc);
  NewStmts.push_back(DS);

  clang::DeclRefExpr* DRE = clang::DeclRefExpr::Create(
      C,
      clang::NestedNameSpecifierLoc(),       // QualifierLoc
      Loc,                                   // TemplateKWLoc
      TmpDecl,
      false,                                 // RefersToEnclosingVariableOrCapture
      Loc,                                   // NameLoc
      Ty,
      clang::VK_LValue
  );

  clang::Stmt *UpdatedStmt = nullptr;
  if (CountRSObjectTypes(Ty.getTypePtr()) == 0) {
    // The expression E is not an RS object itself. Instead of calling
    // rsSetObject(), create an assignment statement to set the value of the
    // temporary "guard" variable to the expression.
    // This can happen if called from RSObjectRefCount::VisitReturnStmt(),
    // when the return expression is not an RS object but references one.
    UpdatedStmt =
      new(C) clang::BinaryOperator(DRE, E, clang::BO_Assign, Ty,
                                   clang::VK_RValue, clang::OK_Ordinary, Loc,
                                   false);

  } else if (!RSExportPrimitiveType::IsRSObjectType(Ty.getTypePtr())) {
    // By definition, this is a struct assignment if we get here
    UpdatedStmt =
        CreateStructRSSetObject(C, DRE, E, Loc, Loc);
  } else {
    UpdatedStmt =
        CreateSingleRSSetObject(C, DRE, E, Loc, Loc);
  }
  NewStmts.push_back(UpdatedStmt);

  return DRE;
}

void RSObjectRefCount::CreateParameterGuard(clang::ASTContext &C,
                                            clang::DeclContext *DC,
                                            clang::ParmVarDecl *PD,
                                            std::vector<clang::Stmt*> &NewStmts) {
  clang::SourceLocation Loc = PD->getLocStart();
  clang::DeclRefExpr* ParamDRE = clang::DeclRefExpr::Create(
      C,
      clang::NestedNameSpecifierLoc(),       // QualifierLoc
      Loc,                                   // TemplateKWLoc
      PD,
      false,                                 // RefersToEnclosingVariableOrCapture
      Loc,                                   // NameLoc
      PD->getType(),
      clang::VK_RValue
  );

  CreateGuard(C, DC, ParamDRE,
              llvm::Twine(".rs.param.") + llvm::Twine(PD->getName()), NewStmts);
}

void RSObjectRefCount::HandleParamsAndLocals(clang::FunctionDecl *FD) {
  std::vector<clang::Stmt*> NewStmts;
  std::list<clang::ParmVarDecl*> ObjParams;
  for (clang::ParmVarDecl *Param : FD->parameters()) {
    clang::QualType QT = Param->getType();
    if (CountRSObjectTypes(QT.getTypePtr())) {
      // Ignore non-object types
      RSObjectRefCount::CreateParameterGuard(mCtx, FD, Param, NewStmts);
      ObjParams.push_back(Param);
    }
  }

  clang::Stmt *OldBody = FD->getBody();
  if (ObjParams.empty()) {
    Visit(OldBody);
    return;
  }

  NewStmts.push_back(OldBody);

  clang::SourceLocation Loc = FD->getLocStart();
  clang::CompoundStmt *NewBody = BuildCompoundStmt(mCtx, NewStmts, Loc);
  Scope S(NewBody);
  for (clang::ParmVarDecl *Param : ObjParams) {
    S.addRSObject(Param);
  }
  mScopeStack.push_back(&S);

  // To avoid adding unnecessary ref counting artifacts to newly added temporary
  // local variables for parameters, visits only the old function body here.
  Visit(OldBody);

  FD->setBody(NewBody);

  S.InsertLocalVarDestructors();
  mScopeStack.pop_back();
}

clang::CompoundStmt* RSObjectRefCount::CreateRetStmtWithTempVar(
    clang::ASTContext& C,
    clang::DeclContext* DC,
    clang::ReturnStmt* RS,
    const unsigned id) {
  std::vector<clang::Stmt*> NewStmts;
  // Since we insert rsClearObj() calls before the return statement, we need
  // to make sure none of the cleared RS objects are referenced in the
  // return statement.
  // For that, we create a new local variable named .rs.retval, assign the
  // original return expression to it, make all necessary rsClearObj()
  // calls, then return .rs.retval. Note rsClearObj() is not called on
  // .rs.retval.
  clang::SourceLocation Loc = RS->getLocStart();
  clang::Expr* RetVal = RS->getRetValue();
  const clang::QualType RetTy = RetVal->getType();
  clang::DeclRefExpr *DRE = CreateGuard(C, DC, RetVal,
                                        llvm::Twine(".rs.retval") + llvm::Twine(id),
                                        NewStmts);

  // Creates a new return statement
  clang::ReturnStmt* NewRet = new (C) clang::ReturnStmt(Loc);
  clang::Expr* CastExpr = clang::ImplicitCastExpr::Create(
      C,
      RetTy,
      clang::CK_LValueToRValue,
      DRE,
      nullptr,
      clang::VK_RValue
  );
  NewRet->setRetValue(CastExpr);
  NewStmts.push_back(NewRet);

  return BuildCompoundStmt(C, NewStmts, Loc);
}

void RSObjectRefCount::VisitDeclStmt(clang::DeclStmt *DS) {
  VisitStmt(DS);
  getCurrentScope()->setCurrentStmt(DS);
  for (clang::DeclStmt::decl_iterator I = DS->decl_begin(), E = DS->decl_end();
       I != E;
       I++) {
    clang::Decl *D = *I;
    if (D->getKind() == clang::Decl::Var) {
      clang::VarDecl *VD = static_cast<clang::VarDecl*>(D);
      DataType DT = DataTypeUnknown;
      clang::Expr *InitExpr = nullptr;
      if (InitializeRSObject(VD, &DT, &InitExpr)) {
        // We need to zero-init all RS object types (including matrices), ...
        getCurrentScope()->AppendRSObjectInit(VD, DS, DT, InitExpr);
        // ... but, only add to the list of RS objects if we have some
        // non-matrix RS object fields.
        if (CountRSObjectTypes(VD->getType().getTypePtr())) {
          getCurrentScope()->addRSObject(VD);
        }
      }
    }
  }
}

void RSObjectRefCount::VisitCallExpr(clang::CallExpr* CE) {
  clang::QualType RetTy;
  const clang::FunctionDecl* FD = CE->getDirectCallee();

  if (FD) {
    // Direct calls

    RetTy = FD->getReturnType();
  } else {
    // Indirect calls through function pointers

    const clang::Expr* Callee = CE->getCallee();
    const clang::Type* CalleeType = Callee->getType().getTypePtr();
    const clang::PointerType* PtrType = CalleeType->getAs<clang::PointerType>();

    if (!PtrType) {
      return;
    }

    const clang::Type* PointeeType = PtrType->getPointeeType().getTypePtr();
    const clang::FunctionType* FuncType = PointeeType->getAs<clang::FunctionType>();

    if (!FuncType) {
      return;
    }

    RetTy = FuncType->getReturnType();
  }

  // The RenderScript runtime API maintains the invariant that the sysRef of a new RS object would
  // be 1, with the exception of rsGetAllocation() (deprecated in API 22), which leaves the sysRef
  // 0 for a new allocation. It is the responsibility of the callee of the API to decrement the
  // sysRef when a reference of the RS object goes out of scope. The compiler generates code to do
  // just that, by creating a temporary variable named ".rs.tmpN" with the result of
  // an RS-object-returning API directly assigned to it, and calling rsClearObject() on .rs.tmpN
  // right before it exits the current scope. Such code generation is skipped for rsGetAllocation()
  // to avoid decrementing its sysRef below zero.

  if (CountRSObjectTypes(RetTy.getTypePtr())==0 ||
      (FD && FD->getName() == "rsGetAllocation")) {
    return;
  }

  clang::SourceLocation Loc = CE->getSourceRange().getBegin();
  std::stringstream ss;
  ss << ".rs.tmp" << getNextID();
  clang::IdentifierInfo *II = &mCtx.Idents.get(ss.str());

  clang::VarDecl* TempVarDecl = clang::VarDecl::Create(
      mCtx,                                  // AST context
      GetDeclContext(),                      // Decl context
      Loc,                                   // Start location
      Loc,                                   // Id location
      II,                                    // Id
      RetTy,                                 // Type
      mCtx.getTrivialTypeSourceInfo(RetTy),  // Type info
      clang::SC_None                         // Storage class
  );
  TempVarDecl->setInit(CE);
  TempVarDecl->markUsed(mCtx);
  clang::Decl* Decls[] = { TempVarDecl };
  const clang::DeclGroupRef DGR = clang::DeclGroupRef::Create(
      mCtx, Decls, sizeof(Decls) / sizeof(*Decls));
  clang::DeclStmt* DS = new (mCtx) clang::DeclStmt(DGR, Loc, Loc);

  getCurrentScope()->InsertStmt(mCtx, DS);

  clang::DeclRefExpr* DRE = clang::DeclRefExpr::Create(
      mCtx,                                  // AST context
      clang::NestedNameSpecifierLoc(),       // QualifierLoc
      Loc,                                   // TemplateKWLoc
      TempVarDecl,
      false,                                 // RefersToEnclosingVariableOrCapture
      Loc,                                   // NameLoc
      RetTy,
      clang::VK_LValue
  );
  clang::Expr* CastExpr = clang::ImplicitCastExpr::Create(
      mCtx,
      RetTy,
      clang::CK_LValueToRValue,
      DRE,
      nullptr,
      clang::VK_RValue
  );

  getCurrentScope()->ReplaceExpr(mCtx, CE, CastExpr);

  // Register TempVarDecl for destruction call (rsClearObj).
  getCurrentScope()->addRSObject(TempVarDecl);
}

void RSObjectRefCount::VisitCompoundStmt(clang::CompoundStmt *CS) {
  if (!emptyScope()) {
    getCurrentScope()->setCurrentStmt(CS);
  }

  if (!CS->body_empty()) {
    // Push a new scope
    Scope *S = new Scope(CS);
    mScopeStack.push_back(S);

    VisitStmt(CS);

    // Destroy the scope
    slangAssert((getCurrentScope() == S) && "Corrupted scope stack!");
    S->InsertLocalVarDestructors();
    mScopeStack.pop_back();
    delete S;
  }
}

void RSObjectRefCount::VisitBinAssign(clang::BinaryOperator *AS) {
  getCurrentScope()->setCurrentStmt(AS);
  clang::QualType QT = AS->getType();

  if (CountRSObjectTypes(QT.getTypePtr())) {
    getCurrentScope()->ReplaceRSObjectAssignment(AS);
  }
}

namespace {

class FindRSObjRefVisitor : public clang::RecursiveASTVisitor<FindRSObjRefVisitor> {
public:
  explicit FindRSObjRefVisitor() : mRefRSObj(false) {}
  bool VisitExpr(clang::Expr* Expression) {
    if (CountRSObjectTypes(Expression->getType().getTypePtr()) > 0) {
      mRefRSObj = true;
      // Found a reference to an RS object. Stop the AST traversal.
      return false;
    }
    return true;
  }

  bool foundRSObjRef() const { return mRefRSObj; }

private:
  bool mRefRSObj;
};

}  // anonymous namespace

void RSObjectRefCount::VisitReturnStmt(clang::ReturnStmt *RS) {
  getCurrentScope()->setCurrentStmt(RS);

  // If there is no local rsObject declared so far, no need to transform the
  // return statement.

  bool RSObjDeclared = false;

  for (const Scope* S : mScopeStack) {
    if (S->hasRSObject()) {
      RSObjDeclared = true;
      break;
    }
  }

  if (!RSObjDeclared) {
    return;
  }

  FindRSObjRefVisitor visitor;

  visitor.TraverseStmt(RS);

  // If the return statement does not return anything, or if it does not reference
  // a rsObject, no need to transform it.

  if (!visitor.foundRSObjRef()) {
    return;
  }

  // Transform the return statement so that it does not potentially return or
  // reference a rsObject that has been cleared.

  clang::CompoundStmt* NewRS;
  NewRS = CreateRetStmtWithTempVar(mCtx, GetDeclContext(), RS, getNextID());

  getCurrentScope()->ReplaceStmt(mCtx, NewRS);
}

void RSObjectRefCount::VisitStmt(clang::Stmt *S) {
  getCurrentScope()->setCurrentStmt(S);
  for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
       I != E;
       I++) {
    if (clang::Stmt *Child = *I) {
      Visit(Child);
    }
  }
}

// This function walks the list of global variables and (potentially) creates
// a single global static destructor function that properly decrements
// reference counts on the contained RS object types.
clang::FunctionDecl *RSObjectRefCount::CreateStaticGlobalDtor() {
  Init();

  clang::DeclContext *DC = mCtx.getTranslationUnitDecl();
  clang::SourceLocation loc;

  llvm::StringRef SR(".rs.dtor");
  clang::IdentifierInfo &II = mCtx.Idents.get(SR);
  clang::DeclarationName N(&II);
  clang::FunctionProtoType::ExtProtoInfo EPI;
  clang::QualType T = mCtx.getFunctionType(mCtx.VoidTy,
      llvm::ArrayRef<clang::QualType>(), EPI);
  clang::FunctionDecl *FD = nullptr;

  // Generate rsClearObject() call chains for every global variable
  // (whether static or extern).
  std::vector<clang::Stmt *> StmtList;
  for (clang::DeclContext::decl_iterator I = DC->decls_begin(),
          E = DC->decls_end(); I != E; I++) {
    clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(*I);
    if (VD) {
      if (CountRSObjectTypes(VD->getType().getTypePtr())) {
        if (!FD) {
          // Only create FD if we are going to use it.
          FD = clang::FunctionDecl::Create(mCtx, DC, loc, loc, N, T, nullptr,
                                           clang::SC_None);
        }
        // Mark VD as used.  It might be unused, except for the destructor.
        // 'markUsed' has side-effects that are caused only if VD is not already
        // used.  Hence no need for an extra check here.
        VD->markUsed(mCtx);
        // Make sure to create any helpers within the function's DeclContext,
        // not the one associated with the global translation unit.
        clang::Stmt *RSClearObjectCall = Scope::ClearRSObject(VD, FD);
        StmtList.push_back(RSClearObjectCall);
      }
    }
  }

  // Nothing needs to be destroyed, so don't emit a dtor.
  if (StmtList.empty()) {
    return nullptr;
  }

  clang::CompoundStmt *CS = BuildCompoundStmt(mCtx, StmtList, loc);

  slangAssert(FD);
  FD->setBody(CS);
  // We need some way to tell if this FD is generated by slang
  FD->setImplicit();

  return FD;
}

bool HasRSObjectType(const clang::Type *T) {
  return CountRSObjectTypes(T) != 0;
}

}  // namespace slang
