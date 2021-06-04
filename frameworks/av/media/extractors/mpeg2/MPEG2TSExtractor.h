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

#ifndef MPEG2_TS_EXTRACTOR_H_

#define MPEG2_TS_EXTRACTOR_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/stagefright/MetaDataBase.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include "mpeg2ts/ATSParser.h"

namespace android {

struct AMessage;
struct AnotherPacketSource;
struct ATSParser;
struct CDataSource;
struct MPEG2TSSource;
class String8;

struct MPEG2TSExtractor : public MediaExtractorPluginHelper {
    explicit MPEG2TSExtractor(DataSourceHelper *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);

    virtual media_status_t setMediaCas(const uint8_t* /*casToken*/, size_t /*size*/) override;

    virtual uint32_t flags() const;
    virtual const char * name() { return "MPEG2TSExtractor"; }

protected:
    virtual ~MPEG2TSExtractor();

private:
    friend struct MPEG2TSSource;

    mutable Mutex mLock;

    DataSourceHelper *mDataSource;

    sp<ATSParser> mParser;

    // Used to remember SyncEvent occurred in feedMore() when called from init(),
    // because init() needs to update |mSourceImpls| before adding SyncPoint.
    ATSParser::SyncEvent mLastSyncEvent;

    Vector<sp<AnotherPacketSource> > mSourceImpls;

    Vector<KeyedVector<int64_t, off64_t> > mSyncPoints;
    // Sync points used for seeking --- normally one for video track is used.
    // If no video track is present, audio track will be used instead.
    KeyedVector<int64_t, off64_t> *mSeekSyncPoints;

    off64_t mOffset;

    static bool isScrambledFormat(MetaDataBase &format);

    void init();
    void addSource(const sp<AnotherPacketSource> &impl);
    // Try to feed more data from source to parser.
    // |isInit| means this function is called inside init(). This is a signal to
    // save SyncEvent so that init() can add SyncPoint after it updates |mSourceImpls|.
    // This function returns OK if expected amount of data is fed from DataSourceHelper to
    // parser and is successfully parsed. Otherwise, various error codes could be
    // returned, e.g., ERROR_END_OF_STREAM, or no data availalbe from DataSourceHelper, or
    // the data has syntax error during parsing, etc.
    status_t feedMore(bool isInit = false);
    status_t seek(int64_t seekTimeUs,
            const MediaTrackHelper::ReadOptions::SeekMode& seekMode);
    status_t queueDiscontinuityForSeek(int64_t actualSeekTimeUs);
    status_t seekBeyond(int64_t seekTimeUs);

    status_t feedUntilBufferAvailable(const sp<AnotherPacketSource> &impl);
    status_t findIndexOfSource(const sp<AnotherPacketSource> &impl, size_t *index);

    // Add a SynPoint derived from |event|.
    void addSyncPoint_l(const ATSParser::SyncEvent &event);

    status_t  estimateDurationsFromTimesUsAtEnd();

    size_t mHeaderSkip;
    DISALLOW_EVIL_CONSTRUCTORS(MPEG2TSExtractor);
};

bool SniffMPEG2TS(DataSourceHelper *source, float *confidence);

}  // namespace android

#endif  // MPEG2_TS_EXTRACTOR_H_
