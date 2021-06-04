//===- MemoryArea.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_MEMORYAREA_H_
#define MCLD_SUPPORT_MEMORYAREA_H_

#include "mcld/Support/Compiler.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>

namespace mcld {

/** \class MemoryArea
 *  \brief MemoryArea is used to manage input read-only memory space.
 */
class MemoryArea {
  friend class MemoryAreaFactory;

 public:
  // constructor by file handler.
  // If the given file handler is read-only, client can not request a region
  // that out of the file size.
  // @param pFileHandle - file handler
  explicit MemoryArea(llvm::StringRef pFilename);

  explicit MemoryArea(const char* pMemBuffer, size_t pSize);

  // request - create a MemoryRegion within a sufficient space
  // find an existing space to hold the MemoryRegion.
  // if MemoryArea does not find such space, then it creates a new space and
  // assign a MemoryRegion into the space.
  llvm::StringRef request(size_t pOffset, size_t pLength);

  size_t size() const;

 private:
  std::unique_ptr<llvm::MemoryBuffer> m_pMemoryBuffer;

 private:
  DISALLOW_COPY_AND_ASSIGN(MemoryArea);
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_MEMORYAREA_H_
