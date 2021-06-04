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
#define LOG_TAG "WAVExtractor"
#include <utils/Log.h>

#include "WAVExtractor.h"

#include <android/binder_ibinder.h> // for AIBinder_getCallingUid
#include <audio_utils/primitives.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <private/android_filesystem_config.h> // for AID_MEDIA
#include <system/audio.h>
#include <utils/String8.h>
#include <cutils/bitops.h>

#define CHANNEL_MASK_USE_CHANNEL_ORDER 0

// NOTE: This code assumes the device processor is little endian.

namespace android {

// MediaServer is capable of handling float extractor output, but general processes
// may not be able to do so.
// TODO: Improve API to set extractor float output.
// (Note: duplicated with FLACExtractor.cpp)
static inline bool shouldExtractorOutputFloat(int bitsPerSample)
{
    return bitsPerSample > 16 && AIBinder_getCallingUid() == AID_MEDIA;
}

enum {
    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_MULAW      = 0x0007,
    WAVE_FORMAT_MSGSM      = 0x0031,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE
};

static const char* WAVEEXT_SUBFORMAT = "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71";
static const char* AMBISONIC_SUBFORMAT = "\x00\x00\x21\x07\xD3\x11\x86\x44\xC8\xC1\xCA\x00\x00\x00";

static uint32_t U32_LE_AT(const uint8_t *ptr) {
    return ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
}

static uint16_t U16_LE_AT(const uint8_t *ptr) {
    return ptr[1] << 8 | ptr[0];
}

struct WAVSource : public MediaTrackHelper {
    WAVSource(
            DataSourceHelper *dataSource,
            AMediaFormat *meta,
            uint16_t waveFormat,
            bool outputFloat,
            off64_t offset, size_t size);

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *meta);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

    bool supportsNonBlockingRead() override { return false; }

protected:
    virtual ~WAVSource();

private:
    static const size_t kMaxFrameSize;

    DataSourceHelper *mDataSource;
    AMediaFormat *mMeta;
    uint16_t mWaveFormat;
    const bool mOutputFloat;
    int32_t mSampleRate;
    int32_t mNumChannels;
    int32_t mBitsPerSample;
    off64_t mOffset;
    size_t mSize;
    bool mStarted;
    off64_t mCurrentPos;

    WAVSource(const WAVSource &);
    WAVSource &operator=(const WAVSource &);
};

WAVExtractor::WAVExtractor(DataSourceHelper *source)
    : mDataSource(source),
      mValidFormat(false),
      mChannelMask(CHANNEL_MASK_USE_CHANNEL_ORDER) {
    mTrackMeta = AMediaFormat_new();
    mInitCheck = init();
}

WAVExtractor::~WAVExtractor() {
    delete mDataSource;
    AMediaFormat_delete(mTrackMeta);
}

media_status_t WAVExtractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_clear(meta);
    if (mInitCheck == OK) {
        AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_WAV);
    }

    return AMEDIA_OK;
}

size_t WAVExtractor::countTracks() {
    return mInitCheck == OK ? 1 : 0;
}

MediaTrackHelper *WAVExtractor::getTrack(size_t index) {
    if (mInitCheck != OK || index > 0) {
        return NULL;
    }

    return new WAVSource(
            mDataSource, mTrackMeta,
            mWaveFormat, shouldExtractorOutputFloat(mBitsPerSample), mDataOffset, mDataSize);
}

media_status_t WAVExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    if (mInitCheck != OK || index > 0) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    const media_status_t status = AMediaFormat_copy(meta, mTrackMeta);
    if (status == OK) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING,
                shouldExtractorOutputFloat(mBitsPerSample)
                        ? kAudioEncodingPcmFloat : kAudioEncodingPcm16bit);
    }
    return status;
}

status_t WAVExtractor::init() {
    uint8_t header[12];
    if (mDataSource->readAt(
                0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return NO_INIT;
    }

    if (memcmp(header, "RIFF", 4) || memcmp(&header[8], "WAVE", 4)) {
        return NO_INIT;
    }

    size_t totalSize = U32_LE_AT(&header[4]);

    off64_t offset = 12;
    size_t remainingSize = totalSize;
    while (remainingSize >= 8) {
        uint8_t chunkHeader[8];
        if (mDataSource->readAt(offset, chunkHeader, 8) < 8) {
            return NO_INIT;
        }

        remainingSize -= 8;
        offset += 8;

        uint32_t chunkSize = U32_LE_AT(&chunkHeader[4]);

        if (chunkSize > remainingSize) {
            return NO_INIT;
        }

        if (!memcmp(chunkHeader, "fmt ", 4)) {
            if (chunkSize < 16) {
                return NO_INIT;
            }

            uint8_t formatSpec[40];
            if (mDataSource->readAt(offset, formatSpec, 2) < 2) {
                return NO_INIT;
            }

            mWaveFormat = U16_LE_AT(formatSpec);
            if (mWaveFormat != WAVE_FORMAT_PCM
                    && mWaveFormat != WAVE_FORMAT_IEEE_FLOAT
                    && mWaveFormat != WAVE_FORMAT_ALAW
                    && mWaveFormat != WAVE_FORMAT_MULAW
                    && mWaveFormat != WAVE_FORMAT_MSGSM
                    && mWaveFormat != WAVE_FORMAT_EXTENSIBLE) {
                return AMEDIA_ERROR_UNSUPPORTED;
            }

            uint8_t fmtSize = 16;
            if (mWaveFormat == WAVE_FORMAT_EXTENSIBLE) {
                fmtSize = 40;
            }
            if (mDataSource->readAt(offset, formatSpec, fmtSize) < fmtSize) {
                return NO_INIT;
            }

            mNumChannels = U16_LE_AT(&formatSpec[2]);

            if (mNumChannels < 1 || mNumChannels > FCC_8) {
                ALOGE("Unsupported number of channels (%d)", mNumChannels);
                return AMEDIA_ERROR_UNSUPPORTED;
            }

            if (mWaveFormat != WAVE_FORMAT_EXTENSIBLE) {
                if (mNumChannels != 1 && mNumChannels != FCC_2) {
                    ALOGW("More than 2 channels (%d) in non-WAVE_EXT, unknown channel mask",
                            mNumChannels);
                }
            }

            mSampleRate = U32_LE_AT(&formatSpec[4]);

            if (mSampleRate == 0) {
                return ERROR_MALFORMED;
            }

            mBitsPerSample = U16_LE_AT(&formatSpec[14]);

            if (mWaveFormat == WAVE_FORMAT_EXTENSIBLE) {
                uint16_t validBitsPerSample = U16_LE_AT(&formatSpec[18]);
                if (validBitsPerSample != mBitsPerSample) {
                    if (validBitsPerSample != 0) {
                        ALOGE("validBits(%d) != bitsPerSample(%d) are not supported",
                                validBitsPerSample, mBitsPerSample);
                        return AMEDIA_ERROR_UNSUPPORTED;
                    } else {
                        // we only support valitBitsPerSample == bitsPerSample but some WAV_EXT
                        // writers don't correctly set the valid bits value, and leave it at 0.
                        ALOGW("WAVE_EXT has 0 valid bits per sample, ignoring");
                    }
                }

                mChannelMask = U32_LE_AT(&formatSpec[20]);
                ALOGV("numChannels=%d channelMask=0x%x", mNumChannels, mChannelMask);
                if ((mChannelMask >> 18) != 0) {
                    ALOGE("invalid channel mask 0x%x", mChannelMask);
                    return ERROR_MALFORMED;
                }

                if ((mChannelMask != CHANNEL_MASK_USE_CHANNEL_ORDER)
                        && (popcount(mChannelMask) != mNumChannels)) {
                    ALOGE("invalid number of channels (%d) in channel mask (0x%x)",
                            popcount(mChannelMask), mChannelMask);
                    return ERROR_MALFORMED;
                }

                // In a WAVE_EXT header, the first two bytes of the GUID stored at byte 24 contain
                // the sample format, using the same definitions as a regular WAV header
                mWaveFormat = U16_LE_AT(&formatSpec[24]);
                if (memcmp(&formatSpec[26], WAVEEXT_SUBFORMAT, 14) &&
                    memcmp(&formatSpec[26], AMBISONIC_SUBFORMAT, 14)) {
                    ALOGE("unsupported GUID");
                    return ERROR_UNSUPPORTED;
                }
            }

            if (mWaveFormat == WAVE_FORMAT_PCM) {
                if (mBitsPerSample != 8 && mBitsPerSample != 16
                    && mBitsPerSample != 24 && mBitsPerSample != 32) {
                    return ERROR_UNSUPPORTED;
                }
            } else if (mWaveFormat == WAVE_FORMAT_IEEE_FLOAT) {
                if (mBitsPerSample != 32) {  // TODO we don't support double
                    return ERROR_UNSUPPORTED;
                }
            }
            else if (mWaveFormat == WAVE_FORMAT_MSGSM) {
                if (mBitsPerSample != 0) {
                    return ERROR_UNSUPPORTED;
                }
            } else if (mWaveFormat == WAVE_FORMAT_MULAW || mWaveFormat == WAVE_FORMAT_ALAW) {
                if (mBitsPerSample != 8) {
                    return ERROR_UNSUPPORTED;
                }
            } else {
                return ERROR_UNSUPPORTED;
            }

            mValidFormat = true;
        } else if (!memcmp(chunkHeader, "data", 4)) {
            if (mValidFormat) {
                mDataOffset = offset;
                mDataSize = chunkSize;

                AMediaFormat_clear(mTrackMeta);

                switch (mWaveFormat) {
                    case WAVE_FORMAT_PCM:
                    case WAVE_FORMAT_IEEE_FLOAT:
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_RAW);
                        break;
                    case WAVE_FORMAT_ALAW:
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_G711_ALAW);
                        break;
                    case WAVE_FORMAT_MSGSM:
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MSGSM);
                        break;
                    default:
                        CHECK_EQ(mWaveFormat, (uint16_t)WAVE_FORMAT_MULAW);
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_G711_MLAW);
                        break;
                }

                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, mNumChannels);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_CHANNEL_MASK, mChannelMask);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE, mSampleRate);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, mBitsPerSample);
                int64_t durationUs = 0;
                if (mWaveFormat == WAVE_FORMAT_MSGSM) {
                    // 65 bytes decode to 320 8kHz samples
                    durationUs =
                        1000000LL * (mDataSize / 65 * 320) / 8000;
                } else {
                    size_t bytesPerSample = mBitsPerSample >> 3;

                    if (!bytesPerSample || !mNumChannels)
                        return AMEDIA_ERROR_MALFORMED;

                    size_t num_samples = mDataSize / (mNumChannels * bytesPerSample);

                    if (!mSampleRate)
                        return AMEDIA_ERROR_MALFORMED;

                    durationUs =
                        1000000LL * num_samples / mSampleRate;
                }

                AMediaFormat_setInt64(mTrackMeta, AMEDIAFORMAT_KEY_DURATION, durationUs);

                return OK;
            }
        }

        offset += chunkSize;
    }

    return NO_INIT;
}

const size_t WAVSource::kMaxFrameSize = 32768;

WAVSource::WAVSource(
        DataSourceHelper *dataSource,
        AMediaFormat *meta,
        uint16_t waveFormat,
        bool outputFloat,
        off64_t offset, size_t size)
    : mDataSource(dataSource),
      mMeta(meta),
      mWaveFormat(waveFormat),
      mOutputFloat(outputFloat),
      mOffset(offset),
      mSize(size),
      mStarted(false) {
    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE, &mSampleRate));
    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &mNumChannels));
    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, &mBitsPerSample));
}

WAVSource::~WAVSource() {
    if (mStarted) {
        stop();
    }
}

media_status_t WAVSource::start() {
    ALOGV("WAVSource::start");

    CHECK(!mStarted);

    // some WAV files may have large audio buffers that use shared memory transfer.
    if (!mBufferGroup->init(4 /* buffers */, kMaxFrameSize)) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    mCurrentPos = mOffset;

    mStarted = true;

    return AMEDIA_OK;
}

media_status_t WAVSource::stop() {
    ALOGV("WAVSource::stop");

    CHECK(mStarted);

    mStarted = false;

    return AMEDIA_OK;
}

media_status_t WAVSource::getFormat(AMediaFormat *meta) {
    ALOGV("WAVSource::getFormat");

    const media_status_t status = AMediaFormat_copy(meta, mMeta);
    if (status == OK) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, kMaxFrameSize);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING,
                mOutputFloat ? kAudioEncodingPcmFloat : kAudioEncodingPcm16bit);
    }
    return status;
}

media_status_t WAVSource::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    *out = NULL;

    if (options != nullptr && options->getNonBlocking() && !mBufferGroup->has_buffers()) {
        return AMEDIA_ERROR_WOULD_BLOCK;
    }

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode)) {
        int64_t pos = 0;

        if (mWaveFormat == WAVE_FORMAT_MSGSM) {
            // 65 bytes decode to 320 8kHz samples
            int64_t samplenumber = (seekTimeUs * mSampleRate) / 1000000;
            int64_t framenumber = samplenumber / 320;
            pos = framenumber * 65;
        } else {
            pos = (seekTimeUs * mSampleRate) / 1000000 * mNumChannels * (mBitsPerSample >> 3);
        }
        if (pos > (off64_t)mSize) {
            pos = mSize;
        }
        mCurrentPos = pos + mOffset;
    }

    MediaBufferHelper *buffer;
    media_status_t err = mBufferGroup->acquire_buffer(&buffer);
    if (err != OK) {
        return err;
    }

    // maxBytesToRead may be reduced so that in-place data conversion will fit in buffer size.
    const size_t bufferSize = std::min(buffer->size(), kMaxFrameSize);
    size_t maxBytesToRead;
    if (mOutputFloat) { // destination is float at 4 bytes per sample, source may be less.
        maxBytesToRead = (mBitsPerSample / 8) * (bufferSize / 4);
    } else { // destination is int16_t at 2 bytes per sample, only source of 8 bits is less.
        maxBytesToRead = mBitsPerSample == 8 ? bufferSize / 2 : bufferSize;
    }

    const size_t maxBytesAvailable =
        (mCurrentPos - mOffset >= (off64_t)mSize)
            ? 0 : mSize - (mCurrentPos - mOffset);

    if (maxBytesToRead > maxBytesAvailable) {
        maxBytesToRead = maxBytesAvailable;
    }

    if (mWaveFormat == WAVE_FORMAT_MSGSM) {
        // Microsoft packs 2 frames into 65 bytes, rather than using separate 33-byte frames,
        // so read multiples of 65, and use smaller buffers to account for ~10:1 expansion ratio
        if (maxBytesToRead > 1024) {
            maxBytesToRead = 1024;
        }
        maxBytesToRead = (maxBytesToRead / 65) * 65;
    } else {
        // read only integral amounts of audio unit frames.
        const size_t inputUnitFrameSize = mNumChannels * mBitsPerSample / 8;
        maxBytesToRead -= maxBytesToRead % inputUnitFrameSize;
    }

    ssize_t n = mDataSource->readAt(
            mCurrentPos, buffer->data(),
            maxBytesToRead);

    if (n <= 0) {
        buffer->release();
        buffer = NULL;

        return AMEDIA_ERROR_END_OF_STREAM;
    }

    buffer->set_range(0, n);

    // TODO: add capability to return data as float PCM instead of 16 bit PCM.
    if (mWaveFormat == WAVE_FORMAT_PCM) {
        const size_t bytesPerFrame = (mBitsPerSample >> 3) * mNumChannels;
        const size_t numFrames = n / bytesPerFrame;
        const size_t numSamples = numFrames * mNumChannels;
        if (mOutputFloat) {
            float *fdest = (float *)buffer->data();
            buffer->set_range(0, 4 * numSamples);
            switch (mBitsPerSample) {
            case 8: {
                memcpy_to_float_from_u8(fdest, (const uint8_t *)buffer->data(), numSamples);
            } break;
            case 16: {
                memcpy_to_float_from_i16(fdest, (const int16_t *)buffer->data(), numSamples);
            } break;
            case 24: {
                memcpy_to_float_from_p24(fdest, (const uint8_t *)buffer->data(), numSamples);
            } break;
            case 32: { // buffer range is correct
                memcpy_to_float_from_i32(fdest, (const int32_t *)buffer->data(), numSamples);
            } break;
            }
        } else {
            int16_t *idest = (int16_t *)buffer->data();
            buffer->set_range(0, 2 * numSamples);
            switch (mBitsPerSample) {
            case 8: {
                memcpy_to_i16_from_u8(idest, (const uint8_t *)buffer->data(), numSamples);
            } break;
            case 16:
                // no conversion needed
                break;
            case 24: {
                memcpy_to_i16_from_p24(idest, (const uint8_t *)buffer->data(), numSamples);
            } break;
            case 32: {
                memcpy_to_i16_from_i32(idest, (const int32_t *)buffer->data(), numSamples);
            } break;
            }
        }
    } else if (mWaveFormat == WAVE_FORMAT_IEEE_FLOAT) {
        if (!mOutputFloat) { // mBitsPerSample == 32
            int16_t *idest = (int16_t *)buffer->data();
            const size_t numSamples = n / 4;
            memcpy_to_i16_from_float(idest, (const float *)buffer->data(), numSamples);
            buffer->set_range(0, 2 * numSamples);
        }
        // Note: if output encoding is float, no need to convert if source is float.
    }

    int64_t timeStampUs = 0;

    if (mWaveFormat == WAVE_FORMAT_MSGSM) {
        timeStampUs = 1000000LL * (mCurrentPos - mOffset) * 320 / 65 / mSampleRate;
    } else {
        size_t bytesPerSample = mBitsPerSample >> 3;
        timeStampUs = 1000000LL * (mCurrentPos - mOffset)
                / (mNumChannels * bytesPerSample) / mSampleRate;
    }

    AMediaFormat *meta = buffer->meta_data();
    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, timeStampUs);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);

    mCurrentPos += n;

    *out = buffer;

    return AMEDIA_OK;
}

////////////////////////////////////////////////////////////////////////////////

static CMediaExtractor* CreateExtractor(
        CDataSource *source,
        void *) {
    return wrap(new WAVExtractor(new DataSourceHelper(source)));
}

static CreatorFunc Sniff(
        CDataSource *source,
        float *confidence,
        void **,
        FreeMetaFunc *) {
    DataSourceHelper *helper = new DataSourceHelper(source);
    char header[12];
    if (helper->readAt(0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        delete helper;
        return NULL;
    }

    if (memcmp(header, "RIFF", 4) || memcmp(&header[8], "WAVE", 4)) {
        delete helper;
        return NULL;
    }

    WAVExtractor *extractor = new WAVExtractor(helper); // extractor owns the helper
    int numTracks = extractor->countTracks();
    delete extractor;
    if (numTracks == 0) {
        return NULL;
    }

    *confidence = 0.3f;

    return CreateExtractor;
}

static const char *extensions[] = {
    "wav",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("7d613858-5837-4a38-84c5-332d1cddee27"),
        1, // version
        "WAV Extractor",
        { .v3 = {Sniff, extensions} },
    };
}

} // extern "C"

} // namespace android
