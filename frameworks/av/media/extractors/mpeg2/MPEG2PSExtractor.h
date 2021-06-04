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

#ifndef MPEG2_PS_EXTRACTOR_H_

#define MPEG2_PS_EXTRACTOR_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/stagefright/MetaDataBase.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>

namespace android {

struct ABuffer;
struct AMessage;
struct Track;
class String8;

struct MPEG2PSExtractor : public MediaExtractorPluginHelper {
    explicit MPEG2PSExtractor(DataSourceHelper *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);

    virtual uint32_t flags() const;
    virtual const char * name() { return "MPEG2PSExtractor"; }

protected:
    virtual ~MPEG2PSExtractor();

private:
    struct Track;
    struct WrappedTrack;

    mutable Mutex mLock;
    DataSourceHelper *mDataSource;

    off64_t mOffset;
    status_t mFinalResult;
    sp<ABuffer> mBuffer;
    KeyedVector<unsigned, Track* > mTracks;
    bool mScanning;

    bool mProgramStreamMapValid;
    KeyedVector<unsigned, unsigned> mStreamTypeByESID;

    status_t feedMore();

    status_t dequeueChunk();
    ssize_t dequeuePack();
    ssize_t dequeueSystemHeader();
    ssize_t dequeuePES();

    DISALLOW_EVIL_CONSTRUCTORS(MPEG2PSExtractor);
};

bool SniffMPEG2PS(DataSourceHelper *source, float *confidence);

}  // namespace android

#endif  // MPEG2_PS_EXTRACTOR_H_

