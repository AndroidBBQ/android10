//===- InputTreeTest.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef UNITTESTS_INPUTTREE_TEST_H
#define UNITTESTS_INPUTTREE_TEST_H

#include <gtest.h>
#include "mcld/MC/ContextFactory.h"
#include "mcld/Support/MemoryAreaFactory.h"

namespace mcld {

class InputTree;
class InputFactory;
class InputBuilder;
class LinkerConfig;

namespace test {

/** \class InputTreeTest
 *  \brief
 *
 *  \see InputTree
 */
class InputTreeTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  InputTreeTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~InputTreeTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::LinkerConfig* m_pConfig;

  mcld::InputFactory* m_pAlloc;
  mcld::InputBuilder* m_pBuilder;
  mcld::MemoryAreaFactory m_MemFactory;
  mcld::ContextFactory m_ContextFactory;

  mcld::InputTree* m_pTestee;
};

}  // namespace of test
}  // namespace of mcld

#endif
