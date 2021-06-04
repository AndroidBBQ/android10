//===- ARMToTHMStub.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ARMToTHMStub.h"
#include "ARMLDBackend.h"

#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <llvm/Support/ELF.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// ARMToTHMStub
//===----------------------------------------------------------------------===//
const uint32_t ARMToTHMStub::PIC_TEMPLATE[] = {
    0xe59fc004,  // ldr   r12, [pc, #4]
    0xe08fc00c,  // add   ip, pc, ip
    0xe12fff1c,  // bx    ip
    0x0          // dcd   R_ARM_REL32(X)
};

const uint32_t ARMToTHMStub::TEMPLATE[] = {
    0xe59fc000,  // ldr   ip, [pc, #0]
    0xe12fff1c,  // bx    ip
    0x0          // dcd   R_ARM_ABS32(X)
};

ARMToTHMStub::ARMToTHMStub(bool pIsOutputPIC)
    : m_pData(NULL), m_Name("A2T_prototype"), m_Size(0x0) {
  if (pIsOutputPIC) {
    m_pData = PIC_TEMPLATE;
    m_Size = sizeof(PIC_TEMPLATE);
    addFixup(12u, 0x0, llvm::ELF::R_ARM_REL32);
  } else {
    m_pData = TEMPLATE;
    m_Size = sizeof(TEMPLATE);
    addFixup(8u, 0x0, llvm::ELF::R_ARM_ABS32);
  }
}

/// for doClone
ARMToTHMStub::ARMToTHMStub(const uint32_t* pData,
                           size_t pSize,
                           const_fixup_iterator pBegin,
                           const_fixup_iterator pEnd)
    : m_pData(pData), m_Name("A2T_veneer"), m_Size(pSize) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it)
    addFixup(**it);
}

ARMToTHMStub::~ARMToTHMStub() {
}

bool ARMToTHMStub::isMyDuty(const class Relocation& pReloc,
                            uint64_t pSource,
                            uint64_t pTargetSymValue) const {
  bool result = false;
  // Check if the branch target is THUMB
  if ((pTargetSymValue & 0x1) != 0x0) {
    switch (pReloc.type()) {
      case llvm::ELF::R_ARM_CALL: {
        // FIXME: Assuming blx is available (i.e., target is armv5 or above!)
        // then, we do not need a stub unless the branch target is too far.
        uint64_t dest = pTargetSymValue + pReloc.addend() + 8u;
        int64_t branch_offset = static_cast<int64_t>(dest) - pSource;
        if ((branch_offset > ARMGNULDBackend::ARM_MAX_FWD_BRANCH_OFFSET) ||
            (branch_offset < ARMGNULDBackend::ARM_MAX_BWD_BRANCH_OFFSET)) {
          result = true;
          break;
        }
        break;
      }
      case llvm::ELF::R_ARM_PC24:
      case llvm::ELF::R_ARM_JUMP24:
      case llvm::ELF::R_ARM_PLT32: {
        // always need a stub to switch mode
        result = true;
        break;
      }
      default:
        break;
    }
  }
  return result;
}

const std::string& ARMToTHMStub::name() const {
  return m_Name;
}

const uint8_t* ARMToTHMStub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t ARMToTHMStub::size() const {
  return m_Size;
}

size_t ARMToTHMStub::alignment() const {
  return 4u;
}

Stub* ARMToTHMStub::doClone() {
  return new ARMToTHMStub(m_pData, m_Size, fixup_begin(), fixup_end());
}

}  // namespace mcld
