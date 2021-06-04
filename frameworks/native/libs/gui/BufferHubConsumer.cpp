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

#include <gui/BufferHubConsumer.h>

namespace android {

using namespace dvr;

/* static */
sp<BufferHubConsumer> BufferHubConsumer::Create(const std::shared_ptr<ConsumerQueue>& queue) {
    sp<BufferHubConsumer> consumer = new BufferHubConsumer;
    consumer->mQueue = queue;
    return consumer;
}

/* static */ sp<BufferHubConsumer> BufferHubConsumer::Create(ConsumerQueueParcelable parcelable) {
    if (!parcelable.IsValid()) {
        ALOGE("BufferHubConsumer::Create: Invalid consumer parcelable.");
        return nullptr;
    }

    sp<BufferHubConsumer> consumer = new BufferHubConsumer;
    consumer->mQueue = ConsumerQueue::Import(parcelable.TakeChannelHandle());
    return consumer;
}

status_t BufferHubConsumer::acquireBuffer(BufferItem* /*buffer*/, nsecs_t /*presentWhen*/,
                                          uint64_t /*maxFrameNumber*/) {
    ALOGE("BufferHubConsumer::acquireBuffer: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::detachBuffer(int /*slot*/) {
    ALOGE("BufferHubConsumer::detachBuffer: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::attachBuffer(int* /*outSlot*/, const sp<GraphicBuffer>& /*buffer*/) {
    ALOGE("BufferHubConsumer::attachBuffer: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::releaseBuffer(int /*buf*/, uint64_t /*frameNumber*/,
                                          EGLDisplay /*display*/, EGLSyncKHR /*fence*/,
                                          const sp<Fence>& /*releaseFence*/) {
    ALOGE("BufferHubConsumer::releaseBuffer: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::consumerConnect(const sp<IConsumerListener>& /*consumer*/,
                                            bool /*controlledByApp*/) {
    ALOGE("BufferHubConsumer::consumerConnect: not implemented.");

    // TODO(b/73267953): Make BufferHub honor producer and consumer connection. Returns NO_ERROR to
    // make IGraphicBufferConsumer_test happy.
    return NO_ERROR;
}

status_t BufferHubConsumer::consumerDisconnect() {
    ALOGE("BufferHubConsumer::consumerDisconnect: not implemented.");

    // TODO(b/73267953): Make BufferHub honor producer and consumer connection. Returns NO_ERROR to
    // make IGraphicBufferConsumer_test happy.
    return NO_ERROR;
}

status_t BufferHubConsumer::getReleasedBuffers(uint64_t* /*slotMask*/) {
    ALOGE("BufferHubConsumer::getReleasedBuffers: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setDefaultBufferSize(uint32_t /*w*/, uint32_t /*h*/) {
    ALOGE("BufferHubConsumer::setDefaultBufferSize: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setMaxBufferCount(int /*bufferCount*/) {
    ALOGE("BufferHubConsumer::setMaxBufferCount: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setMaxAcquiredBufferCount(int /*maxAcquiredBuffers*/) {
    ALOGE("BufferHubConsumer::setMaxAcquiredBufferCount: not implemented.");

    // TODO(b/73267953): Make BufferHub honor producer and consumer connection. Returns NO_ERROR to
    // make IGraphicBufferConsumer_test happy.
    return NO_ERROR;
}

status_t BufferHubConsumer::setConsumerName(const String8& /*name*/) {
    ALOGE("BufferHubConsumer::setConsumerName: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setDefaultBufferFormat(PixelFormat /*defaultFormat*/) {
    ALOGE("BufferHubConsumer::setDefaultBufferFormat: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setDefaultBufferDataSpace(android_dataspace /*defaultDataSpace*/) {
    ALOGE("BufferHubConsumer::setDefaultBufferDataSpace: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setConsumerUsageBits(uint64_t /*usage*/) {
    ALOGE("BufferHubConsumer::setConsumerUsageBits: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setConsumerIsProtected(bool /*isProtected*/) {
    ALOGE("BufferHubConsumer::setConsumerIsProtected: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::setTransformHint(uint32_t /*hint*/) {
    ALOGE("BufferHubConsumer::setTransformHint: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::getSidebandStream(sp<NativeHandle>* /*outStream*/) const {
    ALOGE("BufferHubConsumer::getSidebandStream: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::getOccupancyHistory(
        bool /*forceFlush*/, std::vector<OccupancyTracker::Segment>* /*outHistory*/) {
    ALOGE("BufferHubConsumer::getOccupancyHistory: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::discardFreeBuffers() {
    ALOGE("BufferHubConsumer::discardFreeBuffers: not implemented.");
    return INVALID_OPERATION;
}

status_t BufferHubConsumer::dumpState(const String8& /*prefix*/, String8* /*outResult*/) const {
    ALOGE("BufferHubConsumer::dumpState: not implemented.");
    return INVALID_OPERATION;
}

IBinder* BufferHubConsumer::onAsBinder() {
    ALOGE("BufferHubConsumer::onAsBinder: BufferHubConsumer should never be used as an Binder "
          "object.");
    return nullptr;
}

} // namespace android
