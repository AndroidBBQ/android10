//===- ELFBinaryReaderTest.cpp --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFBinaryReader.h"
#include "mcld/Module.h"
#include "mcld/LinkerScript.h"
#include "mcld/LinkerConfig.h"
#include "mcld/IRBuilder.h"
#include "mcld/GeneralOptions.h"
#include "mcld/MC/Input.h"

#include "ELFBinaryReaderTest.h"

using namespace mcld;
using namespace mcld::test;

// Constructor can do set-up work for all test here.
ELFBinaryReaderTest::ELFBinaryReaderTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
ELFBinaryReaderTest::~ELFBinaryReaderTest() {
}

// SetUp() will be called immediately before each test.
void ELFBinaryReaderTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void ELFBinaryReaderTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(ELFBinaryReaderTest, is_myformat) {
  LinkerScript script;
  Module module("test", script);
  LinkerConfig config;
  IRBuilder builder(module, config);
  ELFBinaryReader* reader = new ELFBinaryReader(builder, config);

  Input input("test.bin");

  bool doContinue = false;
  config.options().setBinaryInput();
  ASSERT_TRUE(reader->isMyFormat(input, doContinue));

  config.options().setBinaryInput(false);
  ASSERT_FALSE(reader->isMyFormat(input, doContinue));

  delete reader;
}
