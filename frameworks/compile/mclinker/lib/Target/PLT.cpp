//===- PLT.cpp ------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Target/PLT.h"

#include "mcld/IRBuilder.h"

namespace mcld {

class GOT;

//===----------------------------------------------------------------------===//
// PLT
//===----------------------------------------------------------------------===//
PLT::PLT(LDSection& pSection) : m_Section(pSection) {
  m_pSectionData = IRBuilder::CreateSectionData(pSection);
}

PLT::~PLT() {
}

}  // namespace mcld
