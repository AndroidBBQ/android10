/*
 * Copyright 2015, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_SLANG_RS_FOREACH_LOWERING_H
#define _FRAMEWORKS_COMPILE_SLANG_RS_FOREACH_LOWERING_H

#include "clang/AST/StmtVisitor.h"

namespace clang {
  class ASTContext;
  class CallExpr;
  class Expr;
  class FunctionDecl;
}

namespace slang {

class RSContext;

class RSForEachLowering : public clang::StmtVisitor<RSForEachLowering> {
 public:
  explicit RSForEachLowering(RSContext* ctxt);

  // Given a FunctionDecl FD and the target API level, either translates all
  // rsForEach() and rsForEachWithOptions() calls inside FD into calls to the
  // low-level rsForEachInternal() API, if FD is not a kernel function itself;
  // or, in the case where FD is a kernel function, reports a compiler error on
  // any calls to either kernel launching API function.
  void handleForEachCalls(clang::FunctionDecl* FD, unsigned int targetAPI);

  void VisitCallExpr(clang::CallExpr *CE);
  void VisitStmt(clang::Stmt *S);

 private:
  RSContext* mCtxt;
  clang::ASTContext& mASTCtxt;
  // A flag, if true, indicating that the visitor is walking inside a kernel
  // function, in which case any call to rsForEach() or rsForEachWithOptions()
  // is a compiler error.
  bool mInsideKernel;

  const clang::FunctionDecl* matchFunctionDesignator(clang::Expr* expr);
  const clang::FunctionDecl* matchKernelLaunchCall(clang::CallExpr* CE,
                                                   int* slot,
                                                   bool* hasOptions);
  clang::FunctionDecl* CreateForEachInternalFunctionDecl();
  clang::Expr* CreateCalleeExprForInternalForEach();
};  // RSForEachLowering

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_RS_FOREACH_LOWERING_H
