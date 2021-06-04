//===- InputTreeTest.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "InputTreeTest.h"

#include <vector>
#include <iostream>

#include "mcld/InputTree.h"
#include "mcld/MC/InputFactory.h"
#include "mcld/LinkerConfig.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/MC/FileAction.h"
#include "mcld/MC/CommandAction.h"

using namespace mcld;
using namespace mcld::test;

// Constructor can do set-up work for all test here.
InputTreeTest::InputTreeTest() : m_MemFactory(10), m_ContextFactory(4) {
  // create testee. modify it if need
  m_pConfig = new mcld::LinkerConfig("arm-none-linux-gnueabi");
  m_pAlloc = new mcld::InputFactory(10, *m_pConfig);
  m_pBuilder = new mcld::InputBuilder(
      *m_pConfig, *m_pAlloc, m_ContextFactory, m_MemFactory, false);
  m_pTestee = new mcld::InputTree();
  m_pBuilder->setCurrentTree(*m_pTestee);
}

// Destructor can do clean-up work that doesn't throw exceptions here.
InputTreeTest::~InputTreeTest() {
  delete m_pTestee;
  delete m_pAlloc;
  delete m_pBuilder;
  delete m_pConfig;
}

// SetUp() will be called immediately before each test.
void InputTreeTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void InputTreeTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//
TEST_F(InputTreeTest, Basic_operation) {
  std::vector<InputAction*> actions;

  size_t position = 0;
  actions.push_back(new StartGroupAction(position++));
  actions.push_back(new InputFileAction(position++, "path1"));
  actions.push_back(new EndGroupAction(position++));

  std::vector<InputAction*>::iterator action;
  for (action = actions.begin(); action != actions.end(); ++action) {
    (*action)->activate(*m_pBuilder);
    delete *action;
  }

  InputTree::iterator node = m_pTestee->root();
  InputTree::const_iterator const_node = node;
  --node;
  --const_node;

  ASSERT_TRUE(isGroup(node));
  ASSERT_TRUE(isGroup(const_node));
  ASSERT_FALSE(m_pAlloc->empty());
  ASSERT_TRUE(1 == m_pAlloc->size());

  --node;

  m_pTestee->enterGroup(node, InputTree::Downward);

  InputTree::const_iterator const_node2 = node;

  ASSERT_FALSE(node.isRoot());

  ASSERT_FALSE(isGroup(node));
  ASSERT_FALSE(isGroup(const_node2));
  ASSERT_FALSE(m_pAlloc->empty());
  ASSERT_FALSE(m_pAlloc->size() == 0);

  ASSERT_TRUE(m_pTestee->size() == 3);
}

TEST_F(InputTreeTest, forLoop_TEST) {
  InputTree::iterator node = m_pTestee->root();

  Input* input = m_pAlloc->produce("FileSpec", "path1");
  m_pTestee->insert<InputTree::Inclusive>(node, *input);
  InputTree::const_iterator const_node = node;
  --node;

  for (int i = 0; i < 100; ++i) {
    Input* input = m_pAlloc->produce("FileSpec", "path1");
    m_pTestee->insert<InputTree::Inclusive>(node, *input);
    ++node;
  }

  m_pTestee->enterGroup(node, InputTree::Downward);
  --node;

  ASSERT_FALSE(node.isRoot());
  ASSERT_TRUE(isGroup(node));
  ASSERT_FALSE(m_pAlloc->empty());
  ASSERT_FALSE(m_pAlloc->size() == 100);

  ASSERT_TRUE(m_pTestee->size() == 102);
}

TEST_F(InputTreeTest, Nesting_Case) {
  InputTree::iterator node = m_pTestee->root();

  for (int i = 0; i < 50; ++i) {
    m_pTestee->enterGroup(node, InputTree::Downward);
    --node;

    Input* input = m_pAlloc->produce("FileSpec", "path1");
    m_pTestee->insert(node, InputTree::Afterward, *input);
    ++node;
  }

  ASSERT_FALSE(node.isRoot());
  ASSERT_FALSE(isGroup(node));
  ASSERT_FALSE(m_pAlloc->empty());
  ASSERT_TRUE(m_pAlloc->size() == 50);
  ASSERT_TRUE(m_pTestee->size() == 100);
}

TEST_F(InputTreeTest, DFSIterator_BasicTraversal) {
  InputTree::iterator node = m_pTestee->root();
  Input* input = m_pAlloc->produce("111", "/");
  m_pTestee->insert<InputTree::Inclusive>(node, *input);
  node.move<InputTree::Inclusive>();

  input = m_pAlloc->produce("10", "/");
  m_pTestee->insert<InputTree::Positional>(node, *input);
  m_pTestee->enterGroup<InputTree::Inclusive>(node);
  node.move<InputTree::Inclusive>();

  input = m_pAlloc->produce("7", "/");
  m_pTestee->insert<InputTree::Inclusive>(node, *input);
  input = m_pAlloc->produce("8", "/");
  m_pTestee->insert<InputTree::Positional>(node, *input);

  InputTree::dfs_iterator dfs_it = m_pTestee->dfs_begin();
  InputTree::dfs_iterator dfs_end = m_pTestee->dfs_end();
  ASSERT_STREQ("111", (*dfs_it)->name().c_str());
  ++dfs_it;
  ASSERT_STREQ("7", (**dfs_it).name().c_str());
  ++dfs_it;
  ASSERT_STREQ("8", (**dfs_it).name().c_str());
  ++dfs_it;
  ASSERT_STREQ("10", (**dfs_it).name().c_str());
  ++dfs_it;
  ASSERT_TRUE(dfs_it == dfs_end);
}
