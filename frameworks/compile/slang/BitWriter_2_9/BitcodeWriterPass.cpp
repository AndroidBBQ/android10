//===--- Bitcode/Writer/BitcodeWriterPass.cpp - Bitcode Writer ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// BitcodeWriterPass implementation.
//
//===----------------------------------------------------------------------===//

#include "ReaderWriter_2_9.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
using namespace llvm;

namespace {
  class WriteBitcodePass : public ModulePass {
    raw_ostream &OS; // raw_ostream to print on

  public:
    static char ID; // Pass identification, replacement for typeid
    explicit WriteBitcodePass(raw_ostream &o)
      : ModulePass(ID), OS(o) {}

    const char *getPassName() const { return "Bitcode Writer"; }

    bool runOnModule(Module &M) {
      bool Changed = false;
      llvm_2_9::WriteBitcodeToFile(&M, OS);
      return Changed;
    }
  };
}

char WriteBitcodePass::ID = 0;

/// createBitcodeWriterPass - Create and return a pass that writes the module
/// to the specified ostream.
llvm::ModulePass *llvm_2_9::createBitcodeWriterPass(llvm::raw_ostream &Str) {
  return new WriteBitcodePass(Str);
}
