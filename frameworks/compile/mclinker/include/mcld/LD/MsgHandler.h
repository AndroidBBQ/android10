//===- MsgHandler.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_MSGHANDLER_H_
#define MCLD_LD_MSGHANDLER_H_
#include "mcld/LD/DiagnosticEngine.h"
#include "mcld/Support/Path.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

#include <string>

namespace mcld {

/** \class MsgHandler
 *  \brief MsgHandler controls the timing to output message.
 */
class MsgHandler {
 public:
  explicit MsgHandler(DiagnosticEngine& pEngine);
  ~MsgHandler();

  bool emit();

  void addString(llvm::StringRef pStr) const;

  void addString(const std::string& pStr) const;

  void addTaggedVal(intptr_t pValue,
                    DiagnosticEngine::ArgumentKind pKind) const;

 private:
  void flushCounts() { m_Engine.state().numArgs = m_NumArgs; }

 private:
  DiagnosticEngine& m_Engine;
  mutable unsigned int m_NumArgs;
};

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    llvm::StringRef pStr) {
  pHandler.addString(pStr);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    const std::string& pStr) {
  pHandler.addString(pStr);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    const sys::fs::Path& pPath) {
  pHandler.addString(pPath.native());
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    const char* pStr) {
  pHandler.addTaggedVal(reinterpret_cast<intptr_t>(pStr),
                        DiagnosticEngine::ak_c_string);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler, int pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_sint);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    unsigned int pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_uint);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler, long pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_sint);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    unsigned long pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_uint);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler,
                                    unsigned long long pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_ulonglong);
  return pHandler;
}

inline const MsgHandler& operator<<(const MsgHandler& pHandler, bool pValue) {
  pHandler.addTaggedVal(pValue, DiagnosticEngine::ak_bool);
  return pHandler;
}

}  // namespace mcld

#endif  // MCLD_LD_MSGHANDLER_H_
