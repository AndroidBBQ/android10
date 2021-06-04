//===- AArch64PLT.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64GOT.h"
#include "AArch64PLT.h"
#include "AArch64RelocationHelpers.h"

#include "mcld/LD/LDSection.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>

#include <new>

namespace mcld {

AArch64PLT0::AArch64PLT0(SectionData& pParent)
    : PLT::Entry<sizeof(aarch64_plt0)>(pParent) {
}

AArch64PLT1::AArch64PLT1(SectionData& pParent)
    : PLT::Entry<sizeof(aarch64_plt1)>(pParent) {
}

//===----------------------------------------------------------------------===//
// AArch64PLT

AArch64PLT::AArch64PLT(LDSection& pSection, AArch64GOT& pGOTPLT)
    : PLT(pSection), m_GOT(pGOTPLT) {
  new AArch64PLT0(*m_pSectionData);
}

AArch64PLT::~AArch64PLT() {
}

bool AArch64PLT::hasPLT1() const {
  return (m_pSectionData->size() > 1);
}

void AArch64PLT::finalizeSectionSize() {
  uint64_t size = (m_pSectionData->size() - 1) * sizeof(aarch64_plt1) +
                  sizeof(aarch64_plt0);
  m_Section.setSize(size);

  uint32_t offset = 0;
  SectionData::iterator frag, fragEnd = m_pSectionData->end();
  for (frag = m_pSectionData->begin(); frag != fragEnd; ++frag) {
    frag->setOffset(offset);
    offset += frag->size();
  }
}

AArch64PLT1* AArch64PLT::create() {
  AArch64PLT1* plt1_entry = new (std::nothrow) AArch64PLT1(*m_pSectionData);
  if (!plt1_entry)
    fatal(diag::fail_allocate_memory_plt);
  return plt1_entry;
}

void AArch64PLT::applyPLT0() {
  // malloc plt0
  iterator first = m_pSectionData->getFragmentList().begin();
  assert(first != m_pSectionData->getFragmentList().end() &&
         "FragmentList is empty, applyPLT0 failed!");
  AArch64PLT0* plt0 = &(llvm::cast<AArch64PLT0>(*first));
  uint32_t* data = NULL;
  data = static_cast<uint32_t*>(malloc(AArch64PLT0::EntrySize));
  if (data == NULL)
    fatal(diag::fail_allocate_memory_plt);
  memcpy(data, aarch64_plt0, AArch64PLT0::EntrySize);

  // apply plt0
  uint64_t plt_base = m_Section.addr();
  assert(plt_base && ".plt base address is NULL!");
  uint64_t got_base = m_GOT.addr();
  assert(got_base && ".got base address is NULL!");

  // apply 2nd instruction
  // get the address of got entry 2
  uint64_t got_ent2_base = got_base + sizeof(AArch64GOTEntry::EntrySize) * 2;
  // compute the immediate
  AArch64Relocator::DWord imm =
      helper_get_page_address(got_ent2_base) -
      helper_get_page_address(plt_base + (sizeof(AArch64PLT0::EntrySize) * 8));
  data[1] = helper_reencode_adr_imm(data[1], imm >> 12);
  // apply 3rd instruction
  data[2] = helper_reencode_add_imm(data[2],
                                    helper_get_page_offset(got_ent2_base) >> 3);
  // apply 4th instruction
  data[3] =
      helper_reencode_add_imm(data[3], helper_get_page_offset(got_ent2_base));
  plt0->setValue(reinterpret_cast<unsigned char*>(data));
}

void AArch64PLT::applyPLT1() {
  uint64_t plt_base = m_Section.addr();
  assert(plt_base && ".plt base address is NULL!");

  uint64_t got_base = m_GOT.addr();
  assert(got_base && ".got base address is NULL!");

  AArch64PLT::iterator it = m_pSectionData->begin();
  AArch64PLT::iterator ie = m_pSectionData->end();
  assert(it != ie && "FragmentList is empty, applyPLT1 failed!");

  uint32_t GOTEntrySize = AArch64GOTEntry::EntrySize;
  // first gotplt1 address
  uint32_t GOTEntryAddress = got_base + GOTEntrySize * 3;
  // first plt1 address
  uint32_t PLTEntryAddress = plt_base + AArch64PLT0::EntrySize;

  ++it;  // skip PLT0
  uint32_t PLT1EntrySize = AArch64PLT1::EntrySize;
  AArch64PLT1* plt1 = NULL;

  uint32_t* Out = NULL;
  while (it != ie) {
    plt1 = &(llvm::cast<AArch64PLT1>(*it));
    Out = static_cast<uint32_t*>(malloc(AArch64PLT1::EntrySize));
    memcpy(Out, aarch64_plt1, AArch64PLT1::EntrySize);
    // apply 1st instruction
    AArch64Relocator::DWord imm = helper_get_page_address(GOTEntryAddress) -
                                  helper_get_page_address(PLTEntryAddress);
    Out[0] = helper_reencode_adr_imm(Out[0], imm >> 12);
    // apply 2nd instruction
    Out[1] = helper_reencode_add_imm(
        Out[1], helper_get_page_offset(GOTEntryAddress) >> 3);
    // apply 3rd instruction
    Out[2] = helper_reencode_add_imm(Out[2],
                                     helper_get_page_offset(GOTEntryAddress));

    plt1->setValue(reinterpret_cast<unsigned char*>(Out));
    ++it;

    GOTEntryAddress += GOTEntrySize;
    PLTEntryAddress += PLT1EntrySize;
  }

  m_GOT.applyGOTPLT(plt_base);
}

uint64_t AArch64PLT::emit(MemoryRegion& pRegion) {
  uint64_t result = 0x0;
  iterator it = begin();

  unsigned char* buffer = pRegion.begin();
  memcpy(buffer,
         llvm::cast<AArch64PLT0>((*it)).getValue(),
         AArch64PLT0::EntrySize);
  result += AArch64PLT0::EntrySize;
  ++it;

  AArch64PLT1* plt1 = NULL;
  AArch64PLT::iterator ie = end();
  while (it != ie) {
    plt1 = &(llvm::cast<AArch64PLT1>(*it));
    memcpy(buffer + result, plt1->getValue(), AArch64PLT1::EntrySize);
    result += AArch64PLT1::EntrySize;
    ++it;
  }
  return result;
}

}  // namespace mcld
