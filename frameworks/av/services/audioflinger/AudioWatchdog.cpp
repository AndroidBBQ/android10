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

#define LOG_TAG "AudioWatchdog"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#include <utils/Log.h>
#include "AudioWatchdog.h"

#ifdef AUDIO_WATCHDOG

namespace android {

void AudioWatchdogDump::dump(int fd)
{
    char buf[32];
    if (mMostRecent != 0) {
        // includes NUL terminator
        ctime_r(&mMostRecent, buf);
    } else {
        strcpy(buf, "N/A\n");
    }
    dprintf(fd, "Watchdog: underruns=%u, logs=%u, most recent underrun log at %s",
            mUnderruns, mLogs, buf);
}

bool AudioWatchdog::threadLoop()
{
    {
        AutoMutex _l(mMyLock);
        if (mPaused) {
            mMyCond.wait(mMyLock);
            // ignore previous timestamp after resume()
            mOldTsValid = false;
            // force an immediate log on first underrun after resume()
            mLogTs.tv_sec = MIN_TIME_BETWEEN_LOGS_SEC;
            mLogTs.tv_nsec = 0;
            // caller will check for exitPending()
            return true;
        }
    }
    struct timespec newTs;
    int rc = clock_gettime(CLOCK_MONOTONIC, &newTs);
    if (rc != 0) {
        pause();
        return false;
    }
    if (!mOldTsValid) {
        mOldTs = newTs;
        mOldTsValid = true;
        return true;
    }
    time_t sec = newTs.tv_sec - mOldTs.tv_sec;
    long nsec = newTs.tv_nsec - mOldTs.tv_nsec;
    if (nsec < 0) {
        --sec;
        nsec += 1000000000;
    }
    mOldTs = newTs;
    // cycleNs is same as sec*1e9 + nsec, but limited to about 4 seconds
    uint32_t cycleNs = nsec;
    if (sec > 0) {
        if (sec < 4) {
            cycleNs += sec * 1000000000;
        } else {
            cycleNs = 4000000000u;
        }
    }
    mLogTs.tv_sec += sec;
    if ((mLogTs.tv_nsec += nsec) >= 1000000000) {
        mLogTs.tv_sec++;
        mLogTs.tv_nsec -= 1000000000;
    }
    if (cycleNs > mMaxCycleNs) {
        mDump->mUnderruns = ++mUnderruns;
        if (mLogTs.tv_sec >= MIN_TIME_BETWEEN_LOGS_SEC) {
            mDump->mLogs = ++mLogs;
            mDump->mMostRecent = time(NULL);
            ALOGW("Insufficient CPU for load: expected=%.1f actual=%.1f ms; underruns=%u logs=%u",
                mPeriodNs * 1e-6, cycleNs * 1e-6, mUnderruns, mLogs);
            mLogTs.tv_sec = 0;
            mLogTs.tv_nsec = 0;
        }
    }
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = mPeriodNs;
    rc = nanosleep(&req, NULL);
    if (!((rc == 0) || (rc == -1 && errno == EINTR))) {
        pause();
        return false;
    }
    return true;
}

void AudioWatchdog::requestExit()
{
    // must be in this order to avoid a race condition
    Thread::requestExit();
    resume();
}

void AudioWatchdog::pause()
{
    AutoMutex _l(mMyLock);
    mPaused = true;
}

void AudioWatchdog::resume()
{
    AutoMutex _l(mMyLock);
    if (mPaused) {
        mPaused = false;
        mMyCond.signal();
    }
}

void AudioWatchdog::setDump(AudioWatchdogDump *dump)
{
    mDump = dump != NULL ? dump : &mDummyDump;
}

}   // namespace android

#endif // AUDIO_WATCHDOG
