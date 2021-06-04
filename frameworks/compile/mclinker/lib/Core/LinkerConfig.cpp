//===- LinkerConfig.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LinkerConfig.h"

#include "mcld/Config/Config.h"
#include "mcld/Support/MsgHandling.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// LinkerConfig
//===----------------------------------------------------------------------===//
LinkerConfig::LinkerConfig()
    : m_Options(),
      m_Targets(),
      m_Attribute(),
      m_CodeGenType(Unknown),
      m_CodePosition(Unset) {
  // FIXME: is here the right place to hold this?
  InitializeDiagnosticEngine(*this);
}

LinkerConfig::LinkerConfig(const std::string& pTripleString)
    : m_Options(),
      m_Targets(pTripleString),
      m_Attribute(),
      m_CodeGenType(Unknown),
      m_CodePosition(Unset) {
  // FIXME: is here the right place to hold this?
  InitializeDiagnosticEngine(*this);
}

LinkerConfig::~LinkerConfig() {
  // FIXME: is here the right place to hold this?
  FinalizeDiagnosticEngine();
}

const char* LinkerConfig::version() {
  return MCLD_VERSION;
}

}  // namespace mcld
