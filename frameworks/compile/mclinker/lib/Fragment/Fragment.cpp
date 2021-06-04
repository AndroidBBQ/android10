//===- Fragment.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "mcld/Fragment/Fragment.h"
#include "mcld/LD/SectionData.h"

#include <llvm/Support/DataTypes.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Fragment
//===----------------------------------------------------------------------===//
Fragment::Fragment()
    : m_Kind(Type(~0)), m_pParent(NULL), m_Offset(~uint64_t(0)) {
}

Fragment::Fragment(Type pKind, SectionData* pParent)
    : m_Kind(pKind), m_pParent(pParent), m_Offset(~uint64_t(0)) {
  if (m_pParent != NULL)
    m_pParent->getFragmentList().push_back(this);
}

Fragment::~Fragment() {
}

uint64_t Fragment::getOffset() const {
  assert(hasOffset() && "Cannot getOffset() before setting it up.");
  return m_Offset;
}

bool Fragment::hasOffset() const {
  return (m_Offset != ~uint64_t(0));
}

}  // namespace mcld
