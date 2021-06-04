//===- Operator.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/Operator.h"

#include "mcld/Script/BinaryOp.h"
#include "mcld/Script/NullaryOp.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/UnaryOp.h"
#include "mcld/Script/TernaryOp.h"
#include "mcld/Support/raw_ostream.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// Operator
//===----------------------------------------------------------------------===//
const char* Operator::OpNames[] = {
    "+",                      "-",                  "!",
    "~",                      "*",                  "/",
    "%",                      "+",                  "-",
    "<<",                     ">>",                 "<",
    "<=",                     ">",                  ">=",
    "==",                     "!=",                 "&",
    "^",                      "|",                  "&&",
    "||",                     "?:",                 "=",
    "+=",                     "-=",                 "*=",
    "/=",                     "&=",                 "|=",
    "<<=",                    ">>=",                "ABSOLUTE",
    "ADDR",                   "ALIGN",              "ALIGNOF",
    "BLOCK",                  "DATA_SEGMENT_ALIGN", "DATA_SEGMENT_END",
    "DATA_SEGMENT_RELRO_END", "DEFINED",            "LENGTH",
    "LOADADDR",               "MAX",                "MIN",
    "NEXT",                   "ORIGIN",             "SEGMENT_START",
    "SIZEOF",                 "SIZEOF_HEADERS",     "MAXPAGESIZE",
    "COMMONPAGESIZE"};

Operator::Operator(Arity pArity, Type pType)
    : ExprToken(ExprToken::OPERATOR), m_Arity(pArity), m_Type(pType) {
  m_pIntOperand = IntOperand::create(0);
}

Operator::~Operator() {
}

void Operator::dump() const {
  mcld::outs() << OpNames[type()];
}

/* Nullary operator */
template <>
Operator& Operator::create<Operator::SIZEOF_HEADERS>() {
  static NullaryOp<Operator::SIZEOF_HEADERS> op;
  return op;
}

template <>
Operator& Operator::create<Operator::MAXPAGESIZE>() {
  static NullaryOp<Operator::MAXPAGESIZE> op;
  return op;
}

template <>
Operator& Operator::create<Operator::COMMONPAGESIZE>() {
  static NullaryOp<Operator::COMMONPAGESIZE> op;
  return op;
}

/* Unary operator */
template <>
Operator& Operator::create<Operator::UNARY_PLUS>() {
  static UnaryOp<Operator::UNARY_PLUS> op;
  return op;
}

template <>
Operator& Operator::create<Operator::UNARY_MINUS>() {
  static UnaryOp<Operator::UNARY_MINUS> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LOGICAL_NOT>() {
  static UnaryOp<Operator::LOGICAL_NOT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::BITWISE_NOT>() {
  static UnaryOp<Operator::BITWISE_NOT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ABSOLUTE>() {
  static UnaryOp<Operator::ABSOLUTE> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ADDR>() {
  static UnaryOp<Operator::ADDR> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ALIGNOF>() {
  static UnaryOp<Operator::ALIGNOF> op;
  return op;
}

template <>
Operator& Operator::create<Operator::DATA_SEGMENT_END>() {
  static UnaryOp<Operator::DATA_SEGMENT_END> op;
  return op;
}

template <>
Operator& Operator::create<Operator::DEFINED>() {
  static UnaryOp<Operator::DEFINED> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LENGTH>() {
  static UnaryOp<Operator::LENGTH> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LOADADDR>() {
  static UnaryOp<Operator::LOADADDR> op;
  return op;
}

template <>
Operator& Operator::create<Operator::NEXT>() {
  static UnaryOp<Operator::NEXT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ORIGIN>() {
  static UnaryOp<Operator::ORIGIN> op;
  return op;
}

template <>
Operator& Operator::create<Operator::SIZEOF>() {
  static UnaryOp<Operator::SIZEOF> op;
  return op;
}

/* Binary operator */
template <>
Operator& Operator::create<Operator::MUL>() {
  static BinaryOp<Operator::MUL> op;
  return op;
}

template <>
Operator& Operator::create<Operator::DIV>() {
  static BinaryOp<Operator::DIV> op;
  return op;
}

template <>
Operator& Operator::create<Operator::MOD>() {
  static BinaryOp<Operator::MOD> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ADD>() {
  static BinaryOp<Operator::ADD> op;
  return op;
}

template <>
Operator& Operator::create<Operator::SUB>() {
  static BinaryOp<Operator::SUB> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LSHIFT>() {
  static BinaryOp<Operator::LSHIFT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::RSHIFT>() {
  static BinaryOp<Operator::RSHIFT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LT>() {
  static BinaryOp<Operator::LT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LE>() {
  static BinaryOp<Operator::LE> op;
  return op;
}

template <>
Operator& Operator::create<Operator::GT>() {
  static BinaryOp<Operator::GT> op;
  return op;
}

template <>
Operator& Operator::create<Operator::GE>() {
  static BinaryOp<Operator::GE> op;
  return op;
}

template <>
Operator& Operator::create<Operator::EQ>() {
  static BinaryOp<Operator::EQ> op;
  return op;
}

template <>
Operator& Operator::create<Operator::NE>() {
  static BinaryOp<Operator::NE> op;
  return op;
}

template <>
Operator& Operator::create<Operator::BITWISE_AND>() {
  static BinaryOp<Operator::BITWISE_AND> op;
  return op;
}

template <>
Operator& Operator::create<Operator::BITWISE_XOR>() {
  static BinaryOp<Operator::BITWISE_XOR> op;
  return op;
}

template <>
Operator& Operator::create<Operator::BITWISE_OR>() {
  static BinaryOp<Operator::BITWISE_OR> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LOGICAL_AND>() {
  static BinaryOp<Operator::LOGICAL_AND> op;
  return op;
}

template <>
Operator& Operator::create<Operator::LOGICAL_OR>() {
  static BinaryOp<Operator::LOGICAL_OR> op;
  return op;
}

template <>
Operator& Operator::create<Operator::ALIGN>() {
  static BinaryOp<Operator::ALIGN> op;
  return op;
}

template <>
Operator& Operator::create<Operator::DATA_SEGMENT_RELRO_END>() {
  static BinaryOp<Operator::DATA_SEGMENT_RELRO_END> op;
  return op;
}

template <>
Operator& Operator::create<Operator::MAX>() {
  static BinaryOp<Operator::MAX> op;
  return op;
}

template <>
Operator& Operator::create<Operator::MIN>() {
  static BinaryOp<Operator::MIN> op;
  return op;
}

template <>
Operator& Operator::create<Operator::SEGMENT_START>() {
  static BinaryOp<Operator::SEGMENT_START> op;
  return op;
}

/* Ternary operator */
template <>
Operator& Operator::create<Operator::TERNARY_IF>() {
  static TernaryOp<Operator::TERNARY_IF> op;
  return op;
}

template <>
Operator& Operator::create<Operator::DATA_SEGMENT_ALIGN>() {
  static TernaryOp<Operator::DATA_SEGMENT_ALIGN> op;
  return op;
}

}  // namespace mcld
