//===- StringList.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/StringList.h"

#include "mcld/Script/StrToken.h"
#include "mcld/Support/GCFactory.h"
#include "mcld/Support/raw_ostream.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<StringList, MCLD_SYMBOLS_PER_INPUT> StringListFactory;
static llvm::ManagedStatic<StringListFactory> g_StringListFactory;

//===----------------------------------------------------------------------===//
// StringList
//===----------------------------------------------------------------------===//
StringList::StringList() {
}

StringList::~StringList() {
}

void StringList::push_back(StrToken* pToken) {
  m_Tokens.push_back(pToken);
}

void StringList::dump() const {
  for (const_iterator it = begin(), ie = end(); it != ie; ++it)
    mcld::outs() << (*it)->name() << "\t";
  mcld::outs() << "\n";
}

StringList* StringList::create() {
  StringList* result = g_StringListFactory->allocate();
  new (result) StringList();
  return result;
}

void StringList::destroy(StringList*& pStringList) {
  g_StringListFactory->destroy(pStringList);
  g_StringListFactory->deallocate(pStringList);
  pStringList = NULL;
}

void StringList::clear() {
  g_StringListFactory->clear();
}

}  // namespace mcld
