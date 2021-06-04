//===- MipsLDBackend.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSLDBACKEND_H_
#define TARGET_MIPS_MIPSLDBACKEND_H_
#include <llvm/Support/ELF.h>
#include "mcld/Target/GNULDBackend.h"
#include "MipsAbiFlags.h"
#include "MipsELFDynamic.h"
#include "MipsGOT.h"
#include "MipsGOTPLT.h"
#include "MipsPLT.h"

namespace mcld {

class LinkerConfig;
class MemoryArea;
class MipsGNUInfo;
class OutputRelocSection;
class SectionMap;

/** \class MipsGNULDBackend
 *  \brief Base linker backend of Mips target of GNU ELF format.
 */
class MipsGNULDBackend : public GNULDBackend {
 public:
  typedef std::vector<LDSymbol*> SymbolListType;

 public:
  MipsGNULDBackend(const LinkerConfig& pConfig, MipsGNUInfo* pInfo);
  ~MipsGNULDBackend();

  bool needsLA25Stub(Relocation::Type pType, const mcld::ResolveInfo* pSym);

  void addNonPICBranchSym(ResolveInfo* rsym);
  bool hasNonPICBranch(const ResolveInfo* rsym) const;

 public:
  /// initTargetSections - initialize target dependent sections in output
  void initTargetSections(Module& pModule, ObjectBuilder& pBuilder);

  /// initTargetSymbols - initialize target dependent symbols in output.
  void initTargetSymbols(IRBuilder& pBuilder, Module& pModule);

  /// getRelocator - return relocator.
  const Relocator* getRelocator() const;
  Relocator* getRelocator();

  /// preLayout - Backend can do any needed modification before layout
  void doPreLayout(IRBuilder& pBuilder);

  /// postLayout - Backend can do any needed modification after layout
  void doPostLayout(Module& pModule, IRBuilder& pBuilder);

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  MipsELFDynamic& dynamic();

  /// dynamic - the dynamic section of the target machine.
  /// Use co-variant return type to return its own dynamic section.
  const MipsELFDynamic& dynamic() const;

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
  /// @param pRegion - the region to write out data
  /// @return the size of the table in the file.
  uint64_t emitSectionData(const LDSection& pSection,
                           MemoryRegion& pRegion) const;

  /// hasEntryInStrTab - symbol has an entry in a .strtab
  bool hasEntryInStrTab(const LDSymbol& pSym) const;

  /// orderSymbolTable - order symbol table before emitting
  void orderSymbolTable(Module& pModule);

  /// readSection - read a target dependent section.
  bool readSection(Input& pInput, SectionData& pSD);

  MipsGOT& getGOT();
  const MipsGOT& getGOT() const;

  MipsPLT& getPLT();
  const MipsPLT& getPLT() const;

  MipsGOTPLT& getGOTPLT();
  const MipsGOTPLT& getGOTPLT() const;

  OutputRelocSection& getRelPLT();
  const OutputRelocSection& getRelPLT() const;

  OutputRelocSection& getRelDyn();
  const OutputRelocSection& getRelDyn() const;

  LDSymbol* getGOTSymbol() { return m_pGOTSymbol; }
  const LDSymbol* getGOTSymbol() const { return m_pGOTSymbol; }

  LDSymbol* getGpDispSymbol() { return m_pGpDispSymbol; }
  const LDSymbol* getGpDispSymbol() const { return m_pGpDispSymbol; }

  SymbolListType& getGlobalGOTSyms() { return m_GlobalGOTSyms; }
  const SymbolListType& getGlobalGOTSyms() const { return m_GlobalGOTSyms; }

  /// getTargetSectionOrder - compute the layout order of ARM target sections
  unsigned int getTargetSectionOrder(const LDSection& pSectHdr) const;

  /// finalizeSymbol - finalize the symbol value
  bool finalizeTargetSymbols();

  /// allocateCommonSymbols - allocate common symbols in the corresponding
  /// sections.
  bool allocateCommonSymbols(Module& pModule);

  /// getTPOffset - return TP_OFFSET against the SHF_TLS
  /// section in the specified input.
  uint64_t getTPOffset(const Input& pInput) const;

  /// getDTPOffset - return DTP_OFFSET against the SHF_TLS
  /// section in the specified input.
  uint64_t getDTPOffset(const Input& pInput) const;

  /// getGP0 - the gp value used to create the relocatable objects
  /// in the specified input.
  uint64_t getGP0(const Input& pInput) const;

 private:
  void defineGOTSymbol(IRBuilder& pBuilder);
  void defineGOTPLTSymbol(IRBuilder& pBuilder);

  bool relaxRelocation(IRBuilder& pBuilder, Relocation& pRel);

  /// emitSymbol32 - emit an ELF32 symbol, override parent's function
  void emitSymbol32(llvm::ELF::Elf32_Sym& pSym32,
                    LDSymbol& pSymbol,
                    char* pStrtab,
                    size_t pStrtabsize,
                    size_t pSymtabIdx);

  /// doCreateProgramHdrs - backend can implement this function to create the
  /// target-dependent segments
  void doCreateProgramHdrs(Module& pModule);

  /// mayRelax - Backends should override this function if they need relaxation
  bool mayRelax() { return true; }

  /// doRelax - Backend can orevride this function to add its relaxation
  /// implementation. Return true if the output (e.g., .text) is "relaxed"
  /// (i.e. layout is changed), and set pFinished to true if everything is fit,
  /// otherwise set it to false.
  bool doRelax(Module& pModule, IRBuilder& pBuilder, bool& pFinished);

  /// initTargetStubs
  bool initTargetStubs();

  /// readRelocation - read ELF32_Rel entry
  bool readRelocation(const llvm::ELF::Elf32_Rel& pRel,
                      Relocation::Type& pType,
                      uint32_t& pSymIdx,
                      uint32_t& pOffset) const;

  /// readRelocation - read ELF32_Rela entry
  bool readRelocation(const llvm::ELF::Elf32_Rela& pRel,
                      Relocation::Type& pType,
                      uint32_t& pSymIdx,
                      uint32_t& pOffset,
                      int32_t& pAddend) const;

  /// readRelocation - read ELF64_Rel entry
  bool readRelocation(const llvm::ELF::Elf64_Rel& pRel,
                      Relocation::Type& pType,
                      uint32_t& pSymIdx,
                      uint64_t& pOffset) const;

  /// readRel - read ELF64_Rela entry
  bool readRelocation(const llvm::ELF::Elf64_Rela& pRel,
                      Relocation::Type& pType,
                      uint32_t& pSymIdx,
                      uint64_t& pOffset,
                      int64_t& pAddend) const;

  /// emitRelocation - write data to the ELF32_Rel entry
  void emitRelocation(llvm::ELF::Elf32_Rel& pRel,
                      Relocation::Type pType,
                      uint32_t pSymIdx,
                      uint32_t pOffset) const;

  /// emitRelocation - write data to the ELF32_Rela entry
  void emitRelocation(llvm::ELF::Elf32_Rela& pRel,
                      Relocation::Type pType,
                      uint32_t pSymIdx,
                      uint32_t pOffset,
                      int32_t pAddend) const;

  /// emitRelocation - write data to the ELF64_Rel entry
  void emitRelocation(llvm::ELF::Elf64_Rel& pRel,
                      Relocation::Type pType,
                      uint32_t pSymIdx,
                      uint64_t pOffset) const;

  /// emitRelocation - write data to the ELF64_Rela entry
  void emitRelocation(llvm::ELF::Elf64_Rela& pRel,
                      Relocation::Type pType,
                      uint32_t pSymIdx,
                      uint64_t pOffset,
                      int64_t pAddend) const;

  /// preMergeSections - hooks to be executed before merging sections
  void preMergeSections(Module& pModule);

  /// mergeSection - merge target dependent sections
  bool mergeSection(Module& pModule, const Input& pInput, LDSection& pSection);

 protected:
  virtual void mergeFlags(Input& pInput, const char* ELF_hdr);

 private:
  typedef llvm::DenseSet<const ResolveInfo*> ResolveInfoSetType;
  typedef llvm::DenseMap<const Input*, llvm::ELF::Elf64_Addr> InputNumMapType;
  typedef llvm::DenseMap<const Input*, uint64_t> ElfFlagsMapType;

 protected:
  Relocator* m_pRelocator;
  MipsGOT* m_pGOT;        // .got
  MipsPLT* m_pPLT;        // .plt
  MipsGOTPLT* m_pGOTPLT;  // .got.plt

 private:
  MipsGNUInfo& m_pInfo;
  llvm::Optional<MipsAbiFlags> m_pAbiInfo;

  OutputRelocSection* m_pRelPlt;  // .rel.plt
  OutputRelocSection* m_pRelDyn;  // .rel.dyn

  MipsELFDynamic* m_pDynamic;
  LDSection* m_psdata;
  LDSection* m_pAbiFlags;
  LDSymbol* m_pGOTSymbol;
  LDSymbol* m_pPLTSymbol;
  LDSymbol* m_pGpDispSymbol;

  SymbolListType m_GlobalGOTSyms;
  ResolveInfoSetType m_HasNonPICBranchSyms;
  InputNumMapType m_GP0Map;
  InputNumMapType m_TpOffsetMap;
  InputNumMapType m_DtpOffsetMap;
  ElfFlagsMapType m_ElfFlagsMap;

  void moveSectionData(SectionData& pFrom, SectionData& pTo);
  void saveTPOffset(const Input& pInput);
};

/** \class Mips32GNULDBackend
 *  \brief Base linker backend of Mips 32-bit target of GNU ELF format.
 */
class Mips32GNULDBackend : public MipsGNULDBackend {
 public:
  Mips32GNULDBackend(const LinkerConfig& pConfig, MipsGNUInfo* pInfo);

 private:
  // MipsGNULDBackend

  bool initRelocator();
  void initTargetSections(Module& pModule, ObjectBuilder& pBuilder);
  size_t getRelEntrySize();
  size_t getRelaEntrySize();
};

/** \class Mips64GNULDBackend
 *  \brief Base linker backend of Mips 64-bit target of GNU ELF format.
 */
class Mips64GNULDBackend : public MipsGNULDBackend {
 public:
  Mips64GNULDBackend(const LinkerConfig& pConfig, MipsGNUInfo* pInfo);

 private:
  // MipsGNULDBackend

  bool initRelocator();
  void initTargetSections(Module& pModule, ObjectBuilder& pBuilder);
  size_t getRelEntrySize();
  size_t getRelaEntrySize();
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSLDBACKEND_H_
