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

#define LOG_TAG "Pipe"
//#define LOG_NDEBUG 0

#include <cutils/atomic.h>
#include <cutils/compiler.h>
#include <utils/Log.h>
#include <media/nbaio/Pipe.h>
#include <audio_utils/roundup.h>

namespace android {

Pipe::Pipe(size_t maxFrames, const NBAIO_Format& format, void *buffer) :
        NBAIO_Sink(format),
        // TODO fifo now supports non-power-of-2 buffer sizes, so could remove the roundup
        mMaxFrames(roundup(maxFrames)),
        mBuffer(buffer == NULL ? malloc(mMaxFrames * Format_frameSize(format)) : buffer),
        mFifo(mMaxFrames, Format_frameSize(format), mBuffer, false /*throttlesWriter*/),
        mFifoWriter(mFifo),
        mReaders(0),
        mFreeBufferInDestructor(buffer == NULL)
{
}

Pipe::~Pipe()
{
    ALOG_ASSERT(android_atomic_acquire_load(&mReaders) == 0);
    if (mFreeBufferInDestructor) {
        free(mBuffer);
    }
}

ssize_t Pipe::write(const void *buffer, size_t count)
{
    // count == 0 is unlikely and not worth checking for
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    ssize_t actual = mFifoWriter.write(buffer, count);
    ALOG_ASSERT(actual <= count);
    if (actual <= 0) {
        return actual;
    }
    mFramesWritten += (size_t) actual;
    return actual;
}

}   // namespace android
