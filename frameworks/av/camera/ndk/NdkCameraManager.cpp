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
#define LOG_TAG "NdkCameraManager"
#define ATRACE_TAG ATRACE_TAG_CAMERA

#include <utils/Log.h>
#include <utils/Trace.h>

#include <camera/NdkCameraManager.h>

#ifdef __ANDROID_VNDK__
#include "ndk_vendor/impl/ACameraManager.h"
#else
#include "impl/ACameraManager.h"
#endif
#include "impl/ACameraMetadata.h"

using namespace android::acam;

EXPORT
ACameraManager* ACameraManager_create() {
    ATRACE_CALL();
    return new ACameraManager();
}

EXPORT
void ACameraManager_delete(ACameraManager* manager) {
    ATRACE_CALL();
    if (manager != nullptr) {
        delete manager;
    }
}

EXPORT
camera_status_t ACameraManager_getCameraIdList(
        ACameraManager* manager, ACameraIdList** cameraIdList) {
    ATRACE_CALL();
    if (manager == nullptr || cameraIdList == nullptr) {
        ALOGE("%s: invalid argument! manager %p, cameraIdList %p",
              __FUNCTION__, manager, cameraIdList);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return manager->getCameraIdList(cameraIdList);
}

EXPORT
void ACameraManager_deleteCameraIdList(ACameraIdList* cameraIdList) {
    ATRACE_CALL();
    if (cameraIdList != nullptr) {
        ACameraManager::deleteCameraIdList(cameraIdList);
    }
}

EXPORT
camera_status_t ACameraManager_registerAvailabilityCallback(
        ACameraManager*, const ACameraManager_AvailabilityCallbacks *callback) {
    ATRACE_CALL();
    if (callback == nullptr) {
        ALOGE("%s: invalid argument! callback is null!", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    if (callback->onCameraAvailable == nullptr || callback->onCameraUnavailable == nullptr) {
        ALOGE("%s: invalid argument! callback %p, "
                "onCameraAvailable %p, onCameraUnavailable %p",
               __FUNCTION__, callback,
               callback->onCameraAvailable, callback->onCameraUnavailable);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    CameraManagerGlobal::getInstance().registerAvailabilityCallback(callback);
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraManager_unregisterAvailabilityCallback(
        ACameraManager*, const ACameraManager_AvailabilityCallbacks *callback) {
    ATRACE_CALL();
    if (callback == nullptr) {
        ALOGE("%s: invalid argument! callback is null!", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    if (callback->onCameraAvailable == nullptr || callback->onCameraUnavailable == nullptr) {
        ALOGE("%s: invalid argument! callback %p, "
                "onCameraAvailable %p, onCameraUnavailable %p",
               __FUNCTION__, callback,
               callback->onCameraAvailable, callback->onCameraUnavailable);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    CameraManagerGlobal::getInstance().unregisterAvailabilityCallback(callback);
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraManager_registerExtendedAvailabilityCallback(
        ACameraManager* /*manager*/, const ACameraManager_ExtendedAvailabilityCallbacks *callback) {
    ATRACE_CALL();
    if (callback == nullptr) {
        ALOGE("%s: invalid argument! callback is null!", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    if ((callback->availabilityCallbacks.onCameraAvailable == nullptr) ||
            (callback->availabilityCallbacks.onCameraUnavailable == nullptr) ||
            (callback->onCameraAccessPrioritiesChanged == nullptr)) {
        ALOGE("%s: invalid argument! callback %p, "
                "onCameraAvailable %p, onCameraUnavailable %p onCameraAccessPrioritiesChanged %p",
               __FUNCTION__, callback,
               callback->availabilityCallbacks.onCameraAvailable,
               callback->availabilityCallbacks.onCameraUnavailable,
               callback->onCameraAccessPrioritiesChanged);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    auto reservedEntriesCount = sizeof(callback->reserved) / sizeof(callback->reserved[0]);
    for (size_t i = 0; i < reservedEntriesCount; i++) {
        if (callback->reserved[i] != nullptr) {
            ALOGE("%s: invalid argument! callback reserved entries must be set to NULL",
                    __FUNCTION__);
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
    }
    CameraManagerGlobal::getInstance().registerExtendedAvailabilityCallback(callback);
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraManager_unregisterExtendedAvailabilityCallback(
        ACameraManager* /*manager*/, const ACameraManager_ExtendedAvailabilityCallbacks *callback) {
    ATRACE_CALL();
    if (callback == nullptr) {
        ALOGE("%s: invalid argument! callback is null!", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    if ((callback->availabilityCallbacks.onCameraAvailable == nullptr) ||
            (callback->availabilityCallbacks.onCameraUnavailable == nullptr) ||
            (callback->onCameraAccessPrioritiesChanged == nullptr)) {
        ALOGE("%s: invalid argument! callback %p, "
                "onCameraAvailable %p, onCameraUnavailable %p onCameraAccessPrioritiesChanged %p",
               __FUNCTION__, callback,
               callback->availabilityCallbacks.onCameraAvailable,
               callback->availabilityCallbacks.onCameraUnavailable,
               callback->onCameraAccessPrioritiesChanged);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    CameraManagerGlobal::getInstance().unregisterExtendedAvailabilityCallback(callback);
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraManager_getCameraCharacteristics(
        ACameraManager* mgr, const char* cameraId, ACameraMetadata** chars){
    ATRACE_CALL();
    if (mgr == nullptr || cameraId == nullptr || chars == nullptr) {
        ALOGE("%s: invalid argument! mgr %p cameraId %p chars %p",
                __FUNCTION__, mgr, cameraId, chars);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    sp<ACameraMetadata> spChars;
    camera_status_t status = mgr->getCameraCharacteristics(cameraId, &spChars);
    if (status != ACAMERA_OK) {
        return status;
    }
    spChars->incStrong((void*) ACameraManager_getCameraCharacteristics);
    *chars = spChars.get();
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACameraManager_openCamera(
        ACameraManager* mgr, const char* cameraId,
        ACameraDevice_StateCallbacks* callback,
        /*out*/ACameraDevice** device) {
    ATRACE_CALL();
    if (mgr == nullptr || cameraId == nullptr || callback == nullptr || device == nullptr) {
        ALOGE("%s: invalid argument! mgr %p cameraId %p callback %p device %p",
                __FUNCTION__, mgr, cameraId, callback, device);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return mgr->openCamera(cameraId, callback, device);
}

#ifdef __ANDROID_VNDK__
EXPORT
camera_status_t ACameraManager_getTagFromName(ACameraManager *mgr, const char* cameraId,
        const char *name, /*out*/uint32_t *tag) {
    ATRACE_CALL();
    if (mgr == nullptr || cameraId == nullptr || name == nullptr) {
        ALOGE("%s: invalid argument! mgr %p cameraId %p name %p",
                __FUNCTION__, mgr, cameraId, name);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return mgr->getTagFromName(cameraId, name, tag);
}
#endif
