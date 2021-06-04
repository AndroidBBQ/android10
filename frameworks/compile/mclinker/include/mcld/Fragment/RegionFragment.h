//===- RegionFragment.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_REGIONFRAGMENT_H_
#define MCLD_FRAGMENT_REGIONFRAGMENT_H_

#include "mcld/Fragment/Fragment.h"

#include <llvm/ADT/StringRef.h>

namespace mcld {

/** \class RegionFragment
 *  \brief RegionFragment is a kind of Fragment containing input memory region
 */
class RegionFragment : public Fragment {
 public:
  explicit RegionFragment(llvm::StringRef pRegion, SectionData* pSD = NULL);

  ~RegionFragment();

  const llvm::StringRef getRegion() const { return m_Region; }
  llvm::StringRef getRegion() { return m_Region; }

  static bool classof(const Fragment* F) {
    return F->getKind() == Fragment::Region;
  }

  static bool classof(const RegionFragment*) { return true; }

  size_t size() const;

 private:
  llvm::StringRef m_Region;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_REGIONFRAGMENT_H_
