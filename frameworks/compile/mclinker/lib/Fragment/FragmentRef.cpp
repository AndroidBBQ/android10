//===- FragmentRef.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/FragmentRef.h"

#include "mcld/Fragment/Fragment.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ManagedStatic.h>

#include <cassert>

namespace mcld {

typedef GCFactory<FragmentRef, MCLD_SECTIONS_PER_INPUT> FragRefFactory;

static llvm::ManagedStatic<FragRefFactory> g_FragRefFactory;

FragmentRef FragmentRef::g_NullFragmentRef;

//===----------------------------------------------------------------------===//
// FragmentRef
//===----------------------------------------------------------------------===//
FragmentRef::FragmentRef() : m_pFragment(NULL), m_Offset(0) {
}

FragmentRef::FragmentRef(Fragment& pFrag, FragmentRef::Offset pOffset)
    : m_pFragment(&pFrag), m_Offset(pOffset) {
}

/// Create - create a fragment reference for a given fragment.
///
/// @param pFrag - the given fragment
/// @param pOffset - the offset, can be larger than the fragment, but can not
///                  be larger than the section size.
/// @return if the offset is legal, return the fragment reference. Otherwise,
/// return NULL.
FragmentRef* FragmentRef::Create(Fragment& pFrag, uint64_t pOffset) {
  int64_t offset = pOffset;
  Fragment* frag = &pFrag;

  while (frag != NULL) {
    offset -= frag->size();
    if (offset <= 0)
      break;
    frag = frag->getNextNode();
  }
  if ((frag != NULL) && (frag->size() != 0)) {
    if (offset == 0)
      frag = frag->getNextNode();
    else
      offset += frag->size();
  }

  if (frag == NULL)
    return Null();

  FragmentRef* result = g_FragRefFactory->allocate();
  new (result) FragmentRef(*frag, offset);

  return result;
}

FragmentRef* FragmentRef::Create(LDSection& pSection, uint64_t pOffset) {
  SectionData* data = NULL;
  switch (pSection.kind()) {
    case LDFileFormat::Relocation:
      // No fragment reference refers to a relocation section
      break;
    case LDFileFormat::EhFrame:
      if (pSection.hasEhFrame())
        data = pSection.getEhFrame()->getSectionData();
      break;
    default:
      data = pSection.getSectionData();
      break;
  }

  if (data == NULL || data->empty()) {
    return Null();
  }

  return Create(data->front(), pOffset);
}

void FragmentRef::Clear() {
  g_FragRefFactory->clear();
}

FragmentRef* FragmentRef::Null() {
  return &g_NullFragmentRef;
}

FragmentRef& FragmentRef::assign(const FragmentRef& pCopy) {
  m_pFragment = const_cast<Fragment*>(pCopy.m_pFragment);
  m_Offset = pCopy.m_Offset;
  return *this;
}

FragmentRef& FragmentRef::assign(Fragment& pFrag, FragmentRef::Offset pOffset) {
  m_pFragment = &pFrag;
  m_Offset = pOffset;
  return *this;
}

void FragmentRef::memcpy(void* pDest, size_t pNBytes, Offset pOffset) const {
  // check if the offset is still in a legal range.
  if (m_pFragment == NULL)
    return;

  unsigned int total_offset = m_Offset + pOffset;
  switch (m_pFragment->getKind()) {
    case Fragment::Region: {
      RegionFragment* region_frag = static_cast<RegionFragment*>(m_pFragment);
      unsigned int total_length = region_frag->getRegion().size();
      if (total_length < (total_offset + pNBytes))
        pNBytes = total_length - total_offset;

      std::memcpy(
          pDest, region_frag->getRegion().begin() + total_offset, pNBytes);
      return;
    }
    case Fragment::Stub: {
      Stub* stub_frag = static_cast<Stub*>(m_pFragment);
      unsigned int total_length = stub_frag->size();
      if (total_length < (total_offset + pNBytes))
        pNBytes = total_length - total_offset;
      std::memcpy(pDest, stub_frag->getContent() + total_offset, pNBytes);
      return;
    }
    case Fragment::Alignment:
    case Fragment::Fillment:
    default:
      return;
  }
}

FragmentRef::Offset FragmentRef::getOutputOffset() const {
  Offset result = 0;
  if (m_pFragment != NULL)
    result = m_pFragment->getOffset();
  return (result + m_Offset);
}

}  // namespace mcld
