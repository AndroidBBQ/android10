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

#include <gui/IGraphicBufferConsumer.h>

#include <gui/BufferItem.h>
#include <gui/IConsumerListener.h>

#include <binder/Parcel.h>

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <utils/NativeHandle.h>
#include <utils/String8.h>

namespace android {

namespace { // Anonymous namespace

enum class Tag : uint32_t {
    ACQUIRE_BUFFER = IBinder::FIRST_CALL_TRANSACTION,
    DETACH_BUFFER,
    ATTACH_BUFFER,
    RELEASE_BUFFER,
    CONSUMER_CONNECT,
    CONSUMER_DISCONNECT,
    GET_RELEASED_BUFFERS,
    SET_DEFAULT_BUFFER_SIZE,
    SET_MAX_BUFFER_COUNT,
    SET_MAX_ACQUIRED_BUFFER_COUNT,
    SET_CONSUMER_NAME,
    SET_DEFAULT_BUFFER_FORMAT,
    SET_DEFAULT_BUFFER_DATA_SPACE,
    SET_CONSUMER_USAGE_BITS,
    SET_CONSUMER_IS_PROTECTED,
    SET_TRANSFORM_HINT,
    GET_SIDEBAND_STREAM,
    GET_OCCUPANCY_HISTORY,
    DISCARD_FREE_BUFFERS,
    DUMP_STATE,
    LAST = DUMP_STATE,
};

} // Anonymous namespace

class BpGraphicBufferConsumer : public SafeBpInterface<IGraphicBufferConsumer> {
public:
    explicit BpGraphicBufferConsumer(const sp<IBinder>& impl)
          : SafeBpInterface<IGraphicBufferConsumer>(impl, "BpGraphicBufferConsumer") {}

    ~BpGraphicBufferConsumer() override;

    status_t acquireBuffer(BufferItem* buffer, nsecs_t presentWhen,
                           uint64_t maxFrameNumber) override {
        using Signature = decltype(&IGraphicBufferConsumer::acquireBuffer);
        return callRemote<Signature>(Tag::ACQUIRE_BUFFER, buffer, presentWhen, maxFrameNumber);
    }

    status_t detachBuffer(int slot) override {
        using Signature = decltype(&IGraphicBufferConsumer::detachBuffer);
        return callRemote<Signature>(Tag::DETACH_BUFFER, slot);
    }

    status_t attachBuffer(int* slot, const sp<GraphicBuffer>& buffer) override {
        using Signature = decltype(&IGraphicBufferConsumer::attachBuffer);
        return callRemote<Signature>(Tag::ATTACH_BUFFER, slot, buffer);
    }

    status_t releaseBuffer(int buf, uint64_t frameNumber,
                           EGLDisplay display __attribute__((unused)),
                           EGLSyncKHR fence __attribute__((unused)),
                           const sp<Fence>& releaseFence) override {
        return callRemote<ReleaseBuffer>(Tag::RELEASE_BUFFER, buf, frameNumber, releaseFence);
    }

    status_t consumerConnect(const sp<IConsumerListener>& consumer, bool controlledByApp) override {
        using Signature = decltype(&IGraphicBufferConsumer::consumerConnect);
        return callRemote<Signature>(Tag::CONSUMER_CONNECT, consumer, controlledByApp);
    }

    status_t consumerDisconnect() override {
        return callRemote<decltype(&IGraphicBufferConsumer::consumerDisconnect)>(
                Tag::CONSUMER_DISCONNECT);
    }

    status_t getReleasedBuffers(uint64_t* slotMask) override {
        using Signature = decltype(&IGraphicBufferConsumer::getReleasedBuffers);
        return callRemote<Signature>(Tag::GET_RELEASED_BUFFERS, slotMask);
    }

    status_t setDefaultBufferSize(uint32_t width, uint32_t height) override {
        using Signature = decltype(&IGraphicBufferConsumer::setDefaultBufferSize);
        return callRemote<Signature>(Tag::SET_DEFAULT_BUFFER_SIZE, width, height);
    }

    status_t setMaxBufferCount(int bufferCount) override {
        using Signature = decltype(&IGraphicBufferConsumer::setMaxBufferCount);
        return callRemote<Signature>(Tag::SET_MAX_BUFFER_COUNT, bufferCount);
    }

    status_t setMaxAcquiredBufferCount(int maxAcquiredBuffers) override {
        using Signature = decltype(&IGraphicBufferConsumer::setMaxAcquiredBufferCount);
        return callRemote<Signature>(Tag::SET_MAX_ACQUIRED_BUFFER_COUNT, maxAcquiredBuffers);
    }

    status_t setConsumerName(const String8& name) override {
        using Signature = decltype(&IGraphicBufferConsumer::setConsumerName);
        return callRemote<Signature>(Tag::SET_CONSUMER_NAME, name);
    }

    status_t setDefaultBufferFormat(PixelFormat defaultFormat) override {
        using Signature = decltype(&IGraphicBufferConsumer::setDefaultBufferFormat);
        return callRemote<Signature>(Tag::SET_DEFAULT_BUFFER_FORMAT, defaultFormat);
    }

    status_t setDefaultBufferDataSpace(android_dataspace defaultDataSpace) override {
        using Signature = decltype(&IGraphicBufferConsumer::setDefaultBufferDataSpace);
        return callRemote<Signature>(Tag::SET_DEFAULT_BUFFER_DATA_SPACE, defaultDataSpace);
    }

    status_t setConsumerUsageBits(uint64_t usage) override {
        using Signature = decltype(&IGraphicBufferConsumer::setConsumerUsageBits);
        return callRemote<Signature>(Tag::SET_CONSUMER_USAGE_BITS, usage);
    }

    status_t setConsumerIsProtected(bool isProtected) override {
        using Signature = decltype(&IGraphicBufferConsumer::setConsumerIsProtected);
        return callRemote<Signature>(Tag::SET_CONSUMER_IS_PROTECTED, isProtected);
    }

    status_t setTransformHint(uint32_t hint) override {
        using Signature = decltype(&IGraphicBufferConsumer::setTransformHint);
        return callRemote<Signature>(Tag::SET_TRANSFORM_HINT, hint);
    }

    status_t getSidebandStream(sp<NativeHandle>* outStream) const override {
        using Signature = decltype(&IGraphicBufferConsumer::getSidebandStream);
        return callRemote<Signature>(Tag::GET_SIDEBAND_STREAM, outStream);
    }

    status_t getOccupancyHistory(bool forceFlush,
                                 std::vector<OccupancyTracker::Segment>* outHistory) override {
        using Signature = decltype(&IGraphicBufferConsumer::getOccupancyHistory);
        return callRemote<Signature>(Tag::GET_OCCUPANCY_HISTORY, forceFlush, outHistory);
    }

    status_t discardFreeBuffers() override {
        return callRemote<decltype(&IGraphicBufferConsumer::discardFreeBuffers)>(
                Tag::DISCARD_FREE_BUFFERS);
    }

    status_t dumpState(const String8& prefix, String8* outResult) const override {
        using Signature = status_t (IGraphicBufferConsumer::*)(const String8&, String8*) const;
        return callRemote<Signature>(Tag::DUMP_STATE, prefix, outResult);
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this translation unit
// (see clang warning -Wweak-vtables)
BpGraphicBufferConsumer::~BpGraphicBufferConsumer() = default;

IMPLEMENT_META_INTERFACE(GraphicBufferConsumer, "android.gui.IGraphicBufferConsumer");

status_t BnGraphicBufferConsumer::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                             uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::ACQUIRE_BUFFER:
            return callLocal(data, reply, &IGraphicBufferConsumer::acquireBuffer);
        case Tag::DETACH_BUFFER:
            return callLocal(data, reply, &IGraphicBufferConsumer::detachBuffer);
        case Tag::ATTACH_BUFFER:
            return callLocal(data, reply, &IGraphicBufferConsumer::attachBuffer);
        case Tag::RELEASE_BUFFER:
            return callLocal(data, reply, &IGraphicBufferConsumer::releaseHelper);
        case Tag::CONSUMER_CONNECT:
            return callLocal(data, reply, &IGraphicBufferConsumer::consumerConnect);
        case Tag::CONSUMER_DISCONNECT:
            return callLocal(data, reply, &IGraphicBufferConsumer::consumerDisconnect);
        case Tag::GET_RELEASED_BUFFERS:
            return callLocal(data, reply, &IGraphicBufferConsumer::getReleasedBuffers);
        case Tag::SET_DEFAULT_BUFFER_SIZE:
            return callLocal(data, reply, &IGraphicBufferConsumer::setDefaultBufferSize);
        case Tag::SET_MAX_BUFFER_COUNT:
            return callLocal(data, reply, &IGraphicBufferConsumer::setMaxBufferCount);
        case Tag::SET_MAX_ACQUIRED_BUFFER_COUNT:
            return callLocal(data, reply, &IGraphicBufferConsumer::setMaxAcquiredBufferCount);
        case Tag::SET_CONSUMER_NAME:
            return callLocal(data, reply, &IGraphicBufferConsumer::setConsumerName);
        case Tag::SET_DEFAULT_BUFFER_FORMAT:
            return callLocal(data, reply, &IGraphicBufferConsumer::setDefaultBufferFormat);
        case Tag::SET_DEFAULT_BUFFER_DATA_SPACE:
            return callLocal(data, reply, &IGraphicBufferConsumer::setDefaultBufferDataSpace);
        case Tag::SET_CONSUMER_USAGE_BITS:
            return callLocal(data, reply, &IGraphicBufferConsumer::setConsumerUsageBits);
        case Tag::SET_CONSUMER_IS_PROTECTED:
            return callLocal(data, reply, &IGraphicBufferConsumer::setConsumerIsProtected);
        case Tag::SET_TRANSFORM_HINT:
            return callLocal(data, reply, &IGraphicBufferConsumer::setTransformHint);
        case Tag::GET_SIDEBAND_STREAM:
            return callLocal(data, reply, &IGraphicBufferConsumer::getSidebandStream);
        case Tag::GET_OCCUPANCY_HISTORY:
            return callLocal(data, reply, &IGraphicBufferConsumer::getOccupancyHistory);
        case Tag::DISCARD_FREE_BUFFERS:
            return callLocal(data, reply, &IGraphicBufferConsumer::discardFreeBuffers);
        case Tag::DUMP_STATE: {
            using Signature = status_t (IGraphicBufferConsumer::*)(const String8&, String8*) const;
            return callLocal<Signature>(data, reply, &IGraphicBufferConsumer::dumpState);
        }
    }
}

} // namespace android
