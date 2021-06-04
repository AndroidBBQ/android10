//===- FileOutputBuffer.h -------------------------------------------------===//
//
//                     the mclinker project
//
// this file is distributed under the university of illinois open source
// license. see license.txt for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_FILEOUTPUTBUFFER_H_
#define MCLD_SUPPORT_FILEOUTPUTBUFFER_H_

#include "mcld/Support/MemoryRegion.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/FileSystem.h>

#include <system_error>

namespace mcld {

class FileHandle;

/// FileOutputBuffer - This interface is borrowed from llvm bassically, and we
/// may use ostream to emit output later.
class FileOutputBuffer {
 public:
  /// Factory method to create an OutputBuffer object which manages a read/write
  /// buffer of the specified size. When committed, the buffer will be written
  /// to the file at the specified path.
  static std::error_code create(FileHandle& pFileHandle,
                                size_t pSize,
                                std::unique_ptr<FileOutputBuffer>& pResult);

  /// Returns a pointer to the start of the buffer.
  uint8_t* getBufferStart() {
    return reinterpret_cast<uint8_t*>(m_pRegion->data());
  }

  /// Returns a pointer to the end of the buffer.
  uint8_t* getBufferEnd() {
    return reinterpret_cast<uint8_t*>(m_pRegion->data()) + m_pRegion->size();
  }

  /// Returns size of the buffer.
  size_t getBufferSize() const { return m_pRegion->size(); }

  MemoryRegion request(size_t pOffset, size_t pLength);

  /// Returns path where file will show up if buffer is committed.
  llvm::StringRef getPath() const;

  ~FileOutputBuffer();

 private:
  FileOutputBuffer(const FileOutputBuffer&);
  FileOutputBuffer& operator=(const FileOutputBuffer&);

  FileOutputBuffer(llvm::sys::fs::mapped_file_region* pRegion,
                   FileHandle& pFileHandle);

  std::unique_ptr<llvm::sys::fs::mapped_file_region> m_pRegion;
  FileHandle& m_FileHandle;
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_FILEOUTPUTBUFFER_H_
