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

#ifndef MEDIA_BUFFER_BASE_H_

#define MEDIA_BUFFER_BASE_H_

#include <media/MediaExtractorPluginApi.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/NdkMediaFormatPriv.h>

namespace android {

class MediaBufferBase;
class MetaDataBase;

class MediaBufferObserver {
public:
    MediaBufferObserver() {}
    virtual ~MediaBufferObserver() {}

    virtual void signalBufferReturned(MediaBufferBase *buffer) = 0;

private:
    MediaBufferObserver(const MediaBufferObserver &);
    MediaBufferObserver &operator=(const MediaBufferObserver &);
};

class MediaBufferBase {
public:
    static MediaBufferBase *Create(size_t size);

    // If MediaBufferGroup is set, decrement the local reference count;
    // if the local reference count drops to 0, return the buffer to the
    // associated MediaBufferGroup.
    //
    // If no MediaBufferGroup is set, the local reference count must be zero
    // when called, whereupon the MediaBuffer is deleted.
    virtual void release() = 0;

    // Increments the local reference count.
    // Use only when MediaBufferGroup is set.
    virtual void add_ref() = 0;

    virtual void *data() const = 0;
    virtual size_t size() const = 0;

    virtual size_t range_offset() const = 0;
    virtual size_t range_length() const = 0;

    virtual void set_range(size_t offset, size_t length) = 0;

    virtual MetaDataBase& meta_data() = 0;

    // Clears meta data and resets the range to the full extent.
    virtual void reset() = 0;

    virtual void setObserver(MediaBufferObserver *group) = 0;

    virtual int refcount() const = 0;

    virtual int localRefcount() const = 0;
    virtual int remoteRefcount() const = 0;

    virtual ~MediaBufferBase() {
        delete mWrapper;
        delete mFormat;
    };

    CMediaBuffer *wrap() {
        if (mWrapper) {
            return mWrapper;
        }
        mWrapper = new CMediaBuffer;
        mWrapper->handle = this;

        mWrapper->release = [](void *handle) -> void {
            ((MediaBufferBase*)handle)->release();
        };

        mWrapper->data = [](void *handle) -> void * {
            return ((MediaBufferBase*)handle)->data();
        };

        mWrapper->size = [](void *handle) -> size_t {
            return ((MediaBufferBase*)handle)->size();
        };

        mWrapper->range_offset = [](void *handle) -> size_t {
            return ((MediaBufferBase*)handle)->range_offset();
        };

        mWrapper->range_length = [](void *handle) -> size_t {
            return ((MediaBufferBase*)handle)->range_length();
        };

        mWrapper->set_range = [](void *handle, size_t offset, size_t length) -> void {
            return ((MediaBufferBase*)handle)->set_range(offset, length);
        };

        mWrapper->meta_data = [](void *handle) -> AMediaFormat* {
            if (((MediaBufferBase*)handle)->mFormat == nullptr) {
                sp<AMessage> msg = new AMessage();
                ((MediaBufferBase*)handle)->mFormat = AMediaFormat_fromMsg(&msg);
            }
            return ((MediaBufferBase*)handle)->mFormat;
        };

        return mWrapper;
    }
protected:
    MediaBufferBase() {
        mWrapper = nullptr;
        mFormat = nullptr;
    }
private:
    CMediaBuffer *mWrapper;
    AMediaFormat *mFormat;
};

}  // namespace android

#endif  // MEDIA_BUFFER_BASE_H_
