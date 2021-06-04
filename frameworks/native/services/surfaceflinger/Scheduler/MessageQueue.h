/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_MESSAGE_QUEUE_H
#define ANDROID_MESSAGE_QUEUE_H

#include <errno.h>
#include <stdint.h>
#include <sys/types.h>

#include <utils/Looper.h>
#include <utils/Timers.h>
#include <utils/threads.h>

#include <gui/IDisplayEventConnection.h>
#include <private/gui/BitTube.h>

#include "Barrier.h"
#include "EventThread.h"

#include <functional>

namespace android {

class SurfaceFlinger;

// ---------------------------------------------------------------------------

class MessageBase : public MessageHandler {
public:
    MessageBase();

    // return true if message has a handler
    virtual bool handler() = 0;

    // waits for the handler to be processed
    void wait() const { barrier.wait(); }

protected:
    virtual ~MessageBase();

private:
    virtual void handleMessage(const Message& message);

    mutable Barrier barrier;
};

class LambdaMessage : public MessageBase {
public:
    explicit LambdaMessage(std::function<void()> handler)
          : MessageBase(), mHandler(std::move(handler)) {}

    bool handler() override {
        mHandler();
        // This return value is no longer checked, so it's always safe to return true
        return true;
    }

private:
    const std::function<void()> mHandler;
};

// ---------------------------------------------------------------------------

class MessageQueue {
public:
    enum {
        INVALIDATE = 0,
        REFRESH = 1,
    };

    virtual ~MessageQueue();

    virtual void init(const sp<SurfaceFlinger>& flinger) = 0;
    // TODO(b/128863962): Remove this function once everything is migrated to Scheduler.
    virtual void setEventThread(EventThread* events, ResyncCallback resyncCallback) = 0;
    virtual void setEventConnection(const sp<EventThreadConnection>& connection) = 0;
    virtual void waitMessage() = 0;
    virtual status_t postMessage(const sp<MessageBase>& message, nsecs_t reltime = 0) = 0;
    virtual void invalidate() = 0;
    virtual void refresh() = 0;
};

// ---------------------------------------------------------------------------

namespace impl {

class MessageQueue final : public android::MessageQueue {
    class Handler : public MessageHandler {
        enum { eventMaskInvalidate = 0x1, eventMaskRefresh = 0x2, eventMaskTransaction = 0x4 };
        MessageQueue& mQueue;
        int32_t mEventMask;

    public:
        explicit Handler(MessageQueue& queue) : mQueue(queue), mEventMask(0) {}
        virtual void handleMessage(const Message& message);
        void dispatchRefresh();
        void dispatchInvalidate();
    };

    friend class Handler;

    sp<SurfaceFlinger> mFlinger;
    sp<Looper> mLooper;
    android::EventThread* mEventThread;
    sp<EventThreadConnection> mEvents;
    gui::BitTube mEventTube;
    sp<Handler> mHandler;

    static int cb_eventReceiver(int fd, int events, void* data);
    int eventReceiver(int fd, int events);

public:
    ~MessageQueue() override = default;
    void init(const sp<SurfaceFlinger>& flinger) override;
    void setEventThread(android::EventThread* events, ResyncCallback resyncCallback) override;
    void setEventConnection(const sp<EventThreadConnection>& connection) override;

    void waitMessage() override;
    status_t postMessage(const sp<MessageBase>& message, nsecs_t reltime = 0) override;

    // sends INVALIDATE message at next VSYNC
    void invalidate() override;

    // sends REFRESH message at next VSYNC
    void refresh() override;
};

// ---------------------------------------------------------------------------

} // namespace impl
} // namespace android

#endif /* ANDROID_MESSAGE_QUEUE_H */
