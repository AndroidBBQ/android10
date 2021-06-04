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
#define LOG_TAG "ACameraManager"

#include <memory>
#include "ACameraManager.h"
#include "ACameraMetadata.h"
#include "ACameraDevice.h"
#include <utils/Vector.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <camera/VendorTagDescriptor.h>

using namespace android::acam;

namespace android {
namespace acam {
// Static member definitions
const char* CameraManagerGlobal::kCameraIdKey   = "CameraId";
const char* CameraManagerGlobal::kCallbackFpKey = "CallbackFp";
const char* CameraManagerGlobal::kContextKey    = "CallbackContext";
Mutex                CameraManagerGlobal::sLock;
CameraManagerGlobal* CameraManagerGlobal::sInstance = nullptr;

CameraManagerGlobal&
CameraManagerGlobal::getInstance() {
    Mutex::Autolock _l(sLock);
    CameraManagerGlobal* instance = sInstance;
    if (instance == nullptr) {
        instance = new CameraManagerGlobal();
        sInstance = instance;
    }
    return *instance;
}

CameraManagerGlobal::~CameraManagerGlobal() {
    // clear sInstance so next getInstance call knows to create a new one
    Mutex::Autolock _sl(sLock);
    sInstance = nullptr;
    Mutex::Autolock _l(mLock);
    if (mCameraService != nullptr) {
        IInterface::asBinder(mCameraService)->unlinkToDeath(mDeathNotifier);
        mCameraService->removeListener(mCameraServiceListener);
    }
    mDeathNotifier.clear();
    if (mCbLooper != nullptr) {
        mCbLooper->unregisterHandler(mHandler->id());
        mCbLooper->stop();
    }
    mCbLooper.clear();
    mHandler.clear();
    mCameraServiceListener.clear();
    mCameraService.clear();
}

static bool isCameraServiceDisabled() {
    char value[PROPERTY_VALUE_MAX];
    property_get("config.disable_cameraservice", value, "0");
    return (strncmp(value, "0", 2) != 0 && strncasecmp(value, "false", 6) != 0);
}

sp<hardware::ICameraService> CameraManagerGlobal::getCameraService() {
    Mutex::Autolock _l(mLock);
    if (mCameraService.get() == nullptr) {
        if (isCameraServiceDisabled()) {
            return mCameraService;
        }

        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16(kCameraServiceName));
            if (binder != nullptr) {
                break;
            }
            ALOGW("CameraService not published, waiting...");
            usleep(kCameraServicePollDelay);
        } while(true);
        if (mDeathNotifier == nullptr) {
            mDeathNotifier = new DeathNotifier(this);
        }
        binder->linkToDeath(mDeathNotifier);
        mCameraService = interface_cast<hardware::ICameraService>(binder);

        // Setup looper thread to perfrom availiability callbacks
        if (mCbLooper == nullptr) {
            mCbLooper = new ALooper;
            mCbLooper->setName("C2N-mgr-looper");
            status_t err = mCbLooper->start(
                    /*runOnCallingThread*/false,
                    /*canCallJava*/       true,
                    PRIORITY_DEFAULT);
            if (err != OK) {
                ALOGE("%s: Unable to start camera service listener looper: %s (%d)",
                        __FUNCTION__, strerror(-err), err);
                mCbLooper.clear();
                return nullptr;
            }
            if (mHandler == nullptr) {
                mHandler = new CallbackHandler();
            }
            mCbLooper->registerHandler(mHandler);
        }

        // register ICameraServiceListener
        if (mCameraServiceListener == nullptr) {
            mCameraServiceListener = new CameraServiceListener(this);
        }
        std::vector<hardware::CameraStatus> cameraStatuses{};
        mCameraService->addListener(mCameraServiceListener, &cameraStatuses);
        for (auto& c : cameraStatuses) {
            onStatusChangedLocked(c.status, c.cameraId);
        }

        // setup vendor tags
        sp<VendorTagDescriptor> desc = new VendorTagDescriptor();
        binder::Status ret = mCameraService->getCameraVendorTagDescriptor(/*out*/desc.get());

        if (ret.isOk()) {
            if (0 < desc->getTagCount()) {
                status_t err = VendorTagDescriptor::setAsGlobalVendorTagDescriptor(desc);
                if (err != OK) {
                    ALOGE("%s: Failed to set vendor tag descriptors, received error %s (%d)",
                            __FUNCTION__, strerror(-err), err);
                }
            } else {
                sp<VendorTagDescriptorCache> cache =
                        new VendorTagDescriptorCache();
                binder::Status res =
                        mCameraService->getCameraVendorTagCache(
                                /*out*/cache.get());
                if (res.serviceSpecificErrorCode() ==
                        hardware::ICameraService::ERROR_DISCONNECTED) {
                    // No camera module available, not an error on devices with no cameras
                    VendorTagDescriptorCache::clearGlobalVendorTagCache();
                } else if (res.isOk()) {
                    status_t err =
                            VendorTagDescriptorCache::setAsGlobalVendorTagCache(
                                    cache);
                    if (err != OK) {
                        ALOGE("%s: Failed to set vendor tag cache,"
                                "received error %s (%d)", __FUNCTION__,
                                strerror(-err), err);
                    }
                } else {
                    VendorTagDescriptorCache::clearGlobalVendorTagCache();
                    ALOGE("%s: Failed to setup vendor tag cache: %s",
                            __FUNCTION__, res.toString8().string());
                }
            }
        } else if (ret.serviceSpecificErrorCode() ==
                hardware::ICameraService::ERROR_DEPRECATED_HAL) {
            ALOGW("%s: Camera HAL too old; does not support vendor tags",
                    __FUNCTION__);
            VendorTagDescriptor::clearGlobalVendorTagDescriptor();
        } else {
            ALOGE("%s: Failed to get vendor tag descriptors: %s",
                    __FUNCTION__, ret.toString8().string());
        }
    }
    ALOGE_IF(mCameraService == nullptr, "no CameraService!?");
    return mCameraService;
}

void CameraManagerGlobal::DeathNotifier::binderDied(const wp<IBinder>&)
{
    ALOGE("Camera service binderDied!");
    sp<CameraManagerGlobal> cm = mCameraManager.promote();
    if (cm != nullptr) {
        AutoMutex lock(cm->mLock);
        for (auto& pair : cm->mDeviceStatusMap) {
            const String8 &cameraId = pair.first;
            cm->onStatusChangedLocked(
                    CameraServiceListener::STATUS_NOT_PRESENT, cameraId);
        }
        cm->mCameraService.clear();
        // TODO: consider adding re-connect call here?
    }
}

void CameraManagerGlobal::registerExtendedAvailabilityCallback(
        const ACameraManager_ExtendedAvailabilityCallbacks *callback) {
    Mutex::Autolock _l(mLock);
    Callback cb(callback);
    mCallbacks.insert(cb);
}

void CameraManagerGlobal::unregisterExtendedAvailabilityCallback(
        const ACameraManager_ExtendedAvailabilityCallbacks *callback) {
    Mutex::Autolock _l(mLock);
    Callback cb(callback);
    mCallbacks.erase(cb);
}

void CameraManagerGlobal::registerAvailabilityCallback(
        const ACameraManager_AvailabilityCallbacks *callback) {
    Mutex::Autolock _l(mLock);
    Callback cb(callback);
    auto pair = mCallbacks.insert(cb);
    // Send initial callbacks if callback is newly registered
    if (pair.second) {
        for (auto& pair : mDeviceStatusMap) {
            const String8& cameraId = pair.first;
            int32_t status = pair.second;

            sp<AMessage> msg = new AMessage(kWhatSendSingleCallback, mHandler);
            ACameraManager_AvailabilityCallback cb = isStatusAvailable(status) ?
                    callback->onCameraAvailable : callback->onCameraUnavailable;
            msg->setPointer(kCallbackFpKey, (void *) cb);
            msg->setPointer(kContextKey, callback->context);
            msg->setString(kCameraIdKey, AString(cameraId));
            msg->post();
        }
    }
}

void CameraManagerGlobal::unregisterAvailabilityCallback(
        const ACameraManager_AvailabilityCallbacks *callback) {
    Mutex::Autolock _l(mLock);
    Callback cb(callback);
    mCallbacks.erase(cb);
}

void CameraManagerGlobal::getCameraIdList(std::vector<String8>* cameraIds) {
    // Ensure that we have initialized/refreshed the list of available devices
    auto cs = getCameraService();
    Mutex::Autolock _l(mLock);

    for(auto& deviceStatus : mDeviceStatusMap) {
        if (deviceStatus.second == hardware::ICameraServiceListener::STATUS_NOT_PRESENT ||
                deviceStatus.second == hardware::ICameraServiceListener::STATUS_ENUMERATING) {
            continue;
        }
        bool camera2Support = false;
        binder::Status serviceRet = cs->supportsCameraApi(String16(deviceStatus.first),
                hardware::ICameraService::API_VERSION_2, &camera2Support);
        if (!serviceRet.isOk() || !camera2Support) {
            continue;
        }
        cameraIds->push_back(deviceStatus.first);
    }
}

bool CameraManagerGlobal::validStatus(int32_t status) {
    switch (status) {
        case hardware::ICameraServiceListener::STATUS_NOT_PRESENT:
        case hardware::ICameraServiceListener::STATUS_PRESENT:
        case hardware::ICameraServiceListener::STATUS_ENUMERATING:
        case hardware::ICameraServiceListener::STATUS_NOT_AVAILABLE:
            return true;
        default:
            return false;
    }
}

bool CameraManagerGlobal::isStatusAvailable(int32_t status) {
    switch (status) {
        case hardware::ICameraServiceListener::STATUS_PRESENT:
            return true;
        default:
            return false;
    }
}

void CameraManagerGlobal::CallbackHandler::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSendSingleCallback:
        {
            ACameraManager_AvailabilityCallback cb;
            void* context;
            AString cameraId;
            bool found = msg->findPointer(kCallbackFpKey, (void**) &cb);
            if (!found) {
                ALOGE("%s: Cannot find camera callback fp!", __FUNCTION__);
                return;
            }
            found = msg->findPointer(kContextKey, &context);
            if (!found) {
                ALOGE("%s: Cannot find callback context!", __FUNCTION__);
                return;
            }
            found = msg->findString(kCameraIdKey, &cameraId);
            if (!found) {
                ALOGE("%s: Cannot find camera ID!", __FUNCTION__);
                return;
            }
            (*cb)(context, cameraId.c_str());
            break;
        }
        case kWhatSendSingleAccessCallback:
        {
            ACameraManager_AccessPrioritiesChangedCallback cb;
            void* context;
            AString cameraId;
            bool found = msg->findPointer(kCallbackFpKey, (void**) &cb);
            if (!found) {
                ALOGE("%s: Cannot find camera callback fp!", __FUNCTION__);
                return;
            }
            found = msg->findPointer(kContextKey, &context);
            if (!found) {
                ALOGE("%s: Cannot find callback context!", __FUNCTION__);
                return;
            }
            (*cb)(context);
            break;
        }
        default:
            ALOGE("%s: unknown message type %d", __FUNCTION__, msg->what());
            break;
    }
}

binder::Status CameraManagerGlobal::CameraServiceListener::onCameraAccessPrioritiesChanged() {
    sp<CameraManagerGlobal> cm = mCameraManager.promote();
    if (cm != nullptr) {
        cm->onCameraAccessPrioritiesChanged();
    } else {
        ALOGE("Cannot deliver camera access priority callback. Global camera manager died");
    }
    return binder::Status::ok();
}

binder::Status CameraManagerGlobal::CameraServiceListener::onStatusChanged(
        int32_t status, const String16& cameraId) {
    sp<CameraManagerGlobal> cm = mCameraManager.promote();
    if (cm != nullptr) {
        cm->onStatusChanged(status, String8(cameraId));
    } else {
        ALOGE("Cannot deliver status change. Global camera manager died");
    }
    return binder::Status::ok();
}

void CameraManagerGlobal::onCameraAccessPrioritiesChanged() {
    Mutex::Autolock _l(mLock);
    for (auto cb : mCallbacks) {
        sp<AMessage> msg = new AMessage(kWhatSendSingleAccessCallback, mHandler);
        ACameraManager_AccessPrioritiesChangedCallback cbFp = cb.mAccessPriorityChanged;
        if (cbFp != nullptr) {
            msg->setPointer(kCallbackFpKey, (void *) cbFp);
            msg->setPointer(kContextKey, cb.mContext);
            msg->post();
        }
    }
}

void CameraManagerGlobal::onStatusChanged(
        int32_t status, const String8& cameraId) {
    Mutex::Autolock _l(mLock);
    onStatusChangedLocked(status, cameraId);
}

void CameraManagerGlobal::onStatusChangedLocked(
        int32_t status, const String8& cameraId) {
    if (!validStatus(status)) {
        ALOGE("%s: Invalid status %d", __FUNCTION__, status);
        return;
    }

    bool firstStatus = (mDeviceStatusMap.count(cameraId) == 0);
    int32_t oldStatus = firstStatus ?
            status : // first status
            mDeviceStatusMap[cameraId];

    if (!firstStatus &&
            isStatusAvailable(status) == isStatusAvailable(oldStatus)) {
        // No status update. No need to send callback
        return;
    }

    // Iterate through all registered callbacks
    mDeviceStatusMap[cameraId] = status;
    for (auto cb : mCallbacks) {
        sp<AMessage> msg = new AMessage(kWhatSendSingleCallback, mHandler);
        ACameraManager_AvailabilityCallback cbFp = isStatusAvailable(status) ?
                cb.mAvailable : cb.mUnavailable;
        msg->setPointer(kCallbackFpKey, (void *) cbFp);
        msg->setPointer(kContextKey, cb.mContext);
        msg->setString(kCameraIdKey, AString(cameraId));
        msg->post();
    }
    if (status == hardware::ICameraServiceListener::STATUS_NOT_PRESENT) {
        mDeviceStatusMap.erase(cameraId);
    }
}

} // namespace acam
} // namespace android

/**
 * ACameraManger Implementation
 */
camera_status_t
ACameraManager::getCameraIdList(ACameraIdList** cameraIdList) {
    Mutex::Autolock _l(mLock);

    std::vector<String8> idList;
    CameraManagerGlobal::getInstance().getCameraIdList(&idList);

    int numCameras = idList.size();
    ACameraIdList *out = new ACameraIdList;
    if (!out) {
        ALOGE("Allocate memory for ACameraIdList failed!");
        return ACAMERA_ERROR_NOT_ENOUGH_MEMORY;
    }
    out->numCameras = numCameras;
    out->cameraIds = new const char*[numCameras];
    if (!out->cameraIds) {
        ALOGE("Allocate memory for ACameraIdList failed!");
        deleteCameraIdList(out);
        return ACAMERA_ERROR_NOT_ENOUGH_MEMORY;
    }
    for (int i = 0; i < numCameras; i++) {
        const char* src = idList[i].string();
        size_t dstSize = strlen(src) + 1;
        char* dst = new char[dstSize];
        if (!dst) {
            ALOGE("Allocate memory for ACameraIdList failed!");
            deleteCameraIdList(out);
            return ACAMERA_ERROR_NOT_ENOUGH_MEMORY;
        }
        strlcpy(dst, src, dstSize);
        out->cameraIds[i] = dst;
    }
    *cameraIdList = out;
    return ACAMERA_OK;
}

void
ACameraManager::deleteCameraIdList(ACameraIdList* cameraIdList) {
    if (cameraIdList != nullptr) {
        if (cameraIdList->cameraIds != nullptr) {
            for (int i = 0; i < cameraIdList->numCameras; i ++) {
                if (cameraIdList->cameraIds[i] != nullptr) {
                    delete[] cameraIdList->cameraIds[i];
                }
            }
            delete[] cameraIdList->cameraIds;
        }
        delete cameraIdList;
    }
}

camera_status_t ACameraManager::getCameraCharacteristics(
        const char* cameraIdStr, sp<ACameraMetadata>* characteristics) {
    Mutex::Autolock _l(mLock);

    sp<hardware::ICameraService> cs = CameraManagerGlobal::getInstance().getCameraService();
    if (cs == nullptr) {
        ALOGE("%s: Cannot reach camera service!", __FUNCTION__);
        return ACAMERA_ERROR_CAMERA_DISCONNECTED;
    }
    CameraMetadata rawMetadata;
    binder::Status serviceRet = cs->getCameraCharacteristics(String16(cameraIdStr), &rawMetadata);
    if (!serviceRet.isOk()) {
        switch(serviceRet.serviceSpecificErrorCode()) {
            case hardware::ICameraService::ERROR_DISCONNECTED:
                ALOGE("%s: Camera %s has been disconnected", __FUNCTION__, cameraIdStr);
                return ACAMERA_ERROR_CAMERA_DISCONNECTED;
            case hardware::ICameraService::ERROR_ILLEGAL_ARGUMENT:
                ALOGE("%s: Camera ID %s does not exist!", __FUNCTION__, cameraIdStr);
                return ACAMERA_ERROR_INVALID_PARAMETER;
            default:
                ALOGE("Get camera characteristics from camera service failed: %s",
                        serviceRet.toString8().string());
                return ACAMERA_ERROR_UNKNOWN; // should not reach here
        }
    }

    *characteristics = new ACameraMetadata(
            rawMetadata.release(), ACameraMetadata::ACM_CHARACTERISTICS);
    return ACAMERA_OK;
}

camera_status_t
ACameraManager::openCamera(
        const char* cameraId,
        ACameraDevice_StateCallbacks* callback,
        /*out*/ACameraDevice** outDevice) {
    sp<ACameraMetadata> chars;
    camera_status_t ret = getCameraCharacteristics(cameraId, &chars);
    Mutex::Autolock _l(mLock);
    if (ret != ACAMERA_OK) {
        ALOGE("%s: cannot get camera characteristics for camera %s. err %d",
                __FUNCTION__, cameraId, ret);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    ACameraDevice* device = new ACameraDevice(cameraId, callback, chars);

    sp<hardware::ICameraService> cs = CameraManagerGlobal::getInstance().getCameraService();
    if (cs == nullptr) {
        ALOGE("%s: Cannot reach camera service!", __FUNCTION__);
        delete device;
        return ACAMERA_ERROR_CAMERA_DISCONNECTED;
    }

    sp<hardware::camera2::ICameraDeviceCallbacks> callbacks = device->getServiceCallback();
    sp<hardware::camera2::ICameraDeviceUser> deviceRemote;
    // No way to get package name from native.
    // Send a zero length package name and let camera service figure it out from UID
    binder::Status serviceRet = cs->connectDevice(
            callbacks, String16(cameraId), String16(""),
            hardware::ICameraService::USE_CALLING_UID, /*out*/&deviceRemote);

    if (!serviceRet.isOk()) {
        ALOGE("%s: connect camera device failed: %s", __FUNCTION__, serviceRet.toString8().string());
        // Convert serviceRet to camera_status_t
        switch(serviceRet.serviceSpecificErrorCode()) {
            case hardware::ICameraService::ERROR_DISCONNECTED:
                ret = ACAMERA_ERROR_CAMERA_DISCONNECTED;
                break;
            case hardware::ICameraService::ERROR_CAMERA_IN_USE:
                ret = ACAMERA_ERROR_CAMERA_IN_USE;
                break;
            case hardware::ICameraService::ERROR_MAX_CAMERAS_IN_USE:
                ret = ACAMERA_ERROR_MAX_CAMERA_IN_USE;
                break;
            case hardware::ICameraService::ERROR_ILLEGAL_ARGUMENT:
                ret = ACAMERA_ERROR_INVALID_PARAMETER;
                break;
            case hardware::ICameraService::ERROR_DEPRECATED_HAL:
                // Should not reach here since we filtered legacy HALs earlier
                ret = ACAMERA_ERROR_INVALID_PARAMETER;
                break;
            case hardware::ICameraService::ERROR_DISABLED:
                ret = ACAMERA_ERROR_CAMERA_DISABLED;
                break;
            case hardware::ICameraService::ERROR_PERMISSION_DENIED:
                ret = ACAMERA_ERROR_PERMISSION_DENIED;
                break;
            case hardware::ICameraService::ERROR_INVALID_OPERATION:
            default:
                ret = ACAMERA_ERROR_UNKNOWN;
                break;
        }

        delete device;
        return ret;
    }
    if (deviceRemote == nullptr) {
        ALOGE("%s: connect camera device failed! remote device is null", __FUNCTION__);
        delete device;
        return ACAMERA_ERROR_CAMERA_DISCONNECTED;
    }
    device->setRemoteDevice(deviceRemote);
    *outDevice = device;
    return ACAMERA_OK;
}

ACameraManager::~ACameraManager() {

}
