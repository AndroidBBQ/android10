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

#ifndef ANDROID_FRAMEWORKS_CAMERASERVICE_V2_0_CAMERASERVICE_H
#define ANDROID_FRAMEWORKS_CAMERASERVICE_V2_0_CAMERASERVICE_H

#include <mutex>
#include <thread>

#include <android/frameworks/cameraservice/common/2.0/types.h>
#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <android/frameworks/cameraservice/service/2.0/types.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>

#include <hidl/Status.h>

#include <CameraService.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace service {
namespace V2_0 {
namespace implementation {

using hardware::hidl_string;
using hardware::ICameraServiceListener;
using hardware::Return;

using HCameraDeviceCallback = frameworks::cameraservice::device::V2_0::ICameraDeviceCallback;
using HCameraMetadata = frameworks::cameraservice::service::V2_0::CameraMetadata;
using HCameraService = frameworks::cameraservice::service::V2_0::ICameraService;
using HCameraServiceListener = frameworks::cameraservice::service::V2_0::ICameraServiceListener;
using HStatus = frameworks::cameraservice::common::V2_0::Status;
using HCameraStatusAndId = frameworks::cameraservice::service::V2_0::CameraStatusAndId;

struct HidlCameraService final : public HCameraService {

    ~HidlCameraService() { };

    // Methods from ::android::frameworks::cameraservice::service::V2.0::ICameraService follow.

    Return<void> connectDevice(const sp<HCameraDeviceCallback>& callback,
                               const hidl_string& cameraId, connectDevice_cb _hidl_cb) override;

    Return<void> addListener(const sp<HCameraServiceListener>& listener,
                             addListener_cb _hidl_cb) override;

    Return<HStatus> removeListener(const sp<HCameraServiceListener>& listener) override;

    Return<void> getCameraCharacteristics(const hidl_string& cameraId,
                                          getCameraCharacteristics_cb _hidl_cb) override;

    Return<void> getCameraVendorTagSections(getCameraVendorTagSections_cb _hidl_cb) override;

    // This method should only be called by the cameraservers main thread to
    // instantiate the hidl cameraserver.
    static sp<HidlCameraService> getInstance(android::CameraService *cs);

private:
    HidlCameraService(android::CameraService *cs) : mAidlICameraService(cs) { };

    sp<hardware::ICameraServiceListener> searchListenerCacheLocked(
        sp<HCameraServiceListener> listener, /*removeIfFound*/ bool shouldRemove = false);

    void addToListenerCacheLocked(sp<HCameraServiceListener> hListener,
                                  sp<hardware::ICameraServiceListener> csListener);

    android::CameraService *const mAidlICameraService = nullptr;

    Mutex mListenerListLock;

    using HIListeners =
        std::pair<sp<HCameraServiceListener>, sp<ICameraServiceListener>>;
    std::list<HIListeners> mListeners;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace service
}  // namespace cameraservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_CAMERASERVICE_V2_0_CAMERASERVICE_H
