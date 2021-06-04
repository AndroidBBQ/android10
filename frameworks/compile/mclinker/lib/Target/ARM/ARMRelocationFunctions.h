//===- ARMRelocationFunction.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMRELOCATIONFUNCTIONS_H_
#define TARGET_ARM_ARMRELOCATIONFUNCTIONS_H_

#define DECL_ARM_APPLY_RELOC_FUNC(Name) \
  static ARMRelocator::Result Name(Relocation& pEntry, ARMRelocator& pParent);

#define DECL_ARM_APPLY_RELOC_FUNCS            \
  DECL_ARM_APPLY_RELOC_FUNC(none)             \
  DECL_ARM_APPLY_RELOC_FUNC(abs32)            \
  DECL_ARM_APPLY_RELOC_FUNC(rel32)            \
  DECL_ARM_APPLY_RELOC_FUNC(gotoff32)         \
  DECL_ARM_APPLY_RELOC_FUNC(base_prel)        \
  DECL_ARM_APPLY_RELOC_FUNC(got_brel)         \
  DECL_ARM_APPLY_RELOC_FUNC(call)             \
  DECL_ARM_APPLY_RELOC_FUNC(thm_call)         \
  DECL_ARM_APPLY_RELOC_FUNC(movw_prel_nc)     \
  DECL_ARM_APPLY_RELOC_FUNC(movw_abs_nc)      \
  DECL_ARM_APPLY_RELOC_FUNC(movt_abs)         \
  DECL_ARM_APPLY_RELOC_FUNC(movt_prel)        \
  DECL_ARM_APPLY_RELOC_FUNC(thm_movw_abs_nc)  \
  DECL_ARM_APPLY_RELOC_FUNC(thm_movw_prel_nc) \
  DECL_ARM_APPLY_RELOC_FUNC(thm_movw_brel)    \
  DECL_ARM_APPLY_RELOC_FUNC(thm_movt_abs)     \
  DECL_ARM_APPLY_RELOC_FUNC(thm_movt_prel)    \
  DECL_ARM_APPLY_RELOC_FUNC(prel31)           \
  DECL_ARM_APPLY_RELOC_FUNC(got_prel)         \
  DECL_ARM_APPLY_RELOC_FUNC(tls)              \
  DECL_ARM_APPLY_RELOC_FUNC(thm_jump8)        \
  DECL_ARM_APPLY_RELOC_FUNC(thm_jump11)       \
  DECL_ARM_APPLY_RELOC_FUNC(thm_jump19)       \
  DECL_ARM_APPLY_RELOC_FUNC(unsupported)

#define DECL_ARM_APPLY_RELOC_FUNC_PTRS                    \
  { &none,               0, "R_ARM_NONE"               }, \
  { &call,               1, "R_ARM_PC24"               }, \
  { &abs32,              2, "R_ARM_ABS32"              }, \
  { &rel32,              3, "R_ARM_REL32"              }, \
  { &unsupported,        4, "R_ARM_LDR_PC_G0"          }, \
  { &unsupported,        5, "R_ARM_ABS16"              }, \
  { &unsupported,        6, "R_ARM_ABS12"              }, \
  { &unsupported,        7, "R_ARM_THM_ABS5"           }, \
  { &unsupported,        8, "R_ARM_ABS8"               }, \
  { &unsupported,        9, "R_ARM_SBREL32"            }, \
  { &thm_call,          10, "R_ARM_THM_CALL"           }, \
  { &unsupported,       11, "R_ARM_THM_PC8"            }, \
  { &unsupported,       12, "R_ARM_BREL_ADJ"           }, \
  { &unsupported,       13, "R_ARM_TLS_DESC"           }, \
  { &unsupported,       14, "R_ARM_THM_SWI8"           }, \
  { &unsupported,       15, "R_ARM_XPC25"              }, \
  { &unsupported,       16, "R_ARM_THM_XPC22"          }, \
  { &unsupported,       17, "R_ARM_TLS_DTPMOD32"       }, \
  { &unsupported,       18, "R_ARM_TLS_DTPOFF32"       }, \
  { &unsupported,       19, "R_ARM_TLS_TPOFF32"        }, \
  { &unsupported,       20, "R_ARM_COPY"               }, \
  { &unsupported,       21, "R_ARM_GLOB_DAT"           }, \
  { &unsupported,       22, "R_ARM_JUMP_SLOT"          }, \
  { &unsupported,       23, "R_ARM_RELATIVE"           }, \
  { &gotoff32,          24, "R_ARM_GOTOFF32"           }, \
  { &base_prel,         25, "R_ARM_BASE_PREL"          }, \
  { &got_brel,          26, "R_ARM_GOT_BREL"           }, \
  { &call,              27, "R_ARM_PLT32"              }, \
  { &call,              28, "R_ARM_CALL"               }, \
  { &call,              29, "R_ARM_JUMP24"             }, \
  { &thm_call,          30, "R_ARM_THM_JUMP24"         }, \
  { &unsupported,       31, "R_ARM_BASE_ABS"           }, \
  { &unsupported,       32, "R_ARM_ALU_PCREL_7_0"      }, \
  { &unsupported,       33, "R_ARM_ALU_PCREL_15_8"     }, \
  { &unsupported,       34, "R_ARM_ALU_PCREL_23_15"    }, \
  { &unsupported,       35, "R_ARM_LDR_SBREL_11_0_NC"  }, \
  { &unsupported,       36, "R_ARM_ALU_SBREL_19_12_NC" }, \
  { &unsupported,       37, "R_ARM_ALU_SBREL_27_20_CK" }, \
  { &abs32,             38, "R_ARM_TARGET1"            }, \
  { &unsupported,       39, "R_ARM_SBREL31"            }, \
  { &none,              40, "R_ARM_V4BX"               }, \
  { &got_prel,          41, "R_ARM_TARGET2"            }, \
  { &prel31,            42, "R_ARM_PREL31"             }, \
  { &movw_abs_nc,       43, "R_ARM_MOVW_ABS_NC"        }, \
  { &movt_abs,          44, "R_ARM_MOVT_ABS"           }, \
  { &movw_prel_nc,      45, "R_ARM_MOVW_PREL_NC"       }, \
  { &movt_prel,         46, "R_ARM_MOVT_PREL"          }, \
  { &thm_movw_abs_nc,   47, "R_ARM_THM_MOVW_ABS_NC"    }, \
  { &thm_movt_abs,      48, "R_ARM_THM_MOVT_ABS"       }, \
  { &thm_movw_prel_nc,  49, "R_ARM_THM_MOVW_PREL_NC"   }, \
  { &thm_movt_prel,     50, "R_ARM_THM_MOVT_PREL"      }, \
  { &thm_jump19,        51, "R_ARM_THM_JUMP19"         }, \
  { &unsupported,       52, "R_ARM_THM_JUMP6"          }, \
  { &unsupported,       53, "R_ARM_THM_ALU_PREL_11_0"  }, \
  { &unsupported,       54, "R_ARM_THM_PC12"           }, \
  { &unsupported,       55, "R_ARM_ABS32_NOI"          }, \
  { &unsupported,       56, "R_ARM_REL32_NOI"          }, \
  { &unsupported,       57, "R_ARM_ALU_PC_G0_NC"       }, \
  { &unsupported,       58, "R_ARM_ALU_PC_G0"          }, \
  { &unsupported,       59, "R_ARM_ALU_PC_G1_NC"       }, \
  { &unsupported,       60, "R_ARM_ALU_PC_G1"          }, \
  { &unsupported,       61, "R_ARM_ALU_PC_G2"          }, \
  { &unsupported,       62, "R_ARM_LDR_PC_G1"          }, \
  { &unsupported,       63, "R_ARM_LDR_PC_G2"          }, \
  { &unsupported,       64, "R_ARM_LDRS_PC_G0"         }, \
  { &unsupported,       65, "R_ARM_LDRS_PC_G1"         }, \
  { &unsupported,       66, "R_ARM_LDRS_PC_G2"         }, \
  { &unsupported,       67, "R_ARM_LDC_PC_G0"          }, \
  { &unsupported,       68, "R_ARM_LDC_PC_G1"          }, \
  { &unsupported,       69, "R_ARM_LDC_PC_G2"          }, \
  { &unsupported,       70, "R_ARM_ALU_SB_G0_NC"       }, \
  { &unsupported,       71, "R_ARM_ALU_SB_G0"          }, \
  { &unsupported,       72, "R_ARM_ALU_SB_G1_NC"       }, \
  { &unsupported,       73, "R_ARM_ALU_SB_G1"          }, \
  { &unsupported,       74, "R_ARM_ALU_SB_G2"          }, \
  { &unsupported,       75, "R_ARM_LDR_SB_G0"          }, \
  { &unsupported,       76, "R_ARM_LDR_SB_G1"          }, \
  { &unsupported,       77, "R_ARM_LDR_SB_G2"          }, \
  { &unsupported,       78, "R_ARM_LDRS_SB_G0"         }, \
  { &unsupported,       79, "R_ARM_LDRS_SB_G1"         }, \
  { &unsupported,       80, "R_ARM_LDRS_SB_G2"         }, \
  { &unsupported,       81, "R_ARM_LDC_SB_G0"          }, \
  { &unsupported,       82, "R_ARM_LDC_SB_G1"          }, \
  { &unsupported,       83, "R_ARM_LDC_SB_G2"          }, \
  { &unsupported,       84, "R_ARM_MOVW_BREL_NC"       }, \
  { &unsupported,       85, "R_ARM_MOVT_BREL"          }, \
  { &unsupported,       86, "R_ARM_MOVW_BREL"          }, \
  { &thm_movw_brel,     87, "R_ARM_THM_MOVW_BREL_NC"   }, \
  { &thm_movt_prel,     88, "R_ARM_THM_MOVT_BREL"      }, \
  { &thm_movw_brel,     89, "R_ARM_THM_MOVW_BREL"      }, \
  { &unsupported,       90, "R_ARM_TLS_GOTDESC"        }, \
  { &unsupported,       91, "R_ARM_TLS_CALL"           }, \
  { &unsupported,       92, "R_ARM_TLS_DESCSEQ"        }, \
  { &unsupported,       93, "R_ARM_THM_TLS_CALL"       }, \
  { &unsupported,       94, "R_ARM_PLT32_ABS"          }, \
  { &unsupported,       95, "R_ARM_GOT_ABS"            }, \
  { &got_prel,          96, "R_ARM_GOT_PREL"           }, \
  { &unsupported,       97, "R_ARM_GOT_PREL12"         }, \
  { &unsupported,       98, "R_ARM_GOTOFF12"           }, \
  { &unsupported,       99, "R_ARM_GOTRELAX"           }, \
  { &unsupported,      100, "R_ARM_GNU_VTENTRY"        }, \
  { &unsupported,      101, "R_ARM_GNU_VTINERIT"       }, \
  { &thm_jump11,       102, "R_ARM_THM_JUMP11"         }, \
  { &thm_jump8,        103, "R_ARM_THM_JUMP8"          }, \
  { &tls,              104, "R_ARM_TLS_GD32"           }, \
  { &unsupported,      105, "R_ARM_TLS_LDM32"          }, \
  { &unsupported,      106, "R_ARM_TLS_LDO32"          }, \
  { &tls,              107, "R_ARM_TLS_IE32"           }, \
  { &tls,              108, "R_ARM_TLS_LE32"           }, \
  { &unsupported,      109, "R_ARM_TLS_LDO12"          }, \
  { &unsupported,      110, "R_ARM_TLS_LE12"           }, \
  { &unsupported,      111, "R_ARM_TLS_IE12GP"         }, \
  { &unsupported,      112, "R_ARM_PRIVATE_0"          }, \
  { &unsupported,      113, "R_ARM_PRIVATE_1"          }, \
  { &unsupported,      114, "R_ARM_PRIVATE_2"          }, \
  { &unsupported,      115, "R_ARM_PRIVATE_3"          }, \
  { &unsupported,      116, "R_ARM_PRIVATE_4"          }, \
  { &unsupported,      117, "R_ARM_PRIVATE_5"          }, \
  { &unsupported,      118, "R_ARM_PRIVATE_6"          }, \
  { &unsupported,      119, "R_ARM_PRIVATE_7"          }, \
  { &unsupported,      120, "R_ARM_PRIVATE_8"          }, \
  { &unsupported,      121, "R_ARM_PRIVATE_9"          }, \
  { &unsupported,      122, "R_ARM_PRIVATE_10"         }, \
  { &unsupported,      123, "R_ARM_PRIVATE_11"         }, \
  { &unsupported,      124, "R_ARM_PRIVATE_12"         }, \
  { &unsupported,      125, "R_ARM_PRIVATE_13"         }, \
  { &unsupported,      126, "R_ARM_PRIVATE_14"         }, \
  { &unsupported,      127, "R_ARM_PRIVATE_15"         }, \
  { &unsupported,      128, "R_ARM_ME_TOO"             }, \
  { &unsupported,      129, "R_ARM_THM_TLS_DESCSEQ16"  }, \
  { &unsupported,      130, "R_ARM_THM_TLS_DESCSEQ32"  }

#endif  // TARGET_ARM_ARMRELOCATIONFUNCTIONS_H_
