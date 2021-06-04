/*
 * Copyright 2011, The Android Open Source Project
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

#include "slang_rs_ast_replace.h"

#include "slang_assert.h"

#include "llvm/Support/Casting.h"

namespace slang {

void RSASTReplace::ReplaceStmt(
    clang::Stmt *OuterStmt,
    clang::Stmt *OldStmt,
    clang::Stmt *NewStmt) {
  mOldStmt = OldStmt;
  mNewStmt = NewStmt;
  mOuterStmt = OuterStmt;

  // This simplifies use in various Stmt visitor passes where the only
  // valid type is an Expr.
  mOldExpr = llvm::dyn_cast<clang::Expr>(OldStmt);
  if (mOldExpr) {
    mNewExpr = llvm::dyn_cast<clang::Expr>(NewStmt);
  }
  Visit(mOuterStmt);
}

void RSASTReplace::ReplaceInCompoundStmt(clang::CompoundStmt *CS) {
  clang::Stmt **UpdatedStmtList = new clang::Stmt*[CS->size()];

  unsigned UpdatedStmtCount = 0;
  clang::CompoundStmt::body_iterator bI = CS->body_begin();
  clang::CompoundStmt::body_iterator bE = CS->body_end();

  for ( ; bI != bE; bI++) {
    if (matchesStmt(*bI)) {
      UpdatedStmtList[UpdatedStmtCount++] = mNewStmt;
    } else {
      UpdatedStmtList[UpdatedStmtCount++] = *bI;
    }
  }

  CS->setStmts(C, llvm::makeArrayRef(UpdatedStmtList, UpdatedStmtCount));

  delete [] UpdatedStmtList;
}

void RSASTReplace::VisitStmt(clang::Stmt *S) {
  // This function does the actual iteration through all sub-Stmt's within
  // a given Stmt. Note that this function is skipped by all of the other
  // Visit* functions if we have already found a higher-level match.
  for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
       I != E;
       I++) {
    if (clang::Stmt *Child = *I) {
      if (!matchesStmt(Child)) {
        Visit(Child);
      }
    }
  }
}

void RSASTReplace::VisitCompoundStmt(clang::CompoundStmt *CS) {
  VisitStmt(CS);
  ReplaceInCompoundStmt(CS);
}

void RSASTReplace::VisitCaseStmt(clang::CaseStmt *CS) {
  if (matchesStmt(CS->getSubStmt())) {
    CS->setSubStmt(mNewStmt);
  } else {
    VisitStmt(CS);
  }
}

void RSASTReplace::VisitDeclStmt(clang::DeclStmt* DS) {
  VisitStmt(DS);
  for (clang::Decl* D : DS->decls()) {
    clang::VarDecl* VD;
    if ((VD = llvm::dyn_cast<clang::VarDecl>(D))) {
      if (matchesExpr(VD->getInit())) {
        VD->setInit(mNewExpr);
      }
    }
  }
}

void RSASTReplace::VisitDefaultStmt(clang::DefaultStmt *DS) {
  if (matchesStmt(DS->getSubStmt())) {
    DS->setSubStmt(mNewStmt);
  } else {
    VisitStmt(DS);
  }
}

void RSASTReplace::VisitDoStmt(clang::DoStmt *DS) {
  if (matchesExpr(DS->getCond())) {
    DS->setCond(mNewExpr);
  } else if (matchesStmt(DS->getBody())) {
    DS->setBody(mNewStmt);
  } else {
    VisitStmt(DS);
  }
}

void RSASTReplace::VisitForStmt(clang::ForStmt *FS) {
  if (matchesStmt(FS->getInit())) {
    FS->setInit(mNewStmt);
  } else if (matchesExpr(FS->getCond())) {
    FS->setCond(mNewExpr);
  } else if (matchesExpr(FS->getInc())) {
    FS->setInc(mNewExpr);
  } else if (matchesStmt(FS->getBody())) {
    FS->setBody(mNewStmt);
  } else {
    VisitStmt(FS);
  }
}

void RSASTReplace::VisitIfStmt(clang::IfStmt *IS) {
  if (matchesExpr(IS->getCond())) {
    IS->setCond(mNewExpr);
  } else if (matchesStmt(IS->getThen())) {
    IS->setThen(mNewStmt);
  } else if (matchesStmt(IS->getElse())) {
    IS->setElse(mNewStmt);
  } else {
    VisitStmt(IS);
  }
}

void RSASTReplace::VisitSwitchCase(clang::SwitchCase *SC) {
  slangAssert(false && "Both case and default have specialized handlers");
  VisitStmt(SC);
}

void RSASTReplace::VisitSwitchStmt(clang::SwitchStmt *SS) {
  if (matchesExpr(SS->getCond())) {
    SS->setCond(mNewExpr);
  } else {
    VisitStmt(SS);
  }
}

void RSASTReplace::VisitWhileStmt(clang::WhileStmt *WS) {
  if (matchesExpr(WS->getCond())) {
    WS->setCond(mNewExpr);
  } else if (matchesStmt(WS->getBody())) {
    WS->setBody(mNewStmt);
  } else {
    VisitStmt(WS);
  }
}

}  // namespace slang
