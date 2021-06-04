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

#ifndef ANDROID_CALLBACKDATASOURCE_H
#define ANDROID_CALLBACKDATASOURCE_H

#include <media/DataSource.h>
#include <media/stagefright/foundation/ADebug.h>

namespace android {

class IDataSource;
class IMemory;

// A stagefright DataSource that wraps a binder IDataSource. It's a "Callback"
// DataSource because it calls back to the IDataSource for data.
class CallbackDataSource : public DataSource {
public:
    explicit CallbackDataSource(const sp<IDataSource>& iDataSource);
    virtual ~CallbackDataSource();

    // DataSource implementation.
    virtual status_t initCheck() const;
    virtual ssize_t readAt(off64_t offset, void *data, size_t size);
    virtual status_t getSize(off64_t *size);
    virtual uint32_t flags();
    virtual void close();
    virtual String8 toString() {
        return mName;
    }
    virtual sp<DecryptHandle> DrmInitialization(const char *mime = NULL);
    virtual sp<IDataSource> getIDataSource() const;

private:
    sp<IDataSource> mIDataSource;
    sp<IMemory> mMemory;
    bool mIsClosed;
    String8 mName;

    DISALLOW_EVIL_CONSTRUCTORS(CallbackDataSource);
};


// A caching DataSource that wraps a CallbackDataSource. For reads smaller
// than kCacheSize it will read up to kCacheSize ahead and cache it.
// This reduces the number of binder round trips to the IDataSource and has a significant
// impact on time taken for filetype sniffing and metadata extraction.
class TinyCacheSource : public DataSource {
public:
    explicit TinyCacheSource(const sp<DataSource>& source);

    virtual status_t initCheck() const;
    virtual ssize_t readAt(off64_t offset, void* data, size_t size);
    virtual status_t getSize(off64_t* size);
    virtual uint32_t flags();
    virtual void close() { mSource->close(); }
    virtual String8 toString() {
        return mName;
    }
    virtual sp<DecryptHandle> DrmInitialization(const char *mime = NULL);
    virtual sp<IDataSource> getIDataSource() const;

private:
    // 2kb comes from experimenting with the time-to-first-frame from a MediaPlayer
    // with an in-memory MediaDataSource source on a Nexus 5. Beyond 2kb there was
    // no improvement.
    enum {
        kCacheSize = 2048,
    };

    sp<DataSource> mSource;
    uint8_t mCache[kCacheSize];
    off64_t mCachedOffset;
    size_t mCachedSize;
    String8 mName;

    DISALLOW_EVIL_CONSTRUCTORS(TinyCacheSource);
};

}; // namespace android

#endif // ANDROID_CALLBACKDATASOURCE_H
