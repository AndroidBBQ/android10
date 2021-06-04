//===- AArch64CA53Erratum835769Stub.cpp -----------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AArch64CA53Erratum835769Stub.h"
#include "AArch64InsnHelpers.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/IRBuilder.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ELF.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64CA53Erratum835769Stub
//===----------------------------------------------------------------------===//
AArch64CA53Erratum835769Stub::AArch64CA53Erratum835769Stub() {
}

/// for doClone
AArch64CA53Erratum835769Stub::AArch64CA53Erratum835769Stub(
    const uint32_t* pData,
    size_t pSize,
    const char* pName,
    const_fixup_iterator pBegin,
    const_fixup_iterator pEnd)
    : AArch64CA53ErratumStub(pData, pSize, pName, pBegin, pEnd) {
}

AArch64CA53Erratum835769Stub::~AArch64CA53Erratum835769Stub() {
}

bool AArch64CA53Erratum835769Stub::isMyDuty(const FragmentRef& pFragRef) const {
  unsigned rt;
  unsigned rt2;
  bool is_pair;
  bool is_load;
  ErratumSequence code;
  pFragRef.memcpy(&code, sizeof(ErratumSequence), 0);

  if (AArch64InsnHelpers::isMLXL(code.insns[1]) &&
      AArch64InsnHelpers::isMemOp(code.insns[0], rt, rt2, is_pair, is_load)) {
    // Any SIMD memory op is independent of the subsequent MLA by definition of
    // the erratum.
    if (AArch64InsnHelpers::getBit(code.insns[0], 26) != 0) {
      return true;
    }

    // If not SIMD, check for integer memory ops and MLA relationship.
    unsigned ra = AArch64InsnHelpers::getRa(code.insns[1]);
    unsigned rm = AArch64InsnHelpers::getRm(code.insns[1]);
    unsigned rn = AArch64InsnHelpers::getRn(code.insns[1]);

    // If this is a load and there's a true(RAW) dependency, we are safe and
    // this is not an erratum sequence.
    if (is_load &&
        ((rt == ra) ||
         (rt == rm) ||
         (rt == rn) ||
         (is_pair && ((rt2 == ra) || (rt2 == rm) || (rt2 == rn))))) {
      return false;
    }

    // We conservatively put out stubs for all other cases (including
    // writebacks).
    return true;
  }

  return false;
}

Stub* AArch64CA53Erratum835769Stub::doClone() {
  return new AArch64CA53Erratum835769Stub(getData(),
                                          size(),
                                          "erratum_835769_veneer",
                                          fixup_begin(),
                                          fixup_end());
}

}  // namespace mcld
