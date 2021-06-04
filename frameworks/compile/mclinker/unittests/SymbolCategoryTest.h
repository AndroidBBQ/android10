//===- headerTest.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SYMBOLCATEGORY_TEST_H
#define MCLD_SYMBOLCATEGORY_TEST_H

#include <gtest.h>

namespace mcld {
class SymbolCategory;

}  // namespace for mcld

namespace mcldtest {

/** \class SymbolCategoryTest
 *  \brief The testcases of symbol category.
 *
 *  \see SymbolCategory
 */
class SymbolCategoryTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  SymbolCategoryTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~SymbolCategoryTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::SymbolCategory* m_pTestee;
};

}  // namespace of mcldtest

#endif
