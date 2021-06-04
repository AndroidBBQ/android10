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

#include <vector>
#include <inttypes.h>
#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <CameraMetadata.h>

#include "ndk_vendor/impl/ACameraDevice.h"
#include "ACameraCaptureSession.h"
#include "ACameraMetadata.h"
#include "ACaptureRequest.h"
#include "utils.h"

using namespace android;

namespace android {
namespace acam {

template<class T>
camera_status_t
CameraDevice::captureLocked(
        sp<ACameraCaptureSession> session,
        /*optional*/T* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    return submitRequestsLocked(
            session, cbs, numRequests, requests, captureSequenceId, /*isRepeating*/false);
}

template<class T>
camera_status_t
CameraDevice::setRepeatingRequestsLocked(
        sp<ACameraCaptureSession> session,
        /*optional*/T* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    return submitRequestsLocked(
            session, cbs, numRequests, requests, captureSequenceId, /*isRepeating*/true);
}

template<class T>
camera_status_t CameraDevice::submitRequestsLocked(
        sp<ACameraCaptureSession> session,
        /*optional*/T* cbs,
        int numRequests, ACaptureRequest** requests,
        /*out*/int* captureSequenceId,
        bool isRepeating)
{
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Camera %s submit capture request failed! ret %d", getId(), ret);
        return ret;
    }

    // Form two vectors of capture request, one for internal tracking
    std::vector<frameworks::cameraservice::device::V2_0::CaptureRequest> requestList;
    Vector<sp<CaptureRequest>> requestsV;
    requestsV.setCapacity(numRequests);
    for (int i = 0; i < numRequests; i++) {
        sp<CaptureRequest> req;
        ret = allocateCaptureRequestLocked(requests[i], req);
        // We need to call this method since after submitRequestList is called,
        // the request metadata queue might have removed the capture request
        // metadata. Therefore we simply add the metadata to its wrapper class,
        // so that it can be retrieved later.
        addRequestSettingsMetadata(requests[i], req);
        if (ret != ACAMERA_OK) {
            ALOGE("Convert capture request to internal format failure! ret %d", ret);
            return ret;
        }
        if (req->mCaptureRequest.streamAndWindowIds.size() == 0) {
            ALOGE("Capture request without output target cannot be submitted!");
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
        requestList.push_back(utils::convertToHidl(req.get()));
        requestsV.push_back(req);
    }
    if (isRepeating) {
        ret = stopRepeatingLocked();
        if (ret != ACAMERA_OK) {
            ALOGE("Camera %s stop repeating failed! ret %d", getId(), ret);
            return ret;
        }
    }

    SubmitInfo info;
    Status status;
    auto remoteRet = mRemote->submitRequestList(requestList, isRepeating,
                                                [&status, &info](auto s, auto &submitInfo) {
                                                    status = s;
                                                    info = submitInfo;
                                                });
    if (!remoteRet.isOk()) {
        ALOGE("%s: Transaction error for submitRequestList call: %s", __FUNCTION__,
              remoteRet.description().c_str());
    }
    if (status != Status::NO_ERROR) {
        return utils::convertFromHidl(status);
    }
    int32_t sequenceId = info.requestId;
    int64_t lastFrameNumber = info.lastFrameNumber;
    if (sequenceId < 0) {
        ALOGE("Camera %s submit request remote failure: ret %d", getId(), sequenceId);
        return ACAMERA_ERROR_UNKNOWN;
    }

    CallbackHolder cbHolder(session, requestsV, isRepeating, cbs);
    mSequenceCallbackMap.insert(std::make_pair(sequenceId, cbHolder));
    if (isRepeating) {
        // stopRepeating above should have cleanup repeating sequence id
        if (mRepeatingSequenceId != REQUEST_ID_NONE) {
            setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
            return ACAMERA_ERROR_CAMERA_DEVICE;
        }
        mRepeatingSequenceId = sequenceId;
    } else {
        mSequenceLastFrameNumberMap.insert(std::make_pair(sequenceId, lastFrameNumber));
    }

    if (mIdle) {
        sp<AMessage> msg = new AMessage(kWhatSessionStateCb, mHandler);
        msg->setPointer(kContextKey, session->mUserSessionCallback.context);
        msg->setObject(kSessionSpKey, session);
        msg->setPointer(kCallbackFpKey, (void*) session->mUserSessionCallback.onActive);
        postSessionMsgAndCleanup(msg);
    }
    mIdle = false;
    mBusySession = session;

    if (captureSequenceId) {
        *captureSequenceId = sequenceId;
    }
    return ACAMERA_OK;
}

} // namespace acam
} // namespace android
