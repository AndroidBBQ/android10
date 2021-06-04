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

#define LOG_TAG "MediaBufferGroup"
#include <utils/Log.h>

#include <list>

#include <binder/MemoryDealer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <utils/threads.h>

namespace android {

// std::min is not constexpr in C++11
template<typename T>
constexpr T MIN(const T &a, const T &b) { return a <= b ? a : b; }

// MediaBufferGroup may create shared memory buffers at a
// smaller threshold than an isolated new MediaBuffer.
static const size_t kSharedMemoryThreshold = MIN(
        (size_t)MediaBuffer::kSharedMemThreshold, (size_t)(4 * 1024));

struct MediaBufferGroup::InternalData {
    Mutex mLock;
    Condition mCondition;
    size_t mGrowthLimit;  // Do not automatically grow group larger than this.
    std::list<MediaBufferBase *> mBuffers;
};

MediaBufferGroup::MediaBufferGroup(size_t growthLimit)
    : mWrapper(nullptr), mInternal(new InternalData()) {
    mInternal->mGrowthLimit = growthLimit;
}

MediaBufferGroup::MediaBufferGroup(size_t buffers, size_t buffer_size, size_t growthLimit)
    : mWrapper(nullptr), mInternal(new InternalData()) {
    init(buffers, buffer_size, growthLimit);
}

void MediaBufferGroup::init(size_t buffers, size_t buffer_size, size_t growthLimit) {
    mInternal->mGrowthLimit = growthLimit;

    if (mInternal->mGrowthLimit > 0 && buffers > mInternal->mGrowthLimit) {
        ALOGW("Preallocated buffers %zu > growthLimit %zu, increasing growthLimit",
                buffers, mInternal->mGrowthLimit);
        mInternal->mGrowthLimit = buffers;
    }

#ifndef NO_IMEMORY
    if (buffer_size >= kSharedMemoryThreshold) {
        ALOGD("creating MemoryDealer");
        // Using a single MemoryDealer is efficient for a group of shared memory objects.
        // This loop guarantees that we use shared memory (no fallback to malloc).

        size_t alignment = MemoryDealer::getAllocationAlignment();
        size_t augmented_size = buffer_size + sizeof(MediaBuffer::SharedControl);
        size_t total = (augmented_size + alignment - 1) / alignment * alignment * buffers;
        sp<MemoryDealer> memoryDealer = new MemoryDealer(total, "MediaBufferGroup");

        for (size_t i = 0; i < buffers; ++i) {
            sp<IMemory> mem = memoryDealer->allocate(augmented_size);
            if (mem.get() == nullptr || mem->pointer() == nullptr) {
                ALOGW("Only allocated %zu shared buffers of size %zu", i, buffer_size);
                break;
            }
            MediaBuffer *buffer = new MediaBuffer(mem);
            buffer->getSharedControl()->clear();
            add_buffer(buffer);
        }
        return;
    }
#else
    (void)kSharedMemoryThreshold;
#endif

    // Non-shared memory allocation.
    for (size_t i = 0; i < buffers; ++i) {
        MediaBuffer *buffer = new MediaBuffer(buffer_size);
        if (buffer->data() == nullptr) {
            delete buffer; // don't call release, it's not properly formed
            ALOGW("Only allocated %zu malloc buffers of size %zu", i, buffer_size);
            break;
        }
        add_buffer(buffer);
    }
}

MediaBufferGroup::~MediaBufferGroup() {
    for (MediaBufferBase *buffer : mInternal->mBuffers) {
        if (buffer->refcount() != 0) {
            const int localRefcount = buffer->localRefcount();
            const int remoteRefcount = buffer->remoteRefcount();

            // Fatal if we have a local refcount.
            LOG_ALWAYS_FATAL_IF(localRefcount != 0,
                    "buffer(%p) localRefcount %d != 0, remoteRefcount %d",
                    buffer, localRefcount, remoteRefcount);

            // Log an error if we have a remaining remote refcount,
            // as the remote process may have died or may have inappropriate behavior.
            // The shared memory associated with the MediaBuffer will
            // automatically be reclaimed when there are no remaining fds
            // associated with it.
            ALOGE("buffer(%p) has residual remoteRefcount %d",
                    buffer, remoteRefcount);
        }
        // gracefully delete.
        buffer->setObserver(nullptr);
        buffer->release();
    }
    delete mInternal;
    delete mWrapper;
}

void MediaBufferGroup::add_buffer(MediaBufferBase *buffer) {
    Mutex::Autolock autoLock(mInternal->mLock);

    // if we're above our growth limit, release buffers if we can
    for (auto it = mInternal->mBuffers.begin();
            mInternal->mGrowthLimit > 0
            && mInternal->mBuffers.size() >= mInternal->mGrowthLimit
            && it != mInternal->mBuffers.end();) {
        if ((*it)->refcount() == 0) {
            (*it)->setObserver(nullptr);
            (*it)->release();
            it = mInternal->mBuffers.erase(it);
        } else {
            ++it;
        }
    }

    buffer->setObserver(this);
    mInternal->mBuffers.emplace_back(buffer);
}

bool MediaBufferGroup::has_buffers() {
    if (mInternal->mBuffers.size() < mInternal->mGrowthLimit) {
        return true; // We can add more buffers internally.
    }
    for (MediaBufferBase *buffer : mInternal->mBuffers) {
        if (buffer->refcount() == 0) {
            return true;
        }
    }
    return false;
}

status_t MediaBufferGroup::acquire_buffer(
        MediaBufferBase **out, bool nonBlocking, size_t requestedSize) {
    Mutex::Autolock autoLock(mInternal->mLock);
    for (;;) {
        size_t smallest = requestedSize;
        size_t biggest = requestedSize;
        MediaBufferBase *buffer = nullptr;
        auto free = mInternal->mBuffers.end();
        for (auto it = mInternal->mBuffers.begin(); it != mInternal->mBuffers.end(); ++it) {
            const size_t size = (*it)->size();
            if (size > biggest) {
                biggest = size;
            }
            if ((*it)->refcount() == 0) {
                if (size >= requestedSize) {
                    buffer = *it;
                    break;
                }
                if (size < smallest) {
                    smallest = size; // always free the smallest buf
                    free = it;
                }
            }
        }
        if (buffer == nullptr
                && (free != mInternal->mBuffers.end()
                    || mInternal->mBuffers.size() < mInternal->mGrowthLimit)) {
            // We alloc before we free so failure leaves group unchanged.
            const size_t allocateSize = requestedSize == 0 ? biggest :
                    requestedSize < SIZE_MAX / 3 * 2 /* NB: ordering */ ?
                    requestedSize * 3 / 2 : requestedSize;
            buffer = new MediaBuffer(allocateSize);
            if (buffer->data() == nullptr) {
                ALOGE("Allocation failure for size %zu", allocateSize);
                delete buffer; // Invalid alloc, prefer not to call release.
                buffer = nullptr;
            } else {
                buffer->setObserver(this);
                if (free != mInternal->mBuffers.end()) {
                    ALOGV("reallocate buffer, requested size %zu vs available %zu",
                            requestedSize, (*free)->size());
                    (*free)->setObserver(nullptr);
                    (*free)->release();
                    *free = buffer; // in-place replace
                } else {
                    ALOGV("allocate buffer, requested size %zu", requestedSize);
                    mInternal->mBuffers.emplace_back(buffer);
                }
            }
        }
        if (buffer != nullptr) {
            buffer->add_ref();
            buffer->reset();
            *out = buffer;
            return OK;
        }
        if (nonBlocking) {
            *out = nullptr;
            return WOULD_BLOCK;
        }
        // All buffers are in use, block until one of them is returned.
        mInternal->mCondition.wait(mInternal->mLock);
    }
    // Never gets here.
}

size_t MediaBufferGroup::buffers() const {
    return mInternal->mBuffers.size();
}

void MediaBufferGroup::signalBufferReturned(MediaBufferBase *) {
    Mutex::Autolock autoLock(mInternal->mLock);
    mInternal->mCondition.signal();
}

}  // namespace android
