//===- X86ELFDynamic.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86ELFDynamic.h"

#include "mcld/LD/ELFFileFormat.h"

namespace mcld {

X86ELFDynamic::X86ELFDynamic(const GNULDBackend& pParent,
                             const LinkerConfig& pConfig)
    : ELFDynamic(pParent, pConfig) {
}

X86ELFDynamic::~X86ELFDynamic() {
}

void X86ELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat) {
  // reservePLTGOT
  if (pFormat.hasGOTPLT())
    reserveOne(llvm::ELF::DT_PLTGOT);
}

void X86ELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat) {
  // applyPLTGOT
  if (pFormat.hasGOTPLT())
    applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOTPLT().addr());
}

}  // namespace mcld
