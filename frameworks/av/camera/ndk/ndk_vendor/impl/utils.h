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

#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <android/frameworks/cameraservice/device/2.0/ICameraDeviceUser.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <camera/NdkCameraDevice.h>
#include <CameraMetadata.h>
#include <hardware/camera3.h>

#ifndef CAMERA_NDK_VENDOR_H
#define CAMERA_NDK_VENDOR_H

using android::hardware::hidl_vec;
using android::hardware::hidl_handle;

namespace android {
namespace acam {
namespace utils {

using CameraMetadata = hardware::camera::common::V1_0::helper::CameraMetadata;
using HCameraMetadata  = frameworks::cameraservice::service::V2_0::CameraMetadata;
using Status = frameworks::cameraservice::common::V2_0::Status;
using TemplateId = frameworks::cameraservice::device::V2_0::TemplateId;
using PhysicalCameraSettings = frameworks::cameraservice::device::V2_0::PhysicalCameraSettings;
using HRotation = frameworks::cameraservice::device::V2_0::OutputConfiguration::Rotation;
using OutputConfiguration = frameworks::cameraservice::device::V2_0::OutputConfiguration;

// Utility class so that CaptureRequest can be stored by sp<>
struct CaptureRequest : public RefBase {
  frameworks::cameraservice::device::V2_0::CaptureRequest mCaptureRequest;
  std::vector<native_handle_t *> mSurfaceList;
  //Physical camera settings metadata is stored here, since the capture request
  //might not contain it. That's since, fmq might have consumed it.
  hidl_vec<PhysicalCameraSettings> mPhysicalCameraSettings;
};

bool areWindowNativeHandlesEqual(hidl_vec<hidl_handle> handles1, hidl_vec<hidl_handle>handles2);

bool areWindowNativeHandlesLessThan(hidl_vec<hidl_handle> handles1, hidl_vec<hidl_handle>handles2);

bool isWindowNativeHandleEqual(const native_handle_t *nh1, const native_handle_t *nh2);

bool isWindowNativeHandleLessThan(const native_handle_t *nh1, const native_handle_t *nh2);

// Convenience wrapper over isWindowNativeHandleLessThan and isWindowNativeHandleEqual
bool isWindowNativeHandleGreaterThan(const native_handle_t *nh1, const native_handle_t *nh2);

// Utility class so the native_handle_t can be compared with  its contents instead
// of just raw pointer comparisons.
struct native_handle_ptr_wrapper {
    native_handle_t *mWindow = nullptr;

    native_handle_ptr_wrapper(native_handle_t *nh) : mWindow(nh) { }

    native_handle_ptr_wrapper() = default;

    operator native_handle_t *() const { return mWindow; }

    bool operator ==(const native_handle_ptr_wrapper other) const {
        return isWindowNativeHandleEqual(mWindow, other.mWindow);
    }

    bool operator != (const native_handle_ptr_wrapper& other) const {
        return !isWindowNativeHandleEqual(mWindow, other.mWindow);
    }

    bool operator < (const native_handle_ptr_wrapper& other) const {
        return isWindowNativeHandleLessThan(mWindow, other.mWindow);
    }

    bool operator > (const native_handle_ptr_wrapper& other) const {
        return !isWindowNativeHandleGreaterThan(mWindow, other.mWindow);
    }

};

// Wrapper around OutputConfiguration. This is needed since HIDL
// OutputConfiguration is auto-generated and marked final. Therefore, operator
// overloads outside the class, will not get picked by clang while trying to
// store OutputConfiguration in maps/sets.
struct OutputConfigurationWrapper {
    OutputConfiguration mOutputConfiguration;

    operator const OutputConfiguration &() const {
        return mOutputConfiguration;
    }

    OutputConfigurationWrapper() {
        mOutputConfiguration.rotation = OutputConfiguration::Rotation::R0;
        // The ndk currently doesn't support deferred surfaces
        mOutputConfiguration.isDeferred = false;
        mOutputConfiguration.width = 0;
        mOutputConfiguration.height = 0;
        // ndk doesn't support inter OutputConfiguration buffer sharing.
        mOutputConfiguration.windowGroupId = -1;
    };

    OutputConfigurationWrapper(OutputConfiguration &outputConfiguration)
            : mOutputConfiguration((outputConfiguration)) { }

    bool operator ==(const OutputConfiguration &other) const {
        const OutputConfiguration &self = mOutputConfiguration;
        return self.rotation == other.rotation && self.windowGroupId == other.windowGroupId &&
                self.physicalCameraId == other.physicalCameraId && self.width == other.width &&
                self.height == other.height && self.isDeferred == other.isDeferred &&
                areWindowNativeHandlesEqual(self.windowHandles, other.windowHandles);
    }

    bool operator < (const OutputConfiguration &other) const {
        if (*this == other) {
            return false;
        }
        const OutputConfiguration &self = mOutputConfiguration;
        if (self.windowGroupId != other.windowGroupId) {
            return self.windowGroupId < other.windowGroupId;
        }

        if (self.width != other.width) {
            return self.width < other.width;
        }

        if (self.height != other.height) {
            return self.height < other.height;
        }

        if (self.rotation != other.rotation) {
            return static_cast<uint32_t>(self.rotation) < static_cast<uint32_t>(other.rotation);
        }

        if (self.isDeferred != other.isDeferred) {
            return self.isDeferred < other.isDeferred;
        }

        if (self.physicalCameraId != other.physicalCameraId) {
            return self.physicalCameraId < other.physicalCameraId;
        }
        return areWindowNativeHandlesLessThan(self.windowHandles, other.windowHandles);
    }

    bool operator != (const OutputConfiguration &other) const {
        return !(*this == other);
    }

    bool operator > (const OutputConfiguration &other) const {
        return (*this != other) && !(*this < other);
    }
};

// Convert CaptureRequest wrappable by sp<> to hidl CaptureRequest.
frameworks::cameraservice::device::V2_0::CaptureRequest convertToHidl(
    const CaptureRequest *captureRequest);

HRotation convertToHidl(int rotation);

bool convertFromHidlCloned(const HCameraMetadata &metadata, CameraMetadata *rawMetadata);

// Note: existing data in dst will be gone.
void convertToHidl(const camera_metadata_t *src, HCameraMetadata* dst, bool shouldOwn = false);

TemplateId convertToHidl(ACameraDevice_request_template templateId);

camera_status_t convertFromHidl(Status status);

} // namespace utils
} // namespace acam
} // namespace android

#endif // CAMERA_NDK_VENDOR_H
