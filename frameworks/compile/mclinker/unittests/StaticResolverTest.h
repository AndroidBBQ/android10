//===- headerTest.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef STATICRESOLVER_TEST_H
#define STATICRESOLVER_TEST_H

#include <gtest.h>
#include "mcld/LinkerConfig.h"

namespace mcld {

class StaticResolver;
class ResolveInfoFactory;
class DiagnosticPrinter;

}  // namespace for mcld

namespace mcldtest {

/** \class StaticResolverTest
 *  \brief The testcases for static resolver
 *
 *  \see StaticResolver
 */
class StaticResolverTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  StaticResolverTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~StaticResolverTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::StaticResolver* m_pResolver;
  mcld::LinkerConfig* m_pConfig;
  mcld::DiagnosticPrinter* m_pPrinter;
};

}  // namespace of mcldtest

#endif
