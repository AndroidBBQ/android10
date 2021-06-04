//===- GroupCmd.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_GROUPCMD_H_
#define MCLD_SCRIPT_GROUPCMD_H_

#include "mcld/Script/ScriptCommand.h"

namespace mcld {

class InputTree;
class InputBuilder;
class GroupReader;
class LinkerConfig;
class StringList;

/** \class GroupCmd
 *  \brief This class defines the interfaces to Group command.
 */

class GroupCmd : public ScriptCommand {
 public:
  GroupCmd(StringList& pStringList,
           InputTree& pInputTree,
           InputBuilder& pBuilder,
           GroupReader& m_GroupReader,
           const LinkerConfig& pConfig);
  ~GroupCmd();

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::GROUP;
  }

  void activate(Module& pModule);

 private:
  StringList& m_StringList;
  InputTree& m_InputTree;
  InputBuilder& m_Builder;
  GroupReader& m_GroupReader;
  const LinkerConfig& m_Config;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_GROUPCMD_H_
