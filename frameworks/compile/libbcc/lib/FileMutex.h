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

#ifndef BCC_SUPPORT_FILE_MUTEX_H
#define BCC_SUPPORT_FILE_MUTEX_H

#include <string>

#include "FileBase.h"

namespace bcc {

// This class is used to acquire write locks.
// TODO(jeanluc) More documentation.
class FileMutex : public FileBase {
public:
  explicit FileMutex(const std::string &pFileToLock)
    : FileBase(pFileToLock + ".lock", O_RDONLY | O_CREAT, kDeleteOnClose) { }

  // Provide a lock() interface filled with default configuration.
  inline bool lockMutex() {
    return FileBase::lock(FileBase::kWriteLock, true, FileBase::kDefaultMaxRetryLock,
                          FileBase::kDefaultRetryLockInterval);
  }
};

} // namespace bcc

#endif  // BCC_SUPPORT_FILE_MUTEX_H
