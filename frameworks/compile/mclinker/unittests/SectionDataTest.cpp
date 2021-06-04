//===- SectionDataTest.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "SectionDataTest.h"

#include "mcld/LD/SectionData.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/LDSection.h"

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
SectionDataTest::SectionDataTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
SectionDataTest::~SectionDataTest() {
}

// SetUp() will be called immediately before each test.
void SectionDataTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void SectionDataTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//

TEST_F(SectionDataTest, constructor_and_trivial_func) {
  LDSection* test = LDSection::Create("test", LDFileFormat::Null, 0, 0);

  SectionData* s = SectionData::Create(*test);
  EXPECT_TRUE(s->getSection().name() == "test" &&
              s->getSection().kind() == LDFileFormat::Null);

  LDSection::Destroy(test);
}

TEST_F(SectionDataTest, Fragment_list_and_iterator) {
  LDSection* test = LDSection::Create("test", LDFileFormat::Null, 0, 0);
  SectionData* s = SectionData::Create(*test);
  EXPECT_TRUE(s->empty());

  new Fragment(Fragment::Alignment, s);
  new Fragment(Fragment::Alignment, s);
  new Fragment(Fragment::Region, s);
  new Fragment(Fragment::Fillment, s);
  new Fragment(Fragment::Target, s);
  EXPECT_TRUE(5 == s->size());

  // iterator
  llvm::iplist<Fragment>::iterator iter = s->begin();
  EXPECT_TRUE(Fragment::Alignment == iter->getKind());
  ++iter;
  EXPECT_TRUE(Fragment::Alignment == iter->getKind());
  ++iter;
  EXPECT_TRUE(Fragment::Region == iter->getKind());
  ++iter;
  EXPECT_TRUE(Fragment::Fillment == iter->getKind());
  ++iter;
  EXPECT_TRUE(Fragment::Target == iter->getKind());
  ++iter;
  EXPECT_TRUE(iter == s->end());

  LDSection::Destroy(test);
}
