//===- MipsELFDynamic.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSELFDYNAMIC_H_
#define TARGET_MIPS_MIPSELFDYNAMIC_H_

#include "mcld/Target/ELFDynamic.h"

namespace mcld {

class MipsGNULDBackend;

class MipsELFDynamic : public ELFDynamic {
 public:
  MipsELFDynamic(const MipsGNULDBackend& pParent, const LinkerConfig& pConfig);

 private:
  const MipsGNULDBackend& m_pParent;
  const LinkerConfig& m_pConfig;

 private:
  void reserveTargetEntries(const ELFFileFormat& pFormat);
  void applyTargetEntries(const ELFFileFormat& pFormat);

  size_t getSymTabNum(const ELFFileFormat& pFormat) const;
  size_t getGotSym(const ELFFileFormat& pFormat) const;
  size_t getLocalGotNum(const ELFFileFormat& pFormat) const;
  uint64_t getBaseAddress();
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSELFDYNAMIC_H_
