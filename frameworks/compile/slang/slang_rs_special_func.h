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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_FUNC_H_
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_FUNC_H_

#include "llvm/ADT/StringRef.h"

#include "clang/AST/Decl.h"

#include "slang_rs_context.h"

namespace slang {

namespace RSSpecialFunc {

inline bool isInitRSFunc(const clang::FunctionDecl *FD) {
  if (!FD) {
    return false;
  }
  const llvm::StringRef Name = FD->getName();
  static llvm::StringRef FuncInit("init");
  return Name.equals(FuncInit);
}

inline bool isDtorRSFunc(const clang::FunctionDecl *FD) {
  if (!FD) {
    return false;
  }
  const llvm::StringRef Name = FD->getName();
  static llvm::StringRef FuncDtor(".rs.dtor");
  return Name.equals(FuncDtor);
}

bool isGraphicsRootRSFunc(unsigned int targetAPI,
                          const clang::FunctionDecl *FD);

inline bool isSpecialRSFunc(unsigned int targetAPI,
                                   const clang::FunctionDecl *FD) {
  return isGraphicsRootRSFunc(targetAPI, FD) || isInitRSFunc(FD) ||
         isDtorRSFunc(FD);
}

bool validateSpecialFuncDecl(unsigned int targetAPI,
                             slang::RSContext *Context,
                             const clang::FunctionDecl *FD);

} // namespace RSSpecialFunc

} // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_FUNC_H
