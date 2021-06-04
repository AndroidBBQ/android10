//===- RpnEvaluator.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/RpnEvaluator.h"

#include "mcld/LD/LDSymbol.h"
#include "mcld/Script/ExprToken.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/Operator.h"
#include "mcld/Script/RpnExpr.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Module.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/DataTypes.h>

#include <stack>

#include <cassert>

namespace mcld {

RpnEvaluator::RpnEvaluator(const Module& pModule,
                           const TargetLDBackend& pBackend)
    : m_Module(pModule), m_Backend(pBackend) {
}

bool RpnEvaluator::eval(const RpnExpr& pExpr, uint64_t& pResult) {
  std::stack<Operand*> operandStack;
  for (RpnExpr::const_iterator it = pExpr.begin(), ie = pExpr.end(); it != ie;
       ++it) {
    switch ((*it)->kind()) {
      case ExprToken::OPERATOR: {
        Operator* op = llvm::cast<Operator>(*it);
        switch (op->arity()) {
          case Operator::NULLARY: {
            operandStack.push(op->eval(m_Module, m_Backend));
            break;
          }
          case Operator::UNARY: {
            Operand* opd = operandStack.top();
            operandStack.pop();
            op->appendOperand(opd);
            operandStack.push(op->eval(m_Module, m_Backend));
            break;
          }
          case Operator::BINARY: {
            Operand* opd2 = operandStack.top();
            operandStack.pop();
            Operand* opd1 = operandStack.top();
            operandStack.pop();
            op->appendOperand(opd1);
            op->appendOperand(opd2);
            operandStack.push(op->eval(m_Module, m_Backend));
            break;
          }
          case Operator::TERNARY: {
            Operand* opd3 = operandStack.top();
            operandStack.pop();
            Operand* opd2 = operandStack.top();
            operandStack.pop();
            Operand* opd1 = operandStack.top();
            operandStack.pop();
            op->appendOperand(opd1);
            op->appendOperand(opd2);
            op->appendOperand(opd3);
            operandStack.push(op->eval(m_Module, m_Backend));
            break;
          }
        }  // end of switch operator arity
        break;
      }

      case ExprToken::OPERAND: {
        Operand* opd = llvm::cast<Operand>(*it);
        switch (opd->type()) {
          case Operand::SYMBOL: {
            // It's possible that there are no operators in an expression, so
            // we set up symbol operand here.
            if (!opd->isDot()) {
              SymOperand* sym_opd = llvm::cast<SymOperand>(opd);
              const LDSymbol* symbol =
                  m_Module.getNamePool().findSymbol(sym_opd->name());
              if (symbol == NULL) {
                fatal(diag::fail_sym_resolution) << __FILE__ << __LINE__
                                                 << "mclinker@googlegroups.com";
              }
              sym_opd->setValue(symbol->value());
            }
            operandStack.push(opd);
            break;
          }
          default:
            operandStack.push(opd);
            break;
        }  // end of switch operand type
        break;
      }
    }  // end of switch
  }    // end of for

  // stack top is result
  assert(operandStack.top()->type() == Operand::SYMBOL ||
         operandStack.top()->type() == Operand::INTEGER ||
         operandStack.top()->type() == Operand::FRAGMENT);
  pResult = operandStack.top()->value();
  return true;
}

}  // namespace mcld
