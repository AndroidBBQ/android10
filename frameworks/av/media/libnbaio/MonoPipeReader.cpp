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

#define LOG_TAG "MonoPipeReader"
//#define LOG_NDEBUG 0

#include <cutils/compiler.h>
#include <utils/Log.h>
#include <media/nbaio/MonoPipeReader.h>

namespace android {

MonoPipeReader::MonoPipeReader(MonoPipe* pipe) :
        NBAIO_Source(pipe->mFormat),
        mPipe(pipe), mFifoReader(mPipe->mFifo, true /*throttlesWriter*/)
{
}

MonoPipeReader::~MonoPipeReader()
{
}

ssize_t MonoPipeReader::availableToRead()
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    ssize_t ret = mFifoReader.available();
    ALOG_ASSERT(ret <= mPipe->mMaxFrames);
    return ret;
}

ssize_t MonoPipeReader::read(void *buffer, size_t count)
{
    // count == 0 is unlikely and not worth checking for explicitly; will be handled automatically
    ssize_t actual = mFifoReader.read(buffer, count);
    ALOG_ASSERT(actual <= count);
    if (CC_UNLIKELY(actual <= 0)) {
        return actual;
    }
    mFramesRead += (size_t) actual;
    return actual;
}

void MonoPipeReader::onTimestamp(const ExtendedTimestamp &timestamp)
{
    mPipe->mTimestampMutator.push(timestamp);
}

}   // namespace android
