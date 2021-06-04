//===- GarbageCollection.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/GarbageCollection.h"

#include "mcld/Fragment/Fragment.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/SectionData.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/TargetLDBackend.h"

#include <llvm/Support/Casting.h>

#include <queue>
#if !defined(MCLD_ON_WIN32)
#include <fnmatch.h>
#define fnmatch0(pattern, string) (fnmatch(pattern, string, 0) == 0)
#else
#include <windows.h>
#include <shlwapi.h>
#define fnmatch0(pattern, string) (PathMatchSpec(string, pattern) == true)
#endif

namespace mcld {

//===----------------------------------------------------------------------===//
// Non-member functions
//===----------------------------------------------------------------------===//
// FIXME: these rules should be added into SectionMap, while currently adding to
// SectionMap will cause the output order change in .text section and leads to
// the .ARM.exidx order incorrect. We should sort the .ARM.exidx.
static const char* pattern_to_keep[] = {".text*personality*",
                                        ".data*personality*",
                                        ".gnu.linkonce.d*personality*",
                                        ".sdata*personality*"};

/// shouldKeep - check the section name for the keep sections
static bool shouldKeep(const std::string& pName) {
  static const unsigned int pattern_size =
      sizeof(pattern_to_keep) / sizeof(pattern_to_keep[0]);
  for (unsigned int i = 0; i < pattern_size; ++i) {
    if (fnmatch0(pattern_to_keep[i], pName.c_str()))
      return true;
  }
  return false;
}

/// shouldProcessGC - check if the section kind is handled in GC
static bool mayProcessGC(const LDSection& pSection) {
  if (pSection.kind() == LDFileFormat::TEXT ||
      pSection.kind() == LDFileFormat::DATA ||
      pSection.kind() == LDFileFormat::BSS ||
      pSection.kind() == LDFileFormat::GCCExceptTable)
    return true;
  return false;
}

//===----------------------------------------------------------------------===//
// GarbageCollection::SectionReachedListMap
//===----------------------------------------------------------------------===//
void GarbageCollection::SectionReachedListMap::addReference(
    const LDSection& pFrom,
    const LDSection& pTo) {
  m_ReachedSections[&pFrom].insert(&pTo);
}

GarbageCollection::SectionListTy&
GarbageCollection::SectionReachedListMap::getReachedList(
    const LDSection& pSection) {
  return m_ReachedSections[&pSection];
}

GarbageCollection::SectionListTy*
GarbageCollection::SectionReachedListMap::findReachedList(
    const LDSection& pSection) {
  ReachedSectionsTy::iterator it = m_ReachedSections.find(&pSection);
  if (it == m_ReachedSections.end())
    return NULL;
  return &it->second;
}

//===----------------------------------------------------------------------===//
// GarbageCollection
//===----------------------------------------------------------------------===//
GarbageCollection::GarbageCollection(const LinkerConfig& pConfig,
                                     const TargetLDBackend& pBackend,
                                     Module& pModule)
    : m_Config(pConfig), m_Backend(pBackend), m_Module(pModule) {
}

GarbageCollection::~GarbageCollection() {
}

bool GarbageCollection::run() {
  // 1. traverse all the relocations to set up the reached sections of each
  // section
  setUpReachedSections();
  m_Backend.setUpReachedSectionsForGC(m_Module, m_SectionReachedListMap);

  // 2. get all sections defined the entry point
  SectionVecTy entry;
  getEntrySections(entry);

  // 3. find all the referenced sections those can be reached by entry
  findReferencedSections(entry);

  // 4. stripSections - set the unreached sections to Ignore
  stripSections();
  return true;
}

void GarbageCollection::setUpReachedSections() {
  // traverse all the input relocations to setup the reached sections
  Module::obj_iterator input, inEnd = m_Module.obj_end();
  for (input = m_Module.obj_begin(); input != inEnd; ++input) {
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      // bypass the discarded relocation section
      // 1. its section kind is changed to Ignore. (The target section is a
      // discarded group section.)
      // 2. it has no reloc data. (All symbols in the input relocs are in the
      // discarded group sections)
      LDSection* reloc_sect = *rs;
      LDSection* apply_sect = reloc_sect->getLink();
      if ((LDFileFormat::Ignore == reloc_sect->kind()) ||
          (!reloc_sect->hasRelocData()))
        continue;

      // bypass the apply target sections which are not handled by gc
      if (!mayProcessGC(*apply_sect))
        continue;

      bool add_first = false;
      SectionListTy* reached_sects = NULL;
      RelocData::iterator reloc_it, rEnd = reloc_sect->getRelocData()->end();
      for (reloc_it = reloc_sect->getRelocData()->begin(); reloc_it != rEnd;
           ++reloc_it) {
        Relocation* reloc = llvm::cast<Relocation>(reloc_it);
        ResolveInfo* sym = reloc->symInfo();
        // only the target symbols defined in the input fragments can make the
        // reference
        if (sym == NULL)
          continue;
        if (!sym->isDefine() || !sym->outSymbol()->hasFragRef())
          continue;

        // only the target symbols defined in the concerned sections can make
        // the reference
        const LDSection* target_sect =
            &sym->outSymbol()->fragRef()->frag()->getParent()->getSection();
        if (!mayProcessGC(*target_sect))
          continue;

        // setup the reached list, if we first add the element to reached list
        // of this section, create an entry in ReachedSections map
        if (!add_first) {
          reached_sects = &m_SectionReachedListMap.getReachedList(*apply_sect);
          add_first = true;
        }
        reached_sects->insert(target_sect);
      }
      reached_sects = NULL;
      add_first = false;
    }
  }
}

void GarbageCollection::getEntrySections(SectionVecTy& pEntry) {
  // all the KEEP sections defined in ldscript are entries, traverse all the
  // input sections and check the SectionMap to find the KEEP sections
  Module::obj_iterator obj, objEnd = m_Module.obj_end();
  SectionMap& sect_map = m_Module.getScript().sectionMap();
  for (obj = m_Module.obj_begin(); obj != objEnd; ++obj) {
    const std::string input_name = (*obj)->name();
    LDContext::sect_iterator sect, sectEnd = (*obj)->context()->sectEnd();
    for (sect = (*obj)->context()->sectBegin(); sect != sectEnd; ++sect) {
      LDSection* section = *sect;
      if (!mayProcessGC(*section))
        continue;

      SectionMap::Input* sm_input =
          sect_map.find(input_name, section->name()).second;
      if (((sm_input != NULL) && (InputSectDesc::Keep == sm_input->policy())) ||
          shouldKeep(section->name()))
        pEntry.push_back(section);
    }
  }

  // when building shared object or the --export-dynamic has been given, the
  // global define symbols are entries
  if (LinkerConfig::DynObj == m_Config.codeGenType() ||
      m_Config.options().exportDynamic()) {
    NamePool::syminfo_iterator info_it,
        info_end = m_Module.getNamePool().syminfo_end();
    for (info_it = m_Module.getNamePool().syminfo_begin(); info_it != info_end;
         ++info_it) {
      ResolveInfo* info = info_it.getEntry();
      if (!info->isDefine() || info->isLocal() ||
          info->shouldForceLocal(m_Config))
        continue;
      LDSymbol* sym = info->outSymbol();
      if (sym == NULL || !sym->hasFragRef())
        continue;

      // only the target symbols defined in the concerned sections can be
      // entries
      const LDSection* sect =
          &sym->fragRef()->frag()->getParent()->getSection();
      if (!mayProcessGC(*sect))
        continue;
      pEntry.push_back(sect);
    }
  }

  // when building executable or PIE
  if (LinkerConfig::Exec == m_Config.codeGenType() ||
      m_Config.options().isPIE()) {
    // 1. the entry symbol is the entry
    LDSymbol* entry_sym =
        m_Module.getNamePool().findSymbol(m_Backend.getEntry(m_Module));
    assert(entry_sym != NULL);
    pEntry.push_back(&entry_sym->fragRef()->frag()->getParent()->getSection());

    // 2. the symbols have been seen in dynamic objects are entries. If
    // --export-dynamic is set, then these sections already been added. No need
    // to add them again
    if (!m_Config.options().exportDynamic()) {
      NamePool::syminfo_iterator info_it,
          info_end = m_Module.getNamePool().syminfo_end();
      for (info_it = m_Module.getNamePool().syminfo_begin();
           info_it != info_end; ++info_it) {
        ResolveInfo* info = info_it.getEntry();
        if (!info->isDefine() || info->isLocal())
          continue;

        if (!info->isInDyn())
          continue;

        LDSymbol* sym = info->outSymbol();
        if (sym == NULL || !sym->hasFragRef())
          continue;

        // only the target symbols defined in the concerned sections can be
        // entries
        const LDSection* sect =
            &sym->fragRef()->frag()->getParent()->getSection();
        if (!mayProcessGC(*sect))
          continue;

        pEntry.push_back(sect);
      }
    }
  }

  // symbols set by -u should not be garbage collected. Set them entries.
  GeneralOptions::const_undef_sym_iterator usym;
  GeneralOptions::const_undef_sym_iterator usymEnd =
      m_Config.options().undef_sym_end();
  for (usym = m_Config.options().undef_sym_begin(); usym != usymEnd; ++usym) {
    LDSymbol* sym = m_Module.getNamePool().findSymbol(*usym);
    assert(sym);
    ResolveInfo* info = sym->resolveInfo();
    assert(info);
    if (!info->isDefine() || !sym->hasFragRef())
      continue;
    // only the symbols defined in the concerned sections can be entries
    const LDSection* sect = &sym->fragRef()->frag()->getParent()->getSection();
    if (!mayProcessGC(*sect))
      continue;
    pEntry.push_back(sect);
  }
}

void GarbageCollection::findReferencedSections(SectionVecTy& pEntry) {
  // list of sections waiting to be processed
  typedef std::queue<const LDSection*> WorkListTy;
  WorkListTy work_list;
  // start from each entry, resolve the transitive closure
  SectionVecTy::iterator entry_it, entry_end = pEntry.end();
  for (entry_it = pEntry.begin(); entry_it != entry_end; ++entry_it) {
    // add entry point to work list
    work_list.push(*entry_it);

    // add section from the work_list to the referencedSections until every
    // reached sections are added
    while (!work_list.empty()) {
      const LDSection* sect = work_list.front();
      work_list.pop();
      // add section to the ReferencedSections, if the section has been put into
      // referencedSections, skip this section
      if (!m_ReferencedSections.insert(sect).second)
        continue;

      // get the section reached list, if the section do not has one, which
      // means no referenced between it and other sections, then skip it
      SectionListTy* reach_list =
          m_SectionReachedListMap.findReachedList(*sect);
      if (reach_list == NULL)
        continue;

      // put the reached sections to work list, skip the one already be in
      // referencedSections
      SectionListTy::iterator it, end = reach_list->end();
      for (it = reach_list->begin(); it != end; ++it) {
        if (m_ReferencedSections.find(*it) == m_ReferencedSections.end())
          work_list.push(*it);
      }
    }
  }
}

void GarbageCollection::stripSections() {
  // Traverse all the input Regular and BSS sections, if a section is not found
  // in the ReferencedSections, then it should be garbage collected
  Module::obj_iterator obj, objEnd = m_Module.obj_end();
  for (obj = m_Module.obj_begin(); obj != objEnd; ++obj) {
    LDContext::sect_iterator sect, sectEnd = (*obj)->context()->sectEnd();
    for (sect = (*obj)->context()->sectBegin(); sect != sectEnd; ++sect) {
      LDSection* section = *sect;
      if (!mayProcessGC(*section))
        continue;

      if (m_ReferencedSections.find(section) == m_ReferencedSections.end()) {
        section->setKind(LDFileFormat::Ignore);
        debug(diag::debug_print_gc_sections) << section->name()
                                             << (*obj)->name();
      }
    }
  }

  // Traverse all the relocation sections, if its target section is set to
  // Ignore, then set the relocation section to Ignore as well
  Module::obj_iterator input, inEnd = m_Module.obj_end();
  for (input = m_Module.obj_begin(); input != inEnd; ++input) {
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      LDSection* reloc_sect = *rs;
      if (LDFileFormat::Ignore == reloc_sect->getLink()->kind())
        reloc_sect->setKind(LDFileFormat::Ignore);
    }
  }
}

}  // namespace mcld
