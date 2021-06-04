/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "SampleIterator"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "SampleIterator.h"

#include <arpa/inet.h>

#include <media/DataSourceBase.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ByteUtils.h>

#include "SampleTable.h"

namespace android {

SampleIterator::SampleIterator(SampleTable *table)
    : mTable(table),
      mInitialized(false),
      mTimeToSampleIndex(0),
      mTTSSampleIndex(0),
      mTTSSampleTime(0),
      mTTSCount(0),
      mTTSDuration(0) {
    reset();
}

void SampleIterator::reset() {
    mSampleToChunkIndex = 0;
    mFirstChunk = 0;
    mFirstChunkSampleIndex = 0;
    mStopChunk = 0;
    mStopChunkSampleIndex = 0;
    mSamplesPerChunk = 0;
    mChunkDesc = 0;
}

status_t SampleIterator::seekTo(uint32_t sampleIndex) {
    ALOGV("seekTo(%d)", sampleIndex);

    if (sampleIndex >= mTable->mNumSampleSizes) {
        return ERROR_END_OF_STREAM;
    }

    if (mTable->mSampleToChunkOffset < 0
            || mTable->mChunkOffsetOffset < 0
            || mTable->mSampleSizeOffset < 0
            || mTable->mTimeToSampleCount == 0) {

        return ERROR_MALFORMED;
    }

    if (mInitialized && mCurrentSampleIndex == sampleIndex) {
        return OK;
    }

    if (!mInitialized || sampleIndex < mFirstChunkSampleIndex) {
        reset();
    }

    if (sampleIndex >= mStopChunkSampleIndex) {
        status_t err;
        if ((err = findChunkRange(sampleIndex)) != OK) {
            ALOGE("findChunkRange failed");
            return err;
        }
    }

    CHECK(sampleIndex < mStopChunkSampleIndex);

    if (mSamplesPerChunk == 0) {
        ALOGE("b/22802344");
        return ERROR_MALFORMED;
    }

    uint32_t chunk =
        (sampleIndex - mFirstChunkSampleIndex) / mSamplesPerChunk
        + mFirstChunk;

    if (!mInitialized || chunk != mCurrentChunkIndex) {
        status_t err;
        if ((err = getChunkOffset(chunk, &mCurrentChunkOffset)) != OK) {
            ALOGE("getChunkOffset return error");
            return err;
        }

        mCurrentChunkSampleSizes.clear();

        uint32_t firstChunkSampleIndex =
            mFirstChunkSampleIndex
                + mSamplesPerChunk * (chunk - mFirstChunk);

        for (uint32_t i = 0; i < mSamplesPerChunk; ++i) {
            size_t sampleSize;
            if ((err = getSampleSizeDirect(
                            firstChunkSampleIndex + i, &sampleSize)) != OK) {
                ALOGE("getSampleSizeDirect return error");
                // stsc sample count is not sync with stsz sample count
                if (err == ERROR_OUT_OF_RANGE) {
                    ALOGW("stsc samples(%d) not sync with stsz samples(%d)", mSamplesPerChunk, i);
                    mSamplesPerChunk = i;
                    break;
                } else{
                    mCurrentChunkSampleSizes.clear();
                    return err;
                }
            }

            mCurrentChunkSampleSizes.push(sampleSize);
        }

        mCurrentChunkIndex = chunk;
    }

    uint32_t chunkRelativeSampleIndex =
        (sampleIndex - mFirstChunkSampleIndex) % mSamplesPerChunk;

    mCurrentSampleOffset = mCurrentChunkOffset;
    for (uint32_t i = 0; i < chunkRelativeSampleIndex; ++i) {
        mCurrentSampleOffset += mCurrentChunkSampleSizes[i];
    }

    mCurrentSampleSize = mCurrentChunkSampleSizes[chunkRelativeSampleIndex];
    if (sampleIndex < mTTSSampleIndex) {
        mTimeToSampleIndex = 0;
        mTTSSampleIndex = 0;
        mTTSSampleTime = 0;
        mTTSCount = 0;
        mTTSDuration = 0;
    }

    status_t err;
    if ((err = findSampleTimeAndDuration(
            sampleIndex, &mCurrentSampleTime, &mCurrentSampleDuration)) != OK) {
        ALOGE("findSampleTime return error");
        return err;
    }

    mCurrentSampleIndex = sampleIndex;

    mInitialized = true;

    return OK;
}

status_t SampleIterator::findChunkRange(uint32_t sampleIndex) {
    CHECK(sampleIndex >= mFirstChunkSampleIndex);

    while (sampleIndex >= mStopChunkSampleIndex) {
        if (mSampleToChunkIndex == mTable->mNumSampleToChunkOffsets) {
            return ERROR_OUT_OF_RANGE;
        }

        mFirstChunkSampleIndex = mStopChunkSampleIndex;

        const SampleTable::SampleToChunkEntry *entry =
            &mTable->mSampleToChunkEntries[mSampleToChunkIndex];

        mFirstChunk = entry->startChunk;
        mSamplesPerChunk = entry->samplesPerChunk;
        mChunkDesc = entry->chunkDesc;

        if (mSampleToChunkIndex + 1 < mTable->mNumSampleToChunkOffsets) {
            mStopChunk = entry[1].startChunk;

            if (mSamplesPerChunk == 0 || mStopChunk < mFirstChunk ||
                (mStopChunk - mFirstChunk) > UINT32_MAX / mSamplesPerChunk ||
                ((mStopChunk - mFirstChunk) * mSamplesPerChunk >
                 UINT32_MAX - mFirstChunkSampleIndex)) {

                return ERROR_OUT_OF_RANGE;
            }
            mStopChunkSampleIndex =
                mFirstChunkSampleIndex
                    + (mStopChunk - mFirstChunk) * mSamplesPerChunk;
        } else {
            mStopChunk = 0xffffffff;
            mStopChunkSampleIndex = 0xffffffff;
        }

        ++mSampleToChunkIndex;
    }

    return OK;
}

status_t SampleIterator::getChunkOffset(uint32_t chunk, off64_t *offset) {
    *offset = 0;

    if (chunk >= mTable->mNumChunkOffsets) {
        return ERROR_OUT_OF_RANGE;
    }

    if (mTable->mChunkOffsetType == SampleTable::kChunkOffsetType32) {
        uint32_t offset32;

        if (mTable->mDataSource->readAt(
                    mTable->mChunkOffsetOffset + 8 + 4 * chunk,
                    &offset32,
                    sizeof(offset32)) < (ssize_t)sizeof(offset32)) {
            return ERROR_IO;
        }

        *offset = ntohl(offset32);
    } else {
        CHECK_EQ(mTable->mChunkOffsetType, SampleTable::kChunkOffsetType64);

        uint64_t offset64;
        if (mTable->mDataSource->readAt(
                    mTable->mChunkOffsetOffset + 8 + 8 * chunk,
                    &offset64,
                    sizeof(offset64)) < (ssize_t)sizeof(offset64)) {
            return ERROR_IO;
        }

        *offset = ntoh64(offset64);
    }

    return OK;
}

status_t SampleIterator::getSampleSizeDirect(
        uint32_t sampleIndex, size_t *size) {
    *size = 0;

    if (sampleIndex >= mTable->mNumSampleSizes) {
        return ERROR_OUT_OF_RANGE;
    }

    if (mTable->mDefaultSampleSize > 0) {
        *size = mTable->mDefaultSampleSize;
        return OK;
    }

    switch (mTable->mSampleSizeFieldSize) {
        case 32:
        {
            uint32_t x;
            if (mTable->mDataSource->readAt(
                        mTable->mSampleSizeOffset + 12 + 4 * sampleIndex,
                        &x, sizeof(x)) < (ssize_t)sizeof(x)) {
                return ERROR_IO;
            }

            *size = ntohl(x);
            break;
        }

        case 16:
        {
            uint16_t x;
            if (mTable->mDataSource->readAt(
                        mTable->mSampleSizeOffset + 12 + 2 * sampleIndex,
                        &x, sizeof(x)) < (ssize_t)sizeof(x)) {
                return ERROR_IO;
            }

            *size = ntohs(x);
            break;
        }

        case 8:
        {
            uint8_t x;
            if (mTable->mDataSource->readAt(
                        mTable->mSampleSizeOffset + 12 + sampleIndex,
                        &x, sizeof(x)) < (ssize_t)sizeof(x)) {
                return ERROR_IO;
            }

            *size = x;
            break;
        }

        default:
        {
            CHECK_EQ(mTable->mSampleSizeFieldSize, 4u);

            uint8_t x;
            if (mTable->mDataSource->readAt(
                        mTable->mSampleSizeOffset + 12 + sampleIndex / 2,
                        &x, sizeof(x)) < (ssize_t)sizeof(x)) {
                return ERROR_IO;
            }

            *size = (sampleIndex & 1) ? x & 0x0f : x >> 4;
            break;
        }
    }

    return OK;
}

status_t SampleIterator::findSampleTimeAndDuration(
        uint32_t sampleIndex, uint64_t *time, uint64_t *duration) {
    if (sampleIndex >= mTable->mNumSampleSizes) {
        return ERROR_OUT_OF_RANGE;
    }

    while (true) {
        if (mTTSSampleIndex > UINT32_MAX - mTTSCount) {
            return ERROR_OUT_OF_RANGE;
        }
        if(sampleIndex < mTTSSampleIndex + mTTSCount) {
            break;
        }
        if (mTimeToSampleIndex == mTable->mTimeToSampleCount ||
            (mTTSDuration != 0 && mTTSCount > UINT64_MAX / mTTSDuration) ||
            mTTSSampleTime > UINT64_MAX - (mTTSCount * mTTSDuration)) {
            return ERROR_OUT_OF_RANGE;
        }

        mTTSSampleIndex += mTTSCount;
        mTTSSampleTime += mTTSCount * mTTSDuration;

        mTTSCount = mTable->mTimeToSample[2 * mTimeToSampleIndex];
        mTTSDuration = mTable->mTimeToSample[2 * mTimeToSampleIndex + 1];

        ++mTimeToSampleIndex;
    }

    // below is equivalent to:
    // *time = mTTSSampleTime + mTTSDuration * (sampleIndex - mTTSSampleIndex);
    uint64_t tmp;
    if (__builtin_sub_overflow(sampleIndex, mTTSSampleIndex, &tmp) ||
            __builtin_mul_overflow(mTTSDuration, tmp, &tmp) ||
            __builtin_add_overflow(mTTSSampleTime, tmp, &tmp)) {
        return ERROR_OUT_OF_RANGE;
    }
    *time = tmp;

    int32_t offset = mTable->getCompositionTimeOffset(sampleIndex);
    if ((offset < 0 && *time < (offset == INT32_MIN ?
            INT64_MAX : uint64_t(-offset))) ||
            (offset > 0 && *time > UINT64_MAX - offset)) {
        ALOGE("%llu + %d would overflow", (unsigned long long) *time, offset);
        return ERROR_OUT_OF_RANGE;
    }
    if (offset > 0) {
        *time += offset;
    } else {
        *time -= (offset == INT64_MIN ? INT64_MAX : (-offset));
    }

    *duration = mTTSDuration;

    return OK;
}

}  // namespace android

