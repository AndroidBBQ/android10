/*
 * Copyright 2018, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VIDEO_FRAME_SCHEDULER_BASE_H_
#define VIDEO_FRAME_SCHEDULER_BASE_H_

#include <utils/RefBase.h>
#include <utils/Timers.h>

#include <media/stagefright/foundation/ABase.h>

namespace android {

struct VideoFrameSchedulerBase : public RefBase {
    VideoFrameSchedulerBase();

    // (re)initialize scheduler
    void init(float videoFps = -1);
    // use in case of video render-time discontinuity, e.g. seek
    void restart();
    // get adjusted nanotime for a video frame render at renderTime
    nsecs_t schedule(nsecs_t renderTime);

    // returns the vsync period for the main display
    nsecs_t getVsyncPeriod();

    // returns the current frames-per-second, or 0.f if not primed
    float getFrameRate();

    virtual void release() = 0;

    static const size_t kHistorySize = 8;
    static const nsecs_t kNanosIn1s = 1000000000;
    static const nsecs_t kDefaultVsyncPeriod = kNanosIn1s / 60;  // 60Hz
    static const nsecs_t kVsyncRefreshPeriod = kNanosIn1s;       // 1 sec

protected:
    virtual ~VideoFrameSchedulerBase();

    nsecs_t mVsyncTime;        // vsync timing from display
    nsecs_t mVsyncPeriod;
    nsecs_t mVsyncRefreshAt;   // next time to refresh timing info

private:
    struct PLL {
        PLL();

        // reset PLL to new PLL
        void reset(float fps = -1);
        // keep current estimate, but restart phase
        void restart();
        // returns period or 0 if not yet primed
        nsecs_t addSample(nsecs_t time);
        nsecs_t getPeriod() const;

    private:
        nsecs_t mPeriod;
        nsecs_t mPhase;

        bool    mPrimed;        // have an estimate for the period
        size_t  mSamplesUsedForPriming;

        nsecs_t mLastTime;      // last input time
        nsecs_t mRefitAt;       // next input time to fit at

        size_t  mNumSamples;    // can go past kHistorySize
        nsecs_t mTimes[kHistorySize];

        void test();
        // returns whether fit was successful
        bool fit(nsecs_t phase, nsecs_t period, size_t numSamples,
                int64_t *a, int64_t *b, int64_t *err);
        void prime(size_t numSamples);
    };

    virtual void updateVsync() = 0;

    nsecs_t mLastVsyncTime;    // estimated vsync time for last frame
    nsecs_t mTimeCorrection;   // running adjustment
    PLL mPll;                  // PLL for video frame rate based on render time

    DISALLOW_EVIL_CONSTRUCTORS(VideoFrameSchedulerBase);
};

}  // namespace android

#endif  // VIDEO_FRAME_SCHEDULER_BASE_H_
