//===- ELFReaderIf.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFREADERIF_H_
#define MCLD_LD_ELFREADERIF_H_

#include "mcld/LinkerConfig.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

namespace mcld {

class IRBuilder;
class FragmentRef;
class LDSection;
class SectionData;

/** \class ELFReaderIF
 *  \brief ELFReaderIF provides common interface for all kind of ELF readers.
 */
class ELFReaderIF {
 public:
  explicit ELFReaderIF(GNULDBackend& pBackend) : m_Backend(pBackend) {}

  virtual ~ELFReaderIF() {}

  /// ELFHeaderSize - return the size of the ELFHeader
  virtual size_t getELFHeaderSize() const = 0;

  /// isELF - is this a ELF file
  virtual bool isELF(const void* pELFHeader) const = 0;

  /// isMyEndian - is this ELF file in the same endian to me?
  virtual bool isMyEndian(const void* pELFHeader) const = 0;

  /// isMyMachine - is this ELF file generated for the same machine.
  virtual bool isMyMachine(const void* pELFHeader) const = 0;

  /// fileType - the file type of this file
  virtual Input::Type fileType(const void* pELFHeader) const = 0;

  /// target - the target backend
  const GNULDBackend& target() const { return m_Backend; }
  GNULDBackend& target() { return m_Backend; }

  /// readSectionHeaders - read ELF section header table and create LDSections
  virtual bool readSectionHeaders(Input& pInput,
                                  const void* pELFHeader) const = 0;

  /// readRegularSection - read a regular section and create fragments.
  virtual bool readRegularSection(Input& pInput, SectionData& pSD) const = 0;

  /// readSymbols - read ELF symbols and create LDSymbol
  virtual bool readSymbols(Input& pInput,
                           IRBuilder& pBuilder,
                           llvm::StringRef pRegion,
                           const char* StrTab) const = 0;

  /// readSignature - read a symbol from the given Input and index in symtab
  /// This is used to get the signature of a group section.
  virtual ResolveInfo* readSignature(Input& pInput,
                                     LDSection& pSymTab,
                                     uint32_t pSymIdx) const = 0;

  /// readRela - read ELF rela and create Relocation
  virtual bool readRela(Input& pInput,
                        LDSection& pSection,
                        llvm::StringRef pRegion) const = 0;

  /// readRel - read ELF rel and create Relocation
  virtual bool readRel(Input& pInput,
                       LDSection& pSection,
                       llvm::StringRef pRegion) const = 0;

  /// readDynamic - read ELF .dynamic in input dynobj
  virtual bool readDynamic(Input& pInput) const = 0;

 protected:
  /// LinkInfo - some section needs sh_link and sh_info, remember them.
  struct LinkInfo {
    LDSection* section;
    uint32_t sh_link;
    uint32_t sh_info;
  };

  typedef std::vector<LinkInfo> LinkInfoList;

 protected:
  ResolveInfo::Type getSymType(uint8_t pInfo, uint16_t pShndx) const;

  ResolveInfo::Desc getSymDesc(uint16_t pShndx, const Input& pInput) const;

  ResolveInfo::Binding getSymBinding(uint8_t pBinding,
                                     uint16_t pShndx,
                                     uint8_t pVisibility) const;

  uint64_t getSymValue(uint64_t pValue,
                       uint16_t pShndx,
                       const Input& pInput) const;

  FragmentRef* getSymFragmentRef(Input& pInput,
                                 uint16_t pShndx,
                                 uint32_t pOffset) const;

  ResolveInfo::Visibility getSymVisibility(uint8_t pVis) const;

 protected:
  GNULDBackend& m_Backend;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFREADERIF_H_
