//===- ContextFactory.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/ContextFactory.h"

#include "mcld/LD/LDContext.h"

namespace mcld {

//===---------------------------------------------------------------------===//
// LDContextFactory
ContextFactory::ContextFactory(size_t pNum)
    : UniqueGCFactoryBase<sys::fs::Path, LDContext, 0>(pNum) {
}

ContextFactory::~ContextFactory() {
}

LDContext* ContextFactory::produce(const sys::fs::Path& pPath) {
  LDContext* result = find(pPath);
  if (result == NULL) {
    result = UniqueGCFactoryBase<sys::fs::Path, LDContext, 0>::allocate();
    new (result) LDContext();
    f_KeyMap.insert(std::make_pair(pPath, result));
  }
  return result;
}

LDContext* ContextFactory::produce(const char* pPath) {
  return produce(sys::fs::Path(pPath));
}

LDContext* ContextFactory::produce() {
  LDContext* result = allocate();
  new (result) LDContext();
  return result;
}

}  // namespace mcld
