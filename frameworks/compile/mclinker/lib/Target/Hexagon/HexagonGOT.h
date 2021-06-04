//===- HexagonGOT.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONGOT_H_
#define TARGET_HEXAGON_HEXAGONGOT_H_

#include "mcld/Target/GOT.h"

namespace mcld {

class LDSection;
class SectionData;

/** \class HexagonGOTEntry
 *  \brief GOT Entry with size of 4 bytes
 */
class HexagonGOTEntry : public GOT::Entry<4> {
 public:
  HexagonGOTEntry(uint64_t pContent, SectionData* pParent)
      : GOT::Entry<4>(pContent, pParent) {}
};

/** \class HexagonGOT
 *  \brief Hexagon Global Offset Table.
 */

class HexagonGOT : public GOT {
 public:
  explicit HexagonGOT(LDSection& pSection);

  ~HexagonGOT();

  HexagonGOTEntry* create();
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONGOT_H_
