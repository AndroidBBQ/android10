//===- IdenticalCodeFolding.h ---------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_IDENTICALCODEFOLDING_H_
#define MCLD_LD_IDENTICALCODEFOLDING_H_

#include <llvm/ADT/MapVector.h>

#include <string>
#include <vector>

namespace mcld {

class Input;
class LDSection;
class LinkerConfig;
class Module;
class Relocation;
class TargetLDBackend;

/** \class IdenticalCodeFolding
 *  \brief Implementation of identical code folding for --icf=[none|all|safe]
 *  @ref Safe ICF: Pointer Safe and Unwinding Aware Identical Code Folding in
 *       Gold, http://research.google.com/pubs/pub36912.html
 */
class IdenticalCodeFolding {
 public:
  typedef std::pair<Input*, size_t> ObjectAndId;
  typedef llvm::MapVector<LDSection*, ObjectAndId> KeptSections;

 private:
  class FoldingCandidate {
   public:
    FoldingCandidate() : sect(NULL), reloc_sect(NULL), obj(NULL) {}
    FoldingCandidate(LDSection* pCode, LDSection* pReloc, Input* pInput)
        : sect(pCode), reloc_sect(pReloc), obj(pInput) {}

    void initConstantContent(
        const TargetLDBackend& pBackend,
        const IdenticalCodeFolding::KeptSections& pKeptSections);
    std::string getContentWithVariables(
        const TargetLDBackend& pBackend,
        const IdenticalCodeFolding::KeptSections& pKeptSections);

    LDSection* sect;
    LDSection* reloc_sect;
    Input* obj;
    std::string content;
    std::vector<Relocation*> variable_relocs;
  };

  typedef std::vector<FoldingCandidate> FoldingCandidates;

 public:
  IdenticalCodeFolding(const LinkerConfig& pConfig,
                       const TargetLDBackend& pBackend,
                       Module& pModule);

  void foldIdenticalCode();

 private:
  void findCandidates(FoldingCandidates& pCandidateList);

  bool matchCandidates(FoldingCandidates& pCandidateList);

 private:
  const LinkerConfig& m_Config;
  const TargetLDBackend& m_Backend;
  Module& m_Module;
  KeptSections m_KeptSections;
};

}  // namespace mcld

#endif  // MCLD_LD_IDENTICALCODEFOLDING_H_
