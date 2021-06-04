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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_H2BGRAPHICBUFFERPRODUCER_H
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_H2BGRAPHICBUFFERPRODUCER_H

#include <gui/IGraphicBufferProducer.h>
#include <gui/IProducerListener.h>
#include <hidl/HybridInterface.h>
#include <ui/Fence.h>

#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

using ::android::BnGraphicBufferProducer;
using ::android::IProducerListener;
using Fence = ::android::Fence;

using HGraphicBufferProducer =
        ::android::hardware::graphics::bufferqueue::V2_0::
        IGraphicBufferProducer;
using HProducerListener =
        ::android::hardware::graphics::bufferqueue::V2_0::
        IProducerListener;
using BGraphicBufferProducer =
        ::android::IGraphicBufferProducer;

struct H2BGraphicBufferProducer
      : public ::android::H2BConverter<HGraphicBufferProducer,
                                       BnGraphicBufferProducer> {
    explicit H2BGraphicBufferProducer(
            sp<HGraphicBufferProducer> const& base) : CBase(base) {}

    virtual status_t requestBuffer(int slot, sp<GraphicBuffer>* buf) override;
    virtual status_t setMaxDequeuedBufferCount(int maxDequeuedBuffers) override;
    virtual status_t setAsyncMode(bool async) override;
    virtual status_t dequeueBuffer(
            int* slot, sp<Fence>* fence,
            uint32_t width, uint32_t height,
            PixelFormat format, uint64_t usage,
            uint64_t* outBufferAge,
            FrameEventHistoryDelta* outTimestamps) override;
    virtual status_t detachBuffer(int slot) override;
    virtual status_t detachNextBuffer(
            sp<GraphicBuffer>* outBuffer,
            sp<Fence>* outFence) override;
    virtual status_t attachBuffer(
            int* outSlot,
            sp<GraphicBuffer> const& buffer) override;
    virtual status_t queueBuffer(
            int slot,
            QueueBufferInput const& input,
            QueueBufferOutput* output) override;
    virtual status_t cancelBuffer(int slot, sp<Fence> const& fence) override;
    virtual int query(int what, int* value) override;
    virtual status_t connect(
            sp<IProducerListener> const& listener,
            int api,
            bool producerControlledByApp,
            QueueBufferOutput* output) override;
    virtual status_t disconnect(
            int api,
            DisconnectMode mode = DisconnectMode::Api) override;
    virtual status_t setSidebandStream(sp<NativeHandle> const& stream) override;
    virtual void allocateBuffers(
            uint32_t width, uint32_t height,
            PixelFormat format, uint64_t usage) override;
    virtual status_t allowAllocation(bool allow) override;
    virtual status_t setGenerationNumber(uint32_t generationNumber) override;
    virtual String8 getConsumerName() const override;
    virtual status_t setSharedBufferMode(bool sharedBufferMode) override;
    virtual status_t setAutoRefresh(bool autoRefresh) override;
    virtual status_t setDequeueTimeout(nsecs_t timeout) override;
    virtual status_t getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer,
          sp<Fence>* outFence, float outTransformMatrix[16]) override;
    virtual void getFrameTimestamps(FrameEventHistoryDelta* outDelta) override;
    virtual status_t getUniqueId(uint64_t* outId) const override;
    virtual status_t getConsumerUsage(uint64_t* outUsage) const override;
};

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_H2BGRAPHICBUFFERPRODUCER_H
