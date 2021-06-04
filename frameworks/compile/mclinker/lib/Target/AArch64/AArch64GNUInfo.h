//===- AArch64GNUInfo.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64GNUINFO_H_
#define TARGET_AARCH64_AARCH64GNUINFO_H_
#include "mcld/Target/GNUInfo.h"

#include <llvm/Support/ELF.h>

namespace mcld {

class AArch64GNUInfo : public GNUInfo {
 public:
  explicit AArch64GNUInfo(const llvm::Triple& pTriple) : GNUInfo(pTriple) {}

  uint32_t machine() const { return llvm::ELF::EM_AARCH64; }

  uint64_t defaultTextSegmentAddr() const { return 0x400000; }

  // There are no processor-specific flags so this field shall contain zero.
  uint64_t flags() const { return 0x0; }
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64GNUINFO_H_
