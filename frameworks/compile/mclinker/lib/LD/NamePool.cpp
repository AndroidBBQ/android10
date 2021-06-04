//===- NamePool.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/NamePool.h"

#include "mcld/LD/StaticResolver.h"

#include <llvm/Support/raw_ostream.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// NamePool
//===----------------------------------------------------------------------===//
NamePool::NamePool(NamePool::size_type pSize)
    : m_pResolver(new StaticResolver()), m_Table(pSize) {
}

NamePool::~NamePool() {
  delete m_pResolver;

  FreeInfoSet::iterator info, iEnd = m_FreeInfoSet.end();
  for (info = m_FreeInfoSet.begin(); info != iEnd; ++info) {
    ResolveInfo::Destroy(*info);
  }
}

/// createSymbol - create a symbol
ResolveInfo* NamePool::createSymbol(const llvm::StringRef& pName,
                                    bool pIsDyn,
                                    ResolveInfo::Type pType,
                                    ResolveInfo::Desc pDesc,
                                    ResolveInfo::Binding pBinding,
                                    ResolveInfo::SizeType pSize,
                                    ResolveInfo::Visibility pVisibility) {
  ResolveInfo** result = m_FreeInfoSet.allocate();
  (*result) = ResolveInfo::Create(pName);
  (*result)->setIsSymbol(true);
  (*result)->setSource(pIsDyn);
  (*result)->setType(pType);
  (*result)->setDesc(pDesc);
  (*result)->setBinding(pBinding);
  (*result)->setVisibility(pVisibility);
  (*result)->setSize(pSize);
  return *result;
}

/// insertSymbol - insert a symbol and resolve it immediately
/// @return the pointer of resolved ResolveInfo
/// @return is the symbol existent?
void NamePool::insertSymbol(const llvm::StringRef& pName,
                            bool pIsDyn,
                            ResolveInfo::Type pType,
                            ResolveInfo::Desc pDesc,
                            ResolveInfo::Binding pBinding,
                            ResolveInfo::SizeType pSize,
                            LDSymbol::ValueType pValue,
                            ResolveInfo::Visibility pVisibility,
                            ResolveInfo* pOldInfo,
                            Resolver::Result& pResult) {
  // We should check if there is any symbol with the same name existed.
  // If it already exists, we should use resolver to decide which symbol
  // should be reserved. Otherwise, we insert the symbol and set up its
  // attributes.
  bool exist = false;
  ResolveInfo* old_symbol = m_Table.insert(pName, exist);
  ResolveInfo* new_symbol = NULL;
  if (exist && old_symbol->isSymbol()) {
    new_symbol = m_Table.getEntryFactory().produce(pName);
  } else {
    exist = false;
    new_symbol = old_symbol;
  }

  new_symbol->setIsSymbol(true);
  new_symbol->setSource(pIsDyn);
  new_symbol->setType(pType);
  new_symbol->setDesc(pDesc);
  new_symbol->setBinding(pBinding);
  new_symbol->setVisibility(pVisibility);
  new_symbol->setSize(pSize);

  if (!exist) {
    // old_symbol is neither existed nor a symbol.
    pResult.info = new_symbol;
    pResult.existent = false;
    pResult.overriden = true;
    return;
  } else if (pOldInfo != NULL) {
    // existent, remember its attribute
    pOldInfo->override(*old_symbol);
  }

  // exist and is a symbol
  // symbol resolution
  bool override = false;
  unsigned int action = Resolver::LastAction;
  if (m_pResolver->resolve(*old_symbol, *new_symbol, override, pValue)) {
    pResult.info = old_symbol;
    pResult.existent = true;
    pResult.overriden = override;
  } else {
    m_pResolver->resolveAgain(*this, action, *old_symbol, *new_symbol, pResult);
  }

  m_Table.getEntryFactory().destroy(new_symbol);
  return;
}

llvm::StringRef NamePool::insertString(const llvm::StringRef& pString) {
  bool exist = false;
  ResolveInfo* resolve_info = m_Table.insert(pString, exist);
  return llvm::StringRef(resolve_info->name(), resolve_info->nameSize());
}

void NamePool::reserve(NamePool::size_type pSize) {
  m_Table.rehash(pSize);
}

NamePool::size_type NamePool::capacity() const {
  return (m_Table.numOfBuckets() - m_Table.numOfEntries());
}

/// findInfo - find the resolved ResolveInfo
ResolveInfo* NamePool::findInfo(const llvm::StringRef& pName) {
  Table::iterator iter = m_Table.find(pName);
  return iter.getEntry();
}

/// findInfo - find the resolved ResolveInfo
const ResolveInfo* NamePool::findInfo(const llvm::StringRef& pName) const {
  Table::const_iterator iter = m_Table.find(pName);
  return iter.getEntry();
}

/// findSymbol - find the resolved output LDSymbol
LDSymbol* NamePool::findSymbol(const llvm::StringRef& pName) {
  ResolveInfo* info = findInfo(pName);
  if (info == NULL)
    return NULL;
  return info->outSymbol();
}

/// findSymbol - find the resolved output LDSymbol
const LDSymbol* NamePool::findSymbol(const llvm::StringRef& pName) const {
  const ResolveInfo* info = findInfo(pName);
  if (info == NULL)
    return NULL;
  return info->outSymbol();
}

}  // namespace mcld
