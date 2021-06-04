//===- MsgHandling.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_MSGHANDLING_H_
#define MCLD_SUPPORT_MSGHANDLING_H_
#include "mcld/LD/MsgHandler.h"

namespace mcld {

class LinkerConfig;
class DiagnosticPrinter;
class DiagnosticLineInfo;

void InitializeDiagnosticEngine(const LinkerConfig& pConfig,
                                DiagnosticPrinter* pPrinter = NULL);

void FinalizeDiagnosticEngine();

bool Diagnose();

DiagnosticEngine& getDiagnosticEngine();

MsgHandler unreachable(unsigned int pID);
MsgHandler fatal(unsigned int pID);
MsgHandler error(unsigned int pID);
MsgHandler warning(unsigned int pID);
MsgHandler debug(unsigned int pID);
MsgHandler note(unsigned int pID);
MsgHandler ignore(unsigned int pID);

}  // namespace mcld

//===----------------------------------------------------------------------===//
//  Inline functions
//===----------------------------------------------------------------------===//
inline mcld::MsgHandler mcld::unreachable(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Unreachable);
}

inline mcld::MsgHandler mcld::fatal(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Fatal);
}

inline mcld::MsgHandler mcld::error(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Error);
}

inline mcld::MsgHandler mcld::warning(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Warning);
}

inline mcld::MsgHandler mcld::debug(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Debug);
}

inline mcld::MsgHandler mcld::note(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Note);
}

inline mcld::MsgHandler mcld::ignore(unsigned int pID) {
  return getDiagnosticEngine().report(pID, DiagnosticEngine::Ignore);
}

#endif  // MCLD_SUPPORT_MSGHANDLING_H_
