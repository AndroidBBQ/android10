//===- ARMELFDynamic.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ARMELFDynamic.h"

#include "mcld/LD/ELFFileFormat.h"

namespace mcld {

ARMELFDynamic::ARMELFDynamic(const GNULDBackend& pParent,
                             const LinkerConfig& pConfig)
    : ELFDynamic(pParent, pConfig) {
}

ARMELFDynamic::~ARMELFDynamic() {
}

void ARMELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat) {
  // reservePLTGOT
  if (pFormat.hasGOT())
    reserveOne(llvm::ELF::DT_PLTGOT);
}

void ARMELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat) {
  // applyPLTGOT
  if (pFormat.hasGOT())
    applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOT().addr());
}

}  // namespace mcld
