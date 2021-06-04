//===- BinaryOp.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_BINARYOP_H_
#define MCLD_SCRIPT_BINARYOP_H_

#include "mcld/Script/Operator.h"

#include <cstddef>

namespace mcld {

class Operand;
class IntOperand;
class Module;
class TargetLDBackend;

/** \class BinaryOP
 *  \brief This class defines the interfaces to an binary operator token.
 */

template <Operator::Type TYPE>
class BinaryOp : public Operator {
 private:
  friend class Operator;

  BinaryOp() : Operator(Operator::BINARY, TYPE), m_Size(0) {
    m_pOperand[0] = m_pOperand[1] = NULL;
  }

 public:
  ~BinaryOp() {}

  IntOperand* eval(const Module& pModule, const TargetLDBackend& pBackend);

  void appendOperand(Operand* pOperand) {
    m_pOperand[m_Size++] = pOperand;
    if (m_Size == 2)
      m_Size = 0;
  }

 private:
  size_t m_Size;
  Operand* m_pOperand[2];
};

template <>
IntOperand* BinaryOp<Operator::MUL>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::DIV>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::MOD>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::ADD>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::SUB>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::LSHIFT>::eval(const Module&,
                                             const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::RSHIFT>::eval(const Module&,
                                             const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::LT>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::LE>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::GT>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::GE>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::EQ>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::NE>::eval(const Module&, const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::BITWISE_AND>::eval(const Module&,
                                                  const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::BITWISE_XOR>::eval(const Module&,
                                                  const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::BITWISE_OR>::eval(const Module&,
                                                 const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::LOGICAL_AND>::eval(const Module&,
                                                  const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::LOGICAL_OR>::eval(const Module&,
                                                 const TargetLDBackend&);

template <>
IntOperand* BinaryOp<Operator::ALIGN>::eval(const Module&,
                                            const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::DATA_SEGMENT_RELRO_END>::eval(
    const Module&,
    const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::MAX>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::MIN>::eval(const Module&,
                                          const TargetLDBackend&);
template <>
IntOperand* BinaryOp<Operator::SEGMENT_START>::eval(const Module&,
                                                    const TargetLDBackend&);

}  // namespace mcld

#endif  // MCLD_SCRIPT_BINARYOP_H_
