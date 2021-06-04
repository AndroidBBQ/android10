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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_CHECK_AST_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_CHECK_AST_H_

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/StmtVisitor.h"

namespace slang {

// This class is designed to walk a Renderscript/Filterscript AST looking for
// violations. Examples of violations for FS are pointer declarations and
// casts (i.e. no pointers allowed in FS whatsoever).
class RSCheckAST : public clang::StmtVisitor<RSCheckAST> {
 private:
  slang::RSContext *Context;
  clang::ASTContext &C;
  clang::SourceManager &mSM;
  bool mValid;
  unsigned int mTargetAPI;
  bool mIsFilterscript;
  bool mInKernel;

  /// @brief Emit warnings for inapproriate uses of rsSetElementAt
  ///
  /// We warn in case generic rsSetElementAt() is used even though the user
  /// could have used a typed rsSetElementAt_<type>() call. Typed calls
  /// allow more aggressive optimization (e.g. due to better alias analysis
  /// results). Hence, we want to steer the users to use them.
  void WarnOnSetElementAt(clang::CallExpr*);

 public:
  explicit RSCheckAST(RSContext *Con, unsigned int TargetAPI,
                      bool IsFilterscript)
      : Context(Con),
        C(Con->getASTContext()),
        mSM(C.getSourceManager()),
        mValid(true),
        mTargetAPI(TargetAPI),
        mIsFilterscript(IsFilterscript),
        mInKernel(false) {
  }

  void VisitStmt(clang::Stmt *S);

  void VisitCallExpr(clang::CallExpr *CE);

  void VisitCastExpr(clang::CastExpr *CE);

  void VisitExpr(clang::Expr *E);

  void VisitDeclStmt(clang::DeclStmt *DS);

  void ValidateFunctionDecl(clang::FunctionDecl *FD);

  void ValidateVarDecl(clang::VarDecl *VD);

  bool Validate();
};

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_CHECK_AST_H_  NOLINT
