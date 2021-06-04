//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/${deployment}/${class_name}.h>
#include "${class_name}Test.h"

using namespace mcld;
using namespace mcld::test;


// Constructor can do set-up work for all test here.
${class_name}Test::${class_name}Test()
{
}

// Destructor can do clean-up work that doesn't throw exceptions here.
${class_name}Test::~${class_name}Test()
{
}

// SetUp() will be called immediately before each test.
void ${class_name}Test::SetUp()
{
}

// TearDown() will be called immediately after each test.
void ${class_name}Test::TearDown()
{
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
/**
TEST_F( ${class_name}Test, name of  the testcase for ${class_name} ) {
	Write you exercise here
}
**/

