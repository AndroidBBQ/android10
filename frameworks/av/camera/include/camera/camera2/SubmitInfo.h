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

#ifndef ANDROID_HARDWARE_CAMERA2_UTIL_SUBMITINFO_H
#define ANDROID_HARDWARE_CAMERA2_UTIL_SUBMITINFO_H

#include <binder/Parcel.h>
#include <binder/Parcelable.h>

namespace android {
namespace hardware {
namespace camera2 {
namespace utils {

struct SubmitInfo : public android::Parcelable {
public:

    int32_t mRequestId;
    int64_t mLastFrameNumber;

    virtual status_t writeToParcel(android::Parcel *parcel) const override;
    virtual status_t readFromParcel(const android::Parcel* parcel) override;

};

} // namespace utils
} // namespace camera2
} // namespace hardware
} // namespace android

#endif
