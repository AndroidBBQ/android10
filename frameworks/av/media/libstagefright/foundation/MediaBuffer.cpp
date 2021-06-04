/*
 * Copyright (C) 2009 The Android Open Source Project
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

#define LOG_TAG "MediaBuffer"
#include <utils/Log.h>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaData.h>

namespace android {

/* static */
std::atomic_int_least32_t MediaBuffer::mUseSharedMemory(0);

MediaBuffer::MediaBuffer(void *data, size_t size)
    : mObserver(NULL),
      mRefCount(0),
      mData(data),
      mSize(size),
      mRangeOffset(0),
      mRangeLength(size),
      mOwnsData(false),
      mMetaData(new MetaDataBase) {
}

MediaBuffer::MediaBuffer(size_t size)
    : mObserver(NULL),
      mRefCount(0),
      mData(NULL),
      mSize(size),
      mRangeOffset(0),
      mRangeLength(size),
      mOwnsData(true),
      mMetaData(new MetaDataBase) {
#ifndef NO_IMEMORY
    if (size < kSharedMemThreshold
            || std::atomic_load_explicit(&mUseSharedMemory, std::memory_order_seq_cst) == 0) {
#endif
        mData = malloc(size);
#ifndef NO_IMEMORY
    } else {
        ALOGV("creating memoryDealer");
        size_t newSize = 0;
        if (!__builtin_add_overflow(size, sizeof(SharedControl), &newSize)) {
            sp<MemoryDealer> memoryDealer = new MemoryDealer(newSize, "MediaBuffer");
            mMemory = memoryDealer->allocate(newSize);
        }
        if (mMemory == NULL) {
            ALOGW("Failed to allocate shared memory, trying regular allocation!");
            mData = malloc(size);
            if (mData == NULL) {
                ALOGE("Out of memory");
            }
        } else {
            getSharedControl()->clear();
            mData = (uint8_t *)mMemory->pointer() + sizeof(SharedControl);
            ALOGV("Allocated shared mem buffer of size %zu @ %p", size, mData);
        }
    }
#endif
}

MediaBuffer::MediaBuffer(const sp<ABuffer> &buffer)
    : mObserver(NULL),
      mRefCount(0),
      mData(buffer->data()),
      mSize(buffer->size()),
      mRangeOffset(0),
      mRangeLength(mSize),
      mBuffer(buffer),
      mOwnsData(false),
      mMetaData(new MetaDataBase) {
}

void MediaBuffer::release() {
    if (mObserver == NULL) {
        // Legacy contract for MediaBuffer without a MediaBufferGroup.
        CHECK_EQ(mRefCount, 0);
        delete this;
        return;
    }

    int prevCount = mRefCount.fetch_sub(1);
    if (prevCount == 1) {
        if (mObserver == NULL) {
            delete this;
            return;
        }

        mObserver->signalBufferReturned(this);
    }
    CHECK(prevCount > 0);
}

void MediaBuffer::claim() {
    CHECK(mObserver != NULL);
    CHECK_EQ(mRefCount.load(std::memory_order_relaxed), 1);

    mRefCount.store(0, std::memory_order_relaxed);
}

void MediaBuffer::add_ref() {
    (void) mRefCount.fetch_add(1);
}

void *MediaBuffer::data() const {
    return mData;
}

size_t MediaBuffer::size() const {
    return mSize;
}

size_t MediaBuffer::range_offset() const {
    return mRangeOffset;
}

size_t MediaBuffer::range_length() const {
    return mRangeLength;
}

void MediaBuffer::set_range(size_t offset, size_t length) {
    if (offset + length > mSize) {
        ALOGE("offset = %zu, length = %zu, mSize = %zu", offset, length, mSize);
    }
    CHECK(offset + length <= mSize);

    mRangeOffset = offset;
    mRangeLength = length;
}

MetaDataBase& MediaBuffer::meta_data() {
    return *mMetaData;
}

void MediaBuffer::reset() {
    mMetaData->clear();
    set_range(0, mSize);
}

MediaBuffer::~MediaBuffer() {
    CHECK(mObserver == NULL);

    if (mOwnsData && mData != NULL && mMemory == NULL) {
        free(mData);
        mData = NULL;
    }

   if (mMemory.get() != nullptr) {
       getSharedControl()->setDeadObject();
   }
   delete mMetaData;
}

void MediaBuffer::setObserver(MediaBufferObserver *observer) {
    CHECK(observer == NULL || mObserver == NULL);
    mObserver = observer;
}

}  // namespace android
