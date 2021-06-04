/*
**
** Copyright (C) 2013, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "CameraBase"
#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/Mutex.h>
#include <cutils/properties.h>

#include <android/hardware/ICameraService.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/IMemory.h>

#include <camera/CameraBase.h>

// needed to instantiate
#include <camera/Camera.h>

#include <system/camera_metadata.h>

namespace android {

namespace hardware {

status_t CameraInfo::writeToParcel(android::Parcel* parcel) const {
    status_t res;
    res = parcel->writeInt32(facing);
    if (res != OK) return res;
    res = parcel->writeInt32(orientation);
    return res;
}

status_t CameraInfo::readFromParcel(const android::Parcel* parcel) {
    status_t res;
    res = parcel->readInt32(&facing);
    if (res != OK) return res;
    res = parcel->readInt32(&orientation);
    return res;
}

status_t CameraStatus::writeToParcel(android::Parcel* parcel) const {
    auto res = parcel->writeString16(String16(cameraId));
    if (res != OK) return res;

    res = parcel->writeInt32(status);
    return res;
}

status_t CameraStatus::readFromParcel(const android::Parcel* parcel) {
    String16 tempCameraId;
    auto res = parcel->readString16(&tempCameraId);
    if (res != OK) return res;
    cameraId = String8(tempCameraId);

    res = parcel->readInt32(&status);
    return res;
}

} // namespace hardware

namespace {
    sp<::android::hardware::ICameraService> gCameraService;
    const int                 kCameraServicePollDelay = 500000; // 0.5s
    const char*               kCameraServiceName      = "media.camera";

    Mutex                     gLock;

    class DeathNotifier : public IBinder::DeathRecipient
    {
    public:
        DeathNotifier() {
        }

        virtual void binderDied(const wp<IBinder>& /*who*/) {
            ALOGV("binderDied");
            Mutex::Autolock _l(gLock);
            gCameraService.clear();
            ALOGW("Camera service died!");
        }
    };

    sp<DeathNotifier>         gDeathNotifier;
}; // namespace anonymous

///////////////////////////////////////////////////////////
// CameraBase definition
///////////////////////////////////////////////////////////

// establish binder interface to camera service
template <typename TCam, typename TCamTraits>
const sp<::android::hardware::ICameraService> CameraBase<TCam, TCamTraits>::getCameraService()
{
    Mutex::Autolock _l(gLock);
    if (gCameraService.get() == 0) {
        char value[PROPERTY_VALUE_MAX];
        property_get("config.disable_cameraservice", value, "0");
        if (strncmp(value, "0", 2) != 0 && strncasecmp(value, "false", 6) != 0) {
            return gCameraService;
        }

        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16(kCameraServiceName));
            if (binder != 0) {
                break;
            }
            ALOGW("CameraService not published, waiting...");
            usleep(kCameraServicePollDelay);
        } while(true);
        if (gDeathNotifier == NULL) {
            gDeathNotifier = new DeathNotifier();
        }
        binder->linkToDeath(gDeathNotifier);
        gCameraService = interface_cast<::android::hardware::ICameraService>(binder);
    }
    ALOGE_IF(gCameraService == 0, "no CameraService!?");
    return gCameraService;
}

template <typename TCam, typename TCamTraits>
sp<TCam> CameraBase<TCam, TCamTraits>::connect(int cameraId,
                                               const String16& clientPackageName,
                                               int clientUid, int clientPid)
{
    ALOGV("%s: connect", __FUNCTION__);
    sp<TCam> c = new TCam(cameraId);
    sp<TCamCallbacks> cl = c;
    const sp<::android::hardware::ICameraService> cs = getCameraService();

    binder::Status ret;
    if (cs != nullptr) {
        TCamConnectService fnConnectService = TCamTraits::fnConnectService;
        ret = (cs.get()->*fnConnectService)(cl, cameraId, clientPackageName, clientUid,
                                               clientPid, /*out*/ &c->mCamera);
    }
    if (ret.isOk() && c->mCamera != nullptr) {
        IInterface::asBinder(c->mCamera)->linkToDeath(c);
        c->mStatus = NO_ERROR;
    } else {
        ALOGW("An error occurred while connecting to camera %d: %s", cameraId,
                (cs == nullptr) ? "Service not available" : ret.toString8().string());
        c.clear();
    }
    return c;
}

template <typename TCam, typename TCamTraits>
void CameraBase<TCam, TCamTraits>::disconnect()
{
    ALOGV("%s: disconnect", __FUNCTION__);
    if (mCamera != 0) {
        mCamera->disconnect();
        IInterface::asBinder(mCamera)->unlinkToDeath(this);
        mCamera = 0;
    }
    ALOGV("%s: disconnect (done)", __FUNCTION__);
}

template <typename TCam, typename TCamTraits>
CameraBase<TCam, TCamTraits>::CameraBase(int cameraId) :
    mStatus(UNKNOWN_ERROR),
    mCameraId(cameraId)
{
}

template <typename TCam, typename TCamTraits>
CameraBase<TCam, TCamTraits>::~CameraBase()
{
}

template <typename TCam, typename TCamTraits>
sp<typename TCamTraits::TCamUser> CameraBase<TCam, TCamTraits>::remote()
{
    return mCamera;
}

template <typename TCam, typename TCamTraits>
status_t CameraBase<TCam, TCamTraits>::getStatus()
{
    return mStatus;
}

template <typename TCam, typename TCamTraits>
void CameraBase<TCam, TCamTraits>::binderDied(const wp<IBinder>& /*who*/) {
    ALOGW("mediaserver's remote binder Camera object died");
    notifyCallback(CAMERA_MSG_ERROR, CAMERA_ERROR_SERVER_DIED, /*ext2*/0);
}

template <typename TCam, typename TCamTraits>
void CameraBase<TCam, TCamTraits>::setListener(const sp<TCamListener>& listener)
{
    Mutex::Autolock _l(mLock);
    mListener = listener;
}

// callback from camera service
template <typename TCam, typename TCamTraits>
void CameraBase<TCam, TCamTraits>::notifyCallback(int32_t msgType,
                                                  int32_t ext1,
                                                  int32_t ext2)
{
    sp<TCamListener> listener;
    {
        Mutex::Autolock _l(mLock);
        listener = mListener;
    }
    if (listener != NULL) {
        listener->notify(msgType, ext1, ext2);
    }
}

template <typename TCam, typename TCamTraits>
int CameraBase<TCam, TCamTraits>::getNumberOfCameras() {
    const sp<::android::hardware::ICameraService> cs = getCameraService();

    if (!cs.get()) {
        // as required by the public Java APIs
        return 0;
    }
    int32_t count;
    binder::Status res = cs->getNumberOfCameras(
            ::android::hardware::ICameraService::CAMERA_TYPE_BACKWARD_COMPATIBLE,
            &count);
    if (!res.isOk()) {
        ALOGE("Error reading number of cameras: %s",
                res.toString8().string());
        count = 0;
    }
    return count;
}

// this can be in BaseCamera but it should be an instance method
template <typename TCam, typename TCamTraits>
status_t CameraBase<TCam, TCamTraits>::getCameraInfo(int cameraId,
        struct hardware::CameraInfo* cameraInfo) {
    const sp<::android::hardware::ICameraService> cs = getCameraService();
    if (cs == 0) return UNKNOWN_ERROR;
    binder::Status res = cs->getCameraInfo(cameraId, cameraInfo);
    return res.isOk() ? OK : res.serviceSpecificErrorCode();
}

template class CameraBase<Camera>;

} // namespace android
