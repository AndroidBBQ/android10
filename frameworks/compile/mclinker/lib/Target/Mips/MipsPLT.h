//===- MipsPLT.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSPLT_H_
#define TARGET_MIPS_MIPSPLT_H_

#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/PLT.h"

namespace mcld {

class MipsGOTPLT;

//===----------------------------------------------------------------------===//
// MipsPLT
//===----------------------------------------------------------------------===//
/** \class MipsPLT
 *  \brief Mips Procedure Linkage Table
 */
class MipsPLT : public PLT {
 public:
  explicit MipsPLT(LDSection& pSection);

  void finalizeSectionSize();

  // hasPLT1 - return if this PLT has any PLTA/PLTB entries
  bool hasPLT1() const;

  uint64_t emit(MemoryRegion& pRegion);

  PLTEntryBase* create();

  void applyAllPLT(MipsGOTPLT& pGOTPLT);
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSPLT_H_
