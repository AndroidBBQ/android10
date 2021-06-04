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

#include "FrameStatistics.h"

#define LOG_TAG "FrameStatistics"

#include <cmath>
#include <inttypes.h>
#include <string>

#include "EGL.h"

#include "Log.h"


namespace swappy {

// NB This is only needed for C++14
constexpr std::chrono::nanoseconds FrameStatistics::LOG_EVERY_N_NS;

void FrameStatistics::updateFrames(EGLnsecsANDROID start, EGLnsecsANDROID end, uint64_t stat[]) {
    const uint64_t deltaTimeNano = end - start;

    uint32_t numFrames = deltaTimeNano / mRefreshPeriod.count();
    numFrames = std::min(numFrames, static_cast<uint32_t>(MAX_FRAME_BUCKETS));
    stat[numFrames]++;
}

void FrameStatistics::updateIdleFrames(EGL::FrameTimestamps& frameStats) {
    updateFrames(frameStats.renderingCompleted,
                 frameStats.compositionLatched,
                 mStats.idleFrames);
}

void FrameStatistics::updateLatencyFrames(swappy::EGL::FrameTimestamps &frameStats,
                                                                        TimePoint frameStartTime) {
    updateFrames(frameStartTime.time_since_epoch().count(),
                 frameStats.presented,
                 mStats.latencyFrames);
}

void FrameStatistics::updateLateFrames(EGL::FrameTimestamps& frameStats) {
    updateFrames(frameStats.requested,
                 frameStats.presented,
                 mStats.lateFrames);
}

void FrameStatistics::updateOffsetFromPreviousFrame(swappy::EGL::FrameTimestamps &frameStats) {
    if (mPrevFrameTime != 0) {
        updateFrames(mPrevFrameTime,
                     frameStats.presented,
                     mStats.offsetFromPreviousFrame);
    }
    mPrevFrameTime = frameStats.presented;
}

// called once per swap
void FrameStatistics::capture(EGLDisplay dpy, EGLSurface surface) {
    const TimePoint frameStartTime = std::chrono::steady_clock::now();

    // first get the next frame id
    std::pair<bool,EGLuint64KHR> nextFrameId = mEgl->getNextFrameId(dpy, surface);
    if (nextFrameId.first) {
        mPendingFrames.push_back({dpy, surface, nextFrameId.second, frameStartTime});
    }

    if (mPendingFrames.empty()) {
        return;
    }


    EGLFrame frame = mPendingFrames.front();
    // make sure we don't lag behind the stats too much
    if (nextFrameId.first && nextFrameId.second - frame.id > MAX_FRAME_LAG) {
        while (mPendingFrames.size() > 1)
            mPendingFrames.erase(mPendingFrames.begin());
        mPrevFrameTime = 0;
        frame = mPendingFrames.front();
    }

    std::unique_ptr<EGL::FrameTimestamps> frameStats =
            mEgl->getFrameTimestamps(frame.dpy, frame.surface, frame.id);

    if (!frameStats) {
        return;
    }

    mPendingFrames.erase(mPendingFrames.begin());

    std::lock_guard<std::mutex> lock(mMutex);
    mStats.totalFrames++;
    updateIdleFrames(*frameStats);
    updateLateFrames(*frameStats);
    updateOffsetFromPreviousFrame(*frameStats);
    updateLatencyFrames(*frameStats, frame.startFrameTime);

    logFrames();
}

void FrameStatistics::logFrames() {
    static auto previousLogTime = std::chrono::steady_clock::now();

    if (std::chrono::steady_clock::now() - previousLogTime < LOG_EVERY_N_NS) {
        return;
    }

    std::string message;
    ALOGI("== Frame statistics ==");
    ALOGI("total frames: %" PRIu64, mStats.totalFrames);
    message += "Buckets:                    ";
    for (int i = 0; i < MAX_FRAME_BUCKETS; i++)
        message += "\t[" + swappy::to_string(i) + "]";
    ALOGI("%s", message.c_str());

    message = "";
    message += "idle frames:                ";
    for (int i = 0; i < MAX_FRAME_BUCKETS; i++)
        message += "\t " + swappy::to_string(mStats.idleFrames[i]);
    ALOGI("%s", message.c_str());

    message = "";
    message += "late frames:                ";
    for (int i = 0; i < MAX_FRAME_BUCKETS; i++)
        message += "\t " + swappy::to_string(mStats.lateFrames[i]);
    ALOGI("%s", message.c_str());

    message = "";
    message += "offset from previous frame: ";
    for (int i = 0; i < MAX_FRAME_BUCKETS; i++)
        message += "\t " + swappy::to_string(mStats.offsetFromPreviousFrame[i]);
    ALOGI("%s", message.c_str());

    message = "";
    message += "frame latency:              ";
    for (int i = 0; i < MAX_FRAME_BUCKETS; i++)
        message += "\t " + swappy::to_string(mStats.latencyFrames[i]);
    ALOGI("%s", message.c_str());

    previousLogTime = std::chrono::steady_clock::now();
}

Swappy_Stats FrameStatistics::getStats() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mStats;
}

} // namespace swappy
