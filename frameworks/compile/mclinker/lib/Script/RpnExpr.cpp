//===- RPNExpr.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/RpnExpr.h"

#include "mcld/Script/ExprToken.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/Operator.h"
#include "mcld/Support/GCFactory.h"
#include "mcld/Support/raw_ostream.h"

#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Casting.h>

namespace mcld {

typedef GCFactory<RpnExpr, MCLD_SYMBOLS_PER_INPUT> ExprFactory;
static llvm::ManagedStatic<ExprFactory> g_ExprFactory;

//===----------------------------------------------------------------------===//
// RpnExpr
//===----------------------------------------------------------------------===//
RpnExpr::RpnExpr() {
}

RpnExpr::~RpnExpr() {
}

bool RpnExpr::hasDot() const {
  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    if ((*it)->kind() == ExprToken::OPERAND &&
        llvm::cast<Operand>(*it)->isDot())
      return true;
  }
  return false;
}

void RpnExpr::dump() const {
  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    (*it)->dump();
    mcld::outs() << " ";
  }
}

void RpnExpr::push_back(ExprToken* pToken) {
  m_TokenQueue.push_back(pToken);
}

RpnExpr* RpnExpr::create() {
  RpnExpr* result = g_ExprFactory->allocate();
  new (result) RpnExpr();
  return result;
}

void RpnExpr::destroy(RpnExpr*& pRpnExpr) {
  g_ExprFactory->destroy(pRpnExpr);
  g_ExprFactory->deallocate(pRpnExpr);
  pRpnExpr = NULL;
}

void RpnExpr::clear() {
  g_ExprFactory->clear();
}

RpnExpr::iterator RpnExpr::insert(iterator pPosition, ExprToken* pToken) {
  return m_TokenQueue.insert(pPosition, pToken);
}

void RpnExpr::erase(iterator pPosition) {
  m_TokenQueue.erase(pPosition);
}

// buildHelperExpr - build the helper expr:
//                   ADDR ( `output_sect' ) + SIZEOF ( `output_sect' )
RpnExpr* RpnExpr::buildHelperExpr(SectionMap::iterator pIter) {
  RpnExpr* expr = RpnExpr::create();
  expr->push_back(SectDescOperand::create(*pIter));
  expr->push_back(&Operator::create<Operator::ADDR>());
  expr->push_back(SectDescOperand::create(*pIter));
  expr->push_back(&Operator::create<Operator::SIZEOF>());
  expr->push_back(&Operator::create<Operator::ADD>());
  return expr;
}

// buildHelperExpr - build the helper expr: `fragment'
RpnExpr* RpnExpr::buildHelperExpr(Fragment& pFrag) {
  RpnExpr* expr = RpnExpr::create();
  expr->push_back(FragOperand::create(pFrag));
  return expr;
}

}  // namespace mcld
