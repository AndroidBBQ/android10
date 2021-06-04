//===- ObjectLinker.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_OBJECT_OBJECTLINKER_H_
#define MCLD_OBJECT_OBJECTLINKER_H_
#include <llvm/Support/DataTypes.h>

namespace mcld {

class ArchiveReader;
class BinaryReader;
class BinaryWriter;
class DynObjReader;
class DynObjWriter;
class ExecWriter;
class FileOutputBuffer;
class GroupReader;
class IRBuilder;
class LinkerConfig;
class Module;
class ObjectReader;
class ObjectWriter;
class Relocation;
class ResolveInfo;
class ScriptReader;
class TargetLDBackend;

/** \class ObjectLinker
 */
class ObjectLinker {
 public:
  ObjectLinker(const LinkerConfig& pConfig, TargetLDBackend& pLDBackend);

  ~ObjectLinker();

  bool initialize(Module& pModule, IRBuilder& pBuilder);

  /// initStdSections - initialize standard sections of the output file.
  bool initStdSections();

  /// addUndefinedSymbols - add symbols set by -u
  void addUndefinedSymbols();

  /// normalize - normalize the input files
  void normalize();

  /// linkable - check the linkability of current LinkerConfig
  ///  Check list:
  ///  - check the Attributes are not violate the constaint
  ///  - check every Input has a correct Attribute
  bool linkable() const;

  /// readRelocations - read all relocation entries
  bool readRelocations();

  /// dataStrippingOpt - optimizations for reducing code size
  void dataStrippingOpt();

  /// mergeSections - put allinput sections into output sections
  bool mergeSections();

  /// addSymbolsToOutput - after all symbols has been resolved, add the symbol
  /// to output
  void addSymbolsToOutput(Module& pModule);

  /// allocateCommonSymobols - allocate fragments for common symbols to the
  /// corresponding sections
  bool allocateCommonSymbols();

  /// addStandardSymbols - shared object and executable files need some
  /// standard symbols
  ///   @return if there are some input symbols with the same name to the
  ///   standard symbols, return false
  bool addStandardSymbols();

  /// addTargetSymbols - some targets, such as MIPS and ARM, need some
  /// target-dependent symbols
  ///   @return if there are some input symbols with the same name to the
  ///   target symbols, return false
  bool addTargetSymbols();

  /// addScriptSymbols - define symbols from the command line option or linker
  /// scripts.
  bool addScriptSymbols();

  /// scanRelocations - scan all relocation entries by output symbols.
  bool scanRelocations();

  /// initStubs - initialize stub-related stuff.
  bool initStubs();

  /// prelayout - help backend to do some modification before layout
  bool prelayout();

  /// layout - linearly layout all output sections and reserve some space
  /// for GOT/PLT
  ///   Because we do not support instruction relaxing in this early version,
  ///   if there is a branch can not jump to its target, we return false
  ///   directly
  bool layout();

  /// postlayout - help backend to do some modification after layout
  bool postlayout();

  /// relocate - applying relocation entries and create relocation
  /// section in the output files
  /// Create relocation section, asking TargetLDBackend to
  /// read the relocation information into RelocationEntry
  /// and push_back into the relocation section
  bool relocation();

  /// finalizeSymbolValue - finalize the symbol value
  bool finalizeSymbolValue();

  /// emitOutput - emit the output file.
  bool emitOutput(FileOutputBuffer& pOutput);

  /// postProcessing - do modificatiion after all processes
  bool postProcessing(FileOutputBuffer& pOutput);

  // -----  readers and writers  ----- //
  const ObjectReader* getObjectReader() const { return m_pObjectReader; }
  ObjectReader* getObjectReader() { return m_pObjectReader; }

  const DynObjReader* getDynObjReader() const { return m_pDynObjReader; }
  DynObjReader* getDynObjReader() { return m_pDynObjReader; }

  const ArchiveReader* getArchiveReader() const { return m_pArchiveReader; }
  ArchiveReader* getArchiveReader() { return m_pArchiveReader; }

  const GroupReader* getGroupReader() const { return m_pGroupReader; }
  GroupReader* getGroupReader() { return m_pGroupReader; }

  const BinaryReader* getBinaryReader() const { return m_pBinaryReader; }
  BinaryReader* getBinaryReader() { return m_pBinaryReader; }

  const ScriptReader* getScriptReader() const { return m_pScriptReader; }
  ScriptReader* getScriptReader() { return m_pScriptReader; }

  const ObjectWriter* getWriter() const { return m_pWriter; }
  ObjectWriter* getWriter() { return m_pWriter; }

 private:
  /// normalSyncRelocationResult - sync relocation result when producing shared
  /// objects or executables
  void normalSyncRelocationResult(FileOutputBuffer& pOutput);

  /// partialSyncRelocationResult - sync relocation result when doing partial
  /// link
  void partialSyncRelocationResult(FileOutputBuffer& pOutput);

  /// writeRelocationResult - helper function of syncRelocationResult, write
  /// relocation target data to output
  void writeRelocationResult(Relocation& pReloc, uint8_t* pOutput);

  /// addSymbolToOutput - add a symbol to output symbol table if it's not a
  /// section symbol and not defined in the discarded section
  void addSymbolToOutput(ResolveInfo& pInfo, Module& pModule);

 private:
  const LinkerConfig& m_Config;
  Module* m_pModule;
  IRBuilder* m_pBuilder;

  TargetLDBackend& m_LDBackend;

  // -----  readers and writers  ----- //
  ObjectReader* m_pObjectReader;
  DynObjReader* m_pDynObjReader;
  ArchiveReader* m_pArchiveReader;
  GroupReader* m_pGroupReader;
  BinaryReader* m_pBinaryReader;
  ScriptReader* m_pScriptReader;
  ObjectWriter* m_pWriter;
};

}  // namespace mcld

#endif  // MCLD_OBJECT_OBJECTLINKER_H_
