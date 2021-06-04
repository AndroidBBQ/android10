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

#ifndef MPEG4_WRITER_H_

#define MPEG4_WRITER_H_

#include <stdio.h>

#include <media/stagefright/MediaWriter.h>
#include <utils/List.h>
#include <utils/threads.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ALooper.h>

namespace android {

struct AMessage;
class MediaBuffer;
struct ABuffer;

class MPEG4Writer : public MediaWriter {
public:
    MPEG4Writer(int fd);

    // Limitations
    // No more than one video and/or one audio source can be added, but
    // multiple metadata sources can be added.
    virtual status_t addSource(const sp<MediaSource> &source);

    // Returns INVALID_OPERATION if there is no source or track.
    virtual status_t start(MetaData *param = NULL);
    virtual status_t stop() { return reset(); }
    virtual status_t pause();
    virtual bool reachedEOS();
    virtual status_t dump(int fd, const Vector<String16>& args);

    void beginBox(const char *fourcc);
    void beginBox(uint32_t id);
    void writeInt8(int8_t x);
    void writeInt16(int16_t x);
    void writeInt32(int32_t x);
    void writeInt64(int64_t x);
    void writeCString(const char *s);
    void writeFourcc(const char *fourcc);
    void write(const void *data, size_t size);
    inline size_t write(const void *ptr, size_t size, size_t nmemb);
    void endBox();
    uint32_t interleaveDuration() const { return mInterleaveDurationUs; }
    status_t setInterleaveDuration(uint32_t duration);
    int32_t getTimeScale() const { return mTimeScale; }

    status_t setGeoData(int latitudex10000, int longitudex10000);
    status_t setCaptureRate(float captureFps);
    status_t setTemporalLayerCount(uint32_t layerCount);
    void notifyApproachingLimit();
    virtual void setStartTimeOffsetMs(int ms) { mStartTimeOffsetMs = ms; }
    virtual int32_t getStartTimeOffsetMs() const { return mStartTimeOffsetMs; }
    virtual status_t setNextFd(int fd);

protected:
    virtual ~MPEG4Writer();

private:
    class Track;
    friend struct AHandlerReflector<MPEG4Writer>;

    enum {
        kWhatSwitch                          = 'swch',
    };

    int  mFd;
    int mNextFd;
    sp<MetaData> mStartMeta;
    status_t mInitCheck;
    bool mIsRealTimeRecording;
    bool mUse4ByteNalLength;
    bool mUse32BitOffset;
    bool mIsFileSizeLimitExplicitlyRequested;
    bool mPaused;
    bool mStarted;  // Writer thread + track threads started successfully
    bool mWriterThreadStarted;  // Only writer thread started successfully
    bool mSendNotify;
    off64_t mOffset;
    off_t mMdatOffset;
    uint8_t *mInMemoryCache;
    off64_t mInMemoryCacheOffset;
    off64_t mInMemoryCacheSize;
    bool  mWriteBoxToMemory;
    off64_t mFreeBoxOffset;
    bool mStreamableFile;
    off64_t mMoovExtraSize;
    uint32_t mInterleaveDurationUs;
    int32_t mTimeScale;
    int64_t mStartTimestampUs;
    int32_t mStartTimeOffsetBFramesUs; // Start time offset when B Frames are present
    int mLatitudex10000;
    int mLongitudex10000;
    bool mAreGeoTagsAvailable;
    int32_t mStartTimeOffsetMs;
    bool mSwitchPending;

    sp<ALooper> mLooper;
    sp<AHandlerReflector<MPEG4Writer> > mReflector;

    Mutex mLock;

    List<Track *> mTracks;

    List<off64_t> mBoxes;

    sp<AMessage> mMetaKeys;

    void setStartTimestampUs(int64_t timeUs);
    int64_t getStartTimestampUs();  // Not const
    int32_t getStartTimeOffsetBFramesUs();
    status_t startTracks(MetaData *params);
    size_t numTracks();
    int64_t estimateMoovBoxSize(int32_t bitRate);
    int64_t estimateFileLevelMetaSize(MetaData *params);
    void writeCachedBoxToFile(const char *type);

    struct Chunk {
        Track               *mTrack;        // Owner
        int64_t             mTimeStampUs;   // Timestamp of the 1st sample
        List<MediaBuffer *> mSamples;       // Sample data

        // Convenient constructor
        Chunk(): mTrack(NULL), mTimeStampUs(0) {}

        Chunk(Track *track, int64_t timeUs, List<MediaBuffer *> samples)
            : mTrack(track), mTimeStampUs(timeUs), mSamples(samples) {
        }

    };
    struct ChunkInfo {
        Track               *mTrack;        // Owner
        List<Chunk>         mChunks;        // Remaining chunks to be written

        // Previous chunk timestamp that has been written
        int64_t mPrevChunkTimestampUs;

        // Max time interval between neighboring chunks
        int64_t mMaxInterChunkDurUs;

    };

    bool            mIsFirstChunk;
    volatile bool   mDone;                  // Writer thread is done?
    pthread_t       mThread;                // Thread id for the writer
    List<ChunkInfo> mChunkInfos;            // Chunk infos
    Condition       mChunkReadyCondition;   // Signal that chunks are available

    // HEIF writing
    typedef key_value_pair_t< const char *, Vector<uint16_t> > ItemRefs;
    typedef struct _ItemInfo {
        bool isGrid() const { return !strcmp("grid", itemType); }
        bool isImage() const { return !strcmp("hvc1", itemType) || isGrid(); }
        const char *itemType;
        uint16_t itemId;
        bool isPrimary;
        bool isHidden;
        union {
            // image item
            struct {
                uint32_t offset;
                uint32_t size;
            };
            // grid item
            struct {
                uint32_t rows;
                uint32_t cols;
                uint32_t width;
                uint32_t height;
            };
        };
        Vector<uint16_t> properties;
        Vector<ItemRefs> refsList;
    } ItemInfo;

    typedef struct _ItemProperty {
        uint32_t type;
        int32_t width;
        int32_t height;
        int32_t rotation;
        sp<ABuffer> hvcc;
    } ItemProperty;

    bool mHasFileLevelMeta;
    bool mHasMoovBox;
    uint32_t mPrimaryItemId;
    uint32_t mAssociationEntryCount;
    uint32_t mNumGrids;
    bool mHasRefs;
    Vector<ItemInfo> mItems;
    Vector<ItemProperty> mProperties;

    // Writer thread handling
    status_t startWriterThread();
    void stopWriterThread();
    static void *ThreadWrapper(void *me);
    void threadFunc();

    // Buffer a single chunk to be written out later.
    void bufferChunk(const Chunk& chunk);

    // Write all buffered chunks from all tracks
    void writeAllChunks();

    // Retrieve the proper chunk to write if there is one
    // Return true if a chunk is found; otherwise, return false.
    bool findChunkToWrite(Chunk *chunk);

    // Actually write the given chunk to the file.
    void writeChunkToFile(Chunk* chunk);

    // Adjust other track media clock (presumably wall clock)
    // based on audio track media clock with the drift time.
    int64_t mDriftTimeUs;
    void setDriftTimeUs(int64_t driftTimeUs);
    int64_t getDriftTimeUs();

    // Return whether the nal length is 4 bytes or 2 bytes
    // Only makes sense for H.264/AVC
    bool useNalLengthFour();

    // Return whether the writer is used for real time recording.
    // In real time recording mode, new samples will be allowed to buffered into
    // chunks in higher priority thread, even though the file writer has not
    // drained the chunks yet.
    // By default, real time recording is on.
    bool isRealTimeRecording() const;

    void lock();
    void unlock();

    // Init all the internal variables for each recording session. Some variables
    // will only need to be set for the first recording session and they will stay
    // the same across all the recording sessions.
    void initInternal(int fd, bool isFirstSession);

    // Acquire lock before calling these methods
    off64_t addSample_l(
            MediaBuffer *buffer, bool usePrefix,
            uint32_t tiffHdrOffset, size_t *bytesWritten);
    void addLengthPrefixedSample_l(MediaBuffer *buffer);
    void addMultipleLengthPrefixedSamples_l(MediaBuffer *buffer);
    uint16_t addProperty_l(const ItemProperty &);
    uint16_t addItem_l(const ItemInfo &);
    void addRefs_l(uint16_t itemId, const ItemRefs &);

    bool exceedsFileSizeLimit();
    bool use32BitFileOffset() const;
    bool exceedsFileDurationLimit();
    bool approachingFileSizeLimit();
    bool isFileStreamable() const;
    void trackProgressStatus(size_t trackId, int64_t timeUs, status_t err = OK);
    void writeCompositionMatrix(int32_t degrees);
    void writeMvhdBox(int64_t durationUs);
    void writeMoovBox(int64_t durationUs);
    void writeFtypBox(MetaData *param);
    void writeUdtaBox();
    void writeGeoDataBox();
    void writeLatitude(int degreex10000);
    void writeLongitude(int degreex10000);
    void finishCurrentSession();

    void addDeviceMeta();
    void writeHdlr(const char *handlerType);
    void writeKeys();
    void writeIlst();
    void writeMoovLevelMetaBox();

    // HEIF writing
    void writeIlocBox();
    void writeInfeBox(uint16_t itemId, const char *type, uint32_t flags);
    void writeIinfBox();
    void writeIpcoBox();
    void writeIpmaBox();
    void writeIprpBox();
    void writeIdatBox();
    void writeIrefBox();
    void writePitmBox();
    void writeFileLevelMetaBox();

    void sendSessionSummary();
    void release();
    status_t switchFd();
    status_t reset(bool stopSource = true);

    static uint32_t getMpeg4Time();

    void onMessageReceived(const sp<AMessage> &msg);

    MPEG4Writer(const MPEG4Writer &);
    MPEG4Writer &operator=(const MPEG4Writer &);
};

}  // namespace android

#endif  // MPEG4_WRITER_H_
