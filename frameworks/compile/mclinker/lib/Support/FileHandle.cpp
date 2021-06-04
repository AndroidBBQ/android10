//===- FileHandle.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Config/Config.h"
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/FileSystem.h"

#include <errno.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <sys/stat.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// FileHandle
//===----------------------------------------------------------------------===//
FileHandle::FileHandle()
    : m_Path(),
      m_Handler(-1),
      m_Size(0),
      m_State(GoodBit),
      m_OpenMode(NotOpen) {
}

FileHandle::~FileHandle() {
  if (isOpened())
    close();
}

inline static int oflag(FileHandle::OpenMode pMode) {
  int result = 0x0;
  if (FileHandle::Unknown == pMode)
    return result;

  if (FileHandle::ReadWrite == (pMode & FileHandle::ReadWrite))
    result |= O_RDWR;
  else if (FileHandle::ReadOnly == (pMode & FileHandle::ReadOnly))
    result |= O_RDONLY;
  else if (FileHandle::WriteOnly == (pMode & FileHandle::WriteOnly))
    result |= O_WRONLY;

  if (FileHandle::Append == (pMode & FileHandle::Append))
    result |= O_APPEND;

  if (FileHandle::Create == (pMode & FileHandle::Create))
    result |= O_CREAT;

  if (FileHandle::Truncate == (pMode & FileHandle::Truncate))
    result |= O_TRUNC;

  return result;
}

inline static bool get_size(int pHandler, unsigned int& pSize) {
  struct ::stat file_stat;
  if (-1 == ::fstat(pHandler, &file_stat)) {
    pSize = 0;
    return false;
  }
  pSize = file_stat.st_size;
  return true;
}

bool FileHandle::open(const sys::fs::Path& pPath,
                      FileHandle::OpenMode pMode,
                      FileHandle::Permission pPerm) {
  if (isOpened() || Unknown == pMode) {
    setState(BadBit);
    return false;
  }

  m_OpenMode = pMode;
  if (System == pPerm)
    m_Handler = sys::fs::detail::open(pPath, oflag(pMode));
  else
    m_Handler = sys::fs::detail::open(pPath, oflag(pMode),
                                      static_cast<int>(pPerm));

  m_Path = pPath;
  if (m_Handler == -1) {
    m_OpenMode = OpenMode(NotOpen);
    setState(FailBit);
    return false;
  }

  if (!get_size(m_Handler, m_Size)) {
    setState(FailBit);
    return false;
  }

  return true;
}

bool FileHandle::delegate(int pFD, FileHandle::OpenModeEnum pMode) {
  if (isOpened()) {
    setState(BadBit);
    return false;
  }

  m_Handler = pFD;
  m_OpenMode = OpenMode(pMode);
  m_State = (GoodBit | DeputedBit);

  if (!get_size(m_Handler, m_Size)) {
    setState(FailBit);
    return false;
  }

  return true;
}

bool FileHandle::close() {
  if (!isOpened()) {
    setState(BadBit);
    return false;
  }

  if (isOwned()) {
    if (::close(m_Handler) == -1) {
      setState(FailBit);
      return false;
    }
  }

  m_Path.native().clear();
  m_Size = 0;
  m_OpenMode = OpenMode(NotOpen);
  cleanState();
  return true;
}

bool FileHandle::truncate(size_t pSize) {
  if (!isOpened() || !isWritable()) {
    setState(BadBit);
    return false;
  }

  if (sys::fs::detail::ftruncate(m_Handler, pSize) == -1) {
    setState(FailBit);
    return false;
  }

  m_Size = pSize;
  return true;
}

bool FileHandle::read(void* pMemBuffer, size_t pStartOffset, size_t pLength) {
  if (!isOpened() || !isReadable()) {
    setState(BadBit);
    return false;
  }

  if (pLength == 0)
    return true;

  ssize_t read_bytes =
      sys::fs::detail::pread(m_Handler, pMemBuffer, pLength, pStartOffset);

  if (read_bytes == -1) {
    setState(FailBit);
    return false;
  }

  return true;
}

bool FileHandle::write(const void* pMemBuffer,
                       size_t pStartOffset,
                       size_t pLength) {
  if (!isOpened() || !isWritable()) {
    setState(BadBit);
    return false;
  }

  if (pLength == 0)
    return true;

  ssize_t write_bytes =
      sys::fs::detail::pwrite(m_Handler, pMemBuffer, pLength, pStartOffset);

  if (write_bytes == -1) {
    setState(FailBit);
    return false;
  }

  return true;
}

void FileHandle::setState(FileHandle::IOState pState) {
  m_State |= pState;
}

void FileHandle::cleanState(FileHandle::IOState pState) {
  m_State = pState;
}

bool FileHandle::isOpened() const {
  if (m_Handler != -1 && m_OpenMode != NotOpen && isGood())
    return true;

  return false;
}

// Assume Unknown OpenMode is readable
bool FileHandle::isReadable() const {
  return (m_OpenMode & ReadOnly);
}

// Assume Unknown OpenMode is writable
bool FileHandle::isWritable() const {
  return (m_OpenMode & WriteOnly);
}

// Assume Unknown OpenMode is both readable and writable
bool FileHandle::isReadWrite() const {
  return (FileHandle::ReadWrite == (m_OpenMode & FileHandle::ReadWrite));
}

bool FileHandle::isGood() const {
  return !(m_State & (BadBit | FailBit));
}

bool FileHandle::isBad() const {
  return (m_State & BadBit);
}

bool FileHandle::isFailed() const {
  return (m_State & (BadBit | FailBit));
}

bool FileHandle::isOwned() const {
  return !(m_State & DeputedBit);
}

}  // namespace mcld
