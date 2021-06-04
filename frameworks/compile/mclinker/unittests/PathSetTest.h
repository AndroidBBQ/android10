//===- PathSetTest.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef PATHSET_TEST_H
#define PATHSET_TEST_H

#include <gtest.h>

namespace mcld {
class PathSet;

}  // namespace for mcld

namespace mcldtest {

/** \class PathSetTest
 *  \brief The testcase of PathSet
 *
 *  \see PathSet
 */
class PathSetTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  PathSetTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~PathSetTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::PathSet* m_pTestee;
};

}  // namespace of mcldtest

#endif
