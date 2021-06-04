//===- ARMRelocator.cpp  --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ARMRelocator.h"
#include "ARMRelocationFunctions.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

namespace mcld {

//=========================================//
// Relocation helper function              //
//=========================================//
static Relocator::DWord getThumbBit(const Relocation& pReloc) {
  // Set thumb bit if
  // - symbol has type of STT_FUNC, is defined and with bit 0 of its value set
  Relocator::DWord thumbBit =
      ((!pReloc.symInfo()->isUndef() || pReloc.symInfo()->isDyn()) &&
       (pReloc.symInfo()->type() == ResolveInfo::Function) &&
       ((pReloc.symValue() & 0x1) != 0))
          ? 1
          : 0;
  return thumbBit;
}

// Using uint64_t to make sure those complicate operations won't cause
// undefined behavior.
static uint64_t helper_sign_extend(uint64_t pVal, uint64_t pOri_width) {
  assert(pOri_width <= 64);
  if (pOri_width == 64)
    return pVal;

  uint64_t mask = (~((uint64_t)0)) >> (64 - pOri_width);
  pVal &= mask;
  // Reverse sign bit, then subtract sign bit.
  uint64_t sign_bit = 1 << (pOri_width - 1);
  return (pVal ^ sign_bit) - sign_bit;
}

static uint64_t helper_bit_select(uint64_t pA, uint64_t pB, uint64_t pMask) {
  return (pA & ~pMask) | (pB & pMask);
}

// Check if symbol can use relocation R_ARM_RELATIVE
static bool helper_use_relative_reloc(const ResolveInfo& pSym,
                                      const ARMRelocator& pFactory) {
  // if symbol is dynamic or undefine or preemptible
  if (pSym.isDyn() || pSym.isUndef() ||
      pFactory.getTarget().isSymbolPreemptible(pSym))
    return false;
  return true;
}

// Strip LSB (THUMB bit) if "S" is a THUMB target.
static inline void helper_clear_thumb_bit(Relocator::DWord& pValue) {
  pValue &= (~0x1);
}

static Relocator::Address helper_get_GOT_address(ResolveInfo& pSym,
                                                 ARMRelocator& pParent) {
  ARMGOTEntry* got_entry = pParent.getSymGOTMap().lookUp(pSym);
  assert(got_entry != NULL);
  return pParent.getTarget().getGOT().addr() + got_entry->getOffset();
}

static ARMGOTEntry& helper_GOT_init(Relocation& pReloc,
                                    bool pHasRel,
                                    ARMRelocator& pParent) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  ARMGNULDBackend& ld_backend = pParent.getTarget();
  assert(pParent.getSymGOTMap().lookUp(*rsym) == NULL);

  ARMGOTEntry* got_entry = ld_backend.getGOT().createGOT();
  pParent.getSymGOTMap().record(*rsym, *got_entry);
  // If we first get this GOT entry, we should initialize it.
  if (!pHasRel) {
    // No corresponding dynamic relocation, initialize to the symbol value.
    got_entry->setValue(ARMRelocator::SymVal);
  } else {
    // Initialize corresponding dynamic relocation.
    Relocation& rel_entry = *ld_backend.getRelDyn().create();
    if (rsym->isLocal() || helper_use_relative_reloc(*rsym, pParent)) {
      // Initialize got entry to target symbol address
      got_entry->setValue(ARMRelocator::SymVal);
      rel_entry.setType(llvm::ELF::R_ARM_RELATIVE);
      rel_entry.setSymInfo(NULL);
    } else {
      // Initialize got entry to 0 for corresponding dynamic relocation.
      got_entry->setValue(0);
      rel_entry.setType(llvm::ELF::R_ARM_GLOB_DAT);
      rel_entry.setSymInfo(rsym);
    }
    rel_entry.targetRef().assign(*got_entry);
  }
  return *got_entry;
}

static Relocator::Address helper_GOT_ORG(ARMRelocator& pParent) {
  return pParent.getTarget().getGOT().addr();
}

static Relocator::Address helper_get_PLT_address(ResolveInfo& pSym,
                                                 ARMRelocator& pParent) {
  ARMPLT1* plt_entry = pParent.getSymPLTMap().lookUp(pSym);
  assert(plt_entry != NULL);
  return pParent.getTarget().getPLT().addr() + plt_entry->getOffset();
}

static ARMPLT1& helper_PLT_init(Relocation& pReloc, ARMRelocator& pParent) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  ARMGNULDBackend& ld_backend = pParent.getTarget();
  assert(pParent.getSymPLTMap().lookUp(*rsym) == NULL);

  // initialize the plt and the corresponding gotplt and dyn relocation
  ARMPLT1* plt_entry = ld_backend.getPLT().create();
  pParent.getSymPLTMap().record(*rsym, *plt_entry);

  assert(pParent.getSymGOTPLTMap().lookUp(*rsym) == NULL &&
         "PLT entry not exist, but DynRel entry exist!");
  ARMGOTEntry* gotplt_entry = ld_backend.getGOT().createGOTPLT();
  pParent.getSymGOTPLTMap().record(*rsym, *gotplt_entry);

  Relocation& rel_entry = *ld_backend.getRelPLT().create();
  rel_entry.setType(llvm::ELF::R_ARM_JUMP_SLOT);
  rel_entry.targetRef().assign(*gotplt_entry);
  rel_entry.setSymInfo(rsym);

  return *plt_entry;
}

// Get an relocation entry in .rel.dyn and set its type to pType,
// its FragmentRef to pReloc->targetFrag() and its ResolveInfo to
// pReloc->symInfo()
static void helper_DynRel_init(Relocation& pReloc,
                               Relocator::Type pType,
                               ARMRelocator& pParent) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  ARMGNULDBackend& ld_backend = pParent.getTarget();

  Relocation& rel_entry = *ld_backend.getRelDyn().create();
  rel_entry.setType(pType);
  rel_entry.targetRef() = pReloc.targetRef();

  if (pType == llvm::ELF::R_ARM_RELATIVE)
    rel_entry.setSymInfo(NULL);
  else
    rel_entry.setSymInfo(rsym);
}

static Relocator::DWord helper_extract_movw_movt_addend(
    Relocator::DWord pTarget) {
  // imm16: [19-16][11-0]
  return helper_sign_extend((((pTarget >> 4)) & 0xf000U) | (pTarget & 0xfffU),
                            16);
}

static Relocator::DWord helper_insert_val_movw_movt_inst(
    Relocator::DWord pTarget,
    Relocator::DWord pImm) {
  // imm16: [19-16][11-0]
  pTarget &= 0xfff0f000U;
  pTarget |= pImm & 0x0fffU;
  pTarget |= (pImm & 0xf000U) << 4;
  return pTarget;
}

static Relocator::DWord helper_extract_thumb_movw_movt_addend(
    Relocator::DWord pValue) {
  // imm16: [19-16][26][14-12][7-0]
  return helper_sign_extend(
      (((pValue >> 4) & 0xf000U) | ((pValue >> 15) & 0x0800U) |
       ((pValue >> 4) & 0x0700U) | (pValue & 0x00ffU)),
      16);
}

static Relocator::DWord helper_insert_val_thumb_movw_movt_inst(
    Relocator::DWord pValue,
    Relocator::DWord pImm) {
  // imm16: [19-16][26][14-12][7-0]
  pValue &= 0xfbf08f00U;
  pValue |= (pImm & 0xf000U) << 4;
  pValue |= (pImm & 0x0800U) << 15;
  pValue |= (pImm & 0x0700U) << 4;
  pValue |= (pImm & 0x00ffU);
  return pValue;
}

static Relocator::DWord helper_thumb32_branch_offset(
    Relocator::DWord pUpper16,
    Relocator::DWord pLower16) {
  Relocator::DWord s = (pUpper16 & (1U << 10)) >> 10,  // 26 bit
      u = pUpper16 & 0x3ffU,                           // 25-16
      l = pLower16 & 0x7ffU,                           // 10-0
      j1 = (pLower16 & (1U << 13)) >> 13,              // 13
      j2 = (pLower16 & (1U << 11)) >> 11;              // 11

  Relocator::DWord i1 = j1 ^ s ? 0 : 1, i2 = j2 ^ s ? 0 : 1;

  // [31-25][24][23][22][21-12][11-1][0]
  //      0   s  i1  i2      u     l  0
  return helper_sign_extend(
      (s << 24) | (i1 << 23) | (i2 << 22) | (u << 12) | (l << 1), 25);
}

static Relocator::DWord helper_thumb32_branch_upper(Relocator::DWord pUpper16,
                                                    Relocator::DWord pOffset) {
  uint32_t sign = ((pOffset & 0x80000000U) >> 31);
  return (pUpper16 & ~0x7ffU) | ((pOffset >> 12) & 0x3ffU) | (sign << 10);
}

static Relocator::DWord helper_thumb32_branch_lower(Relocator::DWord pLower16,
                                                    Relocator::DWord pOffset) {
  uint32_t sign = ((pOffset & 0x80000000U) >> 31);
  return ((pLower16 & ~0x2fffU) | ((((pOffset >> 23) & 1) ^ !sign) << 13) |
          ((((pOffset >> 22) & 1) ^ !sign) << 11) | ((pOffset >> 1) & 0x7ffU));
}

static Relocator::DWord helper_thumb32_cond_branch_offset(
    Relocator::DWord pUpper16,
    Relocator::DWord pLower16) {
  uint32_t s = (pUpper16 & 0x0400U) >> 10;
  uint32_t j1 = (pLower16 & 0x2000U) >> 13;
  uint32_t j2 = (pLower16 & 0x0800U) >> 11;
  uint32_t lower = (pLower16 & 0x07ffU);
  uint32_t upper = (s << 8) | (j2 << 7) | (j1 << 6) | (pUpper16 & 0x003fU);
  return helper_sign_extend((upper << 12) | (lower << 1), 21);
}

static Relocator::DWord helper_thumb32_cond_branch_upper(
    Relocator::DWord pUpper16,
    Relocator::DWord pOffset) {
  uint32_t sign = ((pOffset & 0x80000000U) >> 31);
  return (pUpper16 & 0xfbc0U) | (sign << 10) | ((pOffset & 0x0003f000U) >> 12);
}

static Relocator::DWord helper_thumb32_cond_branch_lower(
    Relocator::DWord pLower16,
    Relocator::DWord pOffset) {
  uint32_t j2 = (pOffset & 0x00080000U) >> 19;
  uint32_t j1 = (pOffset & 0x00040000U) >> 18;
  uint32_t lo = (pOffset & 0x00000ffeU) >> 1;
  return (pLower16 & 0xd000U) | (j1 << 13) | (j2 << 11) | lo;
}

// Return true if overflow
static bool helper_check_signed_overflow(Relocator::DWord pValue,
                                         unsigned bits) {
  int32_t signed_val = static_cast<int32_t>(pValue);
  int32_t max = (1 << (bits - 1)) - 1;
  int32_t min = -(1 << (bits - 1));
  if (signed_val > max || signed_val < min) {
    return true;
  } else {
    return false;
  }
}

//===----------------------------------------------------------------------===//
// Relocation Functions and Tables
//===----------------------------------------------------------------------===//
DECL_ARM_APPLY_RELOC_FUNCS

/// the prototype of applying function
typedef Relocator::Result (*ApplyFunctionType)(Relocation& pReloc,
                                               ARMRelocator& pParent);

// the table entry of applying functions
struct ApplyFunctionTriple {
  ApplyFunctionType func;
  unsigned int type;
  const char* name;
};

// declare the table of applying functions
static const ApplyFunctionTriple ApplyFunctions[] = {
    DECL_ARM_APPLY_RELOC_FUNC_PTRS};

//===----------------------------------------------------------------------===//
// ARMRelocator
//===----------------------------------------------------------------------===//
ARMRelocator::ARMRelocator(ARMGNULDBackend& pParent,
                           const LinkerConfig& pConfig)
    : Relocator(pConfig), m_Target(pParent) {
}

ARMRelocator::~ARMRelocator() {
}

Relocator::Result ARMRelocator::applyRelocation(Relocation& pRelocation) {
  Relocation::Type type = pRelocation.type();
  if (type > 130) {  // 131-255 doesn't noted in ARM spec
    return Relocator::Unknown;
  }

  return ApplyFunctions[type].func(pRelocation, *this);
}

const char* ARMRelocator::getName(Relocator::Type pType) const {
  return ApplyFunctions[pType].name;
}

Relocator::Size ARMRelocator::getSize(Relocation::Type pType) const {
  return 32;
}

void ARMRelocator::addCopyReloc(ResolveInfo& pSym) {
  Relocation& rel_entry = *getTarget().getRelDyn().create();
  rel_entry.setType(llvm::ELF::R_ARM_COPY);
  assert(pSym.outSymbol()->hasFragRef());
  rel_entry.targetRef().assign(*pSym.outSymbol()->fragRef());
  rel_entry.setSymInfo(&pSym);
}

/// defineSymbolForCopyReloc
/// For a symbol needing copy relocation, define a copy symbol in the BSS
/// section and all other reference to this symbol should refer to this
/// copy.
/// This is executed at scan relocation stage.
LDSymbol& ARMRelocator::defineSymbolforCopyReloc(IRBuilder& pBuilder,
                                                 const ResolveInfo& pSym) {
  // get or create corresponding BSS LDSection
  LDSection* bss_sect_hdr = NULL;
  ELFFileFormat* file_format = getTarget().getOutputFormat();
  if (ResolveInfo::ThreadLocal == pSym.type())
    bss_sect_hdr = &file_format->getTBSS();
  else
    bss_sect_hdr = &file_format->getBSS();

  // get or create corresponding BSS SectionData
  SectionData* bss_data = NULL;
  if (bss_sect_hdr->hasSectionData())
    bss_data = bss_sect_hdr->getSectionData();
  else
    bss_data = IRBuilder::CreateSectionData(*bss_sect_hdr);

  // Determine the alignment by the symbol value
  // FIXME: here we use the largest alignment
  uint32_t addralign = config().targets().bitclass() / 8;

  // allocate space in BSS for the copy symbol
  Fragment* frag = new FillFragment(0x0, 1, pSym.size());
  uint64_t size = ObjectBuilder::AppendFragment(*frag, *bss_data, addralign);
  bss_sect_hdr->setSize(bss_sect_hdr->size() + size);

  // change symbol binding to Global if it's a weak symbol
  ResolveInfo::Binding binding = (ResolveInfo::Binding)pSym.binding();
  if (binding == ResolveInfo::Weak)
    binding = ResolveInfo::Global;

  // Define the copy symbol in the bss section and resolve it
  LDSymbol* cpy_sym = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      pSym.name(),
      (ResolveInfo::Type)pSym.type(),
      ResolveInfo::Define,
      binding,
      pSym.size(),  // size
      0x0,          // value
      FragmentRef::Create(*frag, 0x0),
      (ResolveInfo::Visibility)pSym.other());
  return *cpy_sym;
}

/// checkValidReloc - When we attempt to generate a dynamic relocation for
/// ouput file, check if the relocation is supported by dynamic linker.
void ARMRelocator::checkValidReloc(Relocation& pReloc) const {
  // If not PIC object, no relocation type is invalid
  if (!config().isCodeIndep())
    return;

  switch (pReloc.type()) {
    case llvm::ELF::R_ARM_RELATIVE:
    case llvm::ELF::R_ARM_COPY:
    case llvm::ELF::R_ARM_GLOB_DAT:
    case llvm::ELF::R_ARM_JUMP_SLOT:
    case llvm::ELF::R_ARM_ABS32:
    case llvm::ELF::R_ARM_ABS32_NOI:
    case llvm::ELF::R_ARM_PC24:
    case llvm::ELF::R_ARM_TLS_DTPMOD32:
    case llvm::ELF::R_ARM_TLS_DTPOFF32:
    case llvm::ELF::R_ARM_TLS_TPOFF32:
      break;

    default:
      error(diag::non_pic_relocation) << getName(pReloc.type())
                                      << pReloc.symInfo()->name();
      break;
  }
}

bool ARMRelocator::mayHaveFunctionPointerAccess(
    const Relocation& pReloc) const {
  switch (pReloc.type()) {
    case llvm::ELF::R_ARM_PC24:
    case llvm::ELF::R_ARM_THM_CALL:
    case llvm::ELF::R_ARM_PLT32:
    case llvm::ELF::R_ARM_CALL:
    case llvm::ELF::R_ARM_JUMP24:
    case llvm::ELF::R_ARM_THM_JUMP24:
    case llvm::ELF::R_ARM_SBREL31:
    case llvm::ELF::R_ARM_PREL31:
    case llvm::ELF::R_ARM_THM_JUMP19:
    case llvm::ELF::R_ARM_THM_JUMP6:
    case llvm::ELF::R_ARM_THM_JUMP11:
    case llvm::ELF::R_ARM_THM_JUMP8: {
      return false;
    }
    default: { return true; }
  }
}

void ARMRelocator::scanLocalReloc(Relocation& pReloc,
                                  const LDSection& pSection) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch (pReloc.type()) {
    // Set R_ARM_TARGET1 to R_ARM_ABS32
    // FIXME: R_ARM_TARGET1 should be set by option --target1-rel
    // or --target1-rel
    case llvm::ELF::R_ARM_TARGET1:
      pReloc.setType(llvm::ELF::R_ARM_ABS32);
    case llvm::ELF::R_ARM_ABS32:
    case llvm::ELF::R_ARM_ABS32_NOI: {
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rel.dyn
      if (config().isCodeIndep()) {
        helper_DynRel_init(pReloc, llvm::ELF::R_ARM_RELATIVE, *this);
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;
    }

    case llvm::ELF::R_ARM_ABS16:
    case llvm::ELF::R_ARM_ABS12:
    case llvm::ELF::R_ARM_THM_ABS5:
    case llvm::ELF::R_ARM_ABS8:
    case llvm::ELF::R_ARM_BASE_ABS:
    case llvm::ELF::R_ARM_MOVW_ABS_NC:
    case llvm::ELF::R_ARM_MOVT_ABS:
    case llvm::ELF::R_ARM_THM_MOVW_ABS_NC:
    case llvm::ELF::R_ARM_THM_MOVT_ABS: {
      // PIC code should not contain these kinds of relocation
      if (config().isCodeIndep()) {
        error(diag::non_pic_relocation) << getName(pReloc.type())
                                        << pReloc.symInfo()->name();
      }
      return;
    }
    case llvm::ELF::R_ARM_GOTOFF32:
    case llvm::ELF::R_ARM_GOTOFF12: {
      // FIXME: A GOT section is needed
      return;
    }

    // Set R_ARM_TARGET2 to R_ARM_GOT_PREL
    // FIXME: R_ARM_TARGET2 should be set by option --target2
    case llvm::ELF::R_ARM_TARGET2:
      pReloc.setType(llvm::ELF::R_ARM_GOT_PREL);
    case llvm::ELF::R_ARM_GOT_BREL:
    case llvm::ELF::R_ARM_GOT_PREL: {
      // A GOT entry is needed for these relocation type.
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;

      // If building PIC object, a dynamic relocation with
      // type RELATIVE is needed to relocate this GOT entry.
      if (config().isCodeIndep())
        helper_GOT_init(pReloc, true, *this);
      else
        helper_GOT_init(pReloc, false, *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;
    }

    case llvm::ELF::R_ARM_BASE_PREL: {
      // FIXME: Currently we only support R_ARM_BASE_PREL against
      // symbol _GLOBAL_OFFSET_TABLE_
      if (rsym != getTarget().getGOTSymbol()->resolveInfo())
        fatal(diag::base_relocation) << static_cast<int>(pReloc.type())
                                     << rsym->name()
                                     << "mclinker@googlegroups.com";
      return;
    }
    case llvm::ELF::R_ARM_COPY:
    case llvm::ELF::R_ARM_GLOB_DAT:
    case llvm::ELF::R_ARM_JUMP_SLOT:
    case llvm::ELF::R_ARM_RELATIVE: {
      // These are relocation type for dynamic linker, shold not
      // appear in object file.
      fatal(diag::dynamic_relocation) << static_cast<int>(pReloc.type());
      break;
    }
    default: { break; }
  }  // end switch
}

void ARMRelocator::scanGlobalReloc(Relocation& pReloc,
                                   IRBuilder& pBuilder,
                                   const LDSection& pSection) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch (pReloc.type()) {
    // Set R_ARM_TARGET1 to R_ARM_ABS32
    // FIXME: R_ARM_TARGET1 should be set by option --target1-rel
    // or --target1-rel
    case llvm::ELF::R_ARM_TARGET1:
      pReloc.setType(llvm::ELF::R_ARM_ABS32);
    case llvm::ELF::R_ARM_ABS32:
    case llvm::ELF::R_ARM_ABS16:
    case llvm::ELF::R_ARM_ABS12:
    case llvm::ELF::R_ARM_THM_ABS5:
    case llvm::ELF::R_ARM_ABS8:
    case llvm::ELF::R_ARM_BASE_ABS:
    case llvm::ELF::R_ARM_MOVW_ABS_NC:
    case llvm::ELF::R_ARM_MOVT_ABS:
    case llvm::ELF::R_ARM_THM_MOVW_ABS_NC:
    case llvm::ELF::R_ARM_THM_MOVT_ABS:
    case llvm::ELF::R_ARM_ABS32_NOI: {
      // Absolute relocation type, symbol may needs PLT entry or
      // dynamic relocation entry
      if (getTarget().symbolNeedsPLT(*rsym)) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)) {
          // Symbol needs PLT entry, we need to reserve a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      if (getTarget()
              .symbolNeedsDynRel(
                  *rsym, (rsym->reserved() & ReservePLT), true)) {
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym);
          addCopyReloc(*cpy_sym.resolveInfo());
        } else {
          checkValidReloc(pReloc);
          // set Rel bit
          if (helper_use_relative_reloc(*rsym, *this))
            helper_DynRel_init(pReloc, llvm::ELF::R_ARM_RELATIVE, *this);
          else
            helper_DynRel_init(pReloc, pReloc.type(), *this);
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
        }
      }
      return;
    }

    case llvm::ELF::R_ARM_GOTOFF32:
    case llvm::ELF::R_ARM_GOTOFF12: {
      // FIXME: A GOT section is needed
      return;
    }

    case llvm::ELF::R_ARM_BASE_PREL:
    case llvm::ELF::R_ARM_THM_MOVW_BREL_NC:
    case llvm::ELF::R_ARM_THM_MOVW_BREL:
    case llvm::ELF::R_ARM_THM_MOVT_BREL:
      // FIXME: Currently we only support these relocations against
      // symbol _GLOBAL_OFFSET_TABLE_
      if (rsym != getTarget().getGOTSymbol()->resolveInfo()) {
        fatal(diag::base_relocation) << static_cast<int>(pReloc.type())
                                     << rsym->name()
                                     << "mclinker@googlegroups.com";
      }
    case llvm::ELF::R_ARM_REL32:
    case llvm::ELF::R_ARM_LDR_PC_G0:
    case llvm::ELF::R_ARM_SBREL32:
    case llvm::ELF::R_ARM_THM_PC8:
    case llvm::ELF::R_ARM_MOVW_PREL_NC:
    case llvm::ELF::R_ARM_MOVT_PREL:
    case llvm::ELF::R_ARM_THM_MOVW_PREL_NC:
    case llvm::ELF::R_ARM_THM_MOVT_PREL:
    case llvm::ELF::R_ARM_THM_ALU_PREL_11_0:
    case llvm::ELF::R_ARM_THM_PC12:
    case llvm::ELF::R_ARM_REL32_NOI:
    case llvm::ELF::R_ARM_ALU_PC_G0_NC:
    case llvm::ELF::R_ARM_ALU_PC_G0:
    case llvm::ELF::R_ARM_ALU_PC_G1_NC:
    case llvm::ELF::R_ARM_ALU_PC_G1:
    case llvm::ELF::R_ARM_ALU_PC_G2:
    case llvm::ELF::R_ARM_LDR_PC_G1:
    case llvm::ELF::R_ARM_LDR_PC_G2:
    case llvm::ELF::R_ARM_LDRS_PC_G0:
    case llvm::ELF::R_ARM_LDRS_PC_G1:
    case llvm::ELF::R_ARM_LDRS_PC_G2:
    case llvm::ELF::R_ARM_LDC_PC_G0:
    case llvm::ELF::R_ARM_LDC_PC_G1:
    case llvm::ELF::R_ARM_LDC_PC_G2:
    case llvm::ELF::R_ARM_ALU_SB_G0_NC:
    case llvm::ELF::R_ARM_ALU_SB_G0:
    case llvm::ELF::R_ARM_ALU_SB_G1_NC:
    case llvm::ELF::R_ARM_ALU_SB_G1:
    case llvm::ELF::R_ARM_ALU_SB_G2:
    case llvm::ELF::R_ARM_LDR_SB_G0:
    case llvm::ELF::R_ARM_LDR_SB_G1:
    case llvm::ELF::R_ARM_LDR_SB_G2:
    case llvm::ELF::R_ARM_LDRS_SB_G0:
    case llvm::ELF::R_ARM_LDRS_SB_G1:
    case llvm::ELF::R_ARM_LDRS_SB_G2:
    case llvm::ELF::R_ARM_LDC_SB_G0:
    case llvm::ELF::R_ARM_LDC_SB_G1:
    case llvm::ELF::R_ARM_LDC_SB_G2:
    case llvm::ELF::R_ARM_MOVW_BREL_NC:
    case llvm::ELF::R_ARM_MOVT_BREL:
    case llvm::ELF::R_ARM_MOVW_BREL: {
      // Relative addressing relocation, may needs dynamic relocation
      if (getTarget()
              .symbolNeedsDynRel(
                  *rsym, (rsym->reserved() & ReservePLT), false)) {
        // symbol needs dynamic relocation entry, reserve an entry in .rel.dyn
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym);
          addCopyReloc(*cpy_sym.resolveInfo());
        } else {
          checkValidReloc(pReloc);
          // set Rel bit
          helper_DynRel_init(pReloc, pReloc.type(), *this);
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
        }
      }
      return;
    }

    case llvm::ELF::R_ARM_PC24:
    case llvm::ELF::R_ARM_THM_CALL:
    case llvm::ELF::R_ARM_PLT32:
    case llvm::ELF::R_ARM_CALL:
    case llvm::ELF::R_ARM_JUMP24:
    case llvm::ELF::R_ARM_THM_JUMP24:
    case llvm::ELF::R_ARM_SBREL31:
    case llvm::ELF::R_ARM_PREL31:
    case llvm::ELF::R_ARM_THM_JUMP19:
    case llvm::ELF::R_ARM_THM_JUMP6:
    case llvm::ELF::R_ARM_THM_JUMP11:
    case llvm::ELF::R_ARM_THM_JUMP8: {
      // These are branch relocation (except PREL31)
      // A PLT entry is needed when building shared library

      // return if we already create plt for this symbol
      if (rsym->reserved() & ReservePLT)
        return;

      // if the symbol's value can be decided at link time, then no need plt
      if (getTarget().symbolFinalValueIsKnown(*rsym))
        return;

      // if symbol is defined in the ouput file and it's not
      // preemptible, no need plt
      if (rsym->isDefine() && !rsym->isDyn() &&
          !getTarget().isSymbolPreemptible(*rsym)) {
        return;
      }

      // Symbol needs PLT entry, we need to reserve a PLT entry
      // and the corresponding GOT and dynamic relocation entry
      // in .got and .rel.plt.
      helper_PLT_init(pReloc, *this);
      // set PLT bit
      rsym->setReserved(rsym->reserved() | ReservePLT);
      return;
    }

    // Set R_ARM_TARGET2 to R_ARM_GOT_PREL
    // FIXME: R_ARM_TARGET2 should be set by option --target2
    case llvm::ELF::R_ARM_TARGET2:
      pReloc.setType(llvm::ELF::R_ARM_GOT_PREL);
    case llvm::ELF::R_ARM_GOT_BREL:
    case llvm::ELF::R_ARM_GOT_ABS:
    case llvm::ELF::R_ARM_GOT_PREL: {
      // Symbol needs GOT entry, reserve entry in .got
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;
      // if the symbol cannot be fully resolved at link time, then we need a
      // dynamic relocation
      if (!getTarget().symbolFinalValueIsKnown(*rsym))
        helper_GOT_init(pReloc, true, *this);
      else
        helper_GOT_init(pReloc, false, *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;
    }

    case llvm::ELF::R_ARM_COPY:
    case llvm::ELF::R_ARM_GLOB_DAT:
    case llvm::ELF::R_ARM_JUMP_SLOT:
    case llvm::ELF::R_ARM_RELATIVE: {
      // These are relocation type for dynamic linker, shold not
      // appear in object file.
      fatal(diag::dynamic_relocation) << static_cast<int>(pReloc.type());
      break;
    }
    default: { break; }
  }  // end switch
}

void ARMRelocator::scanRelocation(Relocation& pReloc,
                                  IRBuilder& pBuilder,
                                  Module& pModule,
                                  LDSection& pSection,
                                  Input& pInput) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  assert(rsym != NULL &&
         "ResolveInfo of relocation not set while scanRelocation");

  assert(pSection.getLink() != NULL);
  if ((pSection.getLink()->flag() & llvm::ELF::SHF_ALLOC) == 0)
    return;

  // Scan relocation type to determine if an GOT/PLT/Dynamic Relocation
  // entries should be created.
  // FIXME: Below judgements concern nothing about TLS related relocation

  // rsym is local
  if (rsym->isLocal())
    scanLocalReloc(pReloc, pSection);

  // rsym is external
  else
    scanGlobalReloc(pReloc, pBuilder, pSection);

  // check if we shoule issue undefined reference for the relocation target
  // symbol
  if (rsym->isUndef() && !rsym->isDyn() && !rsym->isWeak() && !rsym->isNull())
    issueUndefRef(pReloc, pSection, pInput);
}

uint32_t ARMRelocator::getDebugStringOffset(Relocation& pReloc) const {
  if (pReloc.type() != llvm::ELF::R_ARM_ABS32)
    error(diag::unsupport_reloc_for_debug_string)
        << getName(pReloc.type()) << "mclinker@googlegroups.com";

  if (pReloc.symInfo()->type() == ResolveInfo::Section)
    return pReloc.target() + pReloc.addend();
  else
    return pReloc.symInfo()->outSymbol()->fragRef()->offset() +
               pReloc.target() + pReloc.addend();
}

void ARMRelocator::applyDebugStringOffset(Relocation& pReloc,
                                          uint32_t pOffset) {
  pReloc.target() = pOffset;
}

//=========================================//
// Each relocation function implementation //
//=========================================//

// R_ARM_NONE
ARMRelocator::Result none(Relocation& pReloc, ARMRelocator& pParent) {
  return Relocator::OK;
}

// R_ARM_ABS32: (S + A) | T
ARMRelocator::Result abs32(Relocation& pReloc, ARMRelocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // If the flag of target section is not ALLOC, we will not scan this
  // relocation
  // but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC &
       pReloc.targetRef().frag()->getParent()->getSection().flag()) == 0) {
    pReloc.target() = (S + A) | T;
    return Relocator::OK;
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
      T = 0;  // PLT is not thumb
    }
    // If we generate a dynamic relocation (except R_ARM_RELATIVE)
    // for a place, we should not perform static relocation on it
    // in order to keep the addend store in the place correct.
    if ((rsym->reserved() & ARMRelocator::ReserveRel) &&
        (!helper_use_relative_reloc(*rsym, pParent)))
      return Relocator::OK;
  }

  // perform static relocation
  pReloc.target() = (S + A) | T;
  return Relocator::OK;
}

// R_ARM_REL32: ((S + A) | T) - P
ARMRelocator::Result rel32(Relocation& pReloc, ARMRelocator& pParent) {
  // perform static relocation
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A = pReloc.target() + pReloc.addend();

  // An external symbol may need PLT (this reloc is from a stub/veneer)
  if (!pReloc.symInfo()->isLocal()) {
    if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
      T = 0;  // PLT is not thumb.
    }
  }

  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // perform relocation
  pReloc.target() = ((S + A) | T) - pReloc.place();

  return Relocator::OK;
}

// R_ARM_BASE_PREL: B(S) + A - P
ARMRelocator::Result base_prel(Relocation& pReloc, ARMRelocator& pParent) {
  // perform static relocation
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  pReloc.target() = pReloc.symValue() + A - pReloc.place();
  return Relocator::OK;
}

// R_ARM_GOTOFF32: ((S + A) | T) - GOT_ORG
ARMRelocator::Result gotoff32(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  Relocator::Address S = pReloc.symValue();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  pReloc.target() = ((S + A) | T) - GOT_ORG;
  return Relocator::OK;
}

// R_ARM_GOT_BREL: GOT(S) + A - GOT_ORG
ARMRelocator::Result got_brel(Relocation& pReloc, ARMRelocator& pParent) {
  if (!(pReloc.symInfo()->reserved() & ARMRelocator::ReserveGOT))
    return Relocator::BadReloc;

  Relocator::Address GOT_S = helper_get_GOT_address(*pReloc.symInfo(), pParent);
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  // Apply relocation.
  pReloc.target() = GOT_S + A - GOT_ORG;

  // setup got entry value if needed
  ARMGOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  if (got_entry != NULL && ARMRelocator::SymVal == got_entry->getValue())
    got_entry->setValue(pReloc.symValue());
  return Relocator::OK;
}

// R_ARM_GOT_PREL: GOT(S) + A - P
ARMRelocator::Result got_prel(Relocation& pReloc, ARMRelocator& pParent) {
  if (!(pReloc.symInfo()->reserved() & ARMRelocator::ReserveGOT)) {
    return Relocator::BadReloc;
  }
  Relocator::Address GOT_S = helper_get_GOT_address(*pReloc.symInfo(), pParent);
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::Address P = pReloc.place();

  // Apply relocation.
  pReloc.target() = GOT_S + A - P;

  // setup got entry value if needed
  ARMGOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  if (got_entry != NULL && ARMRelocator::SymVal == got_entry->getValue())
    got_entry->setValue(pReloc.symValue());
  return Relocator::OK;
}

// R_ARM_THM_JUMP8: S + A - P
ARMRelocator::Result thm_jump8(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::DWord P = pReloc.place();
  Relocator::DWord A =
      helper_sign_extend((pReloc.target() & 0x00ff) << 1, 8) + pReloc.addend();
  // S depends on PLT exists or not
  Relocator::Address S = pReloc.symValue();
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT)
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);

  Relocator::DWord X = S + A - P;
  if (helper_check_signed_overflow(X, 9))
    return Relocator::Overflow;
  //                    Make sure the Imm is 0.          Result Mask.
  pReloc.target() = (pReloc.target() & 0xFFFFFF00u) | ((X & 0x01FEu) >> 1);
  return Relocator::OK;
}

// R_ARM_THM_JUMP11: S + A - P
ARMRelocator::Result thm_jump11(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::DWord P = pReloc.place();
  Relocator::DWord A =
      helper_sign_extend((pReloc.target() & 0x07ff) << 1, 11) + pReloc.addend();
  // S depends on PLT exists or not
  Relocator::Address S = pReloc.symValue();
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT)
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);

  Relocator::DWord X = S + A - P;
  if (helper_check_signed_overflow(X, 12))
    return Relocator::Overflow;
  //                    Make sure the Imm is 0.          Result Mask.
  pReloc.target() = (pReloc.target() & 0xFFFFF800u) | ((X & 0x0FFEu) >> 1);
  return Relocator::OK;
}

// R_ARM_THM_JUMP19: ((S + A) | T) - P
ARMRelocator::Result thm_jump19(Relocation& pReloc, ARMRelocator& pParent) {
  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);

  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A =
      helper_thumb32_cond_branch_offset(upper_inst, lower_inst) +
      pReloc.addend();
  Relocator::Address P = pReloc.place();
  Relocator::Address S;
  // if symbol has plt
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT) {
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
    T = 0;  // PLT is not thumb.
  } else {
    S = pReloc.symValue();
    if (T != 0x0)
      helper_clear_thumb_bit(S);
  }

  if (T == 0x0) {
    // FIXME: conditional branch to PLT in THUMB-2 not supported yet
    error(diag::unsupported_cond_branch_reloc)
        << static_cast<int>(pReloc.type());
    return Relocator::BadReloc;
  }

  Relocator::DWord X = ((S + A) | T) - P;
  if (helper_check_signed_overflow(X, 21))
    return Relocator::Overflow;

  upper_inst = helper_thumb32_cond_branch_upper(upper_inst, X);
  lower_inst = helper_thumb32_cond_branch_lower(lower_inst, X);

  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = upper_inst;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = lower_inst;

  return Relocator::OK;
}

// R_ARM_PC24: ((S + A) | T) - P
// R_ARM_PLT32: ((S + A) | T) - P
// R_ARM_JUMP24: ((S + A) | T) - P
// R_ARM_CALL: ((S + A) | T) - P
ARMRelocator::Result call(Relocation& pReloc, ARMRelocator& pParent) {
  // If target is undefined weak symbol, we only need to jump to the
  // next instruction unless it has PLT entry. Rewrite instruction
  // to NOP.
  if (pReloc.symInfo()->isWeak() && pReloc.symInfo()->isUndef() &&
      !pReloc.symInfo()->isDyn() &&
      !(pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT)) {
    // change target to NOP : mov r0, r0
    pReloc.target() = (pReloc.target() & 0xf0000000U) | 0x01a00000;
    return Relocator::OK;
  }

  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A =
      helper_sign_extend((pReloc.target() & 0x00FFFFFFu) << 2, 26) +
      pReloc.addend();
  Relocator::Address P = pReloc.place();
  Relocator::Address S = pReloc.symValue();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // S depends on PLT exists or not
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT) {
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
    T = 0;  // PLT is not thumb.
  }

  // At this moment (after relaxation), if the jump target is thumb instruction,
  // switch mode is needed, rewrite the instruction to BLX
  // FIXME: check if we can use BLX instruction (check from .ARM.attribute
  // CPU ARCH TAG, which should be ARMv5 or above)
  if (T != 0) {
    // cannot rewrite to blx for R_ARM_JUMP24
    if (pReloc.type() == llvm::ELF::R_ARM_JUMP24)
      return Relocator::BadReloc;
    if (pReloc.type() == llvm::ELF::R_ARM_PC24)
      return Relocator::BadReloc;

    pReloc.target() =
        (pReloc.target() & 0xffffff) | 0xfa000000 | (((S + A - P) & 2) << 23);
  }

  Relocator::DWord X = ((S + A) | T) - P;
  // Check X is 24bit sign int. If not, we should use stub or PLT before apply.
  if (helper_check_signed_overflow(X, 26))
    return Relocator::Overflow;
  //                    Make sure the Imm is 0.          Result Mask.
  pReloc.target() = (pReloc.target() & 0xFF000000u) | ((X & 0x03FFFFFEu) >> 2);
  return Relocator::OK;
}

// R_ARM_THM_CALL: ((S + A) | T) - P
// R_ARM_THM_JUMP24: ((S + A) | T) - P
ARMRelocator::Result thm_call(Relocation& pReloc, ARMRelocator& pParent) {
  // If target is undefined weak symbol, we only need to jump to the
  // next instruction unless it has PLT entry. Rewrite instruction
  // to NOP.
  if (pReloc.symInfo()->isWeak() && pReloc.symInfo()->isUndef() &&
      !pReloc.symInfo()->isDyn() &&
      !(pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT)) {
    pReloc.target() = (0xe000U << 16) | 0xbf00U;
    return Relocator::OK;
  }

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);

  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A =
      helper_thumb32_branch_offset(upper_inst, lower_inst) + pReloc.addend();
  Relocator::Address P = pReloc.place();
  Relocator::Address S;

  // if symbol has plt
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT) {
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
    T = 0;  // PLT is not thumb.
  } else {
    S = pReloc.symValue();
    if (T != 0x0)
      helper_clear_thumb_bit(S);
  }

  S = S + A;

  // At this moment (after relaxation), if the jump target is arm
  // instruction, switch mode is needed, rewrite the instruction to BLX
  // FIXME: check if we can use BLX instruction (check from .ARM.attribute
  // CPU ARCH TAG, which should be ARMv5 or above)
  if (T == 0) {
    // cannot rewrite to blx for R_ARM_THM_JUMP24
    if (pReloc.type() == llvm::ELF::R_ARM_THM_JUMP24)
      return Relocator::BadReloc;

    // for BLX, select bit 1 from relocation base address to jump target
    // address
    S = helper_bit_select(S, P, 0x2);
    // rewrite instruction to BLX
    lower_inst &= ~0x1000U;
  } else {
    // otherwise, the instruction should be BL
    lower_inst |= 0x1000U;
  }

  Relocator::DWord X = (S | T) - P;

  // FIXME: Check bit size is 24(thumb2) or 22?
  if (helper_check_signed_overflow(X, 25)) {
    return Relocator::Overflow;
  }

  upper_inst = helper_thumb32_branch_upper(upper_inst, X);
  lower_inst = helper_thumb32_branch_lower(lower_inst, X);

  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = upper_inst;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = lower_inst;

  return Relocator::OK;
}

// R_ARM_MOVW_ABS_NC: (S + A) | T
ARMRelocator::Result movw_abs_nc(Relocation& pReloc, ARMRelocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A =
      helper_extract_movw_movt_addend(pReloc.target()) + pReloc.addend();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();

  // If the flag of target section is not ALLOC, we will not scan this
  // relocation but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC & target_sect.flag()) != 0x0) {
    // use plt
    if (rsym->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
      T = 0;  // PLT is not thumb
    }
  }

  // perform static relocation
  Relocator::DWord X = (S + A) | T;
  pReloc.target() =
      helper_insert_val_movw_movt_inst(pReloc.target() + pReloc.addend(), X);
  return Relocator::OK;
}

// R_ARM_MOVW_PREL_NC: ((S + A) | T) - P
ARMRelocator::Result movw_prel_nc(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord P = pReloc.place();
  Relocator::DWord A =
      helper_extract_movw_movt_addend(pReloc.target()) + pReloc.addend();
  if (T != 0x0)
    helper_clear_thumb_bit(S);
  Relocator::DWord X = ((S + A) | T) - P;

  if (helper_check_signed_overflow(X, 16)) {
    return Relocator::Overflow;
  } else {
    pReloc.target() = helper_insert_val_movw_movt_inst(pReloc.target(), X);
    return Relocator::OK;
  }
}

// R_ARM_MOVT_ABS: S + A
ARMRelocator::Result movt_abs(Relocation& pReloc, ARMRelocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A =
      helper_extract_movw_movt_addend(pReloc.target()) + pReloc.addend();

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();

  // If the flag of target section is not ALLOC, we will not scan this
  // relocation
  // but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC & target_sect.flag()) != 0x0) {
    // use plt
    if (rsym->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
    }
  }

  Relocator::DWord X = S + A;
  X >>= 16;
  // perform static relocation
  pReloc.target() = helper_insert_val_movw_movt_inst(pReloc.target(), X);
  return Relocator::OK;
}

// R_ARM_MOVT_PREL: S + A - P
ARMRelocator::Result movt_prel(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord P = pReloc.place();
  Relocator::DWord A =
      helper_extract_movw_movt_addend(pReloc.target()) + pReloc.addend();
  Relocator::DWord X = S + A - P;
  X >>= 16;

  pReloc.target() = helper_insert_val_movw_movt_inst(pReloc.target(), X);
  return Relocator::OK;
}

// R_ARM_THM_MOVW_ABS_NC: (S + A) | T
ARMRelocator::Result thm_movw_abs_nc(Relocation& pReloc,
                                     ARMRelocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);
  Relocator::DWord val = ((upper_inst) << 16) | (lower_inst);
  Relocator::DWord A =
      helper_extract_thumb_movw_movt_addend(val) + pReloc.addend();

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this
  // relocation
  // but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC & target_sect.flag()) != 0x0) {
    // use plt
    if (rsym->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
      T = 0;  // PLT is not thumb
    }
  }
  Relocator::DWord X = (S + A) | T;

  val = helper_insert_val_thumb_movw_movt_inst(val, X);
  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = val >> 16;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = val & 0xFFFFu;

  return Relocator::OK;
}

// R_ARM_THM_MOVW_PREL_NC: ((S + A) | T) - P
ARMRelocator::Result thm_movw_prel_nc(Relocation& pReloc,
                                      ARMRelocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord P = pReloc.place();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);
  Relocator::DWord val = ((upper_inst) << 16) | (lower_inst);
  Relocator::DWord A =
      helper_extract_thumb_movw_movt_addend(val) + pReloc.addend();
  Relocator::DWord X = ((S + A) | T) - P;

  val = helper_insert_val_thumb_movw_movt_inst(val, X);
  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = val >> 16;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = val & 0xFFFFu;

  return Relocator::OK;
}

// R_ARM_THM_MOVW_BREL_NC: ((S + A) | T) - B(S)
// R_ARM_THM_MOVW_BREL: ((S + A) | T) - B(S)
ARMRelocator::Result thm_movw_brel(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord P = pReloc.place();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);
  Relocator::DWord val = ((upper_inst) << 16) | (lower_inst);
  Relocator::DWord A =
      helper_extract_thumb_movw_movt_addend(val) + pReloc.addend();

  Relocator::DWord X = ((S + A) | T) - P;

  val = helper_insert_val_thumb_movw_movt_inst(val, X);
  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = val >> 16;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = val & 0xFFFFu;

  return Relocator::OK;
}

// R_ARM_THM_MOVT_ABS: S + A
ARMRelocator::Result thm_movt_abs(Relocation& pReloc, ARMRelocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);
  Relocator::DWord val = ((upper_inst) << 16) | (lower_inst);
  Relocator::DWord A =
      helper_extract_thumb_movw_movt_addend(val) + pReloc.addend();

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this
  // relocation but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC & target_sect.flag()) != 0x0) {
    // use plt
    if (rsym->reserved() & ARMRelocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
    }
  }

  Relocator::DWord X = S + A;
  X >>= 16;

  // check 16-bit overflow
  if (helper_check_signed_overflow(X, 16))
    return Relocator::Overflow;
  val = helper_insert_val_thumb_movw_movt_inst(val, X);
  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = val >> 16;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = val & 0xFFFFu;
  return Relocator::OK;
}

// R_ARM_THM_MOVT_PREL: S + A - P
// R_ARM_THM_MOVT_BREL: S + A - B(S)
ARMRelocator::Result thm_movt_prel(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord P = pReloc.place();

  // get lower and upper 16 bit instructions from relocation targetData
  uint16_t upper_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()));
  uint16_t lower_inst = *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1);
  Relocator::DWord val = ((upper_inst) << 16) | (lower_inst);
  Relocator::DWord A =
      helper_extract_thumb_movw_movt_addend(val) + pReloc.addend();
  Relocator::DWord X = S + A - P;
  X >>= 16;

  val = helper_insert_val_thumb_movw_movt_inst(val, X);
  *(reinterpret_cast<uint16_t*>(&pReloc.target())) = val >> 16;
  *(reinterpret_cast<uint16_t*>(&pReloc.target()) + 1) = val & 0xFFFFu;

  return Relocator::OK;
}

// R_ARM_PREL31: ((S + A) | T) - P
ARMRelocator::Result prel31(Relocation& pReloc, ARMRelocator& pParent) {
  Relocator::DWord target = pReloc.target();
  Relocator::DWord T = getThumbBit(pReloc);
  Relocator::DWord A = helper_sign_extend(target, 31) + pReloc.addend();
  Relocator::DWord P = pReloc.place();
  Relocator::Address S = pReloc.symValue();
  if (T != 0x0)
    helper_clear_thumb_bit(S);

  // if symbol has plt
  if (pReloc.symInfo()->reserved() & ARMRelocator::ReservePLT) {
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
    T = 0;  // PLT is not thumb.
  }

  Relocator::DWord X = ((S + A) | T) - P;
  pReloc.target() = helper_bit_select(target, X, 0x7fffffffU);
  if (helper_check_signed_overflow(X, 31))
    return Relocator::Overflow;
  return Relocator::OK;
}

// R_ARM_TLS_GD32: GOT(S) + A - P
// R_ARM_TLS_IE32: GOT(S) + A - P
// R_ARM_TLS_LE32: S + A - tp
ARMRelocator::Result tls(Relocation& pReloc, ARMRelocator& pParent) {
  return Relocator::Unsupported;
}

ARMRelocator::Result unsupported(Relocation& pReloc, ARMRelocator& pParent) {
  return Relocator::Unsupported;
}

}  // namespace mcld
