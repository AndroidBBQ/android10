//===- X86TargetInfo.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/Target.h"
#include "mcld/Support/TargetRegistry.h"

namespace mcld {

mcld::Target TheX86_32Target;
mcld::Target TheX86_64Target;

extern "C" void MCLDInitializeX86LDTargetInfo() {
  // register into mcld::TargetRegistry
  mcld::RegisterTarget<llvm::Triple::x86> X(TheX86_32Target, "x86");
  mcld::RegisterTarget<llvm::Triple::x86_64> Y(TheX86_64Target, "x86-64");
}

}  // namespace mcld
