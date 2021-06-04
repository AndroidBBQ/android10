//===-- TargetLDBackend.h - Target LD Backend -------------------*- C++ -*-===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_TARGETLDBACKEND_H_
#define MCLD_TARGET_TARGETLDBACKEND_H_
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/GarbageCollection.h"
#include "mcld/Support/Compiler.h"
#include "mcld/Support/GCFactoryListTraits.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ilist.h>
#include <llvm/Support/DataTypes.h>

namespace mcld {

class ArchiveReader;
class BinaryReader;
class BinaryWriter;
class BranchIslandFactory;
class DynObjReader;
class DynObjWriter;
class ExecWriter;
class FileOutputBuffer;
class IRBuilder;
class Input;
class LDSection;
class LDSymbol;
class LinkerConfig;
class Module;
class ObjectBuilder;
class ObjectReader;
class ObjectWriter;
class Relocator;
class ResolveInfo;
class SectionData;
class SectionReachedListMap;
class StubFactory;

//===----------------------------------------------------------------------===//
/// TargetLDBackend - Generic interface to target specific assembler backends.
//===----------------------------------------------------------------------===//
class TargetLDBackend {
 public:
  typedef llvm::iplist<Relocation, GCFactoryListTraits<Relocation> >
      ExtraRelocList;
  typedef ExtraRelocList::iterator extra_reloc_iterator;

 protected:
  explicit TargetLDBackend(const LinkerConfig& pConfig);

 public:
  virtual ~TargetLDBackend();

  // -----  target dependent  ----- //
  virtual void initTargetSegments(IRBuilder& pBuilder) {}
  virtual void initTargetSections(Module& pModule, ObjectBuilder& pBuilder) {}
  virtual void initTargetSymbols(IRBuilder& pBuilder, Module& pModule) {}
  virtual void initTargetRelocation(IRBuilder& pBuilder) {}
  virtual bool initStandardSymbols(IRBuilder& pBuilder, Module& pModule) = 0;

  virtual bool initRelocator() = 0;

  virtual Relocator* getRelocator() = 0;
  virtual const Relocator* getRelocator() const = 0;

  // -----  format dependent  ----- //
  virtual ArchiveReader* createArchiveReader(Module&) = 0;
  virtual ObjectReader* createObjectReader(IRBuilder&) = 0;
  virtual DynObjReader* createDynObjReader(IRBuilder&) = 0;
  virtual BinaryReader* createBinaryReader(IRBuilder&) = 0;
  virtual ObjectWriter* createWriter() = 0;

  virtual bool initStdSections(ObjectBuilder& pBuilder) = 0;

  /// layout - layout method
  virtual void layout(Module& pModule) = 0;

  /// preLayout - Backend can do any needed modification before layout
  virtual void preLayout(Module& pModule, IRBuilder& pBuilder) = 0;

  /// postLayout - Backend can do any needed modification after layout
  virtual void postLayout(Module& pModule, IRBuilder& pBuilder) = 0;

  /// postProcessing - Backend can do any needed modification in the final stage
  virtual void postProcessing(FileOutputBuffer& pOutput) = 0;

  /// section start offset in the output file
  virtual size_t sectionStartOffset() const = 0;

  /// computeSectionOrder - compute the layout order of the given section
  virtual unsigned int getSectionOrder(const LDSection& pSectHdr) const = 0;

  /// sizeNamePools - compute the size of regular name pools
  /// In ELF executable files, regular name pools are .symtab, .strtab.,
  /// .dynsym, .dynstr, and .hash
  virtual void sizeNamePools(Module& pModule) = 0;

  /// finalizeSymbol - Linker checks pSymbol.reserved() if it's not zero,
  /// then it will ask backend to finalize the symbol value.
  /// @return ture - if backend set the symbol value sucessfully
  /// @return false - if backend do not recognize the symbol
  virtual bool finalizeSymbols() = 0;

  /// finalizeTLSSymbol - Linker asks backend to set the symbol value when it
  /// meets a TLS symbol
  virtual bool finalizeTLSSymbol(LDSymbol& pSymbol) = 0;

  /// allocateCommonSymbols - allocate common symbols in the corresponding
  /// sections.
  virtual bool allocateCommonSymbols(Module& pModule) = 0;

  /// preMergeSections - hooks to be executed before merging sections
  virtual void preMergeSections(Module& pModule) { }

  /// postMergeSections - hooks to be executed after merging sections
  virtual void postMergeSections(Module& pModule) { }

  /// mergeSection - merge target dependent sections.
  virtual bool mergeSection(Module& pModule,
                            const Input& pInputFile,
                            LDSection& pInputSection) {
    return true;
  }

  /// setUpReachedSectionsForGC - set the reference between two sections for
  /// some special target sections. GC will set up the reference for the Regular
  /// and BSS sections. Backends can also set up the reference if need.
  virtual void setUpReachedSectionsForGC(
      const Module& pModule,
      GarbageCollection::SectionReachedListMap& pSectReachedListMap) const {}

  /// updateSectionFlags - update pTo's flags when merging pFrom
  /// update the output section flags based on input section flags.
  /// FIXME: (Luba) I know ELF need to merge flags, but I'm not sure if
  /// MachO and COFF also need this.
  virtual bool updateSectionFlags(LDSection& pTo, const LDSection& pFrom) {
    return true;
  }

  /// readSection - read a target dependent section
  virtual bool readSection(Input& pInput, SectionData& pSD) { return true; }

  /// sizeInterp - compute the size of program interpreter's name
  /// In ELF executables, this is the length of dynamic linker's path name
  virtual void sizeInterp() = 0;

  /// getEntry - get the entry point name
  virtual llvm::StringRef getEntry(const Module& pModule) const = 0;

  // -----  relaxation  ----- //
  virtual bool initBRIslandFactory() = 0;
  virtual bool initStubFactory() = 0;
  virtual bool initTargetStubs() { return true; }

  virtual BranchIslandFactory* getBRIslandFactory() = 0;
  virtual StubFactory* getStubFactory() = 0;

  /// relax - the relaxation pass
  virtual bool relax(Module& pModule, IRBuilder& pBuilder) = 0;

  /// mayRelax - return true if the backend needs to do relaxation
  virtual bool mayRelax() = 0;

  /// commonPageSize - the common page size of the target machine
  virtual uint64_t commonPageSize() const = 0;

  /// abiPageSize - the abi page size of the target machine
  virtual uint64_t abiPageSize() const = 0;

  /// sortRelocation - sort the dynamic relocations to let dynamic linker
  /// process relocations more efficiently
  virtual void sortRelocation(LDSection& pSection) = 0;

  /// createAndSizeEhFrameHdr - This is seperated since we may add eh_frame
  /// entry in the middle
  virtual void createAndSizeEhFrameHdr(Module& pModule) = 0;

  /// isSymbolPreemptible - whether the symbol can be preemted by other link
  /// units
  virtual bool isSymbolPreemptible(const ResolveInfo& pSym) const = 0;

  /// mayHaveUnsafeFunctionPointerAccess - check if the section may have unsafe
  /// function pointer access
  virtual bool mayHaveUnsafeFunctionPointerAccess(
      const LDSection& pSection) const = 0;

  extra_reloc_iterator extra_reloc_begin() {
    return m_ExtraReloc.begin();
  }

  extra_reloc_iterator extra_reloc_end() {
    return m_ExtraReloc.end();
  }

 protected:
  const LinkerConfig& config() const { return m_Config; }

  /// addExtraRelocation - Add an extra relocation which are automatically
  /// generated by the LD backend.
  void addExtraRelocation(Relocation* reloc) {
    m_ExtraReloc.push_back(reloc);
  }

 private:
  const LinkerConfig& m_Config;

  /// m_ExtraReloc - Extra relocations that are automatically generated by the
  /// linker.
  ExtraRelocList m_ExtraReloc;

 private:
  DISALLOW_COPY_AND_ASSIGN(TargetLDBackend);
};

}  // namespace mcld

#endif  // MCLD_TARGET_TARGETLDBACKEND_H_
