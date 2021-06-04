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

#include <android/hardware/camera/device/3.2/types.h>
#include <cutils/properties.h>
#include <gui/Surface.h>
#include <gui/bufferqueue/1.0/H2BGraphicBufferProducer.h>

#include <hidl/AidlCameraDeviceCallbacks.h>
#include <hidl/Convert.h>
#include <hidl/HidlCameraDeviceUser.h>
#include <android/hardware/camera/device/3.2/types.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace device {
namespace V2_0 {
namespace implementation {

using hardware::cameraservice::utils::conversion::convertToHidl;
using hardware::cameraservice::utils::conversion::convertFromHidl;
using hardware::cameraservice::utils::conversion::B2HStatus;

using hardware::graphics::bufferqueue::V1_0::utils::H2BGraphicBufferProducer;
using hardware::hidl_vec;
using hardware::Return;
using hardware::Void;
using HSubmitInfo = device::V2_0::SubmitInfo;
using hardware::camera2::params::OutputConfiguration;
using hardware::camera2::params::SessionConfiguration;

static constexpr int32_t CAMERA_REQUEST_METADATA_QUEUE_SIZE = 1 << 20 /* 1 MB */;
static constexpr int32_t CAMERA_RESULT_METADATA_QUEUE_SIZE = 1 << 20 /* 1 MB */;

Return<void> HidlCameraDeviceUser::disconnect() {
    mDeviceRemote->disconnect();
    return Void();
}

HidlCameraDeviceUser::HidlCameraDeviceUser(
    const sp<hardware::camera2::ICameraDeviceUser> &deviceRemote)
  : mDeviceRemote(deviceRemote) {
    mInitSuccess = initDevice();
}

bool HidlCameraDeviceUser::initDevice() {
    // TODO: Get request and result metadata queue size from a system property.
    int32_t reqFMQSize = CAMERA_REQUEST_METADATA_QUEUE_SIZE;

    mCaptureRequestMetadataQueue =
        std::make_unique<CaptureRequestMetadataQueue>(static_cast<size_t>(reqFMQSize),
                                                      false /* non blocking */);
    if (!mCaptureRequestMetadataQueue->isValid()) {
        ALOGE("%s: invalid request fmq", __FUNCTION__);
        return false;
    }

    int32_t resFMQSize = CAMERA_RESULT_METADATA_QUEUE_SIZE;
    mCaptureResultMetadataQueue =
        std::make_shared<CaptureResultMetadataQueue>(static_cast<size_t>(resFMQSize),
                                                     false /* non blocking */);
    if (!mCaptureResultMetadataQueue->isValid()) {
        ALOGE("%s: invalid result fmq", __FUNCTION__);
        return false;
    }
    return true;
}

Return<void> HidlCameraDeviceUser::getCaptureRequestMetadataQueue(
    getCaptureRequestMetadataQueue_cb _hidl_cb) {
    if (mInitSuccess) {
        _hidl_cb(*mCaptureRequestMetadataQueue->getDesc());
    }
    return Void();
}

Return<void> HidlCameraDeviceUser::getCaptureResultMetadataQueue(
    getCaptureResultMetadataQueue_cb _hidl_cb) {
    if (mInitSuccess) {
        _hidl_cb(*mCaptureResultMetadataQueue->getDesc());
    }
    return Void();
}

/**
 * To be used only by submitRequestList implementation, since it requires
 * clients to call this method serially, incase fmq is used to send metadata.
 */
bool HidlCameraDeviceUser::copyPhysicalCameraSettings(
    const hidl_vec<HPhysicalCameraSettings> &hPhysicalCameraSettings,
    std::vector<CaptureRequest::PhysicalCameraSettings> *physicalCameraSettings) {
    bool converted = false;
    for (auto &e : hPhysicalCameraSettings) {
        physicalCameraSettings->emplace_back();
        CaptureRequest::PhysicalCameraSettings &physicalCameraSetting =
            physicalCameraSettings->back();
        physicalCameraSetting.id = e.id.c_str();

        // Read the settings either from the fmq or straightaway from the
        // request. We don't need any synchronization, since submitRequestList
        // is guaranteed to be called serially by the client if it decides to
        // use fmq.
        if (e.settings.getDiscriminator() ==
            FmqSizeOrMetadata::hidl_discriminator::fmqMetadataSize) {
            /**
             * Get settings from the fmq.
             */
            HCameraMetadata settingsFmq;
            settingsFmq.resize(e.settings.fmqMetadataSize());
            bool read = mCaptureRequestMetadataQueue->read(settingsFmq.data(),
                                                           e.settings.fmqMetadataSize());
            if (!read) {
                ALOGE("%s capture request settings could't be read from fmq size",
                      __FUNCTION__);
                converted = false;
            } else {
                converted = convertFromHidl(settingsFmq, &physicalCameraSetting.settings);
            }
        } else {
            /**
             * The settings metadata is contained in request settings field.
             */
            converted =
                convertFromHidl(e.settings.metadata(),
                                &physicalCameraSetting.settings);
        }
        if (!converted) {
          ALOGE("%s: Unable to convert physicalCameraSettings from HIDL to AIDL.", __FUNCTION__);
          return false;
        }
    }
    return true;
}

bool HidlCameraDeviceUser::convertRequestFromHidl(const HCaptureRequest &hRequest,
                                                  CaptureRequest *request) {
    // No reprocessing support.
    request->mIsReprocess = false;
    for (const auto &streamAndWindowId : hRequest.streamAndWindowIds) {
        request->mStreamIdxList.push_back(streamAndWindowId.streamId);
        request->mSurfaceIdxList.push_back(streamAndWindowId.windowId);
    }
    return copyPhysicalCameraSettings(hRequest.physicalCameraSettings,
                                      &(request->mPhysicalCameraSettings));
}

Return<void> HidlCameraDeviceUser::submitRequestList(const hidl_vec<HCaptureRequest>& hRequestList,
                                                     bool streaming,
                                                     submitRequestList_cb _hidl_cb) {
    hardware::camera2::utils::SubmitInfo submitInfo;
    HSubmitInfo hSubmitInfo;
    /**
     * Create AIDL CaptureRequest from requestList and graphicBufferProducers.
     */
    std::vector<hardware::camera2::CaptureRequest> requests;
    for (auto &hRequest : hRequestList) {
        requests.emplace_back();
        auto &request = requests.back();
        if (!convertRequestFromHidl(hRequest, &request)) {
            _hidl_cb(HStatus::ILLEGAL_ARGUMENT, hSubmitInfo);
            return Void();
        }
    }
    mDeviceRemote->submitRequestList(requests, streaming, &submitInfo);
    mRequestId = submitInfo.mRequestId;
    convertToHidl(submitInfo, &hSubmitInfo);
    _hidl_cb(HStatus::NO_ERROR, hSubmitInfo);
    return Void();
}

Return<void> HidlCameraDeviceUser::cancelRepeatingRequest(cancelRepeatingRequest_cb _hidl_cb) {
    int64_t lastFrameNumber = 0;
    binder::Status ret = mDeviceRemote->cancelRequest(mRequestId, &lastFrameNumber);
    _hidl_cb(B2HStatus(ret), lastFrameNumber);
    return Void();
}

Return<HStatus> HidlCameraDeviceUser::beginConfigure() {
    binder::Status ret = mDeviceRemote->beginConfigure();
    return B2HStatus(ret);
}

Return<HStatus> HidlCameraDeviceUser::endConfigure(StreamConfigurationMode operatingMode,
                                                   const hidl_vec<uint8_t>& sessionParams) {
    android::CameraMetadata cameraMetadata;
    if (!convertFromHidl(sessionParams, &cameraMetadata)) {
        return HStatus::ILLEGAL_ARGUMENT;
    }

    binder::Status ret = mDeviceRemote->endConfigure(convertFromHidl(operatingMode),
                                                     cameraMetadata);
    return B2HStatus(ret);
}

Return<HStatus> HidlCameraDeviceUser::deleteStream(int32_t streamId) {
    binder::Status ret = mDeviceRemote->deleteStream(streamId);
    return B2HStatus(ret);
}

Return<void> HidlCameraDeviceUser::createStream(const HOutputConfiguration& hOutputConfiguration,
                                                createStream_cb hidl_cb_) {
    OutputConfiguration outputConfiguration =
        convertFromHidl(hOutputConfiguration);
    int32_t newStreamId = 0;
    binder::Status ret = mDeviceRemote->createStream(outputConfiguration, &newStreamId);
    HStatus status = B2HStatus(ret);
    hidl_cb_(status, newStreamId);
    return Void();
}

Return<void> HidlCameraDeviceUser::createDefaultRequest(TemplateId templateId,
                                                        createDefaultRequest_cb _hidl_cb) {
    android::CameraMetadata cameraMetadata;
    binder::Status ret = mDeviceRemote->createDefaultRequest(convertFromHidl(templateId),
                                                             &cameraMetadata);
    HStatus hStatus = B2HStatus(ret);
    HCameraMetadata hidlMetadata;
    const camera_metadata_t *rawMetadata = cameraMetadata.getAndLock();
    convertToHidl(rawMetadata, &hidlMetadata);
    _hidl_cb(hStatus, hidlMetadata);
    cameraMetadata.unlock(rawMetadata);
    return Void();
}

Return<HStatus> HidlCameraDeviceUser::waitUntilIdle() {
    binder::Status ret = mDeviceRemote->waitUntilIdle();
    return B2HStatus(ret);
}

Return<void> HidlCameraDeviceUser::flush(flush_cb _hidl_cb) {
    int64_t lastFrameNumber = 0;
    binder::Status ret = mDeviceRemote->flush(&lastFrameNumber);
    _hidl_cb(B2HStatus(ret),lastFrameNumber);
    return Void();
}

Return<HStatus> HidlCameraDeviceUser::updateOutputConfiguration(
    int32_t streamId,
    const HOutputConfiguration& hOutputConfiguration) {
    OutputConfiguration outputConfiguration = convertFromHidl(hOutputConfiguration);
    binder::Status ret = mDeviceRemote->updateOutputConfiguration(streamId, outputConfiguration);
    return B2HStatus(ret);
}

Return<void> HidlCameraDeviceUser::isSessionConfigurationSupported(
    const HSessionConfiguration& hSessionConfiguration,
    isSessionConfigurationSupported_cb _hidl_cb) {
    bool supported = false;
    SessionConfiguration sessionConfiguration = convertFromHidl(hSessionConfiguration);
    binder::Status ret = mDeviceRemote->isSessionConfigurationSupported(
            sessionConfiguration, &supported);
    HStatus status = B2HStatus(ret);
    _hidl_cb(status, supported);
    return Void();
}

} // implementation
} // V2_0
} // device
} // cameraservice
} // frameworks
} // android
