//===- FileSystem.inc -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/Directory.h"

#include <llvm/Support/ErrorHandling.h>

#include <string>

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

namespace mcld {
namespace sys {
namespace fs {
namespace detail {

std::string static_library_extension = ".a";
std::string shared_library_extension = ".so";
std::string executable_extension = "";
std::string relocatable_extension = ".o";
std::string assembly_extension = ".s";
std::string bitcode_extension = ".bc";

//===----------------------------------------------------------------------===//
// Helper Functions
//===----------------------------------------------------------------------===//
/// read_dir - return true if we read one entry
//  @return value -1: read error
//                 0: read the end
//                 1: success
static int read_dir(intptr_t& pDir, std::string& pOutFilename) {
  errno = 0;
  dirent* cur_dir = ::readdir(reinterpret_cast<DIR*>(pDir));
  if (0 == cur_dir && 0 != errno)
    return -1;

  // idx does not stay at the end, but all elements had beed put into cache.
  if (NULL == cur_dir) {
    return 0;
  }

  llvm::StringRef name(cur_dir->d_name, strlen(cur_dir->d_name));
  if ((name.size() == 1 && name[0] == '.') ||
      (name.size() == 2 && name[0] == '.' && name[1] == '.'))
    return read_dir(pDir, pOutFilename);

  // find a new directory
  pOutFilename.append(name.data(), name.size());
  return 1;
}

void open_dir(Directory& pDir) {
  pDir.m_Handler = reinterpret_cast<intptr_t>(opendir(pDir.path().c_str()));
  if (0 == pDir.m_Handler) {
    errno = 0;  // opendir() will set errno if it failed to open directory.
    // set cache is full, then Directory::begin() can return end().
    pDir.m_CacheFull = true;
    return;
  }
  // read one entry for advance the end element of the cache.
  std::string path(pDir.path().native());
  switch (read_dir(pDir.m_Handler, path)) {
    case 1: {
      // find a new directory
      bool exist = false;
      mcld::sys::fs::PathCache::entry_type* entry =
          pDir.m_Cache.insert(path, exist);
      if (!exist)
        entry->setValue(sys::fs::Path(path));
      return;
    }
    case 0:
      // FIXME: a warning function
      pDir.m_CacheFull = true;
      return;
    default:
    case -1:
      llvm::report_fatal_error(std::string("Can't read directory: ") +
                               pDir.path().native());
  }
}

void close_dir(Directory& pDir) {
  if (pDir.m_Handler)
    closedir(reinterpret_cast<DIR*>(pDir.m_Handler));
  pDir.m_Handler = 0;
}

int open(const Path& pPath, int pOFlag) {
  return ::open(pPath.native().c_str(), pOFlag);
}

int open(const Path& pPath, int pOFlag, int pPerm) {
  mode_t perm = 0;
  if (pPerm & FileHandle::ReadOwner)
    perm |= S_IRUSR;
  if (pPerm & FileHandle::WriteOwner)
    perm |= S_IWUSR;
  if (pPerm & FileHandle::ExeOwner)
    perm |= S_IXUSR;
  if (pPerm & FileHandle::ReadGroup)
    perm |= S_IRGRP;
  if (pPerm & FileHandle::WriteGroup)
    perm |= S_IWGRP;
  if (pPerm & FileHandle::ExeGroup)
    perm |= S_IXGRP;
  if (pPerm & FileHandle::ReadOther)
    perm |= S_IROTH;
  if (pPerm & FileHandle::WriteOther)
    perm |= S_IWOTH;
  if (pPerm & FileHandle::ExeOther)
    perm |= S_IXOTH;

  return ::open(pPath.native().c_str(), pOFlag, perm);
}

ssize_t pread(int pFD, void* pBuf, size_t pCount, off_t pOffset) {
  return ::pread(pFD, pBuf, pCount, pOffset);
}

ssize_t pwrite(int pFD, const void* pBuf, size_t pCount, off_t pOffset) {
  return ::pwrite(pFD, pBuf, pCount, pOffset);
}

int ftruncate(int pFD, size_t pLength) {
  return ::ftruncate(pFD, pLength);
}

void get_pwd(Path& pPWD) {
  char* pwd = (char*)malloc(PATH_MAX);
  pPWD.assign(getcwd(pwd, PATH_MAX));
  free(pwd);
}

}  // namespace detail
}  // namespace fs
}  // namespace sys

//===----------------------------------------------------------------------===//
// FileHandler
//===----------------------------------------------------------------------===//
bool FileHandle::mmap(void*& pMemBuffer, size_t pStartOffset, size_t pLength) {
  if (!isOpened()) {
    setState(BadBit);
    return false;
  }

  if (0 == pLength)
    return true;

  int prot, flag;
  if (isReadable() && !isWritable()) {
    // read-only
    prot = PROT_READ;
    flag = MAP_FILE | MAP_PRIVATE;
  } else if (!isReadable() && isWritable()) {
    // write-only
    prot = PROT_WRITE;
    flag = MAP_FILE | MAP_SHARED;
  } else if (isReadWrite()) {
    // read and write
    prot = PROT_READ | PROT_WRITE;
    flag = MAP_FILE | MAP_SHARED;
  } else {
    // can not read/write
    setState(BadBit);
    return false;
  }

  pMemBuffer = ::mmap(NULL, pLength, prot, flag, m_Handler, pStartOffset);

  if (MAP_FAILED == pMemBuffer) {
    setState(FailBit);
    return false;
  }

  return true;
}

bool FileHandle::munmap(void* pMemBuffer, size_t pLength) {
  if (!isOpened()) {
    setState(BadBit);
    return false;
  }

  if (-1 == ::munmap(pMemBuffer, pLength)) {
    setState(FailBit);
    return false;
  }

  return true;
}

}  // namespace mcld
