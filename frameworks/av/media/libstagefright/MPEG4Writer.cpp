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
#define LOG_TAG "MPEG4Writer"

#include <algorithm>

#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/Log.h>

#include <functional>

#include <media/MediaSource.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/MPEG4Writer.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>
#include <media/mediarecorder.h>
#include <cutils/properties.h>

#include "include/ESDS.h"
#include "include/HevcUtils.h"

#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp) != 0, 0)
#endif

#define WARN_UNLESS(condition, message, ...) \
( (__predict_false(condition)) ? false : ({ \
    ALOGW("Condition %s failed "  message, #condition, ##__VA_ARGS__); \
    true; \
}))

namespace android {

static const int64_t kMinStreamableFileSizeInBytes = 5 * 1024 * 1024;
static const int64_t kMax32BitFileSize = 0x00ffffffffLL; // 2^32-1 : max FAT32
                                                         // filesystem file size
                                                         // used by most SD cards
static const uint8_t kNalUnitTypeSeqParamSet = 0x07;
static const uint8_t kNalUnitTypePicParamSet = 0x08;
static const int64_t kInitialDelayTimeUs     = 700000LL;
static const int64_t kMaxMetadataSize = 0x4000000LL;   // 64MB max per-frame metadata size
static const int64_t kMaxCttsOffsetTimeUs = 30 * 60 * 1000000LL;  // 30 minutes
static const size_t kESDSScratchBufferSize = 10;  // kMaxAtomSize in Mpeg4Extractor 64MB

static const char kMetaKey_Version[]    = "com.android.version";
static const char kMetaKey_Manufacturer[]      = "com.android.manufacturer";
static const char kMetaKey_Model[]      = "com.android.model";

#ifdef SHOW_BUILD
static const char kMetaKey_Build[]      = "com.android.build";
#endif
static const char kMetaKey_CaptureFps[] = "com.android.capture.fps";
static const char kMetaKey_TemporalLayerCount[] = "com.android.video.temporal_layers_count";

static const int kTimestampDebugCount = 10;
static const int kItemIdBase = 10000;
static const char kExifHeader[] = {'E', 'x', 'i', 'f', '\0', '\0'};
static const uint8_t kExifApp1Marker[] = {'E', 'x', 'i', 'f', 0xff, 0xe1};

static const uint8_t kMandatoryHevcNalUnitTypes[3] = {
    kHevcNalUnitTypeVps,
    kHevcNalUnitTypeSps,
    kHevcNalUnitTypePps,
};
static const uint8_t kHevcNalUnitTypes[5] = {
    kHevcNalUnitTypeVps,
    kHevcNalUnitTypeSps,
    kHevcNalUnitTypePps,
    kHevcNalUnitTypePrefixSei,
    kHevcNalUnitTypeSuffixSei,
};
/* uncomment to include build in meta */
//#define SHOW_MODEL_BUILD 1

class MPEG4Writer::Track {
public:
    Track(MPEG4Writer *owner, const sp<MediaSource> &source, size_t trackId);

    ~Track();

    status_t start(MetaData *params);
    status_t stop(bool stopSource = true);
    status_t pause();
    bool reachedEOS();

    int64_t getDurationUs() const;
    int64_t getEstimatedTrackSizeBytes() const;
    int32_t getMetaSizeIncrease(int32_t angle, int32_t trackCount) const;
    void writeTrackHeader(bool use32BitOffset = true);
    int64_t getMinCttsOffsetTimeUs();
    void bufferChunk(int64_t timestampUs);
    bool isAvc() const { return mIsAvc; }
    bool isHevc() const { return mIsHevc; }
    bool isHeic() const { return mIsHeic; }
    bool isAudio() const { return mIsAudio; }
    bool isMPEG4() const { return mIsMPEG4; }
    bool usePrefix() const { return mIsAvc || mIsHevc || mIsHeic; }
    bool isExifData(MediaBufferBase *buffer, uint32_t *tiffHdrOffset) const;
    void addChunkOffset(off64_t offset);
    void addItemOffsetAndSize(off64_t offset, size_t size, bool isExif);
    void flushItemRefs();
    int32_t getTrackId() const { return mTrackId; }
    status_t dump(int fd, const Vector<String16>& args) const;
    static const char *getFourCCForMime(const char *mime);
    const char *getTrackType() const;
    void resetInternal();

private:
    // A helper class to handle faster write box with table entries
    template<class TYPE, unsigned ENTRY_SIZE>
    // ENTRY_SIZE: # of values in each entry
    struct ListTableEntries {
        static_assert(ENTRY_SIZE > 0, "ENTRY_SIZE must be positive");
        ListTableEntries(uint32_t elementCapacity)
            : mElementCapacity(elementCapacity),
            mTotalNumTableEntries(0),
            mNumValuesInCurrEntry(0),
            mCurrTableEntriesElement(NULL) {
            CHECK_GT(mElementCapacity, 0u);
            // Ensure no integer overflow on allocation in add().
            CHECK_LT(ENTRY_SIZE, UINT32_MAX / mElementCapacity);
        }

        // Free the allocated memory.
        ~ListTableEntries() {
            while (!mTableEntryList.empty()) {
                typename List<TYPE *>::iterator it = mTableEntryList.begin();
                delete[] (*it);
                mTableEntryList.erase(it);
            }
        }

        // Replace the value at the given position by the given value.
        // There must be an existing value at the given position.
        // @arg value must be in network byte order
        // @arg pos location the value must be in.
        void set(const TYPE& value, uint32_t pos) {
            CHECK_LT(pos, mTotalNumTableEntries * ENTRY_SIZE);

            typename List<TYPE *>::iterator it = mTableEntryList.begin();
            uint32_t iterations = (pos / (mElementCapacity * ENTRY_SIZE));
            while (it != mTableEntryList.end() && iterations > 0) {
                ++it;
                --iterations;
            }
            CHECK(it != mTableEntryList.end());
            CHECK_EQ(iterations, 0u);

            (*it)[(pos % (mElementCapacity * ENTRY_SIZE))] = value;
        }

        // Get the value at the given position by the given value.
        // @arg value the retrieved value at the position in network byte order.
        // @arg pos location the value must be in.
        // @return true if a value is found.
        bool get(TYPE& value, uint32_t pos) const {
            if (pos >= mTotalNumTableEntries * ENTRY_SIZE) {
                return false;
            }

            typename List<TYPE *>::iterator it = mTableEntryList.begin();
            uint32_t iterations = (pos / (mElementCapacity * ENTRY_SIZE));
            while (it != mTableEntryList.end() && iterations > 0) {
                ++it;
                --iterations;
            }
            CHECK(it != mTableEntryList.end());
            CHECK_EQ(iterations, 0u);

            value = (*it)[(pos % (mElementCapacity * ENTRY_SIZE))];
            return true;
        }

        // adjusts all values by |adjust(value)|
        void adjustEntries(
                std::function<void(size_t /* ix */, TYPE(& /* entry */)[ENTRY_SIZE])> update) {
            size_t nEntries = mTotalNumTableEntries + mNumValuesInCurrEntry / ENTRY_SIZE;
            size_t ix = 0;
            for (TYPE *entryArray : mTableEntryList) {
                size_t num = std::min(nEntries, (size_t)mElementCapacity);
                for (size_t i = 0; i < num; ++i) {
                    update(ix++, (TYPE(&)[ENTRY_SIZE])(*entryArray));
                    entryArray += ENTRY_SIZE;
                }
                nEntries -= num;
            }
        }

        // Store a single value.
        // @arg value must be in network byte order.
        void add(const TYPE& value) {
            CHECK_LT(mNumValuesInCurrEntry, mElementCapacity);
            uint32_t nEntries = mTotalNumTableEntries % mElementCapacity;
            uint32_t nValues  = mNumValuesInCurrEntry % ENTRY_SIZE;
            if (nEntries == 0 && nValues == 0) {
                mCurrTableEntriesElement = new TYPE[ENTRY_SIZE * mElementCapacity];
                CHECK(mCurrTableEntriesElement != NULL);
                mTableEntryList.push_back(mCurrTableEntriesElement);
            }

            uint32_t pos = nEntries * ENTRY_SIZE + nValues;
            mCurrTableEntriesElement[pos] = value;

            ++mNumValuesInCurrEntry;
            if ((mNumValuesInCurrEntry % ENTRY_SIZE) == 0) {
                ++mTotalNumTableEntries;
                mNumValuesInCurrEntry = 0;
            }
        }

        // Write out the table entries:
        // 1. the number of entries goes first
        // 2. followed by the values in the table enties in order
        // @arg writer the writer to actual write to the storage
        void write(MPEG4Writer *writer) const {
            CHECK_EQ(mNumValuesInCurrEntry % ENTRY_SIZE, 0u);
            uint32_t nEntries = mTotalNumTableEntries;
            writer->writeInt32(nEntries);
            for (typename List<TYPE *>::iterator it = mTableEntryList.begin();
                it != mTableEntryList.end(); ++it) {
                CHECK_GT(nEntries, 0u);
                if (nEntries >= mElementCapacity) {
                    writer->write(*it, sizeof(TYPE) * ENTRY_SIZE, mElementCapacity);
                    nEntries -= mElementCapacity;
                } else {
                    writer->write(*it, sizeof(TYPE) * ENTRY_SIZE, nEntries);
                    break;
                }
            }
        }

        // Return the number of entries in the table.
        uint32_t count() const { return mTotalNumTableEntries; }

    private:
        uint32_t         mElementCapacity;  // # entries in an element
        uint32_t         mTotalNumTableEntries;
        uint32_t         mNumValuesInCurrEntry;  // up to ENTRY_SIZE
        TYPE             *mCurrTableEntriesElement;
        mutable List<TYPE *>     mTableEntryList;

        DISALLOW_EVIL_CONSTRUCTORS(ListTableEntries);
    };



    MPEG4Writer *mOwner;
    sp<MetaData> mMeta;
    sp<MediaSource> mSource;
    volatile bool mDone;
    volatile bool mPaused;
    volatile bool mResumed;
    volatile bool mStarted;
    bool mIsAvc;
    bool mIsHevc;
    bool mIsAudio;
    bool mIsVideo;
    bool mIsHeic;
    bool mIsMPEG4;
    bool mGotStartKeyFrame;
    bool mIsMalformed;
    int32_t mTrackId;
    int64_t mTrackDurationUs;
    int64_t mMaxChunkDurationUs;
    int64_t mLastDecodingTimeUs;

    int64_t mEstimatedTrackSizeBytes;
    int64_t mMdatSizeBytes;
    int32_t mTimeScale;

    pthread_t mThread;


    List<MediaBuffer *> mChunkSamples;

    bool                mSamplesHaveSameSize;
    ListTableEntries<uint32_t, 1> *mStszTableEntries;

    ListTableEntries<uint32_t, 1> *mStcoTableEntries;
    ListTableEntries<off64_t, 1> *mCo64TableEntries;
    ListTableEntries<uint32_t, 3> *mStscTableEntries;
    ListTableEntries<uint32_t, 1> *mStssTableEntries;
    ListTableEntries<uint32_t, 2> *mSttsTableEntries;
    ListTableEntries<uint32_t, 2> *mCttsTableEntries;
    ListTableEntries<uint32_t, 3> *mElstTableEntries; // 3columns: segDuration, mediaTime, mediaRate

    int64_t mMinCttsOffsetTimeUs;
    int64_t mMinCttsOffsetTicks;
    int64_t mMaxCttsOffsetTicks;

    // Save the last 10 frames' timestamp and frame type for debug.
    struct TimestampDebugHelperEntry {
        int64_t pts;
        int64_t dts;
        std::string frameType;
    };

    std::list<TimestampDebugHelperEntry> mTimestampDebugHelper;

    // Sequence parameter set or picture parameter set
    struct AVCParamSet {
        AVCParamSet(uint16_t length, const uint8_t *data)
            : mLength(length), mData(data) {}

        uint16_t mLength;
        const uint8_t *mData;
    };
    List<AVCParamSet> mSeqParamSets;
    List<AVCParamSet> mPicParamSets;
    uint8_t mProfileIdc;
    uint8_t mProfileCompatible;
    uint8_t mLevelIdc;

    void *mCodecSpecificData;
    size_t mCodecSpecificDataSize;
    bool mGotAllCodecSpecificData;
    bool mTrackingProgressStatus;

    bool mReachedEOS;
    int64_t mStartTimestampUs;
    int64_t mStartTimeRealUs;
    int64_t mFirstSampleTimeRealUs;
    int64_t mPreviousTrackTimeUs;
    int64_t mTrackEveryTimeDurationUs;

    int32_t mRotation;

    Vector<uint16_t> mProperties;
    ItemRefs mDimgRefs;
    Vector<uint16_t> mExifList;
    uint16_t mImageItemId;
    int32_t mIsPrimary;
    int32_t mWidth, mHeight;
    int32_t mTileWidth, mTileHeight;
    int32_t mGridRows, mGridCols;
    size_t mNumTiles, mTileIndex;

    // Update the audio track's drift information.
    void updateDriftTime(const sp<MetaData>& meta);

    void dumpTimeStamps();

    int64_t getStartTimeOffsetTimeUs() const;
    int32_t getStartTimeOffsetScaledTime() const;

    static void *ThreadWrapper(void *me);
    status_t threadEntry();

    const uint8_t *parseParamSet(
        const uint8_t *data, size_t length, int type, size_t *paramSetLen);

    status_t copyCodecSpecificData(const uint8_t *data, size_t size, size_t minLength = 0);

    status_t makeAVCCodecSpecificData(const uint8_t *data, size_t size);
    status_t copyAVCCodecSpecificData(const uint8_t *data, size_t size);
    status_t parseAVCCodecSpecificData(const uint8_t *data, size_t size);

    status_t makeHEVCCodecSpecificData(const uint8_t *data, size_t size);
    status_t copyHEVCCodecSpecificData(const uint8_t *data, size_t size);
    status_t parseHEVCCodecSpecificData(
            const uint8_t *data, size_t size, HevcParameterSets &paramSets);

    // Track authoring progress status
    void trackProgressStatus(int64_t timeUs, status_t err = OK);
    void initTrackingProgressStatus(MetaData *params);

    void getCodecSpecificDataFromInputFormatIfPossible();

    // Determine the track time scale
    // If it is an audio track, try to use the sampling rate as
    // the time scale; however, if user chooses the overwrite
    // value, the user-supplied time scale will be used.
    void setTimeScale();

    // Simple validation on the codec specific data
    status_t checkCodecSpecificData() const;

    void updateTrackSizeEstimate();
    void addOneStscTableEntry(size_t chunkId, size_t sampleId);
    void addOneStssTableEntry(size_t sampleId);

    // Duration is time scale based
    void addOneSttsTableEntry(size_t sampleCount, int32_t timescaledDur);
    void addOneCttsTableEntry(size_t sampleCount, int32_t timescaledDur);
    void addOneElstTableEntry(uint32_t segmentDuration, int32_t mediaTime,
        int16_t mediaRate, int16_t mediaRateFraction);

    bool isTrackMalFormed() const;
    void sendTrackSummary(bool hasMultipleTracks);

    // Write the boxes
    void writeStcoBox(bool use32BitOffset);
    void writeStscBox();
    void writeStszBox();
    void writeStssBox();
    void writeSttsBox();
    void writeCttsBox();
    void writeD263Box();
    void writePaspBox();
    void writeAvccBox();
    void writeHvccBox();
    void writeUrlBox();
    void writeDrefBox();
    void writeDinfBox();
    void writeDamrBox();
    void writeMdhdBox(uint32_t now);
    void writeSmhdBox();
    void writeVmhdBox();
    void writeNmhdBox();
    void writeHdlrBox();
    void writeTkhdBox(uint32_t now);
    void writeColrBox();
    void writeMp4aEsdsBox();
    void writeMp4vEsdsBox();
    void writeAudioFourCCBox();
    void writeVideoFourCCBox();
    void writeMetadataFourCCBox();
    void writeStblBox(bool use32BitOffset);
    void writeEdtsBox();

    Track(const Track &);
    Track &operator=(const Track &);
};

MPEG4Writer::MPEG4Writer(int fd) {
    initInternal(dup(fd), true /*isFirstSession*/);
}

MPEG4Writer::~MPEG4Writer() {
    reset();

    while (!mTracks.empty()) {
        List<Track *>::iterator it = mTracks.begin();
        delete *it;
        (*it) = NULL;
        mTracks.erase(it);
    }
    mTracks.clear();

    if (mNextFd != -1) {
        close(mNextFd);
    }
}

void MPEG4Writer::initInternal(int fd, bool isFirstSession) {
    ALOGV("initInternal");
    mFd = fd;
    mNextFd = -1;
    mInitCheck = mFd < 0? NO_INIT: OK;

    mInterleaveDurationUs = 1000000;

    mStartTimestampUs = -1LL;
    mStartTimeOffsetMs = -1;
    mStartTimeOffsetBFramesUs = 0;
    mPaused = false;
    mStarted = false;
    mWriterThreadStarted = false;
    mSendNotify = false;

    // Reset following variables for all the sessions and they will be
    // initialized in start(MetaData *param).
    mIsRealTimeRecording = true;
    mUse4ByteNalLength = true;
    mUse32BitOffset = true;
    mOffset = 0;
    mMdatOffset = 0;
    mInMemoryCache = NULL;
    mInMemoryCacheOffset = 0;
    mInMemoryCacheSize = 0;
    mWriteBoxToMemory = false;
    mFreeBoxOffset = 0;
    mStreamableFile = false;
    mTimeScale = -1;
    mHasFileLevelMeta = false;
    mPrimaryItemId = 0;
    mAssociationEntryCount = 0;
    mNumGrids = 0;
    mHasRefs = false;

    // Following variables only need to be set for the first recording session.
    // And they will stay the same for all the recording sessions.
    if (isFirstSession) {
        mMoovExtraSize = 0;
        mHasMoovBox = false;
        mMetaKeys = new AMessage();
        addDeviceMeta();
        mLatitudex10000 = 0;
        mLongitudex10000 = 0;
        mAreGeoTagsAvailable = false;
        mSwitchPending = false;
        mIsFileSizeLimitExplicitlyRequested = false;
    }

    // Verify mFd is seekable
    off64_t off = lseek64(mFd, 0, SEEK_SET);
    if (off < 0) {
        ALOGE("cannot seek mFd: %s (%d) %lld", strerror(errno), errno, (long long)mFd);
        release();
    }
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        (*it)->resetInternal();
    }
}

status_t MPEG4Writer::dump(
        int fd, const Vector<String16>& args) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "   MPEG4Writer %p\n", this);
    result.append(buffer);
    snprintf(buffer, SIZE, "     mStarted: %s\n", mStarted? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        (*it)->dump(fd, args);
    }
    return OK;
}

status_t MPEG4Writer::Track::dump(
        int fd, const Vector<String16>& /* args */) const {
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "     %s track\n", getTrackType());
    result.append(buffer);
    snprintf(buffer, SIZE, "       reached EOS: %s\n",
            mReachedEOS? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "       frames encoded : %d\n", mStszTableEntries->count());
    result.append(buffer);
    snprintf(buffer, SIZE, "       duration encoded : %" PRId64 " us\n", mTrackDurationUs);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return OK;
}

// static
const char *MPEG4Writer::Track::getFourCCForMime(const char *mime) {
    if (mime == NULL) {
        return NULL;
    }
    if (!strncasecmp(mime, "audio/", 6)) {
        if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_NB, mime)) {
            return "samr";
        } else if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_WB, mime)) {
            return "sawb";
        } else if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AAC, mime)) {
            return "mp4a";
        }
    } else if (!strncasecmp(mime, "video/", 6)) {
        if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_MPEG4, mime)) {
            return "mp4v";
        } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_H263, mime)) {
            return "s263";
        } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_AVC, mime)) {
            return "avc1";
        } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_HEVC, mime)) {
            return "hvc1";
        }
    } else if (!strncasecmp(mime, "application/", 12)) {
        return "mett";
    } else if (!strcasecmp(MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC, mime)) {
        return "heic";
    } else {
        ALOGE("Track (%s) other than video/audio/metadata is not supported", mime);
    }
    return NULL;
}

status_t MPEG4Writer::addSource(const sp<MediaSource> &source) {
    Mutex::Autolock l(mLock);
    if (mStarted) {
        ALOGE("Attempt to add source AFTER recording is started");
        return UNKNOWN_ERROR;
    }

    CHECK(source.get() != NULL);

    const char *mime;
    source->getFormat()->findCString(kKeyMIMEType, &mime);

    if (Track::getFourCCForMime(mime) == NULL) {
        ALOGE("Unsupported mime '%s'", mime);
        return ERROR_UNSUPPORTED;
    }

    // This is a metadata track or the first track of either audio or video
    // Go ahead to add the track.
    Track *track = new Track(this, source, 1 + mTracks.size());
    mTracks.push_back(track);

    mHasMoovBox |= !track->isHeic();
    mHasFileLevelMeta |= track->isHeic();

    return OK;
}

status_t MPEG4Writer::startTracks(MetaData *params) {
    if (mTracks.empty()) {
        ALOGE("No source added");
        return INVALID_OPERATION;
    }

    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        status_t err = (*it)->start(params);

        if (err != OK) {
            for (List<Track *>::iterator it2 = mTracks.begin();
                 it2 != it; ++it2) {
                (*it2)->stop();
            }

            return err;
        }
    }
    return OK;
}

void MPEG4Writer::addDeviceMeta() {
    // add device info and estimate space in 'moov'
    char val[PROPERTY_VALUE_MAX];
    size_t n;
    // meta size is estimated by adding up the following:
    // - meta header structures, which occur only once (total 66 bytes)
    // - size for each key, which consists of a fixed header (32 bytes),
    //   plus key length and data length.
    mMoovExtraSize += 66;
    if (property_get("ro.build.version.release", val, NULL)
            && (n = strlen(val)) > 0) {
        mMetaKeys->setString(kMetaKey_Version, val, n + 1);
        mMoovExtraSize += sizeof(kMetaKey_Version) + n + 32;
    }

    if (property_get_bool("media.recorder.show_manufacturer_and_model", false)) {
        if (property_get("ro.product.manufacturer", val, NULL)
                && (n = strlen(val)) > 0) {
            mMetaKeys->setString(kMetaKey_Manufacturer, val, n + 1);
            mMoovExtraSize += sizeof(kMetaKey_Manufacturer) + n + 32;
        }
        if (property_get("ro.product.model", val, NULL)
                && (n = strlen(val)) > 0) {
            mMetaKeys->setString(kMetaKey_Model, val, n + 1);
            mMoovExtraSize += sizeof(kMetaKey_Model) + n + 32;
        }
    }
#ifdef SHOW_MODEL_BUILD
    if (property_get("ro.build.display.id", val, NULL)
            && (n = strlen(val)) > 0) {
        mMetaKeys->setString(kMetaKey_Build, val, n + 1);
        mMoovExtraSize += sizeof(kMetaKey_Build) + n + 32;
    }
#endif
}

int64_t MPEG4Writer::estimateFileLevelMetaSize(MetaData *params) {
    int32_t rotation;
    if (!params || !params->findInt32(kKeyRotation, &rotation)) {
        rotation = 0;
    }

    // base meta size
    int64_t metaSize =     12  // meta fullbox header
                         + 33  // hdlr box
                         + 14  // pitm box
                         + 16  // iloc box (fixed size portion)
                         + 14  // iinf box (fixed size portion)
                         + 32  // iprp box (fixed size protion)
                         + 8   // idat box (when empty)
                         + 12  // iref box (when empty)
                         ;

    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        if ((*it)->isHeic()) {
            metaSize += (*it)->getMetaSizeIncrease(rotation, mTracks.size());
        }
    }

    ALOGV("estimated meta size: %lld", (long long) metaSize);

    // Need at least 8-byte padding at the end, otherwise the left-over
    // freebox may become malformed
    return metaSize + 8;
}

int64_t MPEG4Writer::estimateMoovBoxSize(int32_t bitRate) {
    // This implementation is highly experimental/heurisitic.
    //
    // Statistical analysis shows that metadata usually accounts
    // for a small portion of the total file size, usually < 0.6%.

    // The default MIN_MOOV_BOX_SIZE is set to 0.6% x 1MB / 2,
    // where 1MB is the common file size limit for MMS application.
    // The default MAX _MOOV_BOX_SIZE value is based on about 3
    // minute video recording with a bit rate about 3 Mbps, because
    // statistics also show that most of the video captured are going
    // to be less than 3 minutes.

    // If the estimation is wrong, we will pay the price of wasting
    // some reserved space. This should not happen so often statistically.
    static const int32_t factor = mUse32BitOffset? 1: 2;
    static const int64_t MIN_MOOV_BOX_SIZE = 3 * 1024;  // 3 KB
    static const int64_t MAX_MOOV_BOX_SIZE = (180 * 3000000 * 6LL / 8000);
    int64_t size = MIN_MOOV_BOX_SIZE;

    // Max file size limit is set
    if (mMaxFileSizeLimitBytes != 0 && mIsFileSizeLimitExplicitlyRequested) {
        size = mMaxFileSizeLimitBytes * 6 / 1000;
    }

    // Max file duration limit is set
    if (mMaxFileDurationLimitUs != 0) {
        if (bitRate > 0) {
            int64_t size2 =
                ((mMaxFileDurationLimitUs / 1000) * bitRate * 6) / 8000000;
            if (mMaxFileSizeLimitBytes != 0 && mIsFileSizeLimitExplicitlyRequested) {
                // When both file size and duration limits are set,
                // we use the smaller limit of the two.
                if (size > size2) {
                    size = size2;
                }
            } else {
                // Only max file duration limit is set
                size = size2;
            }
        }
    }

    if (size < MIN_MOOV_BOX_SIZE) {
        size = MIN_MOOV_BOX_SIZE;
    }

    // Any long duration recording will be probably end up with
    // non-streamable mp4 file.
    if (size > MAX_MOOV_BOX_SIZE) {
        size = MAX_MOOV_BOX_SIZE;
    }

    // Account for the extra stuff (Geo, meta keys, etc.)
    size += mMoovExtraSize;

    ALOGI("limits: %" PRId64 "/%" PRId64 " bytes/us, bit rate: %d bps and the"
         " estimated moov size %" PRId64 " bytes",
         mMaxFileSizeLimitBytes, mMaxFileDurationLimitUs, bitRate, size);

    int64_t estimatedSize = factor * size;
    CHECK_GE(estimatedSize, 8);

    return estimatedSize;
}

status_t MPEG4Writer::start(MetaData *param) {
    if (mInitCheck != OK) {
        return UNKNOWN_ERROR;
    }
    mStartMeta = param;

    /*
     * Check mMaxFileSizeLimitBytes at the beginning
     * since mMaxFileSizeLimitBytes may be implicitly
     * changed later for 32-bit file offset even if
     * user does not ask to set it explicitly.
     */
    if (mMaxFileSizeLimitBytes != 0) {
        mIsFileSizeLimitExplicitlyRequested = true;
    }

    int32_t use64BitOffset;
    if (param &&
        param->findInt32(kKey64BitFileOffset, &use64BitOffset) &&
        use64BitOffset) {
        mUse32BitOffset = false;
    }

    if (mUse32BitOffset) {
        // Implicit 32 bit file size limit
        if (mMaxFileSizeLimitBytes == 0) {
            mMaxFileSizeLimitBytes = kMax32BitFileSize;
        }

        // If file size is set to be larger than the 32 bit file
        // size limit, treat it as an error.
        if (mMaxFileSizeLimitBytes > kMax32BitFileSize) {
            ALOGW("32-bit file size limit (%" PRId64 " bytes) too big. "
                 "It is changed to %" PRId64 " bytes",
                mMaxFileSizeLimitBytes, kMax32BitFileSize);
            mMaxFileSizeLimitBytes = kMax32BitFileSize;
        }
    }

    int32_t use2ByteNalLength;
    if (param &&
        param->findInt32(kKey2ByteNalLength, &use2ByteNalLength) &&
        use2ByteNalLength) {
        mUse4ByteNalLength = false;
    }

    int32_t isRealTimeRecording;
    if (param && param->findInt32(kKeyRealTimeRecording, &isRealTimeRecording)) {
        mIsRealTimeRecording = isRealTimeRecording;
    }

    mStartTimestampUs = -1;

    if (mStarted) {
        if (mPaused) {
            mPaused = false;
            return startTracks(param);
        }
        return OK;
    }

    if (!param ||
        !param->findInt32(kKeyTimeScale, &mTimeScale)) {
        mTimeScale = 1000;
    }
    CHECK_GT(mTimeScale, 0);
    ALOGV("movie time scale: %d", mTimeScale);

    /*
     * When the requested file size limit is small, the priority
     * is to meet the file size limit requirement, rather than
     * to make the file streamable. mStreamableFile does not tell
     * whether the actual recorded file is streamable or not.
     */
    mStreamableFile =
        (mMaxFileSizeLimitBytes != 0 &&
         mMaxFileSizeLimitBytes >= kMinStreamableFileSizeInBytes);

    /*
     * mWriteBoxToMemory is true if the amount of data in a file-level meta or
     * moov box is smaller than the reserved free space at the beginning of a
     * file, AND when the content of the box is constructed. Note that video/
     * audio frame data is always written to the file but not in the memory.
     *
     * Before stop()/reset() is called, mWriteBoxToMemory is always
     * false. When reset() is called at the end of a recording session,
     * file-level meta and/or moov box needs to be constructed.
     *
     * 1) Right before the box is constructed, mWriteBoxToMemory to set to
     * mStreamableFile so that if the file is intended to be streamable, it
     * is set to true; otherwise, it is set to false. When the value is set
     * to false, all the content of that box is written immediately to
     * the end of the file. When the value is set to true, all the
     * content of that box is written to an in-memory cache,
     * mInMemoryCache, util the following condition happens. Note
     * that the size of the in-memory cache is the same as the
     * reserved free space at the beginning of the file.
     *
     * 2) While the data of the box is written to an in-memory
     * cache, the data size is checked against the reserved space.
     * If the data size surpasses the reserved space, subsequent box data
     * could no longer be hold in the in-memory cache. This also
     * indicates that the reserved space was too small. At this point,
     * _all_ subsequent box data must be written to the end of the file.
     * mWriteBoxToMemory must be set to false to direct the write
     * to the file.
     *
     * 3) If the data size in the box is smaller than the reserved
     * space after the box is completely constructed, the in-memory
     * cache copy of the box is written to the reserved free space.
     * mWriteBoxToMemory is always set to false after all boxes that
     * using the in-memory cache have been constructed.
     */
    mWriteBoxToMemory = false;
    mInMemoryCache = NULL;
    mInMemoryCacheOffset = 0;


    ALOGV("muxer starting: mHasMoovBox %d, mHasFileLevelMeta %d",
            mHasMoovBox, mHasFileLevelMeta);

    writeFtypBox(param);

    mFreeBoxOffset = mOffset;

    if (mInMemoryCacheSize == 0) {
        int32_t bitRate = -1;
        if (mHasFileLevelMeta) {
            mInMemoryCacheSize += estimateFileLevelMetaSize(param);
        }
        if (mHasMoovBox) {
            if (param) {
                param->findInt32(kKeyBitRate, &bitRate);
            }
            mInMemoryCacheSize += estimateMoovBoxSize(bitRate);
        }
    }
    if (mStreamableFile) {
        // Reserve a 'free' box only for streamable file
        lseek64(mFd, mFreeBoxOffset, SEEK_SET);
        writeInt32(mInMemoryCacheSize);
        write("free", 4);
        mMdatOffset = mFreeBoxOffset + mInMemoryCacheSize;
    } else {
        mMdatOffset = mOffset;
    }

    mOffset = mMdatOffset;
    lseek64(mFd, mMdatOffset, SEEK_SET);
    if (mUse32BitOffset) {
        write("????mdat", 8);
    } else {
        write("\x00\x00\x00\x01mdat????????", 16);
    }

    status_t err = startWriterThread();
    if (err != OK) {
        return err;
    }

    err = startTracks(param);
    if (err != OK) {
        return err;
    }

    mStarted = true;
    return OK;
}

bool MPEG4Writer::use32BitFileOffset() const {
    return mUse32BitOffset;
}

status_t MPEG4Writer::pause() {
    ALOGW("MPEG4Writer: pause is not supported");
    return ERROR_UNSUPPORTED;
}

void MPEG4Writer::stopWriterThread() {
    ALOGD("Stopping writer thread");
    if (!mWriterThreadStarted) {
        return;
    }

    {
        Mutex::Autolock autolock(mLock);

        mDone = true;
        mChunkReadyCondition.signal();
    }

    void *dummy;
    pthread_join(mThread, &dummy);
    mWriterThreadStarted = false;
    ALOGD("Writer thread stopped");
}

/*
 * MP4 file standard defines a composition matrix:
 * | a  b  u |
 * | c  d  v |
 * | x  y  w |
 *
 * the element in the matrix is stored in the following
 * order: {a, b, u, c, d, v, x, y, w},
 * where a, b, c, d, x, and y is in 16.16 format, while
 * u, v and w is in 2.30 format.
 */
void MPEG4Writer::writeCompositionMatrix(int degrees) {
    ALOGV("writeCompositionMatrix");
    uint32_t a = 0x00010000;
    uint32_t b = 0;
    uint32_t c = 0;
    uint32_t d = 0x00010000;
    switch (degrees) {
        case 0:
            break;
        case 90:
            a = 0;
            b = 0x00010000;
            c = 0xFFFF0000;
            d = 0;
            break;
        case 180:
            a = 0xFFFF0000;
            d = 0xFFFF0000;
            break;
        case 270:
            a = 0;
            b = 0xFFFF0000;
            c = 0x00010000;
            d = 0;
            break;
        default:
            CHECK(!"Should never reach this unknown rotation");
            break;
    }

    writeInt32(a);           // a
    writeInt32(b);           // b
    writeInt32(0);           // u
    writeInt32(c);           // c
    writeInt32(d);           // d
    writeInt32(0);           // v
    writeInt32(0);           // x
    writeInt32(0);           // y
    writeInt32(0x40000000);  // w
}

void MPEG4Writer::release() {
    close(mFd);
    mFd = -1;
    if (mNextFd != -1) {
        close(mNextFd);
        mNextFd = -1;
    }
    mInitCheck = NO_INIT;
    mStarted = false;
    free(mInMemoryCache);
    mInMemoryCache = NULL;
}

void MPEG4Writer::finishCurrentSession() {
    reset(false /* stopSource */);
}

status_t MPEG4Writer::switchFd() {
    ALOGV("switchFd");
    Mutex::Autolock l(mLock);
    if (mSwitchPending) {
        return OK;
    }

    if (mNextFd == -1) {
        ALOGW("No FileDescripter for next recording");
        return INVALID_OPERATION;
    }

    mSwitchPending = true;
    sp<AMessage> msg = new AMessage(kWhatSwitch, mReflector);
    status_t err = msg->post();

    return err;
}

status_t MPEG4Writer::reset(bool stopSource) {
    if (mInitCheck != OK) {
        return OK;
    } else {
        if (!mWriterThreadStarted ||
            !mStarted) {
            if (mWriterThreadStarted) {
                stopWriterThread();
            }
            release();
            return OK;
        }
    }

    status_t err = OK;
    int64_t maxDurationUs = 0;
    int64_t minDurationUs = 0x7fffffffffffffffLL;
    int32_t nonImageTrackCount = 0;
    for (List<Track *>::iterator it = mTracks.begin();
        it != mTracks.end(); ++it) {
        status_t status = (*it)->stop(stopSource);
        if (err == OK && status != OK) {
            err = status;
        }

        // skip image tracks
        if ((*it)->isHeic()) continue;
        nonImageTrackCount++;

        int64_t durationUs = (*it)->getDurationUs();
        if (durationUs > maxDurationUs) {
            maxDurationUs = durationUs;
        }
        if (durationUs < minDurationUs) {
            minDurationUs = durationUs;
        }
    }

    if (nonImageTrackCount > 1) {
        ALOGD("Duration from tracks range is [%" PRId64 ", %" PRId64 "] us",
            minDurationUs, maxDurationUs);
    }

    stopWriterThread();

    // Do not write out movie header on error.
    if (err != OK) {
        release();
        return err;
    }

    // Fix up the size of the 'mdat' chunk.
    if (mUse32BitOffset) {
        lseek64(mFd, mMdatOffset, SEEK_SET);
        uint32_t size = htonl(static_cast<uint32_t>(mOffset - mMdatOffset));
        ::write(mFd, &size, 4);
    } else {
        lseek64(mFd, mMdatOffset + 8, SEEK_SET);
        uint64_t size = mOffset - mMdatOffset;
        size = hton64(size);
        ::write(mFd, &size, 8);
    }
    lseek64(mFd, mOffset, SEEK_SET);

    // Construct file-level meta and moov box now
    mInMemoryCacheOffset = 0;
    mWriteBoxToMemory = mStreamableFile;
    if (mWriteBoxToMemory) {
        // There is no need to allocate in-memory cache
        // if the file is not streamable.

        mInMemoryCache = (uint8_t *) malloc(mInMemoryCacheSize);
        CHECK(mInMemoryCache != NULL);
    }

    if (mHasFileLevelMeta) {
        writeFileLevelMetaBox();
        if (mWriteBoxToMemory) {
            writeCachedBoxToFile("meta");
        } else {
            ALOGI("The file meta box is written at the end.");
        }
    }

    if (mHasMoovBox) {
        writeMoovBox(maxDurationUs);
        // mWriteBoxToMemory could be set to false in
        // MPEG4Writer::write() method
        if (mWriteBoxToMemory) {
            writeCachedBoxToFile("moov");
        } else {
            ALOGI("The mp4 file will not be streamable.");
        }
    }
    mWriteBoxToMemory = false;

    // Free in-memory cache for box writing
    if (mInMemoryCache != NULL) {
        free(mInMemoryCache);
        mInMemoryCache = NULL;
        mInMemoryCacheOffset = 0;
    }

    CHECK(mBoxes.empty());

    release();
    return err;
}

/*
 * Writes currently cached box into file.
 *
 * Must be called while mWriteBoxToMemory is true, and will not modify
 * mWriteBoxToMemory. After the call, remaining cache size will be
 * reduced and buffer offset will be set to the beginning of the cache.
 */
void MPEG4Writer::writeCachedBoxToFile(const char *type) {
    CHECK(mWriteBoxToMemory);

    mWriteBoxToMemory = false;
    // Content of the box is saved in the cache, and the in-memory
    // box needs to be written to the file in a single shot.

    CHECK_LE(mInMemoryCacheOffset + 8, mInMemoryCacheSize);

    // Cached box
    lseek64(mFd, mFreeBoxOffset, SEEK_SET);
    mOffset = mFreeBoxOffset;
    write(mInMemoryCache, 1, mInMemoryCacheOffset);

    // Free box
    lseek64(mFd, mOffset, SEEK_SET);
    mFreeBoxOffset = mOffset;
    writeInt32(mInMemoryCacheSize - mInMemoryCacheOffset);
    write("free", 4);

    // Rewind buffering to the beginning, and restore mWriteBoxToMemory flag
    mInMemoryCacheSize -= mInMemoryCacheOffset;
    mInMemoryCacheOffset = 0;
    mWriteBoxToMemory = true;

    ALOGV("dumped out %s box, estimated size remaining %lld",
            type, (long long)mInMemoryCacheSize);
}

uint32_t MPEG4Writer::getMpeg4Time() {
    time_t now = time(NULL);
    // MP4 file uses time counting seconds since midnight, Jan. 1, 1904
    // while time function returns Unix epoch values which starts
    // at 1970-01-01. Lets add the number of seconds between them
    static const uint32_t delta = (66 * 365 + 17) * (24 * 60 * 60);
    if (now < 0 || uint32_t(now) > UINT32_MAX - delta) {
        return 0;
    }
    uint32_t mpeg4Time = uint32_t(now) + delta;
    return mpeg4Time;
}

void MPEG4Writer::writeMvhdBox(int64_t durationUs) {
    uint32_t now = getMpeg4Time();
    beginBox("mvhd");
    writeInt32(0);             // version=0, flags=0
    writeInt32(now);           // creation time
    writeInt32(now);           // modification time
    writeInt32(mTimeScale);    // mvhd timescale
    int32_t duration = (durationUs * mTimeScale + 5E5) / 1E6;
    writeInt32(duration);
    writeInt32(0x10000);       // rate: 1.0
    writeInt16(0x100);         // volume
    writeInt16(0);             // reserved
    writeInt32(0);             // reserved
    writeInt32(0);             // reserved
    writeCompositionMatrix(0); // matrix
    writeInt32(0);             // predefined
    writeInt32(0);             // predefined
    writeInt32(0);             // predefined
    writeInt32(0);             // predefined
    writeInt32(0);             // predefined
    writeInt32(0);             // predefined
    writeInt32(mTracks.size() + 1);  // nextTrackID
    endBox();  // mvhd
}

void MPEG4Writer::writeMoovBox(int64_t durationUs) {
    beginBox("moov");
    writeMvhdBox(durationUs);
    if (mAreGeoTagsAvailable) {
        writeUdtaBox();
    }
    writeMoovLevelMetaBox();
    // Loop through all the tracks to get the global time offset if there is
    // any ctts table appears in a video track.
    int64_t minCttsOffsetTimeUs = kMaxCttsOffsetTimeUs;
    for (List<Track *>::iterator it = mTracks.begin();
        it != mTracks.end(); ++it) {
        if (!(*it)->isHeic()) {
            minCttsOffsetTimeUs =
                std::min(minCttsOffsetTimeUs, (*it)->getMinCttsOffsetTimeUs());
        }
    }
    ALOGI("Ajust the moov start time from %lld us -> %lld us",
            (long long)mStartTimestampUs,
            (long long)(mStartTimestampUs + minCttsOffsetTimeUs - kMaxCttsOffsetTimeUs));
    // Adjust the global start time.
    mStartTimestampUs += minCttsOffsetTimeUs - kMaxCttsOffsetTimeUs;

    // Add mStartTimeOffsetBFramesUs(-ve or zero) to the duration of first entry in STTS.
    mStartTimeOffsetBFramesUs = minCttsOffsetTimeUs - kMaxCttsOffsetTimeUs;
    ALOGV("mStartTimeOffsetBFramesUs :%" PRId32, mStartTimeOffsetBFramesUs);

    for (List<Track *>::iterator it = mTracks.begin();
        it != mTracks.end(); ++it) {
        if (!(*it)->isHeic()) {
            (*it)->writeTrackHeader(mUse32BitOffset);
        }
    }
    endBox();  // moov
}

void MPEG4Writer::writeFtypBox(MetaData *param) {
    beginBox("ftyp");

    int32_t fileType;
    if (!param || !param->findInt32(kKeyFileType, &fileType)) {
        fileType = OUTPUT_FORMAT_MPEG_4;
    }
    if (fileType != OUTPUT_FORMAT_MPEG_4 && fileType != OUTPUT_FORMAT_HEIF) {
        writeFourcc("3gp4");
        writeInt32(0);
        writeFourcc("isom");
        writeFourcc("3gp4");
    } else {
        // Only write "heic" as major brand if the client specified HEIF
        // AND we indeed receive some image heic tracks.
        if (fileType == OUTPUT_FORMAT_HEIF && mHasFileLevelMeta) {
            writeFourcc("heic");
        } else {
            writeFourcc("mp42");
        }
        writeInt32(0);
        if (mHasFileLevelMeta) {
            writeFourcc("mif1");
            writeFourcc("heic");
        }
        if (mHasMoovBox) {
            writeFourcc("isom");
            writeFourcc("mp42");
        }
    }

    endBox();
}

static bool isTestModeEnabled() {
#if (PROPERTY_VALUE_MAX < 5)
#error "PROPERTY_VALUE_MAX must be at least 5"
#endif

    // Test mode is enabled only if rw.media.record.test system
    // property is enabled.
    if (property_get_bool("rw.media.record.test", false)) {
        return true;
    }
    return false;
}

void MPEG4Writer::sendSessionSummary() {
    // Send session summary only if test mode is enabled
    if (!isTestModeEnabled()) {
        return;
    }

    for (List<ChunkInfo>::iterator it = mChunkInfos.begin();
         it != mChunkInfos.end(); ++it) {
        int trackNum = it->mTrack->getTrackId() << 28;
        notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                trackNum | MEDIA_RECORDER_TRACK_INTER_CHUNK_TIME_MS,
                it->mMaxInterChunkDurUs);
    }
}

status_t MPEG4Writer::setInterleaveDuration(uint32_t durationUs) {
    mInterleaveDurationUs = durationUs;
    return OK;
}

void MPEG4Writer::lock() {
    mLock.lock();
}

void MPEG4Writer::unlock() {
    mLock.unlock();
}

off64_t MPEG4Writer::addSample_l(
        MediaBuffer *buffer, bool usePrefix,
        uint32_t tiffHdrOffset, size_t *bytesWritten) {
    off64_t old_offset = mOffset;

    if (usePrefix) {
        addMultipleLengthPrefixedSamples_l(buffer);
    } else {
        if (tiffHdrOffset > 0) {
            tiffHdrOffset = htonl(tiffHdrOffset);
            ::write(mFd, &tiffHdrOffset, 4); // exif_tiff_header_offset field
            mOffset += 4;
        }

        ::write(mFd,
              (const uint8_t *)buffer->data() + buffer->range_offset(),
              buffer->range_length());

        mOffset += buffer->range_length();
    }

    *bytesWritten = mOffset - old_offset;
    return old_offset;
}

static void StripStartcode(MediaBuffer *buffer) {
    if (buffer->range_length() < 4) {
        return;
    }

    const uint8_t *ptr =
        (const uint8_t *)buffer->data() + buffer->range_offset();

    if (!memcmp(ptr, "\x00\x00\x00\x01", 4)) {
        buffer->set_range(
                buffer->range_offset() + 4, buffer->range_length() - 4);
    }
}

void MPEG4Writer::addMultipleLengthPrefixedSamples_l(MediaBuffer *buffer) {
    const uint8_t *dataStart = (const uint8_t *)buffer->data() + buffer->range_offset();
    const uint8_t *currentNalStart = dataStart;
    const uint8_t *nextNalStart;
    const uint8_t *data = dataStart;
    size_t nextNalSize;
    size_t searchSize = buffer->range_length();

    while (getNextNALUnit(&data, &searchSize, &nextNalStart,
            &nextNalSize, true) == OK) {
        size_t currentNalSize = nextNalStart - currentNalStart - 4 /* strip start-code */;
        MediaBuffer *nalBuf = new MediaBuffer((void *)currentNalStart, currentNalSize);
        addLengthPrefixedSample_l(nalBuf);
        nalBuf->release();

        currentNalStart = nextNalStart;
    }

    size_t currentNalOffset = currentNalStart - dataStart;
    buffer->set_range(buffer->range_offset() + currentNalOffset,
            buffer->range_length() - currentNalOffset);
    addLengthPrefixedSample_l(buffer);
}

void MPEG4Writer::addLengthPrefixedSample_l(MediaBuffer *buffer) {
    size_t length = buffer->range_length();

    if (mUse4ByteNalLength) {
        uint8_t x = length >> 24;
        ::write(mFd, &x, 1);
        x = (length >> 16) & 0xff;
        ::write(mFd, &x, 1);
        x = (length >> 8) & 0xff;
        ::write(mFd, &x, 1);
        x = length & 0xff;
        ::write(mFd, &x, 1);

        ::write(mFd,
              (const uint8_t *)buffer->data() + buffer->range_offset(),
              length);

        mOffset += length + 4;
    } else {
        CHECK_LT(length, 65536u);

        uint8_t x = length >> 8;
        ::write(mFd, &x, 1);
        x = length & 0xff;
        ::write(mFd, &x, 1);
        ::write(mFd, (const uint8_t *)buffer->data() + buffer->range_offset(), length);
        mOffset += length + 2;
    }
}

size_t MPEG4Writer::write(
        const void *ptr, size_t size, size_t nmemb) {

    const size_t bytes = size * nmemb;
    if (mWriteBoxToMemory) {

        off64_t boxSize = 8 + mInMemoryCacheOffset + bytes;
        if (boxSize > mInMemoryCacheSize) {
            // The reserved free space at the beginning of the file is not big
            // enough. Boxes should be written to the end of the file from now
            // on, but not to the in-memory cache.

            // We write partial box that is in the memory to the file first.
            for (List<off64_t>::iterator it = mBoxes.begin();
                 it != mBoxes.end(); ++it) {
                (*it) += mOffset;
            }
            lseek64(mFd, mOffset, SEEK_SET);
            ::write(mFd, mInMemoryCache, mInMemoryCacheOffset);
            ::write(mFd, ptr, bytes);
            mOffset += (bytes + mInMemoryCacheOffset);

            // All subsequent boxes will be written to the end of the file.
            mWriteBoxToMemory = false;
        } else {
            memcpy(mInMemoryCache + mInMemoryCacheOffset, ptr, bytes);
            mInMemoryCacheOffset += bytes;
        }
    } else {
        ::write(mFd, ptr, size * nmemb);
        mOffset += bytes;
    }
    return bytes;
}

void MPEG4Writer::beginBox(uint32_t id) {
    mBoxes.push_back(mWriteBoxToMemory?
            mInMemoryCacheOffset: mOffset);

    writeInt32(0);
    writeInt32(id);
}

void MPEG4Writer::beginBox(const char *fourcc) {
    CHECK_EQ(strlen(fourcc), 4u);

    mBoxes.push_back(mWriteBoxToMemory?
            mInMemoryCacheOffset: mOffset);

    writeInt32(0);
    writeFourcc(fourcc);
}

void MPEG4Writer::endBox() {
    CHECK(!mBoxes.empty());

    off64_t offset = *--mBoxes.end();
    mBoxes.erase(--mBoxes.end());

    if (mWriteBoxToMemory) {
        int32_t x = htonl(mInMemoryCacheOffset - offset);
        memcpy(mInMemoryCache + offset, &x, 4);
    } else {
        lseek64(mFd, offset, SEEK_SET);
        writeInt32(mOffset - offset);
        mOffset -= 4;
        lseek64(mFd, mOffset, SEEK_SET);
    }
}

void MPEG4Writer::writeInt8(int8_t x) {
    write(&x, 1, 1);
}

void MPEG4Writer::writeInt16(int16_t x) {
    x = htons(x);
    write(&x, 1, 2);
}

void MPEG4Writer::writeInt32(int32_t x) {
    x = htonl(x);
    write(&x, 1, 4);
}

void MPEG4Writer::writeInt64(int64_t x) {
    x = hton64(x);
    write(&x, 1, 8);
}

void MPEG4Writer::writeCString(const char *s) {
    size_t n = strlen(s);
    write(s, 1, n + 1);
}

void MPEG4Writer::writeFourcc(const char *s) {
    CHECK_EQ(strlen(s), 4u);
    write(s, 1, 4);
}


// Written in +/-DD.DDDD format
void MPEG4Writer::writeLatitude(int degreex10000) {
    bool isNegative = (degreex10000 < 0);
    char sign = isNegative? '-': '+';

    // Handle the whole part
    char str[9];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0) {
        snprintf(str, 5, "%c%.2d.", sign, wholePart);
    } else {
        snprintf(str, 5, "%+.2d.", wholePart);
    }

    // Handle the fractional part
    int fractionalPart = degreex10000 - (wholePart * 10000);
    if (fractionalPart < 0) {
        fractionalPart = -fractionalPart;
    }
    snprintf(&str[4], 5, "%.4d", fractionalPart);

    // Do not write the null terminator
    write(str, 1, 8);
}

// Written in +/- DDD.DDDD format
void MPEG4Writer::writeLongitude(int degreex10000) {
    bool isNegative = (degreex10000 < 0);
    char sign = isNegative? '-': '+';

    // Handle the whole part
    char str[10];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0) {
        snprintf(str, 6, "%c%.3d.", sign, wholePart);
    } else {
        snprintf(str, 6, "%+.3d.", wholePart);
    }

    // Handle the fractional part
    int fractionalPart = degreex10000 - (wholePart * 10000);
    if (fractionalPart < 0) {
        fractionalPart = -fractionalPart;
    }
    snprintf(&str[5], 5, "%.4d", fractionalPart);

    // Do not write the null terminator
    write(str, 1, 9);
}

/*
 * Geodata is stored according to ISO-6709 standard.
 * latitudex10000 is latitude in degrees times 10000, and
 * longitudex10000 is longitude in degrees times 10000.
 * The range for the latitude is in [-90, +90], and
 * The range for the longitude is in [-180, +180]
 */
status_t MPEG4Writer::setGeoData(int latitudex10000, int longitudex10000) {
    // Is latitude or longitude out of range?
    if (latitudex10000 < -900000 || latitudex10000 > 900000 ||
        longitudex10000 < -1800000 || longitudex10000 > 1800000) {
        return BAD_VALUE;
    }

    mLatitudex10000 = latitudex10000;
    mLongitudex10000 = longitudex10000;
    mAreGeoTagsAvailable = true;
    mMoovExtraSize += 30;
    return OK;
}

status_t MPEG4Writer::setCaptureRate(float captureFps) {
    if (captureFps <= 0.0f) {
        return BAD_VALUE;
    }

    // Increase moovExtraSize once only irrespective of how many times
    // setCaptureRate is called.
    bool containsCaptureFps = mMetaKeys->contains(kMetaKey_CaptureFps);
    mMetaKeys->setFloat(kMetaKey_CaptureFps, captureFps);
    if (!containsCaptureFps) {
        mMoovExtraSize += sizeof(kMetaKey_CaptureFps) + 4 + 32;
    }

    return OK;
}

status_t MPEG4Writer::setTemporalLayerCount(uint32_t layerCount) {
    if (layerCount > 9) {
        return BAD_VALUE;
    }

    if (layerCount > 0) {
        mMetaKeys->setInt32(kMetaKey_TemporalLayerCount, layerCount);
        mMoovExtraSize += sizeof(kMetaKey_TemporalLayerCount) + 4 + 32;
    }

    return OK;
}

void MPEG4Writer::notifyApproachingLimit() {
    Mutex::Autolock autolock(mLock);
    // Only notify once.
    if (mSendNotify) {
        return;
    }
    ALOGW("Recorded file size is approaching limit %" PRId64 "bytes",
        mMaxFileSizeLimitBytes);
    notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_FILESIZE_APPROACHING, 0);
    mSendNotify = true;
}

void MPEG4Writer::write(const void *data, size_t size) {
    write(data, 1, size);
}

bool MPEG4Writer::isFileStreamable() const {
    return mStreamableFile;
}

bool MPEG4Writer::exceedsFileSizeLimit() {
    // No limit
    if (mMaxFileSizeLimitBytes == 0) {
        return false;
    }
    int64_t nTotalBytesEstimate = static_cast<int64_t>(mInMemoryCacheSize);
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        nTotalBytesEstimate += (*it)->getEstimatedTrackSizeBytes();
    }

    if (!mStreamableFile) {
        // Add 1024 bytes as error tolerance
        return nTotalBytesEstimate + 1024 >= mMaxFileSizeLimitBytes;
    }

    // Be conservative in the estimate: do not exceed 95% of
    // the target file limit. For small target file size limit, though,
    // this will not help.
    return (nTotalBytesEstimate >= (95 * mMaxFileSizeLimitBytes) / 100);
}

bool MPEG4Writer::approachingFileSizeLimit() {
    // No limit
    if (mMaxFileSizeLimitBytes == 0) {
        return false;
    }

    int64_t nTotalBytesEstimate = static_cast<int64_t>(mInMemoryCacheSize);
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        nTotalBytesEstimate += (*it)->getEstimatedTrackSizeBytes();
    }

    if (!mStreamableFile) {
        // Add 1024 bytes as error tolerance
        return nTotalBytesEstimate + 1024 >= (90 * mMaxFileSizeLimitBytes) / 100;
    }

    return (nTotalBytesEstimate >= (90 * mMaxFileSizeLimitBytes) / 100);
}

bool MPEG4Writer::exceedsFileDurationLimit() {
    // No limit
    if (mMaxFileDurationLimitUs == 0) {
        return false;
    }

    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        if (!(*it)->isHeic() && (*it)->getDurationUs() >= mMaxFileDurationLimitUs) {
            return true;
        }
    }
    return false;
}

bool MPEG4Writer::reachedEOS() {
    bool allDone = true;
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        if (!(*it)->reachedEOS()) {
            allDone = false;
            break;
        }
    }

    return allDone;
}

void MPEG4Writer::setStartTimestampUs(int64_t timeUs) {
    ALOGI("setStartTimestampUs: %" PRId64, timeUs);
    CHECK_GE(timeUs, 0LL);
    Mutex::Autolock autoLock(mLock);
    if (mStartTimestampUs < 0 || mStartTimestampUs > timeUs) {
        mStartTimestampUs = timeUs;
        ALOGI("Earliest track starting time: %" PRId64, mStartTimestampUs);
    }
}

int64_t MPEG4Writer::getStartTimestampUs() {
    Mutex::Autolock autoLock(mLock);
    return mStartTimestampUs;
}

int32_t MPEG4Writer::getStartTimeOffsetBFramesUs() {
    Mutex::Autolock autoLock(mLock);
    return mStartTimeOffsetBFramesUs;
}

size_t MPEG4Writer::numTracks() {
    Mutex::Autolock autolock(mLock);
    return mTracks.size();
}

////////////////////////////////////////////////////////////////////////////////

MPEG4Writer::Track::Track(
        MPEG4Writer *owner, const sp<MediaSource> &source, size_t trackId)
    : mOwner(owner),
      mMeta(source->getFormat()),
      mSource(source),
      mDone(false),
      mPaused(false),
      mResumed(false),
      mStarted(false),
      mGotStartKeyFrame(false),
      mIsMalformed(false),
      mTrackId(trackId),
      mTrackDurationUs(0),
      mEstimatedTrackSizeBytes(0),
      mSamplesHaveSameSize(true),
      mStszTableEntries(new ListTableEntries<uint32_t, 1>(1000)),
      mStcoTableEntries(new ListTableEntries<uint32_t, 1>(1000)),
      mCo64TableEntries(new ListTableEntries<off64_t, 1>(1000)),
      mStscTableEntries(new ListTableEntries<uint32_t, 3>(1000)),
      mStssTableEntries(new ListTableEntries<uint32_t, 1>(1000)),
      mSttsTableEntries(new ListTableEntries<uint32_t, 2>(1000)),
      mCttsTableEntries(new ListTableEntries<uint32_t, 2>(1000)),
      mElstTableEntries(new ListTableEntries<uint32_t, 3>(3)), // Reserve 3 rows, a row has 3 items
      mMinCttsOffsetTimeUs(0),
      mMinCttsOffsetTicks(0),
      mMaxCttsOffsetTicks(0),
      mCodecSpecificData(NULL),
      mCodecSpecificDataSize(0),
      mGotAllCodecSpecificData(false),
      mReachedEOS(false),
      mStartTimestampUs(-1),
      mRotation(0),
      mDimgRefs("dimg"),
      mImageItemId(0),
      mIsPrimary(0),
      mWidth(0),
      mHeight(0),
      mTileWidth(0),
      mTileHeight(0),
      mGridRows(0),
      mGridCols(0),
      mNumTiles(1),
      mTileIndex(0) {
    getCodecSpecificDataFromInputFormatIfPossible();

    const char *mime;
    mMeta->findCString(kKeyMIMEType, &mime);
    mIsAvc = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC);
    mIsHevc = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC);
    mIsAudio = !strncasecmp(mime, "audio/", 6);
    mIsVideo = !strncasecmp(mime, "video/", 6);
    mIsHeic = !strcasecmp(mime, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC);
    mIsMPEG4 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4) ||
               !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC);

    // store temporal layer count
    if (mIsVideo) {
        int32_t count;
        if (mMeta->findInt32(kKeyTemporalLayerCount, &count) && count > 1) {
            mOwner->setTemporalLayerCount(count);
        }
    }

    if (!mIsHeic) {
        setTimeScale();
    } else {
        CHECK(mMeta->findInt32(kKeyWidth, &mWidth) && (mWidth > 0));
        CHECK(mMeta->findInt32(kKeyHeight, &mHeight) && (mHeight > 0));

        int32_t tileWidth, tileHeight, gridRows, gridCols;
        if (mMeta->findInt32(kKeyTileWidth, &tileWidth) && (tileWidth > 0) &&
            mMeta->findInt32(kKeyTileHeight, &tileHeight) && (tileHeight > 0) &&
            mMeta->findInt32(kKeyGridRows, &gridRows) && (gridRows > 0) &&
            mMeta->findInt32(kKeyGridCols, &gridCols) && (gridCols > 0)) {
            mTileWidth = tileWidth;
            mTileHeight = tileHeight;
            mGridRows = gridRows;
            mGridCols = gridCols;
            mNumTiles = gridRows * gridCols;
        }
        if (!mMeta->findInt32(kKeyTrackIsDefault, &mIsPrimary)) {
            mIsPrimary = false;
        }
    }
}

// Clear all the internal states except the CSD data.
void MPEG4Writer::Track::resetInternal() {
    mDone = false;
    mPaused = false;
    mResumed = false;
    mStarted = false;
    mGotStartKeyFrame = false;
    mIsMalformed = false;
    mTrackDurationUs = 0;
    mEstimatedTrackSizeBytes = 0;
    mSamplesHaveSameSize = 0;
    if (mStszTableEntries != NULL) {
        delete mStszTableEntries;
        mStszTableEntries = new ListTableEntries<uint32_t, 1>(1000);
    }
    if (mStcoTableEntries != NULL) {
        delete mStcoTableEntries;
        mStcoTableEntries = new ListTableEntries<uint32_t, 1>(1000);
    }
    if (mCo64TableEntries != NULL) {
        delete mCo64TableEntries;
        mCo64TableEntries = new ListTableEntries<off64_t, 1>(1000);
    }
    if (mStscTableEntries != NULL) {
        delete mStscTableEntries;
        mStscTableEntries = new ListTableEntries<uint32_t, 3>(1000);
    }
    if (mStssTableEntries != NULL) {
        delete mStssTableEntries;
        mStssTableEntries = new ListTableEntries<uint32_t, 1>(1000);
    }
    if (mSttsTableEntries != NULL) {
        delete mSttsTableEntries;
        mSttsTableEntries = new ListTableEntries<uint32_t, 2>(1000);
    }
    if (mCttsTableEntries != NULL) {
        delete mCttsTableEntries;
        mCttsTableEntries = new ListTableEntries<uint32_t, 2>(1000);
    }
    if (mElstTableEntries != NULL) {
        delete mElstTableEntries;
        mElstTableEntries = new ListTableEntries<uint32_t, 3>(3);
    }
    mReachedEOS = false;
}

void MPEG4Writer::Track::updateTrackSizeEstimate() {
    mEstimatedTrackSizeBytes = mMdatSizeBytes;  // media data size

    if (!isHeic() && !mOwner->isFileStreamable()) {
        uint32_t stcoBoxCount = (mOwner->use32BitFileOffset()
                                ? mStcoTableEntries->count()
                                : mCo64TableEntries->count());
        int64_t stcoBoxSizeBytes = stcoBoxCount * 4;
        int64_t stszBoxSizeBytes = mSamplesHaveSameSize? 4: (mStszTableEntries->count() * 4);

        // Reserved free space is not large enough to hold
        // all meta data and thus wasted.
        mEstimatedTrackSizeBytes += mStscTableEntries->count() * 12 +  // stsc box size
                                    mStssTableEntries->count() * 4 +   // stss box size
                                    mSttsTableEntries->count() * 8 +   // stts box size
                                    mCttsTableEntries->count() * 8 +   // ctts box size
                                    mElstTableEntries->count() * 12 +   // elst box size
                                    stcoBoxSizeBytes +           // stco box size
                                    stszBoxSizeBytes;            // stsz box size
    }
}

void MPEG4Writer::Track::addOneStscTableEntry(
        size_t chunkId, size_t sampleId) {
    mStscTableEntries->add(htonl(chunkId));
    mStscTableEntries->add(htonl(sampleId));
    mStscTableEntries->add(htonl(1));
}

void MPEG4Writer::Track::addOneStssTableEntry(size_t sampleId) {
    mStssTableEntries->add(htonl(sampleId));
}

void MPEG4Writer::Track::addOneSttsTableEntry(
        size_t sampleCount, int32_t duration) {

    if (duration == 0) {
        ALOGW("0-duration samples found: %zu", sampleCount);
    }
    mSttsTableEntries->add(htonl(sampleCount));
    mSttsTableEntries->add(htonl(duration));
}

void MPEG4Writer::Track::addOneCttsTableEntry(
        size_t sampleCount, int32_t duration) {

    if (!mIsVideo) {
        return;
    }
    mCttsTableEntries->add(htonl(sampleCount));
    mCttsTableEntries->add(htonl(duration));
}

void MPEG4Writer::Track::addOneElstTableEntry(
    uint32_t segmentDuration, int32_t mediaTime, int16_t mediaRate, int16_t mediaRateFraction) {
    ALOGV("segmentDuration:%u, mediaTime:%d", segmentDuration, mediaTime);
    ALOGV("mediaRate :%" PRId16 ", mediaRateFraction :%" PRId16 ", Ored %u", mediaRate,
        mediaRateFraction, ((((uint32_t)mediaRate) << 16) | ((uint32_t)mediaRateFraction)));
    mElstTableEntries->add(htonl(segmentDuration));
    mElstTableEntries->add(htonl(mediaTime));
    mElstTableEntries->add(htonl((((uint32_t)mediaRate) << 16) | (uint32_t)mediaRateFraction));
}

status_t MPEG4Writer::setNextFd(int fd) {
    ALOGV("addNextFd");
    Mutex::Autolock l(mLock);
    if (mLooper == NULL) {
        mReflector = new AHandlerReflector<MPEG4Writer>(this);
        mLooper = new ALooper;
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    if (mNextFd != -1) {
        // No need to set a new FD yet.
        return INVALID_OPERATION;
    }
    mNextFd = dup(fd);
    return OK;
}

bool MPEG4Writer::Track::isExifData(
        MediaBufferBase *buffer, uint32_t *tiffHdrOffset) const {
    if (!mIsHeic) {
        return false;
    }

    // Exif block starting with 'Exif\0\0'
    size_t length = buffer->range_length();
    uint8_t *data = (uint8_t *)buffer->data() + buffer->range_offset();
    if ((length > sizeof(kExifHeader))
        && !memcmp(data, kExifHeader, sizeof(kExifHeader))) {
        *tiffHdrOffset = sizeof(kExifHeader);
        return true;
    }

    // Exif block starting with fourcc 'Exif' followed by APP1 marker
    if ((length > sizeof(kExifApp1Marker) + 2 + sizeof(kExifHeader))
            && !memcmp(data, kExifApp1Marker, sizeof(kExifApp1Marker))
            && !memcmp(data + sizeof(kExifApp1Marker) + 2, kExifHeader, sizeof(kExifHeader))) {
        // skip 'Exif' fourcc
        buffer->set_range(4, buffer->range_length() - 4);

        // 2-byte APP1 + 2-byte size followed by kExifHeader
        *tiffHdrOffset = 2 + 2 + sizeof(kExifHeader);
        return true;
    }

    return false;
}

void MPEG4Writer::Track::addChunkOffset(off64_t offset) {
    CHECK(!mIsHeic);
    if (mOwner->use32BitFileOffset()) {
        uint32_t value = offset;
        mStcoTableEntries->add(htonl(value));
    } else {
        mCo64TableEntries->add(hton64(offset));
    }
}

void MPEG4Writer::Track::addItemOffsetAndSize(off64_t offset, size_t size, bool isExif) {
    CHECK(mIsHeic);

    if (offset > UINT32_MAX || size > UINT32_MAX) {
        ALOGE("offset or size is out of range: %lld, %lld",
                (long long) offset, (long long) size);
        mIsMalformed = true;
    }
    if (mIsMalformed) {
        return;
    }

    if (isExif) {
         mExifList.push_back(mOwner->addItem_l({
            .itemType = "Exif",
            .isPrimary = false,
            .isHidden = false,
            .offset = (uint32_t)offset,
            .size = (uint32_t)size,
        }));
        return;
    }

    if (mTileIndex >= mNumTiles) {
        ALOGW("Ignoring excess tiles!");
        return;
    }

    // Rotation angle in HEIF is CCW, framework angle is CW.
    int32_t heifRotation = 0;
    switch(mRotation) {
        case 90: heifRotation = 3; break;
        case 180: heifRotation = 2; break;
        case 270: heifRotation = 1; break;
        default: break; // don't set if invalid
    }

    bool hasGrid = (mTileWidth > 0);

    if (mProperties.empty()) {
        mProperties.push_back(mOwner->addProperty_l({
            .type = FOURCC('h', 'v', 'c', 'C'),
            .hvcc = ABuffer::CreateAsCopy(mCodecSpecificData, mCodecSpecificDataSize)
        }));

        mProperties.push_back(mOwner->addProperty_l({
            .type = FOURCC('i', 's', 'p', 'e'),
            .width = hasGrid ? mTileWidth : mWidth,
            .height = hasGrid ? mTileHeight : mHeight,
        }));

        if (!hasGrid && heifRotation > 0) {
            mProperties.push_back(mOwner->addProperty_l({
                .type = FOURCC('i', 'r', 'o', 't'),
                .rotation = heifRotation,
            }));
        }
    }

    mTileIndex++;
    if (hasGrid) {
        mDimgRefs.value.push_back(mOwner->addItem_l({
            .itemType = "hvc1",
            .isPrimary = false,
            .isHidden = true,
            .offset = (uint32_t)offset,
            .size = (uint32_t)size,
            .properties = mProperties,
        }));

        if (mTileIndex == mNumTiles) {
            mProperties.clear();
            mProperties.push_back(mOwner->addProperty_l({
                .type = FOURCC('i', 's', 'p', 'e'),
                .width = mWidth,
                .height = mHeight,
            }));
            if (heifRotation > 0) {
                mProperties.push_back(mOwner->addProperty_l({
                    .type = FOURCC('i', 'r', 'o', 't'),
                    .rotation = heifRotation,
                }));
            }
            mImageItemId = mOwner->addItem_l({
                .itemType = "grid",
                .isPrimary = (mIsPrimary != 0),
                .isHidden = false,
                .rows = (uint32_t)mGridRows,
                .cols = (uint32_t)mGridCols,
                .width = (uint32_t)mWidth,
                .height = (uint32_t)mHeight,
                .properties = mProperties,
            });
        }
    } else {
        mImageItemId = mOwner->addItem_l({
            .itemType = "hvc1",
            .isPrimary = (mIsPrimary != 0),
            .isHidden = false,
            .offset = (uint32_t)offset,
            .size = (uint32_t)size,
            .properties = mProperties,
        });
    }
}

// Flush out the item refs for this track. Note that it must be called after the
// writer thread has stopped, because there might be pending items in the last
// few chunks written by the writer thread (as opposed to the track). In particular,
// it affects the 'dimg' refs for tiled image, as we only have the refs after the
// last tile sample is written.
void MPEG4Writer::Track::flushItemRefs() {
    CHECK(mIsHeic);

    if (mImageItemId > 0) {
        mOwner->addRefs_l(mImageItemId, mDimgRefs);

        if (!mExifList.empty()) {
            // The "cdsc" ref is from the metadata/exif item to the image item.
            // So the refs all contain the image item.
            ItemRefs cdscRefs("cdsc");
            cdscRefs.value.push_back(mImageItemId);
            for (uint16_t exifItem : mExifList) {
                mOwner->addRefs_l(exifItem, cdscRefs);
            }
        }
    }
}

void MPEG4Writer::Track::setTimeScale() {
    ALOGV("setTimeScale");
    // Default time scale
    mTimeScale = 90000;

    if (mIsAudio) {
        // Use the sampling rate as the default time scale for audio track.
        int32_t sampleRate;
        bool success = mMeta->findInt32(kKeySampleRate, &sampleRate);
        CHECK(success);
        mTimeScale = sampleRate;
    }

    // If someone would like to overwrite the timescale, use user-supplied value.
    int32_t timeScale;
    if (mMeta->findInt32(kKeyTimeScale, &timeScale)) {
        mTimeScale = timeScale;
    }

    CHECK_GT(mTimeScale, 0);
}

void MPEG4Writer::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSwitch:
        {
            mLock.lock();
            int fd = mNextFd;
            mNextFd = -1;
            mLock.unlock();
            finishCurrentSession();
            initInternal(fd, false /*isFirstSession*/);
            start(mStartMeta.get());
            mSwitchPending = false;
            notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_NEXT_OUTPUT_FILE_STARTED, 0);
            break;
        }
        default:
        TRESPASS();
    }
}

void MPEG4Writer::Track::getCodecSpecificDataFromInputFormatIfPossible() {
    const char *mime;

    CHECK(mMeta->findCString(kKeyMIMEType, &mime));

    uint32_t type;
    const void *data = NULL;
    size_t size = 0;
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
        mMeta->findData(kKeyAVCC, &type, &data, &size);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC) ||
               !strcasecmp(mime, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC)) {
        mMeta->findData(kKeyHVCC, &type, &data, &size);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)
            || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        if (mMeta->findData(kKeyESDS, &type, &data, &size)) {
            ESDS esds(data, size);
            if (esds.getCodecSpecificInfo(&data, &size) == OK &&
                    data != NULL &&
                    copyCodecSpecificData((uint8_t*)data, size) == OK) {
                mGotAllCodecSpecificData = true;
            }
            return;
        }
    }
    if (data != NULL && copyCodecSpecificData((uint8_t *)data, size) == OK) {
        mGotAllCodecSpecificData = true;
    }
}

MPEG4Writer::Track::~Track() {
    stop();

    delete mStszTableEntries;
    delete mStcoTableEntries;
    delete mCo64TableEntries;
    delete mStscTableEntries;
    delete mSttsTableEntries;
    delete mStssTableEntries;
    delete mCttsTableEntries;
    delete mElstTableEntries;

    mStszTableEntries = NULL;
    mStcoTableEntries = NULL;
    mCo64TableEntries = NULL;
    mStscTableEntries = NULL;
    mSttsTableEntries = NULL;
    mStssTableEntries = NULL;
    mCttsTableEntries = NULL;
    mElstTableEntries = NULL;

    if (mCodecSpecificData != NULL) {
        free(mCodecSpecificData);
        mCodecSpecificData = NULL;
    }
}

void MPEG4Writer::Track::initTrackingProgressStatus(MetaData *params) {
    ALOGV("initTrackingProgressStatus");
    mPreviousTrackTimeUs = -1;
    mTrackingProgressStatus = false;
    mTrackEveryTimeDurationUs = 0;
    {
        int64_t timeUs;
        if (params && params->findInt64(kKeyTrackTimeStatus, &timeUs)) {
            ALOGV("Receive request to track progress status for every %" PRId64 " us", timeUs);
            mTrackEveryTimeDurationUs = timeUs;
            mTrackingProgressStatus = true;
        }
    }
}

// static
void *MPEG4Writer::ThreadWrapper(void *me) {
    ALOGV("ThreadWrapper: %p", me);
    MPEG4Writer *writer = static_cast<MPEG4Writer *>(me);
    writer->threadFunc();
    return NULL;
}

void MPEG4Writer::bufferChunk(const Chunk& chunk) {
    ALOGV("bufferChunk: %p", chunk.mTrack);
    Mutex::Autolock autolock(mLock);
    CHECK_EQ(mDone, false);

    for (List<ChunkInfo>::iterator it = mChunkInfos.begin();
         it != mChunkInfos.end(); ++it) {

        if (chunk.mTrack == it->mTrack) {  // Found owner
            it->mChunks.push_back(chunk);
            mChunkReadyCondition.signal();
            return;
        }
    }

    CHECK(!"Received a chunk for a unknown track");
}

void MPEG4Writer::writeChunkToFile(Chunk* chunk) {
    ALOGV("writeChunkToFile: %" PRId64 " from %s track",
        chunk->mTimeStampUs, chunk->mTrack->getTrackType());

    int32_t isFirstSample = true;
    while (!chunk->mSamples.empty()) {
        List<MediaBuffer *>::iterator it = chunk->mSamples.begin();

        uint32_t tiffHdrOffset;
        if (!(*it)->meta_data().findInt32(
                kKeyExifTiffOffset, (int32_t*)&tiffHdrOffset)) {
            tiffHdrOffset = 0;
        }
        bool isExif = (tiffHdrOffset > 0);
        bool usePrefix = chunk->mTrack->usePrefix() && !isExif;

        size_t bytesWritten;
        off64_t offset = addSample_l(*it, usePrefix, tiffHdrOffset, &bytesWritten);

        if (chunk->mTrack->isHeic()) {
            chunk->mTrack->addItemOffsetAndSize(offset, bytesWritten, isExif);
        } else if (isFirstSample) {
            chunk->mTrack->addChunkOffset(offset);
            isFirstSample = false;
        }

        (*it)->release();
        (*it) = NULL;
        chunk->mSamples.erase(it);
    }
    chunk->mSamples.clear();
}

void MPEG4Writer::writeAllChunks() {
    ALOGV("writeAllChunks");
    size_t outstandingChunks = 0;
    Chunk chunk;
    while (findChunkToWrite(&chunk)) {
        writeChunkToFile(&chunk);
        ++outstandingChunks;
    }

    sendSessionSummary();

    mChunkInfos.clear();
    ALOGD("%zu chunks are written in the last batch", outstandingChunks);
}

bool MPEG4Writer::findChunkToWrite(Chunk *chunk) {
    ALOGV("findChunkToWrite");

    int64_t minTimestampUs = 0x7FFFFFFFFFFFFFFFLL;
    Track *track = NULL;
    for (List<ChunkInfo>::iterator it = mChunkInfos.begin();
         it != mChunkInfos.end(); ++it) {
        if (!it->mChunks.empty()) {
            List<Chunk>::iterator chunkIt = it->mChunks.begin();
            if (chunkIt->mTimeStampUs < minTimestampUs) {
                minTimestampUs = chunkIt->mTimeStampUs;
                track = it->mTrack;
            }
        }
    }

    if (track == NULL) {
        ALOGV("Nothing to be written after all");
        return false;
    }

    if (mIsFirstChunk) {
        mIsFirstChunk = false;
    }

    for (List<ChunkInfo>::iterator it = mChunkInfos.begin();
         it != mChunkInfos.end(); ++it) {
        if (it->mTrack == track) {
            *chunk = *(it->mChunks.begin());
            it->mChunks.erase(it->mChunks.begin());
            CHECK_EQ(chunk->mTrack, track);

            int64_t interChunkTimeUs =
                chunk->mTimeStampUs - it->mPrevChunkTimestampUs;
            if (interChunkTimeUs > it->mPrevChunkTimestampUs) {
                it->mMaxInterChunkDurUs = interChunkTimeUs;
            }

            return true;
        }
    }

    return false;
}

void MPEG4Writer::threadFunc() {
    ALOGV("threadFunc");

    prctl(PR_SET_NAME, (unsigned long)"MPEG4Writer", 0, 0, 0);

    Mutex::Autolock autoLock(mLock);
    while (!mDone) {
        Chunk chunk;
        bool chunkFound = false;

        while (!mDone && !(chunkFound = findChunkToWrite(&chunk))) {
            mChunkReadyCondition.wait(mLock);
        }

        // In real time recording mode, write without holding the lock in order
        // to reduce the blocking time for media track threads.
        // Otherwise, hold the lock until the existing chunks get written to the
        // file.
        if (chunkFound) {
            if (mIsRealTimeRecording) {
                mLock.unlock();
            }
            writeChunkToFile(&chunk);
            if (mIsRealTimeRecording) {
                mLock.lock();
            }
        }
    }

    writeAllChunks();
}

status_t MPEG4Writer::startWriterThread() {
    ALOGV("startWriterThread");

    mDone = false;
    mIsFirstChunk = true;
    mDriftTimeUs = 0;
    for (List<Track *>::iterator it = mTracks.begin();
         it != mTracks.end(); ++it) {
        ChunkInfo info;
        info.mTrack = *it;
        info.mPrevChunkTimestampUs = 0;
        info.mMaxInterChunkDurUs = 0;
        mChunkInfos.push_back(info);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&mThread, &attr, ThreadWrapper, this);
    pthread_attr_destroy(&attr);
    mWriterThreadStarted = true;
    return OK;
}


status_t MPEG4Writer::Track::start(MetaData *params) {
    if (!mDone && mPaused) {
        mPaused = false;
        mResumed = true;
        return OK;
    }

    int64_t startTimeUs;
    if (params == NULL || !params->findInt64(kKeyTime, &startTimeUs)) {
        startTimeUs = 0;
    }
    mStartTimeRealUs = startTimeUs;

    int32_t rotationDegrees;
    if ((mIsVideo || mIsHeic) && params &&
            params->findInt32(kKeyRotation, &rotationDegrees)) {
        mRotation = rotationDegrees;
    }

    initTrackingProgressStatus(params);

    sp<MetaData> meta = new MetaData;
    if (mOwner->isRealTimeRecording() && mOwner->numTracks() > 1) {
        /*
         * This extra delay of accepting incoming audio/video signals
         * helps to align a/v start time at the beginning of a recording
         * session, and it also helps eliminate the "recording" sound for
         * camcorder applications.
         *
         * If client does not set the start time offset, we fall back to
         * use the default initial delay value.
         */
        int64_t startTimeOffsetUs = mOwner->getStartTimeOffsetMs() * 1000LL;
        if (startTimeOffsetUs < 0) {  // Start time offset was not set
            startTimeOffsetUs = kInitialDelayTimeUs;
        }
        startTimeUs += startTimeOffsetUs;
        ALOGI("Start time offset: %" PRId64 " us", startTimeOffsetUs);
    }

    meta->setInt64(kKeyTime, startTimeUs);

    status_t err = mSource->start(meta.get());
    if (err != OK) {
        mDone = mReachedEOS = true;
        return err;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    mDone = false;
    mStarted = true;
    mTrackDurationUs = 0;
    mReachedEOS = false;
    mEstimatedTrackSizeBytes = 0;
    mMdatSizeBytes = 0;
    mMaxChunkDurationUs = 0;
    mLastDecodingTimeUs = -1;

    pthread_create(&mThread, &attr, ThreadWrapper, this);
    pthread_attr_destroy(&attr);

    return OK;
}

status_t MPEG4Writer::Track::pause() {
    mPaused = true;
    return OK;
}

status_t MPEG4Writer::Track::stop(bool stopSource) {
    ALOGD("%s track stopping. %s source", getTrackType(), stopSource ? "Stop" : "Not Stop");
    if (!mStarted) {
        ALOGE("Stop() called but track is not started");
        return ERROR_END_OF_STREAM;
    }

    if (mDone) {
        return OK;
    }

    if (stopSource) {
        ALOGD("%s track source stopping", getTrackType());
        mSource->stop();
        ALOGD("%s track source stopped", getTrackType());
    }

    // Set mDone to be true after sucessfully stop mSource as mSource may be still outputting
    // buffers to the writer.
    mDone = true;

    void *dummy;
    pthread_join(mThread, &dummy);
    status_t err = static_cast<status_t>(reinterpret_cast<uintptr_t>(dummy));

    ALOGD("%s track stopped. %s source", getTrackType(), stopSource ? "Stop" : "Not Stop");
    return err;
}

bool MPEG4Writer::Track::reachedEOS() {
    return mReachedEOS;
}

// static
void *MPEG4Writer::Track::ThreadWrapper(void *me) {
    Track *track = static_cast<Track *>(me);

    status_t err = track->threadEntry();
    return (void *)(uintptr_t)err;
}

static void getNalUnitType(uint8_t byte, uint8_t* type) {
    ALOGV("getNalUnitType: %d", byte);

    // nal_unit_type: 5-bit unsigned integer
    *type = (byte & 0x1F);
}

const uint8_t *MPEG4Writer::Track::parseParamSet(
        const uint8_t *data, size_t length, int type, size_t *paramSetLen) {

    ALOGV("parseParamSet");
    CHECK(type == kNalUnitTypeSeqParamSet ||
          type == kNalUnitTypePicParamSet);

    const uint8_t *nextStartCode = findNextNalStartCode(data, length);
    *paramSetLen = nextStartCode - data;
    if (*paramSetLen == 0) {
        ALOGE("Param set is malformed, since its length is 0");
        return NULL;
    }

    AVCParamSet paramSet(*paramSetLen, data);
    if (type == kNalUnitTypeSeqParamSet) {
        if (*paramSetLen < 4) {
            ALOGE("Seq parameter set malformed");
            return NULL;
        }
        if (mSeqParamSets.empty()) {
            mProfileIdc = data[1];
            mProfileCompatible = data[2];
            mLevelIdc = data[3];
        } else {
            if (mProfileIdc != data[1] ||
                mProfileCompatible != data[2] ||
                mLevelIdc != data[3]) {
                // COULD DO: set profile/level to the lowest required to support all SPSs
                ALOGE("Inconsistent profile/level found in seq parameter sets");
                return NULL;
            }
        }
        mSeqParamSets.push_back(paramSet);
    } else {
        mPicParamSets.push_back(paramSet);
    }
    return nextStartCode;
}

status_t MPEG4Writer::Track::copyAVCCodecSpecificData(
        const uint8_t *data, size_t size) {
    ALOGV("copyAVCCodecSpecificData");

    // 2 bytes for each of the parameter set length field
    // plus the 7 bytes for the header
    return copyCodecSpecificData(data, size, 4 + 7);
}

status_t MPEG4Writer::Track::copyHEVCCodecSpecificData(
        const uint8_t *data, size_t size) {
    ALOGV("copyHEVCCodecSpecificData");

    // Min length of HEVC CSD is 23. (ISO/IEC 14496-15:2014 Chapter 8.3.3.1.2)
    return copyCodecSpecificData(data, size, 23);
}

status_t MPEG4Writer::Track::copyCodecSpecificData(
        const uint8_t *data, size_t size, size_t minLength) {
    if (size < minLength) {
        ALOGE("Codec specific data length too short: %zu", size);
        return ERROR_MALFORMED;
    }

    mCodecSpecificData = malloc(size);
    if (mCodecSpecificData == NULL) {
        ALOGE("Failed allocating codec specific data");
        return NO_MEMORY;
    }
    mCodecSpecificDataSize = size;
    memcpy(mCodecSpecificData, data, size);
    return OK;
}

status_t MPEG4Writer::Track::parseAVCCodecSpecificData(
        const uint8_t *data, size_t size) {

    ALOGV("parseAVCCodecSpecificData");
    // Data starts with a start code.
    // SPS and PPS are separated with start codes.
    // Also, SPS must come before PPS
    uint8_t type = kNalUnitTypeSeqParamSet;
    bool gotSps = false;
    bool gotPps = false;
    const uint8_t *tmp = data;
    const uint8_t *nextStartCode = data;
    size_t bytesLeft = size;
    size_t paramSetLen = 0;
    mCodecSpecificDataSize = 0;
    while (bytesLeft > 4 && !memcmp("\x00\x00\x00\x01", tmp, 4)) {
        getNalUnitType(*(tmp + 4), &type);
        if (type == kNalUnitTypeSeqParamSet) {
            if (gotPps) {
                ALOGE("SPS must come before PPS");
                return ERROR_MALFORMED;
            }
            if (!gotSps) {
                gotSps = true;
            }
            nextStartCode = parseParamSet(tmp + 4, bytesLeft - 4, type, &paramSetLen);
        } else if (type == kNalUnitTypePicParamSet) {
            if (!gotSps) {
                ALOGE("SPS must come before PPS");
                return ERROR_MALFORMED;
            }
            if (!gotPps) {
                gotPps = true;
            }
            nextStartCode = parseParamSet(tmp + 4, bytesLeft - 4, type, &paramSetLen);
        } else {
            ALOGE("Only SPS and PPS Nal units are expected");
            return ERROR_MALFORMED;
        }

        if (nextStartCode == NULL) {
            return ERROR_MALFORMED;
        }

        // Move on to find the next parameter set
        bytesLeft -= nextStartCode - tmp;
        tmp = nextStartCode;
        mCodecSpecificDataSize += (2 + paramSetLen);
    }

    {
        // Check on the number of seq parameter sets
        size_t nSeqParamSets = mSeqParamSets.size();
        if (nSeqParamSets == 0) {
            ALOGE("Cound not find sequence parameter set");
            return ERROR_MALFORMED;
        }

        if (nSeqParamSets > 0x1F) {
            ALOGE("Too many seq parameter sets (%zu) found", nSeqParamSets);
            return ERROR_MALFORMED;
        }
    }

    {
        // Check on the number of pic parameter sets
        size_t nPicParamSets = mPicParamSets.size();
        if (nPicParamSets == 0) {
            ALOGE("Cound not find picture parameter set");
            return ERROR_MALFORMED;
        }
        if (nPicParamSets > 0xFF) {
            ALOGE("Too many pic parameter sets (%zd) found", nPicParamSets);
            return ERROR_MALFORMED;
        }
    }
// FIXME:
// Add chromat_format_idc, bit depth values, etc for AVC/h264 high profile and above
// and remove #if 0
#if 0
    {
        // Check on the profiles
        // These profiles requires additional parameter set extensions
        if (mProfileIdc == 100 || mProfileIdc == 110 ||
            mProfileIdc == 122 || mProfileIdc == 144) {
            ALOGE("Sorry, no support for profile_idc: %d!", mProfileIdc);
            return BAD_VALUE;
        }
    }
#endif
    return OK;
}

status_t MPEG4Writer::Track::makeAVCCodecSpecificData(
        const uint8_t *data, size_t size) {

    if (mCodecSpecificData != NULL) {
        ALOGE("Already have codec specific data");
        return ERROR_MALFORMED;
    }

    if (size < 4) {
        ALOGE("Codec specific data length too short: %zu", size);
        return ERROR_MALFORMED;
    }

    // Data is in the form of AVCCodecSpecificData
    if (memcmp("\x00\x00\x00\x01", data, 4)) {
        return copyAVCCodecSpecificData(data, size);
    }

    if (parseAVCCodecSpecificData(data, size) != OK) {
        return ERROR_MALFORMED;
    }

    // ISO 14496-15: AVC file format
    mCodecSpecificDataSize += 7;  // 7 more bytes in the header
    mCodecSpecificData = malloc(mCodecSpecificDataSize);
    if (mCodecSpecificData == NULL) {
        mCodecSpecificDataSize = 0;
        ALOGE("Failed allocating codec specific data");
        return NO_MEMORY;
    }
    uint8_t *header = (uint8_t *)mCodecSpecificData;
    header[0] = 1;                     // version
    header[1] = mProfileIdc;           // profile indication
    header[2] = mProfileCompatible;    // profile compatibility
    header[3] = mLevelIdc;

    // 6-bit '111111' followed by 2-bit to lengthSizeMinuusOne
    if (mOwner->useNalLengthFour()) {
        header[4] = 0xfc | 3;  // length size == 4 bytes
    } else {
        header[4] = 0xfc | 1;  // length size == 2 bytes
    }

    // 3-bit '111' followed by 5-bit numSequenceParameterSets
    int nSequenceParamSets = mSeqParamSets.size();
    header[5] = 0xe0 | nSequenceParamSets;
    header += 6;
    for (List<AVCParamSet>::iterator it = mSeqParamSets.begin();
         it != mSeqParamSets.end(); ++it) {
        // 16-bit sequence parameter set length
        uint16_t seqParamSetLength = it->mLength;
        header[0] = seqParamSetLength >> 8;
        header[1] = seqParamSetLength & 0xff;

        // SPS NAL unit (sequence parameter length bytes)
        memcpy(&header[2], it->mData, seqParamSetLength);
        header += (2 + seqParamSetLength);
    }

    // 8-bit nPictureParameterSets
    int nPictureParamSets = mPicParamSets.size();
    header[0] = nPictureParamSets;
    header += 1;
    for (List<AVCParamSet>::iterator it = mPicParamSets.begin();
         it != mPicParamSets.end(); ++it) {
        // 16-bit picture parameter set length
        uint16_t picParamSetLength = it->mLength;
        header[0] = picParamSetLength >> 8;
        header[1] = picParamSetLength & 0xff;

        // PPS Nal unit (picture parameter set length bytes)
        memcpy(&header[2], it->mData, picParamSetLength);
        header += (2 + picParamSetLength);
    }

    return OK;
}


status_t MPEG4Writer::Track::parseHEVCCodecSpecificData(
        const uint8_t *data, size_t size, HevcParameterSets &paramSets) {

    ALOGV("parseHEVCCodecSpecificData");
    const uint8_t *tmp = data;
    const uint8_t *nextStartCode = data;
    size_t bytesLeft = size;
    while (bytesLeft > 4 && !memcmp("\x00\x00\x00\x01", tmp, 4)) {
        nextStartCode = findNextNalStartCode(tmp + 4, bytesLeft - 4);
        status_t err = paramSets.addNalUnit(tmp + 4, (nextStartCode - tmp) - 4);
        if (err != OK) {
            return ERROR_MALFORMED;
        }

        // Move on to find the next parameter set
        bytesLeft -= nextStartCode - tmp;
        tmp = nextStartCode;
    }

    size_t csdSize = 23;
    const size_t numNalUnits = paramSets.getNumNalUnits();
    for (size_t i = 0; i < ARRAY_SIZE(kMandatoryHevcNalUnitTypes); ++i) {
        int type = kMandatoryHevcNalUnitTypes[i];
        size_t numParamSets = paramSets.getNumNalUnitsOfType(type);
        if (numParamSets == 0) {
            ALOGE("Cound not find NAL unit of type %d", type);
            return ERROR_MALFORMED;
        }
    }
    for (size_t i = 0; i < ARRAY_SIZE(kHevcNalUnitTypes); ++i) {
        int type = kHevcNalUnitTypes[i];
        size_t numParamSets = paramSets.getNumNalUnitsOfType(type);
        if (numParamSets > 0xffff) {
            ALOGE("Too many seq parameter sets (%zu) found", numParamSets);
            return ERROR_MALFORMED;
        }
        csdSize += 3;
        for (size_t j = 0; j < numNalUnits; ++j) {
            if (paramSets.getType(j) != type) {
                continue;
            }
            csdSize += 2 + paramSets.getSize(j);
        }
    }
    mCodecSpecificDataSize = csdSize;
    return OK;
}

status_t MPEG4Writer::Track::makeHEVCCodecSpecificData(
        const uint8_t *data, size_t size) {

    if (mCodecSpecificData != NULL) {
        ALOGE("Already have codec specific data");
        return ERROR_MALFORMED;
    }

    if (size < 4) {
        ALOGE("Codec specific data length too short: %zu", size);
        return ERROR_MALFORMED;
    }

    // Data is in the form of HEVCCodecSpecificData
    if (memcmp("\x00\x00\x00\x01", data, 4)) {
        return copyHEVCCodecSpecificData(data, size);
    }

    HevcParameterSets paramSets;
    if (parseHEVCCodecSpecificData(data, size, paramSets) != OK) {
        ALOGE("failed parsing codec specific data");
        return ERROR_MALFORMED;
    }

    mCodecSpecificData = malloc(mCodecSpecificDataSize);
    if (mCodecSpecificData == NULL) {
        mCodecSpecificDataSize = 0;
        ALOGE("Failed allocating codec specific data");
        return NO_MEMORY;
    }
    status_t err = paramSets.makeHvcc((uint8_t *)mCodecSpecificData,
            &mCodecSpecificDataSize, mOwner->useNalLengthFour() ? 4 : 2);
    if (err != OK) {
        ALOGE("failed constructing HVCC atom");
        return err;
    }

    return OK;
}

/*
 * Updates the drift time from the audio track so that
 * the video track can get the updated drift time information
 * from the file writer. The fluctuation of the drift time of the audio
 * encoding path is smoothed out with a simple filter by giving a larger
 * weight to more recently drift time. The filter coefficients, 0.5 and 0.5,
 * are heuristically determined.
 */
void MPEG4Writer::Track::updateDriftTime(const sp<MetaData>& meta) {
    int64_t driftTimeUs = 0;
    if (meta->findInt64(kKeyDriftTime, &driftTimeUs)) {
        int64_t prevDriftTimeUs = mOwner->getDriftTimeUs();
        int64_t timeUs = (driftTimeUs + prevDriftTimeUs) >> 1;
        mOwner->setDriftTimeUs(timeUs);
    }
}

void MPEG4Writer::Track::dumpTimeStamps() {
    if (!mTimestampDebugHelper.empty()) {
        std::string timeStampString = "Dumping " + std::string(getTrackType()) + " track's last " +
                                      std::to_string(mTimestampDebugHelper.size()) +
                                      " frames' timestamps(pts, dts) and frame type : ";
        for (const TimestampDebugHelperEntry& entry : mTimestampDebugHelper) {
            timeStampString += "\n(" + std::to_string(entry.pts) + "us, " +
                               std::to_string(entry.dts) + "us " + entry.frameType + ") ";
        }
        ALOGE("%s", timeStampString.c_str());
    } else {
        ALOGE("0 frames to dump timeStamps in %s track ", getTrackType());
    }
}

status_t MPEG4Writer::Track::threadEntry() {
    int32_t count = 0;
    const int64_t interleaveDurationUs = mOwner->interleaveDuration();
    const bool hasMultipleTracks = (mOwner->numTracks() > 1);
    int64_t chunkTimestampUs = 0;
    int32_t nChunks = 0;
    int32_t nActualFrames = 0;        // frames containing non-CSD data (non-0 length)
    int32_t nZeroLengthFrames = 0;
    int64_t lastTimestampUs = 0;      // Previous sample time stamp
    int64_t lastDurationUs = 0;       // Between the previous two samples
    int64_t currDurationTicks = 0;    // Timescale based ticks
    int64_t lastDurationTicks = 0;    // Timescale based ticks
    int32_t sampleCount = 1;          // Sample count in the current stts table entry
    uint32_t previousSampleSize = 0;  // Size of the previous sample
    int64_t previousPausedDurationUs = 0;
    int64_t timestampUs = 0;
    int64_t cttsOffsetTimeUs = 0;
    int64_t currCttsOffsetTimeTicks = 0;   // Timescale based ticks
    int64_t lastCttsOffsetTimeTicks = -1;  // Timescale based ticks
    int32_t cttsSampleCount = 0;           // Sample count in the current ctts table entry
    uint32_t lastSamplesPerChunk = 0;

    if (mIsAudio) {
        prctl(PR_SET_NAME, (unsigned long)"AudioTrackEncoding", 0, 0, 0);
    } else if (mIsVideo) {
        prctl(PR_SET_NAME, (unsigned long)"VideoTrackEncoding", 0, 0, 0);
    } else {
        prctl(PR_SET_NAME, (unsigned long)"MetadataTrackEncoding", 0, 0, 0);
    }

    if (mOwner->isRealTimeRecording()) {
        androidSetThreadPriority(0, ANDROID_PRIORITY_AUDIO);
    }

    sp<MetaData> meta_data;

    status_t err = OK;
    MediaBufferBase *buffer;
    const char *trackName = getTrackType();
    while (!mDone && (err = mSource->read(&buffer)) == OK) {
        if (buffer->range_length() == 0) {
            buffer->release();
            buffer = NULL;
            ++nZeroLengthFrames;
            continue;
        }

        // If the codec specific data has not been received yet, delay pause.
        // After the codec specific data is received, discard what we received
        // when the track is to be paused.
        if (mPaused && !mResumed) {
            buffer->release();
            buffer = NULL;
            continue;
        }

        ++count;

        int32_t isCodecConfig;
        if (buffer->meta_data().findInt32(kKeyIsCodecConfig, &isCodecConfig)
                && isCodecConfig) {
            // if config format (at track addition) already had CSD, keep that
            // UNLESS we have not received any frames yet.
            // TODO: for now the entire CSD has to come in one frame for encoders, even though
            // they need to be spread out for decoders.
            if (mGotAllCodecSpecificData && nActualFrames > 0) {
                ALOGI("ignoring additional CSD for video track after first frame");
            } else {
                mMeta = mSource->getFormat(); // get output format after format change
                status_t err;
                if (mIsAvc) {
                    err = makeAVCCodecSpecificData(
                            (const uint8_t *)buffer->data()
                                + buffer->range_offset(),
                            buffer->range_length());
                } else if (mIsHevc || mIsHeic) {
                    err = makeHEVCCodecSpecificData(
                            (const uint8_t *)buffer->data()
                                + buffer->range_offset(),
                            buffer->range_length());
                } else if (mIsMPEG4) {
                    copyCodecSpecificData((const uint8_t *)buffer->data() + buffer->range_offset(),
                            buffer->range_length());
                }
            }

            buffer->release();
            buffer = NULL;
            if (OK != err) {
                mSource->stop();
                mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_ERROR,
                       mTrackId | MEDIA_RECORDER_TRACK_ERROR_GENERAL, err);
                break;
            }

            mGotAllCodecSpecificData = true;
            continue;
        }

        // Per-frame metadata sample's size must be smaller than max allowed.
        if (!mIsVideo && !mIsAudio && !mIsHeic &&
                buffer->range_length() >= kMaxMetadataSize) {
            ALOGW("Buffer size is %zu. Maximum metadata buffer size is %lld for %s track",
                    buffer->range_length(), (long long)kMaxMetadataSize, trackName);
            buffer->release();
            mSource->stop();
            mIsMalformed = true;
            break;
        }

        bool isExif = false;
        uint32_t tiffHdrOffset = 0;
        int32_t isMuxerData;
        if (buffer->meta_data().findInt32(kKeyIsMuxerData, &isMuxerData) && isMuxerData) {
            // We only support one type of muxer data, which is Exif data block.
            isExif = isExifData(buffer, &tiffHdrOffset);
            if (!isExif) {
                ALOGW("Ignoring bad Exif data block");
                buffer->release();
                buffer = NULL;
                continue;
            }
        }

        ++nActualFrames;

        // Make a deep copy of the MediaBuffer and Metadata and release
        // the original as soon as we can
        MediaBuffer *copy = new MediaBuffer(buffer->range_length());
        memcpy(copy->data(), (uint8_t *)buffer->data() + buffer->range_offset(),
                buffer->range_length());
        copy->set_range(0, buffer->range_length());
        meta_data = new MetaData(buffer->meta_data());
        buffer->release();
        buffer = NULL;

        if (isExif) {
            copy->meta_data().setInt32(kKeyExifTiffOffset, tiffHdrOffset);
        }
        bool usePrefix = this->usePrefix() && !isExif;

        if (usePrefix) StripStartcode(copy);

        size_t sampleSize = copy->range_length();
        if (usePrefix) {
            if (mOwner->useNalLengthFour()) {
                sampleSize += 4;
            } else {
                sampleSize += 2;
            }
        }

        // Max file size or duration handling
        mMdatSizeBytes += sampleSize;
        updateTrackSizeEstimate();

        if (mOwner->exceedsFileSizeLimit()) {
            if (mOwner->switchFd() != OK) {
                ALOGW("Recorded file size exceeds limit %" PRId64 "bytes",
                        mOwner->mMaxFileSizeLimitBytes);
                mSource->stop();
                mOwner->notify(
                        MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED, 0);
            } else {
                ALOGV("%s Current recorded file size exceeds limit %" PRId64 "bytes. Switching output",
                        getTrackType(), mOwner->mMaxFileSizeLimitBytes);
            }
            copy->release();
            break;
        }

        if (mOwner->exceedsFileDurationLimit()) {
            ALOGW("Recorded file duration exceeds limit %" PRId64 "microseconds",
                    mOwner->mMaxFileDurationLimitUs);
            mOwner->notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_DURATION_REACHED, 0);
            copy->release();
            mSource->stop();
            break;
        }

        if (mOwner->approachingFileSizeLimit()) {
            mOwner->notifyApproachingLimit();
        }

        int32_t isSync = false;
        meta_data->findInt32(kKeyIsSyncFrame, &isSync);
        CHECK(meta_data->findInt64(kKeyTime, &timestampUs));

        // For video, skip the first several non-key frames until getting the first key frame.
        if (mIsVideo && !mGotStartKeyFrame && !isSync) {
            ALOGD("Video skip non-key frame");
            copy->release();
            continue;
        }
        if (mIsVideo && isSync) {
            mGotStartKeyFrame = true;
        }
////////////////////////////////////////////////////////////////////////////////

        if (!mIsHeic) {
            if (mStszTableEntries->count() == 0) {
                mFirstSampleTimeRealUs = systemTime() / 1000;
                mOwner->setStartTimestampUs(timestampUs);
                mStartTimestampUs = timestampUs;
                previousPausedDurationUs = mStartTimestampUs;
            }

            if (mResumed) {
                int64_t durExcludingEarlierPausesUs = timestampUs - previousPausedDurationUs;
                if (WARN_UNLESS(durExcludingEarlierPausesUs >= 0LL, "for %s track", trackName)) {
                    copy->release();
                    mSource->stop();
                    mIsMalformed = true;
                    break;
                }

                int64_t pausedDurationUs = durExcludingEarlierPausesUs - mTrackDurationUs;
                if (WARN_UNLESS(pausedDurationUs >= lastDurationUs, "for %s track", trackName)) {
                    copy->release();
                    mSource->stop();
                    mIsMalformed = true;
                    break;
                }

                previousPausedDurationUs += pausedDurationUs - lastDurationUs;
                mResumed = false;
            }
            TimestampDebugHelperEntry timestampDebugEntry;
            timestampUs -= previousPausedDurationUs;
            timestampDebugEntry.pts = timestampUs;
            if (WARN_UNLESS(timestampUs >= 0LL, "for %s track", trackName)) {
                copy->release();
                mSource->stop();
                mIsMalformed = true;
                break;
            }

            if (mIsVideo) {
                /*
                 * Composition time: timestampUs
                 * Decoding time: decodingTimeUs
                 * Composition time offset = composition time - decoding time
                 */
                int64_t decodingTimeUs;
                CHECK(meta_data->findInt64(kKeyDecodingTime, &decodingTimeUs));
                decodingTimeUs -= previousPausedDurationUs;

                // ensure non-negative, monotonic decoding time
                if (mLastDecodingTimeUs < 0) {
                    decodingTimeUs = std::max((int64_t)0, decodingTimeUs);
                } else {
                    // increase decoding time by at least the larger vaule of 1 tick and
                    // 0.1 milliseconds. This needs to take into account the possible
                    // delta adjustment in DurationTicks in below.
                    decodingTimeUs = std::max(mLastDecodingTimeUs +
                            std::max(100, divUp(1000000, mTimeScale)), decodingTimeUs);
                }

                mLastDecodingTimeUs = decodingTimeUs;
                timestampDebugEntry.dts = decodingTimeUs;
                timestampDebugEntry.frameType = isSync ? "Key frame" : "Non-Key frame";
                // Insert the timestamp into the mTimestampDebugHelper
                if (mTimestampDebugHelper.size() >= kTimestampDebugCount) {
                    mTimestampDebugHelper.pop_front();
                }
                mTimestampDebugHelper.push_back(timestampDebugEntry);

                cttsOffsetTimeUs =
                        timestampUs + kMaxCttsOffsetTimeUs - decodingTimeUs;
                if (WARN_UNLESS(cttsOffsetTimeUs >= 0LL, "for %s track", trackName)) {
                    copy->release();
                    mSource->stop();
                    mIsMalformed = true;
                    break;
                }

                timestampUs = decodingTimeUs;
                ALOGV("decoding time: %" PRId64 " and ctts offset time: %" PRId64,
                    timestampUs, cttsOffsetTimeUs);

                // Update ctts box table if necessary
                currCttsOffsetTimeTicks =
                        (cttsOffsetTimeUs * mTimeScale + 500000LL) / 1000000LL;
                if (WARN_UNLESS(currCttsOffsetTimeTicks <= 0x0FFFFFFFFLL, "for %s track", trackName)) {
                    copy->release();
                    mSource->stop();
                    mIsMalformed = true;
                    break;
                }

                if (mStszTableEntries->count() == 0) {
                    // Force the first ctts table entry to have one single entry
                    // so that we can do adjustment for the initial track start
                    // time offset easily in writeCttsBox().
                    lastCttsOffsetTimeTicks = currCttsOffsetTimeTicks;
                    addOneCttsTableEntry(1, currCttsOffsetTimeTicks);
                    cttsSampleCount = 0;      // No sample in ctts box is pending
                } else {
                    if (currCttsOffsetTimeTicks != lastCttsOffsetTimeTicks) {
                        addOneCttsTableEntry(cttsSampleCount, lastCttsOffsetTimeTicks);
                        lastCttsOffsetTimeTicks = currCttsOffsetTimeTicks;
                        cttsSampleCount = 1;  // One sample in ctts box is pending
                    } else {
                        ++cttsSampleCount;
                    }
                }

                // Update ctts time offset range
                if (mStszTableEntries->count() == 0) {
                    mMinCttsOffsetTicks = currCttsOffsetTimeTicks;
                    mMaxCttsOffsetTicks = currCttsOffsetTimeTicks;
                } else {
                    if (currCttsOffsetTimeTicks > mMaxCttsOffsetTicks) {
                        mMaxCttsOffsetTicks = currCttsOffsetTimeTicks;
                    } else if (currCttsOffsetTimeTicks < mMinCttsOffsetTicks) {
                        mMinCttsOffsetTicks = currCttsOffsetTimeTicks;
                        mMinCttsOffsetTimeUs = cttsOffsetTimeUs;
                    }
                }
            }

            if (mOwner->isRealTimeRecording()) {
                if (mIsAudio) {
                    updateDriftTime(meta_data);
                }
            }

            if (WARN_UNLESS(timestampUs >= 0LL, "for %s track", trackName)) {
                copy->release();
                mSource->stop();
                mIsMalformed = true;
                break;
            }

            ALOGV("%s media time stamp: %" PRId64 " and previous paused duration %" PRId64,
                    trackName, timestampUs, previousPausedDurationUs);
            if (timestampUs > mTrackDurationUs) {
                mTrackDurationUs = timestampUs;
            }

            // We need to use the time scale based ticks, rather than the
            // timestamp itself to determine whether we have to use a new
            // stts entry, since we may have rounding errors.
            // The calculation is intended to reduce the accumulated
            // rounding errors.
            currDurationTicks =
                ((timestampUs * mTimeScale + 500000LL) / 1000000LL -
                    (lastTimestampUs * mTimeScale + 500000LL) / 1000000LL);
            if (currDurationTicks < 0LL) {
                ALOGE("do not support out of order frames (timestamp: %lld < last: %lld for %s track",
                        (long long)timestampUs, (long long)lastTimestampUs, trackName);
                copy->release();
                mSource->stop();
                mIsMalformed = true;
                break;
            }

            // if the duration is different for this sample, see if it is close enough to the previous
            // duration that we can fudge it and use the same value, to avoid filling the stts table
            // with lots of near-identical entries.
            // "close enough" here means that the current duration needs to be adjusted by less
            // than 0.1 milliseconds
            if (lastDurationTicks && (currDurationTicks != lastDurationTicks)) {
                int64_t deltaUs = ((lastDurationTicks - currDurationTicks) * 1000000LL
                        + (mTimeScale / 2)) / mTimeScale;
                if (deltaUs > -100 && deltaUs < 100) {
                    // use previous ticks, and adjust timestamp as if it was actually that number
                    // of ticks
                    currDurationTicks = lastDurationTicks;
                    timestampUs += deltaUs;
                }
            }
            mStszTableEntries->add(htonl(sampleSize));
            if (mStszTableEntries->count() > 2) {

                // Force the first sample to have its own stts entry so that
                // we can adjust its value later to maintain the A/V sync.
                if (mStszTableEntries->count() == 3 || currDurationTicks != lastDurationTicks) {
                    addOneSttsTableEntry(sampleCount, lastDurationTicks);
                    sampleCount = 1;
                } else {
                    ++sampleCount;
                }

            }
            if (mSamplesHaveSameSize) {
                if (mStszTableEntries->count() >= 2 && previousSampleSize != sampleSize) {
                    mSamplesHaveSameSize = false;
                }
                previousSampleSize = sampleSize;
            }
            ALOGV("%s timestampUs/lastTimestampUs: %" PRId64 "/%" PRId64,
                    trackName, timestampUs, lastTimestampUs);
            lastDurationUs = timestampUs - lastTimestampUs;
            lastDurationTicks = currDurationTicks;
            lastTimestampUs = timestampUs;

            if (isSync != 0) {
                addOneStssTableEntry(mStszTableEntries->count());
            }

            if (mTrackingProgressStatus) {
                if (mPreviousTrackTimeUs <= 0) {
                    mPreviousTrackTimeUs = mStartTimestampUs;
                }
                trackProgressStatus(timestampUs);
            }
        }
        if (!hasMultipleTracks) {
            size_t bytesWritten;
            off64_t offset = mOwner->addSample_l(
                    copy, usePrefix, tiffHdrOffset, &bytesWritten);

            if (mIsHeic) {
                addItemOffsetAndSize(offset, bytesWritten, isExif);
            } else {
                uint32_t count = (mOwner->use32BitFileOffset()
                            ? mStcoTableEntries->count()
                            : mCo64TableEntries->count());

                if (count == 0) {
                    addChunkOffset(offset);
                }
            }
            copy->release();
            copy = NULL;
            continue;
        }

        mChunkSamples.push_back(copy);
        if (mIsHeic) {
            bufferChunk(0 /*timestampUs*/);
            ++nChunks;
        } else if (interleaveDurationUs == 0) {
            addOneStscTableEntry(++nChunks, 1);
            bufferChunk(timestampUs);
        } else {
            if (chunkTimestampUs == 0) {
                chunkTimestampUs = timestampUs;
            } else {
                int64_t chunkDurationUs = timestampUs - chunkTimestampUs;
                if (chunkDurationUs > interleaveDurationUs) {
                    if (chunkDurationUs > mMaxChunkDurationUs) {
                        mMaxChunkDurationUs = chunkDurationUs;
                    }
                    ++nChunks;
                    if (nChunks == 1 ||  // First chunk
                        lastSamplesPerChunk != mChunkSamples.size()) {
                        lastSamplesPerChunk = mChunkSamples.size();
                        addOneStscTableEntry(nChunks, lastSamplesPerChunk);
                    }
                    bufferChunk(timestampUs);
                    chunkTimestampUs = timestampUs;
                }
            }
        }

    }

    if (isTrackMalFormed()) {
        dumpTimeStamps();
        err = ERROR_MALFORMED;
    }

    mOwner->trackProgressStatus(mTrackId, -1, err);

    if (mIsHeic) {
        if (!mChunkSamples.empty()) {
            bufferChunk(0);
            ++nChunks;
        }
    } else {
        // Last chunk
        if (!hasMultipleTracks) {
            addOneStscTableEntry(1, mStszTableEntries->count());
        } else if (!mChunkSamples.empty()) {
            addOneStscTableEntry(++nChunks, mChunkSamples.size());
            bufferChunk(timestampUs);
        }

        // We don't really know how long the last frame lasts, since
        // there is no frame time after it, just repeat the previous
        // frame's duration.
        if (mStszTableEntries->count() == 1) {
            lastDurationUs = 0;  // A single sample's duration
            lastDurationTicks = 0;
        } else {
            ++sampleCount;  // Count for the last sample
        }

        if (mStszTableEntries->count() <= 2) {
            addOneSttsTableEntry(1, lastDurationTicks);
            if (sampleCount - 1 > 0) {
                addOneSttsTableEntry(sampleCount - 1, lastDurationTicks);
            }
        } else {
            addOneSttsTableEntry(sampleCount, lastDurationTicks);
        }

        // The last ctts box may not have been written yet, and this
        // is to make sure that we write out the last ctts box.
        if (currCttsOffsetTimeTicks == lastCttsOffsetTimeTicks) {
            if (cttsSampleCount > 0) {
                addOneCttsTableEntry(cttsSampleCount, lastCttsOffsetTimeTicks);
            }
        }

        mTrackDurationUs += lastDurationUs;
    }
    mReachedEOS = true;

    sendTrackSummary(hasMultipleTracks);

    ALOGI("Received total/0-length (%d/%d) buffers and encoded %d frames. - %s",
            count, nZeroLengthFrames, mStszTableEntries->count(), trackName);
    if (mIsAudio) {
        ALOGI("Audio track drift time: %" PRId64 " us", mOwner->getDriftTimeUs());
    }

    if (err == ERROR_END_OF_STREAM) {
        return OK;
    }
    return err;
}

bool MPEG4Writer::Track::isTrackMalFormed() const {
    if (mIsMalformed) {
        return true;
    }

    if (!mIsHeic && mStszTableEntries->count() == 0) {  // no samples written
        ALOGE("The number of recorded samples is 0");
        return true;
    }

    if (mIsVideo && mStssTableEntries->count() == 0) {  // no sync frames for video
        ALOGE("There are no sync frames for video track");
        return true;
    }

    if (OK != checkCodecSpecificData()) {         // no codec specific data
        return true;
    }

    return false;
}

void MPEG4Writer::Track::sendTrackSummary(bool hasMultipleTracks) {

    // Send track summary only if test mode is enabled.
    if (!isTestModeEnabled()) {
        return;
    }

    int trackNum = (mTrackId << 28);

    mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_TYPE,
                    mIsAudio ? 0: 1);

    mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_DURATION_MS,
                    mTrackDurationUs / 1000);

    mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_ENCODED_FRAMES,
                    mStszTableEntries->count());

    {
        // The system delay time excluding the requested initial delay that
        // is used to eliminate the recording sound.
        int64_t startTimeOffsetUs = mOwner->getStartTimeOffsetMs() * 1000LL;
        if (startTimeOffsetUs < 0) {  // Start time offset was not set
            startTimeOffsetUs = kInitialDelayTimeUs;
        }
        int64_t initialDelayUs =
            mFirstSampleTimeRealUs - mStartTimeRealUs - startTimeOffsetUs;

        mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_INITIAL_DELAY_MS,
                    (initialDelayUs) / 1000);
    }

    mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_DATA_KBYTES,
                    mMdatSizeBytes / 1024);

    if (hasMultipleTracks) {
        mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_MAX_CHUNK_DUR_MS,
                    mMaxChunkDurationUs / 1000);

        int64_t moovStartTimeUs = mOwner->getStartTimestampUs();
        if (mStartTimestampUs != moovStartTimeUs) {
            int64_t startTimeOffsetUs = mStartTimestampUs - moovStartTimeUs;
            mOwner->notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
                    trackNum | MEDIA_RECORDER_TRACK_INFO_START_OFFSET_MS,
                    startTimeOffsetUs / 1000);
        }
    }
}

void MPEG4Writer::Track::trackProgressStatus(int64_t timeUs, status_t err) {
    ALOGV("trackProgressStatus: %" PRId64 " us", timeUs);

    if (mTrackEveryTimeDurationUs > 0 &&
        timeUs - mPreviousTrackTimeUs >= mTrackEveryTimeDurationUs) {
        ALOGV("Fire time tracking progress status at %" PRId64 " us", timeUs);
        mOwner->trackProgressStatus(mTrackId, timeUs - mPreviousTrackTimeUs, err);
        mPreviousTrackTimeUs = timeUs;
    }
}

void MPEG4Writer::trackProgressStatus(
        size_t trackId, int64_t timeUs, status_t err) {
    Mutex::Autolock lock(mLock);
    int32_t trackNum = (trackId << 28);

    // Error notification
    // Do not consider ERROR_END_OF_STREAM an error
    if (err != OK && err != ERROR_END_OF_STREAM) {
        notify(MEDIA_RECORDER_TRACK_EVENT_ERROR,
               trackNum | MEDIA_RECORDER_TRACK_ERROR_GENERAL,
               err);
        return;
    }

    if (timeUs == -1) {
        // Send completion notification
        notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
               trackNum | MEDIA_RECORDER_TRACK_INFO_COMPLETION_STATUS,
               err);
    } else {
        // Send progress status
        notify(MEDIA_RECORDER_TRACK_EVENT_INFO,
               trackNum | MEDIA_RECORDER_TRACK_INFO_PROGRESS_IN_TIME,
               timeUs / 1000);
    }
}

void MPEG4Writer::setDriftTimeUs(int64_t driftTimeUs) {
    ALOGV("setDriftTimeUs: %" PRId64 " us", driftTimeUs);
    Mutex::Autolock autolock(mLock);
    mDriftTimeUs = driftTimeUs;
}

int64_t MPEG4Writer::getDriftTimeUs() {
    ALOGV("getDriftTimeUs: %" PRId64 " us", mDriftTimeUs);
    Mutex::Autolock autolock(mLock);
    return mDriftTimeUs;
}

bool MPEG4Writer::isRealTimeRecording() const {
    return mIsRealTimeRecording;
}

bool MPEG4Writer::useNalLengthFour() {
    return mUse4ByteNalLength;
}

void MPEG4Writer::Track::bufferChunk(int64_t timestampUs) {
    ALOGV("bufferChunk");

    Chunk chunk(this, timestampUs, mChunkSamples);
    mOwner->bufferChunk(chunk);
    mChunkSamples.clear();
}

int64_t MPEG4Writer::Track::getDurationUs() const {
    return mTrackDurationUs + getStartTimeOffsetTimeUs() + mOwner->getStartTimeOffsetBFramesUs();
}

int64_t MPEG4Writer::Track::getEstimatedTrackSizeBytes() const {
    return mEstimatedTrackSizeBytes;
}

int32_t MPEG4Writer::Track::getMetaSizeIncrease(
        int32_t angle, int32_t trackCount) const {
    CHECK(mIsHeic);

    int32_t grid = (mTileWidth > 0);
    int32_t rotate = (angle > 0);

    // Note that the rotation angle is in the file meta, and we don't have
    // it until start, so here the calculation has to assume rotation.

    // increase to ipco
    int32_t increase = 20 * (grid + 1)              // 'ispe' property
                     + (8 + mCodecSpecificDataSize) // 'hvcC' property
                     ;

    if (rotate) {
        increase += 9;                              // 'irot' property (worst case)
    }

    // increase to iref and idat
    if (grid) {
        increase += (12 + mNumTiles * 2)            // 'dimg' in iref
                  + 12;                             // ImageGrid in 'idat' (worst case)
    }

    increase += (12 + 2);                           // 'cdsc' in iref

    // increase to iloc, iinf
    increase += (16                                 // increase to 'iloc'
              + 21)                                 // increase to 'iinf'
              * (mNumTiles + grid + 1);             // "+1" is for 'Exif'

    // When total # of properties is > 127, the properties id becomes 2-byte.
    // We write 4 properties at most for each image (2x'ispe', 1x'hvcC', 1x'irot').
    // Set the threshold to be 30.
    int32_t propBytes = trackCount > 30 ? 2 : 1;

    // increase to ipma
    increase += (3 + 2 * propBytes) * mNumTiles     // 'ispe' + 'hvcC'
             + grid * (3 + propBytes)               // 'ispe' for grid
             + rotate * propBytes;                  // 'irot' (either on grid or tile)

    return increase;
}

status_t MPEG4Writer::Track::checkCodecSpecificData() const {
    const char *mime;
    CHECK(mMeta->findCString(kKeyMIMEType, &mime));
    if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AAC, mime) ||
        !strcasecmp(MEDIA_MIMETYPE_VIDEO_MPEG4, mime) ||
        !strcasecmp(MEDIA_MIMETYPE_VIDEO_AVC, mime) ||
        !strcasecmp(MEDIA_MIMETYPE_VIDEO_HEVC, mime) ||
        !strcasecmp(MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC, mime)) {
        if (!mCodecSpecificData ||
            mCodecSpecificDataSize <= 0) {
            ALOGE("Missing codec specific data");
            return ERROR_MALFORMED;
        }
    } else {
        if (mCodecSpecificData ||
            mCodecSpecificDataSize > 0) {
            ALOGE("Unexepected codec specific data found");
            return ERROR_MALFORMED;
        }
    }
    return OK;
}

const char *MPEG4Writer::Track::getTrackType() const {
    return mIsAudio ? "Audio" :
           mIsVideo ? "Video" :
           mIsHeic  ? "Image" :
                      "Metadata";
}

void MPEG4Writer::Track::writeTrackHeader(bool use32BitOffset) {
    uint32_t now = getMpeg4Time();
    mOwner->beginBox("trak");
        writeTkhdBox(now);
        writeEdtsBox();
        mOwner->beginBox("mdia");
            writeMdhdBox(now);
            writeHdlrBox();
            mOwner->beginBox("minf");
                if (mIsAudio) {
                    writeSmhdBox();
                } else if (mIsVideo) {
                    writeVmhdBox();
                } else {
                    writeNmhdBox();
                }
                writeDinfBox();
                writeStblBox(use32BitOffset);
            mOwner->endBox();  // minf
        mOwner->endBox();  // mdia
    mOwner->endBox();  // trak
}

int64_t MPEG4Writer::Track::getMinCttsOffsetTimeUs() {
    // For video tracks with ctts table, this should return the minimum ctts
    // offset in the table. For non-video tracks or video tracks without ctts
    // table, this will return kMaxCttsOffsetTimeUs.
    if (mMinCttsOffsetTicks == mMaxCttsOffsetTicks) {
        return kMaxCttsOffsetTimeUs;
    }
    return mMinCttsOffsetTimeUs;
}

void MPEG4Writer::Track::writeStblBox(bool use32BitOffset) {
    mOwner->beginBox("stbl");
    mOwner->beginBox("stsd");
    mOwner->writeInt32(0);               // version=0, flags=0
    mOwner->writeInt32(1);               // entry count
    if (mIsAudio) {
        writeAudioFourCCBox();
    } else if (mIsVideo) {
        writeVideoFourCCBox();
    } else {
        writeMetadataFourCCBox();
    }
    mOwner->endBox();  // stsd
    writeSttsBox();
    if (mIsVideo) {
        writeCttsBox();
        writeStssBox();
    }
    writeStszBox();
    writeStscBox();
    writeStcoBox(use32BitOffset);
    mOwner->endBox();  // stbl
}

void MPEG4Writer::Track::writeMetadataFourCCBox() {
    const char *mime;
    bool success = mMeta->findCString(kKeyMIMEType, &mime);
    CHECK(success);
    const char *fourcc = getFourCCForMime(mime);
    if (fourcc == NULL) {
        ALOGE("Unknown mime type '%s'.", mime);
        TRESPASS();
    }
    mOwner->beginBox(fourcc);    // TextMetaDataSampleEntry

    //  HACK to make the metadata track compliant with the ISO standard.
    //
    //  Metadata track is added from API 26 and the original implementation does not
    //  fully followed the TextMetaDataSampleEntry specified in ISO/IEC 14496-12-2015
    //  in that only the mime_format is written out. content_encoding and
    //  data_reference_index have not been written out. This leads to the failure
    //  when some MP4 parser tries to parse the metadata track according to the
    //  standard. The hack here will make the metadata track compliant with the
    //  standard while still maintaining backwards compatibility. This would enable
    //  Android versions before API 29 to be able to read out the standard compliant
    //  Metadata track generated with Android API 29 and upward. The trick is based
    //  on the fact that the Metadata track must start with prefix “application/” and
    //  those missing fields are not used in Android's Metadata track. By writting
    //  out the mime_format twice, the first mime_format will be used to fill out the
    //  missing reserved, data_reference_index and content encoding fields. On the
    //  parser side, the extracter before API 29  will read out the first mime_format
    //  correctly and drop the second mime_format. The extractor from API 29 will
    //  check if the reserved, data_reference_index and content encoding are filled
    //  with “application” to detect if this is a standard compliant metadata track
    //  and read out the data accordingly.
    mOwner->writeCString(mime);

    mOwner->writeCString(mime);  // metadata mime_format
    mOwner->endBox(); // mett
}

void MPEG4Writer::Track::writeVideoFourCCBox() {
    const char *mime;
    bool success = mMeta->findCString(kKeyMIMEType, &mime);
    CHECK(success);
    const char *fourcc = getFourCCForMime(mime);
    if (fourcc == NULL) {
        ALOGE("Unknown mime type '%s'.", mime);
        TRESPASS();
    }

    mOwner->beginBox(fourcc);        // video format
    mOwner->writeInt32(0);           // reserved
    mOwner->writeInt16(0);           // reserved
    mOwner->writeInt16(1);           // data ref index
    mOwner->writeInt16(0);           // predefined
    mOwner->writeInt16(0);           // reserved
    mOwner->writeInt32(0);           // predefined
    mOwner->writeInt32(0);           // predefined
    mOwner->writeInt32(0);           // predefined

    int32_t width, height;
    success = mMeta->findInt32(kKeyWidth, &width);
    success = success && mMeta->findInt32(kKeyHeight, &height);
    CHECK(success);

    mOwner->writeInt16(width);
    mOwner->writeInt16(height);
    mOwner->writeInt32(0x480000);    // horiz resolution
    mOwner->writeInt32(0x480000);    // vert resolution
    mOwner->writeInt32(0);           // reserved
    mOwner->writeInt16(1);           // frame count
    mOwner->writeInt8(0);            // compressor string length
    mOwner->write("                               ", 31);
    mOwner->writeInt16(0x18);        // depth
    mOwner->writeInt16(-1);          // predefined

    if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_MPEG4, mime)) {
        writeMp4vEsdsBox();
    } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_H263, mime)) {
        writeD263Box();
    } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_AVC, mime)) {
        writeAvccBox();
    } else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_HEVC, mime)) {
        writeHvccBox();
    }

    writePaspBox();
    writeColrBox();
    mOwner->endBox();  // mp4v, s263 or avc1
}

void MPEG4Writer::Track::writeColrBox() {
    ColorAspects aspects;
    memset(&aspects, 0, sizeof(aspects));
    // TRICKY: using | instead of || because we want to execute all findInt32-s
    if (mMeta->findInt32(kKeyColorPrimaries, (int32_t*)&aspects.mPrimaries)
            | mMeta->findInt32(kKeyTransferFunction, (int32_t*)&aspects.mTransfer)
            | mMeta->findInt32(kKeyColorMatrix, (int32_t*)&aspects.mMatrixCoeffs)
            | mMeta->findInt32(kKeyColorRange, (int32_t*)&aspects.mRange)) {
        int32_t primaries, transfer, coeffs;
        bool fullRange;
        ColorUtils::convertCodecColorAspectsToIsoAspects(
                aspects, &primaries, &transfer, &coeffs, &fullRange);
        mOwner->beginBox("colr");
        mOwner->writeFourcc("nclx");
        mOwner->writeInt16(primaries);
        mOwner->writeInt16(transfer);
        mOwner->writeInt16(coeffs);
        mOwner->writeInt8(int8_t(fullRange ? 0x80 : 0x0));
        mOwner->endBox(); // colr
    }
}

void MPEG4Writer::Track::writeAudioFourCCBox() {
    const char *mime;
    bool success = mMeta->findCString(kKeyMIMEType, &mime);
    CHECK(success);
    const char *fourcc = getFourCCForMime(mime);
    if (fourcc == NULL) {
        ALOGE("Unknown mime type '%s'.", mime);
        TRESPASS();
    }

    mOwner->beginBox(fourcc);        // audio format
    mOwner->writeInt32(0);           // reserved
    mOwner->writeInt16(0);           // reserved
    mOwner->writeInt16(0x1);         // data ref index
    mOwner->writeInt32(0);           // reserved
    mOwner->writeInt32(0);           // reserved
    int32_t nChannels;
    CHECK_EQ(true, mMeta->findInt32(kKeyChannelCount, &nChannels));
    mOwner->writeInt16(nChannels);   // channel count
    mOwner->writeInt16(16);          // sample size
    mOwner->writeInt16(0);           // predefined
    mOwner->writeInt16(0);           // reserved

    int32_t samplerate;
    success = mMeta->findInt32(kKeySampleRate, &samplerate);
    CHECK(success);
    mOwner->writeInt32(samplerate << 16);
    if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AAC, mime)) {
        writeMp4aEsdsBox();
    } else if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_NB, mime) ||
               !strcasecmp(MEDIA_MIMETYPE_AUDIO_AMR_WB, mime)) {
        writeDamrBox();
    }
    mOwner->endBox();
}

static void generateEsdsSize(size_t dataLength, size_t* sizeGenerated, uint8_t* buffer) {
    size_t offset = 0, cur = 0;
    size_t more = 0x00;
    *sizeGenerated = 0;
    /* Start with the LSB(7 bits) of dataLength and build the byte sequence upto MSB.
     * Continuation flag(most significant bit) will be set on the first N-1 bytes.
     */
    do {
        buffer[cur++] = (dataLength & 0x7f) | more;
        dataLength >>= 7;
        more = 0x80;
        ++(*sizeGenerated);
    } while (dataLength > 0u);
    --cur;
    // Reverse the newly formed byte sequence.
    while (cur > offset) {
        uint8_t tmp = buffer[cur];
        buffer[cur--] = buffer[offset];
        buffer[offset++] = tmp;
    }
}

void MPEG4Writer::Track::writeMp4aEsdsBox() {
    CHECK(mCodecSpecificData);
    CHECK_GT(mCodecSpecificDataSize, 0u);

    uint8_t sizeESDBuffer[kESDSScratchBufferSize];
    uint8_t sizeDCDBuffer[kESDSScratchBufferSize];
    uint8_t sizeDSIBuffer[kESDSScratchBufferSize];
    size_t sizeESD = 0;
    size_t sizeDCD = 0;
    size_t sizeDSI = 0;
    generateEsdsSize(mCodecSpecificDataSize, &sizeDSI, sizeDSIBuffer);
    generateEsdsSize(mCodecSpecificDataSize + sizeDSI + 14, &sizeDCD, sizeDCDBuffer);
    generateEsdsSize(mCodecSpecificDataSize + sizeDSI + sizeDCD + 21, &sizeESD, sizeESDBuffer);

    mOwner->beginBox("esds");

    mOwner->writeInt32(0);     // version=0, flags=0
    mOwner->writeInt8(0x03);   // ES_DescrTag
    mOwner->write(sizeESDBuffer, sizeESD);
    mOwner->writeInt16(0x0000);// ES_ID
    mOwner->writeInt8(0x00);

    mOwner->writeInt8(0x04);   // DecoderConfigDescrTag
    mOwner->write(sizeDCDBuffer, sizeDCD);
    mOwner->writeInt8(0x40);   // objectTypeIndication ISO/IEC 14492-2
    mOwner->writeInt8(0x15);   // streamType AudioStream

    mOwner->writeInt16(0x03);  // XXX
    mOwner->writeInt8(0x00);   // buffer size 24-bit (0x300)

    int32_t avgBitrate = 0;
    (void)mMeta->findInt32(kKeyBitRate, &avgBitrate);
    int32_t maxBitrate = 0;
    (void)mMeta->findInt32(kKeyMaxBitRate, &maxBitrate);
    mOwner->writeInt32(maxBitrate);
    mOwner->writeInt32(avgBitrate);

    mOwner->writeInt8(0x05);   // DecoderSpecificInfoTag
    mOwner->write(sizeDSIBuffer, sizeDSI);
    mOwner->write(mCodecSpecificData, mCodecSpecificDataSize);

    static const uint8_t kData2[] = {
        0x06,  // SLConfigDescriptorTag
        0x01,
        0x02
    };
    mOwner->write(kData2, sizeof(kData2));

    mOwner->endBox();  // esds
}

void MPEG4Writer::Track::writeMp4vEsdsBox() {
    CHECK(mCodecSpecificData);
    CHECK_GT(mCodecSpecificDataSize, 0u);

    uint8_t sizeESDBuffer[kESDSScratchBufferSize];
    uint8_t sizeDCDBuffer[kESDSScratchBufferSize];
    uint8_t sizeDSIBuffer[kESDSScratchBufferSize];
    size_t sizeESD = 0;
    size_t sizeDCD = 0;
    size_t sizeDSI = 0;
    generateEsdsSize(mCodecSpecificDataSize, &sizeDSI, sizeDSIBuffer);
    generateEsdsSize(mCodecSpecificDataSize + sizeDSI + 14, &sizeDCD, sizeDCDBuffer);
    generateEsdsSize(mCodecSpecificDataSize + sizeDSI + sizeDCD + 21, &sizeESD, sizeESDBuffer);

    mOwner->beginBox("esds");

    mOwner->writeInt32(0);    // version=0, flags=0

    mOwner->writeInt8(0x03);  // ES_DescrTag
    mOwner->write(sizeESDBuffer, sizeESD);
    mOwner->writeInt16(0x0000);  // ES_ID
    mOwner->writeInt8(0x1f);

    mOwner->writeInt8(0x04);  // DecoderConfigDescrTag
    mOwner->write(sizeDCDBuffer, sizeDCD);
    mOwner->writeInt8(0x20);  // objectTypeIndication ISO/IEC 14492-2
    mOwner->writeInt8(0x11);  // streamType VisualStream

    static const uint8_t kData[] = {
        0x01, 0x77, 0x00, // buffer size 96000 bytes
    };
    mOwner->write(kData, sizeof(kData));

    int32_t avgBitrate = 0;
    (void)mMeta->findInt32(kKeyBitRate, &avgBitrate);
    int32_t maxBitrate = 0;
    (void)mMeta->findInt32(kKeyMaxBitRate, &maxBitrate);
    mOwner->writeInt32(maxBitrate);
    mOwner->writeInt32(avgBitrate);

    mOwner->writeInt8(0x05);  // DecoderSpecificInfoTag

    mOwner->write(sizeDSIBuffer, sizeDSI);
    mOwner->write(mCodecSpecificData, mCodecSpecificDataSize);

    static const uint8_t kData2[] = {
        0x06,  // SLConfigDescriptorTag
        0x01,
        0x02
    };
    mOwner->write(kData2, sizeof(kData2));

    mOwner->endBox();  // esds
}

void MPEG4Writer::Track::writeTkhdBox(uint32_t now) {
    mOwner->beginBox("tkhd");
    // Flags = 7 to indicate that the track is enabled, and
    // part of the presentation
    mOwner->writeInt32(0x07);          // version=0, flags=7
    mOwner->writeInt32(now);           // creation time
    mOwner->writeInt32(now);           // modification time
    mOwner->writeInt32(mTrackId);      // track id starts with 1
    mOwner->writeInt32(0);             // reserved
    int64_t trakDurationUs = getDurationUs();
    int32_t mvhdTimeScale = mOwner->getTimeScale();
    int32_t tkhdDuration =
        (trakDurationUs * mvhdTimeScale + 5E5) / 1E6;
    mOwner->writeInt32(tkhdDuration);  // in mvhd timescale
    mOwner->writeInt32(0);             // reserved
    mOwner->writeInt32(0);             // reserved
    mOwner->writeInt16(0);             // layer
    mOwner->writeInt16(0);             // alternate group
    mOwner->writeInt16(mIsAudio ? 0x100 : 0);  // volume
    mOwner->writeInt16(0);             // reserved

    mOwner->writeCompositionMatrix(mRotation);       // matrix

    if (!mIsVideo) {
        mOwner->writeInt32(0);
        mOwner->writeInt32(0);
    } else {
        int32_t width, height;
        bool success = mMeta->findInt32(kKeyDisplayWidth, &width);
        success = success && mMeta->findInt32(kKeyDisplayHeight, &height);

        // Use width/height if display width/height are not present.
        if (!success) {
            success = mMeta->findInt32(kKeyWidth, &width);
            success = success && mMeta->findInt32(kKeyHeight, &height);
        }
        CHECK(success);

        mOwner->writeInt32(width << 16);   // 32-bit fixed-point value
        mOwner->writeInt32(height << 16);  // 32-bit fixed-point value
    }
    mOwner->endBox();  // tkhd
}

void MPEG4Writer::Track::writeVmhdBox() {
    mOwner->beginBox("vmhd");
    mOwner->writeInt32(0x01);        // version=0, flags=1
    mOwner->writeInt16(0);           // graphics mode
    mOwner->writeInt16(0);           // opcolor
    mOwner->writeInt16(0);
    mOwner->writeInt16(0);
    mOwner->endBox();
}

void MPEG4Writer::Track::writeSmhdBox() {
    mOwner->beginBox("smhd");
    mOwner->writeInt32(0);           // version=0, flags=0
    mOwner->writeInt16(0);           // balance
    mOwner->writeInt16(0);           // reserved
    mOwner->endBox();
}

void MPEG4Writer::Track::writeNmhdBox() {
    mOwner->beginBox("nmhd");
    mOwner->writeInt32(0);           // version=0, flags=0
    mOwner->endBox();
}

void MPEG4Writer::Track::writeHdlrBox() {
    mOwner->beginBox("hdlr");
    mOwner->writeInt32(0);             // version=0, flags=0
    mOwner->writeInt32(0);             // component type: should be mhlr
    mOwner->writeFourcc(mIsAudio ? "soun" : (mIsVideo ? "vide" : "meta"));  // component subtype
    mOwner->writeInt32(0);             // reserved
    mOwner->writeInt32(0);             // reserved
    mOwner->writeInt32(0);             // reserved
    // Removing "r" for the name string just makes the string 4 byte aligned
    mOwner->writeCString(mIsAudio ? "SoundHandle": (mIsVideo ? "VideoHandle" : "MetadHandle"));
    mOwner->endBox();
}

void MPEG4Writer::Track::writeEdtsBox(){
    ALOGV("%s : getStartTimeOffsetTimeUs of track:%" PRId64 " us", getTrackType(),
        getStartTimeOffsetTimeUs());

    // Prepone video playback.
    if (mMinCttsOffsetTicks != mMaxCttsOffsetTicks) {
        int32_t mvhdTimeScale = mOwner->getTimeScale();
        uint32_t tkhdDuration = (getDurationUs() * mvhdTimeScale + 5E5) / 1E6;
        int64_t mediaTime = ((kMaxCttsOffsetTimeUs - getMinCttsOffsetTimeUs())
            * mTimeScale + 5E5) / 1E6;
        if (tkhdDuration > 0 && mediaTime > 0) {
            addOneElstTableEntry(tkhdDuration, mediaTime, 1, 0);
        }
    }

    if (mElstTableEntries->count() == 0) {
        return;
    }

    mOwner->beginBox("edts");
        mOwner->beginBox("elst");
            mOwner->writeInt32(0); // version=0, flags=0
            mElstTableEntries->write(mOwner);
        mOwner->endBox(); // elst;
    mOwner->endBox(); // edts
}

void MPEG4Writer::Track::writeMdhdBox(uint32_t now) {
    int64_t trakDurationUs = getDurationUs();
    int64_t mdhdDuration = (trakDurationUs * mTimeScale + 5E5) / 1E6;
    mOwner->beginBox("mdhd");

    if (mdhdDuration > UINT32_MAX) {
        mOwner->writeInt32((1 << 24));            // version=1, flags=0
        mOwner->writeInt64((int64_t)now);         // creation time
        mOwner->writeInt64((int64_t)now);         // modification time
        mOwner->writeInt32(mTimeScale);           // media timescale
        mOwner->writeInt64(mdhdDuration);         // media timescale
    } else {
        mOwner->writeInt32(0);                      // version=0, flags=0
        mOwner->writeInt32(now);                    // creation time
        mOwner->writeInt32(now);                    // modification time
        mOwner->writeInt32(mTimeScale);             // media timescale
        mOwner->writeInt32((int32_t)mdhdDuration);  // use media timescale
    }
    // Language follows the three letter standard ISO-639-2/T
    // 'e', 'n', 'g' for "English", for instance.
    // Each character is packed as the difference between its ASCII value and 0x60.
    // For "English", these are 00101, 01110, 00111.
    // XXX: Where is the padding bit located: 0x15C7?
    const char *lang = NULL;
    int16_t langCode = 0;
    if (mMeta->findCString(kKeyMediaLanguage, &lang) && lang && strnlen(lang, 3) > 2) {
        langCode = ((lang[0] & 0x1f) << 10) | ((lang[1] & 0x1f) << 5) | (lang[2] & 0x1f);
    }
    mOwner->writeInt16(langCode);      // language code
    mOwner->writeInt16(0);             // predefined
    mOwner->endBox();
}

void MPEG4Writer::Track::writeDamrBox() {
    // 3gpp2 Spec AMRSampleEntry fields
    mOwner->beginBox("damr");
    mOwner->writeCString("   ");  // vendor: 4 bytes
    mOwner->writeInt8(0);         // decoder version
    mOwner->writeInt16(0x83FF);   // mode set: all enabled
    mOwner->writeInt8(0);         // mode change period
    mOwner->writeInt8(1);         // frames per sample
    mOwner->endBox();
}

void MPEG4Writer::Track::writeUrlBox() {
    // The table index here refers to the sample description index
    // in the sample table entries.
    mOwner->beginBox("url ");
    mOwner->writeInt32(1);  // version=0, flags=1 (self-contained)
    mOwner->endBox();  // url
}

void MPEG4Writer::Track::writeDrefBox() {
    mOwner->beginBox("dref");
    mOwner->writeInt32(0);  // version=0, flags=0
    mOwner->writeInt32(1);  // entry count (either url or urn)
    writeUrlBox();
    mOwner->endBox();  // dref
}

void MPEG4Writer::Track::writeDinfBox() {
    mOwner->beginBox("dinf");
    writeDrefBox();
    mOwner->endBox();  // dinf
}

void MPEG4Writer::Track::writeAvccBox() {
    CHECK(mCodecSpecificData);
    CHECK_GE(mCodecSpecificDataSize, 5u);

    // Patch avcc's lengthSize field to match the number
    // of bytes we use to indicate the size of a nal unit.
    uint8_t *ptr = (uint8_t *)mCodecSpecificData;
    ptr[4] = (ptr[4] & 0xfc) | (mOwner->useNalLengthFour() ? 3 : 1);
    mOwner->beginBox("avcC");
    mOwner->write(mCodecSpecificData, mCodecSpecificDataSize);
    mOwner->endBox();  // avcC
}


void MPEG4Writer::Track::writeHvccBox() {
    CHECK(mCodecSpecificData);
    CHECK_GE(mCodecSpecificDataSize, 5u);

    // Patch avcc's lengthSize field to match the number
    // of bytes we use to indicate the size of a nal unit.
    uint8_t *ptr = (uint8_t *)mCodecSpecificData;
    ptr[21] = (ptr[21] & 0xfc) | (mOwner->useNalLengthFour() ? 3 : 1);
    mOwner->beginBox("hvcC");
    mOwner->write(mCodecSpecificData, mCodecSpecificDataSize);
    mOwner->endBox();  // hvcC
}

void MPEG4Writer::Track::writeD263Box() {
    mOwner->beginBox("d263");
    mOwner->writeInt32(0);  // vendor
    mOwner->writeInt8(0);   // decoder version
    mOwner->writeInt8(10);  // level: 10
    mOwner->writeInt8(0);   // profile: 0
    mOwner->endBox();  // d263
}

// This is useful if the pixel is not square
void MPEG4Writer::Track::writePaspBox() {
    mOwner->beginBox("pasp");
    mOwner->writeInt32(1 << 16);  // hspacing
    mOwner->writeInt32(1 << 16);  // vspacing
    mOwner->endBox();  // pasp
}

int64_t MPEG4Writer::Track::getStartTimeOffsetTimeUs() const {
    int64_t trackStartTimeOffsetUs = 0;
    int64_t moovStartTimeUs = mOwner->getStartTimestampUs();
    if (mStartTimestampUs != -1 && mStartTimestampUs != moovStartTimeUs) {
        CHECK_GT(mStartTimestampUs, moovStartTimeUs);
        trackStartTimeOffsetUs = mStartTimestampUs - moovStartTimeUs;
    }
    return trackStartTimeOffsetUs;
}

int32_t MPEG4Writer::Track::getStartTimeOffsetScaledTime() const {
    return (getStartTimeOffsetTimeUs() * mTimeScale + 500000LL) / 1000000LL;
}

void MPEG4Writer::Track::writeSttsBox() {
    mOwner->beginBox("stts");
    mOwner->writeInt32(0);  // version=0, flags=0
    if (mMinCttsOffsetTicks == mMaxCttsOffsetTicks) {
        // For non-vdeio tracks or video tracks without ctts table,
        // adjust duration of first sample for tracks to account for
        // first sample not starting at the media start time.
        // TODO: consider signaling this using some offset
        // as this is not quite correct.
        uint32_t duration;
        CHECK(mSttsTableEntries->get(duration, 1));
        duration = htonl(duration);  // Back to host byte order
        int32_t startTimeOffsetScaled = (((getStartTimeOffsetTimeUs() +
            mOwner->getStartTimeOffsetBFramesUs()) * mTimeScale) + 500000LL) / 1000000LL;
        mSttsTableEntries->set(htonl((int32_t)duration + startTimeOffsetScaled), 1);
    }
    mSttsTableEntries->write(mOwner);
    mOwner->endBox();  // stts
}

void MPEG4Writer::Track::writeCttsBox() {
    // There is no B frame at all
    if (mMinCttsOffsetTicks == mMaxCttsOffsetTicks) {
        return;
    }

    // Do not write ctts box when there is no need to have it.
    if (mCttsTableEntries->count() == 0) {
        return;
    }

    ALOGV("ctts box has %d entries with range [%" PRId64 ", %" PRId64 "]",
            mCttsTableEntries->count(), mMinCttsOffsetTicks, mMaxCttsOffsetTicks);

    mOwner->beginBox("ctts");
    mOwner->writeInt32(0);  // version=0, flags=0
    int64_t deltaTimeUs = kMaxCttsOffsetTimeUs - getStartTimeOffsetTimeUs();
    int64_t delta = (deltaTimeUs * mTimeScale + 500000LL) / 1000000LL;
    mCttsTableEntries->adjustEntries([delta](size_t /* ix */, uint32_t (&value)[2]) {
        // entries are <count, ctts> pairs; adjust only ctts
        uint32_t duration = htonl(value[1]); // back to host byte order
        // Prevent overflow and underflow
        if (delta > duration) {
            duration = 0;
        } else if (delta < 0 && UINT32_MAX + delta < duration) {
            duration = UINT32_MAX;
        } else {
            duration -= delta;
        }
        value[1] = htonl(duration);
    });
    mCttsTableEntries->write(mOwner);
    mOwner->endBox();  // ctts
}

void MPEG4Writer::Track::writeStssBox() {
    mOwner->beginBox("stss");
    mOwner->writeInt32(0);  // version=0, flags=0
    mStssTableEntries->write(mOwner);
    mOwner->endBox();  // stss
}

void MPEG4Writer::Track::writeStszBox() {
    mOwner->beginBox("stsz");
    mOwner->writeInt32(0);  // version=0, flags=0
    mOwner->writeInt32(0);
    mStszTableEntries->write(mOwner);
    mOwner->endBox();  // stsz
}

void MPEG4Writer::Track::writeStscBox() {
    mOwner->beginBox("stsc");
    mOwner->writeInt32(0);  // version=0, flags=0
    mStscTableEntries->write(mOwner);
    mOwner->endBox();  // stsc
}

void MPEG4Writer::Track::writeStcoBox(bool use32BitOffset) {
    mOwner->beginBox(use32BitOffset? "stco": "co64");
    mOwner->writeInt32(0);  // version=0, flags=0
    if (use32BitOffset) {
        mStcoTableEntries->write(mOwner);
    } else {
        mCo64TableEntries->write(mOwner);
    }
    mOwner->endBox();  // stco or co64
}

void MPEG4Writer::writeUdtaBox() {
    beginBox("udta");
    writeGeoDataBox();
    endBox();
}

void MPEG4Writer::writeHdlr(const char *handlerType) {
    beginBox("hdlr");
    writeInt32(0); // Version, Flags
    writeInt32(0); // Predefined
    writeFourcc(handlerType);
    writeInt32(0); // Reserved[0]
    writeInt32(0); // Reserved[1]
    writeInt32(0); // Reserved[2]
    writeInt8(0);  // Name (empty)
    endBox();
}

void MPEG4Writer::writeKeys() {
    size_t count = mMetaKeys->countEntries();

    beginBox("keys");
    writeInt32(0);     // Version, Flags
    writeInt32(count); // Entry_count
    for (size_t i = 0; i < count; i++) {
        AMessage::Type type;
        const char *key = mMetaKeys->getEntryNameAt(i, &type);
        size_t n = strlen(key);
        writeInt32(n + 8);
        writeFourcc("mdta");
        write(key, n); // write without the \0
    }
    endBox();
}

void MPEG4Writer::writeIlst() {
    size_t count = mMetaKeys->countEntries();

    beginBox("ilst");
    for (size_t i = 0; i < count; i++) {
        beginBox(i + 1); // key id (1-based)
        beginBox("data");
        AMessage::Type type;
        const char *key = mMetaKeys->getEntryNameAt(i, &type);
        switch (type) {
            case AMessage::kTypeString:
            {
                AString val;
                CHECK(mMetaKeys->findString(key, &val));
                writeInt32(1); // type = UTF8
                writeInt32(0); // default country/language
                write(val.c_str(), strlen(val.c_str())); // write without \0
                break;
            }

            case AMessage::kTypeFloat:
            {
                float val;
                CHECK(mMetaKeys->findFloat(key, &val));
                writeInt32(23); // type = float32
                writeInt32(0);  // default country/language
                writeInt32(*reinterpret_cast<int32_t *>(&val));
                break;
            }

            case AMessage::kTypeInt32:
            {
                int32_t val;
                CHECK(mMetaKeys->findInt32(key, &val));
                writeInt32(67); // type = signed int32
                writeInt32(0);  // default country/language
                writeInt32(val);
                break;
            }

            default:
            {
                ALOGW("Unsupported key type, writing 0 instead");
                writeInt32(77); // type = unsigned int32
                writeInt32(0);  // default country/language
                writeInt32(0);
                break;
            }
        }
        endBox(); // data
        endBox(); // key id
    }
    endBox(); // ilst
}

void MPEG4Writer::writeMoovLevelMetaBox() {
    size_t count = mMetaKeys->countEntries();
    if (count == 0) {
        return;
    }

    beginBox("meta");
    writeHdlr("mdta");
    writeKeys();
    writeIlst();
    endBox();
}

void MPEG4Writer::writeIlocBox() {
    beginBox("iloc");
    // Use version 1 to allow construction method 1 that refers to
    // data in idat box inside meta box.
    writeInt32(0x01000000); // Version = 1, Flags = 0
    writeInt16(0x4400);     // offset_size = length_size = 4
                            // base_offset_size = index_size = 0

    // 16-bit item_count
    size_t itemCount = mItems.size();
    if (itemCount > 65535) {
        ALOGW("Dropping excess items: itemCount %zu", itemCount);
        itemCount = 65535;
    }
    writeInt16((uint16_t)itemCount);

    for (size_t i = 0; i < itemCount; i++) {
        writeInt16(mItems[i].itemId);
        bool isGrid = mItems[i].isGrid();

        writeInt16(isGrid ? 1 : 0); // construction_method
        writeInt16(0); // data_reference_index = 0
        writeInt16(1); // extent_count = 1

        if (isGrid) {
            // offset into the 'idat' box
            writeInt32(mNumGrids++ * 8);
            writeInt32(8);
        } else {
            writeInt32(mItems[i].offset);
            writeInt32(mItems[i].size);
        }
    }
    endBox();
}

void MPEG4Writer::writeInfeBox(
        uint16_t itemId, const char *itemType, uint32_t flags) {
    beginBox("infe");
    writeInt32(0x02000000 | flags); // Version = 2, Flags = 0
    writeInt16(itemId);
    writeInt16(0);          //item_protection_index = 0
    writeFourcc(itemType);
    writeCString("");       // item_name
    endBox();
}

void MPEG4Writer::writeIinfBox() {
    beginBox("iinf");
    writeInt32(0);          // Version = 0, Flags = 0

    // 16-bit item_count
    size_t itemCount = mItems.size();
    if (itemCount > 65535) {
        ALOGW("Dropping excess items: itemCount %zu", itemCount);
        itemCount = 65535;
    }

    writeInt16((uint16_t)itemCount);
    for (size_t i = 0; i < itemCount; i++) {
        writeInfeBox(mItems[i].itemId, mItems[i].itemType,
                (mItems[i].isImage() && mItems[i].isHidden) ? 1 : 0);
    }

    endBox();
}

void MPEG4Writer::writeIdatBox() {
    beginBox("idat");

    for (size_t i = 0; i < mItems.size(); i++) {
        if (mItems[i].isGrid()) {
            writeInt8(0); // version
            // flags == 1 means 32-bit width,height
            int8_t flags = (mItems[i].width > 65535 || mItems[i].height > 65535);
            writeInt8(flags);
            writeInt8(mItems[i].rows - 1);
            writeInt8(mItems[i].cols - 1);
            if (flags) {
                writeInt32(mItems[i].width);
                writeInt32(mItems[i].height);
            } else {
                writeInt16((uint16_t)mItems[i].width);
                writeInt16((uint16_t)mItems[i].height);
            }
        }
    }

    endBox();
}

void MPEG4Writer::writeIrefBox() {
    beginBox("iref");
    writeInt32(0);          // Version = 0, Flags = 0
    {
        for (size_t i = 0; i < mItems.size(); i++) {
            for (size_t r = 0; r < mItems[i].refsList.size(); r++) {
                const ItemRefs &refs = mItems[i].refsList[r];
                beginBox(refs.key);
                writeInt16(mItems[i].itemId);
                size_t refCount = refs.value.size();
                if (refCount > 65535) {
                    ALOGW("too many entries in %s", refs.key);
                    refCount = 65535;
                }
                writeInt16((uint16_t)refCount);
                for (size_t refIndex = 0; refIndex < refCount; refIndex++) {
                    writeInt16(refs.value[refIndex]);
                }
                endBox();
            }
        }
    }
    endBox();
}

void MPEG4Writer::writePitmBox() {
    beginBox("pitm");
    writeInt32(0);          // Version = 0, Flags = 0
    writeInt16(mPrimaryItemId);
    endBox();
}

void MPEG4Writer::writeIpcoBox() {
    beginBox("ipco");
    size_t numProperties = mProperties.size();
    if (numProperties > 32767) {
        ALOGW("Dropping excess properties: numProperties %zu", numProperties);
        numProperties = 32767;
    }
    for (size_t propIndex = 0; propIndex < numProperties; propIndex++) {
        switch (mProperties[propIndex].type) {
            case FOURCC('h', 'v', 'c', 'C'):
            {
                beginBox("hvcC");
                sp<ABuffer> hvcc = mProperties[propIndex].hvcc;
                // Patch avcc's lengthSize field to match the number
                // of bytes we use to indicate the size of a nal unit.
                uint8_t *ptr = (uint8_t *)hvcc->data();
                ptr[21] = (ptr[21] & 0xfc) | (useNalLengthFour() ? 3 : 1);
                write(hvcc->data(), hvcc->size());
                endBox();
                break;
            }
            case FOURCC('i', 's', 'p', 'e'):
            {
                beginBox("ispe");
                writeInt32(0); // Version = 0, Flags = 0
                writeInt32(mProperties[propIndex].width);
                writeInt32(mProperties[propIndex].height);
                endBox();
                break;
            }
            case FOURCC('i', 'r', 'o', 't'):
            {
                beginBox("irot");
                writeInt8(mProperties[propIndex].rotation);
                endBox();
                break;
            }
            default:
                ALOGW("Skipping unrecognized property: type 0x%08x",
                        mProperties[propIndex].type);
        }
    }
    endBox();
}

void MPEG4Writer::writeIpmaBox() {
    beginBox("ipma");
    uint32_t flags = (mProperties.size() > 127) ? 1 : 0;
    writeInt32(flags); // Version = 0

    writeInt32(mAssociationEntryCount);
    for (size_t itemIndex = 0; itemIndex < mItems.size(); itemIndex++) {
        const Vector<uint16_t> &properties = mItems[itemIndex].properties;
        if (properties.empty()) {
            continue;
        }
        writeInt16(mItems[itemIndex].itemId);

        size_t entryCount = properties.size();
        if (entryCount > 255) {
            ALOGW("Dropping excess associations: entryCount %zu", entryCount);
            entryCount = 255;
        }
        writeInt8((uint8_t)entryCount);
        for (size_t propIndex = 0; propIndex < entryCount; propIndex++) {
            if (flags & 1) {
                writeInt16((1 << 15) | properties[propIndex]);
            } else {
                writeInt8((1 << 7) | properties[propIndex]);
            }
        }
    }
    endBox();
}

void MPEG4Writer::writeIprpBox() {
    beginBox("iprp");
    writeIpcoBox();
    writeIpmaBox();
    endBox();
}

void MPEG4Writer::writeFileLevelMetaBox() {
    // patch up the mPrimaryItemId and count items with prop associations
    uint16_t firstVisibleItemId = 0;
    uint16_t firstImageItemId = 0;
    for (size_t index = 0; index < mItems.size(); index++) {
        if (!mItems[index].isImage()) continue;

        if (mItems[index].isPrimary) {
            mPrimaryItemId = mItems[index].itemId;
        }
        if (!firstImageItemId) {
            firstImageItemId = mItems[index].itemId;
        }
        if (!firstVisibleItemId && !mItems[index].isHidden) {
            firstVisibleItemId = mItems[index].itemId;
        }
        if (!mItems[index].properties.empty()) {
            mAssociationEntryCount++;
        }
    }

    if (!firstImageItemId) {
        ALOGE("no valid image was found");
        return;
    }

    if (mPrimaryItemId == 0) {
        if (firstVisibleItemId > 0) {
            ALOGW("didn't find primary, using first visible image");
            mPrimaryItemId = firstVisibleItemId;
        } else {
            ALOGW("no primary and no visible item, using first image");
            mPrimaryItemId = firstImageItemId;
        }
    }

    for (List<Track *>::iterator it = mTracks.begin();
        it != mTracks.end(); ++it) {
        if ((*it)->isHeic()) {
            (*it)->flushItemRefs();
        }
    }

    beginBox("meta");
    writeInt32(0); // Version = 0, Flags = 0
    writeHdlr("pict");
    writeIlocBox();
    writeIinfBox();
    writePitmBox();
    writeIprpBox();
    if (mNumGrids > 0) {
        writeIdatBox();
    }
    if (mHasRefs) {
        writeIrefBox();
    }
    endBox();
}

uint16_t MPEG4Writer::addProperty_l(const ItemProperty &prop) {
    char typeStr[5];
    MakeFourCCString(prop.type, typeStr);
    ALOGV("addProperty_l: %s", typeStr);

    mProperties.push_back(prop);

    // returning 1-based property index
    return mProperties.size();
}

uint16_t MPEG4Writer::addItem_l(const ItemInfo &info) {
    ALOGV("addItem_l: type %s, offset %u, size %u",
            info.itemType, info.offset, info.size);

    size_t index = mItems.size();
    mItems.push_back(info);

    // make the item id start at kItemIdBase
    mItems.editItemAt(index).itemId = index + kItemIdBase;

#if (LOG_NDEBUG==0)
    if (!info.properties.empty()) {
        AString str;
        for (size_t i = 0; i < info.properties.size(); i++) {
            if (i > 0) {
                str.append(", ");
            }
            str.append(info.properties[i]);
        }
        ALOGV("addItem_l: id %d, properties: %s", mItems[index].itemId, str.c_str());
    }
#endif // (LOG_NDEBUG==0)

    return mItems[index].itemId;
}

void MPEG4Writer::addRefs_l(uint16_t itemId, const ItemRefs &refs) {
    if (refs.value.empty()) {
        return;
    }
    if (itemId < kItemIdBase) {
        ALOGW("itemId shouldn't be smaller than kItemIdBase");
        return;
    }

    size_t index = itemId - kItemIdBase;
    mItems.editItemAt(index).refsList.push_back(refs);
    mHasRefs = true;
}

/*
 * Geodata is stored according to ISO-6709 standard.
 */
void MPEG4Writer::writeGeoDataBox() {
    beginBox("\xA9xyz");
    /*
     * For historical reasons, any user data start
     * with "\0xA9", must be followed by its assoicated
     * language code.
     * 0x0012: text string length
     * 0x15c7: lang (locale) code: en
     */
    writeInt32(0x001215c7);
    writeLatitude(mLatitudex10000);
    writeLongitude(mLongitudex10000);
    writeInt8(0x2F);
    endBox();
}

}  // namespace android
