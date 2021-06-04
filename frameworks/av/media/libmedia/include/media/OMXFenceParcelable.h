/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef _OMX_FENCE_PARCELABLE_
#define _OMX_FENCE_PARCELABLE_

#include <binder/Parcel.h>

namespace android {

struct OMXFenceParcelable;

// This is needed temporarily for the OMX HIDL transition.
namespace hardware {
    struct hidl_handle;
namespace media { namespace omx { namespace V1_0 {
namespace implementation {
    void wrapAs(::android::OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
    bool convertTo(::android::OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
}
namespace utils {
    void wrapAs(::android::OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
    bool convertTo(::android::OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
}
}}}}

struct OMXFenceParcelable : public Parcelable {
    OMXFenceParcelable() : mFenceFd(-1) {}
    OMXFenceParcelable(int fenceFd) : mFenceFd(fenceFd) {}

    int get() const { return mFenceFd; }

    status_t readFromParcel(const Parcel* parcel) override;
    status_t writeToParcel(Parcel* parcel) const override;

private:
    // Disable copy ctor and operator=
    OMXFenceParcelable(const OMXFenceParcelable &);
    OMXFenceParcelable &operator=(const OMXFenceParcelable &);

    int mFenceFd;

    // This is needed temporarily for OMX HIDL transition.
    friend void (::android::hardware::media::omx::V1_0::implementation::
            wrapAs)(OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
    friend bool (::android::hardware::media::omx::V1_0::implementation::
            convertTo)(OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
    friend void (::android::hardware::media::omx::V1_0::utils::
            wrapAs)(OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
    friend bool (::android::hardware::media::omx::V1_0::utils::
            convertTo)(OMXFenceParcelable* l,
            ::android::hardware::hidl_handle const& t);
};

inline status_t OMXFenceParcelable::readFromParcel(const Parcel* parcel) {
    int32_t haveFence;
    status_t err = parcel->readInt32(&haveFence);
    if (err == OK && haveFence) {
        int fd = ::dup(parcel->readFileDescriptor());
        if (fd < 0) {
            return fd;
        }
        mFenceFd = fd;
    }
    return err;
}

inline status_t OMXFenceParcelable::writeToParcel(Parcel* parcel) const {
    status_t err = parcel->writeInt32(mFenceFd >= 0);
    if (err == OK && mFenceFd >= 0) {
        err = parcel->writeFileDescriptor(mFenceFd, true /* takeOwnership */);
    }
    return err;
}

} // namespace android

#endif // _OMX_FENCE_PARCELABLE_
