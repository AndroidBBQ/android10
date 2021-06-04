//===- StringTableTest.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef STRINGTABLE_TEST_H
#define STRINGTABLE_TEST_H

#include <gtest.h>

namespace mcld {
class StringTable;

}  // namespace for mcld

namespace mcldtest {

/** \class StringTableTest
 *  \brief
 *
 *  \see StringTable
 */
class StringTableTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  StringTableTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~StringTableTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::StringTable* m_pTestee;
};

}  // namespace of mcldtest

#endif
