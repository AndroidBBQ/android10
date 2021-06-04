/*
 * Copyright (C) 2012-2018 The Android Open Source Project
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

#define LOG_TAG "Camera2-StreamingProcessor"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0
//#define LOG_NNDEBUG 0 // Per-frame verbose logging

#ifdef LOG_NNDEBUG
#define ALOGVV(...) ALOGV(__VA_ARGS__)
#else
#define ALOGVV(...) ((void)0)
#endif

#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <gui/BufferItem.h>
#include <gui/Surface.h>
#include <media/hardware/HardwareAPI.h>

#include "common/CameraDeviceBase.h"
#include "api1/Camera2Client.h"
#include "api1/client2/StreamingProcessor.h"
#include "api1/client2/Camera2Heap.h"

namespace android {
namespace camera2 {

StreamingProcessor::StreamingProcessor(sp<Camera2Client> client):
        mClient(client),
        mDevice(client->getCameraDevice()),
        mId(client->getCameraId()),
        mActiveRequest(NONE),
        mPaused(false),
        mPreviewRequestId(Camera2Client::kPreviewRequestIdStart),
        mPreviewStreamId(NO_STREAM),
        mRecordingRequestId(Camera2Client::kRecordingRequestIdStart),
        mRecordingStreamId(NO_STREAM)
{
}

StreamingProcessor::~StreamingProcessor() {
    deletePreviewStream();
    deleteRecordingStream();
}

status_t StreamingProcessor::setPreviewWindow(const sp<Surface>& window) {
    ATRACE_CALL();
    status_t res;

    res = deletePreviewStream();
    if (res != OK) return res;

    Mutex::Autolock m(mMutex);

    mPreviewWindow = window;

    return OK;
}

status_t StreamingProcessor::setRecordingWindow(const sp<Surface>& window) {
    ATRACE_CALL();
    status_t res;

    res = deleteRecordingStream();
    if (res != OK) return res;

    Mutex::Autolock m(mMutex);

    mRecordingWindow = window;

    return OK;
}

bool StreamingProcessor::haveValidPreviewWindow() const {
    Mutex::Autolock m(mMutex);
    return mPreviewWindow != 0;
}

bool StreamingProcessor::haveValidRecordingWindow() const {
    Mutex::Autolock m(mMutex);
    return mRecordingWindow != nullptr;
}

status_t StreamingProcessor::updatePreviewRequest(const Parameters &params) {
    ATRACE_CALL();
    status_t res;
    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    Mutex::Autolock m(mMutex);
    if (mPreviewRequest.entryCount() == 0) {
        sp<Camera2Client> client = mClient.promote();
        if (client == 0) {
            ALOGE("%s: Camera %d: Client does not exist", __FUNCTION__, mId);
            return INVALID_OPERATION;
        }

        // Use CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG for ZSL streaming case.
        if (params.useZeroShutterLag() && !params.recordingHint) {
            res = device->createDefaultRequest(
                    CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG, &mPreviewRequest);
        } else {
            res = device->createDefaultRequest(CAMERA3_TEMPLATE_PREVIEW,
                    &mPreviewRequest);
        }

        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to create default preview request: "
                    "%s (%d)", __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
    }

    res = params.updateRequest(&mPreviewRequest);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update common entries of preview "
                "request: %s (%d)", __FUNCTION__, mId,
                strerror(-res), res);
        return res;
    }

    res = mPreviewRequest.update(ANDROID_REQUEST_ID,
            &mPreviewRequestId, 1);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update request id for preview: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    return OK;
}

status_t StreamingProcessor::updatePreviewStream(const Parameters &params) {
    ATRACE_CALL();
    Mutex::Autolock m(mMutex);

    status_t res;
    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mPreviewStreamId != NO_STREAM) {
        // Check if stream parameters have to change
        CameraDeviceBase::StreamInfo streamInfo;
        res = device->getStreamInfo(mPreviewStreamId, &streamInfo);
        if (res != OK) {
            ALOGE("%s: Camera %d: Error querying preview stream info: "
                    "%s (%d)", __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
        if (streamInfo.width != (uint32_t)params.previewWidth ||
                streamInfo.height != (uint32_t)params.previewHeight) {
            ALOGV("%s: Camera %d: Preview size switch: %d x %d -> %d x %d",
                    __FUNCTION__, mId, streamInfo.width, streamInfo.height,
                    params.previewWidth, params.previewHeight);
            res = device->waitUntilDrained();
            if (res != OK) {
                ALOGE("%s: Camera %d: Error waiting for preview to drain: "
                        "%s (%d)", __FUNCTION__, mId, strerror(-res), res);
                return res;
            }
            res = device->deleteStream(mPreviewStreamId);
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to delete old output stream "
                        "for preview: %s (%d)", __FUNCTION__, mId,
                        strerror(-res), res);
                return res;
            }
            mPreviewStreamId = NO_STREAM;
        }
    }

    if (mPreviewStreamId == NO_STREAM) {
        res = device->createStream(mPreviewWindow,
                params.previewWidth, params.previewHeight,
                CAMERA2_HAL_PIXEL_FORMAT_OPAQUE, HAL_DATASPACE_UNKNOWN,
                CAMERA3_STREAM_ROTATION_0, &mPreviewStreamId, String8());
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to create preview stream: %s (%d)",
                    __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
    }

    res = device->setStreamTransform(mPreviewStreamId,
            params.previewTransform);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to set preview stream transform: "
                "%s (%d)", __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    return OK;
}

status_t StreamingProcessor::deletePreviewStream() {
    ATRACE_CALL();
    status_t res;

    Mutex::Autolock m(mMutex);

    if (mPreviewStreamId != NO_STREAM) {
        sp<CameraDeviceBase> device = mDevice.promote();
        if (device == 0) {
            ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
            return INVALID_OPERATION;
        }

        ALOGV("%s: for cameraId %d on streamId %d",
            __FUNCTION__, mId, mPreviewStreamId);

        res = device->waitUntilDrained();
        if (res != OK) {
            ALOGE("%s: Error waiting for preview to drain: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
        res = device->deleteStream(mPreviewStreamId);
        if (res != OK) {
            ALOGE("%s: Unable to delete old preview stream: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
        mPreviewStreamId = NO_STREAM;
    }
    return OK;
}

int StreamingProcessor::getPreviewStreamId() const {
    Mutex::Autolock m(mMutex);
    return mPreviewStreamId;
}

status_t StreamingProcessor::updateRecordingRequest(const Parameters &params) {
    ATRACE_CALL();
    status_t res;
    Mutex::Autolock m(mMutex);

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mRecordingRequest.entryCount() == 0) {
        res = device->createDefaultRequest(CAMERA2_TEMPLATE_VIDEO_RECORD,
                &mRecordingRequest);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to create default recording request:"
                    " %s (%d)", __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
    }

    res = params.updateRequest(&mRecordingRequest);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update common entries of recording "
                "request: %s (%d)", __FUNCTION__, mId,
                strerror(-res), res);
        return res;
    }

    res = mRecordingRequest.update(ANDROID_REQUEST_ID,
            &mRecordingRequestId, 1);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update request id for request: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    return OK;
}

status_t StreamingProcessor::recordingStreamNeedsUpdate(
        const Parameters &params, bool *needsUpdate) {
    status_t res;

    if (needsUpdate == 0) {
        ALOGE("%s: Camera %d: invalid argument", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mRecordingStreamId == NO_STREAM) {
        *needsUpdate = true;
        return OK;
    }

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    CameraDeviceBase::StreamInfo streamInfo;
    res = device->getStreamInfo(mRecordingStreamId, &streamInfo);
    if (res != OK) {
        ALOGE("%s: Camera %d: Error querying recording output stream info: "
                "%s (%d)", __FUNCTION__, mId,
                strerror(-res), res);
        return res;
    }

    if (mRecordingWindow == nullptr ||
            streamInfo.width != (uint32_t)params.videoWidth ||
            streamInfo.height != (uint32_t)params.videoHeight ||
            !streamInfo.matchFormat((uint32_t)params.videoFormat) ||
            !streamInfo.matchDataSpace(params.videoDataSpace)) {
        *needsUpdate = true;
        return res;
    }
    *needsUpdate = false;
    return res;
}

status_t StreamingProcessor::updateRecordingStream(const Parameters &params) {
    ATRACE_CALL();
    status_t res;
    Mutex::Autolock m(mMutex);

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mRecordingStreamId != NO_STREAM) {
        // Check if stream parameters have to change
        CameraDeviceBase::StreamInfo streamInfo;
        res = device->getStreamInfo(mRecordingStreamId, &streamInfo);
        if (res != OK) {
            ALOGE("%s: Camera %d: Error querying recording output stream info: "
                    "%s (%d)", __FUNCTION__, mId,
                    strerror(-res), res);
            return res;
        }
        if (streamInfo.width != (uint32_t)params.videoWidth ||
                streamInfo.height != (uint32_t)params.videoHeight ||
                !streamInfo.matchFormat((uint32_t)params.videoFormat) ||
                !streamInfo.matchDataSpace(params.videoDataSpace)) {
            // TODO: Should wait to be sure previous recording has finished
            res = device->deleteStream(mRecordingStreamId);

            if (res == -EBUSY) {
                ALOGV("%s: Camera %d: Device is busy, call "
                      "updateRecordingStream after it becomes idle",
                      __FUNCTION__, mId);
                return res;
            } else if (res != OK) {
                ALOGE("%s: Camera %d: Unable to delete old output stream "
                        "for recording: %s (%d)", __FUNCTION__,
                        mId, strerror(-res), res);
                return res;
            }
            mRecordingStreamId = NO_STREAM;
        }
    }

    if (mRecordingStreamId == NO_STREAM) {
        res = device->createStream(mRecordingWindow,
                params.videoWidth, params.videoHeight,
                params.videoFormat, params.videoDataSpace,
                CAMERA3_STREAM_ROTATION_0, &mRecordingStreamId,
                String8());
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't create output stream for recording: "
                    "%s (%d)", __FUNCTION__, mId,
                    strerror(-res), res);
            return res;
        }
    }

    return OK;
}

status_t StreamingProcessor::deleteRecordingStream() {
    ATRACE_CALL();
    status_t res;

    Mutex::Autolock m(mMutex);

    if (mRecordingStreamId != NO_STREAM) {
        sp<CameraDeviceBase> device = mDevice.promote();
        if (device == 0) {
            ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
            return INVALID_OPERATION;
        }

        res = device->waitUntilDrained();
        if (res != OK) {
            ALOGE("%s: Error waiting for HAL to drain: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
        res = device->deleteStream(mRecordingStreamId);
        if (res != OK) {
            ALOGE("%s: Unable to delete recording stream: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
        mRecordingStreamId = NO_STREAM;
    }
    return OK;
}

int StreamingProcessor::getRecordingStreamId() const {
    return mRecordingStreamId;
}

status_t StreamingProcessor::startStream(StreamType type,
        const Vector<int32_t> &outputStreams) {
    ATRACE_CALL();
    status_t res;

    if (type == NONE) return INVALID_OPERATION;

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    ALOGV("%s: Camera %d: type = %d", __FUNCTION__, mId, type);

    Mutex::Autolock m(mMutex);

    CameraMetadata &request = (type == PREVIEW) ?
            mPreviewRequest : mRecordingRequest;

    res = request.update(
        ANDROID_REQUEST_OUTPUT_STREAMS,
        outputStreams);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to set up preview request: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    res = request.sort();
    if (res != OK) {
        ALOGE("%s: Camera %d: Error sorting preview request: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    res = device->setStreamingRequest(request);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to set preview request to start preview: "
                "%s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }
    mActiveRequest = type;
    mPaused = false;
    mActiveStreamIds = outputStreams;
    return OK;
}

status_t StreamingProcessor::togglePauseStream(bool pause) {
    ATRACE_CALL();
    status_t res;

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    ALOGV("%s: Camera %d: toggling pause to %d", __FUNCTION__, mId, pause);

    Mutex::Autolock m(mMutex);

    if (mActiveRequest == NONE) {
        ALOGE("%s: Camera %d: Can't toggle pause, streaming was not started",
              __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mPaused == pause) {
        return OK;
    }

    if (pause) {
        res = device->clearStreamingRequest();
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't clear stream request: %s (%d)",
                    __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
    } else {
        CameraMetadata &request =
                (mActiveRequest == PREVIEW) ? mPreviewRequest
                                            : mRecordingRequest;
        res = device->setStreamingRequest(request);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to set preview request to resume: "
                    "%s (%d)",
                    __FUNCTION__, mId, strerror(-res), res);
            return res;
        }
    }

    mPaused = pause;
    return OK;
}

status_t StreamingProcessor::stopStream() {
    ATRACE_CALL();
    status_t res;

    Mutex::Autolock m(mMutex);

    sp<CameraDeviceBase> device = mDevice.promote();
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    res = device->clearStreamingRequest();
    if (res != OK) {
        ALOGE("%s: Camera %d: Can't clear stream request: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    mActiveRequest = NONE;
    mActiveStreamIds.clear();
    mPaused = false;

    return OK;
}

int32_t StreamingProcessor::getActiveRequestId() const {
    Mutex::Autolock m(mMutex);
    switch (mActiveRequest) {
        case NONE:
            return 0;
        case PREVIEW:
            return mPreviewRequestId;
        case RECORD:
            return mRecordingRequestId;
        default:
            ALOGE("%s: Unexpected mode %d", __FUNCTION__, mActiveRequest);
            return 0;
    }
}

status_t StreamingProcessor::incrementStreamingIds() {
    ATRACE_CALL();
    Mutex::Autolock m(mMutex);

    mPreviewRequestId++;
    if (mPreviewRequestId >= Camera2Client::kPreviewRequestIdEnd) {
        mPreviewRequestId = Camera2Client::kPreviewRequestIdStart;
    }
    mRecordingRequestId++;
    if (mRecordingRequestId >= Camera2Client::kRecordingRequestIdEnd) {
        mRecordingRequestId = Camera2Client::kRecordingRequestIdStart;
    }
    return OK;
}

status_t StreamingProcessor::dump(int fd, const Vector<String16>& /*args*/) {
    String8 result;

    result.append("  Current requests:\n");
    if (mPreviewRequest.entryCount() != 0) {
        result.append("    Preview request:\n");
        write(fd, result.string(), result.size());
        mPreviewRequest.dump(fd, 2, 6);
        result.clear();
    } else {
        result.append("    Preview request: undefined\n");
    }

    if (mRecordingRequest.entryCount() != 0) {
        result = "    Recording request:\n";
        write(fd, result.string(), result.size());
        mRecordingRequest.dump(fd, 2, 6);
        result.clear();
    } else {
        result = "    Recording request: undefined\n";
    }

    const char* streamTypeString[] = {
        "none", "preview", "record"
    };
    result.append(String8::format("   Active request: %s (paused: %s)\n",
                                  streamTypeString[mActiveRequest],
                                  mPaused ? "yes" : "no"));

    write(fd, result.string(), result.size());

    return OK;
}

}; // namespace camera2
}; // namespace android
