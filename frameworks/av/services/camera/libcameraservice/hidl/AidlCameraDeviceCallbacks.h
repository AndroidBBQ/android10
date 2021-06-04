/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_FRAMEWORKS_AIDL_CAMERADEVICECALLBACKS_H
#define ANDROID_FRAMEWORKS_AIDL_CAMERADEVICECALLBACKS_H

#include <mutex>
#include <thread>

#include <android/frameworks/cameraservice/common/2.0/types.h>
#include <android/frameworks/cameraservice/service/2.0/types.h>
#include <android/frameworks/cameraservice/device/2.0/ICameraDeviceCallback.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <android/hardware/camera2/BnCameraDeviceCallbacks.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>
#include <fmq/MessageQueue.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <CameraService.h>
#include <hidl/CameraHybridInterface.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace device {
namespace V2_0 {
namespace implementation {

using camerahybrid::H2BConverter;
using HCameraDeviceCallback = cameraservice::device::V2_0::ICameraDeviceCallback;
using HPhysicalCaptureResultInfo = cameraservice::device::V2_0::PhysicalCaptureResultInfo;
using android::frameworks::cameraservice::device::V2_0::FmqSizeOrMetadata;

using hardware::camera2::BnCameraDeviceCallbacks;
using hardware::camera2::ICameraDeviceCallbacks;
using hardware::camera2::impl::CaptureResultExtras;
using hardware::camera2::impl::CameraMetadataNative;
using hardware::camera2::impl::PhysicalCaptureResultInfo;
using hardware::kSynchronizedReadWrite;
using hardware::MessageQueue;
using CaptureResultMetadataQueue = MessageQueue<uint8_t, kSynchronizedReadWrite>;

struct H2BCameraDeviceCallbacks :
    public H2BConverter<HCameraDeviceCallback, ICameraDeviceCallbacks, BnCameraDeviceCallbacks> {
    H2BCameraDeviceCallbacks(const sp<HalInterface>& base);

    ~H2BCameraDeviceCallbacks();

    bool initializeLooper();

    virtual binder::Status onDeviceError(int32_t errorCode,
                                         const CaptureResultExtras& resultExtras) override;

    virtual binder::Status onDeviceIdle() override;

    virtual binder::Status onCaptureStarted(const CaptureResultExtras& resultExtras,
                                            int64_t timestamp) override;

    virtual binder::Status onResultReceived(
        const CameraMetadataNative& result, const CaptureResultExtras& resultExtras,
        const std::vector<PhysicalCaptureResultInfo>& physicalCaptureResultInfos) override;

    virtual binder::Status onPrepared(int32_t streamId) override;

    virtual binder::Status onRepeatingRequestError(int64_t lastFrameNumber,
                                                   int32_t repeatingRequestId) override;

    virtual binder::Status onRequestQueueEmpty() override;

    void setCaptureResultMetadataQueue(std::shared_ptr<CaptureResultMetadataQueue> metadataQueue) {
        mCaptureResultMetadataQueue = metadataQueue;
    }

 private:
    // Wrapper struct so that parameters to onResultReceived callback may be
    // sent through an AMessage.
    struct ResultWrapper : public RefBase {
        CameraMetadataNative mResult;
        CaptureResultExtras mResultExtras;
        std::vector<PhysicalCaptureResultInfo> mPhysicalCaptureResultInfos;
        ResultWrapper(CameraMetadataNative &result,
                      const CaptureResultExtras resultExtras,
                      const std::vector<PhysicalCaptureResultInfo> &physicalCaptureResultInfos) :
      // TODO: make this std::movable
      mResult(result), mResultExtras(resultExtras), mPhysicalCaptureResultInfos(physicalCaptureResultInfos) { }
    };

    struct CallbackHandler : public AHandler {
        public:
            void onMessageReceived(const sp<AMessage> &msg) override;
            CallbackHandler(H2BCameraDeviceCallbacks *converter) : mConverter(converter) { }
        private:
            void processResultMessage(sp<ResultWrapper> &resultWrapper);
            wp<H2BCameraDeviceCallbacks> mConverter = nullptr;
            Mutex mMetadataQueueLock;
    };

    void convertResultMetadataToHidl(const camera_metadata *rawMetadata,
                                     FmqSizeOrMetadata *resultMetadata);
    enum {
        kWhatResultReceived,
    };

    static const char *kResultKey;

    std::shared_ptr<CaptureResultMetadataQueue> mCaptureResultMetadataQueue = nullptr;
    sp<CallbackHandler> mHandler = nullptr;
    sp<ALooper> mCbLooper = nullptr;
};

} // implementation
} // V2_0
} // device
} // cameraservice
} // frameworks
} // android
#endif // ANDROID_FRAMEWORKS_AIDL_CAMERADEVICECALLBACKS_H
