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

#ifndef BUFFER_H_
#define BUFFER_H_

#include <android/hardware/drm/1.0/types.h>
#include <utils/RefBase.h>

#include "ClearKeyTypes.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::sp;

struct Buffer : public RefBase {
    explicit Buffer(size_t capacity);

    uint8_t *base() { return reinterpret_cast<uint8_t *>(mData); }
    uint8_t *data() { return reinterpret_cast<uint8_t *>(mData) + mRangeOffset; }
    size_t capacity() const { return mCapacity; }
    size_t size() const { return mRangeLength; }
    size_t offset() const { return mRangeOffset; }

protected:
    virtual ~Buffer();

private:
    void *mData;
    size_t mCapacity;
    size_t mRangeOffset;
    size_t mRangeLength;

    bool mOwnsData;

    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif  // BUFFER_H_
