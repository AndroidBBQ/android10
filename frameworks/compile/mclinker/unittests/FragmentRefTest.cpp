//===- FragmentRefTest ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "FragmentRefTest.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Support/MemoryAreaFactory.h"
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/MemoryRegion.h"
#include "mcld/Support/Path.h"
#include <llvm/ADT/StringRef.h>

using namespace mcld;
using namespace mcld::sys::fs;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
FragmentRefTest::FragmentRefTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
FragmentRefTest::~FragmentRefTest() {
}

// SetUp() will be called immediately before each test.
void FragmentRefTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void FragmentRefTest::TearDown() {
}

//============================================================================//
// Testcases
//
TEST_F(FragmentRefTest, ) {
  Path path(TOPDIR);
  path.append("unittests/test3.txt");
  MemoryAreaFactory* areaFactory = new MemoryAreaFactory(1);
  MemoryArea* area =
      areaFactory->produce(path, FileHandle::OpenMode(FileHandle::ReadWrite),
                           FileHandle::Permission(FileHandle::System));

  llvm::StringRef region = area->request(0, 4096);
  RegionFragment* frag = new RegionFragment(region);
  FragmentRef* ref = FragmentRef::Create(*frag, 0x0);

  ASSERT_EQ('H', region.data()[0]);
  ASSERT_TRUE(4096 == region.size());
  ASSERT_EQ('H', frag->getRegion().data()[0]);
  ASSERT_TRUE(4096 == frag->getRegion().size());
  ASSERT_EQ(frag, ref->frag());
  ASSERT_EQ('H',
            static_cast<RegionFragment*>(ref->frag())->getRegion().data()[0]);
  ASSERT_TRUE(4096 ==
              static_cast<RegionFragment*>(ref->frag())->getRegion().size());
  // ASSERT_EQ('H', ref->deref()[0]);

  ASSERT_TRUE(RegionFragment::classof(frag));

  delete frag;
  delete areaFactory;
}
