//===- ELFSegmentFactory.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFSEGMENTFACTORY_H_
#define MCLD_LD_ELFSEGMENTFACTORY_H_

#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ELF.h>

#include <vector>

namespace mcld {

class ELFSegment;
class LDSection;

/** \class ELFSegmentFactory
 *  \brief provide the interface to create and delete an ELFSegment
 */
class ELFSegmentFactory {
 public:
  typedef std::vector<ELFSegment*> Segments;
  typedef Segments::const_iterator const_iterator;
  typedef Segments::iterator iterator;

  const_iterator begin() const { return m_Segments.begin(); }
  iterator begin() { return m_Segments.begin(); }
  const_iterator end() const { return m_Segments.end(); }
  iterator end() { return m_Segments.end(); }

  const ELFSegment* front() const { return m_Segments.front(); }
  ELFSegment* front() { return m_Segments.front(); }
  const ELFSegment* back() const { return m_Segments.back(); }
  ELFSegment* back() { return m_Segments.back(); }

  size_t size() const { return m_Segments.size(); }

  bool empty() const { return m_Segments.empty(); }

  iterator find(uint32_t pType, uint32_t pFlagSet, uint32_t pFlagClear);

  const_iterator find(uint32_t pType,
                      uint32_t pFlagSet,
                      uint32_t pFlagClear) const;

  iterator find(uint32_t pType, const LDSection* pSection);

  const_iterator find(uint32_t pType, const LDSection* pSection) const;

  /// produce - produce an empty ELF segment information.
  /// this function will create an ELF segment
  /// @param pType - p_type in ELF program header
  ELFSegment* produce(uint32_t pType, uint32_t pFlag = llvm::ELF::PF_R);

  ELFSegment* insert(iterator pPosition,
                     uint32_t pType,
                     uint32_t pFlag = llvm::ELF::PF_R);

  void erase(iterator pSegment);

 private:
  Segments m_Segments;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFSEGMENTFACTORY_H_
