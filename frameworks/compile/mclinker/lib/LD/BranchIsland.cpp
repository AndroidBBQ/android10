//===- BranchIsland.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/BranchIsland.h"

#include "mcld/Fragment/AlignFragment.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/ResolveInfo.h"

#include <sstream>

namespace mcld {

//============================================================================//
// BranchIsland
//============================================================================//
BranchIsland::BranchIsland(Fragment& pEntryFrag, size_t pMaxSize, size_t pIndex)
    : m_Entry(pEntryFrag),
      m_pExit(pEntryFrag.getNextNode()),
      m_pRear(NULL),
      m_MaxSize(pMaxSize),
      m_Name("island-") {
  // island name
  std::ostringstream index;
  index << pIndex;
  m_Name.append(index.str());
}

BranchIsland::~BranchIsland() {
}

/// fragment iterators of the island
SectionData::iterator BranchIsland::begin() {
  return ++iterator(&m_Entry);
}

SectionData::const_iterator BranchIsland::begin() const {
  return ++iterator(&m_Entry);
}

SectionData::iterator BranchIsland::end() {
  if (m_pExit != NULL)
    return iterator(m_pExit);
  return m_Entry.getParent()->end();
}

SectionData::const_iterator BranchIsland::end() const {
  if (m_pExit != NULL)
    return iterator(m_pExit);
  return m_Entry.getParent()->end();
}

uint64_t BranchIsland::offset() const {
  return m_Entry.getOffset() + m_Entry.size();
}

size_t BranchIsland::size() const {
  size_t size = 0x0;
  if (numOfStubs() != 0x0) {
    size = m_pRear->getOffset() + m_pRear->size() -
           m_Entry.getNextNode()->getOffset();
  }
  return size;
}

size_t BranchIsland::maxSize() const {
  return m_MaxSize;
}

const std::string& BranchIsland::name() const {
  return m_Name;
}

size_t BranchIsland::numOfStubs() const {
  return m_StubMap.numOfEntries();
}

/// findStub - return true if there is a stub built from the given prototype
///            for the given relocation
Stub* BranchIsland::findStub(const Stub* pPrototype, const Relocation& pReloc) {
  Key key(pPrototype, pReloc.symInfo()->outSymbol(), pReloc.addend());
  StubMapType::iterator it = m_StubMap.find(key);
  if (it != m_StubMap.end()) {
    assert(it.getEntry()->value() != NULL);
    return it.getEntry()->value();
  }
  return NULL;
}

/// addStub - add a stub into the island
bool BranchIsland::addStub(const Stub* pPrototype,
                           const Relocation& pReloc,
                           Stub& pStub) {
  bool exist = false;
  Key key(pPrototype, pReloc.symInfo()->outSymbol(), pReloc.addend());
  StubEntryType* entry = m_StubMap.insert(key, exist);
  if (!exist) {
    entry->setValue(&pStub);
    m_pRear = &pStub;
    SectionData* sd = m_Entry.getParent();

    // insert alignment fragment
    // TODO: check if we can reduce this alignment fragment for some cases
    AlignFragment* align_frag =
        new AlignFragment(pStub.alignment(), 0x0, 1u, pStub.alignment() - 1);
    align_frag->setParent(sd);
    sd->getFragmentList().insert(end(), align_frag);
    align_frag->setOffset(align_frag->getPrevNode()->getOffset() +
                          align_frag->getPrevNode()->size());

    // insert stub fragment
    pStub.setParent(sd);
    sd->getFragmentList().insert(end(), &pStub);
    pStub.setOffset(pStub.getPrevNode()->getOffset() +
                    pStub.getPrevNode()->size());
  }
  return !exist;
}

void BranchIsland::addStub(Stub& pStub) {
  bool exist = false;
  Key key(&pStub, pStub.symInfo()->outSymbol(), 0);
  m_StubMap.insert(key, exist);

  m_pRear = &pStub;
  SectionData* sd = m_Entry.getParent();

  // insert alignment fragment
  // TODO: check if we can reduce this alignment fragment for some cases
  AlignFragment* align_frag =
      new AlignFragment(pStub.alignment(), 0x0, 1u, pStub.alignment() - 1);
  align_frag->setParent(sd);
  sd->getFragmentList().insert(end(), align_frag);
  align_frag->setOffset(align_frag->getPrevNode()->getOffset() +
                        align_frag->getPrevNode()->size());

  // insert stub fragment
  pStub.setParent(sd);
  sd->getFragmentList().insert(end(), &pStub);
  pStub.setOffset(pStub.getPrevNode()->getOffset() +
                  pStub.getPrevNode()->size());
}

/// addRelocation - add a relocation into island
bool BranchIsland::addRelocation(Relocation& pReloc) {
  m_Relocations.push_back(&pReloc);
  return true;
}

}  // namespace mcld
