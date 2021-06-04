/*
 ** Copyright 2011, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_NN_CACHE_H
#define ANDROID_NN_CACHE_H

#include "BlobCache.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

class NNCache {
public:

    typedef BlobCache::Select Select;
    typedef BlobCache::Capacity Capacity;
    typedef BlobCache::Policy Policy;

    static Policy defaultPolicy() { return BlobCache::defaultPolicy(); }

    // get returns a pointer to the singleton NNCache object.  This
    // singleton object will never be destroyed.
    static NNCache* get();

    // initialize puts the NNCache into an initialized state, such
    // that it is able to insert and retrieve entries from the cache.
    // When not in the initialized state the getBlob and setBlob
    // methods will return without performing any cache operations.
    //
    // The NNCache will cache key/value pairs with key and value sizes
    // less than or equal to maxKeySize and maxValueSize,
    // respectively. The total combined size of ALL cache entries (key
    // sizes plus value sizes) will not exceed maxTotalSize.
    void initialize(size_t maxKeySize, size_t maxValueSize, size_t maxTotalSize,
                    Policy policy = defaultPolicy());

    // terminate puts the NNCache back into the uninitialized state.  When
    // in this state the getBlob and setBlob methods will return without
    // performing any cache operations.
    void terminate();

    // setBlob attempts to insert a new key/value blob pair into the cache.
    void setBlob(const void* key, ssize_t keySize, const void* value,
        ssize_t valueSize);

    // getBlob attempts to retrieve the value blob associated with a given key
    // blob from cache.
    ssize_t getBlob(const void* key, ssize_t keySize,
                    void* value, ssize_t valueSize);
    ssize_t getBlob(const void* key, ssize_t keySize,
                    void** value,  std::function<void*(size_t)> alloc);
    template <typename T>
    ssize_t getBlob(const void* key, size_t keySize,
                    T** value, std::function<void*(size_t)> alloc) {
        void *valueVoid;
        const ssize_t size = getBlob(key, keySize, &valueVoid, alloc);
        *value = static_cast<T*>(valueVoid);
        return size;
    }

    // setCacheFilename sets the name of the file that should be used to store
    // cache contents from one program invocation to another.
    void setCacheFilename(const char* filename);

private:
    // Creation and (the lack of) destruction is handled internally.
    NNCache();
    ~NNCache();

    // Copying is disallowed.
    NNCache(const NNCache&) = delete;
    void operator=(const NNCache&) = delete;

    // getBlobCacheLocked returns the BlobCache object being used to store the
    // key/value blob pairs.  If the BlobCache object has not yet been created,
    // this will do so, loading the serialized cache contents from disk if
    // possible.
    BlobCache* getBlobCacheLocked();

    // saveBlobCache attempts to save the current contents of mBlobCache to
    // disk.
    void saveBlobCacheLocked();

    // loadBlobCache attempts to load the saved cache contents from disk into
    // mBlobCache.
    void loadBlobCacheLocked();

    // mInitialized indicates whether the NNCache is in the initialized
    // state.  It is initialized to false at construction time, and gets set to
    // true when initialize is called.  It is set back to false when terminate
    // is called.  When in this state, the cache behaves as normal.  When not,
    // the getBlob and setBlob methods will return without performing any cache
    // operations.
    bool mInitialized;

    // mMaxKeySize is the maximum key size that will be cached.
    size_t mMaxKeySize;

    // mMaxValueSize is the maximum value size that will be cached.
    size_t mMaxValueSize;

    // mMaxTotalSize is the maximum size that all cache entries can occupy. This
    // includes space for both keys and values.
    size_t mMaxTotalSize;

    // mPolicy is the policy for cleaning the cache.
    Policy mPolicy;

    // mBlobCache is the cache in which the key/value blob pairs are stored.  It
    // is initially NULL, and will be initialized by getBlobCacheLocked the
    // first time it's needed.
    std::unique_ptr<BlobCache> mBlobCache;

    // mFilename is the name of the file for storing cache contents in between
    // program invocations.  It is initialized to an empty string at
    // construction time, and can be set with the setCacheFilename method.  An
    // empty string indicates that the cache should not be saved to or restored
    // from disk.
    std::string mFilename;

    // mSavePending indicates whether or not a deferred save operation is
    // pending.  Each time a key/value pair is inserted into the cache via
    // setBlob, a deferred save is initiated if one is not already pending.
    // This will wait some amount of time and then trigger a save of the cache
    // contents to disk.
    bool mSavePending;

    // mMutex is the mutex used to prevent concurrent access to the member
    // variables. It must be locked whenever the member variables are accessed.
    mutable std::mutex mMutex;

    // sCache is the singleton NNCache object.
    static NNCache sCache;
};

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif // ANDROID_NN_CACHE_H
