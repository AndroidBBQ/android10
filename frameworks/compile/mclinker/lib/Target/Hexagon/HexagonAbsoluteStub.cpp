//===- HexagonAbsoluteStub.cpp --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "HexagonAbsoluteStub.h"
#include "HexagonLDBackend.h"

#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/Fragment/Relocation.h"

#include <llvm/Support/ELF.h>
#include <llvm/Support/MathExtras.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// HexagonAbsoluteStub
//===----------------------------------------------------------------------===//

const uint32_t HexagonAbsoluteStub::TEMPLATE[] = {
    0xbffd7f1d, /* { sp = add (sp, #-8)      */
    0xa79dfcfe, /*   memw (sp + #-8) = r28 } */
    0x723cc000, /* r28.h = #HI (foo)         */
    0x713cc000, /* r28.l = #LO (foo)         */
    0xb01d411d, /* { sp = add (sp, #8)       */
    0x529c4000, /*   jumpr r28               */
    0x919dc01c  /*   r28 = memw (sp) }       */
};

#define FITS_IN_NBITS(D, B) \
  (std::abs(D) < (~(~(int64_t)0 << ((B)-1)) & -(4 * 4)))

HexagonAbsoluteStub::HexagonAbsoluteStub(bool pIsOutputPIC)
    : Stub(), m_Name("HexagonTrampoline"), m_pData(NULL), m_Size(0x0) {
  m_pData = TEMPLATE;
  m_Size = sizeof(TEMPLATE);
  addFixup(8u, 0x0, llvm::ELF::R_HEX_HI16);
  addFixup(12u, 0x0, llvm::ELF::R_HEX_LO16);
}

/// for doClone
HexagonAbsoluteStub::HexagonAbsoluteStub(const uint32_t* pData,
                                         size_t pSize,
                                         const_fixup_iterator pBegin,
                                         const_fixup_iterator pEnd)
    : Stub(), m_Name("AbsVeneer"), m_pData(pData), m_Size(pSize) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it)
    addFixup(**it);
}

HexagonAbsoluteStub::~HexagonAbsoluteStub() {
}

bool HexagonAbsoluteStub::isMyDuty(const class Relocation& pReloc,
                                   uint64_t pSource,
                                   uint64_t pTargetSymValue) const {
  int nbits = 0;
  switch (pReloc.type()) {
    case llvm::ELF::R_HEX_B22_PCREL:
      nbits = 24;
      break;
    case llvm::ELF::R_HEX_B15_PCREL:
      nbits = 17;
      break;
    case llvm::ELF::R_HEX_B7_PCREL:
      nbits = 9;
      break;
    case llvm::ELF::R_HEX_B13_PCREL:
      nbits = 15;
      break;
    case llvm::ELF::R_HEX_B9_PCREL:
      nbits = 17;
      break;
    default:
      return false;
  }

  int64_t offset = pTargetSymValue - pSource;
  // if offset is going to fit in nbits then we dont
  // need a stub to be created
  if (FITS_IN_NBITS(offset, nbits))
    return false;
  return true;
}

const std::string& HexagonAbsoluteStub::name() const {
  return m_Name;
}

const uint8_t* HexagonAbsoluteStub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t HexagonAbsoluteStub::size() const {
  return m_Size;
}

size_t HexagonAbsoluteStub::alignment() const {
  return 4u;
}

Stub* HexagonAbsoluteStub::doClone() {
  return new HexagonAbsoluteStub(m_pData, m_Size, fixup_begin(), fixup_end());
}

}  // namespace mcld
