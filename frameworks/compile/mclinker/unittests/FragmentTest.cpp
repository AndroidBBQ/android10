//===- FragmentTest.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "FragmentTest.h"

#include "mcld/Fragment/Fragment.h"
#include "mcld/LD/SectionData.h"
#include "mcld/LD/LDSection.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
FragmentTest::FragmentTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
FragmentTest::~FragmentTest() {
}

// SetUp() will be called immediately before each test.
void FragmentTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void FragmentTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases

TEST_F(FragmentTest, Fragment_constructor) {
  LDSection* test = LDSection::Create("test", LDFileFormat::Null, 0, 0);
  SectionData* s = SectionData::Create(*test);
  new Fragment(Fragment::Alignment, s);
  EXPECT_TRUE(1 == s->size());
  new Fragment(Fragment::Alignment, s);
  new Fragment(Fragment::Region, s);
  new Fragment(Fragment::Fillment, s);
  new Fragment(Fragment::Target, s);
  EXPECT_TRUE(5 == s->size());

  LDSection::Destroy(test);
  //  SectionData::Destroy(s);
}

TEST_F(FragmentTest, Fragment_trivial_function) {
  LDSection* test = LDSection::Create("test", LDFileFormat::Null, 0, 0);
  SectionData* s = SectionData::Create(*test);
  Fragment* f = new Fragment(Fragment::Alignment, s);

  EXPECT_TRUE(Fragment::Alignment == f->getKind());

  f->setOffset(5566);
  EXPECT_TRUE(5566 == f->getOffset());

  // always return true
  EXPECT_TRUE(f->classof(new Fragment(Fragment::Region, s)));

  LDSection::Destroy(test);
  //  SectionData::Destroy(s);
}
