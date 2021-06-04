//===- MipsLA25Stub.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ResolveInfo.h"
#include "MipsLA25Stub.h"
#include "MipsLDBackend.h"

namespace {

const uint32_t STUB[] = {
    0x3c190000,  // lui $25,%hi(func)
    0x08000000,  // j func
    0x27390000,  // add $25,$25,%lo(func)
    0x00000000   // nop
};

}  // anonymous namespace

namespace mcld {

//===----------------------------------------------------------------------===//
// MipsLA25Stub
//===----------------------------------------------------------------------===//

MipsLA25Stub::MipsLA25Stub(const MipsGNULDBackend& pTarget)
    : m_Target(pTarget),
      m_Name("MipsLA25_Prototype"),
      m_pData(STUB),
      m_Size(sizeof(STUB)) {
  addFixup(0, 0x0, llvm::ELF::R_MIPS_HI16);
  addFixup(4, 0x0, llvm::ELF::R_MIPS_26);
  addFixup(8, 0x0, llvm::ELF::R_MIPS_LO16);
}

MipsLA25Stub::MipsLA25Stub(const MipsGNULDBackend& pTarget,
                           const uint32_t* pData,
                           size_t pSize,
                           const_fixup_iterator pBegin,
                           const_fixup_iterator pEnd)
    : m_Target(pTarget), m_Name("pic"), m_pData(pData), m_Size(pSize) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it)
    addFixup(**it);
}

bool MipsLA25Stub::isMyDuty(const Relocation& pReloc,
                            uint64_t pSource,
                            uint64_t pTargetSymValue) const {
  if (llvm::ELF::R_MIPS_26 != pReloc.type())
    return false;

  const ResolveInfo* rsym = pReloc.symInfo();

  if (!rsym->isDefine())
    return false;

  if (rsym->isDyn() || rsym->isUndef())
    return false;

  if (!m_Target.hasNonPICBranch(rsym))
    return false;

  return true;
}

const std::string& MipsLA25Stub::name() const {
  return m_Name;
}

const uint8_t* MipsLA25Stub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t MipsLA25Stub::size() const {
  return m_Size;
}

size_t MipsLA25Stub::alignment() const {
  return 4;
}

Stub* MipsLA25Stub::doClone() {
  return new MipsLA25Stub(
      m_Target, m_pData, m_Size, fixup_begin(), fixup_end());
}

}  // namespace mcld
