//===- ARMTargetInfo.cpp --------------------------------------------------===//
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

mcld::Target TheARMTarget;
mcld::Target TheThumbTarget;

extern "C" void MCLDInitializeARMLDTargetInfo() {
  // register into mcld::TargetRegistry
  mcld::RegisterTarget<llvm::Triple::arm> X(TheARMTarget, "arm");
  mcld::RegisterTarget<llvm::Triple::thumb> Y(TheThumbTarget, "thumb");
}

}  // namespace mcld
