/*
 * Copyright 2016 The Android Open Source Project
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

#include <ui/HdrCapabilities.h>

namespace android {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#endif

HdrCapabilities::~HdrCapabilities() = default;
HdrCapabilities::HdrCapabilities(HdrCapabilities&& other) noexcept = default;
HdrCapabilities& HdrCapabilities::operator=(HdrCapabilities&& other) noexcept = default;

size_t HdrCapabilities::getFlattenedSize() const {
    return  sizeof(mMaxLuminance) +
            sizeof(mMaxAverageLuminance) +
            sizeof(mMinLuminance) +
            sizeof(int32_t) +
            mSupportedHdrTypes.size() * sizeof(ui::Hdr);
}

status_t HdrCapabilities::flatten(void* buffer, size_t size) const {

    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    int32_t* const buf = static_cast<int32_t*>(buffer);
    reinterpret_cast<float&>(buf[0]) = mMaxLuminance;
    reinterpret_cast<float&>(buf[1]) = mMaxAverageLuminance;
    reinterpret_cast<float&>(buf[2]) = mMinLuminance;
    buf[3] = static_cast<int32_t>(mSupportedHdrTypes.size());
    for (size_t i = 0, c = mSupportedHdrTypes.size(); i < c; ++i) {
        buf[4 + i] = static_cast<int32_t>(mSupportedHdrTypes[i]);
    }
    return NO_ERROR;
}

status_t HdrCapabilities::unflatten(void const* buffer, size_t size) {

    size_t minSize = sizeof(mMaxLuminance) +
                     sizeof(mMaxAverageLuminance) +
                     sizeof(mMinLuminance) +
                     sizeof(int32_t);

    if (size < minSize) {
        return NO_MEMORY;
    }

    int32_t const * const buf = static_cast<int32_t const *>(buffer);
    const size_t itemCount = size_t(buf[3]);

    // check the buffer is large enough
    if (size < minSize + itemCount * sizeof(int32_t)) {
        return BAD_VALUE;
    }

    mMaxLuminance        = reinterpret_cast<float const&>(buf[0]);
    mMaxAverageLuminance = reinterpret_cast<float const&>(buf[1]);
    mMinLuminance        = reinterpret_cast<float const&>(buf[2]);
    if (itemCount) {
        mSupportedHdrTypes.resize(itemCount);
        for (size_t i = 0; i < itemCount; ++i) {
            mSupportedHdrTypes[i] = static_cast<ui::Hdr>(buf[4 + i]);
        }
    }
    return NO_ERROR;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace android
