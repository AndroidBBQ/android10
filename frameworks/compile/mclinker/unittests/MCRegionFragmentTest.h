//===- MCRegionFragment.h - unittest for MCRegionFragment -----------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCREGIONFRAGMENT_TEST_H
#define MCREGIONFRAGMENT_TEST_H

#include <gtest.h>

namespace mcld {
class MCRegionFragment;

}  // namespace for mcld

namespace mcldtest {

/** \class MCRegionFragmentTest
 *  \brief The testcase of MCRegionFragment.
 *
 *  \see MCRegionFragment
 */
class MCRegionFragmentTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  MCRegionFragmentTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~MCRegionFragmentTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::MCRegionFragment* m_pTestee;
};

}  // namespace of mcldtest

#endif
