//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/Path.h"
#include <fcntl.h>
#include <errno.h>
#include "FileHandleTest.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
FileHandleTest::FileHandleTest() {
  // create testee. modify it if need
  m_pTestee = new FileHandle();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
FileHandleTest::~FileHandleTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void FileHandleTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void FileHandleTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(FileHandleTest, open_close) {
  mcld::sys::fs::Path path(TOPDIR);
  path.append("unittests/test.txt");
  ASSERT_TRUE(m_pTestee->open(path, FileHandle::OpenMode(FileHandle::ReadOnly),
                              FileHandle::Permission(FileHandle::System)));
  ASSERT_TRUE(m_pTestee->isOpened());
  ASSERT_TRUE(m_pTestee->isGood());
  ASSERT_TRUE(m_pTestee->isOwned());

  ASSERT_TRUE(27 == m_pTestee->size());

  ASSERT_TRUE(m_pTestee->close());
  ASSERT_FALSE(m_pTestee->isOpened());
  ASSERT_TRUE(m_pTestee->isGood());

  ASSERT_TRUE(0 == m_pTestee->size());
}

TEST_F(FileHandleTest, delegate_close) {
  mcld::sys::fs::Path path(TOPDIR);
  path.append("unittests/test.txt");

  int fd = ::open(path.native().c_str(), O_RDONLY);

  ASSERT_TRUE(m_pTestee->delegate(fd, FileHandle::ReadOnly));
  ASSERT_TRUE(m_pTestee->isOpened());
  ASSERT_TRUE(m_pTestee->isGood());
  ASSERT_FALSE(m_pTestee->isOwned());

  ASSERT_TRUE(27 == m_pTestee->size());

  ASSERT_TRUE(m_pTestee->close());
  ASSERT_FALSE(m_pTestee->isOpened());
  ASSERT_TRUE(m_pTestee->isGood());
  ASSERT_TRUE(m_pTestee->isOwned());

  ASSERT_TRUE(0 == m_pTestee->size());

  int close_result = ::close(fd);
  ASSERT_EQ(0, close_result);
}

TEST_F(FileHandleTest, fail_close) {
  mcld::sys::fs::Path path(TOPDIR);
  path.append("unittests/test.txt");
  ASSERT_TRUE(m_pTestee->open(path, FileHandle::OpenMode(FileHandle::ReadOnly),
                              FileHandle::Permission(FileHandle::System)));
  ASSERT_TRUE(m_pTestee->isOpened());
  ASSERT_TRUE(m_pTestee->isGood());

  ASSERT_TRUE(27 == m_pTestee->size());

  int close_result = ::close(m_pTestee->handler());
  ASSERT_EQ(0, close_result);

  ASSERT_FALSE(m_pTestee->close());
  ASSERT_FALSE(m_pTestee->isOpened());
  ASSERT_FALSE(m_pTestee->isGood());
}
