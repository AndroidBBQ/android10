//===- InputAction.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_INPUTACTION_H_
#define MCLD_MC_INPUTACTION_H_

namespace mcld {

class SearchDirs;
class InputBuilder;

//===----------------------------------------------------------------------===//
// Base InputAction
//===----------------------------------------------------------------------===//
/** \class InputAction
 *  \brief InputAction is a command object to construct mcld::InputTree.
 */
class InputAction {
 protected:
  explicit InputAction(unsigned int pPosition);

 public:
  virtual ~InputAction();

  virtual bool activate(InputBuilder&) const = 0;

  unsigned int position() const { return m_Position; }

  bool operator<(const InputAction& pOther) const {
    return (position() < pOther.position());
  }

 private:
  InputAction();                               // DO_NOT_IMPLEMENT
  InputAction(const InputAction&);             // DO_NOT_IMPLEMENT
  InputAction& operator=(const InputAction&);  // DO_NOT_IMPLEMENT

 private:
  unsigned int m_Position;
};

}  // namespace mcld

#endif  // MCLD_MC_INPUTACTION_H_
