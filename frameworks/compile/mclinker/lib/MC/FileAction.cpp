//===- FileAction.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/FileAction.h"

#include "mcld/MC/Input.h"
#include "mcld/MC/InputBuilder.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// ContextAction
//===----------------------------------------------------------------------===//
ContextAction::ContextAction(unsigned int pPosition) : InputAction(pPosition) {
}

bool ContextAction::activate(InputBuilder& pBuilder) const {
  Input* input = *pBuilder.getCurrentNode();

  if (input->hasContext())
    return false;

  // already got type - for example, bitcode
  if (input->type() == Input::Script || input->type() == Input::Object ||
      input->type() == Input::DynObj || input->type() == Input::Archive)
    return false;

  return pBuilder.setContext(*input);
}

//===----------------------------------------------------------------------===//
// MemoryAreaAction
//===----------------------------------------------------------------------===//
MemoryAreaAction::MemoryAreaAction(unsigned int pPosition,
                                   FileHandle::OpenModeEnum pMode,
                                   FileHandle::PermissionEnum pPerm)
    : InputAction(pPosition), m_Mode(pMode), m_Permission(pPerm) {
}

bool MemoryAreaAction::activate(InputBuilder& pBuilder) const {
  Input* input = *pBuilder.getCurrentNode();

  if (input->hasMemArea())
    return false;

  // already got type - for example, bitcode
  if (input->type() == Input::Script || input->type() == Input::Object ||
      input->type() == Input::DynObj || input->type() == Input::Archive)
    return false;

  return pBuilder.setMemory(*input, m_Mode, m_Permission);
}

}  // namespace mcld
