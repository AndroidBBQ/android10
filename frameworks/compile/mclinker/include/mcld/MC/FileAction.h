//===- FileAction.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_FILEACTION_H_
#define MCLD_MC_FILEACTION_H_
#include "mcld/MC/InputAction.h"
#include "mcld/Support/FileHandle.h"

namespace mcld {

class ContextFactory;

/** \class ContextAction
 *  \brief ContextAction is a command object to create input's LDContext.
 */
class ContextAction : public InputAction {
 public:
  explicit ContextAction(unsigned int pPosition);

  bool activate(InputBuilder& pBuilder) const;
};

/** \class MemoryAreaAction
 *  \brief MemoryAreaAction is a command object to create input's MemoryArea.
 */
class MemoryAreaAction : public InputAction {
 public:
  MemoryAreaAction(unsigned int pPosition,
                   FileHandle::OpenModeEnum pMode,
                   FileHandle::PermissionEnum pPerm = FileHandle::System);

  bool activate(InputBuilder& pBuilder) const;

 private:
  FileHandle::OpenMode m_Mode;
  FileHandle::Permission m_Permission;
};

}  // namespace mcld

#endif  // MCLD_MC_FILEACTION_H_
