//===- ELFReader.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFReaderIf.h"

#include "mcld/IRBuilder.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFReaderIF
//===----------------------------------------------------------------------===//
/// getSymType
ResolveInfo::Type ELFReaderIF::getSymType(uint8_t pInfo,
                                          uint16_t pShndx) const {
  ResolveInfo::Type result = static_cast<ResolveInfo::Type>(pInfo & 0xF);
  if (pShndx == llvm::ELF::SHN_ABS && result == ResolveInfo::Section) {
    // In Mips, __gp_disp is a special section symbol. Its name comes from
    // .strtab, not .shstrtab. However, it is unique. Only it is also a ABS
    // symbol. So here is a tricky to identify __gp_disp and convert it to
    // Object symbol.
    return ResolveInfo::Object;
  }

  return result;
}

/// getSymDesc
ResolveInfo::Desc ELFReaderIF::getSymDesc(uint16_t pShndx,
                                          const Input& pInput) const {
  if (pShndx == llvm::ELF::SHN_UNDEF)
    return ResolveInfo::Undefined;

  if (pShndx < llvm::ELF::SHN_LORESERVE) {
    // an ELF symbol defined in a section which we are not including
    // must be treated as an Undefined.
    if (pInput.context()->getSection(pShndx) == NULL ||
        LDFileFormat::Ignore == pInput.context()->getSection(pShndx)->kind())
      return ResolveInfo::Undefined;
    return ResolveInfo::Define;
  }

  if (pShndx == llvm::ELF::SHN_ABS)
    return ResolveInfo::Define;

  if (pShndx == llvm::ELF::SHN_COMMON)
    return ResolveInfo::Common;

  if (pShndx >= llvm::ELF::SHN_LOPROC && pShndx <= llvm::ELF::SHN_HIPROC)
    return target().getSymDesc(pShndx);

  // FIXME: ELF weak alias should be ResolveInfo::Indirect
  return ResolveInfo::NoneDesc;
}

/// getSymBinding
ResolveInfo::Binding ELFReaderIF::getSymBinding(uint8_t pBinding,
                                                uint16_t pShndx,
                                                uint8_t pVis) const {
  // TODO:
  // if --just-symbols option is enabled, the symbol must covert to Absolute

  switch (pBinding) {
    case llvm::ELF::STB_LOCAL:
      return ResolveInfo::Local;
    case llvm::ELF::STB_GLOBAL:
      if (pShndx == llvm::ELF::SHN_ABS)
        return ResolveInfo::Absolute;
      return ResolveInfo::Global;
    case llvm::ELF::STB_WEAK:
      return ResolveInfo::Weak;
  }

  return ResolveInfo::NoneBinding;
}

/// getSymFragmentRef
FragmentRef* ELFReaderIF::getSymFragmentRef(Input& pInput,
                                            uint16_t pShndx,
                                            uint32_t pOffset) const {
  if (pInput.type() == Input::DynObj)
    return FragmentRef::Null();

  if (pShndx == llvm::ELF::SHN_UNDEF)
    return FragmentRef::Null();

  if (pShndx >= llvm::ELF::SHN_LORESERVE)  // including ABS and COMMON
    return FragmentRef::Null();

  LDSection* sect_hdr = pInput.context()->getSection(pShndx);

  if (sect_hdr == NULL)
    unreachable(diag::unreachable_invalid_section_idx)
        << pShndx << pInput.path().native();

  if (sect_hdr->kind() == LDFileFormat::Ignore)
    return FragmentRef::Null();

  if (sect_hdr->kind() == LDFileFormat::Group)
    return FragmentRef::Null();

  return FragmentRef::Create(*sect_hdr, pOffset);
}

/// getSymVisibility
ResolveInfo::Visibility ELFReaderIF::getSymVisibility(uint8_t pVis) const {
  return static_cast<ResolveInfo::Visibility>(pVis);
}

/// getSymValue - get the section offset of the symbol.
uint64_t ELFReaderIF::getSymValue(uint64_t pValue,
                                  uint16_t pShndx,
                                  const Input& pInput) const {
  if (pInput.type() == Input::Object) {
    // In relocatable files, st_value holds alignment constraints for a symbol
    // whose section index is SHN_COMMON
    if (pShndx == llvm::ELF::SHN_COMMON || pShndx == llvm::ELF::SHN_ABS) {
      return pValue;
    }

    // In relocatable files, st_value holds a section offset for a defined
    // symbol.
    // TODO:
    // if --just-symbols option are enabled, convert the value from section
    // offset
    // to virtual address by adding input section's virtual address.
    // The section's virtual address in relocatable files is normally zero, but
    // people can use link script to change it.
    return pValue;
  }

  // In executable and shared object files, st_value holds a virtual address.
  // the virtual address is needed for alias identification.
  return pValue;
}

}  // namespace mcld
