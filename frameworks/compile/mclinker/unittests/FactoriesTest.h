//===- FactoriesTest.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef FACTORIES_TEST_H
#define FACTORIES_TEST_H
#include <gtest.h>
#include "mcld/ADT/TreeAllocator.h"
#include "mcld/MC/MCLDFile.h"

namespace mcldtest {

/** \class FactoriesTest
 *  \brief Test cases for factories - NodeFactory and MCLDFileFactory.
 *
 *  \see Factories
 */
class FactoriesTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  FactoriesTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~FactoriesTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  typedef mcld::NodeFactory<int> NodeAlloc;
  typedef mcld::MCLDFileFactory FileAlloc;

 protected:
  NodeAlloc* m_pNodeAlloc;
  FileAlloc* m_pFileAlloc;
};

}  // namespace of mcldtest

#endif
