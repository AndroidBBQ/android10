/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NU_MEDIA_EXTRACTOR_H_
#define NU_MEDIA_EXTRACTOR_H_

#include <list>
#include <media/mediaplayer.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AudioPresentationInfo.h>
#include <media/IMediaExtractor.h>
#include <media/MediaSource.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/Vector.h>

namespace android {

struct ABuffer;
struct AMessage;
class DataSource;
struct MediaHTTPService;
class MediaBuffer;
class MediaExtractor;
struct MediaSource;
class MetaData;

struct NuMediaExtractor : public RefBase {
    enum SampleFlags {
        SAMPLE_FLAG_SYNC        = 1,
        SAMPLE_FLAG_ENCRYPTED   = 2,
    };

    // identical to IMediaExtractor::GetTrackMetaDataFlags
    enum GetTrackFormatFlags {
        kIncludeExtensiveMetaData = 1, // reads sample table and possibly stream headers
    };

    NuMediaExtractor();

    status_t setDataSource(
            const sp<MediaHTTPService> &httpService,
            const char *path,
            const KeyedVector<String8, String8> *headers = NULL);

    status_t setDataSource(int fd, off64_t offset, off64_t size);

    status_t setDataSource(const sp<DataSource> &datasource);

    status_t setMediaCas(const HInterfaceToken &casToken);

    size_t countTracks() const;
    status_t getTrackFormat(size_t index, sp<AMessage> *format, uint32_t flags = 0) const;

    status_t getFileFormat(sp<AMessage> *format) const;

    status_t getExifOffsetSize(off64_t *offset, size_t *size) const;

    status_t selectTrack(size_t index, int64_t startTimeUs = -1ll,
            MediaSource::ReadOptions::SeekMode mode =
                MediaSource::ReadOptions::SEEK_CLOSEST_SYNC);
    status_t unselectTrack(size_t index);

    status_t seekTo(
            int64_t timeUs,
            MediaSource::ReadOptions::SeekMode mode =
                MediaSource::ReadOptions::SEEK_CLOSEST_SYNC);

    // Each selected track has a read pointer.
    // advance() advances the read pointer with the lowest timestamp.
    status_t advance();
    // readSampleData() reads the sample with the lowest timestamp.
    status_t readSampleData(const sp<ABuffer> &buffer);

    status_t getSampleSize(size_t *sampleSize);
    status_t getSampleTrackIndex(size_t *trackIndex);
    status_t getSampleTime(int64_t *sampleTimeUs);
    status_t getSampleMeta(sp<MetaData> *sampleMeta);
    status_t getMetrics(Parcel *reply);

    bool getCachedDuration(int64_t *durationUs, bool *eos) const;

    status_t getAudioPresentations(size_t trackIdx, AudioPresentationCollection *presentations);

protected:
    virtual ~NuMediaExtractor();

private:
    enum TrackFlags {
        kIsVorbis       = 1,
    };

    enum {
        kMaxTrackCount = 16384,
    };

    struct Sample {
        Sample();
        Sample(MediaBufferBase *buffer, int64_t timeUs);
        MediaBufferBase *mBuffer;
        int64_t mSampleTimeUs;
    };

    struct TrackInfo {
        sp<IMediaSource> mSource;
        size_t mTrackIndex;
        media_track_type mTrackType;
        size_t mMaxFetchCount;
        status_t mFinalResult;
        std::list<Sample> mSamples;

        uint32_t mTrackFlags;  // bitmask of "TrackFlags"
    };

    mutable Mutex mLock;

    sp<DataSource> mDataSource;

    sp<IMediaExtractor> mImpl;
    HInterfaceToken mCasToken;

    Vector<TrackInfo> mSelectedTracks;
    int64_t mTotalBitrate;  // in bits/sec
    int64_t mDurationUs;

    ssize_t fetchAllTrackSamples(
            int64_t seekTimeUs = -1ll,
            MediaSource::ReadOptions::SeekMode mode =
                MediaSource::ReadOptions::SEEK_CLOSEST_SYNC);
    void fetchTrackSamples(
            TrackInfo *info,
            int64_t seekTimeUs = -1ll,
            MediaSource::ReadOptions::SeekMode mode =
                MediaSource::ReadOptions::SEEK_CLOSEST_SYNC);

    void releaseTrackSamples(TrackInfo *info);
    void releaseAllTrackSamples();

    bool getTotalBitrate(int64_t *bitRate) const;
    status_t updateDurationAndBitrate();
    status_t appendVorbisNumPageSamples(MediaBufferBase *mbuf, const sp<ABuffer> &buffer);

    DISALLOW_EVIL_CONSTRUCTORS(NuMediaExtractor);
};

}  // namespace android

#endif  // NU_MEDIA_EXTRACTOR_H_

