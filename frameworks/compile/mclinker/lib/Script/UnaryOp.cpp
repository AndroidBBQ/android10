//===- UnaryOp.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/UnaryOp.h"

#include "mcld/LD/LDSection.h"
#include "mcld/Object/SectionMap.h"
#include "mcld/Script/Operand.h"
#include "mcld/Module.h"

#include <llvm/Support/Casting.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// UnaryOp
//===----------------------------------------------------------------------===//
template <>
IntOperand* UnaryOp<Operator::UNARY_PLUS>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(+m_pOperand->value());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::UNARY_MINUS>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(-m_pOperand->value());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::LOGICAL_NOT>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(!m_pOperand->value());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::BITWISE_NOT>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(~m_pOperand->value());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::ABSOLUTE>::eval(const Module& pModule,
                                              const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::ADDR>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  const LDSection* sect = NULL;
  switch (m_pOperand->type()) {
    case Operand::SECTION:
      sect = pModule.getSection(llvm::cast<SectOperand>(m_pOperand)->name());
      break;
    case Operand::SECTION_DESC:
      sect =
          llvm::cast<SectDescOperand>(m_pOperand)->outputDesc()->getSection();
      break;
    default:
      assert(0);
      break;
  }
  assert(sect != NULL);
  res->setValue(sect->addr());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::ALIGNOF>::eval(const Module& pModule,
                                             const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  const LDSection* sect = NULL;
  switch (m_pOperand->type()) {
    case Operand::SECTION:
      sect = pModule.getSection(llvm::cast<SectOperand>(m_pOperand)->name());
      break;
    case Operand::SECTION_DESC:
      sect =
          llvm::cast<SectDescOperand>(m_pOperand)->outputDesc()->getSection();
      break;
    default:
      assert(0);
      break;
  }
  assert(sect != NULL);
  res->setValue(sect->align());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::DATA_SEGMENT_END>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand->value());
  return res;
}

template <>
IntOperand* UnaryOp<Operator::DEFINED>::eval(const Module& pModule,
                                             const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::LENGTH>::eval(const Module& pModule,
                                            const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::LOADADDR>::eval(const Module& pModule,
                                              const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::NEXT>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::ORIGIN>::eval(const Module& pModule,
                                            const TargetLDBackend& pBackend) {
  // TODO
  assert(0);
  return result();
}

template <>
IntOperand* UnaryOp<Operator::SIZEOF>::eval(const Module& pModule,
                                            const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  const LDSection* sect = NULL;
  switch (m_pOperand->type()) {
    case Operand::SECTION:
      sect = pModule.getSection(llvm::cast<SectOperand>(m_pOperand)->name());
      break;
    case Operand::SECTION_DESC:
      sect =
          llvm::cast<SectDescOperand>(m_pOperand)->outputDesc()->getSection();
      break;
    default:
      assert(0);
      break;
  }
  assert(sect != NULL);
  res->setValue(sect->size());
  return res;
}

}  // namespace mcld
