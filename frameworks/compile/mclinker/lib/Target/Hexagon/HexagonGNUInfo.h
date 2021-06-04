//===- HexagonGNUInfo.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONGNUINFO_H_
#define TARGET_HEXAGON_HEXAGONGNUINFO_H_
#include "mcld/Target/GNUInfo.h"
#include "mcld/TargetOptions.h"

#include <llvm/Support/ELF.h>

namespace mcld {

class HexagonGNUInfo : public GNUInfo {
 public:
  enum CPUType { V3 = 0x2, V4 = 0x3, V5 };

 public:
  explicit HexagonGNUInfo(const TargetOptions& pTargetOptions);

  uint32_t machine() const { return llvm::ELF::EM_HEXAGON; }

  uint64_t defaultTextSegmentAddr() const { return 0x0; }

  /// flags - the value of ElfXX_Ehdr::e_flags
  uint64_t flags() const;

 private:
  const TargetOptions& m_Options;
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONGNUINFO_H_
