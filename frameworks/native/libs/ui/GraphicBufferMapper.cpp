/*
 * Copyright (C) 2007 The Android Open Source Project
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

#define LOG_TAG "GraphicBufferMapper"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include <ui/GraphicBufferMapper.h>

#include <grallocusage/GrallocUsageConversion.h>

// We would eliminate the non-conforming zero-length array, but we can't since
// this is effectively included from the Linux kernel
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#include <sync/sync.h>
#pragma clang diagnostic pop

#include <utils/Log.h>
#include <utils/Trace.h>

#include <ui/Gralloc.h>
#include <ui/Gralloc2.h>
#include <ui/Gralloc3.h>
#include <ui/GraphicBuffer.h>

#include <system/graphics.h>

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferMapper )

void GraphicBufferMapper::preloadHal() {
    Gralloc2Mapper::preload();
    Gralloc3Mapper::preload();
}

GraphicBufferMapper::GraphicBufferMapper() {
    mMapper = std::make_unique<const Gralloc3Mapper>();
    if (!mMapper->isLoaded()) {
        mMapper = std::make_unique<const Gralloc2Mapper>();
        mMapperVersion = Version::GRALLOC_2;
    } else {
        mMapperVersion = Version::GRALLOC_3;
    }

    if (!mMapper->isLoaded()) {
        LOG_ALWAYS_FATAL("gralloc-mapper is missing");
    }
}

status_t GraphicBufferMapper::importBuffer(buffer_handle_t rawHandle,
        uint32_t width, uint32_t height, uint32_t layerCount,
        PixelFormat format, uint64_t usage, uint32_t stride,
        buffer_handle_t* outHandle)
{
    ATRACE_CALL();

    buffer_handle_t bufferHandle;
    status_t error = mMapper->importBuffer(hardware::hidl_handle(rawHandle), &bufferHandle);
    if (error != NO_ERROR) {
        ALOGW("importBuffer(%p) failed: %d", rawHandle, error);
        return error;
    }

    error = mMapper->validateBufferSize(bufferHandle, width, height, format, layerCount, usage,
                                        stride);
    if (error != NO_ERROR) {
        ALOGE("validateBufferSize(%p) failed: %d", rawHandle, error);
        freeBuffer(bufferHandle);
        return static_cast<status_t>(error);
    }

    *outHandle = bufferHandle;

    return NO_ERROR;
}

void GraphicBufferMapper::getTransportSize(buffer_handle_t handle,
            uint32_t* outTransportNumFds, uint32_t* outTransportNumInts)
{
    mMapper->getTransportSize(handle, outTransportNumFds, outTransportNumInts);
}

status_t GraphicBufferMapper::freeBuffer(buffer_handle_t handle)
{
    ATRACE_CALL();

    mMapper->freeBuffer(handle);

    return NO_ERROR;
}

status_t GraphicBufferMapper::lock(buffer_handle_t handle, uint32_t usage, const Rect& bounds,
                                   void** vaddr, int32_t* outBytesPerPixel,
                                   int32_t* outBytesPerStride) {
    return lockAsync(handle, usage, bounds, vaddr, -1, outBytesPerPixel, outBytesPerStride);
}

status_t GraphicBufferMapper::lockYCbCr(buffer_handle_t handle, uint32_t usage,
        const Rect& bounds, android_ycbcr *ycbcr)
{
    return lockAsyncYCbCr(handle, usage, bounds, ycbcr, -1);
}

status_t GraphicBufferMapper::unlock(buffer_handle_t handle)
{
    int32_t fenceFd = -1;
    status_t error = unlockAsync(handle, &fenceFd);
    if (error == NO_ERROR && fenceFd >= 0) {
        sync_wait(fenceFd, -1);
        close(fenceFd);
    }
    return error;
}

status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle, uint32_t usage, const Rect& bounds,
                                        void** vaddr, int fenceFd, int32_t* outBytesPerPixel,
                                        int32_t* outBytesPerStride) {
    return lockAsync(handle, usage, usage, bounds, vaddr, fenceFd, outBytesPerPixel,
                     outBytesPerStride);
}

status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle, uint64_t producerUsage,
                                        uint64_t consumerUsage, const Rect& bounds, void** vaddr,
                                        int fenceFd, int32_t* outBytesPerPixel,
                                        int32_t* outBytesPerStride) {
    ATRACE_CALL();

    const uint64_t usage = static_cast<uint64_t>(
            android_convertGralloc1To0Usage(producerUsage, consumerUsage));
    return mMapper->lock(handle, usage, bounds, fenceFd, vaddr, outBytesPerPixel,
                         outBytesPerStride);
}

status_t GraphicBufferMapper::lockAsyncYCbCr(buffer_handle_t handle,
        uint32_t usage, const Rect& bounds, android_ycbcr *ycbcr, int fenceFd)
{
    ATRACE_CALL();

    return mMapper->lock(handle, usage, bounds, fenceFd, ycbcr);
}

status_t GraphicBufferMapper::unlockAsync(buffer_handle_t handle, int *fenceFd)
{
    ATRACE_CALL();

    *fenceFd = mMapper->unlock(handle);

    return NO_ERROR;
}

status_t GraphicBufferMapper::isSupported(uint32_t width, uint32_t height,
                                          android::PixelFormat format, uint32_t layerCount,
                                          uint64_t usage, bool* outSupported) {
    return mMapper->isSupported(width, height, format, layerCount, usage, outSupported);
}
// ---------------------------------------------------------------------------
}; // namespace android
