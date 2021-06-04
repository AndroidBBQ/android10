//===- MsgHandler.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/MsgHandler.h"

#include "mcld/LD/DiagnosticEngine.h"

namespace mcld {

MsgHandler::MsgHandler(DiagnosticEngine& pEngine)
    : m_Engine(pEngine), m_NumArgs(0) {
}

MsgHandler::~MsgHandler() {
  emit();
}

bool MsgHandler::emit() {
  flushCounts();
  return m_Engine.emit();
}

void MsgHandler::addString(llvm::StringRef pStr) const {
  assert(m_NumArgs < DiagnosticEngine::MaxArguments &&
         "Too many arguments to diagnostic!");
  m_Engine.state().ArgumentKinds[m_NumArgs] = DiagnosticEngine::ak_std_string;
  m_Engine.state().ArgumentStrs[m_NumArgs++] = pStr.data();
}

void MsgHandler::addString(const std::string& pStr) const {
  assert(m_NumArgs < DiagnosticEngine::MaxArguments &&
         "Too many arguments to diagnostic!");
  m_Engine.state().ArgumentKinds[m_NumArgs] = DiagnosticEngine::ak_std_string;
  m_Engine.state().ArgumentStrs[m_NumArgs++] = pStr;
}

void MsgHandler::addTaggedVal(intptr_t pValue,
                              DiagnosticEngine::ArgumentKind pKind) const {
  assert(m_NumArgs < DiagnosticEngine::MaxArguments &&
         "Too many arguments to diagnostic!");
  m_Engine.state().ArgumentKinds[m_NumArgs] = pKind;
  m_Engine.state().ArgumentVals[m_NumArgs++] = pValue;
}

}  // namespace mcld
