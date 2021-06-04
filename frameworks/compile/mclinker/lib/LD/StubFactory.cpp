//===- StubFactory.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/StubFactory.h"

#include "mcld/IRBuilder.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/BranchIslandFactory.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ResolveInfo.h"

#include <string>

namespace mcld {

//===----------------------------------------------------------------------===//
// StubFactory
//===----------------------------------------------------------------------===//
StubFactory::~StubFactory() {
  for (StubPoolType::iterator it = m_StubPool.begin(), ie = m_StubPool.end();
       it != ie;
       ++it)
    delete (*it);
}

/// addPrototype - register a stub prototype
void StubFactory::addPrototype(Stub* pPrototype) {
  m_StubPool.push_back(pPrototype);
}

/// create - create a stub if needed, otherwise return NULL
Stub* StubFactory::create(Relocation& pReloc,
                          uint64_t pTargetSymValue,
                          IRBuilder& pBuilder,
                          BranchIslandFactory& pBRIslandFactory) {
  // find if there is a prototype stub for the input relocation
  Stub* stub = NULL;
  Stub* prototype = findPrototype(pReloc, pReloc.place(), pTargetSymValue);
  if (prototype != NULL) {
    const Fragment* frag = pReloc.targetRef().frag();
    // find the islands for the input relocation
    std::pair<BranchIsland*, BranchIsland*> islands =
        pBRIslandFactory.getIslands(*frag);
    if (islands.first == NULL) {
      // early exit if we can not find the forward island.
      return NULL;
    }

    // find if there is such a stub in the backward island first.
    if (islands.second != NULL) {
      stub = islands.second->findStub(prototype, pReloc);
    }

    if (stub == NULL) {
      // find if there is such a stub in the forward island.
      stub = islands.first->findStub(prototype, pReloc);
      if (stub == NULL) {
        // create a stub from the prototype
        stub = prototype->clone();

        // apply fixups in this new stub
        stub->applyFixup(pReloc, pBuilder, *islands.first);

        // add stub to the forward branch island
        islands.first->addStub(prototype, pReloc, *stub);
      }
    }
  }
  return stub;
}

Stub* StubFactory::create(FragmentRef& pFragRef,
                          IRBuilder& pBuilder,
                          BranchIslandFactory& pBRIslandFactory) {
  Stub* prototype = findPrototype(pFragRef);
  if (prototype == NULL) {
    return NULL;
  } else {
    std::pair<BranchIsland*, BranchIsland*> islands =
        pBRIslandFactory.getIslands(*(pFragRef.frag()));
    // early exit if we can not find the forward island.
    if (islands.first == NULL) {
      return NULL;
    } else {
      // create a stub from the prototype
      Stub* stub = prototype->clone();

      // apply fixups in this new stub
      stub->applyFixup(pFragRef, pBuilder, *islands.first);

      // add stub to the forward branch island
      islands.first->addStub(*stub);

      return stub;
    }  // (islands.first == NULL)
  }  // if (prototype == NULL)
}

/// findPrototype - find if there is a registered stub prototype for the given
/// relocation
Stub* StubFactory::findPrototype(const Relocation& pReloc,
                                 uint64_t pSource,
                                 uint64_t pTargetSymValue) const {
  for (StubPoolType::const_iterator it = m_StubPool.begin(),
                                    ie = m_StubPool.end(); it != ie; ++it) {
    if ((*it)->isMyDuty(pReloc, pSource, pTargetSymValue))
      return (*it);
  }
  return NULL;
}

Stub* StubFactory::findPrototype(const FragmentRef& pFragRef) const {
  for (StubPoolType::const_iterator it = m_StubPool.begin(),
                                    ie = m_StubPool.end(); it != ie; ++it) {
    if ((*it)->isMyDuty(pFragRef))
      return (*it);
  }
  return NULL;
}

}  // namespace mcld
