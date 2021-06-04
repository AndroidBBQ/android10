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

#include <hidl/AidlCameraServiceListener.h>
#include <hidl/Convert.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace service {
namespace V2_0 {
namespace implementation {

using hardware::cameraservice::utils::conversion::convertToHidlCameraDeviceStatus;

binder::Status H2BCameraServiceListener::onStatusChanged(
    int32_t status, const ::android::String16& cameraId) {
  HCameraDeviceStatus hCameraDeviceStatus = convertToHidlCameraDeviceStatus(status);
  CameraStatusAndId cameraStatusAndId;
  cameraStatusAndId.deviceStatus = hCameraDeviceStatus;
  cameraStatusAndId.cameraId = String8(cameraId).string();
  auto ret = mBase->onStatusChanged(cameraStatusAndId);
  if (!ret.isOk()) {
      ALOGE("%s OnStatusChanged callback failed due to %s",__FUNCTION__,
            ret.description().c_str());
  }
  return binder::Status::ok();
}

::android::binder::Status H2BCameraServiceListener::onTorchStatusChanged(
    int32_t, const ::android::String16&) {
  // We don't implement onTorchStatusChanged
  return binder::Status::ok();
}

} // implementation
} // V2_0
} // common
} // cameraservice
} // frameworks
} // android
