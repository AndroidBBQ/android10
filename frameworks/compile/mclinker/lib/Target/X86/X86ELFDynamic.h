//===- X86ELFDynamic.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86ELFDYNAMIC_H_
#define TARGET_X86_X86ELFDYNAMIC_H_

#include "mcld/Target/ELFDynamic.h"

namespace mcld {

class X86ELFDynamic : public ELFDynamic {
 public:
  X86ELFDynamic(const GNULDBackend& pParent, const LinkerConfig& pConfig);
  ~X86ELFDynamic();

 private:
  void reserveTargetEntries(const ELFFileFormat& pFormat);
  void applyTargetEntries(const ELFFileFormat& pFormat);
};

}  // namespace mcld

#endif  // TARGET_X86_X86ELFDYNAMIC_H_
