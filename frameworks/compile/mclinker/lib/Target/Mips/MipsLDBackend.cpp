//===- MipsLDBackend.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Mips.h"
#include "MipsGNUInfo.h"
#include "MipsELFDynamic.h"
#include "MipsLA25Stub.h"
#include "MipsLDBackend.h"
#include "MipsRelocator.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Module.h"
#include "mcld/Fragment/AlignFragment.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/LD/BranchIslandFactory.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/StubFactory.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/ELFSegment.h"
#include "mcld/LD/ELFSegmentFactory.h"
#include "mcld/MC/Attribute.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/MemoryRegion.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/OutputRelocSection.h"

#include <llvm/ADT/Triple.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MipsABIFlags.h>

#include <vector>

namespace mcld {

//===----------------------------------------------------------------------===//
// MipsGNULDBackend
//===----------------------------------------------------------------------===//
MipsGNULDBackend::MipsGNULDBackend(const LinkerConfig& pConfig,
                                   MipsGNUInfo* pInfo)
    : GNULDBackend(pConfig, pInfo),
      m_pRelocator(NULL),
      m_pGOT(NULL),
      m_pPLT(NULL),
      m_pGOTPLT(NULL),
      m_pInfo(*pInfo),
      m_pRelPlt(NULL),
      m_pRelDyn(NULL),
      m_pDynamic(NULL),
      m_pAbiFlags(NULL),
      m_pGOTSymbol(NULL),
      m_pPLTSymbol(NULL),
      m_pGpDispSymbol(NULL) {
}

MipsGNULDBackend::~MipsGNULDBackend() {
  delete m_pRelocator;
  delete m_pPLT;
  delete m_pRelPlt;
  delete m_pRelDyn;
  delete m_pDynamic;
}

bool MipsGNULDBackend::needsLA25Stub(Relocation::Type pType,
                                     const mcld::ResolveInfo* pSym) {
  if (config().isCodeIndep())
    return false;

  if (llvm::ELF::R_MIPS_26 != pType)
    return false;

  if (pSym->isLocal())
    return false;

  return true;
}

void MipsGNULDBackend::addNonPICBranchSym(ResolveInfo* rsym) {
  m_HasNonPICBranchSyms.insert(rsym);
}

bool MipsGNULDBackend::hasNonPICBranch(const ResolveInfo* rsym) const {
  return m_HasNonPICBranchSyms.count(rsym);
}

void MipsGNULDBackend::initTargetSections(Module& pModule,
                                          ObjectBuilder& pBuilder) {
  if (LinkerConfig::Object == config().codeGenType())
    return;

  ELFFileFormat* file_format = getOutputFormat();

  // initialize .rel.plt
  LDSection& relplt = file_format->getRelPlt();
  m_pRelPlt = new OutputRelocSection(pModule, relplt);

  // initialize .rel.dyn
  LDSection& reldyn = file_format->getRelDyn();
  m_pRelDyn = new OutputRelocSection(pModule, reldyn);

  // initialize .sdata
  m_psdata = pBuilder.CreateSection(
      ".sdata", LDFileFormat::Target, llvm::ELF::SHT_PROGBITS,
      llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE | llvm::ELF::SHF_MIPS_GPREL,
      4);

  // initialize .MIPS.abiflags
  m_pAbiFlags = pBuilder.CreateSection(".MIPS.abiflags", LDFileFormat::Target,
                                       llvm::ELF::SHT_MIPS_ABIFLAGS,
                                       llvm::ELF::SHF_ALLOC, 4);
}

void MipsGNULDBackend::initTargetSymbols(IRBuilder& pBuilder, Module& pModule) {
  // Define the symbol _GLOBAL_OFFSET_TABLE_ if there is a symbol with the
  // same name in input
  m_pGOTSymbol = pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
      "_GLOBAL_OFFSET_TABLE_",
      ResolveInfo::Object,
      ResolveInfo::Define,
      ResolveInfo::Local,
      0x0,                  // size
      0x0,                  // value
      FragmentRef::Null(),  // FragRef
      ResolveInfo::Hidden);

  // Define the symbol _PROCEDURE_LINKAGE_TABLE_ if there is a symbol with the
  // same name in input
  m_pPLTSymbol = pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
      "_PROCEDURE_LINKAGE_TABLE_",
      ResolveInfo::Object,
      ResolveInfo::Define,
      ResolveInfo::Local,
      0x0,                  // size
      0x0,                  // value
      FragmentRef::Null(),  // FragRef
      ResolveInfo::Hidden);

  m_pGpDispSymbol =
      pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
          "_gp_disp",
          ResolveInfo::Section,
          ResolveInfo::Define,
          ResolveInfo::Absolute,
          0x0,                  // size
          0x0,                  // value
          FragmentRef::Null(),  // FragRef
          ResolveInfo::Default);

  pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Unresolve>(
      "_gp",
      ResolveInfo::NoType,
      ResolveInfo::Define,
      ResolveInfo::Absolute,
      0x0,                  // size
      0x0,                  // value
      FragmentRef::Null(),  // FragRef
      ResolveInfo::Default);
}

const Relocator* MipsGNULDBackend::getRelocator() const {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

Relocator* MipsGNULDBackend::getRelocator() {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

void MipsGNULDBackend::doPreLayout(IRBuilder& pBuilder) {
  // initialize .dynamic data
  if (!config().isCodeStatic() && m_pDynamic == NULL)
    m_pDynamic = new MipsELFDynamic(*this, config());

  if (m_pAbiInfo.hasValue())
    m_pAbiFlags->setSize(m_pAbiInfo->size());

  // set .got size
  // when building shared object, the .got section is must.
  if (LinkerConfig::Object != config().codeGenType()) {
    if (LinkerConfig::DynObj == config().codeGenType() || m_pGOT->hasGOT1() ||
        m_pGOTSymbol != NULL) {
      m_pGOT->finalizeScanning(*m_pRelDyn);
      m_pGOT->finalizeSectionSize();

      defineGOTSymbol(pBuilder);
    }

    if (m_pGOTPLT->hasGOT1()) {
      m_pGOTPLT->finalizeSectionSize();

      defineGOTPLTSymbol(pBuilder);
    }

    if (m_pPLT->hasPLT1())
      m_pPLT->finalizeSectionSize();

    ELFFileFormat* file_format = getOutputFormat();

    // set .rel.plt size
    if (!m_pRelPlt->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelPlt().setSize(m_pRelPlt->numOfRelocs() *
                                       getRelEntrySize());
    }

    // set .rel.dyn size
    if (!m_pRelDyn->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelDyn().setSize(m_pRelDyn->numOfRelocs() *
                                       getRelEntrySize());
    }
  }
}

void MipsGNULDBackend::doPostLayout(Module& pModule, IRBuilder& pBuilder) {
  const ELFFileFormat* format = getOutputFormat();

  if (format->hasGOTPLT()) {
    assert(m_pGOTPLT != NULL && "doPostLayout failed, m_pGOTPLT is NULL!");
    m_pGOTPLT->applyAllGOTPLT(m_pPLT->addr());
  }

  if (format->hasPLT()) {
    assert(m_pPLT != NULL && "doPostLayout failed, m_pPLT is NULL!");
    m_pPLT->applyAllPLT(*m_pGOTPLT);
  }

  m_pInfo.setABIVersion(m_pPLT && m_pPLT->hasPLT1() ? 1 : 0);
}

/// dynamic - the dynamic section of the target machine.
/// Use co-variant return type to return its own dynamic section.
MipsELFDynamic& MipsGNULDBackend::dynamic() {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

/// dynamic - the dynamic section of the target machine.
/// Use co-variant return type to return its own dynamic section.
const MipsELFDynamic& MipsGNULDBackend::dynamic() const {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

uint64_t MipsGNULDBackend::emitSectionData(const LDSection& pSection,
                                           MemoryRegion& pRegion) const {
  assert(pRegion.size() && "Size of MemoryRegion is zero!");

  const ELFFileFormat* file_format = getOutputFormat();

  if (file_format->hasGOT() && (&pSection == &(file_format->getGOT()))) {
    return m_pGOT->emit(pRegion);
  }

  if (file_format->hasPLT() && (&pSection == &(file_format->getPLT()))) {
    return m_pPLT->emit(pRegion);
  }

  if (file_format->hasGOTPLT() && (&pSection == &(file_format->getGOTPLT()))) {
    return m_pGOTPLT->emit(pRegion);
  }

  if (&pSection == m_pAbiFlags && m_pAbiInfo.hasValue())
    return MipsAbiFlags::emit(*m_pAbiInfo, pRegion);

  if (&pSection == m_psdata && m_psdata->hasSectionData()) {
    const SectionData* sect_data = pSection.getSectionData();
    SectionData::const_iterator frag_iter, frag_end = sect_data->end();
    uint8_t* out_offset = pRegion.begin();
    for (frag_iter = sect_data->begin(); frag_iter != frag_end; ++frag_iter) {
      size_t size = frag_iter->size();
      switch (frag_iter->getKind()) {
        case Fragment::Fillment: {
          const FillFragment& fill_frag = llvm::cast<FillFragment>(*frag_iter);
          if (fill_frag.getValueSize() == 0) {
            // virtual fillment, ignore it.
            break;
          }
          memset(out_offset, fill_frag.getValue(), fill_frag.size());
          break;
        }
        case Fragment::Region: {
          const RegionFragment& region_frag =
              llvm::cast<RegionFragment>(*frag_iter);
          const char* start = region_frag.getRegion().begin();
          memcpy(out_offset, start, size);
          break;
        }
        case Fragment::Alignment: {
          const AlignFragment& align_frag =
              llvm::cast<AlignFragment>(*frag_iter);
          uint64_t count = size / align_frag.getValueSize();
          switch (align_frag.getValueSize()) {
            case 1u:
              std::memset(out_offset, align_frag.getValue(), count);
              break;
            default:
              llvm::report_fatal_error(
                  "unsupported value size for align fragment emission yet.\n");
              break;
          }  // end switch
          break;
        }
        case Fragment::Null: {
          assert(0x0 == size);
          break;
        }
        default:
          llvm::report_fatal_error("unsupported fragment type.\n");
          break;
      }  // end switch
      out_offset += size;
    }
    return pRegion.size();
  }

  fatal(diag::unrecognized_output_sectoin) << pSection.name()
                                           << "mclinker@googlegroups.com";
  return 0;
}

bool MipsGNULDBackend::hasEntryInStrTab(const LDSymbol& pSym) const {
  return ResolveInfo::Section != pSym.type() || m_pGpDispSymbol == &pSym;
}

namespace {
struct DynsymGOTCompare {
  const MipsGOT& m_pGOT;

  explicit DynsymGOTCompare(const MipsGOT& pGOT) : m_pGOT(pGOT) {}

  bool operator()(const LDSymbol* X, const LDSymbol* Y) const {
    return m_pGOT.dynSymOrderCompare(X, Y);
  }
};
}  // anonymous namespace

void MipsGNULDBackend::orderSymbolTable(Module& pModule) {
  if (config().options().hasGNUHash()) {
    // The MIPS ABI and .gnu.hash require .dynsym to be sorted
    // in different ways. The MIPS ABI requires a mapping between
    // the GOT and the symbol table. At the same time .gnu.hash
    // needs symbols to be grouped by hash code.
    llvm::errs() << ".gnu.hash is incompatible with the MIPS ABI\n";
  }

  Module::SymbolTable& symbols = pModule.getSymbolTable();

  std::stable_sort(
      symbols.dynamicBegin(), symbols.dynamicEnd(), DynsymGOTCompare(*m_pGOT));
}

}  // namespace mcld

namespace llvm {
namespace ELF {
// SHT_MIPS_OPTIONS section's block descriptor.
struct Elf_Options {
  unsigned char kind;  // Determines interpretation of variable
                       // part of descriptor. See ODK_xxx enumeration.
  unsigned char size;  // Byte size of descriptor, including this header.
  Elf64_Half section;  // Section header index of section affected,
                       // or 0 for global options.
  Elf64_Word info;     // Kind-speciﬁc information.
};

// Content of ODK_REGINFO block in SHT_MIPS_OPTIONS section on 32 bit ABI.
struct Elf32_RegInfo {
  Elf32_Word ri_gprmask;     // Mask of general purpose registers used.
  Elf32_Word ri_cprmask[4];  // Mask of co-processor registers used.
  Elf32_Addr ri_gp_value;    // GP register value for this object file.
};

// Content of ODK_REGINFO block in SHT_MIPS_OPTIONS section on 64 bit ABI.
struct Elf64_RegInfo {
  Elf32_Word ri_gprmask;     // Mask of general purpose registers used.
  Elf32_Word ri_pad;         // Padding.
  Elf32_Word ri_cprmask[4];  // Mask of co-processor registers used.
  Elf64_Addr ri_gp_value;    // GP register value for this object file.
};

}  // namespace ELF
}  // namespace llvm

namespace mcld {

static const char* ArchName(uint64_t flagBits) {
  switch (flagBits) {
    case llvm::ELF::EF_MIPS_ARCH_1:
      return "mips1";
    case llvm::ELF::EF_MIPS_ARCH_2:
      return "mips2";
    case llvm::ELF::EF_MIPS_ARCH_3:
      return "mips3";
    case llvm::ELF::EF_MIPS_ARCH_4:
      return "mips4";
    case llvm::ELF::EF_MIPS_ARCH_5:
      return "mips5";
    case llvm::ELF::EF_MIPS_ARCH_32:
      return "mips32";
    case llvm::ELF::EF_MIPS_ARCH_64:
      return "mips64";
    case llvm::ELF::EF_MIPS_ARCH_32R2:
      return "mips32r2";
    case llvm::ELF::EF_MIPS_ARCH_64R2:
      return "mips64r2";
    case llvm::ELF::EF_MIPS_ARCH_32R6:
      return "mips32r6";
    case llvm::ELF::EF_MIPS_ARCH_64R6:
      return "mips64r6";
    default:
      return "Unknown Arch";
  }
}

void MipsGNULDBackend::mergeFlags(Input& pInput, const char* ELF_hdr) {
  bool isTarget64Bit = config().targets().triple().isArch64Bit();
  bool isInput64Bit = ELF_hdr[llvm::ELF::EI_CLASS] == llvm::ELF::ELFCLASS64;

  if (isTarget64Bit != isInput64Bit) {
    fatal(diag::error_Mips_incompatible_class)
        << (isTarget64Bit ? "ELFCLASS64" : "ELFCLASS32")
        << (isInput64Bit ? "ELFCLASS64" : "ELFCLASS32") << pInput.name();
    return;
  }

  m_ElfFlagsMap[&pInput] =
      isInput64Bit ?
          reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(ELF_hdr)->e_flags :
          reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(ELF_hdr)->e_flags;
}

bool MipsGNULDBackend::readSection(Input& pInput, SectionData& pSD) {
  if ((pSD.getSection().flag() & llvm::ELF::SHF_MIPS_GPREL) ||
      (pSD.getSection().type() == llvm::ELF::SHT_MIPS_ABIFLAGS)) {
    uint64_t offset = pInput.fileOffset() + pSD.getSection().offset();
    uint64_t size = pSD.getSection().size();

    Fragment* frag = IRBuilder::CreateRegion(pInput, offset, size);
    ObjectBuilder::AppendFragment(*frag, pSD);
    return true;
  }

  if (pSD.getSection().type() == llvm::ELF::SHT_MIPS_OPTIONS) {
    uint32_t offset = pInput.fileOffset() + pSD.getSection().offset();
    uint32_t size = pSD.getSection().size();

    llvm::StringRef region = pInput.memArea()->request(offset, size);
    if (region.size() > 0) {
      const llvm::ELF::Elf_Options* optb =
          reinterpret_cast<const llvm::ELF::Elf_Options*>(region.begin());
      const llvm::ELF::Elf_Options* opte =
          reinterpret_cast<const llvm::ELF::Elf_Options*>(region.begin() +
                                                          size);

      for (const llvm::ELF::Elf_Options* opt = optb; opt < opte;
           opt += opt->size) {
        switch (opt->kind) {
          default:
            // Nothing to do.
            break;
          case llvm::ELF::ODK_REGINFO:
            if (config().targets().triple().isArch32Bit()) {
              const llvm::ELF::Elf32_RegInfo* reg =
                  reinterpret_cast<const llvm::ELF::Elf32_RegInfo*>(opt + 1);
              m_GP0Map[&pInput] = reg->ri_gp_value;
            } else {
              const llvm::ELF::Elf64_RegInfo* reg =
                  reinterpret_cast<const llvm::ELF::Elf64_RegInfo*>(opt + 1);
              m_GP0Map[&pInput] = reg->ri_gp_value;
            }
            break;
        }
      }
    }

    return true;
  }

  return GNULDBackend::readSection(pInput, pSD);
}

MipsGOT& MipsGNULDBackend::getGOT() {
  assert(m_pGOT != NULL);
  return *m_pGOT;
}

const MipsGOT& MipsGNULDBackend::getGOT() const {
  assert(m_pGOT != NULL);
  return *m_pGOT;
}

MipsPLT& MipsGNULDBackend::getPLT() {
  assert(m_pPLT != NULL);
  return *m_pPLT;
}

const MipsPLT& MipsGNULDBackend::getPLT() const {
  assert(m_pPLT != NULL);
  return *m_pPLT;
}

MipsGOTPLT& MipsGNULDBackend::getGOTPLT() {
  assert(m_pGOTPLT != NULL);
  return *m_pGOTPLT;
}

const MipsGOTPLT& MipsGNULDBackend::getGOTPLT() const {
  assert(m_pGOTPLT != NULL);
  return *m_pGOTPLT;
}

OutputRelocSection& MipsGNULDBackend::getRelPLT() {
  assert(m_pRelPlt != NULL);
  return *m_pRelPlt;
}

const OutputRelocSection& MipsGNULDBackend::getRelPLT() const {
  assert(m_pRelPlt != NULL);
  return *m_pRelPlt;
}

OutputRelocSection& MipsGNULDBackend::getRelDyn() {
  assert(m_pRelDyn != NULL);
  return *m_pRelDyn;
}

const OutputRelocSection& MipsGNULDBackend::getRelDyn() const {
  assert(m_pRelDyn != NULL);
  return *m_pRelDyn;
}

unsigned int MipsGNULDBackend::getTargetSectionOrder(
    const LDSection& pSectHdr) const {
  const ELFFileFormat* file_format = getOutputFormat();

  if (file_format->hasGOT() && (&pSectHdr == &file_format->getGOT()))
    return SHO_DATA;

  if (file_format->hasGOTPLT() && (&pSectHdr == &file_format->getGOTPLT()))
    return SHO_DATA;

  if (file_format->hasPLT() && (&pSectHdr == &file_format->getPLT()))
    return SHO_PLT;

  if (&pSectHdr == m_psdata)
    return SHO_SMALL_DATA;

  if (&pSectHdr == m_pAbiFlags)
    return SHO_RO_NOTE;

  return SHO_UNDEFINED;
}

/// finalizeSymbol - finalize the symbol value
bool MipsGNULDBackend::finalizeTargetSymbols() {
  if (m_pGpDispSymbol != NULL)
    m_pGpDispSymbol->setValue(m_pGOT->getGPDispAddress());

  return true;
}

/// allocateCommonSymbols - allocate common symbols in the corresponding
/// sections. This is called at pre-layout stage.
/// FIXME: Mips needs to allocate small common symbol
bool MipsGNULDBackend::allocateCommonSymbols(Module& pModule) {
  SymbolCategory& symbol_list = pModule.getSymbolTable();

  if (symbol_list.emptyCommons() && symbol_list.emptyFiles() &&
      symbol_list.emptyLocals() && symbol_list.emptyLocalDyns())
    return true;

  SymbolCategory::iterator com_sym, com_end;

  // FIXME: If the order of common symbols is defined, then sort common symbols
  // std::sort(com_sym, com_end, some kind of order);

  // get corresponding BSS LDSection
  ELFFileFormat* file_format = getOutputFormat();
  LDSection& bss_sect = file_format->getBSS();
  LDSection& tbss_sect = file_format->getTBSS();

  // get or create corresponding BSS SectionData
  SectionData* bss_sect_data = NULL;
  if (bss_sect.hasSectionData())
    bss_sect_data = bss_sect.getSectionData();
  else
    bss_sect_data = IRBuilder::CreateSectionData(bss_sect);

  SectionData* tbss_sect_data = NULL;
  if (tbss_sect.hasSectionData())
    tbss_sect_data = tbss_sect.getSectionData();
  else
    tbss_sect_data = IRBuilder::CreateSectionData(tbss_sect);

  // remember original BSS size
  uint64_t bss_offset = bss_sect.size();
  uint64_t tbss_offset = tbss_sect.size();

  // allocate all local common symbols
  com_end = symbol_list.localEnd();

  for (com_sym = symbol_list.localBegin(); com_sym != com_end; ++com_sym) {
    if (ResolveInfo::Common == (*com_sym)->desc()) {
      // We have to reset the description of the symbol here. When doing
      // incremental linking, the output relocatable object may have common
      // symbols. Therefore, we can not treat common symbols as normal symbols
      // when emitting the regular name pools. We must change the symbols'
      // description here.
      (*com_sym)->resolveInfo()->setDesc(ResolveInfo::Define);
      Fragment* frag = new FillFragment(0x0, 1, (*com_sym)->size());

      if (ResolveInfo::ThreadLocal == (*com_sym)->type()) {
        // allocate TLS common symbol in tbss section
        tbss_offset += ObjectBuilder::AppendFragment(
            *frag, *tbss_sect_data, (*com_sym)->value());
        ObjectBuilder::UpdateSectionAlign(tbss_sect, (*com_sym)->value());
        (*com_sym)->setFragmentRef(FragmentRef::Create(*frag, 0));
      } else {
        // FIXME: how to identify small and large common symbols?
        bss_offset += ObjectBuilder::AppendFragment(
            *frag, *bss_sect_data, (*com_sym)->value());
        ObjectBuilder::UpdateSectionAlign(bss_sect, (*com_sym)->value());
        (*com_sym)->setFragmentRef(FragmentRef::Create(*frag, 0));
      }
    }
  }

  // allocate all global common symbols
  com_end = symbol_list.commonEnd();
  for (com_sym = symbol_list.commonBegin(); com_sym != com_end; ++com_sym) {
    // We have to reset the description of the symbol here. When doing
    // incremental linking, the output relocatable object may have common
    // symbols. Therefore, we can not treat common symbols as normal symbols
    // when emitting the regular name pools. We must change the symbols'
    // description here.
    (*com_sym)->resolveInfo()->setDesc(ResolveInfo::Define);
    Fragment* frag = new FillFragment(0x0, 1, (*com_sym)->size());

    if (ResolveInfo::ThreadLocal == (*com_sym)->type()) {
      // allocate TLS common symbol in tbss section
      tbss_offset += ObjectBuilder::AppendFragment(
          *frag, *tbss_sect_data, (*com_sym)->value());
      ObjectBuilder::UpdateSectionAlign(tbss_sect, (*com_sym)->value());
      (*com_sym)->setFragmentRef(FragmentRef::Create(*frag, 0));
    } else {
      // FIXME: how to identify small and large common symbols?
      bss_offset += ObjectBuilder::AppendFragment(
          *frag, *bss_sect_data, (*com_sym)->value());
      ObjectBuilder::UpdateSectionAlign(bss_sect, (*com_sym)->value());
      (*com_sym)->setFragmentRef(FragmentRef::Create(*frag, 0));
    }
  }

  bss_sect.setSize(bss_offset);
  tbss_sect.setSize(tbss_offset);
  symbol_list.changeCommonsToGlobal();
  return true;
}

uint64_t MipsGNULDBackend::getTPOffset(const Input& pInput) const {
  return m_TpOffsetMap.lookup(&pInput);
}

uint64_t MipsGNULDBackend::getDTPOffset(const Input& pInput) const {
  return m_DtpOffsetMap.lookup(&pInput);
}

uint64_t MipsGNULDBackend::getGP0(const Input& pInput) const {
  return m_GP0Map.lookup(&pInput);
}

void MipsGNULDBackend::defineGOTSymbol(IRBuilder& pBuilder) {
  // If we do not reserve any GOT entries, we do not need to re-define GOT
  // symbol.
  if (!m_pGOT->hasGOT1())
    return;

  // define symbol _GLOBAL_OFFSET_TABLE_
  if (m_pGOTSymbol != NULL) {
    pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOT->begin()), 0x0),
        ResolveInfo::Hidden);
  } else {
    m_pGOTSymbol = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOT->begin()), 0x0),
        ResolveInfo::Hidden);
  }
}

void MipsGNULDBackend::defineGOTPLTSymbol(IRBuilder& pBuilder) {
  // define symbol _PROCEDURE_LINKAGE_TABLE_
  if (m_pPLTSymbol != NULL) {
    pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
        "_PROCEDURE_LINKAGE_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pPLT->begin()), 0x0),
        ResolveInfo::Hidden);
  } else {
    m_pPLTSymbol = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        "_PROCEDURE_LINKAGE_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pPLT->begin()), 0x0),
        ResolveInfo::Hidden);
  }
}

/// doCreateProgramHdrs - backend can implement this function to create the
/// target-dependent segments
void MipsGNULDBackend::doCreateProgramHdrs(Module& pModule) {
  if (!m_pAbiFlags || m_pAbiFlags->size() == 0)
    return;

  // create PT_MIPS_ABIFLAGS segment
  ELFSegmentFactory::iterator sit =
      elfSegmentTable().find(llvm::ELF::PT_INTERP, 0x0, 0x0);
  if (sit == elfSegmentTable().end())
    sit = elfSegmentTable().find(llvm::ELF::PT_PHDR, 0x0, 0x0);
  if (sit == elfSegmentTable().end())
    sit = elfSegmentTable().begin();
  else
    ++sit;

  ELFSegment* abiSeg = elfSegmentTable().insert(sit,
                                                llvm::ELF::PT_MIPS_ABIFLAGS,
                                                llvm::ELF::PF_R);
  abiSeg->setAlign(8);
  abiSeg->append(m_pAbiFlags);
}

bool MipsGNULDBackend::relaxRelocation(IRBuilder& pBuilder, Relocation& pRel) {
  uint64_t sym_value = 0x0;

  LDSymbol* symbol = pRel.symInfo()->outSymbol();
  if (symbol->hasFragRef()) {
    uint64_t value = symbol->fragRef()->getOutputOffset();
    uint64_t addr = symbol->fragRef()->frag()->getParent()->getSection().addr();
    sym_value = addr + value;
  }

  Stub* stub = getStubFactory()->create(
      pRel, sym_value, pBuilder, *getBRIslandFactory());

  if (stub == NULL)
    return false;

  assert(stub->symInfo() != NULL);
  // reset the branch target of the reloc to this stub instead
  pRel.setSymInfo(stub->symInfo());

  // increase the size of .symtab and .strtab
  LDSection& symtab = getOutputFormat()->getSymTab();
  LDSection& strtab = getOutputFormat()->getStrTab();
  symtab.setSize(symtab.size() + sizeof(llvm::ELF::Elf32_Sym));
  strtab.setSize(strtab.size() + stub->symInfo()->nameSize() + 1);

  return true;
}

bool MipsGNULDBackend::doRelax(Module& pModule,
                               IRBuilder& pBuilder,
                               bool& pFinished) {
  assert(getStubFactory() != NULL && getBRIslandFactory() != NULL);

  bool isRelaxed = false;

  for (Module::obj_iterator input = pModule.obj_begin();
       input != pModule.obj_end();
       ++input) {
    LDContext* context = (*input)->context();

    for (LDContext::sect_iterator rs = context->relocSectBegin();
         rs != context->relocSectEnd();
         ++rs) {
      LDSection* sec = *rs;

      if (LDFileFormat::Ignore == sec->kind() || !sec->hasRelocData())
        continue;

      for (RelocData::iterator reloc = sec->getRelocData()->begin();
           reloc != sec->getRelocData()->end();
           ++reloc) {
        if (llvm::ELF::R_MIPS_26 != reloc->type())
          continue;

        if (relaxRelocation(pBuilder, *llvm::cast<Relocation>(reloc)))
          isRelaxed = true;
      }
    }
  }

  // find the first fragment w/ invalid offset due to stub insertion
  std::vector<Fragment*> invalid_frags;
  pFinished = true;
  for (BranchIslandFactory::iterator ii = getBRIslandFactory()->begin(),
                                     ie = getBRIslandFactory()->end();
       ii != ie;
       ++ii) {
    BranchIsland& island = *ii;
    if (island.size() > stubGroupSize()) {
      error(diag::err_no_space_to_place_stubs) << stubGroupSize();
      return false;
    }

    if (island.numOfStubs() == 0) {
      continue;
    }

    Fragment* exit = &*island.end();
    if (exit == &*island.begin()->getParent()->end()) {
      continue;
    }

    if ((island.offset() + island.size()) > exit->getOffset()) {
      if (invalid_frags.empty() ||
          (invalid_frags.back()->getParent() != island.getParent())) {
        invalid_frags.push_back(exit);
        pFinished = false;
      }
      continue;
    }
  }

  // reset the offset of invalid fragments
  for (auto it = invalid_frags.begin(), ie = invalid_frags.end(); it != ie;
       ++it) {
    Fragment* invalid = *it;
    while (invalid != NULL) {
      invalid->setOffset(invalid->getPrevNode()->getOffset() +
                         invalid->getPrevNode()->size());
      invalid = invalid->getNextNode();
    }
  }

  // reset the size of section that has stubs inserted.
  if (isRelaxed) {
    SectionData* prev = NULL;
    for (BranchIslandFactory::iterator island = getBRIslandFactory()->begin(),
                                       island_end = getBRIslandFactory()->end();
         island != island_end;
         ++island) {
      SectionData* sd = (*island).begin()->getParent();
      if ((*island).numOfStubs() != 0) {
        if (sd != prev) {
          sd->getSection().setSize(sd->back().getOffset() + sd->back().size());
        }
      }
      prev = sd;
    }
  }

  return isRelaxed;
}

bool MipsGNULDBackend::initTargetStubs() {
  if (getStubFactory() == NULL)
    return false;

  getStubFactory()->addPrototype(new MipsLA25Stub(*this));
  return true;
}

bool MipsGNULDBackend::readRelocation(const llvm::ELF::Elf32_Rel& pRel,
                                      Relocation::Type& pType,
                                      uint32_t& pSymIdx,
                                      uint32_t& pOffset) const {
  return GNULDBackend::readRelocation(pRel, pType, pSymIdx, pOffset);
}

bool MipsGNULDBackend::readRelocation(const llvm::ELF::Elf32_Rela& pRel,
                                      Relocation::Type& pType,
                                      uint32_t& pSymIdx,
                                      uint32_t& pOffset,
                                      int32_t& pAddend) const {
  return GNULDBackend::readRelocation(pRel, pType, pSymIdx, pOffset, pAddend);
}

bool MipsGNULDBackend::readRelocation(const llvm::ELF::Elf64_Rel& pRel,
                                      Relocation::Type& pType,
                                      uint32_t& pSymIdx,
                                      uint64_t& pOffset) const {
  uint64_t r_info = 0x0;
  if (llvm::sys::IsLittleEndianHost) {
    pOffset = pRel.r_offset;
    r_info = pRel.r_info;
  } else {
    pOffset = mcld::bswap64(pRel.r_offset);
    r_info = mcld::bswap64(pRel.r_info);
  }

  // MIPS 64 little endian (we do not support big endian now)
  // has a "special" encoding of r_info relocation
  // field. Instead of one 64 bit little endian number, it is a little
  // endian 32 bit number followed by a 32 bit big endian number.
  pType = mcld::bswap32(r_info >> 32);
  pSymIdx = r_info & 0xffffffff;
  return true;
}

bool MipsGNULDBackend::readRelocation(const llvm::ELF::Elf64_Rela& pRel,
                                      Relocation::Type& pType,
                                      uint32_t& pSymIdx,
                                      uint64_t& pOffset,
                                      int64_t& pAddend) const {
  uint64_t r_info = 0x0;
  if (llvm::sys::IsLittleEndianHost) {
    pOffset = pRel.r_offset;
    r_info = pRel.r_info;
    pAddend = pRel.r_addend;
  } else {
    pOffset = mcld::bswap64(pRel.r_offset);
    r_info = mcld::bswap64(pRel.r_info);
    pAddend = mcld::bswap64(pRel.r_addend);
  }

  pType = mcld::bswap32(r_info >> 32);
  pSymIdx = r_info & 0xffffffff;
  return true;
}

void MipsGNULDBackend::emitRelocation(llvm::ELF::Elf32_Rel& pRel,
                                      Relocation::Type pType,
                                      uint32_t pSymIdx,
                                      uint32_t pOffset) const {
  GNULDBackend::emitRelocation(pRel, pType, pSymIdx, pOffset);
}

void MipsGNULDBackend::emitRelocation(llvm::ELF::Elf32_Rela& pRel,
                                      Relocation::Type pType,
                                      uint32_t pSymIdx,
                                      uint32_t pOffset,
                                      int32_t pAddend) const {
  GNULDBackend::emitRelocation(pRel, pType, pSymIdx, pOffset, pAddend);
}

void MipsGNULDBackend::emitRelocation(llvm::ELF::Elf64_Rel& pRel,
                                      Relocation::Type pType,
                                      uint32_t pSymIdx,
                                      uint64_t pOffset) const {
  uint64_t r_info = mcld::bswap32(pType);
  r_info <<= 32;
  r_info |= pSymIdx;

  pRel.r_info = r_info;
  pRel.r_offset = pOffset;
}

void MipsGNULDBackend::emitRelocation(llvm::ELF::Elf64_Rela& pRel,
                                      Relocation::Type pType,
                                      uint32_t pSymIdx,
                                      uint64_t pOffset,
                                      int64_t pAddend) const {
  uint64_t r_info = mcld::bswap32(pType);
  r_info <<= 32;
  r_info |= pSymIdx;

  pRel.r_info = r_info;
  pRel.r_offset = pOffset;
  pRel.r_addend = pAddend;
}

namespace {
struct ISATreeEdge {
  unsigned child;
  unsigned parent;
};
}

static ISATreeEdge isaTree[] = {
    // MIPS32R6 and MIPS64R6 are not compatible with other extensions

    // MIPS64 extensions.
    {llvm::ELF::EF_MIPS_ARCH_64R2, llvm::ELF::EF_MIPS_ARCH_64},
    // MIPS V extensions.
    {llvm::ELF::EF_MIPS_ARCH_64, llvm::ELF::EF_MIPS_ARCH_5},
    // MIPS IV extensions.
    {llvm::ELF::EF_MIPS_ARCH_5, llvm::ELF::EF_MIPS_ARCH_4},
    // MIPS III extensions.
    {llvm::ELF::EF_MIPS_ARCH_4, llvm::ELF::EF_MIPS_ARCH_3},
    // MIPS32 extensions.
    {llvm::ELF::EF_MIPS_ARCH_32R2, llvm::ELF::EF_MIPS_ARCH_32},
    // MIPS II extensions.
    {llvm::ELF::EF_MIPS_ARCH_3, llvm::ELF::EF_MIPS_ARCH_2},
    {llvm::ELF::EF_MIPS_ARCH_32, llvm::ELF::EF_MIPS_ARCH_2},
    // MIPS I extensions.
    {llvm::ELF::EF_MIPS_ARCH_2, llvm::ELF::EF_MIPS_ARCH_1},
};

static bool isIsaMatched(uint32_t base, uint32_t ext) {
  if (base == ext)
    return true;
  if (base == llvm::ELF::EF_MIPS_ARCH_32 &&
      isIsaMatched(llvm::ELF::EF_MIPS_ARCH_64, ext))
    return true;
  if (base == llvm::ELF::EF_MIPS_ARCH_32R2 &&
      isIsaMatched(llvm::ELF::EF_MIPS_ARCH_64R2, ext))
    return true;
  for (const auto &edge : isaTree) {
    if (ext == edge.child) {
      ext = edge.parent;
      if (ext == base)
        return true;
    }
  }
  return false;
}

static bool getAbiFlags(const Input& pInput, uint64_t elfFlags, bool& hasFlags,
                        MipsAbiFlags& pFlags) {
  MipsAbiFlags pElfFlags = {};
  if (!MipsAbiFlags::fillByElfFlags(pInput, elfFlags, pElfFlags))
    return false;

  const LDContext* ctx = pInput.context();
  for (auto it = ctx->sectBegin(), ie = ctx->sectEnd(); it != ie; ++it)
    if ((*it)->type() == llvm::ELF::SHT_MIPS_ABIFLAGS) {
      if (!MipsAbiFlags::fillBySection(pInput, **it, pFlags))
        return false;
      if (!MipsAbiFlags::isCompatible(pInput, pElfFlags, pFlags))
        return false;
      hasFlags = true;
      return true;
    }

  pFlags = pElfFlags;
  return true;
}

static const char* getNanName(uint64_t flags) {
  return flags & llvm::ELF::EF_MIPS_NAN2008 ? "2008" : "legacy";
}

static bool mergeElfFlags(const Input& pInput, uint64_t& oldElfFlags,
                          uint64_t newElfFlags) {
  // PIC code is inherently CPIC and may not set CPIC flag explicitly.
  // Ensure that this flag will exist in the linked file.
  if (newElfFlags & llvm::ELF::EF_MIPS_PIC)
    newElfFlags |= llvm::ELF::EF_MIPS_CPIC;

  if (newElfFlags & llvm::ELF::EF_MIPS_ARCH_ASE_M16) {
    error(diag::error_Mips_m16_unsupported) << pInput.name();
    return false;
  }

  if (!oldElfFlags) {
    oldElfFlags = newElfFlags;
    return true;
  }

  uint64_t newPic =
      newElfFlags & (llvm::ELF::EF_MIPS_PIC | llvm::ELF::EF_MIPS_CPIC);
  uint64_t oldPic =
      oldElfFlags & (llvm::ELF::EF_MIPS_PIC | llvm::ELF::EF_MIPS_CPIC);

  // Check PIC / CPIC flags compatibility.
  if ((newPic != 0) != (oldPic != 0))
    warning(diag::warn_Mips_abicalls_linking) << pInput.name();

  if (!(newPic & llvm::ELF::EF_MIPS_PIC))
    oldElfFlags &= ~llvm::ELF::EF_MIPS_PIC;
  if (newPic)
    oldElfFlags |= llvm::ELF::EF_MIPS_CPIC;

  // Check ISA compatibility.
  uint64_t newArch = newElfFlags & llvm::ELF::EF_MIPS_ARCH;
  uint64_t oldArch = oldElfFlags & llvm::ELF::EF_MIPS_ARCH;
  if (!isIsaMatched(newArch, oldArch)) {
    if (!isIsaMatched(oldArch, newArch)) {
      error(diag::error_Mips_inconsistent_arch)
          << ArchName(oldArch) << ArchName(newArch) << pInput.name();
      return false;
    }
    oldElfFlags &= ~llvm::ELF::EF_MIPS_ARCH;
    oldElfFlags |= newArch;
  }

  // Check ABI compatibility.
  uint32_t newAbi = newElfFlags & llvm::ELF::EF_MIPS_ABI;
  uint32_t oldAbi = oldElfFlags & llvm::ELF::EF_MIPS_ABI;
  if (newAbi != oldAbi && newAbi && oldAbi) {
    error(diag::error_Mips_inconsistent_abi) << pInput.name();
    return false;
  }

  // Check -mnan flags compatibility.
  if ((newElfFlags & llvm::ELF::EF_MIPS_NAN2008) !=
      (oldElfFlags & llvm::ELF::EF_MIPS_NAN2008)) {
    // Linking -mnan=2008 and -mnan=legacy modules
    error(diag::error_Mips_inconsistent_mnan)
        << getNanName(oldElfFlags) << getNanName(newElfFlags) << pInput.name();
    return false;
  }

  // Check ASE compatibility.
  uint64_t newAse = newElfFlags & llvm::ELF::EF_MIPS_ARCH_ASE;
  uint64_t oldAse = oldElfFlags & llvm::ELF::EF_MIPS_ARCH_ASE;
  if (newAse != oldAse)
    oldElfFlags |= newAse;

  // Check FP64 compatibility.
  if ((newElfFlags & llvm::ELF::EF_MIPS_FP64) !=
      (oldElfFlags & llvm::ELF::EF_MIPS_FP64)) {
    // Linking -mnan=2008 and -mnan=legacy modules
    error(diag::error_Mips_inconsistent_fp64) << pInput.name();
    return false;
  }

  oldElfFlags |= newElfFlags & llvm::ELF::EF_MIPS_NOREORDER;
  oldElfFlags |= newElfFlags & llvm::ELF::EF_MIPS_MICROMIPS;
  oldElfFlags |= newElfFlags & llvm::ELF::EF_MIPS_NAN2008;
  oldElfFlags |= newElfFlags & llvm::ELF::EF_MIPS_32BITMODE;

  return true;
}

void MipsGNULDBackend::saveTPOffset(const Input& pInput) {
  const LDContext* ctx = pInput.context();
  for (auto it = ctx->sectBegin(), ie = ctx->sectEnd(); it != ie; ++it) {
    LDSection* sect = *it;
    if (sect->flag() & llvm::ELF::SHF_TLS) {
      m_TpOffsetMap[&pInput] = sect->addr() + 0x7000;
      m_DtpOffsetMap[&pInput] = sect->addr() + 0x8000;
      break;
    }
  }
}

void MipsGNULDBackend::preMergeSections(Module& pModule) {
  uint64_t elfFlags = 0;
  bool hasAbiFlags = false;
  MipsAbiFlags abiFlags = {};
  for (const Input *input : pModule.getObjectList()) {
    if (input->type() != Input::Object)
      continue;

    uint64_t newElfFlags = m_ElfFlagsMap[input];

    MipsAbiFlags newAbiFlags = {};
    if (!getAbiFlags(*input, newElfFlags, hasAbiFlags, newAbiFlags))
      continue;

    if (!mergeElfFlags(*input, elfFlags, newElfFlags))
      continue;

    if (!MipsAbiFlags::merge(*input, abiFlags, newAbiFlags))
      continue;

    saveTPOffset(*input);
  }

  m_pInfo.setElfFlags(elfFlags);
  if (hasAbiFlags)
    m_pAbiInfo = abiFlags;
}

bool MipsGNULDBackend::mergeSection(Module& pModule, const Input& pInput,
                                    LDSection& pSection) {
  if (pSection.flag() & llvm::ELF::SHF_MIPS_GPREL) {
    SectionData* sd = NULL;
    if (!m_psdata->hasSectionData()) {
      sd = IRBuilder::CreateSectionData(*m_psdata);
      m_psdata->setSectionData(sd);
    }
    sd = m_psdata->getSectionData();
    moveSectionData(*pSection.getSectionData(), *sd);
  } else if (pSection.type() == llvm::ELF::SHT_MIPS_ABIFLAGS) {
    // Nothing to do because we handle all .MIPS.abiflags sections
    // in the preMergeSections method.
  } else {
    ObjectBuilder builder(pModule);
    builder.MergeSection(pInput, pSection);
  }
  return true;
}

void MipsGNULDBackend::moveSectionData(SectionData& pFrom, SectionData& pTo) {
  assert(&pFrom != &pTo && "Cannot move section data to itself!");

  uint64_t offset = pTo.getSection().size();
  AlignFragment* align = NULL;
  if (pFrom.getSection().align() > 1) {
    // if the align constraint is larger than 1, append an alignment
    unsigned int alignment = pFrom.getSection().align();
    align = new AlignFragment(/*alignment*/ alignment,
                              /*the filled value*/ 0x0,
                              /*the size of filled value*/ 1u,
                              /*max bytes to emit*/ alignment - 1);
    align->setOffset(offset);
    align->setParent(&pTo);
    pTo.getFragmentList().push_back(align);
    offset += align->size();
  }

  // move fragments from pFrom to pTO
  SectionData::FragmentListType& from_list = pFrom.getFragmentList();
  SectionData::FragmentListType& to_list = pTo.getFragmentList();
  SectionData::FragmentListType::iterator frag, fragEnd = from_list.end();
  for (frag = from_list.begin(); frag != fragEnd; ++frag) {
    frag->setParent(&pTo);
    frag->setOffset(offset);
    offset += frag->size();
  }
  to_list.splice(to_list.end(), from_list);

  // set up pTo's header
  pTo.getSection().setSize(offset);
}

//===----------------------------------------------------------------------===//
// Mips32GNULDBackend
//===----------------------------------------------------------------------===//
Mips32GNULDBackend::Mips32GNULDBackend(const LinkerConfig& pConfig,
                                       MipsGNUInfo* pInfo)
    : MipsGNULDBackend(pConfig, pInfo) {
}

bool Mips32GNULDBackend::initRelocator() {
  if (m_pRelocator == NULL)
    m_pRelocator = new Mips32Relocator(*this, config());

  return true;
}

void Mips32GNULDBackend::initTargetSections(Module& pModule,
                                            ObjectBuilder& pBuilder) {
  MipsGNULDBackend::initTargetSections(pModule, pBuilder);

  if (LinkerConfig::Object == config().codeGenType())
    return;

  ELFFileFormat* fileFormat = getOutputFormat();

  // initialize .got
  LDSection& got = fileFormat->getGOT();
  m_pGOT = new Mips32GOT(got);

  // initialize .got.plt
  LDSection& gotplt = fileFormat->getGOTPLT();
  m_pGOTPLT = new MipsGOTPLT(gotplt);

  // initialize .plt
  LDSection& plt = fileFormat->getPLT();
  m_pPLT = new MipsPLT(plt);
}

size_t Mips32GNULDBackend::getRelEntrySize() {
  return 8;
}

size_t Mips32GNULDBackend::getRelaEntrySize() {
  return 12;
}

//===----------------------------------------------------------------------===//
// Mips64GNULDBackend
//===----------------------------------------------------------------------===//
Mips64GNULDBackend::Mips64GNULDBackend(const LinkerConfig& pConfig,
                                       MipsGNUInfo* pInfo)
    : MipsGNULDBackend(pConfig, pInfo) {
}

bool Mips64GNULDBackend::initRelocator() {
  if (m_pRelocator == NULL)
    m_pRelocator = new Mips64Relocator(*this, config());

  return true;
}

void Mips64GNULDBackend::initTargetSections(Module& pModule,
                                            ObjectBuilder& pBuilder) {
  MipsGNULDBackend::initTargetSections(pModule, pBuilder);

  if (LinkerConfig::Object == config().codeGenType())
    return;

  ELFFileFormat* fileFormat = getOutputFormat();

  // initialize .got
  LDSection& got = fileFormat->getGOT();
  m_pGOT = new Mips64GOT(got);

  // initialize .got.plt
  LDSection& gotplt = fileFormat->getGOTPLT();
  m_pGOTPLT = new MipsGOTPLT(gotplt);

  // initialize .plt
  LDSection& plt = fileFormat->getPLT();
  m_pPLT = new MipsPLT(plt);
}

size_t Mips64GNULDBackend::getRelEntrySize() {
  return 16;
}

size_t Mips64GNULDBackend::getRelaEntrySize() {
  return 24;
}

//===----------------------------------------------------------------------===//
/// createMipsLDBackend - the help funtion to create corresponding MipsLDBackend
///
static TargetLDBackend* createMipsLDBackend(const LinkerConfig& pConfig) {
  const llvm::Triple& triple = pConfig.targets().triple();

  if (triple.isOSDarwin()) {
    assert(0 && "MachO linker is not supported yet");
  }
  if (triple.isOSWindows()) {
    assert(0 && "COFF linker is not supported yet");
  }

  llvm::Triple::ArchType arch = triple.getArch();

  if (llvm::Triple::mips64el == arch)
    return new Mips64GNULDBackend(pConfig, new MipsGNUInfo(triple));

  assert(arch == llvm::Triple::mipsel);
  return new Mips32GNULDBackend(pConfig, new MipsGNUInfo(triple));
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// Force static initialization.
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeMipsLDBackend() {
  mcld::TargetRegistry::RegisterTargetLDBackend(mcld::TheMipselTarget,
                                                mcld::createMipsLDBackend);
  mcld::TargetRegistry::RegisterTargetLDBackend(mcld::TheMips64elTarget,
                                                mcld::createMipsLDBackend);
}
