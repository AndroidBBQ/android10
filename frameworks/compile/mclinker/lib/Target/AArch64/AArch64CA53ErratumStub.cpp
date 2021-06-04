//===- AArch64CA53ErratumStub.cpp -----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AArch64CA53ErratumStub.h"
#include "AArch64InsnHelpers.h"
#include "AArch64LDBackend.h"
#include "AArch64RelocationHelpers.h"
#include "AArch64Relocator.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/IRBuilder.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ELF.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64CA53ErratumStub
//===----------------------------------------------------------------------===//
const uint32_t AArch64CA53ErratumStub::TEMPLATE[] = {
  0x00000000,  // Placeholder for erratum insn
  0x00000000,  // Palceholder for branch instruction
};

AArch64CA53ErratumStub::AArch64CA53ErratumStub()
    : m_pData(NULL),
      m_Name("erratum__prototype"),
      m_Size(0x0) {
  m_pData = TEMPLATE;
  m_Size = sizeof(TEMPLATE);
  addFixup(0x0, 0, AArch64Relocator::R_AARCH64_REWRITE_INSN);
  addFixup(0x4, 0, llvm::ELF::R_AARCH64_JUMP26);
}

/// for doClone
AArch64CA53ErratumStub::AArch64CA53ErratumStub(const uint32_t* pData,
                                               size_t pSize,
                                               const char* pName,
                                               const_fixup_iterator pBegin,
                                               const_fixup_iterator pEnd)
    : m_pData(pData),
      m_Name(pName),
      m_Size(pSize) {
  for (const_fixup_iterator it = pBegin, ie = pEnd; it != ie; ++it) {
    addFixup(**it);
  }
}

AArch64CA53ErratumStub::~AArch64CA53ErratumStub() {
}

bool AArch64CA53ErratumStub::isMyDuty(const FragmentRef& pFragRef) const {
  return false;
}

void AArch64CA53ErratumStub::applyFixup(FragmentRef& pSrcFragRef,
                                        IRBuilder& pBuilder,
                                        BranchIsland& pIsland) {
  assert(isMyDuty(pSrcFragRef));

  // Build stub symbol name.
  std::string sym_name("__");
  sym_name.append(name())
          .append(llvm::utohexstr(pIsland.numOfStubs()))
          .append("@")
          .append(pIsland.name());

  // Create LDSymbol for the stub.
  LDSymbol* stub_sym =
      pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
          sym_name,
          ResolveInfo::NoType,
          ResolveInfo::Define,
          ResolveInfo::Local,
          size(),
          initSymValue(),
          FragmentRef::Create(*this, initSymValue()),
          ResolveInfo::Default);
  setSymInfo(stub_sym->resolveInfo());

  // Create the target symbol of the stub to the next instruction of erratum
  // pattarn.
  FragmentRef* target = FragmentRef::Create(*pSrcFragRef.frag(),
                                            pSrcFragRef.offset() +
                                                getErratumInsnOffset() +
                                                AArch64InsnHelpers::InsnSize);
  ResolveInfo* target_info = pBuilder.CreateLocalSymbol(*target);

  // Apply the fixups.
  fixup_iterator it = fixup_begin();
  // Rewrite the first instruction as the erratum instruction.
  Relocation* reloc =
      Relocation::Create((*it)->type(),
                         *(FragmentRef::Create(*this, (*it)->offset())),
                         (*it)->addend());
  reloc->setSymInfo(target_info);

  std::unique_ptr<unsigned[]> code(new unsigned[getErratumSequenceSize() / 4]);
  pSrcFragRef.memcpy(code.get(), getErratumSequenceSize(), 0);
  reloc->target() =
      code[getErratumInsnOffset() / AArch64InsnHelpers::InsnSize];
  pIsland.addRelocation(*reloc);

  // Construct the second instruction as a branch to target.
  ++it;
  reloc = Relocation::Create((*it)->type(),
                             *(FragmentRef::Create(*this, (*it)->offset())),
                             (*it)->addend());
  reloc->setSymInfo(target_info);
  reloc->target() = AArch64InsnHelpers::buildBranchInsn();
  pIsland.addRelocation(*reloc);

  assert((++it) == fixup_end());
}

const std::string& AArch64CA53ErratumStub::name() const {
  return m_Name;
}

const uint32_t* AArch64CA53ErratumStub::getData() const {
  return m_pData;
}

const uint8_t* AArch64CA53ErratumStub::getContent() const {
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t AArch64CA53ErratumStub::size() const {
  return m_Size;
}

size_t AArch64CA53ErratumStub::alignment() const {
  return 8;
}

}  // namespace mcld
