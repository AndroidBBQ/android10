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

#include <cstdint>
#include <functional>
#include <memory>

#include <ui/DisplayStatInfo.h>
#include <ui/GraphicTypes.h>

#include "DispSync.h"
#include "EventControlThread.h"
#include "EventThread.h"
#include "IdleTimer.h"
#include "InjectVSyncSource.h"
#include "LayerHistory.h"
#include "RefreshRateConfigs.h"
#include "SchedulerUtils.h"

namespace android {

class EventControlThread;

class Scheduler {
public:
    // Enum to keep track of whether we trigger event to notify choreographer of config changes.
    enum class ConfigEvent { None, Changed };

    // logical or operator with the semantics of at least one of the events is Changed
    friend ConfigEvent operator|(const ConfigEvent& first, const ConfigEvent& second) {
        if (first == ConfigEvent::Changed) return ConfigEvent::Changed;
        if (second == ConfigEvent::Changed) return ConfigEvent::Changed;
        return ConfigEvent::None;
    }

    using RefreshRateType = scheduler::RefreshRateConfigs::RefreshRateType;
    using ChangeRefreshRateCallback = std::function<void(RefreshRateType, ConfigEvent)>;

    // Enum to indicate whether to start the transaction early, or at vsync time.
    enum class TransactionStart { EARLY, NORMAL };

    /* The scheduler handle is a BBinder object passed to the client from which we can extract
     * an ID for subsequent operations.
     */
    class ConnectionHandle : public BBinder {
    public:
        ConnectionHandle(int64_t id) : id(id) {}

        ~ConnectionHandle() = default;

        const int64_t id;
    };

    class Connection {
    public:
        Connection(sp<ConnectionHandle> handle, sp<EventThreadConnection> eventConnection,
                   std::unique_ptr<EventThread> eventThread)
              : handle(handle), eventConnection(eventConnection), thread(std::move(eventThread)) {}

        ~Connection() = default;

        sp<ConnectionHandle> handle;
        sp<EventThreadConnection> eventConnection;
        const std::unique_ptr<EventThread> thread;
    };

    explicit Scheduler(impl::EventControlThread::SetVSyncEnabledFunction function,
                       const scheduler::RefreshRateConfigs& refreshRateConfig);

    virtual ~Scheduler();

    /** Creates an EventThread connection. */
    sp<ConnectionHandle> createConnection(const char* connectionName, nsecs_t phaseOffsetNs,
                                          nsecs_t offsetThresholdForNextVsync,
                                          impl::EventThread::InterceptVSyncsCallback);

    sp<IDisplayEventConnection> createDisplayEventConnection(
            const sp<ConnectionHandle>& handle, ISurfaceComposer::ConfigChanged configChanged);

    // Getter methods.
    EventThread* getEventThread(const sp<ConnectionHandle>& handle);

    // Provides access to the DispSync object for the primary display.
    void withPrimaryDispSync(std::function<void(DispSync&)> const& fn);

    sp<EventThreadConnection> getEventConnection(const sp<ConnectionHandle>& handle);

    // Should be called when receiving a hotplug event.
    void hotplugReceived(const sp<ConnectionHandle>& handle, PhysicalDisplayId displayId,
                         bool connected);

    // Should be called after the screen is turned on.
    void onScreenAcquired(const sp<ConnectionHandle>& handle);

    // Should be called before the screen is turned off.
    void onScreenReleased(const sp<ConnectionHandle>& handle);

    // Should be called when display config changed
    void onConfigChanged(const sp<ConnectionHandle>& handle, PhysicalDisplayId displayId,
                         int32_t configId);

    // Should be called when dumpsys command is received.
    void dump(const sp<ConnectionHandle>& handle, std::string& result) const;

    // Offers ability to modify phase offset in the event thread.
    void setPhaseOffset(const sp<ConnectionHandle>& handle, nsecs_t phaseOffset);

    void getDisplayStatInfo(DisplayStatInfo* stats);

    void enableHardwareVsync();
    void disableHardwareVsync(bool makeUnavailable);
    // Resyncs the scheduler to hardware vsync.
    // If makeAvailable is true, then hardware vsync will be turned on.
    // Otherwise, if hardware vsync is not already enabled then this method will
    // no-op.
    // The period is the vsync period from the current display configuration.
    void resyncToHardwareVsync(bool makeAvailable, nsecs_t period);
    void resync();
    void setRefreshSkipCount(int count);
    // Passes a vsync sample to DispSync. periodFlushed will be true if
    // DispSync detected that the vsync period changed, and false otherwise.
    void addResyncSample(const nsecs_t timestamp, bool* periodFlushed);
    void addPresentFence(const std::shared_ptr<FenceTime>& fenceTime);
    void setIgnorePresentFences(bool ignore);
    nsecs_t getDispSyncExpectedPresentTime();
    // Registers the layer in the scheduler, and returns the handle for future references.
    std::unique_ptr<scheduler::LayerHistory::LayerHandle> registerLayer(std::string const& name,
                                                                        int windowType);

    // Stores present time for a layer.
    void addLayerPresentTimeAndHDR(
            const std::unique_ptr<scheduler::LayerHistory::LayerHandle>& layerHandle,
            nsecs_t presentTime, bool isHDR);
    // Stores visibility for a layer.
    void setLayerVisibility(
            const std::unique_ptr<scheduler::LayerHistory::LayerHandle>& layerHandle, bool visible);
    // Updates FPS based on the most content presented.
    void updateFpsBasedOnContent();
    // Callback that gets invoked when Scheduler wants to change the refresh rate.
    void setChangeRefreshRateCallback(const ChangeRefreshRateCallback&& changeRefreshRateCallback);

    // Returns whether idle timer is enabled or not
    bool isIdleTimerEnabled() { return mSetIdleTimerMs > 0; }

    // Function that resets the idle timer.
    void resetIdleTimer();

    // Function that resets the touch timer.
    void notifyTouchEvent();

    // Function that sets whether display power mode is normal or not.
    void setDisplayPowerState(bool normal);

    // Returns relevant information about Scheduler for dumpsys purposes.
    std::string doDump();

    // calls DispSync::dump() on primary disp sync
    void dumpPrimaryDispSync(std::string& result) const;

    // Get the appropriate refresh type for current conditions.
    RefreshRateType getPreferredRefreshRateType();

protected:
    virtual std::unique_ptr<EventThread> makeEventThread(
            const char* connectionName, DispSync* dispSync, nsecs_t phaseOffsetNs,
            nsecs_t offsetThresholdForNextVsync,
            impl::EventThread::InterceptVSyncsCallback interceptCallback);

private:
    friend class TestableScheduler;

    // In order to make sure that the features don't override themselves, we need a state machine
    // to keep track which feature requested the config change.
    enum class ContentFeatureState { CONTENT_DETECTION_ON, CONTENT_DETECTION_OFF };
    enum class IdleTimerState { EXPIRED, RESET };
    enum class TouchState { INACTIVE, ACTIVE };
    enum class DisplayPowerTimerState { EXPIRED, RESET };

    // Creates a connection on the given EventThread and forwards the given callbacks.
    sp<EventThreadConnection> createConnectionInternal(EventThread*,
                                                       ISurfaceComposer::ConfigChanged);

    nsecs_t calculateAverage() const;
    void updateFrameSkipping(const int64_t skipCount);

    // Function that is called when the timer resets.
    void resetTimerCallback();
    // Function that is called when the timer expires.
    void expiredTimerCallback();
    // Function that is called when the timer resets when paired with a display
    // driver timeout in the kernel. This enables hardware vsync when we move
    // out from idle.
    void resetKernelTimerCallback();
    // Function that is called when the timer expires when paired with a display
    // driver timeout in the kernel. This disables hardware vsync when we move
    // into idle.
    void expiredKernelTimerCallback();
    // Function that is called when the touch timer resets.
    void resetTouchTimerCallback();
    // Function that is called when the touch timer expires.
    void expiredTouchTimerCallback();
    // Function that is called when the display power timer resets.
    void resetDisplayPowerTimerCallback();
    // Function that is called when the display power timer expires.
    void expiredDisplayPowerTimerCallback();
    // Sets vsync period.
    void setVsyncPeriod(const nsecs_t period);
    // handles various timer features to change the refresh rate.
    template <class T>
    void handleTimerStateChanged(T* currentState, T newState, bool eventOnContentDetection);
    // Calculate the new refresh rate type
    RefreshRateType calculateRefreshRateType() REQUIRES(mFeatureStateLock);
    // Acquires a lock and calls the ChangeRefreshRateCallback() with given parameters.
    void changeRefreshRate(RefreshRateType refreshRateType, ConfigEvent configEvent);

    // Helper function to calculate error frames
    float getErrorFrames(float contentFps, float configFps);

    // If fences from sync Framework are supported.
    const bool mHasSyncFramework;

    // The offset in nanoseconds to use, when DispSync timestamps present fence
    // signaling time.
    nsecs_t mDispSyncPresentTimeOffset;

    // Each connection has it's own ID. This variable keeps track of the count.
    static std::atomic<int64_t> sNextId;

    // Connections are stored in a map <connection ID, connection> for easy retrieval.
    std::unordered_map<int64_t, std::unique_ptr<Connection>> mConnections;

    std::mutex mHWVsyncLock;
    bool mPrimaryHWVsyncEnabled GUARDED_BY(mHWVsyncLock);
    bool mHWVsyncAvailable GUARDED_BY(mHWVsyncLock);

    std::atomic<nsecs_t> mLastResyncTime = 0;

    std::unique_ptr<DispSync> mPrimaryDispSync;
    std::unique_ptr<EventControlThread> mEventControlThread;

    // TODO(b/113612090): The following set of variables needs to be revised. For now, this is
    // a proof of concept. We turn on frame skipping if the difference between the timestamps
    // is between 32 and 34ms. We expect this currently for 30fps videos, so we render them at 30Hz.
    nsecs_t mPreviousFrameTimestamp = 0;
    // Keeping track of whether we are skipping the refresh count. If we want to
    // simulate 30Hz rendering, we skip every other frame, and this variable is set
    // to 1.
    int64_t mSkipCount = 0;
    std::array<int64_t, scheduler::ARRAY_SIZE> mTimeDifferences{};
    size_t mCounter = 0;

    // Historical information about individual layers. Used for predicting the refresh rate.
    scheduler::LayerHistory mLayerHistory;

    // Timer that records time between requests for next vsync. If the time is higher than a given
    // interval, a callback is fired. Set this variable to >0 to use this feature.
    int64_t mSetIdleTimerMs = 0;
    std::unique_ptr<scheduler::IdleTimer> mIdleTimer;
    // Enables whether to use idle timer callbacks that support the kernel
    // timer.
    bool mSupportKernelTimer;

    // Timer used to monitor touch events.
    int64_t mSetTouchTimerMs = 0;
    std::unique_ptr<scheduler::IdleTimer> mTouchTimer;

    // Timer used to monitor display power mode.
    int64_t mSetDisplayPowerTimerMs = 0;
    std::unique_ptr<scheduler::IdleTimer> mDisplayPowerTimer;

    std::mutex mCallbackLock;
    ChangeRefreshRateCallback mChangeRefreshRateCallback GUARDED_BY(mCallbackLock);

    // In order to make sure that the features don't override themselves, we need a state machine
    // to keep track which feature requested the config change.
    std::mutex mFeatureStateLock;
    ContentFeatureState mCurrentContentFeatureState GUARDED_BY(mFeatureStateLock) =
            ContentFeatureState::CONTENT_DETECTION_OFF;
    IdleTimerState mCurrentIdleTimerState GUARDED_BY(mFeatureStateLock) = IdleTimerState::RESET;
    TouchState mCurrentTouchState GUARDED_BY(mFeatureStateLock) = TouchState::INACTIVE;
    DisplayPowerTimerState mDisplayPowerTimerState GUARDED_BY(mFeatureStateLock) =
            DisplayPowerTimerState::EXPIRED;
    uint32_t mContentRefreshRate GUARDED_BY(mFeatureStateLock);
    RefreshRateType mRefreshRateType GUARDED_BY(mFeatureStateLock);
    bool mIsHDRContent GUARDED_BY(mFeatureStateLock) = false;
    bool mIsDisplayPowerStateNormal GUARDED_BY(mFeatureStateLock) = true;

    const scheduler::RefreshRateConfigs& mRefreshRateConfigs;

    // Global config to force HDR content to work on DEFAULT refreshRate
    static constexpr bool mForceHDRContentToDefaultRefreshRate = false;
};

} // namespace android
