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

#ifndef CAMERASERVER_CONVERT_HIDL
#define CAMERASERVER_CONVERT_HIDL

#include <vector>

#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <android/frameworks/cameraservice/device/2.0/ICameraDeviceUser.h>
#include <android/frameworks/cameraservice/common/2.0/types.h>
#include <android/frameworks/cameraservice/service/2.0/types.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <android/hardware/camera/common/1.0/types.h>
#include <android/hardware/camera2/ICameraDeviceUser.h>
#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>
#include <android/hardware/ICameraService.h>
#include <fmq/MessageQueue.h>
#include <hardware/camera.h>
#include <hidl/MQDescriptor.h>

namespace android {
namespace hardware {
namespace cameraservice {
namespace utils {
namespace conversion {

using hardware::camera2::impl::CaptureResultExtras;
using hardware::camera2::impl::PhysicalCaptureResultInfo;

using CaptureResultMetadataQueue = MessageQueue<uint8_t, kSynchronizedReadWrite>;
using HCameraMetadata = frameworks::cameraservice::service::V2_0::CameraMetadata;
using HCameraDeviceStatus = frameworks::cameraservice::service::V2_0::CameraDeviceStatus;
using HCameraStatusAndId = frameworks::cameraservice::service::V2_0::CameraStatusAndId;
using HCameraDeviceUser = frameworks::cameraservice::device::V2_0::ICameraDeviceUser;
using HCaptureResultExtras = frameworks::cameraservice::device::V2_0::CaptureResultExtras;
using HCaptureRequest = frameworks::cameraservice::device::V2_0::CaptureRequest;
using HErrorCode = frameworks::cameraservice::device::V2_0::ErrorCode;
using HGraphicBufferProducer = hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer;
using HOutputConfiguration = frameworks::cameraservice::device::V2_0::OutputConfiguration;
using HPhysicalCameraSettings = frameworks::cameraservice::device::V2_0::PhysicalCameraSettings;
using HPhysicalCaptureResultInfo = frameworks::cameraservice::device::V2_0::PhysicalCaptureResultInfo;
using HSessionConfiguration = frameworks::cameraservice::device::V2_0::SessionConfiguration;
using HSubmitInfo = frameworks::cameraservice::device::V2_0::SubmitInfo;
using HStatus = frameworks::cameraservice::common::V2_0::Status;
using HStreamConfigurationMode = frameworks::cameraservice::device::V2_0::StreamConfigurationMode;
using HTemplateId = frameworks::cameraservice::device::V2_0::TemplateId;

// Note: existing data in dst will be gone. Caller still owns the memory of src
void convertToHidl(const camera_metadata_t *src, HCameraMetadata* dst);

int32_t convertFromHidl(HStreamConfigurationMode streamConfigurationMode);

int32_t convertFromHidl(HTemplateId templateId);

bool convertFromHidl(const HCameraMetadata &src, CameraMetadata *dst);

hardware::camera2::params::OutputConfiguration convertFromHidl(
    const HOutputConfiguration &hOutputConfiguration);

hardware::camera2::params::SessionConfiguration convertFromHidl(
    const HSessionConfiguration &hSessionConfiguration);

HCameraDeviceStatus convertToHidlCameraDeviceStatus(int32_t status);

void convertToHidl(const std::vector<hardware::CameraStatus> &src,
                   hidl_vec<HCameraStatusAndId>* dst);

void convertToHidl(const hardware::camera2::utils::SubmitInfo &submitInfo,
                   HSubmitInfo *hSubmitInfo);

HErrorCode convertToHidl(int32_t errorCode);

HCaptureResultExtras convertToHidl(const CaptureResultExtras &captureResultExtras);

hidl_vec<HPhysicalCaptureResultInfo> convertToHidl(
    const std::vector<PhysicalCaptureResultInfo> &physicalCaptureResultInfos,
    std::shared_ptr<CaptureResultMetadataQueue> &captureResultMetadataQueue);

HStatus B2HStatus(const binder::Status &bStatus);

} // conversion
} // utils
} // cameraservice
} // hardware
} //android

#endif //CAMERASERVER_CONVERT_TO_HIDL
