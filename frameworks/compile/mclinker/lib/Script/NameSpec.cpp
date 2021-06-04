//===- NameSpec.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/NameSpec.h"

#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<NameSpec, MCLD_SYMBOLS_PER_INPUT> NameSpecFactory;
static llvm::ManagedStatic<NameSpecFactory> g_NameSpecFactory;

//===----------------------------------------------------------------------===//
// NameSpec
//===----------------------------------------------------------------------===//
NameSpec::NameSpec() {
}

NameSpec::NameSpec(const std::string& pName, bool pAsNeeded)
    : InputToken(InputToken::NameSpec, pName, pAsNeeded) {
}

NameSpec::~NameSpec() {
}

NameSpec* NameSpec::create(const std::string& pName, bool pAsNeeded) {
  NameSpec* result = g_NameSpecFactory->allocate();
  new (result) NameSpec(pName, pAsNeeded);
  return result;
}

void NameSpec::destroy(NameSpec*& pNameSpec) {
  g_NameSpecFactory->destroy(pNameSpec);
  g_NameSpecFactory->deallocate(pNameSpec);
  pNameSpec = NULL;
}

void NameSpec::clear() {
  g_NameSpecFactory->clear();
}

}  // namespace mcld
