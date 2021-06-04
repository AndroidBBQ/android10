//===- HexagonPLT.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONPLT_H_
#define TARGET_HEXAGON_HEXAGONPLT_H_

#include "HexagonGOT.h"
#include "HexagonGOTPLT.h"
#include "mcld/Target/GOT.h"
#include "mcld/Target/PLT.h"
#include "mcld/Support/MemoryRegion.h"

const uint8_t hexagon_plt0[] = {
    0x00, 0x40, 0x00, 0x00,  // { immext (#0)
    0x1c, 0xc0, 0x49, 0x6a,  //   r28 = add (pc, ##GOT0@PCREL) } # address of GOT0   // NOLINT
    0x0e, 0x42, 0x9c, 0xe2,  // { r14 -= add (r28, #16)   # offset of GOTn from GOTa // NOLINT
    0x4f, 0x40, 0x9c, 0x91,  //   r15 = memw (r28 + #8)   # object ID at GOT2
    0x3c, 0xc0, 0x9c, 0x91,  //   r28 = memw (r28 + #4) } # dynamic link at GOT1
    0x0e, 0x42, 0x0e, 0x8c,  // { r14 = asr (r14, #2)     # index of PLTn
    0x00, 0xc0, 0x9c, 0x52,  //   jumpr r28 }             # call dynamic linker
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

const uint8_t hexagon_plt1[] = {
    0x00, 0x40, 0x00, 0x00,  // { immext (#0)
    0x0e, 0xc0, 0x49, 0x6a,  //   r14 = add (pc, ##GOTn@PCREL) } # address of GOTn  // NOLINT
    0x1c, 0xc0, 0x8e, 0x91,  // r28 = memw (r14)                 # contents of GOTn // NOLINT
    0x00, 0xc0, 0x9c, 0x52   // jumpr r28                        # call it
};

namespace mcld {

class GOTEntry;
class HexagonPLT1;
class LinkerConfig;

//===----------------------------------------------------------------------===//
// HexagonPLT Entry
//===----------------------------------------------------------------------===//
class HexagonPLT0 : public PLT::Entry<sizeof(hexagon_plt0)> {
 public:
  HexagonPLT0(SectionData& pParent);
};

//===----------------------------------------------------------------------===//
// HexagonPLT
//===----------------------------------------------------------------------===//
/** \class HexagonPLT
 *  \brief Hexagon Procedure Linkage Table
 */
class HexagonPLT : public PLT {
 public:
  HexagonPLT(LDSection& pSection,
             HexagonGOTPLT& pGOTPLT,
             const LinkerConfig& pConfig);
  ~HexagonPLT();

  // finalizeSectionSize - set LDSection size
  void finalizeSectionSize();

  // hasPLT1 - return if this PLT has any PLT1 entry
  bool hasPLT1() const;

  HexagonPLT1* create();

  void applyPLT0();

  void applyPLT1();

  uint64_t emit(MemoryRegion& pRegion);

  PLTEntryBase* getPLT0() const;

 private:
  HexagonGOTPLT& m_GOTPLT;

  const uint8_t* m_PLT0;
  unsigned int m_PLT0Size;
};

class HexagonPLT1 : public PLT::Entry<sizeof(hexagon_plt1)> {
 public:
  HexagonPLT1(SectionData& pParent);
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONPLT_H_
