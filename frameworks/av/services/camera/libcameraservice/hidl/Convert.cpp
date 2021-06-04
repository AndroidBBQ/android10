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

#include <hidl/Convert.h>
#include <gui/bufferqueue/1.0/H2BGraphicBufferProducer.h>
#include <cutils/native_handle.h>
#include <mediautils/AImageReaderUtils.h>

namespace android {
namespace hardware {
namespace cameraservice {
namespace utils {
namespace conversion {

using hardware::graphics::bufferqueue::V1_0::utils::H2BGraphicBufferProducer;
using aimg::AImageReader_getHGBPFromHandle;

// Note: existing data in dst will be gone. Caller still owns the memory of src
void convertToHidl(const camera_metadata_t *src, HCameraMetadata* dst) {
    if (src == nullptr) {
        ALOGW("%s:attempt to convert empty metadata to Hidl", __FUNCTION__);
        return;
    }
    size_t size = get_camera_metadata_size(src);
    dst->setToExternal((uint8_t *) src, size);
    return;
}

int32_t convertFromHidl(HStreamConfigurationMode streamConfigurationMode) {
    switch (streamConfigurationMode) {
        case HStreamConfigurationMode::CONSTRAINED_HIGH_SPEED_MODE:
            return camera2::ICameraDeviceUser::CONSTRAINED_HIGH_SPEED_MODE;
        case HStreamConfigurationMode::NORMAL_MODE:
            return camera2::ICameraDeviceUser::NORMAL_MODE;
        default:
            // TODO: Fix this
            return camera2::ICameraDeviceUser::VENDOR_MODE_START;
    }
}

int32_t convertFromHidl(HTemplateId templateId) {
    switch(templateId) {
        case HTemplateId::PREVIEW:
            return camera2::ICameraDeviceUser::TEMPLATE_PREVIEW;
        case HTemplateId::STILL_CAPTURE:
            return camera2::ICameraDeviceUser::TEMPLATE_STILL_CAPTURE;
        case HTemplateId::RECORD:
            return camera2::ICameraDeviceUser::TEMPLATE_RECORD;
        case HTemplateId::VIDEO_SNAPSHOT:
            return camera2::ICameraDeviceUser::TEMPLATE_VIDEO_SNAPSHOT;
        case HTemplateId::ZERO_SHUTTER_LAG:
            return camera2::ICameraDeviceUser::TEMPLATE_ZERO_SHUTTER_LAG;
        case HTemplateId::MANUAL:
            return camera2::ICameraDeviceUser::TEMPLATE_MANUAL;
    }
}

int convertFromHidl(HOutputConfiguration::Rotation rotation) {
    switch(rotation) {
        case HOutputConfiguration::Rotation::R0:
            return 0;
        case HOutputConfiguration::Rotation::R90:
            return 1;
        case HOutputConfiguration::Rotation::R180:
            return 2;
        case HOutputConfiguration::Rotation::R270:
            return 3;
    }
}

hardware::camera2::params::OutputConfiguration convertFromHidl(
    const HOutputConfiguration &hOutputConfiguration) {
    std::vector<sp<IGraphicBufferProducer>> iGBPs;
    auto &windowHandles = hOutputConfiguration.windowHandles;
    iGBPs.reserve(windowHandles.size());
    for (auto &handle : windowHandles) {
        iGBPs.push_back(new H2BGraphicBufferProducer(AImageReader_getHGBPFromHandle(handle)));
    }
    String16 physicalCameraId16(hOutputConfiguration.physicalCameraId.c_str());
    hardware::camera2::params::OutputConfiguration outputConfiguration(
        iGBPs, convertFromHidl(hOutputConfiguration.rotation), physicalCameraId16,
        hOutputConfiguration.windowGroupId, OutputConfiguration::SURFACE_TYPE_UNKNOWN, 0, 0,
        (windowHandles.size() > 1));
    return outputConfiguration;
}

hardware::camera2::params::SessionConfiguration convertFromHidl(
    const HSessionConfiguration &hSessionConfiguration) {
    hardware::camera2::params::SessionConfiguration sessionConfig(
            hSessionConfiguration.inputWidth, hSessionConfiguration.inputHeight,
            hSessionConfiguration.inputFormat,
            static_cast<int>(hSessionConfiguration.operationMode));

    for (const auto& hConfig : hSessionConfiguration.outputStreams) {
        hardware::camera2::params::OutputConfiguration config = convertFromHidl(hConfig);
        sessionConfig.addOutputConfiguration(config);
    }

    return sessionConfig;
}

// The camera metadata here is cloned. Since we're reading metadata over
// hwbinder we would need to clone it in order to avoid aligment issues.
bool convertFromHidl(const HCameraMetadata &src, CameraMetadata *dst) {
    const camera_metadata_t *buffer = reinterpret_cast<const camera_metadata_t*>(src.data());
    size_t expectedSize = src.size();
    if (buffer != nullptr) {
        int res = validate_camera_metadata_structure(buffer, &expectedSize);
        if (res == OK || res == CAMERA_METADATA_VALIDATION_SHIFTED) {
            *dst = buffer;
        } else {
            ALOGE("%s: Malformed camera metadata received from HAL", __FUNCTION__);
            return false;
        }
    }
    return true;
}

HCameraDeviceStatus convertToHidlCameraDeviceStatus(int32_t status) {
    HCameraDeviceStatus deviceStatus = HCameraDeviceStatus::STATUS_UNKNOWN;
    switch(status) {
        case hardware::ICameraServiceListener::STATUS_NOT_PRESENT:
            deviceStatus = HCameraDeviceStatus::STATUS_NOT_PRESENT;
            break;
        case hardware::ICameraServiceListener::STATUS_PRESENT:
            deviceStatus = HCameraDeviceStatus::STATUS_PRESENT;
            break;
        case hardware::ICameraServiceListener::STATUS_ENUMERATING:
            deviceStatus = HCameraDeviceStatus::STATUS_ENUMERATING;
            break;
        case hardware::ICameraServiceListener::STATUS_NOT_AVAILABLE:
            deviceStatus = HCameraDeviceStatus::STATUS_NOT_AVAILABLE;
            break;
        default:
            break;
    }
    return deviceStatus;
}

HCaptureResultExtras convertToHidl(const CaptureResultExtras &captureResultExtras) {
    HCaptureResultExtras hCaptureResultExtras;
    hCaptureResultExtras.requestId = captureResultExtras.requestId;
    hCaptureResultExtras.burstId = captureResultExtras.burstId;
    hCaptureResultExtras.frameNumber = captureResultExtras.frameNumber;
    hCaptureResultExtras.partialResultCount = captureResultExtras.partialResultCount;
    hCaptureResultExtras.errorStreamId = captureResultExtras.errorStreamId;
    hCaptureResultExtras.errorPhysicalCameraId = hidl_string(String8(
            captureResultExtras.errorPhysicalCameraId).string());
    return hCaptureResultExtras;
}

HErrorCode convertToHidl(int32_t errorCode) {
    switch(errorCode) {
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_DISCONNECTED:
            return HErrorCode::CAMERA_DISCONNECTED;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_DEVICE :
            return HErrorCode::CAMERA_DEVICE;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_SERVICE:
            return HErrorCode::CAMERA_SERVICE;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_REQUEST:
            return HErrorCode::CAMERA_REQUEST;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_RESULT:
            return HErrorCode::CAMERA_RESULT;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_BUFFER:
            return HErrorCode::CAMERA_BUFFER;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_DISABLED:
            return HErrorCode::CAMERA_DISABLED;
        case camera2::ICameraDeviceCallbacks::ERROR_CAMERA_INVALID_ERROR:
            return HErrorCode::CAMERA_INVALID_ERROR;
        default:
            return HErrorCode::CAMERA_UNKNOWN_ERROR;
    }
}

void convertToHidl(const std::vector<hardware::CameraStatus> &src,
                   hidl_vec<HCameraStatusAndId>* dst) {
    dst->resize(src.size());
    size_t i = 0;
    for (auto &statusAndId : src) {
        auto &a = (*dst)[i++];
        a.cameraId = statusAndId.cameraId.c_str();
        a.deviceStatus = convertToHidlCameraDeviceStatus(statusAndId.status);
    }
    return;
}

void convertToHidl(
    const hardware::camera2::utils::SubmitInfo &submitInfo,
    frameworks::cameraservice::device::V2_0::SubmitInfo *hSubmitInfo) {
    hSubmitInfo->requestId = submitInfo.mRequestId;
    hSubmitInfo->lastFrameNumber = submitInfo.mLastFrameNumber;
}

HStatus B2HStatus(const binder::Status &bStatus) {
    HStatus status = HStatus::NO_ERROR;
    if (bStatus.isOk()) {
        // NO Error here
        return status;
    }
    switch(bStatus.serviceSpecificErrorCode()) {
        case hardware::ICameraService::ERROR_DISCONNECTED:
            status = HStatus::DISCONNECTED;
            break;
        case hardware::ICameraService::ERROR_CAMERA_IN_USE:
            status = HStatus::CAMERA_IN_USE;
            break;
        case hardware::ICameraService::ERROR_MAX_CAMERAS_IN_USE:
            status = HStatus::MAX_CAMERAS_IN_USE;
            break;
        case hardware::ICameraService::ERROR_ILLEGAL_ARGUMENT:
            status = HStatus::ILLEGAL_ARGUMENT;
            break;
        case hardware::ICameraService::ERROR_DEPRECATED_HAL:
            // Should not reach here since we filtered legacy HALs earlier
            status = HStatus::DEPRECATED_HAL;
            break;
        case hardware::ICameraService::ERROR_DISABLED:
            status = HStatus::DISABLED;
            break;
        case hardware::ICameraService::ERROR_PERMISSION_DENIED:
            status = HStatus::PERMISSION_DENIED;
            break;
        case hardware::ICameraService::ERROR_INVALID_OPERATION:
            status = HStatus::INVALID_OPERATION;
            break;
        default:
            status = HStatus::UNKNOWN_ERROR;
            break;
    }
  return status;
}

HPhysicalCaptureResultInfo convertToHidl(
    const PhysicalCaptureResultInfo &physicalCaptureResultInfo,
    std::shared_ptr<CaptureResultMetadataQueue> &captureResultMetadataQueue) {
    HPhysicalCaptureResultInfo hPhysicalCaptureResultInfo;
    hPhysicalCaptureResultInfo.physicalCameraId =
        String8(physicalCaptureResultInfo.mPhysicalCameraId).string();
    const camera_metadata_t *rawMetadata =
        physicalCaptureResultInfo.mPhysicalCameraMetadata.getAndLock();
    // Try using fmq at first.
    size_t metadata_size = get_camera_metadata_size(rawMetadata);
    if ((metadata_size > 0) && (captureResultMetadataQueue->availableToWrite() > 0)) {
        if (captureResultMetadataQueue->write((uint8_t *)rawMetadata, metadata_size)) {
            hPhysicalCaptureResultInfo.physicalCameraMetadata.fmqMetadataSize(metadata_size);
        } else {
            ALOGW("%s Couldn't use fmq, falling back to hwbinder", __FUNCTION__);
            HCameraMetadata metadata;
            convertToHidl(rawMetadata, &metadata);
            hPhysicalCaptureResultInfo.physicalCameraMetadata.metadata(std::move(metadata));
        }
    }
    physicalCaptureResultInfo.mPhysicalCameraMetadata.unlock(rawMetadata);
    return hPhysicalCaptureResultInfo;
}

hidl_vec<HPhysicalCaptureResultInfo> convertToHidl(
    const std::vector<PhysicalCaptureResultInfo> &physicalCaptureResultInfos,
    std::shared_ptr<CaptureResultMetadataQueue> &captureResultMetadataQueue) {
    hidl_vec<HPhysicalCaptureResultInfo> hPhysicalCaptureResultInfos;
    hPhysicalCaptureResultInfos.resize(physicalCaptureResultInfos.size());
    size_t i = 0;
    for (auto &physicalCaptureResultInfo : physicalCaptureResultInfos) {
        hPhysicalCaptureResultInfos[i++] = convertToHidl(physicalCaptureResultInfo,
                                                         captureResultMetadataQueue);
    }
    return hPhysicalCaptureResultInfos;
}

} //conversion
} // utils
} //cameraservice
} // hardware
} // android
