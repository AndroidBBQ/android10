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

#ifndef FLAC_DECODER_H_
#define FLAC_DECODER_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

#include "FLAC/stream_decoder.h"

namespace android {

// packet based FLAC decoder, wrapps libFLAC stream decoder.
class FLACDecoder {

public:
    enum {
        kMaxChannels = 8,
    };

    static FLACDecoder *Create();

    FLAC__StreamMetadata_StreamInfo getStreamInfo() const {
        return mStreamInfo;
    }

    status_t parseMetadata(const uint8_t *inBuffer, size_t inBufferLen);
    status_t decodeOneFrame(const uint8_t *inBuffer, size_t inBufferLen,
            void *outBuffer, size_t *outBufferLen, bool outputFloat = false);
    void flush();
    virtual ~FLACDecoder();

protected:
    FLACDecoder();

private:
    // stream properties
    unsigned getMaxBlockSize() const {
        return mStreamInfo.max_blocksize;
    }
    unsigned getSampleRate() const {
        return mStreamInfo.sample_rate;
    }
    unsigned getChannels() const {
        return mStreamInfo.channels;
    }
    unsigned getBitsPerSample() const {
        return mStreamInfo.bits_per_sample;
    }
    FLAC__uint64 getTotalSamples() const {
        return mStreamInfo.total_samples;
    }

    status_t addDataToBuffer(const uint8_t *inBuffer, size_t inBufferLen);

    FLAC__StreamDecoder *mDecoder;

    uint8_t *mBuffer;  // cache input bit stream data
    size_t mBufferLen;  // the memory size of |mBuffer|
    size_t mBufferPos;  // next byte to read in |mBuffer|
    // size of input data stored in |mBuffer|, always started at offset 0
    size_t mBufferDataSize;

    // cached when the STREAMINFO metadata is parsed by libFLAC
    FLAC__StreamMetadata_StreamInfo mStreamInfo;
    bool mStreamInfoValid;

    // cached when a decoded PCM block is "written" by libFLAC decoder
    bool mWriteRequested;
    bool mWriteCompleted;
    FLAC__FrameHeader mWriteHeader;
    FLAC__int32 const * mWriteBuffer[kMaxChannels];

    // most recent error reported by libFLAC decoder
    FLAC__StreamDecoderErrorStatus mErrorStatus;

    status_t init();

    // FLAC stream decoder callbacks as C++ instance methods
    FLAC__StreamDecoderReadStatus readCallback(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderWriteStatus writeCallback(
            const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
    void metadataCallback(const FLAC__StreamMetadata *metadata);
    void errorCallback(FLAC__StreamDecoderErrorStatus status);

    DISALLOW_EVIL_CONSTRUCTORS(FLACDecoder);
};

}  // namespace android

#endif  // FLAC_DECODER_H_
