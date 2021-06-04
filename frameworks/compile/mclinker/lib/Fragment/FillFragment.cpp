//===- FillFragment.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/FillFragment.h"
#include "mcld/LD/SectionData.h"
#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// FillFragment
//===----------------------------------------------------------------------===//
FillFragment::FillFragment(int64_t pValue,
                           unsigned int pValueSize,
                           uint64_t pSize,
                           SectionData* pSD)
    : Fragment(Fragment::Fillment, pSD),
      m_Value(pValue),
      m_ValueSize(pValueSize),
      m_Size(pSize) {
  assert((!m_ValueSize || (m_Size % m_ValueSize) == 0) &&
         "Fill size must be a multiple of the value size!");
}

}  // namespace mcld
