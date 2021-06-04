//===- FactoriesTest.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <cstdlib>
#include "FactoriesTest.h"
#include <string>

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
FactoriesTest::FactoriesTest() {
  m_pNodeAlloc = new NodeAlloc();
  m_pFileAlloc = new FileAlloc();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
FactoriesTest::~FactoriesTest() {
  delete m_pNodeAlloc;
  delete m_pFileAlloc;
}

// SetUp() will be called immediately before each test.
void FactoriesTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void FactoriesTest::TearDown() {
}

//==========================================================================//
// Testcases
//
TEST_F(FactoriesTest, node_produce) {
  NodeAlloc::NodeType* node = m_pNodeAlloc->produce();
  ASSERT_EQ(1, m_pNodeAlloc->size());
  ASSERT_FALSE(m_pNodeAlloc->empty());
  node = m_pNodeAlloc->produce();
  ASSERT_EQ(2, m_pNodeAlloc->size());
  ASSERT_FALSE(m_pNodeAlloc->empty());
  node = m_pNodeAlloc->produce();
  ASSERT_EQ(3, m_pNodeAlloc->size());
  ASSERT_FALSE(m_pNodeAlloc->empty());
}

TEST_F(FactoriesTest, node_iterate) {
  NodeAlloc::NodeType* node = 0;
  for (int i = 0; i < 100; ++i) {
    node = m_pNodeAlloc->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = i;
  }

  int counter = 0;
  NodeAlloc::iterator data = m_pNodeAlloc->begin();
  NodeAlloc::iterator dEnd = m_pNodeAlloc->end();
  for (; data != dEnd; ++data) {
    ASSERT_EQ(counter, *(*data).data);
    free((*data).data);
    (*data).data = 0;
    ++counter;
  }
}

TEST_F(FactoriesTest, node_delegate_empty) {
  NodeAlloc::NodeType* node = 0;
  for (int i = 0; i < 100; ++i) {
    node = m_pNodeAlloc->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = i;
  }
  NodeAlloc* delegatee = new NodeAlloc();
  m_pNodeAlloc->delegate(*delegatee);
  ASSERT_EQ(100, m_pNodeAlloc->size());
  int counter = 0;
  NodeAlloc::iterator data = m_pNodeAlloc->begin();
  NodeAlloc::iterator dEnd = m_pNodeAlloc->end();
  for (; data != dEnd; ++data) {
    ASSERT_EQ(counter, *(*data).data);
    free((*data).data);
    (*data).data = 0;
    ++counter;
  }
  delete delegatee;
}

TEST_F(FactoriesTest, node_empty_delegate) {
  NodeAlloc::NodeType* node = 0;
  NodeAlloc* delegatee = new NodeAlloc();
  for (int i = 0; i < 100; ++i) {
    node = delegatee->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = i;
  }
  m_pNodeAlloc->delegate(*delegatee);
  ASSERT_EQ(100, m_pNodeAlloc->size());
  int counter = 0;
  NodeAlloc::iterator data = m_pNodeAlloc->begin();
  NodeAlloc::iterator dEnd = m_pNodeAlloc->end();
  for (; data != dEnd; ++data) {
    ASSERT_EQ(counter, *(*data).data);
    free((*data).data);
    (*data).data = 0;
    ++counter;
  }
  ASSERT_EQ(0, delegatee->size());
  ASSERT_TRUE(delegatee->empty());
  delete delegatee;
}

TEST_F(FactoriesTest, node_delegate) {
  NodeAlloc::NodeType* node = 0;
  NodeAlloc* delegatee = new NodeAlloc();
  int counter = 0;
  // produce agent
  for (int i = 0; i < 100; ++i) {
    node = m_pNodeAlloc->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = counter;
    ++counter;
  }

  // produce delegatee
  for (int i = 0; i < 100; ++i) {
    node = delegatee->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = counter;
    ++counter;
  }

  m_pNodeAlloc->delegate(*delegatee);
  ASSERT_EQ(200, m_pNodeAlloc->size());
  ASSERT_FALSE(m_pNodeAlloc->empty());
  NodeAlloc::iterator data = m_pNodeAlloc->begin();
  NodeAlloc::iterator dEnd = m_pNodeAlloc->end();
  for (counter = 0; data != dEnd; ++data) {
    ASSERT_EQ(counter, *(*data).data);
    free((*data).data);
    (*data).data = 0;
    ++counter;
  }
  ASSERT_EQ(0, delegatee->size());
  ASSERT_TRUE(delegatee->empty());
  delete delegatee;
}

TEST_F(FactoriesTest, node_delegate_self) {
  NodeAlloc::NodeType* node = 0;
  for (int i = 0; i < 100; ++i) {
    node = m_pNodeAlloc->produce();
    node->data = (int*)malloc(sizeof(int));
    *(node->data) = i;
  }
  ASSERT_EQ(100, m_pNodeAlloc->size());
  m_pNodeAlloc->delegate(*m_pNodeAlloc);
  ASSERT_EQ(100, m_pNodeAlloc->size());
  ASSERT_FALSE(m_pNodeAlloc->empty());
}

TEST_F(FactoriesTest, file_produce) {
  int counter = 0;
  for (counter = 1; counter < 1000; ++counter) {
    MCLDFile* file = m_pFileAlloc->produce();
    ASSERT_EQ(counter, m_pFileAlloc->size());
    ASSERT_FALSE(m_pFileAlloc->empty());
  }
}

TEST_F(FactoriesTest, file_produce_by_params) {
  int counter = 0;
  for (counter = 1; counter < 1000; ++counter) {
    char name[100];
    sprintf(name, "file %d", counter);
    char path_name[100];
    sprintf(path_name, "/proj/mtk%d", counter);
    MCLDFile* file = m_pFileAlloc->produce(
        string(name), sys::fs::Path(string(path_name)), MCLDFile::Archive);
    ASSERT_EQ(counter, m_pFileAlloc->size());
    ASSERT_FALSE(m_pFileAlloc->empty());
    ASSERT_TRUE(file->isRecognized());
    ASSERT_STREQ(name, file->name().data());
  }
}

TEST_F(FactoriesTest, file_iterate) {
  int counter = 0;
  for (counter = 1; counter < 1000; ++counter) {
    char name[100];
    sprintf(name, "file %d", counter);
    char path_name[100];
    sprintf(path_name, "/proj/mtk%d", counter);
    MCLDFile* file = m_pFileAlloc->produce(
        string(name), sys::fs::Path(string(path_name)), MCLDFile::Archive);
  }

  ASSERT_EQ(counter - 1, m_pFileAlloc->size());
  ASSERT_FALSE(m_pFileAlloc->empty());

  MCLDFileFactory::iterator file = m_pFileAlloc->begin();
  MCLDFileFactory::iterator fEnd = m_pFileAlloc->end();

  while (file != fEnd) {
    ASSERT_TRUE((*file).isRecognized());
    ASSERT_FALSE((*file).name().empty());
    ++file;
  }
}
