//===- NullaryOp.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_NULLARYOP_H_
#define MCLD_SCRIPT_NULLARYOP_H_

#include "mcld/Script/Operator.h"

#include <cassert>

namespace mcld {

class Operand;
class IntOperand;
class Module;
class TargetLDBackend;

/** \class NullaryOp
 *  \brief This class defines the interfaces to an nullary operator token.
 */

template <Operator::Type TYPE>
class NullaryOp : public Operator {
 private:
  friend class Operator;

  NullaryOp() : Operator(Operator::NULLARY, TYPE) {}

 public:
  ~NullaryOp() {}

  IntOperand* eval(const Module& pModule, const TargetLDBackend& pBackend);

  void appendOperand(Operand* pOperand) { assert(0); }
};

template <>
IntOperand* NullaryOp<Operator::SIZEOF_HEADERS>::eval(const Module&,
                                                      const TargetLDBackend&);
template <>
IntOperand* NullaryOp<Operator::MAXPAGESIZE>::eval(const Module&,
                                                   const TargetLDBackend&);

template <>
IntOperand* NullaryOp<Operator::COMMONPAGESIZE>::eval(const Module&,
                                                      const TargetLDBackend&);

}  // namespace mcld

#endif  // MCLD_SCRIPT_NULLARYOP_H_
