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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HGRAPHICBUFFERPRODUCER_H
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HGRAPHICBUFFERPRODUCER_H

#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/bufferqueue/2.0/types.h>
#include <hidl/HidlSupport.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

using HGraphicBufferProducer =
        ::android::hardware::graphics::bufferqueue::V2_0::
        IGraphicBufferProducer;
using BGraphicBufferProducer =
        ::android::
        IGraphicBufferProducer;
using HProducerListener =
        ::android::hardware::graphics::bufferqueue::V2_0::
        IProducerListener;

using ::android::hardware::Return;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

using ::android::hardware::graphics::common::V1_2::HardwareBuffer;

class B2HGraphicBufferProducer : public HGraphicBufferProducer {
public:
    B2HGraphicBufferProducer(sp<BGraphicBufferProducer> const& base);

    virtual Return<HStatus> setMaxDequeuedBufferCount(
            int32_t maxDequeuedBuffers) override;

    virtual Return<void> requestBuffer(
            int32_t slot,
            requestBuffer_cb _hidl_cb) override;

    virtual Return<HStatus> setAsyncMode(bool async) override;

    virtual Return<void> dequeueBuffer(
            DequeueBufferInput const& input,
            dequeueBuffer_cb _hidl_cb) override;

    virtual Return<HStatus> detachBuffer(int32_t slot) override;

    virtual Return<void> detachNextBuffer(
            detachNextBuffer_cb _hidl_cb) override;

    virtual Return<void> attachBuffer(
            HardwareBuffer const& buffer,
            uint32_t generationNumber,
            attachBuffer_cb _hidl_cb) override;

    virtual Return<void> queueBuffer(
            int32_t slot,
            QueueBufferInput const& input,
            queueBuffer_cb _hidl_cb) override;

    virtual Return<HStatus> cancelBuffer(
            int32_t slot,
            hidl_handle const& fence) override;

    virtual Return<void> query(int32_t what, query_cb _hidl_cb) override;

    virtual Return<void> connect(
            sp<HProducerListener> const& listener,
            HConnectionType api,
            bool producerControlledByApp,
            connect_cb _hidl_cb) override;

    virtual Return<HStatus> disconnect(HConnectionType api) override;

    virtual Return<HStatus> allocateBuffers(
            uint32_t width, uint32_t height,
            uint32_t format, uint64_t usage) override;

    virtual Return<HStatus> allowAllocation(bool allow) override;

    virtual Return<HStatus> setGenerationNumber(uint32_t generationNumber) override;

    virtual Return<HStatus> setDequeueTimeout(int64_t timeoutNs) override;

    virtual Return<uint64_t> getUniqueId() override;

    virtual Return<void> getConsumerName(getConsumerName_cb _hidl_cb) override;

protected:
    sp<BGraphicBufferProducer> mBase;
};


}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HGRAPHICBUFFERPRODUCER_H
