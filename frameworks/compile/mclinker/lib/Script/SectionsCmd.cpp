//===- SectionsCmd.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/SectionsCmd.h"

#include "mcld/Support/raw_ostream.h"

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// SectionsCmd
//===----------------------------------------------------------------------===//
SectionsCmd::SectionsCmd() : ScriptCommand(ScriptCommand::SECTIONS) {
}

SectionsCmd::~SectionsCmd() {
  for (iterator it = begin(), ie = end(); it != ie; ++it) {
    if (*it != NULL)
      delete *it;
  }
}

void SectionsCmd::dump() const {
  mcld::outs() << "SECTIONS\n{\n";

  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    switch ((*it)->getKind()) {
      case ScriptCommand::ENTRY:
      case ScriptCommand::ASSIGNMENT:
      case ScriptCommand::OUTPUT_SECT_DESC:
        mcld::outs() << "\t";
        (*it)->dump();
        break;
      default:
        assert(0);
        break;
    }
  }

  mcld::outs() << "}\n";
}

void SectionsCmd::push_back(ScriptCommand* pCommand) {
  switch (pCommand->getKind()) {
    case ScriptCommand::ENTRY:
    case ScriptCommand::ASSIGNMENT:
    case ScriptCommand::OUTPUT_SECT_DESC:
      m_SectionCommands.push_back(pCommand);
      break;
    default:
      assert(0);
      break;
  }
}

void SectionsCmd::activate(Module& pModule) {
  // Assignment between output sections
  SectionCommands assignments;

  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    switch ((*it)->getKind()) {
      case ScriptCommand::ENTRY:
        (*it)->activate(pModule);
        break;
      case ScriptCommand::ASSIGNMENT:
        assignments.push_back(*it);
        break;
      case ScriptCommand::OUTPUT_SECT_DESC: {
        (*it)->activate(pModule);

        iterator assign, assignEnd = assignments.end();
        for (assign = assignments.begin(); assign != assignEnd; ++assign)
          (*assign)->activate(pModule);
        assignments.clear();

        break;
      }
      default:
        assert(0);
        break;
    }
  }
}

}  // namespace mcld
