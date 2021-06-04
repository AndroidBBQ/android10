//===- ObjectBuilder.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Object/ObjectBuilder.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/Fragment/AlignFragment.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/Fragment/NullFragment.h"
#include "mcld/LD/DebugString.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Object/SectionMap.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// ObjectBuilder
//===----------------------------------------------------------------------===//
ObjectBuilder::ObjectBuilder(Module& pTheModule) : m_Module(pTheModule) {
}

/// CreateSection - create an output section.
LDSection* ObjectBuilder::CreateSection(const std::string& pName,
                                        LDFileFormat::Kind pKind,
                                        uint32_t pType,
                                        uint32_t pFlag,
                                        uint32_t pAlign) {
  // try to get one from output LDSection
  SectionMap::const_mapping pair =
      m_Module.getScript().sectionMap().find("*", pName);

  std::string output_name = (pair.first == NULL) ? pName : pair.first->name();

  LDSection* output_sect = m_Module.getSection(output_name);
  if (output_sect == NULL) {
    output_sect = LDSection::Create(pName, pKind, pType, pFlag);
    output_sect->setAlign(pAlign);
    m_Module.getSectionTable().push_back(output_sect);
  }
  return output_sect;
}

/// MergeSection - merge the pInput section to the pOutput section
LDSection* ObjectBuilder::MergeSection(const Input& pInputFile,
                                       LDSection& pInputSection) {
  SectionMap::mapping pair = m_Module.getScript().sectionMap().find(
      pInputFile.path().native(), pInputSection.name());

  if (pair.first != NULL && pair.first->isDiscard()) {
    pInputSection.setKind(LDFileFormat::Ignore);
    return NULL;
  }

  std::string output_name =
      (pair.first == NULL) ? pInputSection.name() : pair.first->name();
  LDSection* target = m_Module.getSection(output_name);

  if (target == NULL) {
    target = LDSection::Create(output_name,
                               pInputSection.kind(),
                               pInputSection.type(),
                               pInputSection.flag());
    target->setAlign(pInputSection.align());
    m_Module.getSectionTable().push_back(target);
  }

  switch (target->kind()) {
    case LDFileFormat::EhFrame: {
      EhFrame* eh_frame = NULL;
      if (target->hasEhFrame())
        eh_frame = target->getEhFrame();
      else
        eh_frame = IRBuilder::CreateEhFrame(*target);

      eh_frame->merge(pInputFile, *pInputSection.getEhFrame());
      UpdateSectionAlign(*target, pInputSection);
      return target;
    }
    case LDFileFormat::DebugString: {
      DebugString* debug_str = NULL;
      if (target->hasDebugString())
        debug_str = target->getDebugString();
      else
        debug_str = IRBuilder::CreateDebugString(*target);

      debug_str->merge(pInputSection);
      UpdateSectionAlign(*target, pInputSection);
      return target;
    }
    default: {
      if (!target->hasSectionData())
        IRBuilder::CreateSectionData(*target);

      SectionData* data = NULL;
      if (pair.first != NULL) {
        assert(pair.second != NULL);
        data = pair.second->getSection()->getSectionData();

        // force input alignment from ldscript if any
        if (pair.first->prolog().hasSubAlign()) {
          pInputSection.setAlign(pair.second->getSection()->align());
        }
      } else {
        // orphan section
        data = target->getSectionData();
      }

      if (MoveSectionData(*pInputSection.getSectionData(), *data)) {
        UpdateSectionAlign(*target, pInputSection);
        return target;
      }
      return NULL;
    }
  }
  return target;
}

/// MoveSectionData - move the fragments of pTO section data to pTo
bool ObjectBuilder::MoveSectionData(SectionData& pFrom, SectionData& pTo) {
  assert(&pFrom != &pTo && "Cannot move section data to itself!");

  uint64_t offset = pTo.getSection().size();
  AlignFragment* align = NULL;
  if (pFrom.getSection().align() > 1) {
    // if the align constraint is larger than 1, append an alignment
    unsigned int alignment = pFrom.getSection().align();
    align = new AlignFragment(/*alignment*/alignment,
                              /*the filled value*/0x0,
                              /*the size of filled value*/1u,
                              /*max bytes to emit*/alignment - 1);
    align->setOffset(offset);
    align->setParent(&pTo);
    pTo.getFragmentList().push_back(align);
    offset += align->size();
  }

  // move fragments from pFrom to pTO
  SectionData::FragmentListType& from_list = pFrom.getFragmentList();
  SectionData::FragmentListType& to_list = pTo.getFragmentList();
  SectionData::FragmentListType::iterator frag, fragEnd = from_list.end();
  for (frag = from_list.begin(); frag != fragEnd; ++frag) {
    frag->setParent(&pTo);
    frag->setOffset(offset);
    offset += frag->size();
  }
  to_list.splice(to_list.end(), from_list);

  // set up pTo's header
  pTo.getSection().setSize(offset);

  return true;
}

/// UpdateSectionAlign - update alignment for input section
void ObjectBuilder::UpdateSectionAlign(LDSection& pTo, const LDSection& pFrom) {
  if (pFrom.align() > pTo.align())
    pTo.setAlign(pFrom.align());
}

/// UpdateSectionAlign - update alignment for input section
void ObjectBuilder::UpdateSectionAlign(LDSection& pSection,
                                       uint32_t pAlignConstraint) {
  if (pSection.align() < pAlignConstraint)
    pSection.setAlign(pAlignConstraint);
}

/// AppendFragment - To append pFrag to the given SectionData pSD.
uint64_t ObjectBuilder::AppendFragment(Fragment& pFrag,
                                       SectionData& pSD,
                                       uint32_t pAlignConstraint) {
  // get initial offset.
  uint64_t offset = 0;
  if (!pSD.empty())
    offset = pSD.back().getOffset() + pSD.back().size();

  AlignFragment* align = NULL;
  if (pAlignConstraint > 1) {
    // if the align constraint is larger than 1, append an alignment
    align = new AlignFragment(/*alignment*/pAlignConstraint,
                              /*the filled value*/0x0,
                              /*the size of filled value*/1u,
                              /*max bytes to emit*/pAlignConstraint - 1);
    align->setOffset(offset);
    align->setParent(&pSD);
    pSD.getFragmentList().push_back(align);
    offset += align->size();
  }

  // append the fragment
  pFrag.setParent(&pSD);
  pFrag.setOffset(offset);
  pSD.getFragmentList().push_back(&pFrag);

  // append the null fragment
  offset += pFrag.size();
  NullFragment* null = new NullFragment(&pSD);
  null->setOffset(offset);

  if (align != NULL)
    return align->size() + pFrag.size();
  else
    return pFrag.size();
}

}  // namespace mcld
