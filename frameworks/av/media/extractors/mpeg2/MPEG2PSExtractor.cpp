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

//#define LOG_NDEBUG 0
#define LOG_TAG "MPEG2PSExtractor"
#include <utils/Log.h>

#include "MPEG2PSExtractor.h"

#include "mpeg2ts/AnotherPacketSource.h"
#include "mpeg2ts/ESQueue.h"

#include <media/DataSourceBase.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

#include <inttypes.h>

namespace android {

struct MPEG2PSExtractor::Track : public MediaTrackHelper {
    Track(MPEG2PSExtractor *extractor,
          unsigned stream_id, unsigned stream_type);

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options);

protected:
    virtual ~Track();

private:
    friend struct MPEG2PSExtractor;

    MPEG2PSExtractor *mExtractor;

    unsigned mStreamID;
    unsigned mStreamType;
    ElementaryStreamQueue *mQueue;
    sp<AnotherPacketSource> mSource;

    status_t appendPESData(
            unsigned PTS_DTS_flags,
            uint64_t PTS, uint64_t DTS,
            const uint8_t *data, size_t size);

    DISALLOW_EVIL_CONSTRUCTORS(Track);
};

struct MPEG2PSExtractor::WrappedTrack : public MediaTrackHelper {
    WrappedTrack(MPEG2PSExtractor *extractor, Track *track);

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options);

protected:
    virtual ~WrappedTrack();

private:
    MPEG2PSExtractor *mExtractor;
    MPEG2PSExtractor::Track *mTrack;

    DISALLOW_EVIL_CONSTRUCTORS(WrappedTrack);
};

////////////////////////////////////////////////////////////////////////////////

MPEG2PSExtractor::MPEG2PSExtractor(DataSourceHelper *source)
    : mDataSource(source),
      mOffset(0),
      mFinalResult(OK),
      mBuffer(new ABuffer(0)),
      mScanning(true),
      mProgramStreamMapValid(false) {
    for (size_t i = 0; i < 500; ++i) {
        if (feedMore() != OK) {
            break;
        }
    }

    // Remove all tracks that were unable to determine their format.
    AMediaFormat *meta = AMediaFormat_new();
    for (size_t i = mTracks.size(); i > 0;) {
        i--;
        if (mTracks.valueAt(i)->getFormat(meta) != AMEDIA_OK) {
            mTracks.removeItemsAt(i);
        }
    }
    AMediaFormat_delete(meta);

    mScanning = false;
}

MPEG2PSExtractor::~MPEG2PSExtractor() {
    delete mDataSource;
}

size_t MPEG2PSExtractor::countTracks() {
    return mTracks.size();
}

MediaTrackHelper *MPEG2PSExtractor::getTrack(size_t index) {
    if (index >= mTracks.size()) {
        return NULL;
    }

    return new WrappedTrack(this, mTracks.valueAt(index));
}

media_status_t MPEG2PSExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    if (index >= mTracks.size()) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    return mTracks.valueAt(index)->getFormat(meta);
}

media_status_t MPEG2PSExtractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_MPEG2PS);

    return AMEDIA_OK;
}

uint32_t MPEG2PSExtractor::flags() const {
    return CAN_PAUSE;
}

status_t MPEG2PSExtractor::feedMore() {
    Mutex::Autolock autoLock(mLock);

    // How much data we're reading at a time
    static const size_t kChunkSize = 8192;

    for (;;) {
        status_t err = dequeueChunk();

        if (err == -EAGAIN && mFinalResult == OK) {
            memmove(mBuffer->base(), mBuffer->data(), mBuffer->size());
            mBuffer->setRange(0, mBuffer->size());

            if (mBuffer->size() + kChunkSize > mBuffer->capacity()) {
                size_t newCapacity = mBuffer->capacity() + kChunkSize;
                sp<ABuffer> newBuffer = new ABuffer(newCapacity);
                memcpy(newBuffer->data(), mBuffer->data(), mBuffer->size());
                newBuffer->setRange(0, mBuffer->size());
                mBuffer = newBuffer;
            }

            ssize_t n = mDataSource->readAt(
                    mOffset, mBuffer->data() + mBuffer->size(), kChunkSize);

            if (n < (ssize_t)kChunkSize) {
                mFinalResult = (n < 0) ? (status_t)n : ERROR_END_OF_STREAM;
                return mFinalResult;
            }

            mBuffer->setRange(mBuffer->offset(), mBuffer->size() + n);
            mOffset += n;
        } else if (err != OK) {
            mFinalResult = err;
            return err;
        } else {
            return OK;
        }
    }
}

status_t MPEG2PSExtractor::dequeueChunk() {
    if (mBuffer->size() < 4) {
        return -EAGAIN;
    }

    if (memcmp("\x00\x00\x01", mBuffer->data(), 3)) {
        return ERROR_MALFORMED;
    }

    unsigned chunkType = mBuffer->data()[3];

    ssize_t res;

    switch (chunkType) {
        case 0xba:
        {
            res = dequeuePack();
            break;
        }

        case 0xbb:
        {
            res = dequeueSystemHeader();
            break;
        }

        default:
        {
            res = dequeuePES();
            break;
        }
    }

    if (res > 0) {
        if (mBuffer->size() < (size_t)res) {
            return -EAGAIN;
        }

        mBuffer->setRange(mBuffer->offset() + res, mBuffer->size() - res);
        res = OK;
    }

    return res;
}

ssize_t MPEG2PSExtractor::dequeuePack() {
    // 32 + 2 + 3 + 1 + 15 + 1 + 15+ 1 + 9 + 1 + 22 + 1 + 1 | +5

    if (mBuffer->size() < 14) {
        return -EAGAIN;
    }

    unsigned pack_stuffing_length = mBuffer->data()[13] & 7;

    return pack_stuffing_length + 14;
}

ssize_t MPEG2PSExtractor::dequeueSystemHeader() {
    if (mBuffer->size() < 6) {
        return -EAGAIN;
    }

    unsigned header_length = U16_AT(mBuffer->data() + 4);

    return header_length + 6;
}

ssize_t MPEG2PSExtractor::dequeuePES() {
    if (mBuffer->size() < 6) {
        return -EAGAIN;
    }

    unsigned PES_packet_length = U16_AT(mBuffer->data() + 4);
    if (PES_packet_length == 0u) {
        ALOGE("PES_packet_length is 0");
        return -EAGAIN;
    }

    size_t n = PES_packet_length + 6;

    if (mBuffer->size() < n) {
        return -EAGAIN;
    }

    ABitReader br(mBuffer->data(), n);

    unsigned packet_startcode_prefix = br.getBits(24);

    ALOGV("packet_startcode_prefix = 0x%08x", packet_startcode_prefix);

    if (packet_startcode_prefix != 1) {
        ALOGV("Supposedly payload_unit_start=1 unit does not start "
             "with startcode.");

        return ERROR_MALFORMED;
    }

    if (packet_startcode_prefix != 0x000001u) {
        ALOGE("Wrong PES prefix");
        return ERROR_MALFORMED;
    }

    unsigned stream_id = br.getBits(8);
    ALOGV("stream_id = 0x%02x", stream_id);

    /* unsigned PES_packet_length = */br.getBits(16);

    if (stream_id == 0xbc) {
        // program_stream_map

        if (!mScanning) {
            return n;
        }

        mStreamTypeByESID.clear();

        /* unsigned current_next_indicator = */br.getBits(1);
        /* unsigned reserved = */br.getBits(2);
        /* unsigned program_stream_map_version = */br.getBits(5);
        /* unsigned reserved = */br.getBits(7);
        /* unsigned marker_bit = */br.getBits(1);
        unsigned program_stream_info_length = br.getBits(16);

        size_t offset = 0;
        while (offset < program_stream_info_length) {
            if (offset + 2 > program_stream_info_length) {
                return ERROR_MALFORMED;
            }

            unsigned descriptor_tag = br.getBits(8);
            unsigned descriptor_length = br.getBits(8);

            ALOGI("found descriptor tag 0x%02x of length %u",
                 descriptor_tag, descriptor_length);

            if (offset + 2 + descriptor_length > program_stream_info_length) {
                return ERROR_MALFORMED;
            }

            br.skipBits(8 * descriptor_length);

            offset += 2 + descriptor_length;
        }

        unsigned elementary_stream_map_length = br.getBits(16);

        offset = 0;
        while (offset < elementary_stream_map_length) {
            if (offset + 4 > elementary_stream_map_length) {
                return ERROR_MALFORMED;
            }

            unsigned stream_type = br.getBits(8);
            unsigned elementary_stream_id = br.getBits(8);

            ALOGI("elementary stream id 0x%02x has stream type 0x%02x",
                 elementary_stream_id, stream_type);

            mStreamTypeByESID.add(elementary_stream_id, stream_type);

            unsigned elementary_stream_info_length = br.getBits(16);

            if (offset + 4 + elementary_stream_info_length
                    > elementary_stream_map_length) {
                return ERROR_MALFORMED;
            }

            offset += 4 + elementary_stream_info_length;
        }

        /* unsigned CRC32 = */br.getBits(32);

        mProgramStreamMapValid = true;
    } else if (stream_id != 0xbe  // padding_stream
            && stream_id != 0xbf  // private_stream_2
            && stream_id != 0xf0  // ECM
            && stream_id != 0xf1  // EMM
            && stream_id != 0xff  // program_stream_directory
            && stream_id != 0xf2  // DSMCC
            && stream_id != 0xf8) {  // H.222.1 type E
        /* unsigned PES_marker_bits = */br.getBits(2);  // should be 0x2(hex)
        /* unsigned PES_scrambling_control = */br.getBits(2);
        /* unsigned PES_priority = */br.getBits(1);
        /* unsigned data_alignment_indicator = */br.getBits(1);
        /* unsigned copyright = */br.getBits(1);
        /* unsigned original_or_copy = */br.getBits(1);

        unsigned PTS_DTS_flags = br.getBits(2);
        ALOGV("PTS_DTS_flags = %u", PTS_DTS_flags);

        unsigned ESCR_flag = br.getBits(1);
        ALOGV("ESCR_flag = %u", ESCR_flag);

        unsigned ES_rate_flag = br.getBits(1);
        ALOGV("ES_rate_flag = %u", ES_rate_flag);

        unsigned DSM_trick_mode_flag = br.getBits(1);
        ALOGV("DSM_trick_mode_flag = %u", DSM_trick_mode_flag);

        unsigned additional_copy_info_flag = br.getBits(1);
        ALOGV("additional_copy_info_flag = %u", additional_copy_info_flag);

        /* unsigned PES_CRC_flag = */br.getBits(1);
        /* PES_extension_flag = */br.getBits(1);

        unsigned PES_header_data_length = br.getBits(8);
        ALOGV("PES_header_data_length = %u", PES_header_data_length);

        unsigned optional_bytes_remaining = PES_header_data_length;

        uint64_t PTS = 0, DTS = 0;

        if (PTS_DTS_flags == 2 || PTS_DTS_flags == 3) {
            if (optional_bytes_remaining < 5u) {
                return ERROR_MALFORMED;
            }

            if (br.getBits(4) != PTS_DTS_flags) {
                return ERROR_MALFORMED;
            }

            PTS = ((uint64_t)br.getBits(3)) << 30;
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            PTS |= ((uint64_t)br.getBits(15)) << 15;
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            PTS |= br.getBits(15);
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            ALOGV("PTS = %" PRIu64, PTS);
            // ALOGI("PTS = %.2f secs", PTS / 90000.0f);

            optional_bytes_remaining -= 5;

            if (PTS_DTS_flags == 3) {
                if (optional_bytes_remaining < 5u) {
                    return ERROR_MALFORMED;
                }

                if (br.getBits(4) != 1u) {
                    return ERROR_MALFORMED;
                }

                DTS = ((uint64_t)br.getBits(3)) << 30;
                if (br.getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }
                DTS |= ((uint64_t)br.getBits(15)) << 15;
                if (br.getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }
                DTS |= br.getBits(15);
                if (br.getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }

                ALOGV("DTS = %" PRIu64, DTS);

                optional_bytes_remaining -= 5;
            }
        }

        if (ESCR_flag) {
            if (optional_bytes_remaining < 6u) {
                return ERROR_MALFORMED;
            }

            br.getBits(2);

            uint64_t ESCR = ((uint64_t)br.getBits(3)) << 30;
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            ESCR |= ((uint64_t)br.getBits(15)) << 15;
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            ESCR |= br.getBits(15);
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            ALOGV("ESCR = %" PRIu64, ESCR);
            /* unsigned ESCR_extension = */br.getBits(9);

            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            optional_bytes_remaining -= 6;
        }

        if (ES_rate_flag) {
            if (optional_bytes_remaining < 3u) {
                return ERROR_MALFORMED;
            }

            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            /* unsigned ES_rate = */br.getBits(22);
            if (br.getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            optional_bytes_remaining -= 3;
        }

        if (br.numBitsLeft() < optional_bytes_remaining * 8) {
            return ERROR_MALFORMED;
        }

        br.skipBits(optional_bytes_remaining * 8);

        // ES data follows.

        if (PES_packet_length < PES_header_data_length + 3) {
            return ERROR_MALFORMED;
        }

        unsigned dataLength =
            PES_packet_length - 3 - PES_header_data_length;

        if (br.numBitsLeft() < dataLength * 8) {
            ALOGE("PES packet does not carry enough data to contain "
                 "payload. (numBitsLeft = %zu, required = %u)",
                 br.numBitsLeft(), dataLength * 8);

            return ERROR_MALFORMED;
        }

        if (br.numBitsLeft() < dataLength * 8) {
            return ERROR_MALFORMED;
        }

        ssize_t index = mTracks.indexOfKey(stream_id);
        if (index < 0 && mScanning) {
            unsigned streamType;

            ssize_t streamTypeIndex;
            if (mProgramStreamMapValid
                    && (streamTypeIndex =
                            mStreamTypeByESID.indexOfKey(stream_id)) >= 0) {
                streamType = mStreamTypeByESID.valueAt(streamTypeIndex);
            } else if ((stream_id & ~0x1f) == 0xc0) {
                // ISO/IEC 13818-3 or ISO/IEC 11172-3 or ISO/IEC 13818-7
                // or ISO/IEC 14496-3 audio
                streamType = ATSParser::STREAMTYPE_MPEG2_AUDIO;
            } else if ((stream_id & ~0x0f) == 0xe0) {
                // ISO/IEC 13818-2 or ISO/IEC 11172-2 or ISO/IEC 14496-2 video
                streamType = ATSParser::STREAMTYPE_MPEG2_VIDEO;
            } else {
                streamType = ATSParser::STREAMTYPE_RESERVED;
            }

            index = mTracks.add(
                    stream_id, new Track(this, stream_id, streamType));
        }

        status_t err = OK;

        if (index >= 0) {
            err =
                mTracks.editValueAt(index)->appendPESData(
                    PTS_DTS_flags, PTS, DTS, br.data(), dataLength);
        }

        br.skipBits(dataLength * 8);

        if (err != OK) {
            return err;
        }
    } else if (stream_id == 0xbe) {  // padding_stream
        if (PES_packet_length == 0u) {
            return ERROR_MALFORMED;
        }
        br.skipBits(PES_packet_length * 8);
    } else {
        if (PES_packet_length == 0u) {
            return ERROR_MALFORMED;
        }
        br.skipBits(PES_packet_length * 8);
    }

    return n;
}

////////////////////////////////////////////////////////////////////////////////

MPEG2PSExtractor::Track::Track(
        MPEG2PSExtractor *extractor, unsigned stream_id, unsigned stream_type)
    : mExtractor(extractor),
      mStreamID(stream_id),
      mStreamType(stream_type),
      mQueue(NULL) {
    bool supported = true;
    ElementaryStreamQueue::Mode mode;

    switch (mStreamType) {
        case ATSParser::STREAMTYPE_H264:
            mode = ElementaryStreamQueue::H264;
            break;
        case ATSParser::STREAMTYPE_MPEG2_AUDIO_ADTS:
            mode = ElementaryStreamQueue::AAC;
            break;
        case ATSParser::STREAMTYPE_MPEG1_AUDIO:
        case ATSParser::STREAMTYPE_MPEG2_AUDIO:
            mode = ElementaryStreamQueue::MPEG_AUDIO;
            break;

        case ATSParser::STREAMTYPE_MPEG1_VIDEO:
        case ATSParser::STREAMTYPE_MPEG2_VIDEO:
            mode = ElementaryStreamQueue::MPEG_VIDEO;
            break;

        case ATSParser::STREAMTYPE_MPEG4_VIDEO:
            mode = ElementaryStreamQueue::MPEG4_VIDEO;
            break;

        default:
            supported = false;
            break;
    }

    if (supported) {
        mQueue = new ElementaryStreamQueue(mode);
    } else {
        ALOGI("unsupported stream ID 0x%02x", stream_id);
    }
}

MPEG2PSExtractor::Track::~Track() {
    delete mQueue;
    mQueue = NULL;
}

media_status_t MPEG2PSExtractor::Track::start() {
    if (mSource == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    // initialize with one small buffer, but allow growth
    mBufferGroup->init(1 /* one buffer */, 256 /* buffer size */, 64 /* max number of buffers */);

    if (mSource->start(NULL) == OK) { // AnotherPacketSource::start doesn't use its argument
        return AMEDIA_OK;
    }
    return AMEDIA_ERROR_UNKNOWN;
}

media_status_t MPEG2PSExtractor::Track::stop() {
    if (mSource == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    if (mSource->stop() == OK) {
        return AMEDIA_OK;
    }
    return AMEDIA_ERROR_UNKNOWN;
}

void copyAMessageToAMediaFormat(AMediaFormat *format, sp<AMessage> msg);

media_status_t MPEG2PSExtractor::Track::getFormat(AMediaFormat *meta) {
    if (mSource == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    sp<MetaData> sourceMeta = mSource->getFormat();
    sp<AMessage> msg;
    convertMetaDataToMessage(sourceMeta, &msg);
    copyAMessageToAMediaFormat(meta, msg);
    return AMEDIA_OK;
}

media_status_t MPEG2PSExtractor::Track::read(
        MediaBufferHelper **buffer, const ReadOptions *options) {
    if (mSource == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    status_t finalResult;
    while (!mSource->hasBufferAvailable(&finalResult)) {
        if (finalResult != OK) {
            return AMEDIA_ERROR_END_OF_STREAM;
        }

        status_t err = mExtractor->feedMore();

        if (err != OK) {
            mSource->signalEOS(err);
        }
    }

    MediaBufferBase *mbuf;
    mSource->read(&mbuf, (MediaTrack::ReadOptions*) options);
    size_t length = mbuf->range_length();
    MediaBufferHelper *outbuf;
    mBufferGroup->acquire_buffer(&outbuf, false, length);
    memcpy(outbuf->data(), mbuf->data(), length);
    outbuf->set_range(0, length);
    *buffer = outbuf;
    MetaDataBase &inMeta = mbuf->meta_data();
    AMediaFormat *outMeta = outbuf->meta_data();
    int64_t val64;
    if (inMeta.findInt64(kKeyTime, &val64)) {
        AMediaFormat_setInt64(outMeta, AMEDIAFORMAT_KEY_TIME_US, val64);
    }
    int32_t val32;
    if (inMeta.findInt32(kKeyIsSyncFrame, &val32)) {
        AMediaFormat_setInt32(outMeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, val32);
    }
    if (inMeta.findInt32(kKeyCryptoMode, &val32)) {
        AMediaFormat_setInt32(outMeta, AMEDIAFORMAT_KEY_CRYPTO_MODE, val32);
    }
    uint32_t bufType;
    const void *bufData;
    size_t bufSize;
    if (inMeta.findData(kKeyCryptoIV, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_IV, bufData, bufSize);
    }
    if (inMeta.findData(kKeyCryptoKey, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_KEY, bufData, bufSize);
    }
    if (inMeta.findData(kKeyPlainSizes, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES, bufData, bufSize);
    }
    if (inMeta.findData(kKeyEncryptedSizes, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES, bufData, bufSize);
    }
    if (inMeta.findData(kKeySEI, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_SEI, bufData, bufSize);
    }
    if (inMeta.findData(kKeyAudioPresentationInfo, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO, bufData, bufSize);
    }
    mbuf->release();
    return AMEDIA_OK;
}

status_t MPEG2PSExtractor::Track::appendPESData(
        unsigned PTS_DTS_flags,
        uint64_t PTS, uint64_t /* DTS */,
        const uint8_t *data, size_t size) {
    if (mQueue == NULL) {
        return OK;
    }

    int64_t timeUs;
    if (PTS_DTS_flags == 2 || PTS_DTS_flags == 3) {
        timeUs = (PTS * 100) / 9;
    } else {
        timeUs = 0;
    }

    status_t err = mQueue->appendData(data, size, timeUs);

    if (err != OK) {
        return err;
    }

    sp<ABuffer> accessUnit;
    while ((accessUnit = mQueue->dequeueAccessUnit()) != NULL) {
        if (mSource == NULL) {
            sp<MetaData> meta = mQueue->getFormat();

            if (meta != NULL) {
                ALOGV("Stream ID 0x%02x now has data.", mStreamID);

                mSource = new AnotherPacketSource(meta);
                mSource->queueAccessUnit(accessUnit);
            }
        } else if (mQueue->getFormat() != NULL) {
            mSource->queueAccessUnit(accessUnit);
        }
    }

    return OK;
}

////////////////////////////////////////////////////////////////////////////////

MPEG2PSExtractor::WrappedTrack::WrappedTrack(
        MPEG2PSExtractor *extractor, Track *track)
    : mExtractor(extractor),
      mTrack(track) {
}

MPEG2PSExtractor::WrappedTrack::~WrappedTrack() {
}

media_status_t MPEG2PSExtractor::WrappedTrack::start() {
    mTrack->mBufferGroup = mBufferGroup;
    return mTrack->start();
}

media_status_t MPEG2PSExtractor::WrappedTrack::stop() {
    return mTrack->stop();
}

media_status_t MPEG2PSExtractor::WrappedTrack::getFormat(AMediaFormat *meta) {
    return mTrack->getFormat(meta);
}

media_status_t MPEG2PSExtractor::WrappedTrack::read(
        MediaBufferHelper **buffer, const ReadOptions *options) {
    return mTrack->read(buffer, options);
}

////////////////////////////////////////////////////////////////////////////////

bool SniffMPEG2PS(
        DataSourceHelper *source, float *confidence) {
    uint8_t header[5];
    if (source->readAt(0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return false;
    }

    if (memcmp("\x00\x00\x01\xba", header, 4) || (header[4] >> 6) != 1) {
        return false;
    }

    *confidence = 0.25f;  // Slightly larger than .mp3 extractor's confidence

    return true;
}

}  // namespace android
