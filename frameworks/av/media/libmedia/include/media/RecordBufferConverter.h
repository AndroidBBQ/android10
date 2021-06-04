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

#ifndef ANDROID_RECORD_BUFFER_CONVERTER_H
#define ANDROID_RECORD_BUFFER_CONVERTER_H

#include <stdint.h>
#include <sys/types.h>

#include <media/AudioBufferProvider.h>
#include <system/audio.h>

class AudioResampler;
class PassthruBufferProvider;

namespace android {

/* The RecordBufferConverter is used for format, channel, and sample rate
 * conversion for a RecordTrack.
 *
 * RecordBufferConverter uses the convert() method rather than exposing a
 * buffer provider interface; this is to save a memory copy.
 *
 * There are legacy conversion requirements for this converter, specifically
 * due to mono handling, so be careful about modifying.
 *
 * Original source audioflinger/Threads.{h,cpp}
 */
class RecordBufferConverter
{
public:
    RecordBufferConverter(
            audio_channel_mask_t srcChannelMask, audio_format_t srcFormat,
            uint32_t srcSampleRate,
            audio_channel_mask_t dstChannelMask, audio_format_t dstFormat,
            uint32_t dstSampleRate);

    ~RecordBufferConverter();

    /* Converts input data from an AudioBufferProvider by format, channelMask,
     * and sampleRate to a destination buffer.
     *
     * Parameters
     *      dst:  buffer to place the converted data.
     * provider:  buffer provider to obtain source data.
     *   frames:  number of frames to convert
     *
     * Returns the number of frames converted.
     */
    size_t convert(void *dst, AudioBufferProvider *provider, size_t frames);

    // returns NO_ERROR if constructor was successful
    status_t initCheck() const {
        // mSrcChannelMask set on successful updateParameters
        return mSrcChannelMask != AUDIO_CHANNEL_INVALID ? NO_ERROR : NO_INIT;
    }

    // allows dynamic reconfigure of all parameters
    status_t updateParameters(
            audio_channel_mask_t srcChannelMask, audio_format_t srcFormat,
            uint32_t srcSampleRate,
            audio_channel_mask_t dstChannelMask, audio_format_t dstFormat,
            uint32_t dstSampleRate);

    // called to reset resampler buffers on record track discontinuity
    void reset();

private:
    // format conversion when not using resampler
    void convertNoResampler(void *dst, const void *src, size_t frames);

    // format conversion when using resampler; modifies src in-place
    void convertResampler(void *dst, /*not-a-const*/ void *src, size_t frames);

    // user provided information
    audio_channel_mask_t mSrcChannelMask;
    audio_format_t       mSrcFormat;
    uint32_t             mSrcSampleRate;
    audio_channel_mask_t mDstChannelMask;
    audio_format_t       mDstFormat;
    uint32_t             mDstSampleRate;

    // derived information
    uint32_t             mSrcChannelCount;
    uint32_t             mDstChannelCount;
    size_t               mDstFrameSize;

    // format conversion buffer
    void                *mBuf;
    size_t               mBufFrames;
    size_t               mBufFrameSize;

    // resampler info
    AudioResampler      *mResampler;

    bool                 mIsLegacyDownmix;  // legacy stereo to mono conversion needed
    bool                 mIsLegacyUpmix;    // legacy mono to stereo conversion needed
    bool                 mRequiresFloat;    // data processing requires float (e.g. resampler)
    PassthruBufferProvider *mInputConverterProvider;    // converts input to float
    int8_t               mIdxAry[sizeof(uint32_t) * 8]; // used for channel mask conversion
};

// ----------------------------------------------------------------------------
} // namespace android

#endif // ANDROID_RECORD_BUFFER_CONVERTER_H
