/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "Camera2-FrameProcessor"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>

#include "common/CameraDeviceBase.h"
#include "api1/Camera2Client.h"
#include "api1/client2/FrameProcessor.h"

namespace android {
namespace camera2 {

FrameProcessor::FrameProcessor(wp<CameraDeviceBase> device,
                               sp<Camera2Client> client) :
    FrameProcessorBase(device),
    mClient(client),
    mLastFrameNumberOfFaces(0),
    mLast3AFrameNumber(-1),
    mLastAEFrameNumber(-1),
    mLastAFrameNumber(-1),
    mLastAWBFrameNumber(-1) {

    sp<CameraDeviceBase> d = device.promote();
    mSynthesize3ANotify = !(d->willNotify3A());

    {
        SharedParameters::Lock l(client->getParameters());

        mUsePartialResult = (mNumPartialResults > 1);

        // Initialize starting 3A state
        m3aState.afTriggerId = l.mParameters.afTriggerCounter;
        m3aState.aeTriggerId = l.mParameters.precaptureTriggerCounter;
        // Check if lens is fixed-focus
        if (l.mParameters.focusMode == Parameters::FOCUS_MODE_FIXED) {
            m3aState.afMode = ANDROID_CONTROL_AF_MODE_OFF;
        } else {
            m3aState.afMode = ANDROID_CONTROL_AF_MODE_AUTO;
        }
        m3aState.awbMode = ANDROID_CONTROL_AWB_MODE_AUTO;
        m3aState.aeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
        m3aState.afState = ANDROID_CONTROL_AF_STATE_INACTIVE;
        m3aState.awbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
    }
}

FrameProcessor::~FrameProcessor() {
}

bool FrameProcessor::processSingleFrame(CaptureResult &frame,
                                        const sp<CameraDeviceBase> &device) {

    sp<Camera2Client> client = mClient.promote();
    if (!client.get()) {
        return false;
    }

    bool isPartialResult = false;
    if (mUsePartialResult) {
        isPartialResult = frame.mResultExtras.partialResultCount < mNumPartialResults;
    }

    if (!isPartialResult && processFaceDetect(frame.mMetadata, client) != OK) {
        return false;
    }

    if (mSynthesize3ANotify) {
        process3aState(frame, client);
    }

    return FrameProcessorBase::processSingleFrame(frame, device);
}

status_t FrameProcessor::processFaceDetect(const CameraMetadata &frame,
        const sp<Camera2Client> &client) {
    status_t res = BAD_VALUE;
    ATRACE_CALL();
    camera_metadata_ro_entry_t entry;
    bool enableFaceDetect;

    {
        SharedParameters::Lock l(client->getParameters());
        enableFaceDetect = l.mParameters.enableFaceDetect;
    }
    entry = frame.find(ANDROID_STATISTICS_FACE_DETECT_MODE);

    // TODO: This should be an error once implementations are compliant
    if (entry.count == 0) {
        return OK;
    }

    uint8_t faceDetectMode = entry.data.u8[0];

    camera_frame_metadata metadata;
    Vector<camera_face_t> faces;
    metadata.number_of_faces = 0;

    if (enableFaceDetect &&
        faceDetectMode != ANDROID_STATISTICS_FACE_DETECT_MODE_OFF) {

        SharedParameters::Lock l(client->getParameters());
        entry = frame.find(ANDROID_STATISTICS_FACE_RECTANGLES);
        if (entry.count == 0) {
            // No faces this frame
            /* warning: locks SharedCameraCallbacks */
            callbackFaceDetection(client, metadata);
            return OK;
        }
        metadata.number_of_faces = entry.count / 4;
        if (metadata.number_of_faces >
                l.mParameters.fastInfo.maxFaces) {
            ALOGE("%s: Camera %d: More faces than expected! (Got %d, max %d)",
                    __FUNCTION__, client->getCameraId(),
                    metadata.number_of_faces, l.mParameters.fastInfo.maxFaces);
            return res;
        }
        const int32_t *faceRects = entry.data.i32;

        entry = frame.find(ANDROID_STATISTICS_FACE_SCORES);
        if (entry.count == 0) {
            ALOGE("%s: Camera %d: Unable to read face scores",
                    __FUNCTION__, client->getCameraId());
            return res;
        }
        const uint8_t *faceScores = entry.data.u8;

        const int32_t *faceLandmarks = NULL;
        const int32_t *faceIds = NULL;

        if (faceDetectMode == ANDROID_STATISTICS_FACE_DETECT_MODE_FULL) {
            entry = frame.find(ANDROID_STATISTICS_FACE_LANDMARKS);
            if (entry.count == 0) {
                ALOGE("%s: Camera %d: Unable to read face landmarks",
                        __FUNCTION__, client->getCameraId());
                return res;
            }
            faceLandmarks = entry.data.i32;

            entry = frame.find(ANDROID_STATISTICS_FACE_IDS);

            if (entry.count == 0) {
                ALOGE("%s: Camera %d: Unable to read face IDs",
                        __FUNCTION__, client->getCameraId());
                return res;
            }
            faceIds = entry.data.i32;
        }

        entry = frame.find(ANDROID_SCALER_CROP_REGION);
        if (entry.count < 4) {
            ALOGE("%s: Camera %d: Unable to read crop region (count = %zu)",
                    __FUNCTION__, client->getCameraId(), entry.count);
            return res;
        }

        Parameters::CropRegion scalerCrop = {
            static_cast<float>(entry.data.i32[0]),
            static_cast<float>(entry.data.i32[1]),
            static_cast<float>(entry.data.i32[2]),
            static_cast<float>(entry.data.i32[3])};

        faces.setCapacity(metadata.number_of_faces);

        size_t maxFaces = metadata.number_of_faces;
        for (size_t i = 0; i < maxFaces; i++) {
            if (faceScores[i] == 0) {
                metadata.number_of_faces--;
                continue;
            }
            if (faceScores[i] > 100) {
                ALOGW("%s: Face index %zu with out of range score %d",
                        __FUNCTION__, i, faceScores[i]);
            }

            camera_face_t face;

            face.rect[0] = l.mParameters.arrayXToNormalizedWithCrop(
                                faceRects[i*4 + 0], scalerCrop);
            face.rect[1] = l.mParameters.arrayYToNormalizedWithCrop(
                                faceRects[i*4 + 1], scalerCrop);
            face.rect[2] = l.mParameters.arrayXToNormalizedWithCrop(
                                faceRects[i*4 + 2], scalerCrop);
            face.rect[3] = l.mParameters.arrayYToNormalizedWithCrop(
                                faceRects[i*4 + 3], scalerCrop);
            face.score = faceScores[i];
            if (faceDetectMode == ANDROID_STATISTICS_FACE_DETECT_MODE_FULL) {
                face.id = faceIds[i];
                face.left_eye[0] = l.mParameters.arrayXToNormalizedWithCrop(
                        faceLandmarks[i*6 + 0], scalerCrop);
                face.left_eye[1] = l.mParameters.arrayYToNormalizedWithCrop(
                        faceLandmarks[i*6 + 1], scalerCrop);
                face.right_eye[0] = l.mParameters.arrayXToNormalizedWithCrop(
                        faceLandmarks[i*6 + 2], scalerCrop);
                face.right_eye[1] = l.mParameters.arrayYToNormalizedWithCrop(
                        faceLandmarks[i*6 + 3], scalerCrop);
                face.mouth[0] = l.mParameters.arrayXToNormalizedWithCrop(
                        faceLandmarks[i*6 + 4], scalerCrop);
                face.mouth[1] = l.mParameters.arrayYToNormalizedWithCrop(
                        faceLandmarks[i*6 + 5], scalerCrop);
            } else {
                face.id = 0;
                face.left_eye[0] = face.left_eye[1] = -2000;
                face.right_eye[0] = face.right_eye[1] = -2000;
                face.mouth[0] = face.mouth[1] = -2000;
            }
            faces.push_back(face);
        }

        metadata.faces = faces.editArray();
    }

    /* warning: locks SharedCameraCallbacks */
    callbackFaceDetection(client, metadata);

    return OK;
}

status_t FrameProcessor::process3aState(const CaptureResult &frame,
        const sp<Camera2Client> &client) {

    ATRACE_CALL();
    const CameraMetadata &metadata = frame.mMetadata;
    camera_metadata_ro_entry_t entry;
    int cameraId = client->getCameraId();

    entry = metadata.find(ANDROID_REQUEST_FRAME_COUNT);
    int32_t frameNumber = entry.data.i32[0];

    // Don't send 3A notifications for the same frame number twice
    if (frameNumber <= mLast3AFrameNumber) {
        ALOGV("%s: Already sent 3A for frame number %d, skipping",
                __FUNCTION__, frameNumber);

        // Remove the entry if there is one for this frame number in mPending3AStates.
        mPending3AStates.removeItem(frameNumber);
        return OK;
    }

    AlgState pendingState;

    ssize_t index = mPending3AStates.indexOfKey(frameNumber);
    if (index != NAME_NOT_FOUND) {
        pendingState = mPending3AStates.valueAt(index);
    }

    // Update 3A states from the result.
    bool gotAllStates = true;

    // TODO: Also use AE mode, AE trigger ID
    bool gotAFState = updatePendingState<uint8_t>(metadata, ANDROID_CONTROL_AF_MODE,
            &pendingState.afMode, frameNumber, cameraId);

    bool gotAWBState = updatePendingState<uint8_t>(metadata, ANDROID_CONTROL_AWB_MODE,
            &pendingState.awbMode, frameNumber, cameraId);

    bool gotAEState = updatePendingState<uint8_t>(metadata, ANDROID_CONTROL_AE_STATE,
            &pendingState.aeState, frameNumber, cameraId);

    gotAFState &= updatePendingState<uint8_t>(metadata, ANDROID_CONTROL_AF_STATE,
            &pendingState.afState, frameNumber, cameraId);

    gotAWBState &= updatePendingState<uint8_t>(metadata, ANDROID_CONTROL_AWB_STATE,
            &pendingState.awbState, frameNumber, cameraId);

    pendingState.afTriggerId = frame.mResultExtras.afTriggerId;
    pendingState.aeTriggerId = frame.mResultExtras.precaptureTriggerId;

    if (gotAEState && (frameNumber > mLastAEFrameNumber)) {
        if (pendingState.aeState != m3aState.aeState ||
                pendingState.aeTriggerId > m3aState.aeTriggerId) {
            ALOGV("%s: Camera %d: AE state %d->%d",
                    __FUNCTION__, cameraId,
                    m3aState.aeState, pendingState.aeState);
            client->notifyAutoExposure(pendingState.aeState, pendingState.aeTriggerId);

            m3aState.aeState = pendingState.aeState;
            m3aState.aeTriggerId = pendingState.aeTriggerId;
            mLastAEFrameNumber = frameNumber;
        }
    }

    if (gotAFState && (frameNumber > mLastAFrameNumber)) {
        if (pendingState.afState != m3aState.afState ||
                pendingState.afMode != m3aState.afMode ||
                pendingState.afTriggerId != m3aState.afTriggerId) {
            ALOGV("%s: Camera %d: AF state %d->%d. AF mode %d->%d. Trigger %d->%d",
                    __FUNCTION__, cameraId,
                    m3aState.afState, pendingState.afState,
                    m3aState.afMode, pendingState.afMode,
                    m3aState.afTriggerId, pendingState.afTriggerId);
            client->notifyAutoFocus(pendingState.afState, pendingState.afTriggerId);

            m3aState.afState = pendingState.afState;
            m3aState.afMode = pendingState.afMode;
            m3aState.afTriggerId = pendingState.afTriggerId;
            mLastAFrameNumber = frameNumber;
        }
    }

    if (gotAWBState && (frameNumber > mLastAWBFrameNumber)) {
        if (pendingState.awbState != m3aState.awbState ||
                pendingState.awbMode != m3aState.awbMode) {
            ALOGV("%s: Camera %d: AWB state %d->%d. AWB mode %d->%d",
                    __FUNCTION__, cameraId,
                    m3aState.awbState, pendingState.awbState,
                    m3aState.awbMode, pendingState.awbMode);
            client->notifyAutoWhitebalance(pendingState.awbState,
                    pendingState.aeTriggerId);

            m3aState.awbMode = pendingState.awbMode;
            m3aState.awbState = pendingState.awbState;
            mLastAWBFrameNumber = frameNumber;
        }
    }

    gotAllStates &= gotAEState & gotAFState & gotAWBState;
    if (!gotAllStates) {
        // If not all states are received, put the pending state to mPending3AStates.
        if (index == NAME_NOT_FOUND) {
            mPending3AStates.add(frameNumber, pendingState);
        } else {
            mPending3AStates.replaceValueAt(index, pendingState);
        }
        return NOT_ENOUGH_DATA;
    }

    if (index != NAME_NOT_FOUND) {
        mPending3AStates.removeItemsAt(index);
    }

    mLast3AFrameNumber = frameNumber;

    return OK;
}

template<typename Src, typename T>
bool FrameProcessor::updatePendingState(const CameraMetadata& result, int32_t tag,
        T* value, int32_t frameNumber, int cameraId) {
    camera_metadata_ro_entry_t entry;
    if (value == NULL) {
        ALOGE("%s: Camera %d: Value to write to is NULL",
                __FUNCTION__, cameraId);
        return false;
    }

    // Already got the value for this tag.
    if (*value != static_cast<T>(NOT_SET)) {
        return true;
    }

    entry = result.find(tag);
    if (entry.count == 0) {
        const camera_metadata *metaBuffer = result.getAndLock();
        ALOGV("%s: Camera %d: No %s provided by HAL for frame %d in this result!",
                __FUNCTION__, cameraId,
                get_local_camera_metadata_tag_name(tag, metaBuffer),
                frameNumber);
        result.unlock(metaBuffer);
        return false;
    } else {
        switch(sizeof(Src)){
            case sizeof(uint8_t):
                *value = static_cast<T>(entry.data.u8[0]);
                break;
            case sizeof(int32_t):
                *value = static_cast<T>(entry.data.i32[0]);
                break;
            default:
                ALOGE("%s: Camera %d: Unsupported source",
                        __FUNCTION__, cameraId);
                return false;
        }
    }
    return true;
}


void FrameProcessor::callbackFaceDetection(const sp<Camera2Client>& client,
                                     const camera_frame_metadata &metadata) {

    camera_frame_metadata *metadata_ptr =
        const_cast<camera_frame_metadata*>(&metadata);

    /**
     * Filter out repeated 0-face callbacks,
     * but not when the last frame was >0
     */
    if (metadata.number_of_faces != 0 ||
        mLastFrameNumberOfFaces != metadata.number_of_faces) {

        Camera2Client::SharedCameraCallbacks::Lock
            l(client->mSharedCameraCallbacks);
        if (l.mRemoteCallback != NULL) {
            l.mRemoteCallback->dataCallback(CAMERA_MSG_PREVIEW_METADATA,
                                            NULL,
                                            metadata_ptr);
        }
    }

    mLastFrameNumberOfFaces = metadata.number_of_faces;
}

}; // namespace camera2
}; // namespace android
