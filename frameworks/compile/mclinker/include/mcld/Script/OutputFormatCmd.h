//===- OutputFormatCmd.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OUTPUTFORMATCMD_H_
#define MCLD_SCRIPT_OUTPUTFORMATCMD_H_

#include "mcld/Script/ScriptCommand.h"

#include <string>
#include <vector>

namespace mcld {

class Module;

/** \class OutputFormatCmd
 *  \brief This class defines the interfaces to OutputFormat command.
 */

class OutputFormatCmd : public ScriptCommand {
 public:
  typedef std::vector<std::string> FormatList;
  typedef FormatList::const_iterator const_iterator;
  typedef FormatList::iterator iterator;

 public:
  explicit OutputFormatCmd(const std::string& pFormat);
  OutputFormatCmd(const std::string& pDefault,
                  const std::string& pBig,
                  const std::string& pLittle);
  ~OutputFormatCmd();

  const_iterator begin() const { return m_FormatList.begin(); }
  iterator begin() { return m_FormatList.begin(); }
  const_iterator end() const { return m_FormatList.end(); }
  iterator end() { return m_FormatList.end(); }

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::OUTPUT_FORMAT;
  }

  void activate(Module& pModule);

 private:
  FormatList m_FormatList;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_OUTPUTFORMATCMD_H_
