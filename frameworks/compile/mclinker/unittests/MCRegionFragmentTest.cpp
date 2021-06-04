//===- MCRegionFragmentTest.cpp - MCRegionFragment implementation ---------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCRegionFragmentTest.h"

#include "mcld/MC/MCRegionFragment.h"
#include "mcld/Support/MemoryAreaFactory.h"
#include "mcld/Support/Path.h"

using namespace mcld;
using namespace mcldtest;
using namespace mcld::sys::fs;

// Constructor can do set-up work for all test here.
MCRegionFragmentTest::MCRegionFragmentTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
MCRegionFragmentTest::~MCRegionFragmentTest() {
}

// SetUp() will be called immediately before each test.
void MCRegionFragmentTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void MCRegionFragmentTest::TearDown() {
}

//==========================================================================//
// Testcases
//

TEST_F(MCRegionFragmentTest, classof_explicit) {
  Path path(TOPDIR);
  path.append("unittests/test3.txt");
  MemoryAreaFactory* areaFactory = new MemoryAreaFactory(1);
  MemoryArea* area = areaFactory->produce(path, MemoryArea::ReadWrite);

  MemoryRegion* region = area->request(0, 4096);
  MCRegionFragment* frag = new MCRegionFragment(*region);

  ASSERT_FALSE(llvm::MCDataFragment::classof(frag));
  ASSERT_TRUE(MCRegionFragment::classof(frag));
  delete frag;
  delete areaFactory;
}

TEST_F(MCRegionFragmentTest, classof_implicit) {
  Path path(TOPDIR);
  path.append("unittests/test3.txt");
  MemoryAreaFactory* areaFactory = new MemoryAreaFactory(1);
  MemoryArea* area = areaFactory->produce(path, MemoryArea::ReadWrite);

  MemoryRegion* region = area->request(0, 4096);
  llvm::MCFragment* frag = new MCRegionFragment(*region);

  ASSERT_FALSE(llvm::MCDataFragment::classof(frag));
  ASSERT_TRUE(MCRegionFragment::classof(frag));
  delete frag;
  delete areaFactory;
}
