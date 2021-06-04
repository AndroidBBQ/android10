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

#ifndef ANDROID_SERVERS_CAMERA_CAMERAFLASHLIGHT_H
#define ANDROID_SERVERS_CAMERA_CAMERAFLASHLIGHT_H

#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include "common/CameraProviderManager.h"
#include "common/CameraDeviceBase.h"
#include "device1/CameraHardwareInterface.h"


namespace android {

/**
 * FlashControlBase is a base class for flash control. It defines the functions
 * that a flash control for each camera module/device version should implement.
 */
class FlashControlBase : public virtual VirtualLightRefBase {
    public:
        virtual ~FlashControlBase();

        // Whether a camera device has a flash unit. Calling this function may
        // cause the torch mode to be turned off in HAL v1 devices. If
        // previously-on torch mode is turned off,
        // callbacks.torch_mode_status_change() should be invoked.
        virtual status_t hasFlashUnit(const String8& cameraId,
                    bool *hasFlash) = 0;

        // set the torch mode to on or off.
        virtual status_t setTorchMode(const String8& cameraId,
                    bool enabled) = 0;
};

/**
 * CameraFlashlight can be used by camera service to control flashflight.
 */
class CameraFlashlight : public virtual VirtualLightRefBase {
    public:
        CameraFlashlight(sp<CameraProviderManager> providerManager,
                CameraProviderManager::StatusListener* callbacks);
        virtual ~CameraFlashlight();

        // Find all flash units. This must be called before other methods. All
        // camera devices must be closed when it's called because HAL v1 devices
        // need to be opened to query available flash modes.
        status_t findFlashUnits();

        // Whether a camera device has a flash unit. Before findFlashUnits() is
        // called, this function always returns false.
        bool hasFlashUnit(const String8& cameraId);

        // set the torch mode to on or off.
        status_t setTorchMode(const String8& cameraId, bool enabled);

        // Notify CameraFlashlight that camera service is going to open a camera
        // device. CameraFlashlight will free the resources that may cause the
        // camera open to fail. Camera service must call this function before
        // opening a camera device.
        status_t prepareDeviceOpen(const String8& cameraId);

        // Notify CameraFlashlight that camera service has closed a camera
        // device. CameraFlashlight may invoke callbacks for torch mode
        // available depending on the implementation.
        status_t deviceClosed(const String8& cameraId);

    private:
        // create flashlight control based on camera module API and camera
        // device API versions.
        status_t createFlashlightControl(const String8& cameraId);

        // mLock should be locked.
        bool hasFlashUnitLocked(const String8& cameraId);

        // Check if flash control is in backward compatible mode (simulated torch API by
        // opening cameras)
        bool isBackwardCompatibleMode(const String8& cameraId);

        sp<FlashControlBase> mFlashControl;

        sp<CameraProviderManager> mProviderManager;

        CameraProviderManager::StatusListener* mCallbacks;
        SortedVector<String8> mOpenedCameraIds;

        // camera id -> if it has a flash unit
        KeyedVector<String8, bool> mHasFlashlightMap;
        bool mFlashlightMapInitialized;

        Mutex mLock; // protect CameraFlashlight API
};

/**
 * Flash control for camera provider v2.4 and above.
 */
class ProviderFlashControl : public FlashControlBase {
    public:
        ProviderFlashControl(sp<CameraProviderManager> providerManager);
        virtual ~ProviderFlashControl();

        // FlashControlBase
        status_t hasFlashUnit(const String8& cameraId, bool *hasFlash);
        status_t setTorchMode(const String8& cameraId, bool enabled);

    private:
        sp<CameraProviderManager> mProviderManager;

        Mutex mLock;
};

/**
 * Flash control for camera module <= v2.3 and camera HAL v1
 */
class CameraHardwareInterfaceFlashControl : public FlashControlBase {
    public:
        CameraHardwareInterfaceFlashControl(
                sp<CameraProviderManager> manager,
                CameraProviderManager::StatusListener* callbacks);
        virtual ~CameraHardwareInterfaceFlashControl();

        // FlashControlBase
        status_t setTorchMode(const String8& cameraId, bool enabled);
        status_t hasFlashUnit(const String8& cameraId, bool *hasFlash);

    private:
        // connect to a camera device
        status_t connectCameraDevice(const String8& cameraId);

        // disconnect and free mDevice
        status_t disconnectCameraDevice();

        // initialize the preview window
        status_t initializePreviewWindow(const sp<CameraHardwareInterface>& device,
                int32_t width, int32_t height);

        // start preview and enable torch
        status_t startPreviewAndTorch();

        // get the smallest surface
        status_t getSmallestSurfaceSize(int32_t *width, int32_t *height);

        // protected by mLock
        // If this function opens camera device in order to check if it has a flash unit, the
        // camera device will remain open if keepDeviceOpen is true and the camera device will be
        // closed if keepDeviceOpen is false. If camera device is already open when calling this
        // function, keepDeviceOpen is ignored.
        status_t hasFlashUnitLocked(const String8& cameraId, bool *hasFlash, bool keepDeviceOpen);

        sp<CameraProviderManager> mProviderManager;
        CameraProviderManager::StatusListener* mCallbacks;
        sp<CameraHardwareInterface> mDevice;
        String8 mCameraId;
        CameraParameters mParameters;
        bool mTorchEnabled;

        sp<IGraphicBufferProducer> mProducer;
        sp<IGraphicBufferConsumer>  mConsumer;
        sp<GLConsumer> mSurfaceTexture;
        sp<Surface> mSurface;

        Mutex mLock;
};

} // namespace android

#endif
