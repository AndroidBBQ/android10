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

#define LOG_TAG "NBAIO"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <media/nbaio/NBAIO.h>

namespace android {

size_t Format_frameSize(const NBAIO_Format& format)
{
    return format.mFrameSize;
}

const NBAIO_Format Format_Invalid = { 0, 0, AUDIO_FORMAT_INVALID, 0 };

unsigned Format_sampleRate(const NBAIO_Format& format)
{
    if (!Format_isValid(format)) {
        return 0;
    }
    return format.mSampleRate;
}

unsigned Format_channelCount(const NBAIO_Format& format)
{
    if (!Format_isValid(format)) {
        return 0;
    }
    return format.mChannelCount;
}

NBAIO_Format Format_from_SR_C(unsigned sampleRate, unsigned channelCount,
        audio_format_t format)
{
    if (sampleRate == 0 || channelCount == 0 || !audio_is_valid_format(format)) {
        return Format_Invalid;
    }
    NBAIO_Format ret;
    ret.mSampleRate = sampleRate;
    ret.mChannelCount = channelCount;
    ret.mFormat = format;
    ret.mFrameSize = audio_is_linear_pcm(format) ?
            channelCount * audio_bytes_per_sample(format) : sizeof(uint8_t);
    return ret;
}

// This is a default implementation; it is expected that subclasses will optimize this.
ssize_t NBAIO_Sink::writeVia(writeVia_t via, size_t total, void *user, size_t block)
{
    if (!mNegotiated) {
        return (ssize_t) NEGOTIATE;
    }
    static const size_t maxBlock = 32;
    size_t frameSize = Format_frameSize(mFormat);
    ALOG_ASSERT(frameSize > 0 && frameSize <= 8);
    // double guarantees alignment for stack similar to what malloc() gives for heap
    if (block == 0 || block > maxBlock) {
        block = maxBlock;
    }
    double buffer[((frameSize * block) + sizeof(double) - 1) / sizeof(double)];
    size_t accumulator = 0;
    while (accumulator < total) {
        size_t count = total - accumulator;
        if (count > block) {
            count = block;
        }
        ssize_t ret = via(user, buffer, count);
        if (ret > 0) {
            ALOG_ASSERT((size_t) ret <= count);
            size_t maxRet = ret;
            ret = write(buffer, maxRet);
            if (ret > 0) {
                ALOG_ASSERT((size_t) ret <= maxRet);
                accumulator += ret;
                continue;
            }
        }
        return accumulator > 0 ? accumulator : ret;
    }
    return accumulator;
}

// This is a default implementation; it is expected that subclasses will optimize this.
ssize_t NBAIO_Source::readVia(readVia_t via, size_t total, void *user, size_t block)
{
    if (!mNegotiated) {
        return (ssize_t) NEGOTIATE;
    }
    static const size_t maxBlock = 32;
    size_t frameSize = Format_frameSize(mFormat);
    ALOG_ASSERT(frameSize > 0 && frameSize <= 8);
    // double guarantees alignment for stack similar to what malloc() gives for heap
    if (block == 0 || block > maxBlock) {
        block = maxBlock;
    }
    double buffer[((frameSize * block) + sizeof(double) - 1) / sizeof(double)];
    size_t accumulator = 0;
    while (accumulator < total) {
        size_t count = total - accumulator;
        if (count > block) {
            count = block;
        }
        ssize_t ret = read(buffer, count);
        if (ret > 0) {
            ALOG_ASSERT((size_t) ret <= count);
            size_t maxRet = ret;
            ret = via(user, buffer, maxRet);
            if (ret > 0) {
                ALOG_ASSERT((size_t) ret <= maxRet);
                accumulator += ret;
                continue;
            }
        }
        return accumulator > 0 ? accumulator : ret;
    }
    return accumulator;
}

// Default implementation that only accepts my mFormat
ssize_t NBAIO_Port::negotiate(const NBAIO_Format offers[], size_t numOffers,
                                  NBAIO_Format counterOffers[], size_t& numCounterOffers)
{
    ALOGV("negotiate offers=%p numOffers=%zu countersOffers=%p numCounterOffers=%zu",
            offers, numOffers, counterOffers, numCounterOffers);
    if (Format_isValid(mFormat)) {
        for (size_t i = 0; i < numOffers; ++i) {
            if (Format_isEqual(offers[i], mFormat)) {
                mNegotiated = true;
                return i;
            }
        }
        if (numCounterOffers > 0) {
            counterOffers[0] = mFormat;
        }
        numCounterOffers = 1;
    } else {
        numCounterOffers = 0;
    }
    return (ssize_t) NEGOTIATE;
}

bool Format_isValid(const NBAIO_Format& format)
{
    return format.mSampleRate != 0 && format.mChannelCount != 0 &&
            format.mFormat != AUDIO_FORMAT_INVALID && format.mFrameSize != 0;
}

bool Format_isEqual(const NBAIO_Format& format1, const NBAIO_Format& format2)
{
    return format1.mSampleRate == format2.mSampleRate &&
            format1.mChannelCount == format2.mChannelCount && format1.mFormat == format2.mFormat &&
            format1.mFrameSize == format2.mFrameSize;
}

}   // namespace android
