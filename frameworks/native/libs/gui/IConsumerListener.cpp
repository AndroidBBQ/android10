/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <gui/IConsumerListener.h>

#include <gui/BufferItem.h>

namespace android {

namespace { // Anonymous

enum class Tag : uint32_t {
    ON_DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    ON_FRAME_AVAILABLE,
    ON_FRAME_REPLACED,
    ON_BUFFERS_RELEASED,
    ON_SIDEBAND_STREAM_CHANGED,
    LAST = ON_SIDEBAND_STREAM_CHANGED,
};

} // Anonymous namespace

class BpConsumerListener : public SafeBpInterface<IConsumerListener> {
public:
    explicit BpConsumerListener(const sp<IBinder>& impl)
          : SafeBpInterface<IConsumerListener>(impl, "BpConsumerListener") {}

    ~BpConsumerListener() override;

    void onDisconnect() override {
        callRemoteAsync<decltype(&IConsumerListener::onDisconnect)>(Tag::ON_DISCONNECT);
    }

    void onFrameAvailable(const BufferItem& item) override {
        callRemoteAsync<decltype(&IConsumerListener::onFrameAvailable)>(Tag::ON_FRAME_AVAILABLE,
                                                                        item);
    }

    void onFrameReplaced(const BufferItem& item) override {
        callRemoteAsync<decltype(&IConsumerListener::onFrameReplaced)>(Tag::ON_FRAME_REPLACED,
                                                                       item);
    }

    void onBuffersReleased() override {
        callRemoteAsync<decltype(&IConsumerListener::onBuffersReleased)>(Tag::ON_BUFFERS_RELEASED);
    }

    void onSidebandStreamChanged() override {
        callRemoteAsync<decltype(&IConsumerListener::onSidebandStreamChanged)>(
                Tag::ON_SIDEBAND_STREAM_CHANGED);
    }

    void addAndGetFrameTimestamps(const NewFrameEventsEntry* /*newTimestamps*/,
                                  FrameEventHistoryDelta* /*outDelta*/) override {
        LOG_ALWAYS_FATAL("IConsumerListener::addAndGetFrameTimestamps cannot be proxied");
    }
};

// Out-of-line virtual method definitions to trigger vtable emission in this translation unit (see
// clang warning -Wweak-vtables)
BpConsumerListener::~BpConsumerListener() = default;
ConsumerListener::~ConsumerListener() = default;

IMPLEMENT_META_INTERFACE(ConsumerListener, "android.gui.IConsumerListener");

status_t BnConsumerListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                        uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::ON_DISCONNECT:
            return callLocalAsync(data, reply, &IConsumerListener::onDisconnect);
        case Tag::ON_FRAME_AVAILABLE:
            return callLocalAsync(data, reply, &IConsumerListener::onFrameAvailable);
        case Tag::ON_FRAME_REPLACED:
            return callLocalAsync(data, reply, &IConsumerListener::onFrameReplaced);
        case Tag::ON_BUFFERS_RELEASED:
            return callLocalAsync(data, reply, &IConsumerListener::onBuffersReleased);
        case Tag::ON_SIDEBAND_STREAM_CHANGED:
            return callLocalAsync(data, reply, &IConsumerListener::onSidebandStreamChanged);
    }
}

} // namespace android
