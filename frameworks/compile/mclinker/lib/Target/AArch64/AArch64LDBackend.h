//===- AArch64LDBackend.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64LDBACKEND_H_
#define TARGET_AARCH64_AARCH64LDBACKEND_H_

#include "AArch64ELFDynamic.h"
#include "AArch64GOT.h"
#include "AArch64PLT.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Target/GNULDBackend.h"
#include "mcld/Target/OutputRelocSection.h"

namespace mcld {

class LinkerConfig;
class GNUInfo;

//===----------------------------------------------------------------------===//
/// AArch64GNULDBackend - linker backend of AArch64 target of GNU ELF format
///
class AArch64GNULDBackend : public GNULDBackend {
 public:
  static constexpr int64_t MAX_FWD_BRANCH_OFFSET = (((1 << 25) - 1) << 2);
  static constexpr int64_t MAX_BWD_BRANCH_OFFSET = (-((1 << 25) << 2));

  static constexpr int64_t MAX_ADRP_IMM = (1 << 20) - 1;
  static constexpr int64_t MIN_ADRP_IMM = -(1 << 20);

 public:
  AArch64GNULDBackend(const LinkerConfig& pConfig, GNUInfo* pInfo);
  ~AArch64GNULDBackend();

 public:
  /// initTargetSections - initialize target dependent sections in output.
  void initTargetSections(Module& pModule, ObjectBuilder& pBuilder);

  /// initTargetSymbols - initialize target dependent symbols in output.
  void initTargetSymbols(IRBuilder& pBuilder, Module& pModule);

  /// initRelocator - create and initialize Relocator.
  bool initRelocator();

  /// getRelocator - return relocator.
  const Relocator* getRelocator() const;
  Relocator* getRelocator();

  /// doPreLayout - Backend can do any needed modification before layout
  void doPreLayout(IRBuilder& pBuilder);

  /// doPostLayout -Backend can do any needed modification after layout
  void doPostLayout(Module& pModule, IRBuilder& pBuilder);

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  AArch64ELFDynamic& dynamic();

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  const AArch64ELFDynamic& dynamic() const;

  /// emitSectionData - write out the section data into the memory region.
  /// When writers get a LDSection whose kind is LDFileFormat::Target, writers
  /// call back target backend to emit the data.
  ///
  /// Backends handle the target-special tables (plt, gp,...) by themselves.
  /// Backend can put the data of the tables in SectionData directly
  ///  - LDSection.getSectionData can get the section data.
  /// Or, backend can put the data into special data structure
  ///  - backend can maintain its own map<LDSection, table> to get the table
  /// from given LDSection.
  ///
  /// @param pSection - the given LDSection
  /// @param pConfig - all options in the command line.
  /// @param pRegion - the region to write out data
  /// @return the size of the table in the file.
  uint64_t emitSectionData(const LDSection& pSection,
                           MemoryRegion& pRegion) const;

  AArch64GOT& getGOT();
  const AArch64GOT& getGOT() const;

  AArch64GOT& getGOTPLT();
  const AArch64GOT& getGOTPLT() const;

  AArch64PLT& getPLT();
  const AArch64PLT& getPLT() const;

  OutputRelocSection& getRelaDyn();
  const OutputRelocSection& getRelaDyn() const;

  OutputRelocSection& getRelaPLT();
  const OutputRelocSection& getRelaPLT() const;

  LDSymbol* getGOTSymbol() { return m_pGOTSymbol; }
  const LDSymbol* getGOTSymbol() const { return m_pGOTSymbol; }

  /// getTargetSectionOrder - compute the layout order of AArch64 target
  /// sections
  unsigned int getTargetSectionOrder(const LDSection& pSectHdr) const;

  /// finalizeTargetSymbols - finalize the symbol value
  bool finalizeTargetSymbols();

  /// mergeSection - merge target dependent sections
  bool mergeSection(Module& pModule, const Input& pInput, LDSection& pSection);

  /// readSection - read target dependent sections
  bool readSection(Input& pInput, SectionData& pSD);

 private:
  void defineGOTSymbol(IRBuilder& pBuilder);

  int64_t maxFwdBranchOffset() const { return MAX_FWD_BRANCH_OFFSET; }
  int64_t maxBwdBranchOffset() const { return MAX_BWD_BRANCH_OFFSET; }

  void scanErrata(Module& pModule,
                  IRBuilder& pBuilder,
                  size_t& num_new_stubs,
                  size_t& stubs_strlen);

  /// mayRelax - Backends should override this function if they need relaxation
  bool mayRelax() { return true; }

  /// doRelax - Backend can orevride this function to add its relaxation
  /// implementation. Return true if the output (e.g., .text) is "relaxed"
  /// (i.e. layout is changed), and set pFinished to true if everything is fit,
  /// otherwise set it to false.
  bool doRelax(Module& pModule, IRBuilder& pBuilder, bool& pFinished);

  /// initTargetStubs
  bool initTargetStubs();

  /// getRelEntrySize - the size in BYTE of rel type relocation
  size_t getRelEntrySize() { return 16; }

  /// getRelEntrySize - the size in BYTE of rela type relocation
  size_t getRelaEntrySize() { return 24; }

  /// doCreateProgramHdrs - backend can implement this function to create the
  /// target-dependent segments
  virtual void doCreateProgramHdrs(Module& pModule);

 private:
  Relocator* m_pRelocator;

  AArch64GOT* m_pGOT;
  AArch64GOT* m_pGOTPLT;
  AArch64PLT* m_pPLT;
  /// m_RelDyn - dynamic relocation table of .rel.dyn
  OutputRelocSection* m_pRelaDyn;
  /// m_RelPLT - dynamic relocation table of .rel.plt
  OutputRelocSection* m_pRelaPLT;

  /// m_pAttrData - attribute data in public ("aeabi") attribute subsection
  // AArch64ELFAttributeData* m_pAttrData;

  AArch64ELFDynamic* m_pDynamic;
  LDSymbol* m_pGOTSymbol;

  //     variable name           :  ELF
  // LDSection* m_pAttributes;      // .ARM.attributes
  // LDSection* m_pPreemptMap;      // .AArch64.preemptmap
  // LDSection* m_pDebugOverlay;    // .AArch64.debug_overlay
  // LDSection* m_pOverlayTable;    // .AArch64.overlay_table
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64LDBACKEND_H_
