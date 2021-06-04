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

//#define LOG_NDEBUG 0

#include "BlobCache.h"

#include <errno.h>
#include <inttypes.h>

#if defined(__ANDROID__)
#include <cutils/properties.h>
#else
#include <string.h>
#include <algorithm>
static const char property_value[] = "[HOST]";
#define PROPERTY_VALUE_MAX (sizeof(property_value) - 1)
static int property_get(const char *key, char *value, const char *default_value) {
    if (!strcmp(key, "ro.build.id")) {
        memcpy(value, property_value, PROPERTY_VALUE_MAX);
        return PROPERTY_VALUE_MAX;
    }
    if (default_value) {
        const size_t len = std::max(strlen(default_value) + 1, size_t(PROPERTY_VALUE_MAX));
        memcpy(value, default_value, len);
    }
    return 0;
}
#endif

#include <log/log.h>

#include <algorithm>
#include <chrono>

namespace android {

// BlobCache::Header::mMagicNumber value
static const uint32_t blobCacheMagic = ('_' << 24) + ('B' << 16) + ('b' << 8) + '$';

// BlobCache::Header::mBlobCacheVersion value
static const uint32_t blobCacheVersion = 3;

// BlobCache::Header::mDeviceVersion value
static const uint32_t blobCacheDeviceVersion = 1;

BlobCache::BlobCache(size_t maxKeySize, size_t maxValueSize, size_t maxTotalSize, Policy policy):
        mMaxKeySize(maxKeySize),
        mMaxValueSize(maxValueSize),
        mMaxTotalSize(maxTotalSize),
        mPolicySelect(policy.first),
        mPolicyCapacity(policy.second),
        mTotalSize(0),
        mAccessCount(0) {
    int64_t now = std::chrono::steady_clock::now().time_since_epoch().count();
#ifdef _WIN32
    srand(now);
#else
    mRandState[0] = (now >> 0) & 0xFFFF;
    mRandState[1] = (now >> 16) & 0xFFFF;
    mRandState[2] = (now >> 32) & 0xFFFF;
#endif
    ALOGV("initializing random seed using %lld", (unsigned long long)now);
}

void BlobCache::set(const void* key, size_t keySize, const void* value,
        size_t valueSize) {
    if (mMaxKeySize < keySize) {
        ALOGV("set: not caching because the key is too large: %zu (limit: %zu)",
                keySize, mMaxKeySize);
        return;
    }
    if (mMaxValueSize < valueSize) {
        ALOGV("set: not caching because the value is too large: %zu (limit: %zu)",
                valueSize, mMaxValueSize);
        return;
    }
    if (mMaxTotalSize < keySize + valueSize) {
        ALOGV("set: not caching because the combined key/value size is too "
                "large: %zu (limit: %zu)", keySize + valueSize, mMaxTotalSize);
        return;
    }
    if (keySize == 0) {
        ALOGW("set: not caching because keySize is 0");
        return;
    }
    if (valueSize <= 0) {
        ALOGW("set: not caching because valueSize is 0");
        return;
    }

    std::shared_ptr<Blob> dummyKey(new Blob(key, keySize, false));
    CacheEntry dummyEntry(dummyKey, NULL, 0);

    while (true) {
        auto index = std::lower_bound(mCacheEntries.begin(), mCacheEntries.end(), dummyEntry);
        if (index == mCacheEntries.end() || dummyEntry < *index) {
            // Create a new cache entry.
            std::shared_ptr<Blob> keyBlob(new Blob(key, keySize, true));
            std::shared_ptr<Blob> valueBlob(new Blob(value, valueSize, true));
            size_t newEntrySize = keySize + valueSize;
            size_t newTotalSize = mTotalSize + newEntrySize;
            if (mMaxTotalSize < newTotalSize) {
                if (isCleanable()) {
                    // Clean the cache and try again.
                    if (!clean(newEntrySize, NoEntry)) {
                        // We have some kind of logic error -- perhaps
                        // an inconsistency between isCleanable() and
                        // findDownTo().
                        ALOGE("set: not caching new key/value pair because "
                              "cleaning failed");
                        break;
                    }
                    continue;
                } else {
                    ALOGV("set: not caching new key/value pair because the "
                            "total cache size limit would be exceeded: %zu "
                            "(limit: %zu)",
                            keySize + valueSize, mMaxTotalSize);
                    break;
                }
            }
            mCacheEntries.insert(index, CacheEntry(keyBlob, valueBlob, ++mAccessCount));
            mTotalSize = newTotalSize;
            ALOGV("set: created new cache entry with %zu byte key and %zu byte value",
                    keySize, valueSize);
        } else {
            // Update the existing cache entry.
            std::shared_ptr<Blob> valueBlob(new Blob(value, valueSize, true));
            std::shared_ptr<Blob> oldValueBlob(index->getValue());
            size_t newTotalSize = mTotalSize + valueSize - oldValueBlob->getSize();
            if (mMaxTotalSize < newTotalSize) {
                if (isCleanable()) {
                    // Clean the cache and try again.
                    if (!clean(index->getKey()->getSize() + valueSize,
                               index - mCacheEntries.begin())) {
                        // We have some kind of logic error -- perhaps
                        // an inconsistency between isCleanable() and
                        // findDownTo().
                        ALOGE("set: not caching new value because "
                              "cleaning failed");
                        break;
                    }
                    continue;
                } else {
                    ALOGV("set: not caching new value because the total cache "
                            "size limit would be exceeded: %zu (limit: %zu)",
                            keySize + valueSize, mMaxTotalSize);
                    break;
                }
            }
            index->setValue(valueBlob);
            index->setRecency(++mAccessCount);
            mTotalSize = newTotalSize;
            ALOGV("set: updated existing cache entry with %zu byte key and %zu byte "
                    "value", keySize, valueSize);
        }
        break;
    }
}

size_t BlobCache::get(const void* key, size_t keySize, void* value,
        size_t valueSize) {
    void *dummy;
    return get(key, keySize, &dummy,
               [value, valueSize](size_t allocSize) {
                   return (allocSize <= valueSize ? value : nullptr);
               });
}

size_t BlobCache::get(const void* key, size_t keySize, void** value,
        std::function<void*(size_t)> alloc) {
    if (mMaxKeySize < keySize) {
        ALOGV("get: not searching because the key is too large: %zu (limit %zu)",
                keySize, mMaxKeySize);
        *value = nullptr;
        return 0;
    }
    std::shared_ptr<Blob> dummyKey(new Blob(key, keySize, false));
    CacheEntry dummyEntry(dummyKey, NULL, 0);
    auto index = std::lower_bound(mCacheEntries.begin(), mCacheEntries.end(), dummyEntry);
    if (index == mCacheEntries.end() || dummyEntry < *index) {
        ALOGV("get: no cache entry found for key of size %zu", keySize);
        *value = nullptr;
        return 0;
    }

    // The key was found. Return the value if we can allocate a buffer.
    std::shared_ptr<Blob> valueBlob(index->getValue());
    size_t valueBlobSize = valueBlob->getSize();
    void *buf = alloc(valueBlobSize);
    if (buf != nullptr) {
        ALOGV("get: copying %zu bytes to caller's buffer", valueBlobSize);
        memcpy(buf, valueBlob->getData(), valueBlobSize);
        *value = buf;
        index->setRecency(++mAccessCount);
    } else {
        ALOGV("get: cannot allocate caller's buffer: needs %zu", valueBlobSize);
        *value = nullptr;
    }
    return valueBlobSize;
}

static inline size_t align4(size_t size) {
    return (size + 3) & ~3;
}

size_t BlobCache::getFlattenedSize() const {
    size_t size = align4(sizeof(Header) + PROPERTY_VALUE_MAX);
    for (const CacheEntry& e :  mCacheEntries) {
        std::shared_ptr<Blob> const& keyBlob = e.getKey();
        std::shared_ptr<Blob> const& valueBlob = e.getValue();
        size += align4(sizeof(EntryHeader) + keyBlob->getSize() + valueBlob->getSize());
    }
    return size;
}

int BlobCache::flatten(void* buffer, size_t size) const {
    // Write the cache header
    if (size < sizeof(Header)) {
        ALOGE("flatten: not enough room for cache header");
        return 0;
    }
    Header* header = reinterpret_cast<Header*>(buffer);
    header->mMagicNumber = blobCacheMagic;
    header->mBlobCacheVersion = blobCacheVersion;
    header->mDeviceVersion = blobCacheDeviceVersion;
    header->mNumEntries = mCacheEntries.size();
    char buildId[PROPERTY_VALUE_MAX];
    header->mBuildIdLength = property_get("ro.build.id", buildId, "");
    memcpy(header->mBuildId, buildId, header->mBuildIdLength);

    // Write cache entries
    uint8_t* byteBuffer = reinterpret_cast<uint8_t*>(buffer);
    off_t byteOffset = align4(sizeof(Header) + header->mBuildIdLength);
    for (const CacheEntry& e :  mCacheEntries) {
        std::shared_ptr<Blob> const& keyBlob = e.getKey();
        std::shared_ptr<Blob> const& valueBlob = e.getValue();
        size_t keySize = keyBlob->getSize();
        size_t valueSize = valueBlob->getSize();

        size_t entrySize = sizeof(EntryHeader) + keySize + valueSize;
        size_t totalSize = align4(entrySize);
        if (byteOffset + totalSize > size) {
            ALOGE("flatten: not enough room for cache entries");
            return -EINVAL;
        }

        EntryHeader* eheader = reinterpret_cast<EntryHeader*>(&byteBuffer[byteOffset]);
        eheader->mKeySize = keySize;
        eheader->mValueSize = valueSize;

        memcpy(eheader->mData, keyBlob->getData(), keySize);
        memcpy(eheader->mData + keySize, valueBlob->getData(), valueSize);

        if (totalSize > entrySize) {
            // We have padding bytes. Those will get written to storage, and contribute to the CRC,
            // so make sure we zero-them to have reproducible results.
            memset(eheader->mData + keySize + valueSize, 0, totalSize - entrySize);
        }

        byteOffset += totalSize;
    }

    return 0;
}

int BlobCache::unflatten(void const* buffer, size_t size) {
    // All errors should result in the BlobCache being in an empty state.
    mCacheEntries.clear();

    // Read the cache header
    if (size < sizeof(Header)) {
        ALOGE("unflatten: not enough room for cache header");
        return -EINVAL;
    }
    const Header* header = reinterpret_cast<const Header*>(buffer);
    if (header->mMagicNumber != blobCacheMagic) {
        ALOGE("unflatten: bad magic number: %" PRIu32, header->mMagicNumber);
        return -EINVAL;
    }
    char buildId[PROPERTY_VALUE_MAX];
    int len = property_get("ro.build.id", buildId, "");
    if (header->mBlobCacheVersion != blobCacheVersion ||
            header->mDeviceVersion != blobCacheDeviceVersion ||
            len != header->mBuildIdLength ||
            strncmp(buildId, header->mBuildId, len)) {
        // We treat version mismatches as an empty cache.
        return 0;
    }

    // Read cache entries
    const uint8_t* byteBuffer = reinterpret_cast<const uint8_t*>(buffer);
    off_t byteOffset = align4(sizeof(Header) + header->mBuildIdLength);
    size_t numEntries = header->mNumEntries;
    for (size_t i = 0; i < numEntries; i++) {
        if (byteOffset + sizeof(EntryHeader) > size) {
            mCacheEntries.clear();
            ALOGE("unflatten: not enough room for cache entry header");
            return -EINVAL;
        }

        const EntryHeader* eheader = reinterpret_cast<const EntryHeader*>(
                &byteBuffer[byteOffset]);
        size_t keySize = eheader->mKeySize;
        size_t valueSize = eheader->mValueSize;
        size_t entrySize = sizeof(EntryHeader) + keySize + valueSize;

        size_t totalSize = align4(entrySize);
        if (byteOffset + totalSize > size) {
            mCacheEntries.clear();
            ALOGE("unflatten: not enough room for cache entry");
            return -EINVAL;
        }

        const uint8_t* data = eheader->mData;
        set(data, keySize, data + keySize, valueSize);

        byteOffset += totalSize;
    }

    return 0;
}

long int BlobCache::blob_random() {
#ifdef _WIN32
    return rand();
#else
    return nrand48(mRandState);
#endif
}

size_t BlobCache::findVictim() {
    switch (mPolicySelect) {
        case Select::RANDOM:
            return size_t(blob_random() % (mCacheEntries.size()));
        case Select::LRU:
            return std::min_element(mCacheEntries.begin(), mCacheEntries.end(),
                                    [](const CacheEntry &a, const CacheEntry &b) {
                                        return a.getRecency() < b.getRecency();
                                    }) - mCacheEntries.begin();
        default:
            ALOGE("findVictim: unknown mPolicySelect: %d", mPolicySelect);
            return 0;
    }
}

size_t BlobCache::findDownTo(size_t newEntrySize, size_t onBehalfOf) {
    auto oldEntrySize = [this, onBehalfOf]() -> size_t {
        if (onBehalfOf == NoEntry)
            return 0;
        const auto &entry = mCacheEntries[onBehalfOf];
        return entry.getKey()->getSize() + entry.getValue()->getSize();
    };
    switch (mPolicyCapacity) {
        case Capacity::HALVE:
            return mMaxTotalSize / 2;
        case Capacity::FIT:
            return mMaxTotalSize - (newEntrySize - oldEntrySize());
        case Capacity::FIT_HALVE:
            return std::min(mMaxTotalSize - (newEntrySize - oldEntrySize()), mMaxTotalSize / 2);
        default:
            ALOGE("findDownTo: unknown mPolicyCapacity: %d", mPolicyCapacity);
            return 0;
    }
}

bool BlobCache::isFit(Capacity capacity) {
    switch (capacity) {
        case Capacity::HALVE:
            return false;
        case Capacity::FIT:
        case Capacity::FIT_HALVE:
            return true;
        default:
            ALOGE("isFit: unknown capacity: %d", capacity);
            return false;
    }
}

bool BlobCache::clean(size_t newEntrySize, size_t onBehalfOf) {
    // Remove a selected cache entry until the total cache size does
    // not exceed downTo.
    const size_t downTo = findDownTo(newEntrySize, onBehalfOf);

    bool cleaned = false;
    while (mTotalSize > downTo) {
        const size_t i = findVictim();
        const CacheEntry& entry(mCacheEntries[i]);
        const size_t entrySize = entry.getKey()->getSize() + entry.getValue()->getSize();
        mTotalSize -= entrySize;
        mCacheEntries.erase(mCacheEntries.begin() + i);
        cleaned = true;
    }
    return cleaned;
}

bool BlobCache::isCleanable() const {
    switch (mPolicyCapacity) {
        case Capacity::HALVE:
            return mTotalSize > mMaxTotalSize / 2;
        default:
            ALOGE("isCleanable: unknown mPolicyCapacity: %d", mPolicyCapacity);
            [[fallthrough]];
        case Capacity::FIT:
        case Capacity::FIT_HALVE:
            return mTotalSize > 0;
    }
}

BlobCache::Blob::Blob(const void* data, size_t size, bool copyData) :
        mData(copyData ? malloc(size) : data),
        mSize(size),
        mOwnsData(copyData) {
    if (data != NULL && copyData) {
        memcpy(const_cast<void*>(mData), data, size);
    }
}

BlobCache::Blob::~Blob() {
    if (mOwnsData) {
        free(const_cast<void*>(mData));
    }
}

bool BlobCache::Blob::operator<(const Blob& rhs) const {
    if (mSize == rhs.mSize) {
        return memcmp(mData, rhs.mData, mSize) < 0;
    } else {
        return mSize < rhs.mSize;
    }
}

const void* BlobCache::Blob::getData() const {
    return mData;
}

size_t BlobCache::Blob::getSize() const {
    return mSize;
}

BlobCache::CacheEntry::CacheEntry(): mRecency(0) {
}

BlobCache::CacheEntry::CacheEntry(
        const std::shared_ptr<Blob>& key, const std::shared_ptr<Blob>& value, uint32_t recency):
        mKey(key),
        mValue(value),
        mRecency(recency) {
}

BlobCache::CacheEntry::CacheEntry(const CacheEntry& ce):
        mKey(ce.mKey),
        mValue(ce.mValue),
        mRecency(ce.mRecency) {
}

bool BlobCache::CacheEntry::operator<(const CacheEntry& rhs) const {
    return *mKey < *rhs.mKey;
}

const BlobCache::CacheEntry& BlobCache::CacheEntry::operator=(const CacheEntry& rhs) {
    mKey = rhs.mKey;
    mValue = rhs.mValue;
    mRecency = rhs.mRecency;
    return *this;
}

std::shared_ptr<BlobCache::Blob> BlobCache::CacheEntry::getKey() const {
    return mKey;
}

std::shared_ptr<BlobCache::Blob> BlobCache::CacheEntry::getValue() const {
    return mValue;
}

void BlobCache::CacheEntry::setValue(const std::shared_ptr<Blob>& value) {
    mValue = value;
}

uint32_t BlobCache::CacheEntry::getRecency() const {
    return mRecency;
}

void BlobCache::CacheEntry::setRecency(uint32_t recency) {
    mRecency = recency;
}

} // namespace android
