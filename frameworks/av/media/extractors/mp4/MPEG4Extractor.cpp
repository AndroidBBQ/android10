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

//#define LOG_NDEBUG 0
#define LOG_TAG "MPEG4Extractor"

#include <ctype.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>
#include <utils/Log.h>

#include "AC4Parser.h"
#include "MPEG4Extractor.h"
#include "SampleTable.h"
#include "ItemTable.h"
#include "include/ESDS.h"

#include <media/DataSourceBase.h>
#include <media/ExtractorUtils.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AudioPresentationInfo.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/OpusHeader.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaDataBase.h>
#include <utils/String8.h>

#include <byteswap.h>
#include "include/ID3.h"

#ifndef UINT32_MAX
#define UINT32_MAX       (4294967295U)
#endif

#define ALAC_SPECIFIC_INFO_SIZE (36)

namespace android {

enum {
    // max track header chunk to return
    kMaxTrackHeaderSize = 32,

    // maximum size of an atom. Some atoms can be bigger according to the spec,
    // but we only allow up to this size.
    kMaxAtomSize = 64 * 1024 * 1024,
};

class MPEG4Source : public MediaTrackHelper {
static const size_t  kMaxPcmFrameSize = 8192;
public:
    // Caller retains ownership of both "dataSource" and "sampleTable".
    MPEG4Source(AMediaFormat *format,
                DataSourceHelper *dataSource,
                int32_t timeScale,
                const sp<SampleTable> &sampleTable,
                Vector<SidxEntry> &sidx,
                const Trex *trex,
                off64_t firstMoofOffset,
                const sp<ItemTable> &itemTable,
                uint64_t elstShiftStartTicks);
    virtual status_t init();

    virtual media_status_t start();
    virtual media_status_t stop();

    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(MediaBufferHelper **buffer, const ReadOptions *options = NULL);
    bool supportsNonBlockingRead() override { return true; }
    virtual media_status_t fragmentedRead(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

    virtual ~MPEG4Source();

private:
    Mutex mLock;

    AMediaFormat *mFormat;
    DataSourceHelper *mDataSource;
    int32_t mTimescale;
    sp<SampleTable> mSampleTable;
    uint32_t mCurrentSampleIndex;
    uint32_t mCurrentFragmentIndex;
    Vector<SidxEntry> &mSegments;
    const Trex *mTrex;
    off64_t mFirstMoofOffset;
    off64_t mCurrentMoofOffset;
    off64_t mNextMoofOffset;
    uint32_t mCurrentTime; // in media timescale ticks
    int32_t mLastParsedTrackId;
    int32_t mTrackId;

    int32_t mCryptoMode;    // passed in from extractor
    int32_t mDefaultIVSize; // passed in from extractor
    uint8_t mCryptoKey[16]; // passed in from extractor
    int32_t mDefaultEncryptedByteBlock;
    int32_t mDefaultSkipByteBlock;
    uint32_t mCurrentAuxInfoType;
    uint32_t mCurrentAuxInfoTypeParameter;
    int32_t mCurrentDefaultSampleInfoSize;
    uint32_t mCurrentSampleInfoCount;
    uint32_t mCurrentSampleInfoAllocSize;
    uint8_t* mCurrentSampleInfoSizes;
    uint32_t mCurrentSampleInfoOffsetCount;
    uint32_t mCurrentSampleInfoOffsetsAllocSize;
    uint64_t* mCurrentSampleInfoOffsets;

    bool mIsAVC;
    bool mIsHEVC;
    bool mIsAC4;
    bool mIsPcm;
    size_t mNALLengthSize;

    bool mStarted;

    MediaBufferHelper *mBuffer;

    uint8_t *mSrcBuffer;

    bool mIsHeif;
    bool mIsAudio;
    sp<ItemTable> mItemTable;

    // Start offset from composition time to presentation time.
    // Support shift only for video tracks through mElstShiftStartTicks for now.
    uint64_t mElstShiftStartTicks;

    size_t parseNALSize(const uint8_t *data) const;
    status_t parseChunk(off64_t *offset);
    status_t parseTrackFragmentHeader(off64_t offset, off64_t size);
    status_t parseTrackFragmentRun(off64_t offset, off64_t size);
    status_t parseSampleAuxiliaryInformationSizes(off64_t offset, off64_t size);
    status_t parseSampleAuxiliaryInformationOffsets(off64_t offset, off64_t size);
    status_t parseClearEncryptedSizes(off64_t offset, bool isSubsampleEncryption, uint32_t flags);
    status_t parseSampleEncryption(off64_t offset);
    // returns -1 for invalid layer ID
    int32_t parseHEVCLayerId(const uint8_t *data, size_t size);

    struct TrackFragmentHeaderInfo {
        enum Flags {
            kBaseDataOffsetPresent         = 0x01,
            kSampleDescriptionIndexPresent = 0x02,
            kDefaultSampleDurationPresent  = 0x08,
            kDefaultSampleSizePresent      = 0x10,
            kDefaultSampleFlagsPresent     = 0x20,
            kDurationIsEmpty               = 0x10000,
        };

        uint32_t mTrackID;
        uint32_t mFlags;
        uint64_t mBaseDataOffset;
        uint32_t mSampleDescriptionIndex;
        uint32_t mDefaultSampleDuration;
        uint32_t mDefaultSampleSize;
        uint32_t mDefaultSampleFlags;

        uint64_t mDataOffset;
    };
    TrackFragmentHeaderInfo mTrackFragmentHeaderInfo;

    struct Sample {
        off64_t offset;
        size_t size;
        uint32_t duration;
        int32_t compositionOffset;
        uint8_t iv[16];
        Vector<size_t> clearsizes;
        Vector<size_t> encryptedsizes;
    };
    Vector<Sample> mCurrentSamples;

    MPEG4Source(const MPEG4Source &);
    MPEG4Source &operator=(const MPEG4Source &);
};

// This custom data source wraps an existing one and satisfies requests
// falling entirely within a cached range from the cache while forwarding
// all remaining requests to the wrapped datasource.
// This is used to cache the full sampletable metadata for a single track,
// possibly wrapping multiple times to cover all tracks, i.e.
// Each CachedRangedDataSource caches the sampletable metadata for a single track.

class CachedRangedDataSource : public DataSourceHelper {
public:
    explicit CachedRangedDataSource(DataSourceHelper *source);
    virtual ~CachedRangedDataSource();

    ssize_t readAt(off64_t offset, void *data, size_t size) override;
    status_t getSize(off64_t *size) override;
    uint32_t flags() override;

    status_t setCachedRange(off64_t offset, size_t size, bool assumeSourceOwnershipOnSuccess);


private:
    Mutex mLock;

    DataSourceHelper *mSource;
    bool mOwnsDataSource;
    off64_t mCachedOffset;
    size_t mCachedSize;
    uint8_t *mCache;

    void clearCache();

    CachedRangedDataSource(const CachedRangedDataSource &);
    CachedRangedDataSource &operator=(const CachedRangedDataSource &);
};

CachedRangedDataSource::CachedRangedDataSource(DataSourceHelper *source)
    : DataSourceHelper(source),
      mSource(source),
      mOwnsDataSource(false),
      mCachedOffset(0),
      mCachedSize(0),
      mCache(NULL) {
}

CachedRangedDataSource::~CachedRangedDataSource() {
    clearCache();
    if (mOwnsDataSource) {
        delete mSource;
    }
}

void CachedRangedDataSource::clearCache() {
    if (mCache) {
        free(mCache);
        mCache = NULL;
    }

    mCachedOffset = 0;
    mCachedSize = 0;
}

ssize_t CachedRangedDataSource::readAt(off64_t offset, void *data, size_t size) {
    Mutex::Autolock autoLock(mLock);

    if (isInRange(mCachedOffset, mCachedSize, offset, size)) {
        memcpy(data, &mCache[offset - mCachedOffset], size);
        return size;
    }

    return mSource->readAt(offset, data, size);
}

status_t CachedRangedDataSource::getSize(off64_t *size) {
    return mSource->getSize(size);
}

uint32_t CachedRangedDataSource::flags() {
    return mSource->flags();
}

status_t CachedRangedDataSource::setCachedRange(off64_t offset,
        size_t size,
        bool assumeSourceOwnershipOnSuccess) {
    Mutex::Autolock autoLock(mLock);

    clearCache();

    mCache = (uint8_t *)malloc(size);

    if (mCache == NULL) {
        return -ENOMEM;
    }

    mCachedOffset = offset;
    mCachedSize = size;

    ssize_t err = mSource->readAt(mCachedOffset, mCache, mCachedSize);

    if (err < (ssize_t)size) {
        clearCache();

        return ERROR_IO;
    }
    mOwnsDataSource = assumeSourceOwnershipOnSuccess;
    return OK;
}

////////////////////////////////////////////////////////////////////////////////

static const bool kUseHexDump = false;

static const char *FourCC2MIME(uint32_t fourcc) {
    switch (fourcc) {
        case FOURCC("mp4a"):
            return MEDIA_MIMETYPE_AUDIO_AAC;

        case FOURCC("samr"):
            return MEDIA_MIMETYPE_AUDIO_AMR_NB;

        case FOURCC("sawb"):
            return MEDIA_MIMETYPE_AUDIO_AMR_WB;

        case FOURCC("ec-3"):
            return MEDIA_MIMETYPE_AUDIO_EAC3;

        case FOURCC("mp4v"):
            return MEDIA_MIMETYPE_VIDEO_MPEG4;

        case FOURCC("s263"):
        case FOURCC("h263"):
        case FOURCC("H263"):
            return MEDIA_MIMETYPE_VIDEO_H263;

        case FOURCC("avc1"):
            return MEDIA_MIMETYPE_VIDEO_AVC;

        case FOURCC("hvc1"):
        case FOURCC("hev1"):
            return MEDIA_MIMETYPE_VIDEO_HEVC;
        case FOURCC("ac-4"):
            return MEDIA_MIMETYPE_AUDIO_AC4;
        case FOURCC("Opus"):
            return MEDIA_MIMETYPE_AUDIO_OPUS;

        case FOURCC("twos"):
        case FOURCC("sowt"):
            return MEDIA_MIMETYPE_AUDIO_RAW;
        case FOURCC("alac"):
            return MEDIA_MIMETYPE_AUDIO_ALAC;
        case FOURCC("fLaC"):
            return MEDIA_MIMETYPE_AUDIO_FLAC;
        case FOURCC("av01"):
            return MEDIA_MIMETYPE_VIDEO_AV1;
        case FOURCC(".mp3"):
        case 0x6D730055: // "ms U" mp3 audio
            return MEDIA_MIMETYPE_AUDIO_MPEG;
        default:
            ALOGW("Unknown fourcc: %c%c%c%c",
                   (fourcc >> 24) & 0xff,
                   (fourcc >> 16) & 0xff,
                   (fourcc >> 8) & 0xff,
                   fourcc & 0xff
                   );
            return "application/octet-stream";
    }
}

static bool AdjustChannelsAndRate(uint32_t fourcc, uint32_t *channels, uint32_t *rate) {
    if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_NB, FourCC2MIME(fourcc))) {
        // AMR NB audio is always mono, 8kHz
        *channels = 1;
        *rate = 8000;
        return true;
    } else if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_WB, FourCC2MIME(fourcc))) {
        // AMR WB audio is always mono, 16kHz
        *channels = 1;
        *rate = 16000;
        return true;
    }
    return false;
}

MPEG4Extractor::MPEG4Extractor(DataSourceHelper *source, const char *mime)
    : mMoofOffset(0),
      mMoofFound(false),
      mMdatFound(false),
      mDataSource(source),
      mInitCheck(NO_INIT),
      mHeaderTimescale(0),
      mIsQT(false),
      mIsHeif(false),
      mHasMoovBox(false),
      mPreferHeif(mime != NULL && !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_HEIF)),
      mFirstTrack(NULL),
      mLastTrack(NULL) {
    ALOGV("mime=%s, mPreferHeif=%d", mime, mPreferHeif);
    mFileMetaData = AMediaFormat_new();
}

MPEG4Extractor::~MPEG4Extractor() {
    Track *track = mFirstTrack;
    while (track) {
        Track *next = track->next;

        delete track;
        track = next;
    }
    mFirstTrack = mLastTrack = NULL;

    for (size_t i = 0; i < mPssh.size(); i++) {
        delete [] mPssh[i].data;
    }
    mPssh.clear();

    delete mDataSource;
    AMediaFormat_delete(mFileMetaData);
}

uint32_t MPEG4Extractor::flags() const {
    return CAN_PAUSE |
            ((mMoofOffset == 0 || mSidxEntries.size() != 0) ?
                    (CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK) : 0);
}

media_status_t MPEG4Extractor::getMetaData(AMediaFormat *meta) {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    AMediaFormat_copy(meta, mFileMetaData);
    return AMEDIA_OK;
}

size_t MPEG4Extractor::countTracks() {
    status_t err;
    if ((err = readMetaData()) != OK) {
        ALOGV("MPEG4Extractor::countTracks: no tracks");
        return 0;
    }

    size_t n = 0;
    Track *track = mFirstTrack;
    while (track) {
        ++n;
        track = track->next;
    }

    ALOGV("MPEG4Extractor::countTracks: %zu tracks", n);
    return n;
}

media_status_t MPEG4Extractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t flags) {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL) {
            return AMEDIA_ERROR_UNKNOWN;
        }

        track = track->next;
        --index;
    }

    if (track == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    [=] {
        int64_t duration;
        int32_t samplerate;
        // Only for audio track.
        if (track->has_elst && mHeaderTimescale != 0 &&
                AMediaFormat_getInt64(track->meta, AMEDIAFORMAT_KEY_DURATION, &duration) &&
                AMediaFormat_getInt32(track->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, &samplerate)) {

            // Elst has to be processed only the first time this function is called.
            track->has_elst = false;

            if (track->elst_segment_duration > INT64_MAX) {
                return;
            }
            int64_t segment_duration = track->elst_segment_duration;
            int64_t media_time = track->elst_media_time;
            int64_t halfscale = track->timescale / 2;

            ALOGV("segment_duration = %" PRId64 ", media_time = %" PRId64
                  ", halfscale = %" PRId64 ", mdhd_timescale = %d, track_timescale = %u",
                  segment_duration, media_time,
                  halfscale, mHeaderTimescale, track->timescale);

            if ((uint32_t)samplerate != track->timescale){
                ALOGV("samplerate:%" PRId32 ", track->timescale and samplerate are different!",
                    samplerate);
            }
            // Both delay and paddingsamples have to be set inorder for either to be
            // effective in the lower layers.
            int64_t delay = 0;
            if (media_time > 0) { // Gapless playback
                // delay = ((media_time * samplerate) + halfscale) / track->timescale;
                if (__builtin_mul_overflow(media_time, samplerate, &delay) ||
                        __builtin_add_overflow(delay, halfscale, &delay) ||
                        (delay /= track->timescale, false) ||
                        delay > INT32_MAX ||
                        delay < INT32_MIN) {
                    ALOGW("ignoring edit list with bogus values");
                    return;
                }
            }
            ALOGV("delay = %" PRId64, delay);
            AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_ENCODER_DELAY, delay);

            int64_t paddingsamples = 0;
            if (segment_duration > 0) {
                int64_t scaled_duration;
                // scaled_duration = duration * mHeaderTimescale;
                if (__builtin_mul_overflow(duration, mHeaderTimescale, &scaled_duration)) {
                    return;
                }
                ALOGV("scaled_duration = %" PRId64, scaled_duration);

                int64_t segment_end;
                int64_t padding;
                int64_t segment_duration_e6;
                int64_t media_time_scaled_e6;
                int64_t media_time_scaled;
                // padding = scaled_duration - ((segment_duration * 1000000) +
                //                  ((media_time * mHeaderTimescale * 1000000)/track->timescale) )
                // segment_duration is based on timescale in movie header box(mdhd)
                // media_time is based on timescale track header/media timescale
                if (__builtin_mul_overflow(segment_duration, 1000000, &segment_duration_e6) ||
                    __builtin_mul_overflow(media_time, mHeaderTimescale, &media_time_scaled) ||
                    __builtin_mul_overflow(media_time_scaled, 1000000, &media_time_scaled_e6)) {
                    return;
                }
                media_time_scaled_e6 /= track->timescale;
                if (__builtin_add_overflow(segment_duration_e6, media_time_scaled_e6, &segment_end)
                    || __builtin_sub_overflow(scaled_duration, segment_end, &padding)) {
                    return;
                }
                ALOGV("segment_end = %" PRId64 ", padding = %" PRId64, segment_end, padding);
                // track duration from media header (which is what AMEDIAFORMAT_KEY_DURATION is)
                // might be slightly shorter than the segment duration, which would make the
                // padding negative. Clamp to zero.
                if (padding > 0) {
                    int64_t halfscale_mht = mHeaderTimescale / 2;
                    int64_t halfscale_e6;
                    int64_t timescale_e6;
                    // paddingsamples = ((padding * samplerate) + (halfscale_mht * 1000000))
                    //                / (mHeaderTimescale * 1000000);
                    if (__builtin_mul_overflow(padding, samplerate, &paddingsamples) ||
                            __builtin_mul_overflow(halfscale_mht, 1000000, &halfscale_e6) ||
                            __builtin_mul_overflow(mHeaderTimescale, 1000000, &timescale_e6) ||
                            __builtin_add_overflow(paddingsamples, halfscale_e6, &paddingsamples) ||
                            (paddingsamples /= timescale_e6, false) ||
                            paddingsamples > INT32_MAX) {
                        return;
                    }
                }
            }
            ALOGV("paddingsamples = %" PRId64, paddingsamples);
            AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_ENCODER_PADDING, paddingsamples);
        }
    }();

    if ((flags & kIncludeExtensiveMetaData)
            && !track->includes_expensive_metadata) {
        track->includes_expensive_metadata = true;

        const char *mime;
        CHECK(AMediaFormat_getString(track->meta, AMEDIAFORMAT_KEY_MIME, &mime));
        if (!strncasecmp("video/", mime, 6)) {
            // MPEG2 tracks do not provide CSD, so read the stream header
            if (!strcmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG2)) {
                off64_t offset;
                size_t size;
                if (track->sampleTable->getMetaDataForSample(
                            0 /* sampleIndex */, &offset, &size, NULL /* sampleTime */) == OK) {
                    if (size > kMaxTrackHeaderSize) {
                        size = kMaxTrackHeaderSize;
                    }
                    uint8_t header[kMaxTrackHeaderSize];
                    if (mDataSource->readAt(offset, &header, size) == (ssize_t)size) {
                        AMediaFormat_setBuffer(track->meta,
                                AMEDIAFORMAT_KEY_MPEG2_STREAM_HEADER, header, size);
                    }
                }
            }

            if (mMoofOffset > 0) {
                int64_t duration;
                if (AMediaFormat_getInt64(track->meta,
                        AMEDIAFORMAT_KEY_DURATION, &duration)) {
                    // nothing fancy, just pick a frame near 1/4th of the duration
                    AMediaFormat_setInt64(track->meta,
                            AMEDIAFORMAT_KEY_THUMBNAIL_TIME, duration / 4);
                }
            } else {
                uint32_t sampleIndex;
                uint64_t sampleTime;
                if (track->timescale != 0 &&
                        track->sampleTable->findThumbnailSample(&sampleIndex) == OK
                        && track->sampleTable->getMetaDataForSample(
                            sampleIndex, NULL /* offset */, NULL /* size */,
                            &sampleTime) == OK) {
                        AMediaFormat_setInt64(track->meta,
                                AMEDIAFORMAT_KEY_THUMBNAIL_TIME,
                                ((int64_t)sampleTime * 1000000) / track->timescale);
                }
            }
        }
    }

    AMediaFormat_copy(meta, track->meta);
    return AMEDIA_OK;
}

status_t MPEG4Extractor::readMetaData() {
    if (mInitCheck != NO_INIT) {
        return mInitCheck;
    }

    off64_t offset = 0;
    status_t err;
    bool sawMoovOrSidx = false;

    while (!((mHasMoovBox && sawMoovOrSidx && (mMdatFound || mMoofFound)) ||
             (mIsHeif && (mPreferHeif || !mHasMoovBox) &&
                     (mItemTable != NULL) && mItemTable->isValid()))) {
        off64_t orig_offset = offset;
        err = parseChunk(&offset, 0);

        if (err != OK && err != UNKNOWN_ERROR) {
            break;
        } else if (offset <= orig_offset) {
            // only continue parsing if the offset was advanced,
            // otherwise we might end up in an infinite loop
            ALOGE("did not advance: %lld->%lld", (long long)orig_offset, (long long)offset);
            err = ERROR_MALFORMED;
            break;
        } else if (err == UNKNOWN_ERROR) {
            sawMoovOrSidx = true;
        }
    }

    if (mIsHeif && (mItemTable != NULL) && (mItemTable->countImages() > 0)) {
        off64_t exifOffset;
        size_t exifSize;
        if (mItemTable->getExifOffsetAndSize(&exifOffset, &exifSize) == OK) {
            AMediaFormat_setInt64(mFileMetaData,
                    AMEDIAFORMAT_KEY_EXIF_OFFSET, (int64_t)exifOffset);
            AMediaFormat_setInt64(mFileMetaData,
                    AMEDIAFORMAT_KEY_EXIF_SIZE, (int64_t)exifSize);
        }
        for (uint32_t imageIndex = 0;
                imageIndex < mItemTable->countImages(); imageIndex++) {
            AMediaFormat *meta = mItemTable->getImageMeta(imageIndex);
            if (meta == NULL) {
                ALOGE("heif image %u has no meta!", imageIndex);
                continue;
            }
            // Some heif files advertise image sequence brands (eg. 'hevc') in
            // ftyp box, but don't have any valid tracks in them. Instead of
            // reporting the entire file as malformed, we override the error
            // to allow still images to be extracted.
            if (err != OK) {
                ALOGW("Extracting still images only");
                err = OK;
            }
            mInitCheck = OK;

            ALOGV("adding HEIF image track %u", imageIndex);
            Track *track = new Track;
            if (mLastTrack != NULL) {
                mLastTrack->next = track;
            } else {
                mFirstTrack = track;
            }
            mLastTrack = track;

            track->meta = meta;
            AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_TRACK_ID, imageIndex);
            track->timescale = 1000000;
        }
    }

    if (mInitCheck == OK) {
        if (findTrackByMimePrefix("video/") != NULL) {
            AMediaFormat_setString(mFileMetaData,
                    AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_MPEG4);
        } else if (findTrackByMimePrefix("audio/") != NULL) {
            AMediaFormat_setString(mFileMetaData,
                    AMEDIAFORMAT_KEY_MIME, "audio/mp4");
        } else if (findTrackByMimePrefix(
                MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC) != NULL) {
            AMediaFormat_setString(mFileMetaData,
                    AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_HEIF);
        } else {
            AMediaFormat_setString(mFileMetaData,
                    AMEDIAFORMAT_KEY_MIME, "application/octet-stream");
        }
    } else {
        mInitCheck = err;
    }

    CHECK_NE(err, (status_t)NO_INIT);

    // copy pssh data into file metadata
    uint64_t psshsize = 0;
    for (size_t i = 0; i < mPssh.size(); i++) {
        psshsize += 20 + mPssh[i].datalen;
    }
    if (psshsize > 0 && psshsize <= UINT32_MAX) {
        char *buf = (char*)malloc(psshsize);
        if (!buf) {
            ALOGE("b/28471206");
            return NO_MEMORY;
        }
        char *ptr = buf;
        for (size_t i = 0; i < mPssh.size(); i++) {
            memcpy(ptr, mPssh[i].uuid, 20); // uuid + length
            memcpy(ptr + 20, mPssh[i].data, mPssh[i].datalen);
            ptr += (20 + mPssh[i].datalen);
        }
        AMediaFormat_setBuffer(mFileMetaData, AMEDIAFORMAT_KEY_PSSH, buf, psshsize);
        free(buf);
    }

    return mInitCheck;
}

struct PathAdder {
    PathAdder(Vector<uint32_t> *path, uint32_t chunkType)
        : mPath(path) {
        mPath->push(chunkType);
    }

    ~PathAdder() {
        mPath->pop();
    }

private:
    Vector<uint32_t> *mPath;

    PathAdder(const PathAdder &);
    PathAdder &operator=(const PathAdder &);
};

static bool underMetaDataPath(const Vector<uint32_t> &path) {
    return path.size() >= 5
        && path[0] == FOURCC("moov")
        && path[1] == FOURCC("udta")
        && path[2] == FOURCC("meta")
        && path[3] == FOURCC("ilst");
}

static bool underQTMetaPath(const Vector<uint32_t> &path, int32_t depth) {
    return path.size() >= 2
            && path[0] == FOURCC("moov")
            && path[1] == FOURCC("meta")
            && (depth == 2
            || (depth == 3
                    && (path[2] == FOURCC("hdlr")
                    ||  path[2] == FOURCC("ilst")
                    ||  path[2] == FOURCC("keys"))));
}

// Given a time in seconds since Jan 1 1904, produce a human-readable string.
static bool convertTimeToDate(int64_t time_1904, String8 *s) {
    // delta between mpeg4 time and unix epoch time
    static const int64_t delta = (((66 * 365 + 17) * 24) * 3600);
    if (time_1904 < INT64_MIN + delta) {
        return false;
    }
    time_t time_1970 = time_1904 - delta;

    char tmp[32];
    struct tm* tm = gmtime(&time_1970);
    if (tm != NULL &&
            strftime(tmp, sizeof(tmp), "%Y%m%dT%H%M%S.000Z", tm) > 0) {
        s->setTo(tmp);
        return true;
    }
    return false;
}

status_t MPEG4Extractor::parseChunk(off64_t *offset, int depth) {
    ALOGV("entering parseChunk %lld/%d", (long long)*offset, depth);

    if (*offset < 0) {
        ALOGE("b/23540914");
        return ERROR_MALFORMED;
    }
    if (depth > 100) {
        ALOGE("b/27456299");
        return ERROR_MALFORMED;
    }
    uint32_t hdr[2];
    if (mDataSource->readAt(*offset, hdr, 8) < 8) {
        return ERROR_IO;
    }
    uint64_t chunk_size = ntohl(hdr[0]);
    int32_t chunk_type = ntohl(hdr[1]);
    off64_t data_offset = *offset + 8;

    if (chunk_size == 1) {
        if (mDataSource->readAt(*offset + 8, &chunk_size, 8) < 8) {
            return ERROR_IO;
        }
        chunk_size = ntoh64(chunk_size);
        data_offset += 8;

        if (chunk_size < 16) {
            // The smallest valid chunk is 16 bytes long in this case.
            return ERROR_MALFORMED;
        }
    } else if (chunk_size == 0) {
        if (depth == 0) {
            // atom extends to end of file
            off64_t sourceSize;
            if (mDataSource->getSize(&sourceSize) == OK) {
                chunk_size = (sourceSize - *offset);
            } else {
                // XXX could we just pick a "sufficiently large" value here?
                ALOGE("atom size is 0, and data source has no size");
                return ERROR_MALFORMED;
            }
        } else {
            // not allowed for non-toplevel atoms, skip it
            *offset += 4;
            return OK;
        }
    } else if (chunk_size < 8) {
        // The smallest valid chunk is 8 bytes long.
        ALOGE("invalid chunk size: %" PRIu64, chunk_size);
        return ERROR_MALFORMED;
    }

    char chunk[5];
    MakeFourCCString(chunk_type, chunk);
    ALOGV("chunk: %s @ %lld, %d", chunk, (long long)*offset, depth);

    if (kUseHexDump) {
        static const char kWhitespace[] = "                                        ";
        const char *indent = &kWhitespace[sizeof(kWhitespace) - 1 - 2 * depth];
        printf("%sfound chunk '%s' of size %" PRIu64 "\n", indent, chunk, chunk_size);

        char buffer[256];
        size_t n = chunk_size;
        if (n > sizeof(buffer)) {
            n = sizeof(buffer);
        }
        if (mDataSource->readAt(*offset, buffer, n)
                < (ssize_t)n) {
            return ERROR_IO;
        }

        hexdump(buffer, n);
    }

    PathAdder autoAdder(&mPath, chunk_type);

    // (data_offset - *offset) is either 8 or 16
    off64_t chunk_data_size = chunk_size - (data_offset - *offset);
    if (chunk_data_size < 0) {
        ALOGE("b/23540914");
        return ERROR_MALFORMED;
    }
    if (chunk_type != FOURCC("mdat") && chunk_data_size > kMaxAtomSize) {
        char errMsg[100];
        sprintf(errMsg, "%s atom has size %" PRId64, chunk, chunk_data_size);
        ALOGE("%s (b/28615448)", errMsg);
        android_errorWriteWithInfoLog(0x534e4554, "28615448", -1, errMsg, strlen(errMsg));
        return ERROR_MALFORMED;
    }

    if (chunk_type != FOURCC("cprt")
            && chunk_type != FOURCC("covr")
            && mPath.size() == 5 && underMetaDataPath(mPath)) {
        off64_t stop_offset = *offset + chunk_size;
        *offset = data_offset;
        while (*offset < stop_offset) {
            status_t err = parseChunk(offset, depth + 1);
            if (err != OK) {
                return err;
            }
        }

        if (*offset != stop_offset) {
            return ERROR_MALFORMED;
        }

        return OK;
    }

    switch(chunk_type) {
        case FOURCC("moov"):
        case FOURCC("trak"):
        case FOURCC("mdia"):
        case FOURCC("minf"):
        case FOURCC("dinf"):
        case FOURCC("stbl"):
        case FOURCC("mvex"):
        case FOURCC("moof"):
        case FOURCC("traf"):
        case FOURCC("mfra"):
        case FOURCC("udta"):
        case FOURCC("ilst"):
        case FOURCC("sinf"):
        case FOURCC("schi"):
        case FOURCC("edts"):
        case FOURCC("wave"):
        {
            if (chunk_type == FOURCC("moov") && depth != 0) {
                ALOGE("moov: depth %d", depth);
                return ERROR_MALFORMED;
            }

            if (chunk_type == FOURCC("moov") && mInitCheck == OK) {
                ALOGE("duplicate moov");
                return ERROR_MALFORMED;
            }

            if (chunk_type == FOURCC("moof") && !mMoofFound) {
                // store the offset of the first segment
                mMoofFound = true;
                mMoofOffset = *offset;
            }

            if (chunk_type == FOURCC("stbl")) {
                ALOGV("sampleTable chunk is %" PRIu64 " bytes long.", chunk_size);

                if (mDataSource->flags()
                        & (DataSourceBase::kWantsPrefetching
                            | DataSourceBase::kIsCachingDataSource)) {
                    CachedRangedDataSource *cachedSource =
                        new CachedRangedDataSource(mDataSource);

                    if (cachedSource->setCachedRange(
                            *offset, chunk_size,
                            true /* assume ownership on success */) == OK) {
                        mDataSource = cachedSource;
                    } else {
                        delete cachedSource;
                    }
                }

                if (mLastTrack == NULL) {
                    return ERROR_MALFORMED;
                }

                mLastTrack->sampleTable = new SampleTable(mDataSource);
            }

            bool isTrack = false;
            if (chunk_type == FOURCC("trak")) {
                if (depth != 1) {
                    ALOGE("trak: depth %d", depth);
                    return ERROR_MALFORMED;
                }
                isTrack = true;

                ALOGV("adding new track");
                Track *track = new Track;
                if (mLastTrack) {
                    mLastTrack->next = track;
                } else {
                    mFirstTrack = track;
                }
                mLastTrack = track;

                track->meta = AMediaFormat_new();
                AMediaFormat_setString(track->meta,
                        AMEDIAFORMAT_KEY_MIME, "application/octet-stream");
            }

            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset;
            while (*offset < stop_offset) {

                // pass udata terminate
                if (mIsQT && stop_offset - *offset == 4 && chunk_type == FOURCC("udta")) {
                    // handle the case that udta terminates with terminate code x00000000
                    // note that 0 terminator is optional and we just handle this case.
                    uint32_t terminate_code = 1;
                    mDataSource->readAt(*offset, &terminate_code, 4);
                    if (0 == terminate_code) {
                        *offset += 4;
                        ALOGD("Terminal code for udta");
                        continue;
                    } else {
                        ALOGW("invalid udta Terminal code");
                    }
                }

                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    if (isTrack) {
                        mLastTrack->skipTrack = true;
                        break;
                    }
                    return err;
                }
            }

            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }

            if (isTrack) {
                int32_t trackId;
                // There must be exactly one track header per track.

                if (!AMediaFormat_getInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_TRACK_ID, &trackId)) {
                    mLastTrack->skipTrack = true;
                }

                status_t err = verifyTrack(mLastTrack);
                if (err != OK) {
                    mLastTrack->skipTrack = true;
                }


                if (mLastTrack->skipTrack) {
                    ALOGV("skipping this track...");
                    Track *cur = mFirstTrack;

                    if (cur == mLastTrack) {
                        delete cur;
                        mFirstTrack = mLastTrack = NULL;
                    } else {
                        while (cur && cur->next != mLastTrack) {
                            cur = cur->next;
                        }
                        if (cur) {
                            cur->next = NULL;
                        }
                        delete mLastTrack;
                        mLastTrack = cur;
                    }

                    return OK;
                }

                // place things we built elsewhere into their final locations

                // put aggregated tx3g data into the metadata
                if (mLastTrack->mTx3gFilled > 0) {
                    ALOGV("Putting %zu bytes of tx3g data into meta data",
                          mLastTrack->mTx3gFilled);
                    AMediaFormat_setBuffer(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_TEXT_FORMAT_DATA,
                        mLastTrack->mTx3gBuffer, mLastTrack->mTx3gFilled);
                    // drop it now to reduce our footprint
                    free(mLastTrack->mTx3gBuffer);
                    mLastTrack->mTx3gBuffer = NULL;
                    mLastTrack->mTx3gFilled = 0;
                    mLastTrack->mTx3gSize = 0;
                }

            } else if (chunk_type == FOURCC("moov")) {
                mInitCheck = OK;

                return UNKNOWN_ERROR;  // Return a dummy error.
            }
            break;
        }

        case FOURCC("schm"):
        {

            *offset += chunk_size;
            if (!mLastTrack) {
                return ERROR_MALFORMED;
            }

            uint32_t scheme_type;
            if (mDataSource->readAt(data_offset + 4, &scheme_type, 4) < 4) {
                return ERROR_IO;
            }
            scheme_type = ntohl(scheme_type);
            int32_t mode = kCryptoModeUnencrypted;
            switch(scheme_type) {
                case FOURCC("cbc1"):
                {
                    mode = kCryptoModeAesCbc;
                    break;
                }
                case FOURCC("cbcs"):
                {
                    mode = kCryptoModeAesCbc;
                    mLastTrack->subsample_encryption = true;
                    break;
                }
                case FOURCC("cenc"):
                {
                    mode = kCryptoModeAesCtr;
                    break;
                }
                case FOURCC("cens"):
                {
                    mode = kCryptoModeAesCtr;
                    mLastTrack->subsample_encryption = true;
                    break;
                }
            }
            if (mode != kCryptoModeUnencrypted) {
                AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CRYPTO_MODE, mode);
            }
            break;
        }


        case FOURCC("elst"):
        {
            *offset += chunk_size;

            if (!mLastTrack) {
                return ERROR_MALFORMED;
            }

            // See 14496-12 8.6.6
            uint8_t version;
            if (mDataSource->readAt(data_offset, &version, 1) < 1) {
                return ERROR_IO;
            }

            uint32_t entry_count;
            if (!mDataSource->getUInt32(data_offset + 4, &entry_count)) {
                return ERROR_IO;
            }

            if (entry_count != 1) {
                // we only support a single entry at the moment, for gapless playback
                // or start offset
                ALOGW("ignoring edit list with %d entries", entry_count);
            } else {
                off64_t entriesoffset = data_offset + 8;
                uint64_t segment_duration;
                int64_t media_time;

                if (version == 1) {
                    if (!mDataSource->getUInt64(entriesoffset, &segment_duration) ||
                            !mDataSource->getUInt64(entriesoffset + 8, (uint64_t*)&media_time)) {
                        return ERROR_IO;
                    }
                } else if (version == 0) {
                    uint32_t sd;
                    int32_t mt;
                    if (!mDataSource->getUInt32(entriesoffset, &sd) ||
                            !mDataSource->getUInt32(entriesoffset + 4, (uint32_t*)&mt)) {
                        return ERROR_IO;
                    }
                    segment_duration = sd;
                    media_time = mt;
                } else {
                    return ERROR_IO;
                }

                // save these for later, because the elst atom might precede
                // the atoms that actually gives us the duration and sample rate
                // needed to calculate the padding and delay values
                mLastTrack->has_elst = true;
                mLastTrack->elst_media_time = media_time;
                mLastTrack->elst_segment_duration = segment_duration;
            }
            break;
        }

        case FOURCC("frma"):
        {
            *offset += chunk_size;

            uint32_t original_fourcc;
            if (mDataSource->readAt(data_offset, &original_fourcc, 4) < 4) {
                return ERROR_IO;
            }
            original_fourcc = ntohl(original_fourcc);
            ALOGV("read original format: %d", original_fourcc);

            if (mLastTrack == NULL) {
                return ERROR_MALFORMED;
            }

            AMediaFormat_setString(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_MIME, FourCC2MIME(original_fourcc));
            uint32_t num_channels = 0;
            uint32_t sample_rate = 0;
            if (AdjustChannelsAndRate(original_fourcc, &num_channels, &sample_rate)) {
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_CHANNEL_COUNT, num_channels);
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_SAMPLE_RATE, sample_rate);
            }

            if (!mIsQT && original_fourcc == FOURCC("alac")) {
                off64_t tmpOffset = *offset;
                status_t err = parseALACSampleEntry(&tmpOffset);
                if (err != OK) {
                    ALOGE("parseALACSampleEntry err:%d Line:%d", err, __LINE__);
                    return err;
                }
                *offset = tmpOffset + 8;
            }

            break;
        }

        case FOURCC("tenc"):
        {
            *offset += chunk_size;

            if (chunk_size < 32) {
                return ERROR_MALFORMED;
            }

            // tenc box contains 1 byte version, 3 byte flags, 3 byte default algorithm id, one byte
            // default IV size, 16 bytes default KeyID
            // (ISO 23001-7)

            uint8_t version;
            if (mDataSource->readAt(data_offset, &version, sizeof(version))
                    < (ssize_t)sizeof(version)) {
                return ERROR_IO;
            }

            uint8_t buf[4];
            memset(buf, 0, 4);
            if (mDataSource->readAt(data_offset + 4, buf + 1, 3) < 3) {
                return ERROR_IO;
            }

            if (mLastTrack == NULL) {
                return ERROR_MALFORMED;
            }

            uint8_t defaultEncryptedByteBlock = 0;
            uint8_t defaultSkipByteBlock = 0;
            uint32_t defaultAlgorithmId = ntohl(*((int32_t*)buf));
            if (version == 1) {
                uint32_t pattern = buf[2];
                defaultEncryptedByteBlock = pattern >> 4;
                defaultSkipByteBlock = pattern & 0xf;
                if (defaultEncryptedByteBlock == 0 && defaultSkipByteBlock == 0) {
                    // use (1,0) to mean "encrypt everything"
                    defaultEncryptedByteBlock = 1;
                }
            } else if (mLastTrack->subsample_encryption) {
                ALOGW("subsample_encryption should be version 1");
            } else if (defaultAlgorithmId > 1) {
                // only 0 (clear) and 1 (AES-128) are valid
                ALOGW("defaultAlgorithmId: %u is a reserved value", defaultAlgorithmId);
                defaultAlgorithmId = 1;
            }

            memset(buf, 0, 4);
            if (mDataSource->readAt(data_offset + 7, buf + 3, 1) < 1) {
                return ERROR_IO;
            }
            uint32_t defaultIVSize = ntohl(*((int32_t*)buf));

            if (defaultAlgorithmId == 0 && defaultIVSize != 0) {
                // only unencrypted data must have 0 IV size
                return ERROR_MALFORMED;
            } else if (defaultIVSize != 0 &&
                    defaultIVSize != 8 &&
                    defaultIVSize != 16) {
                return ERROR_MALFORMED;
            }

            uint8_t defaultKeyId[16];

            if (mDataSource->readAt(data_offset + 8, &defaultKeyId, 16) < 16) {
                return ERROR_IO;
            }

            sp<ABuffer> defaultConstantIv;
            if (defaultAlgorithmId != 0 && defaultIVSize == 0) {

                uint8_t ivlength;
                if (mDataSource->readAt(data_offset + 24, &ivlength, sizeof(ivlength))
                        < (ssize_t)sizeof(ivlength)) {
                    return ERROR_IO;
                }

                if (ivlength != 8 && ivlength != 16) {
                    ALOGW("unsupported IV length: %u", ivlength);
                    return ERROR_MALFORMED;
                }

                defaultConstantIv = new ABuffer(ivlength);
                if (mDataSource->readAt(data_offset + 25, defaultConstantIv->data(), ivlength)
                        < (ssize_t)ivlength) {
                    return ERROR_IO;
                }

                defaultConstantIv->setRange(0, ivlength);
            }

            int32_t tmpAlgorithmId;
            if (!AMediaFormat_getInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_CRYPTO_MODE, &tmpAlgorithmId)) {
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_CRYPTO_MODE, defaultAlgorithmId);
            }

            AMediaFormat_setInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_CRYPTO_DEFAULT_IV_SIZE, defaultIVSize);
            AMediaFormat_setBuffer(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_CRYPTO_KEY, defaultKeyId, 16);
            AMediaFormat_setInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_BYTE_BLOCK, defaultEncryptedByteBlock);
            AMediaFormat_setInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_CRYPTO_SKIP_BYTE_BLOCK, defaultSkipByteBlock);
            if (defaultConstantIv != NULL) {
                AMediaFormat_setBuffer(mLastTrack->meta, AMEDIAFORMAT_KEY_CRYPTO_IV,
                        defaultConstantIv->data(), defaultConstantIv->size());
            }
            break;
        }

        case FOURCC("tkhd"):
        {
            *offset += chunk_size;

            status_t err;
            if ((err = parseTrackHeader(data_offset, chunk_data_size)) != OK) {
                return err;
            }

            break;
        }

        case FOURCC("tref"):
        {
            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset;
            while (*offset < stop_offset) {
                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    return err;
                }
            }
            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }
            break;
        }

        case FOURCC("thmb"):
        {
            *offset += chunk_size;

            if (mLastTrack != NULL) {
                // Skip thumbnail track for now since we don't have an
                // API to retrieve it yet.
                // The thumbnail track can't be accessed by negative index or time,
                // because each timed sample has its own corresponding thumbnail
                // in the thumbnail track. We'll need a dedicated API to retrieve
                // thumbnail at time instead.
                mLastTrack->skipTrack = true;
            }

            break;
        }

        case FOURCC("pssh"):
        {
            *offset += chunk_size;

            PsshInfo pssh;

            if (mDataSource->readAt(data_offset + 4, &pssh.uuid, 16) < 16) {
                return ERROR_IO;
            }

            uint32_t psshdatalen = 0;
            if (mDataSource->readAt(data_offset + 20, &psshdatalen, 4) < 4) {
                return ERROR_IO;
            }
            pssh.datalen = ntohl(psshdatalen);
            ALOGV("pssh data size: %d", pssh.datalen);
            if (chunk_size < 20 || pssh.datalen > chunk_size - 20) {
                // pssh data length exceeds size of containing box
                return ERROR_MALFORMED;
            }

            pssh.data = new (std::nothrow) uint8_t[pssh.datalen];
            if (pssh.data == NULL) {
                return ERROR_MALFORMED;
            }
            ALOGV("allocated pssh @ %p", pssh.data);
            ssize_t requested = (ssize_t) pssh.datalen;
            if (mDataSource->readAt(data_offset + 24, pssh.data, requested) < requested) {
                delete[] pssh.data;
                return ERROR_IO;
            }
            mPssh.push_back(pssh);

            break;
        }

        case FOURCC("mdhd"):
        {
            *offset += chunk_size;

            if (chunk_data_size < 4 || mLastTrack == NULL) {
                return ERROR_MALFORMED;
            }

            uint8_t version;
            if (mDataSource->readAt(
                        data_offset, &version, sizeof(version))
                    < (ssize_t)sizeof(version)) {
                return ERROR_IO;
            }

            off64_t timescale_offset;

            if (version == 1) {
                timescale_offset = data_offset + 4 + 16;
            } else if (version == 0) {
                timescale_offset = data_offset + 4 + 8;
            } else {
                return ERROR_IO;
            }

            uint32_t timescale;
            if (mDataSource->readAt(
                        timescale_offset, &timescale, sizeof(timescale))
                    < (ssize_t)sizeof(timescale)) {
                return ERROR_IO;
            }

            if (!timescale) {
                ALOGE("timescale should not be ZERO.");
                return ERROR_MALFORMED;
            }

            mLastTrack->timescale = ntohl(timescale);

            // 14496-12 says all ones means indeterminate, but some files seem to use
            // 0 instead. We treat both the same.
            int64_t duration = 0;
            if (version == 1) {
                if (mDataSource->readAt(
                            timescale_offset + 4, &duration, sizeof(duration))
                        < (ssize_t)sizeof(duration)) {
                    return ERROR_IO;
                }
                if (duration != -1) {
                    duration = ntoh64(duration);
                }
            } else {
                uint32_t duration32;
                if (mDataSource->readAt(
                            timescale_offset + 4, &duration32, sizeof(duration32))
                        < (ssize_t)sizeof(duration32)) {
                    return ERROR_IO;
                }
                if (duration32 != 0xffffffff) {
                    duration = ntohl(duration32);
                }
            }
            if (duration != 0 && mLastTrack->timescale != 0) {
                long double durationUs = ((long double)duration * 1000000) / mLastTrack->timescale;
                if (durationUs < 0 || durationUs > INT64_MAX) {
                    ALOGE("cannot represent %lld * 1000000 / %lld in 64 bits",
                          (long long) duration, (long long) mLastTrack->timescale);
                    return ERROR_MALFORMED;
                }
                AMediaFormat_setInt64(mLastTrack->meta, AMEDIAFORMAT_KEY_DURATION, durationUs);
            }

            uint8_t lang[2];
            off64_t lang_offset;
            if (version == 1) {
                lang_offset = timescale_offset + 4 + 8;
            } else if (version == 0) {
                lang_offset = timescale_offset + 4 + 4;
            } else {
                return ERROR_IO;
            }

            if (mDataSource->readAt(lang_offset, &lang, sizeof(lang))
                    < (ssize_t)sizeof(lang)) {
                return ERROR_IO;
            }

            // To get the ISO-639-2/T three character language code
            // 1 bit pad followed by 3 5-bits characters. Each character
            // is packed as the difference between its ASCII value and 0x60.
            char lang_code[4];
            lang_code[0] = ((lang[0] >> 2) & 0x1f) + 0x60;
            lang_code[1] = ((lang[0] & 0x3) << 3 | (lang[1] >> 5)) + 0x60;
            lang_code[2] = (lang[1] & 0x1f) + 0x60;
            lang_code[3] = '\0';

            AMediaFormat_setString(mLastTrack->meta, AMEDIAFORMAT_KEY_LANGUAGE, lang_code);

            break;
        }

        case FOURCC("stsd"):
        {
            uint8_t buffer[8];
            if (chunk_data_size < (off64_t)sizeof(buffer)) {
                return ERROR_MALFORMED;
            }

            if (mDataSource->readAt(
                        data_offset, buffer, 8) < 8) {
                return ERROR_IO;
            }

            if (U32_AT(buffer) != 0) {
                // Should be version 0, flags 0.
                return ERROR_MALFORMED;
            }

            uint32_t entry_count = U32_AT(&buffer[4]);

            if (entry_count > 1) {
                // For 3GPP timed text, there could be multiple tx3g boxes contain
                // multiple text display formats. These formats will be used to
                // display the timed text.
                // For encrypted files, there may also be more than one entry.
                const char *mime;

                if (mLastTrack == NULL)
                    return ERROR_MALFORMED;

                CHECK(AMediaFormat_getString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, &mime));
                if (strcasecmp(mime, MEDIA_MIMETYPE_TEXT_3GPP) &&
                        strcasecmp(mime, "application/octet-stream")) {
                    // For now we only support a single type of media per track.
                    mLastTrack->skipTrack = true;
                    *offset += chunk_size;
                    break;
                }
            }
            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset + 8;
            for (uint32_t i = 0; i < entry_count; ++i) {
                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    return err;
                }
            }

            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }
            break;
        }
        case FOURCC("mett"):
        {
            *offset += chunk_size;

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            auto buffer = heapbuffer<uint8_t>(chunk_data_size);
            if (buffer.get() == NULL) {
                return NO_MEMORY;
            }

            if (mDataSource->readAt(
                        data_offset, buffer.get(), chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            // Prior to API 29, the metadata track was not compliant with ISO/IEC
            // 14496-12-2015. This led to some ISO-compliant parsers failing to read the
            // metatrack. As of API 29 and onwards, a change was made to metadata track to
            // make it compliant with the standard. The workaround is to write the
            // null-terminated mime_format string twice. This allows compliant parsers to
            // read the missing reserved, data_reference_index, and content_encoding fields
            // from the first mime_type string. The actual mime_format field would then be
            // read correctly from the second string. The non-compliant Android frameworks
            // from API 28 and earlier would still be able to read the mime_format correctly
            // as it would only read the first null-terminated mime_format string. To enable
            // reading metadata tracks generated from both the non-compliant and compliant
            // formats, a check needs to be done to see which format is used.
            int null_pos = 0;
            const unsigned char *str = buffer.get();
            while (null_pos < chunk_data_size) {
              if (*(str + null_pos) == '\0') {
                break;
              }
              ++null_pos;
            }

            if (null_pos == chunk_data_size - 1) {
              // This is not a standard ompliant metadata track.
              String8 mimeFormat((const char *)(buffer.get()), chunk_data_size);
              AMediaFormat_setString(mLastTrack->meta,
                  AMEDIAFORMAT_KEY_MIME, mimeFormat.string());
            } else {
              // This is a standard compliant metadata track.
              String8 contentEncoding((const char *)(buffer.get() + 8));
              String8 mimeFormat((const char *)(buffer.get() + 8 + contentEncoding.size() + 1),
                  chunk_data_size - 8 - contentEncoding.size() - 1);
              AMediaFormat_setString(mLastTrack->meta,
                  AMEDIAFORMAT_KEY_MIME, mimeFormat.string());
            }
            break;
        }

        case FOURCC("mp4a"):
        case FOURCC("enca"):
        case FOURCC("samr"):
        case FOURCC("sawb"):
        case FOURCC("Opus"):
        case FOURCC("twos"):
        case FOURCC("sowt"):
        case FOURCC("alac"):
        case FOURCC("fLaC"):
        case FOURCC(".mp3"):
        case 0x6D730055: // "ms U" mp3 audio
        {
            if (mIsQT && depth >= 1 && mPath[depth - 1] == FOURCC("wave")) {

                if (chunk_type == FOURCC("alac")) {
                    off64_t offsetTmp = *offset;
                    status_t err = parseALACSampleEntry(&offsetTmp);
                    if (err != OK) {
                        ALOGE("parseALACSampleEntry err:%d Line:%d", err, __LINE__);
                        return err;
                    }
                }

                // Ignore all atoms embedded in QT wave atom
                ALOGV("Ignore all atoms embedded in QT wave atom");
                *offset += chunk_size;
                break;
            }

            uint8_t buffer[8 + 20];
            if (chunk_data_size < (ssize_t)sizeof(buffer)) {
                // Basic AudioSampleEntry size.
                return ERROR_MALFORMED;
            }

            if (mDataSource->readAt(
                        data_offset, buffer, sizeof(buffer)) < (ssize_t)sizeof(buffer)) {
                return ERROR_IO;
            }

            uint16_t data_ref_index __unused = U16_AT(&buffer[6]);
            uint16_t version = U16_AT(&buffer[8]);
            uint32_t num_channels = U16_AT(&buffer[16]);

            uint16_t sample_size = U16_AT(&buffer[18]);
            uint32_t sample_rate = U32_AT(&buffer[24]) >> 16;

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset + sizeof(buffer);

            if (mIsQT) {
                if (version == 1) {
                    if (mDataSource->readAt(*offset, buffer, 16) < 16) {
                        return ERROR_IO;
                    }

#if 0
                    U32_AT(buffer);  // samples per packet
                    U32_AT(&buffer[4]);  // bytes per packet
                    U32_AT(&buffer[8]);  // bytes per frame
                    U32_AT(&buffer[12]);  // bytes per sample
#endif
                    *offset += 16;
                } else if (version == 2) {
                    uint8_t v2buffer[36];
                    if (mDataSource->readAt(*offset, v2buffer, 36) < 36) {
                        return ERROR_IO;
                    }

#if 0
                    U32_AT(v2buffer);  // size of struct only
                    sample_rate = (uint32_t)U64_AT(&v2buffer[4]);  // audio sample rate
                    num_channels = U32_AT(&v2buffer[12]);  // num audio channels
                    U32_AT(&v2buffer[16]);  // always 0x7f000000
                    sample_size = (uint16_t)U32_AT(&v2buffer[20]);  // const bits per channel
                    U32_AT(&v2buffer[24]);  // format specifc flags
                    U32_AT(&v2buffer[28]);  // const bytes per audio packet
                    U32_AT(&v2buffer[32]);  // const LPCM frames per audio packet
#endif
                    *offset += 36;
                }
            }

            if (chunk_type != FOURCC("enca")) {
                // if the chunk type is enca, we'll get the type from the frma box later
                AMediaFormat_setString(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_MIME, FourCC2MIME(chunk_type));
                AdjustChannelsAndRate(chunk_type, &num_channels, &sample_rate);

                if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_RAW, FourCC2MIME(chunk_type))) {
                    AMediaFormat_setInt32(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, sample_size);
                    if (chunk_type == FOURCC("twos")) {
                        AMediaFormat_setInt32(mLastTrack->meta,
                                AMEDIAFORMAT_KEY_PCM_BIG_ENDIAN, 1);
                    }
                }
            }
            ALOGV("*** coding='%s' %d channels, size %d, rate %d\n",
                   chunk, num_channels, sample_size, sample_rate);
            AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, num_channels);
            AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sample_rate);

            if (chunk_type == FOURCC("Opus")) {
                uint8_t opusInfo[AOPUS_OPUSHEAD_MAXSIZE];
                data_offset += sizeof(buffer);
                size_t opusInfoSize = chunk_data_size - sizeof(buffer);

                if (opusInfoSize < AOPUS_OPUSHEAD_MINSIZE ||
                    opusInfoSize > AOPUS_OPUSHEAD_MAXSIZE) {
                    return ERROR_MALFORMED;
                }
                // Read Opus Header
                if (mDataSource->readAt(
                        data_offset, opusInfo, opusInfoSize) < opusInfoSize) {
                    return ERROR_IO;
                }

                // OpusHeader must start with this magic sequence, overwrite first 8 bytes
                // http://wiki.xiph.org/OggOpus#ID_Header
                strncpy((char *)opusInfo, "OpusHead", 8);

                // Version shall be 0 as per mp4 Opus Specific Box
                // (https://opus-codec.org/docs/opus_in_isobmff.html#4.3.2)
                if (opusInfo[8]) {
                    return ERROR_MALFORMED;
                }
                // Force version to 1 as per OpusHead definition
                // (http://wiki.xiph.org/OggOpus#ID_Header)
                opusInfo[8] = 1;

                // Read Opus Specific Box values
                size_t opusOffset = 10;
                uint16_t pre_skip = U16_AT(&opusInfo[opusOffset]);
                uint32_t sample_rate = U32_AT(&opusInfo[opusOffset + 2]);
                uint16_t out_gain = U16_AT(&opusInfo[opusOffset + 6]);

                // Convert Opus Specific Box values. ParseOpusHeader expects
                // the values in LE, however MP4 stores these values as BE
                // https://opus-codec.org/docs/opus_in_isobmff.html#4.3.2
                memcpy(&opusInfo[opusOffset], &pre_skip, sizeof(pre_skip));
                memcpy(&opusInfo[opusOffset + 2], &sample_rate, sizeof(sample_rate));
                memcpy(&opusInfo[opusOffset + 6], &out_gain, sizeof(out_gain));

                static const int64_t kSeekPreRollNs = 80000000;  // Fixed 80 msec
                static const int32_t kOpusSampleRate = 48000;
                int64_t codecDelay = pre_skip * 1000000000ll / kOpusSampleRate;

                AMediaFormat_setBuffer(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_CSD_0, opusInfo, opusInfoSize);
                AMediaFormat_setBuffer(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_CSD_1, &codecDelay, sizeof(codecDelay));
                AMediaFormat_setBuffer(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_CSD_2, &kSeekPreRollNs, sizeof(kSeekPreRollNs));

                data_offset += opusInfoSize;
                *offset = data_offset;
                CHECK_EQ(*offset, stop_offset);
            }

            if (!mIsQT && chunk_type == FOURCC("alac")) {
                data_offset += sizeof(buffer);

                status_t err = parseALACSampleEntry(&data_offset);
                if (err != OK) {
                    ALOGE("parseALACSampleEntry err:%d Line:%d", err, __LINE__);
                    return err;
                }
                *offset = data_offset;
                CHECK_EQ(*offset, stop_offset);
            }

            if (chunk_type == FOURCC("fLaC")) {

                // From https://github.com/xiph/flac/blob/master/doc/isoflac.txt
                // 4 for mime, 4 for blockType and BlockLen, 34 for metadata
                uint8_t flacInfo[4 + 4 + 34];
                // skipping dFla, version
                data_offset += sizeof(buffer) + 12;
                size_t flacOffset = 4;
                // Add flaC header mime type to CSD
                strncpy((char *)flacInfo, "fLaC", 4);
                if (mDataSource->readAt(
                        data_offset, flacInfo + flacOffset, sizeof(flacInfo) - flacOffset) <
                        (ssize_t)sizeof(flacInfo) - flacOffset) {
                    return ERROR_IO;
                }
                data_offset += sizeof(flacInfo) - flacOffset;

                AMediaFormat_setBuffer(mLastTrack->meta, AMEDIAFORMAT_KEY_CSD_0, flacInfo,
                                       sizeof(flacInfo));
                *offset = data_offset;
                CHECK_EQ(*offset, stop_offset);
            }

            while (*offset < stop_offset) {
                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    return err;
                }
            }

            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }
            break;
        }

        case FOURCC("mp4v"):
        case FOURCC("encv"):
        case FOURCC("s263"):
        case FOURCC("H263"):
        case FOURCC("h263"):
        case FOURCC("avc1"):
        case FOURCC("hvc1"):
        case FOURCC("hev1"):
        case FOURCC("av01"):
        {
            uint8_t buffer[78];
            if (chunk_data_size < (ssize_t)sizeof(buffer)) {
                // Basic VideoSampleEntry size.
                return ERROR_MALFORMED;
            }

            if (mDataSource->readAt(
                        data_offset, buffer, sizeof(buffer)) < (ssize_t)sizeof(buffer)) {
                return ERROR_IO;
            }

            uint16_t data_ref_index __unused = U16_AT(&buffer[6]);
            uint16_t width = U16_AT(&buffer[6 + 18]);
            uint16_t height = U16_AT(&buffer[6 + 20]);

            // The video sample is not standard-compliant if it has invalid dimension.
            // Use some default width and height value, and
            // let the decoder figure out the actual width and height (and thus
            // be prepared for INFO_FOMRAT_CHANGED event).
            if (width == 0)  width  = 352;
            if (height == 0) height = 288;

            // printf("*** coding='%s' width=%d height=%d\n",
            //        chunk, width, height);

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            if (chunk_type != FOURCC("encv")) {
                // if the chunk type is encv, we'll get the type from the frma box later
                AMediaFormat_setString(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_MIME, FourCC2MIME(chunk_type));
            }
            AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_WIDTH, width);
            AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_HEIGHT, height);

            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset + sizeof(buffer);
            while (*offset < stop_offset) {
                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    return err;
                }
            }

            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }
            break;
        }

        case FOURCC("stco"):
        case FOURCC("co64"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL)) {
                return ERROR_MALFORMED;
            }

            status_t err =
                mLastTrack->sampleTable->setChunkOffsetParams(
                        chunk_type, data_offset, chunk_data_size);

            *offset += chunk_size;

            if (err != OK) {
                return err;
            }

            break;
        }

        case FOURCC("stsc"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL))
                return ERROR_MALFORMED;

            status_t err =
                mLastTrack->sampleTable->setSampleToChunkParams(
                        data_offset, chunk_data_size);

            *offset += chunk_size;

            if (err != OK) {
                return err;
            }

            break;
        }

        case FOURCC("stsz"):
        case FOURCC("stz2"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL)) {
                return ERROR_MALFORMED;
            }

            status_t err =
                mLastTrack->sampleTable->setSampleSizeParams(
                        chunk_type, data_offset, chunk_data_size);

            *offset += chunk_size;

            if (err != OK) {
                return err;
            }

            adjustRawDefaultFrameSize();

            size_t max_size;
            err = mLastTrack->sampleTable->getMaxSampleSize(&max_size);

            if (err != OK) {
                return err;
            }

            if (max_size != 0) {
                // Assume that a given buffer only contains at most 10 chunks,
                // each chunk originally prefixed with a 2 byte length will
                // have a 4 byte header (0x00 0x00 0x00 0x01) after conversion,
                // and thus will grow by 2 bytes per chunk.
                if (max_size > SIZE_MAX - 10 * 2) {
                    ALOGE("max sample size too big: %zu", max_size);
                    return ERROR_MALFORMED;
                }
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size + 10 * 2);
            } else {
                // No size was specified. Pick a conservatively large size.
                uint32_t width, height;
                if (!AMediaFormat_getInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_WIDTH, (int32_t*)&width) ||
                    !AMediaFormat_getInt32(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_HEIGHT,(int32_t*) &height)) {
                    ALOGE("No width or height, assuming worst case 1080p");
                    width = 1920;
                    height = 1080;
                } else {
                    // A resolution was specified, check that it's not too big. The values below
                    // were chosen so that the calculations below don't cause overflows, they're
                    // not indicating that resolutions up to 32kx32k are actually supported.
                    if (width > 32768 || height > 32768) {
                        ALOGE("can't support %u x %u video", width, height);
                        return ERROR_MALFORMED;
                    }
                }

                const char *mime;
                CHECK(AMediaFormat_getString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, &mime));
                if (!strncmp(mime, "audio/", 6)) {
                    // for audio, use 128KB
                    max_size = 1024 * 128;
                } else if (!strcmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)
                        || !strcmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)) {
                    // AVC & HEVC requires compression ratio of at least 2, and uses
                    // macroblocks
                    max_size = ((width + 15) / 16) * ((height + 15) / 16) * 192;
                } else {
                    // For all other formats there is no minimum compression
                    // ratio. Use compression ratio of 1.
                    max_size = width * height * 3 / 2;
                }
                // HACK: allow 10% overhead
                // TODO: read sample size from traf atom for fragmented MPEG4.
                max_size += max_size / 10;
                AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);
            }

            // NOTE: setting another piece of metadata invalidates any pointers (such as the
            // mimetype) previously obtained, so don't cache them.
            const char *mime;
            CHECK(AMediaFormat_getString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, &mime));
            // Calculate average frame rate.
            if (!strncasecmp("video/", mime, 6)) {
                size_t nSamples = mLastTrack->sampleTable->countSamples();
                if (nSamples == 0) {
                    int32_t trackId;
                    if (AMediaFormat_getInt32(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_TRACK_ID, &trackId)) {
                        for (size_t i = 0; i < mTrex.size(); i++) {
                            Trex *t = &mTrex.editItemAt(i);
                            if (t->track_ID == (uint32_t) trackId) {
                                if (t->default_sample_duration > 0) {
                                    int32_t frameRate =
                                            mLastTrack->timescale / t->default_sample_duration;
                                    AMediaFormat_setInt32(mLastTrack->meta,
                                            AMEDIAFORMAT_KEY_FRAME_RATE, frameRate);
                                }
                                break;
                            }
                        }
                    }
                } else {
                    int64_t durationUs;
                    if (AMediaFormat_getInt64(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_DURATION, &durationUs)) {
                        if (durationUs > 0) {
                            int32_t frameRate = (nSamples * 1000000LL +
                                        (durationUs >> 1)) / durationUs;
                            AMediaFormat_setInt32(mLastTrack->meta,
                                    AMEDIAFORMAT_KEY_FRAME_RATE, frameRate);
                        }
                    }
                    ALOGV("setting frame count %zu", nSamples);
                    AMediaFormat_setInt32(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_FRAME_COUNT, nSamples);
                }
            }

            break;
        }

        case FOURCC("stts"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL))
                return ERROR_MALFORMED;

            *offset += chunk_size;

            if (depth >= 1 && mPath[depth - 1] != FOURCC("stbl")) {
                char chunk[5];
                MakeFourCCString(mPath[depth - 1], chunk);
                ALOGW("stts's parent box (%s) is not stbl, skip it.", chunk);
                break;
            }

            status_t err =
                mLastTrack->sampleTable->setTimeToSampleParams(
                        data_offset, chunk_data_size);

            if (err != OK) {
                return err;
            }

            break;
        }

        case FOURCC("ctts"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL))
                return ERROR_MALFORMED;

            *offset += chunk_size;

            status_t err =
                mLastTrack->sampleTable->setCompositionTimeToSampleParams(
                        data_offset, chunk_data_size);

            if (err != OK) {
                return err;
            }

            break;
        }

        case FOURCC("stss"):
        {
            if ((mLastTrack == NULL) || (mLastTrack->sampleTable == NULL))
                return ERROR_MALFORMED;

            *offset += chunk_size;

            status_t err =
                mLastTrack->sampleTable->setSyncSampleParams(
                        data_offset, chunk_data_size);

            if (err != OK) {
                return err;
            }

            break;
        }

        // \xA9xyz
        case FOURCC("\251xyz"):
        {
            *offset += chunk_size;

            // Best case the total data length inside "\xA9xyz" box would
            // be 9, for instance "\xA9xyz" + "\x00\x05\x15\xc7" + "+0+0/",
            // where "\x00\x05" is the text string length with value = 5,
            // "\0x15\xc7" is the language code = en, and "+0+0/" is a
            // location (string) value with longitude = 0 and latitude = 0.
            // Since some devices encountered in the wild omit the trailing
            // slash, we'll allow that.
            if (chunk_data_size < 8) { // 8 instead of 9 to allow for missing /
                return ERROR_MALFORMED;
            }

            uint16_t len;
            if (!mDataSource->getUInt16(data_offset, &len)) {
                return ERROR_IO;
            }

            // allow "+0+0" without trailing slash
            if (len < 4 || len > chunk_data_size - 4) {
                return ERROR_MALFORMED;
            }
            // The location string following the language code is formatted
            // according to ISO 6709:2008 (https://en.wikipedia.org/wiki/ISO_6709).
            // Allocate 2 extra bytes, in case we need to add a trailing slash,
            // and to add a terminating 0.
            std::unique_ptr<char[]> buffer(new (std::nothrow) char[len+2]());
            if (!buffer) {
                return NO_MEMORY;
            }

            if (mDataSource->readAt(
                        data_offset + 4, &buffer[0], len) < len) {
                return ERROR_IO;
            }

            len = strlen(&buffer[0]);
            if (len < 4) {
                return ERROR_MALFORMED;
            }
            // Add a trailing slash if there wasn't one.
            if (buffer[len - 1] != '/') {
                buffer[len] = '/';
            }
            AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_LOCATION, &buffer[0]);
            break;
        }

        case FOURCC("esds"):
        {
            *offset += chunk_size;

            if (chunk_data_size < 4) {
                return ERROR_MALFORMED;
            }

            auto tmp = heapbuffer<uint8_t>(chunk_data_size);
            uint8_t *buffer = tmp.get();
            if (buffer == NULL) {
                return -ENOMEM;
            }

            if (mDataSource->readAt(
                        data_offset, buffer, chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            if (U32_AT(buffer) != 0) {
                // Should be version 0, flags 0.
                return ERROR_MALFORMED;
            }

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            AMediaFormat_setBuffer(mLastTrack->meta, 
                    AMEDIAFORMAT_KEY_ESDS, &buffer[4], chunk_data_size - 4);

            if (mPath.size() >= 2
                    && mPath[mPath.size() - 2] == FOURCC("mp4a")) {
                // Information from the ESDS must be relied on for proper
                // setup of sample rate and channel count for MPEG4 Audio.
                // The generic header appears to only contain generic
                // information...

                status_t err = updateAudioTrackInfoFromESDS_MPEG4Audio(
                        &buffer[4], chunk_data_size - 4);

                if (err != OK) {
                    return err;
                }
            }
            if (mPath.size() >= 2
                    && mPath[mPath.size() - 2] == FOURCC("mp4v")) {
                // Check if the video is MPEG2
                ESDS esds(&buffer[4], chunk_data_size - 4);

                uint8_t objectTypeIndication;
                if (esds.getObjectTypeIndication(&objectTypeIndication) == OK) {
                    if (objectTypeIndication >= 0x60 && objectTypeIndication <= 0x65) {
                        AMediaFormat_setString(mLastTrack->meta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_MPEG2);
                    }
                }
            }
            break;
        }

        case FOURCC("btrt"):
        {
            *offset += chunk_size;
            if (mLastTrack == NULL) {
                return ERROR_MALFORMED;
            }

            uint8_t buffer[12];
            if (chunk_data_size != sizeof(buffer)) {
                return ERROR_MALFORMED;
            }

            if (mDataSource->readAt(
                    data_offset, buffer, chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            uint32_t maxBitrate = U32_AT(&buffer[4]);
            uint32_t avgBitrate = U32_AT(&buffer[8]);
            if (maxBitrate > 0 && maxBitrate < INT32_MAX) {
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_MAX_BIT_RATE, (int32_t)maxBitrate);
            }
            if (avgBitrate > 0 && avgBitrate < INT32_MAX) {
                AMediaFormat_setInt32(mLastTrack->meta,
                        AMEDIAFORMAT_KEY_BIT_RATE, (int32_t)avgBitrate);
            }
            break;
        }

        case FOURCC("avcC"):
        {
            *offset += chunk_size;

            auto buffer = heapbuffer<uint8_t>(chunk_data_size);

            if (buffer.get() == NULL) {
                ALOGE("b/28471206");
                return NO_MEMORY;
            }

            if (mDataSource->readAt(
                        data_offset, buffer.get(), chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            AMediaFormat_setBuffer(mLastTrack->meta, 
                    AMEDIAFORMAT_KEY_CSD_AVC, buffer.get(), chunk_data_size);

            break;
        }
        case FOURCC("hvcC"):
        {
            auto buffer = heapbuffer<uint8_t>(chunk_data_size);

            if (buffer.get() == NULL) {
                ALOGE("b/28471206");
                return NO_MEMORY;
            }

            if (mDataSource->readAt(
                        data_offset, buffer.get(), chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            AMediaFormat_setBuffer(mLastTrack->meta, 
                    AMEDIAFORMAT_KEY_CSD_HEVC, buffer.get(), chunk_data_size);

            *offset += chunk_size;
            break;
        }
        case FOURCC("av1C"):
        {
            auto buffer = heapbuffer<uint8_t>(chunk_data_size);

            if (buffer.get() == NULL) {
                ALOGE("b/28471206");
                return NO_MEMORY;
            }

            if (mDataSource->readAt(
                        data_offset, buffer.get(), chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            AMediaFormat_setBuffer(mLastTrack->meta,
                   AMEDIAFORMAT_KEY_CSD_0, buffer.get(), chunk_data_size);

            *offset += chunk_size;
            break;
        }
        case FOURCC("d263"):
        {
            *offset += chunk_size;
            /*
             * d263 contains a fixed 7 bytes part:
             *   vendor - 4 bytes
             *   version - 1 byte
             *   level - 1 byte
             *   profile - 1 byte
             * optionally, "d263" box itself may contain a 16-byte
             * bit rate box (bitr)
             *   average bit rate - 4 bytes
             *   max bit rate - 4 bytes
             */
            char buffer[23];
            if (chunk_data_size != 7 &&
                chunk_data_size != 23) {
                ALOGE("Incorrect D263 box size %lld", (long long)chunk_data_size);
                return ERROR_MALFORMED;
            }

            if (mDataSource->readAt(
                    data_offset, buffer, chunk_data_size) < chunk_data_size) {
                return ERROR_IO;
            }

            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            AMediaFormat_setBuffer(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_D263, buffer, chunk_data_size);

            break;
        }

        case FOURCC("meta"):
        {
            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset;
            bool isParsingMetaKeys = underQTMetaPath(mPath, 2);
            if (!isParsingMetaKeys) {
                uint8_t buffer[4];
                if (chunk_data_size < (off64_t)sizeof(buffer)) {
                    *offset = stop_offset;
                    return ERROR_MALFORMED;
                }

                if (mDataSource->readAt(
                            data_offset, buffer, 4) < 4) {
                    *offset = stop_offset;
                    return ERROR_IO;
                }

                if (U32_AT(buffer) != 0) {
                    // Should be version 0, flags 0.

                    // If it's not, let's assume this is one of those
                    // apparently malformed chunks that don't have flags
                    // and completely different semantics than what's
                    // in the MPEG4 specs and skip it.
                    *offset = stop_offset;
                    return OK;
                }
                *offset +=  sizeof(buffer);
            }

            while (*offset < stop_offset) {
                status_t err = parseChunk(offset, depth + 1);
                if (err != OK) {
                    return err;
                }
            }

            if (*offset != stop_offset) {
                return ERROR_MALFORMED;
            }
            break;
        }

        case FOURCC("iloc"):
        case FOURCC("iinf"):
        case FOURCC("iprp"):
        case FOURCC("pitm"):
        case FOURCC("idat"):
        case FOURCC("iref"):
        case FOURCC("ipro"):
        {
            if (mIsHeif) {
                if (mItemTable == NULL) {
                    mItemTable = new ItemTable(mDataSource);
                }
                status_t err = mItemTable->parse(
                        chunk_type, data_offset, chunk_data_size);
                if (err != OK) {
                    return err;
                }
            }
            *offset += chunk_size;
            break;
        }

        case FOURCC("mean"):
        case FOURCC("name"):
        case FOURCC("data"):
        {
            *offset += chunk_size;

            if (mPath.size() == 6 && underMetaDataPath(mPath)) {
                status_t err = parseITunesMetaData(data_offset, chunk_data_size);

                if (err != OK) {
                    return err;
                }
            }

            break;
        }

        case FOURCC("mvhd"):
        {
            *offset += chunk_size;

            if (depth != 1) {
                ALOGE("mvhd: depth %d", depth);
                return ERROR_MALFORMED;
            }
            if (chunk_data_size < 32) {
                return ERROR_MALFORMED;
            }

            uint8_t header[32];
            if (mDataSource->readAt(
                        data_offset, header, sizeof(header))
                    < (ssize_t)sizeof(header)) {
                return ERROR_IO;
            }

            uint64_t creationTime;
            uint64_t duration = 0;
            if (header[0] == 1) {
                creationTime = U64_AT(&header[4]);
                mHeaderTimescale = U32_AT(&header[20]);
                duration = U64_AT(&header[24]);
                if (duration == 0xffffffffffffffff) {
                    duration = 0;
                }
            } else if (header[0] != 0) {
                return ERROR_MALFORMED;
            } else {
                creationTime = U32_AT(&header[4]);
                mHeaderTimescale = U32_AT(&header[12]);
                uint32_t d32 = U32_AT(&header[16]);
                if (d32 == 0xffffffff) {
                    d32 = 0;
                }
                duration = d32;
            }
            if (duration != 0 && mHeaderTimescale != 0 && duration < UINT64_MAX / 1000000) {
                AMediaFormat_setInt64(mFileMetaData,
                        AMEDIAFORMAT_KEY_DURATION, duration * 1000000 / mHeaderTimescale);
            }

            String8 s;
            if (convertTimeToDate(creationTime, &s)) {
                AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_DATE, s.string());
            }

            break;
        }

        case FOURCC("mehd"):
        {
            *offset += chunk_size;

            if (chunk_data_size < 8) {
                return ERROR_MALFORMED;
            }

            uint8_t flags[4];
            if (mDataSource->readAt(
                        data_offset, flags, sizeof(flags))
                    < (ssize_t)sizeof(flags)) {
                return ERROR_IO;
            }

            uint64_t duration = 0;
            if (flags[0] == 1) {
                // 64 bit
                if (chunk_data_size < 12) {
                    return ERROR_MALFORMED;
                }
                mDataSource->getUInt64(data_offset + 4, &duration);
                if (duration == 0xffffffffffffffff) {
                    duration = 0;
                }
            } else if (flags[0] == 0) {
                // 32 bit
                uint32_t d32;
                mDataSource->getUInt32(data_offset + 4, &d32);
                if (d32 == 0xffffffff) {
                    d32 = 0;
                }
                duration = d32;
            } else {
                return ERROR_MALFORMED;
            }

            if (duration != 0 && mHeaderTimescale != 0) {
                AMediaFormat_setInt64(mFileMetaData,
                        AMEDIAFORMAT_KEY_DURATION, duration * 1000000 / mHeaderTimescale);
            }

            break;
        }

        case FOURCC("mdat"):
        {
            mMdatFound = true;

            *offset += chunk_size;
            break;
        }

        case FOURCC("hdlr"):
        {
            *offset += chunk_size;

            if (underQTMetaPath(mPath, 3)) {
                break;
            }

            uint32_t buffer;
            if (mDataSource->readAt(
                        data_offset + 8, &buffer, 4) < 4) {
                return ERROR_IO;
            }

            uint32_t type = ntohl(buffer);
            // For the 3GPP file format, the handler-type within the 'hdlr' box
            // shall be 'text'. We also want to support 'sbtl' handler type
            // for a practical reason as various MPEG4 containers use it.
            if (type == FOURCC("text") || type == FOURCC("sbtl")) {
                if (mLastTrack != NULL) {
                    AMediaFormat_setString(mLastTrack->meta,
                            AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_TEXT_3GPP);
                }
            }

            break;
        }

        case FOURCC("keys"):
        {
            *offset += chunk_size;

            if (underQTMetaPath(mPath, 3)) {
                status_t err = parseQTMetaKey(data_offset, chunk_data_size);
                if (err != OK) {
                    return err;
                }
            }
            break;
        }

        case FOURCC("trex"):
        {
            *offset += chunk_size;

            if (chunk_data_size < 24) {
                return ERROR_IO;
            }
            Trex trex;
            if (!mDataSource->getUInt32(data_offset + 4, &trex.track_ID) ||
                !mDataSource->getUInt32(data_offset + 8, &trex.default_sample_description_index) ||
                !mDataSource->getUInt32(data_offset + 12, &trex.default_sample_duration) ||
                !mDataSource->getUInt32(data_offset + 16, &trex.default_sample_size) ||
                !mDataSource->getUInt32(data_offset + 20, &trex.default_sample_flags)) {
                return ERROR_IO;
            }
            mTrex.add(trex);
            break;
        }

        case FOURCC("tx3g"):
        {
            if (mLastTrack == NULL)
                return ERROR_MALFORMED;

            // complain about ridiculous chunks
            if (chunk_size > kMaxAtomSize) {
                return ERROR_MALFORMED;
            }

            // complain about empty atoms
            if (chunk_data_size <= 0) {
                ALOGE("b/124330204");
                android_errorWriteLog(0x534e4554, "124330204");
                return ERROR_MALFORMED;
            }

            // should fill buffer based on "data_offset" and "chunk_data_size"
            // instead of *offset and chunk_size;
            // but we've been feeding the extra data to consumers for multiple releases and
            // if those apps are compensating for it, we'd break them with such a change
            //

            if (mLastTrack->mTx3gBuffer == NULL) {
                mLastTrack->mTx3gSize = 0;
                mLastTrack->mTx3gFilled = 0;
            }
            if (mLastTrack->mTx3gSize - mLastTrack->mTx3gFilled < chunk_size) {
                size_t growth = kTx3gGrowth;
                if (growth < chunk_size) {
                    growth = chunk_size;
                }
                // although this disallows 2 tx3g atoms of nearly kMaxAtomSize...
                if ((uint64_t) mLastTrack->mTx3gSize + growth > kMaxAtomSize) {
                    ALOGE("b/124330204 - too much space");
                    android_errorWriteLog(0x534e4554, "124330204");
                    return ERROR_MALFORMED;
                }
                uint8_t *updated = (uint8_t *)realloc(mLastTrack->mTx3gBuffer,
                                                mLastTrack->mTx3gSize + growth);
                if (updated == NULL) {
                    return ERROR_MALFORMED;
                }
                mLastTrack->mTx3gBuffer = updated;
                mLastTrack->mTx3gSize += growth;
            }

            if ((size_t)(mDataSource->readAt(*offset,
                                             mLastTrack->mTx3gBuffer + mLastTrack->mTx3gFilled,
                                             chunk_size))
                    < chunk_size) {

                // advance read pointer so we don't end up reading this again
                *offset += chunk_size;
                return ERROR_IO;
            }

            mLastTrack->mTx3gFilled += chunk_size;
            *offset += chunk_size;
            break;
        }

        case FOURCC("covr"):
        {
            *offset += chunk_size;

            ALOGV("chunk_data_size = %" PRId64 " and data_offset = %" PRId64,
                  chunk_data_size, data_offset);

            if (chunk_data_size < 0 || static_cast<uint64_t>(chunk_data_size) >= SIZE_MAX - 1) {
                return ERROR_MALFORMED;
            }
            auto buffer = heapbuffer<uint8_t>(chunk_data_size);
            if (buffer.get() == NULL) {
                ALOGE("b/28471206");
                return NO_MEMORY;
            }
            if (mDataSource->readAt(
                data_offset, buffer.get(), chunk_data_size) != (ssize_t)chunk_data_size) {
                return ERROR_IO;
            }
            const int kSkipBytesOfDataBox = 16;
            if (chunk_data_size <= kSkipBytesOfDataBox) {
                return ERROR_MALFORMED;
            }

            AMediaFormat_setBuffer(mFileMetaData,
                AMEDIAFORMAT_KEY_ALBUMART,
                buffer.get() + kSkipBytesOfDataBox, chunk_data_size - kSkipBytesOfDataBox);

            break;
        }

        case FOURCC("colr"):
        {
            *offset += chunk_size;
            // this must be in a VisualSampleEntry box under the Sample Description Box ('stsd')
            // ignore otherwise
            if (depth >= 2 && mPath[depth - 2] == FOURCC("stsd")) {
                status_t err = parseColorInfo(data_offset, chunk_data_size);
                if (err != OK) {
                    return err;
                }
            }

            break;
        }

        case FOURCC("titl"):
        case FOURCC("perf"):
        case FOURCC("auth"):
        case FOURCC("gnre"):
        case FOURCC("albm"):
        case FOURCC("yrrc"):
        {
            *offset += chunk_size;

            status_t err = parse3GPPMetaData(data_offset, chunk_data_size, depth);

            if (err != OK) {
                return err;
            }

            break;
        }

        case FOURCC("ID32"):
        {
            *offset += chunk_size;

            if (chunk_data_size < 6) {
                return ERROR_MALFORMED;
            }

            parseID3v2MetaData(data_offset + 6);

            break;
        }

        case FOURCC("----"):
        {
            mLastCommentMean.clear();
            mLastCommentName.clear();
            mLastCommentData.clear();
            *offset += chunk_size;
            break;
        }

        case FOURCC("sidx"):
        {
            status_t err = parseSegmentIndex(data_offset, chunk_data_size);
            if (err != OK) {
                return err;
            }
            *offset += chunk_size;
            return UNKNOWN_ERROR; // stop parsing after sidx
        }

        case FOURCC("ac-3"):
        {
            *offset += chunk_size;
            // bypass ac-3 if parse fail
            if (parseAC3SpecificBox(data_offset) != OK) {
                if (mLastTrack != NULL) {
                    ALOGW("Fail to parse ac-3");
                    mLastTrack->skipTrack = true;
                }
            }
            return OK;
        }

        case FOURCC("ec-3"):
        {
            *offset += chunk_size;
            // bypass ec-3 if parse fail
            if (parseEAC3SpecificBox(data_offset) != OK) {
                if (mLastTrack != NULL) {
                    ALOGW("Fail to parse ec-3");
                    mLastTrack->skipTrack = true;
                }
            }
            return OK;
        }

        case FOURCC("ac-4"):
        {
            *offset += chunk_size;
            // bypass ac-4 if parse fail
            if (parseAC4SpecificBox(data_offset) != OK) {
                if (mLastTrack != NULL) {
                    ALOGW("Fail to parse ac-4");
                    mLastTrack->skipTrack = true;
                }
            }
            return OK;
        }

        case FOURCC("ftyp"):
        {
            if (chunk_data_size < 8 || depth != 0) {
                return ERROR_MALFORMED;
            }

            off64_t stop_offset = *offset + chunk_size;
            uint32_t numCompatibleBrands = (chunk_data_size - 8) / 4;
            std::set<uint32_t> brandSet;
            for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
                if (i == 1) {
                    // Skip this index, it refers to the minorVersion,
                    // not a brand.
                    continue;
                }

                uint32_t brand;
                if (mDataSource->readAt(data_offset + 4 * i, &brand, 4) < 4) {
                    return ERROR_MALFORMED;
                }

                brand = ntohl(brand);
                brandSet.insert(brand);
            }

            if (brandSet.count(FOURCC("qt  ")) > 0) {
                mIsQT = true;
            } else {
                if (brandSet.count(FOURCC("mif1")) > 0
                 && brandSet.count(FOURCC("heic")) > 0) {
                    ALOGV("identified HEIF image");

                    mIsHeif = true;
                    brandSet.erase(FOURCC("mif1"));
                    brandSet.erase(FOURCC("heic"));
                }

                if (!brandSet.empty()) {
                    // This means that the file should have moov box.
                    // It could be any iso files (mp4, heifs, etc.)
                    mHasMoovBox = true;
                    if (mIsHeif) {
                        ALOGV("identified HEIF image with other tracks");
                    }
                }
            }

            *offset = stop_offset;

            break;
        }

        default:
        {
            // check if we're parsing 'ilst' for meta keys
            // if so, treat type as a number (key-id).
            if (underQTMetaPath(mPath, 3)) {
                status_t err = parseQTMetaVal(chunk_type, data_offset, chunk_data_size);
                if (err != OK) {
                    return err;
                }
            }

            *offset += chunk_size;
            break;
        }
    }

    return OK;
}

status_t MPEG4Extractor::parseChannelCountSampleRate(
        off64_t *offset, uint16_t *channelCount, uint16_t *sampleRate) {
    // skip 16 bytes:
    //  + 6-byte reserved,
    //  + 2-byte data reference index,
    //  + 8-byte reserved
    *offset += 16;
    if (!mDataSource->getUInt16(*offset, channelCount)) {
        ALOGE("MPEG4Extractor: error while reading sample entry box: cannot read channel count");
        return ERROR_MALFORMED;
    }
    // skip 8 bytes:
    //  + 2-byte channelCount,
    //  + 2-byte sample size,
    //  + 4-byte reserved
    *offset += 8;
    if (!mDataSource->getUInt16(*offset, sampleRate)) {
        ALOGE("MPEG4Extractor: error while reading sample entry box: cannot read sample rate");
        return ERROR_MALFORMED;
    }
    // skip 4 bytes:
    //  + 2-byte sampleRate,
    //  + 2-byte reserved
    *offset += 4;
    return OK;
}

status_t MPEG4Extractor::parseAC4SpecificBox(off64_t offset) {
    if (mLastTrack == NULL) {
        return ERROR_MALFORMED;
    }

    uint16_t sampleRate, channelCount;
    status_t status;
    if ((status = parseChannelCountSampleRate(&offset, &channelCount, &sampleRate)) != OK) {
        return status;
    }
    uint32_t size;
    // + 4-byte size
    // + 4-byte type
    // + 3-byte payload
    const uint32_t kAC4MinimumBoxSize = 4 + 4 + 3;
    if (!mDataSource->getUInt32(offset, &size) || size < kAC4MinimumBoxSize) {
        ALOGE("MPEG4Extractor: error while reading ac-4 block: cannot read specific box size");
        return ERROR_MALFORMED;
    }

    // + 4-byte size
    offset += 4;
    uint32_t type;
    if (!mDataSource->getUInt32(offset, &type) || type != FOURCC("dac4")) {
        ALOGE("MPEG4Extractor: error while reading ac-4 specific block: header not dac4");
        return ERROR_MALFORMED;
    }

    // + 4-byte type
    offset += 4;
    const uint32_t kAC4SpecificBoxPayloadSize = 1176;
    uint8_t chunk[kAC4SpecificBoxPayloadSize];
    ssize_t dsiSize = size - 8; // size of box - size and type fields
    if (dsiSize >= (ssize_t)kAC4SpecificBoxPayloadSize ||
        mDataSource->readAt(offset, chunk, dsiSize) != dsiSize) {
        ALOGE("MPEG4Extractor: error while reading ac-4 specific block: bitstream fields");
        return ERROR_MALFORMED;
    }
    // + size-byte payload
    offset += dsiSize;
    ABitReader br(chunk, dsiSize);
    AC4DSIParser parser(br);
    if (!parser.parse()){
        ALOGE("MPEG4Extractor: error while parsing ac-4 specific block");
        return ERROR_MALFORMED;
    }

    AMediaFormat_setString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_AC4);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);

    AudioPresentationCollection presentations;
    // translate the AC4 presentation information to audio presentations for this track
    AC4DSIParser::AC4Presentations ac4Presentations = parser.getPresentations();
    if (!ac4Presentations.empty()) {
        for (const auto& ac4Presentation : ac4Presentations) {
            auto& presentation = ac4Presentation.second;
            if (!presentation.mEnabled) {
                continue;
            }
            AudioPresentationV1 ap;
            ap.mPresentationId = presentation.mGroupIndex;
            ap.mProgramId = presentation.mProgramID;
            ap.mLanguage = presentation.mLanguage;
            if (presentation.mPreVirtualized) {
                ap.mMasteringIndication = MASTERED_FOR_HEADPHONE;
            } else {
                switch (presentation.mChannelMode) {
                    case AC4Parser::AC4Presentation::kChannelMode_Mono:
                    case AC4Parser::AC4Presentation::kChannelMode_Stereo:
                        ap.mMasteringIndication = MASTERED_FOR_STEREO;
                        break;
                    case AC4Parser::AC4Presentation::kChannelMode_3_0:
                    case AC4Parser::AC4Presentation::kChannelMode_5_0:
                    case AC4Parser::AC4Presentation::kChannelMode_5_1:
                    case AC4Parser::AC4Presentation::kChannelMode_7_0_34:
                    case AC4Parser::AC4Presentation::kChannelMode_7_1_34:
                    case AC4Parser::AC4Presentation::kChannelMode_7_0_52:
                    case AC4Parser::AC4Presentation::kChannelMode_7_1_52:
                        ap.mMasteringIndication = MASTERED_FOR_SURROUND;
                        break;
                    case AC4Parser::AC4Presentation::kChannelMode_7_0_322:
                    case AC4Parser::AC4Presentation::kChannelMode_7_1_322:
                    case AC4Parser::AC4Presentation::kChannelMode_7_0_4:
                    case AC4Parser::AC4Presentation::kChannelMode_7_1_4:
                    case AC4Parser::AC4Presentation::kChannelMode_9_0_4:
                    case AC4Parser::AC4Presentation::kChannelMode_9_1_4:
                    case AC4Parser::AC4Presentation::kChannelMode_22_2:
                        ap.mMasteringIndication = MASTERED_FOR_3D;
                        break;
                    default:
                        ALOGE("Invalid channel mode in AC4 presentation");
                        return ERROR_MALFORMED;
                }
            }

            ap.mAudioDescriptionAvailable = (presentation.mContentClassifier ==
                    AC4Parser::AC4Presentation::kVisuallyImpaired);
            ap.mSpokenSubtitlesAvailable = (presentation.mContentClassifier ==
                    AC4Parser::AC4Presentation::kVoiceOver);
            ap.mDialogueEnhancementAvailable = presentation.mHasDialogEnhancements;
            if (!ap.mLanguage.empty()) {
                ap.mLabels.emplace(ap.mLanguage, presentation.mDescription);
            }
            presentations.push_back(std::move(ap));
        }
    }

    if (presentations.empty()) {
        // Clear audio presentation info in metadata.
        AMediaFormat_setBuffer(
                mLastTrack->meta, AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO, nullptr, 0);
    } else {
        std::ostringstream outStream(std::ios::out);
        serializeAudioPresentations(presentations, &outStream);
        AMediaFormat_setBuffer(
                mLastTrack->meta, AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO,
                outStream.str().data(), outStream.str().size());
    }
    return OK;
}

status_t MPEG4Extractor::parseEAC3SpecificBox(off64_t offset) {
    if (mLastTrack == NULL) {
        return ERROR_MALFORMED;
    }

    uint16_t sampleRate, channels;
    status_t status;
    if ((status = parseChannelCountSampleRate(&offset, &channels, &sampleRate)) != OK) {
        return status;
    }
    uint32_t size;
    // + 4-byte size
    // + 4-byte type
    // + 3-byte payload
    const uint32_t kEAC3SpecificBoxMinSize = 11;
    // 13 + 3 + (8 * (2 + 5 + 5 + 3 + 1 + 3 + 4 + (14 * 9 + 1))) bits == 152 bytes theoretical max
    // calculated from the required bits read below as well as the maximum number of independent
    // and dependant sub streams you can have
    const uint32_t kEAC3SpecificBoxMaxSize = 152;
    if (!mDataSource->getUInt32(offset, &size) ||
        size < kEAC3SpecificBoxMinSize ||
        size > kEAC3SpecificBoxMaxSize) {
        ALOGE("MPEG4Extractor: error while reading eac-3 block: cannot read specific box size");
        return ERROR_MALFORMED;
    }

    offset += 4;
    uint32_t type;
    if (!mDataSource->getUInt32(offset, &type) || type != FOURCC("dec3")) {
        ALOGE("MPEG4Extractor: error while reading eac-3 specific block: header not dec3");
        return ERROR_MALFORMED;
    }

    offset += 4;
    uint8_t* chunk = new (std::nothrow) uint8_t[size];
    if (chunk == NULL) {
        return ERROR_MALFORMED;
    }

    if (mDataSource->readAt(offset, chunk, size) != (ssize_t)size) {
        ALOGE("MPEG4Extractor: error while reading eac-3 specific block: bitstream fields");
        delete[] chunk;
        return ERROR_MALFORMED;
    }

    ABitReader br(chunk, size);
    static const unsigned channelCountTable[] = {2, 1, 2, 3, 3, 4, 4, 5};
    static const unsigned sampleRateTable[] = {48000, 44100, 32000};

    if (br.numBitsLeft() < 16) {
        delete[] chunk;
        return ERROR_MALFORMED;
    }
    unsigned data_rate = br.getBits(13);
    ALOGV("EAC3 data rate = %d", data_rate);

    unsigned num_ind_sub = br.getBits(3) + 1;
    ALOGV("EAC3 independant substreams = %d", num_ind_sub);
    if (br.numBitsLeft() < (num_ind_sub * 23)) {
        delete[] chunk;
        return ERROR_MALFORMED;
    }

    unsigned channelCount = 0;
    for (unsigned i = 0; i < num_ind_sub; i++) {
        unsigned fscod = br.getBits(2);
        if (fscod == 3) {
            ALOGE("Incorrect fscod (3) in EAC3 header");
            delete[] chunk;
            return ERROR_MALFORMED;
        }
        unsigned boxSampleRate = sampleRateTable[fscod];
        if (boxSampleRate != sampleRate) {
            ALOGE("sample rate mismatch: boxSampleRate = %d, sampleRate = %d",
                boxSampleRate, sampleRate);
            delete[] chunk;
            return ERROR_MALFORMED;
        }

        unsigned bsid = br.getBits(5);
        if (bsid == 9 || bsid == 10) {
            ALOGW("EAC3 stream (bsid=%d) may be silenced by the decoder", bsid);
        } else if (bsid > 16) {
            ALOGE("EAC3 stream (bsid=%d) is not compatible with ETSI TS 102 366 v1.4.1", bsid);
            delete[] chunk;
            return ERROR_MALFORMED;
        }

        // skip
        br.skipBits(2);
        unsigned bsmod = br.getBits(3);
        unsigned acmod = br.getBits(3);
        unsigned lfeon = br.getBits(1);
        // we currently only support the first stream
        if (i == 0)
            channelCount = channelCountTable[acmod] + lfeon;
        ALOGV("bsmod = %d, acmod = %d, lfeon = %d", bsmod, acmod, lfeon);

        br.skipBits(3);
        unsigned num_dep_sub = br.getBits(4);
        ALOGV("EAC3 dependant substreams = %d", num_dep_sub);
        if (num_dep_sub != 0) {
            if (br.numBitsLeft() < 9) {
                delete[] chunk;
                return ERROR_MALFORMED;
            }
            static const char* chan_loc_tbl[] = { "Lc/Rc","Lrs/Rrs","Cs","Ts","Lsd/Rsd",
                "Lw/Rw","Lvh/Rvh","Cvh","Lfe2" };
            unsigned chan_loc = br.getBits(9);
            unsigned mask = 1;
            for (unsigned j = 0; j < 9; j++, mask <<= 1) {
                if ((chan_loc & mask) != 0) {
                    // we currently only support the first stream
                    if (i == 0) {
                        channelCount++;
                        // these are 2 channels in the mask
                        if (j == 0 || j == 1 || j == 4 || j == 5 || j == 6) {
                            channelCount++;
                        }
                    }
                    ALOGV(" %s", chan_loc_tbl[j]);
                }
            }
        } else {
            if (br.numBitsLeft() == 0) {
                delete[] chunk;
                return ERROR_MALFORMED;
            }
            br.skipBits(1);
        }
    }

    if (br.numBitsLeft() != 0) {
        if (br.numBitsLeft() < 8) {
            delete[] chunk;
            return ERROR_MALFORMED;
        }
        unsigned mask = br.getBits(8);
        for (unsigned i = 0; i < 8; i++) {
            if (((0x1 << i) && mask) == 0)
                continue;

            if (br.numBitsLeft() < 8) {
                delete[] chunk;
                return ERROR_MALFORMED;
            }
            switch (i) {
                case 0: {
                    unsigned complexity = br.getBits(8);
                    ALOGV("Found a JOC stream with complexity = %d", complexity);
                }break;
                default: {
                    br.skipBits(8);
                }break;
            }
        }
    }
    AMediaFormat_setString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_EAC3);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);

    delete[] chunk;
    return OK;
}

status_t MPEG4Extractor::parseAC3SpecificBox(off64_t offset) {
    if (mLastTrack == NULL) {
        return ERROR_MALFORMED;
    }

    uint16_t sampleRate, channels;
    status_t status;
    if ((status = parseChannelCountSampleRate(&offset, &channels, &sampleRate)) != OK) {
        return status;
    }
    uint32_t size;
    // + 4-byte size
    // + 4-byte type
    // + 3-byte payload
    const uint32_t kAC3SpecificBoxSize = 11;
    if (!mDataSource->getUInt32(offset, &size) || size < kAC3SpecificBoxSize) {
        ALOGE("MPEG4Extractor: error while reading ac-3 block: cannot read specific box size");
        return ERROR_MALFORMED;
    }

    offset += 4;
    uint32_t type;
    if (!mDataSource->getUInt32(offset, &type) || type != FOURCC("dac3")) {
        ALOGE("MPEG4Extractor: error while reading ac-3 specific block: header not dac3");
        return ERROR_MALFORMED;
    }

    offset += 4;
    const uint32_t kAC3SpecificBoxPayloadSize = 3;
    uint8_t chunk[kAC3SpecificBoxPayloadSize];
    if (mDataSource->readAt(offset, chunk, sizeof(chunk)) != sizeof(chunk)) {
        ALOGE("MPEG4Extractor: error while reading ac-3 specific block: bitstream fields");
        return ERROR_MALFORMED;
    }

    ABitReader br(chunk, sizeof(chunk));
    static const unsigned channelCountTable[] = {2, 1, 2, 3, 3, 4, 4, 5};
    static const unsigned sampleRateTable[] = {48000, 44100, 32000};

    unsigned fscod = br.getBits(2);
    if (fscod == 3) {
        ALOGE("Incorrect fscod (3) in AC3 header");
        return ERROR_MALFORMED;
    }
    unsigned boxSampleRate = sampleRateTable[fscod];
    if (boxSampleRate != sampleRate) {
        ALOGE("sample rate mismatch: boxSampleRate = %d, sampleRate = %d",
            boxSampleRate, sampleRate);
        return ERROR_MALFORMED;
    }

    unsigned bsid = br.getBits(5);
    if (bsid > 8) {
        ALOGW("Incorrect bsid in AC3 header. Possibly E-AC-3?");
        return ERROR_MALFORMED;
    }

    // skip
    unsigned bsmod __unused = br.getBits(3);

    unsigned acmod = br.getBits(3);
    unsigned lfeon = br.getBits(1);
    unsigned channelCount = channelCountTable[acmod] + lfeon;

    AMediaFormat_setString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_AC3);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);
    return OK;
}

status_t MPEG4Extractor::parseALACSampleEntry(off64_t *offset) {
    // See 'external/alac/ALACMagicCookieDescription.txt for the detail'.
    // Store ALAC magic cookie (decoder needs it).
    uint8_t alacInfo[12];
    off64_t data_offset = *offset;

    if (mDataSource->readAt(
            data_offset, alacInfo, sizeof(alacInfo)) < (ssize_t)sizeof(alacInfo)) {
        return ERROR_IO;
    }
    uint32_t size = U32_AT(&alacInfo[0]);
    if ((size != ALAC_SPECIFIC_INFO_SIZE) ||
            (U32_AT(&alacInfo[4]) != FOURCC("alac")) ||
            (U32_AT(&alacInfo[8]) != 0)) {
        ALOGV("Size:%u, U32_AT(&alacInfo[4]):%u, U32_AT(&alacInfo[8]):%u",
            size, U32_AT(&alacInfo[4]), U32_AT(&alacInfo[8]));
        return ERROR_MALFORMED;
    }
    data_offset += sizeof(alacInfo);
    uint8_t cookie[size - sizeof(alacInfo)];
    if (mDataSource->readAt(
            data_offset, cookie, sizeof(cookie)) < (ssize_t)sizeof(cookie)) {
        return ERROR_IO;
    }

    uint8_t bitsPerSample = cookie[5];
    AMediaFormat_setInt32(mLastTrack->meta,
            AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, bitsPerSample);
    AMediaFormat_setInt32(mLastTrack->meta,
            AMEDIAFORMAT_KEY_CHANNEL_COUNT, cookie[9]);
    AMediaFormat_setInt32(mLastTrack->meta,
            AMEDIAFORMAT_KEY_SAMPLE_RATE, U32_AT(&cookie[20]));
    AMediaFormat_setBuffer(mLastTrack->meta,
            AMEDIAFORMAT_KEY_CSD_0, cookie, sizeof(cookie));
    data_offset += sizeof(cookie);
    *offset = data_offset;
    return OK;
}

status_t MPEG4Extractor::parseSegmentIndex(off64_t offset, size_t size) {
  ALOGV("MPEG4Extractor::parseSegmentIndex");

    if (size < 12) {
      return -EINVAL;
    }

    uint32_t flags;
    if (!mDataSource->getUInt32(offset, &flags)) {
        return ERROR_MALFORMED;
    }

    uint32_t version = flags >> 24;
    flags &= 0xffffff;

    ALOGV("sidx version %d", version);

    uint32_t referenceId;
    if (!mDataSource->getUInt32(offset + 4, &referenceId)) {
        return ERROR_MALFORMED;
    }

    uint32_t timeScale;
    if (!mDataSource->getUInt32(offset + 8, &timeScale)) {
        return ERROR_MALFORMED;
    }
    ALOGV("sidx refid/timescale: %d/%d", referenceId, timeScale);
    if (timeScale == 0)
        return ERROR_MALFORMED;

    uint64_t earliestPresentationTime;
    uint64_t firstOffset;

    offset += 12;
    size -= 12;

    if (version == 0) {
        if (size < 8) {
            return -EINVAL;
        }
        uint32_t tmp;
        if (!mDataSource->getUInt32(offset, &tmp)) {
            return ERROR_MALFORMED;
        }
        earliestPresentationTime = tmp;
        if (!mDataSource->getUInt32(offset + 4, &tmp)) {
            return ERROR_MALFORMED;
        }
        firstOffset = tmp;
        offset += 8;
        size -= 8;
    } else {
        if (size < 16) {
            return -EINVAL;
        }
        if (!mDataSource->getUInt64(offset, &earliestPresentationTime)) {
            return ERROR_MALFORMED;
        }
        if (!mDataSource->getUInt64(offset + 8, &firstOffset)) {
            return ERROR_MALFORMED;
        }
        offset += 16;
        size -= 16;
    }
    ALOGV("sidx pres/off: %" PRIu64 "/%" PRIu64, earliestPresentationTime, firstOffset);

    if (size < 4) {
        return -EINVAL;
    }

    uint16_t referenceCount;
    if (!mDataSource->getUInt16(offset + 2, &referenceCount)) {
        return ERROR_MALFORMED;
    }
    offset += 4;
    size -= 4;
    ALOGV("refcount: %d", referenceCount);

    if (size < referenceCount * 12) {
        return -EINVAL;
    }

    uint64_t total_duration = 0;
    for (unsigned int i = 0; i < referenceCount; i++) {
        uint32_t d1, d2, d3;

        if (!mDataSource->getUInt32(offset, &d1) ||     // size
            !mDataSource->getUInt32(offset + 4, &d2) || // duration
            !mDataSource->getUInt32(offset + 8, &d3)) { // flags
            return ERROR_MALFORMED;
        }

        if (d1 & 0x80000000) {
            ALOGW("sub-sidx boxes not supported yet");
        }
        bool sap = d3 & 0x80000000;
        uint32_t saptype = (d3 >> 28) & 7;
        if (!sap || (saptype != 1 && saptype != 2)) {
            // type 1 and 2 are sync samples
            ALOGW("not a stream access point, or unsupported type: %08x", d3);
        }
        total_duration += d2;
        offset += 12;
        ALOGV(" item %d, %08x %08x %08x", i, d1, d2, d3);
        SidxEntry se;
        se.mSize = d1 & 0x7fffffff;
        se.mDurationUs = 1000000LL * d2 / timeScale;
        mSidxEntries.add(se);
    }

    uint64_t sidxDuration = total_duration * 1000000 / timeScale;

    if (mLastTrack == NULL)
        return ERROR_MALFORMED;

    int64_t metaDuration;
    if (!AMediaFormat_getInt64(mLastTrack->meta,
                AMEDIAFORMAT_KEY_DURATION, &metaDuration) || metaDuration == 0) {
        AMediaFormat_setInt64(mLastTrack->meta, AMEDIAFORMAT_KEY_DURATION, sidxDuration);
    }
    return OK;
}

status_t MPEG4Extractor::parseQTMetaKey(off64_t offset, size_t size) {
    if (size < 8) {
        return ERROR_MALFORMED;
    }

    uint32_t count;
    if (!mDataSource->getUInt32(offset + 4, &count)) {
        return ERROR_MALFORMED;
    }

    if (mMetaKeyMap.size() > 0) {
        ALOGW("'keys' atom seen again, discarding existing entries");
        mMetaKeyMap.clear();
    }

    off64_t keyOffset = offset + 8;
    off64_t stopOffset = offset + size;
    for (size_t i = 1; i <= count; i++) {
        if (keyOffset + 8 > stopOffset) {
            return ERROR_MALFORMED;
        }

        uint32_t keySize;
        if (!mDataSource->getUInt32(keyOffset, &keySize)
                || keySize < 8
                || keyOffset + keySize > stopOffset) {
            return ERROR_MALFORMED;
        }

        uint32_t type;
        if (!mDataSource->getUInt32(keyOffset + 4, &type)
                || type != FOURCC("mdta")) {
            return ERROR_MALFORMED;
        }

        keySize -= 8;
        keyOffset += 8;

        auto keyData = heapbuffer<uint8_t>(keySize);
        if (keyData.get() == NULL) {
            return ERROR_MALFORMED;
        }
        if (mDataSource->readAt(
                keyOffset, keyData.get(), keySize) < (ssize_t) keySize) {
            return ERROR_MALFORMED;
        }

        AString key((const char *)keyData.get(), keySize);
        mMetaKeyMap.add(i, key);

        keyOffset += keySize;
    }
    return OK;
}

status_t MPEG4Extractor::parseQTMetaVal(
        int32_t keyId, off64_t offset, size_t size) {
    ssize_t index = mMetaKeyMap.indexOfKey(keyId);
    if (index < 0) {
        // corresponding key is not present, ignore
        return ERROR_MALFORMED;
    }

    if (size <= 16) {
        return ERROR_MALFORMED;
    }
    uint32_t dataSize;
    if (!mDataSource->getUInt32(offset, &dataSize)
            || dataSize > size || dataSize <= 16) {
        return ERROR_MALFORMED;
    }
    uint32_t atomFourCC;
    if (!mDataSource->getUInt32(offset + 4, &atomFourCC)
            || atomFourCC != FOURCC("data")) {
        return ERROR_MALFORMED;
    }
    uint32_t dataType;
    if (!mDataSource->getUInt32(offset + 8, &dataType)
            || ((dataType & 0xff000000) != 0)) {
        // not well-known type
        return ERROR_MALFORMED;
    }

    dataSize -= 16;
    offset += 16;

    if (dataType == 23 && dataSize >= 4) {
        // BE Float32
        uint32_t val;
        if (!mDataSource->getUInt32(offset, &val)) {
            return ERROR_MALFORMED;
        }
        if (!strcasecmp(mMetaKeyMap[index].c_str(), "com.android.capture.fps")) {
            AMediaFormat_setFloat(mFileMetaData, AMEDIAFORMAT_KEY_CAPTURE_RATE, *(float *)&val);
        }
    } else if (dataType == 67 && dataSize >= 4) {
        // BE signed int32
        uint32_t val;
        if (!mDataSource->getUInt32(offset, &val)) {
            return ERROR_MALFORMED;
        }
        if (!strcasecmp(mMetaKeyMap[index].c_str(), "com.android.video.temporal_layers_count")) {
            AMediaFormat_setInt32(mFileMetaData,
                    AMEDIAFORMAT_KEY_TEMPORAL_LAYER_COUNT, val);
        }
    } else {
        // add more keys if needed
        ALOGV("ignoring key: type %d, size %d", dataType, dataSize);
    }

    return OK;
}

status_t MPEG4Extractor::parseTrackHeader(
        off64_t data_offset, off64_t data_size) {
    if (data_size < 4) {
        return ERROR_MALFORMED;
    }

    uint8_t version;
    if (mDataSource->readAt(data_offset, &version, 1) < 1) {
        return ERROR_IO;
    }

    size_t dynSize = (version == 1) ? 36 : 24;

    uint8_t buffer[36 + 60];

    if (data_size != (off64_t)dynSize + 60) {
        return ERROR_MALFORMED;
    }

    if (mDataSource->readAt(
                data_offset, buffer, data_size) < (ssize_t)data_size) {
        return ERROR_IO;
    }

    uint64_t ctime __unused, mtime __unused, duration __unused;
    int32_t id;

    if (version == 1) {
        ctime = U64_AT(&buffer[4]);
        mtime = U64_AT(&buffer[12]);
        id = U32_AT(&buffer[20]);
        duration = U64_AT(&buffer[28]);
    } else if (version == 0) {
        ctime = U32_AT(&buffer[4]);
        mtime = U32_AT(&buffer[8]);
        id = U32_AT(&buffer[12]);
        duration = U32_AT(&buffer[20]);
    } else {
        return ERROR_UNSUPPORTED;
    }

    if (mLastTrack == NULL)
        return ERROR_MALFORMED;

    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_TRACK_ID, id);

    size_t matrixOffset = dynSize + 16;
    int32_t a00 = U32_AT(&buffer[matrixOffset]);
    int32_t a01 = U32_AT(&buffer[matrixOffset + 4]);
    int32_t a10 = U32_AT(&buffer[matrixOffset + 12]);
    int32_t a11 = U32_AT(&buffer[matrixOffset + 16]);

#if 0
    int32_t dx = U32_AT(&buffer[matrixOffset + 8]);
    int32_t dy = U32_AT(&buffer[matrixOffset + 20]);

    ALOGI("x' = %.2f * x + %.2f * y + %.2f",
         a00 / 65536.0f, a01 / 65536.0f, dx / 65536.0f);
    ALOGI("y' = %.2f * x + %.2f * y + %.2f",
         a10 / 65536.0f, a11 / 65536.0f, dy / 65536.0f);
#endif

    uint32_t rotationDegrees;

    static const int32_t kFixedOne = 0x10000;
    if (a00 == kFixedOne && a01 == 0 && a10 == 0 && a11 == kFixedOne) {
        // Identity, no rotation
        rotationDegrees = 0;
    } else if (a00 == 0 && a01 == kFixedOne && a10 == -kFixedOne && a11 == 0) {
        rotationDegrees = 90;
    } else if (a00 == 0 && a01 == -kFixedOne && a10 == kFixedOne && a11 == 0) {
        rotationDegrees = 270;
    } else if (a00 == -kFixedOne && a01 == 0 && a10 == 0 && a11 == -kFixedOne) {
        rotationDegrees = 180;
    } else {
        ALOGW("We only support 0,90,180,270 degree rotation matrices");
        rotationDegrees = 0;
    }

    if (rotationDegrees != 0) {
        AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_ROTATION, rotationDegrees);
    }

    // Handle presentation display size, which could be different
    // from the image size indicated by AMEDIAFORMAT_KEY_WIDTH and AMEDIAFORMAT_KEY_HEIGHT.
    uint32_t width = U32_AT(&buffer[dynSize + 52]);
    uint32_t height = U32_AT(&buffer[dynSize + 56]);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_DISPLAY_WIDTH, width >> 16);
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_DISPLAY_HEIGHT, height >> 16);

    return OK;
}

status_t MPEG4Extractor::parseITunesMetaData(off64_t offset, size_t size) {
    if (size == 0) {
        return OK;
    }

    if (size < 4 || size == SIZE_MAX) {
        return ERROR_MALFORMED;
    }

    uint8_t *buffer = new (std::nothrow) uint8_t[size + 1];
    if (buffer == NULL) {
        return ERROR_MALFORMED;
    }
    if (mDataSource->readAt(
                offset, buffer, size) != (ssize_t)size) {
        delete[] buffer;
        buffer = NULL;

        return ERROR_IO;
    }

    uint32_t flags = U32_AT(buffer);

    const char *metadataKey = nullptr;
    char chunk[5];
    MakeFourCCString(mPath[4], chunk);
    ALOGV("meta: %s @ %lld", chunk, (long long)offset);
    switch ((int32_t)mPath[4]) {
        case FOURCC("\251alb"):
        {
            metadataKey = "album";
            break;
        }
        case FOURCC("\251ART"):
        {
            metadataKey = "artist";
            break;
        }
        case FOURCC("aART"):
        {
            metadataKey = "albumartist";
            break;
        }
        case FOURCC("\251day"):
        {
            metadataKey = "year";
            break;
        }
        case FOURCC("\251nam"):
        {
            metadataKey = "title";
            break;
        }
        case FOURCC("\251wrt"):
        {
            metadataKey = "writer";
            break;
        }
        case FOURCC("covr"):
        {
            metadataKey = "albumart";
            break;
        }
        case FOURCC("gnre"):
        case FOURCC("\251gen"):
        {
            metadataKey = "genre";
            break;
        }
        case FOURCC("cpil"):
        {
            if (size == 9 && flags == 21) {
                char tmp[16];
                sprintf(tmp, "%d",
                        (int)buffer[size - 1]);

                AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_COMPILATION, tmp);
            }
            break;
        }
        case FOURCC("trkn"):
        {
            if (size == 16 && flags == 0) {
                char tmp[16];
                uint16_t* pTrack = (uint16_t*)&buffer[10];
                uint16_t* pTotalTracks = (uint16_t*)&buffer[12];
                sprintf(tmp, "%d/%d", ntohs(*pTrack), ntohs(*pTotalTracks));

                AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_CDTRACKNUMBER, tmp);
            }
            break;
        }
        case FOURCC("disk"):
        {
            if ((size == 14 || size == 16) && flags == 0) {
                char tmp[16];
                uint16_t* pDisc = (uint16_t*)&buffer[10];
                uint16_t* pTotalDiscs = (uint16_t*)&buffer[12];
                sprintf(tmp, "%d/%d", ntohs(*pDisc), ntohs(*pTotalDiscs));

                AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_DISCNUMBER, tmp);
            }
            break;
        }
        case FOURCC("----"):
        {
            buffer[size] = '\0';
            switch (mPath[5]) {
                case FOURCC("mean"):
                    mLastCommentMean.setTo((const char *)buffer + 4);
                    break;
                case FOURCC("name"):
                    mLastCommentName.setTo((const char *)buffer + 4);
                    break;
                case FOURCC("data"):
                    if (size < 8) {
                        delete[] buffer;
                        buffer = NULL;
                        ALOGE("b/24346430");
                        return ERROR_MALFORMED;
                    }
                    mLastCommentData.setTo((const char *)buffer + 8);
                    break;
            }

            // Once we have a set of mean/name/data info, go ahead and process
            // it to see if its something we are interested in.  Whether or not
            // were are interested in the specific tag, make sure to clear out
            // the set so we can be ready to process another tuple should one
            // show up later in the file.
            if ((mLastCommentMean.length() != 0) &&
                (mLastCommentName.length() != 0) &&
                (mLastCommentData.length() != 0)) {

                if (mLastCommentMean == "com.apple.iTunes"
                        && mLastCommentName == "iTunSMPB") {
                    int32_t delay, padding;
                    if (sscanf(mLastCommentData,
                               " %*x %x %x %*x", &delay, &padding) == 2) {
                        if (mLastTrack == NULL) {
                            delete[] buffer;
                            return ERROR_MALFORMED;
                        }

                        AMediaFormat_setInt32(mLastTrack->meta,
                                AMEDIAFORMAT_KEY_ENCODER_DELAY, delay);
                        AMediaFormat_setInt32(mLastTrack->meta,
                                AMEDIAFORMAT_KEY_ENCODER_PADDING, padding);
                    }
                }

                mLastCommentMean.clear();
                mLastCommentName.clear();
                mLastCommentData.clear();
            }
            break;
        }

        default:
            break;
    }

    void *tmpData;
    size_t tmpDataSize;
    const char *s;
    if (size >= 8 && metadataKey &&
            !AMediaFormat_getBuffer(mFileMetaData, metadataKey, &tmpData, &tmpDataSize) &&
            !AMediaFormat_getString(mFileMetaData, metadataKey, &s)) {
        if (!strcmp(metadataKey, "albumart")) {
            AMediaFormat_setBuffer(mFileMetaData, metadataKey,
                    buffer + 8, size - 8);
        } else if (!strcmp(metadataKey, "genre")) {
            if (flags == 0) {
                // uint8_t genre code, iTunes genre codes are
                // the standard id3 codes, except they start
                // at 1 instead of 0 (e.g. Pop is 14, not 13)
                // We use standard id3 numbering, so subtract 1.
                int genrecode = (int)buffer[size - 1];
                genrecode--;
                if (genrecode < 0) {
                    genrecode = 255; // reserved for 'unknown genre'
                }
                char genre[10];
                sprintf(genre, "%d", genrecode);

                AMediaFormat_setString(mFileMetaData, metadataKey, genre);
            } else if (flags == 1) {
                // custom genre string
                buffer[size] = '\0';

                AMediaFormat_setString(mFileMetaData, 
                        metadataKey, (const char *)buffer + 8);
            }
        } else {
            buffer[size] = '\0';

            AMediaFormat_setString(mFileMetaData, 
                    metadataKey, (const char *)buffer + 8);
        }
    }

    delete[] buffer;
    buffer = NULL;

    return OK;
}

status_t MPEG4Extractor::parseColorInfo(off64_t offset, size_t size) {
    if (size < 4 || size == SIZE_MAX || mLastTrack == NULL) {
        return ERROR_MALFORMED;
    }

    uint8_t *buffer = new (std::nothrow) uint8_t[size + 1];
    if (buffer == NULL) {
        return ERROR_MALFORMED;
    }
    if (mDataSource->readAt(offset, buffer, size) != (ssize_t)size) {
        delete[] buffer;
        buffer = NULL;

        return ERROR_IO;
    }

    int32_t type = U32_AT(&buffer[0]);
    if ((type == FOURCC("nclx") && size >= 11)
            || (type == FOURCC("nclc") && size >= 10)) {
        // only store the first color specification
        int32_t existingColor;
        if (!AMediaFormat_getInt32(mLastTrack->meta,
                AMEDIAFORMAT_KEY_COLOR_RANGE, &existingColor)) {
            int32_t primaries = U16_AT(&buffer[4]);
            int32_t isotransfer = U16_AT(&buffer[6]);
            int32_t coeffs = U16_AT(&buffer[8]);
            bool fullRange = (type == FOURCC("nclx")) && (buffer[10] & 128);

            int32_t range = 0;
            int32_t standard = 0;
            int32_t transfer = 0;
            ColorUtils::convertIsoColorAspectsToPlatformAspects(
                    primaries, isotransfer, coeffs, fullRange,
                    &range, &standard, &transfer);

            if (range != 0) {
                AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_COLOR_RANGE, range);
            }
            if (standard != 0) {
                AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_COLOR_STANDARD, standard);
            }
            if (transfer != 0) {
                AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_COLOR_TRANSFER, transfer);
            }
        }
    }

    delete[] buffer;
    buffer = NULL;

    return OK;
}

status_t MPEG4Extractor::parse3GPPMetaData(off64_t offset, size_t size, int depth) {
    if (size < 4 || size == SIZE_MAX) {
        return ERROR_MALFORMED;
    }

    uint8_t *buffer = new (std::nothrow) uint8_t[size + 1];
    if (buffer == NULL) {
        return ERROR_MALFORMED;
    }
    if (mDataSource->readAt(
                offset, buffer, size) != (ssize_t)size) {
        delete[] buffer;
        buffer = NULL;

        return ERROR_IO;
    }

    const char *metadataKey = nullptr;
    switch (mPath[depth]) {
        case FOURCC("titl"):
        {
            metadataKey = "title";
            break;
        }
        case FOURCC("perf"):
        {
            metadataKey = "artist";
            break;
        }
        case FOURCC("auth"):
        {
            metadataKey = "writer";
            break;
        }
        case FOURCC("gnre"):
        {
            metadataKey = "genre";
            break;
        }
        case FOURCC("albm"):
        {
            if (buffer[size - 1] != '\0') {
              char tmp[4];
              sprintf(tmp, "%u", buffer[size - 1]);

              AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_CDTRACKNUMBER, tmp);
            }

            metadataKey = "album";
            break;
        }
        case FOURCC("yrrc"):
        {
            if (size < 6) {
                delete[] buffer;
                buffer = NULL;
                ALOGE("b/62133227");
                android_errorWriteLog(0x534e4554, "62133227");
                return ERROR_MALFORMED;
            }
            char tmp[5];
            uint16_t year = U16_AT(&buffer[4]);

            if (year < 10000) {
                sprintf(tmp, "%u", year);

                AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_YEAR, tmp);
            }
            break;
        }

        default:
            break;
    }

    if (metadataKey) {
        bool isUTF8 = true; // Common case
        char16_t *framedata = NULL;
        int len16 = 0; // Number of UTF-16 characters

        // smallest possible valid UTF-16 string w BOM: 0xfe 0xff 0x00 0x00
        if (size < 6) {
            delete[] buffer;
            buffer = NULL;
            return ERROR_MALFORMED;
        }

        if (size - 6 >= 4) {
            len16 = ((size - 6) / 2) - 1; // don't include 0x0000 terminator
            framedata = (char16_t *)(buffer + 6);
            if (0xfffe == *framedata) {
                // endianness marker (BOM) doesn't match host endianness
                for (int i = 0; i < len16; i++) {
                    framedata[i] = bswap_16(framedata[i]);
                }
                // BOM is now swapped to 0xfeff, we will execute next block too
            }

            if (0xfeff == *framedata) {
                // Remove the BOM
                framedata++;
                len16--;
                isUTF8 = false;
            }
            // else normal non-zero-length UTF-8 string
            // we can't handle UTF-16 without BOM as there is no other
            // indication of encoding.
        }

        if (isUTF8) {
            buffer[size] = 0;
            AMediaFormat_setString(mFileMetaData, metadataKey, (const char *)buffer + 6);
        } else {
            // Convert from UTF-16 string to UTF-8 string.
            String8 tmpUTF8str(framedata, len16);
            AMediaFormat_setString(mFileMetaData, metadataKey, tmpUTF8str.string());
        }
    }

    delete[] buffer;
    buffer = NULL;

    return OK;
}

void MPEG4Extractor::parseID3v2MetaData(off64_t offset) {
    ID3 id3(mDataSource, true /* ignorev1 */, offset);

    if (id3.isValid()) {
        struct Map {
            const char *key;
            const char *tag1;
            const char *tag2;
        };
        static const Map kMap[] = {
            { AMEDIAFORMAT_KEY_ALBUM, "TALB", "TAL" },
            { AMEDIAFORMAT_KEY_ARTIST, "TPE1", "TP1" },
            { AMEDIAFORMAT_KEY_ALBUMARTIST, "TPE2", "TP2" },
            { AMEDIAFORMAT_KEY_COMPOSER, "TCOM", "TCM" },
            { AMEDIAFORMAT_KEY_GENRE, "TCON", "TCO" },
            { AMEDIAFORMAT_KEY_TITLE, "TIT2", "TT2" },
            { AMEDIAFORMAT_KEY_YEAR, "TYE", "TYER" },
            { AMEDIAFORMAT_KEY_AUTHOR, "TXT", "TEXT" },
            { AMEDIAFORMAT_KEY_CDTRACKNUMBER, "TRK", "TRCK" },
            { AMEDIAFORMAT_KEY_DISCNUMBER, "TPA", "TPOS" },
            { AMEDIAFORMAT_KEY_COMPILATION, "TCP", "TCMP" },
        };
        static const size_t kNumMapEntries = sizeof(kMap) / sizeof(kMap[0]);

        for (size_t i = 0; i < kNumMapEntries; ++i) {
            const char *ss;
            if (!AMediaFormat_getString(mFileMetaData, kMap[i].key, &ss)) {
                ID3::Iterator *it = new ID3::Iterator(id3, kMap[i].tag1);
                if (it->done()) {
                    delete it;
                    it = new ID3::Iterator(id3, kMap[i].tag2);
                }

                if (it->done()) {
                    delete it;
                    continue;
                }

                String8 s;
                it->getString(&s);
                delete it;

                AMediaFormat_setString(mFileMetaData, kMap[i].key, s);
            }
        }

        size_t dataSize;
        String8 mime;
        const void *data = id3.getAlbumArt(&dataSize, &mime);

        if (data) {
            AMediaFormat_setBuffer(mFileMetaData, AMEDIAFORMAT_KEY_ALBUMART, data, dataSize);
        }
    }
}

MediaTrackHelper *MPEG4Extractor::getTrack(size_t index) {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return NULL;
    }

    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL) {
            return NULL;
        }

        track = track->next;
        --index;
    }

    if (track == NULL) {
        return NULL;
    }


    Trex *trex = NULL;
    int32_t trackId;
    if (AMediaFormat_getInt32(track->meta, AMEDIAFORMAT_KEY_TRACK_ID, &trackId)) {
        for (size_t i = 0; i < mTrex.size(); i++) {
            Trex *t = &mTrex.editItemAt(i);
            if (t->track_ID == (uint32_t) trackId) {
                trex = t;
                break;
            }
        }
    } else {
        ALOGE("b/21657957");
        return NULL;
    }

    ALOGV("getTrack called, pssh: %zu", mPssh.size());

    const char *mime;
    if (!AMediaFormat_getString(track->meta, AMEDIAFORMAT_KEY_MIME, &mime)) {
        return NULL;
    }

    sp<ItemTable> itemTable;
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
        void *data;
        size_t size;
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_AVC, &data, &size)) {
            return NULL;
        }

        const uint8_t *ptr = (const uint8_t *)data;

        if (size < 7 || ptr[0] != 1) {  // configurationVersion == 1
            return NULL;
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)
            || !strcasecmp(mime, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC)) {
        void *data;
        size_t size;
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_HEVC, &data, &size)) {
            return NULL;
        }

        const uint8_t *ptr = (const uint8_t *)data;

        if (size < 22 || ptr[0] != 1) {  // configurationVersion == 1
            return NULL;
        }
        if (!strcasecmp(mime, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC)) {
            itemTable = mItemTable;
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AV1)) {
        void *data;
        size_t size;
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_0, &data, &size)) {
            return NULL;
        }

        const uint8_t *ptr = (const uint8_t *)data;

        if (size < 5 || ptr[0] != 0x81) {  // configurationVersion == 1
            return NULL;
        }
    }

    if (track->has_elst and !strncasecmp("video/", mime, 6) and track->elst_media_time > 0) {
        track->elstShiftStartTicks = track->elst_media_time;
        ALOGV("video track->elstShiftStartTicks :%" PRIu64, track->elstShiftStartTicks);
    }

    MPEG4Source *source =  new MPEG4Source(
            track->meta, mDataSource, track->timescale, track->sampleTable,
            mSidxEntries, trex, mMoofOffset, itemTable,
            track->elstShiftStartTicks);
    if (source->init() != OK) {
        delete source;
        return NULL;
    }
    return source;
}

// static
status_t MPEG4Extractor::verifyTrack(Track *track) {
    const char *mime;
    CHECK(AMediaFormat_getString(track->meta, AMEDIAFORMAT_KEY_MIME, &mime));

    void *data;
    size_t size;
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_AVC, &data, &size)) {
            return ERROR_MALFORMED;
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)) {
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_HEVC, &data, &size)) {
            return ERROR_MALFORMED;
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AV1)) {
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_CSD_0, &data, &size)) {
            return ERROR_MALFORMED;
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG2)
            || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        if (!AMediaFormat_getBuffer(track->meta, AMEDIAFORMAT_KEY_ESDS, &data, &size)) {
            return ERROR_MALFORMED;
        }
    }

    if (track->sampleTable == NULL || !track->sampleTable->isValid()) {
        // Make sure we have all the metadata we need.
        ALOGE("stbl atom missing/invalid.");
        return ERROR_MALFORMED;
    }

    if (track->timescale == 0) {
        ALOGE("timescale invalid.");
        return ERROR_MALFORMED;
    }

    return OK;
}

typedef enum {
    //AOT_NONE             = -1,
    //AOT_NULL_OBJECT      = 0,
    //AOT_AAC_MAIN         = 1, /**< Main profile                              */
    AOT_AAC_LC           = 2,   /**< Low Complexity object                     */
    //AOT_AAC_SSR          = 3,
    //AOT_AAC_LTP          = 4,
    AOT_SBR              = 5,
    //AOT_AAC_SCAL         = 6,
    //AOT_TWIN_VQ          = 7,
    //AOT_CELP             = 8,
    //AOT_HVXC             = 9,
    //AOT_RSVD_10          = 10, /**< (reserved)                                */
    //AOT_RSVD_11          = 11, /**< (reserved)                                */
    //AOT_TTSI             = 12, /**< TTSI Object                               */
    //AOT_MAIN_SYNTH       = 13, /**< Main Synthetic object                     */
    //AOT_WAV_TAB_SYNTH    = 14, /**< Wavetable Synthesis object                */
    //AOT_GEN_MIDI         = 15, /**< General MIDI object                       */
    //AOT_ALG_SYNTH_AUD_FX = 16, /**< Algorithmic Synthesis and Audio FX object */
    AOT_ER_AAC_LC        = 17,   /**< Error Resilient(ER) AAC Low Complexity    */
    //AOT_RSVD_18          = 18, /**< (reserved)                                */
    //AOT_ER_AAC_LTP       = 19, /**< Error Resilient(ER) AAC LTP object        */
    AOT_ER_AAC_SCAL      = 20,   /**< Error Resilient(ER) AAC Scalable object   */
    //AOT_ER_TWIN_VQ       = 21, /**< Error Resilient(ER) TwinVQ object         */
    AOT_ER_BSAC          = 22,   /**< Error Resilient(ER) BSAC object           */
    AOT_ER_AAC_LD        = 23,   /**< Error Resilient(ER) AAC LowDelay object   */
    //AOT_ER_CELP          = 24, /**< Error Resilient(ER) CELP object           */
    //AOT_ER_HVXC          = 25, /**< Error Resilient(ER) HVXC object           */
    //AOT_ER_HILN          = 26, /**< Error Resilient(ER) HILN object           */
    //AOT_ER_PARA          = 27, /**< Error Resilient(ER) Parametric object     */
    //AOT_RSVD_28          = 28, /**< might become SSC                          */
    AOT_PS               = 29,   /**< PS, Parametric Stereo (includes SBR)      */
    //AOT_MPEGS            = 30, /**< MPEG Surround                             */

    AOT_ESCAPE           = 31,   /**< Signal AOT uses more than 5 bits          */

    //AOT_MP3ONMP4_L1      = 32, /**< MPEG-Layer1 in mp4                        */
    //AOT_MP3ONMP4_L2      = 33, /**< MPEG-Layer2 in mp4                        */
    //AOT_MP3ONMP4_L3      = 34, /**< MPEG-Layer3 in mp4                        */
    //AOT_RSVD_35          = 35, /**< might become DST                          */
    //AOT_RSVD_36          = 36, /**< might become ALS                          */
    //AOT_AAC_SLS          = 37, /**< AAC + SLS                                 */
    //AOT_SLS              = 38, /**< SLS                                       */
    //AOT_ER_AAC_ELD       = 39, /**< AAC Enhanced Low Delay                    */

    //AOT_USAC             = 42, /**< USAC                                      */
    //AOT_SAOC             = 43, /**< SAOC                                      */
    //AOT_LD_MPEGS         = 44, /**< Low Delay MPEG Surround                   */

    //AOT_RSVD50           = 50,  /**< Interim AOT for Rsvd50                   */
} AUDIO_OBJECT_TYPE;

status_t MPEG4Extractor::updateAudioTrackInfoFromESDS_MPEG4Audio(
        const void *esds_data, size_t esds_size) {
    ESDS esds(esds_data, esds_size);

    uint8_t objectTypeIndication;
    if (esds.getObjectTypeIndication(&objectTypeIndication) != OK) {
        return ERROR_MALFORMED;
    }

    if (objectTypeIndication == 0xe1) {
        // This isn't MPEG4 audio at all, it's QCELP 14k...
        if (mLastTrack == NULL)
            return ERROR_MALFORMED;

        AMediaFormat_setString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_QCELP);
        return OK;
    }

    if (objectTypeIndication == 0x6B || objectTypeIndication == 0x69) {
        // mp3 audio
        AMediaFormat_setString(mLastTrack->meta,AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG);
        return OK;
    }

    if (mLastTrack != NULL) {
        uint32_t maxBitrate = 0;
        uint32_t avgBitrate = 0;
        esds.getBitRate(&maxBitrate, &avgBitrate);
        if (maxBitrate > 0 && maxBitrate < INT32_MAX) {
            AMediaFormat_setInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_MAX_BIT_RATE, (int32_t)maxBitrate);
        }
        if (avgBitrate > 0 && avgBitrate < INT32_MAX) {
            AMediaFormat_setInt32(mLastTrack->meta,
                    AMEDIAFORMAT_KEY_BIT_RATE, (int32_t)avgBitrate);
        }
    }

    const uint8_t *csd;
    size_t csd_size;
    if (esds.getCodecSpecificInfo(
                (const void **)&csd, &csd_size) != OK) {
        return ERROR_MALFORMED;
    }

    if (kUseHexDump) {
        printf("ESD of size %zu\n", csd_size);
        hexdump(csd, csd_size);
    }

    if (csd_size == 0) {
        // There's no further information, i.e. no codec specific data
        // Let's assume that the information provided in the mpeg4 headers
        // is accurate and hope for the best.

        return OK;
    }

    if (csd_size < 2) {
        return ERROR_MALFORMED;
    }

    if (objectTypeIndication == 0xdd) {
        // vorbis audio
        if (csd[0] != 0x02) {
            return ERROR_MALFORMED;
        }

        // codecInfo starts with two lengths, len1 and len2, that are
        // "Xiph-style-lacing encoded"..

        size_t offset = 1;
        size_t len1 = 0;
        while (offset < csd_size && csd[offset] == 0xff) {
            if (__builtin_add_overflow(len1, 0xff, &len1)) {
                return ERROR_MALFORMED;
            }
            ++offset;
        }
        if (offset >= csd_size) {
            return ERROR_MALFORMED;
        }
        if (__builtin_add_overflow(len1, csd[offset], &len1)) {
            return ERROR_MALFORMED;
        }
        ++offset;
        if (len1 == 0) {
            return ERROR_MALFORMED;
        }

        size_t len2 = 0;
        while (offset < csd_size && csd[offset] == 0xff) {
            if (__builtin_add_overflow(len2, 0xff, &len2)) {
                return ERROR_MALFORMED;
            }
            ++offset;
        }
        if (offset >= csd_size) {
            return ERROR_MALFORMED;
        }
        if (__builtin_add_overflow(len2, csd[offset], &len2)) {
            return ERROR_MALFORMED;
        }
        ++offset;
        if (len2 == 0) {
            return ERROR_MALFORMED;
        }
        if (offset >= csd_size || csd[offset] != 0x01) {
            return ERROR_MALFORMED;
        }
        // formerly kKeyVorbisInfo
        AMediaFormat_setBuffer(mLastTrack->meta,
                AMEDIAFORMAT_KEY_CSD_0, &csd[offset], len1);

        if (__builtin_add_overflow(offset, len1, &offset) ||
                offset >= csd_size || csd[offset] != 0x03) {
            return ERROR_MALFORMED;
        }

        if (__builtin_add_overflow(offset, len2, &offset) ||
                offset >= csd_size || csd[offset] != 0x05) {
            return ERROR_MALFORMED;
        }

        // formerly kKeyVorbisBooks
        AMediaFormat_setBuffer(mLastTrack->meta,
                AMEDIAFORMAT_KEY_CSD_1, &csd[offset], csd_size - offset);
        AMediaFormat_setString(mLastTrack->meta,
                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_VORBIS);

        return OK;
    }

    static uint32_t kSamplingRate[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
        16000, 12000, 11025, 8000, 7350
    };

    ABitReader br(csd, csd_size);
    uint32_t objectType = br.getBits(5);

    if (objectType == 31) {  // AAC-ELD => additional 6 bits
        objectType = 32 + br.getBits(6);
    }

    if (mLastTrack == NULL)
        return ERROR_MALFORMED;

    //keep AOT type
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_AAC_PROFILE, objectType);

    uint32_t freqIndex = br.getBits(4);

    int32_t sampleRate = 0;
    int32_t numChannels = 0;
    if (freqIndex == 15) {
        if (br.numBitsLeft() < 28) return ERROR_MALFORMED;
        sampleRate = br.getBits(24);
        numChannels = br.getBits(4);
    } else {
        if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
        numChannels = br.getBits(4);

        if (freqIndex == 13 || freqIndex == 14) {
            return ERROR_MALFORMED;
        }

        sampleRate = kSamplingRate[freqIndex];
    }

    if (objectType == AOT_SBR || objectType == AOT_PS) {//SBR specific config per 14496-3 tbl 1.13
        if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
        uint32_t extFreqIndex = br.getBits(4);
        int32_t extSampleRate __unused;
        if (extFreqIndex == 15) {
            if (csd_size < 8) {
                return ERROR_MALFORMED;
            }
            if (br.numBitsLeft() < 24) return ERROR_MALFORMED;
            extSampleRate = br.getBits(24);
        } else {
            if (extFreqIndex == 13 || extFreqIndex == 14) {
                return ERROR_MALFORMED;
            }
            extSampleRate = kSamplingRate[extFreqIndex];
        }
        //TODO: save the extension sampling rate value in meta data =>
        //      AMediaFormat_setInt32(mLastTrack->meta, kKeyExtSampleRate, extSampleRate);
    }

    switch (numChannels) {
        // values defined in 14496-3_2009 amendment-4 Table 1.19 - Channel Configuration
        case 0:
        case 1:// FC
        case 2:// FL FR
        case 3:// FC, FL FR
        case 4:// FC, FL FR, RC
        case 5:// FC, FL FR, SL SR
        case 6:// FC, FL FR, SL SR, LFE
            //numChannels already contains the right value
            break;
        case 11:// FC, FL FR, SL SR, RC, LFE
            numChannels = 7;
            break;
        case 7: // FC, FCL FCR, FL FR, SL SR, LFE
        case 12:// FC, FL  FR,  SL SR, RL RR, LFE
        case 14:// FC, FL  FR,  SL SR, LFE, FHL FHR
            numChannels = 8;
            break;
        default:
            return ERROR_UNSUPPORTED;
    }

    {
        if (objectType == AOT_SBR || objectType == AOT_PS) {
            if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
            objectType = br.getBits(5);

            if (objectType == AOT_ESCAPE) {
                if (br.numBitsLeft() < 6) return ERROR_MALFORMED;
                objectType = 32 + br.getBits(6);
            }
        }
        if (objectType == AOT_AAC_LC || objectType == AOT_ER_AAC_LC ||
                objectType == AOT_ER_AAC_LD || objectType == AOT_ER_AAC_SCAL ||
                objectType == AOT_ER_BSAC) {
            if (br.numBitsLeft() < 2) return ERROR_MALFORMED;
            const int32_t frameLengthFlag __unused = br.getBits(1);

            const int32_t dependsOnCoreCoder = br.getBits(1);

            if (dependsOnCoreCoder ) {
                if (br.numBitsLeft() < 14) return ERROR_MALFORMED;
                const int32_t coreCoderDelay __unused = br.getBits(14);
            }

            int32_t extensionFlag = -1;
            if (br.numBitsLeft() > 0) {
                extensionFlag = br.getBits(1);
            } else {
                switch (objectType) {
                // 14496-3 4.5.1.1 extensionFlag
                case AOT_AAC_LC:
                    extensionFlag = 0;
                    break;
                case AOT_ER_AAC_LC:
                case AOT_ER_AAC_SCAL:
                case AOT_ER_BSAC:
                case AOT_ER_AAC_LD:
                    extensionFlag = 1;
                    break;
                default:
                    return ERROR_MALFORMED;
                    break;
                }
                ALOGW("csd missing extension flag; assuming %d for object type %u.",
                        extensionFlag, objectType);
            }

            if (numChannels == 0) {
                int32_t channelsEffectiveNum = 0;
                int32_t channelsNum = 0;
                if (br.numBitsLeft() < 32) {
                    return ERROR_MALFORMED;
                }
                const int32_t ElementInstanceTag __unused = br.getBits(4);
                const int32_t Profile __unused = br.getBits(2);
                const int32_t SamplingFrequencyIndex __unused = br.getBits(4);
                const int32_t NumFrontChannelElements = br.getBits(4);
                const int32_t NumSideChannelElements = br.getBits(4);
                const int32_t NumBackChannelElements = br.getBits(4);
                const int32_t NumLfeChannelElements = br.getBits(2);
                const int32_t NumAssocDataElements __unused = br.getBits(3);
                const int32_t NumValidCcElements __unused = br.getBits(4);

                const int32_t MonoMixdownPresent = br.getBits(1);

                if (MonoMixdownPresent != 0) {
                    if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                    const int32_t MonoMixdownElementNumber __unused = br.getBits(4);
                }

                if (br.numBitsLeft() < 1) return ERROR_MALFORMED;
                const int32_t StereoMixdownPresent = br.getBits(1);
                if (StereoMixdownPresent != 0) {
                    if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                    const int32_t StereoMixdownElementNumber __unused = br.getBits(4);
                }

                if (br.numBitsLeft() < 1) return ERROR_MALFORMED;
                const int32_t MatrixMixdownIndexPresent = br.getBits(1);
                if (MatrixMixdownIndexPresent != 0) {
                    if (br.numBitsLeft() < 3) return ERROR_MALFORMED;
                    const int32_t MatrixMixdownIndex __unused = br.getBits(2);
                    const int32_t PseudoSurroundEnable __unused = br.getBits(1);
                }

                int i;
                for (i=0; i < NumFrontChannelElements; i++) {
                    if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                    const int32_t FrontElementIsCpe = br.getBits(1);
                    const int32_t FrontElementTagSelect __unused = br.getBits(4);
                    channelsNum += FrontElementIsCpe ? 2 : 1;
                }

                for (i=0; i < NumSideChannelElements; i++) {
                    if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                    const int32_t SideElementIsCpe = br.getBits(1);
                    const int32_t SideElementTagSelect __unused = br.getBits(4);
                    channelsNum += SideElementIsCpe ? 2 : 1;
                }

                for (i=0; i < NumBackChannelElements; i++) {
                    if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                    const int32_t BackElementIsCpe = br.getBits(1);
                    const int32_t BackElementTagSelect __unused = br.getBits(4);
                    channelsNum += BackElementIsCpe ? 2 : 1;
                }
                channelsEffectiveNum = channelsNum;

                for (i=0; i < NumLfeChannelElements; i++) {
                    if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                    const int32_t LfeElementTagSelect __unused = br.getBits(4);
                    channelsNum += 1;
                }
                ALOGV("mpeg4 audio channelsNum = %d", channelsNum);
                ALOGV("mpeg4 audio channelsEffectiveNum = %d", channelsEffectiveNum);
                numChannels = channelsNum;
            }
        }
    }

    if (numChannels == 0) {
        return ERROR_UNSUPPORTED;
    }

    if (mLastTrack == NULL)
        return ERROR_MALFORMED;

    int32_t prevSampleRate;
    CHECK(AMediaFormat_getInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, &prevSampleRate));

    if (prevSampleRate != sampleRate) {
        ALOGV("mpeg4 audio sample rate different from previous setting. "
             "was: %d, now: %d", prevSampleRate, sampleRate);
    }

    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);

    int32_t prevChannelCount;
    CHECK(AMediaFormat_getInt32(mLastTrack->meta,
            AMEDIAFORMAT_KEY_CHANNEL_COUNT, &prevChannelCount));

    if (prevChannelCount != numChannels) {
        ALOGV("mpeg4 audio channel count different from previous setting. "
             "was: %d, now: %d", prevChannelCount, numChannels);
    }

    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, numChannels);

    return OK;
}

void MPEG4Extractor::adjustRawDefaultFrameSize() {
    int32_t chanCount = 0;
    int32_t bitWidth = 0;
    const char *mimeStr = NULL;

    if(AMediaFormat_getString(mLastTrack->meta, AMEDIAFORMAT_KEY_MIME, &mimeStr) &&
        !strcasecmp(mimeStr, MEDIA_MIMETYPE_AUDIO_RAW) &&
        AMediaFormat_getInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &chanCount) &&
        AMediaFormat_getInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, &bitWidth)) {
        // samplesize in stsz may not right , so updade default samplesize
        mLastTrack->sampleTable->setPredictSampleSize(chanCount * bitWidth / 8);
    }
}

////////////////////////////////////////////////////////////////////////////////

MPEG4Source::MPEG4Source(
        AMediaFormat *format,
        DataSourceHelper *dataSource,
        int32_t timeScale,
        const sp<SampleTable> &sampleTable,
        Vector<SidxEntry> &sidx,
        const Trex *trex,
        off64_t firstMoofOffset,
        const sp<ItemTable> &itemTable,
        uint64_t elstShiftStartTicks)
    : mFormat(format),
      mDataSource(dataSource),
      mTimescale(timeScale),
      mSampleTable(sampleTable),
      mCurrentSampleIndex(0),
      mCurrentFragmentIndex(0),
      mSegments(sidx),
      mTrex(trex),
      mFirstMoofOffset(firstMoofOffset),
      mCurrentMoofOffset(firstMoofOffset),
      mNextMoofOffset(-1),
      mCurrentTime(0),
      mDefaultEncryptedByteBlock(0),
      mDefaultSkipByteBlock(0),
      mCurrentSampleInfoAllocSize(0),
      mCurrentSampleInfoSizes(NULL),
      mCurrentSampleInfoOffsetsAllocSize(0),
      mCurrentSampleInfoOffsets(NULL),
      mIsAVC(false),
      mIsHEVC(false),
      mIsAC4(false),
      mIsPcm(false),
      mNALLengthSize(0),
      mStarted(false),
      mBuffer(NULL),
      mSrcBuffer(NULL),
      mIsHeif(itemTable != NULL),
      mItemTable(itemTable),
      mElstShiftStartTicks(elstShiftStartTicks) {

    memset(&mTrackFragmentHeaderInfo, 0, sizeof(mTrackFragmentHeaderInfo));

    AMediaFormat_getInt32(mFormat,
            AMEDIAFORMAT_KEY_CRYPTO_MODE, &mCryptoMode);
    mDefaultIVSize = 0;
    AMediaFormat_getInt32(mFormat,
            AMEDIAFORMAT_KEY_CRYPTO_DEFAULT_IV_SIZE, &mDefaultIVSize);
    void *key;
    size_t keysize;
    if (AMediaFormat_getBuffer(mFormat,
            AMEDIAFORMAT_KEY_CRYPTO_KEY, &key, &keysize)) {
        CHECK(keysize <= 16);
        memset(mCryptoKey, 0, 16);
        memcpy(mCryptoKey, key, keysize);
    }

    AMediaFormat_getInt32(mFormat,
            AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_BYTE_BLOCK, &mDefaultEncryptedByteBlock);
    AMediaFormat_getInt32(mFormat,
            AMEDIAFORMAT_KEY_CRYPTO_SKIP_BYTE_BLOCK, &mDefaultSkipByteBlock);

    const char *mime;
    bool success = AMediaFormat_getString(mFormat, AMEDIAFORMAT_KEY_MIME, &mime);
    CHECK(success);

    mIsAVC = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC);
    mIsHEVC = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC) ||
              !strcasecmp(mime, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC);
    mIsAC4 = !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC4);

    if (mIsAVC) {
        void *data;
        size_t size;
        CHECK(AMediaFormat_getBuffer(format, AMEDIAFORMAT_KEY_CSD_AVC, &data, &size));

        const uint8_t *ptr = (const uint8_t *)data;

        CHECK(size >= 7);
        CHECK_EQ((unsigned)ptr[0], 1u);  // configurationVersion == 1

        // The number of bytes used to encode the length of a NAL unit.
        mNALLengthSize = 1 + (ptr[4] & 3);
    } else if (mIsHEVC) {
        void *data;
        size_t size;
        CHECK(AMediaFormat_getBuffer(format, AMEDIAFORMAT_KEY_CSD_HEVC, &data, &size));

        const uint8_t *ptr = (const uint8_t *)data;

        CHECK(size >= 22);
        CHECK_EQ((unsigned)ptr[0], 1u);  // configurationVersion == 1

        mNALLengthSize = 1 + (ptr[14 + 7] & 3);
    }

    mIsPcm = !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW);
    mIsAudio = !strncasecmp(mime, "audio/", 6);

    if (mIsPcm) {
        int32_t numChannels = 0;
        int32_t bitsPerSample = 0;
        CHECK(AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, &bitsPerSample));
        CHECK(AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &numChannels));

        int32_t bytesPerSample = bitsPerSample >> 3;
        int32_t pcmSampleSize = bytesPerSample * numChannels;

        size_t maxSampleSize;
        status_t err = mSampleTable->getMaxSampleSize(&maxSampleSize);
        if (err != OK || maxSampleSize != static_cast<size_t>(pcmSampleSize)
               || bitsPerSample != 16) {
            // Not supported
            mIsPcm = false;
        } else {
            AMediaFormat_setInt32(mFormat,
                    AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, pcmSampleSize * kMaxPcmFrameSize);
        }
    }

    CHECK(AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_TRACK_ID, &mTrackId));

}

status_t MPEG4Source::init() {
    status_t err = OK;
    const char *mime;
    CHECK(AMediaFormat_getString(mFormat, AMEDIAFORMAT_KEY_MIME, &mime));
    if (mFirstMoofOffset != 0) {
        off64_t offset = mFirstMoofOffset;
        err = parseChunk(&offset);
        if(err == OK && !strncasecmp("video/", mime, 6)
            && !mCurrentSamples.isEmpty()) {
            // Start offset should be less or equal to composition time of first sample.
            // ISO : sample_composition_time_offset, version 0 (unsigned) for major brands.
            mElstShiftStartTicks = std::min(mElstShiftStartTicks,
                                            (uint64_t)(*mCurrentSamples.begin()).compositionOffset);
        }
        return err;
    }

    if (!strncasecmp("video/", mime, 6)) {
        uint64_t firstSampleCTS = 0;
        err = mSampleTable->getMetaDataForSample(0, NULL, NULL, &firstSampleCTS);
        // Start offset should be less or equal to composition time of first sample.
        // Composition time stamp of first sample cannot be negative.
        mElstShiftStartTicks = std::min(mElstShiftStartTicks, firstSampleCTS);
    }

    return err;
}

MPEG4Source::~MPEG4Source() {
    if (mStarted) {
        stop();
    }
    free(mCurrentSampleInfoSizes);
    free(mCurrentSampleInfoOffsets);
}

media_status_t MPEG4Source::start() {
    Mutex::Autolock autoLock(mLock);

    CHECK(!mStarted);

    int32_t tmp;
    CHECK(AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, &tmp));
    size_t max_size = tmp;

    // A somewhat arbitrary limit that should be sufficient for 8k video frames
    // If you see the message below for a valid input stream: increase the limit
    const size_t kMaxBufferSize = 64 * 1024 * 1024;
    if (max_size > kMaxBufferSize) {
        ALOGE("bogus max input size: %zu > %zu", max_size, kMaxBufferSize);
        return AMEDIA_ERROR_MALFORMED;
    }
    if (max_size == 0) {
        ALOGE("zero max input size");
        return AMEDIA_ERROR_MALFORMED;
    }

    // Allow up to kMaxBuffers, but not if the total exceeds kMaxBufferSize.
    const size_t kInitialBuffers = 2;
    const size_t kMaxBuffers = 8;
    const size_t realMaxBuffers = min(kMaxBufferSize / max_size, kMaxBuffers);
    mBufferGroup->init(kInitialBuffers, max_size, realMaxBuffers);
    mSrcBuffer = new (std::nothrow) uint8_t[max_size];
    if (mSrcBuffer == NULL) {
        // file probably specified a bad max size
        return AMEDIA_ERROR_MALFORMED;
    }

    mStarted = true;

    return AMEDIA_OK;
}

media_status_t MPEG4Source::stop() {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);

    if (mBuffer != NULL) {
        mBuffer->release();
        mBuffer = NULL;
    }

    delete[] mSrcBuffer;
    mSrcBuffer = NULL;

    mStarted = false;
    mCurrentSampleIndex = 0;

    return AMEDIA_OK;
}

status_t MPEG4Source::parseChunk(off64_t *offset) {
    uint32_t hdr[2];
    if (mDataSource->readAt(*offset, hdr, 8) < 8) {
        return ERROR_IO;
    }
    uint64_t chunk_size = ntohl(hdr[0]);
    uint32_t chunk_type = ntohl(hdr[1]);
    off64_t data_offset = *offset + 8;

    if (chunk_size == 1) {
        if (mDataSource->readAt(*offset + 8, &chunk_size, 8) < 8) {
            return ERROR_IO;
        }
        chunk_size = ntoh64(chunk_size);
        data_offset += 8;

        if (chunk_size < 16) {
            // The smallest valid chunk is 16 bytes long in this case.
            return ERROR_MALFORMED;
        }
    } else if (chunk_size < 8) {
        // The smallest valid chunk is 8 bytes long.
        return ERROR_MALFORMED;
    }

    char chunk[5];
    MakeFourCCString(chunk_type, chunk);
    ALOGV("MPEG4Source chunk %s @ %#llx", chunk, (long long)*offset);

    off64_t chunk_data_size = *offset + chunk_size - data_offset;

    switch(chunk_type) {

        case FOURCC("traf"):
        case FOURCC("moof"): {
            off64_t stop_offset = *offset + chunk_size;
            *offset = data_offset;
            while (*offset < stop_offset) {
                status_t err = parseChunk(offset);
                if (err != OK) {
                    return err;
                }
            }
            if (chunk_type == FOURCC("moof")) {
                // *offset points to the box following this moof. Find the next moof from there.

                while (true) {
                    if (mDataSource->readAt(*offset, hdr, 8) < 8) {
                        // no more box to the end of file.
                        break;
                    }
                    chunk_size = ntohl(hdr[0]);
                    chunk_type = ntohl(hdr[1]);
                    if (chunk_size == 1) {
                        // ISO/IEC 14496-12:2012, 8.8.4 Movie Fragment Box, moof is a Box
                        // which is defined in 4.2 Object Structure.
                        // When chunk_size==1, 8 bytes follows as "largesize".
                        if (mDataSource->readAt(*offset + 8, &chunk_size, 8) < 8) {
                            return ERROR_IO;
                        }
                        chunk_size = ntoh64(chunk_size);
                        if (chunk_size < 16) {
                            // The smallest valid chunk is 16 bytes long in this case.
                            return ERROR_MALFORMED;
                        }
                    } else if (chunk_size == 0) {
                        // next box extends to end of file.
                    } else if (chunk_size < 8) {
                        // The smallest valid chunk is 8 bytes long in this case.
                        return ERROR_MALFORMED;
                    }

                    if (chunk_type == FOURCC("moof")) {
                        mNextMoofOffset = *offset;
                        break;
                    } else if (chunk_size == 0) {
                        break;
                    }
                    *offset += chunk_size;
                }
            }
            break;
        }

        case FOURCC("tfhd"): {
                status_t err;
                if ((err = parseTrackFragmentHeader(data_offset, chunk_data_size)) != OK) {
                    return err;
                }
                *offset += chunk_size;
                break;
        }

        case FOURCC("trun"): {
                status_t err;
                if (mLastParsedTrackId == mTrackId) {
                    if ((err = parseTrackFragmentRun(data_offset, chunk_data_size)) != OK) {
                        return err;
                    }
                }

                *offset += chunk_size;
                break;
        }

        case FOURCC("saiz"): {
            status_t err;
            if ((err = parseSampleAuxiliaryInformationSizes(data_offset, chunk_data_size)) != OK) {
                return err;
            }
            *offset += chunk_size;
            break;
        }
        case FOURCC("saio"): {
            status_t err;
            if ((err = parseSampleAuxiliaryInformationOffsets(data_offset, chunk_data_size))
                    != OK) {
                return err;
            }
            *offset += chunk_size;
            break;
        }

        case FOURCC("senc"): {
            status_t err;
            if ((err = parseSampleEncryption(data_offset)) != OK) {
                return err;
            }
            *offset += chunk_size;
            break;
        }

        case FOURCC("mdat"): {
            // parse DRM info if present
            ALOGV("MPEG4Source::parseChunk mdat");
            // if saiz/saoi was previously observed, do something with the sampleinfos
            *offset += chunk_size;
            break;
        }

        default: {
            *offset += chunk_size;
            break;
        }
    }
    return OK;
}

status_t MPEG4Source::parseSampleAuxiliaryInformationSizes(
        off64_t offset, off64_t size) {
    ALOGV("parseSampleAuxiliaryInformationSizes");
    if (size < 9) {
        return -EINVAL;
    }
    // 14496-12 8.7.12
    uint8_t version;
    if (mDataSource->readAt(
            offset, &version, sizeof(version))
            < (ssize_t)sizeof(version)) {
        return ERROR_IO;
    }

    if (version != 0) {
        return ERROR_UNSUPPORTED;
    }
    offset++;
    size--;

    uint32_t flags;
    if (!mDataSource->getUInt24(offset, &flags)) {
        return ERROR_IO;
    }
    offset += 3;
    size -= 3;

    if (flags & 1) {
        if (size < 13) {
            return -EINVAL;
        }
        uint32_t tmp;
        if (!mDataSource->getUInt32(offset, &tmp)) {
            return ERROR_MALFORMED;
        }
        mCurrentAuxInfoType = tmp;
        offset += 4;
        size -= 4;
        if (!mDataSource->getUInt32(offset, &tmp)) {
            return ERROR_MALFORMED;
        }
        mCurrentAuxInfoTypeParameter = tmp;
        offset += 4;
        size -= 4;
    }

    uint8_t defsize;
    if (mDataSource->readAt(offset, &defsize, 1) != 1) {
        return ERROR_MALFORMED;
    }
    mCurrentDefaultSampleInfoSize = defsize;
    offset++;
    size--;

    uint32_t smplcnt;
    if (!mDataSource->getUInt32(offset, &smplcnt)) {
        return ERROR_MALFORMED;
    }
    mCurrentSampleInfoCount = smplcnt;
    offset += 4;
    size -= 4;
    if (mCurrentDefaultSampleInfoSize != 0) {
        ALOGV("@@@@ using default sample info size of %d", mCurrentDefaultSampleInfoSize);
        return OK;
    }
    if(smplcnt > size) {
        ALOGW("b/124525515 - smplcnt(%u) > size(%ld)", (unsigned int)smplcnt, (unsigned long)size);
        android_errorWriteLog(0x534e4554, "124525515");
        return -EINVAL;
    }
    if (smplcnt > mCurrentSampleInfoAllocSize) {
        uint8_t * newPtr =  (uint8_t*) realloc(mCurrentSampleInfoSizes, smplcnt);
        if (newPtr == NULL) {
            ALOGE("failed to realloc %u -> %u", mCurrentSampleInfoAllocSize, smplcnt);
            return NO_MEMORY;
        }
        mCurrentSampleInfoSizes = newPtr;
        mCurrentSampleInfoAllocSize = smplcnt;
    }

    mDataSource->readAt(offset, mCurrentSampleInfoSizes, smplcnt);
    return OK;
}

status_t MPEG4Source::parseSampleAuxiliaryInformationOffsets(
        off64_t offset, off64_t size) {
    ALOGV("parseSampleAuxiliaryInformationOffsets");
    if (size < 8) {
        return -EINVAL;
    }
    // 14496-12 8.7.13
    uint8_t version;
    if (mDataSource->readAt(offset, &version, sizeof(version)) != 1) {
        return ERROR_IO;
    }
    offset++;
    size--;

    uint32_t flags;
    if (!mDataSource->getUInt24(offset, &flags)) {
        return ERROR_IO;
    }
    offset += 3;
    size -= 3;

    uint32_t entrycount;
    if (!mDataSource->getUInt32(offset, &entrycount)) {
        return ERROR_IO;
    }
    offset += 4;
    size -= 4;
    if (entrycount == 0) {
        return OK;
    }
    if (entrycount > UINT32_MAX / 8) {
        return ERROR_MALFORMED;
    }

    if (entrycount > mCurrentSampleInfoOffsetsAllocSize) {
        uint64_t *newPtr = (uint64_t *)realloc(mCurrentSampleInfoOffsets, entrycount * 8);
        if (newPtr == NULL) {
            ALOGE("failed to realloc %u -> %u",
                    mCurrentSampleInfoOffsetsAllocSize, entrycount * 8);
            return NO_MEMORY;
        }
        mCurrentSampleInfoOffsets = newPtr;
        mCurrentSampleInfoOffsetsAllocSize = entrycount;
    }
    mCurrentSampleInfoOffsetCount = entrycount;

    if (mCurrentSampleInfoOffsets == NULL) {
        return OK;
    }

    for (size_t i = 0; i < entrycount; i++) {
        if (version == 0) {
            if (size < 4) {
                ALOGW("b/124526959");
                android_errorWriteLog(0x534e4554, "124526959");
                return -EINVAL;
            }
            uint32_t tmp;
            if (!mDataSource->getUInt32(offset, &tmp)) {
                return ERROR_IO;
            }
            mCurrentSampleInfoOffsets[i] = tmp;
            offset += 4;
            size -= 4;
        } else {
            if (size < 8) {
                ALOGW("b/124526959");
                android_errorWriteLog(0x534e4554, "124526959");
                return -EINVAL;
            }
            uint64_t tmp;
            if (!mDataSource->getUInt64(offset, &tmp)) {
                return ERROR_IO;
            }
            mCurrentSampleInfoOffsets[i] = tmp;
            offset += 8;
            size -= 8;
        }
    }

    // parse clear/encrypted data

    off64_t drmoffset = mCurrentSampleInfoOffsets[0]; // from moof

    drmoffset += mCurrentMoofOffset;

    return parseClearEncryptedSizes(drmoffset, false, 0);
}

status_t MPEG4Source::parseClearEncryptedSizes(
        off64_t offset, bool isSubsampleEncryption, uint32_t flags) {

    int32_t ivlength;
    if (!AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_CRYPTO_DEFAULT_IV_SIZE, &ivlength)) {
        return ERROR_MALFORMED;
    }

    // only 0, 8 and 16 byte initialization vectors are supported
    if (ivlength != 0 && ivlength != 8 && ivlength != 16) {
        ALOGW("unsupported IV length: %d", ivlength);
        return ERROR_MALFORMED;
    }

    uint32_t sampleCount = mCurrentSampleInfoCount;
    if (isSubsampleEncryption) {
        if (!mDataSource->getUInt32(offset, &sampleCount)) {
            return ERROR_IO;
        }
        offset += 4;
    }

    // read CencSampleAuxiliaryDataFormats
    for (size_t i = 0; i < sampleCount; i++) {
        if (i >= mCurrentSamples.size()) {
            ALOGW("too few samples");
            break;
        }
        Sample *smpl = &mCurrentSamples.editItemAt(i);
        if (!smpl->clearsizes.isEmpty()) {
            continue;
        }

        memset(smpl->iv, 0, 16);
        if (mDataSource->readAt(offset, smpl->iv, ivlength) != ivlength) {
            return ERROR_IO;
        }

        offset += ivlength;

        bool readSubsamples;
        if (isSubsampleEncryption) {
            readSubsamples = flags & 2;
        } else {
            int32_t smplinfosize = mCurrentDefaultSampleInfoSize;
            if (smplinfosize == 0) {
                smplinfosize = mCurrentSampleInfoSizes[i];
            }
            readSubsamples = smplinfosize > ivlength;
        }

        if (readSubsamples) {
            uint16_t numsubsamples;
            if (!mDataSource->getUInt16(offset, &numsubsamples)) {
                return ERROR_IO;
            }
            offset += 2;
            for (size_t j = 0; j < numsubsamples; j++) {
                uint16_t numclear;
                uint32_t numencrypted;
                if (!mDataSource->getUInt16(offset, &numclear)) {
                    return ERROR_IO;
                }
                offset += 2;
                if (!mDataSource->getUInt32(offset, &numencrypted)) {
                    return ERROR_IO;
                }
                offset += 4;
                smpl->clearsizes.add(numclear);
                smpl->encryptedsizes.add(numencrypted);
            }
        } else {
            smpl->clearsizes.add(0);
            smpl->encryptedsizes.add(smpl->size);
        }
    }

    return OK;
}

status_t MPEG4Source::parseSampleEncryption(off64_t offset) {
    uint32_t flags;
    if (!mDataSource->getUInt32(offset, &flags)) { // actually version + flags
        return ERROR_MALFORMED;
    }
    return parseClearEncryptedSizes(offset + 4, true, flags);
}

status_t MPEG4Source::parseTrackFragmentHeader(off64_t offset, off64_t size) {

    if (size < 8) {
        return -EINVAL;
    }

    uint32_t flags;
    if (!mDataSource->getUInt32(offset, &flags)) { // actually version + flags
        return ERROR_MALFORMED;
    }

    if (flags & 0xff000000) {
        return -EINVAL;
    }

    if (!mDataSource->getUInt32(offset + 4, (uint32_t*)&mLastParsedTrackId)) {
        return ERROR_MALFORMED;
    }

    if (mLastParsedTrackId != mTrackId) {
        // this is not the right track, skip it
        return OK;
    }

    mTrackFragmentHeaderInfo.mFlags = flags;
    mTrackFragmentHeaderInfo.mTrackID = mLastParsedTrackId;
    offset += 8;
    size -= 8;

    ALOGV("fragment header: %08x %08x", flags, mTrackFragmentHeaderInfo.mTrackID);

    if (flags & TrackFragmentHeaderInfo::kBaseDataOffsetPresent) {
        if (size < 8) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt64(offset, &mTrackFragmentHeaderInfo.mBaseDataOffset)) {
            return ERROR_MALFORMED;
        }
        offset += 8;
        size -= 8;
    }

    if (flags & TrackFragmentHeaderInfo::kSampleDescriptionIndexPresent) {
        if (size < 4) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt32(offset, &mTrackFragmentHeaderInfo.mSampleDescriptionIndex)) {
            return ERROR_MALFORMED;
        }
        offset += 4;
        size -= 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleDurationPresent) {
        if (size < 4) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt32(offset, &mTrackFragmentHeaderInfo.mDefaultSampleDuration)) {
            return ERROR_MALFORMED;
        }
        offset += 4;
        size -= 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleSizePresent) {
        if (size < 4) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt32(offset, &mTrackFragmentHeaderInfo.mDefaultSampleSize)) {
            return ERROR_MALFORMED;
        }
        offset += 4;
        size -= 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleFlagsPresent) {
        if (size < 4) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt32(offset, &mTrackFragmentHeaderInfo.mDefaultSampleFlags)) {
            return ERROR_MALFORMED;
        }
        offset += 4;
        size -= 4;
    }

    if (!(flags & TrackFragmentHeaderInfo::kBaseDataOffsetPresent)) {
        mTrackFragmentHeaderInfo.mBaseDataOffset = mCurrentMoofOffset;
    }

    mTrackFragmentHeaderInfo.mDataOffset = 0;
    return OK;
}

status_t MPEG4Source::parseTrackFragmentRun(off64_t offset, off64_t size) {

    ALOGV("MPEG4Source::parseTrackFragmentRun");
    if (size < 8) {
        return -EINVAL;
    }

    enum {
        kDataOffsetPresent                  = 0x01,
        kFirstSampleFlagsPresent            = 0x04,
        kSampleDurationPresent              = 0x100,
        kSampleSizePresent                  = 0x200,
        kSampleFlagsPresent                 = 0x400,
        kSampleCompositionTimeOffsetPresent = 0x800,
    };

    uint32_t flags;
    if (!mDataSource->getUInt32(offset, &flags)) {
        return ERROR_MALFORMED;
    }
    // |version| only affects SampleCompositionTimeOffset field.
    // If version == 0, SampleCompositionTimeOffset is uint32_t;
    // Otherwise, SampleCompositionTimeOffset is int32_t.
    // Sample.compositionOffset is defined as int32_t.
    uint8_t version = flags >> 24;
    flags &= 0xffffff;
    ALOGV("fragment run version: 0x%02x, flags: 0x%06x", version, flags);

    if ((flags & kFirstSampleFlagsPresent) && (flags & kSampleFlagsPresent)) {
        // These two shall not be used together.
        return -EINVAL;
    }

    uint32_t sampleCount;
    if (!mDataSource->getUInt32(offset + 4, &sampleCount)) {
        return ERROR_MALFORMED;
    }
    offset += 8;
    size -= 8;

    uint64_t dataOffset = mTrackFragmentHeaderInfo.mDataOffset;

    uint32_t firstSampleFlags = 0;

    if (flags & kDataOffsetPresent) {
        if (size < 4) {
            return -EINVAL;
        }

        int32_t dataOffsetDelta;
        if (!mDataSource->getUInt32(offset, (uint32_t*)&dataOffsetDelta)) {
            return ERROR_MALFORMED;
        }

        dataOffset = mTrackFragmentHeaderInfo.mBaseDataOffset + dataOffsetDelta;

        offset += 4;
        size -= 4;
    }

    if (flags & kFirstSampleFlagsPresent) {
        if (size < 4) {
            return -EINVAL;
        }

        if (!mDataSource->getUInt32(offset, &firstSampleFlags)) {
            return ERROR_MALFORMED;
        }
        offset += 4;
        size -= 4;
    }

    uint32_t sampleDuration = 0, sampleSize = 0, sampleFlags = 0,
             sampleCtsOffset = 0;

    size_t bytesPerSample = 0;
    if (flags & kSampleDurationPresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleDurationPresent) {
        sampleDuration = mTrackFragmentHeaderInfo.mDefaultSampleDuration;
    } else if (mTrex) {
        sampleDuration = mTrex->default_sample_duration;
    }

    if (flags & kSampleSizePresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleSizePresent) {
        sampleSize = mTrackFragmentHeaderInfo.mDefaultSampleSize;
    } else {
        sampleSize = mTrackFragmentHeaderInfo.mDefaultSampleSize;
    }

    if (flags & kSampleFlagsPresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleFlagsPresent) {
        sampleFlags = mTrackFragmentHeaderInfo.mDefaultSampleFlags;
    } else {
        sampleFlags = mTrackFragmentHeaderInfo.mDefaultSampleFlags;
    }

    if (flags & kSampleCompositionTimeOffsetPresent) {
        bytesPerSample += 4;
    } else {
        sampleCtsOffset = 0;
    }

    if (bytesPerSample != 0) {
        if (size < (off64_t)sampleCount * bytesPerSample) {
            return -EINVAL;
        }
    } else {
        if (sampleDuration == 0) {
            ALOGW("b/123389881 sampleDuration == 0");
            android_errorWriteLog(0x534e4554, "124389881 zero");
            return -EINVAL;
        }

        // apply some sanity (vs strict legality) checks
        //
        static constexpr uint32_t kMaxTrunSampleCount = 10000;
        if (sampleCount > kMaxTrunSampleCount) {
            ALOGW("b/123389881 sampleCount(%u) > kMaxTrunSampleCount(%u)",
                  sampleCount, kMaxTrunSampleCount);
            android_errorWriteLog(0x534e4554, "124389881 count");
            return -EINVAL;
        }
    }

    Sample tmp;
    for (uint32_t i = 0; i < sampleCount; ++i) {
        if (flags & kSampleDurationPresent) {
            if (!mDataSource->getUInt32(offset, &sampleDuration)) {
                return ERROR_MALFORMED;
            }
            offset += 4;
        }

        if (flags & kSampleSizePresent) {
            if (!mDataSource->getUInt32(offset, &sampleSize)) {
                return ERROR_MALFORMED;
            }
            offset += 4;
        }

        if (flags & kSampleFlagsPresent) {
            if (!mDataSource->getUInt32(offset, &sampleFlags)) {
                return ERROR_MALFORMED;
            }
            offset += 4;
        }

        if (flags & kSampleCompositionTimeOffsetPresent) {
            if (!mDataSource->getUInt32(offset, &sampleCtsOffset)) {
                return ERROR_MALFORMED;
            }
            offset += 4;
        }

        ALOGV("adding sample %d at offset 0x%08" PRIx64 ", size %u, duration %u, "
              " flags 0x%08x ctsOffset %" PRIu32, i + 1,
                dataOffset, sampleSize, sampleDuration,
                (flags & kFirstSampleFlagsPresent) && i == 0
                    ? firstSampleFlags : sampleFlags, sampleCtsOffset);
        tmp.offset = dataOffset;
        tmp.size = sampleSize;
        tmp.duration = sampleDuration;
        tmp.compositionOffset = sampleCtsOffset;
        memset(tmp.iv, 0, sizeof(tmp.iv));
        if (mCurrentSamples.add(tmp) < 0) {
            ALOGW("b/123389881 failed saving sample(n=%zu)", mCurrentSamples.size());
            android_errorWriteLog(0x534e4554, "124389881 allocation");
            mCurrentSamples.clear();
            return NO_MEMORY;
        }

        dataOffset += sampleSize;
    }

    mTrackFragmentHeaderInfo.mDataOffset = dataOffset;

    return OK;
}

media_status_t MPEG4Source::getFormat(AMediaFormat *meta) {
    Mutex::Autolock autoLock(mLock);
    AMediaFormat_copy(meta, mFormat);
    return AMEDIA_OK;
}

size_t MPEG4Source::parseNALSize(const uint8_t *data) const {
    switch (mNALLengthSize) {
        case 1:
            return *data;
        case 2:
            return U16_AT(data);
        case 3:
            return ((size_t)data[0] << 16) | U16_AT(&data[1]);
        case 4:
            return U32_AT(data);
    }

    // This cannot happen, mNALLengthSize springs to life by adding 1 to
    // a 2-bit integer.
    CHECK(!"Should not be here.");

    return 0;
}

int32_t MPEG4Source::parseHEVCLayerId(const uint8_t *data, size_t size) {
    if (data == nullptr || size < mNALLengthSize + 2) {
        return -1;
    }

    // HEVC NAL-header (16-bit)
    //  1   6      6     3
    // |-|uuuuuu|------|iii|
    //      ^            ^
    //  NAL_type        layer_id + 1
    //
    // Layer-id is non-zero only for Temporal Sub-layer Access pictures (TSA)
    enum {
        TSA_N = 2,
        TSA_R = 3,
        STSA_N = 4,
        STSA_R = 5,
    };

    data += mNALLengthSize;
    uint16_t nalHeader = data[0] << 8 | data[1];

    uint16_t nalType = (nalHeader >> 9) & 0x3Fu;
    if (nalType == TSA_N || nalType == TSA_R || nalType == STSA_N || nalType == STSA_R) {
        int32_t layerIdPlusOne = nalHeader & 0x7u;
        ALOGD_IF(layerIdPlusOne == 0, "got layerId 0 for TSA picture");
        return layerIdPlusOne - 1;
    }
    return 0;
}

media_status_t MPEG4Source::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);

    if (options != nullptr && options->getNonBlocking() && !mBufferGroup->has_buffers()) {
        *out = nullptr;
        return AMEDIA_ERROR_WOULD_BLOCK;
    }

    if (mFirstMoofOffset > 0) {
        return fragmentedRead(out, options);
    }

    *out = NULL;

    int64_t targetSampleTimeUs = -1;

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {

        if (mIsHeif) {
            CHECK(mSampleTable == NULL);
            CHECK(mItemTable != NULL);
            int32_t imageIndex;
            if (!AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_TRACK_ID, &imageIndex)) {
                return AMEDIA_ERROR_MALFORMED;
            }

            status_t err;
            if (seekTimeUs >= 0) {
                err = mItemTable->findImageItem(imageIndex, &mCurrentSampleIndex);
            } else {
                err = mItemTable->findThumbnailItem(imageIndex, &mCurrentSampleIndex);
            }
            if (err != OK) {
                return AMEDIA_ERROR_UNKNOWN;
            }
        } else {
            uint32_t findFlags = 0;
            switch (mode) {
                case ReadOptions::SEEK_PREVIOUS_SYNC:
                    findFlags = SampleTable::kFlagBefore;
                    break;
                case ReadOptions::SEEK_NEXT_SYNC:
                    findFlags = SampleTable::kFlagAfter;
                    break;
                case ReadOptions::SEEK_CLOSEST_SYNC:
                case ReadOptions::SEEK_CLOSEST:
                    findFlags = SampleTable::kFlagClosest;
                    break;
                case ReadOptions::SEEK_FRAME_INDEX:
                    findFlags = SampleTable::kFlagFrameIndex;
                    break;
                default:
                    CHECK(!"Should not be here.");
                    break;
            }
            if( mode != ReadOptions::SEEK_FRAME_INDEX) {
                seekTimeUs += ((long double)mElstShiftStartTicks * 1000000) / mTimescale;
            }

            uint32_t sampleIndex;
            status_t err = mSampleTable->findSampleAtTime(
                    seekTimeUs, 1000000, mTimescale,
                    &sampleIndex, findFlags);

            if (mode == ReadOptions::SEEK_CLOSEST
                    || mode == ReadOptions::SEEK_FRAME_INDEX) {
                // We found the closest sample already, now we want the sync
                // sample preceding it (or the sample itself of course), even
                // if the subsequent sync sample is closer.
                findFlags = SampleTable::kFlagBefore;
            }

            uint32_t syncSampleIndex = sampleIndex;
            // assume every audio sample is a sync sample. This works around
            // seek issues with files that were incorrectly written with an
            // empty or single-sample stss block for the audio track
            if (err == OK && !mIsAudio) {
                err = mSampleTable->findSyncSampleNear(
                        sampleIndex, &syncSampleIndex, findFlags);
            }

            uint64_t sampleTime;
            if (err == OK) {
                err = mSampleTable->getMetaDataForSample(
                        sampleIndex, NULL, NULL, &sampleTime);
            }

            if (err != OK) {
                if (err == ERROR_OUT_OF_RANGE) {
                    // An attempt to seek past the end of the stream would
                    // normally cause this ERROR_OUT_OF_RANGE error. Propagating
                    // this all the way to the MediaPlayer would cause abnormal
                    // termination. Legacy behaviour appears to be to behave as if
                    // we had seeked to the end of stream, ending normally.
                    return AMEDIA_ERROR_END_OF_STREAM;
                }
                ALOGV("end of stream");
                return AMEDIA_ERROR_UNKNOWN;
            }

            if (mode == ReadOptions::SEEK_CLOSEST
                || mode == ReadOptions::SEEK_FRAME_INDEX) {
                sampleTime -= mElstShiftStartTicks;
                targetSampleTimeUs = (sampleTime * 1000000ll) / mTimescale;
            }

#if 0
            uint32_t syncSampleTime;
            CHECK_EQ(OK, mSampleTable->getMetaDataForSample(
                        syncSampleIndex, NULL, NULL, &syncSampleTime));

            ALOGI("seek to time %lld us => sample at time %lld us, "
                 "sync sample at time %lld us",
                 seekTimeUs,
                 sampleTime * 1000000ll / mTimescale,
                 syncSampleTime * 1000000ll / mTimescale);
#endif

            mCurrentSampleIndex = syncSampleIndex;
        }

        if (mBuffer != NULL) {
            mBuffer->release();
            mBuffer = NULL;
        }

        // fall through
    }

    off64_t offset = 0;
    size_t size = 0;
    uint64_t cts, stts;
    bool isSyncSample;
    bool newBuffer = false;
    if (mBuffer == NULL) {
        newBuffer = true;

        status_t err;
        if (!mIsHeif) {
            err = mSampleTable->getMetaDataForSample(
                    mCurrentSampleIndex, &offset, &size, &cts, &isSyncSample, &stts);
            if(err == OK) {
                /* Composition Time Stamp cannot be negative. Some files have video Sample
                * Time(STTS)delta with zero value(b/117402420).  Hence subtract only
                * min(cts, mElstShiftStartTicks), so that audio tracks can be played.
                */
                cts -= std::min(cts, mElstShiftStartTicks);
            }

        } else {
            err = mItemTable->getImageOffsetAndSize(
                    options && options->getSeekTo(&seekTimeUs, &mode) ?
                            &mCurrentSampleIndex : NULL, &offset, &size);

            cts = stts = 0;
            isSyncSample = 0;
            ALOGV("image offset %lld, size %zu", (long long)offset, size);
        }

        if (err != OK) {
            if (err == ERROR_END_OF_STREAM) {
                return AMEDIA_ERROR_END_OF_STREAM;
            }
            return AMEDIA_ERROR_UNKNOWN;
        }

        err = mBufferGroup->acquire_buffer(&mBuffer);

        if (err != OK) {
            CHECK(mBuffer == NULL);
            return AMEDIA_ERROR_UNKNOWN;
        }
        if (size > mBuffer->size()) {
            ALOGE("buffer too small: %zu > %zu", size, mBuffer->size());
            mBuffer->release();
            mBuffer = NULL;
            return AMEDIA_ERROR_UNKNOWN; // ERROR_BUFFER_TOO_SMALL
        }
    }

    if (!mIsAVC && !mIsHEVC && !mIsAC4) {
        if (newBuffer) {
            if (mIsPcm) {
                // The twos' PCM block reader assumes that all samples has the same size.

                uint32_t samplesToRead = mSampleTable->getLastSampleIndexInChunk()
                                                      - mCurrentSampleIndex + 1;
                if (samplesToRead > kMaxPcmFrameSize) {
                    samplesToRead = kMaxPcmFrameSize;
                }

                ALOGV("Reading %d PCM frames of size %zu at index %d to stop of chunk at %d",
                      samplesToRead, size, mCurrentSampleIndex,
                      mSampleTable->getLastSampleIndexInChunk());

               size_t totalSize = samplesToRead * size;
                uint8_t* buf = (uint8_t *)mBuffer->data();
                ssize_t bytesRead = mDataSource->readAt(offset, buf, totalSize);
                if (bytesRead < (ssize_t)totalSize) {
                    mBuffer->release();
                    mBuffer = NULL;

                    return AMEDIA_ERROR_IO;
                }

                AMediaFormat *meta = mBuffer->meta_data();
                AMediaFormat_clear(meta);
                AMediaFormat_setInt64(
                      meta, AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);

                int32_t byteOrder;
                AMediaFormat_getInt32(mFormat,
                        AMEDIAFORMAT_KEY_PCM_BIG_ENDIAN, &byteOrder);

                if (byteOrder == 1) {
                    // Big-endian -> little-endian
                    uint16_t *dstData = (uint16_t *)buf;
                    uint16_t *srcData = (uint16_t *)buf;

                    for (size_t j = 0; j < bytesRead / sizeof(uint16_t); j++) {
                         dstData[j] = ntohs(srcData[j]);
                    }
                }

                mCurrentSampleIndex += samplesToRead;
                mBuffer->set_range(0, totalSize);
            } else {
                ssize_t num_bytes_read =
                    mDataSource->readAt(offset, (uint8_t *)mBuffer->data(), size);

                if (num_bytes_read < (ssize_t)size) {
                    mBuffer->release();
                    mBuffer = NULL;

                    return AMEDIA_ERROR_IO;
                }

                CHECK(mBuffer != NULL);
                mBuffer->set_range(0, size);
                AMediaFormat *meta = mBuffer->meta_data();
                AMediaFormat_clear(meta);
                AMediaFormat_setInt64(
                        meta, AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
                AMediaFormat_setInt64(
                        meta, AMEDIAFORMAT_KEY_DURATION, ((long double)stts * 1000000) / mTimescale);

                if (targetSampleTimeUs >= 0) {
                    AMediaFormat_setInt64(
                            meta, AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
                }

                if (isSyncSample) {
                    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
                }
 
                ++mCurrentSampleIndex;
            }
        }

        *out = mBuffer;
        mBuffer = NULL;

        return AMEDIA_OK;

    } else if (mIsAC4) {
        CHECK(mBuffer != NULL);
        // Make sure there is enough space to write the sync header and the raw frame
        if (mBuffer->range_length() < (7 + size)) {
            mBuffer->release();
            mBuffer = NULL;

            return AMEDIA_ERROR_IO;
        }

        uint8_t *dstData = (uint8_t *)mBuffer->data();
        size_t dstOffset = 0;
        // Add AC-4 sync header to MPEG4 encapsulated AC-4 raw frame
        // AC40 sync word, meaning no CRC at the end of the frame
        dstData[dstOffset++] = 0xAC;
        dstData[dstOffset++] = 0x40;
        dstData[dstOffset++] = 0xFF;
        dstData[dstOffset++] = 0xFF;
        dstData[dstOffset++] = (uint8_t)((size >> 16) & 0xFF);
        dstData[dstOffset++] = (uint8_t)((size >> 8) & 0xFF);
        dstData[dstOffset++] = (uint8_t)((size >> 0) & 0xFF);

        ssize_t numBytesRead = mDataSource->readAt(offset, dstData + dstOffset, size);
        if (numBytesRead != (ssize_t)size) {
            mBuffer->release();
            mBuffer = NULL;

            return AMEDIA_ERROR_IO;
        }

        mBuffer->set_range(0, dstOffset + size);
        AMediaFormat *meta = mBuffer->meta_data();
        AMediaFormat_clear(meta);
        AMediaFormat_setInt64(
                meta, AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
        AMediaFormat_setInt64(
                meta, AMEDIAFORMAT_KEY_DURATION, ((long double)stts * 1000000) / mTimescale);

        if (targetSampleTimeUs >= 0) {
            AMediaFormat_setInt64(
                    meta, AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
        }

        if (isSyncSample) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
        }

        ++mCurrentSampleIndex;

        *out = mBuffer;
        mBuffer = NULL;

        return AMEDIA_OK;
    } else {
        // Whole NAL units are returned but each fragment is prefixed by
        // the start code (0x00 00 00 01).
        ssize_t num_bytes_read = 0;
        num_bytes_read = mDataSource->readAt(offset, mSrcBuffer, size);

        if (num_bytes_read < (ssize_t)size) {
            mBuffer->release();
            mBuffer = NULL;

            return AMEDIA_ERROR_IO;
        }

        uint8_t *dstData = (uint8_t *)mBuffer->data();
        size_t srcOffset = 0;
        size_t dstOffset = 0;

        while (srcOffset < size) {
            bool isMalFormed = !isInRange((size_t)0u, size, srcOffset, mNALLengthSize);
            size_t nalLength = 0;
            if (!isMalFormed) {
                nalLength = parseNALSize(&mSrcBuffer[srcOffset]);
                srcOffset += mNALLengthSize;
                isMalFormed = !isInRange((size_t)0u, size, srcOffset, nalLength);
            }

            if (isMalFormed) {
                //if nallength abnormal,ignore it.
                ALOGW("abnormal nallength, ignore this NAL");
                srcOffset = size;
                break;
            }

            if (nalLength == 0) {
                continue;
            }

            if (dstOffset > SIZE_MAX - 4 ||
                    dstOffset + 4 > SIZE_MAX - nalLength ||
                    dstOffset + 4 + nalLength > mBuffer->size()) {
                ALOGE("b/27208621 : %zu %zu", dstOffset, mBuffer->size());
                android_errorWriteLog(0x534e4554, "27208621");
                mBuffer->release();
                mBuffer = NULL;
                return AMEDIA_ERROR_MALFORMED;
            }

            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 1;
            memcpy(&dstData[dstOffset], &mSrcBuffer[srcOffset], nalLength);
            srcOffset += nalLength;
            dstOffset += nalLength;
        }
        CHECK_EQ(srcOffset, size);
        CHECK(mBuffer != NULL);
        mBuffer->set_range(0, dstOffset);

        AMediaFormat *meta = mBuffer->meta_data();
        AMediaFormat_clear(meta);
        AMediaFormat_setInt64(
                meta, AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
        AMediaFormat_setInt64(
                meta, AMEDIAFORMAT_KEY_DURATION, ((long double)stts * 1000000) / mTimescale);

        if (targetSampleTimeUs >= 0) {
            AMediaFormat_setInt64(
                    meta, AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
        }

        if (mIsAVC) {
            uint32_t layerId = FindAVCLayerId(
                    (const uint8_t *)mBuffer->data(), mBuffer->range_length());
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
        } else if (mIsHEVC) {
            int32_t layerId = parseHEVCLayerId(
                    (const uint8_t *)mBuffer->data(), mBuffer->range_length());
            if (layerId >= 0) {
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
            }
        }

        if (isSyncSample) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
        }

        ++mCurrentSampleIndex;

        *out = mBuffer;
        mBuffer = NULL;

        return AMEDIA_OK;
    }
}

media_status_t MPEG4Source::fragmentedRead(
        MediaBufferHelper **out, const ReadOptions *options) {

    ALOGV("MPEG4Source::fragmentedRead");

    CHECK(mStarted);

    *out = NULL;

    int64_t targetSampleTimeUs = -1;

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {

        seekTimeUs += ((long double)mElstShiftStartTicks * 1000000) / mTimescale;
        ALOGV("shifted seekTimeUs :%" PRId64 ", mElstShiftStartTicks:%" PRIu64, seekTimeUs,
              mElstShiftStartTicks);

        int numSidxEntries = mSegments.size();
        if (numSidxEntries != 0) {
            int64_t totalTime = 0;
            off64_t totalOffset = mFirstMoofOffset;
            for (int i = 0; i < numSidxEntries; i++) {
                const SidxEntry *se = &mSegments[i];
                if (totalTime + se->mDurationUs > seekTimeUs) {
                    // The requested time is somewhere in this segment
                    if ((mode == ReadOptions::SEEK_NEXT_SYNC && seekTimeUs > totalTime) ||
                        (mode == ReadOptions::SEEK_CLOSEST_SYNC &&
                        (seekTimeUs - totalTime) > (totalTime + se->mDurationUs - seekTimeUs))) {
                        // requested next sync, or closest sync and it was closer to the end of
                        // this segment
                        totalTime += se->mDurationUs;
                        totalOffset += se->mSize;
                    }
                    break;
                }
                totalTime += se->mDurationUs;
                totalOffset += se->mSize;
            }
            mCurrentMoofOffset = totalOffset;
            mNextMoofOffset = -1;
            mCurrentSamples.clear();
            mCurrentSampleIndex = 0;
            status_t err = parseChunk(&totalOffset);
            if (err != OK) {
                return AMEDIA_ERROR_UNKNOWN;
            }
            mCurrentTime = totalTime * mTimescale / 1000000ll;
        } else {
            // without sidx boxes, we can only seek to 0
            mCurrentMoofOffset = mFirstMoofOffset;
            mNextMoofOffset = -1;
            mCurrentSamples.clear();
            mCurrentSampleIndex = 0;
            off64_t tmp = mCurrentMoofOffset;
            status_t err = parseChunk(&tmp);
            if (err != OK) {
                return AMEDIA_ERROR_UNKNOWN;
            }
            mCurrentTime = 0;
        }

        if (mBuffer != NULL) {
            mBuffer->release();
            mBuffer = NULL;
        }

        // fall through
    }

    off64_t offset = 0;
    size_t size = 0;
    uint64_t cts = 0;
    bool isSyncSample = false;
    bool newBuffer = false;
    if (mBuffer == NULL || mCurrentSampleIndex >= mCurrentSamples.size()) {
        newBuffer = true;

        if (mBuffer != NULL) {
            mBuffer->release();
            mBuffer = NULL;
        }
        if (mCurrentSampleIndex >= mCurrentSamples.size()) {
            // move to next fragment if there is one
            if (mNextMoofOffset <= mCurrentMoofOffset) {
                return AMEDIA_ERROR_END_OF_STREAM;
            }
            off64_t nextMoof = mNextMoofOffset;
            mCurrentMoofOffset = nextMoof;
            mCurrentSamples.clear();
            mCurrentSampleIndex = 0;
            status_t err = parseChunk(&nextMoof);
            if (err != OK) {
                return AMEDIA_ERROR_UNKNOWN;
            }
            if (mCurrentSampleIndex >= mCurrentSamples.size()) {
                return AMEDIA_ERROR_END_OF_STREAM;
            }
        }

        const Sample *smpl = &mCurrentSamples[mCurrentSampleIndex];
        offset = smpl->offset;
        size = smpl->size;
        cts = mCurrentTime + smpl->compositionOffset;
        /* Composition Time Stamp cannot be negative. Some files have video Sample
        * Time(STTS)delta with zero value(b/117402420).  Hence subtract only
        * min(cts, mElstShiftStartTicks), so that audio tracks can be played.
        */
        cts -= std::min(cts, mElstShiftStartTicks);

        mCurrentTime += smpl->duration;
        isSyncSample = (mCurrentSampleIndex == 0);

        status_t err = mBufferGroup->acquire_buffer(&mBuffer);

        if (err != OK) {
            CHECK(mBuffer == NULL);
            ALOGV("acquire_buffer returned %d", err);
            return AMEDIA_ERROR_UNKNOWN;
        }
        if (size > mBuffer->size()) {
            ALOGE("buffer too small: %zu > %zu", size, mBuffer->size());
            mBuffer->release();
            mBuffer = NULL;
            return AMEDIA_ERROR_UNKNOWN;
        }
    }

    const Sample *smpl = &mCurrentSamples[mCurrentSampleIndex];
    AMediaFormat *bufmeta = mBuffer->meta_data();
    AMediaFormat_clear(bufmeta);
    if (smpl->encryptedsizes.size()) {
        // store clear/encrypted lengths in metadata
        AMediaFormat_setBuffer(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES,
                smpl->clearsizes.array(), smpl->clearsizes.size() * 4);
        AMediaFormat_setBuffer(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES,
                smpl->encryptedsizes.array(), smpl->encryptedsizes.size() * 4);
        AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_DEFAULT_IV_SIZE, mDefaultIVSize);
        AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_MODE, mCryptoMode);
        AMediaFormat_setBuffer(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_KEY, mCryptoKey, 16);
        AMediaFormat_setInt32(bufmeta,
                AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_BYTE_BLOCK, mDefaultEncryptedByteBlock);
        AMediaFormat_setInt32(bufmeta,
                AMEDIAFORMAT_KEY_CRYPTO_SKIP_BYTE_BLOCK, mDefaultSkipByteBlock);

        void *iv = NULL;
        size_t ivlength = 0;
        if (!AMediaFormat_getBuffer(mFormat,
                "crypto-iv", &iv, &ivlength)) {
            iv = (void *) smpl->iv;
            ivlength = 16; // use 16 or the actual size?
        }
        AMediaFormat_setBuffer(bufmeta, AMEDIAFORMAT_KEY_CRYPTO_IV, iv, ivlength);
    }

    if (!mIsAVC && !mIsHEVC) {
        if (newBuffer) {
            if (!isInRange((size_t)0u, mBuffer->size(), size)) {
                mBuffer->release();
                mBuffer = NULL;

                ALOGE("fragmentedRead ERROR_MALFORMED size %zu", size);
                return AMEDIA_ERROR_MALFORMED;
            }

            ssize_t num_bytes_read =
                mDataSource->readAt(offset, (uint8_t *)mBuffer->data(), size);

            if (num_bytes_read < (ssize_t)size) {
                mBuffer->release();
                mBuffer = NULL;

                ALOGE("i/o error");
                return AMEDIA_ERROR_IO;
            }

            CHECK(mBuffer != NULL);
            mBuffer->set_range(0, size);
            AMediaFormat_setInt64(bufmeta,
                    AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
            AMediaFormat_setInt64(bufmeta,
                    AMEDIAFORMAT_KEY_DURATION, ((long double)smpl->duration * 1000000) / mTimescale);

            if (targetSampleTimeUs >= 0) {
                AMediaFormat_setInt64(bufmeta, AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
            }

            if (mIsAVC) {
                uint32_t layerId = FindAVCLayerId(
                        (const uint8_t *)mBuffer->data(), mBuffer->range_length());
                AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
            } else if (mIsHEVC) {
                int32_t layerId = parseHEVCLayerId(
                        (const uint8_t *)mBuffer->data(), mBuffer->range_length());
                if (layerId >= 0) {
                    AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
                }
            }

            if (isSyncSample) {
                AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
            }

            ++mCurrentSampleIndex;
        }

        *out = mBuffer;
        mBuffer = NULL;

        return AMEDIA_OK;

    } else {
        ALOGV("whole NAL");
        // Whole NAL units are returned but each fragment is prefixed by
        // the start code (0x00 00 00 01).
        ssize_t num_bytes_read = 0;
        void *data = NULL;
        bool isMalFormed = false;
        int32_t max_size;
        if (!AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, &max_size)
                || !isInRange((size_t)0u, (size_t)max_size, size)) {
            isMalFormed = true;
        } else {
            data = mSrcBuffer;
        }

        if (isMalFormed || data == NULL) {
            ALOGE("isMalFormed size %zu", size);
            if (mBuffer != NULL) {
                mBuffer->release();
                mBuffer = NULL;
            }
            return AMEDIA_ERROR_MALFORMED;
        }
        num_bytes_read = mDataSource->readAt(offset, data, size);

        if (num_bytes_read < (ssize_t)size) {
            mBuffer->release();
            mBuffer = NULL;

            ALOGE("i/o error");
            return AMEDIA_ERROR_IO;
        }

        uint8_t *dstData = (uint8_t *)mBuffer->data();
        size_t srcOffset = 0;
        size_t dstOffset = 0;

        while (srcOffset < size) {
            isMalFormed = !isInRange((size_t)0u, size, srcOffset, mNALLengthSize);
            size_t nalLength = 0;
            if (!isMalFormed) {
                nalLength = parseNALSize(&mSrcBuffer[srcOffset]);
                srcOffset += mNALLengthSize;
                isMalFormed = !isInRange((size_t)0u, size, srcOffset, nalLength)
                        || !isInRange((size_t)0u, mBuffer->size(), dstOffset, (size_t)4u)
                        || !isInRange((size_t)0u, mBuffer->size(), dstOffset + 4, nalLength);
            }

            if (isMalFormed) {
                ALOGE("Video is malformed; nalLength %zu", nalLength);
                mBuffer->release();
                mBuffer = NULL;
                return AMEDIA_ERROR_MALFORMED;
            }

            if (nalLength == 0) {
                continue;
            }

            if (dstOffset > SIZE_MAX - 4 ||
                    dstOffset + 4 > SIZE_MAX - nalLength ||
                    dstOffset + 4 + nalLength > mBuffer->size()) {
                ALOGE("b/26365349 : %zu %zu", dstOffset, mBuffer->size());
                android_errorWriteLog(0x534e4554, "26365349");
                mBuffer->release();
                mBuffer = NULL;
                return AMEDIA_ERROR_MALFORMED;
            }

            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 0;
            dstData[dstOffset++] = 1;
            memcpy(&dstData[dstOffset], &mSrcBuffer[srcOffset], nalLength);
            srcOffset += nalLength;
            dstOffset += nalLength;
        }
        CHECK_EQ(srcOffset, size);
        CHECK(mBuffer != NULL);
        mBuffer->set_range(0, dstOffset);

        AMediaFormat *bufmeta = mBuffer->meta_data();
        AMediaFormat_setInt64(bufmeta,
                AMEDIAFORMAT_KEY_TIME_US, ((long double)cts * 1000000) / mTimescale);
        AMediaFormat_setInt64(bufmeta,
                AMEDIAFORMAT_KEY_DURATION, ((long double)smpl->duration * 1000000) / mTimescale);

        if (targetSampleTimeUs >= 0) {
            AMediaFormat_setInt64(bufmeta, AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
        }

        if (isSyncSample) {
            AMediaFormat_setInt32(bufmeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
        }

        ++mCurrentSampleIndex;

        *out = mBuffer;
        mBuffer = NULL;

        return AMEDIA_OK;
    }

    return AMEDIA_OK;
}

MPEG4Extractor::Track *MPEG4Extractor::findTrackByMimePrefix(
        const char *mimePrefix) {
    for (Track *track = mFirstTrack; track != NULL; track = track->next) {
        const char *mime;
        if (AMediaFormat_getString(track->meta, AMEDIAFORMAT_KEY_MIME, &mime)
                && !strncasecmp(mime, mimePrefix, strlen(mimePrefix))) {
            return track;
        }
    }

    return NULL;
}

static bool LegacySniffMPEG4(DataSourceHelper *source, float *confidence) {
    uint8_t header[8];

    ssize_t n = source->readAt(4, header, sizeof(header));
    if (n < (ssize_t)sizeof(header)) {
        return false;
    }

    if (!memcmp(header, "ftyp3gp", 7) || !memcmp(header, "ftypmp42", 8)
        || !memcmp(header, "ftyp3gr6", 8) || !memcmp(header, "ftyp3gs6", 8)
        || !memcmp(header, "ftyp3ge6", 8) || !memcmp(header, "ftyp3gg6", 8)
        || !memcmp(header, "ftypisom", 8) || !memcmp(header, "ftypM4V ", 8)
        || !memcmp(header, "ftypM4A ", 8) || !memcmp(header, "ftypf4v ", 8)
        || !memcmp(header, "ftypkddi", 8) || !memcmp(header, "ftypM4VP", 8)
        || !memcmp(header, "ftypmif1", 8) || !memcmp(header, "ftypheic", 8)
        || !memcmp(header, "ftypmsf1", 8) || !memcmp(header, "ftyphevc", 8)) {
        *confidence = 0.4;

        return true;
    }

    return false;
}

static bool isCompatibleBrand(uint32_t fourcc) {
    static const uint32_t kCompatibleBrands[] = {
        FOURCC("isom"),
        FOURCC("iso2"),
        FOURCC("avc1"),
        FOURCC("hvc1"),
        FOURCC("hev1"),
        FOURCC("av01"),
        FOURCC("3gp4"),
        FOURCC("mp41"),
        FOURCC("mp42"),
        FOURCC("dash"),
        FOURCC("nvr1"),

        // Won't promise that the following file types can be played.
        // Just give these file types a chance.
        FOURCC("qt  "),  // Apple's QuickTime
        FOURCC("MSNV"),  // Sony's PSP
        FOURCC("wmf "),

        FOURCC("3g2a"),  // 3GPP2
        FOURCC("3g2b"),
        FOURCC("mif1"),  // HEIF image
        FOURCC("heic"),  // HEIF image
        FOURCC("msf1"),  // HEIF image sequence
        FOURCC("hevc"),  // HEIF image sequence
    };

    for (size_t i = 0;
         i < sizeof(kCompatibleBrands) / sizeof(kCompatibleBrands[0]);
         ++i) {
        if (kCompatibleBrands[i] == fourcc) {
            return true;
        }
    }

    return false;
}

// Attempt to actually parse the 'ftyp' atom and determine if a suitable
// compatible brand is present.
// Also try to identify where this file's metadata ends
// (end of the 'moov' atom) and report it to the caller as part of
// the metadata.
static bool BetterSniffMPEG4(DataSourceHelper *source, float *confidence) {
    // We scan up to 128 bytes to identify this file as an MP4.
    static const off64_t kMaxScanOffset = 128ll;

    off64_t offset = 0ll;
    bool foundGoodFileType = false;
    off64_t moovAtomEndOffset = -1ll;
    bool done = false;

    while (!done && offset < kMaxScanOffset) {
        uint32_t hdr[2];
        if (source->readAt(offset, hdr, 8) < 8) {
            return false;
        }

        uint64_t chunkSize = ntohl(hdr[0]);
        uint32_t chunkType = ntohl(hdr[1]);
        off64_t chunkDataOffset = offset + 8;

        if (chunkSize == 1) {
            if (source->readAt(offset + 8, &chunkSize, 8) < 8) {
                return false;
            }

            chunkSize = ntoh64(chunkSize);
            chunkDataOffset += 8;

            if (chunkSize < 16) {
                // The smallest valid chunk is 16 bytes long in this case.
                return false;
            }

        } else if (chunkSize < 8) {
            // The smallest valid chunk is 8 bytes long.
            return false;
        }

        // (data_offset - offset) is either 8 or 16
        off64_t chunkDataSize = chunkSize - (chunkDataOffset - offset);
        if (chunkDataSize < 0) {
            ALOGE("b/23540914");
            return false;
        }

        char chunkstring[5];
        MakeFourCCString(chunkType, chunkstring);
        ALOGV("saw chunk type %s, size %" PRIu64 " @ %lld",
                chunkstring, chunkSize, (long long)offset);
        switch (chunkType) {
            case FOURCC("ftyp"):
            {
                if (chunkDataSize < 8) {
                    return false;
                }

                uint32_t numCompatibleBrands = (chunkDataSize - 8) / 4;
                for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
                    if (i == 1) {
                        // Skip this index, it refers to the minorVersion,
                        // not a brand.
                        continue;
                    }

                    uint32_t brand;
                    if (source->readAt(
                                chunkDataOffset + 4 * i, &brand, 4) < 4) {
                        return false;
                    }

                    brand = ntohl(brand);

                    if (isCompatibleBrand(brand)) {
                        foundGoodFileType = true;
                        break;
                    }
                }

                if (!foundGoodFileType) {
                    return false;
                }

                break;
            }

            case FOURCC("moov"):
            {
                moovAtomEndOffset = offset + chunkSize;

                done = true;
                break;
            }

            default:
                break;
        }

        offset += chunkSize;
    }

    if (!foundGoodFileType) {
        return false;
    }

    *confidence = 0.4f;

    return true;
}

static CMediaExtractor* CreateExtractor(CDataSource *source, void *) {
    return wrap(new MPEG4Extractor(new DataSourceHelper(source)));
}

static CreatorFunc Sniff(
        CDataSource *source, float *confidence, void **,
        FreeMetaFunc *) {
    DataSourceHelper helper(source);
    if (BetterSniffMPEG4(&helper, confidence)) {
        return CreateExtractor;
    }

    if (LegacySniffMPEG4(&helper, confidence)) {
        ALOGW("Identified supported mpeg4 through LegacySniffMPEG4.");
        return CreateExtractor;
    }

    return NULL;
}

static const char *extensions[] = {
    "3g2",
    "3ga",
    "3gp",
    "3gpp",
    "3gpp2",
    "m4a",
    "m4r",
    "m4v",
    "mov",
    "mp4",
    "qt",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("27575c67-4417-4c54-8d3d-8e626985a164"),
        2, // version
        "MP4 Extractor",
        { .v3 = {Sniff, extensions} },
    };
}

} // extern "C"

}  // namespace android
