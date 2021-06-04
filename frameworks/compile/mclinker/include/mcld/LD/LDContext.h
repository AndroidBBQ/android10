//===- LDContext.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_LDCONTEXT_H_
#define MCLD_LD_LDCONTEXT_H_

#include "mcld/LD/LDFileFormat.h"

#include <llvm/Support/DataTypes.h>

#include <cassert>
#include <string>
#include <vector>

namespace llvm {
class StringRef;
}  // namespace llvm

namespace mcld {

class LDSymbol;
class LDSection;

/** \class LDContext
 *  \brief LDContext stores the data which a object file should has
 */
class LDContext {
 public:
  typedef std::vector<LDSection*> SectionTable;
  typedef SectionTable::iterator sect_iterator;
  typedef SectionTable::const_iterator const_sect_iterator;

  typedef std::vector<LDSymbol*> SymbolTable;
  typedef SymbolTable::iterator sym_iterator;
  typedef SymbolTable::const_iterator const_sym_iterator;

 public:
  // -----  sections  ----- //
  LDContext& appendSection(LDSection& pSection);

  const_sect_iterator sectBegin() const { return m_SectionTable.begin(); }
  sect_iterator sectBegin() { return m_SectionTable.begin(); }

  const_sect_iterator sectEnd() const { return m_SectionTable.end(); }
  sect_iterator sectEnd() { return m_SectionTable.end(); }

  const LDSection* getSection(unsigned int pIdx) const;
  LDSection* getSection(unsigned int pIdx);

  const LDSection* getSection(const std::string& pName) const;
  LDSection* getSection(const std::string& pName);

  size_t getSectionIdx(const std::string& pName) const;

  size_t numOfSections() const { return m_SectionTable.size(); }

  // -----  symbols  ----- //
  const LDSymbol* getSymbol(unsigned int pIdx) const;
  LDSymbol* getSymbol(unsigned int pIdx);

  const LDSymbol* getSymbol(const llvm::StringRef& pName) const;
  LDSymbol* getSymbol(const llvm::StringRef& pName);

  void addSymbol(LDSymbol* pSym) { m_SymTab.push_back(pSym); }

  const_sym_iterator symTabBegin() const { return m_SymTab.begin(); }
  sym_iterator symTabBegin() { return m_SymTab.begin(); }

  const_sym_iterator symTabEnd() const { return m_SymTab.end(); }
  sym_iterator symTabEnd() { return m_SymTab.end(); }

  // -----  relocations  ----- //
  const_sect_iterator relocSectBegin() const { return m_RelocSections.begin(); }
  sect_iterator relocSectBegin() { return m_RelocSections.begin(); }

  const_sect_iterator relocSectEnd() const { return m_RelocSections.end(); }
  sect_iterator relocSectEnd() { return m_RelocSections.end(); }

 private:
  SectionTable m_SectionTable;
  SymbolTable m_SymTab;
  SectionTable m_RelocSections;

  // FIXME : maintain a map<section name, section index>
};

}  // namespace mcld

#endif  // MCLD_LD_LDCONTEXT_H_
