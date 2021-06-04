//===- HexagonELFDynamic.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONELFDYNAMIC_H_
#define TARGET_HEXAGON_HEXAGONELFDYNAMIC_H_

#include "mcld/Target/ELFDynamic.h"

namespace mcld {

class HexagonELFDynamic : public ELFDynamic {
 public:
  HexagonELFDynamic(const GNULDBackend& pParent, const LinkerConfig& pConfig);
  ~HexagonELFDynamic();

 private:
  void reserveTargetEntries(const ELFFileFormat& pFormat);
  void applyTargetEntries(const ELFFileFormat& pFormat);
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONELFDYNAMIC_H_
