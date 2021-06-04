//===- AArch64LongBranchStub.cpp ------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AArch64LongBranchStub.h"
#include "AArch64LDBackend.h"
#include "AArch64RelocationHelpers.h"

#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <llvm/Support/ELF.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64LongBranchStub
//===----------------------------------------------------------------------===//
const uint32_t AArch64LongBranchStub::PIC_TEMPLATE[] = {
  0x58000090,  // ldr   ip0, 0x10
  0x10000011,  // adr   ip1, #0
  0x8b110210,  // add   ip0, ip0, ip1
  0xd61f0200,  // br    ip0
  0x00000000,  // .xword <-- R_AARCH64_PREL64(X+12)
  0x00000000,
};

const uint32_t AArch64LongBranchStub::TEMPLATE[] = {
  0x58000050,  // ldr   ip0, 0x8
  0xd61f0200,  // br    ip0
  0x00000000,  // .xword <-- R_AARCH64_PREL64(X)
  0x00000000,
};

const uint32_t AArch64LongBranchStub::ADRP_TEMPLATE[] = {
  0x90000010,  // adrp  ip0, X <-- R_AARCH64_ADR_PREL_PG_HI21(X)
  0x91000210,  // add   ip0, ip0, :lo12:X <-- R_AARCH64_ADD_ABS_LO12_NC(X)
  0xd61f0200,  // br    ip0
};

AArch64LongBranchStub::AArch64LongBranchStub(bool pIsOutputPIC)
    : m_pData(NULL),
      m_Name("ljmp_prototype"),
      m_Size(0x0) {
  if (pIsOutputPIC) {
    m_pData = PIC_TEMPLATE;
    m_Size = sizeof(PIC_TEMPLATE);
    addFixup(0x10, 12, llvm::ELF::R_AARCH64_PREL64);
  } else {
    m_pData = TEMPLATE;
    m_Size = sizeof(TEMPLATE);
    addFixup(0x8, 0, llvm::ELF::R_AARCH64_PREL64);
  }
}

/// for doClone
AArch64LongBranchStub::AArch64LongBranchStub(const uint32_t* pData,
                                             size_t pSize,
                                             const_fixup_iterator pBegin,
                                             const_fixup_iterator pEnd)
    : m_pData(pData),
      m_Name("ljmp_veneer"),
      m_Size(pSize) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it) {
    addFixup(**it);
  }
}

AArch64LongBranchStub::~AArch64LongBranchStub() {
}

bool AArch64LongBranchStub::isMyDuty(const Relocation& pReloc,
                                     uint64_t pSource,
                                     uint64_t pTargetSymValue) const {
  assert((pReloc.type() == llvm::ELF::R_AARCH64_CALL26) ||
         (pReloc.type() == llvm::ELF::R_AARCH64_JUMP26));
  int64_t dest = pTargetSymValue + pReloc.addend();
  int64_t branch_offset = dest - pSource;
  if ((branch_offset > AArch64GNULDBackend::MAX_FWD_BRANCH_OFFSET) ||
      (branch_offset < AArch64GNULDBackend::MAX_BWD_BRANCH_OFFSET)) {
    return true;
  }
  return false;
}

static bool isValidForADRP(uint64_t pSource, uint64_t pDest) {
  int64_t imm = static_cast<int64_t>((helper_get_page_address(pDest) -
                                      helper_get_page_address(pSource))) >> 12;
  return ((imm <= AArch64GNULDBackend::MAX_ADRP_IMM) &&
          (imm >= AArch64GNULDBackend::MIN_ADRP_IMM));
}

void AArch64LongBranchStub::applyFixup(Relocation& pSrcReloc,
                                       IRBuilder& pBuilder,
                                       BranchIsland& pIsland) {
  // Try to relax the stub itself.
  LDSymbol* symbol = pSrcReloc.symInfo()->outSymbol();
  uint64_t dest = symbol->fragRef()->frag()->getParent()->getSection().addr() +
                  symbol->fragRef()->getOutputOffset();
  uint64_t src = pIsland.getParent()->getSection().addr() +
                 pIsland.offset() +
                 pIsland.size();
  if (isValidForADRP(src, dest)) {
    m_pData = ADRP_TEMPLATE;
    m_Name = "adrp_veneer";
    m_Size = sizeof(ADRP_TEMPLATE);

    getFixupList().clear();
    addFixup(0x0, 0, llvm::ELF::R_AARCH64_ADR_PREL_PG_HI21);
    addFixup(0x4, 0, llvm::ELF::R_AARCH64_ADD_ABS_LO12_NC);
  }

  Stub::applyFixup(pSrcReloc, pBuilder, pIsland);
}

const std::string& AArch64LongBranchStub::name() const {
  return m_Name;
}

const uint8_t* AArch64LongBranchStub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t AArch64LongBranchStub::size() const {
  return m_Size;
}

size_t AArch64LongBranchStub::alignment() const {
  return 8;
}

Stub* AArch64LongBranchStub::doClone() {
  return new AArch64LongBranchStub(m_pData, m_Size, fixup_begin(), fixup_end());
}

}  // namespace mcld
