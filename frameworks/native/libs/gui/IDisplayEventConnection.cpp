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

#include <gui/IDisplayEventConnection.h>

#include <private/gui/BitTube.h>

namespace android {

namespace { // Anonymous

enum class Tag : uint32_t {
    STEAL_RECEIVE_CHANNEL = IBinder::FIRST_CALL_TRANSACTION,
    SET_VSYNC_RATE,
    REQUEST_NEXT_VSYNC,
    LAST = REQUEST_NEXT_VSYNC,
};

} // Anonymous namespace

class BpDisplayEventConnection : public SafeBpInterface<IDisplayEventConnection> {
public:
    explicit BpDisplayEventConnection(const sp<IBinder>& impl)
          : SafeBpInterface<IDisplayEventConnection>(impl, "BpDisplayEventConnection") {}

    ~BpDisplayEventConnection() override;

    status_t stealReceiveChannel(gui::BitTube* outChannel) override {
        return callRemote<decltype(
                &IDisplayEventConnection::stealReceiveChannel)>(Tag::STEAL_RECEIVE_CHANNEL,
                                                                outChannel);
    }

    status_t setVsyncRate(uint32_t count) override {
        return callRemote<decltype(&IDisplayEventConnection::setVsyncRate)>(Tag::SET_VSYNC_RATE,
                                                                            count);
    }

    void requestNextVsync() override {
        callRemoteAsync<decltype(&IDisplayEventConnection::requestNextVsync)>(
                Tag::REQUEST_NEXT_VSYNC);
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this translation unit (see
// clang warning -Wweak-vtables)
BpDisplayEventConnection::~BpDisplayEventConnection() = default;

IMPLEMENT_META_INTERFACE(DisplayEventConnection, "android.gui.DisplayEventConnection");

status_t BnDisplayEventConnection::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                              uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::STEAL_RECEIVE_CHANNEL:
            return callLocal(data, reply, &IDisplayEventConnection::stealReceiveChannel);
        case Tag::SET_VSYNC_RATE:
            return callLocal(data, reply, &IDisplayEventConnection::setVsyncRate);
        case Tag::REQUEST_NEXT_VSYNC:
            return callLocalAsync(data, reply, &IDisplayEventConnection::requestNextVsync);
    }
}

} // namespace android
