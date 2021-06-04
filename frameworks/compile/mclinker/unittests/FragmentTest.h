//===- headerTest.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_TEST_H
#define MCLD_FRAGMENT_TEST_H

#include <gtest.h>

namespace mcld {
class Fragment;

}  // namespace for mcld

namespace mcldtest {

/** \class FragmentTest
 *  \brief Unit test for mcld::Fragment.
 *
 *  \see Fragment
 */
class FragmentTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  FragmentTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~FragmentTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::Fragment* m_pTestee;
};

}  // namespace of mcldtest

#endif
