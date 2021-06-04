//===- IndenticalCodeFolding.cpp ------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/IdenticalCodeFolding.h"

#include "mcld/GeneralOptions.h"
#include "mcld/Module.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/Relocator.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/LinkerConfig.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/Demangle.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Format.h>

#include <cassert>
#include <map>
#include <set>

#include <zlib.h>

namespace mcld {

static bool isSymCtorOrDtor(const ResolveInfo& pSym) {
  // We can always fold ctors and dtors since accessing function pointer in C++
  // is forbidden.
  llvm::StringRef name(pSym.name(), pSym.nameSize());
  if (!name.startswith("_ZZ") && !name.startswith("_ZN")) {
    return false;
  }
  return isCtorOrDtor(pSym.name(), pSym.nameSize());
}

IdenticalCodeFolding::IdenticalCodeFolding(const LinkerConfig& pConfig,
                                           const TargetLDBackend& pBackend,
                                           Module& pModule)
    : m_Config(pConfig), m_Backend(pBackend), m_Module(pModule) {
}

void IdenticalCodeFolding::foldIdenticalCode() {
  // 1. Find folding candidates.
  FoldingCandidates candidate_list;
  findCandidates(candidate_list);

  // 2. Initialize constant section content
  for (size_t i = 0; i < candidate_list.size(); ++i) {
    candidate_list[i].initConstantContent(m_Backend, m_KeptSections);
  }

  // 3. Find identical code until convergence
  bool converged = false;
  size_t iterations = 0;
  while (!converged && (iterations < m_Config.options().getICFIterations())) {
    converged = matchCandidates(candidate_list);
    ++iterations;
  }
  if (m_Config.options().printICFSections()) {
    debug(diag::debug_icf_iterations) << iterations;
  }

  // 4. Fold the identical code
  typedef std::set<Input*> FoldedObjects;
  FoldedObjects folded_objs;
  KeptSections::iterator kept, keptEnd = m_KeptSections.end();
  size_t index = 0;
  for (kept = m_KeptSections.begin(); kept != keptEnd; ++kept, ++index) {
    LDSection* sect = (*kept).first;
    Input* obj = (*kept).second.first;
    size_t kept_index = (*kept).second.second;
    if (index != kept_index) {
      sect->setKind(LDFileFormat::Folded);
      folded_objs.insert(obj);

      if (m_Config.options().printICFSections()) {
        KeptSections::iterator it = m_KeptSections.begin() + kept_index;
        LDSection* kept_sect = (*it).first;
        Input* kept_obj = (*it).second.first;
        debug(diag::debug_icf_folded_section) << sect->name() << obj->name()
                                              << kept_sect->name()
                                              << kept_obj->name();
      }
    }
  }

  // Adjust the fragment reference of the folded symbols.
  FoldedObjects::iterator fobj, fobjEnd = folded_objs.end();
  for (fobj = folded_objs.begin(); fobj != fobjEnd; ++fobj) {
    LDContext::sym_iterator sym, symEnd = (*fobj)->context()->symTabEnd();
    for (sym = (*fobj)->context()->symTabBegin(); sym != symEnd; ++sym) {
      if ((*sym)->hasFragRef() && ((*sym)->type() == ResolveInfo::Function)) {
        LDSymbol* out_sym = (*sym)->resolveInfo()->outSymbol();
        FragmentRef* frag_ref = out_sym->fragRef();
        LDSection* sect = &(frag_ref->frag()->getParent()->getSection());
        if (sect->kind() == LDFileFormat::Folded) {
          size_t kept_index = m_KeptSections[sect].second;
          LDSection* kept_sect = (*(m_KeptSections.begin() + kept_index)).first;
          frag_ref->assign(kept_sect->getSectionData()->front(),
                           frag_ref->offset());
        }
      }
    }  // for each symbol
  }    // for each folded object
}

void IdenticalCodeFolding::findCandidates(FoldingCandidates& pCandidateList) {
  Module::obj_iterator obj, objEnd = m_Module.obj_end();
  for (obj = m_Module.obj_begin(); obj != objEnd; ++obj) {
    std::set<const LDSection*> funcptr_access_set;
    typedef std::map<LDSection*, LDSection*> CandidateMap;
    CandidateMap candidate_map;
    LDContext::sect_iterator sect, sectEnd = (*obj)->context()->sectEnd();
    for (sect = (*obj)->context()->sectBegin(); sect != sectEnd; ++sect) {
      switch ((*sect)->kind()) {
        case LDFileFormat::TEXT: {
          candidate_map.insert(
              std::make_pair(*sect, reinterpret_cast<LDSection*>(NULL)));
          break;
        }
        case LDFileFormat::Relocation: {
          LDSection* target = (*sect)->getLink();
          if (target->kind() == LDFileFormat::TEXT) {
            candidate_map[target] = *sect;
          }

          // Safe icf
          if (m_Config.options().getICFMode() == GeneralOptions::ICF::Safe) {
            RelocData::iterator rel, relEnd = (*sect)->getRelocData()->end();
            for (rel = (*sect)->getRelocData()->begin(); rel != relEnd; ++rel) {
              LDSymbol* sym = rel->symInfo()->outSymbol();
              if (sym->hasFragRef() && (sym->type() == ResolveInfo::Function)) {
                const LDSection* def =
                    &sym->fragRef()->frag()->getParent()->getSection();
                if (!isSymCtorOrDtor(*rel->symInfo()) &&
                    m_Backend.mayHaveUnsafeFunctionPointerAccess(*target) &&
                    m_Backend.getRelocator()
                        ->mayHaveFunctionPointerAccess(*rel)) {
                  funcptr_access_set.insert(def);
                }
              }
            }  // for each reloc
          }

          break;
        }
        default: {
          // skip
          break;
        }
      }  // end of switch
    }    // for each section

    CandidateMap::iterator candidate, candidateEnd = candidate_map.end();
    for (candidate = candidate_map.begin(); candidate != candidateEnd;
         ++candidate) {
      if ((m_Config.options().getICFMode() == GeneralOptions::ICF::All) ||
          (funcptr_access_set.count(candidate->first) == 0)) {
        size_t index = m_KeptSections.size();
        m_KeptSections[candidate->first] = ObjectAndId(*obj, index);
        pCandidateList.push_back(
            FoldingCandidate(candidate->first, candidate->second, *obj));
      }
    }  // for each possible candidate
  }  // for each obj
}

bool IdenticalCodeFolding::matchCandidates(FoldingCandidates& pCandidateList) {
  typedef std::multimap<uint32_t, size_t> ChecksumMap;
  ChecksumMap checksum_map;
  std::vector<std::string> contents(pCandidateList.size());
  bool converged = true;

  for (size_t index = 0; index < pCandidateList.size(); ++index) {
    contents[index] = pCandidateList[index].getContentWithVariables(
        m_Backend, m_KeptSections);
    uint32_t checksum = ::crc32(0xFFFFFFFF,
                                (const uint8_t*)contents[index].c_str(),
                                contents[index].length());

    size_t count = checksum_map.count(checksum);
    if (count == 0) {
      checksum_map.insert(std::make_pair(checksum, index));
    } else {
      std::pair<ChecksumMap::iterator, ChecksumMap::iterator> ret =
          checksum_map.equal_range(checksum);
      for (ChecksumMap::iterator it = ret.first; it != ret.second; ++it) {
        size_t kept_index = (*it).second;
        if (contents[index].compare(contents[kept_index]) == 0) {
          m_KeptSections[pCandidateList[index].sect].second = kept_index;
          converged = false;
          break;
        }
      }
    }
  }

  return converged;
}

void IdenticalCodeFolding::FoldingCandidate::initConstantContent(
    const TargetLDBackend& pBackend,
    const IdenticalCodeFolding::KeptSections& pKeptSections) {
  // Get the static content from text.
  assert(sect != NULL && sect->hasSectionData());
  SectionData::const_iterator frag, fragEnd = sect->getSectionData()->end();
  for (frag = sect->getSectionData()->begin(); frag != fragEnd; ++frag) {
    switch (frag->getKind()) {
      case Fragment::Region: {
        const RegionFragment& region = llvm::cast<RegionFragment>(*frag);
        content.append(region.getRegion().begin(), region.size());
        break;
      }
      default: {
        // FIXME: Currently we only take care of RegionFragment.
        break;
      }
    }
  }

  // Get the static content from relocs.
  if (reloc_sect != NULL && reloc_sect->hasRelocData()) {
    for (Relocation& rel : *reloc_sect->getRelocData()) {
      llvm::format_object<Relocation::Type,
                          Relocation::Address,
                          Relocation::Address,
                          Relocation::Address> rel_info("%x%llx%llx%llx",
                                                        rel.type(),
                                                        rel.symValue(),
                                                        rel.addend(),
                                                        rel.place());
      char rel_str[48];
      rel_info.print(rel_str, sizeof(rel_str));
      content.append(rel_str);

      // Handle the recursive call.
      LDSymbol* sym = rel.symInfo()->outSymbol();
      if ((sym->type() == ResolveInfo::Function) && sym->hasFragRef()) {
        LDSection* def = &sym->fragRef()->frag()->getParent()->getSection();
        if (def == sect) {
          continue;
        }
      }

      if (!pBackend.isSymbolPreemptible(*rel.symInfo()) && sym->hasFragRef() &&
          (pKeptSections.find(
               &sym->fragRef()->frag()->getParent()->getSection()) !=
           pKeptSections.end())) {
        // Mark this reloc as a variable.
        variable_relocs.push_back(&rel);
      } else {
        // TODO: Support inlining merge sections if possible (target-dependent).
        if ((sym->binding() == ResolveInfo::Local) ||
            (sym->binding() == ResolveInfo::Absolute)) {
          // ABS or Local symbols.
          content.append(sym->name()).append(obj->name()).append(
              obj->path().native());
        } else {
          content.append(sym->name());
        }
      }
    }
  }
}

std::string IdenticalCodeFolding::FoldingCandidate::getContentWithVariables(
    const TargetLDBackend& pBackend,
    const IdenticalCodeFolding::KeptSections& pKeptSections) {
  std::string result(content);
  // Compute the variable content from relocs.
  std::vector<Relocation*>::const_iterator rel, relEnd = variable_relocs.end();
  for (rel = variable_relocs.begin(); rel != relEnd; ++rel) {
    LDSymbol* sym = (*rel)->symInfo()->outSymbol();
    LDSection* def = &sym->fragRef()->frag()->getParent()->getSection();
    // Use the kept section index.
    KeptSections::const_iterator it = pKeptSections.find(def);
    llvm::format_object<size_t> kept_info("%x", (*it).second.second);
    char kept_str[8];
    kept_info.print(kept_str, sizeof(kept_str));
    result.append(kept_str);
  }

  return result;
}

}  // namespace mcld
