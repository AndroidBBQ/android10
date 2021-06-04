/*
 * Copyright 2016-2017, The Android Open Source Project
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

#include "GlobalMergePass.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Context.h"
#include "RSAllocationUtils.h"

#include <functional>

#define DEBUG_TYPE "rs2spirv-global-merge"

using namespace llvm;

namespace rs2spirv {

namespace {

class GlobalMergePass : public ModulePass {
public:
  static char ID;
  GlobalMergePass(bool CPU = false) : ModulePass(ID), mForCPU(CPU) {}
  const char *getPassName() const override { return "GlobalMergePass"; }

  bool runOnModule(Module &M) override {
    DEBUG(dbgs() << "RS2SPIRVGlobalMergePass\n");

    SmallVector<GlobalVariable *, 8> Globals;
    if (!collectGlobals(M, Globals)) {
      return false; // Module not modified.
    }

    SmallVector<Type *, 8> Tys;
    Tys.reserve(Globals.size());

    Context &RS2SPIRVCtxt = Context::getInstance();

    uint32_t index = 0;
    for (GlobalVariable *GV : Globals) {
      Tys.push_back(GV->getValueType());
      const char *name = GV->getName().data();
      RS2SPIRVCtxt.addExportVarIndex(name, index);
      index++;
    }

    LLVMContext &LLVMCtxt = M.getContext();

    StructType *MergedTy = StructType::create(LLVMCtxt, "struct.__GPUBuffer");
    MergedTy->setBody(Tys, false);

    // Size calculation has to consider data layout
    const DataLayout &DL = M.getDataLayout();
    const uint64_t BufferSize = DL.getTypeAllocSize(MergedTy);
    RS2SPIRVCtxt.setGlobalSize(BufferSize);

    Type *BufferVarTy = mForCPU ? static_cast<Type *>(PointerType::getUnqual(
                                      Type::getInt8Ty(M.getContext())))
                                : static_cast<Type *>(MergedTy);
    GlobalVariable *MergedGV =
        new GlobalVariable(M, BufferVarTy, false, GlobalValue::ExternalLinkage,
                           nullptr, "__GPUBlock");

    // For CPU, create a constant struct for initial values, which has each of
    // its fields initialized to the original value of the corresponding global
    // variable.
    // During the script initialization, the driver should copy these initial
    // values to the global buffer.
    if (mForCPU) {
      CreateInitFunction(LLVMCtxt, M, MergedGV, MergedTy, BufferSize, Globals);
    }

    const bool forCPU = mForCPU;
    IntegerType *const Int32Ty = Type::getInt32Ty(LLVMCtxt);
    ConstantInt *const Zero = ConstantInt::get(Int32Ty, 0);
    Value *Idx[] = {Zero, nullptr};

    auto InstMaker = [forCPU, MergedGV, MergedTy,
                      &Idx](Instruction *InsertBefore) {
      Value *Base = MergedGV;
      if (forCPU) {
        LoadInst *Load = new LoadInst(MergedGV, "", InsertBefore);
        DEBUG(Load->dump());
        Base = new BitCastInst(Load, PointerType::getUnqual(MergedTy), "",
                               InsertBefore);
        DEBUG(Base->dump());
      }
      GetElementPtrInst *GEP = GetElementPtrInst::CreateInBounds(
          MergedTy, Base, Idx, "", InsertBefore);
      DEBUG(GEP->dump());
      return GEP;
    };

    for (size_t i = 0, e = Globals.size(); i != e; ++i) {
      GlobalVariable *G = Globals[i];
      Idx[1] = ConstantInt::get(Int32Ty, i);
      ReplaceAllUsesWithNewInstructions(G, std::cref(InstMaker));
      G->eraseFromParent();
    }

    // Return true, as the pass modifies module.
    return true;
  }

private:
  // In the User of Value Old, replaces all references of Old with Value New
  static inline void ReplaceUse(User *U, Value *Old, Value *New) {
    for (unsigned i = 0, n = U->getNumOperands(); i < n; ++i) {
      if (U->getOperand(i) == Old) {
        U->getOperandUse(i) = New;
      }
    }
  }

  // Replaces each use of V with new instructions created by
  // funcCreateAndInsert and inserted right before that use. In the cases where
  // the use is not an instruction, but a constant expression, recursively
  // replaces that constant expression with a newly constructed equivalent
  // instruction, before replacing V in that new instruction.
  static inline void ReplaceAllUsesWithNewInstructions(
      Value *V,
      std::function<Instruction *(Instruction *)> funcCreateAndInsert) {
    SmallVector<User *, 8> Users(V->user_begin(), V->user_end());
    for (User *U : Users) {
      if (Instruction *Inst = dyn_cast<Instruction>(U)) {
        DEBUG(dbgs() << "\nBefore replacement:\n");
        DEBUG(Inst->dump());
        DEBUG(dbgs() << "----\n");

        ReplaceUse(U, V, funcCreateAndInsert(Inst));

        DEBUG(Inst->dump());
      } else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
        auto InstMaker([CE, V, &funcCreateAndInsert](Instruction *UserOfU) {
          Instruction *Inst = CE->getAsInstruction();
          Inst->insertBefore(UserOfU);
          ReplaceUse(Inst, V, funcCreateAndInsert(Inst));

          DEBUG(Inst->dump());
          return Inst;
        });
        ReplaceAllUsesWithNewInstructions(U, InstMaker);
      } else {
        DEBUG(U->dump());
        llvm_unreachable("Expecting only Instruction or ConstantExpr");
      }
    }
  }

  static inline void
  CreateInitFunction(LLVMContext &LLVMCtxt, Module &M, GlobalVariable *MergedGV,
                     StructType *MergedTy, const uint64_t BufferSize,
                     const SmallVectorImpl<GlobalVariable *> &Globals) {
    SmallVector<Constant *, 8> Initializers;
    Initializers.reserve(Globals.size());
    for (size_t i = 0, e = Globals.size(); i != e; ++i) {
      GlobalVariable *G = Globals[i];
      Initializers.push_back(G->getInitializer());
    }
    ArrayRef<Constant *> ArrInit(Initializers.begin(), Initializers.end());
    Constant *MergedInitializer = ConstantStruct::get(MergedTy, ArrInit);
    GlobalVariable *MergedInit =
        new GlobalVariable(M, MergedTy, true, GlobalValue::InternalLinkage,
                           MergedInitializer, "__GPUBlock0");

    Function *UserInit = M.getFunction("init");
    // If there is no user-defined init() function, make the new global
    // initialization function the init().
    StringRef FName(UserInit ? ".rsov.global_init" : "init");
    Function *Func;
    FunctionType *FTy = FunctionType::get(Type::getVoidTy(LLVMCtxt), false);
    Func = Function::Create(FTy, GlobalValue::ExternalLinkage, FName, &M);
    BasicBlock *Blk = BasicBlock::Create(LLVMCtxt, "entry", Func);
    IRBuilder<> LLVMIRBuilder(Blk);
    LoadInst *Load = LLVMIRBuilder.CreateLoad(MergedGV);
    LLVMIRBuilder.CreateMemCpy(Load, MergedInit, BufferSize, 0);
    LLVMIRBuilder.CreateRetVoid();

    // If there is a user-defined init() function, add a call to the global
    // initialization function in the beginning of that function.
    if (UserInit) {
      BasicBlock &EntryBlk = UserInit->getEntryBlock();
      CallInst::Create(Func, {}, "", &EntryBlk.front());
    }
  }

  bool collectGlobals(Module &M, SmallVectorImpl<GlobalVariable *> &Globals) {
    for (GlobalVariable &GV : M.globals()) {
      assert(!GV.hasComdat() && "global variable has a comdat section");
      assert(!GV.hasSection() && "global variable has a non-default section");
      assert(!GV.isDeclaration() && "global variable is only a declaration");
      assert(!GV.isThreadLocal() && "global variable is thread-local");
      assert(GV.getType()->getAddressSpace() == 0 &&
             "global variable has non-default address space");

      // TODO: Constants accessed by kernels should be handled differently
      if (GV.isConstant()) {
        continue;
      }

      // Global Allocations are handled differently in separate passes
      if (isRSAllocation(GV)) {
        continue;
      }

      Globals.push_back(&GV);
    }

    return !Globals.empty();
  }

  bool mForCPU;
};

} // namespace

char GlobalMergePass::ID = 0;

ModulePass *createGlobalMergePass(bool CPU) { return new GlobalMergePass(CPU); }

} // namespace rs2spirv
