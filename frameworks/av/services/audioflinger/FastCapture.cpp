/*
 * Copyright (C) 2014 The Android Open Source Project
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

#define LOG_TAG "FastCapture"
//#define LOG_NDEBUG 0

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include "Configuration.h"
#include <audio_utils/format.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <media/AudioBufferProvider.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include "FastCapture.h"

namespace android {

/*static*/ const FastCaptureState FastCapture::sInitial;

FastCapture::FastCapture() : FastThread("cycleC_ms", "loadC_us"),
    mInputSource(NULL), mInputSourceGen(0), mPipeSink(NULL), mPipeSinkGen(0),
    mReadBuffer(NULL), mReadBufferState(-1), mFormat(Format_Invalid), mSampleRate(0),
    // mDummyDumpState
    mTotalNativeFramesRead(0)
{
    mPrevious = &sInitial;
    mCurrent = &sInitial;

    mDummyDumpState = &mDummyFastCaptureDumpState;
}

FastCapture::~FastCapture()
{
}

FastCaptureStateQueue* FastCapture::sq()
{
    return &mSQ;
}

const FastThreadState *FastCapture::poll()
{
    return mSQ.poll();
}

void FastCapture::setNBLogWriter(NBLog::Writer *logWriter __unused)
{
}

void FastCapture::onIdle()
{
    mPreIdle = *(const FastCaptureState *)mCurrent;
    mCurrent = &mPreIdle;
}

void FastCapture::onExit()
{
    free(mReadBuffer);
}

bool FastCapture::isSubClassCommand(FastThreadState::Command command)
{
    switch ((FastCaptureState::Command) command) {
    case FastCaptureState::READ:
    case FastCaptureState::WRITE:
    case FastCaptureState::READ_WRITE:
        return true;
    default:
        return false;
    }
}

void FastCapture::onStateChange()
{
    const FastCaptureState * const current = (const FastCaptureState *) mCurrent;
    const FastCaptureState * const previous = (const FastCaptureState *) mPrevious;
    FastCaptureDumpState * const dumpState = (FastCaptureDumpState *) mDumpState;
    const size_t frameCount = current->mFrameCount;

    bool eitherChanged = false;

    // check for change in input HAL configuration
    NBAIO_Format previousFormat = mFormat;
    if (current->mInputSourceGen != mInputSourceGen) {
        mInputSource = current->mInputSource;
        mInputSourceGen = current->mInputSourceGen;
        if (mInputSource == NULL) {
            mFormat = Format_Invalid;
            mSampleRate = 0;
        } else {
            mFormat = mInputSource->format();
            mSampleRate = Format_sampleRate(mFormat);
#if !LOG_NDEBUG
            unsigned channelCount = Format_channelCount(mFormat);
            ALOG_ASSERT(channelCount >= 1 && channelCount <= FCC_8);
#endif
        }
        dumpState->mSampleRate = mSampleRate;
        eitherChanged = true;
    }

    // check for change in pipe
    if (current->mPipeSinkGen != mPipeSinkGen) {
        mPipeSink = current->mPipeSink;
        mPipeSinkGen = current->mPipeSinkGen;
        eitherChanged = true;
    }

    // input source and pipe sink must be compatible
    if (eitherChanged && mInputSource != NULL && mPipeSink != NULL) {
        ALOG_ASSERT(Format_isEqual(mFormat, mPipeSink->format()));
    }

    if ((!Format_isEqual(mFormat, previousFormat)) || (frameCount != previous->mFrameCount)) {
        // FIXME to avoid priority inversion, don't free here
        free(mReadBuffer);
        mReadBuffer = NULL;
        if (frameCount > 0 && mSampleRate > 0) {
            // FIXME new may block for unbounded time at internal mutex of the heap
            //       implementation; it would be better to have normal capture thread allocate for
            //       us to avoid blocking here and to prevent possible priority inversion
            size_t bufferSize = frameCount * Format_frameSize(mFormat);
            (void)posix_memalign(&mReadBuffer, 32, bufferSize);
            memset(mReadBuffer, 0, bufferSize); // if posix_memalign fails, will segv here.
            mPeriodNs = (frameCount * 1000000000LL) / mSampleRate;      // 1.00
            mUnderrunNs = (frameCount * 1750000000LL) / mSampleRate;    // 1.75
            mOverrunNs = (frameCount * 500000000LL) / mSampleRate;      // 0.50
            mForceNs = (frameCount * 950000000LL) / mSampleRate;        // 0.95
            mWarmupNsMin = (frameCount * 750000000LL) / mSampleRate;    // 0.75
            mWarmupNsMax = (frameCount * 1250000000LL) / mSampleRate;   // 1.25
        } else {
            mPeriodNs = 0;
            mUnderrunNs = 0;
            mOverrunNs = 0;
            mForceNs = 0;
            mWarmupNsMin = 0;
            mWarmupNsMax = LONG_MAX;
        }
        mReadBufferState = -1;
        dumpState->mFrameCount = frameCount;
    }

}

void FastCapture::onWork()
{
    const FastCaptureState * const current = (const FastCaptureState *) mCurrent;
    FastCaptureDumpState * const dumpState = (FastCaptureDumpState *) mDumpState;
    const FastCaptureState::Command command = mCommand;
    size_t frameCount = current->mFrameCount;
    AudioBufferProvider* fastPatchRecordBufferProvider = current->mFastPatchRecordBufferProvider;
    AudioBufferProvider::Buffer patchBuffer;

    if (fastPatchRecordBufferProvider != 0) {
        patchBuffer.frameCount = ~0;
        status_t status = fastPatchRecordBufferProvider->getNextBuffer(&patchBuffer);
        if (status != NO_ERROR) {
            frameCount = 0;
        } else if (patchBuffer.frameCount < frameCount) {
            // TODO: Make sure that it doesn't cause any issues if we just get a small available
            // buffer from the buffer provider.
            frameCount = patchBuffer.frameCount;
        }
    }

    if ((command & FastCaptureState::READ) /*&& isWarm*/) {
        ALOG_ASSERT(mInputSource != NULL);
        ALOG_ASSERT(mReadBuffer != NULL);
        dumpState->mReadSequence++;
        ATRACE_BEGIN("read");
        ssize_t framesRead = mInputSource->read(mReadBuffer, frameCount);
        ATRACE_END();
        dumpState->mReadSequence++;
        if (framesRead >= 0) {
            LOG_ALWAYS_FATAL_IF((size_t) framesRead > frameCount);
            mTotalNativeFramesRead += framesRead;
            dumpState->mFramesRead = mTotalNativeFramesRead;
            mReadBufferState = framesRead;
            patchBuffer.frameCount = framesRead;
        } else {
            dumpState->mReadErrors++;
            mReadBufferState = 0;
        }
        // FIXME rename to attemptedIO
        mAttemptedWrite = true;
    }

    if (command & FastCaptureState::WRITE) {
        ALOG_ASSERT(mPipeSink != NULL);
        ALOG_ASSERT(mReadBuffer != NULL);
        if (mReadBufferState < 0) {
            memset(mReadBuffer, 0, frameCount * Format_frameSize(mFormat));
            mReadBufferState = frameCount;
        }
        if (mReadBufferState > 0) {
            ssize_t framesWritten = mPipeSink->write(mReadBuffer, mReadBufferState);
            audio_track_cblk_t* cblk = current->mCblk;
            if (fastPatchRecordBufferProvider != 0) {
                // This indicates the fast track is a patch record, update the cblk by
                // calling releaseBuffer().
                memcpy_by_audio_format(patchBuffer.raw, current->mFastPatchRecordFormat,
                        mReadBuffer, mFormat.mFormat, framesWritten * mFormat.mChannelCount);
                patchBuffer.frameCount = framesWritten;
                fastPatchRecordBufferProvider->releaseBuffer(&patchBuffer);
            } else if (cblk != NULL && framesWritten > 0) {
                // FIXME This supports at most one fast capture client.
                //       To handle multiple clients this could be converted to an array,
                //       or with a lot more work the control block could be shared by all clients.
                int32_t rear = cblk->u.mStreaming.mRear;
                android_atomic_release_store(framesWritten + rear, &cblk->u.mStreaming.mRear);
                cblk->mServer += framesWritten;
                int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
                if (!(old & CBLK_FUTEX_WAKE)) {
                    // client is never in server process, so don't use FUTEX_WAKE_PRIVATE
                    (void) syscall(__NR_futex, &cblk->mFutex, FUTEX_WAKE, 1);
                }
            }
        }
    }
}

}   // namespace android
