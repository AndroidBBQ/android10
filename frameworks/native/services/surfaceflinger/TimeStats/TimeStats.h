/*
 * Copyright 2018 The Android Open Source Project
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

#include <timestatsproto/TimeStatsHelper.h>
#include <timestatsproto/TimeStatsProtoHeader.h>

#include <hardware/hwcomposer_defs.h>

#include <ui/FenceTime.h>

#include <utils/String16.h>
#include <utils/Vector.h>

#include <deque>
#include <mutex>
#include <optional>
#include <unordered_map>

using namespace android::surfaceflinger;

namespace android {

class TimeStats {
public:
    virtual ~TimeStats() = default;

    virtual void parseArgs(bool asProto, const Vector<String16>& args, std::string& result) = 0;
    virtual bool isEnabled() = 0;
    virtual std::string miniDump() = 0;

    virtual void incrementTotalFrames() = 0;
    virtual void incrementMissedFrames() = 0;
    virtual void incrementClientCompositionFrames() = 0;

    virtual void setPostTime(int32_t layerID, uint64_t frameNumber, const std::string& layerName,
                             nsecs_t postTime) = 0;
    virtual void setLatchTime(int32_t layerID, uint64_t frameNumber, nsecs_t latchTime) = 0;
    virtual void setDesiredTime(int32_t layerID, uint64_t frameNumber, nsecs_t desiredTime) = 0;
    virtual void setAcquireTime(int32_t layerID, uint64_t frameNumber, nsecs_t acquireTime) = 0;
    virtual void setAcquireFence(int32_t layerID, uint64_t frameNumber,
                                 const std::shared_ptr<FenceTime>& acquireFence) = 0;
    virtual void setPresentTime(int32_t layerID, uint64_t frameNumber, nsecs_t presentTime) = 0;
    virtual void setPresentFence(int32_t layerID, uint64_t frameNumber,
                                 const std::shared_ptr<FenceTime>& presentFence) = 0;
    // Clean up the layer record
    virtual void onDestroy(int32_t layerID) = 0;
    // If SF skips or rejects a buffer, remove the corresponding TimeRecord.
    virtual void removeTimeRecord(int32_t layerID, uint64_t frameNumber) = 0;

    virtual void setPowerMode(int32_t powerMode) = 0;
    // Source of truth is RefrehRateStats.
    virtual void recordRefreshRate(uint32_t fps, nsecs_t duration) = 0;
    virtual void setPresentFenceGlobal(const std::shared_ptr<FenceTime>& presentFence) = 0;
};

namespace impl {

class TimeStats : public android::TimeStats {
    struct FrameTime {
        uint64_t frameNumber = 0;
        nsecs_t postTime = 0;
        nsecs_t latchTime = 0;
        nsecs_t acquireTime = 0;
        nsecs_t desiredTime = 0;
        nsecs_t presentTime = 0;
    };

    struct TimeRecord {
        bool ready = false;
        FrameTime frameTime;
        std::shared_ptr<FenceTime> acquireFence;
        std::shared_ptr<FenceTime> presentFence;
    };

    struct LayerRecord {
        std::string layerName;
        // This is the index in timeRecords, at which the timestamps for that
        // specific frame are still not fully received. This is not waiting for
        // fences to signal, but rather waiting to receive those fences/timestamps.
        int32_t waitData = -1;
        uint32_t droppedFrames = 0;
        TimeRecord prevTimeRecord;
        std::deque<TimeRecord> timeRecords;
    };

    struct PowerTime {
        int32_t powerMode = HWC_POWER_MODE_OFF;
        nsecs_t prevTime = 0;
    };

    struct GlobalRecord {
        nsecs_t prevPresentTime = 0;
        std::deque<std::shared_ptr<FenceTime>> presentFences;
    };

public:
    TimeStats() = default;

    void parseArgs(bool asProto, const Vector<String16>& args, std::string& result) override;
    bool isEnabled() override;
    std::string miniDump() override;

    void incrementTotalFrames() override;
    void incrementMissedFrames() override;
    void incrementClientCompositionFrames() override;

    void setPostTime(int32_t layerID, uint64_t frameNumber, const std::string& layerName,
                     nsecs_t postTime) override;
    void setLatchTime(int32_t layerID, uint64_t frameNumber, nsecs_t latchTime) override;
    void setDesiredTime(int32_t layerID, uint64_t frameNumber, nsecs_t desiredTime) override;
    void setAcquireTime(int32_t layerID, uint64_t frameNumber, nsecs_t acquireTime) override;
    void setAcquireFence(int32_t layerID, uint64_t frameNumber,
                         const std::shared_ptr<FenceTime>& acquireFence) override;
    void setPresentTime(int32_t layerID, uint64_t frameNumber, nsecs_t presentTime) override;
    void setPresentFence(int32_t layerID, uint64_t frameNumber,
                         const std::shared_ptr<FenceTime>& presentFence) override;
    // Clean up the layer record
    void onDestroy(int32_t layerID) override;
    // If SF skips or rejects a buffer, remove the corresponding TimeRecord.
    void removeTimeRecord(int32_t layerID, uint64_t frameNumber) override;

    void setPowerMode(int32_t powerMode) override;
    // Source of truth is RefrehRateStats.
    void recordRefreshRate(uint32_t fps, nsecs_t duration) override;
    void setPresentFenceGlobal(const std::shared_ptr<FenceTime>& presentFence) override;

    static const size_t MAX_NUM_TIME_RECORDS = 64;

private:
    bool recordReadyLocked(int32_t layerID, TimeRecord* timeRecord);
    void flushAvailableRecordsToStatsLocked(int32_t layerID);
    void flushPowerTimeLocked();
    void flushAvailableGlobalRecordsToStatsLocked();

    void enable();
    void disable();
    void clear();
    void dump(bool asProto, std::optional<uint32_t> maxLayers, std::string& result);

    std::atomic<bool> mEnabled = false;
    std::mutex mMutex;
    TimeStatsHelper::TimeStatsGlobal mTimeStats;
    // Hashmap for LayerRecord with layerID as the hash key
    std::unordered_map<int32_t, LayerRecord> mTimeStatsTracker;
    PowerTime mPowerTime;
    GlobalRecord mGlobalRecord;

    static const size_t MAX_NUM_LAYER_RECORDS = 200;
};

} // namespace impl

} // namespace android
