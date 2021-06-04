//===- RelocData.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/RelocData.h"

#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<RelocData, MCLD_SECTIONS_PER_INPUT> RelocDataFactory;

static llvm::ManagedStatic<RelocDataFactory> g_RelocDataFactory;

//===----------------------------------------------------------------------===//
// RelocData
//===----------------------------------------------------------------------===//
RelocData::RelocData() : m_pSection(NULL) {
}

RelocData::RelocData(LDSection& pSection) : m_pSection(&pSection) {
}

RelocData* RelocData::Create(LDSection& pSection) {
  RelocData* result = g_RelocDataFactory->allocate();
  new (result) RelocData(pSection);
  return result;
}

void RelocData::Destroy(RelocData*& pSection) {
  pSection->~RelocData();
  g_RelocDataFactory->deallocate(pSection);
  pSection = NULL;
}

void RelocData::Clear() {
  g_RelocDataFactory->clear();
}

RelocData& RelocData::append(Relocation& pRelocation) {
  m_Relocations.push_back(&pRelocation);
  return *this;
}

Relocation& RelocData::remove(Relocation& pRelocation) {
  iterator iter(pRelocation);
  Relocation* rel = m_Relocations.remove(iter);
  return *rel;
}

}  // namespace mcld
