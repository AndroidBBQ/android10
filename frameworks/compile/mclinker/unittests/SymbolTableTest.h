//===- SymbolTableTest.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LD_SYMBOLTABLE_TEST_H
#define LD_SYMBOLTABLE_TEST_H
#include "mcld/LD/StringTable.h"
#include <gtest.h>

namespace mcld {
template <template <class> class, class>
class SymbolTable;
}  // namespace for mcld

namespace mcldtest {

/** \class SymbolTableTest
 *  \brief
 *
 *  \see SymbolTable
 */
class SymbolTableTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  SymbolTableTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~SymbolTableTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::SymbolTable<>* m_pTestee;
  mcld::StringTable m_StrTable;
};

}  // namespace of mcldtest

#endif
