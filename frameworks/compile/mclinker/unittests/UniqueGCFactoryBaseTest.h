//===- UniqueGCFactoryBaseTest.h ------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef UNIQUE_GCFACTORYBASE_TEST_H
#define UNIQUE_GCFACTORYBASE_TEST_H

#include "mcld/Support/UniqueGCFactory.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LD/DiagnosticPrinter.h"
#include <gtest.h>

namespace mcldtest {

/** \class UniqueGCFactoryBaseTest
 *  - check the unique of key.
 *  - make sure the key associates with the same storage of value.
 *  - check if all functions in the GCFactoryBase are available.
 */
class UniqueGCFactoryBaseTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  UniqueGCFactoryBaseTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~UniqueGCFactoryBaseTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 private:
  mcld::LinkerConfig* m_pConfig;
};

}  // namespace of mcldtest

#endif
