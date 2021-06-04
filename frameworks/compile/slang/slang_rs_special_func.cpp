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

#include "slang_rs_special_func.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"

#include "slang_assert.h"
#include "slang_version.h"

namespace slang {

bool RSSpecialFunc::isGraphicsRootRSFunc(unsigned int targetAPI,
                                         const clang::FunctionDecl *FD) {
  if (FD->hasAttr<clang::RenderScriptKernelAttr>()) {
    return false;
  }

  if (!FD->getName().equals("root")) {
    return false;
  }

  if (FD->getNumParams() == 0) {
    // Graphics root function
    return true;
  }

  // Check for legacy graphics root function (with single parameter).
  if ((targetAPI < SLANG_ICS_TARGET_API) && (FD->getNumParams() == 1)) {
    const clang::QualType &IntType = FD->getASTContext().IntTy;
    if (FD->getReturnType().getCanonicalType() == IntType) {
      return true;
    }
  }

  return false;
}

bool
RSSpecialFunc::validateSpecialFuncDecl(unsigned int targetAPI,
                                       slang::RSContext *Context,
                                       clang::FunctionDecl const *FD) {
  slangAssert(Context && FD);
  bool valid = true;
  const clang::ASTContext &C = FD->getASTContext();
  const clang::QualType &IntType = FD->getASTContext().IntTy;

  if (isGraphicsRootRSFunc(targetAPI, FD)) {
    if ((targetAPI < SLANG_ICS_TARGET_API) && (FD->getNumParams() == 1)) {
      // Legacy graphics root function
      const clang::ParmVarDecl *PVD = FD->getParamDecl(0);
      clang::QualType QT = PVD->getType().getCanonicalType();
      if (QT != IntType) {
        Context->ReportError(PVD->getLocation(),
                             "invalid parameter type for legacy "
                             "graphics root() function: %0")
            << PVD->getType();
        valid = false;
      }
    }

    // Graphics root function, so verify that it returns an int
    if (FD->getReturnType().getCanonicalType() != IntType) {
      Context->ReportError(FD->getLocation(),
                           "root() is required to return "
                           "an int for graphics usage");
      valid = false;
    }
  } else if (isInitRSFunc(FD) || isDtorRSFunc(FD)) {
    if (FD->getNumParams() != 0) {
      Context->ReportError(FD->getLocation(),
                           "%0(void) is required to have no "
                           "parameters")
          << FD->getName();
      valid = false;
    }

    if (FD->getReturnType().getCanonicalType() != C.VoidTy) {
      Context->ReportError(FD->getLocation(),
                           "%0(void) is required to have a void "
                           "return type")
          << FD->getName();
      valid = false;
    }
  } else {
    slangAssert(false && "must be called on root, init or .rs.dtor function!");
  }

  return valid;
}

}  // namespace slang
