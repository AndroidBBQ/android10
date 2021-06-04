//===- FileToken.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/FileToken.h"

#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

typedef GCFactory<FileToken, MCLD_SYMBOLS_PER_INPUT> FileTokenFactory;
static llvm::ManagedStatic<FileTokenFactory> g_FileTokenFactory;

//===----------------------------------------------------------------------===//
// FileToken
//===----------------------------------------------------------------------===//
FileToken::FileToken() {
}

FileToken::FileToken(const std::string& pName, bool pAsNeeded)
    : InputToken(InputToken::File, pName, pAsNeeded) {
}

FileToken::~FileToken() {
}

FileToken* FileToken::create(const std::string& pName, bool pAsNeeded) {
  FileToken* result = g_FileTokenFactory->allocate();
  new (result) FileToken(pName, pAsNeeded);
  return result;
}

void FileToken::destroy(FileToken*& pFileToken) {
  g_FileTokenFactory->destroy(pFileToken);
  g_FileTokenFactory->deallocate(pFileToken);
  pFileToken = NULL;
}

void FileToken::clear() {
  g_FileTokenFactory->clear();
}

}  // namespace mcld
