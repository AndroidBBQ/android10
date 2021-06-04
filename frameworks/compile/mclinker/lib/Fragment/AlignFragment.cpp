//===- AlignFragment.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/AlignFragment.h"
#include "mcld/LD/SectionData.h"

#include <llvm/Support/MathExtras.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// AlignFragment
//===----------------------------------------------------------------------===//
AlignFragment::AlignFragment(unsigned int pAlignment,
                             int64_t pValue,
                             unsigned int pValueSize,
                             unsigned int pMaxBytesToEmit,
                             SectionData* pSD)
    : Fragment(Fragment::Alignment, pSD),
      m_Alignment(pAlignment),
      m_Value(pValue),
      m_ValueSize(pValueSize),
      m_MaxBytesToEmit(pMaxBytesToEmit),
      m_bEmitNops(false) {
}

size_t AlignFragment::size() const {
  assert(hasOffset() &&
         "AlignFragment::size() should not be called before layout.");
  uint64_t size = llvm::OffsetToAlignment(getOffset(), m_Alignment);
  if (size > m_MaxBytesToEmit)
    return 0;

  return size;
}

}  // namespace mcld
