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

#include <cinttypes>
#include <mutex>

#include "Scheduler.h"

namespace android {

/*
 * Modulates the vsync-offsets depending on current SurfaceFlinger state.
 */
class VSyncModulator {
private:
    // Number of frames we'll keep the early phase offsets once they are activated for a
    // transaction. This acts as a low-pass filter in case the client isn't quick enough in
    // sending new transactions.
    const int MIN_EARLY_FRAME_COUNT_TRANSACTION = 2;

    // Number of frames we'll keep the early gl phase offsets once they are activated.
    // This acts as a low-pass filter to avoid scenarios where we rapidly
    // switch in and out of gl composition.
    const int MIN_EARLY_GL_FRAME_COUNT_TRANSACTION = 2;

public:
    VSyncModulator();

    // Wrapper for a collection of surfaceflinger/app offsets for a particular
    // configuration .
    struct Offsets {
        scheduler::RefreshRateConfigs::RefreshRateType fpsMode;
        nsecs_t sf;
        nsecs_t app;
    };

    enum class OffsetType {
        Early,
        EarlyGl,
        Late,
    };

    // Sets the phase offsets
    //
    // sfEarly: The phase offset when waking up SF early, which happens when marking a transaction
    //          as early. May be the same as late, in which case we don't shift offsets.
    // sfEarlyGl: Like sfEarly, but only if we used GL composition. If we use both GL composition
    //            and the transaction was marked as early, we'll use sfEarly.
    // sfLate: The regular SF vsync phase offset.
    // appEarly: Like sfEarly, but for the app-vsync
    // appEarlyGl: Like sfEarlyGl, but for the app-vsync.
    // appLate: The regular app vsync phase offset.
    void setPhaseOffsets(Offsets early, Offsets earlyGl, Offsets late,
                         nsecs_t thresholdForNextVsync) EXCLUDES(mMutex);

    // Sets the scheduler and vsync connection handlers.
    void setSchedulerAndHandles(Scheduler* scheduler,
                                Scheduler::ConnectionHandle* appConnectionHandle,
                                Scheduler::ConnectionHandle* sfConnectionHandle) {
        mScheduler = scheduler;
        mAppConnectionHandle = appConnectionHandle;
        mSfConnectionHandle = sfConnectionHandle;
    }

    // Signals that a transaction has started, and changes offsets accordingly.
    void setTransactionStart(Scheduler::TransactionStart transactionStart);

    // Signals that a transaction has been completed, so that we can finish
    // special handling for a transaction.
    void onTransactionHandled();

    // Called when we send a refresh rate change to hardware composer, so that
    // we can move into early offsets.
    void onRefreshRateChangeInitiated();

    // Called when we detect from vsync signals that the refresh rate changed.
    // This way we can move out of early offsets if no longer necessary.
    void onRefreshRateChangeCompleted();

    // Called when the display is presenting a new frame. usedRenderEngine
    // should be set to true if RenderEngine was involved with composing the new
    // frame.
    void onRefreshed(bool usedRenderEngine);

    // Returns the offsets that we are currently using
    Offsets getOffsets() EXCLUDES(mMutex);

private:
    // Returns the next offsets that we should be using
    Offsets getNextOffsets() REQUIRES(mMutex);
    // Returns the next offset type that we should use.
    OffsetType getNextOffsetType();
    // Updates offsets and persists them into the scheduler framework.
    void updateOffsets() EXCLUDES(mMutex);
    void updateOffsetsLocked() REQUIRES(mMutex);
    // Updates the internal offsets and offset type.
    void flushOffsets() REQUIRES(mMutex);

    mutable std::mutex mMutex;
    std::unordered_map<OffsetType, Offsets> mOffsetMap GUARDED_BY(mMutex);
    nsecs_t mThresholdForNextVsync;

    Scheduler* mScheduler = nullptr;
    Scheduler::ConnectionHandle* mAppConnectionHandle = nullptr;
    Scheduler::ConnectionHandle* mSfConnectionHandle = nullptr;

    Offsets mOffsets GUARDED_BY(mMutex) = {Scheduler::RefreshRateType::DEFAULT, 0, 0};

    std::atomic<Scheduler::TransactionStart> mTransactionStart =
            Scheduler::TransactionStart::NORMAL;
    std::atomic<bool> mRefreshRateChangePending = false;
    std::atomic<int> mRemainingEarlyFrameCount = 0;
    std::atomic<int> mRemainingRenderEngineUsageCount = 0;

    bool mTraceDetailedInfo = false;
};

} // namespace android
