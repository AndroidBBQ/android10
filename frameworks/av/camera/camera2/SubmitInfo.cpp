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


#include "camera/camera2/SubmitInfo.h"

namespace android {
namespace hardware {
namespace camera2 {
namespace utils {

status_t SubmitInfo::writeToParcel(android::Parcel *parcel) const {
    status_t res;
    if (parcel == nullptr) return BAD_VALUE;

    res = parcel->writeInt32(mRequestId);
    if (res != OK) return res;

    res = parcel->writeInt64(mLastFrameNumber);
    return res;
}

status_t SubmitInfo::readFromParcel(const android::Parcel *parcel) {
    status_t res;
    if (parcel == nullptr) return BAD_VALUE;

    res = parcel->readInt32(&mRequestId);
    if (res != OK) return res;

    res = parcel->readInt64(&mLastFrameNumber);
    return res;
}

} // namespace utils
} // namespace camera2
} // namespace hardware
} // namespace android
