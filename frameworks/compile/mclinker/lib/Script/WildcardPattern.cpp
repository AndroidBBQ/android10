//===- WildcardPattern.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/WildcardPattern.h"

#include "mcld/Support/GCFactory.h"
#include "mcld/Support/raw_ostream.h"

#include <llvm/Support/ManagedStatic.h>
#include <cassert>

namespace mcld {

typedef GCFactory<WildcardPattern, MCLD_SYMBOLS_PER_INPUT>
    WildcardPatternFactory;
static llvm::ManagedStatic<WildcardPatternFactory> g_WildcardPatternFactory;

//===----------------------------------------------------------------------===//
// WildcardPattern
//===----------------------------------------------------------------------===//
WildcardPattern::WildcardPattern() : m_bIsPrefix(false) {
}

WildcardPattern::WildcardPattern(const std::string& pPattern,
                                 SortPolicy pPolicy)
    : StrToken(StrToken::Wildcard, pPattern), m_SortPolicy(pPolicy) {
  if (pPattern.find_first_of('*') == (pPattern.size() - 1))
    m_bIsPrefix = true;
  else
    m_bIsPrefix = false;
}

WildcardPattern::~WildcardPattern() {
}

llvm::StringRef WildcardPattern::prefix() const {
  if (isPrefix())
    return llvm::StringRef(name().c_str(), name().size() - 1);

  return llvm::StringRef(name());
}

WildcardPattern* WildcardPattern::create(const std::string& pPattern,
                                         SortPolicy pPolicy) {
  WildcardPattern* result = g_WildcardPatternFactory->allocate();
  new (result) WildcardPattern(pPattern, pPolicy);
  return result;
}

void WildcardPattern::destroy(WildcardPattern*& pWildcardPattern) {
  g_WildcardPatternFactory->destroy(pWildcardPattern);
  g_WildcardPatternFactory->deallocate(pWildcardPattern);
  pWildcardPattern = NULL;
}

void WildcardPattern::clear() {
  g_WildcardPatternFactory->clear();
}

}  // namespace mcld
