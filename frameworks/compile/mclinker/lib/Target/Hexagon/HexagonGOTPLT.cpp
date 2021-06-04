//===- HexagonGOTPLT.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "HexagonGOTPLT.h"
#include "HexagonPLT.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// HexagonGOTPLT
//===----------------------------------------------------------------------===//
HexagonGOTPLT::HexagonGOTPLT(LDSection& pSection) : HexagonGOT(pSection) {
  // Skip GOT0 entries
  for (size_t i = 0; i < HexagonGOTPLT0Num; ++i) {
    create();
  }
  pSection.setAlign(8);
}

HexagonGOTPLT::~HexagonGOTPLT() {
}

// Check if we really have GOT PLT entries ?
bool HexagonGOTPLT::hasGOT1() const {
  return (m_SectionData->size() > HexagonGOTPLT0Num);
}

void HexagonGOTPLT::applyGOT0(uint64_t pAddress) {
  llvm::cast<HexagonGOTEntry>(*(m_SectionData->getFragmentList().begin()))
      .setValue(pAddress);
}

void HexagonGOTPLT::applyAllGOTPLT(const HexagonPLT& pPLT) {
  iterator it = begin();
  // skip GOT0
  for (size_t i = 0; i < HexagonGOTPLT0Num; ++i)
    ++it;
  // Set the initial value of the GOT entry to the address
  // of PLT0, the stub calculates the index of the caller directly from
  // the address where the call arised
  for (; it != end(); ++it) {
    llvm::cast<HexagonGOTEntry>(*it).setValue(pPLT.addr());
  }
}

}  // namespace mcld
