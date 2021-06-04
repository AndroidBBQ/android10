//===- ScriptFile.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/ScriptFile.h"

#include "mcld/ADT/HashEntry.h"
#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/Script/AssertCmd.h"
#include "mcld/Script/EntryCmd.h"
#include "mcld/Script/GroupCmd.h"
#include "mcld/Script/InputCmd.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/OutputArchCmd.h"
#include "mcld/Script/OutputCmd.h"
#include "mcld/Script/OutputFormatCmd.h"
#include "mcld/Script/RpnExpr.h"
#include "mcld/Script/ScriptCommand.h"
#include "mcld/Script/SearchDirCmd.h"
#include "mcld/Script/SectionsCmd.h"
#include "mcld/Script/StringList.h"
#include "mcld/Script/StrToken.h"
#include "mcld/MC/Input.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/InputTree.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ManagedStatic.h>

#include <cassert>

namespace mcld {

typedef HashEntry<std::string, void*, hash::StringCompare<std::string> >
    ParserStrEntry;
typedef HashTable<ParserStrEntry,
                  hash::StringHash<hash::DJB>,
                  EntryFactory<ParserStrEntry> > ParserStrPool;
static llvm::ManagedStatic<ParserStrPool> g_ParserStrPool;

//===----------------------------------------------------------------------===//
// ScriptFile
//===----------------------------------------------------------------------===//
ScriptFile::ScriptFile(Kind pKind, Input& pInput, InputBuilder& pBuilder)
    : m_Kind(pKind),
      m_Input(pInput),
      m_Name(pInput.path().native()),
      m_pInputTree(NULL),
      m_Builder(pBuilder),
      m_bHasSectionsCmd(false),
      m_bInSectionsCmd(false),
      m_bInOutputSectDesc(false),
      m_pRpnExpr(NULL),
      m_pStringList(NULL),
      m_bAsNeeded(false) {
  // FIXME: move creation of input tree out of ScriptFile.
  m_pInputTree = new InputTree();
}

ScriptFile::~ScriptFile() {
  for (iterator it = begin(), ie = end(); it != ie; ++it) {
    if (*it != NULL)
      delete *it;
  }
  if (m_pInputTree != NULL)
    delete m_pInputTree;
}

void ScriptFile::dump() const {
  for (const_iterator it = begin(), ie = end(); it != ie; ++it)
    (*it)->dump();
}

void ScriptFile::activate(Module& pModule) {
  for (const_iterator it = begin(), ie = end(); it != ie; ++it)
    (*it)->activate(pModule);
}

void ScriptFile::addEntryPoint(const std::string& pSymbol) {
  EntryCmd* entry = new EntryCmd(pSymbol);

  if (m_bInSectionsCmd) {
    assert(!m_CommandQueue.empty());
    SectionsCmd* sections = llvm::cast<SectionsCmd>(back());
    sections->push_back(entry);
  } else {
    m_CommandQueue.push_back(entry);
  }
}

void ScriptFile::addOutputFormatCmd(const std::string& pName) {
  m_CommandQueue.push_back(new OutputFormatCmd(pName));
}

void ScriptFile::addOutputFormatCmd(const std::string& pDefault,
                                    const std::string& pBig,
                                    const std::string& pLittle) {
  m_CommandQueue.push_back(new OutputFormatCmd(pDefault, pBig, pLittle));
}

void ScriptFile::addInputCmd(StringList& pStringList,
                             ObjectReader& pObjectReader,
                             ArchiveReader& pArchiveReader,
                             DynObjReader& pDynObjReader,
                             const LinkerConfig& pConfig) {
  m_CommandQueue.push_back(new InputCmd(pStringList,
                                        *m_pInputTree,
                                        m_Builder,
                                        pObjectReader,
                                        pArchiveReader,
                                        pDynObjReader,
                                        pConfig));
}

void ScriptFile::addGroupCmd(StringList& pStringList,
                             GroupReader& pGroupReader,
                             const LinkerConfig& pConfig) {
  m_CommandQueue.push_back(new GroupCmd(
      pStringList, *m_pInputTree, m_Builder, pGroupReader, pConfig));
}

void ScriptFile::addOutputCmd(const std::string& pFileName) {
  m_CommandQueue.push_back(new OutputCmd(pFileName));
}

void ScriptFile::addSearchDirCmd(const std::string& pPath) {
  m_CommandQueue.push_back(new SearchDirCmd(pPath));
}

void ScriptFile::addOutputArchCmd(const std::string& pArch) {
  m_CommandQueue.push_back(new OutputArchCmd(pArch));
}

void ScriptFile::addAssertCmd(RpnExpr& pRpnExpr, const std::string& pMessage) {
  m_CommandQueue.push_back(new AssertCmd(pRpnExpr, pMessage));
}

void ScriptFile::addAssignment(const std::string& pSymbolName,
                               RpnExpr& pRpnExpr,
                               Assignment::Type pType) {
  if (m_bInSectionsCmd) {
    assert(!m_CommandQueue.empty());
    SectionsCmd* sections = llvm::cast<SectionsCmd>(back());
    if (m_bInOutputSectDesc) {
      assert(!sections->empty());
      OutputSectDesc* output_desc =
          llvm::cast<OutputSectDesc>(sections->back());
      output_desc->push_back(new Assignment(Assignment::INPUT_SECTION,
                                            pType,
                                            *(SymOperand::create(pSymbolName)),
                                            pRpnExpr));
    } else {
      sections->push_back(new Assignment(Assignment::OUTPUT_SECTION,
                                         pType,
                                         *(SymOperand::create(pSymbolName)),
                                         pRpnExpr));
    }
  } else {
    m_CommandQueue.push_back(new Assignment(Assignment::OUTSIDE_SECTIONS,
                                            pType,
                                            *(SymOperand::create(pSymbolName)),
                                            pRpnExpr));
  }
}

bool ScriptFile::hasSectionsCmd() const {
  return m_bHasSectionsCmd;
}

void ScriptFile::enterSectionsCmd() {
  m_bHasSectionsCmd = true;
  m_bInSectionsCmd = true;
  m_CommandQueue.push_back(new SectionsCmd());
}

void ScriptFile::leaveSectionsCmd() {
  m_bInSectionsCmd = false;
}

void ScriptFile::enterOutputSectDesc(const std::string& pName,
                                     const OutputSectDesc::Prolog& pProlog) {
  assert(!m_CommandQueue.empty());
  assert(m_bInSectionsCmd);
  SectionsCmd* sections = llvm::cast<SectionsCmd>(back());
  sections->push_back(new OutputSectDesc(pName, pProlog));

  m_bInOutputSectDesc = true;
}

void ScriptFile::leaveOutputSectDesc(const OutputSectDesc::Epilog& pEpilog) {
  assert(!m_CommandQueue.empty());
  assert(m_bInSectionsCmd);
  SectionsCmd* sections = llvm::cast<SectionsCmd>(back());

  assert(!sections->empty() && m_bInOutputSectDesc);
  OutputSectDesc* output_desc = llvm::cast<OutputSectDesc>(sections->back());
  output_desc->setEpilog(pEpilog);

  m_bInOutputSectDesc = false;
}

void ScriptFile::addInputSectDesc(InputSectDesc::KeepPolicy pPolicy,
                                  const InputSectDesc::Spec& pSpec) {
  assert(!m_CommandQueue.empty());
  assert(m_bInSectionsCmd);
  SectionsCmd* sections = llvm::cast<SectionsCmd>(back());

  assert(!sections->empty() && m_bInOutputSectDesc);
  OutputSectDesc* output_sect = llvm::cast<OutputSectDesc>(sections->back());

  output_sect->push_back(new InputSectDesc(pPolicy, pSpec, *output_sect));
}

RpnExpr* ScriptFile::createRpnExpr() {
  m_pRpnExpr = RpnExpr::create();
  return m_pRpnExpr;
}

StringList* ScriptFile::createStringList() {
  m_pStringList = StringList::create();
  return m_pStringList;
}

void ScriptFile::setAsNeeded(bool pEnable) {
  m_bAsNeeded = pEnable;
}

const std::string& ScriptFile::createParserStr(const char* pText,
                                               size_t pLength) {
  bool exist = false;
  ParserStrEntry* entry =
      g_ParserStrPool->insert(std::string(pText, pLength), exist);
  return entry->key();
}

void ScriptFile::clearParserStrPool() {
  g_ParserStrPool->clear();
}

}  // namespace mcld
