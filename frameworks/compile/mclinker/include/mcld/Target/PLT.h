//===- PLT.h --------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_PLT_H_
#define MCLD_TARGET_PLT_H_

#include "mcld/Fragment/TargetFragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"

namespace mcld {

class LDSection;
class ResolveInfo;

/** \class PLTEntryDefaultBase
 *  \brief PLTEntryDefaultBase provides the default interface for PLT Entry
 */
class PLTEntryBase : public TargetFragment {
 public:
  explicit PLTEntryBase(SectionData& pParent)
      : TargetFragment(Fragment::Target, &pParent), m_pValue(NULL) {}

  virtual ~PLTEntryBase() { free(m_pValue); }

  void setValue(unsigned char* pValue) { m_pValue = pValue; }

  const unsigned char* getValue() const { return m_pValue; }

  // Used by llvm::cast<>.
  static bool classof(const Fragment* O) { return true; }

 protected:
  unsigned char* m_pValue;
};

/** \class PLT
 *  \brief Procedure linkage table
 */
class PLT {
 public:
  typedef SectionData::iterator iterator;
  typedef SectionData::const_iterator const_iterator;

  template <size_t SIZE, typename EntryBase = PLTEntryBase>
  class Entry : public EntryBase {
   public:
    enum { EntrySize = SIZE };

   public:
    explicit Entry(SectionData& pParent) : EntryBase(pParent) {}

    virtual ~Entry() {}

    size_t size() const { return EntrySize; }
  };

 public:
  explicit PLT(LDSection& pSection);

  virtual ~PLT();

  // finalizeSectionSize - set LDSection size
  virtual void finalizeSectionSize() = 0;

  uint64_t addr() const { return m_Section.addr(); }

  const_iterator begin() const { return m_pSectionData->begin(); }
  iterator begin() { return m_pSectionData->begin(); }
  const_iterator end() const { return m_pSectionData->end(); }
  iterator end() { return m_pSectionData->end(); }

 protected:
  LDSection& m_Section;
  SectionData* m_pSectionData;
};

}  // namespace mcld

#endif  // MCLD_TARGET_PLT_H_
