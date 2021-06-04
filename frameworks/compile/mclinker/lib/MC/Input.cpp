//===- Input.cpp ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/Input.h"

#include "mcld/MC/Attribute.h"
#include "mcld/LD/LDContext.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// mcld::Input
//===----------------------------------------------------------------------===//
Input::Input(llvm::StringRef pName)
    : m_Type(Unknown),
      m_Name(pName.data()),
      m_Path(),
      m_pAttr(NULL),
      m_bNeeded(false),
      m_bNoExport(false),
      m_fileOffset(0),
      m_pMemArea(NULL),
      m_pContext(NULL) {
}

Input::Input(llvm::StringRef pName, const AttributeProxy& pProxy)
    : m_Type(Unknown),
      m_Name(pName.data()),
      m_Path(),
      m_pAttr(const_cast<Attribute*>(pProxy.attr())),
      m_bNeeded(false),
      m_bNoExport(false),
      m_fileOffset(0),
      m_pMemArea(NULL),
      m_pContext(NULL) {
}

Input::Input(llvm::StringRef pName,
             const sys::fs::Path& pPath,
             unsigned int pType,
             off_t pFileOffset)
    : m_Type(pType),
      m_Name(pName.data()),
      m_Path(pPath),
      m_pAttr(NULL),
      m_bNeeded(false),
      m_bNoExport(false),
      m_fileOffset(pFileOffset),
      m_pMemArea(NULL),
      m_pContext(NULL) {
}

Input::Input(llvm::StringRef pName,
             const sys::fs::Path& pPath,
             const AttributeProxy& pProxy,
             unsigned int pType,
             off_t pFileOffset)
    : m_Type(pType),
      m_Name(pName.data()),
      m_Path(pPath),
      m_pAttr(const_cast<Attribute*>(pProxy.attr())),
      m_bNeeded(false),
      m_bNoExport(false),
      m_fileOffset(pFileOffset),
      m_pMemArea(NULL),
      m_pContext(NULL) {
}

Input::~Input() {
  // Attribute is deleted by AttributeFactory
  // MemoryArea is deleted by MemoryAreaFactory
}

}  // namespace mcld
