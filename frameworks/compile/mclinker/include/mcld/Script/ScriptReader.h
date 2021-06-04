//===- ScriptReader.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_SCRIPTREADER_H_
#define MCLD_SCRIPT_SCRIPTREADER_H_

#include "mcld/LD/LDReader.h"

namespace mcld {

class ArchiveReader;
class DynObjReader;
class GroupReader;
class Input;
class LinkerConfig;
class LinkerScript;
class Module;
class ObjectReader;
class ScriptFile;
class TargetLDBackend;

class ScriptReader : public LDReader {
 public:
  ScriptReader(ObjectReader& pObjectReader,
               ArchiveReader& pArchiveReader,
               DynObjReader& pDynObjReader,
               GroupReader& pGroupReader);

  ~ScriptReader();

  /// readScript
  bool readScript(const LinkerConfig& pConfig, ScriptFile& pScriptFile);

  /// isMyFormat
  bool isMyFormat(Input& pInput, bool& pContinue) const;

 private:
  ObjectReader& m_ObjectReader;
  ArchiveReader& m_ArchiveReader;
  DynObjReader& m_DynObjReader;
  GroupReader& m_GroupReader;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_SCRIPTREADER_H_
