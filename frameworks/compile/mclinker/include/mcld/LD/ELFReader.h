//===- ELFReader.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFREADER_H_
#define MCLD_LD_ELFREADER_H_

#include "mcld/LD/ELFReaderIf.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>

namespace mcld {

class IRBuilder;
class LDSection;
class SectionData;

/** \class ELFReader
 *  \brief ELFReader is a template scaffolding for partial specification.
 */
template <size_t BIT, bool LITTLEENDIAN>
class ELFReader {};

/** \class ELFReader<32, true>
 *  \brief ELFReader<32, true> is a 32-bit, little endian ELFReader.
 */
template <>
class ELFReader<32, true> : public ELFReaderIF {
 public:
  typedef llvm::ELF::Elf32_Ehdr ELFHeader;
  typedef llvm::ELF::Elf32_Shdr SectionHeader;
  typedef llvm::ELF::Elf32_Sym Symbol;
  typedef llvm::ELF::Elf32_Rel Rel;
  typedef llvm::ELF::Elf32_Rela Rela;

 public:
  explicit ELFReader(GNULDBackend& pBackend);

  ~ELFReader();

  /// ELFHeaderSize - return the size of the ELFHeader
  size_t getELFHeaderSize() const { return sizeof(ELFHeader); }

  /// isELF - is this a ELF file
  bool isELF(const void* pELFHeader) const;

  /// isMyEndian - is this ELF file in the same endian to me?
  bool isMyEndian(const void* pELFHeader) const;

  /// isMyMachine - is this ELF file generated for the same machine.
  bool isMyMachine(const void* pELFHeader) const;

  /// fileType - the file type of this file
  Input::Type fileType(const void* pELFHeader) const;

  /// readSectionHeaders - read ELF section header table and create LDSections
  bool readSectionHeaders(Input& pInput, const void* pELFHeader) const;

  /// readRegularSection - read a regular section and create fragments.
  bool readRegularSection(Input& pInput, SectionData& pSD) const;

  /// readSymbols - read ELF symbols and create LDSymbol
  bool readSymbols(Input& pInput,
                   IRBuilder& pBuilder,
                   llvm::StringRef pRegion,
                   const char* StrTab) const;

  /// readSignature - read a symbol from the given Input and index in symtab
  /// This is used to get the signature of a group section.
  ResolveInfo* readSignature(Input& pInput,
                             LDSection& pSymTab,
                             uint32_t pSymIdx) const;

  /// readRela - read ELF rela and create Relocation
  bool readRela(Input& pInput,
                LDSection& pSection,
                llvm::StringRef pRegion) const;

  /// readRel - read ELF rel and create Relocation
  bool readRel(Input& pInput,
               LDSection& pSection,
               llvm::StringRef pRegion) const;

  /// readDynamic - read ELF .dynamic in input dynobj
  bool readDynamic(Input& pInput) const;

 private:
  struct AliasInfo {
    LDSymbol* pt_alias;  /// potential alias
    uint64_t ld_value;
    ResolveInfo::Binding ld_binding;
  };

  /// comparison function to sort symbols for analyzing weak alias.
  /// sort symbols by symbol value and then weak before strong.
  static bool less(AliasInfo p1, AliasInfo p2) {
    if (p1.ld_value != p2.ld_value)
      return (p1.ld_value < p2.ld_value);
    if (p1.ld_binding != p2.ld_binding) {
      if (ResolveInfo::Weak == p1.ld_binding)
        return true;
      else if (ResolveInfo::Weak == p2.ld_binding)
        return false;
    }
    return p1.pt_alias->str() < p2.pt_alias->str();
  }
};

/** \class ELFReader<64, true>
 *  \brief ELFReader<64, true> is a 64-bit, little endian ELFReader.
 */
template <>
class ELFReader<64, true> : public ELFReaderIF {
 public:
  typedef llvm::ELF::Elf64_Ehdr ELFHeader;
  typedef llvm::ELF::Elf64_Shdr SectionHeader;
  typedef llvm::ELF::Elf64_Sym Symbol;
  typedef llvm::ELF::Elf64_Rel Rel;
  typedef llvm::ELF::Elf64_Rela Rela;

 public:
  explicit ELFReader(GNULDBackend& pBackend);

  ~ELFReader();

  /// ELFHeaderSize - return the size of the ELFHeader
  size_t getELFHeaderSize() const { return sizeof(ELFHeader); }

  /// isELF - is this a ELF file
  bool isELF(const void* pELFHeader) const;

  /// isMyEndian - is this ELF file in the same endian to me?
  bool isMyEndian(const void* pELFHeader) const;

  /// isMyMachine - is this ELF file generated for the same machine.
  bool isMyMachine(const void* pELFHeader) const;

  /// fileType - the file type of this file
  Input::Type fileType(const void* pELFHeader) const;

  /// readSectionHeaders - read ELF section header table and create LDSections
  bool readSectionHeaders(Input& pInput, const void* pELFHeader) const;

  /// readRegularSection - read a regular section and create fragments.
  bool readRegularSection(Input& pInput, SectionData& pSD) const;

  /// readSymbols - read ELF symbols and create LDSymbol
  bool readSymbols(Input& pInput,
                   IRBuilder& pBuilder,
                   llvm::StringRef pRegion,
                   const char* StrTab) const;

  /// readSignature - read a symbol from the given Input and index in symtab
  /// This is used to get the signature of a group section.
  ResolveInfo* readSignature(Input& pInput,
                             LDSection& pSymTab,
                             uint32_t pSymIdx) const;

  /// readRela - read ELF rela and create Relocation
  bool readRela(Input& pInput,
                LDSection& pSection,
                llvm::StringRef pRegion) const;

  /// readRel - read ELF rel and create Relocation
  bool readRel(Input& pInput,
               LDSection& pSection,
               llvm::StringRef pRegion) const;

  /// readDynamic - read ELF .dynamic in input dynobj
  bool readDynamic(Input& pInput) const;

 private:
  struct AliasInfo {
    LDSymbol* pt_alias;  /// potential alias
    uint64_t ld_value;
    ResolveInfo::Binding ld_binding;
  };

  /// comparison function to sort symbols for analyzing weak alias.
  /// sort symbols by symbol value and then weak before strong.
  static bool less(AliasInfo p1, AliasInfo p2) {
    if (p1.ld_value != p2.ld_value)
      return (p1.ld_value < p2.ld_value);
    if (p1.ld_binding != p2.ld_binding) {
      if (ResolveInfo::Weak == p1.ld_binding)
        return true;
      else if (ResolveInfo::Weak == p2.ld_binding)
        return false;
    }
    return p1.pt_alias->str() < p2.pt_alias->str();
  }
};

}  // namespace mcld

#endif  // MCLD_LD_ELFREADER_H_
