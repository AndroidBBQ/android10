//===- SectionData.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_SECTIONDATA_H_
#define MCLD_LD_SECTIONDATA_H_

#include "mcld/Config/Config.h"
#include "mcld/Fragment/Fragment.h"
#include "mcld/Support/Allocators.h"
#include "mcld/Support/Compiler.h"

#include <llvm/ADT/ilist.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/DataTypes.h>

namespace mcld {

class LDSection;

/** \class SectionData
 *  \brief SectionData provides a container for all Fragments.
 */
class SectionData {
 private:
  friend class Chunk<SectionData, MCLD_SECTIONS_PER_INPUT>;

  SectionData();
  explicit SectionData(LDSection& pSection);

 public:
  typedef llvm::iplist<Fragment> FragmentListType;

  typedef FragmentListType::reference reference;
  typedef FragmentListType::const_reference const_reference;

  typedef FragmentListType::iterator iterator;
  typedef FragmentListType::const_iterator const_iterator;

  typedef FragmentListType::reverse_iterator reverse_iterator;
  typedef FragmentListType::const_reverse_iterator const_reverse_iterator;

 public:
  static SectionData* Create(LDSection& pSection);

  static void Destroy(SectionData*& pSection);

  static void Clear();

  const LDSection& getSection() const { return *m_pSection; }
  LDSection& getSection() { return *m_pSection; }

  const FragmentListType& getFragmentList() const { return m_Fragments; }
  FragmentListType& getFragmentList() { return m_Fragments; }

  size_t size() const { return m_Fragments.size(); }

  bool empty() const { return m_Fragments.empty(); }

  static FragmentListType SectionData::*getSublistAccess(Fragment *) {
    return &SectionData::m_Fragments;
  }

  reference front() { return m_Fragments.front(); }
  const_reference front() const { return m_Fragments.front(); }
  reference back() { return m_Fragments.back(); }
  const_reference back() const { return m_Fragments.back(); }

  const_iterator begin() const { return m_Fragments.begin(); }
  iterator begin() { return m_Fragments.begin(); }
  const_iterator end() const { return m_Fragments.end(); }
  iterator end() { return m_Fragments.end(); }
  const_reverse_iterator rbegin() const { return m_Fragments.rbegin(); }
  reverse_iterator rbegin() { return m_Fragments.rbegin(); }
  const_reverse_iterator rend() const { return m_Fragments.rend(); }
  reverse_iterator rend() { return m_Fragments.rend(); }

 private:
  FragmentListType m_Fragments;
  LDSection* m_pSection;

 private:
  DISALLOW_COPY_AND_ASSIGN(SectionData);
};

}  // namespace mcld

#endif  // MCLD_LD_SECTIONDATA_H_
