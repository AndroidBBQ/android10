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

#include "strip_unknown_attributes_pass.h"
#include "strip_unknown_attributes.h"

namespace slang {

StripUnknownAttributes::StripUnknownAttributes() : ModulePass(ID) {
}


bool StripUnknownAttributes::runOnFunction(llvm::Function &F) {
  return stripUnknownAttributes(F);
}


bool StripUnknownAttributes::runOnModule(llvm::Module &M) {
  bool Changed = false;
  for (llvm::Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    Changed |= runOnFunction(*I);
  }
  return Changed;
}


llvm::ModulePass * createStripUnknownAttributesPass() {
  return new StripUnknownAttributes();
}


char StripUnknownAttributes::ID = 0;
static llvm::RegisterPass<StripUnknownAttributes> RPSUA(
    "StripUnknownAttributes", "Strip Unknown Attributes Pass");

}  // namespace slang
