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

#include "ACameraCaptureSession.h"

#ifdef __ANDROID_VNDK__
#include "ndk_vendor/impl/ACameraDeviceVendor.inc"
#else
#include "ACameraDevice.inc"
#endif

using namespace android;

template <class T>
camera_status_t
ACameraCaptureSession::setRepeatingRequest(
        /*optional*/T* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    sp<acam::CameraDevice> dev = getDeviceSp();
    if (dev == nullptr) {
        ALOGE("Error: Device associated with session %p has been closed!", this);
        return ACAMERA_ERROR_SESSION_CLOSED;
    }

    camera_status_t ret;
    dev->lockDeviceForSessionOps();
    {
        Mutex::Autolock _l(mSessionLock);
        ret = dev->setRepeatingRequestsLocked(
                this, cbs, numRequests, requests, captureSequenceId);
    }
    dev->unlockDevice();
    return ret;
}

template <class T>
camera_status_t ACameraCaptureSession::capture(
        /*optional*/T* cbs,
        int numRequests, ACaptureRequest** requests,
        /*optional*/int* captureSequenceId) {
    sp<acam::CameraDevice> dev = getDeviceSp();
    if (dev == nullptr) {
        ALOGE("Error: Device associated with session %p has been closed!", this);
        return ACAMERA_ERROR_SESSION_CLOSED;
    }
    camera_status_t ret;
    dev->lockDeviceForSessionOps();
    {
        Mutex::Autolock _l(mSessionLock);
        ret = dev->captureLocked(this, cbs, numRequests, requests, captureSequenceId);
    }
    dev->unlockDevice();
    return ret;
}
