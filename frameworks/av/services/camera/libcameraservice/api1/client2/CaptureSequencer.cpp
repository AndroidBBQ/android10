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

#define LOG_TAG "Camera2-CaptureSequencer"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <inttypes.h>

#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/Vector.h>

#include "api1/Camera2Client.h"
#include "api1/client2/CaptureSequencer.h"
#include "api1/client2/Parameters.h"
#include "api1/client2/ZslProcessor.h"

namespace android {
namespace camera2 {

/** Public members */

CaptureSequencer::CaptureSequencer(wp<Camera2Client> client):
        Thread(false),
        mStartCapture(false),
        mBusy(false),
        mNewAEState(false),
        mNewFrameReceived(false),
        mNewCaptureReceived(false),
        mNewCaptureErrorCnt(0),
        mShutterNotified(false),
        mHalNotifiedShutter(false),
        mShutterCaptureId(-1),
        mClient(client),
        mCaptureState(IDLE),
        mStateTransitionCount(0),
        mTriggerId(0),
        mTimeoutCount(0),
        mCaptureId(Camera2Client::kCaptureRequestIdStart) {
    ALOGV("%s", __FUNCTION__);
}

CaptureSequencer::~CaptureSequencer() {
    ALOGV("%s: Exit", __FUNCTION__);
}

void CaptureSequencer::setZslProcessor(const wp<ZslProcessor>& processor) {
    Mutex::Autolock l(mInputMutex);
    mZslProcessor = processor;
}

status_t CaptureSequencer::startCapture() {
    ALOGV("%s", __FUNCTION__);
    ATRACE_CALL();
    Mutex::Autolock l(mInputMutex);
    if (mBusy) {
        ALOGE("%s: Already busy capturing!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if (!mStartCapture) {
        mStartCapture = true;
        mStartCaptureSignal.signal();
    }
    return OK;
}

status_t CaptureSequencer::waitUntilIdle(nsecs_t timeout) {
    ATRACE_CALL();
    ALOGV("%s: Waiting for idle", __FUNCTION__);
    Mutex::Autolock l(mStateMutex);
    status_t res = -1;
    while (mCaptureState != IDLE) {
        nsecs_t startTime = systemTime();

        res = mStateChanged.waitRelative(mStateMutex, timeout);
        if (res != OK) return res;

        timeout -= (systemTime() - startTime);
    }
    ALOGV("%s: Now idle", __FUNCTION__);
    return OK;
}

void CaptureSequencer::notifyAutoExposure(uint8_t newState, int triggerId) {
    ATRACE_CALL();
    Mutex::Autolock l(mInputMutex);
    mAEState = newState;
    mAETriggerId = triggerId;
    if (!mNewAEState) {
        mNewAEState = true;
        mNewNotifySignal.signal();
    }
}

void CaptureSequencer::notifyShutter(const CaptureResultExtras& resultExtras,
                                     nsecs_t timestamp) {
    ATRACE_CALL();
    (void) timestamp;
    Mutex::Autolock l(mInputMutex);
    if (!mHalNotifiedShutter && resultExtras.requestId == mShutterCaptureId) {
        mHalNotifiedShutter = true;
        mShutterNotifySignal.signal();
    }
}

void CaptureSequencer::notifyError(int32_t errorCode, const CaptureResultExtras& resultExtras) {
    ATRACE_CALL();
    bool jpegBufferLost = false;
    if (errorCode == hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_BUFFER) {
        sp<Camera2Client> client = mClient.promote();
        if (client == nullptr) {
            return;
        }
        int captureStreamId = client->getCaptureStreamId();
        if (captureStreamId == resultExtras.errorStreamId) {
            jpegBufferLost = true;
        }
    } else if (errorCode ==
            hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_REQUEST) {
        if (resultExtras.requestId == mShutterCaptureId) {
            jpegBufferLost = true;
        }
    }

    if (jpegBufferLost) {
        sp<MemoryBase> emptyBuffer;
        onCaptureAvailable(/*timestamp*/0, emptyBuffer, /*captureError*/true);
    }
}

void CaptureSequencer::onResultAvailable(const CaptureResult &result) {
    ATRACE_CALL();
    ALOGV("%s: New result available.", __FUNCTION__);
    Mutex::Autolock l(mInputMutex);
    mNewFrameId = result.mResultExtras.requestId;
    mNewFrame = result.mMetadata;
    if (!mNewFrameReceived) {
        mNewFrameReceived = true;
        mNewFrameSignal.signal();
    }
}

void CaptureSequencer::onCaptureAvailable(nsecs_t timestamp,
        const sp<MemoryBase>& captureBuffer, bool captureError) {
    ATRACE_CALL();
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock l(mInputMutex);
    mCaptureTimestamp = timestamp;
    mCaptureBuffer = captureBuffer;
    if (!mNewCaptureReceived) {
        mNewCaptureReceived = true;
        if (captureError) {
            mNewCaptureErrorCnt++;
        } else {
            mNewCaptureErrorCnt = 0;
        }
        mNewCaptureSignal.signal();
    }
}


void CaptureSequencer::dump(int fd, const Vector<String16>& /*args*/) {
    String8 result;
    if (mCaptureRequest.entryCount() != 0) {
        result = "    Capture request:\n";
        write(fd, result.string(), result.size());
        mCaptureRequest.dump(fd, 2, 6);
    } else {
        result = "    Capture request: undefined\n";
        write(fd, result.string(), result.size());
    }
    result = String8::format("    Current capture state: %s\n",
            kStateNames[mCaptureState]);
    result.append("    Latest captured frame:\n");
    write(fd, result.string(), result.size());
    mNewFrame.dump(fd, 2, 6);
}

/** Private members */

const char* CaptureSequencer::kStateNames[CaptureSequencer::NUM_CAPTURE_STATES+1] =
{
    "IDLE",
    "START",
    "ZSL_START",
    "ZSL_WAITING",
    "ZSL_REPROCESSING",
    "STANDARD_START",
    "STANDARD_PRECAPTURE_WAIT",
    "STANDARD_CAPTURE",
    "STANDARD_CAPTURE_WAIT",
    "DONE",
    "ERROR",
    "UNKNOWN"
};

const CaptureSequencer::StateManager
        CaptureSequencer::kStateManagers[CaptureSequencer::NUM_CAPTURE_STATES-1] = {
    &CaptureSequencer::manageIdle,
    &CaptureSequencer::manageStart,
    &CaptureSequencer::manageZslStart,
    &CaptureSequencer::manageZslWaiting,
    &CaptureSequencer::manageZslReprocessing,
    &CaptureSequencer::manageStandardStart,
    &CaptureSequencer::manageStandardPrecaptureWait,
    &CaptureSequencer::manageStandardCapture,
    &CaptureSequencer::manageStandardCaptureWait,
    &CaptureSequencer::manageDone,
};

bool CaptureSequencer::threadLoop() {

    sp<Camera2Client> client = mClient.promote();
    if (client == 0) return false;

    CaptureState currentState;
    {
        Mutex::Autolock l(mStateMutex);
        currentState = mCaptureState;
    }

    currentState = (this->*kStateManagers[currentState])(client);

    Mutex::Autolock l(mStateMutex);
    if (currentState != mCaptureState) {
        if (mCaptureState != IDLE) {
            ATRACE_ASYNC_END(kStateNames[mCaptureState], mStateTransitionCount);
        }
        mCaptureState = currentState;
        mStateTransitionCount++;
        if (mCaptureState != IDLE) {
            ATRACE_ASYNC_BEGIN(kStateNames[mCaptureState], mStateTransitionCount);
        }
        ALOGV("Camera %d: New capture state %s",
                client->getCameraId(), kStateNames[mCaptureState]);
        mStateChanged.signal();
    }

    if (mCaptureState == ERROR) {
        ALOGE("Camera %d: Stopping capture sequencer due to error",
                client->getCameraId());
        return false;
    }

    return true;
}

CaptureSequencer::CaptureState CaptureSequencer::manageIdle(
        sp<Camera2Client> &/*client*/) {
    status_t res;
    Mutex::Autolock l(mInputMutex);
    while (!mStartCapture) {
        res = mStartCaptureSignal.waitRelative(mInputMutex,
                kWaitDuration);
        if (res == TIMED_OUT) break;
    }
    if (mStartCapture) {
        mStartCapture = false;
        mBusy = true;
        return START;
    }
    return IDLE;
}

CaptureSequencer::CaptureState CaptureSequencer::manageDone(sp<Camera2Client> &client) {
    status_t res = OK;
    ATRACE_CALL();
    mCaptureId++;
    if (mCaptureId >= Camera2Client::kCaptureRequestIdEnd) {
        mCaptureId = Camera2Client::kCaptureRequestIdStart;
    }
    {
        Mutex::Autolock l(mInputMutex);
        mBusy = false;
    }

    int takePictureCounter = 0;
    {
        SharedParameters::Lock l(client->getParameters());
        switch (l.mParameters.state) {
            case Parameters::DISCONNECTED:
                ALOGW("%s: Camera %d: Discarding image data during shutdown ",
                        __FUNCTION__, client->getCameraId());
                res = INVALID_OPERATION;
                break;
            case Parameters::STILL_CAPTURE:
                res = client->getCameraDevice()->waitUntilDrained();
                if (res != OK) {
                    ALOGE("%s: Camera %d: Can't idle after still capture: "
                            "%s (%d)", __FUNCTION__, client->getCameraId(),
                            strerror(-res), res);
                }
                l.mParameters.state = Parameters::STOPPED;
                break;
            case Parameters::VIDEO_SNAPSHOT:
                l.mParameters.state = Parameters::RECORD;
                break;
            default:
                ALOGE("%s: Camera %d: Still image produced unexpectedly "
                        "in state %s!",
                        __FUNCTION__, client->getCameraId(),
                        Parameters::getStateName(l.mParameters.state));
                res = INVALID_OPERATION;
        }
        takePictureCounter = l.mParameters.takePictureCounter;
    }
    sp<ZslProcessor> processor = mZslProcessor.promote();
    if (processor != 0) {
        ALOGV("%s: Memory optimization, clearing ZSL queue",
              __FUNCTION__);
        processor->clearZslQueue();
    }

    /**
     * Fire the jpegCallback in Camera#takePicture(..., jpegCallback)
     */
    if (mCaptureBuffer != 0 && res == OK) {
        ATRACE_ASYNC_END(Camera2Client::kTakepictureLabel, takePictureCounter);

        Camera2Client::SharedCameraCallbacks::Lock
            l(client->mSharedCameraCallbacks);
        ALOGV("%s: Sending still image to client", __FUNCTION__);
        if (l.mRemoteCallback != 0) {
            l.mRemoteCallback->dataCallback(CAMERA_MSG_COMPRESSED_IMAGE,
                    mCaptureBuffer, NULL);
        } else {
            ALOGV("%s: No client!", __FUNCTION__);
        }
    }
    mCaptureBuffer.clear();

    return IDLE;
}

CaptureSequencer::CaptureState CaptureSequencer::manageStart(
        sp<Camera2Client> &client) {
    ALOGV("%s", __FUNCTION__);
    status_t res;
    ATRACE_CALL();
    SharedParameters::Lock l(client->getParameters());
    CaptureState nextState = DONE;

    res = updateCaptureRequest(l.mParameters, client);
    if (res != OK ) {
        ALOGE("%s: Camera %d: Can't update still image capture request: %s (%d)",
                __FUNCTION__, client->getCameraId(), strerror(-res), res);
        return DONE;
    }

    else if (l.mParameters.useZeroShutterLag() &&
            l.mParameters.state == Parameters::STILL_CAPTURE &&
            l.mParameters.flashMode != Parameters::FLASH_MODE_ON) {
        nextState = ZSL_START;
    } else {
        nextState = STANDARD_START;
    }
    {
        Mutex::Autolock l(mInputMutex);
        mShutterCaptureId = mCaptureId;
        mHalNotifiedShutter = false;
    }
    mShutterNotified = false;

    return nextState;
}

CaptureSequencer::CaptureState CaptureSequencer::manageZslStart(
        sp<Camera2Client> &client) {
    ALOGV("%s", __FUNCTION__);
    status_t res;
    sp<ZslProcessor> processor = mZslProcessor.promote();
    if (processor == 0) {
        ALOGE("%s: No ZSL queue to use!", __FUNCTION__);
        return DONE;
    }

    // We don't want to get partial results for ZSL capture.
    client->registerFrameListener(mCaptureId, mCaptureId + 1,
            this,
            /*sendPartials*/false);

    // TODO: Actually select the right thing here.
    res = processor->pushToReprocess(mCaptureId);
    if (res != OK) {
        if (res == NOT_ENOUGH_DATA) {
            ALOGV("%s: Camera %d: ZSL queue doesn't have good frame, "
                    "falling back to normal capture", __FUNCTION__,
                    client->getCameraId());
        } else {
            ALOGE("%s: Camera %d: Error in ZSL queue: %s (%d)",
                    __FUNCTION__, client->getCameraId(), strerror(-res), res);
        }
        return STANDARD_START;
    }

    SharedParameters::Lock l(client->getParameters());
    /* warning: this also locks a SharedCameraCallbacks */
    shutterNotifyLocked(l.mParameters, client);
    mShutterNotified = true;
    mTimeoutCount = kMaxTimeoutsForCaptureEnd;
    return STANDARD_CAPTURE_WAIT;
}

CaptureSequencer::CaptureState CaptureSequencer::manageZslWaiting(
        sp<Camera2Client> &/*client*/) {
    ALOGV("%s", __FUNCTION__);
    return DONE;
}

CaptureSequencer::CaptureState CaptureSequencer::manageZslReprocessing(
        sp<Camera2Client> &/*client*/) {
    ALOGV("%s", __FUNCTION__);
    return START;
}

CaptureSequencer::CaptureState CaptureSequencer::manageStandardStart(
        sp<Camera2Client> &client) {
    ATRACE_CALL();

    bool isAeConverged = false;
    // Get the onFrameAvailable callback when the requestID == mCaptureId
    // We don't want to get partial results for normal capture, as we need
    // Get ANDROID_SENSOR_TIMESTAMP from the capture result, but partial
    // result doesn't have to have this metadata available.
    // TODO: Update to use the HALv3 shutter notification for remove the
    // need for this listener and make it faster. see bug 12530628.
    client->registerFrameListener(mCaptureId, mCaptureId + 1,
            this,
            /*sendPartials*/false);

    {
        Mutex::Autolock l(mInputMutex);
        isAeConverged = (mAEState == ANDROID_CONTROL_AE_STATE_CONVERGED);
    }

    {
        SharedParameters::Lock l(client->getParameters());
        // Skip AE precapture when it is already converged and not in force flash mode.
        if (l.mParameters.flashMode != Parameters::FLASH_MODE_ON && isAeConverged) {
            return STANDARD_CAPTURE;
        }

        mTriggerId = l.mParameters.precaptureTriggerCounter++;
    }
    client->getCameraDevice()->triggerPrecaptureMetering(mTriggerId);

    mAeInPrecapture = false;
    mTimeoutCount = kMaxTimeoutsForPrecaptureStart;
    return STANDARD_PRECAPTURE_WAIT;
}

CaptureSequencer::CaptureState CaptureSequencer::manageStandardPrecaptureWait(
        sp<Camera2Client> &/*client*/) {
    status_t res;
    ATRACE_CALL();
    Mutex::Autolock l(mInputMutex);
    while (!mNewAEState) {
        res = mNewNotifySignal.waitRelative(mInputMutex, kWaitDuration);
        if (res == TIMED_OUT) {
            mTimeoutCount--;
            break;
        }
    }
    if (mTimeoutCount <= 0) {
        ALOGW("Timed out waiting for precapture %s",
                mAeInPrecapture ? "end" : "start");
        return STANDARD_CAPTURE;
    }
    if (mNewAEState) {
        if (!mAeInPrecapture) {
            // Waiting to see PRECAPTURE state
            if (mAETriggerId == mTriggerId) {
                if (mAEState == ANDROID_CONTROL_AE_STATE_PRECAPTURE) {
                    ALOGV("%s: Got precapture start", __FUNCTION__);
                    mAeInPrecapture = true;
                    mTimeoutCount = kMaxTimeoutsForPrecaptureEnd;
                } else if (mAEState == ANDROID_CONTROL_AE_STATE_CONVERGED ||
                        mAEState == ANDROID_CONTROL_AE_STATE_FLASH_REQUIRED) {
                    // It is legal to transit to CONVERGED or FLASH_REQUIRED
                    // directly after a trigger.
                    ALOGV("%s: AE is already in good state, start capture", __FUNCTION__);
                    return STANDARD_CAPTURE;
                }
            }
        } else {
            // Waiting to see PRECAPTURE state end
            if (mAETriggerId == mTriggerId &&
                    mAEState != ANDROID_CONTROL_AE_STATE_PRECAPTURE) {
                ALOGV("%s: Got precapture end", __FUNCTION__);
                return STANDARD_CAPTURE;
            }
        }
        mNewAEState = false;
    }
    return STANDARD_PRECAPTURE_WAIT;
}

CaptureSequencer::CaptureState CaptureSequencer::manageStandardCapture(
        sp<Camera2Client> &client) {
    status_t res;
    ATRACE_CALL();
    SharedParameters::Lock l(client->getParameters());
    Vector<int32_t> outputStreams;

    /**
     * Set up output streams in the request
     *  - preview
     *  - capture/jpeg
     *  - callback (if preview callbacks enabled)
     *  - recording (if recording enabled)
     */
    outputStreams.push(client->getPreviewStreamId());

    int captureStreamId = client->getCaptureStreamId();
    if (captureStreamId == Camera2Client::NO_STREAM) {
        res = client->createJpegStreamL(l.mParameters);
        if (res != OK || client->getCaptureStreamId() == Camera2Client::NO_STREAM) {
            ALOGE("%s: Camera %d: cannot create jpeg stream for slowJpeg mode: %s (%d)",
                  __FUNCTION__, client->getCameraId(), strerror(-res), res);
            return DONE;
        }
    }

    outputStreams.push(client->getCaptureStreamId());

    if (l.mParameters.previewCallbackFlags &
            CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK) {
        outputStreams.push(client->getCallbackStreamId());
    }

    if (l.mParameters.state == Parameters::VIDEO_SNAPSHOT) {
        outputStreams.push(client->getRecordingStreamId());
    }

    res = mCaptureRequest.update(ANDROID_REQUEST_OUTPUT_STREAMS,
            outputStreams);
    if (res == OK) {
        res = mCaptureRequest.update(ANDROID_REQUEST_ID,
                &mCaptureId, 1);
    }
    if (res == OK) {
        res = mCaptureRequest.sort();
    }

    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to set up still capture request: %s (%d)",
                __FUNCTION__, client->getCameraId(), strerror(-res), res);
        return DONE;
    }

    // Create a capture copy since CameraDeviceBase#capture takes ownership
    CameraMetadata captureCopy = mCaptureRequest;
    if (captureCopy.entryCount() == 0) {
        ALOGE("%s: Camera %d: Unable to copy capture request for HAL device",
                __FUNCTION__, client->getCameraId());
        return DONE;
    }

    if ((l.mParameters.isDeviceZslSupported) && (l.mParameters.state != Parameters::RECORD) &&
            (l.mParameters.state != Parameters::VIDEO_SNAPSHOT)) {
        // If device ZSL is supported, drop all pending preview buffers to reduce the chance of
        // rendering preview frames newer than the still frame.
        // Additionally, preview must not get interrupted during video recording.
        client->getCameraDevice()->dropStreamBuffers(true, client->getPreviewStreamId());
    }

    /**
     * Clear the streaming request for still-capture pictures
     *   (as opposed to i.e. video snapshots)
     */
    if (l.mParameters.state == Parameters::STILL_CAPTURE) {
        // API definition of takePicture() - stop preview before taking pic
        res = client->stopStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to stop preview for still capture: "
                    "%s (%d)",
                    __FUNCTION__, client->getCameraId(), strerror(-res), res);
            return DONE;
        }
    }

    // TODO: Capture should be atomic with setStreamingRequest here
    res = client->getCameraDevice()->capture(captureCopy);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to submit still image capture request: "
                "%s (%d)",
                __FUNCTION__, client->getCameraId(), strerror(-res), res);
        return DONE;
    }

    mTimeoutCount = kMaxTimeoutsForCaptureEnd;
    return STANDARD_CAPTURE_WAIT;
}

CaptureSequencer::CaptureState CaptureSequencer::manageStandardCaptureWait(
        sp<Camera2Client> &client) {
    status_t res;
    ATRACE_CALL();
    Mutex::Autolock l(mInputMutex);


    // Wait for shutter callback
    while (!mHalNotifiedShutter) {
        if (mTimeoutCount <= 0) {
            break;
        }
        res = mShutterNotifySignal.waitRelative(mInputMutex, kWaitDuration);
        if (res == TIMED_OUT) {
            mTimeoutCount--;
            return STANDARD_CAPTURE_WAIT;
        }
    }

    if (mHalNotifiedShutter) {
        if (!mShutterNotified) {
            SharedParameters::Lock l(client->getParameters());
            /* warning: this also locks a SharedCameraCallbacks */
            shutterNotifyLocked(l.mParameters, client);
            mShutterNotified = true;
        }
    } else if (mTimeoutCount <= 0) {
        ALOGW("Timed out waiting for shutter notification");
        return DONE;
    }

    // Wait for new metadata result (mNewFrame)
    while (!mNewFrameReceived) {
        res = mNewFrameSignal.waitRelative(mInputMutex, kWaitDuration);
        if (res == TIMED_OUT) {
            mTimeoutCount--;
            break;
        }
    }

    // Wait until jpeg was captured by JpegProcessor
    while (mNewFrameReceived && !mNewCaptureReceived) {
        res = mNewCaptureSignal.waitRelative(mInputMutex, kWaitDuration);
        if (res == TIMED_OUT) {
            mTimeoutCount--;
            break;
        }
    }
    if (mNewCaptureReceived) {
        if (mNewCaptureErrorCnt > kMaxRetryCount) {
            ALOGW("Exceeding multiple retry limit of %d due to buffer drop", kMaxRetryCount);
            return DONE;
        } else if (mNewCaptureErrorCnt > 0) {
            ALOGW("Capture error happened, retry %d...", mNewCaptureErrorCnt);
            mNewCaptureReceived = false;
            return STANDARD_CAPTURE;
        }
    }

    if (mTimeoutCount <= 0) {
        ALOGW("Timed out waiting for capture to complete");
        return DONE;
    }
    if (mNewFrameReceived && mNewCaptureReceived) {

        if (mNewFrameId != mCaptureId) {
            ALOGW("Mismatched capture frame IDs: Expected %d, got %d",
                    mCaptureId, mNewFrameId);
        }
        camera_metadata_entry_t entry;
        entry = mNewFrame.find(ANDROID_SENSOR_TIMESTAMP);
        if (entry.count == 0) {
            ALOGE("No timestamp field in capture frame!");
        } else if (entry.count == 1) {
            if (entry.data.i64[0] != mCaptureTimestamp) {
                ALOGW("Mismatched capture timestamps: Metadata frame %" PRId64 ","
                        " captured buffer %" PRId64,
                        entry.data.i64[0],
                        mCaptureTimestamp);
            }
        } else {
            ALOGE("Timestamp metadata is malformed!");
        }
        client->removeFrameListener(mCaptureId, mCaptureId + 1, this);

        mNewFrameReceived = false;
        mNewCaptureReceived = false;
        return DONE;
    }
    return STANDARD_CAPTURE_WAIT;
}

status_t CaptureSequencer::updateCaptureRequest(const Parameters &params,
        sp<Camera2Client> &client) {
    ATRACE_CALL();
    status_t res;
    uint8_t captureIntent = static_cast<uint8_t>(ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE);

    if (mCaptureRequest.entryCount() == 0) {
        res = client->getCameraDevice()->createDefaultRequest(
                CAMERA2_TEMPLATE_STILL_CAPTURE,
                &mCaptureRequest);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to create default still image request:"
                    " %s (%d)", __FUNCTION__, client->getCameraId(),
                    strerror(-res), res);
            return res;
        }
    }

    if (params.state == Parameters::VIDEO_SNAPSHOT) {
        captureIntent = static_cast<uint8_t>(ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT);
    }
    res = mCaptureRequest.update(ANDROID_CONTROL_CAPTURE_INTENT, &captureIntent, 1);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update capture intent: %s (%d)",
                __FUNCTION__, client->getCameraId(), strerror(-res), res);
        return res;
    }

    res = params.updateRequest(&mCaptureRequest);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update common entries of capture "
                "request: %s (%d)", __FUNCTION__, client->getCameraId(),
                strerror(-res), res);
        return res;
    }

    res = params.updateRequestJpeg(&mCaptureRequest);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update JPEG entries of capture "
                "request: %s (%d)", __FUNCTION__, client->getCameraId(),
                strerror(-res), res);
        return res;
    }

    return OK;
}

/*static*/ void CaptureSequencer::shutterNotifyLocked(const Parameters &params,
            const sp<Camera2Client>& client) {
    ATRACE_CALL();

    if (params.state == Parameters::STILL_CAPTURE
        && params.playShutterSound) {
        client->getCameraService()->playSound(CameraService::SOUND_SHUTTER);
    }

    {
        Camera2Client::SharedCameraCallbacks::Lock
            l(client->mSharedCameraCallbacks);

        ALOGV("%s: Notifying of shutter close to client", __FUNCTION__);
        if (l.mRemoteCallback != 0) {
            // ShutterCallback
            l.mRemoteCallback->notifyCallback(CAMERA_MSG_SHUTTER,
                                            /*ext1*/0, /*ext2*/0);

            // RawCallback with null buffer
            l.mRemoteCallback->notifyCallback(CAMERA_MSG_RAW_IMAGE_NOTIFY,
                                            /*ext1*/0, /*ext2*/0);
        } else {
            ALOGV("%s: No client!", __FUNCTION__);
        }
    }
}


}; // namespace camera2
}; // namespace android
