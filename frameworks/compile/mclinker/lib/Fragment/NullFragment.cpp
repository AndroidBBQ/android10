//===- NullFragment.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/NullFragment.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// NullFragment
//===----------------------------------------------------------------------===//
NullFragment::NullFragment(SectionData* pSD) : Fragment(Fragment::Null, pSD) {
}

}  // namespace mcld
