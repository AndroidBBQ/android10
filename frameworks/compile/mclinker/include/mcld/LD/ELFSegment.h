//===- ELFSegment.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFSEGMENT_H_
#define MCLD_LD_ELFSEGMENT_H_
#include "mcld/Config/Config.h"
#include "mcld/Support/Allocators.h"

#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ELF.h>

#include <vector>

namespace mcld {

class LDSection;

/** \class ELFSegment
 *  \brief decribe the program header for ELF executable or shared object
 */
class ELFSegment {
 public:
  typedef std::vector<LDSection*> SectionList;
  typedef SectionList::iterator iterator;
  typedef SectionList::const_iterator const_iterator;
  typedef SectionList::reverse_iterator reverse_iterator;
  typedef SectionList::const_reverse_iterator const_reverse_iterator;

 private:
  friend class Chunk<ELFSegment, MCLD_SEGMENTS_PER_OUTPUT>;
  ELFSegment();
  explicit ELFSegment(uint32_t pType, uint32_t pFlag = llvm::ELF::PF_R);

 public:
  ~ELFSegment();

  ///  -----  iterators  -----  ///
  iterator begin() { return m_SectionList.begin(); }
  const_iterator begin() const { return m_SectionList.begin(); }
  iterator end() { return m_SectionList.end(); }
  const_iterator end() const { return m_SectionList.end(); }

  reverse_iterator rbegin() { return m_SectionList.rbegin(); }
  const_reverse_iterator rbegin() const { return m_SectionList.rbegin(); }
  reverse_iterator rend() { return m_SectionList.rend(); }
  const_reverse_iterator rend() const { return m_SectionList.rend(); }

  LDSection* front() { return m_SectionList.front(); }
  const LDSection* front() const { return m_SectionList.front(); }
  LDSection* back() { return m_SectionList.back(); }
  const LDSection* back() const { return m_SectionList.back(); }

  ///  -----  observers  -----  ///
  uint32_t type() const { return m_Type; }
  uint64_t offset() const { return m_Offset; }
  uint64_t vaddr() const { return m_Vaddr; }
  uint64_t paddr() const { return m_Paddr; }
  uint64_t filesz() const { return m_Filesz; }
  uint64_t memsz() const { return m_Memsz; }
  uint32_t flag() const { return m_Flag; }
  uint64_t align() const { return std::max(m_Align, m_MaxSectionAlign); }

  size_t size() const { return m_SectionList.size(); }
  bool empty() const { return m_SectionList.empty(); }

  bool isLoadSegment() const;
  bool isDataSegment() const;
  bool isBssSegment() const;

  ///  -----  modifiers  -----  ///
  void setOffset(uint64_t pOffset) { m_Offset = pOffset; }

  void setVaddr(uint64_t pVaddr) { m_Vaddr = pVaddr; }

  void setPaddr(uint64_t pPaddr) { m_Paddr = pPaddr; }

  void setFilesz(uint64_t pFilesz) { m_Filesz = pFilesz; }

  void setMemsz(uint64_t pMemsz) { m_Memsz = pMemsz; }

  void setFlag(uint32_t pFlag) { m_Flag = pFlag; }

  void updateFlag(uint32_t pFlag) {
    // PT_TLS segment should be PF_R
    if (llvm::ELF::PT_TLS != m_Type)
      m_Flag |= pFlag;
  }

  void setAlign(uint64_t pAlign) { m_Align = pAlign; }

  iterator insert(iterator pPos, LDSection* pSection);

  void append(LDSection* pSection);

  /* factory methods */
  static ELFSegment* Create(uint32_t pType, uint32_t pFlag = llvm::ELF::PF_R);
  static void Destroy(ELFSegment*& pSegment);
  static void Clear();

 private:
  uint32_t m_Type;             // Type of segment
  uint32_t m_Flag;             // Segment flags
  uint64_t m_Offset;           // File offset where segment is located, in bytes
  uint64_t m_Vaddr;            // Virtual address of the segment
  uint64_t m_Paddr;            // Physical address of the segment (OS-specific)
  uint64_t m_Filesz;           // # of bytes in file image of segment (may be 0)
  uint64_t m_Memsz;            // # of bytes in mem image of segment (may be 0)
  uint64_t m_Align;            // alignment constraint
  uint64_t m_MaxSectionAlign;  // max alignment of the sections in this segment
  SectionList m_SectionList;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFSEGMENT_H_
