//===- SectionSymbolSet.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/SectionSymbolSet.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/NamePool.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// SectionSymbolSet
//===----------------------------------------------------------------------===//

SectionSymbolSet::SectionSymbolSet() {
  m_pSectionSymbolMap = new SectHashTableType(16);
}

SectionSymbolSet::~SectionSymbolSet() {
  delete m_pSectionSymbolMap;
}

bool SectionSymbolSet::add(LDSection& pOutSect, NamePool& pNamePool) {
  // create the resolveInfo for this section symbol
  llvm::StringRef sym_name = llvm::StringRef(pOutSect.name());
  ResolveInfo* sym_info = pNamePool.createSymbol(sym_name,
                                                 false,
                                                 ResolveInfo::Section,
                                                 ResolveInfo::Define,
                                                 ResolveInfo::Local,
                                                 0x0,  // size
                                                 ResolveInfo::Default);

  // create the output section symbol and set its fragRef to the first fragment
  // of the section
  LDSymbol* sym = LDSymbol::Create(*sym_info);
  sym_info->setSymPtr(sym);

  // insert the symbol to the Section to Symbol hash map
  bool exist = false;
  SectHashTableType::entry_type* entry =
      m_pSectionSymbolMap->insert(&pOutSect, exist);
  assert(!exist);
  entry->setValue(sym);

  return true;
}

bool SectionSymbolSet::finalize(LDSection& pOutSect,
                                SymbolTable& pSymTab,
                                bool relocatable) {
  if (!relocatable && pOutSect.size() == 0)
    return true;

  LDSymbol* sym = get(pOutSect);
  assert(sym != NULL);
  SectionData* data = NULL;
  switch (pOutSect.kind()) {
    case LDFileFormat::Relocation:
      // Relocation section should not have section symbol.
      return true;

    case LDFileFormat::EhFrame:
      if (EhFrame* ehframe = pOutSect.getEhFrame())
        data = ehframe->getSectionData();
      break;

    default:
      data = pOutSect.getSectionData();
      break;
  }
  FragmentRef* frag_ref;
  if (data && !data->empty())
    frag_ref = FragmentRef::Create(data->front(), 0x0);
  else
    frag_ref = FragmentRef::Null();
  sym->setFragmentRef(frag_ref);
  // push symbol into output symbol table
  pSymTab.add(*sym);

  return true;
}

LDSymbol* SectionSymbolSet::get(const LDSection& pOutSect) {
  SectHashTableType::iterator entry = m_pSectionSymbolMap->find(&pOutSect);
  return entry.getEntry()->value();
}

const LDSymbol* SectionSymbolSet::get(const LDSection& pOutSect) const {
  SectHashTableType::iterator entry = m_pSectionSymbolMap->find(&pOutSect);
  return entry.getEntry()->value();
}

}  // namespace mcld
