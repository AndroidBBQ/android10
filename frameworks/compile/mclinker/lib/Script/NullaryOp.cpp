//===- NullaryOp.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/NullaryOp.h"

#include "mcld/Script/Operand.h"
#include "mcld/Target/TargetLDBackend.h"

namespace mcld {
//===----------------------------------------------------------------------===//
// NullaryOp
//===----------------------------------------------------------------------===//
template <>
IntOperand* NullaryOp<Operator::SIZEOF_HEADERS>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(pBackend.sectionStartOffset());
  return res;
}

template <>
IntOperand* NullaryOp<Operator::MAXPAGESIZE>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(pBackend.abiPageSize());
  return res;
}

template <>
IntOperand* NullaryOp<Operator::COMMONPAGESIZE>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(pBackend.commonPageSize());
  return res;
}

}  // namespace mcld
