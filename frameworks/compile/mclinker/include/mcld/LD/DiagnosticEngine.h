//===- DiagnosticEngine.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DIAGNOSTICENGINE_H_
#define MCLD_LD_DIAGNOSTICENGINE_H_
#include "mcld/LD/DiagnosticInfos.h"

#include <llvm/Support/DataTypes.h>

#include <string>

namespace mcld {

class DiagnosticLineInfo;
class DiagnosticPrinter;
class Input;
class LinkerConfig;
class MsgHandler;

/** \class DiagnosticEngine
 *  \brief DiagnosticEngine is used to report problems and issues.
 *
 *  DiagnosticEngine is used to report problems and issues. It creates the
 *  Diagnostics and passes them to the DiagnosticPrinter for reporting to the
 *  user.
 *
 *  DiagnosticEngine is a complex class, it is responsible for
 *  - remember the argument string for MsgHandler
 *  - choice the severity of a message by options
 */
class DiagnosticEngine {
 public:
  enum Severity {
    Unreachable,
    Fatal,
    Error,
    Warning,
    Debug,
    Note,
    Ignore,
    None
  };

  enum ArgumentKind {
    ak_std_string,  // std::string
    ak_c_string,    // const char *
    ak_sint,        // int
    ak_uint,        // unsigned int
    ak_ulonglong,   // unsigned long long
    ak_bool         // bool
  };

 public:
  DiagnosticEngine();

  ~DiagnosticEngine();

  void reset(const LinkerConfig& pConfig);

  void setLineInfo(DiagnosticLineInfo& pLineInfo);

  void setPrinter(DiagnosticPrinter& pPrinter, bool pShouldOwnPrinter = true);

  const DiagnosticPrinter* getPrinter() const { return m_pPrinter; }
  DiagnosticPrinter* getPrinter() { return m_pPrinter; }

  DiagnosticPrinter* takePrinter() {
    m_OwnPrinter = false;
    return m_pPrinter;
  }

  bool ownPrinter() const { return m_OwnPrinter; }

  // -----  emission  ----- //
  // emit - process the message to printer
  bool emit();

  // report - issue the message to the printer
  MsgHandler report(uint16_t pID, Severity pSeverity);

 private:
  friend class MsgHandler;
  friend class Diagnostic;

  enum {
    /// MaxArguments - The maximum number of arguments we can hold. We currently
    /// only support up to 10 arguments (%0-%9).
    MaxArguments = 10
  };

  struct State {
   public:
    State() : numArgs(0), ID(-1), severity(None), file(NULL) {}
    ~State() {}

    void reset() {
      numArgs = 0;
      ID = -1;
      severity = None;
      file = NULL;
    }

   public:
    std::string ArgumentStrs[MaxArguments];
    intptr_t ArgumentVals[MaxArguments];
    uint8_t ArgumentKinds[MaxArguments];
    int8_t numArgs;
    uint16_t ID;
    Severity severity;
    Input* file;
  };

 private:
  State& state() { return m_State; }

  const State& state() const { return m_State; }

  DiagnosticInfos& infoMap() {
    assert(m_pInfoMap != NULL && "DiagnosticEngine was not initialized!");
    return *m_pInfoMap;
  }

  const DiagnosticInfos& infoMap() const {
    assert(m_pInfoMap != NULL && "DiagnosticEngine was not initialized!");
    return *m_pInfoMap;
  }

 private:
  const LinkerConfig* m_pConfig;
  DiagnosticLineInfo* m_pLineInfo;
  DiagnosticPrinter* m_pPrinter;
  DiagnosticInfos* m_pInfoMap;
  bool m_OwnPrinter;

  State m_State;
};

}  // namespace mcld

#endif  // MCLD_LD_DIAGNOSTICENGINE_H_
