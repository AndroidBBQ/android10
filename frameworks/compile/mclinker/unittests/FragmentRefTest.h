//===- FragmentRefTest.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCLD_MCFRAGMENT_REF_TEST_H
#define MCLD_MCFRAGMENT_REF_TEST_H

#include <gtest.h>

namespace mcld {
class FragmentRef;

}  // namespace for mcld

namespace mcldtest {

/** \class FragmentRefTest
 *  \brief Reference Test
 *
 *  \see FragmentRef
 */
class FragmentRefTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  FragmentRefTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~FragmentRefTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of mcldtest

#endif
