//===- MipsRelocationFunction.h -------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSRELOCATIONFUNCTIONS_H_
#define TARGET_MIPS_MIPSRELOCATIONFUNCTIONS_H_

#define DECL_MIPS_APPLY_RELOC_FUNC(Name)                        \
  static MipsRelocator::Result Name(MipsRelocationInfo& pReloc, \
                                    MipsRelocator& pParent);

#define DECL_MIPS_APPLY_RELOC_FUNCS   \
  DECL_MIPS_APPLY_RELOC_FUNC(none)    \
  DECL_MIPS_APPLY_RELOC_FUNC(abs32)   \
  DECL_MIPS_APPLY_RELOC_FUNC(rel26)   \
  DECL_MIPS_APPLY_RELOC_FUNC(hi16)    \
  DECL_MIPS_APPLY_RELOC_FUNC(lo16)    \
  DECL_MIPS_APPLY_RELOC_FUNC(gprel16) \
  DECL_MIPS_APPLY_RELOC_FUNC(got16)   \
  DECL_MIPS_APPLY_RELOC_FUNC(pc16)    \
  DECL_MIPS_APPLY_RELOC_FUNC(call16)  \
  DECL_MIPS_APPLY_RELOC_FUNC(gprel32) \
  DECL_MIPS_APPLY_RELOC_FUNC(abs64)   \
  DECL_MIPS_APPLY_RELOC_FUNC(gotdisp) \
  DECL_MIPS_APPLY_RELOC_FUNC(gotoff)  \
  DECL_MIPS_APPLY_RELOC_FUNC(gothi16) \
  DECL_MIPS_APPLY_RELOC_FUNC(gotlo16) \
  DECL_MIPS_APPLY_RELOC_FUNC(sub)     \
  DECL_MIPS_APPLY_RELOC_FUNC(jalr)    \
  DECL_MIPS_APPLY_RELOC_FUNC(pc32)    \
  DECL_MIPS_APPLY_RELOC_FUNC(pc18_s3) \
  DECL_MIPS_APPLY_RELOC_FUNC(pc21_s2) \
  DECL_MIPS_APPLY_RELOC_FUNC(pc19_s2) \
  DECL_MIPS_APPLY_RELOC_FUNC(pc26_s2) \
  DECL_MIPS_APPLY_RELOC_FUNC(pchi16) \
  DECL_MIPS_APPLY_RELOC_FUNC(pclo16) \
  DECL_MIPS_APPLY_RELOC_FUNC(tlshi16) \
  DECL_MIPS_APPLY_RELOC_FUNC(tlslo16) \
  DECL_MIPS_APPLY_RELOC_FUNC(tlsgot)  \
  DECL_MIPS_APPLY_RELOC_FUNC(unsupported)

#define DECL_MIPS_APPLY_RELOC_FUNC_PTRS \
  { &none,          0, "R_MIPS_NONE",                  0}, \
  { &unsupported,   1, "R_MIPS_16",                   16}, \
  { &abs32,         2, "R_MIPS_32",                   32}, \
  { &unsupported,   3, "R_MIPS_REL32",                32}, \
  { &rel26,         4, "R_MIPS_26",                   26}, \
  { &hi16,          5, "R_MIPS_HI16",                 16}, \
  { &lo16,          6, "R_MIPS_LO16",                 16}, \
  { &gprel16,       7, "R_MIPS_GPREL16",              16}, \
  { &unsupported,   8, "R_MIPS_LITERAL",              16}, \
  { &got16,         9, "R_MIPS_GOT16",                16}, \
  { &pc16,         10, "R_MIPS_PC16",                 16}, \
  { &call16,       11, "R_MIPS_CALL16",               16}, \
  { &gprel32,      12, "R_MIPS_GPREL32",              32}, \
  { &none,         13, "R_MIPS_UNUSED1",               0}, \
  { &none,         14, "R_MIPS_UNUSED2",               0}, \
  { &none,         15, "R_MIPS_UNUSED3",               0}, \
  { &unsupported,  16, "R_MIPS_SHIFT5",               32}, \
  { &unsupported,  17, "R_MIPS_SHIFT6",               32}, \
  { &abs64,        18, "R_MIPS_64",                   64}, \
  { &gotdisp,      19, "R_MIPS_GOT_DISP",             16}, \
  { &gotdisp,      20, "R_MIPS_GOT_PAGE",             16}, \
  { &gotoff,       21, "R_MIPS_GOT_OFST",             16}, \
  { &gothi16,      22, "R_MIPS_GOT_HI16",             16}, \
  { &gotlo16,      23, "R_MIPS_GOT_LO16",             16}, \
  { &sub,          24, "R_MIPS_SUB",                  64}, \
  { &unsupported,  25, "R_MIPS_INSERT_A",              0}, \
  { &unsupported,  26, "R_MIPS_INSERT_B",              0}, \
  { &unsupported,  27, "R_MIPS_DELETE",                0}, \
  { &unsupported,  28, "R_MIPS_HIGHER",               16}, \
  { &unsupported,  29, "R_MIPS_HIGHEST",              16}, \
  { &gothi16,      30, "R_MIPS_CALL_HI16",            16}, \
  { &gotlo16,      31, "R_MIPS_CALL_LO16",            16}, \
  { &unsupported,  32, "R_MIPS_SCN_DISP",             32}, \
  { &unsupported,  33, "R_MIPS_REL16",                 0}, \
  { &unsupported,  34, "R_MIPS_ADD_IMMEDIATE",         0}, \
  { &unsupported,  35, "R_MIPS_PJUMP",                 0}, \
  { &unsupported,  36, "R_MIPS_RELGOT",                0}, \
  { &jalr,         37, "R_MIPS_JALR",                 32}, \
  { &unsupported,  38, "R_MIPS_TLS_DTPMOD32",         32}, \
  { &unsupported,  39, "R_MIPS_TLS_DTPREL32",         32}, \
  { &unsupported,  40, "R_MIPS_TLS_DTPMOD64",          0}, \
  { &unsupported,  41, "R_MIPS_TLS_DTPREL64",          0}, \
  { &tlsgot,       42, "R_MIPS_TLS_GD",               16}, \
  { &tlsgot,       43, "R_MIPS_TLS_LDM",              16}, \
  { &tlshi16,      44, "R_MIPS_TLS_DTPREL_HI16",      16}, \
  { &tlslo16,      45, "R_MIPS_TLS_DTPREL_LO16",      16}, \
  { &tlsgot,       46, "R_MIPS_TLS_GOTTPREL",         16}, \
  { &unsupported,  47, "R_MIPS_TLS_TPREL32",          32}, \
  { &unsupported,  48, "R_MIPS_TLS_TPREL64",           0}, \
  { &tlshi16,      49, "R_MIPS_TLS_TPREL_HI16",       16}, \
  { &tlslo16,      50, "R_MIPS_TLS_TPREL_LO16",       16}, \
  { &unsupported,  51, "R_MIPS_GLOB_DAT",              0}, \
  { &unsupported,  52, "",                             0}, \
  { &unsupported,  53, "",                             0}, \
  { &unsupported,  54, "",                             0}, \
  { &unsupported,  55, "",                             0}, \
  { &unsupported,  56, "",                             0}, \
  { &unsupported,  57, "",                             0}, \
  { &unsupported,  58, "",                             0}, \
  { &unsupported,  59, "",                             0}, \
  { &pc21_s2,      60, "R_MIPS_PC21_S2",              21}, \
  { &pc26_s2,      61, "R_MIPS_PC26_S2",              26}, \
  { &pc18_s3,      62, "R_MIPS_PC18_S3",              18}, \
  { &pc19_s2,      63, "R_MIPS_PC19_S2",              19}, \
  { &pchi16,       64, "R_MIPS_PCHI16",               16}, \
  { &pclo16,       65, "R_MIPS_PCLO16",               16}, \
  { &unsupported,  66, "",                             0}, \
  { &unsupported,  67, "",                             0}, \
  { &unsupported,  68, "",                             0}, \
  { &unsupported,  69, "",                             0}, \
  { &unsupported,  70, "",                             0}, \
  { &unsupported,  71, "",                             0}, \
  { &unsupported,  72, "",                             0}, \
  { &unsupported,  73, "",                             0}, \
  { &unsupported,  74, "",                             0}, \
  { &unsupported,  75, "",                             0}, \
  { &unsupported,  76, "",                             0}, \
  { &unsupported,  77, "",                             0}, \
  { &unsupported,  78, "",                             0}, \
  { &unsupported,  79, "",                             0}, \
  { &unsupported,  80, "",                             0}, \
  { &unsupported,  81, "",                             0}, \
  { &unsupported,  82, "",                             0}, \
  { &unsupported,  83, "",                             0}, \
  { &unsupported,  84, "",                             0}, \
  { &unsupported,  85, "",                             0}, \
  { &unsupported,  86, "",                             0}, \
  { &unsupported,  87, "",                             0}, \
  { &unsupported,  88, "",                             0}, \
  { &unsupported,  89, "",                             0}, \
  { &unsupported,  90, "",                             0}, \
  { &unsupported,  91, "",                             0}, \
  { &unsupported,  92, "",                             0}, \
  { &unsupported,  93, "",                             0}, \
  { &unsupported,  94, "",                             0}, \
  { &unsupported,  95, "",                             0}, \
  { &unsupported,  96, "",                             0}, \
  { &unsupported,  97, "",                             0}, \
  { &unsupported,  98, "",                             0}, \
  { &unsupported,  99, "",                             0}, \
  { &unsupported, 100, "R_MIPS16_26",                  0}, \
  { &unsupported, 101, "R_MIPS16_GPREL",               0}, \
  { &unsupported, 102, "R_MIPS16_GOT16",               0}, \
  { &unsupported, 103, "R_MIPS16_CALL16",              0}, \
  { &unsupported, 104, "R_MIPS16_HI16",                0}, \
  { &unsupported, 105, "R_MIPS16_LO16",                0}, \
  { &unsupported, 106, "R_MIPS16_TLS_GD",              0}, \
  { &unsupported, 107, "R_MIPS16_TLS_LDM",             0}, \
  { &unsupported, 108, "R_MIPS16_TLS_DTPREL_HI16",     0}, \
  { &unsupported, 109, "R_MIPS16_TLS_DTPREL_LO16",     0}, \
  { &unsupported, 110, "R_MIPS16_TLS_GOTTPREL",        0}, \
  { &unsupported, 111, "R_MIPS16_TLS_TPREL_HI16",      0}, \
  { &unsupported, 112, "R_MIPS16_TLS_TPREL_LO16",      0}, \
  { &unsupported, 113, "",                             0}, \
  { &unsupported, 114, "",                             0}, \
  { &unsupported, 115, "",                             0}, \
  { &unsupported, 116, "",                             0}, \
  { &unsupported, 117, "",                             0}, \
  { &unsupported, 118, "",                             0}, \
  { &unsupported, 119, "",                             0}, \
  { &unsupported, 120, "",                             0}, \
  { &unsupported, 121, "",                             0}, \
  { &unsupported, 122, "",                             0}, \
  { &unsupported, 123, "",                             0}, \
  { &unsupported, 124, "",                             0}, \
  { &unsupported, 125, "",                             0}, \
  { &unsupported, 126, "R_MIPS_COPY",                  0}, \
  { &unsupported, 127, "R_MIPS_JUMP_SLOT",             0}, \
  { &unsupported, 128, "",                             0}, \
  { &unsupported, 129, "",                             0}, \
  { &unsupported, 130, "",                             0}, \
  { &unsupported, 131, "",                             0}, \
  { &unsupported, 132, "",                             0}, \
  { &unsupported, 133, "R_MICROMIPS_26_S1",            0}, \
  { &unsupported, 134, "R_MICROMIPS_HI16",             0}, \
  { &unsupported, 135, "R_MICROMIPS_LO16",             0}, \
  { &unsupported, 136, "R_MICROMIPS_GPREL16",          0}, \
  { &unsupported, 137, "R_MICROMIPS_LITERAL",          0}, \
  { &unsupported, 138, "R_MICROMIPS_GOT16",            0}, \
  { &unsupported, 139, "R_MICROMIPS_PC7_S1",           0}, \
  { &unsupported, 140, "R_MICROMIPS_PC10_S1",          0}, \
  { &unsupported, 141, "R_MICROMIPS_PC16_S1",          0}, \
  { &unsupported, 142, "R_MICROMIPS_CALL16",           0}, \
  { &unsupported, 143, "R_MICROMIPS_GOT_DISP",         0}, \
  { &unsupported, 144, "R_MICROMIPS_GOT_PAGE",         0}, \
  { &unsupported, 145, "R_MICROMIPS_GOT_OFST",         0}, \
  { &unsupported, 146, "R_MICROMIPS_GOT_HI16",         0}, \
  { &unsupported, 147, "R_MICROMIPS_GOT_LO16",         0}, \
  { &unsupported, 148, "R_MICROMIPS_SUB",              0}, \
  { &unsupported, 149, "R_MICROMIPS_HIGHER",           0}, \
  { &unsupported, 150, "R_MICROMIPS_HIGHEST",          0}, \
  { &unsupported, 151, "R_MICROMIPS_CALL_HI16",        0}, \
  { &unsupported, 152, "R_MICROMIPS_CALL_LO16",        0}, \
  { &unsupported, 153, "R_MICROMIPS_SCN_DISP",         0}, \
  { &unsupported, 154, "R_MICROMIPS_JALR",             0}, \
  { &unsupported, 155, "R_MICROMIPS_HI0_LO16",         0}, \
  { &unsupported, 156, "",                             0}, \
  { &unsupported, 157, "",                             0}, \
  { &unsupported, 158, "",                             0}, \
  { &unsupported, 159, "",                             0}, \
  { &unsupported, 160, "",                             0}, \
  { &unsupported, 161, "",                             0}, \
  { &unsupported, 162, "R_MICROMIPS_TLS_GD",           0}, \
  { &unsupported, 163, "R_MICROMIPS_TLS_LDM",          0}, \
  { &unsupported, 164, "R_MICROMIPS_TLS_DTPREL_HI16",  0}, \
  { &unsupported, 165, "R_MICROMIPS_TLS_DTPREL_LO16",  0}, \
  { &unsupported, 166, "R_MICROMIPS_TLS_GOTTPREL",     0}, \
  { &unsupported, 167, "",                             0}, \
  { &unsupported, 168, "",                             0}, \
  { &unsupported, 169, "R_MICROMIPS_TLS_TPREL_HI16",   0}, \
  { &unsupported, 170, "R_MICROMIPS_TLS_TPREL_LO16",   0}, \
  { &unsupported, 171, "",                             0}, \
  { &unsupported, 172, "R_MICROMIPS_GPREL7_S2",        0}, \
  { &unsupported, 173, "R_MICROMIPS_PC23_S2",          0}, \
  { &unsupported, 174, "",                             0}, \
  { &unsupported, 175, "",                             0}, \
  { &unsupported, 176, "",                             0}, \
  { &unsupported, 177, "",                             0}, \
  { &unsupported, 178, "",                             0}, \
  { &unsupported, 179, "",                             0}, \
  { &unsupported, 180, "",                             0}, \
  { &unsupported, 181, "",                             0}, \
  { &unsupported, 182, "",                             0}, \
  { &unsupported, 183, "",                             0}, \
  { &unsupported, 184, "",                             0}, \
  { &unsupported, 185, "",                             0}, \
  { &unsupported, 186, "",                             0}, \
  { &unsupported, 187, "",                             0}, \
  { &unsupported, 188, "",                             0}, \
  { &unsupported, 189, "",                             0}, \
  { &unsupported, 190, "",                             0}, \
  { &unsupported, 191, "",                             0}, \
  { &unsupported, 192, "",                             0}, \
  { &unsupported, 193, "",                             0}, \
  { &unsupported, 194, "",                             0}, \
  { &unsupported, 195, "",                             0}, \
  { &unsupported, 196, "",                             0}, \
  { &unsupported, 197, "",                             0}, \
  { &unsupported, 198, "",                             0}, \
  { &unsupported, 199, "",                             0}, \
  { &unsupported, 200, "",                             0}, \
  { &unsupported, 201, "",                             0}, \
  { &unsupported, 202, "",                             0}, \
  { &unsupported, 203, "",                             0}, \
  { &unsupported, 204, "",                             0}, \
  { &unsupported, 205, "",                             0}, \
  { &unsupported, 206, "",                             0}, \
  { &unsupported, 207, "",                             0}, \
  { &unsupported, 208, "",                             0}, \
  { &unsupported, 209, "",                             0}, \
  { &unsupported, 210, "",                             0}, \
  { &unsupported, 211, "",                             0}, \
  { &unsupported, 212, "",                             0}, \
  { &unsupported, 213, "",                             0}, \
  { &unsupported, 214, "",                             0}, \
  { &unsupported, 215, "",                             0}, \
  { &unsupported, 216, "",                             0}, \
  { &unsupported, 217, "",                             0}, \
  { &unsupported, 218, "",                             0}, \
  { &unsupported, 219, "",                             0}, \
  { &unsupported, 220, "",                             0}, \
  { &unsupported, 221, "",                             0}, \
  { &unsupported, 222, "",                             0}, \
  { &unsupported, 223, "",                             0}, \
  { &unsupported, 224, "",                             0}, \
  { &unsupported, 225, "",                             0}, \
  { &unsupported, 226, "",                             0}, \
  { &unsupported, 227, "",                             0}, \
  { &unsupported, 228, "",                             0}, \
  { &unsupported, 229, "",                             0}, \
  { &unsupported, 230, "",                             0}, \
  { &unsupported, 231, "",                             0}, \
  { &unsupported, 232, "",                             0}, \
  { &unsupported, 233, "",                             0}, \
  { &unsupported, 234, "",                             0}, \
  { &unsupported, 235, "",                             0}, \
  { &unsupported, 236, "",                             0}, \
  { &unsupported, 237, "",                             0}, \
  { &unsupported, 238, "",                             0}, \
  { &unsupported, 239, "",                             0}, \
  { &unsupported, 240, "",                             0}, \
  { &unsupported, 241, "",                             0}, \
  { &unsupported, 242, "",                             0}, \
  { &unsupported, 243, "",                             0}, \
  { &unsupported, 244, "",                             0}, \
  { &unsupported, 245, "",                             0}, \
  { &unsupported, 246, "",                             0}, \
  { &unsupported, 247, "",                             0}, \
  { &pc32,        248, "R_MIPS_PC32",                 32}, \
  { &unsupported, 249, "",                             0}, \
  { &unsupported, 250, "R_MIPS_GNU_REL16_S2",          0}, \
  { &unsupported, 251, "",                             0}, \
  { &unsupported, 252, "",                             0}, \
  { &unsupported, 253, "R_MIPS_GNU_VTINHERIT",         0}, \
  { &unsupported, 254, "R_MIPS_GNU_VTENTRY",           0}

#endif  // TARGET_MIPS_MIPSRELOCATIONFUNCTIONS_H_
