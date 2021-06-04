//===- Module.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Module.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/NamePool.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/LD/StaticResolver.h"

namespace mcld {

static GCFactory<Module::AliasList, MCLD_SECTIONS_PER_INPUT>
    gc_aliaslist_factory;

//===----------------------------------------------------------------------===//
// Module
//===----------------------------------------------------------------------===//
Module::Module(LinkerScript& pScript) : m_Script(pScript), m_NamePool(1024) {
}

Module::Module(const std::string& pName, LinkerScript& pScript)
    : m_Name(pName), m_Script(pScript), m_NamePool(1024) {
}

Module::~Module() {
}

// Following two functions will be obsolette when we have new section merger.
LDSection* Module::getSection(const std::string& pName) {
  iterator sect, sectEnd = end();
  for (sect = begin(); sect != sectEnd; ++sect) {
    if ((*sect)->name() == pName)
      return *sect;
  }
  return NULL;
}

const LDSection* Module::getSection(const std::string& pName) const {
  const_iterator sect, sectEnd = end();
  for (sect = begin(); sect != sectEnd; ++sect) {
    if ((*sect)->name() == pName)
      return *sect;
  }
  return NULL;
}

void Module::CreateAliasList(const ResolveInfo& pSym) {
  AliasList* result = gc_aliaslist_factory.allocate();
  new (result) AliasList();
  m_AliasLists.push_back(result);
  result->push_back(&pSym);
}

void Module::addAlias(const ResolveInfo& pAlias) {
  assert(m_AliasLists.size() != 0);
  uint32_t last_pos = m_AliasLists.size() - 1;
  m_AliasLists[last_pos]->push_back(&pAlias);
}

Module::AliasList* Module::getAliasList(const ResolveInfo& pSym) {
  std::vector<AliasList*>::iterator list_it, list_it_e = m_AliasLists.end();
  for (list_it = m_AliasLists.begin(); list_it != list_it_e; ++list_it) {
    AliasList& list = **list_it;
    alias_iterator alias_it, alias_it_e = list.end();
    for (alias_it = list.begin(); alias_it != alias_it_e; ++alias_it) {
      if (strcmp((*alias_it)->name(), pSym.name()) == 0) {
        return &list;
      }
    }
  }
  return NULL;
}

}  // namespace mcld
