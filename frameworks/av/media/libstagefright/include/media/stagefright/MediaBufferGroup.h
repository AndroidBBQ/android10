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

#ifndef MEDIA_BUFFER_GROUP_H_

#define MEDIA_BUFFER_GROUP_H_

#include <list>

#include <media/MediaExtractorPluginApi.h>
#include <media/NdkMediaErrorPriv.h>
#include <media/stagefright/MediaBufferBase.h>
#include <utils/Errors.h>
#include <utils/threads.h>

namespace android {

class MediaBufferBase;

class MediaBufferGroup : public MediaBufferObserver {
public:
    MediaBufferGroup(size_t growthLimit = 0);

    // create a media buffer group with preallocated buffers
    MediaBufferGroup(size_t buffers, size_t buffer_size, size_t growthLimit = 0);

    ~MediaBufferGroup();

    void add_buffer(MediaBufferBase *buffer);

    bool has_buffers();

    // If nonBlocking is false, it blocks until a buffer is available and
    // passes it to the caller in *buffer, while returning OK.
    // The returned buffer will have a reference count of 1.
    // If nonBlocking is true and a buffer is not immediately available,
    // buffer is set to NULL and it returns WOULD_BLOCK.
    // If requestedSize is 0, any free MediaBuffer will be returned.
    // If requestedSize is > 0, the returned MediaBuffer should have buffer
    // size of at least requstedSize.
    status_t acquire_buffer(
            MediaBufferBase **buffer, bool nonBlocking = false, size_t requestedSize = 0);

    size_t buffers() const;

    // If buffer is nullptr, have acquire_buffer() check for remote release.
    virtual void signalBufferReturned(MediaBufferBase *buffer);

    CMediaBufferGroup *wrap() {
        if (mWrapper) {
            return mWrapper;
        }

        mWrapper = new CMediaBufferGroup;
        mWrapper->handle = this;

        mWrapper->add_buffer = [](void *handle, size_t size) -> void {
            MediaBufferBase *buf = MediaBufferBase::Create(size);
            ((MediaBufferGroup*)handle)->add_buffer(buf);
        };

        mWrapper->init = [](void *handle,
                size_t buffers, size_t buffer_size, size_t growthLimit) -> bool {
            ((MediaBufferGroup*)handle)->init(buffers, buffer_size, growthLimit);
          //  ((MediaBufferGroup*)handle)->mWrapper->init = nullptr; // enforce call-once
            return true;
        };

        mWrapper->acquire_buffer = [](void *handle,
                CMediaBuffer **buf, bool nonBlocking, size_t requestedSize) -> media_status_t {
            MediaBufferBase *acquiredBuf = nullptr;
            status_t err = ((MediaBufferGroup*)handle)->acquire_buffer(
                    &acquiredBuf, nonBlocking, requestedSize);
            if (err == OK && acquiredBuf != nullptr) {
                *buf = acquiredBuf->wrap();
            } else {
                *buf = nullptr;
            }
            return translate_error(err);
        };

        mWrapper->has_buffers = [](void *handle) -> bool {
            return ((MediaBufferGroup*)handle)->has_buffers();
        };

        return mWrapper;
    }

private:
    CMediaBufferGroup *mWrapper;
    struct InternalData;
    InternalData *mInternal;

    MediaBufferGroup(const MediaBufferGroup &);
    MediaBufferGroup &operator=(const MediaBufferGroup &);
    void init(size_t buffers, size_t buffer_size, size_t growthLimit);
};

}  // namespace android

#endif  // MEDIA_BUFFER_GROUP_H_
