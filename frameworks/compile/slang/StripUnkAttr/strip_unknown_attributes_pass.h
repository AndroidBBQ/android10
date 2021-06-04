/*
 * Copyright 2013, The Android Open Source Project
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

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace slang {

// Remove any readnone/readonly attributes from function parameters,
// and remove any argmemonly attributes from functions.
//
// Jellybean's LLVM version didn't support readnone/readonly as anything
// other than function attributes, so it will fail verification otherwise.
// Since we never ran the verifier in Jellybean, it ends up with potential
// crashes deeper in CodeGen.
//
// Similarly, older LLVM versions don't support argmemonly as a function
// attribute, and so it can trigger an llvm_unreachable assertion when
// we attempt to write out bitcode.
class StripUnknownAttributes : public llvm::ModulePass {
public:
  static char ID;

  StripUnknownAttributes();

  bool runOnFunction(llvm::Function &F);

  // We have to use a ModulePass, since a FunctionPass only gets run on
  // defined Functions (and not declared Functions).
  virtual bool runOnModule(llvm::Module &M);
};

llvm::ModulePass * createStripUnknownAttributesPass();

}  // namespace slang
