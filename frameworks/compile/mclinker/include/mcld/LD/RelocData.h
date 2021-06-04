//===- RelocData.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_RELOCDATA_H_
#define MCLD_LD_RELOCDATA_H_

#include "mcld/ADT/ilist_sort.h"
#include "mcld/Config/Config.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/Support/Allocators.h"
#include "mcld/Support/Compiler.h"
#include "mcld/Support/GCFactoryListTraits.h"

#include <llvm/ADT/ilist.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/DataTypes.h>

#include <list>

namespace mcld {

class LDSection;

/** \class RelocData
 *  \brief RelocData stores Relocation.
 *
 *  Since Relocations are created by GCFactory, we use GCFactoryListTraits for
 *the
 *  RelocationList here to avoid iplist to delete Relocations.
 */
class RelocData {
 private:
  friend class Chunk<RelocData, MCLD_SECTIONS_PER_INPUT>;

  RelocData();
  explicit RelocData(LDSection& pSection);

 public:
  typedef llvm::iplist<Relocation, GCFactoryListTraits<Relocation> >
      RelocationListType;

  typedef RelocationListType::reference reference;
  typedef RelocationListType::const_reference const_reference;

  typedef RelocationListType::iterator iterator;
  typedef RelocationListType::const_iterator const_iterator;

  typedef RelocationListType::reverse_iterator reverse_iterator;
  typedef RelocationListType::const_reverse_iterator const_reverse_iterator;

 public:
  static RelocData* Create(LDSection& pSection);

  static void Destroy(RelocData*& pSection);

  static void Clear();

  const LDSection& getSection() const { return *m_pSection; }
  LDSection& getSection() { return *m_pSection; }

  const RelocationListType& getRelocationList() const { return m_Relocations; }
  RelocationListType& getRelocationList() { return m_Relocations; }

  size_t size() const { return m_Relocations.size(); }

  bool empty() const { return m_Relocations.empty(); }

  RelocData& append(Relocation& pRelocation);
  Relocation& remove(Relocation& pRelocation);

  const_reference front() const { return m_Relocations.front(); }
  reference front() { return m_Relocations.front(); }
  const_reference back() const { return m_Relocations.back(); }
  reference back() { return m_Relocations.back(); }

  const_iterator begin() const { return m_Relocations.begin(); }
  iterator begin() { return m_Relocations.begin(); }
  const_iterator end() const { return m_Relocations.end(); }
  iterator end() { return m_Relocations.end(); }
  const_reverse_iterator rbegin() const { return m_Relocations.rbegin(); }
  reverse_iterator rbegin() { return m_Relocations.rbegin(); }
  const_reverse_iterator rend() const { return m_Relocations.rend(); }
  reverse_iterator rend() { return m_Relocations.rend(); }

  template <class Comparator>
  void sort(Comparator pComparator) {
    mcld::sort(m_Relocations, pComparator);
  }

 private:
  RelocationListType m_Relocations;
  LDSection* m_pSection;

 private:
  DISALLOW_COPY_AND_ASSIGN(RelocData);
};

}  // namespace mcld

#endif  // MCLD_LD_RELOCDATA_H_
