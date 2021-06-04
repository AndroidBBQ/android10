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

// The watchdog thread runs periodically.  It has two functions:
//   (a) verify that adequate CPU time is available, and log
//       as soon as possible when there appears to be a CPU shortage
//   (b) monitor the other threads [not yet implemented]

#ifndef AUDIO_WATCHDOG_H
#define AUDIO_WATCHDOG_H

#include <time.h>
#include <utils/Thread.h>

namespace android {

// Keeps a cache of AudioWatchdog statistics that can be logged by dumpsys.
// The usual caveats about atomicity of information apply.
struct AudioWatchdogDump {
    AudioWatchdogDump() : mUnderruns(0), mLogs(0), mMostRecent(0) { }
    /*virtual*/ ~AudioWatchdogDump() { }
    uint32_t mUnderruns;    // total number of underruns
    uint32_t mLogs;         // total number of log messages
    time_t   mMostRecent;   // time of most recent log
    void     dump(int fd);  // should only be called on a stable copy, not the original
};

class AudioWatchdog : public Thread {

public:
    explicit AudioWatchdog(unsigned periodMs = 50) : Thread(false /*canCallJava*/), mPaused(false),
            mPeriodNs(periodMs * 1000000), mMaxCycleNs(mPeriodNs * 2),
            // mOldTs
            // mLogTs initialized below
            mOldTsValid(false), mUnderruns(0), mLogs(0), mDump(&mDummyDump)
        {
#define MIN_TIME_BETWEEN_LOGS_SEC 60
            // force an immediate log on first underrun
            mLogTs.tv_sec = MIN_TIME_BETWEEN_LOGS_SEC;
            mLogTs.tv_nsec = 0;
        }
    virtual         ~AudioWatchdog() { }

     // Do not call Thread::requestExitAndWait() without first calling requestExit().
    // Thread::requestExitAndWait() is not virtual, and the implementation doesn't do enough.
    virtual void        requestExit();

    // FIXME merge API and implementation with AudioTrackThread
    void            pause();        // suspend thread from execution at next loop boundary
    void            resume();       // allow thread to execute, if not requested to exit

    // Where to store the dump, or NULL to not update
    void            setDump(AudioWatchdogDump* dump);

private:
    virtual bool    threadLoop();

    Mutex           mMyLock;        // Thread::mLock is private
    Condition       mMyCond;        // Thread::mThreadExitedCondition is private
    bool            mPaused;        // whether thread is currently paused

    uint32_t        mPeriodNs;      // nominal period
    uint32_t        mMaxCycleNs;    // maximum allowed time of one cycle before declaring underrun
    struct timespec mOldTs;         // monotonic time when threadLoop last ran
    struct timespec mLogTs;         // time since last log
    bool            mOldTsValid;    // whether mOldTs is valid
    uint32_t        mUnderruns;     // total number of underruns
    uint32_t        mLogs;          // total number of logs
    AudioWatchdogDump*  mDump;      // where to store the dump, always non-NULL
    AudioWatchdogDump   mDummyDump; // default area for dump in case setDump() is not called
};

}   // namespace android

#endif  // AUDIO_WATCHDOG_H
