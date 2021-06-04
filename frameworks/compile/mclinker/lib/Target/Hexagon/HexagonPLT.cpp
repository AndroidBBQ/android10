//===- HexagonPLT.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "HexagonPLT.h"
#include "HexagonRelocationFunctions.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/ELF.h>
#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// PLT entry data
//===----------------------------------------------------------------------===//
HexagonPLT0::HexagonPLT0(SectionData& pParent)
    : PLT::Entry<sizeof(hexagon_plt0)>(pParent) {
}

HexagonPLT1::HexagonPLT1(SectionData& pParent)
    : PLT::Entry<sizeof(hexagon_plt1)>(pParent) {
}

//===----------------------------------------------------------------------===//
// HexagonPLT
//===----------------------------------------------------------------------===//
HexagonPLT::HexagonPLT(LDSection& pSection,
                       HexagonGOTPLT& pGOTPLT,
                       const LinkerConfig& pConfig)
    : PLT(pSection), m_GOTPLT(pGOTPLT) {
  assert(LinkerConfig::DynObj == pConfig.codeGenType() ||
         LinkerConfig::Exec == pConfig.codeGenType() ||
         LinkerConfig::Binary == pConfig.codeGenType());

  m_PLT0 = hexagon_plt0;
  m_PLT0Size = sizeof(hexagon_plt0);
  // create PLT0
  new HexagonPLT0(*m_pSectionData);
  pSection.setAlign(16);
}

HexagonPLT::~HexagonPLT() {
}

PLTEntryBase* HexagonPLT::getPLT0() const {
  iterator first = m_pSectionData->getFragmentList().begin();

  assert(first != m_pSectionData->getFragmentList().end() &&
         "FragmentList is empty, getPLT0 failed!");

  PLTEntryBase* plt0 = &(llvm::cast<PLTEntryBase>(*first));

  return plt0;
}

void HexagonPLT::finalizeSectionSize() {
  uint64_t size = 0;
  // plt0 size
  size = getPLT0()->size();

  // get first plt1 entry
  HexagonPLT::iterator it = begin();
  ++it;
  if (end() != it) {
    // plt1 size
    PLTEntryBase* plt1 = &(llvm::cast<PLTEntryBase>(*it));
    size += (m_pSectionData->size() - 1) * plt1->size();
  }
  m_Section.setSize(size);

  uint32_t offset = 0;
  SectionData::iterator frag, fragEnd = m_pSectionData->end();
  for (frag = m_pSectionData->begin(); frag != fragEnd; ++frag) {
    frag->setOffset(offset);
    offset += frag->size();
  }
}

bool HexagonPLT::hasPLT1() const {
  return (m_pSectionData->size() > 1);
}

HexagonPLT1* HexagonPLT::create() {
  return new HexagonPLT1(*m_pSectionData);
}

void HexagonPLT::applyPLT0() {
  PLTEntryBase* plt0 = getPLT0();
  uint64_t pltBase = m_Section.addr();

  unsigned char* data = 0;
  data = static_cast<unsigned char*>(malloc(plt0->size()));

  if (!data)
    fatal(diag::fail_allocate_memory_plt);

  memcpy(data, m_PLT0, plt0->size());
  uint32_t gotpltAddr = m_GOTPLT.addr();

  int32_t* dest = reinterpret_cast<int32_t*>(data);
  int32_t result = ((gotpltAddr - pltBase) >> 6);
  *dest |= ApplyMask<int32_t>(0xfff3fff, result);
  dest = dest + 1;
  // Already calculated using pltBase
  result = (gotpltAddr - pltBase);
  *(dest) |= ApplyMask<int32_t>(0x1f80, result);

  plt0->setValue(data);
}

void HexagonPLT::applyPLT1() {
  uint64_t plt_base = m_Section.addr();
  assert(plt_base && ".plt base address is NULL!");

  uint64_t got_base = m_GOTPLT.addr();
  assert(got_base && ".got base address is NULL!");

  HexagonPLT::iterator it = m_pSectionData->begin();
  HexagonPLT::iterator ie = m_pSectionData->end();
  assert(it != ie && "FragmentList is empty, applyPLT1 failed!");

  uint32_t GOTEntrySize = HexagonGOTEntry::EntrySize;
  uint32_t GOTEntryAddress = got_base + GOTEntrySize * 4;

  uint64_t PLTEntryAddress =
      plt_base + HexagonPLT0::EntrySize;  // Offset of PLT0

  ++it;  // skip PLT0
  uint64_t PLT1EntrySize = HexagonPLT1::EntrySize;
  HexagonPLT1* plt1 = NULL;

  uint32_t* Out = NULL;
  while (it != ie) {
    plt1 = &(llvm::cast<HexagonPLT1>(*it));
    Out = static_cast<uint32_t*>(malloc(HexagonPLT1::EntrySize));

    if (!Out)
      fatal(diag::fail_allocate_memory_plt);

    memcpy(Out, hexagon_plt1, plt1->size());

    int32_t* dest = reinterpret_cast<int32_t*>(Out);
    int32_t result = ((GOTEntryAddress - PLTEntryAddress) >> 6);
    *dest |= ApplyMask<int32_t>(0xfff3fff, result);
    dest = dest + 1;
    result = (GOTEntryAddress - PLTEntryAddress);
    *(dest) |= ApplyMask<int32_t>(0x1f80, result);

    // Address in the PLT entries point to the corresponding GOT entries
    // TODO: Fixup plt to point to the corresponding GOTEntryAddress
    // We need to borrow the same relocation code to fix the relocation
    plt1->setValue(reinterpret_cast<unsigned char*>(Out));
    ++it;

    GOTEntryAddress += GOTEntrySize;
    PLTEntryAddress += PLT1EntrySize;
  }
}

uint64_t HexagonPLT::emit(MemoryRegion& pRegion) {
  uint64_t result = 0x0;
  iterator it = begin();

  unsigned char* buffer = pRegion.begin();
  memcpy(buffer,
         llvm::cast<HexagonPLT0>((*it)).getValue(),
         HexagonPLT0::EntrySize);
  result += HexagonPLT0::EntrySize;
  ++it;

  HexagonPLT1* plt1 = 0;
  HexagonPLT::iterator ie = end();
  while (it != ie) {
    plt1 = &(llvm::cast<HexagonPLT1>(*it));
    memcpy(buffer + result, plt1->getValue(), HexagonPLT1::EntrySize);
    result += HexagonPLT1::EntrySize;
    ++it;
  }
  return result;
}

}  // namespace mcld
