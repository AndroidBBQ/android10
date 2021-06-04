//===- THMToARMStub.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "THMToARMStub.h"
#include "ARMLDBackend.h"

#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <llvm/Support/ELF.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// THMToARMStub
//===----------------------------------------------------------------------===//
const uint32_t THMToARMStub::PIC_TEMPLATE[] = {
    0x46c04778,  // bx    pc ... nop
    0xe59fc000,  // ldr   ip, [pc, #0]
    0xe08cf00f,  // add   pc, ip, pc
    0x0          // dcd   R_ARM_REL32(X)
};

const uint32_t THMToARMStub::TEMPLATE[] = {
    0x46c04778,  // bx    pc ... nop
    0xe51ff004,  // ldr   pc, [pc, #-4]
    0x0          // dcd   R_ARM_ABS32(X)
};

THMToARMStub::THMToARMStub(bool pIsOutputPIC, bool pUsingThumb2)
    : m_pData(NULL),
      m_Name("T2A_prototype"),
      m_Size(0x0),
      m_bUsingThumb2(pUsingThumb2) {
  if (pIsOutputPIC) {
    m_pData = PIC_TEMPLATE;
    m_Size = sizeof(PIC_TEMPLATE);
    addFixup(12u, -4, llvm::ELF::R_ARM_REL32);
  } else {
    m_pData = TEMPLATE;
    m_Size = sizeof(TEMPLATE);
    addFixup(8u, 0x0, llvm::ELF::R_ARM_ABS32);
  }
}

/// for doClone
THMToARMStub::THMToARMStub(const uint32_t* pData,
                           size_t pSize,
                           const_fixup_iterator pBegin,
                           const_fixup_iterator pEnd,
                           bool pUsingThumb2)
    : m_pData(pData),
      m_Name("T2A_veneer"),
      m_Size(pSize),
      m_bUsingThumb2(pUsingThumb2) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it)
    addFixup(**it);
}

THMToARMStub::~THMToARMStub() {
}

bool THMToARMStub::isMyDuty(const class Relocation& pReloc,
                            uint64_t pSource,
                            uint64_t pTargetSymValue) const {
  bool result = false;
  // Check if the branch target is ARM
  if ((pTargetSymValue & 0x1) == 0x0) {
    switch (pReloc.type()) {
      case llvm::ELF::R_ARM_THM_CALL: {
        // FIXME: Assuming blx is available (i.e., target is armv5 or above!)
        // then, we do not need a stub unless the branch target is too far.
        uint64_t dest = pTargetSymValue + pReloc.addend() + 4u;
        int64_t branch_offset = static_cast<int64_t>(dest) - pSource;
        if (m_bUsingThumb2) {
          if ((branch_offset > ARMGNULDBackend::THM2_MAX_FWD_BRANCH_OFFSET) ||
              (branch_offset < ARMGNULDBackend::THM2_MAX_BWD_BRANCH_OFFSET)) {
            result = true;
            break;
          }
        } else {
          if ((branch_offset > ARMGNULDBackend::THM_MAX_FWD_BRANCH_OFFSET) ||
              (branch_offset < ARMGNULDBackend::THM_MAX_BWD_BRANCH_OFFSET)) {
            result = true;
            break;
          }
        }
        break;
      }
      case llvm::ELF::R_ARM_THM_JUMP24: {
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

const std::string& THMToARMStub::name() const {
  return m_Name;
}

const uint8_t* THMToARMStub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t THMToARMStub::size() const {
  return m_Size;
}

size_t THMToARMStub::alignment() const {
  return 4u;
}

// for T bit of this stub
uint64_t THMToARMStub::initSymValue() const {
  return 0x1;
}

Stub* THMToARMStub::doClone() {
  return new THMToARMStub(
      m_pData, m_Size, fixup_begin(), fixup_end(), m_bUsingThumb2);
}

}  // namespace mcld
