//===- ELFDynObjReader.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFDynObjReader.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LD/ELFReader.h"
#include "mcld/LD/LDContext.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/ErrorHandling.h>

#include <string>

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFDynObjReader
//===----------------------------------------------------------------------===//
ELFDynObjReader::ELFDynObjReader(GNULDBackend& pBackend,
                                 IRBuilder& pBuilder,
                                 const LinkerConfig& pConfig)
    : DynObjReader(), m_pELFReader(0), m_Builder(pBuilder) {
  if (pConfig.targets().is32Bits() && pConfig.targets().isLittleEndian())
    m_pELFReader = new ELFReader<32, true>(pBackend);
  else if (pConfig.targets().is64Bits() && pConfig.targets().isLittleEndian())
    m_pELFReader = new ELFReader<64, true>(pBackend);
}

ELFDynObjReader::~ELFDynObjReader() {
  delete m_pELFReader;
}

/// isMyFormat
bool ELFDynObjReader::isMyFormat(Input& pInput, bool& pContinue) const {
  assert(pInput.hasMemArea());

  // Don't warning about the frequently requests.
  // MemoryArea has a list of cache to handle this.
  size_t hdr_size = m_pELFReader->getELFHeaderSize();
  if (pInput.memArea()->size() < hdr_size)
    return false;

  llvm::StringRef region =
      pInput.memArea()->request(pInput.fileOffset(), hdr_size);

  const char* ELF_hdr = region.begin();
  bool result = true;
  if (!m_pELFReader->isELF(ELF_hdr)) {
    pContinue = true;
    result = false;
  } else if (Input::DynObj != m_pELFReader->fileType(ELF_hdr)) {
    pContinue = true;
    result = false;
  } else if (!m_pELFReader->isMyEndian(ELF_hdr)) {
    pContinue = false;
    result = false;
  } else if (!m_pELFReader->isMyMachine(ELF_hdr)) {
    pContinue = false;
    result = false;
  }
  return result;
}

/// readHeader
bool ELFDynObjReader::readHeader(Input& pInput) {
  assert(pInput.hasMemArea());

  size_t hdr_size = m_pELFReader->getELFHeaderSize();
  llvm::StringRef region =
      pInput.memArea()->request(pInput.fileOffset(), hdr_size);
  const char* ELF_hdr = region.begin();

  bool shdr_result = m_pELFReader->readSectionHeaders(pInput, ELF_hdr);

  // read .dynamic to get the correct SONAME
  bool dyn_result = m_pELFReader->readDynamic(pInput);

  return (shdr_result && dyn_result);
}

/// readSymbols
bool ELFDynObjReader::readSymbols(Input& pInput) {
  assert(pInput.hasMemArea());

  LDSection* symtab_shdr = pInput.context()->getSection(".dynsym");
  if (symtab_shdr == NULL) {
    note(diag::note_has_no_symtab) << pInput.name() << pInput.path()
                                   << ".dynsym";
    return true;
  }

  LDSection* strtab_shdr = symtab_shdr->getLink();
  if (strtab_shdr == NULL) {
    fatal(diag::fatal_cannot_read_strtab) << pInput.name() << pInput.path()
                                          << ".dynsym";
    return false;
  }

  llvm::StringRef symtab_region = pInput.memArea()->request(
      pInput.fileOffset() + symtab_shdr->offset(), symtab_shdr->size());

  llvm::StringRef strtab_region = pInput.memArea()->request(
      pInput.fileOffset() + strtab_shdr->offset(), strtab_shdr->size());
  const char* strtab = strtab_region.begin();
  bool result =
      m_pELFReader->readSymbols(pInput, m_Builder, symtab_region, strtab);
  return result;
}

}  // namespace mcld
