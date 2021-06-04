//===- FillFragment.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_FILLFRAGMENT_H_
#define MCLD_FRAGMENT_FILLFRAGMENT_H_

#include "mcld/Fragment/Fragment.h"

#include <llvm/Support/DataTypes.h>

namespace mcld {

class SectionData;

class FillFragment : public Fragment {
 public:
  FillFragment(int64_t pValue,
               unsigned int pValueSize,
               uint64_t pSize,
               SectionData* pSD = NULL);

  int64_t getValue() const { return m_Value; }

  unsigned getValueSize() const { return m_ValueSize; }

  static bool classof(const Fragment* F) {
    return F->getKind() == Fragment::Fillment;
  }

  static bool classof(const FillFragment*) { return true; }

  size_t size() const { return m_Size; }

 private:
  /// m_Value - Value used for filling bytes
  int64_t m_Value;

  /// m_ValueSize - The size (in bytes) of \arg Value to use when filling, or 0
  /// if this is a virtual fill fragment.
  unsigned int m_ValueSize;

  /// m_Size - The number of bytes to insert.
  uint64_t m_Size;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_FILLFRAGMENT_H_
