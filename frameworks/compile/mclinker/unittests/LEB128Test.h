//===- LEB128Test.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LEB128_TEST_H
#define MCLD_LEB128_TEST_H

#include <gtest.h>

namespace mcldtest {

/** \class LEB128Test
 *  \brief
 *
 *  \see LEB
 */
class LEB128Test : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  LEB128Test();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~LEB128Test();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of mcldtest

#endif
