//===- GarbageCollection.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_GARBAGECOLLECTION_H_
#define MCLD_LD_GARBAGECOLLECTION_H_

#include <map>
#include <set>
#include <vector>

namespace mcld {

class LDSection;
class LinkerConfig;
class Module;
class TargetLDBackend;

/** \class GarbageCollection
 *  \brief Implementation of garbage collection for --gc-section.
 */
class GarbageCollection {
 public:
  typedef std::set<const LDSection*> SectionListTy;
  typedef std::vector<const LDSection*> SectionVecTy;

  /** \class SectionReachedListMap
   *  \brief Map the section to the list of sections which it can reach directly
   */
  class SectionReachedListMap {
   public:
    SectionReachedListMap() {}

    /// addReference - add a reference from pFrom to pTo
    void addReference(const LDSection& pFrom, const LDSection& pTo);

    /// getReachedList - get the list of sections which can be reached by
    /// pSection, create one if the list has not existed
    SectionListTy& getReachedList(const LDSection& pSection);

    /// findReachedList - find the list of sections which can be reached by
    /// pSection, return NULL if the list not exists
    SectionListTy* findReachedList(const LDSection& pSection);

   private:
    typedef std::map<const LDSection*, SectionListTy> ReachedSectionsTy;

   private:
    /// m_ReachedSections - map a section to the reachable sections list
    ReachedSectionsTy m_ReachedSections;
  };

 public:
  GarbageCollection(const LinkerConfig& pConfig,
                    const TargetLDBackend& pBackend,
                    Module& pModule);
  ~GarbageCollection();

  /// run - do garbage collection
  bool run();

 private:
  void setUpReachedSections();
  void findReferencedSections(SectionVecTy& pEntry);
  void getEntrySections(SectionVecTy& pEntry);
  void stripSections();

 private:
  /// m_SectionReachedListMap - map the section to the list of sections which it
  /// can reach directly
  SectionReachedListMap m_SectionReachedListMap;

  /// m_ReferencedSections - a list of sections which can be reached from entry
  SectionListTy m_ReferencedSections;

  const LinkerConfig& m_Config;
  const TargetLDBackend& m_Backend;
  Module& m_Module;
};

}  // namespace mcld

#endif  // MCLD_LD_GARBAGECOLLECTION_H_
