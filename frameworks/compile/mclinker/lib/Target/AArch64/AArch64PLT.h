//===- AArch64PLT.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64PLT_H_
#define TARGET_AARCH64_AARCH64PLT_H_

#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/GOT.h"
#include "mcld/Target/PLT.h"

const uint8_t aarch64_plt0[] = {
    0xf0, 0x7b, 0xbf, 0xa9,  /* stp x16, x30, [sp, #-16]! */
    0x10, 0x00, 0x00, 0x90,  /* adrp x16, (GOT+16) */
    0x11, 0x0A, 0x40, 0xf9,  /* ldr x17, [x16, #PLT_GOT+0x10] */
    0x10, 0x42, 0x00, 0x91,  /* add x16, x16,#PLT_GOT+0x10 */
    0x20, 0x02, 0x1f, 0xd6,  /* br x17 */
    0x1f, 0x20, 0x03, 0xd5,  /* nop */
    0x1f, 0x20, 0x03, 0xd5,  /* nop */
    0x1f, 0x20, 0x03, 0xd5   /* nop */
};

const uint8_t aarch64_plt1[] = {
    0x10, 0x00, 0x00, 0x90,  /* adrp x16, PLTGOT + n * 8 */
    0x11, 0x02, 0x40, 0xf9,  /* ldr x17, [x16, PLTGOT + n * 8] */
    0x10, 0x02, 0x00, 0x91,  /* add x16, x16, :lo12:PLTGOT + n * 8 */
    0x20, 0x02, 0x1f, 0xd6   /* br x17.  */
};

namespace mcld {

class AArch64GOT;

class AArch64PLT0 : public PLT::Entry<sizeof(aarch64_plt0)> {
 public:
  AArch64PLT0(SectionData& pParent);
};

class AArch64PLT1 : public PLT::Entry<sizeof(aarch64_plt1)> {
 public:
  AArch64PLT1(SectionData& pParent);
};

/** \class AArch64PLT
 *  \brief AArch64 Procedure Linkage Table
 */
class AArch64PLT : public PLT {
 public:
  AArch64PLT(LDSection& pSection, AArch64GOT& pGOTPLT);
  ~AArch64PLT();

  // finalizeSectionSize - set LDSection size
  void finalizeSectionSize();

  // hasPLT1 - return if this plt section has any plt1 entry
  bool hasPLT1() const;

  AArch64PLT1* create();

  AArch64PLT0* getPLT0() const;

  void applyPLT0();

  void applyPLT1();

  uint64_t emit(MemoryRegion& pRegion);

 private:
  AArch64GOT& m_GOT;
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64PLT_H_
