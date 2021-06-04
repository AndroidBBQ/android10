//===- SystemUtilsTest.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/SystemUtils.h"
#include "SystemUtilsTest.h"

using namespace mcld;
using namespace mcld::test;

// Constructor can do set-up work for all test here.
SystemUtilsTest::SystemUtilsTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
SystemUtilsTest::~SystemUtilsTest() {
}

// SetUp() will be called immediately before each test.
void SystemUtilsTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void SystemUtilsTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(SystemUtilsTest, test_strerror) {
  ASSERT_TRUE(NULL != mcld::sys::strerror(0));
}
