//===- AArch64Relocator.cpp  ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "mcld/LinkerConfig.h"
#include "mcld/IRBuilder.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/Object/ObjectBuilder.h"

#include "AArch64Relocator.h"
#include "AArch64RelocationFunctions.h"
#include "AArch64RelocationHelpers.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Relocation Functions and Tables
//===----------------------------------------------------------------------===//
DECL_AARCH64_APPLY_RELOC_FUNCS

/// the prototype of applying function
typedef Relocator::Result (*ApplyFunctionType)(Relocation& pReloc,
                                               AArch64Relocator& pParent);

// the table entry of applying functions
class ApplyFunctionEntry {
 public:
  ApplyFunctionEntry() {}
  ApplyFunctionEntry(ApplyFunctionType pFunc,
                     const char* pName,
                     size_t pSize = 0)
      : func(pFunc), name(pName), size(pSize) {}
  ApplyFunctionType func;
  const char* name;
  size_t size;
};
typedef std::map<Relocator::Type, ApplyFunctionEntry> ApplyFunctionMap;

static const ApplyFunctionMap::value_type ApplyFunctionList[] = {
    DECL_AARCH64_APPLY_RELOC_FUNC_PTRS(ApplyFunctionMap::value_type,
                                       ApplyFunctionEntry)};

// declare the table of applying functions
static ApplyFunctionMap ApplyFunctions(ApplyFunctionList,
                                       ApplyFunctionList +
                                           sizeof(ApplyFunctionList) /
                                               sizeof(ApplyFunctionList[0]));

//===----------------------------------------------------------------------===//
// AArch64Relocator
//===----------------------------------------------------------------------===//
AArch64Relocator::AArch64Relocator(AArch64GNULDBackend& pParent,
                                   const LinkerConfig& pConfig)
    : Relocator(pConfig), m_Target(pParent) {
}

AArch64Relocator::~AArch64Relocator() {
}

Relocator::Result AArch64Relocator::applyRelocation(Relocation& pRelocation) {
  Relocation::Type type = pRelocation.type();
  // valid types are 0x0, 0x100-1032, and R_AARCH64_REWRITE_INSN
  if ((type < 0x100 || type > 1032) &&
      (type != 0x0) &&
      (type != R_AARCH64_REWRITE_INSN)) {
    return Relocator::Unknown;
  }
  assert(ApplyFunctions.find(type) != ApplyFunctions.end());
  return ApplyFunctions[type].func(pRelocation, *this);
}

const char* AArch64Relocator::getName(Relocator::Type pType) const {
  assert(ApplyFunctions.find(pType) != ApplyFunctions.end());
  return ApplyFunctions[pType].name;
}

Relocator::Size AArch64Relocator::getSize(Relocation::Type pType) const {
  return ApplyFunctions[pType].size;
}

void AArch64Relocator::addCopyReloc(ResolveInfo& pSym) {
  Relocation& rel_entry = *getTarget().getRelaDyn().create();
  rel_entry.setType(llvm::ELF::R_AARCH64_COPY);
  assert(pSym.outSymbol()->hasFragRef());
  rel_entry.targetRef().assign(*pSym.outSymbol()->fragRef());
  rel_entry.setSymInfo(&pSym);
}

/// defineSymbolForCopyReloc
/// For a symbol needing copy relocation, define a copy symbol in the BSS
/// section and all other reference to this symbol should refer to this
/// copy.
/// This is executed at scan relocation stage.
LDSymbol& AArch64Relocator::defineSymbolforCopyReloc(IRBuilder& pBuilder,
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

void AArch64Relocator::scanLocalReloc(Relocation& pReloc,
                                      const LDSection& pSection) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  switch (pReloc.type()) {
    case llvm::ELF::R_AARCH64_ABS64:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rel.dyn
      if (config().isCodeIndep()) {
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
        // set up the dyn rel directly
        Relocation& reloc = helper_DynRela_init(rsym,
                                                *pReloc.targetRef().frag(),
                                                pReloc.targetRef().offset(),
                                                llvm::ELF::R_AARCH64_RELATIVE,
                                                *this);
        getRelRelMap().record(pReloc, reloc);
      }
      return;

    case llvm::ELF::R_AARCH64_ABS32:
    case llvm::ELF::R_AARCH64_ABS16:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rel.dyn
      if (config().isCodeIndep()) {
        // set up the dyn rel directly
        Relocation& reloc = helper_DynRela_init(rsym,
                                                *pReloc.targetRef().frag(),
                                                pReloc.targetRef().offset(),
                                                pReloc.type(),
                                                *this);
        getRelRelMap().record(pReloc, reloc);
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;

    case llvm::ELF::R_AARCH64_ADR_GOT_PAGE:
    case llvm::ELF::R_AARCH64_LD64_GOT_LO12_NC: {
      // Symbol needs GOT entry, reserve entry in .got
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

    default:
      break;
  }
}

void AArch64Relocator::scanGlobalReloc(Relocation& pReloc,
                                       IRBuilder& pBuilder,
                                       const LDSection& pSection) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  switch (pReloc.type()) {
    case llvm::ELF::R_AARCH64_ABS64:
    case llvm::ELF::R_AARCH64_ABS32:
    case llvm::ELF::R_AARCH64_ABS16:
      // Absolute relocation type, symbol may needs PLT entry or
      // dynamic relocation entry
      if (getTarget().symbolNeedsPLT(*rsym)) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)) {
          // Symbol needs PLT entry, we need a PLT entry
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
        // symbol needs dynamic relocation entry, set up the dynrel entry
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym);
          addCopyReloc(*cpy_sym.resolveInfo());
        } else {
          // set Rel bit and the dyn rel
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
          if (llvm::ELF::R_AARCH64_ABS64 == pReloc.type() &&
              helper_use_relative_reloc(*rsym, *this)) {
            Relocation& reloc =
                helper_DynRela_init(rsym,
                                    *pReloc.targetRef().frag(),
                                    pReloc.targetRef().offset(),
                                    llvm::ELF::R_AARCH64_RELATIVE,
                                    *this);
            getRelRelMap().record(pReloc, reloc);
          } else {
            Relocation& reloc = helper_DynRela_init(rsym,
                                                    *pReloc.targetRef().frag(),
                                                    pReloc.targetRef().offset(),
                                                    pReloc.type(),
                                                    *this);
            getRelRelMap().record(pReloc, reloc);
          }
        }
      }
      return;

    case llvm::ELF::R_AARCH64_PREL64:
    case llvm::ELF::R_AARCH64_PREL32:
    case llvm::ELF::R_AARCH64_PREL16:
      if (getTarget().symbolNeedsPLT(*rsym) &&
          LinkerConfig::DynObj != config().codeGenType()) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)) {
          // Symbol needs PLT entry, we need a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      // Only PC relative relocation against dynamic symbol needs a
      // dynamic relocation.  Only dynamic copy relocation is allowed
      // and PC relative relocation will be resolved to the local copy.
      // All other dynamic relocations may lead to run-time relocation
      // overflow.
      if (getTarget().isDynamicSymbol(*rsym) &&
          getTarget()
              .symbolNeedsDynRel(
                  *rsym, (rsym->reserved() & ReservePLT), false) &&
          getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
        LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym);
        addCopyReloc(*cpy_sym.resolveInfo());
      }
      return;

    case llvm::ELF::R_AARCH64_CONDBR19:
    case llvm::ELF::R_AARCH64_JUMP26:
    case llvm::ELF::R_AARCH64_CALL26: {
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

    case llvm::ELF::R_AARCH64_ADR_PREL_LO21:
    case llvm::ELF::R_AARCH64_ADR_PREL_PG_HI21:
    case llvm::ELF::R_AARCH64_ADR_PREL_PG_HI21_NC:
      if (getTarget()
              .symbolNeedsDynRel(
                  *rsym, (rsym->reserved() & ReservePLT), false)) {
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym);
          addCopyReloc(*cpy_sym.resolveInfo());
        }
      }
      if (getTarget().symbolNeedsPLT(*rsym)) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)) {
          // Symbol needs PLT entry, we need a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }
      return;

    case llvm::ELF::R_AARCH64_ADR_GOT_PAGE:
    case llvm::ELF::R_AARCH64_LD64_GOT_LO12_NC: {
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

    default:
      break;
  }
}

void AArch64Relocator::scanRelocation(Relocation& pReloc,
                                      IRBuilder& pBuilder,
                                      Module& pModule,
                                      LDSection& pSection,
                                      Input& pInput) {
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

bool
AArch64Relocator::mayHaveFunctionPointerAccess(const Relocation& pReloc) const {
  switch (pReloc.type()) {
    case llvm::ELF::R_AARCH64_ADR_PREL_PG_HI21:
    case llvm::ELF::R_AARCH64_ADR_PREL_PG_HI21_NC:
    case llvm::ELF::R_AARCH64_ADD_ABS_LO12_NC:
    case llvm::ELF::R_AARCH64_ADR_GOT_PAGE:
    case llvm::ELF::R_AARCH64_LD64_GOT_LO12_NC: {
      return true;
    }
    default: {
      if (pReloc.symInfo()->isLocal()) {
        // Do not fold any local symbols if building a shared object.
        return (config().codeGenType() == LinkerConfig::DynObj);
      } else {
        // Do not fold any none global defualt symbols if building a shared
        // object.
        return ((config().codeGenType() == LinkerConfig::DynObj) &&
                (pReloc.symInfo()->visibility() != ResolveInfo::Default));
      }
    }
  }
  return false;
}

uint32_t AArch64Relocator::getDebugStringOffset(Relocation& pReloc) const {
  if (pReloc.type() != llvm::ELF::R_AARCH64_ABS32)
    error(diag::unsupport_reloc_for_debug_string)
        << getName(pReloc.type()) << "mclinker@googlegroups.com";

  if (pReloc.symInfo()->type() == ResolveInfo::Section)
    return pReloc.target() + pReloc.addend();
  else
    return pReloc.symInfo()->outSymbol()->fragRef()->offset() +
               pReloc.target() + pReloc.addend();
}

void AArch64Relocator::applyDebugStringOffset(Relocation& pReloc,
                                              uint32_t pOffset) {
  pReloc.target() = pOffset;
}

//===----------------------------------------------------------------------===//
// Each relocation function implementation
//===----------------------------------------------------------------------===//

// R_AARCH64_NONE
Relocator::Result none(Relocation& pReloc, AArch64Relocator& pParent) {
  return Relocator::OK;
}

Relocator::Result unsupported(Relocation& pReloc, AArch64Relocator& pParent) {
  return Relocator::Unsupported;
}

// R_AARCH64_ABS64: S + A
// R_AARCH64_ABS32: S + A
// R_AARCH64_ABS16: S + A
Relocator::Result abs(Relocation& pReloc, AArch64Relocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  Relocation* dyn_rel = pParent.getRelRelMap().lookUp(pReloc);
  bool has_dyn_rel = (dyn_rel != NULL);

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this
  // relocation but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.target() = S + A;
    return Relocator::OK;
  }
  // A local symbol may need RELATIVE Type dynamic relocation
  if (rsym->isLocal() && has_dyn_rel) {
    dyn_rel->setAddend(S + A);
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & AArch64Relocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
    }
    // If we generate a dynamic relocation (except R_AARCH64_64_RELATIVE)
    // for a place, we should not perform static relocation on it
    // in order to keep the addend store in the place correct.
    if (has_dyn_rel) {
      if (llvm::ELF::R_AARCH64_ABS64 == pReloc.type() &&
          llvm::ELF::R_AARCH64_RELATIVE == dyn_rel->type()) {
        dyn_rel->setAddend(S + A);
      } else {
        dyn_rel->setAddend(A);
        return Relocator::OK;
      }
    }
  }

  // perform static relocation
  pReloc.target() = S + A;
  return Relocator::OK;
}

// R_AARCH64_PREL64: S + A - P
// R_AARCH64_PREL32: S + A - P
// R_AARCH64_PREL16: S + A - P
Relocator::Result rel(Relocation& pReloc, AArch64Relocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A = pReloc.addend();
  Relocator::DWord P = pReloc.place();

  if (llvm::ELF::R_AARCH64_PREL64 != pReloc.type())
    A += pReloc.target() & get_mask(pParent.getSize(pReloc.type()));
  else
    A += pReloc.target();

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this
  // relocation but perform static relocation. (e.g., applying .debug section)
  if (0x0 != (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    // if plt entry exists, the S value is the plt entry address
    if (!rsym->isLocal()) {
      if (rsym->reserved() & AArch64Relocator::ReservePLT) {
        S = helper_get_PLT_address(*rsym, pParent);
      }
    }
  }

  Relocator::DWord X = S + A - P;
  pReloc.target() = X;

  if (llvm::ELF::R_AARCH64_PREL64 != pReloc.type() &&
      helper_check_signed_overflow(X, pParent.getSize(pReloc.type())))
    return Relocator::Overflow;
  return Relocator::OK;
}

// R_AARCH64_ADD_ABS_LO12_NC: S + A
Relocator::Result add_abs_lo12(Relocation& pReloc, AArch64Relocator& pParent) {
  Relocator::Address value = 0x0;
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A = pReloc.addend();

  value = helper_get_page_offset(S + A);
  pReloc.target() = helper_reencode_add_imm(pReloc.target(), value);

  return Relocator::OK;
}

// R_AARCH64_ADR_PREL_LO21: S + A - P
Relocator::Result adr_prel_lo21(Relocation& pReloc, AArch64Relocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  // if plt entry exists, the S value is the plt entry address
  if (rsym->reserved() & AArch64Relocator::ReservePLT) {
    S = helper_get_PLT_address(*rsym, pParent);
  }
  Relocator::DWord A = pReloc.addend();
  Relocator::DWord P = pReloc.place();
  Relocator::DWord X = S + A - P;

  pReloc.target() = helper_reencode_adr_imm(pReloc.target(), X);

  return Relocator::OK;
}

// R_AARCH64_ADR_PREL_PG_HI21: ((PG(S + A) - PG(P)) >> 12)
// R_AARCH64_ADR_PREL_PG_HI21_NC: ((PG(S + A) - PG(P)) >> 12)
Relocator::Result adr_prel_pg_hi21(Relocation& pReloc,
                                   AArch64Relocator& pParent) {
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue();
  // if plt entry exists, the S value is the plt entry address
  if (rsym->reserved() & AArch64Relocator::ReservePLT) {
    S = helper_get_PLT_address(*rsym, pParent);
  }
  Relocator::DWord A = pReloc.addend();
  Relocator::DWord P = pReloc.place();
  Relocator::DWord X =
      helper_get_page_address(S + A) - helper_get_page_address(P);

  pReloc.target() = helper_reencode_adr_imm(pReloc.target(), (X >> 12));

  return Relocator::OK;
}

// R_AARCH64_CALL26: S + A - P
// R_AARCH64_JUMP26: S + A - P
Relocator::Result call(Relocation& pReloc, AArch64Relocator& pParent) {
  // If target is undefined weak symbol, we only need to jump to the
  // next instruction unless it has PLT entry. Rewrite instruction
  // to NOP.
  if (pReloc.symInfo()->isWeak() && pReloc.symInfo()->isUndef() &&
      !pReloc.symInfo()->isDyn() &&
      !(pReloc.symInfo()->reserved() & AArch64Relocator::ReservePLT)) {
    // change target to NOP
    pReloc.target() = 0xd503201f;
    return Relocator::OK;
  }

  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A = pReloc.addend();
  Relocator::Address P = pReloc.place();

  // S depends on PLT exists or not
  if (pReloc.symInfo()->reserved() & AArch64Relocator::ReservePLT)
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);

  Relocator::DWord X = S + A - P;
  // TODO: check overflow..

  pReloc.target() = helper_reencode_branch_offset_26(pReloc.target(), X >> 2);

  return Relocator::OK;
}

// R_AARCH64_CONDBR19: S + A - P
Relocator::Result condbr(Relocation& pReloc, AArch64Relocator& pParent) {
  // If target is undefined weak symbol, we only need to jump to the
  // next instruction unless it has PLT entry. Rewrite instruction
  // to NOP.
  if (pReloc.symInfo()->isWeak() && pReloc.symInfo()->isUndef() &&
      !pReloc.symInfo()->isDyn() &&
      !(pReloc.symInfo()->reserved() & AArch64Relocator::ReservePLT)) {
    // change target to NOP
    pReloc.target() = 0xd503201f;
    return Relocator::OK;
  }

  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A = pReloc.addend();
  Relocator::Address P = pReloc.place();

  // S depends on PLT exists or not
  if (pReloc.symInfo()->reserved() & AArch64Relocator::ReservePLT)
    S = helper_get_PLT_address(*pReloc.symInfo(), pParent);

  Relocator::DWord X = S + A - P;
  // TODO: check overflow..

  pReloc.target() = helper_reencode_cond_branch_ofs_19(pReloc.target(), X >> 2);

  return Relocator::OK;
}

// R_AARCH64_ADR_GOT_PAGE: Page(G(GDAT(S+A))) - Page(P)
Relocator::Result adr_got_page(Relocation& pReloc, AArch64Relocator& pParent) {
  if (!(pReloc.symInfo()->reserved() & AArch64Relocator::ReserveGOT)) {
    return Relocator::BadReloc;
  }

  Relocator::Address GOT_S = helper_get_GOT_address(*pReloc.symInfo(), pParent);
  Relocator::DWord A = pReloc.addend();
  Relocator::Address P = pReloc.place();
  Relocator::DWord X =
      helper_get_page_address(GOT_S + A) - helper_get_page_address(P);

  pReloc.target() = helper_reencode_adr_imm(pReloc.target(), (X >> 12));

  // setup got entry value if needed
  AArch64GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  if (got_entry != NULL && AArch64Relocator::SymVal == got_entry->getValue())
    got_entry->setValue(pReloc.symValue());
  // setup relocation addend if needed
  Relocation* dyn_rela = pParent.getRelRelMap().lookUp(pReloc);
  if ((dyn_rela != NULL) && (AArch64Relocator::SymVal == dyn_rela->addend())) {
    dyn_rela->setAddend(pReloc.symValue());
  }
  return Relocator::OK;
}

// R_AARCH64_LD64_GOT_LO12_NC: G(GDAT(S+A))
Relocator::Result ld64_got_lo12(Relocation& pReloc, AArch64Relocator& pParent) {
  if (!(pReloc.symInfo()->reserved() & AArch64Relocator::ReserveGOT)) {
    return Relocator::BadReloc;
  }

  Relocator::Address GOT_S = helper_get_GOT_address(*pReloc.symInfo(), pParent);
  Relocator::DWord A = pReloc.addend();
  Relocator::DWord X = helper_get_page_offset(GOT_S + A);

  pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), (X >> 3));

  // setup got entry value if needed
  AArch64GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  if (got_entry != NULL && AArch64Relocator::SymVal == got_entry->getValue())
    got_entry->setValue(pReloc.symValue());

  // setup relocation addend if needed
  Relocation* dyn_rela = pParent.getRelRelMap().lookUp(pReloc);
  if ((dyn_rela != NULL) && (AArch64Relocator::SymVal == dyn_rela->addend())) {
    dyn_rela->setAddend(pReloc.symValue());
  }

  return Relocator::OK;
}

// R_AARCH64_LDST8_ABS_LO12_NC: S + A
// R_AARCH64_LDST16_ABS_LO12_NC: S + A
// R_AARCH64_LDST32_ABS_LO12_NC: S + A
// R_AARCH64_LDST64_ABS_LO12_NC: S + A
// R_AARCH64_LDST128_ABS_LO12_NC: S + A
Relocator::Result ldst_abs_lo12(Relocation& pReloc, AArch64Relocator& pParent) {
  Relocator::Address S = pReloc.symValue();
  Relocator::DWord A = pReloc.addend();
  Relocator::DWord X = helper_get_page_offset(S + A);

  switch (pReloc.type()) {
    case llvm::ELF::R_AARCH64_LDST8_ABS_LO12_NC:
      pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), X);
      break;
    case llvm::ELF::R_AARCH64_LDST16_ABS_LO12_NC:
      pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), (X >> 1));
      break;
    case llvm::ELF::R_AARCH64_LDST32_ABS_LO12_NC:
      pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), (X >> 2));
      break;
    case llvm::ELF::R_AARCH64_LDST64_ABS_LO12_NC:
      pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), (X >> 3));
      break;
    case llvm::ELF::R_AARCH64_LDST128_ABS_LO12_NC:
      pReloc.target() = helper_reencode_ldst_pos_imm(pReloc.target(), (X >> 4));
      break;
    default:
      break;
  }
  return Relocator::OK;
}

}  // namespace mcld
