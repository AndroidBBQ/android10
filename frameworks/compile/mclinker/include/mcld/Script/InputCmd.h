//===- InputCmd.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_INPUTCMD_H_
#define MCLD_SCRIPT_INPUTCMD_H_

#include "mcld/Script/ScriptCommand.h"

namespace mcld {

class ArchiveReader;
class DynObjReader;
class InputBuilder;
class InputTree;
class LinkerConfig;
class ObjectReader;
class StringList;

/** \class InputCmd
 *  \brief This class defines the interfaces to Input command.
 */

class InputCmd : public ScriptCommand {
 public:
  InputCmd(StringList& pStringList,
           InputTree& pInputTree,
           InputBuilder& pBuilder,
           ObjectReader& pObjectReader,
           ArchiveReader& pArchiveReader,
           DynObjReader& pDynObjReader,
           const LinkerConfig& pConfig);
  ~InputCmd();

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::INPUT;
  }

  void activate(Module& pModule);

 private:
  StringList& m_StringList;
  InputTree& m_InputTree;
  InputBuilder& m_Builder;
  ObjectReader& m_ObjectReader;
  ArchiveReader& m_ArchiveReader;
  DynObjReader& m_DynObjReader;
  const LinkerConfig& m_Config;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_INPUTCMD_H_
