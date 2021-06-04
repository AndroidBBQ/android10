//===- LDSection.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/LDSection.h"

#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<LDSection, MCLD_SECTIONS_PER_INPUT> SectionFactory;

static llvm::ManagedStatic<SectionFactory> g_SectFactory;

//===----------------------------------------------------------------------===//
// LDSection
//===----------------------------------------------------------------------===//
LDSection::LDSection()
    : m_Name(),
      m_Kind(LDFileFormat::Ignore),
      m_Type(0x0),
      m_Flag(0x0),
      m_Size(0),
      m_Offset(~uint64_t(0)),
      m_Addr(0x0),
      m_Align(0),
      m_Info(0),
      m_pLink(NULL),
      m_Index(0) {
  m_Data.sect_data = NULL;
}

LDSection::LDSection(const std::string& pName,
                     LDFileFormat::Kind pKind,
                     uint32_t pType,
                     uint32_t pFlag,
                     uint64_t pSize,
                     uint64_t pAddr)
    : m_Name(pName),
      m_Kind(pKind),
      m_Type(pType),
      m_Flag(pFlag),
      m_Size(pSize),
      m_Offset(~uint64_t(0)),
      m_Addr(pAddr),
      m_Align(0),
      m_Info(0),
      m_pLink(NULL),
      m_Index(0) {
  m_Data.sect_data = NULL;
}

LDSection::~LDSection() {
}

bool LDSection::hasOffset() const {
  return (m_Offset != ~uint64_t(0));
}

LDSection* LDSection::Create(const std::string& pName,
                             LDFileFormat::Kind pKind,
                             uint32_t pType,
                             uint32_t pFlag,
                             uint64_t pSize,
                             uint64_t pAddr) {
  LDSection* result = g_SectFactory->allocate();
  new (result) LDSection(pName, pKind, pType, pFlag, pSize, pAddr);
  return result;
}

void LDSection::Destroy(LDSection*& pSection) {
  g_SectFactory->destroy(pSection);
  g_SectFactory->deallocate(pSection);
  pSection = NULL;
}

void LDSection::Clear() {
  g_SectFactory->clear();
}

bool LDSection::hasSectionData() const {
  assert(LDFileFormat::Relocation != kind() && LDFileFormat::EhFrame != kind());
  return (m_Data.sect_data != NULL);
}

bool LDSection::hasRelocData() const {
  assert(LDFileFormat::Relocation == kind());
  return (m_Data.reloc_data != NULL);
}

bool LDSection::hasEhFrame() const {
  assert(LDFileFormat::EhFrame == kind());
  return (m_Data.eh_frame != NULL);
}

bool LDSection::hasDebugString() const {
  assert(LDFileFormat::DebugString == kind());
  return (NULL != m_Data.debug_string);
}

}  // namespace mcld
