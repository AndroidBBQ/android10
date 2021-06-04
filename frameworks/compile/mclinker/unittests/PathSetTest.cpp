//===- PathSetTest.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/PathSet.h"
#include "PathSetTest.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
PathSetTest::PathSetTest() {
  // create testee. modify it if need
  m_pTestee = new PathSet();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
PathSetTest::~PathSetTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void PathSetTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void PathSetTest::TearDown() {
}

//==========================================================================//
// Testcases
//

TEST_F(PathSetTest, ) {
}
