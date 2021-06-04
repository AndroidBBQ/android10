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

#define LOG_TAG "ACameraVendorUtils"

#include <utils/Log.h>

#include "utils.h"

namespace android {
namespace acam {
namespace utils {

// Convert CaptureRequest wrappable by sp<> to hidl CaptureRequest.
frameworks::cameraservice::device::V2_0::CaptureRequest
convertToHidl(const CaptureRequest *captureRequest) {
    frameworks::cameraservice::device::V2_0::CaptureRequest hCaptureRequest;
    hCaptureRequest.physicalCameraSettings = captureRequest->mCaptureRequest.physicalCameraSettings;
    hCaptureRequest.streamAndWindowIds = captureRequest->mCaptureRequest.streamAndWindowIds;
    return hCaptureRequest;
}

HRotation convertToHidl(int rotation) {
    HRotation hRotation = HRotation::R0;
    switch(rotation) {
        case CAMERA3_STREAM_ROTATION_90:
            hRotation = HRotation::R90;
            break;
        case CAMERA3_STREAM_ROTATION_180:
            hRotation = HRotation::R180;
            break;
        case CAMERA3_STREAM_ROTATION_270:
            hRotation = HRotation::R270;
            break;
        default:
            break;
    }
    return hRotation;
}

bool convertFromHidlCloned(const HCameraMetadata &metadata, CameraMetadata *rawMetadata) {
    const camera_metadata *buffer = (camera_metadata_t*)(metadata.data());
    size_t expectedSize = metadata.size();
    int ret = validate_camera_metadata_structure(buffer, &expectedSize);
    if (ret == OK || ret == CAMERA_METADATA_VALIDATION_SHIFTED) {
        *rawMetadata = buffer;
    } else {
        ALOGE("%s: Malformed camera metadata received from caller", __FUNCTION__);
        return false;
    }
    return true;
}

// Note: existing data in dst will be gone. dst owns memory if shouldOwn is set
//       to true.
void convertToHidl(const camera_metadata_t *src, HCameraMetadata* dst, bool shouldOwn) {
    if (src == nullptr) {
        return;
    }
    size_t size = get_camera_metadata_size(src);
    dst->setToExternal((uint8_t *) src, size, shouldOwn);
    return;
}

TemplateId convertToHidl(ACameraDevice_request_template templateId) {
    switch(templateId) {
        case TEMPLATE_STILL_CAPTURE:
            return TemplateId::STILL_CAPTURE;
        case TEMPLATE_RECORD:
            return TemplateId::RECORD;
        case TEMPLATE_VIDEO_SNAPSHOT:
            return TemplateId::VIDEO_SNAPSHOT;
        case TEMPLATE_ZERO_SHUTTER_LAG:
            return TemplateId::ZERO_SHUTTER_LAG;
        case TEMPLATE_MANUAL:
            return TemplateId::MANUAL;
        default:
            return TemplateId::PREVIEW;
    }
}

camera_status_t convertFromHidl(Status status) {
    camera_status_t ret = ACAMERA_OK;
    switch(status) {
        case Status::NO_ERROR:
            break;
        case Status::DISCONNECTED:
            ret = ACAMERA_ERROR_CAMERA_DISCONNECTED;
            break;
        case Status::CAMERA_IN_USE:
            ret = ACAMERA_ERROR_CAMERA_IN_USE;
            break;
        case Status::MAX_CAMERAS_IN_USE:
            ret = ACAMERA_ERROR_MAX_CAMERA_IN_USE;
            break;
        case Status::ILLEGAL_ARGUMENT:
            ret = ACAMERA_ERROR_INVALID_PARAMETER;
            break;
        case Status::DEPRECATED_HAL:
            // Should not reach here since we filtered legacy HALs earlier
            ret = ACAMERA_ERROR_INVALID_PARAMETER;
            break;
        case Status::DISABLED:
            ret = ACAMERA_ERROR_CAMERA_DISABLED;
            break;
        case Status::PERMISSION_DENIED:
            ret = ACAMERA_ERROR_PERMISSION_DENIED;
            break;
        case Status::INVALID_OPERATION:
            ret = ACAMERA_ERROR_INVALID_OPERATION;
            break;
        default:
            ret = ACAMERA_ERROR_UNKNOWN;
            break;
    }
    return ret;
}

bool isWindowNativeHandleEqual(const native_handle_t *nh1, const native_handle_t *nh2) {
    if (nh1->numFds !=0 || nh2->numFds !=0) {
        ALOGE("Invalid window native handles being compared");
        return false;
    }
    if (nh1->version != nh2->version || nh1->numFds != nh2->numFds ||
        nh1->numInts != nh2->numInts) {
        return false;
    }
    for (int i = 0; i < nh1->numInts; i++) {
        if(nh1->data[i] != nh2->data[i]) {
            return false;
        }
    }
    return true;
}

bool isWindowNativeHandleLessThan(const native_handle_t *nh1, const native_handle_t *nh2) {
    if (isWindowNativeHandleEqual(nh1, nh2)) {
        return false;
    }
    if (nh1->numInts != nh2->numInts) {
        return nh1->numInts < nh2->numInts;
    }

    for (int i = 0; i < nh1->numInts; i++) {
        if (nh1->data[i] != nh2->data[i]) {
            return nh1->data[i] < nh2->data[i];
        }
    }
    return false;
}

bool isWindowNativeHandleGreaterThan(const native_handle_t *nh1, const native_handle_t *nh2) {
    return !isWindowNativeHandleLessThan(nh1, nh2) && !isWindowNativeHandleEqual(nh1, nh2);
}

bool areWindowNativeHandlesEqual(hidl_vec<hidl_handle> handles1, hidl_vec<hidl_handle> handles2) {
    if (handles1.size() != handles2.size()) {
        return false;
    }
    for (int i = 0; i < handles1.size(); i++) {
        if (!isWindowNativeHandleEqual(handles1[i], handles2[i])) {
            return false;
        }
    }
    return true;
}

bool areWindowNativeHandlesLessThan(hidl_vec<hidl_handle> handles1, hidl_vec<hidl_handle>handles2) {
    if (handles1.size() != handles2.size()) {
        return handles1.size() < handles2.size();
    }
    for (int i = 0; i < handles1.size(); i++) {
        const native_handle_t *handle1 = handles1[i].getNativeHandle();
        const native_handle_t *handle2 = handles2[i].getNativeHandle();
        if (!isWindowNativeHandleEqual(handle1, handle2)) {
            return isWindowNativeHandleLessThan(handle1, handle2);
        }
    }
    return false;
}

} // namespace utils
} // namespace acam
} // namespace android
