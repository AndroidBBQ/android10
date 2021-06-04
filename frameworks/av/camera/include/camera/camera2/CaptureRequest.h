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

#ifndef ANDROID_HARDWARE_PHOTOGRAPHY_CAPTUREREQUEST_H
#define ANDROID_HARDWARE_PHOTOGRAPHY_CAPTUREREQUEST_H

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <binder/Parcelable.h>
#include <camera/CameraMetadata.h>

namespace android {

class Surface;

namespace hardware {
namespace camera2 {

struct CaptureRequest : public Parcelable {

    // those are needed so we can use a forward declaration of Surface, otherwise
    // the type is incomplete when the ctor/dtors are generated. This has the added
    // benefit that ctor/dtors are not inlined, which is good because they're not trivial
    // (because of the vtable and Vector<>)
    CaptureRequest();
    CaptureRequest(const CaptureRequest& rhs);
    CaptureRequest(CaptureRequest&& rhs) noexcept;
    virtual ~CaptureRequest();

    struct PhysicalCameraSettings {
        std::string id;
        CameraMetadata settings;
    };
    std::vector<PhysicalCameraSettings> mPhysicalCameraSettings;

    // Used by NDK client to pass surfaces by stream/surface index.
    bool                    mSurfaceConverted = false;

    // Starting in Android O, create a Surface from Parcel will take one extra
    // IPC call.
    Vector<sp<Surface> >    mSurfaceList;
    // Optional way of passing surface list since passing Surface over binder
    // is expensive. Use the stream/surface index from current output configuration
    // to represent an configured output Surface. When stream/surface index is used,
    // set mSurfaceList to zero length to save unparcel time.
    Vector<int>             mStreamIdxList;
    Vector<int>             mSurfaceIdxList; // per stream surface list index

    bool                    mIsReprocess;

    void*                   mContext; // arbitrary user context from NDK apps, null for java apps

    /**
     * Keep impl up-to-date with CaptureRequest.java in frameworks/base
     */
    // used by cameraserver to receive CaptureRequest from java/NDK client
    status_t                readFromParcel(const android::Parcel* parcel) override;
    // used by NDK client to send CaptureRequest to cameraserver
    status_t                writeToParcel(android::Parcel* parcel) const override;
};

} // namespace camera2
} // namespace hardware

struct CaptureRequest :
        public RefBase, public hardware::camera2::CaptureRequest {
  public:
    // Same as android::hardware::camera2::CaptureRequest, except that you can
    // put this in an sp<>
};

} // namespace android

#endif
