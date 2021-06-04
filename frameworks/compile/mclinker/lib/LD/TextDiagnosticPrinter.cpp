//===- TextDiagnosticPrinter.cpp ------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/TextDiagnosticPrinter.h"

#include "mcld/LinkerConfig.h"

#include <llvm/Support/Signals.h>

#include <string>

namespace mcld {

static const enum llvm::raw_ostream::Colors UnreachableColor =
    llvm::raw_ostream::RED;
static const enum llvm::raw_ostream::Colors FatalColor =
    llvm::raw_ostream::YELLOW;
static const enum llvm::raw_ostream::Colors ErrorColor = llvm::raw_ostream::RED;
static const enum llvm::raw_ostream::Colors WarningColor =
    llvm::raw_ostream::MAGENTA;
static const enum llvm::raw_ostream::Colors DebugColor =
    llvm::raw_ostream::CYAN;
static const enum llvm::raw_ostream::Colors NoteColor =
    llvm::raw_ostream::GREEN;
static const enum llvm::raw_ostream::Colors IgnoreColor =
    llvm::raw_ostream::BLUE;

//===----------------------------------------------------------------------===//
// TextDiagnosticPrinter
TextDiagnosticPrinter::TextDiagnosticPrinter(llvm::raw_ostream& pOStream,
                                             const LinkerConfig& pConfig)
    : m_OStream(pOStream), m_Config(pConfig), m_pInput(NULL) {
}

TextDiagnosticPrinter::~TextDiagnosticPrinter() {
}

/// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
/// capturing it to a log as needed.
void TextDiagnosticPrinter::handleDiagnostic(
    DiagnosticEngine::Severity pSeverity,
    const Diagnostic& pInfo) {
  DiagnosticPrinter::handleDiagnostic(pSeverity, pInfo);

  std::string out_string;
  pInfo.format(out_string);

  switch (pSeverity) {
    case DiagnosticEngine::Unreachable: {
      m_OStream.changeColor(UnreachableColor, true);
      m_OStream << "Unreachable: ";
      m_OStream.resetColor();
      m_OStream << out_string << "\n";
      break;
    }
    case DiagnosticEngine::Fatal: {
      m_OStream.changeColor(FatalColor, true);
      m_OStream << "Fatal: ";
      m_OStream.resetColor();
      m_OStream << out_string << "\n";
      break;
    }
    case DiagnosticEngine::Error: {
      m_OStream.changeColor(ErrorColor, true);
      m_OStream << "Error: ";
      m_OStream.resetColor();
      m_OStream << out_string << "\n";
      break;
    }
    case DiagnosticEngine::Warning: {
      m_OStream.changeColor(WarningColor, true);
      m_OStream << "Warning: ";
      m_OStream.resetColor();
      m_OStream << out_string << "\n";
      break;
    }
    case DiagnosticEngine::Debug: {
      // show debug message only if verbose >= 0
      if (m_Config.options().verbose() >= 0) {
        m_OStream.changeColor(DebugColor, true);
        m_OStream << "Debug: ";
        m_OStream.resetColor();
        m_OStream << out_string << "\n";
      }
      break;
    }
    case DiagnosticEngine::Note: {
      // show ignored message only if verbose >= 1
      if (m_Config.options().verbose() >= 1) {
        m_OStream.changeColor(NoteColor, true);
        m_OStream << "Note: ";
        m_OStream.resetColor();
        m_OStream << out_string << "\n";
      }
      break;
    }
    case DiagnosticEngine::Ignore: {
      // show ignored message only if verbose >= 2
      if (m_Config.options().verbose() >= 2) {
        m_OStream.changeColor(IgnoreColor, true);
        m_OStream << "Ignore: ";
        m_OStream.resetColor();
        m_OStream << out_string << "\n";
      }
      break;
    }
    default:
      break;
  }

  switch (pSeverity) {
    case DiagnosticEngine::Unreachable: {
      m_OStream << "\n\n";
      m_OStream.changeColor(llvm::raw_ostream::YELLOW);
      m_OStream << "You encounter a bug of MCLinker, please report to:\n"
                << "  mclinker@googlegroups.com\n";
      m_OStream.resetColor();
    }
    /** fall through **/
    case DiagnosticEngine::Fatal: {
      // If we reached here, we are failing ungracefully. Run the interrupt
      // handlers
      // to make sure any special cleanups get done, in particular that we
      // remove
      // files registered with RemoveFileOnSignal.
      llvm::sys::RunInterruptHandlers();
      exit(1);
      break;
    }
    case DiagnosticEngine::Error: {
      int16_t error_limit = m_Config.options().maxErrorNum();
      if ((error_limit != -1) &&
          (getNumErrors() > static_cast<unsigned>(error_limit))) {
        m_OStream << "\n\n";
        m_OStream.changeColor(llvm::raw_ostream::YELLOW);
        m_OStream << "too many error messages (>" << error_limit << ")...\n";
        m_OStream.resetColor();
        llvm::sys::RunInterruptHandlers();
        exit(1);
      }
      break;
    }
    case DiagnosticEngine::Warning: {
      int16_t warning_limit = m_Config.options().maxWarnNum();
      if ((warning_limit != -1) &&
          (getNumWarnings() > static_cast<unsigned>(warning_limit))) {
        m_OStream << "\n\n";
        m_OStream.changeColor(llvm::raw_ostream::YELLOW);
        m_OStream << "too many warning messages (>" << warning_limit
                  << ")...\n";
        m_OStream.resetColor();
        llvm::sys::RunInterruptHandlers();
        exit(1);
      }
    }
    default:
      break;
  }
}

void TextDiagnosticPrinter::beginInput(const Input& pInput,
                                       const LinkerConfig& pConfig) {
  m_pInput = &pInput;
}

void TextDiagnosticPrinter::endInput() {
  m_pInput = NULL;
}

}  // namespace mcld
