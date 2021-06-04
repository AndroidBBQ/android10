//===- MemoryAreaFactory.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/MemoryAreaFactory.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/SystemUtils.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// MemoryAreaFactory
//===----------------------------------------------------------------------===//
MemoryAreaFactory::MemoryAreaFactory(size_t pNum)
    : GCFactory<MemoryArea, 0>(pNum) {
}

MemoryAreaFactory::~MemoryAreaFactory() {
}

MemoryArea* MemoryAreaFactory::produce(const sys::fs::Path& pPath,
                                       FileHandle::OpenMode pMode) {
  llvm::StringRef name(pPath.native());
  if (m_AreaMap.find(name) == m_AreaMap.end()) {
    MemoryArea* result = allocate();
    new (result) MemoryArea(name);
    m_AreaMap[name] = result;
    return result;
  }

  return m_AreaMap[name];
}

MemoryArea* MemoryAreaFactory::produce(const sys::fs::Path& pPath,
                                       FileHandle::OpenMode pMode,
                                       FileHandle::Permission pPerm) {
  llvm::StringRef name(pPath.native());
  if (m_AreaMap.find(name) == m_AreaMap.end()) {
    MemoryArea* result = allocate();
    new (result) MemoryArea(name);
    m_AreaMap[name] = result;
    return result;
  }

  return m_AreaMap[name];
}

MemoryArea* MemoryAreaFactory::produce(void* pMemBuffer, size_t pSize) {
  const char* base = reinterpret_cast<const char*>(pMemBuffer);
  llvm::StringRef name(base, pSize);
  if (m_AreaMap.find(name) == m_AreaMap.end()) {
    MemoryArea* result = allocate();
    new (result) MemoryArea(base, pSize);
    m_AreaMap[name] = result;
    return result;
  }

  return m_AreaMap[name];
}

MemoryArea* MemoryAreaFactory::produce(int pFD, FileHandle::OpenMode pMode) {
  // TODO
  return NULL;
}

void MemoryAreaFactory::destruct(MemoryArea* pArea) {
  destroy(pArea);
  deallocate(pArea);
}

}  // namespace mcld
