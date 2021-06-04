//===- Operand.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/Operand.h"

#include "mcld/Fragment/Fragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Support/GCFactory.h"
#include "mcld/Support/raw_ostream.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Operand
//===----------------------------------------------------------------------===//
Operand::Operand(Type pType) : ExprToken(ExprToken::OPERAND), m_Type(pType) {
}

Operand::~Operand() {
}

//===----------------------------------------------------------------------===//
// SymOperand
//===----------------------------------------------------------------------===//
typedef GCFactory<SymOperand, MCLD_SYMBOLS_PER_INPUT> SymOperandFactory;
static llvm::ManagedStatic<SymOperandFactory> g_SymOperandFactory;

SymOperand::SymOperand() : Operand(Operand::SYMBOL), m_Value(0) {
}

SymOperand::SymOperand(const std::string& pName)
    : Operand(Operand::SYMBOL), m_Name(pName), m_Value(0) {
}

void SymOperand::dump() const {
  mcld::outs() << m_Name;
}

bool SymOperand::isDot() const {
  assert(!m_Name.empty());
  return m_Name.size() == 1 && m_Name[0] == '.';
}

SymOperand* SymOperand::create(const std::string& pName) {
  SymOperand* result = g_SymOperandFactory->allocate();
  new (result) SymOperand(pName);
  return result;
}

void SymOperand::destroy(SymOperand*& pOperand) {
  g_SymOperandFactory->destroy(pOperand);
  g_SymOperandFactory->deallocate(pOperand);
  pOperand = NULL;
}

void SymOperand::clear() {
  g_SymOperandFactory->clear();
}

//===----------------------------------------------------------------------===//
// IntOperand
//===----------------------------------------------------------------------===//
typedef GCFactory<IntOperand, MCLD_SYMBOLS_PER_INPUT> IntOperandFactory;
static llvm::ManagedStatic<IntOperandFactory> g_IntOperandFactory;

IntOperand::IntOperand() : Operand(Operand::INTEGER), m_Value(0) {
}

IntOperand::IntOperand(uint64_t pValue)
    : Operand(Operand::INTEGER), m_Value(pValue) {
}

void IntOperand::dump() const {
  mcld::outs() << m_Value;
}

IntOperand* IntOperand::create(uint64_t pValue) {
  IntOperand* result = g_IntOperandFactory->allocate();
  new (result) IntOperand(pValue);
  return result;
}

void IntOperand::destroy(IntOperand*& pOperand) {
  g_IntOperandFactory->destroy(pOperand);
  g_IntOperandFactory->deallocate(pOperand);
  pOperand = NULL;
}

void IntOperand::clear() {
  g_IntOperandFactory->clear();
}

//===----------------------------------------------------------------------===//
// SectOperand
//===----------------------------------------------------------------------===//
typedef GCFactory<SectOperand, MCLD_SECTIONS_PER_INPUT> SectOperandFactory;
static llvm::ManagedStatic<SectOperandFactory> g_SectOperandFactory;
SectOperand::SectOperand() : Operand(Operand::SECTION) {
}

SectOperand::SectOperand(const std::string& pName)
    : Operand(Operand::SECTION), m_Name(pName) {
}

void SectOperand::dump() const {
  mcld::outs() << m_Name;
}

SectOperand* SectOperand::create(const std::string& pName) {
  SectOperand* result = g_SectOperandFactory->allocate();
  new (result) SectOperand(pName);
  return result;
}

void SectOperand::destroy(SectOperand*& pOperand) {
  g_SectOperandFactory->destroy(pOperand);
  g_SectOperandFactory->deallocate(pOperand);
  pOperand = NULL;
}

void SectOperand::clear() {
  g_SectOperandFactory->clear();
}

//===----------------------------------------------------------------------===//
// SectDescOperand
//===----------------------------------------------------------------------===//
typedef GCFactory<SectDescOperand, MCLD_SECTIONS_PER_INPUT>
    SectDescOperandFactory;
static llvm::ManagedStatic<SectDescOperandFactory> g_SectDescOperandFactory;
SectDescOperand::SectDescOperand()
    : Operand(Operand::SECTION_DESC), m_pOutputDesc(NULL) {
}

SectDescOperand::SectDescOperand(const SectionMap::Output* pOutputDesc)
    : Operand(Operand::SECTION_DESC), m_pOutputDesc(pOutputDesc) {
}

void SectDescOperand::dump() const {
  assert(m_pOutputDesc != NULL);
  mcld::outs() << m_pOutputDesc->getSection()->name();
}

SectDescOperand* SectDescOperand::create(
    const SectionMap::Output* pOutputDesc) {
  SectDescOperand* result = g_SectDescOperandFactory->allocate();
  new (result) SectDescOperand(pOutputDesc);
  return result;
}

void SectDescOperand::destroy(SectDescOperand*& pOperand) {
  g_SectDescOperandFactory->destroy(pOperand);
  g_SectDescOperandFactory->deallocate(pOperand);
  pOperand = NULL;
}

void SectDescOperand::clear() {
  g_SectDescOperandFactory->clear();
}

//===----------------------------------------------------------------------===//
// FragOperand
//===----------------------------------------------------------------------===//
typedef GCFactory<FragOperand, MCLD_SYMBOLS_PER_INPUT> FragOperandFactory;
static llvm::ManagedStatic<FragOperandFactory> g_FragOperandFactory;

FragOperand::FragOperand() : Operand(Operand::FRAGMENT), m_pFragment(NULL) {
}

FragOperand::FragOperand(Fragment& pFragment)
    : Operand(Operand::FRAGMENT), m_pFragment(&pFragment) {
}

void FragOperand::dump() const {
  mcld::outs() << "fragment";
}

uint64_t FragOperand::value() const {
  return m_pFragment->getOffset() +
         m_pFragment->getParent()->getSection().addr();
}

FragOperand* FragOperand::create(Fragment& pFragment) {
  FragOperand* result = g_FragOperandFactory->allocate();
  new (result) FragOperand(pFragment);
  return result;
}

void FragOperand::destroy(FragOperand*& pOperand) {
  g_FragOperandFactory->destroy(pOperand);
  g_FragOperandFactory->deallocate(pOperand);
  pOperand = NULL;
}

void FragOperand::clear() {
  g_FragOperandFactory->clear();
}

}  // namespace mcld
