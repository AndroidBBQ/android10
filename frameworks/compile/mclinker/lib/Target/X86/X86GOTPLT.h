//===- X86GOTPLT.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86GOTPLT_H_
#define TARGET_X86_X86GOTPLT_H_

#include "X86GOT.h"

#include <llvm/ADT/DenseMap.h>

namespace mcld {

class LDSection;
class X86PLT;

const unsigned int X86GOTPLT0Num = 3;

/** \class X86_32GOTPLT
 *  \brief X86_32 .got.plt section.
 */
class X86_32GOTPLT : public X86_32GOT {
 public:
  explicit X86_32GOTPLT(LDSection& pSection);

  ~X86_32GOTPLT();

  // hasGOT1 - return if this section has any GOT1 entry
  bool hasGOT1() const;

  void applyGOT0(uint64_t pAddress);

  void applyAllGOTPLT(const X86PLT& pPLT);
};

/** \class X86_64GOTPLT
 *  \brief X86_64 .got.plt section.
 */
class X86_64GOTPLT : public X86_64GOT {
 public:
  explicit X86_64GOTPLT(LDSection& pSection);

  ~X86_64GOTPLT();

  // hasGOT1 - return if this section has any GOT1 entry
  bool hasGOT1() const;

  void applyGOT0(uint64_t pAddress);

  void applyAllGOTPLT(const X86PLT& pPLT);
};

}  // namespace mcld

#endif  // TARGET_X86_X86GOTPLT_H_
