//===- StrToken.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/StrToken.h"

#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<StrToken, MCLD_SYMBOLS_PER_INPUT> StrTokenFactory;
static llvm::ManagedStatic<StrTokenFactory> g_StrTokenFactory;

//===----------------------------------------------------------------------===//
// StrToken
//===----------------------------------------------------------------------===//
StrToken::StrToken() : m_Kind(Unknown) {
}

StrToken::StrToken(Kind pKind, const std::string& pString)
    : m_Kind(pKind), m_Name(pString) {
}

StrToken::~StrToken() {
}

StrToken* StrToken::create(const std::string& pString) {
  StrToken* result = g_StrTokenFactory->allocate();
  new (result) StrToken(String, pString);
  return result;
}

void StrToken::destroy(StrToken*& pStrToken) {
  g_StrTokenFactory->destroy(pStrToken);
  g_StrTokenFactory->deallocate(pStrToken);
  pStrToken = NULL;
}

void StrToken::clear() {
  g_StrTokenFactory->clear();
}

}  // namespace mcld
