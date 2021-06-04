/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_WGRAPHICBUFFERPRODUCER_H_
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_WGRAPHICBUFFERPRODUCER_H_

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <binder/Binder.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/IProducerListener.h>
#include <gui/bufferqueue/1.0/Conversion.h>
#include <gui/bufferqueue/1.0/WProducerListener.h>
#include <system/window.h>

#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>

namespace android {

using ::android::hardware::media::V1_0::AnwBuffer;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

typedef ::android::hardware::graphics::bufferqueue::V1_0::
        IGraphicBufferProducer HGraphicBufferProducer;
typedef ::android::hardware::graphics::bufferqueue::V1_0::
        IProducerListener HProducerListener;

typedef ::android::IGraphicBufferProducer BGraphicBufferProducer;
typedef ::android::IProducerListener BProducerListener;
using ::android::BnGraphicBufferProducer;

#ifndef LOG
struct LOG_dummy {
    template <typename T>
    LOG_dummy& operator<< (const T&) { return *this; }
};

#define LOG(x)  LOG_dummy()
#endif

// Instantiate only if HGraphicBufferProducer is base of BASE.
template <typename BASE,
          typename = typename std::enable_if<std::is_base_of<HGraphicBufferProducer, BASE>::value>::type>
struct TWGraphicBufferProducer : public BASE {
    TWGraphicBufferProducer(sp<BGraphicBufferProducer> const& base) : mBase(base) {}
    Return<void> requestBuffer(int32_t slot, HGraphicBufferProducer::requestBuffer_cb _hidl_cb) override {
        sp<GraphicBuffer> buf;
        status_t status = mBase->requestBuffer(slot, &buf);
        AnwBuffer anwBuffer{};
        if (buf != nullptr) {
            ::android::conversion::wrapAs(&anwBuffer, *buf);
        }
        _hidl_cb(static_cast<int32_t>(status), anwBuffer);
        return Void();
    }

    Return<int32_t> setMaxDequeuedBufferCount(int32_t maxDequeuedBuffers) override {
        return static_cast<int32_t>(mBase->setMaxDequeuedBufferCount(
                static_cast<int>(maxDequeuedBuffers)));
    }

    Return<int32_t> setAsyncMode(bool async) override {
        return static_cast<int32_t>(mBase->setAsyncMode(async));
    }

    Return<void> dequeueBuffer(
            uint32_t width, uint32_t height,
            ::android::hardware::graphics::common::V1_0::PixelFormat format, uint32_t usage,
            bool getFrameTimestamps, HGraphicBufferProducer::dequeueBuffer_cb _hidl_cb) override {
        int slot{};
        sp<Fence> fence;
        ::android::FrameEventHistoryDelta outTimestamps;
        status_t status = mBase->dequeueBuffer(
            &slot, &fence, width, height,
            static_cast<::android::PixelFormat>(format), usage, nullptr,
            getFrameTimestamps ? &outTimestamps : nullptr);
        hidl_handle tFence{};
        HGraphicBufferProducer::FrameEventHistoryDelta tOutTimestamps{};

        native_handle_t* nh = nullptr;
        if ((fence == nullptr) || !::android::conversion::wrapAs(&tFence, &nh, *fence)) {
            LOG(ERROR) << "TWGraphicBufferProducer::dequeueBuffer - "
                    "Invalid output fence";
            _hidl_cb(static_cast<int32_t>(status),
                     static_cast<int32_t>(slot),
                     tFence,
                     tOutTimestamps);
            return Void();
        }
        std::vector<std::vector<native_handle_t*> > nhAA;
        if (getFrameTimestamps && !::android::conversion::wrapAs(&tOutTimestamps, &nhAA, outTimestamps)) {
            LOG(ERROR) << "TWGraphicBufferProducer::dequeueBuffer - "
                    "Invalid output timestamps";
            _hidl_cb(static_cast<int32_t>(status),
                     static_cast<int32_t>(slot),
                     tFence,
                     tOutTimestamps);
            native_handle_delete(nh);
            return Void();
        }

        _hidl_cb(static_cast<int32_t>(status),
                static_cast<int32_t>(slot),
                tFence,
                tOutTimestamps);
        native_handle_delete(nh);
        if (getFrameTimestamps) {
            for (auto& nhA : nhAA) {
                for (auto& handle : nhA) {
                    native_handle_delete(handle);
                }
            }
        }
        return Void();
    }

    Return<int32_t> detachBuffer(int32_t slot) override {
        return static_cast<int32_t>(mBase->detachBuffer(slot));
    }

    Return<void> detachNextBuffer(HGraphicBufferProducer::detachNextBuffer_cb _hidl_cb) override {
        sp<GraphicBuffer> outBuffer;
        sp<Fence> outFence;
        status_t status = mBase->detachNextBuffer(&outBuffer, &outFence);
        AnwBuffer tBuffer{};
        hidl_handle tFence{};

        if (outBuffer == nullptr) {
            LOG(ERROR) << "TWGraphicBufferProducer::detachNextBuffer - "
                    "Invalid output buffer";
            _hidl_cb(static_cast<int32_t>(status), tBuffer, tFence);
            return Void();
        }
        ::android::conversion::wrapAs(&tBuffer, *outBuffer);
        native_handle_t* nh = nullptr;
        if ((outFence != nullptr) && !::android::conversion::wrapAs(&tFence, &nh, *outFence)) {
            LOG(ERROR) << "TWGraphicBufferProducer::detachNextBuffer - "
                    "Invalid output fence";
            _hidl_cb(static_cast<int32_t>(status), tBuffer, tFence);
            return Void();
        }

        _hidl_cb(static_cast<int32_t>(status), tBuffer, tFence);
        native_handle_delete(nh);
        return Void();
    }

    Return<void> attachBuffer(const AnwBuffer& buffer, HGraphicBufferProducer::attachBuffer_cb _hidl_cb) override {
        int outSlot;
        sp<GraphicBuffer> lBuffer = new GraphicBuffer();
        if (!::android::conversion::convertTo(lBuffer.get(), buffer)) {
            LOG(ERROR) << "TWGraphicBufferProducer::attachBuffer - "
                    "Invalid input native window buffer";
            _hidl_cb(static_cast<int32_t>(BAD_VALUE), -1);
            return Void();
        }
        status_t status = mBase->attachBuffer(&outSlot, lBuffer);

        _hidl_cb(static_cast<int32_t>(status), static_cast<int32_t>(outSlot));
        return Void();
    }

    Return<void> queueBuffer(
            int32_t slot, const HGraphicBufferProducer::QueueBufferInput& input,
            HGraphicBufferProducer::queueBuffer_cb _hidl_cb) override {
        HGraphicBufferProducer::QueueBufferOutput tOutput{};
        BGraphicBufferProducer::QueueBufferInput lInput(
                0, false, HAL_DATASPACE_UNKNOWN,
                ::android::Rect(0, 0, 1, 1),
                NATIVE_WINDOW_SCALING_MODE_FREEZE,
                0, ::android::Fence::NO_FENCE);
        if (!::android::conversion::convertTo(&lInput, input)) {
            LOG(ERROR) << "TWGraphicBufferProducer::queueBuffer - "
                    "Invalid input";
            _hidl_cb(static_cast<int32_t>(BAD_VALUE), tOutput);
            return Void();
        }
        BGraphicBufferProducer::QueueBufferOutput lOutput;
        status_t status = mBase->queueBuffer(
                static_cast<int>(slot), lInput, &lOutput);

        std::vector<std::vector<native_handle_t*> > nhAA;
        if (!::android::conversion::wrapAs(&tOutput, &nhAA, lOutput)) {
            LOG(ERROR) << "TWGraphicBufferProducer::queueBuffer - "
                    "Invalid output";
            _hidl_cb(static_cast<int32_t>(BAD_VALUE), tOutput);
            return Void();
        }

        _hidl_cb(static_cast<int32_t>(status), tOutput);
        for (auto& nhA : nhAA) {
            for (auto& nh : nhA) {
                native_handle_delete(nh);
            }
        }
        return Void();
    }

    Return<int32_t> cancelBuffer(int32_t slot, const hidl_handle& fence) override {
        sp<Fence> lFence = new Fence();
        if (!::android::conversion::convertTo(lFence.get(), fence)) {
            LOG(ERROR) << "TWGraphicBufferProducer::cancelBuffer - "
                    "Invalid input fence";
            return static_cast<int32_t>(BAD_VALUE);
        }
        return static_cast<int32_t>(mBase->cancelBuffer(static_cast<int>(slot), lFence));
    }

    Return<void> query(int32_t what, HGraphicBufferProducer::query_cb _hidl_cb) override {
        int lValue;
        int lReturn = mBase->query(static_cast<int>(what), &lValue);
        _hidl_cb(static_cast<int32_t>(lReturn), static_cast<int32_t>(lValue));
        return Void();
    }

    Return<void> connect(const sp<HProducerListener>& listener,
            int32_t api, bool producerControlledByApp,
            HGraphicBufferProducer::connect_cb _hidl_cb) override {
        sp<BProducerListener> lListener = listener == nullptr ?
                nullptr : new LWProducerListener(listener);
        BGraphicBufferProducer::QueueBufferOutput lOutput;
        status_t status = mBase->connect(lListener,
                static_cast<int>(api),
                producerControlledByApp,
                &lOutput);

        HGraphicBufferProducer::QueueBufferOutput tOutput{};
        std::vector<std::vector<native_handle_t*> > nhAA;
        if (!::android::conversion::wrapAs(&tOutput, &nhAA, lOutput)) {
            LOG(ERROR) << "TWGraphicBufferProducer::connect - "
                    "Invalid output";
            _hidl_cb(static_cast<int32_t>(status), tOutput);
            return Void();
        }

        _hidl_cb(static_cast<int32_t>(status), tOutput);
        for (auto& nhA : nhAA) {
            for (auto& nh : nhA) {
                native_handle_delete(nh);
            }
        }
        return Void();
    }

    Return<int32_t> disconnect(
            int32_t api,
            HGraphicBufferProducer::DisconnectMode mode) override {
        return static_cast<int32_t>(mBase->disconnect(
                static_cast<int>(api),
                ::android::conversion::toGuiDisconnectMode(mode)));
    }

    Return<int32_t> setSidebandStream(const hidl_handle& stream) override {
        return static_cast<int32_t>(mBase->setSidebandStream(NativeHandle::create(
                stream ? native_handle_clone(stream) : NULL, true)));
    }

    Return<void> allocateBuffers(
            uint32_t width, uint32_t height,
            ::android::hardware::graphics::common::V1_0::PixelFormat format,
            uint32_t usage) override {
        mBase->allocateBuffers(
                width, height,
                static_cast<::android::PixelFormat>(format),
                usage);
        return Void();
    }

    Return<int32_t> allowAllocation(bool allow) override {
        return static_cast<int32_t>(mBase->allowAllocation(allow));
    }

    Return<int32_t> setGenerationNumber(uint32_t generationNumber) override {
        return static_cast<int32_t>(mBase->setGenerationNumber(generationNumber));
    }

    Return<void> getConsumerName(HGraphicBufferProducer::getConsumerName_cb _hidl_cb) override {
        _hidl_cb(mBase->getConsumerName().string());
        return Void();
    }

    Return<int32_t> setSharedBufferMode(bool sharedBufferMode) override {
        return static_cast<int32_t>(mBase->setSharedBufferMode(sharedBufferMode));
    }

    Return<int32_t> setAutoRefresh(bool autoRefresh) override {
        return static_cast<int32_t>(mBase->setAutoRefresh(autoRefresh));
    }

    Return<int32_t> setDequeueTimeout(int64_t timeoutNs) override {
        return static_cast<int32_t>(mBase->setDequeueTimeout(timeoutNs));
    }

    Return<void> getLastQueuedBuffer(HGraphicBufferProducer::getLastQueuedBuffer_cb _hidl_cb) override {
        sp<GraphicBuffer> lOutBuffer = new GraphicBuffer();
        sp<Fence> lOutFence = new Fence();
        float lOutTransformMatrix[16];
        status_t status = mBase->getLastQueuedBuffer(
                &lOutBuffer, &lOutFence, lOutTransformMatrix);

        AnwBuffer tOutBuffer{};
        if (lOutBuffer != nullptr) {
            ::android::conversion::wrapAs(&tOutBuffer, *lOutBuffer);
        }
        hidl_handle tOutFence{};
        native_handle_t* nh = nullptr;
        if ((lOutFence == nullptr) || !::android::conversion::wrapAs(&tOutFence, &nh, *lOutFence)) {
            LOG(ERROR) << "TWGraphicBufferProducer::getLastQueuedBuffer - "
                    "Invalid output fence";
            _hidl_cb(static_cast<int32_t>(status),
                    tOutBuffer,
                    tOutFence,
                    hidl_array<float, 16>());
            return Void();
        }
        hidl_array<float, 16> tOutTransformMatrix(lOutTransformMatrix);

        _hidl_cb(static_cast<int32_t>(status), tOutBuffer, tOutFence, tOutTransformMatrix);
        native_handle_delete(nh);
        return Void();
    }

    Return<void> getFrameTimestamps(HGraphicBufferProducer::getFrameTimestamps_cb _hidl_cb) override {
        ::android::FrameEventHistoryDelta lDelta;
        mBase->getFrameTimestamps(&lDelta);

        HGraphicBufferProducer::FrameEventHistoryDelta tDelta{};
        std::vector<std::vector<native_handle_t*> > nhAA;
        if (!::android::conversion::wrapAs(&tDelta, &nhAA, lDelta)) {
            LOG(ERROR) << "TWGraphicBufferProducer::getFrameTimestamps - "
                    "Invalid output frame timestamps";
            _hidl_cb(tDelta);
            return Void();
        }

        _hidl_cb(tDelta);
        for (auto& nhA : nhAA) {
            for (auto& nh : nhA) {
                native_handle_delete(nh);
            }
        }
        return Void();
    }

    Return<void> getUniqueId(HGraphicBufferProducer::getUniqueId_cb _hidl_cb) override {
        uint64_t outId{};
        status_t status = mBase->getUniqueId(&outId);
        _hidl_cb(static_cast<int32_t>(status), outId);
        return Void();
    }

private:
    sp<BGraphicBufferProducer> mBase;
};

}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_WGRAPHICBUFFERPRODUCER_H_
