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

#ifndef BCC_RS_TRANSFORMS_H
#define BCC_RS_TRANSFORMS_H

namespace llvm {
  class ModulePass;
  class FunctionPass;
}

namespace bcc {

extern const char BCC_INDEX_VAR_NAME[];

llvm::ModulePass *
createRSKernelExpandPass(bool pEnableStepOpt);

llvm::FunctionPass *
createRSInvariantPass();

llvm::FunctionPass *
createRSInvokeHelperPass();

llvm::ModulePass * createRSEmbedInfoPass();

llvm::ModulePass * createRSGlobalInfoPass(bool pSkipConstants);

llvm::ModulePass * createRSScreenFunctionsPass();

llvm::ModulePass * createRSIsThreadablePass();

llvm::ModulePass * createRSX86_64CallConvPass();

llvm::ModulePass * createRSAddDebugInfoPass();

llvm::FunctionPass *createRSX86TranslateGEPPass();

} // end namespace bcc

#endif // BCC_RS_TRANSFORMS_H
