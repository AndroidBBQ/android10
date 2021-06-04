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

#define LOG_TAG "SourceAudioBufferProvider"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <media/nbaio/SourceAudioBufferProvider.h>

namespace android {

SourceAudioBufferProvider::SourceAudioBufferProvider(const sp<NBAIO_Source>& source) :
    mSource(source),
    // mFrameSize below
    mAllocated(NULL), mSize(0), mOffset(0), mRemaining(0), mGetCount(0), mFramesReleased(0)
{
    ALOG_ASSERT(source != 0);

    // negotiate with source
    NBAIO_Format counterOffers[1];
    size_t numCounterOffers = 1;
    ssize_t index = source->negotiate(NULL, 0, counterOffers, numCounterOffers);
    ALOG_ASSERT(index == (ssize_t) NEGOTIATE && numCounterOffers > 0);
    numCounterOffers = 0;
    index = source->negotiate(counterOffers, 1, NULL, numCounterOffers);
    ALOG_ASSERT(index == 0);
    mFrameSize = Format_frameSize(source->format());
}

SourceAudioBufferProvider::~SourceAudioBufferProvider()
{
    free(mAllocated);
}

status_t SourceAudioBufferProvider::getNextBuffer(Buffer *buffer)
{
    ALOG_ASSERT(buffer != NULL && buffer->frameCount > 0 && mGetCount == 0);
    // any leftover data available?
    if (mRemaining > 0) {
        ALOG_ASSERT(mOffset + mRemaining <= mSize);
        if (mRemaining < buffer->frameCount) {
            buffer->frameCount = mRemaining;
        }
        buffer->raw = (char *) mAllocated + (mOffset * mFrameSize);
        mGetCount = buffer->frameCount;
        return OK;
    }
    // do we need to reallocate?
    if (buffer->frameCount > mSize) {
        free(mAllocated);
        // Android convention is to _not_ check the return value of malloc and friends.
        // But in this case the calloc() can also fail due to integer overflow,
        // so we check and recover.
        mAllocated = calloc(buffer->frameCount, mFrameSize);
        if (mAllocated == NULL) {
            mSize = 0;
            goto fail;
        }
        mSize = buffer->frameCount;
    }
    {
        // read from source
        ssize_t actual = mSource->read(mAllocated, buffer->frameCount);
        if (actual > 0) {
            ALOG_ASSERT((size_t) actual <= buffer->frameCount);
            mOffset = 0;
            mRemaining = actual;
            buffer->raw = mAllocated;
            buffer->frameCount = actual;
            mGetCount = actual;
            return OK;
        }
    }
fail:
    buffer->raw = NULL;
    buffer->frameCount = 0;
    mGetCount = 0;
    return NOT_ENOUGH_DATA;
}

void SourceAudioBufferProvider::releaseBuffer(Buffer *buffer)
{
    ALOG_ASSERT((buffer != NULL) &&
            (buffer->raw == (char *) mAllocated + (mOffset * mFrameSize)) &&
            (buffer->frameCount <= mGetCount) &&
            (mGetCount <= mRemaining) &&
            (mOffset + mRemaining <= mSize));
    mOffset += buffer->frameCount;
    mRemaining -= buffer->frameCount;
    mFramesReleased += buffer->frameCount;
    buffer->raw = NULL;
    buffer->frameCount = 0;
    mGetCount = 0;
}

size_t SourceAudioBufferProvider::framesReady() const
{
    ssize_t avail = mSource->availableToRead();
    return avail < 0 ? 0 : (size_t) avail;
}

int64_t SourceAudioBufferProvider::framesReleased() const
{
    return mFramesReleased;
}

void SourceAudioBufferProvider::onTimestamp(const ExtendedTimestamp &timestamp)
{
    mSource->onTimestamp(timestamp);
}

}   // namespace android
