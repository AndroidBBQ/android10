//===- AlignFragment.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_ALIGNFRAGMENT_H_
#define MCLD_FRAGMENT_ALIGNFRAGMENT_H_

#include "mcld/Fragment/Fragment.h"

namespace mcld {

class SectionData;

class AlignFragment : public Fragment {
 public:
  AlignFragment(unsigned int pAlignment,
                int64_t pValue,
                unsigned int pValueSize,
                unsigned int pMaxBytesToEmit,
                SectionData* pSD = NULL);

  unsigned int getAlignment() const { return m_Alignment; }

  int64_t getValue() const { return m_Value; }

  unsigned int getValueSize() const { return m_ValueSize; }

  unsigned int getMaxBytesToEmit() const { return m_MaxBytesToEmit; }

  bool hasEmitNops() const { return m_bEmitNops; }

  void setEmitNops(bool pValue) { m_bEmitNops = pValue; }

  static bool classof(const Fragment* F) {
    return F->getKind() == Fragment::Alignment;
  }

  static bool classof(const AlignFragment*) { return true; }

  size_t size() const;

 private:
  /// Alignment - The alignment to ensure, in bytes.
  unsigned int m_Alignment;

  /// Value - Value to use for filling padding bytes.
  int64_t m_Value;

  /// ValueSize - The size of the integer (in bytes) of \arg Value.
  unsigned int m_ValueSize;

  /// MaxBytesToEmit - The maximum number of bytes to emit; if the alignment
  /// cannot be satisfied in this width then this fragment is ignored.
  unsigned int m_MaxBytesToEmit;

  /// EmitNops - Flag to indicate that (optimal) NOPs should be emitted instead
  /// of using the provided value. The exact interpretation of this flag is
  /// target dependent.
  bool m_bEmitNops : 1;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_ALIGNFRAGMENT_H_
