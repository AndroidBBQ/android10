/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "nnCache_test"
//#define LOG_NDEBUG 0

#include <stdlib.h>
#include <string.h>

#include <memory>

#include <android-base/file.h>
#include <gtest/gtest.h>
#include <log/log.h>

#include "nnCache.h"

// Cache size limits.
static const size_t maxKeySize = 12 * 1024;
static const size_t maxValueSize = 64 * 1024;
static const size_t maxTotalSize = 2 * 1024 * 1024;

namespace android {

class NNCacheTest : public ::testing::TestWithParam<NNCache::Policy> {
protected:
    virtual void SetUp() {
        mCache = NNCache::get();
    }

    virtual void TearDown() {
        mCache->setCacheFilename("");
        mCache->terminate();
    }

    NNCache* mCache;
};

INSTANTIATE_TEST_CASE_P(Policy, NNCacheTest,
    ::testing::Values(NNCache::Policy(NNCache::Select::RANDOM, NNCache::Capacity::HALVE),
                      NNCache::Policy(NNCache::Select::LRU, NNCache::Capacity::HALVE),

                      NNCache::Policy(NNCache::Select::RANDOM, NNCache::Capacity::FIT),
                      NNCache::Policy(NNCache::Select::LRU, NNCache::Capacity::FIT),

                      NNCache::Policy(NNCache::Select::RANDOM, NNCache::Capacity::FIT_HALVE),
                      NNCache::Policy(NNCache::Select::LRU, NNCache::Capacity::FIT_HALVE)));

TEST_P(NNCacheTest, UninitializedCacheAlwaysMisses) {
    uint8_t buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mCache->setBlob("abcd", 4, "efgh", 4);
    ASSERT_EQ(0, mCache->getBlob("abcd", 4, buf, 4));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);
}

TEST_P(NNCacheTest, InitializedCacheAlwaysHits) {
    uint8_t buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mCache->initialize(maxKeySize, maxValueSize, maxTotalSize, GetParam());
    mCache->setBlob("abcd", 4, "efgh", 4);
    ASSERT_EQ(4, mCache->getBlob("abcd", 4, buf, 4));
    ASSERT_EQ('e', buf[0]);
    ASSERT_EQ('f', buf[1]);
    ASSERT_EQ('g', buf[2]);
    ASSERT_EQ('h', buf[3]);
}

TEST_P(NNCacheTest, TerminatedCacheAlwaysMisses) {
    uint8_t buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mCache->initialize(maxKeySize, maxValueSize, maxTotalSize, GetParam());
    mCache->setBlob("abcd", 4, "efgh", 4);

    // cache entry lost after terminate
    mCache->terminate();
    ASSERT_EQ(0, mCache->getBlob("abcd", 4, buf, 4));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);

    // cache insertion ignored after terminate
    mCache->setBlob("abcd", 4, "efgh", 4);
    ASSERT_EQ(0, mCache->getBlob("abcd", 4, buf, 4));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);
}

// Also see corresponding test in BlobCache_test.cpp.
// The purpose of this test here is to ensure that Policy
// setting makes it through from NNCache to BlobCache.
TEST_P(NNCacheTest, ExceedingTotalLimitFitsBigEntry) {
    enum {
        MAX_KEY_SIZE = 6,
        MAX_VALUE_SIZE = 8,
        MAX_TOTAL_SIZE = 13,
    };

    mCache->initialize(MAX_KEY_SIZE, MAX_VALUE_SIZE, MAX_TOTAL_SIZE, GetParam());

    // Fill up the entire cache with 1 char key/value pairs.
    const int maxEntries = MAX_TOTAL_SIZE / 2;
    for (int i = 0; i < maxEntries; i++) {
        uint8_t k = i;
        mCache->setBlob(&k, 1, "x", 1);
    }
    // Insert one more entry, causing a cache overflow.
    const int bigValueSize = std::min((MAX_TOTAL_SIZE * 3) / 4 - 1, int(MAX_VALUE_SIZE));
    ASSERT_GT(bigValueSize+1, MAX_TOTAL_SIZE / 2);  // Check testing assumption
    {
        unsigned char buf[MAX_VALUE_SIZE];
        for (int i = 0; i < bigValueSize; i++)
            buf[i] = 0xee;
        uint8_t k = maxEntries;
        mCache->setBlob(&k, 1, buf, bigValueSize);
    }
    // Count the number and size of entries in the cache.
    int numCached = 0;
    size_t sizeCached = 0;
    for (int i = 0; i < maxEntries+1; i++) {
        uint8_t k = i;
        size_t size = mCache->getBlob(&k, 1, NULL, 0);
        if (size) {
            numCached++;
            sizeCached += (size + 1);
        }
    }
    switch (GetParam().second) {
        case NNCache::Capacity::HALVE:
            // New value is too big for this cleaning algorithm.  So
            // we cleaned the cache, but did not insert the new value.
            ASSERT_EQ(maxEntries/2, numCached);
            ASSERT_EQ(size_t((maxEntries/2)*2), sizeCached);
            break;
        case NNCache::Capacity::FIT:
        case NNCache::Capacity::FIT_HALVE: {
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

class NNCacheSerializationTest : public NNCacheTest {

protected:

    virtual void SetUp() {
        NNCacheTest::SetUp();
        mTempFile.reset(new TemporaryFile());
    }

    virtual void TearDown() {
        mTempFile.reset(nullptr);
        NNCacheTest::TearDown();
    }

    std::unique_ptr<TemporaryFile> mTempFile;

    void yesStringBlob(const char *key, const char *value) {
        SCOPED_TRACE(key);

        uint8_t buf[10];
        memset(buf, 0xee, sizeof(buf));
        const size_t keySize = strlen(key);
        const size_t valueSize = strlen(value);
        ASSERT_LE(valueSize, sizeof(buf));  // Check testing assumption

        ASSERT_EQ(ssize_t(valueSize), mCache->getBlob(key, keySize, buf, sizeof(buf)));
        for (size_t i = 0; i < valueSize; i++) {
            SCOPED_TRACE(i);
            ASSERT_EQ(value[i], buf[i]);
        }
    }

    void noStringBlob(const char *key) {
        SCOPED_TRACE(key);

        uint8_t buf[10];
        memset(buf, 0xee, sizeof(buf));
        const size_t keySize = strlen(key);

        ASSERT_EQ(ssize_t(0), mCache->getBlob(key, keySize, buf, sizeof(buf)));
        for (size_t i = 0; i < sizeof(buf); i++) {
            SCOPED_TRACE(i);
            ASSERT_EQ(0xee, buf[i]);
        }
    }

};

TEST_P(NNCacheSerializationTest, ReinitializedCacheContainsValues) {
    uint8_t buf[4] = { 0xee, 0xee, 0xee, 0xee };
    mCache->setCacheFilename(&mTempFile->path[0]);
    mCache->initialize(maxKeySize, maxValueSize, maxTotalSize, GetParam());
    mCache->setBlob("abcd", 4, "efgh", 4);
    mCache->terminate();
    mCache->initialize(maxKeySize, maxValueSize, maxTotalSize, GetParam());

    // For get-with-allocator, verify that:
    // - we get the expected value size
    // - we do not modify the buffer that value pointer originally points to
    // - the value pointer gets set to something other than nullptr
    // - the newly-allocated buffer is set properly
    uint8_t *bufPtr = &buf[0];
    ASSERT_EQ(4, mCache->getBlob("abcd", 4, &bufPtr, malloc));
    ASSERT_EQ(0xee, buf[0]);
    ASSERT_EQ(0xee, buf[1]);
    ASSERT_EQ(0xee, buf[2]);
    ASSERT_EQ(0xee, buf[3]);
    ASSERT_NE(nullptr, bufPtr);
    ASSERT_EQ('e', bufPtr[0]);
    ASSERT_EQ('f', bufPtr[1]);
    ASSERT_EQ('g', bufPtr[2]);
    ASSERT_EQ('h', bufPtr[3]);

    ASSERT_EQ(4, mCache->getBlob("abcd", 4, buf, 4));
    ASSERT_EQ('e', buf[0]);
    ASSERT_EQ('f', buf[1]);
    ASSERT_EQ('g', buf[2]);
    ASSERT_EQ('h', buf[3]);
}

TEST_P(NNCacheSerializationTest, ReinitializedCacheContainsValuesSizeConstrained) {
    mCache->setCacheFilename(&mTempFile->path[0]);
    mCache->initialize(6, 10, maxTotalSize, GetParam());
    mCache->setBlob("abcd", 4, "efgh", 4);
    mCache->setBlob("abcdef", 6, "ijkl", 4);
    mCache->setBlob("ab", 2, "abcdefghij", 10);
    {
        SCOPED_TRACE("before terminate()");
        yesStringBlob("abcd", "efgh");
        yesStringBlob("abcdef", "ijkl");
        yesStringBlob("ab", "abcdefghij");
    }
    mCache->terminate();
    // Re-initialize cache with lower key/value sizes.
    mCache->initialize(5, 7, maxTotalSize, GetParam());
    {
        SCOPED_TRACE("after second initialize()");
        yesStringBlob("abcd", "efgh");
        noStringBlob("abcdef");  // key too large
        noStringBlob("ab");  // value too large
    }
}

}
