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

#define LOG_TAG "AudioBufferProviderSource"
//#define LOG_NDEBUG 0

#include <cutils/compiler.h>
#include <utils/Log.h>
#include <media/nbaio/AudioBufferProviderSource.h>

namespace android {

AudioBufferProviderSource::AudioBufferProviderSource(AudioBufferProvider *provider,
                                                     const NBAIO_Format& format) :
    NBAIO_Source(format), mProvider(provider), mConsumed(0)
{
    ALOG_ASSERT(provider != NULL);
    ALOG_ASSERT(Format_isValid(format));
}

AudioBufferProviderSource::~AudioBufferProviderSource()
{
    if (mBuffer.raw != NULL) {
        mProvider->releaseBuffer(&mBuffer);
    }
}

ssize_t AudioBufferProviderSource::availableToRead()
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    return mBuffer.raw != NULL ? mBuffer.frameCount - mConsumed : 0;
}

ssize_t AudioBufferProviderSource::read(void *buffer, size_t count)
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    if (CC_UNLIKELY(mBuffer.raw == NULL)) {
        mBuffer.frameCount = count;
        status_t status = mProvider->getNextBuffer(&mBuffer);
        if (status != OK) {
            return status == NOT_ENOUGH_DATA ? (ssize_t) WOULD_BLOCK : (ssize_t) status;
        }
        ALOG_ASSERT(mBuffer.raw != NULL);
        // mConsumed is 0 either from constructor or after releaseBuffer()
    }
    size_t available = mBuffer.frameCount - mConsumed;
    if (CC_UNLIKELY(count > available)) {
        count = available;
    }
    // count could be zero, either because count was zero on entry or
    // available is zero, but both are unlikely so don't check for that
    memcpy(buffer, (char *) mBuffer.raw + (mConsumed * mFrameSize), count * mFrameSize);
    if (CC_UNLIKELY((mConsumed += count) >= mBuffer.frameCount)) {
        mProvider->releaseBuffer(&mBuffer);
        mBuffer.raw = NULL;
        mConsumed = 0;
    }
    mFramesRead += count;
    // For better responsiveness with large values of count,
    // return a short count rather than continuing with next buffer.
    // This gives the caller a chance to interpolate other actions.
    return count;
}

ssize_t AudioBufferProviderSource::readVia(readVia_t via, size_t total, void *user, size_t block)
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    if (CC_UNLIKELY(block == 0)) {
        block = ~0;
    }
    for (size_t accumulator = 0; ; ) {
        ALOG_ASSERT(accumulator <= total);
        size_t count = total - accumulator;
        if (CC_UNLIKELY(count == 0)) {
            return accumulator;
        }
        if (CC_LIKELY(count > block)) {
            count = block;
        }
        // 1 <= count <= block
        if (CC_UNLIKELY(mBuffer.raw == NULL)) {
            mBuffer.frameCount = count;
            status_t status = mProvider->getNextBuffer(&mBuffer);
            if (CC_LIKELY(status == OK)) {
                ALOG_ASSERT(mBuffer.raw != NULL && mBuffer.frameCount <= count);
                // mConsumed is 0 either from constructor or after releaseBuffer()
                continue;
            }
            // FIXME simplify logic - does the initial count and block checks again for no reason;
            //       don't you just want to fall through to the size_t available line?
            if (CC_LIKELY(status == NOT_ENOUGH_DATA)) {
                status = WOULD_BLOCK;
            }
            return accumulator > 0 ? accumulator : (ssize_t) status;
        }
        size_t available = mBuffer.frameCount - mConsumed;
        if (CC_UNLIKELY(count > available)) {
            count = available;
        }
        if (CC_LIKELY(count > 0)) {
            ssize_t ret = via(user, (char *) mBuffer.raw + (mConsumed * mFrameSize), count);

            if (CC_UNLIKELY(ret <= 0)) {
                if (CC_LIKELY(accumulator > 0)) {
                    return accumulator;
                }
                return ret;
            }
            ALOG_ASSERT((size_t) ret <= count);
            mFramesRead += ret;
            accumulator += ret;
            if (CC_LIKELY((mConsumed += ret) < mBuffer.frameCount)) {
                continue;
            }
        }
        mProvider->releaseBuffer(&mBuffer);
        mBuffer.raw = NULL;
        mConsumed = 0;
        // don't get next buffer until we really need it
    }
}

}   // namespace android
