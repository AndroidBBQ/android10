//===- ARMException.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ARMException.h"

#include "ARMLDBackend.h"

#include "mcld/Fragment/RegionFragment.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/LDContext.h"
#include "mcld/Support/MsgHandling.h"

#include <memory>

namespace mcld {

static RegionFragment* findRegionFragment(LDSection& pSection) {
  SectionData* sectData = pSection.getSectionData();
  for (SectionData::iterator it = sectData->begin(),
                             end = sectData->end(); it != end; ++it) {
    if (it->getKind() == Fragment::Region) {
      return static_cast<RegionFragment*>(&*it);
    }
  }
  return NULL;
}

void ARMExData::addInputMap(Input* pInput,
                            std::unique_ptr<ARMInputExMap> pExMap) {
  assert(m_Inputs.find(pInput) == m_Inputs.end() &&
         "multiple maps for an input");

  ARMInputExMap* exMap = pExMap.get();

  // Add mapping to the input-to-exdata map.
  m_Inputs.insert(std::make_pair(pInput, std::move(pExMap)));

  // Add mapping to the fragment-to-exdata map.
  for (ARMInputExMap::iterator it = exMap->begin(), end = exMap->end();
       it != end; ++it) {
    ARMExSectionTuple* exTuple = it->second.get();
    m_ExIdxToTuple[exTuple->getExIdxFragment()] = exTuple;
  }
}

std::unique_ptr<ARMExData> ARMExData::create(Module& pModule) {
  std::unique_ptr<ARMExData> exData(new ARMExData());
  for (Module::obj_iterator it = pModule.obj_begin(),
                            end = pModule.obj_end(); it != end; ++it) {
    Input* input = *it;
    exData->addInputMap(input, ARMInputExMap::create(*input));
  }
  return exData;
}

std::unique_ptr<ARMInputExMap> ARMInputExMap::create(Input& pInput) {
  std::unique_ptr<ARMInputExMap> exMap(new ARMInputExMap());

  // Scan the input and collect all related sections.
  LDContext* ctx = pInput.context();
  for (LDContext::sect_iterator it = ctx->sectBegin(),
                                end = ctx->sectEnd(); it != end; ++it) {
    LDSection* sect = *it;
    if (sect->type() == llvm::ELF::SHT_ARM_EXIDX) {
      ARMExSectionTuple* exTuple = exMap->getOrCreateByExSection(*sect);
      exTuple->setExIdxSection(sect);
      exTuple->setTextSection(sect->getLink());
      if (sect->getLink() == NULL) {
        fatal(diag::eh_missing_text_section) << sect->name() << pInput.name();
      }
    }
  }

  // Remove the invalid exception tuples and convert LDSection to RegionFragment
  // or RelocData.
  ARMInputExMap::iterator it = exMap->begin();
  ARMInputExMap::iterator end = exMap->end();
  while (it != end) {
    ARMExSectionTuple* exTuple = it->second.get();
    LDSection* const text = exTuple->getTextSection();
    LDSection* const exIdx = exTuple->getExIdxSection();

    // Ignore the exception section if the text section is ignored.
    if ((text->kind() == LDFileFormat::Ignore) ||
        (text->kind() == LDFileFormat::Folded)) {
      // Set the related exception sections as LDFileFormat::Ignore.
      exIdx->setKind(LDFileFormat::Ignore);
      // Remove this tuple from the input exception map.
      ARMInputExMap::iterator deadIt = it++;
      exMap->erase(deadIt);
      continue;
    }

    // Get RegionFragment from ".text", ".ARM.exidx", and ".ARM.extab" sections.
    RegionFragment* textFrag = findRegionFragment(*text);
    RegionFragment* exIdxFrag = findRegionFragment(*exIdx);

    exTuple->setTextFragment(textFrag);
    exTuple->setExIdxFragment(exIdxFrag);

    // If there is no region fragment in the .ARM.extab section, then we can
    // skip this tuple.
    if (exIdxFrag == NULL) {
      ARMInputExMap::iterator deadIt = it++;
      exMap->erase(deadIt);
      continue;
    }

    // Check next tuple
    ++it;
  }

  return exMap;
}

}  // namespace mcld
