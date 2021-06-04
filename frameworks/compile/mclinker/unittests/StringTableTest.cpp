//===- StringTableTest.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "StringTableTest.h"
#include "mcld/LD/StringTable.h"
#include <cstring>

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
StringTableTest::StringTableTest() {
  // create testee. modify it if need
  Resolver* R = new Resolver();
  StrSymPool* Pool = new StrSymPool(1, 1, *R);
  m_pTestee = new StringTable(*Pool);
}

// Destructor can do clean-up work that doesn't throw exceptions here.
StringTableTest::~StringTableTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void StringTableTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void StringTableTest::TearDown() {
}

//==========================================================================//
// Testcases
//
TEST_F(StringTableTest, different_string_size) {
  int size = 127 - 32;
  for (int i = 32; i < 127; ++i) {
    char c[2];
    c[0] = i;
    c[1] = '\0';
    ASSERT_NE(m_pTestee->insert(c), c);
  }
  ASSERT_EQ(m_pTestee->size(), size);
}

TEST_F(StringTableTest, traverse_begin_to_end) {
  m_pTestee->insert("Hello");
  m_pTestee->insert("World");
  m_pTestee->insert("Media");
  m_pTestee->insert("Tek");
  StringTable::iterator it = m_pTestee->begin();
  ASSERT_STREQ(*it, "Hello");
  ++it;
  ASSERT_STREQ(*it, "World");
  ++it;
  ASSERT_STREQ(*it, "Media");
  ++it;
  ASSERT_STREQ(*it, "Tek");
  ++it;
  ASSERT_EQ(it, m_pTestee->end());
}

TEST_F(StringTableTest, null_string) {
  m_pTestee->insert("");
  ASSERT_STREQ(*(m_pTestee->begin()), "");
  ASSERT_EQ(m_pTestee->size(), 1);
}
