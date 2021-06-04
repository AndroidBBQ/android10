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

#include "RSTransforms.h"
#include "RSUtils.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>

namespace {

/*
 * RSInvariantPass - This pass looks for Loads that access
 * RsExpandKernelDriverInfo instances (which should never be written by
 * a script, only by the driver) and marks them "invariant.load".
 *
 * There should be only two sources of Loads from such instances:
 * - An instance can appear as an argument of type
 *   "RsExpandKernelDriverInfoPfx*" passed to a .expand function by
 *   the driver.
 * - An instance can appear as an argument of type
 *   "rs_kernel_context_t*" passed to an API query function by the
 *   user.
 * Only the compiler-generated .expand functions and the API query
 * functions can see the fields of RsExpandKernelDriverInfo --
 * rs_kernel_context_t is opaque to user code, so there cannot be any
 * Loads from it in user code.
 *
 * This pass should be run
 * - after foreachexp, so that it can see the Loads generated within
 *   .expand functions
 * - before inlining, so that it can recognize API query function
 *   arguments.
 *
 * WARNINGS:
 * - If user code or APIs can modify RsExpandKernelDriverInfo
 *   instances, this pass MAY ALLOW ILLEGAL OPTIMIZATION.
 * - If this pass runs at a different time, it may be ineffective
 *   (fail to mark some or all eligible Loads, and thereby cost
 *   performance).
 * - If the names of the data types change, this pass may be
 *   ineffective.
 * - If the patterns by which fields are loaded from
 *   RsExpandKernelDriverInfo instances change, this pass may be
 *   ineffective.
 */
class RSInvariantPass : public llvm::FunctionPass {
public:
  static char ID;

  RSInvariantPass() : FunctionPass(ID), EmptyMDNode(nullptr) { }

  virtual bool doInitialization(llvm::Module &M) {
    EmptyMDNode = llvm::MDNode::get(M.getContext(), llvm::None);
    return true;
  }

  virtual bool runOnFunction(llvm::Function &F) {
    bool Changed = false;

    for (llvm::Value &Arg : F.args()) {
      const llvm::Type *ArgType = Arg.getType();
      if (ArgType->isPointerTy()) {
        const llvm::Type *ArgPtrDomainType =  ArgType->getPointerElementType();
        if (auto ArgPtrDomainStructType = llvm::dyn_cast<llvm::StructType>(ArgPtrDomainType)) {
          if (!ArgPtrDomainStructType->isLiteral()) {
            const llvm::StringRef StructName = getUnsuffixedStructName(ArgPtrDomainStructType);
            if (StructName.equals("struct.rs_kernel_context_t") || StructName.equals("RsExpandKernelDriverInfoPfx")) {
              Changed |= markInvariantUserLoads(&Arg);
            }
          }
        }
      }
    }

    return Changed;
  }

  virtual const char *getPassName() const {
    return "Renderscript Invariant Load Annotation";
  }

private:

  /*
   * Follow def->use chains rooted at Value through calculations
   * "based on" Value (see the "based on" definition at
   * http://llvm.org/docs/LangRef.html#pointer-aliasing-rules).  If a
   * chain reaches the pointer operand of a Load, mark that Load as
   * "invariant.load" -- i.e., it accesses memory which does not
   * change.
   */
  bool markInvariantUserLoads(llvm::Value *Value) {
    bool Changed = false;
    for (llvm::Use &Use : Value->uses()) {
      llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(Use.getUser());

      /*
       * We only examine a small set of opcodes here, because these
       * are the opcodes that currently appear in the patterns of
       * interest (foreachexp-generated code, and
       * rsGet*(rs_kernel_context_t*) APIs).  Other opcodes could be
       * added if necessary.
       */
      if (auto BitCast = llvm::dyn_cast<llvm::BitCastInst>(Inst)) {
        Changed |= markInvariantUserLoads(BitCast);
      } else if (auto GetElementPtr = llvm::dyn_cast<llvm::GetElementPtrInst>(Inst)) {
        if (Use.get() == GetElementPtr->getPointerOperand())
          Changed |= markInvariantUserLoads(GetElementPtr);
      } else if (auto Load = llvm::dyn_cast<llvm::LoadInst>(Inst)) {
        if (Use.get() == Load->getPointerOperand()) {
          Load->setMetadata("invariant.load", EmptyMDNode);
          Changed = true;
        }
      }
    }
    return Changed;
  }

  // Pointer to empty metadata node used for "invariant.load" marking.
  llvm::MDNode *EmptyMDNode;
}; // end RSInvariantPass

char RSInvariantPass::ID = 0;
llvm::RegisterPass<RSInvariantPass> X("rsinvariant", "RS Invariant Load Pass");

} // end anonymous namespace

namespace bcc {

llvm::FunctionPass *
createRSInvariantPass() {
  return new RSInvariantPass();
}

} // end namespace bcc
