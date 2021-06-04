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

#define LOG_TAG "PipeReader"
//#define LOG_NDEBUG 0

#include <cutils/compiler.h>
#include <cutils/atomic.h>
#include <utils/Log.h>
#include <media/nbaio/PipeReader.h>

namespace android {

PipeReader::PipeReader(Pipe& pipe) :
        NBAIO_Source(pipe.mFormat),
        mPipe(pipe), mFifoReader(mPipe.mFifo, false /*throttlesWriter*/, false /*flush*/),
        mFramesOverrun(0),
        mOverruns(0)
{
    android_atomic_inc(&pipe.mReaders);
}

PipeReader::~PipeReader()
{
#if !LOG_NDEBUG
    int32_t readers =
#else
    (void)
#endif
            android_atomic_dec(&mPipe.mReaders);
    ALOG_ASSERT(readers > 0);
}

ssize_t PipeReader::availableToRead()
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    size_t lost;
    ssize_t avail = mFifoReader.available(&lost);
    if (avail == -EOVERFLOW || lost > 0) {
        mFramesOverrun += lost;
        ++mOverruns;
        avail = OVERRUN;
    }
    return avail;
}

ssize_t PipeReader::read(void *buffer, size_t count)
{
    size_t lost;
    ssize_t actual = mFifoReader.read(buffer, count, NULL /*timeout*/, &lost);
    ALOG_ASSERT(actual <= count);
    if (actual == -EOVERFLOW || lost > 0) {
        mFramesOverrun += lost;
        ++mOverruns;
        actual = OVERRUN;
    }
    if (actual <= 0) {
        return actual;
    }
    mFramesRead += (size_t) actual;
    return actual;
}

ssize_t PipeReader::flush()
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    size_t lost;
    ssize_t flushed = mFifoReader.flush(&lost);
    if (flushed == -EOVERFLOW || lost > 0) {
        mFramesOverrun += lost;
        ++mOverruns;
        flushed = OVERRUN;
    }
    if (flushed <= 0) {
        return flushed;
    }
    mFramesRead += (size_t) flushed;  // we consider flushed frames as read, but not lost frames
    return flushed;
}

}   // namespace android
