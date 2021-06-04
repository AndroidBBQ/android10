//===- LDSection.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_LDSECTION_H_
#define MCLD_LD_LDSECTION_H_

#include "mcld/Config/Config.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/Support/Allocators.h"

#include <llvm/Support/DataTypes.h>

#include <string>

namespace mcld {

class DebugString;
class EhFrame;
class RelocData;
class SectionData;

/** \class LDSection
 *  \brief LDSection represents a section header entry. It is a unified
 *  abstraction of a section header entry for various file formats.
 */
class LDSection {
 private:
  friend class Chunk<LDSection, MCLD_SECTIONS_PER_INPUT>;

  LDSection();

  LDSection(const std::string& pName,
            LDFileFormat::Kind pKind,
            uint32_t pType,
            uint32_t pFlag,
            uint64_t pSize = 0,
            uint64_t pAddr = 0);

 public:
  ~LDSection();

  static LDSection* Create(const std::string& pName,
                           LDFileFormat::Kind pKind,
                           uint32_t pType,
                           uint32_t pFlag,
                           uint64_t pSize = 0,
                           uint64_t pAddr = 0);

  static void Destroy(LDSection*& pSection);

  static void Clear();

  bool hasOffset() const;

  /// name - the name of this section.
  const std::string& name() const { return m_Name; }

  /// kind - the kind of this section, such as Text, BSS, GOT, and so on.
  /// from LDFileFormat::Kind
  LDFileFormat::Kind kind() const { return m_Kind; }

  /// type - The categorizes the section's contents and semantics. It's
  /// different from llvm::SectionKind. Type is format-dependent, but
  /// llvm::SectionKind is format independent and is used for bit-code.
  ///   In ELF, it is sh_type
  ///   In MachO, it's type field of struct section::flags
  uint32_t type() const { return m_Type; }

  /// flag - An integer describes miscellaneous attributes.
  ///   In ELF, it is sh_flags.
  ///   In MachO, it's attribute field of struct section::flags
  uint32_t flag() const { return m_Flag; }

  /// size - An integer specifying the size in bytes of the virtual memory
  /// occupied by this section.
  ///   In ELF, if the type() is SHT_NOBITS, this function return zero.
  ///   Before layouting, output's LDSection::size() should return zero.
  uint64_t size() const { return m_Size; }

  /// offset - An integer specifying the offset of this section in the file.
  ///   Before layouting, output's LDSection::offset() should return zero.
  uint64_t offset() const { return m_Offset; }

  /// addr - An integer specifying the virtual address of this section in the
  /// virtual image.
  ///   Before layouting, output's LDSection::offset() should return zero.
  ///   ELF uses sh_addralign to set alignment constraints. In LLVM, alignment
  ///   constraint is set in SectionData::setAlignment. addr() contains the
  ///   original ELF::sh_addr. Modulo sh_addr by sh_addralign is not necessary.
  ///   MachO uses the same scenario.
  ///
  ///   Because addr() in output is changing during linking, MCLinker does not
  ///   store the address of the output here. The address is in Layout
  uint64_t addr() const { return m_Addr; }

  /// align - An integer specifying the align of this section in the file.
  ///   Before layouting, output's LDSection::align() should return zero.
  uint32_t align() const { return m_Align; }

  size_t index() const { return m_Index; }

  /// getLink - return the Link. When a section A needs the other section B
  /// during linking or loading, we say B is A's Link section.
  /// In ELF, InfoLink section control the ElfNN_Shdr::sh_link and sh_info.
  ///
  /// @return if the section needs no other sections, return NULL
  LDSection* getLink() { return m_pLink; }

  const LDSection* getLink() const { return m_pLink; }

  size_t getInfo() const { return m_Info; }

  void setKind(LDFileFormat::Kind pKind) { m_Kind = pKind; }

  void setSize(uint64_t size) { m_Size = size; }

  void setOffset(uint64_t Offset) { m_Offset = Offset; }

  void setAddr(uint64_t addr) { m_Addr = addr; }

  void setAlign(uint32_t align) { m_Align = align; }

  void setFlag(uint32_t flag) { m_Flag = flag; }

  void setType(uint32_t type) { m_Type = type; }

  // -----  SectionData  ----- //
  const SectionData* getSectionData() const { return m_Data.sect_data; }
  SectionData* getSectionData() { return m_Data.sect_data; }

  void setSectionData(SectionData* pSD) { m_Data.sect_data = pSD; }

  bool hasSectionData() const;

  // ------  RelocData  ------ //
  const RelocData* getRelocData() const { return m_Data.reloc_data; }
  RelocData* getRelocData() { return m_Data.reloc_data; }

  void setRelocData(RelocData* pRD) { m_Data.reloc_data = pRD; }

  bool hasRelocData() const;

  // ------  EhFrame  ------ //
  const EhFrame* getEhFrame() const { return m_Data.eh_frame; }
  EhFrame* getEhFrame() { return m_Data.eh_frame; }

  void setEhFrame(EhFrame* pEhFrame) { m_Data.eh_frame = pEhFrame; }

  bool hasEhFrame() const;

  // ------  DebugString  ------ //
  const DebugString* getDebugString() const { return m_Data.debug_string; }
  DebugString*       getDebugString()       { return m_Data.debug_string; }

  void setDebugString(DebugString* pDebugString)
  { m_Data.debug_string = pDebugString; }

  bool hasDebugString() const;

  /// setLink - set the sections should link with.
  /// if pLink is NULL, no Link section is set.
  void setLink(LDSection* pLink) { m_pLink = pLink; }

  void setInfo(size_t pInfo) { m_Info = pInfo; }

  void setIndex(size_t pIndex) { m_Index = pIndex; }

 private:
  union Data {
    SectionData* sect_data;
    RelocData*   reloc_data;
    EhFrame*     eh_frame;
    DebugString* debug_string;
  };

 private:
  std::string m_Name;

  LDFileFormat::Kind m_Kind;
  uint32_t m_Type;
  uint32_t m_Flag;

  uint64_t m_Size;
  uint64_t m_Offset;
  uint64_t m_Addr;
  uint32_t m_Align;

  size_t m_Info;
  LDSection* m_pLink;

  /// m_Data - the SectionData or RelocData of this section
  Data m_Data;

  /// m_Index - the index of the file
  size_t m_Index;
};  // end of LDSection

}  // namespace mcld

#endif  // MCLD_LD_LDSECTION_H_
