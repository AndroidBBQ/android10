//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/SymbolCategory.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/LDSymbol.h"
#include <iostream>
#include "SymbolCategoryTest.h"

using namespace std;
using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
SymbolCategoryTest::SymbolCategoryTest() {
  // create testee. modify it if need
  m_pTestee = new SymbolCategory();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
SymbolCategoryTest::~SymbolCategoryTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void SymbolCategoryTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void SymbolCategoryTest::TearDown() {
}

//==========================================================================//
// Testcases
//

TEST_F(SymbolCategoryTest, upward_test) {
  ResolveInfo* a = ResolveInfo::Create("a");
  ResolveInfo* b = ResolveInfo::Create("b");
  ResolveInfo* c = ResolveInfo::Create("c");
  ResolveInfo* d = ResolveInfo::Create("d");
  ResolveInfo* e = ResolveInfo::Create("e");
  e->setBinding(ResolveInfo::Global);
  d->setBinding(ResolveInfo::Weak);
  c->setDesc(ResolveInfo::Common);
  c->setBinding(ResolveInfo::Global);
  b->setBinding(ResolveInfo::Local);
  a->setType(ResolveInfo::File);

  LDSymbol* aa = LDSymbol::Create(*a);
  LDSymbol* bb = LDSymbol::Create(*b);
  LDSymbol* cc = LDSymbol::Create(*c);
  LDSymbol* dd = LDSymbol::Create(*d);
  LDSymbol* ee = LDSymbol::Create(*e);

  m_pTestee->add(*ee);
  m_pTestee->add(*dd);
  m_pTestee->add(*cc);
  m_pTestee->add(*bb);
  m_pTestee->add(*aa);

  SymbolCategory::iterator sym = m_pTestee->begin();
  ASSERT_STREQ("a", (*sym)->name());
  ++sym;
  ASSERT_STREQ("b", (*sym)->name());
  ++sym;
  ASSERT_STREQ("c", (*sym)->name());
  ++sym;
  ASSERT_STREQ("d", (*sym)->name());
  ++sym;
  ASSERT_STREQ("e", (*sym)->name());

  ASSERT_TRUE(1 == m_pTestee->numOfLocals());
  ASSERT_TRUE(1 == m_pTestee->numOfCommons());
  ASSERT_TRUE(2 == m_pTestee->numOfDynamics());
  ASSERT_TRUE(5 == m_pTestee->numOfSymbols());
}

TEST_F(SymbolCategoryTest, change_local_to_dynamic) {
  ResolveInfo* a = ResolveInfo::Create("a");
  ResolveInfo* b = ResolveInfo::Create("b");
  ResolveInfo* c = ResolveInfo::Create("c");
  ResolveInfo* d = ResolveInfo::Create("d");
  ResolveInfo* e = ResolveInfo::Create("e");

  a->setBinding(ResolveInfo::Local);
  b->setBinding(ResolveInfo::Local);
  c->setBinding(ResolveInfo::Local);
  d->setDesc(ResolveInfo::Common);
  d->setBinding(ResolveInfo::Global);
  e->setBinding(ResolveInfo::Global);

  LDSymbol* aa = LDSymbol::Create(*a);
  LDSymbol* bb = LDSymbol::Create(*b);
  LDSymbol* cc = LDSymbol::Create(*c);
  LDSymbol* dd = LDSymbol::Create(*d);
  LDSymbol* ee = LDSymbol::Create(*e);

  a->setSymPtr(aa);
  b->setSymPtr(bb);
  c->setSymPtr(cc);
  d->setSymPtr(dd);
  e->setSymPtr(ee);

  m_pTestee->add(*ee);
  m_pTestee->add(*dd);
  m_pTestee->add(*cc);
  m_pTestee->add(*bb);
  m_pTestee->add(*aa);

  SymbolCategory::iterator sym = m_pTestee->begin();
  ASSERT_STREQ("c", (*sym)->name());
  ++sym;
  ASSERT_STREQ("b", (*sym)->name());
  ++sym;
  ASSERT_STREQ("a", (*sym)->name());
  ++sym;
  ASSERT_STREQ("d", (*sym)->name());
  ++sym;
  ASSERT_STREQ("e", (*sym)->name());

  m_pTestee->changeToDynamic(*bb);

  sym = m_pTestee->begin();
  ASSERT_STREQ("c", (*sym)->name());
  ++sym;
  ASSERT_STREQ("a", (*sym)->name());
  ++sym;
  ASSERT_STREQ("b", (*sym)->name());
  ++sym;
  ASSERT_STREQ("d", (*sym)->name());
  ++sym;
  ASSERT_STREQ("e", (*sym)->name());
}
