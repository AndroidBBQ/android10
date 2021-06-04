//===- ScriptCommand.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_SCRIPTCOMMAND_H_
#define MCLD_SCRIPT_SCRIPTCOMMAND_H_

namespace mcld {

class Module;

/** \class ScriptCommand
 *  \brief This class defines the interfaces to a script command.
 */
class ScriptCommand {
 public:
  enum Kind {
    ASSERT,
    ASSIGNMENT,
    ENTRY,
    GROUP,
    INPUT,
    INPUT_SECT_DESC,
    OUTPUT,
    OUTPUT_ARCH,
    OUTPUT_FORMAT,
    SEARCH_DIR,
    OUTPUT_SECT_DESC,
    SECTIONS
  };

 protected:
  explicit ScriptCommand(Kind pKind) : m_Kind(pKind) {}

 public:
  virtual ~ScriptCommand() = 0;

  virtual void dump() const = 0;

  virtual void activate(Module&) = 0;

  Kind getKind() const { return m_Kind; }

 private:
  Kind m_Kind;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_SCRIPTCOMMAND_H_
