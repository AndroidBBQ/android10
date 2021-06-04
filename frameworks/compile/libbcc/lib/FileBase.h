/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BCC_SUPPORT_FILE_BASE_H
#define BCC_SUPPORT_FILE_BASE_H

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <system_error>

namespace android {
  class FileMap;
}

namespace bcc {

class FileBase {
public:
  enum OpenModeEnum {
    kReadMode       = 1 << 0,
    kWriteMode      = 1 << 1,
    kReadWriteMode  = (kReadMode | kWriteMode),
  };

  enum FlagEnum {
    kBinary = 1 << 0,
    kTruncate = 1 << 1,
    kAppend = 1 << 2,
    kDeleteOnClose = 1 << 3
  };

  enum LockModeEnum {
    // The shared resource behind the Stream locked in ReadLock mode can be
    // locked by other processes at the same time.
    kReadLock,

    // The shared resource behind the Stream locked in WriteLock mode can only
    // be locked by one process. It's exclusive. That is, the shared resource
    // cannot have both ReadLock and WriteLock simultaneously.
    kWriteLock
  };

  // Default configuration to the lock().
  enum {
    kDefaultMaxRetryLock = 4,
    kDefaultRetryLockInterval = 200000UL,
  };

protected:
  // Grant direct access of the internal file descriptor to the sub-class and
  // error message such that they can implement their own I/O functionality.
  int mFD;

  std::error_code mError;

private:
  std::string mName;

  // The 2nd argument to the POSIX open().
  unsigned mOpenFlags;

  // True true if we should call unlock() in destructor.
  bool mShouldUnlock;

  // True if file should be deleted in destructor.
  bool mShouldDelete;

  // Open mName with flag mOpenFlags (using POSIX open().)
  bool open();

  // Return true if mFD is the corresponded file descriptor to the file named
  // mName on the filesystem. This check may returns failed, for example,
  // someone re-create the file with the same name after we openning the file.
  bool checkFileIntegrity();

  inline bool reopen() {
    // It's a private method, and all its callers are the few that can invoke it.
    // That is, the pre-condition will be checked by the caller. Therefore, we don't
    // need to check it again in reopen().
    close();
    return open();
  }

private:
  FileBase(FileBase &); // Do not implement.
  void operator=(const FileBase &); // Do not implement.

protected:
  // pOpenFlags is the 2nd argument to the POSIX open(). pFlags are the flags to
  // FileBase. It's a bit set composed by the value defined in
  // FileBase::FlagEnum.
  FileBase(const std::string &pFilename, unsigned pOpenFlags, unsigned pFlags);

  void detectError();

public:
  // Lock the file descriptor in given pMode. If pNonblocking is true, the lock
  // request issued will return immediately when the shared resource is locked.
  // In this case, it retries pMaxRetry times, each wait pRetryInterval (in
  // usecs) before the previous retry getting done.
  //
  // Only file is allowed to use this API.
  bool lock(enum LockModeEnum pMode, bool pNonblocking = true,
            unsigned pMaxRetry = kDefaultMaxRetryLock,
            useconds_t pRetryInterval = kDefaultRetryLockInterval);

  void unlock();

  inline bool hasError() const
  { return (bool) mError; }

  inline const std::error_code &getError() const
  { return mError; }

  // The return value of std::error_code::message() is obtained upon the call
  // and is passed by value (that is, it's not a member of std::error_code.)
  inline std::string getErrorMessage() const
  { return mError.message(); }

  void close();

  virtual ~FileBase();
};

} // end namespace bcc

#endif  // BCC_SUPPORT_FILE_BASE_H
