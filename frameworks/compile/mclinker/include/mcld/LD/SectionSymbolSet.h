//===- SectionSymbolSet.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_SECTIONSYMBOLSET_H_
#define MCLD_LD_SECTIONSYMBOLSET_H_

#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/HashEntry.h"
#include "mcld/MC/SymbolCategory.h"

namespace mcld {

class LDSection;
class NamePool;
class LDSymbol;

/** \class SectionSymbolSet
 *  \brief SectionSymbolSet contains the section symbols defined by linker for
 *   the output sections
 */
class SectionSymbolSet {
 public:
  typedef SymbolCategory SymbolTable;

 public:
  SectionSymbolSet();
  ~SectionSymbolSet();

  /// add - create and add an section symbol for the output
  /// LDSection
  bool add(LDSection& pOutSect, NamePool& pNamePool);

  /// finalize - set section symbols' fragmentRef and push it into the output
  /// symbol table
  bool finalize(LDSection& pOutSect, SymbolTable& pSymTab, bool relocatable);

  /// get - get the section symbol for section pOutpSect
  LDSymbol* get(const LDSection& pOutSect);
  const LDSymbol* get(const LDSection& pOutSect) const;

 private:
  /// sectCompare - hash compare function for LDSection*
  struct SectCompare {
    bool operator()(const LDSection* X, const LDSection* Y) const {
      return (X == Y);
    }
  };

  /// SectPtrHash - hash function for LDSection*
  struct SectPtrHash {
    size_t operator()(const LDSection* pKey) const {
      return (unsigned((uintptr_t)pKey) >> 4) ^
             (unsigned((uintptr_t)pKey) >> 9);
    }
  };

  typedef HashEntry<const LDSection*, LDSymbol*, SectCompare> SectHashEntryType;
  typedef HashTable<SectHashEntryType,
                    SectPtrHash,
                    EntryFactory<SectHashEntryType> > SectHashTableType;

 private:
  SectHashTableType* m_pSectionSymbolMap;
};

}  // namespace mcld

#endif  // MCLD_LD_SECTIONSYMBOLSET_H_
