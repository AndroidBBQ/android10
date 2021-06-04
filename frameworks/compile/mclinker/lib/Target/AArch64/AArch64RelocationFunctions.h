//===- AArch64RelocationFunction.h ----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64RELOCATIONFUNCTIONS_H_
#define TARGET_AARCH64_AARCH64RELOCATIONFUNCTIONS_H_

#define DECL_AARCH64_APPLY_RELOC_FUNC(Name)                \
  static AArch64Relocator::Result Name(Relocation& pEntry, \
                                       AArch64Relocator& pParent);

#define DECL_AARCH64_APPLY_RELOC_FUNCS            \
  DECL_AARCH64_APPLY_RELOC_FUNC(none)             \
  DECL_AARCH64_APPLY_RELOC_FUNC(abs)              \
  DECL_AARCH64_APPLY_RELOC_FUNC(rel)              \
  DECL_AARCH64_APPLY_RELOC_FUNC(call)             \
  DECL_AARCH64_APPLY_RELOC_FUNC(condbr)           \
  DECL_AARCH64_APPLY_RELOC_FUNC(adr_prel_lo21)    \
  DECL_AARCH64_APPLY_RELOC_FUNC(adr_prel_pg_hi21) \
  DECL_AARCH64_APPLY_RELOC_FUNC(add_abs_lo12)     \
  DECL_AARCH64_APPLY_RELOC_FUNC(adr_got_page)     \
  DECL_AARCH64_APPLY_RELOC_FUNC(ld64_got_lo12)    \
  DECL_AARCH64_APPLY_RELOC_FUNC(ldst_abs_lo12)    \
  DECL_AARCH64_APPLY_RELOC_FUNC(unsupported)

#define DECL_AARCH64_APPLY_RELOC_FUNC_PTRS(ValueType, MappedType)                              /* NOLINT */\
  ValueType(0x0,   MappedType(&none,             "R_AARCH64_NULL",                        0)), /* NOLINT */\
  ValueType(0x1,   MappedType(&none,             "R_AARCH64_REWRITE_INSN",               32)), /* NOLINT */\
  ValueType(0x100, MappedType(&none,             "R_AARCH64_NONE",                        0)), /* NOLINT */\
  ValueType(0x101, MappedType(&abs,              "R_AARCH64_ABS64",                      64)), /* NOLINT */\
  ValueType(0x102, MappedType(&abs,              "R_AARCH64_ABS32",                      32)), /* NOLINT */\
  ValueType(0x103, MappedType(&abs,              "R_AARCH64_ABS16",                      16)), /* NOLINT */\
  ValueType(0x104, MappedType(&rel,              "R_AARCH64_PREL64",                     64)), /* NOLINT */\
  ValueType(0x105, MappedType(&rel,              "R_AARCH64_PREL32",                     32)), /* NOLINT */\
  ValueType(0x106, MappedType(&rel,              "R_AARCH64_PREL16",                     16)), /* NOLINT */\
  ValueType(0x107, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G0",                0)), /* NOLINT */\
  ValueType(0x108, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G0_NC",             0)), /* NOLINT */\
  ValueType(0x109, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G1",                0)), /* NOLINT */\
  ValueType(0x10a, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G1_NC",             0)), /* NOLINT */\
  ValueType(0x10b, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G2",                0)), /* NOLINT */\
  ValueType(0x10c, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G2_NC",             0)), /* NOLINT */\
  ValueType(0x10d, MappedType(&unsupported,      "R_AARCH64_MOVW_UABS_G3",                0)), /* NOLINT */\
  ValueType(0x10e, MappedType(&unsupported,      "R_AARCH64_MOVW_SABS_G0",                0)), /* NOLINT */\
  ValueType(0x10f, MappedType(&unsupported,      "R_AARCH64_MOVW_SABS_G1",                0)), /* NOLINT */\
  ValueType(0x110, MappedType(&unsupported,      "R_AARCH64_MOVW_SABS_G2",                0)), /* NOLINT */\
  ValueType(0x111, MappedType(&unsupported,      "R_AARCH64_LD_PREL_LO19",                0)), /* NOLINT */\
  ValueType(0x112, MappedType(&adr_prel_lo21,    "R_AARCH64_ADR_PREL_LO21",              32)), /* NOLINT */\
  ValueType(0x113, MappedType(&adr_prel_pg_hi21, "R_AARCH64_ADR_PREL_PG_HI21",           32)), /* NOLINT */\
  ValueType(0x114, MappedType(&adr_prel_pg_hi21, "R_AARCH64_ADR_PREL_PG_HI21_NC",        32)), /* NOLINT */\
  ValueType(0x115, MappedType(&add_abs_lo12,     "R_AARCH64_ADD_ABS_LO12_NC",            32)), /* NOLINT */\
  ValueType(0x116, MappedType(&ldst_abs_lo12,    "R_AARCH64_LDST8_ABS_LO12_NC",          32)), /* NOLINT */\
  ValueType(0x117, MappedType(&unsupported,      "R_AARCH64_TSTBR14",                     0)), /* NOLINT */\
  ValueType(0x118, MappedType(&condbr,           "R_AARCH64_CONDBR19",                   32)), /* NOLINT */\
  ValueType(0x11a, MappedType(&call,             "R_AARCH64_JUMP26",                     32)), /* NOLINT */\
  ValueType(0x11b, MappedType(&call,             "R_AARCH64_CALL26",                     32)), /* NOLINT */\
  ValueType(0x11c, MappedType(&ldst_abs_lo12,    "R_AARCH64_LDST16_ABS_LO12_NC",         32)), /* NOLINT */\
  ValueType(0x11d, MappedType(&ldst_abs_lo12,    "R_AARCH64_LDST32_ABS_LO12_NC",         32)), /* NOLINT */\
  ValueType(0x11e, MappedType(&ldst_abs_lo12,    "R_AARCH64_LDST64_ABS_LO12_NC",         32)), /* NOLINT */\
  ValueType(0x12b, MappedType(&ldst_abs_lo12,    "R_AARCH64_LDST128_ABS_LO12_NC",        32)), /* NOLINT */\
  ValueType(0x137, MappedType(&adr_got_page,     "R_AARCH64_ADR_GOT_PAGE",               32)), /* NOLINT */\
  ValueType(0x138, MappedType(&ld64_got_lo12,    "R_AARCH64_LD64_GOT_LO12_NC",           32)), /* NOLINT */\
  ValueType(0x20b, MappedType(&unsupported,      "R_AARCH64_TLSLD_MOVW_DTPREL_G2",        0)), /* NOLINT */\
  ValueType(0x20c, MappedType(&unsupported,      "R_AARCH64_TLSLD_MOVW_DTPREL_G1",        0)), /* NOLINT */\
  ValueType(0x20d, MappedType(&unsupported,      "R_AARCH64_TLSLD_MOVW_DTPREL_G1_NC",     0)), /* NOLINT */\
  ValueType(0x20e, MappedType(&unsupported,      "R_AARCH64_TLSLD_MOVW_DTPREL_G0",        0)), /* NOLINT */\
  ValueType(0x20f, MappedType(&unsupported,      "R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC",     0)), /* NOLINT */\
  ValueType(0x210, MappedType(&unsupported,      "R_AARCH64_TLSLD_ADD_DTPREL_HI12",       0)), /* NOLINT */\
  ValueType(0x211, MappedType(&unsupported,      "R_AARCH64_TLSLD_ADD_DTPREL_LO12",       0)), /* NOLINT */\
  ValueType(0x212, MappedType(&unsupported,      "R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC",    0)), /* NOLINT */\
  ValueType(0x213, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST8_DTPREL_LO12",     0)), /* NOLINT */\
  ValueType(0x214, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC",  0)), /* NOLINT */\
  ValueType(0x215, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST16_DTPREL_LO12",    0)), /* NOLINT */\
  ValueType(0x216, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC", 0)), /* NOLINT */\
  ValueType(0x217, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST32_DTPREL_LO12",    0)), /* NOLINT */\
  ValueType(0x218, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC", 0)), /* NOLINT */\
  ValueType(0x219, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST64_DTPREL_LO12",    0)), /* NOLINT */\
  ValueType(0x21a, MappedType(&unsupported,      "R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC", 0)), /* NOLINT */\
  ValueType(0x21b, MappedType(&unsupported,      "R_AARCH64_TLSIE_MOVW_GOTTPREL_G1",      0)), /* NOLINT */\
  ValueType(0x21c, MappedType(&unsupported,      "R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC",   0)), /* NOLINT */\
  ValueType(0x21d, MappedType(&unsupported,      "R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21",   0)), /* NOLINT */\
  ValueType(0x21e, MappedType(&unsupported,      "R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC", 0)), /* NOLINT */\
  ValueType(0x21f, MappedType(&unsupported,      "R_AARCH64_TLSIE_LD_GOTTPREL_PREL19",    0)), /* NOLINT */\
  ValueType(0x220, MappedType(&unsupported,      "R_AARCH64_TLSLE_MOVW_TPREL_G2",         0)), /* NOLINT */\
  ValueType(0x221, MappedType(&unsupported,      "R_AARCH64_TLSLE_MOVW_TPREL_G1",         0)), /* NOLINT */\
  ValueType(0x222, MappedType(&unsupported,      "R_AARCH64_TLSLE_MOVW_TPREL_G1_NC",      0)), /* NOLINT */\
  ValueType(0x223, MappedType(&unsupported,      "R_AARCH64_TLSLE_MOVW_TPREL_G0",         0)), /* NOLINT */\
  ValueType(0x224, MappedType(&unsupported,      "R_AARCH64_TLSLE_MOVW_TPREL_G0_NC",      0)), /* NOLINT */\
  ValueType(0x225, MappedType(&unsupported,      "R_AARCH64_TLSLE_ADD_TPREL_HI12",        0)), /* NOLINT */\
  ValueType(0x226, MappedType(&unsupported,      "R_AARCH64_TLSLE_ADD_TPREL_LO12",        0)), /* NOLINT */\
  ValueType(0x227, MappedType(&unsupported,      "R_AARCH64_TLSLE_ADD_TPREL_LO12_NC",     0)), /* NOLINT */\
  ValueType(0x228, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST8_TPREL_LO12",      0)), /* NOLINT */\
  ValueType(0x229, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC",   0)), /* NOLINT */\
  ValueType(0x22a, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST16_TPREL_LO12",     0)), /* NOLINT */\
  ValueType(0x22b, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC",  0)), /* NOLINT */\
  ValueType(0x22c, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST32_TPREL_LO12",     0)), /* NOLINT */\
  ValueType(0x22d, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC",  0)), /* NOLINT */\
  ValueType(0x22e, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST64_TPREL_LO12",     0)), /* NOLINT */\
  ValueType(0x22f, MappedType(&unsupported,      "R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC",  0)), /* NOLINT */\
  ValueType(0x232, MappedType(&unsupported,      "R_AARCH64_TLSDESC_ADR_PAGE",            0)), /* NOLINT */\
  ValueType(0x233, MappedType(&unsupported,      "R_AARCH64_TLSDESC_LD64_LO12_NC",        0)), /* NOLINT */\
  ValueType(0x234, MappedType(&unsupported,      "R_AARCH64_TLSDESC_ADD_LO12_NC",         0)), /* NOLINT */\
  ValueType(0x239, MappedType(&unsupported,      "R_AARCH64_TLSDESC_CALL",                0)), /* NOLINT */\
  ValueType(1024,  MappedType(&unsupported,      "R_AARCH64_COPY",                        0)), /* NOLINT */\
  ValueType(1025,  MappedType(&unsupported,      "R_AARCH64_GLOB_DAT",                    0)), /* NOLINT */\
  ValueType(1026,  MappedType(&unsupported,      "R_AARCH64_JUMP_SLOT",                   0)), /* NOLINT */\
  ValueType(1027,  MappedType(&unsupported,      "R_AARCH64_RELATIVE",                    0)), /* NOLINT */\
  ValueType(1028,  MappedType(&unsupported,      "R_AARCH64_TLS_DTPREL64",                0)), /* NOLINT */\
  ValueType(1029,  MappedType(&unsupported,      "R_AARCH64_TLS_DTPMOD64",                0)), /* NOLINT */\
  ValueType(1030,  MappedType(&unsupported,      "R_AARCH64_TLS_TPREL64",                 0)), /* NOLINT */\
  ValueType(1031,  MappedType(&unsupported,      "R_AARCH64_TLSDESC",                     0)), /* NOLINT */\
  ValueType(1032,  MappedType(&unsupported,      "R_AARCH64_IRELATIVE",                   0))  /* NOLINT */

#endif  // TARGET_AARCH64_AARCH64RELOCATIONFUNCTIONS_H_
