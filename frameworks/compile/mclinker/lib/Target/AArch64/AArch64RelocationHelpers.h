//===- AArch64RelocationHelpers.h -----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64RELOCATIONHELPERS_H_
#define TARGET_AARCH64_AARCH64RELOCATIONHELPERS_H_

#include "AArch64Relocator.h"
#include <llvm/Support/Host.h>

namespace mcld {
//===----------------------------------------------------------------------===//
// Relocation helper functions
//===----------------------------------------------------------------------===//
// Return true if overflow
static inline bool helper_check_signed_overflow(Relocator::DWord pValue,
                                                unsigned bits) {
  if (bits >= sizeof(int64_t) * 8)
    return false;
  int64_t signed_val = static_cast<int64_t>(pValue);
  int64_t max = (1 << (bits - 1)) - 1;
  int64_t min = -(1 << (bits - 1));
  if (signed_val > max || signed_val < min)
    return true;
  return false;
}

static inline Relocator::Address helper_get_page_address(
    Relocator::Address pValue) {
  return (pValue & ~(Relocator::Address)0xFFF);
}

static inline Relocator::Address helper_get_page_offset(
    Relocator::Address pValue) {
  return (pValue & (Relocator::Address)0xFFF);
}

static inline uint32_t get_mask(uint32_t pValue) {
  return ((1u << (pValue)) - 1);
}

static inline uint32_t helper_reencode_adr_imm(uint32_t pInst, uint32_t pImm) {
  return (pInst & ~((get_mask(2) << 29) | (get_mask(19) << 5))) |
         ((pImm & get_mask(2)) << 29) | ((pImm & (get_mask(19) << 2)) << 3);
}

// Reencode the imm field of add immediate.
static inline uint32_t helper_reencode_add_imm(uint32_t pInst, uint32_t pImm) {
  return (pInst & ~(get_mask(12) << 10)) | ((pImm & get_mask(12)) << 10);
}

// Encode the 26-bit offset of unconditional branch.
static inline uint32_t helper_reencode_branch_offset_26(uint32_t pInst,
                                                        uint32_t pOff) {
  return (pInst & ~get_mask(26)) | (pOff & get_mask(26));
}

// Encode the 19-bit offset of conditional branch and compare & branch.
static inline uint32_t helper_reencode_cond_branch_ofs_19(uint32_t pInst,
                                                          uint32_t pOff) {
  return (pInst & ~(get_mask(19) << 5)) | ((pOff & get_mask(19)) << 5);
}

// Reencode the imm field of ld/st pos immediate.
static inline uint32_t helper_reencode_ldst_pos_imm(uint32_t pInst,
                                                    uint32_t pImm) {
  return (pInst & ~(get_mask(12) << 10)) | ((pImm & get_mask(12)) << 10);
}

static inline uint32_t helper_get_upper32(Relocator::DWord pData) {
  if (llvm::sys::IsLittleEndianHost)
    return pData >> 32;
  return pData & 0xFFFFFFFF;
}

static inline void helper_put_upper32(uint32_t pData, Relocator::DWord& pDes) {
  *(reinterpret_cast<uint32_t*>(&pDes)) = pData;
}

static inline Relocator::Address helper_get_PLT_address(
    ResolveInfo& pSym,
    AArch64Relocator& pParent) {
  PLTEntryBase* plt_entry = pParent.getSymPLTMap().lookUp(pSym);
  assert(plt_entry != NULL);
  return pParent.getTarget().getPLT().addr() + plt_entry->getOffset();
}

static inline AArch64PLT1& helper_PLT_init(Relocation& pReloc,
                                           AArch64Relocator& pParent) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  AArch64GNULDBackend& ld_backend = pParent.getTarget();
  assert(pParent.getSymPLTMap().lookUp(*rsym) == NULL);

  AArch64PLT1* plt_entry = ld_backend.getPLT().create();
  pParent.getSymPLTMap().record(*rsym, *plt_entry);

  // initialize plt and the corresponding gotplt and dyn rel entry.
  assert(pParent.getSymGOTPLTMap().lookUp(*rsym) == NULL &&
         "PLT entry not exist, but DynRel entry exist!");
  AArch64GOTEntry* gotplt_entry = ld_backend.getGOTPLT().createGOTPLT();
  pParent.getSymGOTPLTMap().record(*rsym, *gotplt_entry);

  // init the corresponding rel entry in .rela.plt
  Relocation& rel_entry = *ld_backend.getRelaPLT().create();
  rel_entry.setType(llvm::ELF::R_AARCH64_JUMP_SLOT);
  rel_entry.targetRef().assign(*gotplt_entry);
  rel_entry.setSymInfo(rsym);
  return *plt_entry;
}

/// helper_DynRel - Get an relocation entry in .rela.dyn
static inline Relocation& helper_DynRela_init(ResolveInfo* pSym,
                                              Fragment& pFrag,
                                              uint64_t pOffset,
                                              Relocator::Type pType,
                                              AArch64Relocator& pParent) {
  AArch64GNULDBackend& ld_backend = pParent.getTarget();
  Relocation& rel_entry = *ld_backend.getRelaDyn().create();
  rel_entry.setType(pType);
  rel_entry.targetRef().assign(pFrag, pOffset);
  if (pType == llvm::ELF::R_AARCH64_RELATIVE || pSym == NULL)
    rel_entry.setSymInfo(NULL);
  else
    rel_entry.setSymInfo(pSym);

  return rel_entry;
}

/// helper_use_relative_reloc - Check if symbol can use relocation
/// R_AARCH64_RELATIVE
static inline bool helper_use_relative_reloc(const ResolveInfo& pSym,
                                             const AArch64Relocator& pParent) {
  // if symbol is dynamic or undefine or preemptible
  if (pSym.isDyn() || pSym.isUndef() ||
      pParent.getTarget().isSymbolPreemptible(pSym))
    return false;
  return true;
}

static inline Relocator::Address helper_get_GOT_address(
    ResolveInfo& pSym,
    AArch64Relocator& pParent) {
  AArch64GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(pSym);
  assert(got_entry != NULL);
  return pParent.getTarget().getGOT().addr() + got_entry->getOffset();
}

static inline Relocator::Address helper_GOT_ORG(AArch64Relocator& pParent) {
  return pParent.getTarget().getGOT().addr();
}

static inline AArch64GOTEntry& helper_GOT_init(Relocation& pReloc,
                                               bool pHasRel,
                                               AArch64Relocator& pParent) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  AArch64GNULDBackend& ld_backend = pParent.getTarget();
  assert(pParent.getSymGOTMap().lookUp(*rsym) == NULL);

  AArch64GOTEntry* got_entry = ld_backend.getGOT().createGOT();
  pParent.getSymGOTMap().record(*rsym, *got_entry);

  // If we first get this GOT entry, we should initialize it.
  if (!pHasRel) {
    // No corresponding dynamic relocation, initialize to the symbol value.
    got_entry->setValue(AArch64Relocator::SymVal);
  } else {
    // Initialize got_entry content and the corresponding dynamic relocation.
    if (helper_use_relative_reloc(*rsym, pParent)) {
      got_entry->setValue(AArch64Relocator::SymVal);
      Relocation& rel_entry = helper_DynRela_init(
          rsym, *got_entry, 0x0, llvm::ELF::R_AARCH64_RELATIVE, pParent);
      rel_entry.setAddend(AArch64Relocator::SymVal);
      pParent.getRelRelMap().record(pReloc, rel_entry);
    } else {
      helper_DynRela_init(rsym, *got_entry, 0x0, llvm::ELF::R_AARCH64_GLOB_DAT,
                          pParent);
      got_entry->setValue(0);
    }
  }
  return *got_entry;
}

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64RELOCATIONHELPERS_H_
