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

#include <numeric>

#include "Scheduler/RefreshRateConfigs.h"
#include "Scheduler/SchedulerUtils.h"
#include "TimeStats/TimeStats.h"

#include "android-base/stringprintf.h"
#include "utils/Timers.h"

namespace android {
namespace scheduler {

/**
 * Class to encapsulate statistics about refresh rates that the display is using. When the power
 * mode is set to HWC_POWER_MODE_NORMAL, SF is switching between refresh rates that are stored in
 * the device's configs. Otherwise, we assume the HWC is running in power saving mode under the
 * hood (eg. the device is in DOZE, or screen off mode).
 */
class RefreshRateStats {
    static constexpr int64_t MS_PER_S = 1000;
    static constexpr int64_t MS_PER_MIN = 60 * MS_PER_S;
    static constexpr int64_t MS_PER_HOUR = 60 * MS_PER_MIN;
    static constexpr int64_t MS_PER_DAY = 24 * MS_PER_HOUR;

public:
    RefreshRateStats(const RefreshRateConfigs& refreshRateConfigs, TimeStats& timeStats,
                     int currentConfigMode, int currentPowerMode)
          : mRefreshRateConfigs(refreshRateConfigs),
            mTimeStats(timeStats),
            mCurrentConfigMode(currentConfigMode),
            mCurrentPowerMode(currentPowerMode) {}

    // Sets power mode.
    void setPowerMode(int mode) {
        if (mCurrentPowerMode == mode) {
            return;
        }
        flushTime();
        mCurrentPowerMode = mode;
    }

    // Sets config mode. If the mode has changed, it records how much time was spent in the previous
    // mode.
    void setConfigMode(int mode) {
        if (mCurrentConfigMode == mode) {
            return;
        }
        flushTime();
        mCurrentConfigMode = mode;
    }

    // Returns a map between human readable refresh rate and number of seconds the device spent in
    // that mode.
    std::unordered_map<std::string, int64_t> getTotalTimes() {
        // If the power mode is on, then we are probably switching between the config modes. If
        // it's not then the screen is probably off. Make sure to flush times before printing
        // them.
        flushTime();

        std::unordered_map<std::string, int64_t> totalTime;
        // Multiple configs may map to the same name, e.g. "60fps". Add the
        // times for such configs together.
        for (const auto& [config, time] : mConfigModesTotalTime) {
            totalTime[mRefreshRateConfigs.getRefreshRateFromConfigId(config).name] = 0;
        }
        for (const auto& [config, time] : mConfigModesTotalTime) {
            totalTime[mRefreshRateConfigs.getRefreshRateFromConfigId(config).name] += time;
        }
        totalTime["ScreenOff"] = mScreenOffTime;
        return totalTime;
    }

    // Traverses through the map of config modes and returns how long they've been running in easy
    // to read format.
    std::string doDump() const {
        std::ostringstream stream;
        stream << "+  Refresh rate: running time in seconds\n";
        for (const auto& [name, time] : const_cast<RefreshRateStats*>(this)->getTotalTimes()) {
            stream << name << ": " << getDateFormatFromMs(time) << '\n';
        }
        return stream.str();
    }

private:
    // Calculates the time that passed in ms between the last time we recorded time and the time
    // this method was called.
    void flushTime() {
        nsecs_t currentTime = systemTime();
        nsecs_t timeElapsed = currentTime - mPreviousRecordedTime;
        int64_t timeElapsedMs = ns2ms(timeElapsed);
        mPreviousRecordedTime = currentTime;

        uint32_t fps = 0;
        if (mCurrentPowerMode == HWC_POWER_MODE_NORMAL) {
            // Normal power mode is counted under different config modes.
            if (mConfigModesTotalTime.find(mCurrentConfigMode) == mConfigModesTotalTime.end()) {
                mConfigModesTotalTime[mCurrentConfigMode] = 0;
            }
            mConfigModesTotalTime[mCurrentConfigMode] += timeElapsedMs;
            fps = mRefreshRateConfigs.getRefreshRateFromConfigId(mCurrentConfigMode).fps;
        } else {
            mScreenOffTime += timeElapsedMs;
        }
        mTimeStats.recordRefreshRate(fps, timeElapsed);
    }

    // Formats the time in milliseconds into easy to read format.
    static std::string getDateFormatFromMs(int64_t timeMs) {
        auto [days, dayRemainderMs] = std::div(timeMs, MS_PER_DAY);
        auto [hours, hourRemainderMs] = std::div(dayRemainderMs, MS_PER_HOUR);
        auto [mins, minsRemainderMs] = std::div(hourRemainderMs, MS_PER_MIN);
        auto [sec, secRemainderMs] = std::div(minsRemainderMs, MS_PER_S);
        return base::StringPrintf("%" PRId64 "d%02" PRId64 ":%02" PRId64 ":%02" PRId64
                                  ".%03" PRId64,
                                  days, hours, mins, sec, secRemainderMs);
    }

    // Keeps information about refresh rate configs that device has.
    const RefreshRateConfigs& mRefreshRateConfigs;

    // Aggregate refresh rate statistics for telemetry.
    TimeStats& mTimeStats;

    int mCurrentConfigMode;
    int32_t mCurrentPowerMode;

    std::unordered_map<int /* config */, int64_t /* duration in ms */> mConfigModesTotalTime;
    int64_t mScreenOffTime = 0;

    nsecs_t mPreviousRecordedTime = systemTime();
};

} // namespace scheduler
} // namespace android
