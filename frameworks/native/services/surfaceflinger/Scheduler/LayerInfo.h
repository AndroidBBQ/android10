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

#include <cinttypes>
#include <cstdint>
#include <deque>
#include <mutex>
#include <numeric>
#include <string>

#include <log/log.h>

#include <utils/Mutex.h>
#include <utils/Timers.h>

#include "SchedulerUtils.h"

namespace android {
namespace scheduler {

/*
 * This class represents information about individial layers.
 */
class LayerInfo {
    /**
     * Struct that keeps the information about the refresh rate for last
     * HISTORY_SIZE frames. This is used to better determine the refresh rate
     * for individual layers.
     */
    class RefreshRateHistory {
    public:
        explicit RefreshRateHistory(nsecs_t minRefreshDuration)
              : mMinRefreshDuration(minRefreshDuration) {}
        void insertRefreshRate(int refreshRate) {
            mElements.push_back(refreshRate);
            if (mElements.size() > HISTORY_SIZE) {
                mElements.pop_front();
            }
        }

        float getRefreshRateAvg() const {
            if (mElements.empty()) {
                return 1e9f / mMinRefreshDuration;
            }

            return scheduler::calculate_mean(mElements);
        }

        void clearHistory() { mElements.clear(); }

    private:
        std::deque<nsecs_t> mElements;
        static constexpr size_t HISTORY_SIZE = 30;
        const nsecs_t mMinRefreshDuration;
    };

    /**
     * Struct that keeps the information about the present time for last
     * HISTORY_SIZE frames. This is used to better determine whether the given layer
     * is still relevant and it's refresh rate should be considered.
     */
    class PresentTimeHistory {
    public:
        void insertPresentTime(nsecs_t presentTime) {
            mElements.push_back(presentTime);
            if (mElements.size() > HISTORY_SIZE) {
                mElements.pop_front();
            }
        }

        // Checks whether the present time that was inserted HISTORY_SIZE ago is within a
        // certain threshold: TIME_EPSILON_NS.
        bool isRelevant() const {
            if (mElements.size() < 2) {
                return false;
            }

            // The layer had to publish at least HISTORY_SIZE or HISTORY_TIME of updates
            if (mElements.size() != HISTORY_SIZE &&
                mElements.at(mElements.size() - 1) - mElements.at(0) < HISTORY_TIME.count()) {
                return false;
            }

            // The last update should not be older than OBSOLETE_TIME_EPSILON_NS nanoseconds.
            const int64_t obsoleteEpsilon =
                    systemTime() - scheduler::OBSOLETE_TIME_EPSILON_NS.count();
            if (mElements.at(mElements.size() - 1) < obsoleteEpsilon) {
                return false;
            }

            return true;
        }

        bool isLowActivityLayer() const {
            // We want to make sure that we received more than two frames from the layer
            // in order to check low activity.
            if (mElements.size() < scheduler::LOW_ACTIVITY_BUFFERS + 1) {
                return false;
            }

            const int64_t obsoleteEpsilon =
                    systemTime() - scheduler::LOW_ACTIVITY_EPSILON_NS.count();
            // Check the frame before last to determine whether there is low activity.
            // If that frame is older than LOW_ACTIVITY_EPSILON_NS, the layer is sending
            // infrequent updates.
            if (mElements.at(mElements.size() - (scheduler::LOW_ACTIVITY_BUFFERS + 1)) <
                obsoleteEpsilon) {
                return true;
            }

            return false;
        }

        void clearHistory() { mElements.clear(); }

    private:
        std::deque<nsecs_t> mElements;
        static constexpr size_t HISTORY_SIZE = 90;
        static constexpr std::chrono::nanoseconds HISTORY_TIME = 1s;
    };

public:
    LayerInfo(const std::string name, float minRefreshRate, float maxRefreshRate);
    ~LayerInfo();

    LayerInfo(const LayerInfo&) = delete;
    LayerInfo& operator=(const LayerInfo&) = delete;

    // Records the last requested oresent time. It also stores information about when
    // the layer was last updated. If the present time is farther in the future than the
    // updated time, the updated time is the present time.
    void setLastPresentTime(nsecs_t lastPresentTime);

    void setHDRContent(bool isHdr) {
        std::lock_guard lock(mLock);
        mIsHDR = isHdr;
    }

    void setVisibility(bool visible) {
        std::lock_guard lock(mLock);
        mIsVisible = visible;
    }

    // Checks the present time history to see whether the layer is relevant.
    bool isRecentlyActive() const {
        std::lock_guard lock(mLock);
        return mPresentTimeHistory.isRelevant();
    }

    // Calculate the average refresh rate.
    float getDesiredRefreshRate() const {
        std::lock_guard lock(mLock);

        if (mPresentTimeHistory.isLowActivityLayer()) {
            return 1e9f / mLowActivityRefreshDuration;
        }
        return mRefreshRateHistory.getRefreshRateAvg();
    }

    bool getHDRContent() {
        std::lock_guard lock(mLock);
        return mIsHDR;
    }

    bool isVisible() {
        std::lock_guard lock(mLock);
        return mIsVisible;
    }

    // Return the last updated time. If the present time is farther in the future than the
    // updated time, the updated time is the present time.
    nsecs_t getLastUpdatedTime() {
        std::lock_guard lock(mLock);
        return mLastUpdatedTime;
    }

    std::string getName() const { return mName; }

    void clearHistory() {
        std::lock_guard lock(mLock);
        mRefreshRateHistory.clearHistory();
        mPresentTimeHistory.clearHistory();
    }

private:
    const std::string mName;
    const nsecs_t mMinRefreshDuration;
    const nsecs_t mLowActivityRefreshDuration;
    mutable std::mutex mLock;
    nsecs_t mLastUpdatedTime GUARDED_BY(mLock) = 0;
    nsecs_t mLastPresentTime GUARDED_BY(mLock) = 0;
    RefreshRateHistory mRefreshRateHistory GUARDED_BY(mLock);
    PresentTimeHistory mPresentTimeHistory GUARDED_BY(mLock);
    bool mIsHDR GUARDED_BY(mLock) = false;
    bool mIsVisible GUARDED_BY(mLock) = false;
};

} // namespace scheduler
} // namespace android