//===- HexagonLDBackend.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONLDBACKEND_H_
#define TARGET_HEXAGON_HEXAGONLDBACKEND_H_

#include "HexagonELFDynamic.h"
#include "HexagonGOT.h"
#include "HexagonPLT.h"
#include "HexagonGOTPLT.h"
#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Target/GNULDBackend.h"
#include "mcld/Target/OutputRelocSection.h"

namespace mcld {

class HexagonGNUInfo;
class LinkerConfig;

//===----------------------------------------------------------------------===//
/// HexagonLDBackend - linker backend of Hexagon target of GNU ELF format
///
class HexagonLDBackend : public GNULDBackend {
 public:
  HexagonLDBackend(const LinkerConfig& pConfig, HexagonGNUInfo* pInfo);

  ~HexagonLDBackend();

  uint32_t machine() const;

  HexagonGOT& getGOT();

  const HexagonGOT& getGOT() const;

  HexagonPLT& getPLT();

  const HexagonPLT& getPLT() const;

  /// preLayout - Backend can do any needed modification before layout
  void doPreLayout(IRBuilder& pBuilder);

  bool allocateCommonSymbols(Module& pModule);

  /// postLayout - Backend can do any needed modification after layout
  void doPostLayout(Module& pModule, IRBuilder& pBuilder);

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  HexagonELFDynamic& dynamic();

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  const HexagonELFDynamic& dynamic() const;

  /// emitSectionData - write out the section data into the memory region.
  /// When writers get a LDSection whose kind is LDFileFormat::Target, writers
  /// call back target backend to emit the data.
  ///
  /// Backends handle the target-special tables (plt, gp,...) by themselves.
  /// Backend can put the data of the tables in MCSectionData directly
  ///  - LDSection.getSectionData can get the section data.
  /// Or, backend can put the data into special data structure
  ///  - backend can maintain its own map<LDSection, table> to get the table
  /// from given LDSection.
  ///
  /// @param pSection - the given LDSection
  /// @param pLayout - for comouting the size of fragment
  /// @param pRegion - the region to write out data
  /// @return the size of the table in the file.
  uint64_t emitSectionData(const LDSection& pSection,
                           MemoryRegion& pRegion) const;

  /// initRelocator - create and initialize Relocator.
  bool initRelocator();

  /// getRelocator - return relocator.
  const Relocator* getRelocator() const;
  Relocator* getRelocator();

  ResolveInfo::Desc getSymDesc(uint16_t shndx) const {
    if (shndx >= llvm::ELF::SHN_HEXAGON_SCOMMON &&
        shndx <= llvm::ELF::SHN_HEXAGON_SCOMMON_8)
      return ResolveInfo::Common;
    return ResolveInfo::NoneDesc;
  }

  void initTargetSections(Module& pModule, ObjectBuilder& pBuilder);

  void initTargetSymbols(IRBuilder& pBuilder, Module& pModule);

  bool initBRIslandFactory();

  bool initStubFactory();

  bool mayRelax() { return true; }

  bool doRelax(Module& pModule, IRBuilder& pBuilder, bool& pFinished);

  bool initTargetStubs();

  OutputRelocSection& getRelaDyn();

  const OutputRelocSection& getRelaDyn() const;

  HexagonGOTPLT& getGOTPLT();

  const HexagonGOTPLT& getGOTPLT() const;

  OutputRelocSection& getRelaPLT();

  const OutputRelocSection& getRelaPLT() const;

  /// getTargetSectionOrder - compute the layout order of Hexagon target section
  unsigned int getTargetSectionOrder(const LDSection& pSectHdr) const;

  /// finalizeTargetSymbols - finalize the symbol value
  bool finalizeTargetSymbols();

  /// mergeSection - merge target dependent sections
  bool mergeSection(Module& pModule, const Input& pInput, LDSection& pSection);

  /// readSection - read target dependent sections
  bool readSection(Input& pInput, SectionData& pSD);

  bool MoveCommonData(SectionData& pFrom, SectionData& pTo);

  bool MoveSectionDataAndSort(SectionData& pFrom, SectionData& pTo);

  bool SetSDataSection();

  uint32_t getGP() { return m_psdata->addr(); }

  Relocation::Type getCopyRelType() const { return m_CopyRel; }

  virtual uint32_t getGOTSymbolAddr() { return m_pGOTSymbol->value(); }

 protected:
  void defineGOTSymbol(IRBuilder& pBuilder, Fragment&);

 private:
  /// getRelEntrySize - the size in BYTE of rela type relocation
  size_t getRelEntrySize() { return 0; }

  /// getRelaEntrySize - the size in BYTE of rela type relocation
  size_t getRelaEntrySize() { return 12; }

  /// doCreateProgramHdrs - backend can implement this function to create the
  /// target-dependent segments
  void doCreateProgramHdrs(Module& pModule);

  /// maxFwdBranchOffset
  int64_t maxFwdBranchOffset() const { return ~(~0U << 6); }

  virtual void setGOTSectionSize(IRBuilder& pBuilder);

  virtual uint64_t emitGOTSectionData(MemoryRegion& pRegion) const;

  virtual uint64_t emitGOTPLTSectionData(MemoryRegion& pRegion,
                                         const ELFFileFormat* FileFormat) const;

  virtual void setRelaDynSize();
  virtual void setRelaPLTSize();

 private:
  Relocator* m_pRelocator;
  HexagonGOT* m_pGOT;
  HexagonGOTPLT* m_pGOTPLT;
  HexagonPLT* m_pPLT;
  /// m_RelaDyn - dynamic relocation table of .rela.dyn
  OutputRelocSection* m_pRelaDyn;
  /// m_RelaPLT - dynamic relocation table of .rela.plt
  OutputRelocSection* m_pRelaPLT;

  HexagonELFDynamic* m_pDynamic;

  LDSection* m_psdata;
  LDSection* m_pscommon_1;
  LDSection* m_pscommon_2;
  LDSection* m_pscommon_4;
  LDSection* m_pscommon_8;
  LDSection* m_pstart;
  LDSymbol* m_psdabase;

  LDSymbol* m_pGOTSymbol;
  LDSymbol* m_pBSSEnd;
  Relocation::Type m_CopyRel;
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONLDBACKEND_H_
