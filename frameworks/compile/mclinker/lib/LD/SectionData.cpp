//===- SectionData.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/SectionData.h"

#include "mcld/LD/LDSection.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<SectionData, MCLD_SECTIONS_PER_INPUT> SectDataFactory;

static llvm::ManagedStatic<SectDataFactory> g_SectDataFactory;

//===----------------------------------------------------------------------===//
// SectionData
//===----------------------------------------------------------------------===//
SectionData::SectionData() : m_pSection(NULL) {
}

SectionData::SectionData(LDSection& pSection) : m_pSection(&pSection) {
}

SectionData* SectionData::Create(LDSection& pSection) {
  SectionData* result = g_SectDataFactory->allocate();
  new (result) SectionData(pSection);
  return result;
}

void SectionData::Destroy(SectionData*& pSection) {
  pSection->~SectionData();
  g_SectDataFactory->deallocate(pSection);
  pSection = NULL;
}

void SectionData::Clear() {
  g_SectDataFactory->clear();
}

}  // namespace mcld
