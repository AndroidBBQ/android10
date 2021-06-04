//===- AArch64ELFDynamic.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64ELFDYNAMIC_H_
#define TARGET_AARCH64_AARCH64ELFDYNAMIC_H_

#include "mcld/Target/ELFDynamic.h"

namespace mcld {

class AArch64ELFDynamic : public ELFDynamic {
 public:
  AArch64ELFDynamic(const GNULDBackend& pParent, const LinkerConfig& pConfig);
  ~AArch64ELFDynamic();

 private:
  void reserveTargetEntries(const ELFFileFormat& pFormat);
  void applyTargetEntries(const ELFFileFormat& pFormat);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64ELFDYNAMIC_H_
