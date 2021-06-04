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

#ifndef ANDROID_UI_GRALLOC3_H
#define ANDROID_UI_GRALLOC3_H

#include <string>

#include <android/hardware/graphics/allocator/3.0/IAllocator.h>
#include <android/hardware/graphics/common/1.1/types.h>
#include <android/hardware/graphics/mapper/3.0/IMapper.h>
#include <ui/Gralloc.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <utils/StrongPointer.h>

namespace android {

class Gralloc3Mapper : public GrallocMapper {
public:
    static void preload();

    Gralloc3Mapper();

    bool isLoaded() const override;

    status_t createDescriptor(void* bufferDescriptorInfo, void* outBufferDescriptor) const override;

    status_t importBuffer(const hardware::hidl_handle& rawHandle,
                          buffer_handle_t* outBufferHandle) const override;

    void freeBuffer(buffer_handle_t bufferHandle) const override;

    status_t validateBufferSize(buffer_handle_t bufferHandle, uint32_t width, uint32_t height,
                                android::PixelFormat format, uint32_t layerCount, uint64_t usage,
                                uint32_t stride) const override;

    void getTransportSize(buffer_handle_t bufferHandle, uint32_t* outNumFds,
                          uint32_t* outNumInts) const override;

    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                  int acquireFence, void** outData, int32_t* outBytesPerPixel,
                  int32_t* outBytesPerStride) const override;

    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                  int acquireFence, android_ycbcr* ycbcr) const override;

    int unlock(buffer_handle_t bufferHandle) const override;

    status_t isSupported(uint32_t width, uint32_t height, android::PixelFormat format,
                         uint32_t layerCount, uint64_t usage, bool* outSupported) const override;

private:
    // Determines whether the passed info is compatible with the mapper.
    status_t validateBufferDescriptorInfo(
            hardware::graphics::mapper::V3_0::IMapper::BufferDescriptorInfo* descriptorInfo) const;

    sp<hardware::graphics::mapper::V3_0::IMapper> mMapper;
};

class Gralloc3Allocator : public GrallocAllocator {
public:
    // An allocator relies on a mapper, and that mapper must be alive at all
    // time.
    Gralloc3Allocator(const Gralloc3Mapper& mapper);

    bool isLoaded() const override;

    std::string dumpDebugInfo() const override;

    status_t allocate(uint32_t width, uint32_t height, PixelFormat format, uint32_t layerCount,
                      uint64_t usage, uint32_t bufferCount, uint32_t* outStride,
                      buffer_handle_t* outBufferHandles) const override;

private:
    const Gralloc3Mapper& mMapper;
    sp<hardware::graphics::allocator::V3_0::IAllocator> mAllocator;
};

} // namespace android

#endif // ANDROID_UI_GRALLOC3_H
