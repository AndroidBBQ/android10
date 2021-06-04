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

#define LOG_TAG "Camera3-CompositeStream"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>

#include "common/CameraDeviceBase.h"
#include "CameraDeviceClient.h"
#include "CompositeStream.h"

namespace android {
namespace camera3 {

CompositeStream::CompositeStream(wp<CameraDeviceBase> device,
        wp<hardware::camera2::ICameraDeviceCallbacks> cb) :
        mDevice(device),
        mRemoteCallback(cb),
        mNumPartialResults(1),
        mErrorState(false) {
    sp<CameraDeviceBase> cameraDevice = device.promote();
    if (cameraDevice.get() != nullptr) {
        CameraMetadata staticInfo = cameraDevice->info();
        camera_metadata_entry_t entry = staticInfo.find(ANDROID_REQUEST_PARTIAL_RESULT_COUNT);
        if (entry.count > 0) {
            mNumPartialResults = entry.data.i32[0];
        }
    }
}

status_t CompositeStream::createStream(const std::vector<sp<Surface>>& consumers,
        bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
        camera3_stream_rotation_t rotation, int * id, const String8& physicalCameraId,
        std::vector<int> * surfaceIds, int streamSetId, bool isShared) {
    if (hasDeferredConsumer) {
        ALOGE("%s: Deferred consumers not supported in case of composite streams!",
                __FUNCTION__);
        return BAD_VALUE;
    }

    if (streamSetId != camera3::CAMERA3_STREAM_ID_INVALID) {
        ALOGE("%s: Surface groups not supported in case of composite streams!",
                __FUNCTION__);
        return BAD_VALUE;
    }

    if (isShared) {
        ALOGE("%s: Shared surfaces not supported in case of composite streams!",
                __FUNCTION__);
        return BAD_VALUE;
    }

    return createInternalStreams(consumers, hasDeferredConsumer, width, height, format, rotation, id,
            physicalCameraId, surfaceIds, streamSetId, isShared);
}

status_t CompositeStream::deleteStream() {
    {
        Mutex::Autolock l(mMutex);
        mPendingCaptureResults.clear();
        mCaptureResults.clear();
        mFrameNumberMap.clear();
        mErrorFrameNumbers.clear();
    }

    return deleteInternalStreams();
}

void CompositeStream::onBufferRequestForFrameNumber(uint64_t frameNumber, int streamId,
        const CameraMetadata& /*settings*/) {
    Mutex::Autolock l(mMutex);
    if (!mErrorState && (streamId == getStreamId())) {
        mPendingCaptureResults.emplace(frameNumber, CameraMetadata());
    }
}

void CompositeStream::onBufferReleased(const BufferInfo& bufferInfo) {
    Mutex::Autolock l(mMutex);
    if (!mErrorState && !bufferInfo.mError) {
        mFrameNumberMap.emplace(bufferInfo.mFrameNumber, bufferInfo.mTimestamp);
        mInputReadyCondition.signal();
    }
}

void CompositeStream::eraseResult(int64_t frameNumber) {
    Mutex::Autolock l(mMutex);

    auto it = mPendingCaptureResults.find(frameNumber);
    if (it == mPendingCaptureResults.end()) {
        return;
    }

    it = mPendingCaptureResults.erase(it);
}

void CompositeStream::onResultAvailable(const CaptureResult& result) {
    bool resultError = false;
    {
        Mutex::Autolock l(mMutex);

        uint64_t frameNumber = result.mResultExtras.frameNumber;
        bool resultReady = false;
        auto it = mPendingCaptureResults.find(frameNumber);
        if (it != mPendingCaptureResults.end()) {
            it->second.append(result.mMetadata);
            if (result.mResultExtras.partialResultCount >= mNumPartialResults) {
                auto entry = it->second.find(ANDROID_SENSOR_TIMESTAMP);
                if (entry.count == 1) {
                    auto ts = entry.data.i64[0];
                    mCaptureResults.emplace(ts, std::make_tuple(frameNumber, it->second));
                    resultReady = true;
                } else {
                    ALOGE("%s: Timestamp metadata entry missing for frameNumber: %" PRIu64,
                            __FUNCTION__, frameNumber);
                    resultError = true;
                }
                mPendingCaptureResults.erase(it);
            }
        }

        if (resultReady) {
            mInputReadyCondition.signal();
        }
    }

    if (resultError) {
        onResultError(result.mResultExtras);
    }
}

void CompositeStream::flagAnErrorFrameNumber(int64_t frameNumber) {
    Mutex::Autolock l(mMutex);
    mErrorFrameNumbers.emplace(frameNumber);
    mInputReadyCondition.signal();
}

status_t CompositeStream::registerCompositeStreamListener(int32_t streamId) {
    sp<CameraDeviceBase> device = mDevice.promote();
    if (device.get() == nullptr) {
        return NO_INIT;
    }

    auto ret = device->addBufferListenerForStream(streamId, this);
    if (ret != OK) {
        ALOGE("%s: Failed to register composite stream listener!", __FUNCTION__);
    }

    return ret;
}

bool CompositeStream::onError(int32_t errorCode, const CaptureResultExtras& resultExtras) {
    auto ret = false;
    switch (errorCode) {
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_RESULT:
            onResultError(resultExtras);
            break;
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_BUFFER:
            ret = onStreamBufferError(resultExtras);
            break;
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_REQUEST:
            // Invalid request, this shouldn't affect composite streams.
            break;
        default:
            ALOGE("%s: Unrecoverable error: %d detected!", __FUNCTION__, errorCode);
            Mutex::Autolock l(mMutex);
            mErrorState = true;
            break;
    }

    return ret;
}

void CompositeStream::notifyError(int64_t frameNumber) {
    sp<hardware::camera2::ICameraDeviceCallbacks> remoteCb =
        mRemoteCallback.promote();

    if ((frameNumber >= 0) && (remoteCb.get() != nullptr)) {
        CaptureResultExtras extras;
        extras.errorStreamId = getStreamId();
        extras.frameNumber = frameNumber;
        remoteCb->onDeviceError(
                hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_BUFFER,
                extras);
    }
}

}; // namespace camera3
}; // namespace android
