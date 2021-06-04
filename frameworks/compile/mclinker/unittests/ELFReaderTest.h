//===- ELFReaderTest.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ELFREADER_TEST_H
#define MCLD_ELFREADER_TEST_H

#include <gtest.h>
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/LD/ELFReaderIf.h"
#include "mcld/LD/ELFReader.h"
#include "mcld/LD/ELFObjectReader.h"
#include "mcld/Target/GNULDBackend.h"
#include "mcld/MC/InputBuilder.h"

namespace mcld {
template <>
class ELFReader<64, true>;
}  // namespace for mcld

namespace mcldtest {

class ELFReaderTest : public ::testing::Test {
 public:
  // Constructor can do set-up work for all test here.
  ELFReaderTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~ELFReaderTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

 protected:
  mcld::Input* m_pInput;
  mcld::LinkerConfig* m_pConfig;
  mcld::LinkerScript* m_pScript;
  mcld::GNUInfo* m_pInfo;
  mcld::GNULDBackend* m_pLDBackend;
  mcld::ELFReaderIF* m_pELFReader;
  mcld::Module* m_pModule;
  mcld::IRBuilder* m_pIRBuilder;
  mcld::ELFObjectReader* m_pELFObjReader;
};

}  // namespace of mcldtest

#endif
