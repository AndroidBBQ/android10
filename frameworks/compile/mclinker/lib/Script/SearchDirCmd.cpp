//===- SearchDirCmd.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/SearchDirCmd.h"

#include "mcld/Support/raw_ostream.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// SearchDirCmd
//===----------------------------------------------------------------------===//
SearchDirCmd::SearchDirCmd(const std::string& pPath)
    : ScriptCommand(ScriptCommand::SEARCH_DIR), m_Path(pPath) {
}

SearchDirCmd::~SearchDirCmd() {
}

void SearchDirCmd::dump() const {
  mcld::outs() << "SEARCH_DIR ( " << m_Path << " )\n";
}

void SearchDirCmd::activate(Module& pModule) {
  pModule.getScript().directories().insert(m_Path);
}

}  // namespace mcld
