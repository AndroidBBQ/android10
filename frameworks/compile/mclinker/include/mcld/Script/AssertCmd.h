//===- AssertCmd.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_ASSERTCMD_H_
#define MCLD_SCRIPT_ASSERTCMD_H_

#include "mcld/Script/ScriptCommand.h"

#include <string>

namespace mcld {

class RpnExpr;
class Module;

/** \class AssertCmd
 *  \brief This class defines the interfaces to assert command.
 */

class AssertCmd : public ScriptCommand {
 public:
  AssertCmd(RpnExpr& pRpnExpr, const std::string& pMessage);

  ~AssertCmd();

  AssertCmd& operator=(const AssertCmd& pAssertCmd);

  const RpnExpr& getRpnExpr() const { return m_RpnExpr; }
  RpnExpr& getRpnExpr() { return m_RpnExpr; }

  const std::string& message() const { return m_Message; }

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::ASSERT;
  }

  void activate(Module& pModule);

 private:
  RpnExpr& m_RpnExpr;
  std::string m_Message;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_ASSERTCMD_H_
