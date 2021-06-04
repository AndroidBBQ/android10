//===- PathTest.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef PATH_TEST_H
#define PATH_TEST_H

#include "mcld/Support/Path.h"
#include <gtest.h>

namespace mcldtest {

/** \class PathTest
 *  \brief a testcase for mcld::Path and its non-member funtions.
 *
 *  \see Path
 */
class PathTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  PathTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~PathTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::sys::fs::Path* m_pTestee;
};

}  // namespace of mcldtest

#endif
