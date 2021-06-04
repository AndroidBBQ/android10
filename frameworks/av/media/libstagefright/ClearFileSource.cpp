/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "ClearFileSource"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/ClearFileSource.h>
#include <media/stagefright/Utils.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace android {

ClearFileSource::ClearFileSource(const char *filename)
    : mFd(-1),
      mOffset(0),
      mLength(-1),
      mName("<null>") {

    if (filename) {
        mName = String8::format("FileSource(%s)", filename);
    }
    ALOGV("%s", filename);
    mFd = open(filename, O_LARGEFILE | O_RDONLY);

    if (mFd >= 0) {
        mLength = lseek64(mFd, 0, SEEK_END);
    } else {
        ALOGE("Failed to open file '%s'. (%s)", filename, strerror(errno));
    }
}

ClearFileSource::ClearFileSource(int fd, int64_t offset, int64_t length)
    : mFd(fd),
      mOffset(offset),
      mLength(length),
      mName("<null>") {
    ALOGV("fd=%d (%s), offset=%lld, length=%lld",
            fd, nameForFd(fd).c_str(), (long long) offset, (long long) length);

    if (mOffset < 0) {
        mOffset = 0;
    }
    if (mLength < 0) {
        mLength = 0;
    }
    if (mLength > INT64_MAX - mOffset) {
        mLength = INT64_MAX - mOffset;
    }
    struct stat s;
    if (fstat(fd, &s) == 0) {
        if (mOffset > s.st_size) {
            mOffset = s.st_size;
            mLength = 0;
        }
        if (mOffset + mLength > s.st_size) {
            mLength = s.st_size - mOffset;
        }
    }
    if (mOffset != offset || mLength != length) {
        ALOGW("offset/length adjusted from %lld/%lld to %lld/%lld",
                (long long) offset, (long long) length,
                (long long) mOffset, (long long) mLength);
    }

    mName = String8::format(
            "FileSource(fd(%s), %lld, %lld)",
            nameForFd(fd).c_str(),
            (long long) mOffset,
            (long long) mLength);

}

ClearFileSource::~ClearFileSource() {
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
}

status_t ClearFileSource::initCheck() const {
    return mFd >= 0 ? OK : NO_INIT;
}

ssize_t ClearFileSource::readAt(off64_t offset, void *data, size_t size) {
    if (mFd < 0) {
        return NO_INIT;
    }

    Mutex::Autolock autoLock(mLock);
    if (mLength >= 0) {
        if (offset >= mLength) {
            return 0;  // read beyond EOF.
        }
        uint64_t numAvailable = mLength - offset;
        if ((uint64_t)size > numAvailable) {
            size = numAvailable;
        }
    }
    return readAt_l(offset, data, size);
}

ssize_t ClearFileSource::readAt_l(off64_t offset, void *data, size_t size) {
    off64_t result = lseek64(mFd, offset + mOffset, SEEK_SET);
    if (result == -1) {
        ALOGE("seek to %lld failed", (long long)(offset + mOffset));
        return UNKNOWN_ERROR;
    }

    return ::read(mFd, data, size);
}

status_t ClearFileSource::getSize(off64_t *size) {
    Mutex::Autolock autoLock(mLock);

    if (mFd < 0) {
        return NO_INIT;
    }

    *size = mLength;

    return OK;
}

}  // namespace android
