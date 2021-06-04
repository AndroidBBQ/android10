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

#ifndef ANDROID_AUDIO_FAST_THREAD_H
#define ANDROID_AUDIO_FAST_THREAD_H

#include "Configuration.h"
#ifdef CPU_FREQUENCY_STATISTICS
#include <cpustats/ThreadCpuUsage.h>
#endif
#include <utils/Thread.h>
#include "FastThreadState.h"

namespace android {

// FastThread is the common abstract base class of FastMixer and FastCapture
class FastThread : public Thread {

public:
            FastThread(const char *cycleMs, const char *loadUs);
    virtual ~FastThread();

private:
    // implement Thread::threadLoop()
    virtual bool threadLoop();

protected:
    // callouts to subclass in same lexical order as they were in original FastMixer.cpp
    // FIXME need comments
    virtual const FastThreadState *poll() = 0;
    virtual void setNBLogWriter(NBLog::Writer *logWriter __unused) { }
    virtual void onIdle() = 0;
    virtual void onExit() = 0;
    virtual bool isSubClassCommand(FastThreadState::Command command) = 0;
    virtual void onStateChange() = 0;
    virtual void onWork() = 0;

    // FIXME these former local variables need comments
    const FastThreadState*  mPrevious;
    const FastThreadState*  mCurrent;
    struct timespec mOldTs;
    bool            mOldTsValid;
    long            mSleepNs;       // -1: busy wait, 0: sched_yield, > 0: nanosleep
    long            mPeriodNs;      // expected period; the time required to render one mix buffer
    long            mUnderrunNs;    // underrun likely when write cycle is greater than this value
    long            mOverrunNs;     // overrun likely when write cycle is less than this value
    long            mForceNs;       // if overrun detected,
                                    // force the write cycle to take this much time
    long            mWarmupNsMin;   // warmup complete when write cycle is greater than or equal to
                                    // this value
    long            mWarmupNsMax;   // and less than or equal to this value
    FastThreadDumpState* mDummyDumpState;
    FastThreadDumpState* mDumpState;
    bool            mIgnoreNextOverrun;     // used to ignore initial overrun and first after an
                                            // underrun
#ifdef FAST_THREAD_STATISTICS
    struct timespec mOldLoad;       // previous value of clock_gettime(CLOCK_THREAD_CPUTIME_ID)
    bool            mOldLoadValid;  // whether oldLoad is valid
    uint32_t        mBounds;
    bool            mFull;          // whether we have collected at least mSamplingN samples
#ifdef CPU_FREQUENCY_STATISTICS
    ThreadCpuUsage  mTcu;           // for reading the current CPU clock frequency in kHz
#endif
#endif
    unsigned        mColdGen;       // last observed mColdGen
    bool            mIsWarm;        // true means ready to mix,
                                    // false means wait for warmup before mixing
    struct timespec   mMeasuredWarmupTs;  // how long did it take for warmup to complete
    uint32_t          mWarmupCycles;  // counter of number of loop cycles during warmup phase
    uint32_t          mWarmupConsecutiveInRangeCycles;    // number of consecutive cycles in range
    const sp<NBLog::Writer> mDummyNBLogWriter{new NBLog::Writer()};
    status_t          mTimestampStatus;

    FastThreadState::Command mCommand;
    bool            mAttemptedWrite;

    char            mCycleMs[16];   // cycle_ms + suffix
    char            mLoadUs[16];    // load_us + suffix

};  // class FastThread

}   // android

#endif  // ANDROID_AUDIO_FAST_THREAD_H
