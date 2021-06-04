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
#define LOG_TAG "H2BGraphicBufferProducer@2.0"

#include <android-base/logging.h>

#include <android/hardware/graphics/common/1.2/types.h>
#include <gui/bufferqueue/2.0/B2HProducerListener.h>
#include <gui/bufferqueue/2.0/H2BGraphicBufferProducer.h>
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

bool b2h(BQueueBufferInput const& from, HQueueBufferInput* to,
         HFenceWrapper* hFenceWrapper) {
    to->timestamp = from.timestamp;
    to->isAutoTimestamp = static_cast<bool>(from.isAutoTimestamp);
    to->dataSpace = static_cast<int32_t>(from.dataSpace);
    to->transform = static_cast<int32_t>(from.transform);
    to->stickyTransform = static_cast<int32_t>(from.stickyTransform);
    if (!b2h(from.crop, &to->crop) ||
            !b2h(from.surfaceDamage, &to->surfaceDamage) ||
            !b2h(from.fence, hFenceWrapper)) {
        return false;
    }
    to->fence = hFenceWrapper->getHandle();
    return true;
}

bool h2b(HQueueBufferOutput const& from, BQueueBufferOutput* to) {
    to->width = from.width;
    to->height = from.height;
    to->transformHint = static_cast<uint32_t>(from.transformHint);
    to->numPendingBuffers = from.numPendingBuffers;
    to->nextFrameNumber = from.nextFrameNumber;
    to->bufferReplaced = from.bufferReplaced;
    return true;
}

} // unnamed namespace

// H2BGraphicBufferProducer
// ========================

status_t H2BGraphicBufferProducer::requestBuffer(int slot,
                                                 sp<GraphicBuffer>* bBuffer) {
    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->requestBuffer(slot,
            [&converted, &bStatus, bBuffer](
                    HStatus hStatus,
                    HardwareBuffer const& hBuffer,
                    uint32_t generationNumber) {
                converted =
                        h2b(hStatus, &bStatus) &&
                        h2b(hBuffer, bBuffer);
                if (*bBuffer) {
                    (*bBuffer)->setGenerationNumber(generationNumber);
                }
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "requestBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "requestBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::setMaxDequeuedBufferCount(
        int maxDequeuedBuffers) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->setMaxDequeuedBufferCount(
            static_cast<int32_t>(maxDequeuedBuffers));
    if (!transResult.isOk()) {
        LOG(ERROR) << "setMaxDequeuedBufferCount: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "setMaxDequeuedBufferCount: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::setAsyncMode(bool async) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->setAsyncMode(async);
    if (!transResult.isOk()) {
        LOG(ERROR) << "setAsyncMode: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "setAsyncMode: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::dequeueBuffer(
        int* slot, sp<BFence>* fence,
        uint32_t w, uint32_t h,
        PixelFormat format, uint64_t usage,
        uint64_t* outBufferAge, FrameEventHistoryDelta* /* outTimestamps */) {

    using HInput = HGraphicBufferProducer::DequeueBufferInput;
    HInput input{w, h, static_cast<uint32_t>(format), usage};

    using HOutput = HGraphicBufferProducer::DequeueBufferOutput;
    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->dequeueBuffer(input,
            [&converted, &bStatus, slot, fence, outBufferAge] (
                    HStatus hStatus, int32_t hSlot, HOutput const& hOutput) {
                converted = h2b(hStatus, &bStatus);
                if (!converted || bStatus != OK) {
                    return;
                }
                *slot = hSlot;
                *outBufferAge = hOutput.bufferAge;
                bStatus =
                        (hOutput.bufferNeedsReallocation ?
                        BUFFER_NEEDS_REALLOCATION : 0) |
                        (hOutput.releaseAllBuffers ?
                        RELEASE_ALL_BUFFERS : 0);
                converted = h2b(hOutput.fence, fence);
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "dequeueBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "dequeueBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::detachBuffer(int slot) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->detachBuffer(
            static_cast<int32_t>(slot));
    if (!transResult.isOk()) {
        LOG(ERROR) << "detachBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "detachBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::detachNextBuffer(
        sp<GraphicBuffer>* outBuffer, sp<BFence>* outFence) {
    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->detachNextBuffer(
            [&converted, &bStatus, outBuffer, outFence] (
                    HStatus hStatus,
                    HardwareBuffer const& hBuffer,
                    hidl_handle const& hFence) {
                converted = h2b(hStatus, &bStatus) &&
                    h2b(hBuffer, outBuffer) &&
                    h2b(hFence, outFence);
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "detachNextBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "detachNextBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::attachBuffer(
        int* outSlot, sp<GraphicBuffer> const& buffer) {
    HardwareBuffer hBuffer{};
    uint32_t hGenerationNumber{};
    if (!b2h(buffer, &hBuffer, &hGenerationNumber)) {
        LOG(ERROR) << "attachBuffer: invalid input buffer.";
        return BAD_VALUE;
    }

    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->attachBuffer(hBuffer, hGenerationNumber,
            [&converted, &bStatus, outSlot](
                    HStatus hStatus, int32_t hSlot, bool releaseAllBuffers) {
                converted = h2b(hStatus, &bStatus);
                *outSlot = static_cast<int>(hSlot);
                if (converted && releaseAllBuffers && bStatus == OK) {
                    bStatus = IGraphicBufferProducer::RELEASE_ALL_BUFFERS;
                }
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "attachBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "attachBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::queueBuffer(
        int slot,
        QueueBufferInput const& input,
        QueueBufferOutput* output) {
    HQueueBufferInput hInput{};
    HFenceWrapper hFenceWrapper;
    if (!b2h(input, &hInput, &hFenceWrapper)) {
        LOG(ERROR) << "queueBuffer: corrupted input.";
        return UNKNOWN_ERROR;
    }

    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->queueBuffer(
            static_cast<int32_t>(slot),
            hInput,
            [&converted, &bStatus, output](
                    HStatus hStatus,
                    HQueueBufferOutput const& hOutput) {
                converted = h2b(hStatus, &bStatus) && h2b(hOutput, output);
            });

    if (!transResult.isOk()) {
        LOG(ERROR) << "queueBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "queueBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::cancelBuffer(int slot, sp<BFence> const& fence) {
    HFenceWrapper hFenceWrapper;
    if (!b2h(fence, &hFenceWrapper)) {
        LOG(ERROR) << "cancelBuffer: corrupted input fence.";
        return UNKNOWN_ERROR;
    }
    status_t bStatus{};
    Return<HStatus> transResult = mBase->cancelBuffer(
            static_cast<int32_t>(slot),
            hFenceWrapper.getHandle());
    if (!transResult.isOk()) {
        LOG(ERROR) << "cancelBuffer: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "cancelBuffer: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

int H2BGraphicBufferProducer::query(int what, int* value) {
    int result{};
    Return<void> transResult = mBase->query(
            static_cast<int32_t>(what),
            [&result, value](int32_t r, int32_t v) {
                result = static_cast<int>(r);
                *value = static_cast<int>(v);
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "query: transaction failed.";
        return FAILED_TRANSACTION;
    }
    return result;
}

status_t H2BGraphicBufferProducer::connect(
        sp<IProducerListener> const& listener, int api,
        bool producerControlledByApp, QueueBufferOutput* output) {
    HConnectionType hConnectionType;
    if (!b2h(api, &hConnectionType)) {
        LOG(ERROR) << "connect: corrupted input connection type.";
        return UNKNOWN_ERROR;
    }
    sp<HProducerListener> hListener = nullptr;
    if (listener && listener->needsReleaseNotify()) {
        hListener = new B2HProducerListener(listener);
        if (!hListener) {
            LOG(ERROR) << "connect: failed to wrap listener.";
            return UNKNOWN_ERROR;
        }
    }

    bool converted{};
    status_t bStatus{};
    Return<void> transResult = mBase->connect(
            hListener,
            hConnectionType,
            producerControlledByApp,
            [&converted, &bStatus, output](
                    HStatus hStatus,
                    HQueueBufferOutput const& hOutput) {
                converted = h2b(hStatus, &bStatus) && h2b(hOutput, output);
            });
    if (!transResult.isOk()) {
        LOG(ERROR) << "connect: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!converted) {
        LOG(ERROR) << "connect: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;

}

status_t H2BGraphicBufferProducer::disconnect(int api, DisconnectMode mode) {
    HConnectionType hConnectionType;
    if (mode == DisconnectMode::AllLocal) {
        hConnectionType = HConnectionType::CURRENTLY_CONNECTED;
    } else if (!b2h(api, &hConnectionType)) {
        LOG(ERROR) << "connect: corrupted input connection type.";
        return UNKNOWN_ERROR;
    }

    status_t bStatus{};
    Return<HStatus> transResult = mBase->disconnect(hConnectionType);
    if (!transResult.isOk()) {
        LOG(ERROR) << "disconnect: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "disconnect: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::setSidebandStream(
        sp<NativeHandle> const& stream) {
    if (stream) {
        LOG(INFO) << "setSidebandStream: not supported.";
        return INVALID_OPERATION;
    }
    return OK;
}

void H2BGraphicBufferProducer::allocateBuffers(
        uint32_t width, uint32_t height,
        PixelFormat format, uint64_t usage) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->allocateBuffers(
            width, height, static_cast<uint32_t>(format), usage);
    if (!transResult.isOk()) {
        LOG(ERROR) << "allocateBuffer: transaction failed.";
        return;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "allocateBuffer: corrupted transaction.";
        return;
    }
}

status_t H2BGraphicBufferProducer::allowAllocation(bool allow) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->allowAllocation(allow);
    if (!transResult.isOk()) {
        LOG(ERROR) << "allowAllocation: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "allowAllocation: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::setGenerationNumber(
        uint32_t generationNumber) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->setGenerationNumber(generationNumber);
    if (!transResult.isOk()) {
        LOG(ERROR) << "setGenerationNumber: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "setGenerationNumber: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

String8 H2BGraphicBufferProducer::getConsumerName() const {
    String8 bName;
    Return<void> transResult = mBase->getConsumerName(
            [&bName](hidl_string const& name) {
                bName = name.c_str();
            });
    return bName;
}

status_t H2BGraphicBufferProducer::setSharedBufferMode(bool sharedBufferMode) {
    if (sharedBufferMode) {
        LOG(INFO) << "setSharedBufferMode: not supported.";
        return INVALID_OPERATION;
    }
    return OK;
}

status_t H2BGraphicBufferProducer::setAutoRefresh(bool autoRefresh) {
    if (autoRefresh) {
        LOG(INFO) << "setAutoRefresh: not supported.";
        return INVALID_OPERATION;
    }
    return OK;
}

status_t H2BGraphicBufferProducer::setDequeueTimeout(nsecs_t timeout) {
    status_t bStatus{};
    Return<HStatus> transResult = mBase->setDequeueTimeout(
            static_cast<int64_t>(timeout));
    if (!transResult.isOk()) {
        LOG(ERROR) << "setDequeueTimeout: transaction failed.";
        return FAILED_TRANSACTION;
    }
    if (!h2b(static_cast<HStatus>(transResult), &bStatus)) {
        LOG(ERROR) << "setDequeueTimeout: corrupted transaction.";
        return FAILED_TRANSACTION;
    }
    return bStatus;
}

status_t H2BGraphicBufferProducer::getLastQueuedBuffer(
        sp<GraphicBuffer>*,
        sp<BFence>*,
        float[16]) {
    LOG(INFO) << "getLastQueuedBuffer: not supported.";
    return INVALID_OPERATION;
}

void H2BGraphicBufferProducer::getFrameTimestamps(FrameEventHistoryDelta*) {
    LOG(INFO) << "getFrameTimestamps: not supported.";
}

status_t H2BGraphicBufferProducer::getUniqueId(uint64_t* outId) const {
    Return<uint64_t> transResult = mBase->getUniqueId();
    if (!transResult.isOk()) {
        LOG(ERROR) << "getUniqueId: transaction failed.";
        return FAILED_TRANSACTION;
    }
    *outId = static_cast<uint64_t>(transResult);
    return OK;
}

status_t H2BGraphicBufferProducer::getConsumerUsage(uint64_t*) const {
    LOG(INFO) << "getConsumerUsage: not supported.";
    return INVALID_OPERATION;
}

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android
