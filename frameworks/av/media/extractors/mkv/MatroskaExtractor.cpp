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

//#define LOG_NDEBUG 0
#define LOG_TAG "MatroskaExtractor"
#include <utils/Log.h>

#include "FLACDecoder.h"
#include "MatroskaExtractor.h"
#include "common/webmids.h"

#include <media/DataSourceBase.h>
#include <media/ExtractorUtils.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaDataUtils.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <utils/String8.h>

#include <arpa/inet.h>
#include <inttypes.h>
#include <vector>

namespace android {

struct DataSourceBaseReader : public mkvparser::IMkvReader {
    explicit DataSourceBaseReader(DataSourceHelper *source)
        : mSource(source) {
    }

    virtual int Read(long long position, long length, unsigned char* buffer) {
        CHECK(position >= 0);
        CHECK(length >= 0);

        if (length == 0) {
            return 0;
        }

        ssize_t n = mSource->readAt(position, buffer, length);

        if (n <= 0) {
            return -1;
        }

        return 0;
    }

    virtual int Length(long long* total, long long* available) {
        off64_t size;
        if (mSource->getSize(&size) != OK) {
            if (total) {
                *total = -1;
            }
            if (available) {
                *available = (long long)((1ull << 63) - 1);
            }

            return 0;
        }

        if (total) {
            *total = size;
        }

        if (available) {
            *available = size;
        }

        return 0;
    }

private:
    DataSourceHelper *mSource;

    DataSourceBaseReader(const DataSourceBaseReader &);
    DataSourceBaseReader &operator=(const DataSourceBaseReader &);
};

////////////////////////////////////////////////////////////////////////////////

struct BlockIterator {
    BlockIterator(MatroskaExtractor *extractor, unsigned long trackNum, unsigned long index);

    bool eos() const;

    void advance();
    void reset();

    void seek(
            int64_t seekTimeUs, bool isAudio,
            int64_t *actualFrameTimeUs);

    const mkvparser::Block *block() const;
    int64_t blockTimeUs() const;

private:
    MatroskaExtractor *mExtractor;
    long long mTrackNum;
    unsigned long mIndex;

    const mkvparser::Cluster *mCluster;
    const mkvparser::BlockEntry *mBlockEntry;
    long mBlockEntryIndex;

    unsigned long mTrackType;
    void seekwithoutcue_l(int64_t seekTimeUs, int64_t *actualFrameTimeUs);

    void advance_l();

    BlockIterator(const BlockIterator &);
    BlockIterator &operator=(const BlockIterator &);
};

struct MatroskaSource : public MediaTrackHelper {
    MatroskaSource(MatroskaExtractor *extractor, size_t index);

    virtual media_status_t start();
    virtual media_status_t stop();

    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options);

protected:
    virtual ~MatroskaSource();

private:
    enum Type {
        AVC,
        AAC,
        HEVC,
        MP3,
        PCM,
        VORBIS,
        OTHER
    };

    MatroskaExtractor *mExtractor;
    size_t mTrackIndex;
    Type mType;
    bool mIsAudio;
    BlockIterator mBlockIter;
    ssize_t mNALSizeLen;  // for type AVC or HEVC

    List<MediaBufferHelper *> mPendingFrames;

    int64_t mCurrentTS; // add for mp3
    uint32_t mMP3Header;

    media_status_t findMP3Header(uint32_t * header,
        const uint8_t *dataSource, int length, int *outStartPos);
    media_status_t mp3FrameRead(
            MediaBufferHelper **out, const ReadOptions *options,
            int64_t targetSampleTimeUs);

    status_t advance();

    status_t setWebmBlockCryptoInfo(MediaBufferHelper *mbuf);
    media_status_t readBlock();
    void clearPendingFrames();

    MatroskaSource(const MatroskaSource &);
    MatroskaSource &operator=(const MatroskaSource &);
};

const mkvparser::Track* MatroskaExtractor::TrackInfo::getTrack() const {
    return mExtractor->mSegment->GetTracks()->GetTrackByNumber(mTrackNum);
}

// This function does exactly the same as mkvparser::Cues::Find, except that it
// searches in our own track based vectors. We should not need this once mkvparser
// adds the same functionality.
const mkvparser::CuePoint::TrackPosition *MatroskaExtractor::TrackInfo::find(
        long long timeNs) const {
    ALOGV("mCuePoints.size %zu", mCuePoints.size());
    if (mCuePoints.empty()) {
        return NULL;
    }

    const mkvparser::CuePoint* cp = mCuePoints.itemAt(0);
    const mkvparser::Track* track = getTrack();
    if (timeNs <= cp->GetTime(mExtractor->mSegment)) {
        return cp->Find(track);
    }

    // Binary searches through relevant cues; assumes cues are ordered by timecode.
    // If we do detect out-of-order cues, return NULL.
    size_t lo = 0;
    size_t hi = mCuePoints.size();
    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        const mkvparser::CuePoint* const midCp = mCuePoints.itemAt(mid);
        const long long cueTimeNs = midCp->GetTime(mExtractor->mSegment);
        if (cueTimeNs <= timeNs) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (lo == 0) {
        return NULL;
    }

    cp = mCuePoints.itemAt(lo - 1);
    if (cp->GetTime(mExtractor->mSegment) > timeNs) {
        return NULL;
    }

    return cp->Find(track);
}

MatroskaSource::MatroskaSource(
        MatroskaExtractor *extractor, size_t index)
    : mExtractor(extractor),
      mTrackIndex(index),
      mType(OTHER),
      mIsAudio(false),
      mBlockIter(mExtractor,
                 mExtractor->mTracks.itemAt(index).mTrackNum,
                 index),
      mNALSizeLen(-1),
      mCurrentTS(0),
      mMP3Header(0) {
    MatroskaExtractor::TrackInfo &trackInfo = mExtractor->mTracks.editItemAt(index);
    AMediaFormat *meta = trackInfo.mMeta;

    const char *mime;
    CHECK(AMediaFormat_getString(meta, AMEDIAFORMAT_KEY_MIME, &mime));

    mIsAudio = !strncasecmp("audio/", mime, 6);

    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
        mType = AVC;

        int32_t nalSizeLen = trackInfo.mNalLengthSize;
        if (nalSizeLen >= 0 && nalSizeLen <= 4) {
            mNALSizeLen = nalSizeLen;
        } else {
            ALOGE("No AVC mNALSizeLen");
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)) {
        mType = HEVC;

        int32_t nalSizeLen = trackInfo.mNalLengthSize;
        if (nalSizeLen >= 0 && nalSizeLen <= 4) {
            mNALSizeLen = nalSizeLen;
        } else {
            ALOGE("No HEVC mNALSizeLen");
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        mType = AAC;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
        mType = MP3;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW)) {
        mType = PCM;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
        mType = VORBIS;
    }
}

MatroskaSource::~MatroskaSource() {
    clearPendingFrames();
}

media_status_t MatroskaSource::start() {
    if (mType == AVC && mNALSizeLen < 0) {
        return AMEDIA_ERROR_MALFORMED;
    }

    // allocate one small initial buffer, but leave plenty of room to grow
    mBufferGroup->init(1 /* number of buffers */, 1024 /* buffer size */, 64 /* growth limit */);
    mBlockIter.reset();

    if (mType == MP3 && mMP3Header == 0) {
        int start = -1;
        media_status_t err = findMP3Header(&mMP3Header, NULL, 0, &start);
        if (err != OK) {
            ALOGE("No mp3 header found");
            clearPendingFrames();
            return err;
        }
    }

    return AMEDIA_OK;
}

media_status_t MatroskaSource::stop() {
    clearPendingFrames();

    return AMEDIA_OK;
}

media_status_t MatroskaSource::getFormat(AMediaFormat *meta) {
    return AMediaFormat_copy(meta, mExtractor->mTracks.itemAt(mTrackIndex).mMeta);
}

////////////////////////////////////////////////////////////////////////////////

BlockIterator::BlockIterator(
        MatroskaExtractor *extractor, unsigned long trackNum, unsigned long index)
    : mExtractor(extractor),
      mTrackNum(trackNum),
      mIndex(index),
      mCluster(NULL),
      mBlockEntry(NULL),
      mBlockEntryIndex(0) {
    mTrackType = mExtractor->mSegment->GetTracks()->GetTrackByNumber(trackNum)->GetType();
    reset();
}

bool BlockIterator::eos() const {
    return mCluster == NULL || mCluster->EOS();
}

void BlockIterator::advance() {
    Mutex::Autolock autoLock(mExtractor->mLock);
    advance_l();
}

void BlockIterator::advance_l() {
    for (;;) {
        long res = mCluster->GetEntry(mBlockEntryIndex, mBlockEntry);
        ALOGV("GetEntry returned %ld", res);

        long long pos;
        long len;
        if (res < 0) {
            // Need to parse this cluster some more

            CHECK_EQ(res, mkvparser::E_BUFFER_NOT_FULL);

            res = mCluster->Parse(pos, len);
            ALOGV("Parse returned %ld", res);

            if (res < 0) {
                // I/O error

                ALOGE("Cluster::Parse returned result %ld", res);

                mCluster = NULL;
                break;
            }

            continue;
        } else if (res == 0) {
            // We're done with this cluster

            const mkvparser::Cluster *nextCluster;
            res = mExtractor->mSegment->ParseNext(
                    mCluster, nextCluster, pos, len);
            ALOGV("ParseNext returned %ld", res);

            if (res != 0) {
                // EOF or error

                mCluster = NULL;
                break;
            }

            CHECK_EQ(res, 0);
            CHECK(nextCluster != NULL);
            CHECK(!nextCluster->EOS());

            mCluster = nextCluster;

            res = mCluster->Parse(pos, len);
            ALOGV("Parse (2) returned %ld", res);

            if (res < 0) {
                // I/O error

                ALOGE("Cluster::Parse returned result %ld", res);

                mCluster = NULL;
                break;
            }

            mBlockEntryIndex = 0;
            continue;
        }

        CHECK(mBlockEntry != NULL);
        CHECK(mBlockEntry->GetBlock() != NULL);
        ++mBlockEntryIndex;

        if (mBlockEntry->GetBlock()->GetTrackNumber() == mTrackNum) {
            break;
        }
    }
}

void BlockIterator::reset() {
    Mutex::Autolock autoLock(mExtractor->mLock);

    mCluster = mExtractor->mSegment->GetFirst();
    mBlockEntry = NULL;
    mBlockEntryIndex = 0;

    do {
        advance_l();
    } while (!eos() && block()->GetTrackNumber() != mTrackNum);
}

void BlockIterator::seek(
        int64_t seekTimeUs, bool isAudio,
        int64_t *actualFrameTimeUs) {
    Mutex::Autolock autoLock(mExtractor->mLock);

    *actualFrameTimeUs = -1ll;

    if (seekTimeUs > INT64_MAX / 1000ll ||
            seekTimeUs < INT64_MIN / 1000ll ||
            (mExtractor->mSeekPreRollNs > 0 &&
                    (seekTimeUs * 1000ll) < INT64_MIN + mExtractor->mSeekPreRollNs) ||
            (mExtractor->mSeekPreRollNs < 0 &&
                    (seekTimeUs * 1000ll) > INT64_MAX + mExtractor->mSeekPreRollNs)) {
        ALOGE("cannot seek to %lld", (long long) seekTimeUs);
        return;
    }

    const int64_t seekTimeNs = seekTimeUs * 1000ll - mExtractor->mSeekPreRollNs;

    mkvparser::Segment* const pSegment = mExtractor->mSegment;

    // Special case the 0 seek to avoid loading Cues when the application
    // extraneously seeks to 0 before playing.
    if (seekTimeNs <= 0) {
        ALOGV("Seek to beginning: %" PRId64, seekTimeUs);
        mCluster = pSegment->GetFirst();
        mBlockEntryIndex = 0;
        do {
            advance_l();
        } while (!eos() && block()->GetTrackNumber() != mTrackNum);
        return;
    }

    ALOGV("Seeking to: %" PRId64, seekTimeUs);

    // If the Cues have not been located then find them.
    const mkvparser::Cues* pCues = pSegment->GetCues();
    const mkvparser::SeekHead* pSH = pSegment->GetSeekHead();
    if (!pCues && pSH) {
        const size_t count = pSH->GetCount();
        const mkvparser::SeekHead::Entry* pEntry;
        ALOGV("No Cues yet");

        for (size_t index = 0; index < count; index++) {
            pEntry = pSH->GetEntry(index);

            if (pEntry->id == libwebm::kMkvCues) { // Cues ID
                long len; long long pos;
                pSegment->ParseCues(pEntry->pos, pos, len);
                pCues = pSegment->GetCues();
                ALOGV("Cues found");
                break;
            }
        }

        if (!pCues) {
            ALOGV("No Cues in file,seek without cue data");
            seekwithoutcue_l(seekTimeUs, actualFrameTimeUs);
            return;
        }
    }
    else if (!pSH) {
        ALOGV("No SeekHead, seek without cue data");
        seekwithoutcue_l(seekTimeUs, actualFrameTimeUs);
        return;
    }

    const mkvparser::CuePoint* pCP;
    mkvparser::Tracks const *pTracks = pSegment->GetTracks();
    while (!pCues->DoneParsing()) {
        pCues->LoadCuePoint();
        pCP = pCues->GetLast();
        ALOGV("pCP = %s", pCP == NULL ? "NULL" : "not NULL");
        if (pCP == NULL)
            continue;

        size_t trackCount = mExtractor->mTracks.size();
        for (size_t index = 0; index < trackCount; ++index) {
            MatroskaExtractor::TrackInfo& track = mExtractor->mTracks.editItemAt(index);
            const mkvparser::Track *pTrack = pTracks->GetTrackByNumber(track.mTrackNum);
            if (pTrack && pTrack->GetType() == 1 && pCP->Find(pTrack)) { // VIDEO_TRACK
                track.mCuePoints.push_back(pCP);
            }
        }

        if (pCP->GetTime(pSegment) >= seekTimeNs) {
            ALOGV("Parsed past relevant Cue");
            break;
        }
    }

    const mkvparser::CuePoint::TrackPosition *pTP = NULL;
    const mkvparser::Track *thisTrack = pTracks->GetTrackByNumber(mTrackNum);
    if (thisTrack->GetType() == 1) { // video
        MatroskaExtractor::TrackInfo& track = mExtractor->mTracks.editItemAt(mIndex);
        pTP = track.find(seekTimeNs);
    } else {
        // The Cue index is built around video keyframes
        unsigned long int trackCount = pTracks->GetTracksCount();
        for (size_t index = 0; index < trackCount; ++index) {
            const mkvparser::Track *pTrack = pTracks->GetTrackByIndex(index);
            if (pTrack && pTrack->GetType() == 1 && pCues->Find(seekTimeNs, pTrack, pCP, pTP)) {
                ALOGV("Video track located at %zu", index);
                break;
            }
        }
    }


    // Always *search* based on the video track, but finalize based on mTrackNum
    if (!pTP) {
        ALOGE("Did not locate the video track for seeking");
        seekwithoutcue_l(seekTimeUs, actualFrameTimeUs);
        return;
    }

    mCluster = pSegment->FindOrPreloadCluster(pTP->m_pos);

    CHECK(mCluster);
    CHECK(!mCluster->EOS());

    // mBlockEntryIndex starts at 0 but m_block starts at 1
    CHECK_GT(pTP->m_block, 0);
    mBlockEntryIndex = pTP->m_block - 1;

    for (;;) {
        advance_l();

        if (eos()) break;

        if (isAudio || block()->IsKey()) {
            // Accept the first key frame
            int64_t frameTimeUs = (block()->GetTime(mCluster) + 500LL) / 1000LL;
            if (thisTrack->GetType() == 1 || frameTimeUs >= seekTimeUs) {
                *actualFrameTimeUs = frameTimeUs;
                ALOGV("Requested seek point: %" PRId64 " actual: %" PRId64,
                      seekTimeUs, *actualFrameTimeUs);
                break;
            }
        }
    }
}

const mkvparser::Block *BlockIterator::block() const {
    CHECK(!eos());

    return mBlockEntry->GetBlock();
}

int64_t BlockIterator::blockTimeUs() const {
    if (mCluster == NULL || mBlockEntry == NULL) {
        return -1;
    }
    return (mBlockEntry->GetBlock()->GetTime(mCluster) + 500ll) / 1000ll;
}

void BlockIterator::seekwithoutcue_l(int64_t seekTimeUs, int64_t *actualFrameTimeUs) {
    mCluster = mExtractor->mSegment->FindCluster(seekTimeUs * 1000ll);
    const long status = mCluster->GetFirst(mBlockEntry);
    if (status < 0) {  // error
        ALOGE("get last blockenry failed!");
        mCluster = NULL;
        return;
    }
    mBlockEntryIndex = 0;
    while (!eos() && ((block()->GetTrackNumber() != mTrackNum) || (blockTimeUs() < seekTimeUs))) {
        advance_l();
    }

    // video track will seek to the next key frame.
    if (mTrackType == 1) {
        while (!eos() && ((block()->GetTrackNumber() != mTrackNum) ||
                      !mBlockEntry->GetBlock()->IsKey())) {
            advance_l();
        }
    }
    *actualFrameTimeUs = blockTimeUs();
     ALOGV("seekTimeUs:%lld, actualFrameTimeUs:%lld, tracknum:%lld",
              (long long)seekTimeUs, (long long)*actualFrameTimeUs, (long long)mTrackNum);
}

////////////////////////////////////////////////////////////////////////////////

static unsigned U24_AT(const uint8_t *ptr) {
    return ptr[0] << 16 | ptr[1] << 8 | ptr[2];
}

static AString uriDebugString(const char *uri) {
    // find scheme
    AString scheme;
    for (size_t i = 0; i < strlen(uri); i++) {
        const char c = uri[i];
        if (!isascii(c)) {
            break;
        } else if (isalpha(c)) {
            continue;
        } else if (i == 0) {
            // first character must be a letter
            break;
        } else if (isdigit(c) || c == '+' || c == '.' || c =='-') {
            continue;
        } else if (c != ':') {
            break;
        }
        scheme = AString(uri, 0, i);
        scheme.append("://<suppressed>");
        return scheme;
    }
    return AString("<no-scheme URI suppressed>");
}

void MatroskaSource::clearPendingFrames() {
    while (!mPendingFrames.empty()) {
        MediaBufferHelper *frame = *mPendingFrames.begin();
        mPendingFrames.erase(mPendingFrames.begin());

        frame->release();
        frame = NULL;
    }
}

status_t MatroskaSource::setWebmBlockCryptoInfo(MediaBufferHelper *mbuf) {
    if (mbuf->range_length() < 1 || mbuf->range_length() - 1 > INT32_MAX) {
        // 1-byte signal
        return ERROR_MALFORMED;
    }

    const uint8_t *data = (const uint8_t *)mbuf->data() + mbuf->range_offset();
    bool encrypted = data[0] & 0x1;
    bool partitioned = data[0] & 0x2;
    if (encrypted && mbuf->range_length() < 9) {
        // 1-byte signal + 8-byte IV
        return ERROR_MALFORMED;
    }

    AMediaFormat *meta = mbuf->meta_data();
    if (encrypted) {
        uint8_t ctrCounter[16] = { 0 };
        const uint8_t *keyId;
        size_t keyIdSize;
        AMediaFormat *trackMeta = mExtractor->mTracks.itemAt(mTrackIndex).mMeta;
        AMediaFormat_getBuffer(trackMeta, AMEDIAFORMAT_KEY_CRYPTO_KEY,
                (void**)&keyId, &keyIdSize);
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_KEY, keyId, keyIdSize);
        memcpy(ctrCounter, data + 1, 8);
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_IV, ctrCounter, 16);
        if (partitioned) {
            /*  0                   1                   2                   3
             *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             * |  Signal Byte  |                                               |
             * +-+-+-+-+-+-+-+-+             IV                                |
             * |                                                               |
             * |               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             * |               | num_partition |     Partition 0 offset ->     |
             * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
             * |     -> Partition 0 offset     |              ...              |
             * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
             * |             ...               |     Partition n-1 offset ->   |
             * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
             * |     -> Partition n-1 offset   |                               |
             * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
             * |                    Clear/encrypted sample data                |
             * |                                                               |
             * |                                                               |
             * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             */
            if (mbuf->range_length() < 10) {
                return ERROR_MALFORMED;
            }
            uint8_t numPartitions = data[9];
            if (mbuf->range_length() - 10 < numPartitions * sizeof(uint32_t)) {
                return ERROR_MALFORMED;
            }
            std::vector<uint32_t> plainSizes, encryptedSizes;
            uint32_t prev = 0;
            uint32_t frameOffset = 10 + numPartitions * sizeof(uint32_t);
            const uint32_t *partitions = reinterpret_cast<const uint32_t*>(data + 10);
            for (uint32_t i = 0; i <= numPartitions; ++i) {
                uint32_t p_i = i < numPartitions
                        ? ntohl(partitions[i])
                        : (mbuf->range_length() - frameOffset);
                if (p_i < prev) {
                    return ERROR_MALFORMED;
                }
                uint32_t size = p_i - prev;
                prev = p_i;
                if (i % 2) {
                    encryptedSizes.push_back(size);
                } else {
                    plainSizes.push_back(size);
                }
            }
            if (plainSizes.size() > encryptedSizes.size()) {
                encryptedSizes.push_back(0);
            }
            uint32_t sizeofPlainSizes = sizeof(uint32_t) * plainSizes.size();
            uint32_t sizeofEncryptedSizes = sizeof(uint32_t) * encryptedSizes.size();
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES,
                    plainSizes.data(), sizeofPlainSizes);
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES,
                    encryptedSizes.data(), sizeofEncryptedSizes);
            mbuf->set_range(frameOffset, mbuf->range_length() - frameOffset);
        } else {
            /*
             *  0                   1                   2                   3
             *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  |  Signal Byte  |                                               |
             *  +-+-+-+-+-+-+-+-+             IV                                |
             *  |                                                               |
             *  |               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  |               |                                               |
             *  |-+-+-+-+-+-+-+-+                                               |
             *  :               Bytes 1..N of encrypted frame                   :
             *  |                                                               |
             *  |                                                               |
             *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             */
            int32_t plainSizes[] = { 0 };
            int32_t encryptedSizes[] = { static_cast<int32_t>(mbuf->range_length() - 9) };
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES,
                    plainSizes, sizeof(plainSizes));
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES,
                    encryptedSizes, sizeof(encryptedSizes));
            mbuf->set_range(9, mbuf->range_length() - 9);
        }
    } else {
        /*
         *  0                   1                   2                   3
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |  Signal Byte  |                                               |
         *  +-+-+-+-+-+-+-+-+                                               |
         *  :               Bytes 1..N of unencrypted frame                 :
         *  |                                                               |
         *  |                                                               |
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */
        int32_t plainSizes[] = { static_cast<int32_t>(mbuf->range_length() - 1) };
        int32_t encryptedSizes[] = { 0 };
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES,
                plainSizes, sizeof(plainSizes));
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES,
                encryptedSizes, sizeof(encryptedSizes));
        mbuf->set_range(1, mbuf->range_length() - 1);
    }

    return OK;
}

media_status_t MatroskaSource::readBlock() {
    CHECK(mPendingFrames.empty());

    if (mBlockIter.eos()) {
        return AMEDIA_ERROR_END_OF_STREAM;
    }

    const mkvparser::Block *block = mBlockIter.block();

    int64_t timeUs = mBlockIter.blockTimeUs();

    for (int i = 0; i < block->GetFrameCount(); ++i) {
        MatroskaExtractor::TrackInfo *trackInfo = &mExtractor->mTracks.editItemAt(mTrackIndex);
        const mkvparser::Block::Frame &frame = block->GetFrame(i);
        size_t len = frame.len;
        if (SIZE_MAX - len < trackInfo->mHeaderLen) {
            return AMEDIA_ERROR_MALFORMED;
        }

        len += trackInfo->mHeaderLen;
        MediaBufferHelper *mbuf;
        mBufferGroup->acquire_buffer(&mbuf, false /* nonblocking */, len /* requested size */);
        mbuf->set_range(0, len);
        uint8_t *data = static_cast<uint8_t *>(mbuf->data());
        if (trackInfo->mHeader) {
            memcpy(data, trackInfo->mHeader, trackInfo->mHeaderLen);
        }

        AMediaFormat *meta = mbuf->meta_data();
        AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, timeUs);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, block->IsKey());

        if (mType == VORBIS) {
            int32_t sampleRate;
            if (!AMediaFormat_getInt32(trackInfo->mMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE,
                                       &sampleRate)) {
                return AMEDIA_ERROR_MALFORMED;
            }
            int64_t durationUs;
            if (!AMediaFormat_getInt64(trackInfo->mMeta, AMEDIAFORMAT_KEY_DURATION,
                                       &durationUs)) {
                return AMEDIA_ERROR_MALFORMED;
            }
            // TODO: Explore if this can be handled similar to MPEG4 extractor where padding is
            // signalled instead of VALID_SAMPLES
            // Remaining valid samples in Vorbis track
            if (durationUs > timeUs) {
                int32_t validSamples = ((durationUs - timeUs) * sampleRate) / 1000000ll;
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_VALID_SAMPLES, validSamples);
            }
        }

        status_t err = frame.Read(mExtractor->mReader, data + trackInfo->mHeaderLen);
        if (err == OK
                && mExtractor->mIsWebm
                && trackInfo->mEncrypted) {
            err = setWebmBlockCryptoInfo(mbuf);
        }

        if (err != OK) {
            mPendingFrames.clear();

            mBlockIter.advance();
            mbuf->release();
            return AMEDIA_ERROR_UNKNOWN;
        }

        mPendingFrames.push_back(mbuf);
    }

    mBlockIter.advance();

    return AMEDIA_OK;
}

//the value of kMP3HeaderMask is from MP3Extractor
static const uint32_t kMP3HeaderMask = 0xfffe0c00;

media_status_t MatroskaSource::findMP3Header(uint32_t * header,
        const uint8_t *dataSource, int length, int *outStartPos) {
    if (NULL == header) {
        ALOGE("header is null!");
        return AMEDIA_ERROR_END_OF_STREAM;
    }

    //to find header start position
    if (0 != *header) {
        if (NULL == dataSource) {
            *outStartPos = -1;
            return AMEDIA_OK;
        }
        uint32_t tmpCode = 0;
        for (int i = 0; i < length; i++) {
            tmpCode = (tmpCode << 8) + dataSource[i];
            if ((tmpCode & kMP3HeaderMask) == (*header & kMP3HeaderMask)) {
                *outStartPos = i - 3;
                return AMEDIA_OK;
            }
        }
        *outStartPos = -1;
        return AMEDIA_OK;
    }

    //to find mp3 header
    uint32_t code = 0;
    while (0 == *header) {
        while (mPendingFrames.empty()) {
            media_status_t err = readBlock();
            if (err != OK) {
                clearPendingFrames();
                return err;
            }
        }
        MediaBufferHelper *frame = *mPendingFrames.begin();
        size_t size = frame->range_length();
        size_t offset = frame->range_offset();
        size_t i;
        size_t frame_size;
        for (i = 0; i < size; i++) {
            ALOGV("data[%zu]=%x", i, *((uint8_t*)frame->data() + offset + i));
            code = (code << 8) + *((uint8_t*)frame->data() + offset + i);
            if (GetMPEGAudioFrameSize(code, &frame_size, NULL, NULL, NULL)) {
                *header = code;
                mBlockIter.reset();
                clearPendingFrames();
                return AMEDIA_OK;
            }
        }
    }

    return AMEDIA_ERROR_END_OF_STREAM;
}

media_status_t MatroskaSource::mp3FrameRead(
        MediaBufferHelper **out, const ReadOptions *options,
        int64_t targetSampleTimeUs) {
    MediaBufferHelper *frame = *mPendingFrames.begin();
    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {
        CHECK(AMediaFormat_getInt64(frame->meta_data(),
                    AMEDIAFORMAT_KEY_TIME_US, &mCurrentTS));
        if (mCurrentTS < 0) {
            mCurrentTS = 0;
            AMediaFormat_setInt64(frame->meta_data(),
                    AMEDIAFORMAT_KEY_TIME_US, mCurrentTS);
        }
    }

    int32_t start = -1;
    while (start < 0) {
        //find header start position
        findMP3Header(&mMP3Header,
            (const uint8_t*)frame->data() + frame->range_offset(),
            frame->range_length(), &start);
        ALOGV("start=%d, frame->range_length() = %zu, frame->range_offset() =%zu",
                      start, frame->range_length(), frame->range_offset());
        if (start >= 0)
            break;
        frame->release();
        mPendingFrames.erase(mPendingFrames.begin());
        while (mPendingFrames.empty()) {
            media_status_t err = readBlock();
            if (err != OK) {
                clearPendingFrames();
                return err;
            }
        }
        frame = *mPendingFrames.begin();
    }

    frame->set_range(frame->range_offset() + start, frame->range_length() - start);

    uint32_t header = *(uint32_t*)((uint8_t*)frame->data() + frame->range_offset());
    header = ((header >> 24) & 0xff) | ((header >> 8) & 0xff00) |
                    ((header << 8) & 0xff0000) | ((header << 24) & 0xff000000);
    size_t frame_size;
    int out_sampling_rate;
    int out_channels;
    int out_bitrate;
    if (!GetMPEGAudioFrameSize(header, &frame_size,
                               &out_sampling_rate, &out_channels, &out_bitrate)) {
        ALOGE("MP3 Header read fail!!");
        return AMEDIA_ERROR_UNSUPPORTED;
    }

    MediaBufferHelper *buffer;
    mBufferGroup->acquire_buffer(&buffer, false /* nonblocking */, frame_size /* requested size */);
    buffer->set_range(0, frame_size);

    uint8_t *data = static_cast<uint8_t *>(buffer->data());
    ALOGV("MP3 frame %zu frame->range_length() %zu", frame_size, frame->range_length());

    if (frame_size > frame->range_length()) {
        memcpy(data, (uint8_t*)(frame->data()) + frame->range_offset(), frame->range_length());
        size_t sumSize = 0;
        sumSize += frame->range_length();
        size_t needSize = frame_size - frame->range_length();
        frame->release();
        mPendingFrames.erase(mPendingFrames.begin());
        while (mPendingFrames.empty()) {
            media_status_t err = readBlock();
            if (err != OK) {
                clearPendingFrames();
                return err;
            }
        }
        frame = *mPendingFrames.begin();
        size_t offset = frame->range_offset();
        size_t size = frame->range_length();

        // the next buffer frame is not enough to fullfill mp3 frame,
        // we have to read until mp3 frame is completed.
        while (size < needSize) {
            memcpy(data + sumSize, (uint8_t*)(frame->data()) + offset, size);
            needSize -= size;
            sumSize += size;
            frame->release();
            mPendingFrames.erase(mPendingFrames.begin());
            while (mPendingFrames.empty()) {
                media_status_t err = readBlock();
                if (err != OK) {
                    clearPendingFrames();
                    return err;
                }
            }
            frame = *mPendingFrames.begin();
            offset = frame->range_offset();
            size = frame->range_length();
        }
        memcpy(data + sumSize, (uint8_t*)(frame->data()) + offset, needSize);
        frame->set_range(offset + needSize, size - needSize);
     } else {
        size_t offset = frame->range_offset();
        size_t size = frame->range_length();
        memcpy(data, (uint8_t*)(frame->data()) + offset, frame_size);
        frame->set_range(offset + frame_size, size - frame_size);
    }
    if (frame->range_length() < 4) {
        frame->release();
        frame = NULL;
        mPendingFrames.erase(mPendingFrames.begin());
    }
    ALOGV("MatroskaSource::read MP3 frame kKeyTime=%lld,kKeyTargetTime=%lld",
                    (long long)mCurrentTS, (long long)targetSampleTimeUs);
    AMediaFormat_setInt64(buffer->meta_data(),
            AMEDIAFORMAT_KEY_TIME_US, mCurrentTS);
    mCurrentTS += (int64_t)frame_size * 8000ll / out_bitrate;

    if (targetSampleTimeUs >= 0ll)
        AMediaFormat_setInt64(buffer->meta_data(),
                AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
    *out = buffer;
    ALOGV("MatroskaSource::read MP3, keyTime=%lld for next frame", (long long)mCurrentTS);
    return AMEDIA_OK;
}

media_status_t MatroskaSource::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    *out = NULL;

    int64_t targetSampleTimeUs = -1ll;

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {
        if (mode == ReadOptions::SEEK_FRAME_INDEX) {
            return AMEDIA_ERROR_UNSUPPORTED;
        }

        if (!mExtractor->isLiveStreaming()) {
            clearPendingFrames();

            // The audio we want is located by using the Cues to seek the video
            // stream to find the target Cluster then iterating to finalize for
            // audio.
            int64_t actualFrameTimeUs;
            mBlockIter.seek(seekTimeUs, mIsAudio, &actualFrameTimeUs);
            if (mode == ReadOptions::SEEK_CLOSEST) {
                targetSampleTimeUs = actualFrameTimeUs;
            }
        }
    }

    while (mPendingFrames.empty()) {
        media_status_t err = readBlock();

        if (err != OK) {
            clearPendingFrames();

            return err;
        }
    }

    if (mType == MP3) {
        return mp3FrameRead(out, options, targetSampleTimeUs);
    }

    MediaBufferHelper *frame = *mPendingFrames.begin();
    mPendingFrames.erase(mPendingFrames.begin());

    if ((mType != AVC && mType != HEVC) || mNALSizeLen == 0) {
        if (targetSampleTimeUs >= 0ll) {
            AMediaFormat_setInt64(frame->meta_data(),
                    AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
        }

        if (mType == PCM) {
            int32_t bitPerFrame = 16;
            int32_t bigEndian = 0;
            AMediaFormat *meta = AMediaFormat_new();
            if (getFormat(meta) == AMEDIA_OK && meta != NULL) {
                AMediaFormat_getInt32(meta,
                                AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, &bitPerFrame);
                AMediaFormat_getInt32(meta,
                                AMEDIAFORMAT_KEY_PCM_BIG_ENDIAN, &bigEndian);
            }
            AMediaFormat_delete(meta);
            if (bigEndian == 1 && bitPerFrame == 16) {
                // Big-endian -> little-endian
                uint16_t *dstData = (uint16_t *)frame->data() + frame->range_offset();
                uint16_t *srcData = (uint16_t *)frame->data() + frame->range_offset();
                for (size_t i = 0; i < frame->range_length() / 2; i++) {
                    dstData[i] = ntohs(srcData[i]);
                }
            }
        }

        *out = frame;

        return AMEDIA_OK;
    }

    // Each input frame contains one or more NAL fragments, each fragment
    // is prefixed by mNALSizeLen bytes giving the fragment length,
    // followed by a corresponding number of bytes containing the fragment.
    // We output all these fragments into a single large buffer separated
    // by startcodes (0x00 0x00 0x00 0x01).
    //
    // When mNALSizeLen is 0, we assume the data is already in the format
    // desired.

    const uint8_t *srcPtr =
        (const uint8_t *)frame->data() + frame->range_offset();

    size_t srcSize = frame->range_length();

    size_t dstSize = 0;
    MediaBufferHelper *buffer = NULL;
    uint8_t *dstPtr = NULL;

    for (int32_t pass = 0; pass < 2; ++pass) {
        size_t srcOffset = 0;
        size_t dstOffset = 0;
        while (srcOffset + mNALSizeLen <= srcSize) {
            size_t NALsize;
            switch (mNALSizeLen) {
                case 1: NALsize = srcPtr[srcOffset]; break;
                case 2: NALsize = U16_AT(srcPtr + srcOffset); break;
                case 3: NALsize = U24_AT(srcPtr + srcOffset); break;
                case 4: NALsize = U32_AT(srcPtr + srcOffset); break;
                default:
                    TRESPASS();
            }

            if (srcOffset + mNALSizeLen + NALsize <= srcOffset + mNALSizeLen) {
                frame->release();
                frame = NULL;

                return AMEDIA_ERROR_MALFORMED;
            } else if (srcOffset + mNALSizeLen + NALsize > srcSize) {
                break;
            }

            if (pass == 1) {
                memcpy(&dstPtr[dstOffset], "\x00\x00\x00\x01", 4);

                if (frame != buffer) {
                    memcpy(&dstPtr[dstOffset + 4],
                           &srcPtr[srcOffset + mNALSizeLen],
                           NALsize);
                }
            }

            dstOffset += 4;  // 0x00 00 00 01
            dstOffset += NALsize;

            srcOffset += mNALSizeLen + NALsize;
        }

        if (srcOffset < srcSize) {
            // There were trailing bytes or not enough data to complete
            // a fragment.

            frame->release();
            frame = NULL;

            return AMEDIA_ERROR_MALFORMED;
        }

        if (pass == 0) {
            dstSize = dstOffset;

            if (dstSize == srcSize && mNALSizeLen == 4) {
                // In this special case we can re-use the input buffer by substituting
                // each 4-byte nal size with a 4-byte start code
                buffer = frame;
            } else {
                mBufferGroup->acquire_buffer(
                        &buffer, false /* nonblocking */, dstSize /* requested size */);
                buffer->set_range(0, dstSize);
            }

            AMediaFormat *frameMeta = frame->meta_data();
            int64_t timeUs;
            CHECK(AMediaFormat_getInt64(frameMeta, AMEDIAFORMAT_KEY_TIME_US, &timeUs));
            int32_t isSync;
            CHECK(AMediaFormat_getInt32(frameMeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, &isSync));

            AMediaFormat *bufMeta = buffer->meta_data();
            AMediaFormat_setInt64(bufMeta, AMEDIAFORMAT_KEY_TIME_US, timeUs);
            AMediaFormat_setInt32(bufMeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, isSync);

            dstPtr = (uint8_t *)buffer->data();
        }
    }

    if (frame != buffer) {
        frame->release();
        frame = NULL;
    }

    if (targetSampleTimeUs >= 0ll) {
        AMediaFormat_setInt64(buffer->meta_data(),
                AMEDIAFORMAT_KEY_TARGET_TIME, targetSampleTimeUs);
    }

    *out = buffer;

    return AMEDIA_OK;
}

////////////////////////////////////////////////////////////////////////////////

enum WaveID {
    MKV_RIFF_WAVE_FORMAT_PCM = 0x0001,
    MKV_RIFF_WAVE_FORMAT_ADPCM_ms = 0x0002,
    MKV_RIFF_WAVE_FORMAT_ADPCM_ima_wav = 0x0011,
    MKV_RIFF_WAVE_FORMAT_MPEGL12 = 0x0050,
    MKV_RIFF_WAVE_FORMAT_MPEGL3 = 0x0055,
    MKV_RIFF_WAVE_FORMAT_WMAV1 = 0x0160,
    MKV_RIFF_WAVE_FORMAT_WMAV2 = 0x0161,
};

static const char *MKVWave2MIME(uint16_t id) {
    switch (id) {
        case  MKV_RIFF_WAVE_FORMAT_MPEGL12:
            return MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II;

        case  MKV_RIFF_WAVE_FORMAT_MPEGL3:
            return MEDIA_MIMETYPE_AUDIO_MPEG;

        case MKV_RIFF_WAVE_FORMAT_PCM:
            return MEDIA_MIMETYPE_AUDIO_RAW;

        case MKV_RIFF_WAVE_FORMAT_ADPCM_ms:
            return MEDIA_MIMETYPE_AUDIO_MS_ADPCM;
        case MKV_RIFF_WAVE_FORMAT_ADPCM_ima_wav:
            return MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM;

        case MKV_RIFF_WAVE_FORMAT_WMAV1:
        case MKV_RIFF_WAVE_FORMAT_WMAV2:
            return MEDIA_MIMETYPE_AUDIO_WMA;
        default:
            ALOGW("unknown wave %x", id);
            return "";
    };
}

static bool isMkvAudioCsdSizeOK(const char* mime, size_t csdSize) {
    if ((!strcmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM) && csdSize < 50) ||
        (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM) && csdSize < 20) ||
        (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_WMA) && csdSize < 28) ||
        (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG) && csdSize < 30)) {
        return false;
    }
    return true;
}

// trans all FOURCC  to lower char
static uint32_t FourCCtoLower(uint32_t fourcc) {
    uint8_t ch_1 = tolower((fourcc >> 24) & 0xff);
    uint8_t ch_2 = tolower((fourcc >> 16) & 0xff);
    uint8_t ch_3 = tolower((fourcc >> 8) & 0xff);
    uint8_t ch_4 = tolower((fourcc) & 0xff);
    uint32_t fourcc_out = ch_1 << 24 | ch_2 << 16 | ch_3 << 8 | ch_4;

    return fourcc_out;
}

static const char *MKVFourCC2MIME(uint32_t fourcc) {
    ALOGV("MKVFourCC2MIME fourcc 0x%8.8x", fourcc);
    uint32_t lowerFourcc = FourCCtoLower(fourcc);
    switch (lowerFourcc) {
        case FOURCC("mp4v"):
            return MEDIA_MIMETYPE_VIDEO_MPEG4;

        case FOURCC("s263"):
        case FOURCC("h263"):
            return MEDIA_MIMETYPE_VIDEO_H263;

        case FOURCC("avc1"):
        case FOURCC("h264"):
            return MEDIA_MIMETYPE_VIDEO_AVC;

        case FOURCC("mpg2"):
            return MEDIA_MIMETYPE_VIDEO_MPEG2;

        case FOURCC("xvid"):
            return MEDIA_MIMETYPE_VIDEO_XVID;

        case FOURCC("divx"):
        case FOURCC("dx50"):
            return MEDIA_MIMETYPE_VIDEO_DIVX;

        case FOURCC("div3"):
        case FOURCC("div4"):
            return MEDIA_MIMETYPE_VIDEO_DIVX3;

        case FOURCC("mjpg"):
        case FOURCC("mppg"):
            return MEDIA_MIMETYPE_VIDEO_MJPEG;

        default:
            char fourccString[5];
            MakeFourCCString(fourcc, fourccString);
            ALOGW("mkv unsupport fourcc %s", fourccString);
            return "";
    }
}


MatroskaExtractor::MatroskaExtractor(DataSourceHelper *source)
    : mDataSource(source),
      mReader(new DataSourceBaseReader(mDataSource)),
      mSegment(NULL),
      mExtractedThumbnails(false),
      mIsWebm(false),
      mSeekPreRollNs(0) {
    off64_t size;
    mIsLiveStreaming =
        (mDataSource->flags()
            & (DataSourceBase::kWantsPrefetching
                | DataSourceBase::kIsCachingDataSource))
        && mDataSource->getSize(&size) != OK;

    mkvparser::EBMLHeader ebmlHeader;
    long long pos;
    if (ebmlHeader.Parse(mReader, pos) < 0) {
        return;
    }

    if (ebmlHeader.m_docType && !strcmp("webm", ebmlHeader.m_docType)) {
        mIsWebm = true;
    }

    long long ret =
        mkvparser::Segment::CreateInstance(mReader, pos, mSegment);

    if (ret) {
        CHECK(mSegment == NULL);
        return;
    }

    if (mIsLiveStreaming) {
        // from mkvparser::Segment::Load(), but stop at first cluster
        ret = mSegment->ParseHeaders();
        if (ret == 0) {
            long len;
            ret = mSegment->LoadCluster(pos, len);
            if (ret >= 1) {
                // no more clusters
                ret = 0;
            }
        } else if (ret > 0) {
            ret = mkvparser::E_BUFFER_NOT_FULL;
        }
    } else {
        ret = mSegment->ParseHeaders();
        if (ret < 0) {
            ALOGE("Segment parse header return fail %lld", ret);
            delete mSegment;
            mSegment = NULL;
            return;
        } else if (ret == 0) {
            const mkvparser::Cues* mCues = mSegment->GetCues();
            const mkvparser::SeekHead* mSH = mSegment->GetSeekHead();
            if ((mCues == NULL) && (mSH != NULL)) {
                size_t count = mSH->GetCount();
                const mkvparser::SeekHead::Entry* mEntry;
                for (size_t index = 0; index < count; index++) {
                    mEntry = mSH->GetEntry(index);
                    if (mEntry->id == libwebm::kMkvCues) { // Cues ID
                        long len;
                        long long pos;
                        mSegment->ParseCues(mEntry->pos, pos, len);
                        mCues = mSegment->GetCues();
                        ALOGV("find cue data by seekhead");
                        break;
                    }
                }
            }

            if (mCues) {
                long len;
                ret = mSegment->LoadCluster(pos, len);
                ALOGV("has Cue data, Cluster num=%ld", mSegment->GetCount());
            } else  {
                long status_Load = mSegment->Load();
                ALOGW("no Cue data,Segment Load status:%ld",status_Load);
            }
        } else if (ret > 0) {
            ret = mkvparser::E_BUFFER_NOT_FULL;
        }
    }

    if (ret < 0) {
        char uri[1024];
        if(!mDataSource->getUri(uri, sizeof(uri))) {
            uri[0] = '\0';
        }
        ALOGW("Corrupt %s source: %s", mIsWebm ? "webm" : "matroska",
                uriDebugString(uri).c_str());
        delete mSegment;
        mSegment = NULL;
        return;
    }

#if 0
    const mkvparser::SegmentInfo *info = mSegment->GetInfo();
    ALOGI("muxing app: %s, writing app: %s",
         info->GetMuxingAppAsUTF8(),
         info->GetWritingAppAsUTF8());
#endif

    addTracks();
}

MatroskaExtractor::~MatroskaExtractor() {
    delete mSegment;
    mSegment = NULL;

    delete mReader;
    mReader = NULL;

    delete mDataSource;

    for (size_t i = 0; i < mTracks.size(); ++i) {
        TrackInfo *info = &mTracks.editItemAt(i);
        if (info->mMeta) {
            AMediaFormat_delete(info->mMeta);
        }
    }
}

size_t MatroskaExtractor::countTracks() {
    return mTracks.size();
}

MediaTrackHelper *MatroskaExtractor::getTrack(size_t index) {
    if (index >= mTracks.size()) {
        return NULL;
    }

    return new MatroskaSource(this, index);
}

media_status_t MatroskaExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t flags) {
    if (index >= mTracks.size()) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    if ((flags & kIncludeExtensiveMetaData) && !mExtractedThumbnails
            && !isLiveStreaming()) {
        findThumbnails();
        mExtractedThumbnails = true;
    }

    return AMediaFormat_copy(meta, mTracks.itemAt(index).mMeta);
}

bool MatroskaExtractor::isLiveStreaming() const {
    return mIsLiveStreaming;
}

static int bytesForSize(size_t size) {
    // use at most 28 bits (4 times 7)
    CHECK(size <= 0xfffffff);

    if (size > 0x1fffff) {
        return 4;
    } else if (size > 0x3fff) {
        return 3;
    } else if (size > 0x7f) {
        return 2;
    }
    return 1;
}

static void storeSize(uint8_t *data, size_t &idx, size_t size) {
    int numBytes = bytesForSize(size);
    idx += numBytes;

    data += idx;
    size_t next = 0;
    while (numBytes--) {
        *--data = (size & 0x7f) | next;
        size >>= 7;
        next = 0x80;
    }
}

static void addESDSFromCodecPrivate(
        AMediaFormat *meta,
        bool isAudio, const void *priv, size_t privSize) {

    int privSizeBytesRequired = bytesForSize(privSize);
    int esdsSize2 = 14 + privSizeBytesRequired + privSize;
    int esdsSize2BytesRequired = bytesForSize(esdsSize2);
    int esdsSize1 = 4 + esdsSize2BytesRequired + esdsSize2;
    int esdsSize1BytesRequired = bytesForSize(esdsSize1);
    size_t esdsSize = 1 + esdsSize1BytesRequired + esdsSize1;
    uint8_t *esds = new uint8_t[esdsSize];

    size_t idx = 0;
    esds[idx++] = 0x03;
    storeSize(esds, idx, esdsSize1);
    esds[idx++] = 0x00; // ES_ID
    esds[idx++] = 0x00; // ES_ID
    esds[idx++] = 0x00; // streamDependenceFlag, URL_Flag, OCRstreamFlag
    esds[idx++] = 0x04;
    storeSize(esds, idx, esdsSize2);
    esds[idx++] = isAudio ? 0x40   // Audio ISO/IEC 14496-3
                          : 0x20;  // Visual ISO/IEC 14496-2
    for (int i = 0; i < 12; i++) {
        esds[idx++] = 0x00;
    }
    esds[idx++] = 0x05;
    storeSize(esds, idx, privSize);
    memcpy(esds + idx, priv, privSize);

    AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_0, priv, privSize);

    delete[] esds;
    esds = NULL;
}

status_t addVorbisCodecInfo(
        AMediaFormat *meta,
        const void *_codecPrivate, size_t codecPrivateSize) {
    // hexdump(_codecPrivate, codecPrivateSize);

    if (codecPrivateSize < 1) {
        return ERROR_MALFORMED;
    }

    const uint8_t *codecPrivate = (const uint8_t *)_codecPrivate;

    if (codecPrivate[0] != 0x02) {
        return ERROR_MALFORMED;
    }

    // codecInfo starts with two lengths, len1 and len2, that are
    // "Xiph-style-lacing encoded"...

    size_t offset = 1;
    size_t len1 = 0;
    while (offset < codecPrivateSize && codecPrivate[offset] == 0xff) {
        if (len1 > (SIZE_MAX - 0xff)) {
            return ERROR_MALFORMED; // would overflow
        }
        len1 += 0xff;
        ++offset;
    }
    if (offset >= codecPrivateSize) {
        return ERROR_MALFORMED;
    }
    if (len1 > (SIZE_MAX - codecPrivate[offset])) {
        return ERROR_MALFORMED; // would overflow
    }
    len1 += codecPrivate[offset++];

    size_t len2 = 0;
    while (offset < codecPrivateSize && codecPrivate[offset] == 0xff) {
        if (len2 > (SIZE_MAX - 0xff)) {
            return ERROR_MALFORMED; // would overflow
        }
        len2 += 0xff;
        ++offset;
    }
    if (offset >= codecPrivateSize) {
        return ERROR_MALFORMED;
    }
    if (len2 > (SIZE_MAX - codecPrivate[offset])) {
        return ERROR_MALFORMED; // would overflow
    }
    len2 += codecPrivate[offset++];

    if (len1 > SIZE_MAX - len2 || offset > SIZE_MAX - (len1 + len2) ||
            codecPrivateSize < offset + len1 + len2) {
        return ERROR_MALFORMED;
    }

    if (codecPrivate[offset] != 0x01) {
        return ERROR_MALFORMED;
    }
    // formerly kKeyVorbisInfo
    AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_0, &codecPrivate[offset], len1);

    offset += len1;
    if (codecPrivate[offset] != 0x03) {
        return ERROR_MALFORMED;
    }

    offset += len2;
    if (codecPrivate[offset] != 0x05) {
        return ERROR_MALFORMED;
    }

    // formerly kKeyVorbisBooks
    AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_1,
            &codecPrivate[offset], codecPrivateSize - offset);

    return OK;
}

static status_t addFlacMetadata(
        AMediaFormat *meta,
        const void *codecPrivate, size_t codecPrivateSize) {
    // hexdump(codecPrivate, codecPrivateSize);

    // formerly kKeyFlacMetadata
    AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_0, codecPrivate, codecPrivateSize);

    int32_t maxInputSize = 64 << 10;
    FLACDecoder *flacDecoder = FLACDecoder::Create();
    if (flacDecoder != NULL
            && flacDecoder->parseMetadata((const uint8_t*)codecPrivate, codecPrivateSize) == OK) {
        FLAC__StreamMetadata_StreamInfo streamInfo = flacDecoder->getStreamInfo();
        maxInputSize = streamInfo.max_framesize;
        if (maxInputSize == 0) {
            // In case max framesize is not available, use raw data size as max framesize,
            // assuming there is no expansion.
            if (streamInfo.max_blocksize != 0
                    && streamInfo.channels != 0
                    && ((streamInfo.bits_per_sample + 7) / 8) >
                        INT32_MAX / streamInfo.max_blocksize / streamInfo.channels) {
                delete flacDecoder;
                return ERROR_MALFORMED;
            }
            maxInputSize = ((streamInfo.bits_per_sample + 7) / 8)
                * streamInfo.max_blocksize * streamInfo.channels;
        }
    }
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, maxInputSize);

    delete flacDecoder;
    return OK;
}

status_t MatroskaExtractor::synthesizeAVCC(TrackInfo *trackInfo, size_t index) {
    BlockIterator iter(this, trackInfo->mTrackNum, index);
    if (iter.eos()) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block *block = iter.block();
    if (block->GetFrameCount() <= 0) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block::Frame &frame = block->GetFrame(0);
    auto tmpData = heapbuffer<unsigned char>(frame.len);
    long n = frame.Read(mReader, tmpData.get());
    if (n != 0) {
        return ERROR_MALFORMED;
    }

    if (!MakeAVCCodecSpecificData(trackInfo->mMeta, tmpData.get(), frame.len)) {
        return ERROR_MALFORMED;
    }

    // Override the synthesized nal length size, which is arbitrary
    trackInfo->mNalLengthSize = 0;
    return OK;
}

status_t MatroskaExtractor::synthesizeMPEG2(TrackInfo *trackInfo, size_t index) {
    ALOGV("synthesizeMPEG2");
    BlockIterator iter(this, trackInfo->mTrackNum, index);
    if (iter.eos()) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block *block = iter.block();
    if (block->GetFrameCount() <= 0) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block::Frame &frame = block->GetFrame(0);
    auto tmpData = heapbuffer<unsigned char>(frame.len);
    long n = frame.Read(mReader, tmpData.get());
    if (n != 0) {
        return ERROR_MALFORMED;
    }

    size_t header_start = 0;
    size_t header_lenth = 0;
    for (header_start = 0; header_start < frame.len - 4; header_start++) {
        if (ntohl(0x000001b3) == *(uint32_t*)((uint8_t*)tmpData.get() + header_start)) {
            break;
        }
    }
    bool isComplete_csd = false;
    for (header_lenth = 0; header_lenth < frame.len - 4 - header_start; header_lenth++) {
        if (ntohl(0x000001b8) == *(uint32_t*)((uint8_t*)tmpData.get()
                                + header_start + header_lenth)) {
            isComplete_csd = true;
            break;
        }
    }
    if (!isComplete_csd) {
        ALOGE("can't parse complete csd for MPEG2!");
        return ERROR_MALFORMED;
    }
    addESDSFromCodecPrivate(trackInfo->mMeta, false,
                              (uint8_t*)(tmpData.get()) + header_start, header_lenth);

    return OK;

}

status_t MatroskaExtractor::synthesizeMPEG4(TrackInfo *trackInfo, size_t index) {
    ALOGV("synthesizeMPEG4");
    BlockIterator iter(this, trackInfo->mTrackNum, index);
    if (iter.eos()) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block *block = iter.block();
    if (block->GetFrameCount() <= 0) {
        return ERROR_MALFORMED;
    }

    const mkvparser::Block::Frame &frame = block->GetFrame(0);
    auto tmpData = heapbuffer<unsigned char>(frame.len);
    long n = frame.Read(mReader, tmpData.get());
    if (n != 0) {
        return ERROR_MALFORMED;
    }

     size_t vosend;
     bool isComplete_csd = false;
     for (vosend = 0; (long)vosend < frame.len - 4; vosend++) {
         if (ntohl(0x000001b6) == *(uint32_t*)((uint8_t*)tmpData.get() + vosend)) {
             isComplete_csd = true;
             break;  // Send VOS until VOP
         }
     }
     if (!isComplete_csd) {
         ALOGE("can't parse complete csd for MPEG4!");
         return ERROR_MALFORMED;
     }
     addESDSFromCodecPrivate(trackInfo->mMeta, false, tmpData.get(), vosend);

    return OK;

}


static inline bool isValidInt32ColourValue(long long value) {
    return value != mkvparser::Colour::kValueNotPresent
            && value >= INT32_MIN
            && value <= INT32_MAX;
}

static inline bool isValidUint16ColourValue(long long value) {
    return value != mkvparser::Colour::kValueNotPresent
            && value >= 0
            && value <= UINT16_MAX;
}

static inline bool isValidPrimary(const mkvparser::PrimaryChromaticity *primary) {
    return primary != NULL && primary->x >= 0 && primary->x <= 1
             && primary->y >= 0 && primary->y <= 1;
}

void MatroskaExtractor::getColorInformation(
        const mkvparser::VideoTrack *vtrack, AMediaFormat *meta) {
    const mkvparser::Colour *color = vtrack->GetColour();
    if (color == NULL) {
        return;
    }

    // Color Aspects
    {
        int32_t primaries = 2; // ISO unspecified
        int32_t isotransfer = 2; // ISO unspecified
        int32_t coeffs = 2; // ISO unspecified
        bool fullRange = false; // default
        bool rangeSpecified = false;

        if (isValidInt32ColourValue(color->primaries)) {
            primaries = color->primaries;
        }
        if (isValidInt32ColourValue(color->transfer_characteristics)) {
            isotransfer = color->transfer_characteristics;
        }
        if (isValidInt32ColourValue(color->matrix_coefficients)) {
            coeffs = color->matrix_coefficients;
        }
        if (color->range != mkvparser::Colour::kValueNotPresent
                && color->range != 0 /* MKV unspecified */) {
            // We only support MKV broadcast range (== limited) and full range.
            // We treat all other value as the default limited range.
            fullRange = color->range == 2 /* MKV fullRange */;
            rangeSpecified = true;
        }

        int32_t range = 0;
        int32_t standard = 0;
        int32_t transfer = 0;
        ColorUtils::convertIsoColorAspectsToPlatformAspects(
                primaries, isotransfer, coeffs, fullRange,
                &range, &standard, &transfer);
        if (range != 0) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_COLOR_RANGE, range);
        }
        if (standard != 0) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_COLOR_STANDARD, standard);
        }
        if (transfer != 0) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_COLOR_TRANSFER, transfer);
        }
    }

    // HDR Static Info
    {
        HDRStaticInfo info, nullInfo; // nullInfo is a fully unspecified static info
        memset(&info, 0, sizeof(info));
        memset(&nullInfo, 0, sizeof(nullInfo));
        if (isValidUint16ColourValue(color->max_cll)) {
            info.sType1.mMaxContentLightLevel = color->max_cll;
        }
        if (isValidUint16ColourValue(color->max_fall)) {
            info.sType1.mMaxFrameAverageLightLevel = color->max_fall;
        }
        const mkvparser::MasteringMetadata *mastering = color->mastering_metadata;
        if (mastering != NULL) {
            // Convert matroska values to HDRStaticInfo equivalent values for each fully specified
            // group. See CTA-681.3 section 3.2.1 for more info.
            if (mastering->luminance_max >= 0.5 && mastering->luminance_max < 65535.5) {
                info.sType1.mMaxDisplayLuminance = (uint16_t)(mastering->luminance_max + 0.5);
            }
            if (mastering->luminance_min >= 0.00005 && mastering->luminance_min < 6.55355) {
                // HDRStaticInfo Type1 stores min luminance scaled 10000:1
                info.sType1.mMinDisplayLuminance =
                    (uint16_t)(10000 * mastering->luminance_min + 0.5);
            }
            // HDRStaticInfo Type1 stores primaries scaled 50000:1
            if (isValidPrimary(mastering->white_point)) {
                info.sType1.mW.x = (uint16_t)(50000 * mastering->white_point->x + 0.5);
                info.sType1.mW.y = (uint16_t)(50000 * mastering->white_point->y + 0.5);
            }
            if (isValidPrimary(mastering->r) && isValidPrimary(mastering->g)
                    && isValidPrimary(mastering->b)) {
                info.sType1.mR.x = (uint16_t)(50000 * mastering->r->x + 0.5);
                info.sType1.mR.y = (uint16_t)(50000 * mastering->r->y + 0.5);
                info.sType1.mG.x = (uint16_t)(50000 * mastering->g->x + 0.5);
                info.sType1.mG.y = (uint16_t)(50000 * mastering->g->y + 0.5);
                info.sType1.mB.x = (uint16_t)(50000 * mastering->b->x + 0.5);
                info.sType1.mB.y = (uint16_t)(50000 * mastering->b->y + 0.5);
            }
        }
        // Only advertise static info if at least one of the groups have been specified.
        if (memcmp(&info, &nullInfo, sizeof(info)) != 0) {
            info.mID = HDRStaticInfo::kType1;
            ColorUtils::setHDRStaticInfoIntoAMediaFormat(info, meta);
        }
    }
}

status_t MatroskaExtractor::initTrackInfo(
        const mkvparser::Track *track, AMediaFormat *meta, TrackInfo *trackInfo) {
    trackInfo->mTrackNum = track->GetNumber();
    trackInfo->mMeta = meta;
    trackInfo->mExtractor = this;
    trackInfo->mEncrypted = false;
    trackInfo->mHeader = NULL;
    trackInfo->mHeaderLen = 0;
    trackInfo->mNalLengthSize = -1;

    for(size_t i = 0; i < track->GetContentEncodingCount(); i++) {
        const mkvparser::ContentEncoding *encoding = track->GetContentEncodingByIndex(i);
        for(size_t j = 0; j < encoding->GetEncryptionCount(); j++) {
            const mkvparser::ContentEncoding::ContentEncryption *encryption;
            encryption = encoding->GetEncryptionByIndex(j);
            AMediaFormat_setBuffer(trackInfo->mMeta,
                    AMEDIAFORMAT_KEY_CRYPTO_KEY, encryption->key_id, encryption->key_id_len);
            trackInfo->mEncrypted = true;
            break;
        }

        for(size_t j = 0; j < encoding->GetCompressionCount(); j++) {
            const mkvparser::ContentEncoding::ContentCompression *compression;
            compression = encoding->GetCompressionByIndex(j);
            ALOGV("compression algo %llu settings_len %lld",
                compression->algo, compression->settings_len);
            if (compression->algo == 3
                    && compression->settings
                    && compression->settings_len > 0) {
                trackInfo->mHeader = compression->settings;
                trackInfo->mHeaderLen = compression->settings_len;
            }
        }
    }

    return OK;
}

void MatroskaExtractor::addTracks() {
    const mkvparser::Tracks *tracks = mSegment->GetTracks();

    AMediaFormat *meta = nullptr;

    for (size_t index = 0; index < tracks->GetTracksCount(); ++index) {
        const mkvparser::Track *track = tracks->GetTrackByIndex(index);

        if (track == NULL) {
            // Apparently this is currently valid (if unexpected) behaviour
            // of the mkv parser lib.
            continue;
        }

        const char *const codecID = track->GetCodecId();
        ALOGV("codec id = %s", codecID);
        ALOGV("codec name = %s", track->GetCodecNameAsUTF8());

        if (codecID == NULL) {
            ALOGW("unknown codecID is not supported.");
            continue;
        }

        size_t codecPrivateSize;
        const unsigned char *codecPrivate =
            track->GetCodecPrivate(codecPrivateSize);

        enum { VIDEO_TRACK = 1, AUDIO_TRACK = 2 };

        if (meta) {
            AMediaFormat_clear(meta);
        } else {
            meta = AMediaFormat_new();
        }

        status_t err = OK;
        int32_t nalSize = -1;

        bool isSetCsdFrom1stFrame = false;

        switch (track->GetType()) {
            case VIDEO_TRACK:
            {
                const mkvparser::VideoTrack *vtrack =
                    static_cast<const mkvparser::VideoTrack *>(track);

                if (!strcmp("V_MPEG4/ISO/AVC", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_AVC);
                    AMediaFormat_setBuffer(meta,
                           AMEDIAFORMAT_KEY_CSD_AVC, codecPrivate, codecPrivateSize);
                    if (codecPrivateSize > 4) {
                        nalSize = 1 + (codecPrivate[4] & 3);
                    }
                } else if (!strcmp("V_MPEGH/ISO/HEVC", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_HEVC);
                    if (codecPrivateSize > 0) {
                        AMediaFormat_setBuffer(meta,
                               AMEDIAFORMAT_KEY_CSD_HEVC, codecPrivate, codecPrivateSize);
                        if (codecPrivateSize > 14 + 7) {
                            nalSize = 1 + (codecPrivate[14 + 7] & 3);
                        }
                    } else {
                        ALOGW("HEVC is detected, but does not have configuration.");
                        continue;
                    }
                } else if (!strcmp("V_MPEG4/ISO/ASP", codecID)) {
                    AMediaFormat_setString(meta,
                            AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_MPEG4);
                    if (codecPrivateSize > 0) {
                        addESDSFromCodecPrivate(
                                meta, false, codecPrivate, codecPrivateSize);
                    } else {
                        ALOGW("%s is detected, but does not have configuration.",
                                codecID);
                        isSetCsdFrom1stFrame = true;
                    }
                } else if (!strcmp("V_VP8", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_VP8);
                } else if (!strcmp("V_VP9", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_VP9);
                    if (codecPrivateSize > 0) {
                      // 'csd-0' for VP9 is the Blob of Codec Private data as
                      // specified in http://www.webmproject.org/vp9/profiles/.
                      AMediaFormat_setBuffer(meta,
                             AMEDIAFORMAT_KEY_CSD_0, codecPrivate, codecPrivateSize);
                    }
                } else if (!strcmp("V_AV1", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_VIDEO_AV1);
                    if (codecPrivateSize > 0) {
                        // 'csd-0' for AV1 is the Blob of Codec Private data as
                        // specified in https://aomediacodec.github.io/av1-isobmff/.
                        AMediaFormat_setBuffer(
                                meta, AMEDIAFORMAT_KEY_CSD_0, codecPrivate, codecPrivateSize);
                    }
                } else if (!strcmp("V_MPEG2", codecID) || !strcmp("V_MPEG1", codecID)) {
                        AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME,
                                MEDIA_MIMETYPE_VIDEO_MPEG2);
                        if (codecPrivate != NULL) {
                            addESDSFromCodecPrivate(meta, false, codecPrivate, codecPrivateSize);
                        } else {
                            ALOGW("No specific codec private data, find it from the first frame");
                            isSetCsdFrom1stFrame = true;
                        }
                } else if (!strcmp("V_MJPEG", codecID)) {
                        AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME,
                                MEDIA_MIMETYPE_VIDEO_MJPEG);
                } else if (!strcmp("V_MS/VFW/FOURCC", codecID)) {
                    if (NULL == codecPrivate ||codecPrivateSize < 20) {
                        ALOGE("V_MS/VFW/FOURCC has no valid private data(%p),codecPrivateSize:%zu",
                                 codecPrivate, codecPrivateSize);
                        continue;
                    } else {
                        uint32_t fourcc = *(uint32_t *)(codecPrivate + 16);
                        fourcc = ntohl(fourcc);
                        const char* mime = MKVFourCC2MIME(fourcc);
                        ALOGV("V_MS/VFW/FOURCC type is %s", mime);
                        if (!strncasecmp("video/", mime, 6)) {
                            AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, mime);
                        } else {
                            ALOGE("V_MS/VFW/FOURCC continue,unsupport video type=%s,fourcc=0x%08x.",
                                 mime, fourcc);
                            continue;
                        }
                        if (!strcmp(mime, MEDIA_MIMETYPE_VIDEO_AVC) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_XVID) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG2) ||
                            !strcmp(mime, MEDIA_MIMETYPE_VIDEO_H263)) {
                            isSetCsdFrom1stFrame = true;
                        } else {
                            ALOGW("FourCC have unsupport codec, type=%s,fourcc=0x%08x.",
                                  mime, fourcc);
                            continue;
                        }
                    }
                } else {
                    ALOGW("%s is not supported.", codecID);
                    continue;
                }

                const long long width = vtrack->GetWidth();
                const long long height = vtrack->GetHeight();
                if (width <= 0 || width > INT32_MAX) {
                    ALOGW("track width exceeds int32_t, %lld", width);
                    continue;
                }
                if (height <= 0 || height > INT32_MAX) {
                    ALOGW("track height exceeds int32_t, %lld", height);
                    continue;
                }
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_WIDTH, (int32_t)width);
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_HEIGHT, (int32_t)height);

                // setting display width/height is optional
                const long long displayUnit = vtrack->GetDisplayUnit();
                const long long displayWidth = vtrack->GetDisplayWidth();
                const long long displayHeight = vtrack->GetDisplayHeight();
                if (displayWidth > 0 && displayWidth <= INT32_MAX
                        && displayHeight > 0 && displayHeight <= INT32_MAX) {
                    switch (displayUnit) {
                    case 0: // pixels
                        AMediaFormat_setInt32(meta,
                                AMEDIAFORMAT_KEY_DISPLAY_WIDTH, (int32_t)displayWidth);
                        AMediaFormat_setInt32(meta,
                                AMEDIAFORMAT_KEY_DISPLAY_HEIGHT, (int32_t)displayHeight);
                        break;
                    case 1: // centimeters
                    case 2: // inches
                    case 3: // aspect ratio
                    {
                        // Physical layout size is treated the same as aspect ratio.
                        // Note: displayWidth and displayHeight are never zero as they are
                        // checked in the if above.
                        const long long computedWidth =
                                std::max(width, height * displayWidth / displayHeight);
                        const long long computedHeight =
                                std::max(height, width * displayHeight / displayWidth);
                        if (computedWidth <= INT32_MAX && computedHeight <= INT32_MAX) {
                            AMediaFormat_setInt32(meta,
                                    AMEDIAFORMAT_KEY_DISPLAY_WIDTH, (int32_t)computedWidth);
                            AMediaFormat_setInt32(meta,
                                    AMEDIAFORMAT_KEY_DISPLAY_HEIGHT, (int32_t)computedHeight);
                        }
                        break;
                    }
                    default: // unknown display units, perhaps future version of spec.
                        break;
                    }
                }

                getColorInformation(vtrack, meta);

                break;
            }

            case AUDIO_TRACK:
            {
                const mkvparser::AudioTrack *atrack =
                    static_cast<const mkvparser::AudioTrack *>(track);

                if (!strcmp("A_AAC", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_AAC);
                    if (codecPrivateSize < 2) {
                        ALOGW("Incomplete AAC Codec Info %zu byte", codecPrivateSize);
                        continue;
                    }
                    addESDSFromCodecPrivate(
                            meta, true, codecPrivate, codecPrivateSize);
                } else if (!strcmp("A_VORBIS", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_VORBIS);

                    err = addVorbisCodecInfo(
                            meta, codecPrivate, codecPrivateSize);
                } else if (!strcmp("A_OPUS", codecID)) {
                    AMediaFormat_setString(meta,
                            AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_OPUS);
                    AMediaFormat_setBuffer(meta,
                            AMEDIAFORMAT_KEY_CSD_0, codecPrivate, codecPrivateSize);
                    int64_t codecDelay = track->GetCodecDelay();
                    AMediaFormat_setBuffer(meta,
                            AMEDIAFORMAT_KEY_CSD_1, &codecDelay, sizeof(codecDelay));
                    mSeekPreRollNs = track->GetSeekPreRoll();
                    AMediaFormat_setBuffer(meta,
                            AMEDIAFORMAT_KEY_CSD_2, &mSeekPreRollNs, sizeof(mSeekPreRollNs));
                } else if (!strcmp("A_MPEG/L3", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG);
                } else if (!strcmp("A_FLAC", codecID)) {
                    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_FLAC);
                    err = addFlacMetadata(meta, codecPrivate, codecPrivateSize);
                } else if (!strcmp("A_MPEG/L2", codecID)) {
                    AMediaFormat_setString(meta,
                            AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II);
                } else if (!strcmp("A_PCM/INT/LIT", codecID) ||
                         !strcmp("A_PCM/INT/BIG", codecID)) {
                    AMediaFormat_setString(meta,
                            AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_RAW);
                    int32_t bigEndian = !strcmp("A_PCM/INT/BIG", codecID) ? 1: 0;
                    AMediaFormat_setInt32(meta,
                            AMEDIAFORMAT_KEY_PCM_BIG_ENDIAN, bigEndian);
                } else if ((!strcmp("A_MS/ACM", codecID))) {
                    if ((NULL == codecPrivate) || (codecPrivateSize < 18)) {
                        ALOGW("unsupported audio: A_MS/ACM has no valid private data: %s, size: %zu",
                               codecPrivate == NULL ? "null" : "non-null", codecPrivateSize);
                        continue;
                    } else {
                        uint16_t ID = *(uint16_t *)codecPrivate;
                        const char* mime = MKVWave2MIME(ID);
                        ALOGV("A_MS/ACM type is %s", mime);
                        if (!strncasecmp("audio/", mime, 6) &&
                                isMkvAudioCsdSizeOK(mime, codecPrivateSize)) {
                            AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, mime);
                        } else {
                            ALOGE("A_MS/ACM continue, unsupported audio type=%s, csdSize:%zu",
                                mime, codecPrivateSize);
                            continue;
                        }
                        if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_WMA)) {
                            addESDSFromCodecPrivate(meta, true, codecPrivate, codecPrivateSize);
                        } else if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM) ||
                                    !strcmp(mime, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM)) {
                            uint32_t blockAlign = *(uint16_t*)(codecPrivate + 12);
                            addESDSFromCodecPrivate(meta, true, &blockAlign, sizeof(blockAlign));
                        }
                    }
                } else {
                    ALOGW("%s is not supported.", codecID);
                    continue;
                }

                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, atrack->GetSamplingRate());
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, atrack->GetChannels());
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, atrack->GetBitDepth());
                break;
            }

            default:
                continue;
        }

        const char *language = track->GetLanguage();
        if (language != NULL) {
           char lang[4];
           strncpy(lang, language, 3);
           lang[3] = '\0';
           AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_LANGUAGE, lang);
        }

        if (err != OK) {
            ALOGE("skipping track, codec specific data was malformed.");
            continue;
        }

        long long durationNs = mSegment->GetDuration();
        AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_DURATION, (durationNs + 500) / 1000);

        const char *mimetype = "";
        if (!AMediaFormat_getString(meta, AMEDIAFORMAT_KEY_MIME, &mimetype)) {
            // do not add this track to the track list
            ALOGW("ignoring track with unknown mime");
            continue;
        }

        mTracks.push();
        size_t n = mTracks.size() - 1;
        TrackInfo *trackInfo = &mTracks.editItemAt(n);
        initTrackInfo(track, meta, trackInfo);
        trackInfo->mNalLengthSize = nalSize;

        if ((!strcmp("V_MPEG4/ISO/AVC", codecID) && codecPrivateSize == 0) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_AVC) && isSetCsdFrom1stFrame)) {
            // Attempt to recover from AVC track without codec private data
            err = synthesizeAVCC(trackInfo, n);
            if (err != OK) {
                mTracks.pop();
                continue;
            }
        } else if ((!strcmp("V_MPEG2", codecID) && codecPrivateSize == 0) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_MPEG2) && isSetCsdFrom1stFrame)) {
            // Attempt to recover from MPEG2 track without codec private data
            err = synthesizeMPEG2(trackInfo, n);
            if (err != OK) {
                mTracks.pop();
                continue;
            }
        } else if ((!strcmp("V_MPEG4/ISO/ASP", codecID) && codecPrivateSize == 0) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_MPEG4) && isSetCsdFrom1stFrame) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_XVID) && isSetCsdFrom1stFrame) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_DIVX) && isSetCsdFrom1stFrame) ||
            (!strcmp(mimetype, MEDIA_MIMETYPE_VIDEO_DIVX3) && isSetCsdFrom1stFrame)) {
            // Attempt to recover from MPEG4 track without codec private data
            err = synthesizeMPEG4(trackInfo, n);
            if (err != OK) {
                mTracks.pop();
                continue;
            }
        }
        // the TrackInfo owns the metadata now
        meta = nullptr;
    }
    if (meta) {
        AMediaFormat_delete(meta);
    }
}

void MatroskaExtractor::findThumbnails() {
    for (size_t i = 0; i < mTracks.size(); ++i) {
        TrackInfo *info = &mTracks.editItemAt(i);

        const char *mime;
        CHECK(AMediaFormat_getString(info->mMeta, AMEDIAFORMAT_KEY_MIME, &mime));

        if (strncasecmp(mime, "video/", 6)) {
            continue;
        }

        BlockIterator iter(this, info->mTrackNum, i);
        int32_t j = 0;
        int64_t thumbnailTimeUs = 0;
        size_t maxBlockSize = 0;
        while (!iter.eos() && j < 20) {
            if (iter.block()->IsKey()) {
                ++j;

                size_t blockSize = 0;
                for (int k = 0; k < iter.block()->GetFrameCount(); ++k) {
                    blockSize += iter.block()->GetFrame(k).len;
                }

                if (blockSize > maxBlockSize) {
                    maxBlockSize = blockSize;
                    thumbnailTimeUs = iter.blockTimeUs();
                }
            }
            iter.advance();
        }
        AMediaFormat_setInt64(info->mMeta,
                AMEDIAFORMAT_KEY_THUMBNAIL_TIME, thumbnailTimeUs);
    }
}

media_status_t MatroskaExtractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_setString(meta,
            AMEDIAFORMAT_KEY_MIME, mIsWebm ? "video/webm" : MEDIA_MIMETYPE_CONTAINER_MATROSKA);

    return AMEDIA_OK;
}

uint32_t MatroskaExtractor::flags() const {
    uint32_t x = CAN_PAUSE;
    if (!isLiveStreaming()) {
        x |= CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK;
    }

    return x;
}

bool SniffMatroska(
        DataSourceHelper *source, float *confidence) {
    DataSourceBaseReader reader(source);
    mkvparser::EBMLHeader ebmlHeader;
    long long pos;
    if (ebmlHeader.Parse(&reader, pos) < 0) {
        return false;
    }

    *confidence = 0.6;

    return true;
}

static const char *extensions[] = {
    "mka",
    "mkv",
    "webm",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("abbedd92-38c4-4904-a4c1-b3f45f899980"),
        1,
        "Matroska Extractor",
        {
            .v3 = {
                [](
                    CDataSource *source,
                    float *confidence,
                    void **,
                    FreeMetaFunc *) -> CreatorFunc {
                    DataSourceHelper helper(source);
                    if (SniffMatroska(&helper, confidence)) {
                        return [](
                                CDataSource *source,
                                void *) -> CMediaExtractor* {
                            return wrap(new MatroskaExtractor(new DataSourceHelper(source)));};
                    }
                    return NULL;
                },
                extensions
            }
        }
    };
}

} // extern "C"

}  // namespace android
