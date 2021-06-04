//===- ARMPLT.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMPLT_H_
#define TARGET_ARM_ARMPLT_H_

#include "mcld/Target/GOT.h"
#include "mcld/Target/PLT.h"
#include "mcld/Support/MemoryRegion.h"

const uint32_t arm_plt0[] = {
    0xe52de004,  // str   lr, [sp, #-4]!
    0xe59fe004,  // ldr   lr, [pc, #4]
    0xe08fe00e,  // add   lr, pc, lr
    0xe5bef008,  // ldr   pc, [lr, #8]!
    0x00000000   // &GOT[0] - .
};

const uint32_t arm_plt1[] = {
    0xe28fc600,  // add   ip, pc, #0xNN00000
    0xe28cca00,  // add   ip, ip, #0xNN000
    0xe5bcf000   // ldr   pc, [ip, #0xNNN]!
};

namespace mcld {

class ARMGOT;

class ARMPLT0 : public PLT::Entry<sizeof(arm_plt0)> {
 public:
  ARMPLT0(SectionData& pParent);
};

class ARMPLT1 : public PLT::Entry<sizeof(arm_plt1)> {
 public:
  ARMPLT1(SectionData& pParent);
};

/** \class ARMPLT
 *  \brief ARM Procedure Linkage Table
 */
class ARMPLT : public PLT {
 public:
  ARMPLT(LDSection& pSection, ARMGOT& pGOTPLT);
  ~ARMPLT();

  // finalizeSectionSize - set LDSection size
  void finalizeSectionSize();

  // hasPLT1 - return if this plt section has any plt1 entry
  bool hasPLT1() const;

  ARMPLT1* create();

  ARMPLT0* getPLT0() const;

  void applyPLT0();

  void applyPLT1();

  uint64_t emit(MemoryRegion& pRegion);

 private:
  ARMGOT& m_GOT;
};

}  // namespace mcld

#endif  // TARGET_ARM_ARMPLT_H_
