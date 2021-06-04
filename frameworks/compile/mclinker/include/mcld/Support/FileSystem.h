//===- FileSystem.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This file declares the mcld::sys::fs:: namespace. It follows TR2/boost
// filesystem (v3), but modified to remove exception handling and the
// path class.
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_FILESYSTEM_H_
#define MCLD_SUPPORT_FILESYSTEM_H_

#include "mcld/Config/Config.h"
#include "mcld/Support/PathCache.h"

#include <iosfwd>
#include <locale>
#include <string>

namespace mcld {
namespace sys {

namespace fs {

enum FileType {
  StatusError,
  StatusUnknown = StatusError,
  FileNotFound,
  RegularFile,
  DirectoryFile,
  SymlinkFile,
  BlockFile,
  CharacterFile,
  FifoFile,
  SocketFile,
  ReparseFile,
  TypeUnknown,
  StatusKnown,
  IsSymLink
};

/** \class FileStatus
 *  \brief FileStatus
 */
class FileStatus {
 public:
  FileStatus() : m_Value(StatusError) {}

  explicit FileStatus(FileType v) : m_Value(v) {}

  void setType(FileType v) { m_Value = v; }
  FileType type() const { return m_Value; }

 private:
  FileType m_Value;
};

inline bool operator==(const FileStatus& rhs, const FileStatus& lhs) {
  return rhs.type() == lhs.type();
}

inline bool operator!=(const FileStatus& rhs, const FileStatus& lhs) {
  return !(rhs == lhs);
}

class Path;
class DirIterator;
class Directory;

bool exists(const Path& pPath);
bool is_directory(const Path& pPath);

namespace detail {

extern Path::StringType static_library_extension;
extern Path::StringType shared_library_extension;
extern Path::StringType executable_extension;
extern Path::StringType relocatable_extension;
extern Path::StringType assembly_extension;
extern Path::StringType bitcode_extension;

size_t canonicalize(Path::StringType& pPathName);
bool not_found_error(int perrno);
void status(const Path& p, FileStatus& pFileStatus);
void symlink_status(const Path& p, FileStatus& pFileStatus);
mcld::sys::fs::PathCache::entry_type* bring_one_into_cache(DirIterator& pIter);
void open_dir(Directory& pDir);
void close_dir(Directory& pDir);
void get_pwd(Path& pPWD);

int open(const Path& pPath, int pOFlag);
int open(const Path& pPath, int pOFlag, int pPermission);
ssize_t pread(int pFD, void* pBuf, size_t pCount, off_t pOffset);
ssize_t pwrite(int pFD, const void* pBuf, size_t pCount, off_t pOffset);
int ftruncate(int pFD, size_t pLength);
void* mmap(void* pAddr,
           size_t pLen,
           int pProt,
           int pFlags,
           int pFD,
           off_t pOffset);
int munmap(void* pAddr, size_t pLen);

}  // namespace detail
}  // namespace fs
}  // namespace sys
}  // namespace mcld

#endif  // MCLD_SUPPORT_FILESYSTEM_H_
