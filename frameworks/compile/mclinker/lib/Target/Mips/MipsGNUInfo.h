//===- MipsGNUInfo.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSGNUINFO_H_
#define TARGET_MIPS_MIPSGNUINFO_H_
#include "mcld/Target/GNUInfo.h"

#include <llvm/Support/ELF.h>
namespace mcld {

class MipsGNUInfo : public GNUInfo {
 public:
  explicit MipsGNUInfo(const llvm::Triple& pTriple);

  void setABIVersion(uint8_t ver);
  void setElfFlags(uint64_t flags);

  // GNUInfo
  uint32_t machine() const;
  uint8_t ABIVersion() const;
  uint64_t defaultTextSegmentAddr() const;
  uint64_t flags() const;
  const char* entry() const;
  const char* dyld() const;
  uint64_t abiPageSize() const;

 private:
  uint8_t m_ABIVersion;
  uint64_t m_ElfFlags;
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSGNUINFO_H_
