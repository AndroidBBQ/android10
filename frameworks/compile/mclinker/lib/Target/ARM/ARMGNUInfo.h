//===- ARMGNUInfo.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMGNUINFO_H_
#define TARGET_ARM_ARMGNUINFO_H_
#include "mcld/Target/GNUInfo.h"

#include <llvm/Support/ELF.h>

namespace mcld {

class ARMGNUInfo : public GNUInfo {
 public:
  explicit ARMGNUInfo(const llvm::Triple& pTriple) : GNUInfo(pTriple) {}

  uint32_t machine() const { return llvm::ELF::EM_ARM; }

  uint64_t defaultTextSegmentAddr() const { return 0x8000; }

  uint64_t flags() const { return llvm::ELF::EF_ARM_EABI_VER5; }
};

}  // namespace mcld

#endif  // TARGET_ARM_ARMGNUINFO_H_
