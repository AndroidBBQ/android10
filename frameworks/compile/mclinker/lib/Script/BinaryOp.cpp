//===- BinaryOp.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/BinaryOp.h"

#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/ADT/SizeTraits.h"
#include "mcld/Script/Operand.h"
#include "mcld/Target/TargetLDBackend.h"

#include <llvm/Support/Casting.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// BinaryOp
//===----------------------------------------------------------------------===//
template <>
IntOperand* BinaryOp<Operator::MUL>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() * m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::DIV>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() / m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::MOD>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() % m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::ADD>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() + m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::SUB>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() - m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::LSHIFT>::eval(const Module& pModule,
                                             const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() << m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::RSHIFT>::eval(const Module& pModule,
                                             const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() >> m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::LT>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() < m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::LE>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() <= m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::GT>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() > m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::GE>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() >= m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::EQ>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() == m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::NE>::eval(const Module& pModule,
                                         const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() != m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::BITWISE_AND>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() & m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::BITWISE_XOR>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() ^ m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::BITWISE_OR>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() | m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::LOGICAL_AND>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() && m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::LOGICAL_OR>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  res->setValue(m_pOperand[0]->value() || m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::ALIGN>::eval(const Module& pModule,
                                            const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  uint64_t value = m_pOperand[0]->value();
  uint64_t align = m_pOperand[1]->value();
  alignAddress(value, align);
  res->setValue(value);
  return res;
}

template <>
IntOperand* BinaryOp<Operator::DATA_SEGMENT_RELRO_END>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  /* FIXME: Currently we handle relro in a different way, and now the result
     of this expression won't affect DATA_SEGMENT_ALIGN. */
  IntOperand* res = result();
  uint64_t value = m_pOperand[0]->value() + m_pOperand[1]->value();
  alignAddress(value, pBackend.commonPageSize());
  res->setValue(value);
  return res;
}

template <>
IntOperand* BinaryOp<Operator::MAX>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  if (m_pOperand[0]->value() >= m_pOperand[1]->value())
    res->setValue(m_pOperand[0]->value());
  else
    res->setValue(m_pOperand[1]->value());
  return res;
}

template <>
IntOperand* BinaryOp<Operator::MIN>::eval(const Module& pModule,
                                          const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  if (m_pOperand[0]->value() <= m_pOperand[1]->value())
    res->setValue(m_pOperand[0]->value());
  else
    res->setValue(m_pOperand[1]->value());
  return res;
}

/* SEGMENT_START(segment, default) */
template <>
IntOperand* BinaryOp<Operator::SEGMENT_START>::eval(
    const Module& pModule,
    const TargetLDBackend& pBackend) {
  IntOperand* res = result();
  /* Currently we look up segment address from -T command line options. */
  SectOperand* sect = llvm::cast<SectOperand>(m_pOperand[0]);
  const LinkerScript::AddressMap& addressMap = pModule.getScript().addressMap();
  LinkerScript::AddressMap::const_iterator addr;
  if (sect->name().compare("text-segment") == 0)
    addr = addressMap.find(".text");
  else if (sect->name().compare("data-segment") == 0)
    addr = addressMap.find(".data");
  else if (sect->name().compare("bss-segment") == 0)
    addr = addressMap.find(".bss");
  else
    addr = addressMap.find(sect->name());

  if (addr != addressMap.end())
    res->setValue(addr.getEntry()->value());
  else {
    assert(m_pOperand[1]->type() == Operand::INTEGER);
    res->setValue(m_pOperand[1]->value());
  }
  return res;
}

}  // namespace mcld
