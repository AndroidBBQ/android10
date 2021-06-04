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

#define LOG_TAG "Camera2ClientBase"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <inttypes.h>

#include <utils/Log.h>
#include <utils/Trace.h>

#include <cutils/properties.h>
#include <gui/Surface.h>
#include <gui/Surface.h>

#include "common/Camera2ClientBase.h"

#include "api2/CameraDeviceClient.h"

#include "device3/Camera3Device.h"
#include "utils/CameraThreadState.h"

namespace android {
using namespace camera2;

// Interface used by CameraService

template <typename TClientBase>
Camera2ClientBase<TClientBase>::Camera2ClientBase(
        const sp<CameraService>& cameraService,
        const sp<TCamCallbacks>& remoteCallback,
        const String16& clientPackageName,
        const String8& cameraId,
        int api1CameraId,
        int cameraFacing,
        int clientPid,
        uid_t clientUid,
        int servicePid):
        TClientBase(cameraService, remoteCallback, clientPackageName,
                cameraId, api1CameraId, cameraFacing, clientPid, clientUid, servicePid),
        mSharedCameraCallbacks(remoteCallback),
        mDeviceVersion(cameraService->getDeviceVersion(TClientBase::mCameraIdStr)),
        mDevice(new Camera3Device(cameraId)),
        mDeviceActive(false), mApi1CameraId(api1CameraId)
{
    ALOGI("Camera %s: Opened. Client: %s (PID %d, UID %d)", cameraId.string(),
            String8(clientPackageName).string(), clientPid, clientUid);

    mInitialClientPid = clientPid;
    LOG_ALWAYS_FATAL_IF(mDevice == 0, "Device should never be NULL here.");
}

template <typename TClientBase>
status_t Camera2ClientBase<TClientBase>::checkPid(const char* checkLocation)
        const {

    int callingPid = CameraThreadState::getCallingPid();
    if (callingPid == TClientBase::mClientPid) return NO_ERROR;

    ALOGE("%s: attempt to use a locked camera from a different process"
            " (old pid %d, new pid %d)", checkLocation, TClientBase::mClientPid, callingPid);
    return PERMISSION_DENIED;
}

template <typename TClientBase>
status_t Camera2ClientBase<TClientBase>::initialize(sp<CameraProviderManager> manager,
        const String8& monitorTags) {
    return initializeImpl(manager, monitorTags);
}

template <typename TClientBase>
template <typename TProviderPtr>
status_t Camera2ClientBase<TClientBase>::initializeImpl(TProviderPtr providerPtr,
        const String8& monitorTags) {
    ATRACE_CALL();
    ALOGV("%s: Initializing client for camera %s", __FUNCTION__,
          TClientBase::mCameraIdStr.string());
    status_t res;

    // Verify ops permissions
    res = TClientBase::startCameraOps();
    if (res != OK) {
        return res;
    }

    if (mDevice == NULL) {
        ALOGE("%s: Camera %s: No device connected",
                __FUNCTION__, TClientBase::mCameraIdStr.string());
        return NO_INIT;
    }

    res = mDevice->initialize(providerPtr, monitorTags);
    if (res != OK) {
        ALOGE("%s: Camera %s: unable to initialize device: %s (%d)",
                __FUNCTION__, TClientBase::mCameraIdStr.string(), strerror(-res), res);
        return res;
    }

    wp<CameraDeviceBase::NotificationListener> weakThis(this);
    res = mDevice->setNotifyCallback(weakThis);

    return OK;
}

template <typename TClientBase>
Camera2ClientBase<TClientBase>::~Camera2ClientBase() {
    ATRACE_CALL();

    TClientBase::mDestructionStarted = true;

    disconnect();

    ALOGI("Closed Camera %s. Client was: %s (PID %d, UID %u)",
            TClientBase::mCameraIdStr.string(),
            String8(TClientBase::mClientPackageName).string(),
            mInitialClientPid, TClientBase::mClientUid);
}

template <typename TClientBase>
status_t Camera2ClientBase<TClientBase>::dumpClient(int fd,
                                              const Vector<String16>& args) {
    String8 result;
    result.appendFormat("Camera2ClientBase[%s] (%p) PID: %d, dump:\n",
            TClientBase::mCameraIdStr.string(),
            (TClientBase::getRemoteCallback() != NULL ?
                    IInterface::asBinder(TClientBase::getRemoteCallback()).get() : NULL),
            TClientBase::mClientPid);
    result.append("  State: ");

    write(fd, result.string(), result.size());
    // TODO: print dynamic/request section from most recent requests

    return dumpDevice(fd, args);
}

template <typename TClientBase>
status_t Camera2ClientBase<TClientBase>::dumpDevice(
                                                int fd,
                                                const Vector<String16>& args) {
    String8 result;

    result = "  Device dump:\n";
    write(fd, result.string(), result.size());

    sp<CameraDeviceBase> device = mDevice;
    if (!device.get()) {
        result = "  *** Device is detached\n";
        write(fd, result.string(), result.size());
        return NO_ERROR;
    }

    status_t res = device->dump(fd, args);
    if (res != OK) {
        result = String8::format("   Error dumping device: %s (%d)",
                strerror(-res), res);
        write(fd, result.string(), result.size());
    }

    return NO_ERROR;
}

// ICameraClient2BaseUser interface


template <typename TClientBase>
binder::Status Camera2ClientBase<TClientBase>::disconnect() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);

    binder::Status res = binder::Status::ok();
    // Allow both client and the media server to disconnect at all times
    int callingPid = CameraThreadState::getCallingPid();
    if (callingPid != TClientBase::mClientPid &&
        callingPid != TClientBase::mServicePid) return res;

    ALOGV("Camera %s: Shutting down", TClientBase::mCameraIdStr.string());

    detachDevice();

    CameraService::BasicClient::disconnect();

    ALOGV("Camera %s: Shut down complete complete", TClientBase::mCameraIdStr.string());

    return res;
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::detachDevice() {
    if (mDevice == 0) return;
    mDevice->disconnect();

    ALOGV("Camera %s: Detach complete", TClientBase::mCameraIdStr.string());
}

template <typename TClientBase>
status_t Camera2ClientBase<TClientBase>::connect(
        const sp<TCamCallbacks>& client) {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);

    if (TClientBase::mClientPid != 0 &&
        CameraThreadState::getCallingPid() != TClientBase::mClientPid) {

        ALOGE("%s: Camera %s: Connection attempt from pid %d; "
                "current locked to pid %d",
                __FUNCTION__,
                TClientBase::mCameraIdStr.string(),
                CameraThreadState::getCallingPid(),
                TClientBase::mClientPid);
        return BAD_VALUE;
    }

    TClientBase::mClientPid = CameraThreadState::getCallingPid();

    TClientBase::mRemoteCallback = client;
    mSharedCameraCallbacks = client;

    return OK;
}

/** Device-related methods */

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyError(
        int32_t errorCode,
        const CaptureResultExtras& resultExtras) {
    ALOGE("Error condition %d reported by HAL, requestId %" PRId32, errorCode,
          resultExtras.requestId);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyIdle() {
    if (mDeviceActive) {
        getCameraService()->updateProxyDeviceState(
            hardware::ICameraServiceProxy::CAMERA_STATE_IDLE, TClientBase::mCameraIdStr,
            TClientBase::mCameraFacing, TClientBase::mClientPackageName,
            ((mApi1CameraId < 0) ? hardware::ICameraServiceProxy::CAMERA_API_LEVEL_2 :
             hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1));
    }
    mDeviceActive = false;

    ALOGV("Camera device is now idle");
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyShutter(const CaptureResultExtras& resultExtras,
                                                   nsecs_t timestamp) {
    (void)resultExtras;
    (void)timestamp;

    if (!mDeviceActive) {
        getCameraService()->updateProxyDeviceState(
            hardware::ICameraServiceProxy::CAMERA_STATE_ACTIVE, TClientBase::mCameraIdStr,
            TClientBase::mCameraFacing, TClientBase::mClientPackageName,
            ((mApi1CameraId < 0) ? hardware::ICameraServiceProxy::CAMERA_API_LEVEL_2 :
             hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1));
    }
    mDeviceActive = true;

    ALOGV("%s: Shutter notification for request id %" PRId32 " at time %" PRId64,
            __FUNCTION__, resultExtras.requestId, timestamp);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyAutoFocus(uint8_t newState,
                                                     int triggerId) {
    (void)newState;
    (void)triggerId;

    ALOGV("%s: Autofocus state now %d, last trigger %d",
          __FUNCTION__, newState, triggerId);

}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyAutoExposure(uint8_t newState,
                                                        int triggerId) {
    (void)newState;
    (void)triggerId;

    ALOGV("%s: Autoexposure state now %d, last trigger %d",
            __FUNCTION__, newState, triggerId);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyAutoWhitebalance(uint8_t newState,
                                                            int triggerId) {
    (void)newState;
    (void)triggerId;

    ALOGV("%s: Auto-whitebalance state now %d, last trigger %d",
            __FUNCTION__, newState, triggerId);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyPrepared(int streamId) {
    (void)streamId;

    ALOGV("%s: Stream %d now prepared",
            __FUNCTION__, streamId);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyRequestQueueEmpty() {

    ALOGV("%s: Request queue now empty", __FUNCTION__);
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::notifyRepeatingRequestError(long lastFrameNumber) {
    (void)lastFrameNumber;

    ALOGV("%s: Repeating request was stopped. Last frame number is %ld",
            __FUNCTION__, lastFrameNumber);
}

template <typename TClientBase>
int Camera2ClientBase<TClientBase>::getCameraId() const {
    return mApi1CameraId;
}

template <typename TClientBase>
int Camera2ClientBase<TClientBase>::getCameraDeviceVersion() const {
    return mDeviceVersion;
}

template <typename TClientBase>
const sp<CameraDeviceBase>& Camera2ClientBase<TClientBase>::getCameraDevice() {
    return mDevice;
}

template <typename TClientBase>
const sp<CameraService>& Camera2ClientBase<TClientBase>::getCameraService() {
    return TClientBase::sCameraService;
}

template <typename TClientBase>
Camera2ClientBase<TClientBase>::SharedCameraCallbacks::Lock::Lock(
        SharedCameraCallbacks &client) :

        mRemoteCallback(client.mRemoteCallback),
        mSharedClient(client) {

    mSharedClient.mRemoteCallbackLock.lock();
}

template <typename TClientBase>
Camera2ClientBase<TClientBase>::SharedCameraCallbacks::Lock::~Lock() {
    mSharedClient.mRemoteCallbackLock.unlock();
}

template <typename TClientBase>
Camera2ClientBase<TClientBase>::SharedCameraCallbacks::SharedCameraCallbacks(
        const sp<TCamCallbacks>&client) :

        mRemoteCallback(client) {
}

template <typename TClientBase>
typename Camera2ClientBase<TClientBase>::SharedCameraCallbacks&
Camera2ClientBase<TClientBase>::SharedCameraCallbacks::operator=(
        const sp<TCamCallbacks>&client) {

    Mutex::Autolock l(mRemoteCallbackLock);
    mRemoteCallback = client;
    return *this;
}

template <typename TClientBase>
void Camera2ClientBase<TClientBase>::SharedCameraCallbacks::clear() {
    Mutex::Autolock l(mRemoteCallbackLock);
    mRemoteCallback.clear();
}

template class Camera2ClientBase<CameraService::Client>;
template class Camera2ClientBase<CameraDeviceClientBase>;

} // namespace android
