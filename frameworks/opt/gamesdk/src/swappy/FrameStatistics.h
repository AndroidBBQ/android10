/*
 * Copyright 2019 The Android Open Source Project
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

#pragma once

#include "EGL.h"
#include "Thread.h"

#include <array>
#include <map>
#include <vector>

#include <swappy/swappy_extra.h>

using TimePoint = std::chrono::steady_clock::time_point;
using namespace std::chrono_literals;

namespace swappy {


class FrameStatistics {
public:
    explicit FrameStatistics(std::shared_ptr<EGL> egl,
                             std::chrono::nanoseconds refreshPeriod) :
            mEgl(egl), mRefreshPeriod(refreshPeriod) {};
    ~FrameStatistics() = default;

    void capture(EGLDisplay dpy, EGLSurface surface);

    Swappy_Stats getStats();

private:
    static constexpr int MAX_FRAME_LAG = 10;
    static constexpr std::chrono::nanoseconds LOG_EVERY_N_NS = 1s;

    void updateFrames(EGLnsecsANDROID start, EGLnsecsANDROID end, uint64_t stat[]);
    void updateIdleFrames(EGL::FrameTimestamps& frameStats) REQUIRES(mMutex);
    void updateLateFrames(EGL::FrameTimestamps& frameStats) REQUIRES(mMutex);
    void updateOffsetFromPreviousFrame(EGL::FrameTimestamps& frameStats) REQUIRES(mMutex);
    void updateLatencyFrames(EGL::FrameTimestamps& frameStats,
                             TimePoint frameStartTime) REQUIRES(mMutex);
    void logFrames() REQUIRES(mMutex);

    std::shared_ptr<EGL> mEgl;
    const std::chrono::nanoseconds mRefreshPeriod;

    struct EGLFrame {
        EGLDisplay dpy;
        EGLSurface surface;
        EGLuint64KHR id;
        TimePoint startFrameTime;
    };
    std::vector<EGLFrame> mPendingFrames;
    EGLnsecsANDROID mPrevFrameTime = 0;

    std::mutex mMutex;
    Swappy_Stats mStats GUARDED_BY(mMutex)= {};
};

} //namespace swappy
