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

#ifndef _ACAMERA_MANAGER_H
#define _ACAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>

#include <android-base/parseint.h>
#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>

#include <CameraMetadata.h>
#include <utils/StrongPointer.h>
#include <utils/Mutex.h>

#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

#include <set>
#include <map>

namespace android {
namespace acam {

using ICameraService = frameworks::cameraservice::service::V2_0::ICameraService;
using CameraDeviceStatus = frameworks::cameraservice::service::V2_0::CameraDeviceStatus;
using ICameraServiceListener = frameworks::cameraservice::service::V2_0::ICameraServiceListener;
using CameraStatusAndId = frameworks::cameraservice::service::V2_0::CameraStatusAndId;
using Status = frameworks::cameraservice::common::V2_0::Status;
using VendorTagSection = frameworks::cameraservice::common::V2_0::VendorTagSection;
using VendorTag = frameworks::cameraservice::common::V2_0::VendorTag;
using IBase = android::hidl::base::V1_0::IBase;
using android::hardware::hidl_string;
using hardware::Void;

/**
 * Per-process singleton instance of CameraManger. Shared by all ACameraManager
 * instances. Created when first ACameraManager is created and destroyed when
 * all ACameraManager instances are deleted.
 *
 * TODO: maybe CameraManagerGlobal is better suited in libcameraclient?
 */
class CameraManagerGlobal final : public RefBase {
  public:
    static CameraManagerGlobal& getInstance();
    sp<ICameraService> getCameraService();

    void registerAvailabilityCallback(
            const ACameraManager_AvailabilityCallbacks *callback);
    void unregisterAvailabilityCallback(
            const ACameraManager_AvailabilityCallbacks *callback);

    void registerExtendedAvailabilityCallback(
            const ACameraManager_ExtendedAvailabilityCallbacks* /*callback*/) {}
    void unregisterExtendedAvailabilityCallback(
            const ACameraManager_ExtendedAvailabilityCallbacks* /*callback*/) {}

    /**
     * Return camera IDs that support camera2
     */
    void getCameraIdList(std::vector<hidl_string> *cameraIds);

  private:
    sp<ICameraService> mCameraService;
    const int          kCameraServicePollDelay = 500000; // 0.5s
    Mutex              mLock;
    class DeathNotifier : public android::hardware::hidl_death_recipient {
      public:
        explicit DeathNotifier(CameraManagerGlobal* cm) : mCameraManager(cm) {}
      protected:
        // IBinder::DeathRecipient implementation
        virtual void serviceDied(uint64_t cookie, const wp<IBase> &who);
      private:
        const wp<CameraManagerGlobal> mCameraManager;
    };
    sp<DeathNotifier> mDeathNotifier;

    class CameraServiceListener final : public ICameraServiceListener {
      public:
        explicit CameraServiceListener(CameraManagerGlobal* cm) : mCameraManager(cm) {}
        android::hardware::Return<void> onStatusChanged(
            const CameraStatusAndId &statusAndId) override;

      private:
        const wp<CameraManagerGlobal> mCameraManager;
    };
    sp<CameraServiceListener> mCameraServiceListener;

    // Wrapper of ACameraManager_AvailabilityCallbacks so we can store it in std::set
    struct Callback {
        explicit Callback(const ACameraManager_AvailabilityCallbacks *callback) :
            mAvailable(callback->onCameraAvailable),
            mUnavailable(callback->onCameraUnavailable),
            mContext(callback->context) {}

        bool operator == (const Callback& other) const {
            return (mAvailable == other.mAvailable &&
                    mUnavailable == other.mUnavailable &&
                    mContext == other.mContext);
        }
        bool operator != (const Callback& other) const {
            return !(*this == other);
        }
        bool operator < (const Callback& other) const {
            if (*this == other) return false;
            if (mContext != other.mContext) return mContext < other.mContext;
            if (mAvailable != other.mAvailable) return mAvailable < other.mAvailable;
            return mUnavailable < other.mUnavailable;
        }
        bool operator > (const Callback& other) const {
            return (*this != other && !(*this < other));
        }
        ACameraManager_AvailabilityCallback mAvailable;
        ACameraManager_AvailabilityCallback mUnavailable;
        void*                               mContext;
    };
    std::set<Callback> mCallbacks;

    // definition of handler and message
    enum {
        kWhatSendSingleCallback
    };
    static const char* kCameraIdKey;
    static const char* kCallbackFpKey;
    static const char* kContextKey;
    class CallbackHandler : public AHandler {
      public:
        CallbackHandler() {}
        void onMessageReceived(const sp<AMessage> &msg) override;
    };
    sp<CallbackHandler> mHandler;
    sp<ALooper>         mCbLooper; // Looper thread where callbacks actually happen on

    void onStatusChanged(const CameraStatusAndId &statusAndId);
    void onStatusChangedLocked(const CameraStatusAndId &statusAndId);
    bool setupVendorTags();

    // Utils for status
    static bool validStatus(CameraDeviceStatus status);
    static bool isStatusAvailable(CameraDeviceStatus status);

    // The sort logic must match the logic in
    // libcameraservice/common/CameraProviderManager.cpp::getAPI1CompatibleCameraDeviceIds
    struct CameraIdComparator {
        bool operator()(const hidl_string& a, const hidl_string& b) const {
            uint32_t aUint = 0, bUint = 0;
            bool aIsUint = base::ParseUint(a.c_str(), &aUint);
            bool bIsUint = base::ParseUint(b.c_str(), &bUint);

            // Uint device IDs first
            if (aIsUint && bIsUint) {
                return aUint < bUint;
            } else if (aIsUint) {
                return true;
            } else if (bIsUint) {
                return false;
            }
            // Simple string compare if both id are not uint
            return a < b;
        }
    };

    // Map camera_id -> status
    std::map<hidl_string, CameraDeviceStatus, CameraIdComparator> mDeviceStatusMap;

    // For the singleton instance
    static Mutex sLock;
    static CameraManagerGlobal* sInstance;
    CameraManagerGlobal() {};
    ~CameraManagerGlobal();
};

} // namespace acam;
} // namespace android;

/**
 * ACameraManager opaque struct definition
 * Leave outside of android namespace because it's NDK struct
 */
struct ACameraManager {
    ACameraManager() :
            mGlobalManager(&(android::acam::CameraManagerGlobal::getInstance())) {}
    ~ACameraManager();
    camera_status_t getCameraIdList(ACameraIdList** cameraIdList);
    static void     deleteCameraIdList(ACameraIdList* cameraIdList);

    camera_status_t getCameraCharacteristics(
            const char* cameraId, android::sp<ACameraMetadata>* characteristics);

    camera_status_t openCamera(const char* cameraId,
                               ACameraDevice_StateCallbacks* callback,
                               /*out*/ACameraDevice** device);
    camera_status_t getTagFromName(const char *cameraId, const char *name, uint32_t *tag);

  private:
    enum {
        kCameraIdListNotInit = -1
    };
    android::Mutex         mLock;
    android::sp<android::acam::CameraManagerGlobal> mGlobalManager;
};

#endif //_ACAMERA_MANAGER_H
