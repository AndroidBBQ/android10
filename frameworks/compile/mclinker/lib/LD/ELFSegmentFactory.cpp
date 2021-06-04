//===- ELFSegmentFactory.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFSegmentFactory.h"
#include "mcld/LD/ELFSegment.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFSegmentFactory
//===----------------------------------------------------------------------===//

ELFSegmentFactory::iterator ELFSegmentFactory::find(uint32_t pType,
                                                    uint32_t pFlagSet,
                                                    uint32_t pFlagClear) {
  iterator segment, segEnd = end();
  for (segment = begin(); segment != segEnd; ++segment) {
    if ((*segment)->type() == pType &&
        ((*segment)->flag() & pFlagSet) == pFlagSet &&
        ((*segment)->flag() & pFlagClear) == 0x0) {
      return segment;
    }
  }
  return segEnd;
}

ELFSegmentFactory::const_iterator ELFSegmentFactory::find(
    uint32_t pType,
    uint32_t pFlagSet,
    uint32_t pFlagClear) const {
  const_iterator segment, segEnd = end();
  for (segment = begin(); segment != segEnd; ++segment) {
    if ((*segment)->type() == pType &&
        ((*segment)->flag() & pFlagSet) == pFlagSet &&
        ((*segment)->flag() & pFlagClear) == 0x0) {
      return segment;
    }
  }
  return segEnd;
}

ELFSegmentFactory::iterator ELFSegmentFactory::find(uint32_t pType,
                                                    const LDSection* pSection) {
  iterator segment, segEnd = end();
  for (segment = begin(); segment != segEnd; ++segment) {
    if ((*segment)->type() == pType) {
      ELFSegment::iterator sect, sectEnd = (*segment)->end();
      for (sect = (*segment)->begin(); sect != sectEnd; ++sect) {
        if (*sect == pSection)
          return segment;
      }  // for each section
    }
  }  // for each segment
  return segEnd;
}

ELFSegmentFactory::const_iterator ELFSegmentFactory::find(
    uint32_t pType,
    const LDSection* pSection) const {
  const_iterator segment, segEnd = end();
  for (segment = begin(); segment != segEnd; ++segment) {
    if ((*segment)->type() == pType) {
      ELFSegment::const_iterator sect, sectEnd = (*segment)->end();
      for (sect = (*segment)->begin(); sect != sectEnd; ++sect) {
        if (*sect == pSection)
          return segment;
      }  // for each section
    }
  }  // for each segment
  return segEnd;
}

ELFSegment* ELFSegmentFactory::produce(uint32_t pType, uint32_t pFlag) {
  m_Segments.push_back(ELFSegment::Create(pType, pFlag));
  return back();
}

ELFSegment* ELFSegmentFactory::insert(iterator pPosition,
                                      uint32_t pType,
                                      uint32_t pFlag) {
  return *(m_Segments.insert(pPosition, ELFSegment::Create(pType, pFlag)));
}

void ELFSegmentFactory::erase(iterator pSegment) {
  m_Segments.erase(pSegment);
}

}  // namespace mcld
