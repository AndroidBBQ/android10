//===- MipsRelocator.cpp  -----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "MipsRelocator.h"
#include "MipsRelocationFunctions.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/OutputRelocSection.h"
#include "mcld/LD/ELFFileFormat.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/ELF.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// MipsRelocationInfo
//===----------------------------------------------------------------------===//
class MipsRelocationInfo {
 public:
  static bool HasSubType(const Relocation& pParent, Relocation::Type pType) {
    if (llvm::ELF::R_MIPS_NONE == pType)
      return true;

    for (Relocation::Type type = pParent.type();
         llvm::ELF::R_MIPS_NONE != (type & 0xff);
         type >>= 8) {
      if ((type & 0xff) == pType)
        return true;
    }

    return false;
  }

  MipsRelocationInfo(Relocation& pParent, bool pIsRel)
      : m_Parent(&pParent),
        m_Type(pParent.type()),
        m_Addend(pIsRel ? pParent.target() : pParent.addend()),
        m_Symbol(pParent.symValue()),
        m_Result(pParent.target()) {}

  bool isNone() const { return llvm::ELF::R_MIPS_NONE == type(); }
  bool isFirst() const { return type() == (parent().type() & 0xff); }
  bool isLast() const { return llvm::ELF::R_MIPS_NONE == (m_Type >> 8); }

  MipsRelocationInfo next() const {
    return MipsRelocationInfo(*m_Parent, m_Type >> 8, result(), result());
  }

  const Relocation& parent() const { return *m_Parent; }

  Relocation& parent() { return *m_Parent; }

  Relocation::Type type() const { return m_Type & 0xff; }

  Relocation::DWord A() const { return m_Addend; }

  Relocation::DWord S() const { return m_Symbol; }

  Relocation::DWord P() const { return parent().place(); }

  Relocation::DWord result() const { return m_Result; }

  Relocation::DWord& result() { return m_Result; }

 private:
  Relocation* m_Parent;
  Relocation::Type m_Type;
  Relocation::DWord m_Addend;
  Relocation::DWord m_Symbol;
  Relocation::DWord m_Result;

  MipsRelocationInfo(Relocation& pParent, Relocation::Type pType,
                     Relocation::DWord pResult, Relocation::DWord pAddend)
      : m_Parent(&pParent),
        m_Type(pType),
        m_Addend(pAddend),
        m_Symbol(0),
        m_Result(pResult) {}
};

static void helper_PLT_init(MipsRelocationInfo& pReloc,
                            MipsRelocator& pParent) {
  ResolveInfo* rsym = pReloc.parent().symInfo();
  assert(pParent.getSymPLTMap().lookUp(*rsym) == NULL && "PLT entry exists");

  MipsGNULDBackend& backend = pParent.getTarget();
  PLTEntryBase* pltEntry = backend.getPLT().create();
  pParent.getSymPLTMap().record(*rsym, *pltEntry);

  assert(pParent.getSymGOTPLTMap().lookUp(*rsym) == NULL &&
         "PLT entry not exist, but DynRel entry exist!");
  Fragment* gotpltEntry = backend.getGOTPLT().create();
  pParent.getSymGOTPLTMap().record(*rsym, *gotpltEntry);

  Relocation* relEntry = backend.getRelPLT().create();
  relEntry->setType(llvm::ELF::R_MIPS_JUMP_SLOT);
  relEntry->targetRef().assign(*gotpltEntry);
  relEntry->setSymInfo(rsym);
}

static Relocator::Address helper_get_PLT_address(ResolveInfo& pSym,
                                                 MipsRelocator& pParent) {
  PLTEntryBase* plt_entry = pParent.getSymPLTMap().lookUp(pSym);
  assert(plt_entry != NULL);
  return pParent.getTarget().getPLT().addr() + plt_entry->getOffset();
}

//===----------------------------------------------------------------------===//
// Relocation Functions and Tables
//===----------------------------------------------------------------------===//
DECL_MIPS_APPLY_RELOC_FUNCS

/// the prototype of applying function
typedef Relocator::Result (*ApplyFunctionType)(MipsRelocationInfo&,
                                               MipsRelocator& pParent);

// the table entry of applying functions
struct ApplyFunctionTriple {
  ApplyFunctionType func;
  unsigned int type;
  const char* name;
  unsigned int size;
};

// declare the table of applying functions
static const ApplyFunctionTriple ApplyFunctions[] = {
    DECL_MIPS_APPLY_RELOC_FUNC_PTRS};

//===----------------------------------------------------------------------===//
// MipsRelocator
//===----------------------------------------------------------------------===//
MipsRelocator::MipsRelocator(MipsGNULDBackend& pParent,
                             const LinkerConfig& pConfig)
    : Relocator(pConfig),
      m_Target(pParent),
      m_pApplyingInput(NULL),
      m_CurrentLo16Reloc(NULL) {
}

Relocator::Result MipsRelocator::applyRelocation(Relocation& pReloc) {
  // If m_CurrentLo16Reloc is not NULL we are processing
  // postponed relocation. Otherwise check relocation type
  // and postpone it for later handling.
  if (m_CurrentLo16Reloc == NULL && isPostponed(pReloc)) {
    postponeRelocation(pReloc);
    return OK;
  }

  for (MipsRelocationInfo info(pReloc, isRel()); !info.isNone();
       info = info.next()) {
    if (info.type() >= sizeof(ApplyFunctions) / sizeof(ApplyFunctions[0]))
      return Unknown;

    const ApplyFunctionTriple& triple = ApplyFunctions[info.type()];

    Result res = triple.func(info, *this);
    if (OK != res)
      return res;

    if (info.isLast()) {
      uint64_t mask = 0xFFFFFFFFFFFFFFFFULL >> (64 - triple.size);
      pReloc.target() &= ~mask;
      pReloc.target() |= info.result() & mask;
    }
  }

  return OK;
}

const char* MipsRelocator::getName(Relocation::Type pType) const {
  return ApplyFunctions[pType & 0xff].name;
}

void MipsRelocator::scanRelocation(Relocation& pReloc,
                                   IRBuilder& pBuilder,
                                   Module& pModule,
                                   LDSection& pSection,
                                   Input& pInput) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  assert(rsym != NULL &&
         "ResolveInfo of relocation not set while scanRelocation");

  // Skip relocation against _gp_disp
  if (getTarget().getGpDispSymbol() != NULL &&
      rsym == getTarget().getGpDispSymbol()->resolveInfo())
    return;

  assert(pSection.getLink() != NULL);
  if ((pSection.getLink()->flag() & llvm::ELF::SHF_ALLOC) == 0)
    return;

  for (MipsRelocationInfo info(pReloc, isRel()); !info.isNone();
       info = info.next()) {
    // We test isLocal or if pInputSym is not a dynamic symbol
    // We assume -Bsymbolic to bind all symbols internaly via !rsym->isDyn()
    // Don't put undef symbols into local entries.
    if (isLocalReloc(*rsym))
      scanLocalReloc(info, pBuilder, pSection);
    else
      scanGlobalReloc(info, pBuilder, pSection);

    if (getTarget().needsLA25Stub(info.type(), info.parent().symInfo()))
      getTarget().addNonPICBranchSym(pReloc.symInfo());
  }

  // Check if we should issue undefined reference
  // for the relocation target symbol.
  if (rsym->isUndef() && !rsym->isDyn() && !rsym->isWeak() && !rsym->isNull())
    issueUndefRef(pReloc, pSection, pInput);
}

bool MipsRelocator::initializeScan(Input& pInput) {
  if (LinkerConfig::Object != config().codeGenType())
    getTarget().getGOT().initializeScan(pInput);
  return true;
}

bool MipsRelocator::finalizeScan(Input& pInput) {
  if (LinkerConfig::Object != config().codeGenType())
    getTarget().getGOT().finalizeScan(pInput);
  return true;
}

bool MipsRelocator::initializeApply(Input& pInput) {
  m_pApplyingInput = &pInput;
  return true;
}

bool MipsRelocator::finalizeApply(Input& pInput) {
  m_pApplyingInput = NULL;
  return true;
}

void MipsRelocator::scanLocalReloc(MipsRelocationInfo& pReloc,
                                   IRBuilder& pBuilder,
                                   const LDSection& pSection) {
  ResolveInfo* rsym = pReloc.parent().symInfo();

  switch (pReloc.type()) {
    case llvm::ELF::R_MIPS_NONE:
    case llvm::ELF::R_MIPS_16:
      break;
    case llvm::ELF::R_MIPS_32:
    case llvm::ELF::R_MIPS_64:
      if (pReloc.isFirst() && LinkerConfig::DynObj == config().codeGenType()) {
        // TODO: (simon) The gold linker does not create an entry in .rel.dyn
        // section if the symbol section flags contains SHF_EXECINSTR.
        // 1. Find the reason of this condition.
        // 2. Check this condition here.
        getTarget().getRelDyn().reserveEntry();
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      break;
    case llvm::ELF::R_MIPS_REL32:
    case llvm::ELF::R_MIPS_26:
    case llvm::ELF::R_MIPS_HI16:
    case llvm::ELF::R_MIPS_LO16:
    case llvm::ELF::R_MIPS_SHIFT5:
    case llvm::ELF::R_MIPS_SHIFT6:
    case llvm::ELF::R_MIPS_SUB:
    case llvm::ELF::R_MIPS_INSERT_A:
    case llvm::ELF::R_MIPS_INSERT_B:
    case llvm::ELF::R_MIPS_DELETE:
    case llvm::ELF::R_MIPS_HIGHER:
    case llvm::ELF::R_MIPS_HIGHEST:
    case llvm::ELF::R_MIPS_SCN_DISP:
    case llvm::ELF::R_MIPS_REL16:
    case llvm::ELF::R_MIPS_ADD_IMMEDIATE:
    case llvm::ELF::R_MIPS_PJUMP:
    case llvm::ELF::R_MIPS_RELGOT:
    case llvm::ELF::R_MIPS_JALR:
    case llvm::ELF::R_MIPS_GLOB_DAT:
    case llvm::ELF::R_MIPS_COPY:
    case llvm::ELF::R_MIPS_JUMP_SLOT:
      break;
    case llvm::ELF::R_MIPS_GOT16:
    case llvm::ELF::R_MIPS_CALL16:
    case llvm::ELF::R_MIPS_GOT_HI16:
    case llvm::ELF::R_MIPS_CALL_HI16:
    case llvm::ELF::R_MIPS_GOT_LO16:
    case llvm::ELF::R_MIPS_CALL_LO16:
    case llvm::ELF::R_MIPS_GOT_DISP:
    case llvm::ELF::R_MIPS_GOT_PAGE:
    case llvm::ELF::R_MIPS_GOT_OFST:
      if (getTarget()
              .getGOT()
              .reserveLocalEntry(*rsym, pReloc.type(), pReloc.A())) {
        if (getTarget().getGOT().hasMultipleGOT())
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      break;
    case llvm::ELF::R_MIPS_GPREL32:
    case llvm::ELF::R_MIPS_GPREL16:
    case llvm::ELF::R_MIPS_LITERAL:
      break;
    case llvm::ELF::R_MIPS_TLS_GD:
      getTarget().getGOT().reserveTLSGdEntry(*rsym);
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_LDM:
      getTarget().getGOT().reserveTLSLdmEntry();
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_GOTTPREL:
      getTarget().getGOT().reserveTLSGotEntry(*rsym);
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_DTPMOD32:
    case llvm::ELF::R_MIPS_TLS_DTPREL32:
    case llvm::ELF::R_MIPS_TLS_DTPMOD64:
    case llvm::ELF::R_MIPS_TLS_DTPREL64:
    case llvm::ELF::R_MIPS_TLS_DTPREL_HI16:
    case llvm::ELF::R_MIPS_TLS_DTPREL_LO16:
    case llvm::ELF::R_MIPS_TLS_TPREL32:
    case llvm::ELF::R_MIPS_TLS_TPREL64:
    case llvm::ELF::R_MIPS_TLS_TPREL_HI16:
    case llvm::ELF::R_MIPS_TLS_TPREL_LO16:
      break;
    case llvm::ELF::R_MIPS_PC16:
    case llvm::ELF::R_MIPS_PC32:
    case llvm::ELF::R_MIPS_PC18_S3:
    case llvm::ELF::R_MIPS_PC19_S2:
    case llvm::ELF::R_MIPS_PC21_S2:
    case llvm::ELF::R_MIPS_PC26_S2:
    case llvm::ELF::R_MIPS_PCHI16:
    case llvm::ELF::R_MIPS_PCLO16:
      break;
    default:
      fatal(diag::unknown_relocation) << static_cast<int>(pReloc.type())
                                      << rsym->name();
  }
}

void MipsRelocator::scanGlobalReloc(MipsRelocationInfo& pReloc,
                                    IRBuilder& pBuilder,
                                    const LDSection& pSection) {
  ResolveInfo* rsym = pReloc.parent().symInfo();
  bool hasPLT = rsym->reserved() & ReservePLT;

  switch (pReloc.type()) {
    case llvm::ELF::R_MIPS_NONE:
    case llvm::ELF::R_MIPS_INSERT_A:
    case llvm::ELF::R_MIPS_INSERT_B:
    case llvm::ELF::R_MIPS_DELETE:
    case llvm::ELF::R_MIPS_TLS_DTPMOD64:
    case llvm::ELF::R_MIPS_TLS_DTPREL64:
    case llvm::ELF::R_MIPS_REL16:
    case llvm::ELF::R_MIPS_ADD_IMMEDIATE:
    case llvm::ELF::R_MIPS_PJUMP:
    case llvm::ELF::R_MIPS_RELGOT:
    case llvm::ELF::R_MIPS_TLS_TPREL64:
      break;
    case llvm::ELF::R_MIPS_32:
    case llvm::ELF::R_MIPS_64:
      if (pReloc.isFirst() &&
          getTarget().symbolNeedsDynRel(*rsym, hasPLT, true)) {
        getTarget().getRelDyn().reserveEntry();
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
        if (!getTarget().symbolFinalValueIsKnown(*rsym))
          getTarget().getGOT().reserveGlobalEntry(*rsym);
      }
      break;
    case llvm::ELF::R_MIPS_HI16:
    case llvm::ELF::R_MIPS_LO16:
      if (getTarget().symbolNeedsDynRel(*rsym, hasPLT, true) ||
          getTarget().symbolNeedsCopyReloc(pReloc.parent(), *rsym)) {
        getTarget().getRelDyn().reserveEntry();
        LDSymbol& cpySym = defineSymbolforCopyReloc(pBuilder, *rsym);
        addCopyReloc(*cpySym.resolveInfo());
      }
      break;
    case llvm::ELF::R_MIPS_GOT16:
    case llvm::ELF::R_MIPS_CALL16:
    case llvm::ELF::R_MIPS_GOT_DISP:
    case llvm::ELF::R_MIPS_GOT_HI16:
    case llvm::ELF::R_MIPS_CALL_HI16:
    case llvm::ELF::R_MIPS_GOT_LO16:
    case llvm::ELF::R_MIPS_CALL_LO16:
    case llvm::ELF::R_MIPS_GOT_PAGE:
    case llvm::ELF::R_MIPS_GOT_OFST:
      if (getTarget().getGOT().reserveGlobalEntry(*rsym)) {
        if (getTarget().getGOT().hasMultipleGOT())
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      break;
    case llvm::ELF::R_MIPS_LITERAL:
    case llvm::ELF::R_MIPS_GPREL32:
      fatal(diag::invalid_global_relocation) << static_cast<int>(pReloc.type())
                                             << rsym->name();
      break;
    case llvm::ELF::R_MIPS_GPREL16:
      break;
    case llvm::ELF::R_MIPS_26:
      // Create a PLT entry if the symbol requires it and does not have it.
      if (getTarget().symbolNeedsPLT(*rsym) && !hasPLT) {
        helper_PLT_init(pReloc, *this);
        rsym->setReserved(rsym->reserved() | ReservePLT);
      }
      break;
    case llvm::ELF::R_MIPS_16:
    case llvm::ELF::R_MIPS_SHIFT5:
    case llvm::ELF::R_MIPS_SHIFT6:
    case llvm::ELF::R_MIPS_SUB:
    case llvm::ELF::R_MIPS_HIGHER:
    case llvm::ELF::R_MIPS_HIGHEST:
    case llvm::ELF::R_MIPS_SCN_DISP:
      break;
    case llvm::ELF::R_MIPS_TLS_GD:
      getTarget().getGOT().reserveTLSGdEntry(*rsym);
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_LDM:
      getTarget().getGOT().reserveTLSLdmEntry();
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_GOTTPREL:
      getTarget().getGOT().reserveTLSGotEntry(*rsym);
      getTarget().checkAndSetHasTextRel(*pSection.getLink());
      break;
    case llvm::ELF::R_MIPS_TLS_DTPREL32:
    case llvm::ELF::R_MIPS_TLS_DTPREL_HI16:
    case llvm::ELF::R_MIPS_TLS_DTPREL_LO16:
    case llvm::ELF::R_MIPS_TLS_TPREL32:
    case llvm::ELF::R_MIPS_TLS_TPREL_HI16:
    case llvm::ELF::R_MIPS_TLS_TPREL_LO16:
      break;
    case llvm::ELF::R_MIPS_REL32:
    case llvm::ELF::R_MIPS_JALR:
    case llvm::ELF::R_MIPS_PC16:
    case llvm::ELF::R_MIPS_PC32:
    case llvm::ELF::R_MIPS_PC18_S3:
    case llvm::ELF::R_MIPS_PC19_S2:
    case llvm::ELF::R_MIPS_PC21_S2:
    case llvm::ELF::R_MIPS_PC26_S2:
    case llvm::ELF::R_MIPS_PCHI16:
    case llvm::ELF::R_MIPS_PCLO16:
      break;
    case llvm::ELF::R_MIPS_COPY:
    case llvm::ELF::R_MIPS_GLOB_DAT:
    case llvm::ELF::R_MIPS_JUMP_SLOT:
      fatal(diag::dynamic_relocation) << static_cast<int>(pReloc.type());
      break;
    default:
      fatal(diag::unknown_relocation) << static_cast<int>(pReloc.type())
                                      << rsym->name();
  }
}

bool MipsRelocator::isPostponed(const Relocation& pReloc) const {
  if (isN64ABI())
    return false;

  if (MipsRelocationInfo::HasSubType(pReloc, llvm::ELF::R_MIPS_HI16) ||
      MipsRelocationInfo::HasSubType(pReloc, llvm::ELF::R_MIPS_PCHI16))
    return true;

  if (MipsRelocationInfo::HasSubType(pReloc, llvm::ELF::R_MIPS_GOT16) &&
      pReloc.symInfo()->isLocal())
    return true;

  return false;
}

void MipsRelocator::addCopyReloc(ResolveInfo& pSym) {
  Relocation& relEntry = *getTarget().getRelDyn().consumeEntry();
  relEntry.setType(llvm::ELF::R_MIPS_COPY);
  assert(pSym.outSymbol()->hasFragRef());
  relEntry.targetRef().assign(*pSym.outSymbol()->fragRef());
  relEntry.setSymInfo(&pSym);
}

LDSymbol& MipsRelocator::defineSymbolforCopyReloc(IRBuilder& pBuilder,
                                                  const ResolveInfo& pSym) {
  // Get or create corresponding BSS LDSection
  ELFFileFormat* fileFormat = getTarget().getOutputFormat();
  LDSection* bssSectHdr = ResolveInfo::ThreadLocal == pSym.type()
                              ? &fileFormat->getTBSS()
                              : &fileFormat->getBSS();

  // Get or create corresponding BSS SectionData
  SectionData* bssData = bssSectHdr->hasSectionData()
                             ? bssSectHdr->getSectionData()
                             : IRBuilder::CreateSectionData(*bssSectHdr);

  // Determine the alignment by the symbol value
  // FIXME: here we use the largest alignment
  uint32_t addrAlign = config().targets().bitclass() / 8;

  // Allocate space in BSS for the copy symbol
  Fragment* frag = new FillFragment(0x0, 1, pSym.size());
  uint64_t size = ObjectBuilder::AppendFragment(*frag, *bssData, addrAlign);
  bssSectHdr->setSize(bssSectHdr->size() + size);

  // Change symbol binding to Global if it's a weak symbol
  ResolveInfo::Binding binding = (ResolveInfo::Binding)pSym.binding();
  if (binding == ResolveInfo::Weak)
    binding = ResolveInfo::Global;

  // Define the copy symbol in the bss section and resolve it
  LDSymbol* cpySym = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      pSym.name(),
      (ResolveInfo::Type)pSym.type(),
      ResolveInfo::Define,
      binding,
      pSym.size(),  // size
      0x0,          // value
      FragmentRef::Create(*frag, 0x0),
      (ResolveInfo::Visibility)pSym.other());

  // Output all other alias symbols if any
  Module::AliasList* alias_list = pBuilder.getModule().getAliasList(pSym);
  if (alias_list == NULL)
    return *cpySym;

  for (Module::alias_iterator it = alias_list->begin(), ie = alias_list->end();
       it != ie;
       ++it) {
    const ResolveInfo* alias = *it;
    if (alias == &pSym || !alias->isDyn())
      continue;

    pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        alias->name(),
        (ResolveInfo::Type)alias->type(),
        ResolveInfo::Define,
        binding,
        alias->size(),  // size
        0x0,            // value
        FragmentRef::Create(*frag, 0x0),
        (ResolveInfo::Visibility)alias->other());
  }

  return *cpySym;
}

void MipsRelocator::postponeRelocation(Relocation& pReloc) {
  ResolveInfo* rsym = pReloc.symInfo();
  m_PostponedRelocs[rsym].insert(&pReloc);
}

void MipsRelocator::applyPostponedRelocations(MipsRelocationInfo& pLo16Reloc) {
  m_CurrentLo16Reloc = &pLo16Reloc;

  ResolveInfo* rsym = pLo16Reloc.parent().symInfo();

  RelocationSet& relocs = m_PostponedRelocs[rsym];
  for (RelocationSet::iterator it = relocs.begin(); it != relocs.end(); ++it)
    (*it)->apply(*this);

  m_PostponedRelocs.erase(rsym);

  m_CurrentLo16Reloc = NULL;
}

bool MipsRelocator::isGpDisp(const Relocation& pReloc) const {
  return strcmp("_gp_disp", pReloc.symInfo()->name()) == 0;
}

bool MipsRelocator::isRel() const {
  return config().targets().is32Bits();
}

bool MipsRelocator::isLocalReloc(ResolveInfo& pSym) const {
  if (pSym.isUndef())
    return false;

  return pSym.isLocal() || !getTarget().isDynamicSymbol(pSym) || !pSym.isDyn();
}

Relocator::Address MipsRelocator::getGPAddress() {
  return getTarget().getGOT().getGPAddr(getApplyingInput());
}

Relocator::Address MipsRelocator::getTPOffset() {
  return getTarget().getTPOffset(getApplyingInput());
}

Relocator::Address MipsRelocator::getDTPOffset() {
  return getTarget().getDTPOffset(getApplyingInput());
}

Relocator::Address MipsRelocator::getGP0() {
  return getTarget().getGP0(getApplyingInput());
}

Fragment& MipsRelocator::getLocalGOTEntry(MipsRelocationInfo& pReloc,
                                          Relocation::DWord entryValue) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.parent().symInfo();
  MipsGOT& got = getTarget().getGOT();

  assert(isLocalReloc(*rsym) &&
         "Attempt to get a global GOT entry for the local relocation");

  Fragment* got_entry = got.lookupLocalEntry(rsym, entryValue);

  // Found a mapping, then return the mapped entry immediately.
  if (got_entry != NULL)
    return *got_entry;

  // Not found.
  got_entry = got.consumeLocal();

  if (got.isPrimaryGOTConsumed())
    setupRel32DynEntry(*FragmentRef::Create(*got_entry, 0), NULL);
  else
    got.setEntryValue(got_entry, entryValue);

  got.recordLocalEntry(rsym, entryValue, got_entry);

  return *got_entry;
}

Fragment& MipsRelocator::getGlobalGOTEntry(MipsRelocationInfo& pReloc) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.parent().symInfo();
  MipsGOT& got = getTarget().getGOT();

  assert(!isLocalReloc(*rsym) &&
         "Attempt to get a local GOT entry for the global relocation");

  Fragment* got_entry = got.lookupGlobalEntry(rsym);

  // Found a mapping, then return the mapped entry immediately.
  if (got_entry != NULL)
    return *got_entry;

  // Not found.
  got_entry = got.consumeGlobal();

  if (got.isPrimaryGOTConsumed())
    setupRel32DynEntry(*FragmentRef::Create(*got_entry, 0), rsym);
  else
    got.setEntryValue(got_entry, pReloc.parent().symValue());

  got.recordGlobalEntry(rsym, got_entry);

  return *got_entry;
}

Fragment& MipsRelocator::getTLSGOTEntry(MipsRelocationInfo& pReloc) {
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.parent().symInfo();
  MipsGOT& got = getTarget().getGOT();

  Fragment* modEntry = got.lookupTLSEntry(rsym, pReloc.type());

  // Found a mapping, then return the mapped entry immediately.
  if (modEntry != NULL)
    return *modEntry;

  // Not found.
  modEntry = got.consumeTLS(pReloc.type());
  setupTLSDynEntry(*modEntry, rsym, pReloc.type());
  got.recordTLSEntry(rsym, modEntry, pReloc.type());

  return *modEntry;
}

Relocator::Address MipsRelocator::getGOTOffset(MipsRelocationInfo& pReloc) {
  ResolveInfo* rsym = pReloc.parent().symInfo();
  MipsGOT& got = getTarget().getGOT();

  if (isLocalReloc(*rsym)) {
    uint64_t value = pReloc.S();

    if (ResolveInfo::Section == rsym->type())
      value += pReloc.A();

    return got.getGPRelOffset(getApplyingInput(),
                              getLocalGOTEntry(pReloc, value));
  } else {
    return got.getGPRelOffset(getApplyingInput(), getGlobalGOTEntry(pReloc));
  }
}

Relocator::Address MipsRelocator::getTLSGOTOffset(MipsRelocationInfo& pReloc) {
  MipsGOT& got = getTarget().getGOT();
  return got.getGPRelOffset(getApplyingInput(), getTLSGOTEntry(pReloc));
}

void MipsRelocator::createDynRel(MipsRelocationInfo& pReloc) {
  Relocator::DWord A = pReloc.A();
  Relocator::DWord S = pReloc.S();

  ResolveInfo* rsym = pReloc.parent().symInfo();

  if (getTarget().isDynamicSymbol(*rsym)) {
    setupRel32DynEntry(pReloc.parent().targetRef(), rsym);
    // Don't add symbol value that will be resolved by the dynamic linker.
    pReloc.result() = A;
  } else {
    setupRel32DynEntry(pReloc.parent().targetRef(), NULL);
    pReloc.result() = A + S;
  }

  if (!isLocalReloc(*rsym) && !getTarget().symbolFinalValueIsKnown(*rsym))
    getGlobalGOTEntry(pReloc);
}

uint64_t MipsRelocator::calcAHL(const MipsRelocationInfo& pHiReloc) {
  if (isN64ABI())
    return pHiReloc.A();

  assert(m_CurrentLo16Reloc != NULL &&
         "There is no saved R_MIPS_LO16 relocation");

  uint64_t AHI = pHiReloc.A() & 0xFFFF;
  uint64_t ALO = m_CurrentLo16Reloc->A() & 0xFFFF;
  uint64_t AHL = (AHI << 16) + int16_t(ALO);

  return AHL;
}

bool MipsRelocator::isN64ABI() const {
  return config().targets().is64Bits();
}

uint32_t MipsRelocator::getDebugStringOffset(Relocation& pReloc) const {
  if (pReloc.type() != llvm::ELF::R_MIPS_32)
    error(diag::unsupport_reloc_for_debug_string)
        << getName(pReloc.type()) << "mclinker@googlegroups.com";
  if (pReloc.symInfo()->type() == ResolveInfo::Section)
    return pReloc.target() + pReloc.addend();
  else
    return pReloc.symInfo()->outSymbol()->fragRef()->offset() +
               pReloc.target() + pReloc.addend();
}

void MipsRelocator::applyDebugStringOffset(Relocation& pReloc,
                                           uint32_t pOffset) {
  pReloc.target() = pOffset;
}

void MipsRelocator::setupRelDynEntry(FragmentRef& pFragRef, ResolveInfo* pSym,
                                     Relocation::Type pType) {
  Relocation& relEntry = *getTarget().getRelDyn().consumeEntry();
  relEntry.setType(pType);
  relEntry.targetRef() = pFragRef;
  relEntry.setSymInfo(pSym);
}

//===----------------------------------------------------------------------===//
// Mips32Relocator
//===----------------------------------------------------------------------===//
Mips32Relocator::Mips32Relocator(Mips32GNULDBackend& pParent,
                                 const LinkerConfig& pConfig)
    : MipsRelocator(pParent, pConfig) {
}

void Mips32Relocator::setupRel32DynEntry(FragmentRef& pFragRef,
                                         ResolveInfo* pSym) {
  setupRelDynEntry(pFragRef, pSym, llvm::ELF::R_MIPS_REL32);
}

void Mips32Relocator::setupTLSDynEntry(Fragment& pFrag, ResolveInfo* pSym,
                                       Relocation::Type pType) {
  pSym = pSym->isLocal() ? nullptr : pSym;
  if (pType == llvm::ELF::R_MIPS_TLS_GD) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPMOD32);
    FragmentRef& relFrag = *FragmentRef::Create(*pFrag.getNextNode(), 0);
    setupRelDynEntry(relFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPREL32);
  } else if (pType == llvm::ELF::R_MIPS_TLS_LDM) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPMOD32);
  } else if (pType == llvm::ELF::R_MIPS_TLS_GOTTPREL) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_TPREL32);
  } else {
    llvm_unreachable("Unexpected relocation");
  }
}

Relocator::Size Mips32Relocator::getSize(Relocation::Type pType) const {
  return ApplyFunctions[pType & 0xff].size;
}

//===----------------------------------------------------------------------===//
// Mips64Relocator
//===----------------------------------------------------------------------===//
Mips64Relocator::Mips64Relocator(Mips64GNULDBackend& pParent,
                                 const LinkerConfig& pConfig)
    : MipsRelocator(pParent, pConfig) {
}

void Mips64Relocator::setupRel32DynEntry(FragmentRef& pFragRef,
                                         ResolveInfo* pSym) {
  Relocation::Type type = llvm::ELF::R_MIPS_REL32 | llvm::ELF::R_MIPS_64 << 8;
  setupRelDynEntry(pFragRef, pSym, type);
}

void Mips64Relocator::setupTLSDynEntry(Fragment& pFrag, ResolveInfo* pSym,
                                       Relocation::Type pType) {
  pSym = pSym->isLocal() ? nullptr : pSym;
  if (pType == llvm::ELF::R_MIPS_TLS_GD) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPMOD64);
    FragmentRef& relFrag = *FragmentRef::Create(*pFrag.getNextNode(), 0);
    setupRelDynEntry(relFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPREL64);
  } else if (pType == llvm::ELF::R_MIPS_TLS_LDM) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_DTPMOD64);
  } else if (pType == llvm::ELF::R_MIPS_TLS_GOTTPREL) {
    FragmentRef& modFrag = *FragmentRef::Create(pFrag, 0);
    setupRelDynEntry(modFrag, pSym, llvm::ELF::R_MIPS_TLS_TPREL64);
  } else {
    llvm_unreachable("Unexpected relocation");
  }
}

Relocator::Size Mips64Relocator::getSize(Relocation::Type pType) const {
  if (((pType >> 16) & 0xff) != llvm::ELF::R_MIPS_NONE)
    return ApplyFunctions[(pType >> 16) & 0xff].size;
  if (((pType >> 8) & 0xff) != llvm::ELF::R_MIPS_NONE)
    return ApplyFunctions[(pType >> 8) & 0xff].size;
  return ApplyFunctions[pType & 0xff].size;
}

//=========================================//
// Relocation functions implementation     //
//=========================================//

// R_MIPS_NONE and those unsupported/deprecated relocation type
static MipsRelocator::Result none(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  return Relocator::OK;
}

// R_MIPS_32: S + A
static MipsRelocator::Result abs32(MipsRelocationInfo& pReloc,
                                   MipsRelocator& pParent) {
  ResolveInfo* rsym = pReloc.parent().symInfo();

  Relocator::DWord A = pReloc.A();
  Relocator::DWord S = pReloc.S();

  LDSection& target_sect =
      pReloc.parent().targetRef().frag()->getParent()->getSection();

  // If the flag of target section is not ALLOC, we will not scan this
  // relocation
  // but perform static relocation. (e.g., applying .debug section)
  if ((llvm::ELF::SHF_ALLOC & target_sect.flag()) == 0x0) {
    pReloc.result() = S + A;
    return Relocator::OK;
  }

  if (rsym->reserved() & MipsRelocator::ReserveRel) {
    pParent.createDynRel(pReloc);
    return Relocator::OK;
  }

  pReloc.result() = S + A;

  return Relocator::OK;
}

// R_MIPS_26:
//   local   : ((A | ((P + 4) & 0x3F000000)) + S) >> 2
//   external: (sign–extend(A) + S) >> 2
static MipsRelocator::Result rel26(MipsRelocationInfo& pReloc,
                                   MipsRelocator& pParent) {
  ResolveInfo* rsym = pReloc.parent().symInfo();

  int32_t A = pParent.isN64ABI() ? pReloc.A() : (pReloc.A() & 0x03FFFFFF) << 2;
  int32_t P = pReloc.P();
  int32_t S = rsym->reserved() & MipsRelocator::ReservePLT
                  ? helper_get_PLT_address(*rsym, pParent)
                  : pReloc.S();

  if (rsym->isLocal())
    pReloc.result() = A | ((P + 4) & 0x3F000000);
  else
    pReloc.result() = signExtend<28>(A);

  pReloc.result() = (pReloc.result() + S) >> 2;

  return Relocator::OK;
}

// R_MIPS_HI16:
//   local/external: ((AHL + S) - (short)(AHL + S)) >> 16
//   _gp_disp      : ((AHL + GP - P) - (short)(AHL + GP - P)) >> 16
static MipsRelocator::Result hi16(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  uint64_t AHL = pParent.calcAHL(pReloc);

  if (pParent.isGpDisp(pReloc.parent())) {
    int32_t P = pReloc.P();
    int32_t GP = pParent.getGPAddress();
    pReloc.result() = ((AHL + GP - P) - (int16_t)(AHL + GP - P)) >> 16;
  } else {
    int32_t S = pReloc.S();
    if (pParent.isN64ABI())
      pReloc.result() = (pReloc.A() + S + 0x8000ull) >> 16;
    else
      pReloc.result() = ((AHL + S) - (int16_t)(AHL + S)) >> 16;
  }

  return Relocator::OK;
}

// R_MIPS_LO16:
//   local/external: AHL + S
//   _gp_disp      : AHL + GP - P + 4
static MipsRelocator::Result lo16(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  // AHL is a combination of HI16 and LO16 addends. But R_MIPS_LO16
  // uses low 16 bits of the AHL. That is why we do not need R_MIPS_HI16
  // addend here.
  int32_t AHL = (pReloc.A() & 0xFFFF);

  if (pParent.isGpDisp(pReloc.parent())) {
    int32_t P = pReloc.P();
    int32_t GP = pParent.getGPAddress();
    pReloc.result() = AHL + GP - P + 4;
  } else {
    int32_t S = pReloc.S();
    pReloc.result() = AHL + S;
  }

  pParent.applyPostponedRelocations(pReloc);

  return Relocator::OK;
}

// R_MIPS_GPREL16:
//   external: sign–extend(A) + S - GP
//   local   : sign–extend(A) + S + GP0 – GP
static MipsRelocator::Result gprel16(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  // Remember to add the section offset to A.
  uint64_t A = pReloc.A();
  uint64_t S = pReloc.S();
  uint64_t GP0 = pParent.getGP0();
  uint64_t GP = pParent.getGPAddress();

  ResolveInfo* rsym = pReloc.parent().symInfo();
  if (rsym->isLocal())
    pReloc.result() = A + S + GP0 - GP;
  else
    pReloc.result() = A + S - GP;

  return Relocator::OK;
}

// R_MIPS_GOT16:
//   local   : G (calculate AHL and put high 16 bit to GOT)
//   external: G
static MipsRelocator::Result got16(MipsRelocationInfo& pReloc,
                                   MipsRelocator& pParent) {
  if (pReloc.parent().symInfo()->isLocal()) {
    int32_t AHL = pParent.calcAHL(pReloc);
    int32_t S = pReloc.S();
    int32_t res = (AHL + S + 0x8000) & 0xFFFF0000;

    MipsGOT& got = pParent.getTarget().getGOT();

    Fragment& got_entry = pParent.getLocalGOTEntry(pReloc, res);

    pReloc.result() = got.getGPRelOffset(pParent.getApplyingInput(), got_entry);
  } else {
    pReloc.result() = pParent.getGOTOffset(pReloc);
  }

  return Relocator::OK;
}

// R_MIPS_GOTHI16:
//   external: (G - (short)G) >> 16 + A
static MipsRelocator::Result gothi16(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  Relocator::Address G = pParent.getGOTOffset(pReloc);
  int32_t A = pReloc.A();

  pReloc.result() = (G - (int16_t)G) >> (16 + A);

  return Relocator::OK;
}

// R_MIPS_GOTLO16:
//   external: G & 0xffff
static MipsRelocator::Result gotlo16(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  pReloc.result() = pParent.getGOTOffset(pReloc) & 0xffff;

  return Relocator::OK;
}

// R_MIPS_SUB:
//   external/local: S - A
static MipsRelocator::Result sub(MipsRelocationInfo& pReloc,
                                 MipsRelocator& pParent) {
  uint64_t S = pReloc.S();
  uint64_t A = pReloc.A();

  pReloc.result() = S - A;

  return Relocator::OK;
}

// R_MIPS_CALL16: G
static MipsRelocator::Result call16(MipsRelocationInfo& pReloc,
                                    MipsRelocator& pParent) {
  pReloc.result() = pParent.getGOTOffset(pReloc);

  return Relocator::OK;
}

// R_MIPS_GPREL32: A + S + GP0 - GP
static MipsRelocator::Result gprel32(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  // Remember to add the section offset to A.
  uint64_t A = pReloc.A();
  uint64_t S = pReloc.S();
  uint64_t GP0 = pParent.getGP0();
  uint64_t GP = pParent.getGPAddress();

  pReloc.result() = A + S + GP0 - GP;

  return Relocator::OK;
}

// R_MIPS_64: S + A
static MipsRelocator::Result abs64(MipsRelocationInfo& pReloc,
                                   MipsRelocator& pParent) {
  // FIXME (simon): Consider to merge with abs32() or use the same function
  // but with another mask size.
  ResolveInfo* rsym = pReloc.parent().symInfo();

  Relocator::DWord A = pReloc.A();
  Relocator::DWord S = pReloc.S();

  LDSection& target_sect =
      pReloc.parent().targetRef().frag()->getParent()->getSection();

  // If the flag of target section is not ALLOC, we will not scan this
  // relocation
  // but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.result() = S + A;
    return Relocator::OK;
  }

  if (rsym->reserved() & MipsRelocator::ReserveRel) {
    pParent.createDynRel(pReloc);
    return Relocator::OK;
  }

  pReloc.result() = S + A;

  return Relocator::OK;
}

// R_MIPS_GOT_DISP / R_MIPS_GOT_PAGE: G
static MipsRelocator::Result gotdisp(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  pReloc.result() = pParent.getGOTOffset(pReloc);

  return Relocator::OK;
}

// R_MIPS_GOT_OFST:
static MipsRelocator::Result gotoff(MipsRelocationInfo& pReloc,
                                    MipsRelocator& pParent) {
  // FIXME (simon): Needs to be implemented.
  return Relocator::OK;
}

// R_MIPS_JALR:
static MipsRelocator::Result jalr(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  return Relocator::OK;
}

// R_MIPS_PC16
static MipsRelocator::Result pc16(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  int64_t A = signExtend<18>(pReloc.A() << 2);
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = (A + S - P) >> 2;
  return Relocator::OK;
}

// R_MIPS_PC32
static MipsRelocator::Result pc32(MipsRelocationInfo& pReloc,
                                  MipsRelocator& pParent) {
  int64_t A = pReloc.A();
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = A + S - P;
  return Relocator::OK;
}

// R_MIPS_PC18_S3
static MipsRelocator::Result pc18_s3(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  int64_t A = signExtend<21>(pReloc.A() << 3);
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = (S + A - ((P | 7) ^ 7)) >> 3;
  return Relocator::OK;
}

// R_MIPS_PC19_S2
static MipsRelocator::Result pc19_s2(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  int64_t A = signExtend<21>(pReloc.A() << 2);
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = (A + S - P) >> 2;
  return Relocator::OK;
}

// R_MIPS_PC21_S2
static MipsRelocator::Result pc21_s2(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  int32_t A = signExtend<23>(pReloc.A() << 2);
  int32_t S = pReloc.S();
  int32_t P = pReloc.P();
  pReloc.result() = (A + S - P) >> 2;
  return Relocator::OK;
}

// R_MIPS_PC26_S2
static MipsRelocator::Result pc26_s2(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  int64_t A = signExtend<28>(pReloc.A() << 2);
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = (A + S - P) >> 2;
  return Relocator::OK;
}

// R_MIPS_PCHI16
static MipsRelocator::Result pchi16(MipsRelocationInfo& pReloc,
                                    MipsRelocator& pParent) {
  uint64_t AHL = pParent.calcAHL(pReloc);
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = (S + AHL - P + 0x8000) >> 16;
  return Relocator::OK;
}

// R_MIPS_PCLO16
static MipsRelocator::Result pclo16(MipsRelocationInfo& pReloc,
                                    MipsRelocator& pParent) {
  int32_t AHL = pReloc.A() & 0xFFFF;
  int64_t S = pReloc.S();
  int64_t P = pReloc.P();
  pReloc.result() = S + AHL - P;
  pParent.applyPostponedRelocations(pReloc);
  return Relocator::OK;
}

// R_MIPS_TLS_TPREL_HI16, R_MIPS_TLS_DTPREL_HI16
//   local/external: (A + S - TP Offset) >> 16
//   _gp_disp      : (A + GP - P - TP Offset) >> 16
static MipsRelocator::Result tlshi16(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  uint64_t A = pReloc.A() & 0xFFFF;
  if (pReloc.type() == llvm::ELF::R_MIPS_TLS_TPREL_HI16)
    A -= pParent.getTPOffset();
  else if (pReloc.type() == llvm::ELF::R_MIPS_TLS_DTPREL_HI16)
    A -= pParent.getDTPOffset();
  else
    llvm_unreachable("Unexpected relocation");

  if (pParent.isGpDisp(pReloc.parent()))
    pReloc.result() = (A + pReloc.S() - pReloc.P() + 0x8000) >> 16;
  else
    pReloc.result() = (A + pReloc.S() + 0x8000) >> 16;

  return Relocator::OK;
}

// R_MIPS_TLS_TPREL_LO16, R_MIPS_TLS_DTPREL_LO16
//   local/external: A + S - TP Offset
//   _gp_disp      : A + GP - P + 4 - TP Offset
static MipsRelocator::Result tlslo16(MipsRelocationInfo& pReloc,
                                     MipsRelocator& pParent) {
  uint64_t A = pReloc.A() & 0xFFFF;
  if (pReloc.type() == llvm::ELF::R_MIPS_TLS_TPREL_LO16)
    A -= pParent.getTPOffset();
  else if (pReloc.type() == llvm::ELF::R_MIPS_TLS_DTPREL_LO16)
    A -= pParent.getDTPOffset();
  else
    llvm_unreachable("Unexpected relocation");

  if (pParent.isGpDisp(pReloc.parent()))
    pReloc.result() = A + pReloc.S() - pReloc.P() + 4;
  else
    pReloc.result() = A + pReloc.S();

  return Relocator::OK;
}

// R_MIPS_TLS_GD, R_MIPS_TLS_LDM
static MipsRelocator::Result tlsgot(MipsRelocationInfo& pReloc,
                                    MipsRelocator& pParent) {
  pReloc.result() = pParent.getTLSGOTOffset(pReloc);
  return Relocator::OK;
}

static MipsRelocator::Result unsupported(MipsRelocationInfo& pReloc,
                                         MipsRelocator& pParent) {
  return Relocator::Unsupported;
}

}  // namespace mcld
