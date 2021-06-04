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

#include <inttypes.h>

#define LOG_TAG "MonoPipe"
//#define LOG_NDEBUG 0

#include <cutils/compiler.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <media/AudioBufferProvider.h>
#include <media/nbaio/MonoPipe.h>
#include <audio_utils/roundup.h>


namespace android {

MonoPipe::MonoPipe(size_t reqFrames, const NBAIO_Format& format, bool writeCanBlock) :
        NBAIO_Sink(format),
        // TODO fifo now supports non-power-of-2 buffer sizes, so could remove the roundup
        mMaxFrames(roundup(reqFrames)),
        mBuffer(malloc(mMaxFrames * Format_frameSize(format))),
        mFifo(mMaxFrames, Format_frameSize(format), mBuffer, true /*throttlesWriter*/),
        mFifoWriter(mFifo),
        mWriteTsValid(false),
        // mWriteTs
        mSetpoint((reqFrames * 11) / 16),
        mWriteCanBlock(writeCanBlock),
        mIsShutdown(false),
        // mTimestampShared
        mTimestampMutator(&mTimestampShared),
        mTimestampObserver(&mTimestampShared)
{
}

MonoPipe::~MonoPipe()
{
    free(mBuffer);
}

ssize_t MonoPipe::availableToWrite()
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    // uses mMaxFrames not reqFrames, so allows "over-filling" the pipe beyond requested limit
    ssize_t ret = mFifoWriter.available();
    ALOG_ASSERT(ret <= mMaxFrames);
    return ret;
}

ssize_t MonoPipe::write(const void *buffer, size_t count)
{
    if (CC_UNLIKELY(!mNegotiated)) {
        return NEGOTIATE;
    }
    size_t totalFramesWritten = 0;
    while (count > 0) {
        ssize_t actual = mFifoWriter.write(buffer, count);
        ALOG_ASSERT(actual <= count);
        if (actual < 0) {
            if (totalFramesWritten == 0) {
                return actual;
            }
            break;
        }
        size_t written = (size_t) actual;
        totalFramesWritten += written;
        if (!mWriteCanBlock || mIsShutdown) {
            break;
        }
        count -= written;
        buffer = (char *) buffer + (written * mFrameSize);
        // TODO Replace this whole section by audio_util_fifo's setpoint feature.
        // Simulate blocking I/O by sleeping at different rates, depending on a throttle.
        // The throttle tries to keep the mean pipe depth near the setpoint, with a slight jitter.
        uint32_t ns;
        if (written > 0) {
            ssize_t avail = mFifoWriter.available();
            ALOG_ASSERT(avail <= mMaxFrames);
            if (avail < 0) {
                // don't return avail as status, because totalFramesWritten > 0
                break;
            }
            size_t filled = mMaxFrames - (size_t) avail;
            // FIXME cache these values to avoid re-computation
            if (filled <= mSetpoint / 2) {
                // pipe is (nearly) empty, fill quickly
                ns = written * ( 500000000 / Format_sampleRate(mFormat));
            } else if (filled <= (mSetpoint * 3) / 4) {
                // pipe is below setpoint, fill at slightly faster rate
                ns = written * ( 750000000 / Format_sampleRate(mFormat));
            } else if (filled <= (mSetpoint * 5) / 4) {
                // pipe is at setpoint, fill at nominal rate
                ns = written * (1000000000 / Format_sampleRate(mFormat));
            } else if (filled <= (mSetpoint * 3) / 2) {
                // pipe is above setpoint, fill at slightly slower rate
                ns = written * (1150000000 / Format_sampleRate(mFormat));
            } else if (filled <= (mSetpoint * 7) / 4) {
                // pipe is overflowing, fill slowly
                ns = written * (1350000000 / Format_sampleRate(mFormat));
            } else {
                // pipe is severely overflowing
                ns = written * (1750000000 / Format_sampleRate(mFormat));
            }
        } else {
            ns = count * (1350000000 / Format_sampleRate(mFormat));
        }
        if (ns > 999999999) {
            ns = 999999999;
        }
        struct timespec nowTs;
        bool nowTsValid = !clock_gettime(CLOCK_MONOTONIC, &nowTs);
        // deduct the elapsed time since previous write() completed
        if (nowTsValid && mWriteTsValid) {
            time_t sec = nowTs.tv_sec - mWriteTs.tv_sec;
            long nsec = nowTs.tv_nsec - mWriteTs.tv_nsec;
            ALOGE_IF(sec < 0 || (sec == 0 && nsec < 0),
                    "clock_gettime(CLOCK_MONOTONIC) failed: was %ld.%09ld but now %ld.%09ld",
                    mWriteTs.tv_sec, mWriteTs.tv_nsec, nowTs.tv_sec, nowTs.tv_nsec);
            if (nsec < 0) {
                --sec;
                nsec += 1000000000;
            }
            if (sec == 0) {
                if ((long) ns > nsec) {
                    ns -= nsec;
                } else {
                    ns = 0;
                }
            }
        }
        if (ns > 0) {
            const struct timespec req = {0, static_cast<long>(ns)};
            nanosleep(&req, NULL);
        }
        // record the time that this write() completed
        if (nowTsValid) {
            mWriteTs = nowTs;
            if ((mWriteTs.tv_nsec += ns) >= 1000000000) {
                mWriteTs.tv_nsec -= 1000000000;
                ++mWriteTs.tv_sec;
            }
        }
        mWriteTsValid = nowTsValid;
    }
    mFramesWritten += totalFramesWritten;
    return totalFramesWritten;
}

void MonoPipe::setAvgFrames(size_t setpoint)
{
    mSetpoint = setpoint;
}

void MonoPipe::shutdown(bool newState)
{
    mIsShutdown = newState;
}

bool MonoPipe::isShutdown()
{
    return mIsShutdown;
}

status_t MonoPipe::getTimestamp(ExtendedTimestamp &timestamp)
{
    ExtendedTimestamp ets;
    if (mTimestampObserver.poll(ets)) {
        timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] =
                ets.mPosition[ExtendedTimestamp::LOCATION_KERNEL];
        timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] =
                ets.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];
        return OK;
    }
    return INVALID_OPERATION;
}

}   // namespace android
