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

#ifndef ES_QUEUE_H_

#define ES_QUEUE_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AMessage.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <vector>

#include "HlsSampleDecryptor.h"

namespace android {

struct ABuffer;
class MetaData;

struct ElementaryStreamQueue {
    enum Mode {
        INVALID = 0,
        H264,
        AAC,
        AC3,
        EAC3,
        AC4,
        MPEG_AUDIO,
        MPEG_VIDEO,
        MPEG4_VIDEO,
        PCM_AUDIO,
        METADATA,
    };

    enum Flags {
        // Data appended to the queue is always at access unit boundaries.
        kFlag_AlignedData = 1,
        kFlag_ScrambledData = 2,
        kFlag_SampleEncryptedData = 4,
    };
    explicit ElementaryStreamQueue(Mode mode, uint32_t flags = 0);

    status_t appendData(const void *data, size_t size,
            int64_t timeUs, int32_t payloadOffset = 0,
            uint32_t pesScramblingControl = 0);

    void appendScrambledData(
            const void *data, size_t size,
            size_t leadingClearBytes,
            int32_t keyId, bool isSync,
            sp<ABuffer> clearSizes, sp<ABuffer> encSizes);

    void signalEOS();
    void clear(bool clearFormat);

    sp<ABuffer> dequeueAccessUnit();

    sp<MetaData> getFormat();

    bool isScrambled() const;

    void setCasInfo(int32_t systemId, const std::vector<uint8_t> &sessionId);

    void signalNewSampleAesKey(const sp<AMessage> &keyItem);

private:
    struct RangeInfo {
        int64_t mTimestampUs;
        size_t mLength;
        int32_t mPesOffset;
        uint32_t mPesScramblingControl;
    };

    struct ScrambledRangeInfo {
        size_t mLength;
        size_t mLeadingClearBytes;
        int32_t mKeyId;
        int32_t mIsSync;
        sp<ABuffer> mClearSizes;
        sp<ABuffer> mEncSizes;
    };

    Mode mMode;
    uint32_t mFlags;
    bool mEOSReached;

    sp<ABuffer> mBuffer;
    List<RangeInfo> mRangeInfos;

    sp<ABuffer> mScrambledBuffer;
    List<ScrambledRangeInfo> mScrambledRangeInfos;
    int32_t mCASystemId;
    std::vector<uint8_t> mCasSessionId;

    sp<MetaData> mFormat;

    sp<HlsSampleDecryptor> mSampleDecryptor;
    int mAUIndex;

    bool isSampleEncrypted() const {
        return (mFlags & kFlag_SampleEncryptedData) != 0;
    }

    sp<ABuffer> dequeueAccessUnitH264();
    sp<ABuffer> dequeueAccessUnitAAC();
    sp<ABuffer> dequeueAccessUnitEAC3();
    sp<ABuffer> dequeueAccessUnitAC4();
    sp<ABuffer> dequeueAccessUnitMPEGAudio();
    sp<ABuffer> dequeueAccessUnitMPEGVideo();
    sp<ABuffer> dequeueAccessUnitMPEG4Video();
    sp<ABuffer> dequeueAccessUnitPCMAudio();
    sp<ABuffer> dequeueAccessUnitMetadata();

    // consume a logical (compressed) access unit of size "size",
    // returns its timestamp in us (or -1 if no time information).
    int64_t fetchTimestamp(size_t size,
            int32_t *pesOffset = NULL,
            int32_t *pesScramblingControl = NULL);

    sp<ABuffer> dequeueScrambledAccessUnit();

    DISALLOW_EVIL_CONSTRUCTORS(ElementaryStreamQueue);
};

}  // namespace android

#endif  // ES_QUEUE_H_
