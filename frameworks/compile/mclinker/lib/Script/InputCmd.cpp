//===- InputCmd.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/InputCmd.h"

#include "mcld/LD/Archive.h"
#include "mcld/LD/ArchiveReader.h"
#include "mcld/LD/DynObjReader.h"
#include "mcld/LD/ObjectReader.h"
#include "mcld/MC/Attribute.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/Script/InputToken.h"
#include "mcld/Script/StringList.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/Path.h"
#include "mcld/Support/raw_ostream.h"
#include "mcld/InputTree.h"
#include "mcld/LinkerScript.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Module.h"

#include <llvm/Support/Casting.h>

#include <cassert>
#include <iostream>

namespace mcld {

//===----------------------------------------------------------------------===//
// InputCmd
//===----------------------------------------------------------------------===//
InputCmd::InputCmd(StringList& pStringList,
                   InputTree& pInputTree,
                   InputBuilder& pBuilder,
                   ObjectReader& pObjectReader,
                   ArchiveReader& pArchiveReader,
                   DynObjReader& pDynObjReader,
                   const LinkerConfig& pConfig)
    : ScriptCommand(ScriptCommand::INPUT),
      m_StringList(pStringList),
      m_InputTree(pInputTree),
      m_Builder(pBuilder),
      m_ObjectReader(pObjectReader),
      m_ArchiveReader(pArchiveReader),
      m_DynObjReader(pDynObjReader),
      m_Config(pConfig) {
}

InputCmd::~InputCmd() {
}

void InputCmd::dump() const {
  mcld::outs() << "INPUT ( ";
  bool prev = false, cur = false;
  for (StringList::const_iterator it = m_StringList.begin(),
                                  ie = m_StringList.end();
       it != ie;
       ++it) {
    assert((*it)->kind() == StrToken::Input);
    InputToken* input = llvm::cast<InputToken>(*it);
    cur = input->asNeeded();
    if (!prev && cur)
      mcld::outs() << "AS_NEEDED ( ";
    else if (prev && !cur)
      mcld::outs() << " )";

    if (input->type() == InputToken::NameSpec)
      mcld::outs() << "-l";
    mcld::outs() << input->name() << " ";

    prev = cur;
  }

  if (!m_StringList.empty() && prev)
    mcld::outs() << " )";

  mcld::outs() << " )\n";
}

void InputCmd::activate(Module& pModule) {
  LinkerScript& script = pModule.getScript();
  // construct the INPUT tree
  m_Builder.setCurrentTree(m_InputTree);

  bool is_begin_marked = false;
  InputTree::iterator input_begin;

  for (StringList::const_iterator it = m_StringList.begin(),
                                  ie = m_StringList.end();
       it != ie;
       ++it) {
    assert((*it)->kind() == StrToken::Input);
    InputToken* token = llvm::cast<InputToken>(*it);
    if (token->asNeeded())
      m_Builder.getAttributes().setAsNeeded();
    else
      m_Builder.getAttributes().unsetAsNeeded();

    switch (token->type()) {
      case InputToken::File: {
        sys::fs::Path path;

        // 1. Looking for file in the sysroot prefix, if a sysroot prefix is
        // configured and the filename starts with '/'
        if (script.hasSysroot() &&
            (token->name().size() > 0 && token->name()[0] == '/')) {
          path = script.sysroot();
          path.append(token->name());
        } else {
          // 2. Try to open the file in CWD
          path.assign(token->name());
          if (!sys::fs::exists(path)) {
            // 3. Search through the library search path
            sys::fs::Path* p =
                script.directories().find(token->name(), Input::Script);
            if (p != NULL)
              path = *p;
          }
        }

        if (!sys::fs::exists(path))
          fatal(diag::err_cannot_open_input) << path.filename() << path;

        m_Builder.createNode<InputTree::Positional>(
            path.filename().native(), path, Input::Unknown);
        break;
      }
      case InputToken::NameSpec: {
        const sys::fs::Path* path = NULL;
        // find out the real path of the namespec.
        if (m_Builder.getConstraint().isSharedSystem()) {
          // In the system with shared object support, we can find both archive
          // and shared object.
          if (m_Builder.getAttributes().isStatic()) {
            // with --static, we must search an archive.
            path = script.directories().find(token->name(), Input::Archive);
          } else {
            // otherwise, with --Bdynamic, we can find either an archive or a
            // shared object.
            path = script.directories().find(token->name(), Input::DynObj);
          }
        } else {
          // In the system without shared object support, only look for an
          // archive
          path = script.directories().find(token->name(), Input::Archive);
        }

        if (path == NULL)
          fatal(diag::err_cannot_find_namespec) << token->name();

        m_Builder.createNode<InputTree::Positional>(
            token->name(), *path, Input::Unknown);
        break;
      }
      default:
        assert(0 && "Invalid script token in INPUT!");
        break;
    }  // end of switch

    InputTree::iterator input = m_Builder.getCurrentNode();
    if (!is_begin_marked) {
      input_begin = input;
      is_begin_marked = true;
    }
    assert(*input != NULL);
    if (!m_Builder.setMemory(**input,
                             FileHandle::OpenMode(FileHandle::ReadOnly),
                             FileHandle::Permission(FileHandle::System))) {
      error(diag::err_cannot_open_input) << (*input)->name()
                                         << (*input)->path();
    }
    m_Builder.setContext(**input);
  }

  for (InputTree::iterator input = input_begin, ie = m_InputTree.end();
       input != ie;
       ++input) {
    bool doContinue = false;
    if (m_ObjectReader.isMyFormat(**input, doContinue)) {
      (*input)->setType(Input::Object);
      m_ObjectReader.readHeader(**input);
      m_ObjectReader.readSections(**input);
      m_ObjectReader.readSymbols(**input);
      pModule.getObjectList().push_back(*input);
    } else if (doContinue && m_DynObjReader.isMyFormat(**input, doContinue)) {
      (*input)->setType(Input::DynObj);
      m_DynObjReader.readHeader(**input);
      m_DynObjReader.readSymbols(**input);
      pModule.getLibraryList().push_back(*input);
    } else if (doContinue && m_ArchiveReader.isMyFormat(**input, doContinue)) {
      (*input)->setType(Input::Archive);
      if (m_Config.options().isInExcludeLIBS(**input)) {
        (*input)->setNoExport();
      }
      Archive archive(**input, m_Builder);
      m_ArchiveReader.readArchive(m_Config, archive);
      if (archive.numOfObjectMember() > 0) {
        m_InputTree.merge<InputTree::Inclusive>(input, archive.inputs());
      }
    } else {
      if (m_Config.options().warnMismatch())
        warning(diag::warn_unrecognized_input_file)
            << (*input)->path() << m_Config.targets().triple().str();
    }
  }
}

}  // namespace mcld
