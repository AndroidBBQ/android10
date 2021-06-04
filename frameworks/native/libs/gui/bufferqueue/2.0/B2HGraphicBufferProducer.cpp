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

//#define LOG_NDEBUG 0
#define LOG_TAG "B2HGraphicBufferProducer@2.0"

#include <android-base/logging.h>

#include <android/hardware/graphics/bufferqueue/2.0/types.h>
#include <android/hardware/graphics/common/1.2/types.h>
#include <gui/bufferqueue/2.0/H2BProducerListener.h>
#include <gui/bufferqueue/2.0/B2HGraphicBufferProducer.h>
#include <gui/bufferqueue/2.0/types.h>
#include <ui/GraphicBuffer.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <vndk/hardware_buffer.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

namespace /* unnamed */ {

using BQueueBufferInput = ::android::
        IGraphicBufferProducer::QueueBufferInput;
using HQueueBufferInput = ::android::hardware::graphics::bufferqueue::V2_0::
        IGraphicBufferProducer::QueueBufferInput;
using BQueueBufferOutput = ::android::
        IGraphicBufferProducer::QueueBufferOutput;
using HQueueBufferOutput = ::android::hardware::graphics::bufferqueue::V2_0::
        IGraphicBufferProducer::QueueBufferOutput;

using ::android::hardware::graphics::bufferqueue::V2_0::utils::b2h;
using ::android::hardware::graphics::bufferqueue::V2_0::utils::h2b;

bool b2h(BQueueBufferOutput const& from, HQueueBufferOutput* to) {
    to->width = from.width;
    to->height = from.height;
    to->transformHint = static_cast<int32_t>(from.transformHint);
    to->numPendingBuffers = from.numPendingBuffers;
    to->nextFrameNumber = from.nextFrameNumber;
    to->bufferReplaced = from.bufferReplaced;
    return true;
}

} // unnamed namespace

// B2HGraphicBufferProducer
// ========================

B2HGraphicBufferProducer::B2HGraphicBufferProducer(
        sp<BGraphicBufferProducer> const& base)
      : mBase{base} {
}

Return<HStatus> B2HGraphicBufferProducer::setMaxDequeuedBufferCount(
        int32_t maxDequeuedBuffers) {
    HStatus hStatus{};
    bool converted = b2h(
            mBase->setMaxDequeuedBufferCount(
                static_cast<int>(maxDequeuedBuffers)),
            &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<void> B2HGraphicBufferProducer::requestBuffer(
        int32_t slot,
        requestBuffer_cb _hidl_cb) {
    sp<GraphicBuffer> bBuffer;
    HStatus hStatus{};
    HardwareBuffer hBuffer{};
    uint32_t hGenerationNumber{};
    bool converted =
            b2h(mBase->requestBuffer(
                    static_cast<int>(slot), &bBuffer),
                &hStatus) &&
            b2h(bBuffer, &hBuffer, &hGenerationNumber);
    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR,
             hBuffer, hGenerationNumber);
    return {};
}

Return<HStatus> B2HGraphicBufferProducer::setAsyncMode(bool async) {
    HStatus hStatus{};
    bool converted = b2h(mBase->setAsyncMode(async), &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<void> B2HGraphicBufferProducer::dequeueBuffer(
        DequeueBufferInput const& input,
        dequeueBuffer_cb _hidl_cb) {
    int bSlot{};
    sp<BFence> bFence;
    HStatus hStatus{};
    DequeueBufferOutput hOutput{};
    HFenceWrapper hFenceWrapper;
    bool converted =
            b2h(mBase->dequeueBuffer(
                    &bSlot,
                    &bFence,
                    input.width,
                    input.height,
                    static_cast<PixelFormat>(input.format),
                    input.usage,
                    &hOutput.bufferAge,
                    nullptr /* outTimestamps */),
                &hStatus,
                &hOutput.bufferNeedsReallocation,
                &hOutput.releaseAllBuffers) &&
            b2h(bFence, &hFenceWrapper);
    hOutput.fence = hFenceWrapper.getHandle();
    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR,
             static_cast<int32_t>(bSlot),
             hOutput);
    return {};
}

Return<HStatus> B2HGraphicBufferProducer::detachBuffer(int32_t slot) {
    HStatus hStatus{};
    bool converted = b2h(
            mBase->detachBuffer(static_cast<int>(slot)), &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<void> B2HGraphicBufferProducer::detachNextBuffer(
        detachNextBuffer_cb _hidl_cb) {
    sp<GraphicBuffer> bBuffer;
    sp<BFence> bFence;
    HStatus hStatus{};
    HardwareBuffer hBuffer{};
    HFenceWrapper hFenceWrapper;
    bool converted =
            b2h(mBase->detachNextBuffer(&bBuffer, &bFence), &hStatus) &&
            b2h(bBuffer, &hBuffer) &&
            b2h(bFence, &hFenceWrapper);
    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR,
             hBuffer,
             hFenceWrapper.getHandle());
    return {};
}

Return<void> B2HGraphicBufferProducer::attachBuffer(
        HardwareBuffer const& hBuffer,
        uint32_t generationNumber,
        attachBuffer_cb _hidl_cb) {
    sp<GraphicBuffer> bBuffer;
    if (!h2b(hBuffer, &bBuffer) || !bBuffer) {
        _hidl_cb(HStatus::UNKNOWN_ERROR,
                 static_cast<int32_t>(SlotIndex::INVALID),
                 false);
        return {};
    }
    bBuffer->setGenerationNumber(generationNumber);

    int bSlot{};
    HStatus hStatus{};
    bool releaseAllBuffers{};
    bool converted = b2h(
            mBase->attachBuffer(&bSlot, bBuffer), &hStatus,
            nullptr /* bufferNeedsReallocation */,
            &releaseAllBuffers);
    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR,
             static_cast<int32_t>(bSlot),
             releaseAllBuffers);
    return {};
}

Return<void> B2HGraphicBufferProducer::queueBuffer(
        int32_t slot,
        QueueBufferInput const& hInput,
        queueBuffer_cb _hidl_cb) {
    BQueueBufferInput bInput{
            hInput.timestamp,
            hInput.isAutoTimestamp,
            static_cast<android_dataspace>(hInput.dataSpace),
            {}, /* crop */
            0 /* scalingMode = NATIVE_WINDOW_SCALING_MODE_FREEZE */,
            static_cast<uint32_t>(hInput.transform),
            {}, /* fence */
            static_cast<uint32_t>(hInput.stickyTransform),
            false /* getFrameTimestamps */};

    // Convert crop.
    if (!h2b(hInput.crop, &bInput.crop)) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, QueueBufferOutput{});
        return {};
    }

    // Convert surfaceDamage.
    if (!h2b(hInput.surfaceDamage, &bInput.surfaceDamage)) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, QueueBufferOutput{});
        return {};
    }

    // Convert fence.
    if (!h2b(hInput.fence, &bInput.fence)) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, QueueBufferOutput{});
        return {};
    }

    BQueueBufferOutput bOutput{};
    HStatus hStatus{};
    QueueBufferOutput hOutput{};
    bool converted =
            b2h(
                mBase->queueBuffer(static_cast<int>(slot), bInput, &bOutput),
                &hStatus) &&
            b2h(bOutput, &hOutput);

    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR, hOutput);
    return {};
}

Return<HStatus> B2HGraphicBufferProducer::cancelBuffer(
        int32_t slot,
        hidl_handle const& fence) {
    sp<BFence> bFence;
    if (!h2b(fence.getNativeHandle(), &bFence)) {
        return {HStatus::UNKNOWN_ERROR};
    }
    HStatus hStatus{};
    bool converted = b2h(
            mBase->cancelBuffer(static_cast<int>(slot), bFence),
            &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<void> B2HGraphicBufferProducer::query(int32_t what, query_cb _hidl_cb) {
    int value{};
    int result = mBase->query(static_cast<int>(what), &value);
    _hidl_cb(static_cast<int32_t>(result), static_cast<int32_t>(value));
    return {};
}

Return<void> B2HGraphicBufferProducer::connect(
        sp<HProducerListener> const& hListener,
        HConnectionType hConnectionType,
        bool producerControlledByApp,
        connect_cb _hidl_cb) {
    sp<BProducerListener> bListener = new H2BProducerListener(hListener);
    int bConnectionType{};
    if (!bListener || !h2b(hConnectionType, &bConnectionType)) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, QueueBufferOutput{});
        return {};
    }
    BQueueBufferOutput bOutput{};
    HStatus hStatus{};
    QueueBufferOutput hOutput{};
    bool converted =
            b2h(mBase->connect(bListener,
                               bConnectionType,
                               producerControlledByApp,
                               &bOutput),
                &hStatus) &&
            b2h(bOutput, &hOutput);
    _hidl_cb(converted ? hStatus : HStatus::UNKNOWN_ERROR, hOutput);
    return {};
}

Return<HStatus> B2HGraphicBufferProducer::disconnect(
        HConnectionType hConnectionType) {
    int bConnectionType;
    if (!h2b(hConnectionType, &bConnectionType)) {
        return {HStatus::UNKNOWN_ERROR};
    }
    HStatus hStatus{};
    bool converted = b2h(mBase->disconnect(bConnectionType), &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<HStatus> B2HGraphicBufferProducer::allocateBuffers(
        uint32_t width, uint32_t height,
        uint32_t format, uint64_t usage) {
    mBase->allocateBuffers(
            width, height, static_cast<PixelFormat>(format), usage);
    return {HStatus::OK};
}

Return<HStatus> B2HGraphicBufferProducer::allowAllocation(bool allow) {
    HStatus hStatus{};
    bool converted = b2h(mBase->allowAllocation(allow), &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<HStatus> B2HGraphicBufferProducer::setGenerationNumber(
        uint32_t generationNumber) {
    HStatus hStatus{};
    bool converted = b2h(
            mBase->setGenerationNumber(generationNumber),
            &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<HStatus> B2HGraphicBufferProducer::setDequeueTimeout(
        int64_t timeoutNs) {
    HStatus hStatus{};
    bool converted = b2h(
            mBase->setDequeueTimeout(static_cast<nsecs_t>(timeoutNs)),
            &hStatus);
    return {converted ? hStatus : HStatus::UNKNOWN_ERROR};
}

Return<uint64_t> B2HGraphicBufferProducer::getUniqueId() {
    uint64_t outId{};
    HStatus hStatus{};
    bool converted = b2h(mBase->getUniqueId(&outId), &hStatus);
    return {converted ? outId : 0};
}

Return<void> B2HGraphicBufferProducer::getConsumerName(
        getConsumerName_cb _hidl_cb) {
    _hidl_cb(hidl_string{mBase->getConsumerName().c_str()});
    return {};
}

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

