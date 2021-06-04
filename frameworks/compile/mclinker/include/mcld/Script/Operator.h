//===- Operator.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OPERATOR_H_
#define MCLD_SCRIPT_OPERATOR_H_

#include "mcld/Script/ExprToken.h"
#include <llvm/Support/DataTypes.h>

namespace mcld {

class IntOperand;
class Module;
class Operand;
class TargetLDBackend;

/** \class Operator
 *  \brief This class defines the interfaces to an operator token.
 */

class Operator : public ExprToken {
 public:
  enum Arity { NULLARY, UNARY, BINARY, TERNARY };

  enum Type {
    /* arithmetic operator */
    UNARY_PLUS = 0,
    UNARY_MINUS = 1,
    LOGICAL_NOT = 2,
    BITWISE_NOT = 3,
    MUL = 4,
    DIV = 5,
    MOD = 6,
    ADD = 7,
    SUB = 8,
    LSHIFT = 9,
    RSHIFT = 10,
    LT = 11,
    LE = 12,
    GT = 13,
    GE = 14,
    EQ = 15,
    NE = 16,
    BITWISE_AND = 17,
    BITWISE_XOR = 18,
    BITWISE_OR = 19,
    LOGICAL_AND = 20,
    LOGICAL_OR = 21,
    TERNARY_IF = 22,
    ASSIGN = 23,
    ADD_ASSIGN = 24,
    SUB_ASSIGN = 25,
    MUL_ASSIGN = 26,
    DIV_ASSIGN = 27,
    AND_ASSIGN = 28,
    OR_ASSIGN = 29,
    LS_ASSIGN = 30,
    RS_ASSIGN = 31,
    /* function */
    ABSOLUTE = 32,
    ADDR = 33,
    ALIGN = 34,
    ALIGNOF = 35,
    BLOCK = 36,
    DATA_SEGMENT_ALIGN = 37,
    DATA_SEGMENT_END = 38,
    DATA_SEGMENT_RELRO_END = 39,
    DEFINED = 40,
    LENGTH = 41,
    LOADADDR = 42,
    MAX = 43,
    MIN = 44,
    NEXT = 45,
    ORIGIN = 46,
    SEGMENT_START = 47,
    SIZEOF = 48,
    SIZEOF_HEADERS = 49,
    MAXPAGESIZE = 50,
    COMMONPAGESIZE = 51
  };

  static const char* OpNames[];

 protected:
  Operator(Arity pArity, Type pType);

  const IntOperand* result() const { return m_pIntOperand; }
  IntOperand* result() { return m_pIntOperand; }

 public:
  virtual ~Operator();

  Arity arity() const { return m_Arity; }

  Type type() const { return m_Type; }

  virtual void dump() const;

  virtual IntOperand* eval(const Module& pModule,
                           const TargetLDBackend& pBackend) = 0;

  virtual void appendOperand(Operand* pOperand) = 0;

  static bool classof(const ExprToken* pToken) {
    return pToken->kind() == ExprToken::OPERATOR;
  }

  template <Operator::Type TYPE>
  static Operator& create();

 private:
  Arity m_Arity;
  Type m_Type;
  IntOperand* m_pIntOperand;
};

/* Nullary operator */
template <>
Operator& Operator::create<Operator::SIZEOF_HEADERS>();
template <>
Operator& Operator::create<Operator::MAXPAGESIZE>();
template <>
Operator& Operator::create<Operator::COMMONPAGESIZE>();

/* Unary operator */
template <>
Operator& Operator::create<Operator::UNARY_PLUS>();
template <>
Operator& Operator::create<Operator::UNARY_MINUS>();
template <>
Operator& Operator::create<Operator::LOGICAL_NOT>();
template <>
Operator& Operator::create<Operator::BITWISE_NOT>();

template <>
Operator& Operator::create<Operator::ABSOLUTE>();
template <>
Operator& Operator::create<Operator::ADDR>();
template <>
Operator& Operator::create<Operator::ALIGNOF>();
template <>
Operator& Operator::create<Operator::DATA_SEGMENT_END>();
template <>
Operator& Operator::create<Operator::DEFINED>();
template <>
Operator& Operator::create<Operator::LENGTH>();
template <>
Operator& Operator::create<Operator::LOADADDR>();
template <>
Operator& Operator::create<Operator::NEXT>();
template <>
Operator& Operator::create<Operator::ORIGIN>();
template <>
Operator& Operator::create<Operator::SIZEOF>();

/* Binary operator */
template <>
Operator& Operator::create<Operator::MUL>();
template <>
Operator& Operator::create<Operator::DIV>();
template <>
Operator& Operator::create<Operator::MOD>();
template <>
Operator& Operator::create<Operator::ADD>();
template <>
Operator& Operator::create<Operator::SUB>();
template <>
Operator& Operator::create<Operator::LSHIFT>();
template <>
Operator& Operator::create<Operator::RSHIFT>();
template <>
Operator& Operator::create<Operator::LT>();
template <>
Operator& Operator::create<Operator::LE>();
template <>
Operator& Operator::create<Operator::GT>();
template <>
Operator& Operator::create<Operator::GE>();
template <>
Operator& Operator::create<Operator::EQ>();
template <>
Operator& Operator::create<Operator::NE>();
template <>
Operator& Operator::create<Operator::BITWISE_AND>();
template <>
Operator& Operator::create<Operator::BITWISE_XOR>();
template <>
Operator& Operator::create<Operator::BITWISE_OR>();
template <>
Operator& Operator::create<Operator::LOGICAL_AND>();
template <>
Operator& Operator::create<Operator::LOGICAL_OR>();

template <>
Operator& Operator::create<Operator::ALIGN>();
template <>
Operator& Operator::create<Operator::DATA_SEGMENT_RELRO_END>();
template <>
Operator& Operator::create<Operator::MAX>();
template <>
Operator& Operator::create<Operator::MIN>();
template <>
Operator& Operator::create<Operator::SEGMENT_START>();

/* Ternary operator */
template <>
Operator& Operator::create<Operator::TERNARY_IF>();

template <>
Operator& Operator::create<Operator::DATA_SEGMENT_ALIGN>();
}  // namespace mcld

#endif  // MCLD_SCRIPT_OPERATOR_H_
