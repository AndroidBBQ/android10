//===- DiagnosticPrinter.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DIAGNOSTICPRINTER_H_
#define MCLD_LD_DIAGNOSTICPRINTER_H_

#include "mcld/LD/Diagnostic.h"
#include "mcld/LD/DiagnosticEngine.h"

namespace mcld {

/** \class DiagnosticPrinter
 *  \brief DiagnosticPrinter provides the interface to customize diagnostic
 *  messages and output.
 */
class DiagnosticPrinter {
 public:
  DiagnosticPrinter();

  virtual ~DiagnosticPrinter();

  virtual void beginInput(const Input& pInput, const LinkerConfig& pConfig) {}

  virtual void endInput() {}

  virtual void finish() {}

  virtual void clear() { m_NumErrors = m_NumWarnings = 0; }

  /// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed.
  virtual void handleDiagnostic(DiagnosticEngine::Severity pSeverity,
                                const Diagnostic& pInfo);

  unsigned int getNumErrors() const { return m_NumErrors; }
  unsigned int getNumWarnings() const { return m_NumWarnings; }

 protected:
  unsigned int m_NumErrors;
  unsigned int m_NumWarnings;
};

}  // namespace mcld

#endif  // MCLD_LD_DIAGNOSTICPRINTER_H_
