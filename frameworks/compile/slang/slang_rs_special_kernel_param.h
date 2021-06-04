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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_KERNEL_PARAM_H_
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_KERNEL_PARAM_H_

#include <functional>
#include <string>

#include "clang/AST/Decl.h"

namespace slang {

class RSContext;

bool isSpecialKernelParameter(const llvm::StringRef Name);

std::string listSpecialKernelParameters(unsigned int api);

bool processSpecialKernelParameters(
  RSContext *Context,
  const std::function<std::string ()> &DiagnosticDescription,
  const clang::FunctionDecl *FD,
  size_t *IndexOfFirstSpecialParameter,
  unsigned int *SignatureMetadata);
}

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_SPECIAL_KERNEL_PARAM_H_
