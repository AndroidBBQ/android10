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

#ifndef SAMPLE_TABLE_H_

#define SAMPLE_TABLE_H_

#include <sys/types.h>
#include <stdint.h>

#include <media/MediaExtractorPluginHelper.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

namespace android {

class DataSourceHelper;
struct SampleIterator;

class SampleTable : public RefBase {
public:
    explicit SampleTable(DataSourceHelper *source);

    bool isValid() const;

    // type can be 'stco' or 'co64'.
    status_t setChunkOffsetParams(
            uint32_t type, off64_t data_offset, size_t data_size);

    status_t setSampleToChunkParams(off64_t data_offset, size_t data_size);

    // type can be 'stsz' or 'stz2'.
    status_t setSampleSizeParams(
            uint32_t type, off64_t data_offset, size_t data_size);

    status_t setTimeToSampleParams(off64_t data_offset, size_t data_size);

    status_t setCompositionTimeToSampleParams(
            off64_t data_offset, size_t data_size);

    status_t setSyncSampleParams(off64_t data_offset, size_t data_size);

    ////////////////////////////////////////////////////////////////////////////

    uint32_t countChunkOffsets() const;

    uint32_t countSamples() const;

    status_t getMaxSampleSize(size_t *size);

    status_t getMetaDataForSample(
            uint32_t sampleIndex,
            off64_t *offset,
            size_t *size,
            uint64_t *compositionTime,
            bool *isSyncSample = NULL,
            uint64_t *sampleDuration = NULL);

    // call only after getMetaDataForSample has been called successfully.
    uint32_t getLastSampleIndexInChunk();

    enum {
        kFlagBefore,
        kFlagAfter,
        kFlagClosest,
        kFlagFrameIndex,
    };
    status_t findSampleAtTime(
            uint64_t req_time, uint64_t scale_num, uint64_t scale_den,
            uint32_t *sample_index, uint32_t flags);

    status_t findSyncSampleNear(
            uint32_t start_sample_index, uint32_t *sample_index,
            uint32_t flags);

    status_t findThumbnailSample(uint32_t *sample_index);

    void setPredictSampleSize(uint32_t sampleSize) {
        mDefaultSampleSize = sampleSize;
    }

protected:
    ~SampleTable();

private:
    struct CompositionDeltaLookup;

    static const uint32_t kChunkOffsetType32;
    static const uint32_t kChunkOffsetType64;
    static const uint32_t kSampleSizeType32;
    static const uint32_t kSampleSizeTypeCompact;

    // Limit the total size of all internal tables to 200MiB.
    static const size_t kMaxTotalSize = 200 * (1 << 20);

    DataSourceHelper *mDataSource;
    Mutex mLock;

    off64_t mChunkOffsetOffset;
    uint32_t mChunkOffsetType;
    uint32_t mNumChunkOffsets;

    off64_t mSampleToChunkOffset;
    uint32_t mNumSampleToChunkOffsets;

    off64_t mSampleSizeOffset;
    uint32_t mSampleSizeFieldSize;
    uint32_t mDefaultSampleSize;
    uint32_t mNumSampleSizes;

    bool mHasTimeToSample;
    uint32_t mTimeToSampleCount;
    uint32_t* mTimeToSample;

    struct SampleTimeEntry {
        uint32_t mSampleIndex;
        uint64_t mCompositionTime;
    };
    SampleTimeEntry *mSampleTimeEntries;

    int32_t *mCompositionTimeDeltaEntries;
    size_t mNumCompositionTimeDeltaEntries;
    CompositionDeltaLookup *mCompositionDeltaLookup;

    off64_t mSyncSampleOffset;
    uint32_t mNumSyncSamples;
    uint32_t *mSyncSamples;
    size_t mLastSyncSampleIndex;

    SampleIterator *mSampleIterator;

    struct SampleToChunkEntry {
        uint32_t startChunk;
        uint32_t samplesPerChunk;
        uint32_t chunkDesc;
    };
    SampleToChunkEntry *mSampleToChunkEntries;

    // Approximate size of all tables combined.
    uint64_t mTotalSize;

    friend struct SampleIterator;

    // normally we don't round
    inline uint64_t getSampleTime(
            size_t sample_index, uint64_t scale_num, uint64_t scale_den) const {
        return (sample_index < (size_t)mNumSampleSizes && mSampleTimeEntries != NULL
                && scale_den != 0)
                ? (mSampleTimeEntries[sample_index].mCompositionTime * scale_num) / scale_den : 0;
    }

    status_t getSampleSize_l(uint32_t sample_index, size_t *sample_size);
    int32_t getCompositionTimeOffset(uint32_t sampleIndex);

    static int CompareIncreasingTime(const void *, const void *);

    void buildSampleEntriesTable();

    SampleTable(const SampleTable &);
    SampleTable &operator=(const SampleTable &);
};

}  // namespace android

#endif  // SAMPLE_TABLE_H_
