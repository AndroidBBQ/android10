//===- BranchIslandFactory.cpp --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/BranchIslandFactory.h"

#include "mcld/Fragment/Fragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Module.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// BranchIslandFactory
//===----------------------------------------------------------------------===//

/// ctor
/// @param pMaxFwdBranchRange - the max forward branch range of the target
/// @param pMaxBwdBranchRange - the max backward branch range of the target
/// @param pMaxIslandSize - the predefined value for the max size of a island
BranchIslandFactory::BranchIslandFactory(int64_t pMaxFwdBranchRange,
                                         int64_t pMaxBwdBranchRange,
                                         size_t pMaxIslandSize)
    : GCFactory<BranchIsland, 0>(1u),  // magic number
      m_MaxFwdBranchRange(pMaxFwdBranchRange - pMaxIslandSize),
      m_MaxBwdBranchRange(pMaxBwdBranchRange + pMaxIslandSize),
      m_MaxIslandSize(pMaxIslandSize) {
}

BranchIslandFactory::~BranchIslandFactory() {
}

/// group - group fragments and create islands when needed
/// @param pSectionData - the SectionData holds fragments need to be grouped
void BranchIslandFactory::group(Module& pModule) {
  for (Module::iterator sect = pModule.begin(), sectEnd = pModule.end();
       sect != sectEnd; ++sect) {
    if (((*sect)->kind() == LDFileFormat::TEXT) && (*sect)->hasSectionData()) {
      SectionData& sd = *((*sect)->getSectionData());
      uint64_t group_end = m_MaxFwdBranchRange;
      for (SectionData::iterator it = sd.begin(), ie = sd.end(); it != ie;
           ++it) {
        if ((*it).getOffset() + (*it).size() > group_end) {
          Fragment* frag = (*it).getPrevNode();
          while (frag != NULL && frag->getKind() == Fragment::Alignment) {
            frag = frag->getPrevNode();
          }
          if (frag != NULL) {
            produce(*frag);
            group_end = (*it).getOffset() + m_MaxFwdBranchRange;
          }
        }
      }
      if (getIslands(sd.back()).first == NULL)
        produce(sd.back());
    }
  }
}

/// produce - produce a island for the given fragment
/// @param pFragment - the fragment needs a branch island
BranchIsland* BranchIslandFactory::produce(Fragment& pFragment) {
  BranchIsland* island = allocate();
  new (island) BranchIsland(pFragment,        // entry fragment to the island
                            m_MaxIslandSize,  // the max size of the island
                            size() - 1u);     // index in the island factory
  return island;
}

/// getIsland - find fwd and bwd islands for the fragment
/// @param pFragment - the fragment needs a branch island
std::pair<BranchIsland*, BranchIsland*> BranchIslandFactory::getIslands(
    const Fragment& pFragment) {
  BranchIsland* fwd = NULL;
  BranchIsland* bwd = NULL;
  for (iterator it = begin(), ie = end(), prev = ie; it != ie;
       prev = it, ++it) {
    if (pFragment.getParent() != (*it).getParent()) {
      continue;
    }

    if ((pFragment.getOffset() < (*it).offset()) &&
        ((pFragment.getOffset() + m_MaxFwdBranchRange) >= (*it).offset())) {
      fwd = &*it;

      if ((prev != ie) && (pFragment.getParent() == (*prev).getParent())) {
        int64_t bwd_off = (int64_t)pFragment.getOffset() + m_MaxBwdBranchRange;
        if ((pFragment.getOffset() > (*prev).offset()) &&
            (bwd_off <= (int64_t)(*prev).offset())) {
          bwd = &*prev;
        }
      }
      break;
    }
  }
  return std::make_pair(fwd, bwd);
}

}  // namespace mcld
