//===- SystemUtilsTest.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_UNITTEST_SYSTEM_UTILS_TEST_H
#define MCLD_UNITTEST_SYSTEM_UTILS_TEST_H

#include <gtest.h>

namespace mcld {
namespace test {

class SystemUtilsTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  SystemUtilsTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~SystemUtilsTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of test
}  // namespace of mcld

#endif
