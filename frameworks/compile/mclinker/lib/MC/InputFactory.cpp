//===- InputFactory.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/InputFactory.h"

#include "mcld/LinkerConfig.h"
#include "mcld/AttributeOption.h"
#include "mcld/MC/AttributeSet.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// InputFactory
//===----------------------------------------------------------------------===//
InputFactory::InputFactory(size_t pNum, const LinkerConfig& pConfig)
    : GCFactory<Input, 0>(pNum) {
  m_pAttrSet = new AttributeSet(16, pConfig.attribute().predefined());
  m_pLast = new AttributeProxy(*m_pAttrSet,
                               pConfig.attribute().predefined(),
                               pConfig.attribute().constraint());
}

InputFactory::~InputFactory() {
  delete m_pAttrSet;
  delete m_pLast;
}

Input* InputFactory::produce(llvm::StringRef pName,
                             const sys::fs::Path& pPath,
                             unsigned int pType,
                             off_t pFileOffset) {
  Input* result = Alloc::allocate();
  new (result) Input(pName, pPath, *m_pLast, pType, pFileOffset);
  return result;
}

Input* InputFactory::produce(llvm::StringRef pName,
                             const char* pPath,
                             unsigned int pType,
                             off_t pFileOffset) {
  Input* result = Alloc::allocate();
  new (result) Input(pName, sys::fs::Path(pPath), *m_pLast, pType, pFileOffset);
  return result;
}

}  // namespace mcld
