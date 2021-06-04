//===- ARMGOT.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ARMGOT.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>

namespace {
const unsigned int ARMGOT0Num = 3;
}  // end of anonymous namespace

namespace mcld {

//===----------------------------------------------------------------------===//
// ARMGOT
ARMGOT::ARMGOT(LDSection& pSection)
    : GOT(pSection), m_pGOTPLTFront(NULL), m_pGOTFront(NULL) {
  // create GOT0, and put them into m_SectionData immediately
  for (unsigned int i = 0; i < ARMGOT0Num; ++i)
    new ARMGOTEntry(0, m_SectionData);
}

ARMGOT::~ARMGOT() {
}

bool ARMGOT::hasGOT1() const {
  return ((!m_GOT.empty()) || (!m_GOTPLT.empty()));
}

ARMGOTEntry* ARMGOT::createGOT() {
  ARMGOTEntry* entry = new ARMGOTEntry(0, NULL);
  m_GOT.push_back(entry);
  return entry;
}

ARMGOTEntry* ARMGOT::createGOTPLT() {
  ARMGOTEntry* entry = new ARMGOTEntry(0, NULL);
  m_GOTPLT.push_back(entry);
  return entry;
}

void ARMGOT::finalizeSectionSize() {
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
      ARMGOTEntry* entry = *it;
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
      ARMGOTEntry* entry = *it;
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

void ARMGOT::applyGOT0(uint64_t pAddress) {
  llvm::cast<ARMGOTEntry>(*(m_SectionData->getFragmentList().begin()))
      .setValue(pAddress);
}

void ARMGOT::applyGOTPLT(uint64_t pPLTBase) {
  if (m_pGOTPLTFront == NULL)
    return;

  SectionData::iterator entry(m_pGOTPLTFront);
  SectionData::iterator e_end;
  if (m_pGOTFront == NULL)
    e_end = m_SectionData->end();
  else
    e_end = SectionData::iterator(m_pGOTFront);

  while (entry != e_end) {
    llvm::cast<ARMGOTEntry>(entry)->setValue(pPLTBase);
    ++entry;
  }
}

uint64_t ARMGOT::emit(MemoryRegion& pRegion) {
  uint32_t* buffer = reinterpret_cast<uint32_t*>(pRegion.begin());

  ARMGOTEntry* got = NULL;
  uint64_t result = 0x0;
  for (iterator it = begin(), ie = end(); it != ie; ++it, ++buffer) {
    got = &(llvm::cast<ARMGOTEntry>((*it)));
    *buffer = static_cast<uint32_t>(got->getValue());
    result += ARMGOTEntry::EntrySize;
  }
  return result;
}

}  // namespace mcld
