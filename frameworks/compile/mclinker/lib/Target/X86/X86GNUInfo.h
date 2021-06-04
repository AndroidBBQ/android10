//===- X86GNUInfo.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86GNUINFO_H_
#define TARGET_X86_X86GNUINFO_H_
#include "mcld/Target/GNUInfo.h"

#include <llvm/Support/ELF.h>

namespace mcld {

class X86_32GNUInfo : public GNUInfo {
 public:
  explicit X86_32GNUInfo(const llvm::Triple& pTriple) : GNUInfo(pTriple) {}

  uint32_t machine() const { return llvm::ELF::EM_386; }

  uint64_t defaultTextSegmentAddr() const { return 0x08048000; }

  /// flags - the value of ElfXX_Ehdr::e_flags
  /// FIXME
  uint64_t flags() const { return 0x0; }
};

class X86_64GNUInfo : public GNUInfo {
 public:
  explicit X86_64GNUInfo(const llvm::Triple& pTriple) : GNUInfo(pTriple) {}

  uint32_t machine() const { return llvm::ELF::EM_X86_64; }

  uint64_t defaultTextSegmentAddr() const { return 0x400000; }

  /// flags - the value of ElfXX_Ehdr::e_flags
  /// FIXME
  uint64_t flags() const { return 0x0; }
};

}  // namespace mcld

#endif  // TARGET_X86_X86GNUINFO_H_
