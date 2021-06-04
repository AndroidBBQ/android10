//===- ELFDynamic.cpp -------------    ------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/ELFDynamic.h"
#include "mcld/Target/GNULDBackend.h"
#include "mcld/LinkerConfig.h"

#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Host.h>

namespace mcld {
namespace elf_dynamic {

//===----------------------------------------------------------------------===//
// elf_dynamic::EntryIF
//===----------------------------------------------------------------------===//
EntryIF::EntryIF() {
}

EntryIF::~EntryIF() {
}

}  // namespace elf_dynamic

//===----------------------------------------------------------------------===//
// ELFDynamic
//===----------------------------------------------------------------------===//
ELFDynamic::ELFDynamic(const GNULDBackend& pParent, const LinkerConfig& pConfig)
    : m_pEntryFactory(NULL), m_Backend(pParent), m_Config(pConfig), m_Idx(0) {
  // FIXME: support big-endian machine.
  if (m_Config.targets().is32Bits()) {
    if (m_Config.targets().isLittleEndian())
      m_pEntryFactory = new elf_dynamic::Entry<32, true>();
  } else if (m_Config.targets().is64Bits()) {
    if (m_Config.targets().isLittleEndian())
      m_pEntryFactory = new elf_dynamic::Entry<64, true>();
  } else {
    fatal(diag::unsupported_bitclass) << m_Config.targets().triple().str()
                                      << m_Config.targets().bitclass();
  }
}

ELFDynamic::~ELFDynamic() {
  if (m_pEntryFactory != NULL)
    delete m_pEntryFactory;

  EntryListType::iterator entry, entryEnd = m_EntryList.end();
  for (entry = m_EntryList.begin(); entry != entryEnd; ++entry) {
    if (*entry != NULL)
      delete (*entry);
  }

  entryEnd = m_NeedList.end();
  for (entry = m_NeedList.begin(); entry != entryEnd; ++entry) {
    if (*entry != NULL)
      delete (*entry);
  }
}

size_t ELFDynamic::size() const {
  return (m_NeedList.size() + m_EntryList.size());
}

size_t ELFDynamic::numOfBytes() const {
  return size() * entrySize();
}

size_t ELFDynamic::entrySize() const {
  return m_pEntryFactory->size();
}

void ELFDynamic::reserveOne(uint64_t pTag) {
  assert(m_pEntryFactory != NULL);
  m_EntryList.push_back(m_pEntryFactory->clone());
}

void ELFDynamic::applyOne(uint64_t pTag, uint64_t pValue) {
  assert(m_Idx < m_EntryList.size());
  m_EntryList[m_Idx]->setValue(pTag, pValue);
  ++m_Idx;
}

/// reserveEntries - reserve entries
void ELFDynamic::reserveEntries(const ELFFileFormat& pFormat) {
  if (LinkerConfig::DynObj == m_Config.codeGenType()) {
    reserveOne(llvm::ELF::DT_SONAME);

    if (m_Config.options().Bsymbolic())
      reserveOne(llvm::ELF::DT_SYMBOLIC);
  }

  if (pFormat.hasInit())
    reserveOne(llvm::ELF::DT_INIT);

  if (pFormat.hasFini())
    reserveOne(llvm::ELF::DT_FINI);

  if (pFormat.hasPreInitArray()) {
    reserveOne(llvm::ELF::DT_PREINIT_ARRAY);
    reserveOne(llvm::ELF::DT_PREINIT_ARRAYSZ);
  }

  if (pFormat.hasInitArray()) {
    reserveOne(llvm::ELF::DT_INIT_ARRAY);
    reserveOne(llvm::ELF::DT_INIT_ARRAYSZ);
  }

  if (pFormat.hasFiniArray()) {
    reserveOne(llvm::ELF::DT_FINI_ARRAY);
    reserveOne(llvm::ELF::DT_FINI_ARRAYSZ);
  }

  if (pFormat.hasHashTab())
    reserveOne(llvm::ELF::DT_HASH);

  if (pFormat.hasGNUHashTab())
    reserveOne(llvm::ELF::DT_GNU_HASH);

  if (pFormat.hasDynSymTab()) {
    reserveOne(llvm::ELF::DT_SYMTAB);
    reserveOne(llvm::ELF::DT_SYMENT);
  }

  if (pFormat.hasDynStrTab()) {
    reserveOne(llvm::ELF::DT_STRTAB);
    reserveOne(llvm::ELF::DT_STRSZ);
  }

  reserveTargetEntries(pFormat);

  if (pFormat.hasRelPlt() || pFormat.hasRelaPlt()) {
    reserveOne(llvm::ELF::DT_PLTREL);
    reserveOne(llvm::ELF::DT_JMPREL);
    reserveOne(llvm::ELF::DT_PLTRELSZ);
  }

  if (pFormat.hasRelDyn()) {
    reserveOne(llvm::ELF::DT_REL);
    reserveOne(llvm::ELF::DT_RELSZ);
    reserveOne(llvm::ELF::DT_RELENT);
  }

  if (pFormat.hasRelaDyn()) {
    reserveOne(llvm::ELF::DT_RELA);
    reserveOne(llvm::ELF::DT_RELASZ);
    reserveOne(llvm::ELF::DT_RELAENT);
  }

  uint64_t dt_flags = 0x0;
  if (m_Config.options().hasOrigin())
    dt_flags |= llvm::ELF::DF_ORIGIN;
  if (m_Config.options().Bsymbolic())
    dt_flags |= llvm::ELF::DF_SYMBOLIC;
  if (m_Config.options().hasNow())
    dt_flags |= llvm::ELF::DF_BIND_NOW;
  if (m_Backend.hasTextRel())
    dt_flags |= llvm::ELF::DF_TEXTREL;
  if (m_Backend.hasStaticTLS() &&
      (LinkerConfig::DynObj == m_Config.codeGenType()))
    dt_flags |= llvm::ELF::DF_STATIC_TLS;

  if ((m_Config.options().hasNewDTags() && dt_flags != 0x0) ||
      (dt_flags & llvm::ELF::DF_STATIC_TLS) != 0x0)
    reserveOne(llvm::ELF::DT_FLAGS);

  if (m_Backend.hasTextRel())
    reserveOne(llvm::ELF::DT_TEXTREL);

  if (m_Config.options().hasNow() || m_Config.options().hasLoadFltr() ||
      m_Config.options().hasOrigin() || m_Config.options().hasInterPose() ||
      m_Config.options().hasNoDefaultLib() || m_Config.options().hasNoDump() ||
      m_Config.options().Bgroup() ||
      ((LinkerConfig::DynObj == m_Config.codeGenType()) &&
       (m_Config.options().hasNoDelete() || m_Config.options().hasInitFirst() ||
        m_Config.options().hasNoDLOpen()))) {
    reserveOne(llvm::ELF::DT_FLAGS_1);
  }

  unsigned num_spare_dtags = m_Config.options().getNumSpareDTags();
  for (unsigned i = 0; i < num_spare_dtags; ++i) {
    reserveOne(llvm::ELF::DT_NULL);
  }
}

/// applyEntries - apply entries
void ELFDynamic::applyEntries(const ELFFileFormat& pFormat) {
  if (LinkerConfig::DynObj == m_Config.codeGenType() &&
      m_Config.options().Bsymbolic()) {
    applyOne(llvm::ELF::DT_SYMBOLIC, 0x0);
  }

  if (pFormat.hasInit())
    applyOne(llvm::ELF::DT_INIT, pFormat.getInit().addr());

  if (pFormat.hasFini())
    applyOne(llvm::ELF::DT_FINI, pFormat.getFini().addr());

  if (pFormat.hasPreInitArray()) {
    applyOne(llvm::ELF::DT_PREINIT_ARRAY, pFormat.getPreInitArray().addr());
    applyOne(llvm::ELF::DT_PREINIT_ARRAYSZ, pFormat.getPreInitArray().size());
  }

  if (pFormat.hasInitArray()) {
    applyOne(llvm::ELF::DT_INIT_ARRAY, pFormat.getInitArray().addr());
    applyOne(llvm::ELF::DT_INIT_ARRAYSZ, pFormat.getInitArray().size());
  }

  if (pFormat.hasFiniArray()) {
    applyOne(llvm::ELF::DT_FINI_ARRAY, pFormat.getFiniArray().addr());
    applyOne(llvm::ELF::DT_FINI_ARRAYSZ, pFormat.getFiniArray().size());
  }

  if (pFormat.hasHashTab())
    applyOne(llvm::ELF::DT_HASH, pFormat.getHashTab().addr());

  if (pFormat.hasGNUHashTab())
    applyOne(llvm::ELF::DT_GNU_HASH, pFormat.getGNUHashTab().addr());

  if (pFormat.hasDynSymTab()) {
    applyOne(llvm::ELF::DT_SYMTAB, pFormat.getDynSymTab().addr());
    applyOne(llvm::ELF::DT_SYMENT, symbolSize());
  }

  if (pFormat.hasDynStrTab()) {
    applyOne(llvm::ELF::DT_STRTAB, pFormat.getDynStrTab().addr());
    applyOne(llvm::ELF::DT_STRSZ, pFormat.getDynStrTab().size());
  }

  applyTargetEntries(pFormat);

  if (pFormat.hasRelPlt()) {
    applyOne(llvm::ELF::DT_PLTREL, llvm::ELF::DT_REL);
    applyOne(llvm::ELF::DT_JMPREL, pFormat.getRelPlt().addr());
    applyOne(llvm::ELF::DT_PLTRELSZ, pFormat.getRelPlt().size());
  } else if (pFormat.hasRelaPlt()) {
    applyOne(llvm::ELF::DT_PLTREL, llvm::ELF::DT_RELA);
    applyOne(llvm::ELF::DT_JMPREL, pFormat.getRelaPlt().addr());
    applyOne(llvm::ELF::DT_PLTRELSZ, pFormat.getRelaPlt().size());
  }

  if (pFormat.hasRelDyn()) {
    applyOne(llvm::ELF::DT_REL, pFormat.getRelDyn().addr());
    applyOne(llvm::ELF::DT_RELSZ, pFormat.getRelDyn().size());
    applyOne(llvm::ELF::DT_RELENT, m_pEntryFactory->relSize());
  }

  if (pFormat.hasRelaDyn()) {
    applyOne(llvm::ELF::DT_RELA, pFormat.getRelaDyn().addr());
    applyOne(llvm::ELF::DT_RELASZ, pFormat.getRelaDyn().size());
    applyOne(llvm::ELF::DT_RELAENT, m_pEntryFactory->relaSize());
  }

  if (m_Backend.hasTextRel()) {
    applyOne(llvm::ELF::DT_TEXTREL, 0x0);

    if (m_Config.options().warnSharedTextrel() &&
        LinkerConfig::DynObj == m_Config.codeGenType())
      mcld::warning(mcld::diag::warn_shared_textrel);
  }

  uint64_t dt_flags = 0x0;
  if (m_Config.options().hasOrigin())
    dt_flags |= llvm::ELF::DF_ORIGIN;
  if (m_Config.options().Bsymbolic())
    dt_flags |= llvm::ELF::DF_SYMBOLIC;
  if (m_Config.options().hasNow())
    dt_flags |= llvm::ELF::DF_BIND_NOW;
  if (m_Backend.hasTextRel())
    dt_flags |= llvm::ELF::DF_TEXTREL;
  if (m_Backend.hasStaticTLS() &&
      (LinkerConfig::DynObj == m_Config.codeGenType()))
    dt_flags |= llvm::ELF::DF_STATIC_TLS;

  if ((m_Config.options().hasNewDTags() && dt_flags != 0x0) ||
      (dt_flags & llvm::ELF::DF_STATIC_TLS) != 0)
    applyOne(llvm::ELF::DT_FLAGS, dt_flags);

  uint64_t dt_flags_1 = 0x0;
  if (m_Config.options().hasNow())
    dt_flags_1 |= llvm::ELF::DF_1_NOW;
  if (m_Config.options().hasLoadFltr())
    dt_flags_1 |= llvm::ELF::DF_1_LOADFLTR;
  if (m_Config.options().hasOrigin())
    dt_flags_1 |= llvm::ELF::DF_1_ORIGIN;
  if (m_Config.options().hasInterPose())
    dt_flags_1 |= llvm::ELF::DF_1_INTERPOSE;
  if (m_Config.options().hasNoDefaultLib())
    dt_flags_1 |= llvm::ELF::DF_1_NODEFLIB;
  if (m_Config.options().hasNoDump())
    dt_flags_1 |= llvm::ELF::DF_1_NODUMP;
  if (m_Config.options().Bgroup())
    dt_flags_1 |= llvm::ELF::DF_1_GROUP;
  if (LinkerConfig::DynObj == m_Config.codeGenType()) {
    if (m_Config.options().hasNoDelete())
      dt_flags_1 |= llvm::ELF::DF_1_NODELETE;
    if (m_Config.options().hasInitFirst())
      dt_flags_1 |= llvm::ELF::DF_1_INITFIRST;
    if (m_Config.options().hasNoDLOpen())
      dt_flags_1 |= llvm::ELF::DF_1_NOOPEN;
  }
  if (dt_flags_1 != 0x0)
    applyOne(llvm::ELF::DT_FLAGS_1, dt_flags_1);

  unsigned num_spare_dtags = m_Config.options().getNumSpareDTags();
  for (unsigned i = 0; i < num_spare_dtags; ++i) {
    applyOne(llvm::ELF::DT_NULL, 0x0);
  }
}

/// symbolSize
size_t ELFDynamic::symbolSize() const {
  return m_pEntryFactory->symbolSize();
}

/// reserveNeedEntry - reserve on DT_NEED entry.
void ELFDynamic::reserveNeedEntry() {
  m_NeedList.push_back(m_pEntryFactory->clone());
}

/// emit
void ELFDynamic::emit(const LDSection& pSection, MemoryRegion& pRegion) const {
  if (pRegion.size() < pSection.size()) {
    llvm::report_fatal_error(llvm::Twine("the given memory is smaller") +
                             llvm::Twine(" than the section's demaind.\n"));
  }

  uint8_t* address = reinterpret_cast<uint8_t*>(pRegion.begin());
  EntryListType::const_iterator entry, entryEnd = m_NeedList.end();
  for (entry = m_NeedList.begin(); entry != entryEnd; ++entry)
    address += (*entry)->emit(address);

  entryEnd = m_EntryList.end();
  for (entry = m_EntryList.begin(); entry != entryEnd; ++entry)
    address += (*entry)->emit(address);
}

void ELFDynamic::applySoname(uint64_t pStrTabIdx) {
  applyOne(llvm::ELF::DT_SONAME, pStrTabIdx);
}

}  // namespace mcld
