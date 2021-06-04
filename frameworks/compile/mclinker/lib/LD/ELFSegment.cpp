//===- ELFSegment.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFSegment.h"

#include "mcld/Config/Config.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

#include <cassert>

namespace mcld {

typedef GCFactory<ELFSegment, MCLD_SEGMENTS_PER_OUTPUT> ELFSegmentFactory;
static llvm::ManagedStatic<ELFSegmentFactory> g_ELFSegmentFactory;

//===----------------------------------------------------------------------===//
// ELFSegment
//===----------------------------------------------------------------------===//
ELFSegment::ELFSegment()
    : m_Type(llvm::ELF::PT_NULL),
      m_Flag(llvm::ELF::PF_R),
      m_Offset(0x0),
      m_Vaddr(0x0),
      m_Paddr(0x0),
      m_Filesz(0x0),
      m_Memsz(0x0),
      m_Align(0x0),
      m_MaxSectionAlign(0x0) {
}

ELFSegment::ELFSegment(uint32_t pType, uint32_t pFlag)
    : m_Type(pType),
      m_Flag(pFlag),
      m_Offset(0x0),
      m_Vaddr(0x0),
      m_Paddr(0x0),
      m_Filesz(0x0),
      m_Memsz(0x0),
      m_Align(0x0),
      m_MaxSectionAlign(0x0) {
}

ELFSegment::~ELFSegment() {
}

bool ELFSegment::isLoadSegment() const {
  return type() == llvm::ELF::PT_LOAD;
}

bool ELFSegment::isDataSegment() const {
  return (type() == llvm::ELF::PT_LOAD) && ((flag() & llvm::ELF::PF_W) != 0x0);
}

bool ELFSegment::isBssSegment() const {
  if (!isDataSegment())
    return false;
  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    if ((*it)->kind() != LDFileFormat::BSS)
      return false;
  }
  return true;
}

ELFSegment::iterator ELFSegment::insert(ELFSegment::iterator pPos,
                                        LDSection* pSection) {
  return m_SectionList.insert(pPos, pSection);
}

void ELFSegment::append(LDSection* pSection) {
  assert(pSection != NULL);
  if (pSection->align() > m_MaxSectionAlign)
    m_MaxSectionAlign = pSection->align();
  m_SectionList.push_back(pSection);
}

ELFSegment* ELFSegment::Create(uint32_t pType, uint32_t pFlag) {
  ELFSegment* seg = g_ELFSegmentFactory->allocate();
  new (seg) ELFSegment(pType, pFlag);
  return seg;
}

void ELFSegment::Destroy(ELFSegment*& pSegment) {
  g_ELFSegmentFactory->destroy(pSegment);
  g_ELFSegmentFactory->deallocate(pSegment);
  pSegment = NULL;
}

void ELFSegment::Clear() {
  g_ELFSegmentFactory->clear();
}

}  // namespace mcld
