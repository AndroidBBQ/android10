//===- OutputCmd.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OUTPUTCMD_H_
#define MCLD_SCRIPT_OUTPUTCMD_H_

#include "mcld/Script/ScriptCommand.h"

#include <string>

namespace mcld {

class Module;

/** \class OutputCmd
 *  \brief This class defines the interfaces to Output command.
 */

class OutputCmd : public ScriptCommand {
 public:
  explicit OutputCmd(const std::string& pOutputFile);

  ~OutputCmd();

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::OUTPUT;
  }

  void activate(Module& pModule);

 private:
  std::string m_OutputFile;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_OUTPUTCMD_H_
