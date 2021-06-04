//===- BinTreeTest.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "BinTreeTest.h"

#include "mcld/ADT/TypeTraits.h"
#include "mcld/InputTree.h"
#include <string>

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
BinTreeTest::BinTreeTest() {
  // create testee. modify it if need
  m_pTestee = new BinaryTree<int>();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
BinTreeTest::~BinTreeTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void BinTreeTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void BinTreeTest::TearDown() {
}

//==========================================================================//
// Testcases
//

/// General
TEST_F(BinTreeTest, Two_non_null_tree_merge) {
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 0);
  --pos;
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 1);
  m_pTestee->join<TreeIteratorBase::Leftward>(pos, 1);
  --pos;
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 2);
  m_pTestee->join<TreeIteratorBase::Leftward>(pos, 2);

  BinaryTree<int>* mergeTree = new BinaryTree<int>;
  BinaryTree<int>::iterator pos2 = mergeTree->root();
  mergeTree->join<TreeIteratorBase::Rightward>(pos2, 1);
  --pos2;
  mergeTree->join<TreeIteratorBase::Rightward>(pos2, 1);
  mergeTree->join<TreeIteratorBase::Leftward>(pos2, 1);

  m_pTestee->merge<TreeIteratorBase::Rightward>(pos, *mergeTree);
  delete mergeTree;
  EXPECT_TRUE(m_pTestee->size() == 8);
}

/// ---- TEST - 2 ----
TEST_F(BinTreeTest, A_null_tree_merge_a_non_null_tree) {
  BinaryTree<int>::iterator pos = m_pTestee->root();

  BinaryTree<int>* mergeTree = new BinaryTree<int>;
  mergeTree->join<TreeIteratorBase::Rightward>(pos, 0);
  --pos;
  mergeTree->join<TreeIteratorBase::Rightward>(pos, 1);
  mergeTree->join<TreeIteratorBase::Leftward>(pos, 1);
  --pos;
  mergeTree->join<TreeIteratorBase::Rightward>(pos, 2);
  mergeTree->join<TreeIteratorBase::Leftward>(pos, 2);

  m_pTestee->merge<TreeIteratorBase::Rightward>(pos, *mergeTree);

  delete mergeTree;
  EXPECT_TRUE(m_pTestee->size() == 5);
}

TEST_F(BinTreeTest, A_non_null_tree_merge_a_null_tree) {
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 0);
  --pos;
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 1);
  m_pTestee->join<TreeIteratorBase::Leftward>(pos, 1);
  --pos;
  m_pTestee->join<TreeIteratorBase::Rightward>(pos, 2);
  m_pTestee->join<TreeIteratorBase::Leftward>(pos, 2);

  BinaryTree<int>* mergeTree = new BinaryTree<int>;
  BinaryTree<int>::iterator pos2 = mergeTree->root();
  mergeTree->merge<TreeIteratorBase::Rightward>(pos2, *m_pTestee);

  // delete m_pTestee;
  EXPECT_TRUE(mergeTree->size() == 5);
  delete mergeTree;
}

TEST_F(BinTreeTest, Two_null_tree_merge) {
  BinaryTree<int>::iterator pos = m_pTestee->root();

  BinaryTree<int>* mergeTree = new BinaryTree<int>;
  BinaryTree<int>::iterator pos2 = mergeTree->root();

  mergeTree->merge<TreeIteratorBase::Rightward>(pos2, *m_pTestee);

  // delete m_pTestee;
  EXPECT_TRUE(mergeTree->size() == 0);
  delete mergeTree;
}

TEST_F(BinTreeTest, DFSIterator_BasicTraversal) {
  int a = 111, b = 10, c = 9, d = 8, e = 7;
  BinaryTree<int>::iterator pos = m_pTestee->root();

  m_pTestee->join<InputTree::Inclusive>(pos, a);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, b);
  m_pTestee->join<InputTree::Inclusive>(pos, c);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, d);
  m_pTestee->join<InputTree::Inclusive>(pos, e);

  BinaryTree<int>::dfs_iterator dfs_it = m_pTestee->dfs_begin();
  BinaryTree<int>::dfs_iterator dfs_end = m_pTestee->dfs_end();

  ASSERT_EQ(111, **dfs_it);
  ++dfs_it;
  EXPECT_EQ(9, **dfs_it);
  ++dfs_it;
  EXPECT_EQ(7, **dfs_it);
  ++dfs_it;
  EXPECT_EQ(8, **dfs_it);
  ++dfs_it;
  EXPECT_EQ(10, **dfs_it);
  ++dfs_it;
  EXPECT_TRUE(dfs_it == dfs_end);
}

TEST_F(BinTreeTest, DFSIterator_RightMostTree) {
  int a = 0, b = 1, c = 2, d = 3, e = 4;
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<InputTree::Inclusive>(pos, a);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, b);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, c);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, d);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, e);

  BinaryTree<int>::dfs_iterator dfs_it = m_pTestee->dfs_begin();
  BinaryTree<int>::dfs_iterator dfs_end = m_pTestee->dfs_end();

  ASSERT_EQ(0, **dfs_it);
  ++dfs_it;
  ASSERT_EQ(1, **dfs_it);
  ++dfs_it;
  ASSERT_EQ(2, **dfs_it);
  ++dfs_it;
  ASSERT_EQ(3, **dfs_it);
  ++dfs_it;
  ASSERT_EQ(4, **dfs_it);
  ++dfs_it;
  ASSERT_TRUE(dfs_it == dfs_end);
}

TEST_F(BinTreeTest, DFSIterator_SingleNode) {
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<InputTree::Inclusive>(pos, 0);
  BinaryTree<int>::dfs_iterator dfs_it = m_pTestee->dfs_begin();
  BinaryTree<int>::dfs_iterator dfs_end = m_pTestee->dfs_end();
  int counter = 0;
  while (dfs_it != dfs_end) {
    ++counter;
    ++dfs_it;
  }
  ASSERT_EQ(1, counter);
}

TEST_F(BinTreeTest, BFSIterator_BasicTraversal) {
  int a = 111, b = 10, c = 9, d = 8, e = 7;
  BinaryTree<int>::iterator pos = m_pTestee->root();

  m_pTestee->join<InputTree::Inclusive>(pos, a);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, b);
  m_pTestee->join<InputTree::Inclusive>(pos, c);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, d);
  m_pTestee->join<InputTree::Inclusive>(pos, e);

  BinaryTree<int>::bfs_iterator bfs_it = m_pTestee->bfs_begin();
  BinaryTree<int>::bfs_iterator bfs_end = m_pTestee->bfs_end();

  ASSERT_EQ(111, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(10, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(9, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(8, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(7, **bfs_it);
  ++bfs_it;
  ASSERT_TRUE(bfs_it == bfs_end);
  bfs_it = m_pTestee->bfs_begin();
  bfs_end = m_pTestee->bfs_end();
}

TEST_F(BinTreeTest, BFSIterator_RightMostTree) {
  int a = 0, b = 1, c = 2, d = 3, e = 4;
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<InputTree::Inclusive>(pos, a);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, b);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, c);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, d);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, e);

  BinaryTree<int>::bfs_iterator bfs_it = m_pTestee->bfs_begin();
  BinaryTree<int>::bfs_iterator bfs_end = m_pTestee->bfs_end();

  ASSERT_EQ(0, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(1, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(2, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(3, **bfs_it);
  ++bfs_it;
  ASSERT_EQ(4, **bfs_it);
  ++bfs_it;
  ASSERT_TRUE(bfs_it == bfs_end);
}

TEST_F(BinTreeTest, BFSIterator_SingleNode) {
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<InputTree::Inclusive>(pos, 0);
  BinaryTree<int>::bfs_iterator bfs_it = m_pTestee->bfs_begin();
  BinaryTree<int>::bfs_iterator bfs_end = m_pTestee->bfs_end();
  int counter = 0;
  while (bfs_it != bfs_end) {
    ++counter;
    ++bfs_it;
  }
  ASSERT_EQ(1, counter);
}

TEST_F(BinTreeTest, TreeIterator) {
  int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5;
  BinaryTree<int>::iterator pos = m_pTestee->root();
  m_pTestee->join<InputTree::Inclusive>(pos, a);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, b);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Inclusive>(pos, c);
  m_pTestee->join<InputTree::Positional>(pos, f);
  pos.move<InputTree::Inclusive>();
  m_pTestee->join<InputTree::Positional>(pos, d);
  pos.move<InputTree::Positional>();
  m_pTestee->join<InputTree::Positional>(pos, e);

  BinaryTree<int>::iterator it = m_pTestee->begin();
  BinaryTree<int>::iterator end = m_pTestee->end();

  ASSERT_EQ(0, **it);
  ++it;
  ASSERT_EQ(1, **it);
  --it;
  ASSERT_EQ(2, **it);
  ++it;
  ASSERT_EQ(3, **it);
  ++it;
  ASSERT_EQ(4, **it);
  ++it;
  ASSERT_TRUE(it == end);

  it = m_pTestee->begin();
  ++it;
  ++it;
  ASSERT_EQ(5, **it);
  ++it;
  ASSERT_TRUE(it == end);
}
