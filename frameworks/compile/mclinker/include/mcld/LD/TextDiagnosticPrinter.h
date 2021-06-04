//===- TextDiagnosticPrinter.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_TEXTDIAGNOSTICPRINTER_H_
#define MCLD_LD_TEXTDIAGNOSTICPRINTER_H_
#include "mcld/LD/DiagnosticPrinter.h"

#include <llvm/Support/raw_ostream.h>

namespace mcld {

class LinkerConfig;

/** \class TextDiagnosticPrinter
 *  \brief The plain, text-based DiagnosticPrinter.
 */
class TextDiagnosticPrinter : public DiagnosticPrinter {
 public:
  TextDiagnosticPrinter(llvm::raw_ostream& pOStream,
                        const LinkerConfig& pConfig);

  virtual ~TextDiagnosticPrinter();

  /// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed.
  virtual void handleDiagnostic(DiagnosticEngine::Severity pSeverity,
                                const Diagnostic& pInfo);

  virtual void beginInput(const Input& pInput, const LinkerConfig& pConfig);

  virtual void endInput();

 private:
  llvm::raw_ostream& m_OStream;
  const LinkerConfig& m_Config;
  const Input* m_pInput;
};

}  // namespace mcld

#endif  // MCLD_LD_TEXTDIAGNOSTICPRINTER_H_
