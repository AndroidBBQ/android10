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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <random>

#include <gtest/gtest.h>

#include "BlobCache.h"

namespace android {

template<typename T> using sp = std::shared_ptr<T>;

class BlobCacheTest : public ::testing::TestWithParam<BlobCache::Policy> {
protected:

    enum {
        OK = 0,
        BAD_VALUE = -EINVAL
    };

    enum {
        MAX_KEY_SIZE = 6,
        MAX_VALUE_SIZE = 8,
        MAX_TOTAL_SIZE = 13,
    };

    virtual void SetUp() {
        mBC.reset(new BlobCache(MAX_KEY_SIZE, MAX_VALUE_SIZE, MAX_TOTAL_SIZE, GetParam()));
    }

    virtual void TearDown() {
        mBC.reset();
    }

    std::unique_ptr<BlobCache> mBC;
};

INSTANTIATE_TEST_CASE_P(Policy, BlobCacheTest,
    ::testing::Values(BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::HALVE),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::HALVE),

                      BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::FIT),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::FIT),

                      BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::FIT_HALVE),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::FIT_HALVE)));

TEST_P(BlobCacheTest, CacheSingleValueSucceeds) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, buf, 4));
    ASSERT_EQ('e', buf[0]);
    ASSERT_EQ('f', buf[1]);
    ASSERT_EQ('g', buf[2]);
    ASSERT_EQ('h', buf[3]);
}

TEST_P(BlobCacheTest, CacheTwoValuesSucceeds) {
    unsigned char buf[2] = { 0xee, 0xee };
    mBC->set("ab", 2, "cd", 2);
    mBC->set("ef", 2, "gh", 2);
    ASSERT_EQ(size_t(2), mBC->get("ab", 2, buf, 2));
    ASSERT_EQ('c', buf[0]);
    ASSERT_EQ('d', buf[1]);
    ASSERT_EQ(size_t(2), mBC->get("ef", 2, buf, 2));
    ASSERT_EQ('g', buf[0]);
    ASSERT_EQ('h', buf[1]);
}

TEST_P(BlobCacheTest, CacheTwoValuesMallocSucceeds) {
    unsigned char *bufPtr;
    mBC->set("ab", 2, "cd", 2);
    mBC->set("ef", 2, "gh", 2);

    bufPtr = nullptr;
    ASSERT_EQ(size_t(2), mBC->get("ab", 2, &bufPtr, malloc));
    ASSERT_NE(nullptr, bufPtr);
    ASSERT_EQ('c', bufPtr[0]);
    ASSERT_EQ('d', bufPtr[1]);
    free(bufPtr);

    bufPtr = nullptr;
    ASSERT_EQ(size_t(2), mBC->get("ef", 2, &bufPtr, malloc));
    ASSERT_NE(nullptr, bufPtr);
    ASSERT_EQ('g', bufPtr[0]);
    ASSERT_EQ('h', bufPtr[1]);
    free(bufPtr);
}

TEST_P(BlobCacheTest, GetOnlyWritesInsideBounds) {
    unsigned char buf[6] = { 0xee, 0xee, 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, buf+1, 4));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ('e', buf[1]);
    ASSERT_EQ('f', buf[2]);
    ASSERT_EQ('g', buf[3]);
    ASSERT_EQ('h', buf[4]);
    ASSERT_EQ(0xee, buf[5]);
}

TEST_P(BlobCacheTest, GetOnlyWritesIfBufferIsLargeEnough) {
    unsigned char buf[3] = { 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, buf, 3));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
}

TEST_P(BlobCacheTest, GetWithFailedAllocator) {
    unsigned char buf[3] = { 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);

    // If allocator fails, verify that we set the value pointer to
    // nullptr, and that we do not modify the buffer that the value
    // pointer originally pointed to.
    unsigned char *bufPtr = &buf[0];
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, &bufPtr, [](size_t) -> void* { return nullptr; }));
    ASSERT_EQ(nullptr, bufPtr);
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
}

TEST_P(BlobCacheTest, GetDoesntAccessNullBuffer) {
    mBC->set("abcd", 4, "efgh", 4);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, NULL, 0));
}

TEST_P(BlobCacheTest, MultipleSetsCacheLatestValue) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    mBC->set("abcd", 4, "ijkl", 4);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, buf, 4));
    ASSERT_EQ('i', buf[0]);
    ASSERT_EQ('j', buf[1]);
    ASSERT_EQ('k', buf[2]);
    ASSERT_EQ('l', buf[3]);
}

TEST_P(BlobCacheTest, SecondSetKeepsFirstValueIfTooLarge) {
    unsigned char buf[MAX_VALUE_SIZE+1] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    mBC->set("abcd", 4, buf, MAX_VALUE_SIZE+1);
    ASSERT_EQ(size_t(4), mBC->get("abcd", 4, buf, 4));
    ASSERT_EQ('e', buf[0]);
    ASSERT_EQ('f', buf[1]);
    ASSERT_EQ('g', buf[2]);
    ASSERT_EQ('h', buf[3]);
}

TEST_P(BlobCacheTest, DoesntCacheIfKeyIsTooBig) {
    char key[MAX_KEY_SIZE+1];
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    for (int i = 0; i < MAX_KEY_SIZE+1; i++) {
        key[i] = 'a';
    }
    mBC->set(key, MAX_KEY_SIZE+1, "bbbb", 4);

    ASSERT_EQ(size_t(0), mBC->get(key, MAX_KEY_SIZE+1, buf, 4));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);

    // If key is too large, verify that we do not call the allocator,
    // that we set the value pointer to nullptr, and that we do not
    // modify the buffer that the value pointer originally pointed to.
    unsigned char *bufPtr = &buf[0];
    bool calledAlloc = false;
    ASSERT_EQ(size_t(0), mBC->get(key, MAX_KEY_SIZE+1, &bufPtr,
                                  [&calledAlloc](size_t) -> void* {
                                      calledAlloc = true;
                                      return nullptr; }));
    ASSERT_EQ(false, calledAlloc);
    ASSERT_EQ(nullptr, bufPtr);
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);
}

TEST_P(BlobCacheTest, DoesntCacheIfValueIsTooBig) {
    unsigned char buf[MAX_VALUE_SIZE+1];
    for (int i = 0; i < MAX_VALUE_SIZE+1; i++) {
        buf[i] = 'b';
    }
    mBC->set("abcd", 4, buf, MAX_VALUE_SIZE+1);
    for (int i = 0; i < MAX_VALUE_SIZE+1; i++) {
        buf[i] = 0xee;
    }
    ASSERT_EQ(size_t(0), mBC->get("abcd", 4, buf, MAX_VALUE_SIZE+1));
    for (int i = 0; i < MAX_VALUE_SIZE+1; i++) {
        SCOPED_TRACE(i);
        ASSERT_EQ(0xee, buf[i]);
    }
}

TEST_P(BlobCacheTest, DoesntCacheIfKeyValuePairIsTooBig) {
    // Check a testing assumptions
    ASSERT_TRUE(MAX_TOTAL_SIZE < MAX_KEY_SIZE + MAX_VALUE_SIZE);
    ASSERT_TRUE(MAX_KEY_SIZE < MAX_TOTAL_SIZE);

    enum { bufSize = MAX_TOTAL_SIZE - MAX_KEY_SIZE + 1 };

    char key[MAX_KEY_SIZE];
    char buf[bufSize];
    for (int i = 0; i < MAX_KEY_SIZE; i++) {
        key[i] = 'a';
    }
    for (int i = 0; i < bufSize; i++) {
        buf[i] = 'b';
    }

    mBC->set(key, MAX_KEY_SIZE, buf, MAX_VALUE_SIZE);
    ASSERT_EQ(size_t(0), mBC->get(key, MAX_KEY_SIZE, NULL, 0));
}

TEST_P(BlobCacheTest, CacheMaxKeySizeSucceeds) {
    char key[MAX_KEY_SIZE];
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    for (int i = 0; i < MAX_KEY_SIZE; i++) {
        key[i] = 'a';
    }
    mBC->set(key, MAX_KEY_SIZE, "wxyz", 4);
    ASSERT_EQ(size_t(4), mBC->get(key, MAX_KEY_SIZE, buf, 4));
    ASSERT_EQ('w', buf[0]);
    ASSERT_EQ('x', buf[1]);
    ASSERT_EQ('y', buf[2]);
    ASSERT_EQ('z', buf[3]);
}

TEST_P(BlobCacheTest, CacheMaxValueSizeSucceeds) {
    char buf[MAX_VALUE_SIZE];
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        buf[i] = 'b';
    }
    mBC->set("abcd", 4, buf, MAX_VALUE_SIZE);
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        buf[i] = 0xee;
    }
    ASSERT_EQ(size_t(MAX_VALUE_SIZE), mBC->get("abcd", 4, buf,
            MAX_VALUE_SIZE));
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        SCOPED_TRACE(i);
        ASSERT_EQ('b', buf[i]);
    }
}

TEST_P(BlobCacheTest, CacheMaxKeyValuePairSizeSucceeds) {
    // Check a testing assumption
    ASSERT_TRUE(MAX_KEY_SIZE < MAX_TOTAL_SIZE);

    enum { bufSize = MAX_TOTAL_SIZE - MAX_KEY_SIZE };

    char key[MAX_KEY_SIZE];
    char buf[bufSize];
    for (int i = 0; i < MAX_KEY_SIZE; i++) {
        key[i] = 'a';
    }
    for (int i = 0; i < bufSize; i++) {
        buf[i] = 'b';
    }

    mBC->set(key, MAX_KEY_SIZE, buf, bufSize);
    ASSERT_EQ(size_t(bufSize), mBC->get(key, MAX_KEY_SIZE, NULL, 0));
}

TEST_P(BlobCacheTest, CacheMinKeyAndValueSizeSucceeds) {
    unsigned char buf[1] = { 0xee };
    mBC->set("x", 1, "y", 1);
    ASSERT_EQ(size_t(1), mBC->get("x", 1, buf, 1));
    ASSERT_EQ('y', buf[0]);
}

TEST_P(BlobCacheTest, CacheSizeDoesntExceedTotalLimit) {
    for (int i = 0; i < 256; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, "x", 1);
    }
    int numCached = 0;
    for (int i = 0; i < 256; i++) {
        uint8_t k = i;
        if (mBC->get(&k, 1, NULL, 0) == 1) {
            numCached++;
        }
    }
    ASSERT_GE(MAX_TOTAL_SIZE / 2, numCached);
}

TEST_P(BlobCacheTest, ExceedingTotalLimitHalvesCacheSize) {
    if (GetParam().second == BlobCache::Capacity::FIT)
        return;  // test doesn't apply for this policy

    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, "x", 1);
    }
    // Insert one more entry, causing a cache overflow.
    {
        uint8_t k = maxEntries;
        mBC->set(&k, 1, "x", 1);
    }
    // Count the number of entries in the cache; and check which
    // entries they are.
    int numCached = 0;
    for (int i = 0; i < maxEntries+1; i++) {
        uint8_t k = i;
        bool found = (mBC->get(&k, 1, NULL, 0) == 1);
        if (found)
            numCached++;
        if (GetParam().first == BlobCache::Select::LRU) {
            SCOPED_TRACE(i);
            ASSERT_EQ(found, i >= maxEntries/2);
        }
    }
    ASSERT_EQ(maxEntries/2 + 1, numCached);
}

TEST_P(BlobCacheTest, ExceedingTotalLimitJustFitsSmallEntry) {
    if (GetParam().second != BlobCache::Capacity::FIT)
        return;  // test doesn't apply for this policy

    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, "x", 1);
    }
    // Insert one more entry, causing a cache overflow.
    {
        uint8_t k = maxEntries;
        mBC->set(&k, 1, "x", 1);
    }
    // Count the number of entries in the cache.
    int numCached = 0;
    for (int i = 0; i < maxEntries+1; i++) {
        uint8_t k = i;
        if (mBC->get(&k, 1, NULL, 0) == 1)
            numCached++;
    }
    ASSERT_EQ(maxEntries, numCached);
}

// Also see corresponding test in nnCache_test.cpp
TEST_P(BlobCacheTest, ExceedingTotalLimitFitsBigEntry) {
    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, "x", 1);
    }
    // Insert one more entry, causing a cache overflow.
    const int bigValueSize = std::min((MAX_TOTAL_SIZE * 3) / 4 - 1, int(MAX_VALUE_SIZE));
    ASSERT_GT(bigValueSize+1, MAX_TOTAL_SIZE / 2);  // Check testing assumption
    {
        unsigned char buf[MAX_VALUE_SIZE];
        for (int i = 0; i < bigValueSize; i++)
            buf[i] = 0xee;
        uint8_t k = maxEntries;
        mBC->set(&k, 1, buf, bigValueSize);
    }
    // Count the number and size of entries in the cache.
    int numCached = 0;
    size_t sizeCached = 0;
    for (int i = 0; i < maxEntries+1; i++) {
        uint8_t k = i;
        size_t size = mBC->get(&k, 1, NULL, 0);
        if (size) {
            numCached++;
            sizeCached += (size + 1);
        }
    }
    switch (GetParam().second) {
        case BlobCache::Capacity::HALVE:
            // New value is too big for this cleaning algorithm.  So
            // we cleaned the cache, but did not insert the new value.
            ASSERT_EQ(maxEntries/2, numCached);
            ASSERT_EQ(size_t((maxEntries/2)*2), sizeCached);
            break;
        case BlobCache::Capacity::FIT:
        case BlobCache::Capacity::FIT_HALVE: {
            // We had to clean more than half the cache to fit the new
            // value.
            const int initialNumEntries = maxEntries;
            const int initialSizeCached = initialNumEntries * 2;
            const int initialFreeSpace = MAX_TOTAL_SIZE - initialSizeCached;

            // (bigValueSize + 1) = value size + key size
            // trailing "+ 1" is in order to round up
            // "/ 2" is because initial entries are size 2 (1 byte key, 1 byte value)
            const int cleanNumEntries = ((bigValueSize + 1) - initialFreeSpace + 1) / 2;

            const int cleanSpace = cleanNumEntries * 2;
            const int postCleanNumEntries = initialNumEntries - cleanNumEntries;
            const int postCleanSizeCached = initialSizeCached - cleanSpace;
            ASSERT_EQ(postCleanNumEntries + 1, numCached);
            ASSERT_EQ(size_t(postCleanSizeCached + bigValueSize + 1), sizeCached);

            break;
        }
        default:
            FAIL() << "Unknown Capacity value";
    }
}

TEST_P(BlobCacheTest, FailedGetWithAllocator) {
    // If get doesn't find anything, verify that we do not call the
    // allocator, that we set the value pointer to nullptr, and that
    // we do not modify the buffer that the value pointer originally
    // pointed to.
    unsigned char buf[1] = { 0xee };
    unsigned char *bufPtr = &buf[0];
    bool calledAlloc = false;
    ASSERT_EQ(size_t(0), mBC->get("a", 1, &bufPtr,
                                  [&calledAlloc](size_t) -> void* {
                                      calledAlloc = true;
                                      return nullptr; }));
    ASSERT_EQ(false, calledAlloc);
    ASSERT_EQ(nullptr, bufPtr);
    ASSERT_EQ(0xee, buf[0]);
}

TEST_P(BlobCacheTest, ExceedingTotalLimitRemovesLRUEntries) {
    if (GetParam().first != BlobCache::Select::LRU)
        return;  // test doesn't apply for this policy

    // Fill up the entire cache with 1 char key/value pairs.
    static const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, "x", 1);
    }

    // Access entries in some known pseudorandom order.
    int accessSequence[maxEntries];
    std::iota(&accessSequence[0], &accessSequence[maxEntries], 0);
    std::mt19937 randomEngine(MAX_TOTAL_SIZE /* seed */);
    std::shuffle(&accessSequence[0], &accessSequence[maxEntries], randomEngine);
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = accessSequence[i];
        uint8_t buf[1];
        // If we were to pass NULL to get() as the value pointer, this
        // won't count as an access for LRU purposes.
        mBC->get(&k, 1, buf, 1);
    }

    // Insert one more entry, causing a cache overflow.
    {
        uint8_t k = maxEntries;
        mBC->set(&k, 1, "x", 1);
    }

    // Check which entries are in the cache.  We expect to see the
    // "one more entry" we just added, and also the most-recently
    // accessed (according to accessSequence).  That is, we should
    // find exactly the entries with the following keys:
    // . maxEntries
    // . accessSequence[j..maxEntries-1] for some 0 <= j < maxEntries
    uint8_t k = maxEntries;
    ASSERT_EQ(size_t(1), mBC->get(&k, 1, NULL, 0));
    bool foundAny = false;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = accessSequence[i];
        bool found = (mBC->get(&k, 1, NULL, 0) == 1);
        if (foundAny == found)
            continue;
        if (!foundAny) {
            // found == true, so we just discovered j == i
            foundAny = true;
        } else {
            // foundAny == true, found == false -- oops
            FAIL() << "found [" << i-1 << "]th entry but not [" << i << "]th entry";
        }
    }
}

class BlobCacheFlattenTest : public BlobCacheTest {
protected:
    virtual void SetUp() {
        BlobCacheTest::SetUp();
        mBC2.reset(new BlobCache(MAX_KEY_SIZE, MAX_VALUE_SIZE, MAX_TOTAL_SIZE, GetParam()));
    }

    virtual void TearDown() {
        mBC2.reset();
        BlobCacheTest::TearDown();
    }

    void roundTrip() {
        size_t size = mBC->getFlattenedSize();
        uint8_t* flat = new uint8_t[size];
        ASSERT_EQ(OK, mBC->flatten(flat, size));
        ASSERT_EQ(OK, mBC2->unflatten(flat, size));
        delete[] flat;
    }

    sp<BlobCache> mBC2;
};

INSTANTIATE_TEST_CASE_P(Policy, BlobCacheFlattenTest,
    ::testing::Values(BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::HALVE),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::HALVE),

                      BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::FIT),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::FIT),

                      BlobCache::Policy(BlobCache::Select::RANDOM, BlobCache::Capacity::FIT_HALVE),
                      BlobCache::Policy(BlobCache::Select::LRU, BlobCache::Capacity::FIT_HALVE)));

TEST_P(BlobCacheFlattenTest, FlattenOneValue) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);
    roundTrip();
    ASSERT_EQ(size_t(4), mBC2->get("abcd", 4, buf, 4));
    ASSERT_EQ('e', buf[0]);
    ASSERT_EQ('f', buf[1]);
    ASSERT_EQ('g', buf[2]);
    ASSERT_EQ('h', buf[3]);
}

TEST_P(BlobCacheFlattenTest, FlattenFullCache) {
    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, &k, 1);
    }

    roundTrip();

    // Verify the deserialized cache
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        uint8_t v = 0xee;
        ASSERT_EQ(size_t(1), mBC2->get(&k, 1, &v, 1));
        ASSERT_EQ(k, v);
    }
}

TEST_P(BlobCacheFlattenTest, FlattenDoesntChangeCache) {
    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, &k, 1);
    }

    size_t size = mBC->getFlattenedSize();
    uint8_t* flat = new uint8_t[size];
    ASSERT_EQ(OK, mBC->flatten(flat, size));
    delete[] flat;

    // Verify the cache that we just serialized
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        uint8_t v = 0xee;
        ASSERT_EQ(size_t(1), mBC->get(&k, 1, &v, 1));
        ASSERT_EQ(k, v);
    }
}

TEST_P(BlobCacheFlattenTest, FlattenCatchesBufferTooSmall) {
    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mBC->set(&k, 1, &k, 1);
    }

    size_t size = mBC->getFlattenedSize() - 1;
    uint8_t* flat = new uint8_t[size];
    // ASSERT_EQ(BAD_VALUE, mBC->flatten(flat, size));
    // TODO: The above fails. I expect this is so because getFlattenedSize()
    // overstimates the size by using PROPERTY_VALUE_MAX.
    delete[] flat;
}

TEST_P(BlobCacheFlattenTest, UnflattenCatchesBadMagic) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);

    size_t size = mBC->getFlattenedSize();
    uint8_t* flat = new uint8_t[size];
    ASSERT_EQ(OK, mBC->flatten(flat, size));
    flat[1] = ~flat[1];

    // Bad magic should cause an error.
    ASSERT_EQ(BAD_VALUE, mBC2->unflatten(flat, size));
    delete[] flat;

    // The error should cause the unflatten to result in an empty cache
    ASSERT_EQ(size_t(0), mBC2->get("abcd", 4, buf, 4));
}

TEST_P(BlobCacheFlattenTest, UnflattenCatchesBadBlobCacheVersion) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);

    size_t size = mBC->getFlattenedSize();
    uint8_t* flat = new uint8_t[size];
    ASSERT_EQ(OK, mBC->flatten(flat, size));
    flat[5] = ~flat[5];

    // Version mismatches shouldn't cause errors, but should not use the
    // serialized entries
    ASSERT_EQ(OK, mBC2->unflatten(flat, size));
    delete[] flat;

    // The version mismatch should cause the unflatten to result in an empty
    // cache
    ASSERT_EQ(size_t(0), mBC2->get("abcd", 4, buf, 4));
}

TEST_P(BlobCacheFlattenTest, UnflattenCatchesBadBlobCacheDeviceVersion) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);

    size_t size = mBC->getFlattenedSize();
    uint8_t* flat = new uint8_t[size];
    ASSERT_EQ(OK, mBC->flatten(flat, size));
    flat[10] = ~flat[10];

    // Version mismatches shouldn't cause errors, but should not use the
    // serialized entries
    ASSERT_EQ(OK, mBC2->unflatten(flat, size));
    delete[] flat;

    // The version mismatch should cause the unflatten to result in an empty
    // cache
    ASSERT_EQ(size_t(0), mBC2->get("abcd", 4, buf, 4));
}

TEST_P(BlobCacheFlattenTest, UnflattenCatchesBufferTooSmall) {
    unsigned char buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mBC->set("abcd", 4, "efgh", 4);

    size_t size = mBC->getFlattenedSize();
    uint8_t* flat = new uint8_t[size];
    ASSERT_EQ(OK, mBC->flatten(flat, size));

    // A buffer truncation shouldt cause an error
    // ASSERT_EQ(BAD_VALUE, mBC2->unflatten(flat, size-1));
    // TODO: The above appears to fail because getFlattenedSize() is
    // conservative.
    delete[] flat;

    // The error should cause the unflatten to result in an empty cache
    ASSERT_EQ(size_t(0), mBC2->get("abcd", 4, buf, 4));
}

} // namespace android
