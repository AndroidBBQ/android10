//===- MipsAbiFlags.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "MipsAbiFlags.h"

#include "mcld/Fragment/RegionFragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/MipsABIFlags.h>

namespace mcld {

// SHT_MIPS_ABIFLAGS has the same format for both 32/64-bit targets.
// We do not support linking of big-endian code now so 32-bit LE
// combination is Okay.
typedef llvm::object::ELFType<llvm::support::little, false> ELF32LE;
typedef llvm::object::Elf_Mips_ABIFlags<ELF32LE> ElfMipsAbiFlags;

uint64_t MipsAbiFlags::size() {
  return sizeof(ElfMipsAbiFlags);
}

uint64_t MipsAbiFlags::emit(const MipsAbiFlags& pInfo, MemoryRegion& pRegion) {
  auto* buf = reinterpret_cast<ElfMipsAbiFlags*>(pRegion.begin());
  buf->version = 0;
  buf->isa_level = pInfo.m_IsaLevel;
  buf->isa_rev = pInfo.m_IsaRev;
  buf->gpr_size = pInfo.m_GprSize;
  buf->cpr1_size = pInfo.m_Cpr1Size;
  buf->cpr2_size = pInfo.m_Cpr2Size;
  buf->fp_abi = pInfo.m_FpAbi;
  buf->isa_ext = pInfo.m_IsaExt;
  buf->ases = pInfo.m_Ases;
  buf->flags1 = pInfo.m_Flags1;
  buf->flags2 = 0;
  return size();
}

bool MipsAbiFlags::fillBySection(const Input& pInput, const LDSection& pSection,
                                 MipsAbiFlags& mipsAbi) {
  assert(pSection.type() == llvm::ELF::SHT_MIPS_ABIFLAGS &&
         "Unexpected section type");

  if (pSection.size() != size()) {
    error(diag::error_Mips_abiflags_invalid_size) << pInput.name();
    return false;
  }

  const SectionData* secData = pSection.getSectionData();
  if (secData->size() != 2 || !llvm::isa<RegionFragment>(secData->front())) {
    error(diag::error_Mips_abiflags_invalid_size) << pInput.name();
    return false;
  }

  const auto& frag = llvm::cast<RegionFragment>(secData->front());
  auto* data =
      reinterpret_cast<const ElfMipsAbiFlags*>(frag.getRegion().data());
  if (data->version != 0) {
    error(diag::error_Mips_abiflags_invalid_version) << int(data->version)
                                                     << pInput.name();
    return false;
  }

  mipsAbi.m_IsaLevel = data->isa_level;
  mipsAbi.m_IsaRev = data->isa_rev;
  mipsAbi.m_GprSize = data->gpr_size;
  mipsAbi.m_Cpr1Size = data->cpr1_size;
  mipsAbi.m_Cpr2Size = data->cpr2_size;
  mipsAbi.m_FpAbi = data->fp_abi;
  mipsAbi.m_IsaExt = data->isa_ext;
  mipsAbi.m_Ases = data->ases;
  mipsAbi.m_Flags1 = data->flags1;
  return true;
}

static unsigned getIsaLevel(uint64_t flags) {
  switch (flags & llvm::ELF::EF_MIPS_ARCH) {
    case llvm::ELF::EF_MIPS_ARCH_1:
      return 1;
    case llvm::ELF::EF_MIPS_ARCH_2:
      return 2;
    case llvm::ELF::EF_MIPS_ARCH_3:
      return 3;
    case llvm::ELF::EF_MIPS_ARCH_4:
      return 4;
    case llvm::ELF::EF_MIPS_ARCH_5:
      return 5;
    case llvm::ELF::EF_MIPS_ARCH_32:
    case llvm::ELF::EF_MIPS_ARCH_32R2:
    case llvm::ELF::EF_MIPS_ARCH_32R6:
      return 32;
    case llvm::ELF::EF_MIPS_ARCH_64:
    case llvm::ELF::EF_MIPS_ARCH_64R2:
    case llvm::ELF::EF_MIPS_ARCH_64R6:
      return 64;
    default:
      // We check ELF flags and show error in case
      // of unknown value in other place.
      llvm_unreachable("Unknown MIPS architecture flag");
  }
}

static unsigned getIsaRev(uint64_t flags) {
  switch (flags & llvm::ELF::EF_MIPS_ARCH) {
    case llvm::ELF::EF_MIPS_ARCH_1:
    case llvm::ELF::EF_MIPS_ARCH_2:
    case llvm::ELF::EF_MIPS_ARCH_3:
    case llvm::ELF::EF_MIPS_ARCH_4:
    case llvm::ELF::EF_MIPS_ARCH_5:
      return 0;
    case llvm::ELF::EF_MIPS_ARCH_32:
    case llvm::ELF::EF_MIPS_ARCH_64:
      return 1;
    case llvm::ELF::EF_MIPS_ARCH_32R2:
    case llvm::ELF::EF_MIPS_ARCH_64R2:
      return 2;
    case llvm::ELF::EF_MIPS_ARCH_32R6:
    case llvm::ELF::EF_MIPS_ARCH_64R6:
      return 6;
    default:
      // We check ELF flags and show error in case
      // of unknown value in other place.
      llvm_unreachable("Unknown MIPS architecture flag");
  }
}

static unsigned getIsaExt(uint64_t flags) {
  switch (flags & llvm::ELF::EF_MIPS_MACH) {
    case 0:
      return llvm::Mips::AFL_EXT_NONE;
    case llvm::ELF::EF_MIPS_MACH_3900: return llvm::Mips::AFL_EXT_3900;
    case llvm::ELF::EF_MIPS_MACH_4010: return llvm::Mips::AFL_EXT_4010;
    case llvm::ELF::EF_MIPS_MACH_4100: return llvm::Mips::AFL_EXT_4010;
    case llvm::ELF::EF_MIPS_MACH_4111: return llvm::Mips::AFL_EXT_4111;
    case llvm::ELF::EF_MIPS_MACH_4120: return llvm::Mips::AFL_EXT_4120;
    case llvm::ELF::EF_MIPS_MACH_4650: return llvm::Mips::AFL_EXT_4650;
    case llvm::ELF::EF_MIPS_MACH_5400: return llvm::Mips::AFL_EXT_5400;
    case llvm::ELF::EF_MIPS_MACH_5500: return llvm::Mips::AFL_EXT_5500;
    case llvm::ELF::EF_MIPS_MACH_5900: return llvm::Mips::AFL_EXT_5900;
    case llvm::ELF::EF_MIPS_MACH_SB1: return llvm::Mips::AFL_EXT_SB1;
    case llvm::ELF::EF_MIPS_MACH_LS2E: return llvm::Mips::AFL_EXT_LOONGSON_2E;
    case llvm::ELF::EF_MIPS_MACH_LS2F: return llvm::Mips::AFL_EXT_LOONGSON_2F;
    case llvm::ELF::EF_MIPS_MACH_LS3A: return llvm::Mips::AFL_EXT_LOONGSON_3A;
    case llvm::ELF::EF_MIPS_MACH_OCTEON3: return llvm::Mips::AFL_EXT_OCTEON3;
    case llvm::ELF::EF_MIPS_MACH_OCTEON2: return llvm::Mips::AFL_EXT_OCTEON2;
    case llvm::ELF::EF_MIPS_MACH_OCTEON: return llvm::Mips::AFL_EXT_OCTEON;
    case llvm::ELF::EF_MIPS_MACH_XLR: return llvm::Mips::AFL_EXT_XLR;
    default:
      // We check ELF flags and show error in case
      // of unknown value in other place.
      llvm_unreachable("Unknown MIPS extension flag");
  }
}

static bool is32BitElfFlags(uint64_t flags) {
  if (flags & llvm::ELF::EF_MIPS_32BITMODE)
    return true;

  uint64_t arch = flags & llvm::ELF::EF_MIPS_ARCH;
  if (arch == llvm::ELF::EF_MIPS_ARCH_1 ||
      arch == llvm::ELF::EF_MIPS_ARCH_2 ||
      arch == llvm::ELF::EF_MIPS_ARCH_32 ||
      arch == llvm::ELF::EF_MIPS_ARCH_32R2 ||
      arch == llvm::ELF::EF_MIPS_ARCH_32R6)
    return true;

  uint64_t abi = flags & llvm::ELF::EF_MIPS_ABI;
  if (abi == llvm::ELF::EF_MIPS_ABI_O32 || abi == llvm::ELF::EF_MIPS_ABI_EABI32)
    return true;

  return false;
}

bool MipsAbiFlags::fillByElfFlags(const Input& pInput, uint64_t elfFlags,
                                  MipsAbiFlags& mipsAbi) {
  mipsAbi.m_IsaLevel = getIsaLevel(elfFlags);
  mipsAbi.m_IsaRev = getIsaRev(elfFlags);
  mipsAbi.m_IsaExt = getIsaExt(elfFlags);

  mipsAbi.m_GprSize = is32BitElfFlags(elfFlags) ?
                      llvm::Mips::AFL_REG_32 : llvm::Mips::AFL_REG_64;

  mipsAbi.m_Cpr1Size = llvm::Mips::AFL_REG_NONE;
  mipsAbi.m_Cpr2Size = llvm::Mips::AFL_REG_NONE;
  mipsAbi.m_FpAbi = llvm::Mips::Val_GNU_MIPS_ABI_FP_ANY;

  mipsAbi.m_Ases = 0;
  if (elfFlags & llvm::ELF::EF_MIPS_MICROMIPS)
    mipsAbi.m_Ases |= llvm::Mips::AFL_ASE_MICROMIPS;
  if (elfFlags & llvm::ELF::EF_MIPS_ARCH_ASE_M16)
    mipsAbi.m_Ases |= llvm::Mips::AFL_ASE_MIPS16;
  if (elfFlags & llvm::ELF::EF_MIPS_ARCH_ASE_MDMX)
    mipsAbi.m_Ases |= llvm::Mips::AFL_ASE_MDMX;

  mipsAbi.m_Flags1 = 0;
  return true;
}

bool MipsAbiFlags::isCompatible(const Input& pInput, const MipsAbiFlags& elf,
                                const MipsAbiFlags& abi) {
  unsigned isaRev = abi.m_IsaRev;
  if (isaRev == 3 || isaRev == 5)
    isaRev = 2;
  if (abi.m_IsaLevel != elf.m_IsaLevel || isaRev != elf.m_IsaRev) {
    warning(diag::warn_Mips_isa_incompatible) << pInput.name();
    return false;
  }
  if (abi.m_IsaExt != elf.m_IsaExt) {
    warning(diag::warn_Mips_isa_ext_incompatible) << pInput.name();
    return false;
  }
  if ((abi.m_Ases & elf.m_Ases) != elf.m_Ases) {
    warning(diag::warn_Mips_ases_incompatible) << pInput.name();
    return false;
  }
  return true;
}

static bool isFpGreater(uint64_t fpA, uint64_t fpB) {
  if (fpB == llvm::Mips::Val_GNU_MIPS_ABI_FP_ANY)
    return true;
  if (fpB == llvm::Mips::Val_GNU_MIPS_ABI_FP_64A &&
      fpA == llvm::Mips::Val_GNU_MIPS_ABI_FP_64)
    return true;
  if (fpB != llvm::Mips::Val_GNU_MIPS_ABI_FP_XX)
    return false;
  return fpA == llvm::Mips::Val_GNU_MIPS_ABI_FP_DOUBLE ||
         fpA == llvm::Mips::Val_GNU_MIPS_ABI_FP_64 ||
         fpA == llvm::Mips::Val_GNU_MIPS_ABI_FP_64A;
}

static llvm::StringRef getFpAbiName(uint64_t abi) {
  switch (abi) {
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_ANY:
      return "<any>";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_DOUBLE:
      return "-mdouble-float";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_SINGLE:
      return "-msingle-float";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_SOFT:
      return "-msoft-float";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_OLD_64:
      return "-mips32r2 -mfp64 (old)";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_XX:
      return "-mfpxx";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_64:
      return "-mgp32 -mfp64";
    case llvm::Mips::Val_GNU_MIPS_ABI_FP_64A:
      return "-mgp32 -mfp64 -mno-odd-spreg";
    default:
      return "<unknown>";
  }
}

bool MipsAbiFlags::merge(const Input& pInput, MipsAbiFlags& oldFlags,
                         const MipsAbiFlags& newFlags) {
  if (oldFlags.m_IsaLevel == 0) {
    oldFlags = newFlags;
    return true;
  }

  if (newFlags.m_IsaLevel > oldFlags.m_IsaLevel)
    oldFlags.m_IsaLevel = newFlags.m_IsaLevel;

  oldFlags.m_IsaRev = std::max(oldFlags.m_IsaRev, newFlags.m_IsaRev);
  oldFlags.m_GprSize = std::max(oldFlags.m_GprSize, newFlags.m_GprSize);
  oldFlags.m_Cpr1Size = std::max(oldFlags.m_Cpr1Size, newFlags.m_Cpr1Size);
  oldFlags.m_Cpr2Size = std::max(oldFlags.m_Cpr2Size, newFlags.m_Cpr2Size);
  oldFlags.m_Ases |= newFlags.m_Ases;
  oldFlags.m_Flags1 |= newFlags.m_Flags1;

  if (oldFlags.m_FpAbi == newFlags.m_FpAbi)
    return true;

  if (isFpGreater(newFlags.m_FpAbi, oldFlags.m_FpAbi)) {
    oldFlags.m_FpAbi = newFlags.m_FpAbi;
    return true;
  }

  if (isFpGreater(oldFlags.m_FpAbi, newFlags.m_FpAbi))
    return true;

  llvm::StringRef oldAbiName = getFpAbiName(oldFlags.m_FpAbi);
  llvm::StringRef newAbiName = getFpAbiName(newFlags.m_FpAbi);
  warning(diag::warn_Mips_fp_abi_incompatible) << oldAbiName << newAbiName
                                               << pInput.name();
  return false;
}
}  // namespace mcld
