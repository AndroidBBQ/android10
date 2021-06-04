//===- EntryCmd.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/EntryCmd.h"

#include "mcld/Support/raw_ostream.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// EntryCmd
//===----------------------------------------------------------------------===//
EntryCmd::EntryCmd(const std::string& pEntry)
    : ScriptCommand(ScriptCommand::ENTRY), m_Entry(pEntry) {
}

EntryCmd::~EntryCmd() {
}

void EntryCmd::dump() const {
  mcld::outs() << "ENTRY ( " << m_Entry << " )\n";
}

void EntryCmd::activate(Module& pModule) {
  LinkerScript& script = pModule.getScript();
  if (!script.hasEntry())
    script.setEntry(m_Entry);
}

}  // namespace mcld
