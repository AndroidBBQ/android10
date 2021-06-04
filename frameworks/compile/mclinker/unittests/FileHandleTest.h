//===- headerTest.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FILEHANDLE_TEST_H
#define MCLD_FILEHANDLE_TEST_H

#include <gtest.h>

namespace mcld {
class FileHandle;

}  // namespace for mcld

namespace mcldtest {

/** \class FileHandleTest
 *  \brief
 *
 *  \see FileHandle
 */
class FileHandleTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  FileHandleTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~FileHandleTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::FileHandle* m_pTestee;
};

}  // namespace of mcldtest

#endif
