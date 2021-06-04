//===- impl.cpp -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86GOT.h"

#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/SectionData.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// X86_32GOT
//===----------------------------------------------------------------------===//
X86_32GOT::X86_32GOT(LDSection& pSection) : GOT(pSection) {
}

X86_32GOT::~X86_32GOT() {
}

X86_32GOTEntry* X86_32GOT::create() {
  return new X86_32GOTEntry(0, m_SectionData);
}

//===----------------------------------------------------------------------===//
// X86_64GOT
//===----------------------------------------------------------------------===//
X86_64GOT::X86_64GOT(LDSection& pSection) : GOT(pSection) {
}

X86_64GOT::~X86_64GOT() {
}

X86_64GOTEntry* X86_64GOT::create() {
  return new X86_64GOTEntry(0, m_SectionData);
}

}  // namespace mcld
