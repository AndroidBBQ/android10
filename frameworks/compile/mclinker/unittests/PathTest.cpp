//===- PathTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "PathTest.h"
#include "mcld/Support/FileSystem.h"
#include <string>

//
using namespace mcld;
using namespace mcld::sys::fs;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
PathTest::PathTest() {
  // create testee. modify it if need
  m_pTestee = new Path();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
PathTest::~PathTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void PathTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void PathTest::TearDown() {
}

//==========================================================================//
// Testcases
//
TEST_F(PathTest, should_exist) {
  std::string root(TOPDIR);
  root += "/test/lit.cfg";
  m_pTestee->assign(root);
  EXPECT_TRUE(exists(*m_pTestee));

  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_TRUE(exists(*m_pTestee));
}

TEST_F(PathTest, should_not_exist) {
  const std::string root = "/luck";
  m_pTestee->assign(root);
  EXPECT_FALSE(exists(*m_pTestee));

  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_FALSE(exists(*m_pTestee));
}

TEST_F(PathTest, should_is_directory) {
  const std::string root = "../././..";
  m_pTestee->assign(root);
  EXPECT_TRUE(exists(*m_pTestee));
  EXPECT_TRUE(is_directory(*m_pTestee));
  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_TRUE(exists(*m_pTestee));
  EXPECT_TRUE(is_directory(*m_pTestee));
}

TEST_F(PathTest, should_not_is_directory) {
  const std::string root = "/luck";
  m_pTestee->assign(root);
  EXPECT_FALSE(exists(*m_pTestee));
  EXPECT_FALSE(is_directory(*m_pTestee));
  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_FALSE(exists(*m_pTestee));
  EXPECT_FALSE(is_directory(*m_pTestee));
}

TEST_F(PathTest, should_equal) {
  const std::string root = "aaa/bbb/../../ccc/";
  m_pTestee->assign(root);

  Path* p2 = new Path("ccc///////");

  EXPECT_TRUE(*m_pTestee == *p2);

  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_TRUE(*m_pTestee == *m_pTestee);
  delete p2;
}

TEST_F(PathTest, should_not_equal) {
  const std::string root = "aa/";
  Path* p2 = new Path("aaa//");
  //  p2->assign(root);
  m_pTestee->assign(root);
  EXPECT_TRUE(*m_pTestee != *p2);

  delete m_pTestee;
  m_pTestee = new Path(root);
  EXPECT_TRUE(*m_pTestee != *p2);
  delete p2;
}

TEST_F(PathTest, append_success) {
  const std::string root = "aa/";
  m_pTestee->assign(root);
  m_pTestee->append("aaa");
  std::string a("aa/aaa");
  EXPECT_TRUE(m_pTestee->native() == "aa/aaa");
  delete m_pTestee;
  m_pTestee = new Path("aa/");
  m_pTestee->append("/aaa");
  EXPECT_TRUE(m_pTestee->native() == "aa/aaa");
  delete m_pTestee;
  m_pTestee = new Path("aa");
  m_pTestee->append("/aaa");
  EXPECT_TRUE(m_pTestee->native() == "aa/aaa");
  delete m_pTestee;
  m_pTestee = new Path("aa");
  m_pTestee->append("aaa");
  EXPECT_TRUE(m_pTestee->native() == "aa/aaa");
}

TEST_F(PathTest, should_become_generic_string) {
  m_pTestee->assign("/etc/../dev/../usr//lib//");
  EXPECT_STREQ("/usr/lib/", m_pTestee->generic_string().c_str());
}

TEST_F(PathTest, parent_path) {
  m_pTestee->assign("aa/bb/cc/dd");
  EXPECT_STREQ("aa/bb/cc", m_pTestee->parent_path().c_str());
  delete m_pTestee;
  m_pTestee = new Path("/aa/bb/");
  EXPECT_STREQ("/aa/bb", m_pTestee->parent_path().c_str());
  delete m_pTestee;
  m_pTestee = new Path("/aa/bb");
  EXPECT_STREQ("/aa", m_pTestee->parent_path().c_str());
  delete m_pTestee;
  m_pTestee = new Path("aa/");
  EXPECT_STREQ("aa", m_pTestee->parent_path().c_str());
  delete m_pTestee;
  m_pTestee = new Path("aa");
  EXPECT_TRUE(m_pTestee->parent_path().empty());
}

TEST_F(PathTest, filename) {
  m_pTestee->assign("aa/bb/cc");
  EXPECT_STREQ("cc", m_pTestee->filename().c_str());

  m_pTestee->assign("aa/bb/");
  EXPECT_STREQ("", m_pTestee->filename().c_str());

  m_pTestee->assign("aa");
  EXPECT_STREQ("aa", m_pTestee->filename().c_str());
}
