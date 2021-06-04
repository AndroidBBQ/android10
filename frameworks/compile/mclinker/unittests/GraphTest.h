//===- GraphTest.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_GRAPH_TEST_H
#define MCLD_GRAPH_TEST_H

#include <gtest.h>

namespace mcld {
namespace test {

class GraphTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  GraphTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~GraphTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of test
}  // namespace of mcld

#endif
