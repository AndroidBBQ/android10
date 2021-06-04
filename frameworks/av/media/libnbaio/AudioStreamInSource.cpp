/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "AudioStreamInSource"
//#define LOG_NDEBUG 0

#include <cutils/compiler.h>
#include <utils/Log.h>
#include <media/audiohal/StreamHalInterface.h>
#include <media/nbaio/AudioStreamInSource.h>

namespace android {

AudioStreamInSource::AudioStreamInSource(sp<StreamInHalInterface> stream) :
        NBAIO_Source(),
        mStream(stream),
        mStreamBufferSizeBytes(0),
        mFramesOverrun(0),
        mOverruns(0)
{
    ALOG_ASSERT(stream != 0);
}

AudioStreamInSource::~AudioStreamInSource()
{
    mStream.clear();
}

ssize_t AudioStreamInSource::negotiate(const NBAIO_Format offers[], size_t numOffers,
                                      NBAIO_Format counterOffers[], size_t& numCounterOffers)
{
    if (!Format_isValid(mFormat)) {
        status_t result;
        result = mStream->getBufferSize(&mStreamBufferSizeBytes);
        if (result != OK) return result;
        audio_format_t streamFormat;
        uint32_t sampleRate;
        audio_channel_mask_t channelMask;
        result = mStream->getAudioProperties(&sampleRate, &channelMask, &streamFormat);
        if (result != OK) return result;
        mFormat = Format_from_SR_C(sampleRate,
                audio_channel_count_from_in_mask(channelMask), streamFormat);
        mFrameSize = Format_frameSize(mFormat);
    }
    return NBAIO_Source::negotiate(offers, numOffers, counterOffers, numCounterOffers);
}

int64_t AudioStreamInSource::framesOverrun()
{
    uint32_t framesOverrun;
    status_t result = mStream->getInputFramesLost(&framesOverrun);
    if (result == OK && framesOverrun > 0) {
        mFramesOverrun += framesOverrun;
        // FIXME only increment for contiguous ranges
        ++mOverruns;
    } else if (result != OK) {
        ALOGE("Error when retrieving lost frames count from HAL: %d", result);
    }
    return mFramesOverrun;
}

ssize_t AudioStreamInSource::read(void *buffer, size_t count)
{
    if (CC_UNLIKELY(!Format_isValid(mFormat))) {
        return NEGOTIATE;
    }
    size_t bytesRead;
    status_t result = mStream->read(buffer, count * mFrameSize, &bytesRead);
    if (result == OK && bytesRead > 0) {
        size_t framesRead = bytesRead / mFrameSize;
        mFramesRead += framesRead;
        return framesRead;
    } else {
        ALOGE_IF(result != OK, "Error while reading data from HAL: %d", result);
        return bytesRead;
    }
}

}   // namespace android
