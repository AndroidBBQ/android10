/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_TAG "Camera2-FrameProcessorBase"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>

#include "common/FrameProcessorBase.h"
#include "common/CameraDeviceBase.h"

namespace android {
namespace camera2 {

FrameProcessorBase::FrameProcessorBase(wp<CameraDeviceBase> device) :
    Thread(/*canCallJava*/false),
    mDevice(device),
    mNumPartialResults(1) {
    sp<CameraDeviceBase> cameraDevice = device.promote();
    if (cameraDevice != 0) {
        CameraMetadata staticInfo = cameraDevice->info();
        camera_metadata_entry_t entry = staticInfo.find(ANDROID_REQUEST_PARTIAL_RESULT_COUNT);
        if (entry.count > 0) {
            mNumPartialResults = entry.data.i32[0];
        }
    }
}

FrameProcessorBase::~FrameProcessorBase() {
    ALOGV("%s: Exit", __FUNCTION__);
}

status_t FrameProcessorBase::registerListener(int32_t minId,
        int32_t maxId, const wp<FilteredListener>& listener, bool sendPartials) {
    Mutex::Autolock l(mInputMutex);
    List<RangeListener>::iterator item = mRangeListeners.begin();
    while (item != mRangeListeners.end()) {
        if (item->minId == minId &&
                item->maxId == maxId &&
                item->listener == listener) {
            // already registered, just return
            ALOGV("%s: Attempt to register the same client twice, ignoring",
                    __FUNCTION__);
            return OK;
        }
        item++;
    }
    ALOGV("%s: Registering listener for frame id range %d - %d",
            __FUNCTION__, minId, maxId);
    RangeListener rListener = { minId, maxId, listener, sendPartials };
    mRangeListeners.push_back(rListener);
    return OK;
}

status_t FrameProcessorBase::removeListener(int32_t minId,
                                           int32_t maxId,
                                           const wp<FilteredListener>& listener) {
    Mutex::Autolock l(mInputMutex);
    List<RangeListener>::iterator item = mRangeListeners.begin();
    while (item != mRangeListeners.end()) {
        if (item->minId == minId &&
                item->maxId == maxId &&
                item->listener == listener) {
            item = mRangeListeners.erase(item);
        } else {
            item++;
        }
    }
    return OK;
}

void FrameProcessorBase::dump(int fd, const Vector<String16>& /*args*/) {
    String8 result("    Latest received frame:\n");
    write(fd, result.string(), result.size());

    CameraMetadata lastFrame;
    std::map<std::string, CameraMetadata> lastPhysicalFrames;
    {
        // Don't race while dumping metadata
        Mutex::Autolock al(mLastFrameMutex);
        lastFrame = CameraMetadata(mLastFrame);

        for (const auto& physicalFrame : mLastPhysicalFrames) {
            lastPhysicalFrames.emplace(String8(physicalFrame.mPhysicalCameraId),
                    physicalFrame.mPhysicalCameraMetadata);
        }
    }
    lastFrame.dump(fd, /*verbosity*/2, /*indentation*/6);

    for (const auto& physicalFrame : lastPhysicalFrames) {
        result = String8::format("   Latest received frame for physical camera %s:\n",
                physicalFrame.first.c_str());
        write(fd, result.string(), result.size());
        CameraMetadata lastPhysicalMetadata = CameraMetadata(physicalFrame.second);
        lastPhysicalMetadata.sort();
        lastPhysicalMetadata.dump(fd, /*verbosity*/2, /*indentation*/6);
    }
}

bool FrameProcessorBase::threadLoop() {
    status_t res;

    sp<CameraDeviceBase> device;
    {
        device = mDevice.promote();
        if (device == 0) return false;
    }

    res = device->waitForNextFrame(kWaitDuration);
    if (res == OK) {
        processNewFrames(device);
    } else if (res != TIMED_OUT) {
        ALOGE("FrameProcessorBase: Error waiting for new "
                "frames: %s (%d)", strerror(-res), res);
    }

    return true;
}

void FrameProcessorBase::processNewFrames(const sp<CameraDeviceBase> &device) {
    status_t res;
    ATRACE_CALL();
    CaptureResult result;

    ALOGV("%s: Camera %s: Process new frames", __FUNCTION__, device->getId().string());

    while ( (res = device->getNextResult(&result)) == OK) {

        // TODO: instead of getting frame number from metadata, we should read
        // this from result.mResultExtras when CameraDeviceBase interface is fixed.
        camera_metadata_entry_t entry;

        entry = result.mMetadata.find(ANDROID_REQUEST_FRAME_COUNT);
        if (entry.count == 0) {
            ALOGE("%s: Camera %s: Error reading frame number",
                    __FUNCTION__, device->getId().string());
            break;
        }
        ATRACE_INT("cam2_frame", entry.data.i32[0]);

        if (!processSingleFrame(result, device)) {
            break;
        }

        if (!result.mMetadata.isEmpty()) {
            Mutex::Autolock al(mLastFrameMutex);
            mLastFrame.acquire(result.mMetadata);

            mLastPhysicalFrames = std::move(result.mPhysicalMetadatas);
        }
    }
    if (res != NOT_ENOUGH_DATA) {
        ALOGE("%s: Camera %s: Error getting next frame: %s (%d)",
                __FUNCTION__, device->getId().string(), strerror(-res), res);
        return;
    }

    return;
}

bool FrameProcessorBase::processSingleFrame(CaptureResult &result,
                                            const sp<CameraDeviceBase> &device) {
    ALOGV("%s: Camera %s: Process single frame (is empty? %d)",
            __FUNCTION__, device->getId().string(), result.mMetadata.isEmpty());
    return processListeners(result, device) == OK;
}

status_t FrameProcessorBase::processListeners(const CaptureResult &result,
        const sp<CameraDeviceBase> &device) {
    ATRACE_CALL();

    camera_metadata_ro_entry_t entry;

    // Check if this result is partial.
    bool isPartialResult =
            result.mResultExtras.partialResultCount < mNumPartialResults;

    // TODO: instead of getting requestID from CameraMetadata, we should get it
    // from CaptureResultExtras. This will require changing Camera2Device.
    // Currently Camera2Device uses MetadataQueue to store results, which does not
    // include CaptureResultExtras.
    entry = result.mMetadata.find(ANDROID_REQUEST_ID);
    if (entry.count == 0) {
        ALOGE("%s: Camera %s: Error reading frame id", __FUNCTION__, device->getId().string());
        return BAD_VALUE;
    }
    int32_t requestId = entry.data.i32[0];

    List<sp<FilteredListener> > listeners;
    {
        Mutex::Autolock l(mInputMutex);

        List<RangeListener>::iterator item = mRangeListeners.begin();
        // Don't deliver partial results to listeners that don't want them
        while (item != mRangeListeners.end()) {
            if (requestId >= item->minId && requestId < item->maxId &&
                    (!isPartialResult || item->sendPartials)) {
                sp<FilteredListener> listener = item->listener.promote();
                if (listener == 0) {
                    item = mRangeListeners.erase(item);
                    continue;
                } else {
                    listeners.push_back(listener);
                }
            }
            item++;
        }
    }
    ALOGV("%s: Camera %s: Got %zu range listeners out of %zu", __FUNCTION__,
          device->getId().string(), listeners.size(), mRangeListeners.size());

    List<sp<FilteredListener> >::iterator item = listeners.begin();
    for (; item != listeners.end(); item++) {
        (*item)->onResultAvailable(result);
    }
    return OK;
}

}; // namespace camera2
}; // namespace android
