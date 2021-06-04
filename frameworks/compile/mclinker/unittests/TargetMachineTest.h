//===- TargetMachineTest.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGETMACHINE_TEST_H
#define TARGETMACHINE_TEST_H
#include "mcld/Target/TargetMachine.h"
#include <gtest.h>

namespace mcldTEST {

/** \class TargetMachineTest
 *  \brief
 *
 *  \see TargetMachine
 */
class TargetMachineTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  TargetMachineTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~TargetMachineTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of BOLDTEST

#endif
