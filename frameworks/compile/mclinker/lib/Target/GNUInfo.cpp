//===- GNUInfo.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Target/GNUInfo.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// GNUInfo
//===----------------------------------------------------------------------===//
GNUInfo::GNUInfo(const llvm::Triple& pTriple) : m_Triple(pTriple) {
}

uint8_t GNUInfo::OSABI() const {
  switch (m_Triple.getOS()) {
    case llvm::Triple::FreeBSD:
      return llvm::ELF::ELFOSABI_FREEBSD;
    case llvm::Triple::Linux:
      return llvm::ELF::ELFOSABI_LINUX;
    default:
      return llvm::ELF::ELFOSABI_NONE;
  }
}

}  // namespace mcld
