//===- TernaryOp.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/TernaryOp.h"

#include "mcld/ADT/SizeTraits.h"
#include "mcld/Script/Operand.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// TernaryOp
//===----------------------------------------------------------------------===//
template <>
IntOperand* TernaryOp<Operator::TERNARY_IF>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  if (m_pOperand[0]->value())
    res->setValue(m_pOperand[1]->value());
  else
    res->setValue(m_pOperand[2]->value());
  return res;
}

/* DATA_SEGMENT_ALIGN(maxpagesize, commonpagesize) */
template <>
IntOperand* TernaryOp<Operator::DATA_SEGMENT_ALIGN>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  /* This is equivalent to either
       (ALIGN(maxpagesize) + (. & (maxpagesize - 1)))
     or
       (ALIGN(maxpagesize) + (. & (maxpagesize - commonpagesize)))
   */
  IntOperand* res = result();
  uint64_t dot = m_pOperand[0]->value();
  uint64_t maxPageSize = m_pOperand[1]->value();
  uint64_t commonPageSize = m_pOperand[2]->value();
  uint64_t form1 = 0, form2 = 0;

  alignAddress(dot, maxPageSize);

  form1 = dot + (dot & (maxPageSize - 1));
  form2 = dot + (dot & (maxPageSize - commonPageSize));

  if (form1 <= form2)
    res->setValue(form1);
  else
    res->setValue(form2);
  return res;
}

}  // namespace mcld
