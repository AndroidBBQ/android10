//===- EhFrame.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/EhFrame.h"

#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/MC/Input.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<EhFrame, MCLD_SECTIONS_PER_INPUT> EhFrameFactory;

static llvm::ManagedStatic<EhFrameFactory> g_EhFrameFactory;

//===----------------------------------------------------------------------===//
// EhFrame::Record
//===----------------------------------------------------------------------===//
EhFrame::Record::Record(llvm::StringRef pRegion) : RegionFragment(pRegion) {
}

EhFrame::Record::~Record() {
  // llvm::iplist will manage and delete the fragments
}

//===----------------------------------------------------------------------===//
// EhFrame::CIE
//===----------------------------------------------------------------------===//
EhFrame::CIE::CIE(llvm::StringRef pRegion)
    : EhFrame::Record(pRegion),
      m_FDEEncode(0u),
      m_Mergeable(false),
      m_pReloc(0),
      m_PersonalityOffset(0) {
}

EhFrame::CIE::~CIE() {
}

//===----------------------------------------------------------------------===//
// EhFrame::FDE
//===----------------------------------------------------------------------===//
EhFrame::FDE::FDE(llvm::StringRef pRegion, EhFrame::CIE& pCIE)
    : EhFrame::Record(pRegion), m_pCIE(&pCIE) {
}

EhFrame::FDE::~FDE() {
}

void EhFrame::FDE::setCIE(EhFrame::CIE& pCIE) {
  m_pCIE = &pCIE;
  m_pCIE->add(*this);
}

//===----------------------------------------------------------------------===//
// EhFrame::GeneratedCIE
//===----------------------------------------------------------------------===//
EhFrame::GeneratedCIE::GeneratedCIE(llvm::StringRef pRegion)
    : EhFrame::CIE(pRegion) {
}

EhFrame::GeneratedCIE::~GeneratedCIE() {
}

//===----------------------------------------------------------------------===//
// EhFrame::GeneratedFDE
//===----------------------------------------------------------------------===//
EhFrame::GeneratedFDE::GeneratedFDE(llvm::StringRef pRegion, CIE& pCIE)
    : EhFrame::FDE(pRegion, pCIE) {
}

EhFrame::GeneratedFDE::~GeneratedFDE() {
}

//===----------------------------------------------------------------------===//
// EhFrame
//===----------------------------------------------------------------------===//
EhFrame::EhFrame() : m_pSection(NULL), m_pSectionData(NULL) {
}

EhFrame::EhFrame(LDSection& pSection)
    : m_pSection(&pSection), m_pSectionData(NULL) {
  m_pSectionData = SectionData::Create(pSection);
}

EhFrame::~EhFrame() {
}

EhFrame* EhFrame::Create(LDSection& pSection) {
  EhFrame* result = g_EhFrameFactory->allocate();
  new (result) EhFrame(pSection);
  return result;
}

void EhFrame::Destroy(EhFrame*& pSection) {
  pSection->~EhFrame();
  g_EhFrameFactory->deallocate(pSection);
  pSection = NULL;
}

void EhFrame::Clear() {
  g_EhFrameFactory->clear();
}

const LDSection& EhFrame::getSection() const {
  assert(m_pSection != NULL);
  return *m_pSection;
}

LDSection& EhFrame::getSection() {
  assert(m_pSection != NULL);
  return *m_pSection;
}

void EhFrame::addFragment(Fragment& pFrag) {
  uint32_t offset = 0;
  if (!m_pSectionData->empty())
    offset = m_pSectionData->back().getOffset() + m_pSectionData->back().size();

  m_pSectionData->getFragmentList().push_back(&pFrag);
  pFrag.setParent(m_pSectionData);
  pFrag.setOffset(offset);
}

void EhFrame::addCIE(EhFrame::CIE& pCIE, bool pAlsoAddFragment) {
  m_CIEs.push_back(&pCIE);
  if (pAlsoAddFragment)
    addFragment(pCIE);
}

void EhFrame::addFDE(EhFrame::FDE& pFDE, bool pAlsoAddFragment) {
  pFDE.getCIE().add(pFDE);
  if (pAlsoAddFragment)
    addFragment(pFDE);
}

size_t EhFrame::numOfFDEs() const {
  // FDE number only used by .eh_frame_hdr computation, and the number of CIE
  // is usually not too many. It is worthy to compromise space by time
  size_t size = 0u;
  for (const_cie_iterator i = cie_begin(), e = cie_end(); i != e; ++i)
    size += (*i)->numOfFDEs();
  return size;
}

EhFrame& EhFrame::merge(const Input& pInput, EhFrame& pFrame) {
  assert(this != &pFrame);
  if (pFrame.emptyCIEs()) {
    // May be a partial linking, or the eh_frame has no data.
    // Just append the fragments.
    moveInputFragments(pFrame);
    return *this;
  }

  const LDContext& ctx = *pInput.context();
  const LDSection* rel_sec = 0;
  for (LDContext::const_sect_iterator ri = ctx.relocSectBegin(),
                                      re = ctx.relocSectEnd();
       ri != re;
       ++ri) {
    if ((*ri)->getLink() == &pFrame.getSection()) {
      rel_sec = *ri;
      break;
    }
  }
  pFrame.setupAttributes(rel_sec);

  // Most CIE will be merged, so we don't reserve space first.
  for (cie_iterator i = pFrame.cie_begin(), e = pFrame.cie_end(); i != e; ++i) {
    CIE& input_cie = **i;
    // CIE number is usually very few, so we just use vector sequential search.
    if (!input_cie.getMergeable()) {
      moveInputFragments(pFrame, input_cie);
      addCIE(input_cie, /*AlsoAddFragment=*/false);
      continue;
    }

    cie_iterator out_i = cie_begin();
    for (cie_iterator out_e = cie_end(); out_i != out_e; ++out_i) {
      CIE& output_cie = **out_i;
      if (output_cie == input_cie) {
        // This input CIE can be merged
        moveInputFragments(pFrame, input_cie, &output_cie);
        removeAndUpdateCIEForFDE(pFrame, input_cie, output_cie, rel_sec);
        break;
      }
    }
    if (out_i == cie_end()) {
      moveInputFragments(pFrame, input_cie);
      addCIE(input_cie, /*AlsoAddFragment=*/false);
    }
  }
  return *this;
}

void EhFrame::setupAttributes(const LDSection* rel_sec) {
  for (cie_iterator i = cie_begin(), e = cie_end(); i != e; ++i) {
    CIE* cie = *i;
    removeDiscardedFDE(*cie, rel_sec);

    if (cie->getPersonalityName().size() == 0) {
      // There's no personality data encoding inside augmentation string.
      cie->setMergeable();
    } else {
      if (!rel_sec) {
        // No relocation to eh_frame section
        assert(cie->getPersonalityName() != "" &&
               "PR name should be a symbol address or offset");
        continue;
      }
      const RelocData* reloc_data = rel_sec->getRelocData();
      for (RelocData::const_iterator ri = reloc_data->begin(),
                                     re = reloc_data->end();
           ri != re;
           ++ri) {
        const Relocation& rel = *ri;
        if (rel.targetRef().getOutputOffset() ==
            cie->getOffset() + cie->getPersonalityOffset()) {
          cie->setMergeable();
          cie->setPersonalityName(rel.symInfo()->outSymbol()->name());
          cie->setRelocation(rel);
          break;
        }
      }

      assert(cie->getPersonalityName() != "" &&
             "PR name should be a symbol address or offset");
    }
  }
}

void EhFrame::removeDiscardedFDE(CIE& pCIE, const LDSection* pRelocSect) {
  if (!pRelocSect)
    return;

  typedef std::vector<FDE*> FDERemoveList;
  FDERemoveList to_be_removed_fdes;
  const RelocData* reloc_data = pRelocSect->getRelocData();
  for (fde_iterator i = pCIE.begin(), e = pCIE.end(); i != e; ++i) {
    FDE& fde = **i;
    for (RelocData::const_iterator ri = reloc_data->begin(),
                                   re = reloc_data->end();
         ri != re;
         ++ri) {
      const Relocation& rel = *ri;
      if (rel.targetRef().getOutputOffset() ==
          fde.getOffset() + getDataStartOffset<32>()) {
        bool has_section = rel.symInfo()->outSymbol()->hasFragRef();
        if (!has_section)
          // The section was discarded, just ignore this FDE.
          // This may happen when redundant group section was read.
          to_be_removed_fdes.push_back(&fde);
        break;
      }
    }
  }

  for (FDERemoveList::iterator i = to_be_removed_fdes.begin(),
                               e = to_be_removed_fdes.end();
       i != e;
       ++i) {
    FDE& fde = **i;
    fde.getCIE().remove(fde);

    // FIXME: This traverses relocations from the beginning on each FDE, which
    // may cause performance degration. Actually relocations will be sequential
    // order, so we can bookkeep the previously found relocation for next use.
    // Note: We must ensure FDE order is ordered.
    for (RelocData::const_iterator ri = reloc_data->begin(),
                                   re = reloc_data->end();
         ri != re;) {
      Relocation& rel = const_cast<Relocation&>(*ri++);
      if (rel.targetRef().getOutputOffset() >= fde.getOffset() &&
          rel.targetRef().getOutputOffset() < fde.getOffset() + fde.size()) {
        const_cast<RelocData*>(reloc_data)->remove(rel);
      }
    }
  }
}

void EhFrame::removeAndUpdateCIEForFDE(EhFrame& pInFrame,
                                       CIE& pInCIE,
                                       CIE& pOutCIE,
                                       const LDSection* rel_sect) {
  // Make this relocation to be ignored.
  Relocation* rel = const_cast<Relocation*>(pInCIE.getRelocation());
  if (rel && rel_sect)
    const_cast<RelocData*>(rel_sect->getRelocData())->remove(*rel);

  // Update the CIE-pointed FDEs
  for (fde_iterator i = pInCIE.begin(), e = pInCIE.end(); i != e; ++i)
    (*i)->setCIE(pOutCIE);

  // We cannot know whether there are references to this fragment, so just
  // keep it in input fragment list instead of memory deallocation
  pInCIE.clearFDEs();
}

void EhFrame::moveInputFragments(EhFrame& pInFrame) {
  SectionData& in_sd = *pInFrame.getSectionData();
  SectionData::FragmentListType& in_frag_list = in_sd.getFragmentList();
  SectionData& out_sd = *getSectionData();
  SectionData::FragmentListType& out_frag_list = out_sd.getFragmentList();

  while (!in_frag_list.empty()) {
    Fragment* frag = in_frag_list.remove(in_frag_list.begin());
    out_frag_list.push_back(frag);
    frag->setParent(&out_sd);
  }
}

void EhFrame::moveInputFragments(EhFrame& pInFrame, CIE& pInCIE, CIE* pOutCIE) {
  SectionData& in_sd = *pInFrame.getSectionData();
  SectionData::FragmentListType& in_frag_list = in_sd.getFragmentList();
  SectionData& out_sd = *getSectionData();
  SectionData::FragmentListType& out_frag_list = out_sd.getFragmentList();

  if (!pOutCIE) {
    // Newly inserted
    Fragment* frag = in_frag_list.remove(SectionData::iterator(pInCIE));
    out_frag_list.push_back(frag);
    frag->setParent(&out_sd);
    for (fde_iterator i = pInCIE.begin(), e = pInCIE.end(); i != e; ++i) {
      frag = in_frag_list.remove(SectionData::iterator(**i));
      out_frag_list.push_back(frag);
      frag->setParent(&out_sd);
    }
    return;
  }

  SectionData::iterator cur_iter(*pOutCIE);
  assert(cur_iter != out_frag_list.end());
  for (fde_iterator i = pInCIE.begin(), e = pInCIE.end(); i != e; ++i) {
    Fragment* frag = in_frag_list.remove(SectionData::iterator(**i));
    cur_iter = out_frag_list.insertAfter(cur_iter, frag);
    frag->setParent(&out_sd);
  }
}

size_t EhFrame::computeOffsetSize() {
  size_t offset = 0u;
  SectionData::FragmentListType& frag_list =
      getSectionData()->getFragmentList();
  for (SectionData::iterator i = frag_list.begin(), e = frag_list.end(); i != e;
       ++i) {
    Fragment& frag = *i;
    frag.setOffset(offset);
    offset += frag.size();
  }
  getSection().setSize(offset);
  return offset;
}

bool operator==(const EhFrame::CIE& p1, const EhFrame::CIE& p2) {
  return p1.getPersonalityName() == p2.getPersonalityName() &&
         p1.getAugmentationData() == p2.getAugmentationData();
}

}  // namespace mcld
