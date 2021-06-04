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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <pthread.h>
#include <sched.h>
#include <sys/types.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <type_traits>

#include <android-base/stringprintf.h>

#include <cutils/compiler.h>
#include <cutils/sched_policy.h>

#include <gui/DisplayEventReceiver.h>

#include <utils/Errors.h>
#include <utils/Trace.h>

#include "EventThread.h"

using namespace std::chrono_literals;

namespace android {

using base::StringAppendF;
using base::StringPrintf;

namespace {

auto vsyncPeriod(VSyncRequest request) {
    return static_cast<std::underlying_type_t<VSyncRequest>>(request);
}

std::string toString(VSyncRequest request) {
    switch (request) {
        case VSyncRequest::None:
            return "VSyncRequest::None";
        case VSyncRequest::Single:
            return "VSyncRequest::Single";
        default:
            return StringPrintf("VSyncRequest::Periodic{period=%d}", vsyncPeriod(request));
    }
}

std::string toString(const EventThreadConnection& connection) {
    return StringPrintf("Connection{%p, %s}", &connection,
                        toString(connection.vsyncRequest).c_str());
}

std::string toString(const DisplayEventReceiver::Event& event) {
    switch (event.header.type) {
        case DisplayEventReceiver::DISPLAY_EVENT_HOTPLUG:
            return StringPrintf("Hotplug{displayId=%" ANDROID_PHYSICAL_DISPLAY_ID_FORMAT ", %s}",
                                event.header.displayId,
                                event.hotplug.connected ? "connected" : "disconnected");
        case DisplayEventReceiver::DISPLAY_EVENT_VSYNC:
            return StringPrintf("VSync{displayId=%" ANDROID_PHYSICAL_DISPLAY_ID_FORMAT
                                ", count=%u}",
                                event.header.displayId, event.vsync.count);
        case DisplayEventReceiver::DISPLAY_EVENT_CONFIG_CHANGED:
            return StringPrintf("ConfigChanged{displayId=%" ANDROID_PHYSICAL_DISPLAY_ID_FORMAT
                                ", configId=%u}",
                                event.header.displayId, event.config.configId);
        default:
            return "Event{}";
    }
}

DisplayEventReceiver::Event makeHotplug(PhysicalDisplayId displayId, nsecs_t timestamp,
                                        bool connected) {
    DisplayEventReceiver::Event event;
    event.header = {DisplayEventReceiver::DISPLAY_EVENT_HOTPLUG, displayId, timestamp};
    event.hotplug.connected = connected;
    return event;
}

DisplayEventReceiver::Event makeVSync(PhysicalDisplayId displayId, nsecs_t timestamp,
                                      uint32_t count) {
    DisplayEventReceiver::Event event;
    event.header = {DisplayEventReceiver::DISPLAY_EVENT_VSYNC, displayId, timestamp};
    event.vsync.count = count;
    return event;
}

DisplayEventReceiver::Event makeConfigChanged(PhysicalDisplayId displayId, int32_t configId) {
    DisplayEventReceiver::Event event;
    event.header = {DisplayEventReceiver::DISPLAY_EVENT_CONFIG_CHANGED, displayId, systemTime()};
    event.config.configId = configId;
    return event;
}

} // namespace

EventThreadConnection::EventThreadConnection(EventThread* eventThread,
                                             ResyncCallback resyncCallback,
                                             ISurfaceComposer::ConfigChanged configChanged)
      : resyncCallback(std::move(resyncCallback)),
        configChanged(configChanged),
        mEventThread(eventThread),
        mChannel(gui::BitTube::DefaultSize) {}

EventThreadConnection::~EventThreadConnection() {
    // do nothing here -- clean-up will happen automatically
    // when the main thread wakes up
}

void EventThreadConnection::onFirstRef() {
    // NOTE: mEventThread doesn't hold a strong reference on us
    mEventThread->registerDisplayEventConnection(this);
}

status_t EventThreadConnection::stealReceiveChannel(gui::BitTube* outChannel) {
    outChannel->setReceiveFd(mChannel.moveReceiveFd());
    return NO_ERROR;
}

status_t EventThreadConnection::setVsyncRate(uint32_t rate) {
    mEventThread->setVsyncRate(rate, this);
    return NO_ERROR;
}

void EventThreadConnection::requestNextVsync() {
    ATRACE_NAME("requestNextVsync");
    mEventThread->requestNextVsync(this);
}

status_t EventThreadConnection::postEvent(const DisplayEventReceiver::Event& event) {
    ssize_t size = DisplayEventReceiver::sendEvents(&mChannel, &event, 1);
    return size < 0 ? status_t(size) : status_t(NO_ERROR);
}

// ---------------------------------------------------------------------------

EventThread::~EventThread() = default;

namespace impl {

EventThread::EventThread(std::unique_ptr<VSyncSource> src,
                         InterceptVSyncsCallback interceptVSyncsCallback, const char* threadName)
      : EventThread(nullptr, std::move(src), std::move(interceptVSyncsCallback), threadName) {}

EventThread::EventThread(VSyncSource* src, InterceptVSyncsCallback interceptVSyncsCallback,
                         const char* threadName)
      : EventThread(src, nullptr, std::move(interceptVSyncsCallback), threadName) {}

EventThread::EventThread(VSyncSource* src, std::unique_ptr<VSyncSource> uniqueSrc,
                         InterceptVSyncsCallback interceptVSyncsCallback, const char* threadName)
      : mVSyncSource(src),
        mVSyncSourceUnique(std::move(uniqueSrc)),
        mInterceptVSyncsCallback(std::move(interceptVSyncsCallback)),
        mThreadName(threadName) {
    if (src == nullptr) {
        mVSyncSource = mVSyncSourceUnique.get();
    }
    mVSyncSource->setCallback(this);

    mThread = std::thread([this]() NO_THREAD_SAFETY_ANALYSIS {
        std::unique_lock<std::mutex> lock(mMutex);
        threadMain(lock);
    });

    pthread_setname_np(mThread.native_handle(), threadName);

    pid_t tid = pthread_gettid_np(mThread.native_handle());

    // Use SCHED_FIFO to minimize jitter
    constexpr int EVENT_THREAD_PRIORITY = 2;
    struct sched_param param = {0};
    param.sched_priority = EVENT_THREAD_PRIORITY;
    if (pthread_setschedparam(mThread.native_handle(), SCHED_FIFO, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO for EventThread");
    }

    set_sched_policy(tid, SP_FOREGROUND);
}

EventThread::~EventThread() {
    mVSyncSource->setCallback(nullptr);

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mState = State::Quit;
        mCondition.notify_all();
    }
    mThread.join();
}

void EventThread::setPhaseOffset(nsecs_t phaseOffset) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVSyncSource->setPhaseOffset(phaseOffset);
}

sp<EventThreadConnection> EventThread::createEventConnection(
        ResyncCallback resyncCallback, ISurfaceComposer::ConfigChanged configChanged) const {
    return new EventThreadConnection(const_cast<EventThread*>(this), std::move(resyncCallback),
                                     configChanged);
}

status_t EventThread::registerDisplayEventConnection(const sp<EventThreadConnection>& connection) {
    std::lock_guard<std::mutex> lock(mMutex);

    // this should never happen
    auto it = std::find(mDisplayEventConnections.cbegin(),
            mDisplayEventConnections.cend(), connection);
    if (it != mDisplayEventConnections.cend()) {
        ALOGW("DisplayEventConnection %p already exists", connection.get());
        mCondition.notify_all();
        return ALREADY_EXISTS;
    }

    mDisplayEventConnections.push_back(connection);
    mCondition.notify_all();
    return NO_ERROR;
}

void EventThread::removeDisplayEventConnectionLocked(const wp<EventThreadConnection>& connection) {
    auto it = std::find(mDisplayEventConnections.cbegin(),
            mDisplayEventConnections.cend(), connection);
    if (it != mDisplayEventConnections.cend()) {
        mDisplayEventConnections.erase(it);
    }
}

void EventThread::setVsyncRate(uint32_t rate, const sp<EventThreadConnection>& connection) {
    if (static_cast<std::underlying_type_t<VSyncRequest>>(rate) < 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(mMutex);

    const auto request = rate == 0 ? VSyncRequest::None : static_cast<VSyncRequest>(rate);
    if (connection->vsyncRequest != request) {
        connection->vsyncRequest = request;
        mCondition.notify_all();
    }
}

void EventThread::requestNextVsync(const sp<EventThreadConnection>& connection) {
    if (connection->resyncCallback) {
        connection->resyncCallback();
    }

    std::lock_guard<std::mutex> lock(mMutex);

    if (connection->vsyncRequest == VSyncRequest::None) {
        connection->vsyncRequest = VSyncRequest::Single;
        mCondition.notify_all();
    }
}

void EventThread::onScreenReleased() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mVSyncState || mVSyncState->synthetic) {
        return;
    }

    mVSyncState->synthetic = true;
    mCondition.notify_all();
}

void EventThread::onScreenAcquired() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mVSyncState || !mVSyncState->synthetic) {
        return;
    }

    mVSyncState->synthetic = false;
    mCondition.notify_all();
}

void EventThread::onVSyncEvent(nsecs_t timestamp) {
    std::lock_guard<std::mutex> lock(mMutex);

    LOG_FATAL_IF(!mVSyncState);
    mPendingEvents.push_back(makeVSync(mVSyncState->displayId, timestamp, ++mVSyncState->count));
    mCondition.notify_all();
}

void EventThread::onHotplugReceived(PhysicalDisplayId displayId, bool connected) {
    std::lock_guard<std::mutex> lock(mMutex);

    mPendingEvents.push_back(makeHotplug(displayId, systemTime(), connected));
    mCondition.notify_all();
}

void EventThread::onConfigChanged(PhysicalDisplayId displayId, int32_t configId) {
    std::lock_guard<std::mutex> lock(mMutex);

    mPendingEvents.push_back(makeConfigChanged(displayId, configId));
    mCondition.notify_all();
}

void EventThread::threadMain(std::unique_lock<std::mutex>& lock) {
    DisplayEventConsumers consumers;

    while (mState != State::Quit) {
        std::optional<DisplayEventReceiver::Event> event;

        // Determine next event to dispatch.
        if (!mPendingEvents.empty()) {
            event = mPendingEvents.front();
            mPendingEvents.pop_front();

            switch (event->header.type) {
                case DisplayEventReceiver::DISPLAY_EVENT_HOTPLUG:
                    if (event->hotplug.connected && !mVSyncState) {
                        mVSyncState.emplace(event->header.displayId);
                    } else if (!event->hotplug.connected && mVSyncState &&
                               mVSyncState->displayId == event->header.displayId) {
                        mVSyncState.reset();
                    }
                    break;

                case DisplayEventReceiver::DISPLAY_EVENT_VSYNC:
                    if (mInterceptVSyncsCallback) {
                        mInterceptVSyncsCallback(event->header.timestamp);
                    }
                    break;
            }
        }

        bool vsyncRequested = false;

        // Find connections that should consume this event.
        auto it = mDisplayEventConnections.begin();
        while (it != mDisplayEventConnections.end()) {
            if (const auto connection = it->promote()) {
                vsyncRequested |= connection->vsyncRequest != VSyncRequest::None;

                if (event && shouldConsumeEvent(*event, connection)) {
                    consumers.push_back(connection);
                }

                ++it;
            } else {
                it = mDisplayEventConnections.erase(it);
            }
        }

        if (!consumers.empty()) {
            dispatchEvent(*event, consumers);
            consumers.clear();
        }

        State nextState;
        if (mVSyncState && vsyncRequested) {
            nextState = mVSyncState->synthetic ? State::SyntheticVSync : State::VSync;
        } else {
            ALOGW_IF(!mVSyncState, "Ignoring VSYNC request while display is disconnected");
            nextState = State::Idle;
        }

        if (mState != nextState) {
            if (mState == State::VSync) {
                mVSyncSource->setVSyncEnabled(false);
            } else if (nextState == State::VSync) {
                mVSyncSource->setVSyncEnabled(true);
            }

            mState = nextState;
        }

        if (event) {
            continue;
        }

        // Wait for event or client registration/request.
        if (mState == State::Idle) {
            mCondition.wait(lock);
        } else {
            // Generate a fake VSYNC after a long timeout in case the driver stalls. When the
            // display is off, keep feeding clients at 60 Hz.
            const auto timeout = mState == State::SyntheticVSync ? 16ms : 1000ms;
            if (mCondition.wait_for(lock, timeout) == std::cv_status::timeout) {
                ALOGW_IF(mState == State::VSync, "Faking VSYNC due to driver stall");

                LOG_FATAL_IF(!mVSyncState);
                mPendingEvents.push_back(makeVSync(mVSyncState->displayId,
                                                   systemTime(SYSTEM_TIME_MONOTONIC),
                                                   ++mVSyncState->count));
            }
        }
    }
}

bool EventThread::shouldConsumeEvent(const DisplayEventReceiver::Event& event,
                                     const sp<EventThreadConnection>& connection) const {
    switch (event.header.type) {
        case DisplayEventReceiver::DISPLAY_EVENT_HOTPLUG:
            return true;

        case DisplayEventReceiver::DISPLAY_EVENT_CONFIG_CHANGED:
            return connection->configChanged == ISurfaceComposer::eConfigChangedDispatch;

        case DisplayEventReceiver::DISPLAY_EVENT_VSYNC:
            switch (connection->vsyncRequest) {
                case VSyncRequest::None:
                    return false;
                case VSyncRequest::Single:
                    connection->vsyncRequest = VSyncRequest::None;
                    return true;
                case VSyncRequest::Periodic:
                    return true;
                default:
                    return event.vsync.count % vsyncPeriod(connection->vsyncRequest) == 0;
            }

        default:
            return false;
    }
}

void EventThread::dispatchEvent(const DisplayEventReceiver::Event& event,
                                const DisplayEventConsumers& consumers) {
    for (const auto& consumer : consumers) {
        switch (consumer->postEvent(event)) {
            case NO_ERROR:
                break;

            case -EAGAIN:
                // TODO: Try again if pipe is full.
                ALOGW("Failed dispatching %s for %s", toString(event).c_str(),
                      toString(*consumer).c_str());
                break;

            default:
                // Treat EPIPE and other errors as fatal.
                removeDisplayEventConnectionLocked(consumer);
        }
    }
}

void EventThread::dump(std::string& result) const {
    std::lock_guard<std::mutex> lock(mMutex);

    StringAppendF(&result, "%s: state=%s VSyncState=", mThreadName, toCString(mState));
    if (mVSyncState) {
        StringAppendF(&result, "{displayId=%" ANDROID_PHYSICAL_DISPLAY_ID_FORMAT ", count=%u%s}\n",
                      mVSyncState->displayId, mVSyncState->count,
                      mVSyncState->synthetic ? ", synthetic" : "");
    } else {
        StringAppendF(&result, "none\n");
    }

    StringAppendF(&result, "  pending events (count=%zu):\n", mPendingEvents.size());
    for (const auto& event : mPendingEvents) {
        StringAppendF(&result, "    %s\n", toString(event).c_str());
    }

    StringAppendF(&result, "  connections (count=%zu):\n", mDisplayEventConnections.size());
    for (const auto& ptr : mDisplayEventConnections) {
        if (const auto connection = ptr.promote()) {
            StringAppendF(&result, "    %s\n", toString(*connection).c_str());
        }
    }
}

const char* EventThread::toCString(State state) {
    switch (state) {
        case State::Idle:
            return "Idle";
        case State::Quit:
            return "Quit";
        case State::SyntheticVSync:
            return "SyntheticVSync";
        case State::VSync:
            return "VSync";
    }
}

} // namespace impl
} // namespace android
