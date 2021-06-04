//===- HexagonRelocationFunction.h ----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONRELOCATIONFUNCTIONS_H_
#define TARGET_HEXAGON_HEXAGONRELOCATIONFUNCTIONS_H_

typedef struct {
  const char* insnSyntax;
  uint32_t insnMask;
  uint32_t insnCmpMask;
  uint32_t insnBitMask;
  bool isDuplex;
} Instruction;

//===--------------------------------------------------------------------===//
// Relocation helper function
//===--------------------------------------------------------------------===//
template<typename T1, typename T2>
T1 ApplyMask(T2 pMask, T1 pData) {
  T1 result = 0;
  size_t off = 0;

  for (size_t bit = 0; bit != sizeof (T1) * 8; ++bit) {
    const bool valBit = (pData >> off) & 1;
    const bool maskBit = (pMask >> bit) & 1;
    if (maskBit) {
      result |= static_cast<T1>(valBit) << bit;
      ++off;
    }
  }
  return result;
}

#define DECL_HEXAGON_APPLY_RELOC_FUNC(Name)                \
  static HexagonRelocator::Result Name(Relocation& pEntry, \
                                       HexagonRelocator& pParent);

#define DECL_HEXAGON_APPLY_RELOC_FUNCS            \
  DECL_HEXAGON_APPLY_RELOC_FUNC(none)             \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocPCREL)       \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocGPREL)       \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocAbs)         \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocPLTB22PCREL) \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocGOTREL)      \
  DECL_HEXAGON_APPLY_RELOC_FUNC(relocGOT)         \
  DECL_HEXAGON_APPLY_RELOC_FUNC(unsupported)

#define DECL_HEXAGON_APPLY_RELOC_FUNC_PTRS             \
  { &none,             0, "R_HEX_NONE"              }, \
  { &relocPCREL,       1, "R_HEX_B22_PCREL"         }, \
  { &relocPCREL,       2, "R_HEX_B15_PCREL"         }, \
  { &relocPCREL,       3, "R_HEX_B7_PCREL"          }, \
  { &relocAbs,         4, "R_HEX_LO16"              }, \
  { &relocAbs,         5, "R_HEX_HI16"              }, \
  { &relocAbs,         6, "R_HEX_32"                }, \
  { &relocAbs,         7, "R_HEX_16"                }, \
  { &relocAbs,         8, "R_HEX_8"                 }, \
  { &relocGPREL,       9, "R_HEX_GPREL16_0"         }, \
  { &relocGPREL,       10, "R_HEX_GPREL16_1"        }, \
  { &relocGPREL,       11, "R_HEX_GPREL16_2"        }, \
  { &relocGPREL,       12, "R_HEX_GPREL16_3"        }, \
  { &unsupported,      13, "R_HEX_HL16"             }, \
  { &relocPCREL,       14, "R_HEX_B13_PCREL"        }, \
  { &relocPCREL,       15, "R_HEX_B9_PCREL"         }, \
  { &relocPCREL,       16, "R_HEX_B32_PCREL_X"      }, \
  { &relocAbs,         17, "R_HEX_32_6_X"           }, \
  { &relocPCREL,       18, "R_HEX_B22_PCREL_X"      }, \
  { &relocPCREL,       19, "R_HEX_B15_PCREL_X"      }, \
  { &relocPCREL,       20, "R_HEX_B13_PCREL_X"      }, \
  { &relocPCREL,       21, "R_HEX_B9_PCREL_X"       }, \
  { &relocPCREL,       22, "R_HEX_B7_PCREL_X"       }, \
  { &relocAbs,         23, "R_HEX_16_X"             }, \
  { &relocAbs,         24, "R_HEX_12_X"             }, \
  { &relocAbs,         25, "R_HEX_11_X"             }, \
  { &relocAbs,         26, "R_HEX_10_X"             }, \
  { &relocAbs,         27, "R_HEX_9_X"              }, \
  { &relocAbs,         28, "R_HEX_8_X"              }, \
  { &relocAbs,         29, "R_HEX_7_X"              }, \
  { &relocAbs,         30, "R_HEX_6_X"              }, \
  { &relocPCREL,       31, "R_HEX_32_PCREL"         }, \
  { &none,             32, "R_HEX_COPY"             }, \
  { &none,             33, "R_HEX_GLOB_DAT"         }, \
  { &none,             34, "R_HEX_JMP_SLOT"         }, \
  { &none,             35, "R_HEX_RELATIVE"         }, \
  { &relocPLTB22PCREL, 36, "R_HEX_PLT_B22_PCREL"    }, \
  { &relocGOTREL,      37, "R_HEX_GOTREL_LO16"      }, \
  { &relocGOTREL,      38, "R_HEX_GOTREL_HI16"      }, \
  { &relocGOTREL,      39, "R_HEX_GOTREL_32"        }, \
  { &relocGOT,         40, "R_HEX_GOT_LO16"         }, \
  { &relocGOT,         41, "R_HEX_GOT_HI16"         }, \
  { &relocGOT,         42, "R_HEX_GOT_32"           }, \
  { &relocGOT,         43, "R_HEX_GOT_16"           }, \
  { &unsupported,      44, "R_HEX_DTPMOD_32"        }, \
  { &unsupported,      45, "R_HEX_DTPREL_LO16"      }, \
  { &unsupported,      46, "R_HEX_DTPREL_HI16"      }, \
  { &unsupported,      47, "R_HEX_DTPREL_32"        }, \
  { &unsupported,      48, "R_HEX_DTPREL_16"        }, \
  { &unsupported,      49, "R_HEX_GD_PLT_B22_PCREL" }, \
  { &unsupported,      50, "R_HEX_GD_GOT_LO16"      }, \
  { &unsupported,      51, "R_HEX_GD_GOT_HI16"      }, \
  { &unsupported,      52, "R_HEX_GD_GOT_32"        }, \
  { &unsupported,      53, "R_HEX_GD_GOT_16"        }, \
  { &unsupported,      54, "R_HEX_IE_LO16"          }, \
  { &unsupported,      55, "R_HEX_IE_HI16"          }, \
  { &unsupported,      56, "R_HEX_IE_32"            }, \
  { &unsupported,      57, "R_HEX_IE_GOT_LO16"      }, \
  { &unsupported,      58, "R_HEX_IE_GOT_HI16"      }, \
  { &unsupported,      59, "R_HEX_IE_GOT_32"        }, \
  { &unsupported,      60, "R_HEX_IE_GOT_16"        }, \
  { &unsupported,      61, "R_HEX_TPREL_LO16"       }, \
  { &unsupported,      62, "R_HEX_TPREL_HI16"       }, \
  { &unsupported,      63, "R_HEX_TPREL_32"         }, \
  { &unsupported,      64, "R_HEX_TPREL_16"         }, \
  { &relocPCREL,       65, "R_HEX_6_PCREL_X"        }, \
  { &relocGOTREL,      66, "R_HEX_GOTREL_32_6_X"    }, \
  { &relocGOTREL,      67, "R_HEX_GOTREL_16_X"      }, \
  { &relocGOTREL,      68, "R_HEX_GOTREL_11_X"      }, \
  { &relocGOT,         69, "R_HEX_GOT_32_6_X"       }, \
  { &relocGOT,         70, "R_HEX_GOT_16_X"         }, \
  { &relocGOT,         71, "R_HEX_GOT_11_X"         }, \
  { &unsupported,      72, "R_HEX_DTPREL_32_6_X"    }, \
  { &unsupported,      73, "R_HEX_DTPREL_16_X"      }, \
  { &unsupported,      74, "R_HEX_DTPREL_11_X"      }, \
  { &unsupported,      75, "R_HEX_GD_GOT_32_6_X"    }, \
  { &unsupported,      76, "R_HEX_GD_GOT_16_X"      }, \
  { &unsupported,      77, "R_HEX_GD_GOT_11_X"      }, \
  { &unsupported,      78, "R_HEX_IE_32_6_X"        }, \
  { &unsupported,      79, "R_HEX_IE_16_X"          }, \
  { &unsupported,      80, "R_HEX_IE_GOT_32_6_X"    }, \
  { &unsupported,      81, "R_HEX_IE_GOT_16_X"      }, \
  { &unsupported,      82, "R_HEX_IE_GOT_11_X"      }, \
  { &unsupported,      83, "R_HEX_TPREL_32_6_X"     }, \
  { &unsupported,      84, "R_HEX_TPREL_16_X"       }, \
  { &unsupported,      85, "R_HEX_TPREL_11_X"       }

#endif  // TARGET_HEXAGON_HEXAGONRELOCATIONFUNCTIONS_H_
