//===- MipsGOTPLT.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSGOTPLT_H_
#define TARGET_MIPS_MIPSGOTPLT_H_

#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/GOT.h"
#include <llvm/ADT/DenseMap.h>

namespace mcld {

class LDSection;

/** \class MipsGOTPLT
 *  \brief Mips .got.plt section.
 */
class MipsGOTPLT : public GOT {
 public:
  explicit MipsGOTPLT(LDSection& pSection);

  // hasGOT1 - return if this section has any GOT1 entry
  bool hasGOT1() const;

  uint64_t getEntryAddr(size_t num) const;

  uint64_t emit(MemoryRegion& pRegion);

  Fragment* create();

  void applyAllGOTPLT(uint64_t pltAddr);
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSGOTPLT_H_
