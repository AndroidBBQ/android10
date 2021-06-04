//===- AArch64GOT.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64GOT.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>

namespace {
const unsigned int AArch64GOT0Num = 3;
}  // end of anonymous namespace

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64GOT
AArch64GOT::AArch64GOT(LDSection& pSection)
    : GOT(pSection), m_pGOTPLTFront(NULL), m_pGOTFront(NULL) {
}

AArch64GOT::~AArch64GOT() {
}

void AArch64GOT::createGOT0() {
  // create GOT0, and put them into m_SectionData immediately
  for (unsigned int i = 0; i < AArch64GOT0Num; ++i)
    new AArch64GOTEntry(0, m_SectionData);
}

bool AArch64GOT::hasGOT1() const {
  return ((!m_GOT.empty()) || (!m_GOTPLT.empty()));
}

AArch64GOTEntry* AArch64GOT::createGOT() {
  AArch64GOTEntry* entry = new AArch64GOTEntry(0, NULL);
  m_GOT.push_back(entry);
  return entry;
}

AArch64GOTEntry* AArch64GOT::createGOTPLT() {
  AArch64GOTEntry* entry = new AArch64GOTEntry(0, NULL);
  m_GOTPLT.push_back(entry);
  return entry;
}

void AArch64GOT::finalizeSectionSize() {
  uint32_t offset = 0;
  SectionData::FragmentListType& frag_list = m_SectionData->getFragmentList();
  // setup GOT0 offset
  SectionData::iterator frag, fragEnd = m_SectionData->end();
  for (frag = m_SectionData->begin(); frag != fragEnd; ++frag) {
    frag->setOffset(offset);
    offset += frag->size();
  }

  // push GOTPLT into the SectionData and setup the offset
  if (!m_GOTPLT.empty()) {
    m_pGOTPLTFront = m_GOTPLT.front();
    entry_iterator it, end = m_GOTPLT.end();
    for (it = m_GOTPLT.begin(); it != end; ++it) {
      AArch64GOTEntry* entry = *it;
      frag_list.push_back(entry);
      entry->setParent(m_SectionData);
      entry->setOffset(offset);
      offset += entry->size();
    }
  }
  m_GOTPLT.clear();

  // push GOT into the SectionData and setup the offset
  if (!m_GOT.empty()) {
    m_pGOTFront = m_GOT.front();
    entry_iterator it, end = m_GOT.end();
    for (it = m_GOT.begin(); it != end; ++it) {
      AArch64GOTEntry* entry = *it;
      frag_list.push_back(entry);
      entry->setParent(m_SectionData);
      entry->setOffset(offset);
      offset += entry->size();
    }
  }
  m_GOT.clear();

  // set section size
  m_Section.setSize(offset);
}

void AArch64GOT::applyGOT0(uint64_t pAddress) {
  llvm::cast<AArch64GOTEntry>(*(m_SectionData->getFragmentList().begin()))
      .setValue(pAddress);
}

void AArch64GOT::applyGOTPLT(uint64_t pPLTBase) {
  if (m_pGOTPLTFront == NULL)
    return;

  SectionData::iterator entry(m_pGOTPLTFront);
  SectionData::iterator e_end;
  if (m_pGOTFront == NULL)
    e_end = m_SectionData->end();
  else
    e_end = SectionData::iterator(m_pGOTFront);

  while (entry != e_end) {
    llvm::cast<AArch64GOTEntry>(entry)->setValue(pPLTBase);
    ++entry;
  }
}

uint64_t AArch64GOT::emit(MemoryRegion& pRegion) {
  uint64_t* buffer = reinterpret_cast<uint64_t*>(pRegion.begin());

  AArch64GOTEntry* got = NULL;
  uint64_t result = 0x0;
  for (iterator it = begin(), ie = end(); it != ie; ++it, ++buffer) {
    got = &(llvm::cast<AArch64GOTEntry>((*it)));
    *buffer = static_cast<uint64_t>(got->getValue());
    result += AArch64GOTEntry::EntrySize;
  }
  return result;
}

}  // namespace mcld
