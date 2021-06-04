/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef REMOTE_DATA_SOURCE_H_
#define REMOTE_DATA_SOURCE_H_

#include <binder/IMemory.h>
#include <binder/MemoryDealer.h>
#include <media/DataSource.h>
#include <media/IDataSource.h>

namespace android {

// Originally in MediaExtractor.cpp
class RemoteDataSource : public BnDataSource {
public:
    static sp<IDataSource> wrap(const sp<DataSource> &source) {
        if (source.get() == nullptr) {
            return nullptr;
        }
        if (source->getIDataSource().get() != nullptr) {
            return source->getIDataSource();
        }
        return new RemoteDataSource(source);
    }

    virtual ~RemoteDataSource() {
        close();
    }
    virtual sp<IMemory> getIMemory() {
        return mMemory;
    }
    virtual ssize_t readAt(off64_t offset, size_t size) {
        ALOGV("readAt(%lld, %zu)", (long long)offset, size);
        if (size > kBufferSize) {
            size = kBufferSize;
        }
        return mSource->readAt(offset, mMemory->pointer(), size);
    }
    virtual status_t getSize(off64_t *size) {
        return mSource->getSize(size);
    }
    virtual void close() {
        // Protect strong pointer assignments. This also can be called from the binder
        // clean-up procedure which is running on a separate thread.
        Mutex::Autolock lock(mCloseLock);
        mSource = nullptr;
        mMemory = nullptr;
    }
    virtual uint32_t getFlags() {
        return mSource->flags();
    }
    virtual String8 toString()  {
        return mName;
    }
    virtual sp<DecryptHandle> DrmInitialization(const char *mime) {
        return mSource->DrmInitialization(mime);
    }

private:
    enum {
        kBufferSize = 64 * 1024,
    };

    sp<IMemory> mMemory;
    sp<DataSource> mSource;
    String8 mName;
    Mutex mCloseLock;

    explicit RemoteDataSource(const sp<DataSource> &source) {
        mSource = source;
        sp<MemoryDealer> memoryDealer = new MemoryDealer(kBufferSize, "RemoteDataSource");
        mMemory = memoryDealer->allocate(kBufferSize);
        if (mMemory.get() == nullptr) {
            ALOGE("Failed to allocate memory!");
        }
        mName = String8::format("RemoteDataSource(%s)", mSource->toString().string());
    }

    DISALLOW_EVIL_CONSTRUCTORS(RemoteDataSource);
};

}  // namespace android

#endif  // REMOTE_DATA_SOURCE_H_
