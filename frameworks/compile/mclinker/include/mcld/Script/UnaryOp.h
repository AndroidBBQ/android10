//===- UnaryOp.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_UNARYOP_H_
#define MCLD_SCRIPT_UNARYOP_H_

#include "mcld/Script/Operator.h"

#include <cstddef>

namespace mcld {

class IntOperand;
class Module;
class Operand;
class TargetLDBackend;

/** \class UnaryOp
 *  \brief This class defines the interfaces to an unary operator token.
 */

template <Operator::Type TYPE>
class UnaryOp : public Operator {
 private:
  friend class Operator;

  UnaryOp() : Operator(Operator::UNARY, TYPE), m_pOperand(NULL) {}

 public:
  ~UnaryOp() {}

  IntOperand* eval(const Module& pModule, const TargetLDBackend& pBackend);

  void appendOperand(Operand* pOperand) { m_pOperand = pOperand; }

 private:
  Operand* m_pOperand;
};

template <>
IntOperand* UnaryOp<Operator::UNARY_PLUS>::eval(const Module&,
                                                const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::UNARY_MINUS>::eval(const Module&,
                                                 const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::LOGICAL_NOT>::eval(const Module&,
                                                 const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::BITWISE_NOT>::eval(const Module&,
                                                 const TargetLDBackend&);

template <>
IntOperand* UnaryOp<Operator::ABSOLUTE>::eval(const Module&,
                                              const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::ADDR>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::ALIGNOF>::eval(const Module&,
                                             const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::DATA_SEGMENT_END>::eval(const Module&,
                                                      const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::DEFINED>::eval(const Module&,
                                             const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::LENGTH>::eval(const Module&,
                                            const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::LOADADDR>::eval(const Module&,
                                              const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::NEXT>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::ORIGIN>::eval(const Module&,
                                            const TargetLDBackend&);
template <>
IntOperand* UnaryOp<Operator::SIZEOF>::eval(const Module&,
                                            const TargetLDBackend&);

}  // namespace mcld

#endif  // MCLD_SCRIPT_UNARYOP_H_
