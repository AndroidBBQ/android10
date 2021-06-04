/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "CallbackDataSource"
#include <utils/Log.h>

#include "include/CallbackDataSource.h"

#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <media/IDataSource.h>
#include <media/stagefright/foundation/ADebug.h>

#include <algorithm>

namespace android {

CallbackDataSource::CallbackDataSource(
    const sp<IDataSource>& binderDataSource)
    : mIDataSource(binderDataSource),
      mIsClosed(false) {
    // Set up the buffer to read into.
    mMemory = mIDataSource->getIMemory();
    mName = String8::format("CallbackDataSource(%d->%d, %s)",
            getpid(),
            IPCThreadState::self()->getCallingPid(),
            mIDataSource->toString().string());

}

CallbackDataSource::~CallbackDataSource() {
    ALOGV("~CallbackDataSource");
    close();
}

status_t CallbackDataSource::initCheck() const {
    if (mMemory == NULL) {
        return UNKNOWN_ERROR;
    }
    return OK;
}

ssize_t CallbackDataSource::readAt(off64_t offset, void* data, size_t size) {
    if (mMemory == NULL || data == NULL) {
        return -1;
    }

    // IDataSource can only read up to mMemory->size() bytes at a time, but this
    // method should be able to read any number of bytes, so read in a loop.
    size_t totalNumRead = 0;
    size_t numLeft = size;
    const size_t bufferSize = mMemory->size();

    while (numLeft > 0) {
        size_t numToRead = std::min(numLeft, bufferSize);
        ssize_t numRead =
            mIDataSource->readAt(offset + totalNumRead, numToRead);
        // A negative return value represents an error. Pass it on.
        if (numRead < 0) {
            return numRead == ERROR_END_OF_STREAM && totalNumRead > 0 ? totalNumRead : numRead;
        }
        // A zero return value signals EOS. Return the bytes read so far.
        if (numRead == 0) {
            return totalNumRead;
        }
        if ((size_t)numRead > numToRead) {
            return ERROR_OUT_OF_RANGE;
        }
        CHECK(numRead >= 0 && (size_t)numRead <= bufferSize);
        memcpy(((uint8_t*)data) + totalNumRead, mMemory->pointer(), numRead);
        numLeft -= numRead;
        totalNumRead += numRead;
    }

    return totalNumRead;
}

status_t CallbackDataSource::getSize(off64_t *size) {
    status_t err = mIDataSource->getSize(size);
    if (err != OK) {
        return err;
    }
    if (*size < 0) {
        // IDataSource will set size to -1 to indicate unknown size, but
        // DataSource returns ERROR_UNSUPPORTED for that.
        return ERROR_UNSUPPORTED;
    }
    return OK;
}

uint32_t CallbackDataSource::flags() {
    return mIDataSource->getFlags();
}

void CallbackDataSource::close() {
    if (!mIsClosed) {
        mIDataSource->close();
        mIsClosed = true;
    }
}

sp<DecryptHandle> CallbackDataSource::DrmInitialization(const char *mime) {
    return mIDataSource->DrmInitialization(mime);
}

sp<IDataSource> CallbackDataSource::getIDataSource() const {
    return mIDataSource;
}

TinyCacheSource::TinyCacheSource(const sp<DataSource>& source)
    : mSource(source), mCachedOffset(0), mCachedSize(0) {
    mName = String8::format("TinyCacheSource(%s)", mSource->toString().string());
}

status_t TinyCacheSource::initCheck() const {
    return mSource->initCheck();
}

ssize_t TinyCacheSource::readAt(off64_t offset, void* data, size_t size) {
    // Check if the cache satisfies the read.
    if (mCachedOffset <= offset
            && offset < (off64_t) (mCachedOffset + mCachedSize)) {
        if (offset + size <= mCachedOffset + mCachedSize) {
            memcpy(data, &mCache[offset - mCachedOffset], size);
            return size;
        } else {
            // If the cache hits only partially, flush the cache and read the
            // remainder.

            // This value is guaranteed to be greater than 0 because of the
            // enclosing if statement.
            const ssize_t remaining = mCachedOffset + mCachedSize - offset;
            memcpy(data, &mCache[offset - mCachedOffset], remaining);
            const ssize_t readMore = readAt(offset + remaining,
                    (uint8_t*)data + remaining, size - remaining);
            if (readMore < 0) {
                return readMore;
            }
            return remaining + readMore;
        }
    }

    if (size >= kCacheSize) {
        return mSource->readAt(offset, data, size);
    }

    // Fill the cache and copy to the caller.
    const ssize_t numRead = mSource->readAt(offset, mCache, kCacheSize);
    if (numRead <= 0) {
        // Flush cache on error
        mCachedSize = 0;
        mCachedOffset = 0;
        return numRead;
    }
    if ((size_t)numRead > kCacheSize) {
        // Flush cache on error
        mCachedSize = 0;
        mCachedOffset = 0;
        return ERROR_OUT_OF_RANGE;
    }

    mCachedSize = numRead;
    mCachedOffset = offset;
    CHECK(mCachedSize <= kCacheSize && mCachedOffset >= 0);
    const size_t numToReturn = std::min(size, (size_t)numRead);
    memcpy(data, mCache, numToReturn);

    return numToReturn;
}

status_t TinyCacheSource::getSize(off64_t *size) {
    return mSource->getSize(size);
}

uint32_t TinyCacheSource::flags() {
    return mSource->flags();
}

sp<DecryptHandle> TinyCacheSource::DrmInitialization(const char *mime) {
    // flush cache when DrmInitialization occurs since decrypted
    // data may differ from what is in cache.
    mCachedOffset = 0;
    mCachedSize = 0;
    return mSource->DrmInitialization(mime);
}

sp<IDataSource> TinyCacheSource::getIDataSource() const {
    return mSource->getIDataSource();
}

} // namespace android
