//===- OutputRelocSection.h -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_OUTPUTRELOCSECTION_H_
#define MCLD_TARGET_OUTPUTRELOCSECTION_H_

#include "mcld/LD/RelocData.h"

namespace mcld {

class LDSymbol;
class Module;
class Relocation;
class RelocationFactory;

/** \class OutputRelocSection
 *  \brief Dynamic relocation section for ARM .rel.dyn and .rel.plt
 */
class OutputRelocSection {
 public:
  OutputRelocSection(Module& pModule, LDSection& pSection);

  ~OutputRelocSection();

  /// create - create an dynamic relocation entry
  Relocation* create();

  void reserveEntry(size_t pNum = 1);

  Relocation* consumeEntry();

  /// addSymbolToDynSym - add local symbol to TLS category so that it'll be
  /// emitted into .dynsym
  bool addSymbolToDynSym(LDSymbol& pSymbol);

  // ----- observers ----- //
  bool empty() { return m_pRelocData->empty(); }

  size_t numOfRelocs();

 private:
  typedef RelocData::iterator RelocIterator;

 private:
  Module& m_Module;

  /// m_RelocData - the output RelocData which contains the dynamic
  /// relocations
  RelocData* m_pRelocData;

  /// m_isVisit - First time visit the function getEntry() or not
  bool m_isVisit;

  /// m_ValidEntryIterator - point to the first valid entry
  RelocIterator m_ValidEntryIterator;
};

}  // namespace mcld

#endif  // MCLD_TARGET_OUTPUTRELOCSECTION_H_
