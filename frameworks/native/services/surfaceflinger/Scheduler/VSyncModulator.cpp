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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "VSyncModulator.h"

#include <cutils/properties.h>
#include <utils/Trace.h>

#include <cinttypes>
#include <mutex>

namespace android {

using RefreshRateType = scheduler::RefreshRateConfigs::RefreshRateType;
VSyncModulator::VSyncModulator() {
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.sf.vsync_trace_detailed_info", value, "0");
    mTraceDetailedInfo = atoi(value);
    // Populate the offset map with some default offsets.
    const Offsets defaultOffsets = {RefreshRateType::DEFAULT, 0, 0};
    setPhaseOffsets(defaultOffsets, defaultOffsets, defaultOffsets, 0);
}

void VSyncModulator::setPhaseOffsets(Offsets early, Offsets earlyGl, Offsets late,
                                     nsecs_t thresholdForNextVsync) {
    std::lock_guard<std::mutex> lock(mMutex);
    mOffsetMap.insert_or_assign(OffsetType::Early, early);
    mOffsetMap.insert_or_assign(OffsetType::EarlyGl, earlyGl);
    mOffsetMap.insert_or_assign(OffsetType::Late, late);
    mThresholdForNextVsync = thresholdForNextVsync;
    updateOffsetsLocked();
}

void VSyncModulator::setTransactionStart(Scheduler::TransactionStart transactionStart) {
    if (transactionStart == Scheduler::TransactionStart::EARLY) {
        mRemainingEarlyFrameCount = MIN_EARLY_FRAME_COUNT_TRANSACTION;
    }

    // An early transaction stays an early transaction.
    if (transactionStart == mTransactionStart ||
        mTransactionStart == Scheduler::TransactionStart::EARLY) {
        return;
    }
    mTransactionStart = transactionStart;
    updateOffsets();
}

void VSyncModulator::onTransactionHandled() {
    if (mTransactionStart == Scheduler::TransactionStart::NORMAL) return;
    mTransactionStart = Scheduler::TransactionStart::NORMAL;
    updateOffsets();
}

void VSyncModulator::onRefreshRateChangeInitiated() {
    if (mRefreshRateChangePending) {
        return;
    }
    mRefreshRateChangePending = true;
    updateOffsets();
}

void VSyncModulator::onRefreshRateChangeCompleted() {
    if (!mRefreshRateChangePending) {
        return;
    }
    mRefreshRateChangePending = false;
    updateOffsets();
}

void VSyncModulator::onRefreshed(bool usedRenderEngine) {
    bool updateOffsetsNeeded = false;
    if (mRemainingEarlyFrameCount > 0) {
        mRemainingEarlyFrameCount--;
        updateOffsetsNeeded = true;
    }
    if (usedRenderEngine) {
        mRemainingRenderEngineUsageCount = MIN_EARLY_GL_FRAME_COUNT_TRANSACTION;
        updateOffsetsNeeded = true;
    } else if (mRemainingRenderEngineUsageCount > 0) {
        mRemainingRenderEngineUsageCount--;
        updateOffsetsNeeded = true;
    }
    if (updateOffsetsNeeded) {
        updateOffsets();
    }
}

VSyncModulator::Offsets VSyncModulator::getOffsets() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mOffsets;
}

VSyncModulator::Offsets VSyncModulator::getNextOffsets() {
    return mOffsetMap.at(getNextOffsetType());
}

VSyncModulator::OffsetType VSyncModulator::getNextOffsetType() {
    // Early offsets are used if we're in the middle of a refresh rate
    // change, or if we recently begin a transaction.
    if (mTransactionStart == Scheduler::TransactionStart::EARLY || mRemainingEarlyFrameCount > 0 ||
        mRefreshRateChangePending) {
        return OffsetType::Early;
    } else if (mRemainingRenderEngineUsageCount > 0) {
        return OffsetType::EarlyGl;
    } else {
        return OffsetType::Late;
    }
}

void VSyncModulator::updateOffsets() {
    std::lock_guard<std::mutex> lock(mMutex);
    updateOffsetsLocked();
}

void VSyncModulator::updateOffsetsLocked() {
    const Offsets desired = getNextOffsets();

    if (mSfConnectionHandle != nullptr) {
        mScheduler->setPhaseOffset(mSfConnectionHandle, desired.sf);
    }

    if (mAppConnectionHandle != nullptr) {
        mScheduler->setPhaseOffset(mAppConnectionHandle, desired.app);
    }

    flushOffsets();
}

void VSyncModulator::flushOffsets() {
    OffsetType type = getNextOffsetType();
    mOffsets = mOffsetMap.at(type);
    if (!mTraceDetailedInfo) {
        return;
    }
    ATRACE_INT("Vsync-EarlyOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::DEFAULT && type == OffsetType::Early);
    ATRACE_INT("Vsync-EarlyGLOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::DEFAULT && type == OffsetType::EarlyGl);
    ATRACE_INT("Vsync-LateOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::DEFAULT && type == OffsetType::Late);
    ATRACE_INT("Vsync-HighFpsEarlyOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::PERFORMANCE && type == OffsetType::Early);
    ATRACE_INT("Vsync-HighFpsEarlyGLOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::PERFORMANCE && type == OffsetType::EarlyGl);
    ATRACE_INT("Vsync-HighFpsLateOffsetsOn",
               mOffsets.fpsMode == RefreshRateType::PERFORMANCE && type == OffsetType::Late);
}

} // namespace android
