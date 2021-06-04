/*
 * Copyright 2016, The Android Open Source Project
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

#include "InlinePreparationPass.h"

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Context.h"

#define DEBUG_TYPE "rs2spirv-inline"

using namespace llvm;

namespace rs2spirv {

namespace {

class InlinePreparationPass : public ModulePass {
public:
  static char ID;
  explicit InlinePreparationPass() : ModulePass(ID) {}

  const char *getPassName() const override { return "InlinePreparationPass"; }

  bool runOnModule(Module &M) override {
    DEBUG(dbgs() << "InlinePreparationPass\n");

    rs2spirv::Context &Ctxt = rs2spirv::Context::getInstance();

    for (auto &F : M.functions()) {
      if (F.isDeclaration()) {
        continue;
      }

      if (Ctxt.isForEachKernel(F.getName())) {
        continue; // Skip kernels.
      }

      F.addFnAttr(Attribute::AlwaysInline);
      F.setLinkage(GlobalValue::InternalLinkage);

      DEBUG(dbgs() << "Marked as alwaysinline:\t" << F.getName() << '\n');
    }

    // Returns true, because this pass modifies the Module.
    return true;
  }
};

} // namespace

char InlinePreparationPass::ID = 0;

ModulePass *createInlinePreparationPass() {
  return new InlinePreparationPass();
}

} // namespace rs2spirv
