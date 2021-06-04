//===- HashTableTest.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCLD_HASH_TABLE_TEST_H
#define MCLD_HASH_TABLE_TEST_H

#include <gtest.h>

namespace mcldtest {

/** \class HashTableTest
 *  \brief Testcase for HashTable
 *
 *  \see HashTable
 */
class HashTableTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  HashTableTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~HashTableTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();
};

}  // namespace of mcldtest

#endif
