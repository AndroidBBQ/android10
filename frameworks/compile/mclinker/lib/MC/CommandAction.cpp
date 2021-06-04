//===- CommandAction.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/CommandAction.h"

#include "mcld/LinkerConfig.h"
#include "mcld/MC/Attribute.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/MC/SearchDirs.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/FileSystem.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// Derived Positional Option
//===----------------------------------------------------------------------===//
// InputFileAction
//===----------------------------------------------------------------------===//
InputFileAction::InputFileAction(unsigned int pPosition,
                                 const sys::fs::Path& pPath)
    : InputAction(pPosition), m_Path(pPath) {
}

InputFileAction::InputFileAction(unsigned int pPosition,
                                 const char* pPath)
    : InputAction(pPosition), m_Path(pPath) {
}

bool InputFileAction::activate(InputBuilder& pBuilder) const {
  pBuilder.createNode<InputTree::Positional>(path().stem().native(), path());
  return true;
}

//===----------------------------------------------------------------------===//
// NamespecAction
//===----------------------------------------------------------------------===//
NamespecAction::NamespecAction(unsigned int pPosition,
                               const std::string& pNamespec,
                               const SearchDirs& pSearchDirs)
    : InputAction(pPosition), m_Namespec(pNamespec), m_SearchDirs(pSearchDirs) {
}

bool NamespecAction::activate(InputBuilder& pBuilder) const {
  const sys::fs::Path* path = NULL;
  // find out the real path of the namespec.
  if (pBuilder.getConstraint().isSharedSystem()) {
    // In the system with shared object support, we can find both archive
    // and shared object.

    if (pBuilder.getAttributes().isStatic()) {
      // with --static, we must search an archive.
      path = m_SearchDirs.find(namespec(), Input::Archive);
    } else {
      // otherwise, with --Bdynamic, we can find either an archive or a
      // shared object.
      path = m_SearchDirs.find(namespec(), Input::DynObj);
    }
  } else {
    // In the system without shared object support, we only look for an archive
    path = m_SearchDirs.find(namespec(), Input::Archive);
  }

  if (path == NULL) {
    fatal(diag::err_cannot_find_namespec) << namespec();
    return false;
  }

  pBuilder.createNode<InputTree::Positional>(namespec(), *path);
  return true;
}

//===----------------------------------------------------------------------===//
// BitcodeAction
//===----------------------------------------------------------------------===//
BitcodeAction::BitcodeAction(unsigned int pPosition, const sys::fs::Path& pPath)
    : InputAction(pPosition), m_Path(pPath) {
}

bool BitcodeAction::activate(InputBuilder& pBuilder) const {
  pBuilder.createNode<InputTree::Positional>(
      "bitcode", path(), Input::External);
  return true;
}

//===----------------------------------------------------------------------===//
// StartGroupAction
//===----------------------------------------------------------------------===//
StartGroupAction::StartGroupAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool StartGroupAction::activate(InputBuilder& pBuilder) const {
  if (pBuilder.isInGroup()) {
    fatal(diag::fatal_forbid_nest_group);
    return false;
  }
  pBuilder.enterGroup();
  return true;
}

//===----------------------------------------------------------------------===//
// EndGroupAction
//===----------------------------------------------------------------------===//
EndGroupAction::EndGroupAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool EndGroupAction::activate(InputBuilder& pBuilder) const {
  pBuilder.exitGroup();
  return true;
}

//===----------------------------------------------------------------------===//
// WholeArchiveAction
//===----------------------------------------------------------------------===//
WholeArchiveAction::WholeArchiveAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool WholeArchiveAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().setWholeArchive();
  return true;
}

//===----------------------------------------------------------------------===//
// NoWholeArchiveAction
//===----------------------------------------------------------------------===//
NoWholeArchiveAction::NoWholeArchiveAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool NoWholeArchiveAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().unsetWholeArchive();
  return true;
}

//===----------------------------------------------------------------------===//
// AsNeededAction
//===----------------------------------------------------------------------===//
AsNeededAction::AsNeededAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool AsNeededAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().setAsNeeded();
  return true;
}

//===----------------------------------------------------------------------===//
// NoAsNeededAction
//===----------------------------------------------------------------------===//
NoAsNeededAction::NoAsNeededAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool NoAsNeededAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().unsetAsNeeded();
  return true;
}

//===----------------------------------------------------------------------===//
// AddNeededAction
//===----------------------------------------------------------------------===//
AddNeededAction::AddNeededAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool AddNeededAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().setAddNeeded();
  return true;
}

//===----------------------------------------------------------------------===//
// NoAddNeededAction
//===----------------------------------------------------------------------===//
NoAddNeededAction::NoAddNeededAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool NoAddNeededAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().unsetAddNeeded();
  return true;
}

//===----------------------------------------------------------------------===//
// BDynamicAction
//===----------------------------------------------------------------------===//
BDynamicAction::BDynamicAction(unsigned int pPosition)
    : InputAction(pPosition) {
}

bool BDynamicAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().setDynamic();
  return true;
}

//===----------------------------------------------------------------------===//
// BStaticAction
//===----------------------------------------------------------------------===//
BStaticAction::BStaticAction(unsigned int pPosition) : InputAction(pPosition) {
}

bool BStaticAction::activate(InputBuilder& pBuilder) const {
  pBuilder.getAttributes().setStatic();
  return true;
}

//===----------------------------------------------------------------------===//
// DefSymAction
//===----------------------------------------------------------------------===//
DefSymAction::DefSymAction(unsigned int pPosition,
                           const std::string& pAssignment)
    : InputAction(pPosition), m_Assignment(pAssignment) {
}

bool DefSymAction::activate(InputBuilder& pBuilder) const {
  pBuilder.createNode<InputTree::Positional>("defsym", sys::fs::Path("NAN"));
  Input* input = *pBuilder.getCurrentNode();
  pBuilder.setContext(*input, false);

  // FIXME
  void* base = static_cast<void*>(const_cast<char*>(m_Assignment.data()));
  pBuilder.setMemory(*input, base, m_Assignment.size());
  return true;
}

//===----------------------------------------------------------------------===//
// ScriptAction
//===----------------------------------------------------------------------===//
ScriptAction::ScriptAction(unsigned int pPosition,
                           const std::string& pFileName,
                           ScriptFile::Kind pKind,
                           const SearchDirs& pSearchDirs)
    : InputAction(pPosition),
      m_FileName(pFileName),
      m_Kind(pKind),
      m_SearchDirs(pSearchDirs) {
}

bool ScriptAction::activate(InputBuilder& pBuilder) const {
  sys::fs::Path path(m_FileName);

  if (!exists(path)) {
    const sys::fs::Path* res = m_SearchDirs.find(m_FileName, Input::Script);
    if (res == NULL) {
      switch (m_Kind) {
        case ScriptFile::LDScript:
          fatal(diag::err_cannot_find_scriptfile) << "linker script"
                                                  << m_FileName;
          break;
        case ScriptFile::VersionScript:
          fatal(diag::err_cannot_find_scriptfile) << "version script"
                                                  << m_FileName;
          break;
        case ScriptFile::DynamicList:
          fatal(diag::err_cannot_find_scriptfile) << "dynamic list"
                                                  << m_FileName;
          break;
        default:
          break;
      }
      return false;
    }
    path.assign(res->native());
  }

  pBuilder.createNode<InputTree::Positional>(path.stem().native(), path);

  return true;
}

}  // namespace mcld
