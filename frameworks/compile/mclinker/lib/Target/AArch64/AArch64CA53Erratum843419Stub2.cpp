//===- AArch64CA53Erratum843419Stub2.cpp ----------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AArch64CA53Erratum843419Stub.h"
#include "AArch64CA53Erratum843419Stub2.h"
#include "AArch64InsnHelpers.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/IRBuilder.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ELF.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64CA53Erratum843419Stub2
//===----------------------------------------------------------------------===//
AArch64CA53Erratum843419Stub2::AArch64CA53Erratum843419Stub2() {
}

/// for doClone
AArch64CA53Erratum843419Stub2::AArch64CA53Erratum843419Stub2(
    const uint32_t* pData,
    size_t pSize,
    const char* pName,
    const_fixup_iterator pBegin,
    const_fixup_iterator pEnd)
    : AArch64CA53ErratumStub(pData, pSize, pName, pBegin, pEnd) {
}

AArch64CA53Erratum843419Stub2::~AArch64CA53Erratum843419Stub2() {
}

bool AArch64CA53Erratum843419Stub2::isMyDuty(
    const FragmentRef& pFragRef) const {
  if ((pFragRef.offset() + AArch64InsnHelpers::InsnSize * 4) >
      pFragRef.frag()->size()) {
    return false;
  }

  // The first instruction must be ending at 0xFF8 or 0xFFC.
  const uint64_t vma = pFragRef.frag()->getParent()->getSection().addr() +
                       pFragRef.getOutputOffset();
  const unsigned page_offset = (vma & 0xFFF);
  if ((page_offset != 0xFF8) && (page_offset != 0xFFC)) {
    return false;
  }

  ErratumSequence code;
  pFragRef.memcpy(&code, AArch64InsnHelpers::InsnSize * 4, 0);

  if (AArch64CA53Erratum843419Stub::isErratum843419Sequence(code.insns[0],
                                                            code.insns[1],
                                                            code.insns[3])) {
    return true;
  }

  return false;
}

Stub* AArch64CA53Erratum843419Stub2::doClone() {
  return new AArch64CA53Erratum843419Stub2(getData(),
                                           size(),
                                           "erratum_843419_veneer",
                                           fixup_begin(),
                                           fixup_end());
}

}  // namespace mcld
