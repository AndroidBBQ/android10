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

#include "FileBase.h"
#include "Log.h"

#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <new>

using namespace bcc;

#ifdef _WIN32
// TODO: Fix flock usage under windows
#define LOCK_SH 0
#define LOCK_EX 0
#define LOCK_NB 0
#define LOCK_UN 0

int flock(int fd, int operation) {
  return 0;
}
#endif  // _WIN32

FileBase::FileBase(const std::string &pFilename,
                   unsigned pOpenFlags,
                   unsigned pFlags)
  : mFD(-1),
    mError(),
    mName(pFilename), mOpenFlags(pOpenFlags),
    mShouldUnlock(false),
    mShouldDelete(false) {
  // Process pFlags
#ifdef O_BINARY
  if (pFlags & kBinary) {
    mOpenFlags |= O_BINARY;
  }
#endif
  if (pFlags & kTruncate) {
    mOpenFlags |= O_TRUNC;
  }

  if (pFlags & kAppend) {
    mOpenFlags |= O_APPEND;
  }

  if (pFlags & kDeleteOnClose) {
    mShouldDelete = true;
  }

  // Open the file.
  open();

  return;
}

FileBase::~FileBase() {
  close();
}

bool FileBase::open() {
  do {
    // FIXME: Hard-coded permissions (0644) for newly created file should be
    //        removed and provide a way to let the user configure the value.
    mFD = ::open(mName.c_str(), mOpenFlags, 0644);
    if (mFD > 0) {
      return true;
    }

    // Some errors occurred ...
    if (errno != EINTR) {
      detectError();
      return false;
    }
  } while (true);
  // unreachable
}


bool FileBase::checkFileIntegrity() {
  // Check the file integrity by examining whether the inode referring to the mFD
  // and to the file mName are the same.
  struct stat fd_stat, file_stat;

  // Get the file status of file descriptor mFD.
  do {
    if (::fstat(mFD, &fd_stat) == 0) {
      break;
    } else if (errno != EINTR) {
      detectError();
      return false;
    }
  } while (true);

  // Get the file status of file mName.
  do {
    if (::stat(mName.c_str(), &file_stat) == 0) {
      break;
    } else if (errno != EINTR) {
      detectError();
      return false;
    }
  } while (true);

  return ((fd_stat.st_dev == file_stat.st_dev) &&
          (fd_stat.st_ino == file_stat.st_ino));
}

void FileBase::detectError() {
  // Read error from errno.
  mError.assign(errno, std::generic_category());
}

bool FileBase::lock(enum LockModeEnum pMode,
                    bool pNonblocking,
                    unsigned pMaxRetry,
                    useconds_t pRetryInterval) {
  int lock_operation;
  unsigned retry = 0;

  // Check the state.
  if ((mFD < 0) || hasError()) {
    return false;
  }

  // Return immediately if it's already locked.
  if (mShouldUnlock) {
    return true;
  }

  // Determine the lock operation (2nd argument) to the flock().
  if (pMode == kReadLock) {
    lock_operation = LOCK_SH;
  } else if (pMode == kWriteLock) {
    lock_operation = LOCK_EX;
  } else {
    mError = std::make_error_code(std::errc::invalid_argument);
    return false;
  }

  if (pNonblocking) {
    lock_operation |= LOCK_NB;
  }

  do {
    if (::flock(mFD, lock_operation) == 0) {
      mShouldUnlock = true;
      // Here we got a lock but we need to check whether the mFD still
      // "represents" the filename (mName) we opened in the contructor. This
      // check may failed when another process deleted the original file mFD
      // mapped when we were trying to obtain the lock on the file.
      if (!checkFileIntegrity()) {
        if (hasError() || !reopen()) {
          // Error occurred when check the file integrity or re-open the file.
          return false;
        } else {
          // Wait a while before the next try.
          ::usleep(pRetryInterval);
          retry++;
          continue;
        }
      }

      return true;
    }

    // flock() was not performed successfully. Check the errno to see whether
    // it's retry-able.
    if (errno == EINTR) {
      // flock() was interrupted by delivery of a signal. Restart without
      // decrement the retry counter.
      continue;
    } else if (errno == EWOULDBLOCK) {
      // The file descriptor was locked by others, wait for a while before next
      // retry.
      retry++;
      ::usleep(pRetryInterval);
    } else {
      // There's a fatal error occurs when perform flock(). Return immediately
      // without further retry.
      detectError();
      return false;
    }
  } while (retry <= pMaxRetry);

  return false;
}

void FileBase::unlock() {
  if (mFD < 0) {
    return;
  }

  do {
    if (::flock(mFD, LOCK_UN) == 0) {
      mShouldUnlock = false;
      return;
    }
  } while (errno == EINTR);

  detectError();
  return;
}

void FileBase::close() {
  if (mShouldUnlock) {
    unlock();
    mShouldUnlock = false;
  }
  if (mFD > 0) {
    ::close(mFD);
    mFD = -1;
  }
  if (mShouldDelete) {
    int res = ::remove(mName.c_str());
    if (res != 0) {
      ALOGE("Failed to remove file: %s - %s", mName.c_str(), ::strerror(res));
    }
  }
  return;
}
