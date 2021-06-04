//===- FileOutputBuffer.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/FileOutputBuffer.h"
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/Path.h"

namespace mcld {

FileOutputBuffer::FileOutputBuffer(llvm::sys::fs::mapped_file_region* pRegion,
                                   FileHandle& pFileHandle)
    : m_pRegion(pRegion), m_FileHandle(pFileHandle) {
}

FileOutputBuffer::~FileOutputBuffer() {
  // Unmap buffer, letting OS flush dirty pages to file on disk.
  m_pRegion.reset();
}

std::error_code
FileOutputBuffer::create(FileHandle& pFileHandle,
                         size_t pSize,
                         std::unique_ptr<FileOutputBuffer>& pResult) {
  std::error_code ec;

  // Resize the file before mapping the file region.
  ec = llvm::sys::fs::resize_file(pFileHandle.handler(), pSize);
  if (ec)
    return ec;

  std::unique_ptr<llvm::sys::fs::mapped_file_region> mapped_file(
      new llvm::sys::fs::mapped_file_region(pFileHandle.handler(),
          llvm::sys::fs::mapped_file_region::readwrite, pSize, 0, ec));
  if (ec)
    return ec;

  pResult.reset(new FileOutputBuffer(mapped_file.get(), pFileHandle));
  if (pResult)
    mapped_file.release();

  return std::error_code();
}

MemoryRegion FileOutputBuffer::request(size_t pOffset, size_t pLength) {
  if (pOffset > getBufferSize() || (pOffset + pLength) > getBufferSize())
    return MemoryRegion();
  return MemoryRegion(getBufferStart() + pOffset, pLength);
}

llvm::StringRef FileOutputBuffer::getPath() const {
  return m_FileHandle.path().native();
}

}  // namespace mcld
