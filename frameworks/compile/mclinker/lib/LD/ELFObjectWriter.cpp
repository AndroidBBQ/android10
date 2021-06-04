//===- ELFObjectWriter.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFObjectWriter.h"

#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/ADT/SizeTraits.h"
#include "mcld/Fragment/AlignFragment.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/Fragment/NullFragment.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/DebugString.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/ELFSegment.h"
#include "mcld/LD/ELFSegmentFactory.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/GNUInfo.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Errc.h>
#include <llvm/Support/ErrorHandling.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFObjectWriter
//===----------------------------------------------------------------------===//
ELFObjectWriter::ELFObjectWriter(GNULDBackend& pBackend,
                                 const LinkerConfig& pConfig)
    : ObjectWriter(), m_Backend(pBackend), m_Config(pConfig) {
}

ELFObjectWriter::~ELFObjectWriter() {
}

void ELFObjectWriter::writeSection(Module& pModule,
                                   FileOutputBuffer& pOutput,
                                   LDSection* section) {
  MemoryRegion region;
  // Request output region
  switch (section->kind()) {
    case LDFileFormat::Note:
      if (section->getSectionData() == NULL)
        return;
    // Fall through
    case LDFileFormat::TEXT:
    case LDFileFormat::DATA:
    case LDFileFormat::Relocation:
    case LDFileFormat::Target:
    case LDFileFormat::Debug:
    case LDFileFormat::DebugString:
    case LDFileFormat::GCCExceptTable:
    case LDFileFormat::EhFrame: {
      region = pOutput.request(section->offset(), section->size());
      if (region.size() == 0) {
        return;
      }
      break;
    }
    case LDFileFormat::Null:
    case LDFileFormat::NamePool:
    case LDFileFormat::BSS:
    case LDFileFormat::MetaData:
    case LDFileFormat::Version:
    case LDFileFormat::EhFrameHdr:
    case LDFileFormat::StackNote:
      // Ignore these sections
      return;
    default:
      llvm::errs() << "WARNING: unsupported section kind: " << section->kind()
                   << " of section " << section->name() << ".\n";
      return;
  }

  // Write out sections with data
  switch (section->kind()) {
    case LDFileFormat::GCCExceptTable:
    case LDFileFormat::TEXT:
    case LDFileFormat::DATA:
    case LDFileFormat::Debug:
    case LDFileFormat::Note:
      emitSectionData(*section, region);
      break;
    case LDFileFormat::EhFrame:
      emitEhFrame(pModule, *section->getEhFrame(), region);
      break;
    case LDFileFormat::Relocation:
      // sort relocation for the benefit of the dynamic linker.
      target().sortRelocation(*section);

      emitRelocation(m_Config, *section, region);
      break;
    case LDFileFormat::Target:
      target().emitSectionData(*section, region);
      break;
    case LDFileFormat::DebugString:
      section->getDebugString()->emit(region);
      break;
    default:
      llvm_unreachable("invalid section kind");
  }
}

std::error_code ELFObjectWriter::writeObject(Module& pModule,
                                             FileOutputBuffer& pOutput) {
  bool is_dynobj = m_Config.codeGenType() == LinkerConfig::DynObj;
  bool is_exec = m_Config.codeGenType() == LinkerConfig::Exec;
  bool is_binary = m_Config.codeGenType() == LinkerConfig::Binary;
  bool is_object = m_Config.codeGenType() == LinkerConfig::Object;

  assert(is_dynobj || is_exec || is_binary || is_object);

  if (is_dynobj || is_exec) {
    // Allow backend to sort symbols before emitting
    target().orderSymbolTable(pModule);

    // Write out the interpreter section: .interp
    target().emitInterp(pOutput);

    // Write out name pool sections: .dynsym, .dynstr, .hash
    target().emitDynNamePools(pModule, pOutput);
  }

  if (is_object || is_dynobj || is_exec) {
    // Write out name pool sections: .symtab, .strtab
    target().emitRegNamePools(pModule, pOutput);
  }

  if (is_binary) {
    // Iterate over the loadable segments and write the corresponding sections
    ELFSegmentFactory::iterator seg, segEnd = target().elfSegmentTable().end();

    for (seg = target().elfSegmentTable().begin(); seg != segEnd; ++seg) {
      if (llvm::ELF::PT_LOAD == (*seg)->type()) {
        ELFSegment::iterator sect, sectEnd = (*seg)->end();
        for (sect = (*seg)->begin(); sect != sectEnd; ++sect)
          writeSection(pModule, pOutput, *sect);
      }
    }
  } else {
    // Write out regular ELF sections
    Module::iterator sect, sectEnd = pModule.end();
    for (sect = pModule.begin(); sect != sectEnd; ++sect)
      writeSection(pModule, pOutput, *sect);

    emitShStrTab(target().getOutputFormat()->getShStrTab(), pModule, pOutput);

    if (m_Config.targets().is32Bits()) {
      // Write out ELF header
      // Write out section header table
      writeELFHeader<32>(m_Config, pModule, pOutput);
      if (is_dynobj || is_exec)
        emitProgramHeader<32>(pOutput);

      emitSectionHeader<32>(pModule, m_Config, pOutput);
    } else if (m_Config.targets().is64Bits()) {
      // Write out ELF header
      // Write out section header table
      writeELFHeader<64>(m_Config, pModule, pOutput);
      if (is_dynobj || is_exec)
        emitProgramHeader<64>(pOutput);

      emitSectionHeader<64>(pModule, m_Config, pOutput);
    } else {
      return llvm::make_error_code(llvm::errc::function_not_supported);
    }
  }

  return std::error_code();
}

// getOutputSize - count the final output size
size_t ELFObjectWriter::getOutputSize(const Module& pModule) const {
  if (m_Config.targets().is32Bits()) {
    return getLastStartOffset<32>(pModule) +
           sizeof(ELFSizeTraits<32>::Shdr) * pModule.size();
  } else if (m_Config.targets().is64Bits()) {
    return getLastStartOffset<64>(pModule) +
           sizeof(ELFSizeTraits<64>::Shdr) * pModule.size();
  } else {
    assert(0 && "Invalid ELF Class");
    return 0;
  }
}

// writeELFHeader - emit ElfXX_Ehdr
template <size_t SIZE>
void ELFObjectWriter::writeELFHeader(const LinkerConfig& pConfig,
                                     const Module& pModule,
                                     FileOutputBuffer& pOutput) const {
  typedef typename ELFSizeTraits<SIZE>::Ehdr ElfXX_Ehdr;
  typedef typename ELFSizeTraits<SIZE>::Shdr ElfXX_Shdr;
  typedef typename ELFSizeTraits<SIZE>::Phdr ElfXX_Phdr;

  // ELF header must start from 0x0
  MemoryRegion region = pOutput.request(0, sizeof(ElfXX_Ehdr));
  ElfXX_Ehdr* header = reinterpret_cast<ElfXX_Ehdr*>(region.begin());

  memcpy(header->e_ident, llvm::ELF::ElfMagic, llvm::ELF::EI_MAG3 + 1);

  header->e_ident[llvm::ELF::EI_CLASS] =
      (SIZE == 32) ? llvm::ELF::ELFCLASS32 : llvm::ELF::ELFCLASS64;
  header->e_ident[llvm::ELF::EI_DATA] =
      pConfig.targets().isLittleEndian()
          ? llvm::ELF::ELFDATA2LSB : llvm::ELF::ELFDATA2MSB;
  header->e_ident[llvm::ELF::EI_VERSION] = target().getInfo().ELFVersion();
  header->e_ident[llvm::ELF::EI_OSABI] = target().getInfo().OSABI();
  header->e_ident[llvm::ELF::EI_ABIVERSION] = target().getInfo().ABIVersion();

  // FIXME: add processor-specific and core file types.
  switch (pConfig.codeGenType()) {
    case LinkerConfig::Object:
      header->e_type = llvm::ELF::ET_REL;
      break;
    case LinkerConfig::DynObj:
      header->e_type = llvm::ELF::ET_DYN;
      break;
    case LinkerConfig::Exec:
      header->e_type = llvm::ELF::ET_EXEC;
      break;
    default:
      llvm::errs() << "unspported output file type: " << pConfig.codeGenType()
                   << ".\n";
      header->e_type = llvm::ELF::ET_NONE;
  }
  header->e_machine = target().getInfo().machine();
  header->e_version = header->e_ident[llvm::ELF::EI_VERSION];
  header->e_entry = getEntryPoint(pConfig, pModule);

  if (LinkerConfig::Object != pConfig.codeGenType())
    header->e_phoff = sizeof(ElfXX_Ehdr);
  else
    header->e_phoff = 0x0;

  header->e_shoff = getLastStartOffset<SIZE>(pModule);
  header->e_flags = target().getInfo().flags();
  header->e_ehsize = sizeof(ElfXX_Ehdr);
  header->e_phentsize = sizeof(ElfXX_Phdr);
  header->e_phnum = target().elfSegmentTable().size();
  header->e_shentsize = sizeof(ElfXX_Shdr);
  header->e_shnum = pModule.size();
  header->e_shstrndx = pModule.getSection(".shstrtab")->index();
}

/// getEntryPoint
uint64_t ELFObjectWriter::getEntryPoint(const LinkerConfig& pConfig,
                                        const Module& pModule) const {
  llvm::StringRef entry_name = target().getEntry(pModule);
  uint64_t result = 0x0;

  bool issue_warning = (pModule.getScript().hasEntry() &&
                        LinkerConfig::Object != pConfig.codeGenType() &&
                        LinkerConfig::DynObj != pConfig.codeGenType());

  const LDSymbol* entry_symbol = pModule.getNamePool().findSymbol(entry_name);

  // found the symbol
  if (entry_symbol != NULL) {
    if (entry_symbol->desc() != ResolveInfo::Define && issue_warning) {
      llvm::errs() << "WARNING: entry symbol '" << entry_symbol->name()
                   << "' exists but is not defined.\n";
    }
    result = entry_symbol->value();
  } else {
    // not in the symbol pool
    // We should parse entry as a number.
    // @ref GNU ld manual, Options -e. e.g., -e 0x1000.
    char* endptr;
    result = strtoull(entry_name.data(), &endptr, 0);
    if (*endptr != '\0') {
      if (issue_warning) {
        llvm::errs() << "cannot find entry symbol '" << entry_name.data()
                     << "'.\n";
      }
      result = 0x0;
    }
  }
  return result;
}

// emitSectionHeader - emit ElfXX_Shdr
template <size_t SIZE>
void ELFObjectWriter::emitSectionHeader(const Module& pModule,
                                        const LinkerConfig& pConfig,
                                        FileOutputBuffer& pOutput) const {
  typedef typename ELFSizeTraits<SIZE>::Shdr ElfXX_Shdr;

  // emit section header
  unsigned int sectNum = pModule.size();
  unsigned int header_size = sizeof(ElfXX_Shdr) * sectNum;
  MemoryRegion region =
      pOutput.request(getLastStartOffset<SIZE>(pModule), header_size);
  ElfXX_Shdr* shdr = reinterpret_cast<ElfXX_Shdr*>(region.begin());

  // Iterate the SectionTable in LDContext
  unsigned int sectIdx = 0;
  unsigned int shstridx = 0;  // NULL section has empty name
  for (; sectIdx < sectNum; ++sectIdx) {
    const LDSection* ld_sect = pModule.getSectionTable().at(sectIdx);
    shdr[sectIdx].sh_name = shstridx;
    shdr[sectIdx].sh_type = ld_sect->type();
    shdr[sectIdx].sh_flags = ld_sect->flag();
    shdr[sectIdx].sh_addr = ld_sect->addr();
    shdr[sectIdx].sh_offset = ld_sect->offset();
    shdr[sectIdx].sh_size = ld_sect->size();
    shdr[sectIdx].sh_addralign = ld_sect->align();
    shdr[sectIdx].sh_entsize = getSectEntrySize<SIZE>(*ld_sect);
    shdr[sectIdx].sh_link = getSectLink(*ld_sect, pConfig);
    shdr[sectIdx].sh_info = getSectInfo(*ld_sect);

    // adjust strshidx
    shstridx += ld_sect->name().size() + 1;
  }
}

// emitProgramHeader - emit ElfXX_Phdr
template <size_t SIZE>
void ELFObjectWriter::emitProgramHeader(FileOutputBuffer& pOutput) const {
  typedef typename ELFSizeTraits<SIZE>::Ehdr ElfXX_Ehdr;
  typedef typename ELFSizeTraits<SIZE>::Phdr ElfXX_Phdr;

  uint64_t start_offset, phdr_size;

  start_offset = sizeof(ElfXX_Ehdr);
  phdr_size = sizeof(ElfXX_Phdr);
  // Program header must start directly after ELF header
  MemoryRegion region = pOutput.request(
      start_offset, target().elfSegmentTable().size() * phdr_size);

  ElfXX_Phdr* phdr = reinterpret_cast<ElfXX_Phdr*>(region.begin());

  // Iterate the elf segment table in GNULDBackend
  size_t index = 0;
  ELFSegmentFactory::const_iterator seg = target().elfSegmentTable().begin(),
                                    segEnd = target().elfSegmentTable().end();
  for (; seg != segEnd; ++seg, ++index) {
    phdr[index].p_type = (*seg)->type();
    phdr[index].p_flags = (*seg)->flag();
    phdr[index].p_offset = (*seg)->offset();
    phdr[index].p_vaddr = (*seg)->vaddr();
    phdr[index].p_paddr = (*seg)->paddr();
    phdr[index].p_filesz = (*seg)->filesz();
    phdr[index].p_memsz = (*seg)->memsz();
    phdr[index].p_align = (*seg)->align();
  }
}

/// emitShStrTab - emit section string table
void ELFObjectWriter::emitShStrTab(const LDSection& pShStrTab,
                                   const Module& pModule,
                                   FileOutputBuffer& pOutput) {
  // write out data
  MemoryRegion region = pOutput.request(pShStrTab.offset(), pShStrTab.size());
  char* data = reinterpret_cast<char*>(region.begin());
  size_t shstrsize = 0;
  Module::const_iterator section, sectEnd = pModule.end();
  for (section = pModule.begin(); section != sectEnd; ++section) {
    ::memcpy(reinterpret_cast<char*>(data + shstrsize),
             (*section)->name().data(),
             (*section)->name().size());
    shstrsize += (*section)->name().size() + 1;
  }
}

/// emitSectionData
void ELFObjectWriter::emitSectionData(const LDSection& pSection,
                                      MemoryRegion& pRegion) const {
  const SectionData* sd = NULL;
  switch (pSection.kind()) {
    case LDFileFormat::Relocation:
      assert(pSection.hasRelocData());
      return;
    case LDFileFormat::EhFrame:
      assert(pSection.hasEhFrame());
      sd = pSection.getEhFrame()->getSectionData();
      break;
    default:
      assert(pSection.hasSectionData());
      sd = pSection.getSectionData();
      break;
  }
  emitSectionData(*sd, pRegion);
}

/// emitEhFrame
void ELFObjectWriter::emitEhFrame(Module& pModule,
                                  EhFrame& pFrame,
                                  MemoryRegion& pRegion) const {
  emitSectionData(*pFrame.getSectionData(), pRegion);

  // Patch FDE field (offset to CIE)
  for (EhFrame::cie_iterator i = pFrame.cie_begin(), e = pFrame.cie_end();
       i != e;
       ++i) {
    EhFrame::CIE& cie = **i;
    for (EhFrame::fde_iterator fi = cie.begin(), fe = cie.end(); fi != fe;
         ++fi) {
      EhFrame::FDE& fde = **fi;
      if (fde.getRecordType() == EhFrame::RECORD_GENERATED) {
        // Patch PLT offset
        LDSection* plt_sect = pModule.getSection(".plt");
        assert(plt_sect && "We have no plt but have corresponding eh_frame?");
        uint64_t plt_offset = plt_sect->offset();
        // FDE entry for PLT is always 32-bit
        uint64_t fde_offset = pFrame.getSection().offset() + fde.getOffset() +
                              EhFrame::getDataStartOffset<32>();
        int32_t offset = fde_offset - plt_offset;
        if (plt_offset < fde_offset)
          offset = -offset;
        memcpy(pRegion.begin() + fde.getOffset() +
                   EhFrame::getDataStartOffset<32>(),
               &offset,
               4);
        uint32_t size = plt_sect->size();
        memcpy(pRegion.begin() + fde.getOffset() +
                   EhFrame::getDataStartOffset<32>() + 4,
               &size,
               4);
      }
      uint64_t fde_cie_ptr_offset = fde.getOffset() +
                                    EhFrame::getDataStartOffset<32>() -
                                    /*ID*/ 4;
      uint64_t cie_start_offset = cie.getOffset();
      int32_t offset = fde_cie_ptr_offset - cie_start_offset;
      if (fde_cie_ptr_offset < cie_start_offset)
        offset = -offset;
      memcpy(pRegion.begin() + fde_cie_ptr_offset, &offset, 4);
    }  // for loop fde_iterator
  }    // for loop cie_iterator
}

/// emitRelocation
void ELFObjectWriter::emitRelocation(const LinkerConfig& pConfig,
                                     const LDSection& pSection,
                                     MemoryRegion& pRegion) const {
  const RelocData* sect_data = pSection.getRelocData();
  assert(sect_data != NULL && "SectionData is NULL in emitRelocation!");

  if (pSection.type() == llvm::ELF::SHT_REL) {
    if (pConfig.targets().is32Bits())
      emitRel<32>(pConfig, *sect_data, pRegion);
    else if (pConfig.targets().is64Bits())
      emitRel<64>(pConfig, *sect_data, pRegion);
    else {
      fatal(diag::unsupported_bitclass) << pConfig.targets().triple().str()
                                        << pConfig.targets().bitclass();
    }
  } else if (pSection.type() == llvm::ELF::SHT_RELA) {
    if (pConfig.targets().is32Bits())
      emitRela<32>(pConfig, *sect_data, pRegion);
    else if (pConfig.targets().is64Bits())
      emitRela<64>(pConfig, *sect_data, pRegion);
    else {
      fatal(diag::unsupported_bitclass) << pConfig.targets().triple().str()
                                        << pConfig.targets().bitclass();
    }
  } else
    llvm::report_fatal_error("unsupported relocation section type!");
}

// emitRel - emit ElfXX_Rel
template <size_t SIZE>
void ELFObjectWriter::emitRel(const LinkerConfig& pConfig,
                              const RelocData& pRelocData,
                              MemoryRegion& pRegion) const {
  typedef typename ELFSizeTraits<SIZE>::Rel ElfXX_Rel;
  typedef typename ELFSizeTraits<SIZE>::Addr ElfXX_Addr;
  typedef typename ELFSizeTraits<SIZE>::Word ElfXX_Word;

  ElfXX_Rel* rel = reinterpret_cast<ElfXX_Rel*>(pRegion.begin());

  const Relocation* relocation = 0;
  const FragmentRef* frag_ref = 0;

  for (RelocData::const_iterator it = pRelocData.begin(), ie = pRelocData.end();
       it != ie;
       ++it, ++rel) {
    ElfXX_Addr r_offset = 0;
    ElfXX_Word r_sym = 0;

    relocation = &(llvm::cast<Relocation>(*it));
    frag_ref = &(relocation->targetRef());

    if (LinkerConfig::DynObj == pConfig.codeGenType() ||
        LinkerConfig::Exec == pConfig.codeGenType()) {
      r_offset = static_cast<ElfXX_Addr>(
          frag_ref->frag()->getParent()->getSection().addr() +
          frag_ref->getOutputOffset());
    } else {
      r_offset = static_cast<ElfXX_Addr>(frag_ref->getOutputOffset());
    }

    if (relocation->symInfo() == NULL)
      r_sym = 0;
    else
      r_sym = static_cast<ElfXX_Word>(
          target().getSymbolIdx(relocation->symInfo()->outSymbol()));

    target().emitRelocation(*rel, relocation->type(), r_sym, r_offset);
  }
}

// emitRela - emit ElfXX_Rela
template <size_t SIZE>
void ELFObjectWriter::emitRela(const LinkerConfig& pConfig,
                               const RelocData& pRelocData,
                               MemoryRegion& pRegion) const {
  typedef typename ELFSizeTraits<SIZE>::Rela ElfXX_Rela;
  typedef typename ELFSizeTraits<SIZE>::Addr ElfXX_Addr;
  typedef typename ELFSizeTraits<SIZE>::Word ElfXX_Word;

  ElfXX_Rela* rel = reinterpret_cast<ElfXX_Rela*>(pRegion.begin());

  const Relocation* relocation = 0;
  const FragmentRef* frag_ref = 0;

  for (RelocData::const_iterator it = pRelocData.begin(), ie = pRelocData.end();
       it != ie;
       ++it, ++rel) {
    ElfXX_Addr r_offset = 0;
    ElfXX_Word r_sym = 0;

    relocation = &(llvm::cast<Relocation>(*it));
    frag_ref = &(relocation->targetRef());

    if (LinkerConfig::DynObj == pConfig.codeGenType() ||
        LinkerConfig::Exec == pConfig.codeGenType()) {
      r_offset = static_cast<ElfXX_Addr>(
          frag_ref->frag()->getParent()->getSection().addr() +
          frag_ref->getOutputOffset());
    } else {
      r_offset = static_cast<ElfXX_Addr>(frag_ref->getOutputOffset());
    }

    if (relocation->symInfo() == NULL)
      r_sym = 0;
    else
      r_sym = static_cast<ElfXX_Word>(
          target().getSymbolIdx(relocation->symInfo()->outSymbol()));

    target().emitRelocation(
        *rel, relocation->type(), r_sym, r_offset, relocation->addend());
  }
}

/// getSectEntrySize - compute ElfXX_Shdr::sh_entsize
template <size_t SIZE>
uint64_t ELFObjectWriter::getSectEntrySize(const LDSection& pSection) const {
  typedef typename ELFSizeTraits<SIZE>::Word ElfXX_Word;
  typedef typename ELFSizeTraits<SIZE>::Sym ElfXX_Sym;
  typedef typename ELFSizeTraits<SIZE>::Rel ElfXX_Rel;
  typedef typename ELFSizeTraits<SIZE>::Rela ElfXX_Rela;
  typedef typename ELFSizeTraits<SIZE>::Dyn ElfXX_Dyn;

  if (llvm::ELF::SHT_DYNSYM == pSection.type() ||
      llvm::ELF::SHT_SYMTAB == pSection.type())
    return sizeof(ElfXX_Sym);
  if (llvm::ELF::SHT_REL == pSection.type())
    return sizeof(ElfXX_Rel);
  if (llvm::ELF::SHT_RELA == pSection.type())
    return sizeof(ElfXX_Rela);
  if (llvm::ELF::SHT_HASH == pSection.type() ||
      llvm::ELF::SHT_GNU_HASH == pSection.type())
    return sizeof(ElfXX_Word);
  if (llvm::ELF::SHT_DYNAMIC == pSection.type())
    return sizeof(ElfXX_Dyn);
  // FIXME: We should get the entsize from input since the size of each
  // character is specified in the section header's sh_entsize field.
  // For example, traditional string is 0x1, UCS-2 is 0x2, ... and so on.
  // Ref: http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html
  if (pSection.flag() & llvm::ELF::SHF_STRINGS)
    return 0x1;
  return 0x0;
}

/// getSectLink - compute ElfXX_Shdr::sh_link
uint64_t ELFObjectWriter::getSectLink(const LDSection& pSection,
                                      const LinkerConfig& pConfig) const {
  if (llvm::ELF::SHT_SYMTAB == pSection.type())
    return target().getOutputFormat()->getStrTab().index();
  if (llvm::ELF::SHT_DYNSYM == pSection.type())
    return target().getOutputFormat()->getDynStrTab().index();
  if (llvm::ELF::SHT_DYNAMIC == pSection.type())
    return target().getOutputFormat()->getDynStrTab().index();
  if (llvm::ELF::SHT_HASH == pSection.type() ||
      llvm::ELF::SHT_GNU_HASH == pSection.type())
    return target().getOutputFormat()->getDynSymTab().index();
  if (llvm::ELF::SHT_REL == pSection.type() ||
      llvm::ELF::SHT_RELA == pSection.type()) {
    if (LinkerConfig::Object == pConfig.codeGenType())
      return target().getOutputFormat()->getSymTab().index();
    else
      return target().getOutputFormat()->getDynSymTab().index();
  }
  // FIXME: currently we link ARM_EXIDX section to output text section here
  if (llvm::ELF::SHT_ARM_EXIDX == pSection.type())
    return target().getOutputFormat()->getText().index();
  return llvm::ELF::SHN_UNDEF;
}

/// getSectInfo - compute ElfXX_Shdr::sh_info
uint64_t ELFObjectWriter::getSectInfo(const LDSection& pSection) const {
  if (llvm::ELF::SHT_SYMTAB == pSection.type() ||
      llvm::ELF::SHT_DYNSYM == pSection.type())
    return pSection.getInfo();

  if (llvm::ELF::SHT_REL == pSection.type() ||
      llvm::ELF::SHT_RELA == pSection.type()) {
    const LDSection* info_link = pSection.getLink();
    if (info_link != NULL)
      return info_link->index();
  }

  return 0x0;
}

/// getLastStartOffset
template <>
uint64_t ELFObjectWriter::getLastStartOffset<32>(const Module& pModule) const {
  const LDSection* lastSect = pModule.back();
  assert(lastSect != NULL);
  return Align<32>(lastSect->offset() + lastSect->size());
}

/// getLastStartOffset
template <>
uint64_t ELFObjectWriter::getLastStartOffset<64>(const Module& pModule) const {
  const LDSection* lastSect = pModule.back();
  assert(lastSect != NULL);
  return Align<64>(lastSect->offset() + lastSect->size());
}

/// emitSectionData
void ELFObjectWriter::emitSectionData(const SectionData& pSD,
                                      MemoryRegion& pRegion) const {
  SectionData::const_iterator fragIter, fragEnd = pSD.end();
  size_t cur_offset = 0;
  for (fragIter = pSD.begin(); fragIter != fragEnd; ++fragIter) {
    size_t size = fragIter->size();
    switch (fragIter->getKind()) {
      case Fragment::Region: {
        const RegionFragment& region_frag =
            llvm::cast<RegionFragment>(*fragIter);
        const char* from = region_frag.getRegion().begin();
        memcpy(pRegion.begin() + cur_offset, from, size);
        break;
      }
      case Fragment::Alignment: {
        // TODO: emit values with different sizes (> 1 byte), and emit nops
        const AlignFragment& align_frag = llvm::cast<AlignFragment>(*fragIter);
        uint64_t count = size / align_frag.getValueSize();
        switch (align_frag.getValueSize()) {
          case 1u:
            std::memset(
                pRegion.begin() + cur_offset, align_frag.getValue(), count);
            break;
          default:
            llvm::report_fatal_error(
                "unsupported value size for align fragment emission yet.\n");
            break;
        }
        break;
      }
      case Fragment::Fillment: {
        const FillFragment& fill_frag = llvm::cast<FillFragment>(*fragIter);
        if (0 == size || 0 == fill_frag.getValueSize() ||
            0 == fill_frag.size()) {
          // ignore virtual fillment
          break;
        }

        uint64_t num_tiles = fill_frag.size() / fill_frag.getValueSize();
        for (uint64_t i = 0; i != num_tiles; ++i) {
          std::memset(pRegion.begin() + cur_offset,
                      fill_frag.getValue(),
                      fill_frag.getValueSize());
        }
        break;
      }
      case Fragment::Stub: {
        const Stub& stub_frag = llvm::cast<Stub>(*fragIter);
        memcpy(pRegion.begin() + cur_offset, stub_frag.getContent(), size);
        break;
      }
      case Fragment::Null: {
        assert(0x0 == size);
        break;
      }
      case Fragment::Target:
        llvm::report_fatal_error(
            "Target fragment should not be in a regular section.\n");
        break;
      default:
        llvm::report_fatal_error(
            "invalid fragment should not be in a regular section.\n");
        break;
    }
    cur_offset += size;
  }
}

}  // namespace mcld
