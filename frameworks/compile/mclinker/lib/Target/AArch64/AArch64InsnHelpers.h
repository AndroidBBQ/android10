//===- AArch64InsnHelpers.h -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64INSNHELPERS_H_
#define TARGET_AARCH64_AARCH64INSNHELPERS_H_

#include "mcld/Support/Compiler.h"

namespace mcld {

class AArch64InsnHelpers {
 public:
  typedef uint32_t InsnType;

  static constexpr unsigned InsnSize = 4;

  // Zero register encoding - 31.
  static constexpr unsigned ZR = 31;

  static unsigned getBits(InsnType insn, int pos, int l) {
    return (insn >> pos) & ((1 << l) - 1);
  }

  static unsigned getRt(InsnType insn) {
    return getBits(insn, 0, 5);
  }

  static unsigned getRt2(InsnType insn) {
    return getBits(insn, 10, 5);
  }

  static unsigned getRa(InsnType insn) {
    return getBits(insn, 10, 5);
  }

  static unsigned getRd(InsnType insn) {
    return getBits(insn, 0, 5);
  }

  static unsigned getRn(InsnType insn) {
    return getBits(insn, 5, 5);
  }

  static unsigned getRm(InsnType insn) {
    return getBits(insn, 16, 5);
  }

  static unsigned getBit(InsnType insn, int pos) {
    return getBits(insn, pos, 1);
  }

  static unsigned getOp31(InsnType insn) {
    return getBits(insn, 21, 3);
  }

  // All ld/st ops. See C4-182 of the ARM ARM. The encoding space for LD_PCREL,
  // LDST_RO, LDST_UI and LDST_UIMM cover prefetch ops.
  static bool isLD(InsnType insn) {
    return (getBit(insn, 22) == 1);
  }

  static bool isLDST(InsnType insn) {
    return (((insn) & 0x0a000000) == 0x08000000);
  }

  static bool isLDSTEX(InsnType insn) {
    return (((insn) & 0x3f000000) == 0x08000000);
  }

  static bool isLDSTPCREL(InsnType insn) {
    return (((insn) & 0x3b000000) == 0x18000000);
  }

  static bool isLDSTNAP(InsnType insn) {
    return (((insn) & 0x3b800000) == 0x28000000);
  }

  static bool isLDSTPPI(InsnType insn) {
    return (((insn) & 0x3b800000) == 0x28800000);
  }

  static bool isLDSTPO(InsnType insn) {
    return (((insn) & 0x3b800000) == 0x29000000);
  }

  static bool isLDSTPPRE(InsnType insn) {
    return (((insn) & 0x3b800000) == 0x29800000);
  }

  static bool isLDSTUI(InsnType insn) {
    return (((insn) & 0x3b200c00) == 0x38000000);
  }

  static bool isLDSTPIIMM(InsnType insn) {
    return (((insn) & 0x3b200c00) == 0x38000400);
  }

  static bool isLDSTU(InsnType insn) {
    return (((insn) & 0x3b200c00) == 0x38000800);
  }

  static bool isLDSTPREIMM(InsnType insn) {
    return (((insn) & 0x3b200c00) == 0x38000c00);
  }

  static bool isLDSTRO(InsnType insn) {
    return (((insn) & 0x3b200c00) == 0x38200800);
  }

  static bool isLDSTUIMM(InsnType insn) {
    return (((insn) & 0x3b000000) == 0x39000000);
  }

  static bool isLDSTSIMDM(InsnType insn) {
    return (((insn) & 0xbfbf0000) == 0x0c000000);
  }

  static bool isLDSTSIMDMPI(InsnType insn) {
    return (((insn) & 0xbfa00000) == 0x0c800000);
  }

  static bool isLDSTSIMDS(InsnType insn) {
    return (((insn) & 0xbf9f0000) == 0x0d000000);
  }

  static bool isLDSTSIMDSPI(InsnType insn) {
    return (((insn) & 0xbf800000) == 0x0d800000);
  }

  // Return true if INSN is a mac insn.
  static bool isMAC(InsnType insn) {
    return (insn & 0xff000000) == 0x9b000000;
  }

  // Return true if INSN is multiply-accumulate
  static bool isMLXL(InsnType insn) {
    unsigned op31 = getOp31(insn);
    // Exclude MUL instructions which are encoded as a multiple-accumulate with
    // RA = XZR
    if (isMAC(insn) &&
        ((op31 == 0) || (op31 == 1) || (op31 == 5)) &&
        getRa(insn) != ZR) {
      return true;
    }
    return false;
  }

  // Classify an INSN if it is indeed a load/store.
  //
  // Return true if INSN is a LD/ST instruction otherwise return false. For
  // scalar LD/ST instructions is_pair is false, rt is returned and rt2 is set
  // equal to rt. For LD/ST pair instructions is_pair is true, rt and rt2 are
  // returned.
  static bool isMemOp(InsnType insn,
                      unsigned& rt,
                      unsigned& rt2,
                      bool& is_pair,
                      bool& is_load) {
    // Bail out quickly if INSN doesn't fall into the the load-store encoding
    // space.
    if (!isLDST(insn)) {
      return false;
    }

    is_pair = false;
    is_load = false;

    if (isLDSTEX(insn)) {
      rt = getRt(insn);
      rt2 = rt;
      if (getBit(insn, 21) == 1) {
        is_pair = true;
        rt2 = getRt2(insn);
      }
      is_load = isLD(insn);
      return true;
    } else if (isLDSTNAP(insn) ||
               isLDSTPPI(insn) ||
               isLDSTPO(insn) ||
               isLDSTPPRE(insn)) {
      rt = getRt(insn);
      rt2 = getRt2(insn);
      is_pair = true;
      is_load = isLD(insn);
    } else if (isLDSTPCREL(insn) ||
               isLDSTUI(insn) ||
               isLDSTPIIMM(insn) ||
               isLDSTU(insn) ||
               isLDSTPREIMM(insn) ||
               isLDSTRO(insn) ||
               isLDSTUIMM(insn)) {
      rt = getRt(insn);
      rt2 = rt;
      unsigned opc = getBits(insn, 22, 2);
      unsigned v = getBit(insn, 26);
      unsigned opc_v = opc | (v << 2);
      if (isLDSTPCREL(insn) ||
          ((opc_v == 1) ||
           (opc_v == 2) ||
           (opc_v == 3) ||
           (opc_v == 5) ||
           (opc_v == 7))) {
        is_load = true;
      }
      return true;
    } else if (isLDSTSIMDM(insn) || isLDSTSIMDMPI(insn)) {
      unsigned opcode = (insn >> 12) & 0xf;
      rt = getRt(insn);
      is_load = (getBit(insn, 22) != 0);
      switch (opcode) {
        case 0:
        case 2: {
          rt2 = rt + 3;
          return true;
        }
        case 4:
        case 6: {
          rt2 = rt + 2;
          return true;
        }
        case 7: {
          rt2 = rt;
          return true;
        }
        case 8:
        case 10: {
          rt2 = rt + 1;
          return true;
        }
        default: {
          return false;
        }
      }  // switch (opcode)
    } else if (isLDSTSIMDS(insn) || isLDSTSIMDSPI(insn)) {
      unsigned r = (insn >> 21) & 1;
      unsigned opcode = (insn >> 13) & 0x7;
      rt = getRt(insn);
      is_load = (getBit(insn, 22) != 0);
      switch (opcode) {
        case 0:
        case 2:
        case 4:
        case 6: {
          rt2 = rt + r;
          return true;
        }
        case 1:
        case 3:
        case 5:
        case 7: {
          rt2 = rt + ((r == 0) ? 2 : 3);
          return true;
        }
        default: {
          return false;
        }
      }  // switch (opcode)
    }

    return false;
  }

  static InsnType buildBranchInsn() {
    return 0x14000000;
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(AArch64InsnHelpers);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64INSNHELPERS_H_
