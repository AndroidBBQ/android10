/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <sys/types.h>

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <android-base/thread_annotations.h>

#include <gui/DisplayEventReceiver.h>
#include <gui/IDisplayEventConnection.h>
#include <private/gui/BitTube.h>

#include <utils/Errors.h>

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

class EventThread;
class EventThreadTest;
class SurfaceFlinger;

// ---------------------------------------------------------------------------

using ResyncCallback = std::function<void()>;

enum class VSyncRequest {
    None = -1,
    Single = 0,
    Periodic = 1,
    // Subsequent values are periods.
};

class VSyncSource {
public:
    class Callback {
    public:
        virtual ~Callback() {}
        virtual void onVSyncEvent(nsecs_t when) = 0;
    };

    virtual ~VSyncSource() {}
    virtual void setVSyncEnabled(bool enable) = 0;
    virtual void setCallback(Callback* callback) = 0;
    virtual void setPhaseOffset(nsecs_t phaseOffset) = 0;
};

class EventThreadConnection : public BnDisplayEventConnection {
public:
    EventThreadConnection(EventThread*, ResyncCallback,
                          ISurfaceComposer::ConfigChanged configChanged);
    virtual ~EventThreadConnection();

    virtual status_t postEvent(const DisplayEventReceiver::Event& event);

    status_t stealReceiveChannel(gui::BitTube* outChannel) override;
    status_t setVsyncRate(uint32_t rate) override;
    void requestNextVsync() override; // asynchronous

    // Called in response to requestNextVsync.
    const ResyncCallback resyncCallback;

    VSyncRequest vsyncRequest = VSyncRequest::None;
    const ISurfaceComposer::ConfigChanged configChanged;

private:
    virtual void onFirstRef();
    EventThread* const mEventThread;
    gui::BitTube mChannel;
};

class EventThread {
public:
    virtual ~EventThread();

    virtual sp<EventThreadConnection> createEventConnection(
            ResyncCallback, ISurfaceComposer::ConfigChanged configChanged) const = 0;

    // called before the screen is turned off from main thread
    virtual void onScreenReleased() = 0;

    // called after the screen is turned on from main thread
    virtual void onScreenAcquired() = 0;

    virtual void onHotplugReceived(PhysicalDisplayId displayId, bool connected) = 0;

    // called when SF changes the active config and apps needs to be notified about the change
    virtual void onConfigChanged(PhysicalDisplayId displayId, int32_t configId) = 0;

    virtual void dump(std::string& result) const = 0;

    virtual void setPhaseOffset(nsecs_t phaseOffset) = 0;

    virtual status_t registerDisplayEventConnection(
            const sp<EventThreadConnection>& connection) = 0;
    virtual void setVsyncRate(uint32_t rate, const sp<EventThreadConnection>& connection) = 0;
    // Requests the next vsync. If resetIdleTimer is set to true, it resets the idle timer.
    virtual void requestNextVsync(const sp<EventThreadConnection>& connection) = 0;
};

namespace impl {

class EventThread : public android::EventThread, private VSyncSource::Callback {
public:
    using InterceptVSyncsCallback = std::function<void(nsecs_t)>;

    // TODO(b/128863962): Once the Scheduler is complete this constructor will become obsolete.
    EventThread(VSyncSource*, InterceptVSyncsCallback, const char* threadName);
    EventThread(std::unique_ptr<VSyncSource>, InterceptVSyncsCallback, const char* threadName);
    ~EventThread();

    sp<EventThreadConnection> createEventConnection(
            ResyncCallback, ISurfaceComposer::ConfigChanged configChanged) const override;

    status_t registerDisplayEventConnection(const sp<EventThreadConnection>& connection) override;
    void setVsyncRate(uint32_t rate, const sp<EventThreadConnection>& connection) override;
    void requestNextVsync(const sp<EventThreadConnection>& connection) override;

    // called before the screen is turned off from main thread
    void onScreenReleased() override;

    // called after the screen is turned on from main thread
    void onScreenAcquired() override;

    void onHotplugReceived(PhysicalDisplayId displayId, bool connected) override;

    void onConfigChanged(PhysicalDisplayId displayId, int32_t configId) override;

    void dump(std::string& result) const override;

    void setPhaseOffset(nsecs_t phaseOffset) override;

private:
    friend EventThreadTest;

    using DisplayEventConsumers = std::vector<sp<EventThreadConnection>>;

    // TODO(b/128863962): Once the Scheduler is complete this constructor will become obsolete.
    EventThread(VSyncSource* src, std::unique_ptr<VSyncSource> uniqueSrc,
                InterceptVSyncsCallback interceptVSyncsCallback, const char* threadName);

    void threadMain(std::unique_lock<std::mutex>& lock) REQUIRES(mMutex);

    bool shouldConsumeEvent(const DisplayEventReceiver::Event& event,
                            const sp<EventThreadConnection>& connection) const REQUIRES(mMutex);
    void dispatchEvent(const DisplayEventReceiver::Event& event,
                       const DisplayEventConsumers& consumers) REQUIRES(mMutex);

    void removeDisplayEventConnectionLocked(const wp<EventThreadConnection>& connection)
            REQUIRES(mMutex);

    // Implements VSyncSource::Callback
    void onVSyncEvent(nsecs_t timestamp) override;

    // TODO(b/128863962): Once the Scheduler is complete this pointer will become obsolete.
    VSyncSource* mVSyncSource GUARDED_BY(mMutex) = nullptr;
    std::unique_ptr<VSyncSource> mVSyncSourceUnique GUARDED_BY(mMutex) = nullptr;

    const InterceptVSyncsCallback mInterceptVSyncsCallback;
    const char* const mThreadName;

    std::thread mThread;
    mutable std::mutex mMutex;
    mutable std::condition_variable mCondition;

    std::vector<wp<EventThreadConnection>> mDisplayEventConnections GUARDED_BY(mMutex);
    std::deque<DisplayEventReceiver::Event> mPendingEvents GUARDED_BY(mMutex);

    // VSYNC state of connected display.
    struct VSyncState {
        explicit VSyncState(PhysicalDisplayId displayId) : displayId(displayId) {}

        const PhysicalDisplayId displayId;

        // Number of VSYNC events since display was connected.
        uint32_t count = 0;

        // True if VSYNC should be faked, e.g. when display is off.
        bool synthetic = false;
    };

    // TODO(b/74619554): Create per-display threads waiting on respective VSYNC signals,
    // and support headless mode by injecting a fake display with synthetic VSYNC.
    std::optional<VSyncState> mVSyncState GUARDED_BY(mMutex);

    // State machine for event loop.
    enum class State {
        Idle,
        Quit,
        SyntheticVSync,
        VSync,
    };

    State mState GUARDED_BY(mMutex) = State::Idle;

    static const char* toCString(State);
};

// ---------------------------------------------------------------------------

} // namespace impl
} // namespace android
