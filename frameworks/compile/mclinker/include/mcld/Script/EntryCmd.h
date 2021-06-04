//===- EntryCmd.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_ENTRYCMD_H_
#define MCLD_SCRIPT_ENTRYCMD_H_

#include "mcld/Script/ScriptCommand.h"

#include <string>

namespace mcld {

class Module;

/** \class EntryCmd
 *  \brief This class defines the interfaces to Entry command.
 */

class EntryCmd : public ScriptCommand {
 public:
  explicit EntryCmd(const std::string& pEntry);
  ~EntryCmd();

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::ENTRY;
  }

  void activate(Module& pModule);

 private:
  std::string m_Entry;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_ENTRYCMD_H_
