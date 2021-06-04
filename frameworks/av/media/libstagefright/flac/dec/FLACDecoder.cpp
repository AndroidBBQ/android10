/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "FLACDecoder"
#include <utils/Log.h>

#include "FLACDecoder.h"

#include <audio_utils/primitives.h> // float_from_i32
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>

namespace android {

// These are the corresponding callbacks with C++ calling conventions
FLAC__StreamDecoderReadStatus FLACDecoder::readCallback(
        FLAC__byte buffer[], size_t *bytes) {
    if (mBuffer == nullptr || mBufferLen == 0) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    size_t actual = *bytes;
    if (actual > mBufferDataSize - mBufferPos) {
        actual = mBufferDataSize - mBufferPos;
    }
    memcpy(buffer, mBuffer + mBufferPos, actual);
    mBufferPos += actual;
    *bytes = actual;
    return (actual == 0 ? FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM
                        : FLAC__STREAM_DECODER_READ_STATUS_CONTINUE);
}

FLAC__StreamDecoderWriteStatus FLACDecoder::writeCallback(
        const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    if (!mWriteRequested) {
        ALOGE("writeCallback: unexpected");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    mWriteRequested = false;
    // FLAC decoder doesn't free or realloc buffer until next frame or finish
    mWriteHeader = frame->header;
    memmove(mWriteBuffer, buffer, sizeof(const FLAC__int32 * const) * getChannels());
    mWriteCompleted = true;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FLACDecoder::metadataCallback(const FLAC__StreamMetadata *metadata)
{
    switch (metadata->type) {
        case FLAC__METADATA_TYPE_STREAMINFO:
        {
            if (mStreamInfoValid) {
                ALOGE("metadataCallback: unexpected STREAMINFO");
            } else {
                mStreamInfo = metadata->data.stream_info;
                mStreamInfoValid = true;
            }
            break;
        }

        /* TODO: enable metadata parsing below.
        case FLAC__METADATA_TYPE_VORBIS_COMMENT:
        {
            const FLAC__StreamMetadata_VorbisComment *vc;
            vc = &metadata->data.vorbis_comment;
            for (FLAC__uint32 i = 0; i < vc->num_comments; ++i) {
                FLAC__StreamMetadata_VorbisComment_Entry *vce;
                vce = &vc->comments[i];
                if (mFileMetadata != 0 && vce->entry != NULL) {
                    parseVorbisComment(mFileMetadata, (const char *) vce->entry,
                            vce->length);
                }
            }
            break;
        }

        case FLAC__METADATA_TYPE_PICTURE:
        {
            if (mFileMetadata != 0) {
                const FLAC__StreamMetadata_Picture *p = &metadata->data.picture;
                mFileMetadata->setData(kKeyAlbumArt,
                        MetaData::TYPE_NONE, p->data, p->data_length);
                mFileMetadata->setCString(kKeyAlbumArtMIME, p->mime_type);
            }
            break;
        }
        */

        default:
            ALOGW("metadataCallback: unexpected type %u", metadata->type);
            break;
    }
}

void FLACDecoder::errorCallback(FLAC__StreamDecoderErrorStatus status)
{
    ALOGE("errorCallback: status=%d", status);
    mErrorStatus = status;
}

// Copy samples from FLAC native 32-bit non-interleaved to 16-bit signed
// or 32-bit float interleaved.
// TODO: Consider moving to audio_utils.  See similar code at FLACExtractor.cpp
// These are candidates for optimization if needed.
static void copyTo16Signed(
        short *dst,
        const int *const *src,
        unsigned nSamples,
        unsigned nChannels,
        unsigned bitsPerSample) {
    const int leftShift = 16 - (int)bitsPerSample; // cast to int to prevent unsigned overflow.
    if (leftShift >= 0) {
        for (unsigned i = 0; i < nSamples; ++i) {
            for (unsigned c = 0; c < nChannels; ++c) {
                *dst++ = src[c][i] << leftShift;
            }
        }
    } else {
        const int rightShift = -leftShift;
        for (unsigned i = 0; i < nSamples; ++i) {
            for (unsigned c = 0; c < nChannels; ++c) {
                *dst++ = src[c][i] >> rightShift;
            }
        }
    }
}

static void copyToFloat(
        float *dst,
        const int *const *src,
        unsigned nSamples,
        unsigned nChannels,
        unsigned bitsPerSample) {
    const unsigned leftShift = 32 - bitsPerSample;
    for (unsigned i = 0; i < nSamples; ++i) {
        for (unsigned c = 0; c < nChannels; ++c) {
            *dst++ = float_from_i32(src[c][i] << leftShift);
        }
    }
}

// static
FLACDecoder *FLACDecoder::Create() {
    FLACDecoder *decoder = new (std::nothrow) FLACDecoder();
    if (decoder == NULL || decoder->init() != OK) {
        delete decoder;
        return NULL;
    }
    return decoder;
}

FLACDecoder::FLACDecoder()
    : mDecoder(NULL),
      mBuffer(NULL),
      mBufferLen(0),
      mBufferPos(0),
      mBufferDataSize(0),
      mStreamInfoValid(false),
      mWriteRequested(false),
      mWriteCompleted(false),
      mErrorStatus((FLAC__StreamDecoderErrorStatus) -1) {
    ALOGV("ctor:");
    memset(&mStreamInfo, 0, sizeof(mStreamInfo));
    memset(&mWriteHeader, 0, sizeof(mWriteHeader));
    memset(&mWriteBuffer, 0, sizeof(mWriteBuffer));
}

FLACDecoder::~FLACDecoder() {
    ALOGV("dtor:");
    if (mDecoder != NULL) {
        FLAC__stream_decoder_delete(mDecoder);
        mDecoder = NULL;
    }
    if (mBuffer != NULL) {
        free(mBuffer);
    }
}

status_t FLACDecoder::init() {
    ALOGV("init:");
    // setup libFLAC stream decoder
    mDecoder = FLAC__stream_decoder_new();
    if (mDecoder == NULL) {
        ALOGE("init: failed to create libFLAC stream decoder");
        return NO_INIT;
    }
    FLAC__stream_decoder_set_md5_checking(mDecoder, false);
    FLAC__stream_decoder_set_metadata_ignore_all(mDecoder);
    FLAC__stream_decoder_set_metadata_respond(
            mDecoder, FLAC__METADATA_TYPE_STREAMINFO);
    /*
    FLAC__stream_decoder_set_metadata_respond(
            mDecoder, FLAC__METADATA_TYPE_PICTURE);
    FLAC__stream_decoder_set_metadata_respond(
            mDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
    */
    static auto read_callback =
        [] (const FLAC__StreamDecoder * /* decoder */,
            FLAC__byte buffer[],
            size_t *bytes,
            void *client_data) -> FLAC__StreamDecoderReadStatus {
            return ((FLACDecoder *) client_data)->readCallback(buffer, bytes); };

    static auto write_callback =
        [] (const FLAC__StreamDecoder * /* decoder */,
            const FLAC__Frame *frame,
            const FLAC__int32 * const buffer[],
            void *client_data) -> FLAC__StreamDecoderWriteStatus {
            return ((FLACDecoder *) client_data)->writeCallback(frame, buffer); };

    static auto metadata_callback =
        [] (const FLAC__StreamDecoder * /* decoder */,
            const FLAC__StreamMetadata *metadata,
            void *client_data) {
            ((FLACDecoder *) client_data)->metadataCallback(metadata); };

    static auto error_callback =
        [] (const FLAC__StreamDecoder * /* decoder */,
            FLAC__StreamDecoderErrorStatus status,
            void *client_data) {
            ((FLACDecoder *) client_data)->errorCallback(status); };

    FLAC__StreamDecoderInitStatus initStatus =
        FLAC__stream_decoder_init_stream(
                mDecoder,
                read_callback,
                NULL /* seek_callback */,
                NULL /* tell_callback */,
                NULL /* length_callback */,
                NULL /* eof_callback */,
                write_callback,
                metadata_callback,
                error_callback,
                (void *)this);
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        ALOGE("init: init_stream failed, returned %d", initStatus);
        return NO_INIT;
    }
    return OK;
}

void FLACDecoder::flush() {
    ALOGV("flush:");
    mBufferPos = 0;
    mBufferDataSize = 0;
    mStreamInfoValid = false;
    if (!FLAC__stream_decoder_reset(mDecoder)) {
        ALOGE("flush: failed to reset FLAC stream decoder");
    }
}

status_t FLACDecoder::parseMetadata(const uint8_t *inBuffer, size_t inBufferLen) {
    ALOGV("parseMetadata: input size(%zu)", inBufferLen);
    //hexdump(inBuffer, inBufferLen);

    if (mStreamInfoValid) {
        ALOGE("parseMetadata: already have full metadata blocks");
        return ERROR_MALFORMED;
    }

    status_t err = addDataToBuffer(inBuffer, inBufferLen);
    if (err != OK) {
        ALOGE("parseMetadata: addDataToBuffer returns error %d", err);
        return err;
    }

    if (!FLAC__stream_decoder_process_until_end_of_metadata(mDecoder)) {
        if (!FLAC__stream_decoder_reset(mDecoder)) {
            ALOGE("parseMetadata: failed to reset FLAC stream decoder");
            return FAILED_TRANSACTION;
        }
        mBufferPos = 0;
        ALOGV("parseMetadata: do not have full metadata blocks yet");
        return WOULD_BLOCK;
    }

    if (!mStreamInfoValid) {
        ALOGE("parseMetadata: missing STREAMINFO");
        return ERROR_MALFORMED;
    }

    // check block size
    if (getMaxBlockSize() == 0) {
        ALOGE("wrong max blocksize %u", getMaxBlockSize());
        mStreamInfoValid = false;
        return ERROR_MALFORMED;
    }

    // check channel count
    if (getChannels() == 0 || getChannels() > kMaxChannels) {
        ALOGE("unsupported channel count %u", getChannels());
        mStreamInfoValid = false;
        return ERROR_MALFORMED;
    }

    // check bit depth
    switch (getBitsPerSample()) {
        case 8:
        case 16:
        case 24:
        case 32: // generally rare, but is supported in the framework
            break;

        default:
            ALOGE("parseMetadata: unsupported bits per sample %u", getBitsPerSample());
            mStreamInfoValid = false;
            return ERROR_MALFORMED;
    }

    // Now we have all metadata blocks.
    mBufferPos = 0;
    mBufferDataSize = 0;

    return OK;
}

status_t FLACDecoder::decodeOneFrame(const uint8_t *inBuffer, size_t inBufferLen,
        void *outBuffer, size_t *outBufferLen, bool outputFloat) {
    ALOGV("decodeOneFrame: input size(%zu)", inBufferLen);

    if (!mStreamInfoValid) {
        ALOGW("decodeOneFrame: no streaminfo metadata block");
    }

    if (inBufferLen != 0) {
        status_t err = addDataToBuffer(inBuffer, inBufferLen);
        if (err != OK) {
            ALOGW("decodeOneFrame: addDataToBuffer returns error %d", err);
            return err;
        }
    }

    mWriteRequested = true;
    mWriteCompleted = false;
    if (!FLAC__stream_decoder_process_single(mDecoder)) {
        ALOGE("decodeOneFrame: process_single failed");
        return ERROR_MALFORMED;
    }
    if (!mWriteCompleted) {
        ALOGV("decodeOneFrame: write did not complete");
        if (outBufferLen) {
            *outBufferLen = 0;
        }
        return OK;
    }

    // frame header should be consistent with STREAMINFO
    unsigned blocksize = mWriteHeader.blocksize;
    if (blocksize == 0 || blocksize > getMaxBlockSize()) {
        ALOGE("decodeOneFrame: write invalid blocksize %u", blocksize);
        return ERROR_MALFORMED;
    }
    if (mWriteHeader.sample_rate != getSampleRate() ||
        mWriteHeader.channels != getChannels() ||
        mWriteHeader.bits_per_sample != getBitsPerSample()) {
        ALOGE("decodeOneFrame: parameters are changed mid-stream: %d/%d/%d -> %d/%d/%d",
                getSampleRate(), getChannels(), getBitsPerSample(),
                mWriteHeader.sample_rate, mWriteHeader.channels, mWriteHeader.bits_per_sample);
        return ERROR_MALFORMED;
    }
    if (mWriteHeader.number_type != FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) {
        ALOGE("decodeOneFrame: number type is %d, expected %d",
                mWriteHeader.number_type, FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
        return ERROR_MALFORMED;
    }

    const unsigned channels = getChannels();
    const size_t sampleSize = outputFloat ? sizeof(float) : sizeof(int16_t);
    const size_t frameSize = channels * sampleSize;
    size_t bufferSize = blocksize * frameSize;
    if (bufferSize > *outBufferLen) {
        ALOGW("decodeOneFrame: output buffer holds only partial frame %zu:%zu",
                *outBufferLen, bufferSize);
        blocksize = *outBufferLen / frameSize;
        bufferSize = blocksize * frameSize;
    }

    // copy PCM from FLAC write buffer to output buffer, with interleaving

    const unsigned bitsPerSample = getBitsPerSample();
    if (outputFloat) {
        copyToFloat(reinterpret_cast<float*>(outBuffer),
                    mWriteBuffer,
                    blocksize,
                    channels,
                    bitsPerSample);
    } else {
        copyTo16Signed(reinterpret_cast<short*>(outBuffer),
                       mWriteBuffer,
                       blocksize,
                       channels,
                       bitsPerSample);
    }
    *outBufferLen = bufferSize;
    return OK;
}

status_t FLACDecoder::addDataToBuffer(const uint8_t *inBuffer, size_t inBufferLen) {
    // mBufferPos should be no larger than mBufferDataSize
    if (inBufferLen > SIZE_MAX - (mBufferDataSize - mBufferPos)) {
        ALOGE("addDataToBuffer: input buffer is too large");
        return ERROR_MALFORMED;
    }

    if (inBufferLen > mBufferLen - mBufferDataSize) {
        if (mBufferPos > 0) {
            memmove(mBuffer, mBuffer + mBufferPos, mBufferDataSize - mBufferPos);
            mBufferDataSize -= mBufferPos;
            mBufferPos = 0;
        }
        if (inBufferLen > mBufferLen - mBufferDataSize) {
            mBuffer = (uint8_t*)realloc(mBuffer, mBufferDataSize + inBufferLen);
            if (mBuffer == nullptr) {
                mBufferDataSize = 0;
                mBufferLen = 0;
                ALOGE("decodeOneFrame: failed to allocate memory for input buffer");
                return NO_MEMORY;
            }
            mBufferLen = mBufferDataSize + inBufferLen;
        }
    }

    memcpy(mBuffer + mBufferDataSize, inBuffer, inBufferLen);
    mBufferDataSize += inBufferLen;
    return OK;
}

}  // namespace android
