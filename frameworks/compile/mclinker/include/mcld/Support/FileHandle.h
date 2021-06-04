//===- FileHandle.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_FILEHANDLE_H_
#define MCLD_SUPPORT_FILEHANDLE_H_
#include "mcld/ADT/Flags.h"
#include "mcld/Support/Path.h"

#include <errno.h>

namespace mcld {

/** \class FileHandle
 *  \brief FileHandle class provides an interface for reading from and writing
 *  to files.
 *
 *  Operators of FileHandle should neither throw exceptions nor call expressive
 *  diagnostic output.
 */
class FileHandle {
 public:
  enum IOState {
    GoodBit = 0,           // no error
    BadBit = 1L << 0,      // error due to the inappropriate operation
    EOFBit = 1L << 1,      // reached End-Of-File
    FailBit = 1L << 2,     // internal logic fail
    DeputedBit = 1L << 3,  // the file descriptor is delegated
    IOStateEnd = 1L << 16
  };

  enum OpenModeEnum {
    NotOpen = 0x00,
    ReadOnly = 0x01,
    WriteOnly = 0x02,
    ReadWrite = ReadOnly | WriteOnly,
    Append = 0x04,
    Create = 0x08,
    Truncate = 0x10,
    Unknown = 0xFF
  };

  typedef Flags<OpenModeEnum> OpenMode;

  enum PermissionEnum {
    ReadOwner = 0x0400,
    WriteOwner = 0x0200,
    ExeOwner = 0x0100,
    ReadGroup = 0x0040,
    WriteGroup = 0x0020,
    ExeGroup = 0x0010,
    ReadOther = 0x0004,
    WriteOther = 0x0002,
    ExeOther = 0x0001,
    System = 0xFFFF
  };

  typedef Flags<PermissionEnum> Permission;

 public:
  FileHandle();

  ~FileHandle();

  /// open - open the file.
  /// @return if we meet any trouble during opening the file, return false.
  ///         use rdstate() to see what happens.
  bool open(const sys::fs::Path& pPath, OpenMode pMode, Permission pPerm);

  bool delegate(int pFD, OpenModeEnum pMode = Unknown);

  bool close();

  void setState(IOState pState);

  void cleanState(IOState pState = GoodBit);

  // truncate - truncate the file up to the pSize.
  bool truncate(size_t pSize);

  bool read(void* pMemBuffer, size_t pStartOffset, size_t pLength);

  bool write(const void* pMemBuffer, size_t pStartOffset, size_t pLength);

  bool mmap(void*& pMemBuffer, size_t pStartOffset, size_t pLength);

  bool munmap(void* pMemBuffer, size_t pLength);

  // -----  observers  ----- //
  const sys::fs::Path& path() const { return m_Path; }

  size_t size() const { return m_Size; }

  int handler() const { return m_Handler; }

  uint16_t rdstate() const { return m_State; }

  bool isOpened() const;

  bool isGood() const;

  bool isBad() const;

  bool isFailed() const;

  bool isOwned() const;

  bool isReadable() const;

  bool isWritable() const;

  bool isReadWrite() const;

  int error() const { return errno; }

 private:
  sys::fs::Path m_Path;
  int m_Handler;
  unsigned int m_Size;
  uint16_t m_State;
  OpenMode m_OpenMode;
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_FILEHANDLE_H_
