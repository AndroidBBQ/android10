//===- TargetRegistry.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/TargetRegistry.h"

namespace mcld {

TargetRegistry::TargetListTy mcld::TargetRegistry::s_TargetList;

//===----------------------------------------------------------------------===//
// TargetRegistry
//===----------------------------------------------------------------------===//
void TargetRegistry::RegisterTarget(Target& pTarget,
                                    const char* pName,
                                    Target::TripleMatchQualityFnTy pQualityFn) {
  pTarget.Name = pName;
  pTarget.TripleMatchQualityFn = pQualityFn;

  s_TargetList.push_back(&pTarget);
}

const Target* TargetRegistry::lookupTarget(const std::string& pTriple,
                                           std::string& pError) {
  if (empty()) {
    pError = "Unable to find target for this triple (no target are registered)";
    return NULL;
  }

  llvm::Triple triple(pTriple);
  Target* best = NULL, * ambiguity = NULL;
  unsigned int highest = 0;

  for (iterator target = begin(), ie = end(); target != ie; ++target) {
    unsigned int quality = (*target)->getTripleQuality(triple);
    if (quality > 0) {
      if (best == NULL || highest < quality) {
        highest = quality;
        best = *target;
        ambiguity = NULL;
      } else if (highest == quality) {
        ambiguity = *target;
      }
    }
  }

  if (best == NULL) {
    pError = "No availaible targets are compatible with this triple.";
    return NULL;
  }

  if (NULL != ambiguity) {
    pError = std::string("Ambiguous targets: \"") + best->name() + "\" and \"" +
             ambiguity->name() + "\"";
    return NULL;
  }

  return best;
}

const Target* TargetRegistry::lookupTarget(const std::string& pArchName,
                                           llvm::Triple& pTriple,
                                           std::string& pError) {
  const Target* result = NULL;
  if (!pArchName.empty()) {
    for (mcld::TargetRegistry::iterator it = mcld::TargetRegistry::begin(),
                                        ie = mcld::TargetRegistry::end();
         it != ie;
         ++it) {
      if (pArchName == (*it)->name()) {
        result = *it;
        break;
      }
    }

    if (result == NULL) {
      pError = std::string("invalid target '") + pArchName + "'.\n";
      return NULL;
    }

    // Adjust the triple to match (if known), otherwise stick with the
    // module/host triple.
    llvm::Triple::ArchType type =
        llvm::Triple::getArchTypeForLLVMName(pArchName);
    if (llvm::Triple::UnknownArch != type)
      pTriple.setArch(type);
  } else {
    std::string error;
    result = lookupTarget(pTriple.getTriple(), error);
    if (result == NULL) {
      pError = std::string("unable to get target for `") + pTriple.getTriple() +
               "'\n" + "(Detail: " + error + ")\n";
      return NULL;
    }
  }
  return result;
}

}  // namespace mcld
