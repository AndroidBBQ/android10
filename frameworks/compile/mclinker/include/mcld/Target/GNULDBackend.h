//===- GNULDBackend.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_GNULDBACKEND_H_
#define MCLD_TARGET_GNULDBACKEND_H_

#include "mcld/Module.h"
#include "mcld/LD/ELFBinaryReader.h"
#include "mcld/LD/ELFDynObjReader.h"
#include "mcld/LD/ELFObjectReader.h"
#include "mcld/LD/ELFObjectWriter.h"
#include "mcld/LD/GNUArchiveReader.h"
#include "mcld/Target/TargetLDBackend.h"

#include <llvm/Support/ELF.h>

#include <cstdint>

namespace mcld {

class BranchIslandFactory;
class EhFrameHdr;
class ELFAttribute;
class ELFDynamic;
class ELFDynObjFileFormat;
class ELFExecFileFormat;
class ELFFileFormat;
class ELFObjectFileFormat;
class ELFSegmentFactory;
class GNUInfo;
class IRBuilder;
class Layout;
class LinkerConfig;
class LinkerScript;
class Module;
class Relocation;
class StubFactory;

/** \class GNULDBackend
 *  \brief GNULDBackend provides a common interface for all GNU Unix-OS
 *  LDBackend.
 */
class GNULDBackend : public TargetLDBackend {
 protected:
  GNULDBackend(const LinkerConfig& pConfig, GNUInfo* pInfo);

 public:
  virtual ~GNULDBackend();

  // -----  readers/writers  ----- //
  GNUArchiveReader* createArchiveReader(Module& pModule);
  ELFObjectReader* createObjectReader(IRBuilder& pBuilder);
  ELFDynObjReader* createDynObjReader(IRBuilder& pBuilder);
  ELFBinaryReader* createBinaryReader(IRBuilder& pBuilder);
  ELFObjectWriter* createWriter();

  // -----  output sections  ----- //
  /// initStdSections - initialize standard sections of the output file.
  bool initStdSections(ObjectBuilder& pBuilder);

  /// getOutputFormat - get the sections of the output file.
  const ELFFileFormat* getOutputFormat() const;
  ELFFileFormat* getOutputFormat();

  // -----  target symbols ----- //
  /// initStandardSymbols - initialize standard symbols.
  /// Some section symbols is undefined in input object, and linkers must set
  /// up its value. Take __init_array_begin for example. This symbol is an
  /// undefined symbol in input objects. ObjectLinker must finalize its value
  /// to the begin of the .init_array section, then relocation enties to
  /// __init_array_begin can be applied without emission of "undefined
  /// reference to `__init_array_begin'".
  bool initStandardSymbols(IRBuilder& pBuilder, Module& pModule);

  /// finalizeSymbol - Linker checks pSymbol.reserved() if it's not zero,
  /// then it will ask backend to finalize the symbol value.
  /// @return ture - if backend set the symbol value sucessfully
  /// @return false - if backend do not recognize the symbol
  bool finalizeSymbols() {
    return (finalizeStandardSymbols() && finalizeTargetSymbols());
  }

  /// finalizeStandardSymbols - set the value of standard symbols
  virtual bool finalizeStandardSymbols();

  /// finalizeTargetSymbols - set the value of target symbols
  virtual bool finalizeTargetSymbols() = 0;

  /// finalizeTLSSymbol - set the value of a TLS symbol
  virtual bool finalizeTLSSymbol(LDSymbol& pSymbol);

  size_t sectionStartOffset() const;

  const GNUInfo& getInfo() const { return *m_pInfo; }
  GNUInfo& getInfo() { return *m_pInfo; }

  bool hasTextRel() const { return m_bHasTextRel; }

  bool hasStaticTLS() const { return m_bHasStaticTLS; }

  /// getSegmentStartAddr - return the start address of the segment
  uint64_t getSegmentStartAddr(const LinkerScript& pScript) const;

  /// sizeShstrtab - compute the size of .shstrtab
  void sizeShstrtab(Module& pModule);

  /// sizeNamePools - compute the size of regular name pools
  /// In ELF executable files, regular name pools are .symtab, .strtab.,
  /// .dynsym, .dynstr, and .hash
  virtual void sizeNamePools(Module& pModule);

  /// emitSectionData - emit target-dependent section data
  virtual uint64_t emitSectionData(const LDSection& pSection,
                                   MemoryRegion& pRegion) const = 0;

  /// emitRegNamePools - emit regular name pools - .symtab, .strtab
  virtual void emitRegNamePools(const Module& pModule,
                                FileOutputBuffer& pOutput);

  /// emitNamePools - emit dynamic name pools - .dyntab, .dynstr, .hash
  virtual void emitDynNamePools(Module& pModule, FileOutputBuffer& pOutput);

  /// emitELFHashTab - emit .hash
  virtual void emitELFHashTab(const Module::SymbolTable& pSymtab,
                              FileOutputBuffer& pOutput);

  /// emitGNUHashTab - emit .gnu.hash
  virtual void emitGNUHashTab(Module::SymbolTable& pSymtab,
                              FileOutputBuffer& pOutput);

  /// sizeInterp - compute the size of program interpreter's name
  /// In ELF executables, this is the length of dynamic linker's path name
  virtual void sizeInterp();

  /// emitInterp - emit the .interp
  virtual void emitInterp(FileOutputBuffer& pOutput);

  /// hasEntryInStrTab - symbol has an entry in a .strtab
  virtual bool hasEntryInStrTab(const LDSymbol& pSym) const;

  /// orderSymbolTable - order symbol table before emitting
  virtual void orderSymbolTable(Module& pModule);

  void setHasStaticTLS(bool pVal = true) { m_bHasStaticTLS = pVal; }

  /// getSectionOrder - compute the layout order of the section
  /// Layout calls this function to get the default order of the pSectHdr.
  /// If the pSectHdr.type() is LDFileFormat::Target, then getSectionOrder()
  /// will call getTargetSectionOrder().
  ///
  /// If targets favors certain order for general sections, please override
  /// this function.
  ///
  /// @see getTargetSectionOrder
  virtual unsigned int getSectionOrder(const LDSection& pSectHdr) const;

  /// getTargetSectionOrder - compute the layout order of target section
  /// If the target favors certain order for the given gSectHdr, please
  /// override this function.
  ///
  /// By default, this function returns the maximun order, and pSectHdr
  /// will be the last section to be laid out.
  virtual unsigned int getTargetSectionOrder(const LDSection& pSectHdr) const {
    return (unsigned int)-1;
  }

  /// elfSegmentTable - return the reference of the elf segment table
  ELFSegmentFactory& elfSegmentTable();

  /// elfSegmentTable - return the reference of the elf segment table
  const ELFSegmentFactory& elfSegmentTable() const;

  /// commonPageSize - the common page size of the target machine
  uint64_t commonPageSize() const;

  /// abiPageSize - the abi page size of the target machine
  uint64_t abiPageSize() const;

  /// getSymbolIdx - get the symbol index of ouput symbol table
  size_t getSymbolIdx(const LDSymbol* pSymbol) const;

  /// allocateCommonSymbols - allocate common symbols in the corresponding
  /// sections.
  /// Different concrete target backend may overlap this function.
  virtual bool allocateCommonSymbols(Module& pModule);

  /// mergeFlags - update set of ELF header flags
  virtual void mergeFlags(Input& pInput, const char* ELF_hdr) {}

  /// updateSectionFlags - update pTo's flags when merging pFrom
  /// update the output section flags based on input section flags.
  virtual bool updateSectionFlags(LDSection& pTo, const LDSection& pFrom);

  /// readRelocation - read ELF32_Rel entry
  virtual bool readRelocation(const llvm::ELF::Elf32_Rel& pRel,
                              uint32_t& pType,
                              uint32_t& pSymIdx,
                              uint32_t& pOffset) const;

  /// readRelocation - read ELF32_Rela entry
  virtual bool readRelocation(const llvm::ELF::Elf32_Rela& pRel,
                              uint32_t& pType,
                              uint32_t& pSymIdx,
                              uint32_t& pOffset,
                              int32_t& pAddend) const;

  /// readRelocation - read ELF64_Rel entry
  virtual bool readRelocation(const llvm::ELF::Elf64_Rel& pRel,
                              uint32_t& pType,
                              uint32_t& pSymIdx,
                              uint64_t& pOffset) const;

  /// readRel - read ELF64_Rela entry
  virtual bool readRelocation(const llvm::ELF::Elf64_Rela& pRel,
                              uint32_t& pType,
                              uint32_t& pSymIdx,
                              uint64_t& pOffset,
                              int64_t& pAddend) const;

  /// emitRelocation - write data to the ELF32_Rel entry
  virtual void emitRelocation(llvm::ELF::Elf32_Rel& pRel,
                              uint32_t pType,
                              uint32_t pSymIdx,
                              uint32_t pOffset) const;

  /// emitRelocation - write data to the ELF32_Rela entry
  virtual void emitRelocation(llvm::ELF::Elf32_Rela& pRel,
                              uint32_t pType,
                              uint32_t pSymIdx,
                              uint32_t pOffset,
                              int32_t pAddend) const;

  /// emitRelocation - write data to the ELF64_Rel entry
  virtual void emitRelocation(llvm::ELF::Elf64_Rel& pRel,
                              uint32_t pType,
                              uint32_t pSymIdx,
                              uint64_t pOffset) const;

  /// emitRelocation - write data to the ELF64_Rela entry
  virtual void emitRelocation(llvm::ELF::Elf64_Rela& pRel,
                              uint32_t pType,
                              uint32_t pSymIdx,
                              uint64_t pOffset,
                              int64_t pAddend) const;

  /// symbolNeedsPLT - return whether the symbol needs a PLT entry
  bool symbolNeedsPLT(const ResolveInfo& pSym) const;

  /// symbolNeedsCopyReloc - return whether the symbol needs a copy relocation
  bool symbolNeedsCopyReloc(const Relocation& pReloc,
                            const ResolveInfo& pSym) const;

  /// symbolNeedsDynRel - return whether the symbol needs a dynamic relocation
  bool symbolNeedsDynRel(const ResolveInfo& pSym,
                         bool pSymHasPLT,
                         bool isAbsReloc) const;

  /// isSymbolPreemptible - whether the symbol can be preemted by other link
  /// units
  bool isSymbolPreemptible(const ResolveInfo& pSym) const;

  /// symbolHasFinalValue - return true if the symbol's value can be decided at
  /// link time
  bool symbolFinalValueIsKnown(const ResolveInfo& pSym) const;

  /// isDynamicSymbol
  bool isDynamicSymbol(const LDSymbol& pSymbol) const;

  /// isDynamicSymbol
  bool isDynamicSymbol(const ResolveInfo& pResolveInfo) const;

  virtual ResolveInfo::Desc getSymDesc(uint16_t pShndx) const {
    return ResolveInfo::Define;
  }

  bool hasTDATASymbol() const { return (f_pTDATA != NULL); }
  bool hasTBSSSymbol() const { return (f_pTBSS != NULL); }

  void setTDATASymbol(LDSymbol& pTDATA) { f_pTDATA = &pTDATA; }
  void setTBSSSymbol(LDSymbol& pTBSS) { f_pTBSS = &pTBSS; }

  // getTDATASymbol - get section symbol of .tdata
  LDSymbol& getTDATASymbol();
  const LDSymbol& getTDATASymbol() const;

  /// getTBSSSymbol - get section symbol of .tbss
  LDSymbol& getTBSSSymbol();
  const LDSymbol& getTBSSSymbol() const;

  /// getEntry - get the entry point name
  llvm::StringRef getEntry(const Module& pModule) const;

  //  -----  relaxation  -----  //
  /// initBRIslandFactory - initialize the branch island factory for relaxation
  bool initBRIslandFactory();

  /// initStubFactory - initialize the stub factory for relaxation
  bool initStubFactory();

  /// getBRIslandFactory
  BranchIslandFactory* getBRIslandFactory() { return m_pBRIslandFactory; }

  /// getStubFactory
  StubFactory* getStubFactory() { return m_pStubFactory; }

  /// maxFwdBranchOffset - return the max forward branch offset of the backend.
  /// Target can override this function if needed.
  virtual int64_t maxFwdBranchOffset() const { return INT64_MAX; }

  /// maxBwdBranchOffset - return the max backward branch offset of the backend.
  /// Target can override this function if needed.
  virtual int64_t maxBwdBranchOffset() const { return 0; }

  /// stubGroupSize - return the group size to place stubs between sections.
  virtual unsigned stubGroupSize() const;

  /// checkAndSetHasTextRel - check pSection flag to set HasTextRel
  void checkAndSetHasTextRel(const LDSection& pSection);

  /// sortRelocation - sort the dynamic relocations to let dynamic linker
  /// process relocations more efficiently
  void sortRelocation(LDSection& pSection);

  /// createAndSizeEhFrameHdr - This is seperated since we may add eh_frame
  /// entry in the middle
  void createAndSizeEhFrameHdr(Module& pModule);

  /// attribute - the attribute section data.
  ELFAttribute& attribute() { return *m_pAttribute; }

  /// attribute - the attribute section data.
  const ELFAttribute& attribute() const { return *m_pAttribute; }

  /// mayHaveUnsafeFunctionPointerAccess - check if the section may have unsafe
  /// function pointer access
  bool mayHaveUnsafeFunctionPointerAccess(const LDSection& pSection) const;

 protected:
  /// getRelEntrySize - the size in BYTE of rel type relocation
  virtual size_t getRelEntrySize() = 0;

  /// getRelEntrySize - the size in BYTE of rela type relocation
  virtual size_t getRelaEntrySize() = 0;

  uint64_t getSymbolSize(const LDSymbol& pSymbol) const;

  uint64_t getSymbolInfo(const LDSymbol& pSymbol) const;

  uint64_t getSymbolValue(const LDSymbol& pSymbol) const;

  uint64_t getSymbolShndx(const LDSymbol& pSymbol) const;

  /// isTemporary - Whether pSymbol is a local label.
  virtual bool isTemporary(const LDSymbol& pSymbol) const;

  /// getHashBucketCount - calculate hash bucket count.
  static unsigned getHashBucketCount(unsigned pNumOfSymbols, bool pIsGNUStyle);

  /// getGNUHashMaskbitslog2 - calculate the number of mask bits in log2
  unsigned getGNUHashMaskbitslog2(unsigned pNumOfSymbols) const;

  /// emitSymbol32 - emit an ELF32 symbol
  void emitSymbol32(llvm::ELF::Elf32_Sym& pSym32,
                    LDSymbol& pSymbol,
                    char* pStrtab,
                    size_t pStrtabsize,
                    size_t pSymtabIdx);

  /// emitSymbol64 - emit an ELF64 symbol
  void emitSymbol64(llvm::ELF::Elf64_Sym& pSym64,
                    LDSymbol& pSymbol,
                    char* pStrtab,
                    size_t pStrtabsize,
                    size_t pSymtabIdx);

 protected:
  /// createProgramHdrs - base on output sections to create the program headers
  void createProgramHdrs(Module& pModule);

  /// doCreateProgramHdrs - backend can implement this function to create the
  /// target-dependent segments
  virtual void doCreateProgramHdrs(Module& pModule) = 0;

  /// setupProgramHdrs - set up the attributes of segments
  ///  (i.e., offset, addresses, file/mem size, flag,  and alignment)
  void setupProgramHdrs(const LinkerScript& pScript);

  /// getSegmentFlag - give a section flag and return the corresponding segment
  /// flag
  inline uint32_t getSegmentFlag(const uint32_t pSectionFlag);

  /// setupGNUStackInfo - setup the section flag of .note.GNU-stack in output
  void setupGNUStackInfo(Module& pModule);

  /// setOutputSectionOffset - helper function to set output sections' offset.
  void setOutputSectionOffset(Module& pModule);

  /// setOutputSectionAddress - helper function to set output sections' address.
  void setOutputSectionAddress(Module& pModule);

  /// placeOutputSections - place output sections based on SectionMap
  void placeOutputSections(Module& pModule);

  /// layout - layout method
  void layout(Module& pModule);

  /// preLayout - Backend can do any needed modification before layout
  void preLayout(Module& pModule, IRBuilder& pBuilder);

  /// postLayout -Backend can do any needed modification after layout
  void postLayout(Module& pModule, IRBuilder& pBuilder);

  /// preLayout - Backend can do any needed modification before layout
  virtual void doPreLayout(IRBuilder& pBuilder) = 0;

  /// postLayout -Backend can do any needed modification after layout
  virtual void doPostLayout(Module& pModule, IRBuilder& pLinker) = 0;

  /// postProcessing - Backend can do any needed modification in the final stage
  void postProcessing(FileOutputBuffer& pOutput);

  /// dynamic - the dynamic section of the target machine.
  virtual ELFDynamic& dynamic() = 0;

  /// dynamic - the dynamic section of the target machine.
  virtual const ELFDynamic& dynamic() const = 0;

  /// relax - the relaxation pass
  virtual bool relax(Module& pModule, IRBuilder& pBuilder);

  /// mayRelax - Backends should override this function if they need relaxation
  virtual bool mayRelax() { return false; }

  /// doRelax - Backend can orevride this function to add its relaxation
  /// implementation. Return true if the output (e.g., .text) is "relaxed"
  /// (i.e. layout is changed), and set pFinished to true if everything is fit,
  /// otherwise set it to false.
  virtual bool doRelax(Module& pModule, IRBuilder& pBuilder, bool& pFinished) {
    return false;
  }

 protected:
  // Based on Kind in LDFileFormat to define basic section orders for ELF.
  enum SectionOrder {
    SHO_NULL = 0,         // NULL
    SHO_INTERP,           // .interp
    SHO_RO_NOTE,          // .note.ABI-tag, .note.gnu.build-id
    SHO_NAMEPOOL,         // *.hash, .dynsym, .dynstr
    SHO_RELOCATION,       // .rel.*, .rela.*
    SHO_REL_PLT,          // .rel.plt should come after other .rel.*
    SHO_INIT,             // .init
    SHO_PLT,              // .plt
    SHO_TEXT,             // .text
    SHO_FINI,             // .fini
    SHO_RO,               // .rodata
    SHO_EXCEPTION,        // .eh_frame_hdr, .eh_frame, .gcc_except_table
    SHO_TLS_DATA,         // .tdata
    SHO_TLS_BSS,          // .tbss
    SHO_RELRO_LOCAL,      // .data.rel.ro.local
    SHO_RELRO,            // .data.rel.ro,
    SHO_RELRO_LAST,       // for x86 to adjust .got if needed
    SHO_NON_RELRO_FIRST,  // for x86 to adjust .got.plt if needed
    SHO_DATA,             // .data
    SHO_LARGE_DATA,       // .ldata
    SHO_RW_NOTE,          //
    SHO_SMALL_DATA,       // .sdata
    SHO_SMALL_BSS,        // .sbss
    SHO_BSS,              // .bss
    SHO_LARGE_BSS,        // .lbss
    SHO_UNDEFINED,        // default order
    SHO_STRTAB            // .strtab
  };

  // for -z combreloc
  struct RelocCompare {
    explicit RelocCompare(const GNULDBackend& pBackend) : m_Backend(pBackend) {}
    bool operator()(const Relocation& X, const Relocation& Y) const;

   private:
    const GNULDBackend& m_Backend;
  };

  // for gnu style hash table
  struct DynsymCompare {
    bool needGNUHash(const LDSymbol& X) const;

    bool operator()(const LDSymbol* X, const LDSymbol* Y) const;
  };

  struct SymCompare {
    bool operator()(const LDSymbol* X, const LDSymbol* Y) const {
      return (X == Y);
    }
  };

  struct SymPtrHash {
    size_t operator()(const LDSymbol* pKey) const {
      return (unsigned((uintptr_t)pKey) >> 4) ^
             (unsigned((uintptr_t)pKey) >> 9);
    }
  };

  typedef HashEntry<LDSymbol*, size_t, SymCompare> SymHashEntryType;
  typedef HashTable<SymHashEntryType,
                    SymPtrHash,
                    EntryFactory<SymHashEntryType> > HashTableType;

 protected:
  ELFObjectReader* m_pObjectReader;

  // -----  file formats  ----- //
  ELFDynObjFileFormat* m_pDynObjFileFormat;
  ELFExecFileFormat* m_pExecFileFormat;
  ELFObjectFileFormat* m_pObjectFileFormat;

  // GNUInfo
  GNUInfo* m_pInfo;

  // ELF segment factory
  ELFSegmentFactory* m_pELFSegmentTable;

  // branch island factory
  BranchIslandFactory* m_pBRIslandFactory;

  // stub factory
  StubFactory* m_pStubFactory;

  // map the LDSymbol to its index in the output symbol table
  HashTableType* m_pSymIndexMap;

  // section .eh_frame_hdr
  EhFrameHdr* m_pEhFrameHdr;

  // attribute section
  ELFAttribute* m_pAttribute;

  // ----- dynamic flags ----- //
  // DF_TEXTREL of DT_FLAGS
  bool m_bHasTextRel;

  // DF_STATIC_TLS of DT_FLAGS
  bool m_bHasStaticTLS;

  // -----  standard symbols  ----- //
  // section symbols
  LDSymbol* f_pPreInitArrayStart;
  LDSymbol* f_pPreInitArrayEnd;
  LDSymbol* f_pInitArrayStart;
  LDSymbol* f_pInitArrayEnd;
  LDSymbol* f_pFiniArrayStart;
  LDSymbol* f_pFiniArrayEnd;
  LDSymbol* f_pStack;
  LDSymbol* f_pDynamic;

  // section symbols for .tdata and .tbss
  LDSymbol* f_pTDATA;
  LDSymbol* f_pTBSS;

  // segment symbols
  LDSymbol* f_pExecutableStart;
  LDSymbol* f_pEText;
  LDSymbol* f_p_EText;
  LDSymbol* f_p__EText;
  LDSymbol* f_pEData;
  LDSymbol* f_p_EData;
  LDSymbol* f_pBSSStart;
  LDSymbol* f_pEnd;
  LDSymbol* f_p_End;
};

}  // namespace mcld

#endif  // MCLD_TARGET_GNULDBACKEND_H_
