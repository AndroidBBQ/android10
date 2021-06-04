//===- GCFactoryListTraitsTest.cpp ----------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "GCFactoryListTraitsTest.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
GCFactoryListTraitsTest::GCFactoryListTraitsTest() {
  // Allocate the nodes.
  m_pNodesAlloc = new Node* [10];
#define ALLOCATE_NODE(i) m_pNodesAlloc[(i)] = m_NodeFactory.produce(i);
  ALLOCATE_NODE(0);
  ALLOCATE_NODE(1);
  ALLOCATE_NODE(2);
  ALLOCATE_NODE(3);
  ALLOCATE_NODE(4);
  ALLOCATE_NODE(5);
  ALLOCATE_NODE(6);
  ALLOCATE_NODE(7);
  ALLOCATE_NODE(8);
  ALLOCATE_NODE(9);
#undef ALLOCATE_NODE
}

// Destructor can do clean-up work that doesn't throw exceptions here.
GCFactoryListTraitsTest::~GCFactoryListTraitsTest() {
}

// SetUp() will be called immediately before each test.
void GCFactoryListTraitsTest::SetUp() {
  // Reset the node value and (re)insert into the iplist.
  for (unsigned i = 0; i < 10; i++) {
    m_pNodesAlloc[i]->setValue(m_pNodesAlloc[i]->getInitialValue());
    m_pNodeList.push_back(m_pNodesAlloc[i]);
  }
}

// TearDown() will be called immediately after each test.
void GCFactoryListTraitsTest::TearDown() {
  // Erasing of llvm::iplist won't destroy the allocation of the nodes managed
  // by the GCFactory (i.e., NodeFactory.)
  m_pNodeList.clear();
}

//==========================================================================//
// Testcases
//

#define CHECK_NODE_VALUE(v_)           \
  do {                                 \
    ASSERT_TRUE(v_ == it->getValue()); \
    it++;                              \
  } while (false)

#define CHECK_LIST_VALUE(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) \
  do {                                                            \
    llvm::iplist<Node>::const_iterator it = m_pNodeList.begin();  \
    CHECK_NODE_VALUE(v1);                                         \
    CHECK_NODE_VALUE(v2);                                         \
    CHECK_NODE_VALUE(v3);                                         \
    CHECK_NODE_VALUE(v4);                                         \
    CHECK_NODE_VALUE(v5);                                         \
    CHECK_NODE_VALUE(v6);                                         \
    CHECK_NODE_VALUE(v7);                                         \
    CHECK_NODE_VALUE(v8);                                         \
    CHECK_NODE_VALUE(v9);                                         \
    CHECK_NODE_VALUE(v10);                                        \
  } while (false)

TEST_F(GCFactoryListTraitsTest, Basic) {
  ASSERT_TRUE(10 == m_pNodeList.size());
  CHECK_LIST_VALUE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
}

TEST_F(GCFactoryListTraitsTest, BasicAgain) {
  ASSERT_TRUE(10 == m_pNodeList.size());
  CHECK_LIST_VALUE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
}

TEST_F(GCFactoryListTraitsTest, Clear) {
  m_pNodeList.clear();
  ASSERT_TRUE(0 == m_pNodeList.size());
}

TEST_F(GCFactoryListTraitsTest, PushThenPop) {
  Node* NewNode = m_NodeFactory.produce(11);
  m_pNodeList.push_back(NewNode);
  ASSERT_TRUE(11 == m_pNodeList.size());
  m_pNodeList.pop_back();
  ASSERT_TRUE(10 == m_pNodeList.size());
}

TEST_F(GCFactoryListTraitsTest, CodeIterator) {
  // to test whether there's compilation error for const template
  for (llvm::iplist<Node>::const_iterator I = m_pNodeList.begin(),
                                          E = m_pNodeList.end();
       I != E;
       I++)
    I->getValue();
}

TEST_F(GCFactoryListTraitsTest, Empty) {
  ASSERT_FALSE(m_pNodeList.empty());
  m_pNodeList.clear();
  ASSERT_TRUE(m_pNodeList.empty());
}

TEST_F(GCFactoryListTraitsTest, EraseAndSize) {
  ASSERT_FALSE(m_pNodeList.empty());
  m_pNodeList.erase(m_pNodeList.begin());
  m_pNodeList.erase(m_pNodeList.begin());
  ASSERT_TRUE(m_pNodeList.size() == 8);
}

#undef CHECK_LIST_VALUE
#undef CHECK_NODE_VALUE
