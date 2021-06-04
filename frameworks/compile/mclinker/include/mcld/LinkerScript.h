//===- LinkerScript.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LINKERSCRIPT_H_
#define MCLD_LINKERSCRIPT_H_

#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringEntry.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/MC/SearchDirs.h"
#include "mcld/Object/SectionMap.h"
#include "mcld/Script/AssertCmd.h"
#include "mcld/Script/Assignment.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>

#include <string>
#include <vector>

namespace mcld {

class LDSymbol;

/** \class LinkerScript
 *
 */
class LinkerScript {
 public:
  typedef HashTable<StringEntry<llvm::StringRef>,
                    hash::StringHash<hash::DJB>,
                    StringEntryFactory<llvm::StringRef> > SymbolRenameMap;

  typedef HashTable<StringEntry<uint64_t>,
                    hash::StringHash<hash::DJB>,
                    StringEntryFactory<uint64_t> > AddressMap;

  typedef std::vector<std::pair<LDSymbol*, Assignment> > Assignments;

  typedef std::vector<AssertCmd> Assertions;

  typedef llvm::SmallVector<std::string, 8> DefSyms;

 public:
  LinkerScript();

  ~LinkerScript();

  const SymbolRenameMap& renameMap() const { return m_SymbolRenames; }
  SymbolRenameMap& renameMap() { return m_SymbolRenames; }

  const AddressMap& addressMap() const { return m_AddressMap; }
  AddressMap& addressMap() { return m_AddressMap; }

  const SectionMap& sectionMap() const { return m_SectionMap; }
  SectionMap& sectionMap() { return m_SectionMap; }

  const Assignments& assignments() const { return m_Assignments; }
  Assignments& assignments() { return m_Assignments; }

  const Assertions& assertions() const { return m_Assertions; }
  Assertions& assertions() { return m_Assertions; }

  const DefSyms& defsyms() const { return m_DefSyms; }
  DefSyms& defsyms() { return m_DefSyms; }

  /// search directory
  const SearchDirs& directories() const { return m_SearchDirs; }
  SearchDirs& directories() { return m_SearchDirs; }

  /// sysroot
  const sys::fs::Path& sysroot() const;

  void setSysroot(const sys::fs::Path& pPath);

  bool hasSysroot() const;

  /// entry point
  const std::string& entry() const;

  void setEntry(const std::string& pEntry);

  bool hasEntry() const;

  /// output filename
  const std::string& outputFile() const;

  void setOutputFile(const std::string& pOutputFile);

  bool hasOutputFile() const;

 private:
  SymbolRenameMap m_SymbolRenames;
  AddressMap m_AddressMap;
  SectionMap m_SectionMap;
  Assignments m_Assignments;
  Assertions m_Assertions;
  DefSyms m_DefSyms;
  SearchDirs m_SearchDirs;
  std::string m_Entry;
  std::string m_OutputFile;
};

}  // namespace mcld

#endif  // MCLD_LINKERSCRIPT_H_
