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

#ifndef ANDROID_FRAMEWORKS_CAMERADEVICEUSER_V2_0_CAMERADEVICEUSER_H
#define ANDROID_FRAMEWORKS_CAMERADEVICEUSER_V2_0_CAMERADEVICEUSER_H

#include <mutex>
#include <memory>
#include <thread>

#include <android/frameworks/cameraservice/common/2.0/types.h>
#include <android/frameworks/cameraservice/service/2.0/types.h>
#include <android/frameworks/cameraservice/device/2.0/ICameraDeviceUser.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <android/hardware/camera2/ICameraDeviceCallbacks.h>
#include <fmq/MessageQueue.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <CameraService.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace device {
namespace V2_0 {
namespace implementation {

using frameworks::cameraservice::device::V2_0::StreamConfigurationMode;
using hardware::camera2::CaptureRequest;
using hardware::hidl_vec;
using hardware::kSynchronizedReadWrite;
using hardware::MessageQueue;
using hardware::MQDescriptorSync;
using hardware::Return;
using CaptureResultMetadataQueue = MessageQueue<uint8_t, kSynchronizedReadWrite>;
using CaptureRequestMetadataQueue = MessageQueue<uint8_t, kSynchronizedReadWrite>;
using TemplateId = frameworks::cameraservice::device::V2_0::TemplateId;

using HCameraDeviceUser = device::V2_0::ICameraDeviceUser;
using HCameraMetadata = cameraservice::service::V2_0::CameraMetadata;
using HCaptureRequest = device::V2_0::CaptureRequest;
using HSessionConfiguration = frameworks::cameraservice::device::V2_0::SessionConfiguration;
using HOutputConfiguration = frameworks::cameraservice::device::V2_0::OutputConfiguration;
using HPhysicalCameraSettings = frameworks::cameraservice::device::V2_0::PhysicalCameraSettings;
using HStatus = frameworks::cameraservice::common::V2_0::Status;

static constexpr int32_t REQUEST_ID_NONE = -1;

struct HidlCameraDeviceUser final : public HCameraDeviceUser {
    HidlCameraDeviceUser(const sp<hardware::camera2::ICameraDeviceUser> &deviceRemote);

    ~HidlCameraDeviceUser() { }

    virtual Return<void> disconnect() override;

    virtual Return<void> getCaptureRequestMetadataQueue(
        getCaptureRequestMetadataQueue_cb _hidl_cb) override;

    virtual Return<void> getCaptureResultMetadataQueue(
        getCaptureResultMetadataQueue_cb _hidl_cb) override;

    virtual Return<void> submitRequestList(const hidl_vec<HCaptureRequest>& requestList,
                                           bool streaming, submitRequestList_cb _hidl_cb) override;

    virtual Return<void> cancelRepeatingRequest(cancelRepeatingRequest_cb _hidl_cb) override;

    virtual Return<HStatus> beginConfigure() override;

    virtual Return<HStatus> endConfigure(StreamConfigurationMode operatingMode,
                                         const hidl_vec<uint8_t>& sessionParams);

    virtual Return<HStatus> deleteStream(int32_t streamId) override;

    virtual Return<void> createStream(const HOutputConfiguration& outputConfiguration,
                                      createStream_cb _hidl_cb) override;

    Return<void> createDefaultRequest(TemplateId templateId,
                                      createDefaultRequest_cb _hidl_cb) override;

    virtual Return<HStatus> waitUntilIdle() override;

    virtual Return<void> flush(flush_cb _hidl_cb) override;

    virtual Return<HStatus> updateOutputConfiguration(
        int32_t streamId, const HOutputConfiguration& outputConfiguration) override;

    virtual Return<void> isSessionConfigurationSupported(
        const HSessionConfiguration& sessionConfiguration,
        isSessionConfigurationSupported_cb _hidl_cb) override;

    bool initStatus() { return mInitSuccess; }

    std::shared_ptr<CaptureResultMetadataQueue> getCaptureResultMetadataQueue() {
        return mCaptureResultMetadataQueue;
    }

 private:
    bool initDevice();

    bool convertRequestFromHidl(const HCaptureRequest &hRequest, CaptureRequest *request);

    bool copyPhysicalCameraSettings(
        const hidl_vec<HPhysicalCameraSettings> &hPhysicalCameraSettings,
        std::vector<CaptureRequest::PhysicalCameraSettings> *physicalCameraSettings);

    const sp<hardware::camera2::ICameraDeviceUser> mDeviceRemote;
    std::unique_ptr<CaptureRequestMetadataQueue> mCaptureRequestMetadataQueue = nullptr;
    std::shared_ptr<CaptureResultMetadataQueue> mCaptureResultMetadataQueue = nullptr;
    bool mInitSuccess = false;
    int32_t mRequestId = REQUEST_ID_NONE;
};

} // implementation
} // V2_0
} // device
} // cameraservice
} // frameworks
} // android
#endif // ANDROID_FRAMEOWORKS_CAMERADEVICEUSER_V2_0_CAMERADEVICEUSER_H
