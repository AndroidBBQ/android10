//===- HexagonGNUInfo.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "HexagonGNUInfo.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// HexagonGNUInfo
//===----------------------------------------------------------------------===//
HexagonGNUInfo::HexagonGNUInfo(const TargetOptions& pTargetOptions)
    : GNUInfo(pTargetOptions.triple()), m_Options(pTargetOptions) {
}

/// flags - the value of ElfXX_Ehdr::e_flags
uint64_t HexagonGNUInfo::flags() const {
  return llvm::StringSwitch<uint64_t>(m_Options.getTargetCPU())
      .Case("hexagonv4", V4)
      .Case("hexagonv5", V5)
      .Default(V4);
}

}  // namespace mcld
