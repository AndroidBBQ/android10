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

#define LOG_TAG "FastThread"
//#define LOG_NDEBUG 0

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include "Configuration.h"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <audio_utils/clock.h>
#include <cutils/atomic.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include "FastThread.h"
#include "FastThreadDumpState.h"
#include "TypedLogger.h"

#define FAST_DEFAULT_NS    999999999L   // ~1 sec: default time to sleep
#define FAST_HOT_IDLE_NS     1000000L   // 1 ms: time to sleep while hot idling
#define MIN_WARMUP_CYCLES          2    // minimum number of consecutive in-range loop cycles
                                        // to wait for warmup
#define MAX_WARMUP_CYCLES         10    // maximum number of loop cycles to wait for warmup

namespace android {

FastThread::FastThread(const char *cycleMs, const char *loadUs) : Thread(false /*canCallJava*/),
    // re-initialized to &sInitial by subclass constructor
    mPrevious(NULL), mCurrent(NULL),
    /* mOldTs({0, 0}), */
    mOldTsValid(false),
    mSleepNs(-1),
    mPeriodNs(0),
    mUnderrunNs(0),
    mOverrunNs(0),
    mForceNs(0),
    mWarmupNsMin(0),
    mWarmupNsMax(LONG_MAX),
    // re-initialized to &mDummySubclassDumpState by subclass constructor
    mDummyDumpState(NULL),
    mDumpState(NULL),
    mIgnoreNextOverrun(true),
#ifdef FAST_THREAD_STATISTICS
    // mOldLoad
    mOldLoadValid(false),
    mBounds(0),
    mFull(false),
    // mTcu
#endif
    mColdGen(0),
    mIsWarm(false),
    /* mMeasuredWarmupTs({0, 0}), */
    mWarmupCycles(0),
    mWarmupConsecutiveInRangeCycles(0),
    mTimestampStatus(INVALID_OPERATION),

    mCommand(FastThreadState::INITIAL),
#if 0
    frameCount(0),
#endif
    mAttemptedWrite(false)
    // mCycleMs(cycleMs)
    // mLoadUs(loadUs)
{
    mOldTs.tv_sec = 0;
    mOldTs.tv_nsec = 0;
    mMeasuredWarmupTs.tv_sec = 0;
    mMeasuredWarmupTs.tv_nsec = 0;
    strlcpy(mCycleMs, cycleMs, sizeof(mCycleMs));
    strlcpy(mLoadUs, loadUs, sizeof(mLoadUs));
}

FastThread::~FastThread()
{
}

bool FastThread::threadLoop()
{
    // LOGT now works even if tlNBLogWriter is nullptr, but we're considering changing that,
    // so this initialization permits a future change to remove the check for nullptr.
    tlNBLogWriter = mDummyNBLogWriter.get();
    for (;;) {

        // either nanosleep, sched_yield, or busy wait
        if (mSleepNs >= 0) {
            if (mSleepNs > 0) {
                ALOG_ASSERT(mSleepNs < 1000000000);
                const struct timespec req = {0, mSleepNs};
                nanosleep(&req, NULL);
            } else {
                sched_yield();
            }
        }
        // default to long sleep for next cycle
        mSleepNs = FAST_DEFAULT_NS;

        // poll for state change
        const FastThreadState *next = poll();
        if (next == NULL) {
            // continue to use the default initial state until a real state is available
            // FIXME &sInitial not available, should save address earlier
            //ALOG_ASSERT(mCurrent == &sInitial && previous == &sInitial);
            next = mCurrent;
        }

        mCommand = next->mCommand;
        if (next != mCurrent) {

            // As soon as possible of learning of a new dump area, start using it
            mDumpState = next->mDumpState != NULL ? next->mDumpState : mDummyDumpState;
            tlNBLogWriter = next->mNBLogWriter != NULL ?
                    next->mNBLogWriter : mDummyNBLogWriter.get();
            setNBLogWriter(tlNBLogWriter); // FastMixer informs its AudioMixer, FastCapture ignores

            // We want to always have a valid reference to the previous (non-idle) state.
            // However, the state queue only guarantees access to current and previous states.
            // So when there is a transition from a non-idle state into an idle state, we make a
            // copy of the last known non-idle state so it is still available on return from idle.
            // The possible transitions are:
            //  non-idle -> non-idle    update previous from current in-place
            //  non-idle -> idle        update previous from copy of current
            //  idle     -> idle        don't update previous
            //  idle     -> non-idle    don't update previous
            if (!(mCurrent->mCommand & FastThreadState::IDLE)) {
                if (mCommand & FastThreadState::IDLE) {
                    onIdle();
                    mOldTsValid = false;
#ifdef FAST_THREAD_STATISTICS
                    mOldLoadValid = false;
#endif
                    mIgnoreNextOverrun = true;
                }
                mPrevious = mCurrent;
            }
            mCurrent = next;
        }
#if !LOG_NDEBUG
        next = NULL;    // not referenced again
#endif

        mDumpState->mCommand = mCommand;

        // FIXME what does this comment mean?
        // << current, previous, command, dumpState >>

        switch (mCommand) {
        case FastThreadState::INITIAL:
        case FastThreadState::HOT_IDLE:
            mSleepNs = FAST_HOT_IDLE_NS;
            continue;
        case FastThreadState::COLD_IDLE:
            // only perform a cold idle command once
            // FIXME consider checking previous state and only perform if previous != COLD_IDLE
            if (mCurrent->mColdGen != mColdGen) {
                int32_t *coldFutexAddr = mCurrent->mColdFutexAddr;
                ALOG_ASSERT(coldFutexAddr != NULL);
                int32_t old = android_atomic_dec(coldFutexAddr);
                if (old <= 0) {
                    syscall(__NR_futex, coldFutexAddr, FUTEX_WAIT_PRIVATE, old - 1, NULL);
                }
                int policy = sched_getscheduler(0) & ~SCHED_RESET_ON_FORK;
                if (!(policy == SCHED_FIFO || policy == SCHED_RR)) {
                    ALOGE("did not receive expected priority boost on time");
                }
                // This may be overly conservative; there could be times that the normal mixer
                // requests such a brief cold idle that it doesn't require resetting this flag.
                mIsWarm = false;
                mMeasuredWarmupTs.tv_sec = 0;
                mMeasuredWarmupTs.tv_nsec = 0;
                mWarmupCycles = 0;
                mWarmupConsecutiveInRangeCycles = 0;
                mSleepNs = -1;
                mColdGen = mCurrent->mColdGen;
#ifdef FAST_THREAD_STATISTICS
                mBounds = 0;
                mFull = false;
#endif
                mOldTsValid = !clock_gettime(CLOCK_MONOTONIC, &mOldTs);
                mTimestampStatus = INVALID_OPERATION;
            } else {
                mSleepNs = FAST_HOT_IDLE_NS;
            }
            continue;
        case FastThreadState::EXIT:
            onExit();
            return false;
        default:
            LOG_ALWAYS_FATAL_IF(!isSubClassCommand(mCommand));
            break;
        }

        // there is a non-idle state available to us; did the state change?
        if (mCurrent != mPrevious) {
            onStateChange();
#if 1   // FIXME shouldn't need this
            // only process state change once
            mPrevious = mCurrent;
#endif
        }

        // do work using current state here
        mAttemptedWrite = false;
        onWork();

        // To be exactly periodic, compute the next sleep time based on current time.
        // This code doesn't have long-term stability when the sink is non-blocking.
        // FIXME To avoid drift, use the local audio clock or watch the sink's fill status.
        struct timespec newTs;
        int rc = clock_gettime(CLOCK_MONOTONIC, &newTs);
        if (rc == 0) {
            if (mOldTsValid) {
                time_t sec = newTs.tv_sec - mOldTs.tv_sec;
                long nsec = newTs.tv_nsec - mOldTs.tv_nsec;
                ALOGE_IF(sec < 0 || (sec == 0 && nsec < 0),
                        "clock_gettime(CLOCK_MONOTONIC) failed: was %ld.%09ld but now %ld.%09ld",
                        mOldTs.tv_sec, mOldTs.tv_nsec, newTs.tv_sec, newTs.tv_nsec);
                if (nsec < 0) {
                    --sec;
                    nsec += 1000000000;
                }
                // To avoid an initial underrun on fast tracks after exiting standby,
                // do not start pulling data from tracks and mixing until warmup is complete.
                // Warmup is considered complete after the earlier of:
                //      MIN_WARMUP_CYCLES consecutive in-range write() attempts,
                //          where "in-range" means mWarmupNsMin <= cycle time <= mWarmupNsMax
                //      MAX_WARMUP_CYCLES write() attempts.
                // This is overly conservative, but to get better accuracy requires a new HAL API.
                if (!mIsWarm && mAttemptedWrite) {
                    mMeasuredWarmupTs.tv_sec += sec;
                    mMeasuredWarmupTs.tv_nsec += nsec;
                    if (mMeasuredWarmupTs.tv_nsec >= 1000000000) {
                        mMeasuredWarmupTs.tv_sec++;
                        mMeasuredWarmupTs.tv_nsec -= 1000000000;
                    }
                    ++mWarmupCycles;
                    if (mWarmupNsMin <= nsec && nsec <= mWarmupNsMax) {
                        ALOGV("warmup cycle %d in range: %.03f ms", mWarmupCycles, nsec * 1e-9);
                        ++mWarmupConsecutiveInRangeCycles;
                    } else {
                        ALOGV("warmup cycle %d out of range: %.03f ms", mWarmupCycles, nsec * 1e-9);
                        mWarmupConsecutiveInRangeCycles = 0;
                    }
                    if ((mWarmupConsecutiveInRangeCycles >= MIN_WARMUP_CYCLES) ||
                            (mWarmupCycles >= MAX_WARMUP_CYCLES)) {
                        mIsWarm = true;
                        mDumpState->mMeasuredWarmupTs = mMeasuredWarmupTs;
                        mDumpState->mWarmupCycles = mWarmupCycles;
                        const double measuredWarmupMs = (mMeasuredWarmupTs.tv_sec * 1e3) +
                                (mMeasuredWarmupTs.tv_nsec * 1e-6);
                        LOG_WARMUP_TIME(measuredWarmupMs);
                    }
                }
                mSleepNs = -1;
                if (mIsWarm) {
                    if (sec > 0 || nsec > mUnderrunNs) {
                        ATRACE_NAME("underrun");
                        // FIXME only log occasionally
                        ALOGV("underrun: time since last cycle %d.%03ld sec",
                                (int) sec, nsec / 1000000L);
                        mDumpState->mUnderruns++;
                        LOG_UNDERRUN(audio_utils_ns_from_timespec(&newTs));
                        mIgnoreNextOverrun = true;
                    } else if (nsec < mOverrunNs) {
                        if (mIgnoreNextOverrun) {
                            mIgnoreNextOverrun = false;
                        } else {
                            // FIXME only log occasionally
                            ALOGV("overrun: time since last cycle %d.%03ld sec",
                                    (int) sec, nsec / 1000000L);
                            mDumpState->mOverruns++;
                            LOG_OVERRUN(audio_utils_ns_from_timespec(&newTs));
                        }
                        // This forces a minimum cycle time. It:
                        //  - compensates for an audio HAL with jitter due to sample rate conversion
                        //  - works with a variable buffer depth audio HAL that never pulls at a
                        //    rate < than mOverrunNs per buffer.
                        //  - recovers from overrun immediately after underrun
                        // It doesn't work with a non-blocking audio HAL.
                        mSleepNs = mForceNs - nsec;
                    } else {
                        mIgnoreNextOverrun = false;
                    }
                }
#ifdef FAST_THREAD_STATISTICS
                if (mIsWarm) {
                    // advance the FIFO queue bounds
                    size_t i = mBounds & (mDumpState->mSamplingN - 1);
                    mBounds = (mBounds & 0xFFFF0000) | ((mBounds + 1) & 0xFFFF);
                    if (mFull) {
                        //mBounds += 0x10000;
                        __builtin_add_overflow(mBounds, 0x10000, &mBounds);
                    } else if (!(mBounds & (mDumpState->mSamplingN - 1))) {
                        mFull = true;
                    }
                    // compute the delta value of clock_gettime(CLOCK_MONOTONIC)
                    uint32_t monotonicNs = nsec;
                    if (sec > 0 && sec < 4) {
                        monotonicNs += sec * 1000000000;
                    }
                    // compute raw CPU load = delta value of clock_gettime(CLOCK_THREAD_CPUTIME_ID)
                    uint32_t loadNs = 0;
                    struct timespec newLoad;
                    rc = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &newLoad);
                    if (rc == 0) {
                        if (mOldLoadValid) {
                            sec = newLoad.tv_sec - mOldLoad.tv_sec;
                            nsec = newLoad.tv_nsec - mOldLoad.tv_nsec;
                            if (nsec < 0) {
                                --sec;
                                nsec += 1000000000;
                            }
                            loadNs = nsec;
                            if (sec > 0 && sec < 4) {
                                loadNs += sec * 1000000000;
                            }
                        } else {
                            // first time through the loop
                            mOldLoadValid = true;
                        }
                        mOldLoad = newLoad;
                    }
#ifdef CPU_FREQUENCY_STATISTICS
                    // get the absolute value of CPU clock frequency in kHz
                    int cpuNum = sched_getcpu();
                    uint32_t kHz = mTcu.getCpukHz(cpuNum);
                    kHz = (kHz << 4) | (cpuNum & 0xF);
#endif
                    // save values in FIFO queues for dumpsys
                    // these stores #1, #2, #3 are not atomic with respect to each other,
                    // or with respect to store #4 below
                    mDumpState->mMonotonicNs[i] = monotonicNs;
                    LOG_WORK_TIME(monotonicNs);
                    mDumpState->mLoadNs[i] = loadNs;
#ifdef CPU_FREQUENCY_STATISTICS
                    mDumpState->mCpukHz[i] = kHz;
#endif
                    // this store #4 is not atomic with respect to stores #1, #2, #3 above, but
                    // the newest open & oldest closed halves are atomic with respect to each other
                    mDumpState->mBounds = mBounds;
                    ATRACE_INT(mCycleMs, monotonicNs / 1000000);
                    ATRACE_INT(mLoadUs, loadNs / 1000);
                }
#endif
            } else {
                // first time through the loop
                mOldTsValid = true;
                mSleepNs = mPeriodNs;
                mIgnoreNextOverrun = true;
            }
            mOldTs = newTs;
        } else {
            // monotonic clock is broken
            mOldTsValid = false;
            mSleepNs = mPeriodNs;
        }

    }   // for (;;)

    // never return 'true'; Thread::_threadLoop() locks mutex which can result in priority inversion
}

}   // namespace android
