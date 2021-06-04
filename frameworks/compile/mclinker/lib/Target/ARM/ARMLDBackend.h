//===- ARMLDBackend.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMLDBACKEND_H_
#define TARGET_ARM_ARMLDBACKEND_H_

#include "ARMELFDynamic.h"
#include "ARMException.h"
#include "ARMGOT.h"
#include "ARMPLT.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Target/GNULDBackend.h"
#include "mcld/Target/OutputRelocSection.h"

#include <memory>

namespace mcld {

class ARMELFAttributeData;
class GNUInfo;
class LinkerConfig;

//===----------------------------------------------------------------------===//
/// ARMGNULDBackend - linker backend of ARM target of GNU ELF format
///
class ARMGNULDBackend : public GNULDBackend {
 public:
  // max branch offsets for ARM, THUMB, and THUMB2
  static const int32_t ARM_MAX_FWD_BRANCH_OFFSET = ((((1 << 23) - 1) << 2) + 8);
  static const int32_t ARM_MAX_BWD_BRANCH_OFFSET = ((-((1 << 23) << 2)) + 8);
  static const int32_t THM_MAX_FWD_BRANCH_OFFSET = ((1 << 22) - 2 + 4);
  static const int32_t THM_MAX_BWD_BRANCH_OFFSET = (-(1 << 22) + 4);
  static const int32_t THM2_MAX_FWD_BRANCH_OFFSET = (((1 << 24) - 2) + 4);
  static const int32_t THM2_MAX_BWD_BRANCH_OFFSET = (-(1 << 24) + 4);

 public:
  ARMGNULDBackend(const LinkerConfig& pConfig, GNUInfo* pInfo);
  ~ARMGNULDBackend();

 public:
  typedef std::vector<llvm::ELF::Elf32_Dyn*> ELF32DynList;

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
  ARMELFDynamic& dynamic();

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  const ARMELFDynamic& dynamic() const;

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

  ARMGOT& getGOT();
  const ARMGOT& getGOT() const;

  ARMPLT& getPLT();
  const ARMPLT& getPLT() const;

  OutputRelocSection& getRelDyn();
  const OutputRelocSection& getRelDyn() const;

  OutputRelocSection& getRelPLT();
  const OutputRelocSection& getRelPLT() const;

  ARMELFAttributeData& getAttributeData();
  const ARMELFAttributeData& getAttributeData() const;

  LDSymbol* getGOTSymbol() { return m_pGOTSymbol; }
  const LDSymbol* getGOTSymbol() const { return m_pGOTSymbol; }

  /// getTargetSectionOrder - compute the layout order of ARM target sections
  unsigned int getTargetSectionOrder(const LDSection& pSectHdr) const;

  /// finalizeTargetSymbols - finalize the symbol value
  bool finalizeTargetSymbols();

  /// preMergeSections - hooks to be executed before merging sections
  virtual void preMergeSections(Module& pModule);

  /// postMergeSections - hooks to be executed after merging sections
  virtual void postMergeSections(Module& pModule);

  /// mergeSection - merge target dependent sections
  bool mergeSection(Module& pModule, const Input& pInput, LDSection& pSection);

  /// setUpReachedSectionsForGC - set the reference from section XXX to
  /// .ARM.exidx.XXX to make sure GC correctly handle section exidx
  void setUpReachedSectionsForGC(
      const Module& pModule,
      GarbageCollection::SectionReachedListMap& pSectReachedListMap) const;

  /// readSection - read target dependent sections
  bool readSection(Input& pInput, SectionData& pSD);

  /// mayHaveUnsafeFunctionPointerAccess - check if the section may have unsafe
  /// function pointer access
  bool mayHaveUnsafeFunctionPointerAccess(const LDSection& pSection) const;

 private:
  void defineGOTSymbol(IRBuilder& pBuilder);

  /// maxFwdBranchOffset
  int64_t maxFwdBranchOffset() const;
  /// maxBwdBranchOffset
  int64_t maxBwdBranchOffset() const;

  /// mayRelax - Backends should override this function if they need relaxation
  bool mayRelax() { return true; }

  /// relax - the relaxation pass
  virtual bool relax(Module& pModule, IRBuilder& pBuilder);

  /// doRelax - Backend can orevride this function to add its relaxation
  /// implementation. Return true if the output (e.g., .text) is "relaxed"
  /// (i.e. layout is changed), and set pFinished to true if everything is fit,
  /// otherwise set it to false.
  bool doRelax(Module& pModule, IRBuilder& pBuilder, bool& pFinished);

  /// initTargetStubs
  bool initTargetStubs();

  /// getRelEntrySize - the size in BYTE of rel type relocation
  size_t getRelEntrySize() { return 8; }

  /// getRelEntrySize - the size in BYTE of rela type relocation
  size_t getRelaEntrySize() {
    assert(0 && "ARM backend with Rela type relocation\n");
    return 12;
  }

  /// doCreateProgramHdrs - backend can implement this function to create the
  /// target-dependent segments
  virtual void doCreateProgramHdrs(Module& pModule);

  /// rewriteExceptionSection - rewrite the output .ARM.exidx section.
  void rewriteARMExIdxSection(Module& pModule);

 private:
  Relocator* m_pRelocator;

  ARMGOT* m_pGOT;
  ARMPLT* m_pPLT;
  /// m_RelDyn - dynamic relocation table of .rel.dyn
  OutputRelocSection* m_pRelDyn;
  /// m_RelPLT - dynamic relocation table of .rel.plt
  OutputRelocSection* m_pRelPLT;

  /// m_pAttrData - attribute data in public ("aeabi") attribute subsection
  ARMELFAttributeData* m_pAttrData;

  ARMELFDynamic* m_pDynamic;
  LDSymbol* m_pGOTSymbol;
  LDSymbol* m_pEXIDXStart;
  LDSymbol* m_pEXIDXEnd;

  //     variable name           :  ELF
  LDSection* m_pEXIDX;       // .ARM.exidx
  LDSection* m_pEXTAB;       // .ARM.extab
  LDSection* m_pAttributes;  // .ARM.attributes
  //  LDSection* m_pPreemptMap;      // .ARM.preemptmap
  //  LDSection* m_pDebugOverlay;    // .ARM.debug_overlay
  //  LDSection* m_pOverlayTable;    // .ARM.overlay_table

  // m_pExData - exception handling section data structures
  std::unique_ptr<ARMExData> m_pExData;
};

}  // namespace mcld

#endif  // TARGET_ARM_ARMLDBACKEND_H_
