//===- AArch64ELFDynamic.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64ELFDynamic.h"

#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LinkerConfig.h"

namespace mcld {

AArch64ELFDynamic::AArch64ELFDynamic(const GNULDBackend& pParent,
                                     const LinkerConfig& pConfig)
    : ELFDynamic(pParent, pConfig) {
}

AArch64ELFDynamic::~AArch64ELFDynamic() {
}

void AArch64ELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat) {
  // reservePLTGOT
  if (config().options().hasNow()) {
    if (pFormat.hasGOT())
      reserveOne(llvm::ELF::DT_PLTGOT);
  } else {
    if (pFormat.hasGOTPLT())
      reserveOne(llvm::ELF::DT_PLTGOT);
  }
}

void AArch64ELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat) {
  // applyPLTGOT
  if (config().options().hasNow()) {
    if (pFormat.hasGOT())
      applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOT().addr());
  } else {
    if (pFormat.hasGOTPLT())
      applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOTPLT().addr());
  }
}

}  // namespace mcld
