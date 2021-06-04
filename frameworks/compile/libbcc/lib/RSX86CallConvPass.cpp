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

#include "Assert.h"
#include "Log.h"
#include "RSUtils.h"

#include <algorithm>
#include <vector>

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

namespace { // anonymous namespace

static const bool kDebug = false;

/* RSX86_64CallConvPass: This pass scans for calls to Renderscript functions in
 * the CPU reference driver.  For such calls, it  identifies the
 * pass-by-reference large-object pointer arguments introduced by the frontend
 * to conform to the AArch64 calling convention (AAPCS).  These pointer
 * arguments are converted to pass-by-value to match the calling convention of
 * the CPU reference driver.
 */
class RSX86_64CallConvPass: public llvm::ModulePass {
private:
  bool IsRSFunctionOfInterest(llvm::Function &F) {
  // Only Renderscript functions that are not defined locally be considered
    if (!F.empty()) // defined locally
      return false;

    // llvm intrinsic or internal function
    llvm::StringRef FName = F.getName();
    if (FName.startswith("llvm."))
      return false;

    // All other functions need to be checked for large-object parameters.
    // Disallowed (non-Renderscript) functions are detected by a different pass.
    return true;
  }

  // Test if this argument needs to be converted to pass-by-value.
  bool IsDerefNeeded(llvm::Function *F, llvm::Argument &Arg) {
    unsigned ArgNo = Arg.getArgNo();
    llvm::Type *ArgTy = Arg.getType();

    // Do not consider arguments with 'sret' attribute.  Parameters with this
    // attribute are actually pointers to structure return values.
    if (Arg.hasStructRetAttr())
      return false;

    // Dereference needed only if type is a pointer to a struct
    if (!ArgTy->isPointerTy() || !ArgTy->getPointerElementType()->isStructTy())
      return false;

    // Dereference needed only for certain RS struct objects.
    llvm::Type *StructTy = ArgTy->getPointerElementType();
    if (!isRsObjectType(StructTy))
      return false;

    // TODO Find a better way to encode exceptions
    llvm::StringRef FName = F->getName();
    // rsSetObject's first parameter is a pointer
    if (FName.find("rsSetObject") != std::string::npos && ArgNo == 0)
      return false;
    // rsClearObject's first parameter is a pointer
    if (FName.find("rsClearObject") != std::string::npos && ArgNo == 0)
      return false;
    // rsForEachInternal's fifth parameter is a pointer
    if (FName.find("rsForEachInternal") != std::string::npos && ArgNo == 4)
      return false;

    return true;
  }

  // Compute which arguments to this function need be converted to pass-by-value
  bool FillArgsToDeref(llvm::Function *F, std::vector<unsigned> &ArgNums) {
    bccAssert(ArgNums.size() == 0);

    for (auto &Arg: F->getArgumentList()) {
      if (IsDerefNeeded(F, Arg)) {
        ArgNums.push_back(Arg.getArgNo());

        if (kDebug) {
          ALOGV("Lowering argument %u for function %s\n", Arg.getArgNo(),
                F->getName().str().c_str());
        }
      }
    }
    return ArgNums.size() > 0;
  }

  llvm::Function *RedefineFn(llvm::Function *OrigFn,
                             std::vector<unsigned> &ArgsToDeref) {

    llvm::FunctionType *FTy = OrigFn->getFunctionType();
    std::vector<llvm::Type *> Params(FTy->param_begin(), FTy->param_end());

    llvm::FunctionType *NewTy = llvm::FunctionType::get(FTy->getReturnType(),
                                                        Params,
                                                        FTy->isVarArg());
    llvm::Function *NewFn = llvm::Function::Create(NewTy,
                                                   OrigFn->getLinkage(),
                                                   OrigFn->getName(),
                                                   OrigFn->getParent());

    // Add the ByVal attribute to the attribute list corresponding to this
    // argument.  The list at index (i+1) corresponds to the i-th argument.  The
    // list at index 0 corresponds to the return value's attribute.
    for (auto i: ArgsToDeref) {
      NewFn->addAttribute(i+1, llvm::Attribute::ByVal);
    }

    NewFn->copyAttributesFrom(OrigFn);
    NewFn->takeName(OrigFn);

    for (auto AI=OrigFn->arg_begin(), AE=OrigFn->arg_end(),
              NAI=NewFn->arg_begin();
         AI != AE; ++ AI, ++NAI) {
      NAI->takeName(&*AI);
    }

    return NewFn;
  }

  void ReplaceCallInsn(llvm::CallSite &CS,
                       llvm::Function *NewFn,
                       std::vector<unsigned> &ArgsToDeref) {

    llvm::CallInst *CI = llvm::cast<llvm::CallInst>(CS.getInstruction());
    std::vector<llvm::Value *> Args(CS.arg_begin(), CS.arg_end());
    auto NewCI = llvm::CallInst::Create(NewFn, Args, "", CI);

    // Add the ByVal attribute to the attribute list corresponding to this
    // argument.  The list at index (i+1) corresponds to the i-th argument.  The
    // list at index 0 corresponds to the return value's attribute.
    for (auto i: ArgsToDeref) {
      NewCI->addAttribute(i+1, llvm::Attribute::ByVal);
    }
    if (CI->isTailCall())
      NewCI->setTailCall();

    if (!CI->getType()->isVoidTy())
      CI->replaceAllUsesWith(NewCI);

    CI->eraseFromParent();
  }

public:
  static char ID;

  RSX86_64CallConvPass()
    : ModulePass (ID) {
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    // This pass does not use any other analysis passes, but it does
    // modify the existing functions in the module (thus altering the CFG).
  }

  bool runOnModule(llvm::Module &M) override {
    // Avoid adding Functions and altering FunctionList while iterating over it
    // by collecting functions and processing them later.
    std::vector<llvm::Function *> FunctionsToHandle;

    auto &FunctionList = M.getFunctionList();
    for (auto &OrigFn: FunctionList) {
      if (!IsRSFunctionOfInterest(OrigFn))
        continue;
      FunctionsToHandle.push_back(&OrigFn);
    }

    for (auto OrigFn: FunctionsToHandle) {
      std::vector<unsigned> ArgsToDeref;
      if (!FillArgsToDeref(OrigFn, ArgsToDeref))
        continue;

      // Replace all calls to OrigFn and erase it from parent.
      llvm::Function *NewFn = RedefineFn(OrigFn, ArgsToDeref);
      while (!OrigFn->use_empty()) {
        llvm::CallSite CS(OrigFn->user_back());
        ReplaceCallInsn(CS, NewFn, ArgsToDeref);
      }
      OrigFn->eraseFromParent();
    }

    return FunctionsToHandle.size() > 0;
  }

};

}

char RSX86_64CallConvPass::ID = 0;

static llvm::RegisterPass<RSX86_64CallConvPass> X("X86-64-calling-conv",
  "remove AArch64 assumptions from calls in X86-64");

namespace bcc {

llvm::ModulePass *
createRSX86_64CallConvPass() {
  return new RSX86_64CallConvPass();
}

}
