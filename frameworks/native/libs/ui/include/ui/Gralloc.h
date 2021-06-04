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

#ifndef ANDROID_UI_GRALLOC_H
#define ANDROID_UI_GRALLOC_H

#include <string>

#include <hidl/HidlSupport.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <utils/StrongPointer.h>

namespace android {

// A wrapper to IMapper
class GrallocMapper {
public:
    virtual ~GrallocMapper();

    virtual bool isLoaded() const = 0;

    virtual status_t createDescriptor(void* bufferDescriptorInfo,
                                      void* outBufferDescriptor) const = 0;

    // Import a buffer that is from another HAL, another process, or is
    // cloned.
    //
    // The returned handle must be freed with freeBuffer.
    virtual status_t importBuffer(const hardware::hidl_handle& rawHandle,
                                  buffer_handle_t* outBufferHandle) const = 0;

    virtual void freeBuffer(buffer_handle_t bufferHandle) const = 0;

    virtual status_t validateBufferSize(buffer_handle_t bufferHandle, uint32_t width,
                                        uint32_t height, android::PixelFormat format,
                                        uint32_t layerCount, uint64_t usage,
                                        uint32_t stride) const = 0;

    virtual void getTransportSize(buffer_handle_t bufferHandle, uint32_t* outNumFds,
                                  uint32_t* outNumInts) const = 0;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                          int acquireFence, void** outData, int32_t* outBytesPerPixel,
                          int32_t* outBytesPerStride) const = 0;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                          int acquireFence, android_ycbcr* ycbcr) const = 0;

    // unlock returns a fence sync object (or -1) and the fence sync object is
    // owned by the caller
    virtual int unlock(buffer_handle_t bufferHandle) const = 0;

    // isSupported queries whether or not a buffer with the given width, height,
    // format, layer count, and usage can be allocated on the device.  If
    // *outSupported is set to true, a buffer with the given specifications may be successfully
    // allocated if resources are available.  If false, a buffer with the given specifications will
    // never successfully allocate on this device. Note that this function is not guaranteed to be
    // supported on all devices, in which case a status_t of INVALID_OPERATION will be returned.
    virtual status_t isSupported(uint32_t width, uint32_t height, android::PixelFormat format,
                                 uint32_t layerCount, uint64_t usage, bool* outSupported) const = 0;
};

// A wrapper to IAllocator
class GrallocAllocator {
public:
    virtual ~GrallocAllocator();

    virtual bool isLoaded() const = 0;

    virtual std::string dumpDebugInfo() const = 0;

    /*
     * The returned buffers are already imported and must not be imported
     * again.  outBufferHandles must point to a space that can contain at
     * least "bufferCount" buffer_handle_t.
     */
    virtual status_t allocate(uint32_t width, uint32_t height, PixelFormat format,
                              uint32_t layerCount, uint64_t usage, uint32_t bufferCount,
                              uint32_t* outStride, buffer_handle_t* outBufferHandles) const = 0;
};

} // namespace android

#endif // ANDROID_UI_GRALLOC_H
