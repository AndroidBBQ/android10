//===- ELFReader.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFReader.h"

#include "mcld/IRBuilder.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/GNUInfo.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

#include <iostream>

#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFReader<32, true>
//===----------------------------------------------------------------------===//
/// constructor
ELFReader<32, true>::ELFReader(GNULDBackend& pBackend) : ELFReaderIF(pBackend) {
}

/// destructor
ELFReader<32, true>::~ELFReader() {
}

/// isELF - is this a ELF file
bool ELFReader<32, true>::isELF(const void* pELFHeader) const {
  const llvm::ELF::Elf32_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(pELFHeader);
  if (memcmp(llvm::ELF::ElfMagic, hdr, 4) == 0)
    return true;
  return false;
}

/// readRegularSection - read a regular section and create fragments.
bool ELFReader<32, true>::readRegularSection(Input& pInput,
                                             SectionData& pSD) const {
  uint32_t offset = pInput.fileOffset() + pSD.getSection().offset();
  uint32_t size = pSD.getSection().size();

  Fragment* frag = IRBuilder::CreateRegion(pInput, offset, size);
  ObjectBuilder::AppendFragment(*frag, pSD);
  return true;
}

/// readSymbols - read ELF symbols and create LDSymbol
bool ELFReader<32, true>::readSymbols(Input& pInput,
                                      IRBuilder& pBuilder,
                                      llvm::StringRef pRegion,
                                      const char* pStrTab) const {
  // get number of symbols
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf32_Sym);
  const llvm::ELF::Elf32_Sym* symtab =
      reinterpret_cast<const llvm::ELF::Elf32_Sym*>(pRegion.begin());

  uint32_t st_name = 0x0;
  uint32_t st_value = 0x0;
  uint32_t st_size = 0x0;
  uint8_t st_info = 0x0;
  uint8_t st_other = 0x0;
  uint16_t st_shndx = 0x0;

  // skip the first NULL symbol
  pInput.context()->addSymbol(LDSymbol::Null());

  /// recording symbols added from DynObj to analyze weak alias
  std::vector<AliasInfo> potential_aliases;
  bool is_dyn_obj = (pInput.type() == Input::DynObj);
  for (size_t idx = 1; idx < entsize; ++idx) {
    st_info = symtab[idx].st_info;
    st_other = symtab[idx].st_other;

    if (llvm::sys::IsLittleEndianHost) {
      st_name = symtab[idx].st_name;
      st_value = symtab[idx].st_value;
      st_size = symtab[idx].st_size;
      st_shndx = symtab[idx].st_shndx;
    } else {
      st_name = mcld::bswap32(symtab[idx].st_name);
      st_value = mcld::bswap32(symtab[idx].st_value);
      st_size = mcld::bswap32(symtab[idx].st_size);
      st_shndx = mcld::bswap16(symtab[idx].st_shndx);
    }

    // If the section should not be included, set the st_shndx SHN_UNDEF
    // - A section in interrelated groups are not included.
    if (pInput.type() == Input::Object && st_shndx < llvm::ELF::SHN_LORESERVE &&
        st_shndx != llvm::ELF::SHN_UNDEF) {
      if (pInput.context()->getSection(st_shndx) == NULL)
        st_shndx = llvm::ELF::SHN_UNDEF;
    }

    // get ld_type
    ResolveInfo::Type ld_type = getSymType(st_info, st_shndx);

    // get ld_desc
    ResolveInfo::Desc ld_desc = getSymDesc(st_shndx, pInput);

    // get ld_binding
    ResolveInfo::Binding ld_binding =
        getSymBinding((st_info >> 4), st_shndx, st_other);

    // get ld_value - ld_value must be section relative.
    uint64_t ld_value = getSymValue(st_value, st_shndx, pInput);

    // get ld_vis
    ResolveInfo::Visibility ld_vis = getSymVisibility(st_other);

    // get section
    LDSection* section = NULL;
    if (st_shndx < llvm::ELF::SHN_LORESERVE)  // including ABS and COMMON
      section = pInput.context()->getSection(st_shndx);

    // get ld_name
    std::string ld_name;
    if (ResolveInfo::Section == ld_type) {
      // Section symbol's st_name is the section index.
      assert(section != NULL && "get a invalid section");
      ld_name = section->name();
    } else {
      ld_name = std::string(pStrTab + st_name);
    }

    LDSymbol* psym = pBuilder.AddSymbol(pInput,
                                        ld_name,
                                        ld_type,
                                        ld_desc,
                                        ld_binding,
                                        st_size,
                                        ld_value,
                                        section,
                                        ld_vis);

    if (is_dyn_obj && psym != NULL && ResolveInfo::Undefined != ld_desc &&
        (ResolveInfo::Global == ld_binding ||
         ResolveInfo::Weak == ld_binding) &&
        ResolveInfo::Object == ld_type) {
      AliasInfo p;
      p.pt_alias = psym;
      p.ld_binding = ld_binding;
      p.ld_value = ld_value;
      potential_aliases.push_back(p);
    }
  }  // end of for loop

  // analyze weak alias
  // FIXME: it is better to let IRBuilder handle alias anlysis.
  //        1. eliminate code duplication
  //        2. easy to know if a symbol is from .so
  //           (so that it may be a potential alias)
  if (is_dyn_obj) {
    // sort symbols by symbol value and then weak before strong
    std::sort(potential_aliases.begin(), potential_aliases.end(), less);

    // for each weak symbol, find out all its aliases, and
    // then link them as a circular list in Module
    std::vector<AliasInfo>::iterator sym_it, sym_e;
    sym_e = potential_aliases.end();
    for (sym_it = potential_aliases.begin(); sym_it != sym_e; ++sym_it) {
      if (ResolveInfo::Weak != sym_it->ld_binding)
        continue;

      Module& pModule = pBuilder.getModule();
      std::vector<AliasInfo>::iterator alias_it = sym_it + 1;
      while (alias_it != sym_e) {
        if (sym_it->ld_value != alias_it->ld_value)
          break;

        if (sym_it + 1 == alias_it)
          pModule.CreateAliasList(*sym_it->pt_alias->resolveInfo());
        pModule.addAlias(*alias_it->pt_alias->resolveInfo());
        ++alias_it;
      }

      sym_it = alias_it - 1;
    }  // end of for loop
  }

  return true;
}

//===----------------------------------------------------------------------===//
// ELFReader::read relocations - read ELF rela and rel, and create Relocation
//===----------------------------------------------------------------------===//
/// ELFReader::readRela - read ELF rela and create Relocation
bool ELFReader<32, true>::readRela(Input& pInput,
                                   LDSection& pSection,
                                   llvm::StringRef pRegion) const {
  // get the number of rela
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf32_Rela);
  const llvm::ELF::Elf32_Rela* relaTab =
      reinterpret_cast<const llvm::ELF::Elf32_Rela*>(pRegion.begin());

  for (size_t idx = 0; idx < entsize; ++idx) {
    Relocation::Type r_type = 0x0;
    uint32_t r_sym = 0x0;
    uint32_t r_offset = 0x0;
    int32_t r_addend = 0;
    if (!target()
             .readRelocation(relaTab[idx], r_type, r_sym, r_offset, r_addend)) {
      return false;
    }

    LDSymbol* symbol = pInput.context()->getSymbol(r_sym);
    if (symbol == NULL) {
      fatal(diag::err_cannot_read_symbol) << r_sym << pInput.path();
    }

    IRBuilder::AddRelocation(pSection, r_type, *symbol, r_offset, r_addend);
  }  // end of for
  return true;
}

/// readRel - read ELF rel and create Relocation
bool ELFReader<32, true>::readRel(Input& pInput,
                                  LDSection& pSection,
                                  llvm::StringRef pRegion) const {
  // get the number of rel
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf32_Rel);
  const llvm::ELF::Elf32_Rel* relTab =
      reinterpret_cast<const llvm::ELF::Elf32_Rel*>(pRegion.begin());

  for (size_t idx = 0; idx < entsize; ++idx) {
    Relocation::Type r_type = 0x0;
    uint32_t r_sym = 0x0;
    uint32_t r_offset = 0x0;

    if (!target().readRelocation(relTab[idx], r_type, r_sym, r_offset))
      return false;

    LDSymbol* symbol = pInput.context()->getSymbol(r_sym);
    if (symbol == NULL) {
      fatal(diag::err_cannot_read_symbol) << r_sym << pInput.path();
    }

    IRBuilder::AddRelocation(pSection, r_type, *symbol, r_offset);
  }  // end of for
  return true;
}

/// isMyEndian - is this ELF file in the same endian to me?
bool ELFReader<32, true>::isMyEndian(const void* pELFHeader) const {
  const llvm::ELF::Elf32_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(pELFHeader);

  return (hdr->e_ident[llvm::ELF::EI_DATA] == llvm::ELF::ELFDATA2LSB);
}

/// isMyMachine - is this ELF file generated for the same machine.
bool ELFReader<32, true>::isMyMachine(const void* pELFHeader) const {
  const llvm::ELF::Elf32_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(pELFHeader);

  if (llvm::sys::IsLittleEndianHost)
    return (hdr->e_machine == target().getInfo().machine());
  return (mcld::bswap16(hdr->e_machine) == target().getInfo().machine());
}

/// fileType - return the file type
Input::Type ELFReader<32, true>::fileType(const void* pELFHeader) const {
  const llvm::ELF::Elf32_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(pELFHeader);
  uint32_t type = 0x0;
  if (llvm::sys::IsLittleEndianHost)
    type = hdr->e_type;
  else
    type = mcld::bswap16(hdr->e_type);

  switch (type) {
    case llvm::ELF::ET_REL:
      return Input::Object;
    case llvm::ELF::ET_EXEC:
      return Input::Exec;
    case llvm::ELF::ET_DYN:
      return Input::DynObj;
    case llvm::ELF::ET_CORE:
      return Input::CoreFile;
    case llvm::ELF::ET_NONE:
    default:
      return Input::Unknown;
  }
}

/// readSectionHeaders - read ELF section header table and create LDSections
bool ELFReader<32, true>::readSectionHeaders(Input& pInput,
                                             const void* pELFHeader) const {
  const llvm::ELF::Elf32_Ehdr* ehdr =
      reinterpret_cast<const llvm::ELF::Elf32_Ehdr*>(pELFHeader);

  uint32_t shoff = 0x0;
  uint16_t shentsize = 0x0;
  uint32_t shnum = 0x0;
  uint32_t shstrtab = 0x0;

  if (llvm::sys::IsLittleEndianHost) {
    shoff = ehdr->e_shoff;
    shentsize = ehdr->e_shentsize;
    shnum = ehdr->e_shnum;
    shstrtab = ehdr->e_shstrndx;
  } else {
    shoff = mcld::bswap32(ehdr->e_shoff);
    shentsize = mcld::bswap16(ehdr->e_shentsize);
    shnum = mcld::bswap16(ehdr->e_shnum);
    shstrtab = mcld::bswap16(ehdr->e_shstrndx);
  }

  // If the file has no section header table, e_shoff holds zero.
  if (shoff == 0x0)
    return true;

  const llvm::ELF::Elf32_Shdr* shdr = NULL;
  llvm::StringRef shdr_region;
  uint32_t sh_name = 0x0;
  uint32_t sh_type = 0x0;
  uint32_t sh_flags = 0x0;
  uint32_t sh_offset = 0x0;
  uint32_t sh_size = 0x0;
  uint32_t sh_link = 0x0;
  uint32_t sh_info = 0x0;
  uint32_t sh_addralign = 0x0;

  // if shnum and shstrtab overflow, the actual values are in the 1st shdr
  if (shnum == llvm::ELF::SHN_UNDEF || shstrtab == llvm::ELF::SHN_XINDEX) {
    shdr_region =
        pInput.memArea()->request(pInput.fileOffset() + shoff, shentsize);
    shdr = reinterpret_cast<const llvm::ELF::Elf32_Shdr*>(shdr_region.begin());

    if (llvm::sys::IsLittleEndianHost) {
      sh_size = shdr->sh_size;
      sh_link = shdr->sh_link;
    } else {
      sh_size = mcld::bswap32(shdr->sh_size);
      sh_link = mcld::bswap32(shdr->sh_link);
    }

    if (shnum == llvm::ELF::SHN_UNDEF)
      shnum = sh_size;
    if (shstrtab == llvm::ELF::SHN_XINDEX)
      shstrtab = sh_link;

    shoff += shentsize;
  }

  shdr_region =
      pInput.memArea()->request(pInput.fileOffset() + shoff, shnum * shentsize);
  const llvm::ELF::Elf32_Shdr* shdrTab =
      reinterpret_cast<const llvm::ELF::Elf32_Shdr*>(shdr_region.begin());

  // get .shstrtab first
  shdr = &shdrTab[shstrtab];
  if (llvm::sys::IsLittleEndianHost) {
    sh_offset = shdr->sh_offset;
    sh_size = shdr->sh_size;
  } else {
    sh_offset = mcld::bswap32(shdr->sh_offset);
    sh_size = mcld::bswap32(shdr->sh_size);
  }

  llvm::StringRef sect_name_region =
      pInput.memArea()->request(pInput.fileOffset() + sh_offset, sh_size);
  const char* sect_name = sect_name_region.begin();

  LinkInfoList link_info_list;

  // create all LDSections, including first NULL section.
  for (size_t idx = 0; idx < shnum; ++idx) {
    if (llvm::sys::IsLittleEndianHost) {
      sh_name = shdrTab[idx].sh_name;
      sh_type = shdrTab[idx].sh_type;
      sh_flags = shdrTab[idx].sh_flags;
      sh_offset = shdrTab[idx].sh_offset;
      sh_size = shdrTab[idx].sh_size;
      sh_link = shdrTab[idx].sh_link;
      sh_info = shdrTab[idx].sh_info;
      sh_addralign = shdrTab[idx].sh_addralign;
    } else {
      sh_name = mcld::bswap32(shdrTab[idx].sh_name);
      sh_type = mcld::bswap32(shdrTab[idx].sh_type);
      sh_flags = mcld::bswap32(shdrTab[idx].sh_flags);
      sh_offset = mcld::bswap32(shdrTab[idx].sh_offset);
      sh_size = mcld::bswap32(shdrTab[idx].sh_size);
      sh_link = mcld::bswap32(shdrTab[idx].sh_link);
      sh_info = mcld::bswap32(shdrTab[idx].sh_info);
      sh_addralign = mcld::bswap32(shdrTab[idx].sh_addralign);
    }

    LDSection* section = IRBuilder::CreateELFHeader(
        pInput, sect_name + sh_name, sh_type, sh_flags, sh_addralign);
    section->setSize(sh_size);
    section->setOffset(sh_offset);
    section->setInfo(sh_info);

    if (sh_link != 0x0 || sh_info != 0x0) {
      LinkInfo link_info = {section, sh_link, sh_info};
      link_info_list.push_back(link_info);
    }
  }  // end of for

  // set up InfoLink
  LinkInfoList::iterator info, infoEnd = link_info_list.end();
  for (info = link_info_list.begin(); info != infoEnd; ++info) {
    if (LDFileFormat::Relocation == info->section->kind())
      info->section->setLink(pInput.context()->getSection(info->sh_info));
    else
      info->section->setLink(pInput.context()->getSection(info->sh_link));
  }

  return true;
}

/// readSignature - read a symbol from the given Input and index in symtab
/// This is used to get the signature of a group section.
ResolveInfo* ELFReader<32, true>::readSignature(Input& pInput,
                                                LDSection& pSymTab,
                                                uint32_t pSymIdx) const {
  LDSection* symtab = &pSymTab;
  LDSection* strtab = symtab->getLink();
  assert(symtab != NULL && strtab != NULL);

  uint32_t offset = pInput.fileOffset() + symtab->offset() +
                    sizeof(llvm::ELF::Elf32_Sym) * pSymIdx;
  llvm::StringRef symbol_region =
      pInput.memArea()->request(offset, sizeof(llvm::ELF::Elf32_Sym));
  const llvm::ELF::Elf32_Sym* entry =
      reinterpret_cast<const llvm::ELF::Elf32_Sym*>(symbol_region.begin());

  uint32_t st_name = 0x0;
  uint8_t st_info = 0x0;
  uint8_t st_other = 0x0;
  uint16_t st_shndx = 0x0;
  st_info = entry->st_info;
  st_other = entry->st_other;
  if (llvm::sys::IsLittleEndianHost) {
    st_name = entry->st_name;
    st_shndx = entry->st_shndx;
  } else {
    st_name = mcld::bswap32(entry->st_name);
    st_shndx = mcld::bswap16(entry->st_shndx);
  }

  llvm::StringRef strtab_region = pInput.memArea()->request(
      pInput.fileOffset() + strtab->offset(), strtab->size());

  // get ld_name
  llvm::StringRef ld_name(strtab_region.begin() + st_name);

  ResolveInfo* result = ResolveInfo::Create(ld_name);
  result->setSource(pInput.type() == Input::DynObj);
  result->setType(static_cast<ResolveInfo::Type>(st_info & 0xF));
  result->setDesc(getSymDesc(st_shndx, pInput));
  result->setBinding(getSymBinding((st_info >> 4), st_shndx, st_other));
  result->setVisibility(getSymVisibility(st_other));

  return result;
}

/// readDynamic - read ELF .dynamic in input dynobj
bool ELFReader<32, true>::readDynamic(Input& pInput) const {
  assert(pInput.type() == Input::DynObj);
  const LDSection* dynamic_sect = pInput.context()->getSection(".dynamic");
  if (dynamic_sect == NULL) {
    fatal(diag::err_cannot_read_section) << ".dynamic";
  }
  const LDSection* dynstr_sect = dynamic_sect->getLink();
  if (dynstr_sect == NULL) {
    fatal(diag::err_cannot_read_section) << ".dynstr";
  }

  llvm::StringRef dynamic_region = pInput.memArea()->request(
      pInput.fileOffset() + dynamic_sect->offset(), dynamic_sect->size());

  llvm::StringRef dynstr_region = pInput.memArea()->request(
      pInput.fileOffset() + dynstr_sect->offset(), dynstr_sect->size());

  const llvm::ELF::Elf32_Dyn* dynamic =
      reinterpret_cast<const llvm::ELF::Elf32_Dyn*>(dynamic_region.begin());
  const char* dynstr = dynstr_region.begin();
  bool hasSOName = false;
  size_t numOfEntries = dynamic_sect->size() / sizeof(llvm::ELF::Elf32_Dyn);

  for (size_t idx = 0; idx < numOfEntries; ++idx) {
    llvm::ELF::Elf32_Sword d_tag = 0x0;
    llvm::ELF::Elf32_Word d_val = 0x0;

    if (llvm::sys::IsLittleEndianHost) {
      d_tag = dynamic[idx].d_tag;
      d_val = dynamic[idx].d_un.d_val;
    } else {
      d_tag = mcld::bswap32(dynamic[idx].d_tag);
      d_val = mcld::bswap32(dynamic[idx].d_un.d_val);
    }

    switch (d_tag) {
      case llvm::ELF::DT_SONAME:
        assert(d_val < dynstr_sect->size());
        pInput.setName(sys::fs::Path(dynstr + d_val).filename().native());
        hasSOName = true;
        break;
      case llvm::ELF::DT_NEEDED:
        // TODO:
        break;
      case llvm::ELF::DT_NULL:
      default:
        break;
    }
  }

  // if there is no SONAME in .dynamic, then set it from input path
  if (!hasSOName)
    pInput.setName(pInput.path().filename().native());

  return true;
}

//===----------------------------------------------------------------------===//
// ELFReader<64, true>
//===----------------------------------------------------------------------===//
/// constructor
ELFReader<64, true>::ELFReader(GNULDBackend& pBackend) : ELFReaderIF(pBackend) {
}

/// destructor
ELFReader<64, true>::~ELFReader() {
}

/// isELF - is this a ELF file
bool ELFReader<64, true>::isELF(const void* pELFHeader) const {
  const llvm::ELF::Elf64_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(pELFHeader);
  if (memcmp(llvm::ELF::ElfMagic, hdr, 4) == 0)
    return true;
  return false;
}

/// readRegularSection - read a regular section and create fragments.
bool ELFReader<64, true>::readRegularSection(Input& pInput,
                                             SectionData& pSD) const {
  uint64_t offset = pInput.fileOffset() + pSD.getSection().offset();
  uint64_t size = pSD.getSection().size();

  Fragment* frag = IRBuilder::CreateRegion(pInput, offset, size);
  ObjectBuilder::AppendFragment(*frag, pSD);
  return true;
}

/// readSymbols - read ELF symbols and create LDSymbol
bool ELFReader<64, true>::readSymbols(Input& pInput,
                                      IRBuilder& pBuilder,
                                      llvm::StringRef pRegion,
                                      const char* pStrTab) const {
  // get number of symbols
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf64_Sym);
  const llvm::ELF::Elf64_Sym* symtab =
      reinterpret_cast<const llvm::ELF::Elf64_Sym*>(pRegion.begin());

  uint32_t st_name = 0x0;
  uint64_t st_value = 0x0;
  uint64_t st_size = 0x0;
  uint8_t st_info = 0x0;
  uint8_t st_other = 0x0;
  uint16_t st_shndx = 0x0;

  // skip the first NULL symbol
  pInput.context()->addSymbol(LDSymbol::Null());

  /// recording symbols added from DynObj to analyze weak alias
  std::vector<AliasInfo> potential_aliases;
  bool is_dyn_obj = (pInput.type() == Input::DynObj);
  for (size_t idx = 1; idx < entsize; ++idx) {
    st_info = symtab[idx].st_info;
    st_other = symtab[idx].st_other;

    if (llvm::sys::IsLittleEndianHost) {
      st_name = symtab[idx].st_name;
      st_value = symtab[idx].st_value;
      st_size = symtab[idx].st_size;
      st_shndx = symtab[idx].st_shndx;
    } else {
      st_name = mcld::bswap32(symtab[idx].st_name);
      st_value = mcld::bswap64(symtab[idx].st_value);
      st_size = mcld::bswap64(symtab[idx].st_size);
      st_shndx = mcld::bswap16(symtab[idx].st_shndx);
    }

    // If the section should not be included, set the st_shndx SHN_UNDEF
    // - A section in interrelated groups are not included.
    if (pInput.type() == Input::Object && st_shndx < llvm::ELF::SHN_LORESERVE &&
        st_shndx != llvm::ELF::SHN_UNDEF) {
      if (pInput.context()->getSection(st_shndx) == NULL)
        st_shndx = llvm::ELF::SHN_UNDEF;
    }

    // get ld_type
    ResolveInfo::Type ld_type = getSymType(st_info, st_shndx);

    // get ld_desc
    ResolveInfo::Desc ld_desc = getSymDesc(st_shndx, pInput);

    // get ld_binding
    ResolveInfo::Binding ld_binding =
        getSymBinding((st_info >> 4), st_shndx, st_other);

    // get ld_value - ld_value must be section relative.
    uint64_t ld_value = getSymValue(st_value, st_shndx, pInput);

    // get ld_vis
    ResolveInfo::Visibility ld_vis = getSymVisibility(st_other);

    // get section
    LDSection* section = NULL;
    if (st_shndx < llvm::ELF::SHN_LORESERVE)  // including ABS and COMMON
      section = pInput.context()->getSection(st_shndx);

    // get ld_name
    std::string ld_name;
    if (ResolveInfo::Section == ld_type) {
      // Section symbol's st_name is the section index.
      assert(section != NULL && "get a invalid section");
      ld_name = section->name();
    } else {
      ld_name = std::string(pStrTab + st_name);
    }

    LDSymbol* psym = pBuilder.AddSymbol(pInput,
                                        ld_name,
                                        ld_type,
                                        ld_desc,
                                        ld_binding,
                                        st_size,
                                        ld_value,
                                        section,
                                        ld_vis);

    if (is_dyn_obj && psym != NULL && ResolveInfo::Undefined != ld_desc &&
        (ResolveInfo::Global == ld_binding ||
         ResolveInfo::Weak == ld_binding) &&
        ResolveInfo::Object == ld_type) {
      AliasInfo p;
      p.pt_alias = psym;
      p.ld_binding = ld_binding;
      p.ld_value = ld_value;
      potential_aliases.push_back(p);
    }
  }  // end of for loop

  // analyze weak alias here
  if (is_dyn_obj) {
    // sort symbols by symbol value and then weak before strong
    std::sort(potential_aliases.begin(), potential_aliases.end(), less);

    // for each weak symbol, find out all its aliases, and
    // then link them as a circular list in Module
    std::vector<AliasInfo>::iterator sym_it, sym_e;
    sym_e = potential_aliases.end();
    for (sym_it = potential_aliases.begin(); sym_it != sym_e; ++sym_it) {
      if (ResolveInfo::Weak != sym_it->ld_binding)
        continue;

      Module& pModule = pBuilder.getModule();
      std::vector<AliasInfo>::iterator alias_it = sym_it + 1;
      while (alias_it != sym_e) {
        if (sym_it->ld_value != alias_it->ld_value)
          break;

        if (sym_it + 1 == alias_it)
          pModule.CreateAliasList(*sym_it->pt_alias->resolveInfo());
        pModule.addAlias(*alias_it->pt_alias->resolveInfo());
        ++alias_it;
      }

      sym_it = alias_it - 1;
    }  // end of for loop
  }
  return true;
}

//===----------------------------------------------------------------------===//
// ELFReader::read relocations - read ELF rela and rel, and create Relocation
//===----------------------------------------------------------------------===//
/// ELFReader::readRela - read ELF rela and create Relocation
bool ELFReader<64, true>::readRela(Input& pInput,
                                   LDSection& pSection,
                                   llvm::StringRef pRegion) const {
  // get the number of rela
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf64_Rela);
  const llvm::ELF::Elf64_Rela* relaTab =
      reinterpret_cast<const llvm::ELF::Elf64_Rela*>(pRegion.begin());

  for (size_t idx = 0; idx < entsize; ++idx) {
    Relocation::Type r_type = 0x0;
    uint32_t r_sym = 0x0;
    uint64_t r_offset = 0x0;
    int64_t r_addend = 0;
    if (!target()
             .readRelocation(relaTab[idx], r_type, r_sym, r_offset, r_addend)) {
      return false;
    }

    LDSymbol* symbol = pInput.context()->getSymbol(r_sym);
    if (symbol == NULL) {
      fatal(diag::err_cannot_read_symbol) << r_sym << pInput.path();
    }

    IRBuilder::AddRelocation(pSection, r_type, *symbol, r_offset, r_addend);
  }  // end of for
  return true;
}

/// readRel - read ELF rel and create Relocation
bool ELFReader<64, true>::readRel(Input& pInput,
                                  LDSection& pSection,
                                  llvm::StringRef pRegion) const {
  // get the number of rel
  size_t entsize = pRegion.size() / sizeof(llvm::ELF::Elf64_Rel);
  const llvm::ELF::Elf64_Rel* relTab =
      reinterpret_cast<const llvm::ELF::Elf64_Rel*>(pRegion.begin());

  for (size_t idx = 0; idx < entsize; ++idx) {
    Relocation::Type r_type = 0x0;
    uint32_t r_sym = 0x0;
    uint64_t r_offset = 0x0;
    if (!target().readRelocation(relTab[idx], r_type, r_sym, r_offset))
      return false;

    LDSymbol* symbol = pInput.context()->getSymbol(r_sym);
    if (symbol == NULL) {
      fatal(diag::err_cannot_read_symbol) << r_sym << pInput.path();
    }

    IRBuilder::AddRelocation(pSection, r_type, *symbol, r_offset);
  }  // end of for
  return true;
}

/// isMyEndian - is this ELF file in the same endian to me?
bool ELFReader<64, true>::isMyEndian(const void* pELFHeader) const {
  const llvm::ELF::Elf64_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(pELFHeader);

  return (hdr->e_ident[llvm::ELF::EI_DATA] == llvm::ELF::ELFDATA2LSB);
}

/// isMyMachine - is this ELF file generated for the same machine.
bool ELFReader<64, true>::isMyMachine(const void* pELFHeader) const {
  const llvm::ELF::Elf64_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(pELFHeader);

  if (llvm::sys::IsLittleEndianHost)
    return (hdr->e_machine == target().getInfo().machine());
  return (mcld::bswap16(hdr->e_machine) == target().getInfo().machine());
}

/// fileType - return the file type
Input::Type ELFReader<64, true>::fileType(const void* pELFHeader) const {
  const llvm::ELF::Elf64_Ehdr* hdr =
      reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(pELFHeader);
  uint32_t type = 0x0;
  if (llvm::sys::IsLittleEndianHost)
    type = hdr->e_type;
  else
    type = mcld::bswap16(hdr->e_type);

  switch (type) {
    case llvm::ELF::ET_REL:
      return Input::Object;
    case llvm::ELF::ET_EXEC:
      return Input::Exec;
    case llvm::ELF::ET_DYN:
      return Input::DynObj;
    case llvm::ELF::ET_CORE:
      return Input::CoreFile;
    case llvm::ELF::ET_NONE:
    default:
      return Input::Unknown;
  }
}

/// readSectionHeaders - read ELF section header table and create LDSections
bool ELFReader<64, true>::readSectionHeaders(Input& pInput,
                                             const void* pELFHeader) const {
  const llvm::ELF::Elf64_Ehdr* ehdr =
      reinterpret_cast<const llvm::ELF::Elf64_Ehdr*>(pELFHeader);

  uint64_t shoff = 0x0;
  uint16_t shentsize = 0x0;
  uint32_t shnum = 0x0;
  uint32_t shstrtab = 0x0;

  if (llvm::sys::IsLittleEndianHost) {
    shoff = ehdr->e_shoff;
    shentsize = ehdr->e_shentsize;
    shnum = ehdr->e_shnum;
    shstrtab = ehdr->e_shstrndx;
  } else {
    shoff = mcld::bswap64(ehdr->e_shoff);
    shentsize = mcld::bswap16(ehdr->e_shentsize);
    shnum = mcld::bswap16(ehdr->e_shnum);
    shstrtab = mcld::bswap16(ehdr->e_shstrndx);
  }

  // If the file has no section header table, e_shoff holds zero.
  if (shoff == 0x0)
    return true;

  const llvm::ELF::Elf64_Shdr* shdr = NULL;
  llvm::StringRef shdr_region;
  uint32_t sh_name = 0x0;
  uint32_t sh_type = 0x0;
  uint64_t sh_flags = 0x0;
  uint64_t sh_offset = 0x0;
  uint64_t sh_size = 0x0;
  uint32_t sh_link = 0x0;
  uint32_t sh_info = 0x0;
  uint64_t sh_addralign = 0x0;

  // if shnum and shstrtab overflow, the actual values are in the 1st shdr
  if (shnum == llvm::ELF::SHN_UNDEF || shstrtab == llvm::ELF::SHN_XINDEX) {
    shdr_region =
        pInput.memArea()->request(pInput.fileOffset() + shoff, shentsize);
    shdr = reinterpret_cast<const llvm::ELF::Elf64_Shdr*>(shdr_region.begin());

    if (llvm::sys::IsLittleEndianHost) {
      sh_size = shdr->sh_size;
      sh_link = shdr->sh_link;
    } else {
      sh_size = mcld::bswap64(shdr->sh_size);
      sh_link = mcld::bswap32(shdr->sh_link);
    }

    if (shnum == llvm::ELF::SHN_UNDEF)
      shnum = sh_size;
    if (shstrtab == llvm::ELF::SHN_XINDEX)
      shstrtab = sh_link;

    shoff += shentsize;
  }

  shdr_region =
      pInput.memArea()->request(pInput.fileOffset() + shoff, shnum * shentsize);
  const llvm::ELF::Elf64_Shdr* shdrTab =
      reinterpret_cast<const llvm::ELF::Elf64_Shdr*>(shdr_region.begin());

  // get .shstrtab first
  shdr = &shdrTab[shstrtab];
  if (llvm::sys::IsLittleEndianHost) {
    sh_offset = shdr->sh_offset;
    sh_size = shdr->sh_size;
  } else {
    sh_offset = mcld::bswap64(shdr->sh_offset);
    sh_size = mcld::bswap64(shdr->sh_size);
  }

  llvm::StringRef sect_name_region =
      pInput.memArea()->request(pInput.fileOffset() + sh_offset, sh_size);
  const char* sect_name = sect_name_region.begin();

  LinkInfoList link_info_list;

  // create all LDSections, including first NULL section.
  for (size_t idx = 0; idx < shnum; ++idx) {
    if (llvm::sys::IsLittleEndianHost) {
      sh_name = shdrTab[idx].sh_name;
      sh_type = shdrTab[idx].sh_type;
      sh_flags = shdrTab[idx].sh_flags;
      sh_offset = shdrTab[idx].sh_offset;
      sh_size = shdrTab[idx].sh_size;
      sh_link = shdrTab[idx].sh_link;
      sh_info = shdrTab[idx].sh_info;
      sh_addralign = shdrTab[idx].sh_addralign;
    } else {
      sh_name = mcld::bswap32(shdrTab[idx].sh_name);
      sh_type = mcld::bswap32(shdrTab[idx].sh_type);
      sh_flags = mcld::bswap64(shdrTab[idx].sh_flags);
      sh_offset = mcld::bswap64(shdrTab[idx].sh_offset);
      sh_size = mcld::bswap64(shdrTab[idx].sh_size);
      sh_link = mcld::bswap32(shdrTab[idx].sh_link);
      sh_info = mcld::bswap32(shdrTab[idx].sh_info);
      sh_addralign = mcld::bswap64(shdrTab[idx].sh_addralign);
    }

    LDSection* section = IRBuilder::CreateELFHeader(
        pInput, sect_name + sh_name, sh_type, sh_flags, sh_addralign);
    section->setSize(sh_size);
    section->setOffset(sh_offset);
    section->setInfo(sh_info);

    if (sh_link != 0x0 || sh_info != 0x0) {
      LinkInfo link_info = {section, sh_link, sh_info};
      link_info_list.push_back(link_info);
    }
  }  // end of for

  // set up InfoLink
  LinkInfoList::iterator info, infoEnd = link_info_list.end();
  for (info = link_info_list.begin(); info != infoEnd; ++info) {
    if (LDFileFormat::Relocation == info->section->kind())
      info->section->setLink(pInput.context()->getSection(info->sh_info));
    else
      info->section->setLink(pInput.context()->getSection(info->sh_link));
  }

  return true;
}

/// readSignature - read a symbol from the given Input and index in symtab
/// This is used to get the signature of a group section.
ResolveInfo* ELFReader<64, true>::readSignature(Input& pInput,
                                                LDSection& pSymTab,
                                                uint32_t pSymIdx) const {
  LDSection* symtab = &pSymTab;
  LDSection* strtab = symtab->getLink();
  assert(symtab != NULL && strtab != NULL);

  uint64_t offset = pInput.fileOffset() + symtab->offset() +
                    sizeof(llvm::ELF::Elf64_Sym) * pSymIdx;
  llvm::StringRef symbol_region =
      pInput.memArea()->request(offset, sizeof(llvm::ELF::Elf64_Sym));
  const llvm::ELF::Elf64_Sym* entry =
      reinterpret_cast<const llvm::ELF::Elf64_Sym*>(symbol_region.begin());

  uint32_t st_name = 0x0;
  uint8_t st_info = 0x0;
  uint8_t st_other = 0x0;
  uint16_t st_shndx = 0x0;
  st_info = entry->st_info;
  st_other = entry->st_other;
  if (llvm::sys::IsLittleEndianHost) {
    st_name = entry->st_name;
    st_shndx = entry->st_shndx;
  } else {
    st_name = mcld::bswap32(entry->st_name);
    st_shndx = mcld::bswap16(entry->st_shndx);
  }

  llvm::StringRef strtab_region = pInput.memArea()->request(
      pInput.fileOffset() + strtab->offset(), strtab->size());

  // get ld_name
  llvm::StringRef ld_name(strtab_region.begin() + st_name);

  ResolveInfo* result = ResolveInfo::Create(ld_name);
  result->setSource(pInput.type() == Input::DynObj);
  result->setType(static_cast<ResolveInfo::Type>(st_info & 0xF));
  result->setDesc(getSymDesc(st_shndx, pInput));
  result->setBinding(getSymBinding((st_info >> 4), st_shndx, st_other));
  result->setVisibility(getSymVisibility(st_other));

  return result;
}

/// readDynamic - read ELF .dynamic in input dynobj
bool ELFReader<64, true>::readDynamic(Input& pInput) const {
  assert(pInput.type() == Input::DynObj);
  const LDSection* dynamic_sect = pInput.context()->getSection(".dynamic");
  if (dynamic_sect == NULL) {
    fatal(diag::err_cannot_read_section) << ".dynamic";
  }
  const LDSection* dynstr_sect = dynamic_sect->getLink();
  if (dynstr_sect == NULL) {
    fatal(diag::err_cannot_read_section) << ".dynstr";
  }

  llvm::StringRef dynamic_region = pInput.memArea()->request(
      pInput.fileOffset() + dynamic_sect->offset(), dynamic_sect->size());

  llvm::StringRef dynstr_region = pInput.memArea()->request(
      pInput.fileOffset() + dynstr_sect->offset(), dynstr_sect->size());

  const llvm::ELF::Elf64_Dyn* dynamic =
      reinterpret_cast<const llvm::ELF::Elf64_Dyn*>(dynamic_region.begin());
  const char* dynstr = dynstr_region.begin();
  bool hasSOName = false;
  size_t numOfEntries = dynamic_sect->size() / sizeof(llvm::ELF::Elf64_Dyn);

  for (size_t idx = 0; idx < numOfEntries; ++idx) {
    llvm::ELF::Elf64_Sxword d_tag = 0x0;
    llvm::ELF::Elf64_Xword d_val = 0x0;

    if (llvm::sys::IsLittleEndianHost) {
      d_tag = dynamic[idx].d_tag;
      d_val = dynamic[idx].d_un.d_val;
    } else {
      d_tag = mcld::bswap64(dynamic[idx].d_tag);
      d_val = mcld::bswap64(dynamic[idx].d_un.d_val);
    }

    switch (d_tag) {
      case llvm::ELF::DT_SONAME:
        assert(d_val < dynstr_sect->size());
        pInput.setName(sys::fs::Path(dynstr + d_val).filename().native());
        hasSOName = true;
        break;
      case llvm::ELF::DT_NEEDED:
        // TODO:
        break;
      case llvm::ELF::DT_NULL:
      default:
        break;
    }
  }

  // if there is no SONAME in .dynamic, then set it from input path
  if (!hasSOName)
    pInput.setName(pInput.path().filename().native());

  return true;
}

}  // namespace mcld
