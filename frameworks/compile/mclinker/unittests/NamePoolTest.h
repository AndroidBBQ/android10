//===- NamePoolTest.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef STRSYMPOOL_TEST_H
#define STRSYMPOOL_TEST_H

#include <gtest.h>

namespace mcld {
class NamePool;

}  // namespace for mcld

namespace mcldtest {

/** \class NamePoolTest
 *  \brief
 *
 *  \see NamePool
 */
class NamePoolTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  NamePoolTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~NamePoolTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::NamePool* m_pTestee;
};

}  // namespace of mcldtest

#endif
