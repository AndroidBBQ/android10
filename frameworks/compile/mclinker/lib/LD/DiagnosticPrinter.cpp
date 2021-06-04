//===- DiagnosticPrinter.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DiagnosticPrinter.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// DiagnosticPrinter
//===----------------------------------------------------------------------===//
DiagnosticPrinter::DiagnosticPrinter() : m_NumErrors(0), m_NumWarnings(0) {
}

DiagnosticPrinter::~DiagnosticPrinter() {
  clear();
}

/// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
/// capturing it to a log as needed.
void DiagnosticPrinter::handleDiagnostic(DiagnosticEngine::Severity pSeverity,
                                         const Diagnostic& pInfo) {
  if (pSeverity == DiagnosticEngine::Warning)
    ++m_NumWarnings;

  if (pSeverity <= DiagnosticEngine::Error)
    ++m_NumErrors;
}

}  // namespace mcld
