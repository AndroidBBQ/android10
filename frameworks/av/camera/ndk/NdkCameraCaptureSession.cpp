/*
 * Copyright (C) 2015 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkCameraCaptureSession"
#define ATRACE_TAG ATRACE_TAG_CAMERA

#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/Trace.h>

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCaptureRequest.h>
#include <camera/NdkCameraCaptureSession.h>
#include "impl/ACameraCaptureSession.h"

#include "impl/ACameraCaptureSession.inc"

using namespace android;

EXPORT
void ACameraCaptureSession_close(ACameraCaptureSession* session) {
    ATRACE_CALL();
    if (session != nullptr) {
        session->closeByApp();
    }
    return;
}

EXPORT
camera_status_t ACameraCaptureSession_getDevice(
        ACameraCaptureSession* session, ACameraDevice **device) {
    ATRACE_CALL();
    if (session == nullptr || device == nullptr) {
        ALOGE("%s: Error: invalid input: session %p, device %p",
                __FUNCTION__, session, device);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        *device = nullptr;
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    *device = session->getDevice();
    if (*device == nullptr) {
        // Should not reach here
        ALOGE("%s: unknown failure: device is null", __FUNCTION__);
        return ACAMERA_ERROR_UNKNOWN;
    }
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraCaptureSession_capture(
        ACameraCaptureSession* session, /*optional*/ACameraCaptureSession_captureCallbacks* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    ATRACE_CALL();
    if (session == nullptr || requests == nullptr || numRequests < 1) {
        ALOGE("%s: Error: invalid input: session %p, numRequest %d, requests %p",
                __FUNCTION__, session, numRequests, requests);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        if (captureSequenceId != nullptr) {
            *captureSequenceId = CAPTURE_SEQUENCE_ID_NONE;
        }
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    return session->capture(
            cbs, numRequests, requests, captureSequenceId);
}

EXPORT
camera_status_t ACameraCaptureSession_logicalCamera_capture(
        ACameraCaptureSession* session,
        /*optional*/ACameraCaptureSession_logicalCamera_captureCallbacks* lcbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    ATRACE_CALL();
    if (session == nullptr || requests == nullptr || numRequests < 1) {
        ALOGE("%s: Error: invalid input: session %p, numRequest %d, requests %p",
                __FUNCTION__, session, numRequests, requests);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        if (captureSequenceId) {
            *captureSequenceId = CAPTURE_SEQUENCE_ID_NONE;
        }
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    return session->capture(
            lcbs, numRequests, requests, captureSequenceId);
}

EXPORT
camera_status_t ACameraCaptureSession_setRepeatingRequest(
        ACameraCaptureSession* session, /*optional*/ACameraCaptureSession_captureCallbacks* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    ATRACE_CALL();
    if (session == nullptr || requests == nullptr || numRequests < 1) {
        ALOGE("%s: Error: invalid input: session %p, numRequest %d, requests %p",
                __FUNCTION__, session, numRequests, requests);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        if (captureSequenceId) {
            *captureSequenceId = CAPTURE_SEQUENCE_ID_NONE;
        }
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    return session->setRepeatingRequest(cbs, numRequests, requests, captureSequenceId);
}

EXPORT
camera_status_t ACameraCaptureSession_logicalCamera_setRepeatingRequest(
        ACameraCaptureSession* session,
        /*optional*/ACameraCaptureSession_logicalCamera_captureCallbacks* lcbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    ATRACE_CALL();
    if (session == nullptr || requests == nullptr || numRequests < 1) {
        ALOGE("%s: Error: invalid input: session %p, numRequest %d, requests %p",
                __FUNCTION__, session, numRequests, requests);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        if (captureSequenceId) {
            *captureSequenceId = CAPTURE_SEQUENCE_ID_NONE;
        }
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    return session->setRepeatingRequest(lcbs, numRequests, requests, captureSequenceId);
}

EXPORT
camera_status_t ACameraCaptureSession_stopRepeating(ACameraCaptureSession* session) {
    ATRACE_CALL();
    if (session == nullptr) {
        ALOGE("%s: Error: session is null", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        return ACAMERA_ERROR_SESSION_CLOSED;
    }
    return session->stopRepeating();
}

EXPORT
camera_status_t ACameraCaptureSession_abortCaptures(ACameraCaptureSession* session) {
    ATRACE_CALL();
    if (session == nullptr) {
        ALOGE("%s: Error: session is null", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        return ACAMERA_ERROR_SESSION_CLOSED;
    }
    return session->abortCaptures();
}

EXPORT
camera_status_t ACameraCaptureSession_updateSharedOutput(ACameraCaptureSession* session,
        ACaptureSessionOutput* output) {
    ATRACE_CALL();
    if (session == nullptr) {
        ALOGE("%s: Error: session is null", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (session->isClosed()) {
        ALOGE("%s: session %p is already closed", __FUNCTION__, session);
        return ACAMERA_ERROR_SESSION_CLOSED;
    }
    return session->updateOutputConfiguration(output);
}
