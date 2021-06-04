//===- OutputArchCmd.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OUTPUTARCHCMD_H_
#define MCLD_SCRIPT_OUTPUTARCHCMD_H_

#include "mcld/Script/ScriptCommand.h"

#include <string>

namespace mcld {

class Module;

/** \class OutputArchCmd
 *  \brief This class defines the interfaces to OutputArch command.
 */

class OutputArchCmd : public ScriptCommand {
 public:
  explicit OutputArchCmd(const std::string& pArch);
  ~OutputArchCmd();

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::OUTPUT_ARCH;
  }

  void activate(Module& pModule);

 private:
  std::string m_Arch;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_OUTPUTARCHCMD_H_
