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

#ifndef CLEARKEY_ANDROID_HARDWARE_DRM_V1_1_TYPECONVERT
#define CLEARKEY_ANDROID_HARDWARE_DRM_V1_1_TYPECONVERT

#include <vector>

#include <android/hardware/drm/1.0/types.h>

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_vec;

template<typename T> const hidl_vec<T> toHidlVec(const std::vector<T> &vec) {
    hidl_vec<T> hVec;
    hVec.setToExternal(const_cast<T *>(vec.data()), vec.size());
    return hVec;
}

template<typename T> hidl_vec<T> toHidlVec(std::vector<T> &vec) {
    hidl_vec<T> hVec;
    hVec.setToExternal(vec.data(), vec.size());
    return hVec;
}

template<typename T> const std::vector<T> toVector(const hidl_vec<T> &hVec) {
    std::vector<T> vec;
    vec.assign(hVec.data(), hVec.data() + hVec.size());
    return *const_cast<const std::vector<T> *>(&vec);
}

template<typename T> std::vector<T> toVector(hidl_vec<T> &hVec) {
    std::vector<T> vec;
    vec.assign(hVec.data(), hVec.data() + hVec.size());
    return vec;
}

template<typename T, size_t SIZE> const std::vector<T> toVector(
        const hidl_array<T, SIZE> &hArray) {
    std::vector<T> vec;
    vec.assign(hArray.data(), hArray.data() + hArray.size());
    return vec;
}

template<typename T, size_t SIZE> std::vector<T> toVector(
        hidl_array<T, SIZE> &hArray) {
    std::vector<T> vec;
    vec.assign(hArray.data(), hArray.data() + hArray.size());
    return vec;
}

inline Status toStatus_1_0(Status_V1_2 status) {
  switch (status) {
    case Status_V1_2::ERROR_DRM_INSUFFICIENT_SECURITY:
    case Status_V1_2::ERROR_DRM_FRAME_TOO_LARGE:
    case Status_V1_2::ERROR_DRM_SESSION_LOST_STATE:
      return Status::ERROR_DRM_UNKNOWN;
    default:
      return static_cast<Status>(status);
  }
}

}  // namespace clearkey
}  // namespace V1_2
}  // namespace drm
}  // namespace hardware
}  // namespace android

#endif // CLEARKEY_ANDROID_HARDWARE_DRM_V1_1_TYPECONVERT
