/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef SHARED_MEMORY_BUFFER_H_

#define SHARED_MEMORY_BUFFER_H_

#include <media/MediaCodecBuffer.h>
#include <android/hidl/memory/1.0/IMemory.h>

namespace android {

struct AMessage;
class IMemory;

/**
 * MediaCodecBuffer implementation based on IMemory.
 */
class SharedMemoryBuffer : public MediaCodecBuffer {
public:
    typedef ::android::hidl::memory::V1_0::IMemory TMemory;
    SharedMemoryBuffer(const sp<AMessage> &format, const sp<IMemory> &mem);
    SharedMemoryBuffer(const sp<AMessage> &format, const sp<TMemory> &mem);

    virtual ~SharedMemoryBuffer() = default;

private:
    SharedMemoryBuffer() = delete;

    const sp<IMemory> mMemory;
    const sp<TMemory> mTMemory;
};

}  // namespace android

#endif  // SHARED_MEMORY_BUFFER_H_
