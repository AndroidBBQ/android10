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
#define LOG_TAG "MP3Extractor"
#include <utils/Log.h>

#include "MP3Extractor.h"

#include "ID3.h"
#include "VBRISeeker.h"
#include "XINGSeeker.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <utils/String8.h>

namespace android {

// Everything must match except for
// protection, bitrate, padding, private bits, mode, mode extension,
// copyright bit, original bit and emphasis.
// Yes ... there are things that must indeed match...
static const uint32_t kMask = 0xfffe0c00;

static bool Resync(
        DataSourceHelper *source, uint32_t match_header,
        off64_t *inout_pos, off64_t *post_id3_pos, uint32_t *out_header) {
    if (post_id3_pos != NULL) {
        *post_id3_pos = 0;
    }

    if (*inout_pos == 0) {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.

        for (;;) {
            uint8_t id3header[10];
            if (source->readAt(*inout_pos, id3header, sizeof(id3header))
                    < (ssize_t)sizeof(id3header)) {
                // If we can't even read these 10 bytes, we might as well bail
                // out, even if there _were_ 10 bytes of valid mp3 audio data...
                return false;
            }

            if (memcmp("ID3", id3header, 3)) {
                break;
            }

            // Skip the ID3v2 header.

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;

            *inout_pos += len;

            ALOGV("skipped ID3 tag, new starting offset is %lld (0x%016llx)",
                    (long long)*inout_pos, (long long)*inout_pos);
        }

        if (post_id3_pos != NULL) {
            *post_id3_pos = *inout_pos;
        }
    }

    off64_t pos = *inout_pos;
    bool valid = false;

    const size_t kMaxReadBytes = 1024;
    const size_t kMaxBytesChecked = 128 * 1024;
    uint8_t buf[kMaxReadBytes];
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;

    do {
        if (pos >= (off64_t)(*inout_pos + kMaxBytesChecked)) {
            // Don't scan forever.
            ALOGV("giving up at offset %lld", (long long)pos);
            break;
        }

        if (remainingBytes < 4) {
            if (reachEOS) {
                break;
            } else {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                 * The next read position should start from the end of
                 * the last buffer, and thus should include the remaining
                 * bytes in the buffer.
                 */
                totalBytesRead = source->readAt(pos + remainingBytes,
                                                buf + remainingBytes,
                                                bytesToRead);
                if (totalBytesRead <= 0) {
                    break;
                }
                reachEOS = (totalBytesRead != bytesToRead);
                totalBytesRead += remainingBytes;
                remainingBytes = totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        if (match_header != 0 && (header & kMask) != (match_header & kMask)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        size_t frame_size;
        int sample_rate, num_channels, bitrate;
        if (!GetMPEGAudioFrameSize(
                    header, &frame_size,
                    &sample_rate, &num_channels, &bitrate)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        ALOGV("found possible 1st frame at %lld (header = 0x%08x)", (long long)pos, header);

        // We found what looks like a valid frame,
        // now find its successors.

        off64_t test_pos = pos + frame_size;

        valid = true;
        for (int j = 0; j < 3; ++j) {
            uint8_t tmp[4];
            if (source->readAt(test_pos, tmp, 4) < 4) {
                valid = false;
                break;
            }

            uint32_t test_header = U32_AT(tmp);

            ALOGV("subsequent header is %08x", test_header);

            if ((test_header & kMask) != (header & kMask)) {
                valid = false;
                break;
            }

            size_t test_frame_size;
            if (!GetMPEGAudioFrameSize(
                        test_header, &test_frame_size)) {
                valid = false;
                break;
            }

            ALOGV("found subsequent frame #%d at %lld", j + 2, (long long)test_pos);

            test_pos += test_frame_size;
        }

        if (valid) {
            *inout_pos = pos;

            if (out_header != NULL) {
                *out_header = header;
            }
        } else {
            ALOGV("no dice, no valid sequence of frames found.");
        }

        ++pos;
        ++tmp;
        --remainingBytes;
    } while (!valid);

    return valid;
}

class MP3Source : public MediaTrackHelper {
public:
    MP3Source(
            AMediaFormat *meta, DataSourceHelper *source,
            off64_t first_frame_pos, uint32_t fixed_header,
            MP3Seeker *seeker);

    virtual media_status_t start();
    virtual media_status_t stop();

    virtual media_status_t getFormat(AMediaFormat *meta);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

protected:
    virtual ~MP3Source();

private:
    static const size_t kMaxFrameSize;
    AMediaFormat *mMeta;
    DataSourceHelper *mDataSource;
    off64_t mFirstFramePos;
    uint32_t mFixedHeader;
    off64_t mCurrentPos;
    int64_t mCurrentTimeUs;
    bool mStarted;
    MP3Seeker *mSeeker;

    int64_t mBasisTimeUs;
    int64_t mSamplesRead;

    MP3Source(const MP3Source &);
    MP3Source &operator=(const MP3Source &);
};

struct Mp3Meta {
    off64_t pos;
    off64_t post_id3_pos;
    uint32_t header;
};

MP3Extractor::MP3Extractor(
        DataSourceHelper *source, Mp3Meta *meta)
    : mInitCheck(NO_INIT),
      mDataSource(source),
      mFirstFramePos(-1),
      mFixedHeader(0),
      mSeeker(NULL) {

    off64_t pos = 0;
    off64_t post_id3_pos;
    uint32_t header;
    bool success;

    if (meta != NULL) {
        // The sniffer has already done all the hard work for us, simply
        // accept its judgement.
        pos = meta->pos;
        header = meta->header;
        post_id3_pos = meta->post_id3_pos;
        success = true;
    } else {
        success = Resync(mDataSource, 0, &pos, &post_id3_pos, &header);
    }

    if (!success) {
        // mInitCheck will remain NO_INIT
        return;
    }

    mFirstFramePos = pos;
    mFixedHeader = header;
    XINGSeeker *seeker = XINGSeeker::CreateFromSource(mDataSource, mFirstFramePos);

    mMeta = AMediaFormat_new();
    if (seeker == NULL) {
        mSeeker = VBRISeeker::CreateFromSource(mDataSource, post_id3_pos);
    } else {
        mSeeker = seeker;
        int encd = seeker->getEncoderDelay();
        int encp = seeker->getEncoderPadding();
        if (encd != 0 || encp != 0) {
            AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_ENCODER_DELAY, encd);
            AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_ENCODER_PADDING, encp);
        }
    }

    if (mSeeker != NULL) {
        // While it is safe to send the XING/VBRI frame to the decoder, this will
        // result in an extra 1152 samples being output. In addition, the bitrate
        // of the Xing header might not match the rest of the file, which could
        // lead to problems when seeking. The real first frame to decode is after
        // the XING/VBRI frame, so skip there.
        size_t frame_size;
        int sample_rate;
        int num_channels;
        int bitrate;
        GetMPEGAudioFrameSize(
                header, &frame_size, &sample_rate, &num_channels, &bitrate);
        pos += frame_size;
        if (!Resync(mDataSource, 0, &pos, &post_id3_pos, &header)) {
            // mInitCheck will remain NO_INIT
            return;
        }
        mFirstFramePos = pos;
        mFixedHeader = header;
    }

    size_t frame_size;
    int sample_rate;
    int num_channels;
    int bitrate;
    GetMPEGAudioFrameSize(
            header, &frame_size, &sample_rate, &num_channels, &bitrate);

    unsigned layer = 4 - ((header >> 17) & 3);

    switch (layer) {
        case 1:
            AMediaFormat_setString(mMeta,
                    AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I);
            break;
        case 2:
            AMediaFormat_setString(mMeta,
                    AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II);
            break;
        case 3:
            AMediaFormat_setString(mMeta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG);
            break;
        default:
            TRESPASS();
    }

    AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sample_rate);
    AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_BIT_RATE, bitrate * 1000);
    AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, num_channels);

    int64_t durationUs;

    if (mSeeker == NULL || !mSeeker->getDuration(&durationUs)) {
        off64_t fileSize;
        if (mDataSource->getSize(&fileSize) == OK) {
            off64_t dataLength = fileSize - mFirstFramePos;
            if (dataLength > INT64_MAX / 8000LL) {
                // duration would overflow
                durationUs = INT64_MAX;
            } else {
                durationUs = 8000LL * dataLength / bitrate;
            }
        } else {
            durationUs = -1;
        }
    }

    if (durationUs >= 0) {
        AMediaFormat_setInt64(mMeta, AMEDIAFORMAT_KEY_DURATION, durationUs);
    }

    mInitCheck = OK;

    // Get iTunes-style gapless info if present.
    // When getting the id3 tag, skip the V1 tags to prevent the source cache
    // from being iterated to the end of the file.
    DataSourceHelper helper(mDataSource);
    ID3 id3(&helper, true);
    if (id3.isValid()) {
        ID3::Iterator *com = new ID3::Iterator(id3, "COM");
        if (com->done()) {
            delete com;
            com = new ID3::Iterator(id3, "COMM");
        }
        while(!com->done()) {
            String8 commentdesc;
            String8 commentvalue;
            com->getString(&commentdesc, &commentvalue);
            const char * desc = commentdesc.string();
            const char * value = commentvalue.string();

            // first 3 characters are the language, which we don't care about
            if(strlen(desc) > 3 && strcmp(desc + 3, "iTunSMPB") == 0) {

                int32_t delay, padding;
                if (sscanf(value, " %*x %x %x %*x", &delay, &padding) == 2) {
                    AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_ENCODER_DELAY, delay);
                    AMediaFormat_setInt32(mMeta, AMEDIAFORMAT_KEY_ENCODER_PADDING, padding);
                }
                break;
            }
            com->next();
        }
        delete com;
        com = NULL;
    }
}

MP3Extractor::~MP3Extractor() {
    delete mSeeker;
    delete mDataSource;
    AMediaFormat_delete(mMeta);
}

size_t MP3Extractor::countTracks() {
    return mInitCheck != OK ? 0 : 1;
}

MediaTrackHelper *MP3Extractor::getTrack(size_t index) {
    if (mInitCheck != OK || index != 0) {
        return NULL;
    }

    return new MP3Source(
            mMeta, mDataSource, mFirstFramePos, mFixedHeader,
            mSeeker);
}

media_status_t MP3Extractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    if (mInitCheck != OK || index != 0) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    AMediaFormat_copy(meta, mMeta);
    return AMEDIA_OK;
}

////////////////////////////////////////////////////////////////////////////////

// The theoretical maximum frame size for an MPEG audio stream should occur
// while playing a Layer 2, MPEGv2.5 audio stream at 160kbps (with padding).
// The size of this frame should be...
// ((1152 samples/frame * 160000 bits/sec) /
//  (8000 samples/sec * 8 bits/byte)) + 1 padding byte/frame = 2881 bytes/frame.
// Set our max frame size to the nearest power of 2 above this size (aka, 4kB)
const size_t MP3Source::kMaxFrameSize = (1 << 12); /* 4096 bytes */
MP3Source::MP3Source(
        AMediaFormat *meta, DataSourceHelper *source,
        off64_t first_frame_pos, uint32_t fixed_header,
        MP3Seeker *seeker)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mFixedHeader(fixed_header),
      mCurrentPos(0),
      mCurrentTimeUs(0),
      mStarted(false),
      mSeeker(seeker),
      mBasisTimeUs(0),
      mSamplesRead(0) {
}

MP3Source::~MP3Source() {
    if (mStarted) {
        stop();
    }
}

media_status_t MP3Source::start() {
    CHECK(!mStarted);

    mBufferGroup->add_buffer(kMaxFrameSize);

    mCurrentPos = mFirstFramePos;
    mCurrentTimeUs = 0;

    mBasisTimeUs = mCurrentTimeUs;
    mSamplesRead = 0;

    mStarted = true;

    return AMEDIA_OK;
}

media_status_t MP3Source::stop() {
    CHECK(mStarted);

    mStarted = false;

    return AMEDIA_OK;
}

media_status_t MP3Source::getFormat(AMediaFormat *meta) {
    return AMediaFormat_copy(meta, mMeta);
}

media_status_t MP3Source::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    *out = NULL;

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    bool seekCBR = false;

    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode)) {
        int64_t actualSeekTimeUs = seekTimeUs;
        if (mSeeker == NULL
                || !mSeeker->getOffsetForTime(&actualSeekTimeUs, &mCurrentPos)) {
            int32_t bitrate;
            if (!AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_BIT_RATE, &bitrate)) {
                // bitrate is in bits/sec.
                ALOGI("no bitrate");

                return AMEDIA_ERROR_UNSUPPORTED;
            }

            mCurrentTimeUs = seekTimeUs;
            mCurrentPos = mFirstFramePos + seekTimeUs * bitrate / 8000000;
            seekCBR = true;
        } else {
            mCurrentTimeUs = actualSeekTimeUs;
        }

        mBasisTimeUs = mCurrentTimeUs;
        mSamplesRead = 0;
    }

    MediaBufferHelper *buffer;
    status_t err = mBufferGroup->acquire_buffer(&buffer);
    if (err != OK) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    size_t frame_size;
    int bitrate;
    int num_samples;
    int sample_rate;
    for (;;) {
        ssize_t n = mDataSource->readAt(mCurrentPos, buffer->data(), 4);
        if (n < 4) {
            buffer->release();
            buffer = NULL;

            return ((n < 0 && n != ERROR_END_OF_STREAM) ?
                    AMEDIA_ERROR_UNKNOWN : AMEDIA_ERROR_END_OF_STREAM);
        }

        uint32_t header = U32_AT((const uint8_t *)buffer->data());

        if ((header & kMask) == (mFixedHeader & kMask)
            && GetMPEGAudioFrameSize(
                header, &frame_size, &sample_rate, NULL,
                &bitrate, &num_samples)) {

            // re-calculate mCurrentTimeUs because we might have called Resync()
            if (seekCBR) {
                mCurrentTimeUs = (mCurrentPos - mFirstFramePos) * 8000 / bitrate;
                mBasisTimeUs = mCurrentTimeUs;
            }

            break;
        }

        // Lost sync.
        ALOGV("lost sync! header = 0x%08x, old header = 0x%08x\n", header, mFixedHeader);

        off64_t pos = mCurrentPos;
        if (!Resync(mDataSource, mFixedHeader, &pos, NULL, NULL)) {
            ALOGE("Unable to resync. Signalling end of stream.");

            buffer->release();
            buffer = NULL;

            return AMEDIA_ERROR_END_OF_STREAM;
        }

        mCurrentPos = pos;

        // Try again with the new position.
    }

    CHECK(frame_size <= buffer->size());

    ssize_t n = mDataSource->readAt(mCurrentPos, buffer->data(), frame_size);
    if (n < (ssize_t)frame_size) {
        buffer->release();
        buffer = NULL;

        return ((n < 0 && n != ERROR_END_OF_STREAM) ?
                AMEDIA_ERROR_UNKNOWN : AMEDIA_ERROR_END_OF_STREAM);
    }

    buffer->set_range(0, frame_size);

    AMediaFormat *meta = buffer->meta_data();
    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, mCurrentTimeUs);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);

    mCurrentPos += frame_size;

    mSamplesRead += num_samples;
    mCurrentTimeUs = mBasisTimeUs + ((mSamplesRead * 1000000) / sample_rate);

    *out = buffer;

    return AMEDIA_OK;
}

media_status_t MP3Extractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_clear(meta);
    if (mInitCheck != OK) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MPEG);

    DataSourceHelper helper(mDataSource);
    ID3 id3(&helper);

    if (!id3.isValid()) {
        return AMEDIA_OK;
    }

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

        AMediaFormat_setString(meta, kMap[i].key, s.string());
    }

    size_t dataSize;
    String8 mime;
    const void *data = id3.getAlbumArt(&dataSize, &mime);

    if (data) {
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_ALBUMART, data, dataSize);
    }

    return AMEDIA_OK;
}

static CMediaExtractor* CreateExtractor(
        CDataSource *source,
        void *meta) {
    Mp3Meta *metaData = static_cast<Mp3Meta *>(meta);
    return wrap(new MP3Extractor(new DataSourceHelper(source), metaData));
}

static CreatorFunc Sniff(
        CDataSource *source, float *confidence, void **meta,
        FreeMetaFunc *freeMeta) {
    off64_t pos = 0;
    off64_t post_id3_pos;
    uint32_t header;
    uint8_t mpeg_header[5];
    DataSourceHelper helper(source);
    if (helper.readAt(0, mpeg_header, sizeof(mpeg_header)) < (ssize_t)sizeof(mpeg_header)) {
        return NULL;
    }

    if (!memcmp("\x00\x00\x01\xba", mpeg_header, 4) && (mpeg_header[4] >> 4) == 2) {
        ALOGV("MPEG1PS container is not supported!");
        return NULL;
    }
    if (!Resync(&helper, 0, &pos, &post_id3_pos, &header)) {
        return NULL;
    }

    Mp3Meta *mp3Meta = new Mp3Meta;
    mp3Meta->pos = pos;
    mp3Meta->header = header;
    mp3Meta->post_id3_pos = post_id3_pos;
    *meta = mp3Meta;
    *freeMeta = ::free;

    *confidence = 0.2f;

    return CreateExtractor;
}

static const char *extensions[] = {
    "mp2",
    "mp3",
    "mpeg",
    "mpg",
    "mpga",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("812a3f6c-c8cf-46de-b529-3774b14103d4"),
        1, // version
        "MP3 Extractor",
        { .v3 = {Sniff, extensions} }
    };
}

} // extern "C"

}  // namespace android
