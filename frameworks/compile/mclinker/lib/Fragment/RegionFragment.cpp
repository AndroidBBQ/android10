//===- RegionFragment.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/RegionFragment.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// RegionFragment
//===----------------------------------------------------------------------===//
RegionFragment::RegionFragment(llvm::StringRef pRegion, SectionData* pSD)
    : Fragment(Fragment::Region, pSD), m_Region(pRegion) {
}

RegionFragment::~RegionFragment() {
}

size_t RegionFragment::size() const {
  return m_Region.size();
}

}  // namespace mcld
