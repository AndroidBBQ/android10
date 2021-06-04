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

#define LOG_TAG "GraphicBuffer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <ui/GraphicBuffer.h>

#include <cutils/atomic.h>

#include <grallocusage/GrallocUsageConversion.h>

#ifndef LIBUI_IN_VNDK
#include <ui/BufferHubBuffer.h>
#endif // LIBUI_IN_VNDK

#include <ui/Gralloc2.h>
#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBufferMapper.h>
#include <utils/Trace.h>

namespace android {

// ===========================================================================
// Buffer and implementation of ANativeWindowBuffer
// ===========================================================================

static uint64_t getUniqueId() {
    static volatile int32_t nextId = 0;
    uint64_t id = static_cast<uint64_t>(getpid()) << 32;
    id |= static_cast<uint32_t>(android_atomic_inc(&nextId));
    return id;
}

sp<GraphicBuffer> GraphicBuffer::from(ANativeWindowBuffer* anwb) {
    return static_cast<GraphicBuffer *>(anwb);
}

GraphicBuffer* GraphicBuffer::fromAHardwareBuffer(AHardwareBuffer* buffer) {
    return reinterpret_cast<GraphicBuffer*>(buffer);
}

GraphicBuffer const* GraphicBuffer::fromAHardwareBuffer(AHardwareBuffer const* buffer) {
    return reinterpret_cast<GraphicBuffer const*>(buffer);
}

AHardwareBuffer* GraphicBuffer::toAHardwareBuffer() {
    return reinterpret_cast<AHardwareBuffer*>(this);
}

AHardwareBuffer const* GraphicBuffer::toAHardwareBuffer() const {
    return reinterpret_cast<AHardwareBuffer const*>(this);
}

GraphicBuffer::GraphicBuffer()
    : BASE(), mOwner(ownData), mBufferMapper(GraphicBufferMapper::get()),
      mInitCheck(NO_ERROR), mId(getUniqueId()), mGenerationNumber(0)
{
    width  =
    height =
    stride =
    format =
    usage_deprecated = 0;
    usage  = 0;
    layerCount = 0;
    handle = nullptr;
}

// deprecated
GraphicBuffer::GraphicBuffer(uint32_t inWidth, uint32_t inHeight,
        PixelFormat inFormat, uint32_t inUsage, std::string requestorName)
    : GraphicBuffer(inWidth, inHeight, inFormat, 1, static_cast<uint64_t>(inUsage), requestorName)
{
}

GraphicBuffer::GraphicBuffer(uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
                             uint32_t inLayerCount, uint64_t inUsage, std::string requestorName)
      : GraphicBuffer() {
    mInitCheck = initWithSize(inWidth, inHeight, inFormat, inLayerCount, inUsage,
                              std::move(requestorName));
}

// deprecated
GraphicBuffer::GraphicBuffer(uint32_t inWidth, uint32_t inHeight,
        PixelFormat inFormat, uint32_t inLayerCount, uint32_t inUsage,
        uint32_t inStride, native_handle_t* inHandle, bool keepOwnership)
    : GraphicBuffer(inHandle, keepOwnership ? TAKE_HANDLE : WRAP_HANDLE,
            inWidth, inHeight, inFormat, inLayerCount, static_cast<uint64_t>(inUsage),
            inStride)
{
}

GraphicBuffer::GraphicBuffer(const native_handle_t* inHandle, HandleWrapMethod method,
                             uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
                             uint32_t inLayerCount, uint64_t inUsage, uint32_t inStride)
      : GraphicBuffer() {
    mInitCheck = initWithHandle(inHandle, method, inWidth, inHeight, inFormat, inLayerCount,
                                inUsage, inStride);
}

#ifndef LIBUI_IN_VNDK
GraphicBuffer::GraphicBuffer(std::unique_ptr<BufferHubBuffer> buffer) : GraphicBuffer() {
    if (buffer == nullptr) {
        mInitCheck = BAD_VALUE;
        return;
    }

    mInitCheck = initWithHandle(buffer->duplicateHandle(), /*method=*/TAKE_UNREGISTERED_HANDLE,
                                buffer->desc().width, buffer->desc().height,
                                static_cast<PixelFormat>(buffer->desc().format),
                                buffer->desc().layers, buffer->desc().usage, buffer->desc().stride);
    mBufferId = buffer->id();
    mBufferHubBuffer = std::move(buffer);
}
#endif // LIBUI_IN_VNDK

GraphicBuffer::~GraphicBuffer()
{
    ATRACE_CALL();
    if (handle) {
        free_handle();
    }
    for (auto& [callback, context] : mDeathCallbacks) {
        callback(context, mId);
    }
}

void GraphicBuffer::free_handle()
{
    if (mOwner == ownHandle) {
        mBufferMapper.freeBuffer(handle);
    } else if (mOwner == ownData) {
        GraphicBufferAllocator& allocator(GraphicBufferAllocator::get());
        allocator.free(handle);
    }
    handle = nullptr;
}

status_t GraphicBuffer::initCheck() const {
    return static_cast<status_t>(mInitCheck);
}

void GraphicBuffer::dumpAllocationsToSystemLog()
{
    GraphicBufferAllocator::dumpToSystemLog();
}

ANativeWindowBuffer* GraphicBuffer::getNativeBuffer() const
{
    return static_cast<ANativeWindowBuffer*>(
            const_cast<GraphicBuffer*>(this));
}

status_t GraphicBuffer::reallocate(uint32_t inWidth, uint32_t inHeight,
        PixelFormat inFormat, uint32_t inLayerCount, uint64_t inUsage)
{
    if (mOwner != ownData)
        return INVALID_OPERATION;

    if (handle &&
            static_cast<int>(inWidth) == width &&
            static_cast<int>(inHeight) == height &&
            inFormat == format &&
            inLayerCount == layerCount &&
            inUsage == usage)
        return NO_ERROR;

    if (handle) {
        GraphicBufferAllocator& allocator(GraphicBufferAllocator::get());
        allocator.free(handle);
        handle = nullptr;
    }
    return initWithSize(inWidth, inHeight, inFormat, inLayerCount, inUsage, "[Reallocation]");
}

bool GraphicBuffer::needsReallocation(uint32_t inWidth, uint32_t inHeight,
        PixelFormat inFormat, uint32_t inLayerCount, uint64_t inUsage)
{
    if (static_cast<int>(inWidth) != width) return true;
    if (static_cast<int>(inHeight) != height) return true;
    if (inFormat != format) return true;
    if (inLayerCount != layerCount) return true;
    if ((usage & inUsage) != inUsage) return true;
    if ((usage & USAGE_PROTECTED) != (inUsage & USAGE_PROTECTED)) return true;
    return false;
}

status_t GraphicBuffer::initWithSize(uint32_t inWidth, uint32_t inHeight,
        PixelFormat inFormat, uint32_t inLayerCount, uint64_t inUsage,
        std::string requestorName)
{
    GraphicBufferAllocator& allocator = GraphicBufferAllocator::get();
    uint32_t outStride = 0;
    status_t err = allocator.allocate(inWidth, inHeight, inFormat, inLayerCount,
            inUsage, &handle, &outStride, mId,
            std::move(requestorName));
    if (err == NO_ERROR) {
        mBufferMapper.getTransportSize(handle, &mTransportNumFds, &mTransportNumInts);

        width = static_cast<int>(inWidth);
        height = static_cast<int>(inHeight);
        format = inFormat;
        layerCount = inLayerCount;
        usage = inUsage;
        usage_deprecated = int(usage);
        stride = static_cast<int>(outStride);
    }
    return err;
}

status_t GraphicBuffer::initWithHandle(const native_handle_t* inHandle, HandleWrapMethod method,
                                       uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
                                       uint32_t inLayerCount, uint64_t inUsage, uint32_t inStride) {
    ANativeWindowBuffer::width = static_cast<int>(inWidth);
    ANativeWindowBuffer::height = static_cast<int>(inHeight);
    ANativeWindowBuffer::stride = static_cast<int>(inStride);
    ANativeWindowBuffer::format = inFormat;
    ANativeWindowBuffer::usage = inUsage;
    ANativeWindowBuffer::usage_deprecated = int(inUsage);

    ANativeWindowBuffer::layerCount = inLayerCount;

    mOwner = (method == WRAP_HANDLE) ? ownNone : ownHandle;

    if (method == TAKE_UNREGISTERED_HANDLE || method == CLONE_HANDLE) {
        buffer_handle_t importedHandle;
        status_t err = mBufferMapper.importBuffer(inHandle, inWidth, inHeight, inLayerCount,
                                                  inFormat, inUsage, inStride, &importedHandle);
        if (err != NO_ERROR) {
            initWithHandle(nullptr, WRAP_HANDLE, 0, 0, 0, 0, 0, 0);

            return err;
        }

        if (method == TAKE_UNREGISTERED_HANDLE) {
            native_handle_close(inHandle);
            native_handle_delete(const_cast<native_handle_t*>(inHandle));
        }

        inHandle = importedHandle;
        mBufferMapper.getTransportSize(inHandle, &mTransportNumFds, &mTransportNumInts);
    }

    ANativeWindowBuffer::handle = inHandle;

    return NO_ERROR;
}

status_t GraphicBuffer::lock(uint32_t inUsage, void** vaddr, int32_t* outBytesPerPixel,
                             int32_t* outBytesPerStride) {
    const Rect lockBounds(width, height);
    status_t res = lock(inUsage, lockBounds, vaddr, outBytesPerPixel, outBytesPerStride);
    return res;
}

status_t GraphicBuffer::lock(uint32_t inUsage, const Rect& rect, void** vaddr,
                             int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    if (rect.left < 0 || rect.right  > width ||
        rect.top  < 0 || rect.bottom > height) {
        ALOGE("locking pixels (%d,%d,%d,%d) outside of buffer (w=%d, h=%d)",
                rect.left, rect.top, rect.right, rect.bottom,
                width, height);
        return BAD_VALUE;
    }

    status_t res = getBufferMapper().lock(handle, inUsage, rect, vaddr, outBytesPerPixel,
                                          outBytesPerStride);

    return res;
}

status_t GraphicBuffer::lockYCbCr(uint32_t inUsage, android_ycbcr* ycbcr)
{
    const Rect lockBounds(width, height);
    status_t res = lockYCbCr(inUsage, lockBounds, ycbcr);
    return res;
}

status_t GraphicBuffer::lockYCbCr(uint32_t inUsage, const Rect& rect,
        android_ycbcr* ycbcr)
{
    if (rect.left < 0 || rect.right  > width ||
        rect.top  < 0 || rect.bottom > height) {
        ALOGE("locking pixels (%d,%d,%d,%d) outside of buffer (w=%d, h=%d)",
                rect.left, rect.top, rect.right, rect.bottom,
                width, height);
        return BAD_VALUE;
    }
    status_t res = getBufferMapper().lockYCbCr(handle, inUsage, rect, ycbcr);
    return res;
}

status_t GraphicBuffer::unlock()
{
    status_t res = getBufferMapper().unlock(handle);
    return res;
}

status_t GraphicBuffer::lockAsync(uint32_t inUsage, void** vaddr, int fenceFd,
                                  int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    const Rect lockBounds(width, height);
    status_t res =
            lockAsync(inUsage, lockBounds, vaddr, fenceFd, outBytesPerPixel, outBytesPerStride);
    return res;
}

status_t GraphicBuffer::lockAsync(uint32_t inUsage, const Rect& rect, void** vaddr, int fenceFd,
                                  int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    return lockAsync(inUsage, inUsage, rect, vaddr, fenceFd, outBytesPerPixel, outBytesPerStride);
}

status_t GraphicBuffer::lockAsync(uint64_t inProducerUsage, uint64_t inConsumerUsage,
                                  const Rect& rect, void** vaddr, int fenceFd,
                                  int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    if (rect.left < 0 || rect.right  > width ||
        rect.top  < 0 || rect.bottom > height) {
        ALOGE("locking pixels (%d,%d,%d,%d) outside of buffer (w=%d, h=%d)",
                rect.left, rect.top, rect.right, rect.bottom,
                width, height);
        return BAD_VALUE;
    }

    status_t res = getBufferMapper().lockAsync(handle, inProducerUsage, inConsumerUsage, rect,
                                               vaddr, fenceFd, outBytesPerPixel, outBytesPerStride);

    return res;
}

status_t GraphicBuffer::lockAsyncYCbCr(uint32_t inUsage, android_ycbcr* ycbcr,
        int fenceFd)
{
    const Rect lockBounds(width, height);
    status_t res = lockAsyncYCbCr(inUsage, lockBounds, ycbcr, fenceFd);
    return res;
}

status_t GraphicBuffer::lockAsyncYCbCr(uint32_t inUsage, const Rect& rect,
        android_ycbcr* ycbcr, int fenceFd)
{
    if (rect.left < 0 || rect.right  > width ||
        rect.top  < 0 || rect.bottom > height) {
        ALOGE("locking pixels (%d,%d,%d,%d) outside of buffer (w=%d, h=%d)",
                rect.left, rect.top, rect.right, rect.bottom,
                width, height);
        return BAD_VALUE;
    }
    status_t res = getBufferMapper().lockAsyncYCbCr(handle, inUsage, rect, ycbcr, fenceFd);
    return res;
}

status_t GraphicBuffer::unlockAsync(int *fenceFd)
{
    status_t res = getBufferMapper().unlockAsync(handle, fenceFd);
    return res;
}

status_t GraphicBuffer::isSupported(uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
                                    uint32_t inLayerCount, uint64_t inUsage,
                                    bool* outSupported) const {
    return mBufferMapper.isSupported(inWidth, inHeight, inFormat, inLayerCount, inUsage,
                                     outSupported);
}

size_t GraphicBuffer::getFlattenedSize() const {
#ifndef LIBUI_IN_VNDK
    if (mBufferHubBuffer != nullptr) {
        return 48;
    }
#endif
    return static_cast<size_t>(13 + (handle ? mTransportNumInts : 0)) * sizeof(int);
}

size_t GraphicBuffer::getFdCount() const {
#ifndef LIBUI_IN_VNDK
    if (mBufferHubBuffer != nullptr) {
        return 0;
    }
#endif
    return static_cast<size_t>(handle ? mTransportNumFds : 0);
}

status_t GraphicBuffer::flatten(void*& buffer, size_t& size, int*& fds, size_t& count) const {
#ifndef LIBUI_IN_VNDK
    if (mBufferHubBuffer != nullptr) {
        return flattenBufferHubBuffer(buffer, size);
    }
#endif
    size_t sizeNeeded = GraphicBuffer::getFlattenedSize();
    if (size < sizeNeeded) return NO_MEMORY;

    size_t fdCountNeeded = GraphicBuffer::getFdCount();
    if (count < fdCountNeeded) return NO_MEMORY;

    int32_t* buf = static_cast<int32_t*>(buffer);
    buf[0] = 'GB01';
    buf[1] = width;
    buf[2] = height;
    buf[3] = stride;
    buf[4] = format;
    buf[5] = static_cast<int32_t>(layerCount);
    buf[6] = int(usage); // low 32-bits
    buf[7] = static_cast<int32_t>(mId >> 32);
    buf[8] = static_cast<int32_t>(mId & 0xFFFFFFFFull);
    buf[9] = static_cast<int32_t>(mGenerationNumber);
    buf[10] = 0;
    buf[11] = 0;
    buf[12] = int(usage >> 32); // high 32-bits

    if (handle) {
        buf[10] = int32_t(mTransportNumFds);
        buf[11] = int32_t(mTransportNumInts);
        memcpy(fds, handle->data, static_cast<size_t>(mTransportNumFds) * sizeof(int));
        memcpy(buf + 13, handle->data + handle->numFds,
               static_cast<size_t>(mTransportNumInts) * sizeof(int));
    }

    buffer = static_cast<void*>(static_cast<uint8_t*>(buffer) + sizeNeeded);
    size -= sizeNeeded;
    if (handle) {
        fds += mTransportNumFds;
        count -= static_cast<size_t>(mTransportNumFds);
    }
    return NO_ERROR;
}

status_t GraphicBuffer::unflatten(void const*& buffer, size_t& size, int const*& fds,
                                  size_t& count) {
    // Check if size is not smaller than buf[0] is supposed to take.
    if (size < sizeof(int)) {
        return NO_MEMORY;
    }

    int const* buf = static_cast<int const*>(buffer);

    // NOTE: it turns out that some media code generates a flattened GraphicBuffer manually!!!!!
    // see H2BGraphicBufferProducer.cpp
    uint32_t flattenWordCount = 0;
    if (buf[0] == 'GB01') {
        // new version with 64-bits usage bits
        flattenWordCount = 13;
    } else if (buf[0] == 'GBFR') {
        // old version, when usage bits were 32-bits
        flattenWordCount = 12;
    } else if (buf[0] == 'BHBB') { // BufferHub backed buffer.
#ifndef LIBUI_IN_VNDK
        return unflattenBufferHubBuffer(buffer, size);
#else
        return BAD_TYPE;
#endif
    } else {
        return BAD_TYPE;
    }

    if (size < 12 * sizeof(int)) {
        android_errorWriteLog(0x534e4554, "114223584");
        return NO_MEMORY;
    }

    const size_t numFds  = static_cast<size_t>(buf[10]);
    const size_t numInts = static_cast<size_t>(buf[11]);

    // Limit the maxNumber to be relatively small. The number of fds or ints
    // should not come close to this number, and the number itself was simply
    // chosen to be high enough to not cause issues and low enough to prevent
    // overflow problems.
    const size_t maxNumber = 4096;
    if (numFds >= maxNumber || numInts >= (maxNumber - flattenWordCount)) {
        width = height = stride = format = usage_deprecated = 0;
        layerCount = 0;
        usage = 0;
        handle = nullptr;
        ALOGE("unflatten: numFds or numInts is too large: %zd, %zd", numFds, numInts);
        return BAD_VALUE;
    }

    const size_t sizeNeeded = (flattenWordCount + numInts) * sizeof(int);
    if (size < sizeNeeded) return NO_MEMORY;

    size_t fdCountNeeded = numFds;
    if (count < fdCountNeeded) return NO_MEMORY;

    if (handle) {
        // free previous handle if any
        free_handle();
    }

    if (numFds || numInts) {
        width  = buf[1];
        height = buf[2];
        stride = buf[3];
        format = buf[4];
        layerCount = static_cast<uintptr_t>(buf[5]);
        usage_deprecated = buf[6];
        if (flattenWordCount == 13) {
            usage = (uint64_t(buf[12]) << 32) | uint32_t(buf[6]);
        } else {
            usage = uint64_t(usage_deprecated);
        }
        native_handle* h =
                native_handle_create(static_cast<int>(numFds), static_cast<int>(numInts));
        if (!h) {
            width = height = stride = format = usage_deprecated = 0;
            layerCount = 0;
            usage = 0;
            handle = nullptr;
            ALOGE("unflatten: native_handle_create failed");
            return NO_MEMORY;
        }
        memcpy(h->data, fds, numFds * sizeof(int));
        memcpy(h->data + numFds, buf + flattenWordCount, numInts * sizeof(int));
        handle = h;
    } else {
        width = height = stride = format = usage_deprecated = 0;
        layerCount = 0;
        usage = 0;
        handle = nullptr;
    }

    mId = static_cast<uint64_t>(buf[7]) << 32;
    mId |= static_cast<uint32_t>(buf[8]);

    mGenerationNumber = static_cast<uint32_t>(buf[9]);

    mOwner = ownHandle;

    if (handle != nullptr) {
        buffer_handle_t importedHandle;
        status_t err = mBufferMapper.importBuffer(handle, uint32_t(width), uint32_t(height),
                uint32_t(layerCount), format, usage, uint32_t(stride), &importedHandle);
        if (err != NO_ERROR) {
            width = height = stride = format = usage_deprecated = 0;
            layerCount = 0;
            usage = 0;
            handle = nullptr;
            ALOGE("unflatten: registerBuffer failed: %s (%d)", strerror(-err), err);
            return err;
        }

        native_handle_close(handle);
        native_handle_delete(const_cast<native_handle_t*>(handle));
        handle = importedHandle;
        mBufferMapper.getTransportSize(handle, &mTransportNumFds, &mTransportNumInts);
    }

    buffer = static_cast<void const*>(static_cast<uint8_t const*>(buffer) + sizeNeeded);
    size -= sizeNeeded;
    fds += numFds;
    count -= numFds;
    return NO_ERROR;
}

void GraphicBuffer::addDeathCallback(GraphicBufferDeathCallback deathCallback, void* context) {
    mDeathCallbacks.emplace_back(deathCallback, context);
}

#ifndef LIBUI_IN_VNDK
status_t GraphicBuffer::flattenBufferHubBuffer(void*& buffer, size_t& size) const {
    sp<NativeHandle> tokenHandle = mBufferHubBuffer->duplicate();
    if (tokenHandle == nullptr || tokenHandle->handle() == nullptr ||
        tokenHandle->handle()->numFds != 0) {
        return BAD_VALUE;
    }

    // Size needed for one label, one number of ints inside the token, one generation number and
    // the token itself.
    int numIntsInToken = tokenHandle->handle()->numInts;
    const size_t sizeNeeded = static_cast<size_t>(3 + numIntsInToken) * sizeof(int);
    if (size < sizeNeeded) {
        ALOGE("%s: needed size %d, given size %d. Not enough memory.", __FUNCTION__,
              static_cast<int>(sizeNeeded), static_cast<int>(size));
        return NO_MEMORY;
    }
    size -= sizeNeeded;

    int* buf = static_cast<int*>(buffer);
    buf[0] = 'BHBB';
    buf[1] = numIntsInToken;
    memcpy(buf + 2, tokenHandle->handle()->data, static_cast<size_t>(numIntsInToken) * sizeof(int));
    buf[2 + numIntsInToken] = static_cast<int32_t>(mGenerationNumber);

    return NO_ERROR;
}

status_t GraphicBuffer::unflattenBufferHubBuffer(void const*& buffer, size_t& size) {
    const int* buf = static_cast<const int*>(buffer);
    int numIntsInToken = buf[1];
    // Size needed for one label, one number of ints inside the token, one generation number and
    // the token itself.
    const size_t sizeNeeded = static_cast<size_t>(3 + numIntsInToken) * sizeof(int);
    if (size < sizeNeeded) {
        ALOGE("%s: needed size %d, given size %d. Not enough memory.", __FUNCTION__,
              static_cast<int>(sizeNeeded), static_cast<int>(size));
        return NO_MEMORY;
    }
    size -= sizeNeeded;
    native_handle_t* importToken = native_handle_create(/*numFds=*/0, /*numInts=*/numIntsInToken);
    memcpy(importToken->data, buf + 2, static_cast<size_t>(buf[1]) * sizeof(int));
    sp<NativeHandle> importTokenHandle = NativeHandle::create(importToken, /*ownHandle=*/true);
    std::unique_ptr<BufferHubBuffer> bufferHubBuffer = BufferHubBuffer::import(importTokenHandle);
    if (bufferHubBuffer == nullptr || bufferHubBuffer.get() == nullptr) {
        return BAD_VALUE;
    }
    // Reconstruct this GraphicBuffer object using the new BufferHubBuffer object.
    if (handle) {
        free_handle();
    }
    mId = 0;
    mGenerationNumber = static_cast<uint32_t>(buf[2 + numIntsInToken]);
    mInitCheck =
            initWithHandle(bufferHubBuffer->duplicateHandle(), /*method=*/TAKE_UNREGISTERED_HANDLE,
                           bufferHubBuffer->desc().width, bufferHubBuffer->desc().height,
                           static_cast<PixelFormat>(bufferHubBuffer->desc().format),
                           bufferHubBuffer->desc().layers, bufferHubBuffer->desc().usage,
                           bufferHubBuffer->desc().stride);
    mBufferId = bufferHubBuffer->id();
    mBufferHubBuffer.reset(std::move(bufferHubBuffer.get()));

    return NO_ERROR;
}

bool GraphicBuffer::isBufferHubBuffer() const {
    return mBufferHubBuffer != nullptr;
}
#endif // LIBUI_IN_VNDK

// ---------------------------------------------------------------------------

}; // namespace android
