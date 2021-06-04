//===- X86RelocationFunction.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86RELOCATIONFUNCTIONS_H_
#define TARGET_X86_X86RELOCATIONFUNCTIONS_H_

#define DECL_X86_32_APPLY_RELOC_FUNC(Name)             \
  static X86Relocator::Result Name(Relocation& pEntry, \
                                   X86_32Relocator& pParent);

#define DECL_X86_32_APPLY_RELOC_FUNCS      \
  DECL_X86_32_APPLY_RELOC_FUNC(none)       \
  DECL_X86_32_APPLY_RELOC_FUNC(abs)        \
  DECL_X86_32_APPLY_RELOC_FUNC(rel)        \
  DECL_X86_32_APPLY_RELOC_FUNC(plt32)      \
  DECL_X86_32_APPLY_RELOC_FUNC(got32)      \
  DECL_X86_32_APPLY_RELOC_FUNC(gotoff32)   \
  DECL_X86_32_APPLY_RELOC_FUNC(gotpc32)    \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_gd)     \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_ie)     \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_gotie)  \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_le)     \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_ldm)    \
  DECL_X86_32_APPLY_RELOC_FUNC(tls_ldo_32) \
  DECL_X86_32_APPLY_RELOC_FUNC(unsupported)

#define DECL_X86_32_APPLY_RELOC_FUNC_PTRS           \
  { &none,          0, "R_386_NONE",          0  }, \
  { &abs,           1, "R_386_32",            32 }, \
  { &rel,           2, "R_386_PC32",          32 }, \
  { &got32,         3, "R_386_GOT32",         32 }, \
  { &plt32,         4, "R_386_PLT32",         32 }, \
  { &none,          5, "R_386_COPY",          0  }, \
  { &none,          6, "R_386_GLOB_DAT",      0  }, \
  { &none,          7, "R_386_JMP_SLOT",      0  }, \
  { &none,          8, "R_386_RELATIVE",      0  }, \
  { &gotoff32,      9, "R_386_GOTOFF",        32 }, \
  { &gotpc32,      10, "R_386_GOTPC",         32 }, \
  { &unsupported,  11, "R_386_32PLT",         0  }, \
  { &unsupported,  12, "",                    0  }, \
  { &unsupported,  13, "",                    0  }, \
  { &unsupported,  14, "R_386_TLS_TPOFF",     0  }, \
  { &tls_ie,       15, "R_386_TLS_IE",        32 }, \
  { &tls_gotie,    16, "R_386_TLS_GOTIE",     32 }, \
  { &tls_le,       17, "R_386_TLS_LE",        32 }, \
  { &tls_gd,       18, "R_386_TLS_GD",        32 }, \
  { &tls_ldm,      19, "R_386_TLS_LDM",       32 }, \
  { &abs,          20, "R_386_16",            16 }, \
  { &rel,          21, "R_386_PC16",          16 }, \
  { &abs,          22, "R_386_8",             8  }, \
  { &rel,          23, "R_386_PC8",           8  }, \
  { &unsupported,  24, "R_386_TLS_GD_32",     0  }, \
  { &unsupported,  25, "R_386_TLS_GD_PUSH",   0  }, \
  { &unsupported,  26, "R_386_TLS_GD_CALL",   0  }, \
  { &unsupported,  27, "R_386_TLS_GD_POP",    0  }, \
  { &unsupported,  28, "R_386_TLS_LDM_32",    0  }, \
  { &unsupported,  29, "R_386_TLS_LDM_PUSH",  0  }, \
  { &unsupported,  30, "R_386_TLS_LDM_CALL",  0  }, \
  { &unsupported,  31, "R_386_TLS_LDM_POP",   0  }, \
  { &tls_ldo_32,   32, "R_386_TLS_LDO_32",    32 }, \
  { &unsupported,  33, "R_386_TLS_IE_32",     0  }, \
  { &unsupported,  34, "R_386_TLS_LE_32",     0  }, \
  { &unsupported,  35, "R_386_TLS_DTPMOD32",  0  }, \
  { &unsupported,  36, "R_386_TLS_DTPOFF32",  0  }, \
  { &unsupported,  37, "R_386_TLS_TPOFF32",   0  }, \
  { &unsupported,  38, "",                    0  }, \
  { &unsupported,  39, "R_386_TLS_GOTDESC",   0  }, \
  { &unsupported,  40, "R_386_TLS_DESC_CALL", 0  }, \
  { &unsupported,  41, "R_386_TLS_DESC",      0  }, \
  { &unsupported,  42, "R_386_IRELATIVE",     0  }, \
  { &unsupported,  43, "R_386_NUM",           0  }, \
  { &none,         44, "R_386_TLS_OPT",       32 }

#define DECL_X86_64_APPLY_RELOC_FUNC(Name)             \
  static X86Relocator::Result Name(Relocation& pEntry, \
                                   X86_64Relocator& pParent);

#define DECL_X86_64_APPLY_RELOC_FUNCS    \
  DECL_X86_64_APPLY_RELOC_FUNC(none)     \
  DECL_X86_64_APPLY_RELOC_FUNC(abs)      \
  DECL_X86_64_APPLY_RELOC_FUNC(signed32) \
  DECL_X86_64_APPLY_RELOC_FUNC(gotpcrel) \
  DECL_X86_64_APPLY_RELOC_FUNC(plt32)    \
  DECL_X86_64_APPLY_RELOC_FUNC(rel)      \
  DECL_X86_64_APPLY_RELOC_FUNC(unsupported)

#define DECL_X86_64_APPLY_RELOC_FUNC_PTRS               \
  { &none,         0, "R_X86_64_NONE",            0  }, \
  { &abs,          1, "R_X86_64_64",              64 }, \
  { &rel,          2, "R_X86_64_PC32",            32 }, \
  { &unsupported,  3, "R_X86_64_GOT32",           32 }, \
  { &plt32,        4, "R_X86_64_PLT32",           32 }, \
  { &none,         5, "R_X86_64_COPY",            0  }, \
  { &none,         6, "R_X86_64_GLOB_DAT",        0  }, \
  { &none,         7, "R_X86_64_JMP_SLOT",        0  }, \
  { &none,         8, "R_X86_64_RELATIVE",        0  }, \
  { &gotpcrel,     9, "R_X86_64_GOTPCREL",        32 }, \
  { &abs,         10, "R_X86_64_32",              32 }, \
  { &signed32,    11, "R_X86_64_32S",             32 }, \
  { &abs,         12, "R_X86_64_16",              16 }, \
  { &rel,         13, "R_X86_64_PC16",            16 }, \
  { &abs,         14, "R_X86_64_8",               8  }, \
  { &rel,         15, "R_X86_64_PC8",             8  }, \
  { &none,        16, "R_X86_64_DTPMOD64",        0  }, \
  { &unsupported, 17, "R_X86_64_DTPOFF64",        0  }, \
  { &none,        18, "R_X86_64_TPOFF64",         0  }, \
  { &unsupported, 19, "R_X86_64_TLSGD",           0  }, \
  { &unsupported, 20, "R_X86_64_TLSLD",           0  }, \
  { &unsupported, 21, "R_X86_64_DTPOFF32",        0  }, \
  { &unsupported, 22, "R_X86_64_GOTTPOFF",        0  }, \
  { &unsupported, 23, "R_X86_64_TPOFF32",         0  }, \
  { &unsupported, 24, "R_X86_64_PC64",            64 }, \
  { &unsupported, 25, "R_X86_64_GOTOFF64",        64 }, \
  { &unsupported, 26, "R_X86_64_GOTPC32",         32 }, \
  { &unsupported, 27, "R_X86_64_GOT64",           64 }, \
  { &unsupported, 28, "R_X86_64_GOTPCREL64",      64 }, \
  { &unsupported, 29, "R_X86_64_GOTPC64",         64 }, \
  { &unsupported, 30, "R_X86_64_GOTPLT64",        64 }, \
  { &unsupported, 31, "R_X86_64_PLTOFF64",        64 }, \
  { &unsupported, 32, "R_X86_64_SIZE32",          32 }, \
  { &unsupported, 33, "R_X86_64_SIZE64",          64 }, \
  { &unsupported, 34, "R_X86_64_GOTPC32_TLSDESC", 0  }, \
  { &unsupported, 35, "R_X86_64_TLSDESC_CALL",    0  }, \
  { &none,        36, "R_X86_64_TLSDESC",         0  }, \
  { &none,        37, "R_X86_64_IRELATIVE",       0  }, \
  { &none,        38, "R_X86_64_RELATIVE64",      0  }

#endif  // TARGET_X86_X86RELOCATIONFUNCTIONS_H_
