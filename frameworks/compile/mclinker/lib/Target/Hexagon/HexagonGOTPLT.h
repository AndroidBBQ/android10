//===- HexagonGOTPLT.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONGOTPLT_H_
#define TARGET_HEXAGON_HEXAGONGOTPLT_H_

#include "HexagonGOT.h"
#include <llvm/ADT/DenseMap.h>

namespace mcld {

class HexagonPLT;
class LDSection;

// Hexagon creates 4 entries for the GOTPLT0 entry
const unsigned int HexagonGOTPLT0Num = 4;

/** \class HexagonGOTPLT
 *  \brief Hexagon .got.plt section.
 */
class HexagonGOTPLT : public HexagonGOT {
 public:
  explicit HexagonGOTPLT(LDSection& pSection);

  ~HexagonGOTPLT();

  // hasGOT1 - return if this section has any GOT1 entry
  bool hasGOT1() const;

  void applyGOT0(uint64_t pAddress);

  void applyAllGOTPLT(const HexagonPLT& pPLT);
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONGOTPLT_H_
