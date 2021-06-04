//===- HexagonGOT.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "HexagonGOT.h"

#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/SectionData.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// HexagonGOT
//===----------------------------------------------------------------------===//
HexagonGOT::HexagonGOT(LDSection& pSection) : GOT(pSection) {
}

HexagonGOT::~HexagonGOT() {
}

HexagonGOTEntry* HexagonGOT::create() {
  return new HexagonGOTEntry(0, m_SectionData);
}

}  // namespace mcld
