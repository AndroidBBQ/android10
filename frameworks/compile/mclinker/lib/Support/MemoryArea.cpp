//===- MemoryArea.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/ErrorOr.h>

#include <cassert>
#include <system_error>

namespace mcld {

//===--------------------------------------------------------------------===//
// MemoryArea
//===--------------------------------------------------------------------===//
MemoryArea::MemoryArea(llvm::StringRef pFilename) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer> > buffer_or_error =
      llvm::MemoryBuffer::getFile(pFilename,
                                  /*FileSize*/ -1,
                                  /*RequiresNullTerminator*/ false);
  if (!buffer_or_error) {
    fatal(diag::fatal_cannot_read_input) << pFilename.str();
  }
  m_pMemoryBuffer = std::move(buffer_or_error.get());
}

MemoryArea::MemoryArea(const char* pMemBuffer, size_t pSize) {
  llvm::StringRef mem(pMemBuffer, pSize);
  m_pMemoryBuffer =
      llvm::MemoryBuffer::getMemBuffer(mem,
                                       /*BufferName*/ "NaN",
                                       /*RequiresNullTerminator*/ false);
}

llvm::StringRef MemoryArea::request(size_t pOffset, size_t pLength) {
  return llvm::StringRef(m_pMemoryBuffer->getBufferStart() + pOffset, pLength);
}

size_t MemoryArea::size() const {
  return m_pMemoryBuffer->getBufferSize();
}

}  // namespace mcld
