//===- BinTreeTest.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef BINTREE_TEST_H
#define BINTREE_TEST_H

#include "mcld/ADT/BinTree.h"

#include <gtest.h>

namespace mcld {
class BinTree;

}  // namespace for mcld

namespace mcldtest {

/** \class BinTreeTest
 *  \brief Make sure the interface of BinTree , such as insert , traversal ,
 *etc..
 *
 *  \see BinTree
 */
class BinTreeTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  BinTreeTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~BinTreeTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::BinaryTree<int>* m_pTestee;
};

}  // namespace of mcldtest

#endif
