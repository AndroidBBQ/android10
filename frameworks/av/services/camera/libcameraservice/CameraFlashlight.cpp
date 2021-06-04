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

#define LOG_TAG "CameraFlashlight"
#define ATRACE_TAG ATRACE_TAG_CAMERA
// #define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>
#include <cutils/properties.h>

#include "camera/CameraMetadata.h"
#include "CameraFlashlight.h"
#include "gui/IGraphicBufferConsumer.h"
#include "gui/BufferQueue.h"
#include "camera/camera2/CaptureRequest.h"
#include "device3/Camera3Device.h"


namespace android {

using hardware::camera::common::V1_0::TorchModeStatus;

/////////////////////////////////////////////////////////////////////
// CameraFlashlight implementation begins
// used by camera service to control flashflight.
/////////////////////////////////////////////////////////////////////

CameraFlashlight::CameraFlashlight(sp<CameraProviderManager> providerManager,
        CameraProviderManager::StatusListener* callbacks) :
        mProviderManager(providerManager),
        mCallbacks(callbacks),
        mFlashlightMapInitialized(false) {
}

CameraFlashlight::~CameraFlashlight() {
}

status_t CameraFlashlight::createFlashlightControl(const String8& cameraId) {
    ALOGV("%s: creating a flash light control for camera %s", __FUNCTION__,
            cameraId.string());
    if (mFlashControl != NULL) {
        return INVALID_OPERATION;
    }

    if (mProviderManager->supportSetTorchMode(cameraId.string())) {
        mFlashControl = new ProviderFlashControl(mProviderManager);
    } else {
        // Only HAL1 devices do not support setTorchMode
        mFlashControl =
                new CameraHardwareInterfaceFlashControl(mProviderManager, mCallbacks);
    }

    return OK;
}

status_t CameraFlashlight::setTorchMode(const String8& cameraId, bool enabled) {
    if (!mFlashlightMapInitialized) {
        ALOGE("%s: findFlashUnits() must be called before this method.",
               __FUNCTION__);
        return NO_INIT;
    }

    ALOGV("%s: set torch mode of camera %s to %d", __FUNCTION__,
            cameraId.string(), enabled);

    status_t res = OK;
    Mutex::Autolock l(mLock);

    if (mOpenedCameraIds.indexOf(cameraId) != NAME_NOT_FOUND) {
        // This case is needed to avoid state corruption during the following call sequence:
        // CameraService::setTorchMode for camera ID 0 begins, does torch status checks
        // CameraService::connect for camera ID 0 begins, calls prepareDeviceOpen, ends
        // CameraService::setTorchMode for camera ID 0 continues, calls
        //        CameraFlashlight::setTorchMode

        // TODO: Move torch status checks and state updates behind this CameraFlashlight lock
        // to avoid other similar race conditions.
        ALOGE("%s: Camera device %s is in use, cannot set torch mode.",
                __FUNCTION__, cameraId.string());
        return -EBUSY;
    }

    if (mFlashControl == NULL) {
        res = createFlashlightControl(cameraId);
        if (res) {
            return res;
        }
        res =  mFlashControl->setTorchMode(cameraId, enabled);
        return res;
    }

    // if flash control already exists, turning on torch mode may fail if it's
    // tied to another camera device for module v2.3 and below.
    res = mFlashControl->setTorchMode(cameraId, enabled);
    if (res == BAD_INDEX) {
        // flash control is tied to another camera device, need to close it and
        // try again.
        mFlashControl.clear();
        res = createFlashlightControl(cameraId);
        if (res) {
            return res;
        }
        res = mFlashControl->setTorchMode(cameraId, enabled);
    }

    return res;
}

status_t CameraFlashlight::findFlashUnits() {
    Mutex::Autolock l(mLock);
    status_t res;

    std::vector<String8> cameraIds;
    std::vector<std::string> ids = mProviderManager->getCameraDeviceIds();
    int numberOfCameras = static_cast<int>(ids.size());
    cameraIds.resize(numberOfCameras);
    // No module, must be provider
    for (size_t i = 0; i < cameraIds.size(); i++) {
        cameraIds[i] = String8(ids[i].c_str());
    }

    mFlashControl.clear();

    for (auto &id : cameraIds) {
        ssize_t index = mHasFlashlightMap.indexOfKey(id);
        if (0 <= index) {
            continue;
        }

        bool hasFlash = false;
        res = createFlashlightControl(id);
        if (res) {
            ALOGE("%s: failed to create flash control for %s", __FUNCTION__,
                    id.string());
        } else {
            res = mFlashControl->hasFlashUnit(id, &hasFlash);
            if (res == -EUSERS || res == -EBUSY) {
                ALOGE("%s: failed to check if camera %s has a flash unit. Some "
                        "camera devices may be opened", __FUNCTION__,
                        id.string());
                return res;
            } else if (res) {
                ALOGE("%s: failed to check if camera %s has a flash unit. %s"
                        " (%d)", __FUNCTION__, id.string(), strerror(-res),
                        res);
            }

            mFlashControl.clear();
        }
        mHasFlashlightMap.add(id, hasFlash);
    }

    mFlashlightMapInitialized = true;
    return OK;
}

bool CameraFlashlight::hasFlashUnit(const String8& cameraId) {
    Mutex::Autolock l(mLock);
    return hasFlashUnitLocked(cameraId);
}

bool CameraFlashlight::hasFlashUnitLocked(const String8& cameraId) {
    if (!mFlashlightMapInitialized) {
        ALOGE("%s: findFlashUnits() must be called before this method.",
               __FUNCTION__);
        return false;
    }

    ssize_t index = mHasFlashlightMap.indexOfKey(cameraId);
    if (index == NAME_NOT_FOUND) {
        // Might be external camera
        ALOGW("%s: camera %s not present when findFlashUnits() was called",
                __FUNCTION__, cameraId.string());
        return false;
    }

    return mHasFlashlightMap.valueAt(index);
}

bool CameraFlashlight::isBackwardCompatibleMode(const String8& cameraId) {
    bool backwardCompatibleMode = false;
    if (mProviderManager != nullptr &&
            !mProviderManager->supportSetTorchMode(cameraId.string())) {
        backwardCompatibleMode = true;
    }
    return backwardCompatibleMode;
}

status_t CameraFlashlight::prepareDeviceOpen(const String8& cameraId) {
    ALOGV("%s: prepare for device open", __FUNCTION__);

    Mutex::Autolock l(mLock);
    if (!mFlashlightMapInitialized) {
        ALOGE("%s: findFlashUnits() must be called before this method.",
               __FUNCTION__);
        return NO_INIT;
    }

    if (isBackwardCompatibleMode(cameraId)) {
        // framework is going to open a camera device, all flash light control
        // should be closed for backward compatible support.
        mFlashControl.clear();

        if (mOpenedCameraIds.size() == 0) {
            // notify torch unavailable for all cameras with a flash
            std::vector<std::string> ids = mProviderManager->getCameraDeviceIds();
            int numCameras = static_cast<int>(ids.size());
            for (int i = 0; i < numCameras; i++) {
                String8 id8(ids[i].c_str());
                if (hasFlashUnitLocked(id8)) {
                    mCallbacks->onTorchStatusChanged(
                            id8, TorchModeStatus::NOT_AVAILABLE);
                }
            }
        }

        // close flash control that may be opened by calling hasFlashUnitLocked.
        mFlashControl.clear();
    }

    if (mOpenedCameraIds.indexOf(cameraId) == NAME_NOT_FOUND) {
        mOpenedCameraIds.add(cameraId);
    }

    return OK;
}

status_t CameraFlashlight::deviceClosed(const String8& cameraId) {
    ALOGV("%s: device %s is closed", __FUNCTION__, cameraId.string());

    Mutex::Autolock l(mLock);
    if (!mFlashlightMapInitialized) {
        ALOGE("%s: findFlashUnits() must be called before this method.",
               __FUNCTION__);
        return NO_INIT;
    }

    ssize_t index = mOpenedCameraIds.indexOf(cameraId);
    if (index == NAME_NOT_FOUND) {
        ALOGE("%s: couldn't find camera %s in the opened list", __FUNCTION__,
                cameraId.string());
    } else {
        mOpenedCameraIds.removeAt(index);
    }

    // Cannot do anything until all cameras are closed.
    if (mOpenedCameraIds.size() != 0)
        return OK;

    if (isBackwardCompatibleMode(cameraId)) {
        // notify torch available for all cameras with a flash
        std::vector<std::string> ids = mProviderManager->getCameraDeviceIds();
        int numCameras = static_cast<int>(ids.size());
        for (int i = 0; i < numCameras; i++) {
            String8 id8(ids[i].c_str());
            if (hasFlashUnitLocked(id8)) {
                mCallbacks->onTorchStatusChanged(
                        id8, TorchModeStatus::AVAILABLE_OFF);
            }
        }
    }

    return OK;
}
// CameraFlashlight implementation ends


FlashControlBase::~FlashControlBase() {
}

/////////////////////////////////////////////////////////////////////
// ModuleFlashControl implementation begins
// Flash control for camera module v2.4 and above.
/////////////////////////////////////////////////////////////////////
ProviderFlashControl::ProviderFlashControl(sp<CameraProviderManager> providerManager) :
        mProviderManager(providerManager) {
}

ProviderFlashControl::~ProviderFlashControl() {
}

status_t ProviderFlashControl::hasFlashUnit(const String8& cameraId, bool *hasFlash) {
    if (!hasFlash) {
        return BAD_VALUE;
    }
    *hasFlash = mProviderManager->hasFlashUnit(cameraId.string());
    return OK;
}

status_t ProviderFlashControl::setTorchMode(const String8& cameraId, bool enabled) {
    ALOGV("%s: set camera %s torch mode to %d", __FUNCTION__,
            cameraId.string(), enabled);

    return mProviderManager->setTorchMode(cameraId.string(), enabled);
}
// ProviderFlashControl implementation ends

/////////////////////////////////////////////////////////////////////
// CameraHardwareInterfaceFlashControl implementation begins
// Flash control for camera module <= v2.3 and camera HAL v1
/////////////////////////////////////////////////////////////////////

CameraHardwareInterfaceFlashControl::CameraHardwareInterfaceFlashControl(
        sp<CameraProviderManager> manager,
        CameraProviderManager::StatusListener* callbacks) :
        mProviderManager(manager),
        mCallbacks(callbacks),
        mTorchEnabled(false) {
}

CameraHardwareInterfaceFlashControl::~CameraHardwareInterfaceFlashControl() {
    disconnectCameraDevice();

    mSurface.clear();
    mSurfaceTexture.clear();
    mProducer.clear();
    mConsumer.clear();

    if (mTorchEnabled) {
        if (mCallbacks) {
            ALOGV("%s: notify the framework that torch was turned off",
                    __FUNCTION__);
            mCallbacks->onTorchStatusChanged(mCameraId, TorchModeStatus::AVAILABLE_OFF);
        }
    }
}

status_t CameraHardwareInterfaceFlashControl::setTorchMode(
        const String8& cameraId, bool enabled) {
    Mutex::Autolock l(mLock);

    // pre-check
    status_t res;
    if (enabled) {
        bool hasFlash = false;
        // Check if it has a flash unit and leave camera device open.
        res = hasFlashUnitLocked(cameraId, &hasFlash, /*keepDeviceOpen*/true);
        // invalid camera?
        if (res) {
            // hasFlashUnitLocked() returns BAD_INDEX if mDevice is connected to
            // another camera device.
            return res == BAD_INDEX ? BAD_INDEX : -EINVAL;
        }
        // no flash unit?
        if (!hasFlash) {
            // Disconnect camera device if it has no flash.
            disconnectCameraDevice();
            return -ENOSYS;
        }
    } else if (mDevice == NULL || cameraId != mCameraId) {
        // disabling the torch mode of an un-opened or different device.
        return OK;
    } else {
        // disabling the torch mode of currently opened device
        disconnectCameraDevice();
        mTorchEnabled = false;
        mCallbacks->onTorchStatusChanged(cameraId, TorchModeStatus::AVAILABLE_OFF);
        return OK;
    }

    res = startPreviewAndTorch();
    if (res) {
        return res;
    }

    mTorchEnabled = true;
    mCallbacks->onTorchStatusChanged(cameraId, TorchModeStatus::AVAILABLE_ON);
    return OK;
}

status_t CameraHardwareInterfaceFlashControl::hasFlashUnit(
        const String8& cameraId, bool *hasFlash) {
    Mutex::Autolock l(mLock);
    // Close device after checking if it has a flash unit.
    return hasFlashUnitLocked(cameraId, hasFlash, /*keepDeviceOpen*/false);
}

status_t CameraHardwareInterfaceFlashControl::hasFlashUnitLocked(
        const String8& cameraId, bool *hasFlash, bool keepDeviceOpen) {
    bool closeCameraDevice = false;

    if (!hasFlash) {
        return BAD_VALUE;
    }

    status_t res;
    if (mDevice == NULL) {
        // Connect to camera device to query if it has a flash unit.
        res = connectCameraDevice(cameraId);
        if (res) {
            return res;
        }
        // Close camera device only when it is just opened and the caller doesn't want to keep
        // the camera device open.
        closeCameraDevice = !keepDeviceOpen;
    }

    if (cameraId != mCameraId) {
        return BAD_INDEX;
    }

    const char *flashMode =
            mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES);
    if (flashMode && strstr(flashMode, CameraParameters::FLASH_MODE_TORCH)) {
        *hasFlash = true;
    } else {
        *hasFlash = false;
    }

    if (closeCameraDevice) {
        res = disconnectCameraDevice();
        if (res != OK) {
            ALOGE("%s: Failed to disconnect camera device. %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }
    }

    return OK;
}

status_t CameraHardwareInterfaceFlashControl::startPreviewAndTorch() {
    status_t res = OK;
    res = mDevice->startPreview();
    if (res) {
        ALOGE("%s: start preview failed. %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    mParameters.set(CameraParameters::KEY_FLASH_MODE,
            CameraParameters::FLASH_MODE_TORCH);

    return mDevice->setParameters(mParameters);
}

status_t CameraHardwareInterfaceFlashControl::getSmallestSurfaceSize(
        int32_t *width, int32_t *height) {
    if (!width || !height) {
        return BAD_VALUE;
    }

    int32_t w = INT32_MAX;
    int32_t h = 1;
    Vector<Size> sizes;

    mParameters.getSupportedPreviewSizes(sizes);
    for (size_t i = 0; i < sizes.size(); i++) {
        Size s = sizes[i];
        if (w * h > s.width * s.height) {
            w = s.width;
            h = s.height;
        }
    }

    if (w == INT32_MAX) {
        return NAME_NOT_FOUND;
    }

    *width = w;
    *height = h;

    return OK;
}

status_t CameraHardwareInterfaceFlashControl::initializePreviewWindow(
        const sp<CameraHardwareInterface>& device, int32_t width, int32_t height) {
    status_t res;
    BufferQueue::createBufferQueue(&mProducer, &mConsumer);

    mSurfaceTexture = new GLConsumer(mConsumer, 0, GLConsumer::TEXTURE_EXTERNAL,
            true, true);
    if (mSurfaceTexture == NULL) {
        return NO_MEMORY;
    }

    int32_t format = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    res = mSurfaceTexture->setDefaultBufferSize(width, height);
    if (res) {
        return res;
    }
    res = mSurfaceTexture->setDefaultBufferFormat(format);
    if (res) {
        return res;
    }

    mSurface = new Surface(mProducer, /*useAsync*/ true);
    if (mSurface == NULL) {
        return NO_MEMORY;
    }

    res = native_window_api_connect(mSurface.get(), NATIVE_WINDOW_API_CAMERA);
    if (res) {
        ALOGE("%s: Unable to connect to native window", __FUNCTION__);
        return res;
    }

    return device->setPreviewWindow(mSurface);
}

status_t CameraHardwareInterfaceFlashControl::connectCameraDevice(
        const String8& cameraId) {
    sp<CameraHardwareInterface> device =
            new CameraHardwareInterface(cameraId.string());

    status_t res = device->initialize(mProviderManager);
    if (res) {
        ALOGE("%s: initializing camera %s failed", __FUNCTION__,
                cameraId.string());
        return res;
    }

    // need to set __get_memory in set_callbacks().
    device->setCallbacks(NULL, NULL, NULL, NULL, NULL);

    mParameters = device->getParameters();

    int32_t width, height;
    res = getSmallestSurfaceSize(&width, &height);
    if (res) {
        ALOGE("%s: failed to get smallest surface size for camera %s",
                __FUNCTION__, cameraId.string());
        return res;
    }

    res = initializePreviewWindow(device, width, height);
    if (res) {
        ALOGE("%s: failed to initialize preview window for camera %s",
                __FUNCTION__, cameraId.string());
        return res;
    }

    mCameraId = cameraId;
    mDevice = device;
    return OK;
}

status_t CameraHardwareInterfaceFlashControl::disconnectCameraDevice() {
    if (mDevice == NULL) {
        return OK;
    }

    if (mParameters.get(CameraParameters::KEY_FLASH_MODE)) {
        // There is a flash, turn if off.
        // (If there isn't one, leave the parameter null)
        mParameters.set(CameraParameters::KEY_FLASH_MODE,
                CameraParameters::FLASH_MODE_OFF);
        mDevice->setParameters(mParameters);
    }
    mDevice->stopPreview();
    status_t res = native_window_api_disconnect(mSurface.get(),
            NATIVE_WINDOW_API_CAMERA);
    if (res) {
        ALOGW("%s: native_window_api_disconnect failed: %s (%d)",
                __FUNCTION__, strerror(-res), res);
    }
    mDevice->setPreviewWindow(NULL);
    mDevice->release();
    mDevice = NULL;

    return OK;
}
// CameraHardwareInterfaceFlashControl implementation ends

}
