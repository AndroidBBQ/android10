//===- TargetOptions.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/TargetOptions.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// TargetOptions
//===----------------------------------------------------------------------===//
TargetOptions::TargetOptions()
    : m_Endian(Unknown),
      m_BitClass(0),
      m_GPSize(8),
      m_StubGroupSize(0),
      m_FixCA53Erratum835769(false) {
}

TargetOptions::TargetOptions(const std::string& pTriple)
    : m_Triple(pTriple),
      m_Endian(Unknown),
      m_BitClass(0),
      m_GPSize(8),
      m_StubGroupSize(0),
      m_FixCA53Erratum835769(false) {
}

TargetOptions::~TargetOptions() {
}

void TargetOptions::setTriple(const llvm::Triple& pTriple) {
  m_Triple = pTriple;
}

void TargetOptions::setTriple(const std::string& pTriple) {
  m_Triple.setTriple(pTriple);
}

void TargetOptions::setArch(const std::string& pArchName) {
  m_ArchName = pArchName;
}

void TargetOptions::setTargetCPU(const std::string& pCPU) {
  m_TargetCPU = pCPU;
}

}  // namespace mcld
