//===- HexagonELFDynamic.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "HexagonELFDynamic.h"

#include "mcld/LD/ELFFileFormat.h"

namespace mcld {

HexagonELFDynamic::HexagonELFDynamic(const GNULDBackend& pParent,
                                     const LinkerConfig& pConfig)
    : ELFDynamic(pParent, pConfig) {
}

HexagonELFDynamic::~HexagonELFDynamic() {
}

void HexagonELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat) {
  // reservePLTGOT
  if (pFormat.hasGOTPLT())
    reserveOne(llvm::ELF::DT_PLTGOT);
}

void HexagonELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat) {
  // applyPLTGOT
  if (pFormat.hasGOTPLT())
    applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOTPLT().addr());
}

}  // namespace mcld
