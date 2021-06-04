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

#define LOG_TAG "SampleTable"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <limits>

#include "SampleTable.h"
#include "SampleIterator.h"

#include <arpa/inet.h>

#include <media/MediaExtractorPluginApi.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ByteUtils.h>

/* TODO: remove after being merged into other branches */
#ifndef UINT32_MAX
#define UINT32_MAX       (4294967295U)
#endif

namespace android {

// static
const uint32_t SampleTable::kChunkOffsetType32 = FOURCC("stco");
// static
const uint32_t SampleTable::kChunkOffsetType64 = FOURCC("co64");
// static
const uint32_t SampleTable::kSampleSizeType32 = FOURCC("stsz");
// static
const uint32_t SampleTable::kSampleSizeTypeCompact = FOURCC("stz2");

////////////////////////////////////////////////////////////////////////////////

const off64_t kMaxOffset = std::numeric_limits<off64_t>::max();

struct SampleTable::CompositionDeltaLookup {
    CompositionDeltaLookup();

    void setEntries(
            const int32_t *deltaEntries, size_t numDeltaEntries);

    int32_t getCompositionTimeOffset(uint32_t sampleIndex);

private:
    Mutex mLock;

    const int32_t *mDeltaEntries;
    size_t mNumDeltaEntries;

    size_t mCurrentDeltaEntry;
    size_t mCurrentEntrySampleIndex;

    DISALLOW_EVIL_CONSTRUCTORS(CompositionDeltaLookup);
};

SampleTable::CompositionDeltaLookup::CompositionDeltaLookup()
    : mDeltaEntries(NULL),
      mNumDeltaEntries(0),
      mCurrentDeltaEntry(0),
      mCurrentEntrySampleIndex(0) {
}

void SampleTable::CompositionDeltaLookup::setEntries(
        const int32_t *deltaEntries, size_t numDeltaEntries) {
    Mutex::Autolock autolock(mLock);

    mDeltaEntries = deltaEntries;
    mNumDeltaEntries = numDeltaEntries;
    mCurrentDeltaEntry = 0;
    mCurrentEntrySampleIndex = 0;
}

int32_t SampleTable::CompositionDeltaLookup::getCompositionTimeOffset(
        uint32_t sampleIndex) {
    Mutex::Autolock autolock(mLock);

    if (mDeltaEntries == NULL) {
        return 0;
    }

    if (sampleIndex < mCurrentEntrySampleIndex) {
        mCurrentDeltaEntry = 0;
        mCurrentEntrySampleIndex = 0;
    }

    while (mCurrentDeltaEntry < mNumDeltaEntries) {
        uint32_t sampleCount = mDeltaEntries[2 * mCurrentDeltaEntry];
        if (sampleIndex < mCurrentEntrySampleIndex + sampleCount) {
            return mDeltaEntries[2 * mCurrentDeltaEntry + 1];
        }

        mCurrentEntrySampleIndex += sampleCount;
        ++mCurrentDeltaEntry;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

SampleTable::SampleTable(DataSourceHelper *source)
    : mDataSource(source),
      mChunkOffsetOffset(-1),
      mChunkOffsetType(0),
      mNumChunkOffsets(0),
      mSampleToChunkOffset(-1),
      mNumSampleToChunkOffsets(0),
      mSampleSizeOffset(-1),
      mSampleSizeFieldSize(0),
      mDefaultSampleSize(0),
      mNumSampleSizes(0),
      mHasTimeToSample(false),
      mTimeToSampleCount(0),
      mTimeToSample(NULL),
      mSampleTimeEntries(NULL),
      mCompositionTimeDeltaEntries(NULL),
      mNumCompositionTimeDeltaEntries(0),
      mCompositionDeltaLookup(new CompositionDeltaLookup),
      mSyncSampleOffset(-1),
      mNumSyncSamples(0),
      mSyncSamples(NULL),
      mLastSyncSampleIndex(0),
      mSampleToChunkEntries(NULL),
      mTotalSize(0) {
    mSampleIterator = new SampleIterator(this);
}

SampleTable::~SampleTable() {
    delete[] mSampleToChunkEntries;
    mSampleToChunkEntries = NULL;

    delete[] mSyncSamples;
    mSyncSamples = NULL;

    delete[] mTimeToSample;
    mTimeToSample = NULL;

    delete mCompositionDeltaLookup;
    mCompositionDeltaLookup = NULL;

    delete[] mCompositionTimeDeltaEntries;
    mCompositionTimeDeltaEntries = NULL;

    delete[] mSampleTimeEntries;
    mSampleTimeEntries = NULL;

    delete mSampleIterator;
    mSampleIterator = NULL;
}

bool SampleTable::isValid() const {
    return mChunkOffsetOffset >= 0
        && mSampleToChunkOffset >= 0
        && mSampleSizeOffset >= 0
        && mHasTimeToSample;
}

status_t SampleTable::setChunkOffsetParams(
        uint32_t type, off64_t data_offset, size_t data_size) {
    if (mChunkOffsetOffset >= 0) {
        return ERROR_MALFORMED;
    }

    CHECK(type == kChunkOffsetType32 || type == kChunkOffsetType64);

    mChunkOffsetOffset = data_offset;
    mChunkOffsetType = type;

    if (data_size < 8) {
        return ERROR_MALFORMED;
    }

    uint8_t header[8];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    if (U32_AT(header) != 0) {
        // Expected version = 0, flags = 0.
        return ERROR_MALFORMED;
    }

    mNumChunkOffsets = U32_AT(&header[4]);

    if (mChunkOffsetType == kChunkOffsetType32) {
      if ((data_size - 8) / 4 < mNumChunkOffsets) {
            return ERROR_MALFORMED;
        }
    } else {
      if ((data_size - 8) / 8 < mNumChunkOffsets) {
            return ERROR_MALFORMED;
        }
    }

    return OK;
}

status_t SampleTable::setSampleToChunkParams(
        off64_t data_offset, size_t data_size) {
    if (mSampleToChunkOffset >= 0) {
        // already set
        return ERROR_MALFORMED;
    }

    if (data_offset < 0) {
        return ERROR_MALFORMED;
    }

    mSampleToChunkOffset = data_offset;

    if (data_size < 8) {
        return ERROR_MALFORMED;
    }

    uint8_t header[8];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    if (U32_AT(header) != 0) {
        // Expected version = 0, flags = 0.
        return ERROR_MALFORMED;
    }

    mNumSampleToChunkOffsets = U32_AT(&header[4]);

    if ((data_size - 8) / sizeof(SampleToChunkEntry) < mNumSampleToChunkOffsets) {
        return ERROR_MALFORMED;
    }

    if ((uint64_t)kMaxTotalSize / sizeof(SampleToChunkEntry) <=
            (uint64_t)mNumSampleToChunkOffsets) {
        ALOGE("Sample-to-chunk table size too large.");
        return ERROR_OUT_OF_RANGE;
    }

    mTotalSize += (uint64_t)mNumSampleToChunkOffsets *
            sizeof(SampleToChunkEntry);
    if (mTotalSize > kMaxTotalSize) {
        ALOGE("Sample-to-chunk table size would make sample table too large.\n"
              "    Requested sample-to-chunk table size = %llu\n"
              "    Eventual sample table size >= %llu\n"
              "    Allowed sample table size = %llu\n",
              (unsigned long long)mNumSampleToChunkOffsets *
                      sizeof(SampleToChunkEntry),
              (unsigned long long)mTotalSize,
              (unsigned long long)kMaxTotalSize);
        return ERROR_OUT_OF_RANGE;
    }

    mSampleToChunkEntries =
        new (std::nothrow) SampleToChunkEntry[mNumSampleToChunkOffsets];
    if (!mSampleToChunkEntries) {
        ALOGE("Cannot allocate sample-to-chunk table with %llu entries.",
                (unsigned long long)mNumSampleToChunkOffsets);
        return ERROR_OUT_OF_RANGE;
    }

    if (mNumSampleToChunkOffsets == 0) {
        return OK;
    }

    if ((off64_t)(kMaxOffset - 8 -
            ((mNumSampleToChunkOffsets - 1) * sizeof(SampleToChunkEntry)))
            < mSampleToChunkOffset) {
        return ERROR_MALFORMED;
    }

    for (uint32_t i = 0; i < mNumSampleToChunkOffsets; ++i) {
        uint8_t buffer[sizeof(SampleToChunkEntry)];

        if (mDataSource->readAt(
                    mSampleToChunkOffset + 8 + i * sizeof(SampleToChunkEntry),
                    buffer,
                    sizeof(buffer))
                != (ssize_t)sizeof(buffer)) {
            return ERROR_IO;
        }
        // chunk index is 1 based in the spec.
        if (U32_AT(buffer) < 1) {
            ALOGE("b/23534160");
            return ERROR_OUT_OF_RANGE;
        }

        // We want the chunk index to be 0-based.
        mSampleToChunkEntries[i].startChunk = U32_AT(buffer) - 1;
        mSampleToChunkEntries[i].samplesPerChunk = U32_AT(&buffer[4]);
        mSampleToChunkEntries[i].chunkDesc = U32_AT(&buffer[8]);
    }

    return OK;
}

status_t SampleTable::setSampleSizeParams(
        uint32_t type, off64_t data_offset, size_t data_size) {
    if (mSampleSizeOffset >= 0) {
        return ERROR_MALFORMED;
    }

    CHECK(type == kSampleSizeType32 || type == kSampleSizeTypeCompact);

    mSampleSizeOffset = data_offset;

    if (data_size < 12) {
        return ERROR_MALFORMED;
    }

    uint8_t header[12];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    if (U32_AT(header) != 0) {
        // Expected version = 0, flags = 0.
        return ERROR_MALFORMED;
    }

    mDefaultSampleSize = U32_AT(&header[4]);
    mNumSampleSizes = U32_AT(&header[8]);
    if (mNumSampleSizes > (UINT32_MAX - 12) / 16) {
        ALOGE("b/23247055, mNumSampleSizes(%u)", mNumSampleSizes);
        return ERROR_MALFORMED;
    }

    if (type == kSampleSizeType32) {
        mSampleSizeFieldSize = 32;

        if (mDefaultSampleSize != 0) {
            return OK;
        }

        if (data_size < 12 + mNumSampleSizes * 4) {
            return ERROR_MALFORMED;
        }
    } else {
        if ((mDefaultSampleSize & 0xffffff00) != 0) {
            // The high 24 bits are reserved and must be 0.
            return ERROR_MALFORMED;
        }

        mSampleSizeFieldSize = mDefaultSampleSize & 0xff;
        mDefaultSampleSize = 0;

        if (mSampleSizeFieldSize != 4 && mSampleSizeFieldSize != 8
            && mSampleSizeFieldSize != 16) {
            return ERROR_MALFORMED;
        }

        if (data_size < 12 + (mNumSampleSizes * mSampleSizeFieldSize + 4) / 8) {
            return ERROR_MALFORMED;
        }
    }

    return OK;
}

status_t SampleTable::setTimeToSampleParams(
        off64_t data_offset, size_t data_size) {
    if (mHasTimeToSample || data_size < 8) {
        return ERROR_MALFORMED;
    }

    uint8_t header[8];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    if (U32_AT(header) != 0) {
        // Expected version = 0, flags = 0.
        return ERROR_MALFORMED;
    }

    mTimeToSampleCount = U32_AT(&header[4]);
    if (mTimeToSampleCount > (data_size - 8) / (2 * sizeof(uint32_t))) {
        ALOGE("Time-to-sample table size too large.");
        return ERROR_OUT_OF_RANGE;
    }

    uint64_t allocSize = (uint64_t)mTimeToSampleCount * 2 * sizeof(uint32_t);
    mTotalSize += allocSize;
    if (mTotalSize > kMaxTotalSize) {
        ALOGE("Time-to-sample table size would make sample table too large.\n"
              "    Requested time-to-sample table size = %llu\n"
              "    Eventual sample table size >= %llu\n"
              "    Allowed sample table size = %llu\n",
              (unsigned long long)allocSize,
              (unsigned long long)mTotalSize,
              (unsigned long long)kMaxTotalSize);
        return ERROR_OUT_OF_RANGE;
    }

    mTimeToSample = new (std::nothrow) uint32_t[mTimeToSampleCount * 2];
    if (!mTimeToSample) {
        ALOGE("Cannot allocate time-to-sample table with %llu entries.",
                (unsigned long long)mTimeToSampleCount);
        return ERROR_OUT_OF_RANGE;
    }

    if (mDataSource->readAt(data_offset + 8, mTimeToSample,
            (size_t)allocSize) < (ssize_t)allocSize) {
        ALOGE("Incomplete data read for time-to-sample table.");
        return ERROR_IO;
    }

    for (size_t i = 0; i < mTimeToSampleCount * 2; ++i) {
        mTimeToSample[i] = ntohl(mTimeToSample[i]);
    }

    mHasTimeToSample = true;
    return OK;
}

// NOTE: per 14996-12, version 0 ctts contains unsigned values, while version 1
// contains signed values, however some software creates version 0 files that
// contain signed values, so we're always treating the values as signed,
// regardless of version.
status_t SampleTable::setCompositionTimeToSampleParams(
        off64_t data_offset, size_t data_size) {
    ALOGI("There are reordered frames present.");

    if (mCompositionTimeDeltaEntries != NULL || data_size < 8) {
        return ERROR_MALFORMED;
    }

    uint8_t header[8];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header))
            < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    uint32_t flags = U32_AT(header);
    uint32_t version = flags >> 24;
    flags &= 0xffffff;

    if ((version != 0 && version != 1) || flags != 0) {
        // Expected version = 0 or 1, flags = 0.
        return ERROR_MALFORMED;
    }

    size_t numEntries = U32_AT(&header[4]);

    if (((SIZE_MAX / 8) - 1 < numEntries) || (data_size != (numEntries + 1) * 8)) {
        return ERROR_MALFORMED;
    }

    mNumCompositionTimeDeltaEntries = numEntries;
    uint64_t allocSize = (uint64_t)numEntries * 2 * sizeof(int32_t);
    if (allocSize > kMaxTotalSize) {
        ALOGE("Composition-time-to-sample table size too large.");
        return ERROR_OUT_OF_RANGE;
    }

    mTotalSize += allocSize;
    if (mTotalSize > kMaxTotalSize) {
        ALOGE("Composition-time-to-sample table would make sample table too large.\n"
              "    Requested composition-time-to-sample table size = %llu\n"
              "    Eventual sample table size >= %llu\n"
              "    Allowed sample table size = %llu\n",
              (unsigned long long)allocSize,
              (unsigned long long)mTotalSize,
              (unsigned long long)kMaxTotalSize);
        return ERROR_OUT_OF_RANGE;
    }

    mCompositionTimeDeltaEntries = new (std::nothrow) int32_t[2 * numEntries];
    if (!mCompositionTimeDeltaEntries) {
        ALOGE("Cannot allocate composition-time-to-sample table with %llu "
                "entries.", (unsigned long long)numEntries);
        return ERROR_OUT_OF_RANGE;
    }

    if (mDataSource->readAt(data_offset + 8, mCompositionTimeDeltaEntries,
            (size_t)allocSize) < (ssize_t)allocSize) {
        delete[] mCompositionTimeDeltaEntries;
        mCompositionTimeDeltaEntries = NULL;

        return ERROR_IO;
    }

    for (size_t i = 0; i < 2 * numEntries; ++i) {
        mCompositionTimeDeltaEntries[i] = ntohl(mCompositionTimeDeltaEntries[i]);
    }

    mCompositionDeltaLookup->setEntries(
            mCompositionTimeDeltaEntries, mNumCompositionTimeDeltaEntries);

    return OK;
}

status_t SampleTable::setSyncSampleParams(off64_t data_offset, size_t data_size) {
    if (mSyncSampleOffset >= 0 || data_size < 8) {
        return ERROR_MALFORMED;
    }

    uint8_t header[8];
    if (mDataSource->readAt(
                data_offset, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return ERROR_IO;
    }

    if (U32_AT(header) != 0) {
        // Expected version = 0, flags = 0.
        return ERROR_MALFORMED;
    }

    uint32_t numSyncSamples = U32_AT(&header[4]);

    if (numSyncSamples < 2) {
        ALOGV("Table of sync samples is empty or has only a single entry!");
    }

    uint64_t allocSize = (uint64_t)numSyncSamples * sizeof(uint32_t);
    if (allocSize > data_size - 8) {
        ALOGW("b/124771364 - allocSize(%lu) > size(%lu)",
                (unsigned long)allocSize, (unsigned long)(data_size - 8));
        android_errorWriteLog(0x534e4554, "124771364");
        return ERROR_MALFORMED;
    }
    if (allocSize > kMaxTotalSize) {
        ALOGE("Sync sample table size too large.");
        return ERROR_OUT_OF_RANGE;
    }

    mTotalSize += allocSize;
    if (mTotalSize > kMaxTotalSize) {
        ALOGE("Sync sample table size would make sample table too large.\n"
              "    Requested sync sample table size = %llu\n"
              "    Eventual sample table size >= %llu\n"
              "    Allowed sample table size = %llu\n",
              (unsigned long long)allocSize,
              (unsigned long long)mTotalSize,
              (unsigned long long)kMaxTotalSize);
        return ERROR_OUT_OF_RANGE;
    }

    mSyncSamples = new (std::nothrow) uint32_t[numSyncSamples];
    if (!mSyncSamples) {
        ALOGE("Cannot allocate sync sample table with %llu entries.",
                (unsigned long long)numSyncSamples);
        return ERROR_OUT_OF_RANGE;
    }

    if (mDataSource->readAt(data_offset + 8, mSyncSamples,
            (size_t)allocSize) != (ssize_t)allocSize) {
        delete[] mSyncSamples;
        mSyncSamples = NULL;
        return ERROR_IO;
    }

    for (size_t i = 0; i < numSyncSamples; ++i) {
        if (mSyncSamples[i] == 0) {
            ALOGE("b/32423862, unexpected zero value in stss");
            continue;
        }
        mSyncSamples[i] = ntohl(mSyncSamples[i]) - 1;
    }

    mSyncSampleOffset = data_offset;
    mNumSyncSamples = numSyncSamples;

    return OK;
}

uint32_t SampleTable::countChunkOffsets() const {
    return mNumChunkOffsets;
}

uint32_t SampleTable::countSamples() const {
    return mNumSampleSizes;
}

status_t SampleTable::getMaxSampleSize(size_t *max_size) {
    Mutex::Autolock autoLock(mLock);

    *max_size = 0;

    for (uint32_t i = 0; i < mNumSampleSizes; ++i) {
        size_t sample_size;
        status_t err = getSampleSize_l(i, &sample_size);

        if (err != OK) {
            return err;
        }

        if (sample_size > *max_size) {
            *max_size = sample_size;
        }
    }

    return OK;
}

uint32_t abs_difference(uint64_t time1, uint64_t time2) {
    return time1 > time2 ? time1 - time2 : time2 - time1;
}

// static
int SampleTable::CompareIncreasingTime(const void *_a, const void *_b) {
    const SampleTimeEntry *a = (const SampleTimeEntry *)_a;
    const SampleTimeEntry *b = (const SampleTimeEntry *)_b;

    if (a->mCompositionTime < b->mCompositionTime) {
        return -1;
    } else if (a->mCompositionTime > b->mCompositionTime) {
        return 1;
    }

    return 0;
}

void SampleTable::buildSampleEntriesTable() {
    Mutex::Autolock autoLock(mLock);

    if (mSampleTimeEntries != NULL || mNumSampleSizes == 0) {
        if (mNumSampleSizes == 0) {
            ALOGE("b/23247055, mNumSampleSizes(%u)", mNumSampleSizes);
        }
        return;
    }

    mTotalSize += (uint64_t)mNumSampleSizes * sizeof(SampleTimeEntry);
    if (mTotalSize > kMaxTotalSize) {
        ALOGE("Sample entry table size would make sample table too large.\n"
              "    Requested sample entry table size = %llu\n"
              "    Eventual sample table size >= %llu\n"
              "    Allowed sample table size = %llu\n",
              (unsigned long long)mNumSampleSizes * sizeof(SampleTimeEntry),
              (unsigned long long)mTotalSize,
              (unsigned long long)kMaxTotalSize);
        return;
    }

    mSampleTimeEntries = new (std::nothrow) SampleTimeEntry[mNumSampleSizes];
    if (!mSampleTimeEntries) {
        ALOGE("Cannot allocate sample entry table with %llu entries.",
                (unsigned long long)mNumSampleSizes);
        return;
    }

    uint32_t sampleIndex = 0;
    uint64_t sampleTime = 0;

    for (uint32_t i = 0; i < mTimeToSampleCount; ++i) {
        uint32_t n = mTimeToSample[2 * i];
        uint32_t delta = mTimeToSample[2 * i + 1];

        for (uint32_t j = 0; j < n; ++j) {
            if (sampleIndex < mNumSampleSizes) {
                // Technically this should always be the case if the file
                // is well-formed, but you know... there's (gasp) malformed
                // content out there.

                mSampleTimeEntries[sampleIndex].mSampleIndex = sampleIndex;

                int32_t compTimeDelta =
                    mCompositionDeltaLookup->getCompositionTimeOffset(
                            sampleIndex);

                if ((compTimeDelta < 0 && sampleTime <
                        (compTimeDelta == INT32_MIN ?
                                INT32_MAX : uint32_t(-compTimeDelta)))
                        || (compTimeDelta > 0 &&
                                sampleTime > UINT64_MAX - compTimeDelta)) {
                    ALOGE("%llu + %d would overflow, clamping",
                            (unsigned long long) sampleTime, compTimeDelta);
                    if (compTimeDelta < 0) {
                        sampleTime = 0;
                    } else {
                        sampleTime = UINT64_MAX;
                    }
                    compTimeDelta = 0;
                }

                mSampleTimeEntries[sampleIndex].mCompositionTime =
                        compTimeDelta > 0 ? sampleTime + compTimeDelta:
                                sampleTime - (-compTimeDelta);
            }

            ++sampleIndex;
            if (sampleTime > UINT64_MAX - delta) {
                ALOGE("%llu + %u would overflow, clamping",
                    (unsigned long long) sampleTime, delta);
                sampleTime = UINT64_MAX;
            } else {
                sampleTime += delta;
            }
        }
    }

    qsort(mSampleTimeEntries, mNumSampleSizes, sizeof(SampleTimeEntry),
          CompareIncreasingTime);
}

status_t SampleTable::findSampleAtTime(
        uint64_t req_time, uint64_t scale_num, uint64_t scale_den,
        uint32_t *sample_index, uint32_t flags) {
    buildSampleEntriesTable();

    if (mSampleTimeEntries == NULL) {
        return ERROR_OUT_OF_RANGE;
    }

    if (flags == kFlagFrameIndex) {
        if (req_time >= mNumSampleSizes) {
            return ERROR_OUT_OF_RANGE;
        }
        *sample_index = mSampleTimeEntries[req_time].mSampleIndex;
        return OK;
    }

    uint32_t left = 0;
    uint32_t right_plus_one = mNumSampleSizes;
    while (left < right_plus_one) {
        uint32_t center = left + (right_plus_one - left) / 2;
        uint64_t centerTime =
            getSampleTime(center, scale_num, scale_den);

        if (req_time < centerTime) {
            right_plus_one = center;
        } else if (req_time > centerTime) {
            left = center + 1;
        } else {
            *sample_index = mSampleTimeEntries[center].mSampleIndex;
            return OK;
        }
    }

    uint32_t closestIndex = left;

    if (closestIndex == mNumSampleSizes) {
        if (flags == kFlagAfter) {
            return ERROR_OUT_OF_RANGE;
        }
        flags = kFlagBefore;
    } else if (closestIndex == 0) {
        if (flags == kFlagBefore) {
            // normally we should return out of range, but that is
            // treated as end-of-stream.  instead return first sample
            //
            // return ERROR_OUT_OF_RANGE;
        }
        flags = kFlagAfter;
    }

    switch (flags) {
        case kFlagBefore:
        {
            --closestIndex;
            break;
        }

        case kFlagAfter:
        {
            // nothing to do
            break;
        }

        default:
        {
            CHECK(flags == kFlagClosest);
            // pick closest based on timestamp. use abs_difference for safety
            if (abs_difference(
                    getSampleTime(closestIndex, scale_num, scale_den), req_time) >
                abs_difference(
                    req_time, getSampleTime(closestIndex - 1, scale_num, scale_den))) {
                --closestIndex;
            }
            break;
        }
    }

    *sample_index = mSampleTimeEntries[closestIndex].mSampleIndex;
    return OK;
}

status_t SampleTable::findSyncSampleNear(
        uint32_t start_sample_index, uint32_t *sample_index, uint32_t flags) {
    Mutex::Autolock autoLock(mLock);

    *sample_index = 0;

    if (mSyncSampleOffset < 0) {
        // All samples are sync-samples.
        *sample_index = start_sample_index;
        return OK;
    }

    if (mNumSyncSamples == 0) {
        *sample_index = 0;
        return OK;
    }

    uint32_t left = 0;
    uint32_t right_plus_one = mNumSyncSamples;
    while (left < right_plus_one) {
        uint32_t center = left + (right_plus_one - left) / 2;
        uint32_t x = mSyncSamples[center];

        if (start_sample_index < x) {
            right_plus_one = center;
        } else if (start_sample_index > x) {
            left = center + 1;
        } else {
            *sample_index = x;
            return OK;
        }
    }

    if (left == mNumSyncSamples) {
        if (flags == kFlagAfter) {
            ALOGE("tried to find a sync frame after the last one: %d", left);
            return ERROR_OUT_OF_RANGE;
        }
        flags = kFlagBefore;
    }
    else if (left == 0) {
        if (flags == kFlagBefore) {
            ALOGE("tried to find a sync frame before the first one: %d", left);

            // normally we should return out of range, but that is
            // treated as end-of-stream.  instead seek to first sync
            //
            // return ERROR_OUT_OF_RANGE;
        }
        flags = kFlagAfter;
    }

    // Now ssi[left - 1] <(=) start_sample_index <= ssi[left]
    switch (flags) {
        case kFlagBefore:
        {
            --left;
            break;
        }
        case kFlagAfter:
        {
            // nothing to do
            break;
        }
        default:
        {
            // this route is not used, but implement it nonetheless
            CHECK(flags == kFlagClosest);

            status_t err = mSampleIterator->seekTo(start_sample_index);
            if (err != OK) {
                return err;
            }
            uint64_t sample_time = mSampleIterator->getSampleTime();

            err = mSampleIterator->seekTo(mSyncSamples[left]);
            if (err != OK) {
                return err;
            }
            uint64_t upper_time = mSampleIterator->getSampleTime();

            err = mSampleIterator->seekTo(mSyncSamples[left - 1]);
            if (err != OK) {
                return err;
            }
            uint64_t lower_time = mSampleIterator->getSampleTime();

            // use abs_difference for safety
            if (abs_difference(upper_time, sample_time) >
                abs_difference(sample_time, lower_time)) {
                --left;
            }
            break;
        }
    }

    *sample_index = mSyncSamples[left];
    return OK;
}

status_t SampleTable::findThumbnailSample(uint32_t *sample_index) {
    Mutex::Autolock autoLock(mLock);

    if (mSyncSampleOffset < 0) {
        // All samples are sync-samples.
        *sample_index = 0;
        return OK;
    }

    uint32_t bestSampleIndex = 0;
    size_t maxSampleSize = 0;

    static const size_t kMaxNumSyncSamplesToScan = 20;

    // Consider the first kMaxNumSyncSamplesToScan sync samples and
    // pick the one with the largest (compressed) size as the thumbnail.

    size_t numSamplesToScan = mNumSyncSamples;
    if (numSamplesToScan > kMaxNumSyncSamplesToScan) {
        numSamplesToScan = kMaxNumSyncSamplesToScan;
    }

    for (size_t i = 0; i < numSamplesToScan; ++i) {
        uint32_t x = mSyncSamples[i];

        // Now x is a sample index.
        size_t sampleSize;
        status_t err = getSampleSize_l(x, &sampleSize);
        if (err != OK) {
            return err;
        }

        if (i == 0 || sampleSize > maxSampleSize) {
            bestSampleIndex = x;
            maxSampleSize = sampleSize;
        }
    }

    *sample_index = bestSampleIndex;

    return OK;
}

status_t SampleTable::getSampleSize_l(
        uint32_t sampleIndex, size_t *sampleSize) {
    return mSampleIterator->getSampleSizeDirect(
            sampleIndex, sampleSize);
}

uint32_t SampleTable::getLastSampleIndexInChunk() {
    Mutex::Autolock autoLock(mLock);
    return mSampleIterator->getLastSampleIndexInChunk();
}

status_t SampleTable::getMetaDataForSample(
        uint32_t sampleIndex,
        off64_t *offset,
        size_t *size,
        uint64_t *compositionTime,
        bool *isSyncSample,
        uint64_t *sampleDuration) {
    Mutex::Autolock autoLock(mLock);

    status_t err;
    if ((err = mSampleIterator->seekTo(sampleIndex)) != OK) {
        return err;
    }

    if (offset) {
        *offset = mSampleIterator->getSampleOffset();
    }

    if (size) {
        *size = mSampleIterator->getSampleSize();
    }

    if (compositionTime) {
        *compositionTime = mSampleIterator->getSampleTime();
    }

    if (isSyncSample) {
        *isSyncSample = false;
        if (mSyncSampleOffset < 0) {
            // Every sample is a sync sample.
            *isSyncSample = true;
        } else {
            size_t i = (mLastSyncSampleIndex < mNumSyncSamples)
                    && (mSyncSamples[mLastSyncSampleIndex] <= sampleIndex)
                ? mLastSyncSampleIndex : 0;

            while (i < mNumSyncSamples && mSyncSamples[i] < sampleIndex) {
                ++i;
            }

            if (i < mNumSyncSamples && mSyncSamples[i] == sampleIndex) {
                *isSyncSample = true;
            }

            mLastSyncSampleIndex = i;
        }
    }

    if (sampleDuration) {
        *sampleDuration = mSampleIterator->getSampleDuration();
    }

    return OK;
}

int32_t SampleTable::getCompositionTimeOffset(uint32_t sampleIndex) {
    return mCompositionDeltaLookup->getCompositionTimeOffset(sampleIndex);
}

}  // namespace android
