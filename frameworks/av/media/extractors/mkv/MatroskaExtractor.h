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

#ifndef MATROSKA_EXTRACTOR_H_

#define MATROSKA_EXTRACTOR_H_

#include "mkvparser/mkvparser.h"

#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/NdkMediaFormat.h>
#include <utils/Vector.h>
#include <utils/threads.h>

namespace android {

struct AMessage;
class String8;

class MetaData;
struct DataSourceBaseReader;
struct MatroskaSource;

struct MatroskaExtractor : public MediaExtractorPluginHelper {
    explicit MatroskaExtractor(DataSourceHelper *source);

    virtual size_t countTracks();

    virtual MediaTrackHelper *getTrack(size_t index);

    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);

    virtual uint32_t flags() const;

    virtual const char * name() { return "MatroskaExtractor"; }

protected:
    virtual ~MatroskaExtractor();

private:
    friend struct MatroskaSource;
    friend struct BlockIterator;

    struct TrackInfo {
        TrackInfo() {
            mMeta = NULL;
        }

        ~TrackInfo() {
        }
        unsigned long mTrackNum;
        bool mEncrypted;
        AMediaFormat *mMeta;
        const MatroskaExtractor *mExtractor;
        Vector<const mkvparser::CuePoint*> mCuePoints;

        // mHeader points to memory managed by mkvparser;
        // mHeader would be deleted when mSegment is deleted
        // in ~MatroskaExtractor.
        unsigned char *mHeader;
        size_t mHeaderLen;
        int32_t mNalLengthSize;

        const mkvparser::Track* getTrack() const;
        const mkvparser::CuePoint::TrackPosition *find(long long timeNs) const;
    };

    Mutex mLock;
    Vector<TrackInfo> mTracks;

    DataSourceHelper *mDataSource;
    DataSourceBaseReader *mReader;
    mkvparser::Segment *mSegment;
    bool mExtractedThumbnails;
    bool mIsLiveStreaming;
    bool mIsWebm;
    int64_t mSeekPreRollNs;

    status_t synthesizeAVCC(TrackInfo *trackInfo, size_t index);
    status_t synthesizeMPEG2(TrackInfo *trackInfo, size_t index);
    status_t synthesizeMPEG4(TrackInfo *trackInfo, size_t index);
    status_t initTrackInfo(
            const mkvparser::Track *track,
            AMediaFormat *meta,
            TrackInfo *trackInfo);
    void addTracks();
    void findThumbnails();
    void getColorInformation(
            const mkvparser::VideoTrack *vtrack,
            AMediaFormat *meta);
    bool isLiveStreaming() const;

    MatroskaExtractor(const MatroskaExtractor &);
    MatroskaExtractor &operator=(const MatroskaExtractor &);
};

}  // namespace android

#endif  // MATROSKA_EXTRACTOR_H_
