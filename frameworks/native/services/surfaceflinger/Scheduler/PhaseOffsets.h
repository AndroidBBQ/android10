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
#include <unordered_map>

#include "RefreshRateConfigs.h"
#include "VSyncModulator.h"

namespace android {
namespace scheduler {

/*
 * This class encapsulates offsets for different refresh rates. Depending
 * on what refresh rate we are using, and wheter we are composing in GL,
 * different offsets will help us with latency. This class keeps track of
 * which mode the device is on, and returns approprate offsets when needed.
 */
class PhaseOffsets {
public:
    struct Offsets {
        VSyncModulator::Offsets early;
        VSyncModulator::Offsets earlyGl;
        VSyncModulator::Offsets late;
    };

    virtual ~PhaseOffsets();

    virtual nsecs_t getCurrentAppOffset() = 0;
    virtual nsecs_t getCurrentSfOffset() = 0;
    virtual Offsets getOffsetsForRefreshRate(
            RefreshRateConfigs::RefreshRateType refreshRateType) const = 0;
    virtual Offsets getCurrentOffsets() const = 0;
    virtual void setRefreshRateType(RefreshRateConfigs::RefreshRateType refreshRateType) = 0;
    virtual nsecs_t getOffsetThresholdForNextVsync() const = 0;
    virtual void dump(std::string& result) const = 0;
};

namespace impl {
class PhaseOffsets : public scheduler::PhaseOffsets {
public:
    PhaseOffsets();

    nsecs_t getCurrentAppOffset() override;
    nsecs_t getCurrentSfOffset() override;

    // Returns early, early GL, and late offsets for Apps and SF for a given refresh rate.
    Offsets getOffsetsForRefreshRate(
            RefreshRateConfigs::RefreshRateType refreshRateType) const override;

    // Returns early, early GL, and late offsets for Apps and SF.
    Offsets getCurrentOffsets() const override {
        return getOffsetsForRefreshRate(mRefreshRateType);
    }

    // This function should be called when the device is switching between different
    // refresh rates, to properly update the offsets.
    void setRefreshRateType(RefreshRateConfigs::RefreshRateType refreshRateType) override {
        mRefreshRateType = refreshRateType;
    }

    nsecs_t getOffsetThresholdForNextVsync() const override { return mOffsetThresholdForNextVsync; }

    // Returns current offsets in human friendly format.
    void dump(std::string& result) const override;

private:
    std::atomic<RefreshRateConfigs::RefreshRateType> mRefreshRateType =
            RefreshRateConfigs::RefreshRateType::DEFAULT;

    std::unordered_map<RefreshRateConfigs::RefreshRateType, Offsets> mOffsets;
    nsecs_t mOffsetThresholdForNextVsync;
};
} // namespace impl

} // namespace scheduler
} // namespace android
