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

#include "Buffer.h"

#include <android/hardware/drm/1.0/types.h>

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

Buffer::Buffer(size_t capacity)
      : mRangeOffset(0),
      mOwnsData(true) {
    mData = malloc(capacity);
    if (mData == nullptr) {
        mCapacity = 0;
        mRangeLength = 0;
    } else {
        mCapacity = capacity;
        mRangeLength = capacity;
    }
}

Buffer::~Buffer() {
    if (mOwnsData) {
        if (mData != nullptr) {
            free(mData);
            mData = nullptr;
        }
    }
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android
