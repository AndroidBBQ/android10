//===- SymbolTableTest.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/SymbolTable.h"
#include "SymbolTableTest.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
SymbolTableTest::SymbolTableTest() {
  // create testee. modify it if need
  m_pTestee = new SymbolTable<>(m_StrTable);
}

// Destructor can do clean-up work that doesn't throw exceptions here.
SymbolTableTest::~SymbolTableTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void SymbolTableTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void SymbolTableTest::TearDown() {
}

//==========================================================================//
// Testcases
TEST_F(SymbolTableTest, init) {
  mcld::SymbolTable<>::iterator it;
  it = m_pTestee->begin();
  ASSERT_EQ(it, m_pTestee->end());
}
