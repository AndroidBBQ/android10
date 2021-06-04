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

#include "bcc/Config.h"
#include "Assert.h"
#include "Log.h"
#include "RSTransforms.h"

#include <cstdlib>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>

namespace { // anonymous namespace

/* This pass translates GEPs that index into structs or arrays of structs to
 * GEPs with an int8* operand and a byte offset.  This translation is done to
 * enforce on x86 the ARM alignment rule that 64-bit scalars be 8-byte aligned
 * for structs with such scalars.
 */
class RSX86TranslateGEPPass : public llvm::FunctionPass {
private:
  static char ID;
  llvm::LLVMContext *Context;
  const llvm::DataLayout DL;

  // Walk a GEP instruction and return true if any type indexed is a struct.
  bool GEPIndexesStructType(const llvm::GetElementPtrInst *GEP) {
    for (llvm::gep_type_iterator GTI = gep_type_begin(GEP),
                                 GTE = gep_type_end(GEP);
         GTI != GTE; ++GTI) {
      if (llvm::dyn_cast<llvm::StructType>(*GTI)) {
        return true;
      }
    }
    return false;
  }

  // Helper method to add two llvm::Value parameters
  llvm::Value *incrementOffset(llvm::Value *accum, llvm::Value *incr,
                               llvm::Instruction *InsertBefore) {
    if (accum == nullptr)
      return incr;
    return llvm::BinaryOperator::CreateAdd(accum, incr, "", InsertBefore);
  }

  // Compute the byte offset for a GEP from the GEP's base pointer operand.
  // Based on visitGetElementPtrInst in llvm/lib/Transforms/Scalar/SROA.cpp.
  // The difference is that this function handles non-constant array indices and
  // constructs a sequence of instructions to calculate the offset.  These
  // instructions might not be the most efficient way to calculate this offset,
  // but we rely on subsequent optimizations to do necessary fold/combine.
  llvm::Value *computeGEPOffset(llvm::GetElementPtrInst *GEP) {
    llvm::Value *Offset = nullptr;

    for (llvm::gep_type_iterator GTI = gep_type_begin(GEP),
                                 GTE = gep_type_end(GEP);
         GTI != GTE; ++GTI) {
      if (llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(*GTI)) {
        llvm::ConstantInt *OpC = llvm::dyn_cast<llvm::ConstantInt>(GTI.getOperand());
        if (!OpC) {
          ALOGE("Operand for struct type is not constant!");
          bccAssert(false);
        }

        // Offset = Offset + EltOffset for index into a struct
        const llvm::StructLayout *SL = DL.getStructLayout(STy);
        unsigned EltOffset = SL->getElementOffset(OpC->getZExtValue());
        llvm::Value *Incr = llvm::ConstantInt::get(
                                llvm::Type::getInt32Ty(*Context), EltOffset);
        Offset = incrementOffset(Offset, Incr, GEP);
      } else {
        // Offset = Offset + Index * EltSize for index into an array or a vector
        llvm::Value *EltSize = llvm::ConstantInt::get(
                                 llvm::Type::getInt32Ty(*Context),
                                 DL.getTypeAllocSize(GTI.getIndexedType()));
        llvm::Value *Incr = llvm::BinaryOperator::CreateMul(
                                GTI.getOperand() /* Index */,
                                EltSize, "", GEP);
        Offset = incrementOffset(Offset, Incr, GEP);
      }
    }

    return Offset;
  }

  void translateGEP(llvm::GetElementPtrInst *GEP) {
    // cast GEP pointer operand to int8*
    llvm::CastInst *Int8Ptr = llvm::CastInst::CreatePointerCast(
                                  GEP->getPointerOperand(),
                                  llvm::Type::getInt8PtrTy(*Context),
                                  "to.int8ptr",
                                  GEP);
    llvm::Value *Indices[1] = {computeGEPOffset(GEP)};

    // index into the int8* based on the byte offset
    llvm::GetElementPtrInst *Int8PtrGEP = llvm::GetElementPtrInst::Create(
        llvm::Type::getInt8Ty(*Context), Int8Ptr, llvm::makeArrayRef(Indices),
        "int8ptr.indexed", GEP);
    Int8PtrGEP->setIsInBounds(GEP->isInBounds());

    // cast the indexed int8* back to the type of the original GEP
    llvm::CastInst *OutCast = llvm::CastInst::CreatePointerCast(
        Int8PtrGEP, GEP->getType(), "to.orig.geptype", GEP);

    GEP->replaceAllUsesWith(OutCast);
  }

public:
  RSX86TranslateGEPPass()
    : FunctionPass (ID), DL(X86_CUSTOM_DL_STRING) {
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    // This pass is run in isolation in a separate pass manager.  So setting
    // AnalysisUsage is unnecessary.  Set just for completeness.
    AU.setPreservesCFG();
  }

  virtual bool runOnFunction(llvm::Function &F) override {
    bool changed = false;
    Context = &F.getParent()->getContext();

    // To avoid updating/deleting instructions while walking a BasicBlock's instructions,
    // collect the GEPs that need to be translated and process them
    // subsequently.
    std::vector<llvm::GetElementPtrInst *> GEPsToHandle;

    for (auto &BB: F) {
      for (auto &I: BB) {
        if (auto *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(&I)) {
          if (GEPIndexesStructType(GEP)) {
            GEPsToHandle.push_back(GEP);
          }
        }
      }
    }

    for (auto *GEP: GEPsToHandle) {
      // Translate GEPs and erase them
      translateGEP(GEP);
      changed = true;
      GEP->eraseFromParent();
    }

    return changed;
  }

  virtual const char *getPassName() const override {
    return "Translate GEPs on structs, intended for x86 target";
  }
};

}

char RSX86TranslateGEPPass::ID = 0;

namespace bcc {

llvm::FunctionPass *
createRSX86TranslateGEPPass() {
  return new RSX86TranslateGEPPass();
}

}
