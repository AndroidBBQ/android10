//===- MipsPLT.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>
#include "mcld/Support/MsgHandling.h"
#include "MipsGOTPLT.h"
#include "MipsPLT.h"

namespace {

const uint32_t PLT0[] = {
    0x3c1c0000,  // lui $28, %hi(&GOTPLT[0])
    0x8f990000,  // lw $25, %lo(&GOTPLT[0])($28)
    0x279c0000,  // addiu $28, $28, %lo(&GOTPLT[0])
    0x031cc023,  // subu $24, $24, $28
    0x03e07821,  // move $15, $31
    0x0018c082,  // srl $24, $24, 2
    0x0320f809,  // jalr $25
    0x2718fffe   // subu $24, $24, 2
};

const uint32_t PLTA[] = {
    0x3c0f0000,  // lui $15, %hi(.got.plt entry)
    0x8df90000,  // l[wd] $25, %lo(.got.plt entry)($15)
    0x03200008,  // jr $25
    0x25f80000   // addiu $24, $15, %lo(.got.plt entry)
};

}  // anonymous namespace

namespace mcld {

//===----------------------------------------------------------------------===//
// MipsPLT0 Entry
//===----------------------------------------------------------------------===//
class MipsPLT0 : public PLT::Entry<sizeof(PLT0)> {
 public:
  MipsPLT0(SectionData& pParent) : PLT::Entry<sizeof(PLT0)>(pParent) {}
};

//===----------------------------------------------------------------------===//
// MipsPLTA Entry
//===----------------------------------------------------------------------===//
class MipsPLTA : public PLT::Entry<sizeof(PLTA)> {
 public:
  MipsPLTA(SectionData& pParent) : PLT::Entry<sizeof(PLTA)>(pParent) {}
};

//===----------------------------------------------------------------------===//
// MipsPLT
//===----------------------------------------------------------------------===//
MipsPLT::MipsPLT(LDSection& pSection) : PLT(pSection) {
  new MipsPLT0(*m_pSectionData);
}

void MipsPLT::finalizeSectionSize() {
  uint64_t size = sizeof(PLT0) + (m_pSectionData->size() - 1) * sizeof(PLTA);
  m_Section.setSize(size);

  uint32_t offset = 0;
  SectionData::iterator frag, fragEnd = m_pSectionData->end();
  for (frag = m_pSectionData->begin(); frag != fragEnd; ++frag) {
    frag->setOffset(offset);
    offset += frag->size();
  }
}

bool MipsPLT::hasPLT1() const {
  return m_pSectionData->size() > 1;
}

uint64_t MipsPLT::emit(MemoryRegion& pRegion) {
  uint64_t result = 0x0;
  iterator it = begin();

  unsigned char* buffer = pRegion.begin();
  memcpy(buffer, llvm::cast<MipsPLT0>((*it)).getValue(), MipsPLT0::EntrySize);
  result += MipsPLT0::EntrySize;
  ++it;

  MipsPLTA* plta = 0;
  for (iterator ie = end(); it != ie; ++it) {
    plta = &(llvm::cast<MipsPLTA>(*it));
    memcpy(buffer + result, plta->getValue(), MipsPLTA::EntrySize);
    result += MipsPLTA::EntrySize;
  }
  return result;
}

PLTEntryBase* MipsPLT::create() {
  return new MipsPLTA(*m_pSectionData);
}

void MipsPLT::applyAllPLT(MipsGOTPLT& pGOTPLT) {
  assert(m_Section.addr() && ".plt base address is NULL!");

  size_t count = 0;
  for (iterator it = m_pSectionData->begin(); it != m_pSectionData->end();
       ++it) {
    PLTEntryBase* plt = &(llvm::cast<PLTEntryBase>(*it));

    if (it == m_pSectionData->begin()) {
      uint32_t* data = static_cast<uint32_t*>(malloc(plt->size()));

      if (!data)
        fatal(diag::fail_allocate_memory_plt);

      memcpy(data, PLT0, plt->size());

      uint64_t gotAddr = pGOTPLT.addr();

      data[0] |= ((gotAddr + 0x8000) >> 16) & 0xffff;
      data[1] |= gotAddr & 0xffff;
      data[2] |= gotAddr & 0xffff;

      plt->setValue(reinterpret_cast<unsigned char*>(data));
    } else {
      uint32_t* data = static_cast<uint32_t*>(malloc(plt->size()));

      if (!data)
        fatal(diag::fail_allocate_memory_plt);

      memcpy(data, PLTA, plt->size());

      uint64_t gotEntryAddr = pGOTPLT.getEntryAddr(count++);

      data[0] |= ((gotEntryAddr + 0x8000) >> 16) & 0xffff;
      data[1] |= gotEntryAddr & 0xffff;
      data[3] |= gotEntryAddr & 0xffff;

      plt->setValue(reinterpret_cast<unsigned char*>(data));
    }
  }
}

}  // namespace mcld
