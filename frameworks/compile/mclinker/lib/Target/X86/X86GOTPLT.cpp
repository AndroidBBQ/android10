//===- X86GOTPLT.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86GOTPLT.h"
#include "X86PLT.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// X86_32GOTPLT
//===----------------------------------------------------------------------===//
X86_32GOTPLT::X86_32GOTPLT(LDSection& pSection) : X86_32GOT(pSection) {
  // create GOT0 entries
  for (size_t i = 0; i < X86GOTPLT0Num; ++i)
    create();
}

X86_32GOTPLT::~X86_32GOTPLT() {
}

bool X86_32GOTPLT::hasGOT1() const {
  return (m_SectionData->size() > X86GOTPLT0Num);
}

void X86_32GOTPLT::applyGOT0(uint64_t pAddress) {
  llvm::cast<X86_32GOTEntry>(*(m_SectionData->getFragmentList().begin()))
      .setValue(pAddress);
}

void X86_32GOTPLT::applyAllGOTPLT(const X86PLT& pPLT) {
  iterator it = begin();
  // skip GOT0
  for (size_t i = 0; i < X86GOTPLT0Num; ++i)
    ++it;
  // address of corresponding plt entry
  uint64_t plt_addr = pPLT.addr() + pPLT.getPLT0Size();
  for (; it != end(); ++it) {
    llvm::cast<X86_32GOTEntry>(*it).setValue(plt_addr + 6);
    plt_addr += pPLT.getPLT1Size();
  }
}

//===----------------------------------------------------------------------===//
// X86_64GOTPLT
//===----------------------------------------------------------------------===//
X86_64GOTPLT::X86_64GOTPLT(LDSection& pSection) : X86_64GOT(pSection) {
  for (size_t i = 0; i < X86GOTPLT0Num; ++i)
    create();
}

X86_64GOTPLT::~X86_64GOTPLT() {
}

bool X86_64GOTPLT::hasGOT1() const {
  return (m_SectionData->size() > X86GOTPLT0Num);
}

void X86_64GOTPLT::applyGOT0(uint64_t pAddress) {
  llvm::cast<X86_64GOTEntry>(*(m_SectionData->getFragmentList().begin()))
      .setValue(pAddress);
}

void X86_64GOTPLT::applyAllGOTPLT(const X86PLT& pPLT) {
  iterator it = begin();
  // skip GOT0
  for (size_t i = 0; i < X86GOTPLT0Num; ++i)
    ++it;
  // address of corresponding plt entry
  uint64_t plt_addr = pPLT.addr() + pPLT.getPLT0Size();
  for (; it != end(); ++it) {
    llvm::cast<X86_64GOTEntry>(*it).setValue(plt_addr + 6);
    plt_addr += pPLT.getPLT1Size();
  }
}

}  // namespace mcld
