//===- CommandAction.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_COMMANDACTION_H_
#define MCLD_MC_COMMANDACTION_H_

#include "mcld/MC/InputAction.h"
#include "mcld/Script/ScriptFile.h"
#include "mcld/Support/Path.h"

#include <string>

namespace mcld {

class SearchDirs;
class InputBuilder;
class LinkerConfig;

//===----------------------------------------------------------------------===//
// Derived InputAction
//===----------------------------------------------------------------------===//
/// InputFileAction
class InputFileAction : public InputAction {
 public:
  InputFileAction(unsigned int pPosition, const sys::fs::Path& pPath);

  InputFileAction(unsigned int pPosition, const char* pPath);

  const sys::fs::Path& path() const { return m_Path; }

  bool activate(InputBuilder&) const;

 private:
  sys::fs::Path m_Path;
};

/// NamespecAction
class NamespecAction : public InputAction {
 public:
  NamespecAction(unsigned int pPosition,
                 const std::string& pNamespec,
                 const SearchDirs& pSearchDirs);

  const std::string& namespec() const { return m_Namespec; }

  bool activate(InputBuilder&) const;

 private:
  std::string m_Namespec;
  const SearchDirs& m_SearchDirs;
};

/// BitcodeAction
class BitcodeAction : public InputAction {
 public:
  BitcodeAction(unsigned int pPosition, const sys::fs::Path& pPath);

  const sys::fs::Path& path() const { return m_Path; }

  bool activate(InputBuilder&) const;

 private:
  sys::fs::Path m_Path;
};

/// StartGroupAction
class StartGroupAction : public InputAction {
 public:
  explicit StartGroupAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// EndGroupAction
class EndGroupAction : public InputAction {
 public:
  explicit EndGroupAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// WholeArchiveAction
class WholeArchiveAction : public InputAction {
 public:
  explicit WholeArchiveAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// NoWholeArchiveAction
class NoWholeArchiveAction : public InputAction {
 public:
  explicit NoWholeArchiveAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// AsNeededAction
class AsNeededAction : public InputAction {
 public:
  explicit AsNeededAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// NoAsNeededAction
class NoAsNeededAction : public InputAction {
 public:
  explicit NoAsNeededAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// AddNeededAction
class AddNeededAction : public InputAction {
 public:
  explicit AddNeededAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// NoAddNeededAction
class NoAddNeededAction : public InputAction {
 public:
  explicit NoAddNeededAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// BDynamicAction
class BDynamicAction : public InputAction {
 public:
  explicit BDynamicAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// BStaticAction
class BStaticAction : public InputAction {
 public:
  explicit BStaticAction(unsigned int pPosition);

  bool activate(InputBuilder&) const;
};

/// DefSymAction
class DefSymAction : public InputAction {
 public:
  explicit DefSymAction(unsigned int pPosition, const std::string& pAssignment);

  bool activate(InputBuilder&) const;

  const std::string& assignment() const { return m_Assignment; }

 private:
  const std::string& m_Assignment;
};

/// ScriptAction
class ScriptAction : public InputAction {
 public:
  ScriptAction(unsigned int pPosition,
               const std::string& pFileName,
               ScriptFile::Kind pKind,
               const SearchDirs& pSearchDirs);

  bool activate(InputBuilder&) const;

  const std::string& filename() const { return m_FileName; }

  ScriptFile::Kind kind() const { return m_Kind; }

 private:
  std::string m_FileName;
  ScriptFile::Kind m_Kind;
  const SearchDirs& m_SearchDirs;
};

}  // namespace mcld

#endif  // MCLD_MC_COMMANDACTION_H_
