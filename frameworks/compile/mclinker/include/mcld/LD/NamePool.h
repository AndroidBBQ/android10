//===- NamePool.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_NAMEPOOL_H_
#define MCLD_LD_NAMEPOOL_H_

#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/Config/Config.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/Resolver.h"
#include "mcld/Support/Compiler.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/ADT/StringRef.h>

#include <utility>

namespace mcld {

/** \class NamePool
 *  \brief Store symbol and search symbol by name. Can help symbol resolution.
 *
 *  - MCLinker is responsed for creating NamePool.
 */
class NamePool {
 public:
  typedef HashTable<ResolveInfo, hash::StringHash<hash::DJB> > Table;
  typedef Table::iterator syminfo_iterator;
  typedef Table::const_iterator const_syminfo_iterator;

  typedef GCFactory<ResolveInfo*, 128> FreeInfoSet;
  typedef FreeInfoSet::iterator freeinfo_iterator;
  typedef FreeInfoSet::const_iterator const_freeinfo_iterator;

  typedef size_t size_type;

 public:
  explicit NamePool(size_type pSize = 3);

  ~NamePool();

  // -----  modifiers  ----- //
  /// createSymbol - create a symbol but do not insert into the pool.
  /// The created symbol did not go through the path of symbol resolution.
  ResolveInfo* createSymbol(
      const llvm::StringRef& pName,
      bool pIsDyn,
      ResolveInfo::Type pType,
      ResolveInfo::Desc pDesc,
      ResolveInfo::Binding pBinding,
      ResolveInfo::SizeType pSize,
      ResolveInfo::Visibility pVisibility = ResolveInfo::Default);

  /// insertSymbol - insert a symbol and resolve the symbol immediately
  /// @param pOldInfo - if pOldInfo is not NULL, the old ResolveInfo being
  ///                   overriden is kept in pOldInfo.
  /// @param pResult the result of symbol resultion.
  /// @note pResult.override is true if the output LDSymbol also need to be
  ///       overriden
  void insertSymbol(const llvm::StringRef& pName,
                    bool pIsDyn,
                    ResolveInfo::Type pType,
                    ResolveInfo::Desc pDesc,
                    ResolveInfo::Binding pBinding,
                    ResolveInfo::SizeType pSize,
                    LDSymbol::ValueType pValue,
                    ResolveInfo::Visibility pVisibility,
                    ResolveInfo* pOldInfo,
                    Resolver::Result& pResult);

  /// findSymbol - find the resolved output LDSymbol
  const LDSymbol* findSymbol(const llvm::StringRef& pName) const;
  LDSymbol* findSymbol(const llvm::StringRef& pName);

  /// findInfo - find the resolved ResolveInfo
  const ResolveInfo* findInfo(const llvm::StringRef& pName) const;
  ResolveInfo* findInfo(const llvm::StringRef& pName);

  /// insertString - insert a string
  /// if the string has existed, modify pString to the existing string
  /// @return the StringRef points to the hash table
  llvm::StringRef insertString(const llvm::StringRef& pString);

  // -----  observers  ----- //
  size_type size() const { return m_Table.numOfEntries(); }

  bool empty() const { return m_Table.empty(); }

  // syminfo_iterator - traverse the ResolveInfo in the resolved HashTable
  syminfo_iterator syminfo_begin() { return m_Table.begin(); }

  syminfo_iterator syminfo_end() { return m_Table.end(); }

  const_syminfo_iterator syminfo_begin() const { return m_Table.begin(); }

  const_syminfo_iterator syminfo_end() const { return m_Table.end(); }

  // freeinfo_iterator - traverse the ResolveInfo those do not need to be
  // resolved, for example, local symbols
  freeinfo_iterator freeinfo_begin() { return m_FreeInfoSet.begin(); }

  freeinfo_iterator freeinfo_end() { return m_FreeInfoSet.end(); }

  const_freeinfo_iterator freeinfo_begin() const {
    return m_FreeInfoSet.begin();
  }

  const_freeinfo_iterator freeinfo_end() const { return m_FreeInfoSet.end(); }

  // -----  capacity  ----- //
  void reserve(size_type pN);

  size_type capacity() const;

 private:
  Resolver* m_pResolver;
  Table m_Table;
  FreeInfoSet m_FreeInfoSet;

 private:
  DISALLOW_COPY_AND_ASSIGN(NamePool);
};

}  // namespace mcld

#endif  // MCLD_LD_NAMEPOOL_H_
