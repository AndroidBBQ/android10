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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_AST_REPLACE_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_AST_REPLACE_H_

#include "clang/AST/StmtVisitor.h"

#include "slang_assert.h"
#include "clang/AST/ASTContext.h"

namespace clang {
  class Diagnostic;
  class Expr;
  class Stmt;
}

namespace slang {

class RSASTReplace : public clang::StmtVisitor<RSASTReplace> {
 private:
  const clang::ASTContext &C;
  clang::Stmt *mOuterStmt;
  clang::Stmt *mOldStmt;
  clang::Stmt *mNewStmt;
  clang::Expr *mOldExpr;
  clang::Expr *mNewExpr;

  inline bool matchesExpr(const clang::Expr *E) const {
    bool retVal = mOldExpr && (mOldExpr == E);
    if (retVal) {
      slangAssert(mNewExpr &&
          "Cannot replace an expression if we don't have a new expression");
    }
    return retVal;
  }

  inline bool matchesStmt(const clang::Stmt *S) const {
    slangAssert(mOldStmt);
    return mOldStmt == S;
  }

  void ReplaceInCompoundStmt(clang::CompoundStmt *CS);

 public:
  explicit RSASTReplace(const clang::ASTContext &Con)
      : C(Con),
        mOuterStmt(nullptr),
        mOldStmt(nullptr),
        mNewStmt(nullptr),
        mOldExpr(nullptr),
        mNewExpr(nullptr) {
  }

  void VisitStmt(clang::Stmt *S);
  void VisitCompoundStmt(clang::CompoundStmt *CS);
  void VisitCaseStmt(clang::CaseStmt *CS);
  void VisitDeclStmt(clang::DeclStmt* DS);
  void VisitDefaultStmt(clang::DefaultStmt *DS);
  void VisitDoStmt(clang::DoStmt *DS);
  void VisitForStmt(clang::ForStmt *FS);
  void VisitIfStmt(clang::IfStmt *IS);
  void VisitSwitchCase(clang::SwitchCase *SC);
  void VisitSwitchStmt(clang::SwitchStmt *SS);
  void VisitWhileStmt(clang::WhileStmt *WS);

  // Replace all instances of OldStmt in OuterStmt with NewStmt.
  void ReplaceStmt(
      clang::Stmt *OuterStmt,
      clang::Stmt *OldStmt,
      clang::Stmt *NewStmt);
};

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_AST_REPLACE_H_  NOLINT
