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
#include <hardware/camera.h>

#include <hidl/AidlCameraDeviceCallbacks.h>
#include <hidl/Convert.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace device {
namespace V2_0 {
namespace implementation {

using hardware::hidl_vec;
using HCaptureResultExtras = android::frameworks::cameraservice::device::V2_0::CaptureResultExtras;
using HPhysicalCaptureResultInfo = android::frameworks::cameraservice::device::V2_0::PhysicalCaptureResultInfo;
using HCameraMetadata = android::frameworks::cameraservice::device::V2_0::CameraMetadata;

const char *H2BCameraDeviceCallbacks::kResultKey = "CaptureResult";

H2BCameraDeviceCallbacks::H2BCameraDeviceCallbacks(const sp<HalInterface>& base) : CBase(base) { }

bool H2BCameraDeviceCallbacks::initializeLooper() {
    mCbLooper = new ALooper;
    mCbLooper->setName("cs-looper");
    status_t err = mCbLooper->start(/*runOnCallingThread*/ false, /*canCallJava*/ false,
                                    PRIORITY_DEFAULT);
    if (err !=OK) {
        ALOGE("Unable to start camera device callback looper");
        return false;
    }
    mHandler = new CallbackHandler(this);
    mCbLooper->registerHandler(mHandler);
    return true;
}

H2BCameraDeviceCallbacks::~H2BCameraDeviceCallbacks() {
    if (mCbLooper != nullptr) {
        if (mHandler != nullptr) {
            mCbLooper->unregisterHandler(mHandler->id());
        }
        mCbLooper->stop();
    }
    mCbLooper.clear();
    mHandler.clear();
}

binder::Status H2BCameraDeviceCallbacks::onDeviceError(
    int32_t errorCode, const CaptureResultExtras& resultExtras) {
    using hardware::cameraservice::utils::conversion::convertToHidl;
    HCaptureResultExtras hCaptureResultExtras = convertToHidl(resultExtras);
    auto ret = mBase->onDeviceError(convertToHidl(errorCode), hCaptureResultExtras);
    if (!ret.isOk()) {
        ALOGE("%s OnDeviceError callback failed due to %s",__FUNCTION__,
              ret.description().c_str());
    }
    return binder::Status::ok();
}

binder::Status H2BCameraDeviceCallbacks::onDeviceIdle() {
    auto ret = mBase->onDeviceIdle();
    if (!ret.isOk()) {
          ALOGE("%s OnDeviceIdle callback failed due to %s",__FUNCTION__,
                ret.description().c_str());
    }
    return binder::Status::ok();
}

binder::Status H2BCameraDeviceCallbacks::onCaptureStarted(
    const CaptureResultExtras& resultExtras, int64_t timestamp) {
    using hardware::cameraservice::utils::conversion::convertToHidl;
    HCaptureResultExtras hCaptureResultExtras = convertToHidl(resultExtras);
    auto ret = mBase->onCaptureStarted(hCaptureResultExtras, timestamp);
    if (!ret.isOk()) {
        ALOGE("%s OnCaptureCallback failed due to %s",__FUNCTION__,
              ret.description().c_str());
    }
    return binder::Status::ok();
}

void H2BCameraDeviceCallbacks::convertResultMetadataToHidl(const camera_metadata_t *rawMetadata,
                                                           FmqSizeOrMetadata *hResultMetadata) {
    // First try writing to fmq.
    size_t metadata_size = get_camera_metadata_size(rawMetadata);
    if ((metadata_size > 0) &&
        (mCaptureResultMetadataQueue->availableToWrite() > 0)) {
        if (mCaptureResultMetadataQueue->write((uint8_t *)rawMetadata, metadata_size)) {
            hResultMetadata->fmqMetadataSize(metadata_size);
        } else {
            ALOGW("%s Couldn't use fmq, falling back to hwbinder", __FUNCTION__);
            HCameraMetadata metadata;
            hardware::cameraservice::utils::conversion::convertToHidl(rawMetadata, &metadata);
            hResultMetadata->metadata(std::move(metadata));
        }
    }
}

void H2BCameraDeviceCallbacks::CallbackHandler::onMessageReceived(const sp<AMessage> &msg) {
    sp<RefBase> obj = nullptr;
    sp<ResultWrapper> resultWrapper = nullptr;
    bool found = false;
    switch (msg->what()) {
        case kWhatResultReceived:
            found = msg->findObject(kResultKey, &obj);
            if (!found || obj == nullptr) {
                ALOGE("Cannot find result object in callback message");
                return;
            }
            resultWrapper = static_cast<ResultWrapper *>(obj.get());
            processResultMessage(resultWrapper);
            break;
        default:
            ALOGE("Unknown callback sent");
            break;
    }
    return;
}

void H2BCameraDeviceCallbacks::CallbackHandler::processResultMessage(
    sp<ResultWrapper> &resultWrapper) {
    sp<H2BCameraDeviceCallbacks> converter = mConverter.promote();
    if (converter == nullptr) {
        ALOGE("Callback wrapper has died, result callback cannot be made");
        return;
    }
    CameraMetadataNative &result = resultWrapper->mResult;
    auto resultExtras = resultWrapper->mResultExtras;
    HCaptureResultExtras hResultExtras =
            hardware::cameraservice::utils::conversion::convertToHidl(resultExtras);

    // Convert Metadata into HCameraMetadata;
    FmqSizeOrMetadata hResult;
    const camera_metadata_t *rawMetadata = result.getAndLock();
    converter->convertResultMetadataToHidl(rawMetadata, &hResult);
    result.unlock(rawMetadata);
    auto &physicalCaptureResultInfos = resultWrapper->mPhysicalCaptureResultInfos;
    hidl_vec<HPhysicalCaptureResultInfo> hPhysicalCaptureResultInfos =
            hardware::cameraservice::utils::conversion::convertToHidl(
                    physicalCaptureResultInfos, converter->mCaptureResultMetadataQueue);
    auto ret = converter->mBase->onResultReceived(hResult, hResultExtras,
                                                  hPhysicalCaptureResultInfos);
    if (!ret.isOk()) {
          ALOGE("%s OnResultReceived callback failed due to %s",__FUNCTION__,
                ret.description().c_str());
    }
}

binder::Status H2BCameraDeviceCallbacks::onResultReceived(
    const CameraMetadataNative& result,
    const CaptureResultExtras& resultExtras,
    const ::std::vector<PhysicalCaptureResultInfo>& physicalCaptureResultInfos) {
    // Wrap CameraMetadata, resultExtras and physicalCaptureResultInfos in on
    // sp<RefBase>-able structure and post it.
    sp<ResultWrapper> resultWrapper = new ResultWrapper(const_cast<CameraMetadataNative &>(result),
                                                        resultExtras, physicalCaptureResultInfos);
    sp<AMessage> msg = new AMessage(kWhatResultReceived, mHandler);
    msg->setObject(kResultKey, resultWrapper);
    msg->post();
    return binder::Status::ok();
}

binder::Status H2BCameraDeviceCallbacks::onPrepared(int32_t streamId) {
    // not implemented
    // To silence Wunused-parameter.
    (void) streamId;
    return binder::Status::ok();
}

binder::Status H2BCameraDeviceCallbacks::onRepeatingRequestError(
    int64_t lastFrameNumber,
    int32_t repeatingRequestId) {
    auto ret =
        mBase->onRepeatingRequestError(lastFrameNumber, repeatingRequestId);
    if (!ret.isOk()) {
        ALOGE("%s OnRepeatingRequestEror callback failed due to %s",__FUNCTION__,
              ret.description().c_str());
    }
    return binder::Status::ok();
}

binder::Status H2BCameraDeviceCallbacks::onRequestQueueEmpty() {
    // not implemented
    return binder::Status::ok();
}

} // implementation
} // V2_0
} // device
} // cameraservice
} // frameworks
} // android
