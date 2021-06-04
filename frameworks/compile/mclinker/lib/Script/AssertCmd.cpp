//===- AssertCmd.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/AssertCmd.h"

#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/Script/RpnExpr.h"
#include "mcld/Support/raw_ostream.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// AssertCmd
//===----------------------------------------------------------------------===//
AssertCmd::AssertCmd(RpnExpr& pRpnExpr, const std::string& pMessage)
    : ScriptCommand(ScriptCommand::ASSERT),
      m_RpnExpr(pRpnExpr),
      m_Message(pMessage) {
}

AssertCmd::~AssertCmd() {
}

AssertCmd& AssertCmd::operator=(const AssertCmd& pAssertCmd) {
  return *this;
}

void AssertCmd::dump() const {
  mcld::outs() << "Assert ( ";

  m_RpnExpr.dump();

  mcld::outs() << " , " << m_Message << " )\n";
}

void AssertCmd::activate(Module& pModule) {
  pModule.getScript().assertions().push_back(*this);
}

}  // namespace mcld
