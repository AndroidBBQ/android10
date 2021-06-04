//===- MemoryAreaFactory.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_MEMORYAREAFACTORY_H_
#define MCLD_SUPPORT_MEMORYAREAFACTORY_H_
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/GCFactory.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/Path.h"

#include <llvm/ADT/StringMap.h>

namespace mcld {

/** \class MemoryAreaFactory
 *  \brief MemoryAreaFactory avoids creating duplicated MemoryAreas of the
 *   same file.
 *
 *  Users can give duplicated input files on the command line. In order to
 *  prevent opening the same file twice, and create redundant MemoryRegions,
 *  mcld::Input should not create MemoryArea directly. Instead, it should ask
 *  MemoryAreaFactory and get the unique MemoryArea.
 *
 *  The timing of opening and closing files is not strictly bound to the
 *  constructor and destructor of MCLDFile. For mcld::Output, MCLinker
 *  opens the file rather after assigning offset to sections. On the other
 *  aside, mcld::Input opens the file at constructor. In order to hide the
 *  file operations, MemoryAreaFactory actually open the file untill the first
 *  MemoryRegion is requested.
 *
 *  @see MemoryRegion
 */
class MemoryAreaFactory : public GCFactory<MemoryArea, 0> {
 public:
  explicit MemoryAreaFactory(size_t pNum);

  virtual ~MemoryAreaFactory();

  // produce - create a MemoryArea and open its file.
  MemoryArea* produce(const sys::fs::Path& pPath, FileHandle::OpenMode pMode);

  // produce - create a MemoryArea and open its file.
  MemoryArea* produce(const sys::fs::Path& pPath,
                      FileHandle::OpenMode pMode,
                      FileHandle::Permission pPerm);

  // Create a MemoryArea with an universal space.
  // The created MemoryArea is not moderated by m_HandleToArea.
  MemoryArea* produce(void* pMemBuffer, size_t pSize);

  // Create a MemoryArea by the given file handler
  // The created MemoryArea is not moderated by m_HandleToArea.
  MemoryArea* produce(int pFD, FileHandle::OpenMode pMode);

  void destruct(MemoryArea* pArea);

 private:
  llvm::StringMap<MemoryArea*> m_AreaMap;
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_MEMORYAREAFACTORY_H_
