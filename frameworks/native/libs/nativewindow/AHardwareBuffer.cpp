/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "AHardwareBuffer"

#include <vndk/hardware_buffer.h>

#include <errno.h>
#include <sys/socket.h>
#include <memory>

#include <cutils/native_handle.h>
#include <log/log.h>
#include <utils/StrongPointer.h>
#include <ui/GraphicBuffer.h>
#include <system/graphics.h>

#include <private/android/AHardwareBufferHelpers.h>
#include <android/hardware/graphics/common/1.1/types.h>


static constexpr int kFdBufferSize = 128 * sizeof(int);  // 128 ints

using namespace android;

// ----------------------------------------------------------------------------
// Public functions
// ----------------------------------------------------------------------------

int AHardwareBuffer_allocate(const AHardwareBuffer_Desc* desc, AHardwareBuffer** outBuffer) {
    if (!outBuffer || !desc) return BAD_VALUE;
    if (!AHardwareBuffer_isValidDescription(desc, /*log=*/true)) return BAD_VALUE;

    int format = AHardwareBuffer_convertToPixelFormat(desc->format);
    uint64_t usage = AHardwareBuffer_convertToGrallocUsageBits(desc->usage);
    sp<GraphicBuffer> gbuffer(new GraphicBuffer(
            desc->width, desc->height, format, desc->layers, usage,
            std::string("AHardwareBuffer pid [") + std::to_string(getpid()) + "]"));

    status_t err = gbuffer->initCheck();
    if (err != 0 || gbuffer->handle == 0) {
        if (err == NO_MEMORY) {
            GraphicBuffer::dumpAllocationsToSystemLog();
        }
        ALOGE("GraphicBuffer(w=%u, h=%u, lc=%u) failed (%s), handle=%p",
                desc->width, desc->height, desc->layers, strerror(-err), gbuffer->handle);
        return err;
    }

    *outBuffer = AHardwareBuffer_from_GraphicBuffer(gbuffer.get());

    // Ensure the buffer doesn't get destroyed when the sp<> goes away.
    AHardwareBuffer_acquire(*outBuffer);
    return NO_ERROR;
}

void AHardwareBuffer_acquire(AHardwareBuffer* buffer) {
    // incStrong/decStrong token must be the same, doesn't matter what it is
    AHardwareBuffer_to_GraphicBuffer(buffer)->incStrong((void*)AHardwareBuffer_acquire);
}

void AHardwareBuffer_release(AHardwareBuffer* buffer) {
    // incStrong/decStrong token must be the same, doesn't matter what it is
    AHardwareBuffer_to_GraphicBuffer(buffer)->decStrong((void*)AHardwareBuffer_acquire);
}

void AHardwareBuffer_describe(const AHardwareBuffer* buffer,
        AHardwareBuffer_Desc* outDesc) {
    if (!buffer || !outDesc) return;

    const GraphicBuffer* gbuffer = AHardwareBuffer_to_GraphicBuffer(buffer);

    outDesc->width = gbuffer->getWidth();
    outDesc->height = gbuffer->getHeight();
    outDesc->layers = gbuffer->getLayerCount();
    outDesc->format = AHardwareBuffer_convertFromPixelFormat(uint32_t(gbuffer->getPixelFormat()));
    outDesc->usage = AHardwareBuffer_convertFromGrallocUsageBits(gbuffer->getUsage());
    outDesc->stride = gbuffer->getStride();
    outDesc->rfu0 = 0;
    outDesc->rfu1 = 0;
}

int AHardwareBuffer_lockAndGetInfo(AHardwareBuffer* buffer, uint64_t usage,
        int32_t fence, const ARect* rect, void** outVirtualAddress,
        int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    if (outBytesPerPixel) *outBytesPerPixel = -1;
    if (outBytesPerStride) *outBytesPerStride = -1;

    if (!buffer) {
        return BAD_VALUE;
    }

    if (usage & ~(AHARDWAREBUFFER_USAGE_CPU_READ_MASK |
                  AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK)) {
        ALOGE("Invalid usage flags passed to AHardwareBuffer_lock; only "
                "AHARDWAREBUFFER_USAGE_CPU_* flags are allowed");
        return BAD_VALUE;
    }

    usage = AHardwareBuffer_convertToGrallocUsageBits(usage);
    GraphicBuffer* gbuffer = AHardwareBuffer_to_GraphicBuffer(buffer);

    //Mapper implementations before 3.0 will not return bytes per pixel or
    //bytes per stride information.
    if (gbuffer->getBufferMapperVersion() == GraphicBufferMapper::Version::GRALLOC_2) {
        ALOGE("Mapper versions before 3.0 cannot retrieve bytes per pixel and bytes per stride info");
        return INVALID_OPERATION;
    }

    if (gbuffer->getLayerCount() > 1) {
        ALOGE("Buffer with multiple layers passed to AHardwareBuffer_lock; "
                "only buffers with one layer are allowed");
        return INVALID_OPERATION;
    }

    Rect bounds;
    if (!rect) {
        bounds.set(Rect(gbuffer->getWidth(), gbuffer->getHeight()));
    } else {
        bounds.set(Rect(rect->left, rect->top, rect->right, rect->bottom));
    }
    int32_t bytesPerPixel;
    int32_t bytesPerStride;
    int result = gbuffer->lockAsync(usage, usage, bounds, outVirtualAddress, fence, &bytesPerPixel, &bytesPerStride);

    // if hardware returns -1 for bytes per pixel or bytes per stride, we fail
    // and unlock the buffer
    if (bytesPerPixel == -1 || bytesPerStride == -1) {
        gbuffer->unlock();
        return INVALID_OPERATION;
    }

    if (outBytesPerPixel) *outBytesPerPixel = bytesPerPixel;
    if (outBytesPerStride) *outBytesPerStride = bytesPerStride;
    return result;
}

int AHardwareBuffer_lock(AHardwareBuffer* buffer, uint64_t usage,
                         int32_t fence, const ARect* rect, void** outVirtualAddress) {
    int32_t bytesPerPixel;
    int32_t bytesPerStride;

    if (!buffer) return BAD_VALUE;

    if (usage & ~(AHARDWAREBUFFER_USAGE_CPU_READ_MASK |
                  AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK)) {
        ALOGE("Invalid usage flags passed to AHardwareBuffer_lock; only "
                "AHARDWAREBUFFER_USAGE_CPU_* flags are allowed");
        return BAD_VALUE;
    }

    usage = AHardwareBuffer_convertToGrallocUsageBits(usage);
    GraphicBuffer* gbuffer = AHardwareBuffer_to_GraphicBuffer(buffer);

    if (gbuffer->getLayerCount() > 1) {
        ALOGE("Buffer with multiple layers passed to AHardwareBuffer_lock; "
                "only buffers with one layer are allowed");
        return INVALID_OPERATION;
    }

    Rect bounds;
    if (!rect) {
        bounds.set(Rect(gbuffer->getWidth(), gbuffer->getHeight()));
    } else {
        bounds.set(Rect(rect->left, rect->top, rect->right, rect->bottom));
    }
    return gbuffer->lockAsync(usage, usage, bounds, outVirtualAddress, fence, &bytesPerPixel, &bytesPerStride);
}

int AHardwareBuffer_lockPlanes(AHardwareBuffer* buffer, uint64_t usage,
        int32_t fence, const ARect* rect, AHardwareBuffer_Planes* outPlanes) {
    if (!buffer || !outPlanes) return BAD_VALUE;

    if (usage & ~(AHARDWAREBUFFER_USAGE_CPU_READ_MASK |
                  AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK)) {
        ALOGE("Invalid usage flags passed to AHardwareBuffer_lock; only "
                " AHARDWAREBUFFER_USAGE_CPU_* flags are allowed");
        return BAD_VALUE;
    }

    usage = AHardwareBuffer_convertToGrallocUsageBits(usage);
    GraphicBuffer* gBuffer = AHardwareBuffer_to_GraphicBuffer(buffer);
    Rect bounds;
    if (!rect) {
        bounds.set(Rect(gBuffer->getWidth(), gBuffer->getHeight()));
    } else {
        bounds.set(Rect(rect->left, rect->top, rect->right, rect->bottom));
    }
    int format = AHardwareBuffer_convertFromPixelFormat(uint32_t(gBuffer->getPixelFormat()));
    memset(outPlanes->planes, 0, sizeof(outPlanes->planes));
    if (AHardwareBuffer_formatIsYuv(format)) {
      android_ycbcr yuvData;
      int result = gBuffer->lockAsyncYCbCr(usage, bounds, &yuvData, fence);
      if (result == 0) {
        outPlanes->planeCount = 3;
        outPlanes->planes[0].data = yuvData.y;
        outPlanes->planes[0].pixelStride = 1;
        outPlanes->planes[0].rowStride = yuvData.ystride;
        outPlanes->planes[1].data = yuvData.cb;
        outPlanes->planes[1].pixelStride = yuvData.chroma_step;
        outPlanes->planes[1].rowStride = yuvData.cstride;
        outPlanes->planes[2].data = yuvData.cr;
        outPlanes->planes[2].pixelStride = yuvData.chroma_step;
        outPlanes->planes[2].rowStride = yuvData.cstride;
      } else {
        outPlanes->planeCount = 0;
      }
      return result;
    } else {
      const uint32_t pixelStride = AHardwareBuffer_bytesPerPixel(format);
      outPlanes->planeCount = 1;
      outPlanes->planes[0].pixelStride = pixelStride;
      outPlanes->planes[0].rowStride = gBuffer->getStride() * pixelStride;
      return gBuffer->lockAsync(usage, usage, bounds, &outPlanes->planes[0].data, fence);
    }
}

int AHardwareBuffer_unlock(AHardwareBuffer* buffer, int32_t* fence) {
    if (!buffer) return BAD_VALUE;

    GraphicBuffer* gBuffer = AHardwareBuffer_to_GraphicBuffer(buffer);
    if (fence == nullptr)
        return gBuffer->unlock();
    else
        return gBuffer->unlockAsync(fence);
}

int AHardwareBuffer_sendHandleToUnixSocket(const AHardwareBuffer* buffer, int socketFd) {
    if (!buffer) return BAD_VALUE;
    const GraphicBuffer* gBuffer = AHardwareBuffer_to_GraphicBuffer(buffer);

    size_t flattenedSize = gBuffer->getFlattenedSize();
    size_t fdCount = gBuffer->getFdCount();

    std::unique_ptr<uint8_t[]> data(new uint8_t[flattenedSize]);
    std::unique_ptr<int[]> fds(new int[fdCount]);

    // Make copies of needed items since flatten modifies them, and we don't
    // want to send anything if there's an error during flatten.
    size_t flattenedSizeCopy = flattenedSize;
    size_t fdCountCopy = fdCount;
    void* dataStart = data.get();
    int* fdsStart = fds.get();
    status_t err = gBuffer->flatten(dataStart, flattenedSizeCopy, fdsStart,
            fdCountCopy);
    if (err != NO_ERROR) {
        return err;
    }

    struct iovec iov[1];
    iov[0].iov_base = data.get();
    iov[0].iov_len = flattenedSize;

    char buf[CMSG_SPACE(kFdBufferSize)];
    struct msghdr msg = {
            .msg_control = buf,
            .msg_controllen = sizeof(buf),
            .msg_iov = &iov[0],
            .msg_iovlen = 1,
    };

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * fdCount);
    int* fdData = reinterpret_cast<int*>(CMSG_DATA(cmsg));
    memcpy(fdData, fds.get(), sizeof(int) * fdCount);
    msg.msg_controllen = cmsg->cmsg_len;

    int result;
    do {
        result = sendmsg(socketFd, &msg, 0);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        result = errno;
        ALOGE("Error writing AHardwareBuffer to socket: error %#x (%s)",
                result, strerror(result));
        return -result;
    }

    return NO_ERROR;
}

int AHardwareBuffer_recvHandleFromUnixSocket(int socketFd, AHardwareBuffer** outBuffer) {
    if (!outBuffer) return BAD_VALUE;

    static constexpr int kMessageBufferSize = 4096 * sizeof(int);

    std::unique_ptr<char[]> dataBuf(new char[kMessageBufferSize]);
    char fdBuf[CMSG_SPACE(kFdBufferSize)];
    struct iovec iov[1];
    iov[0].iov_base = dataBuf.get();
    iov[0].iov_len = kMessageBufferSize;

    struct msghdr msg = {
            .msg_control = fdBuf,
            .msg_controllen = sizeof(fdBuf),
            .msg_iov = &iov[0],
            .msg_iovlen = 1,
    };

    int result;
    do {
        result = recvmsg(socketFd, &msg, 0);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        result = errno;
        ALOGE("Error reading AHardwareBuffer from socket: error %#x (%s)",
                result, strerror(result));
        return -result;
    }

    if (msg.msg_iovlen != 1) {
        ALOGE("Error reading AHardwareBuffer from socket: bad data length");
        return INVALID_OPERATION;
    }

    if (msg.msg_controllen % sizeof(int) != 0) {
        ALOGE("Error reading AHardwareBuffer from socket: bad fd length");
        return INVALID_OPERATION;
    }

    size_t dataLen = msg.msg_iov[0].iov_len;
    const void* data = static_cast<const void*>(msg.msg_iov[0].iov_base);
    if (!data) {
        ALOGE("Error reading AHardwareBuffer from socket: no buffer data");
        return INVALID_OPERATION;
    }

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg) {
        ALOGE("Error reading AHardwareBuffer from socket: no fd header");
        return INVALID_OPERATION;
    }

    size_t fdCount = msg.msg_controllen >> 2;
    const int* fdData = reinterpret_cast<const int*>(CMSG_DATA(cmsg));
    if (!fdData) {
        ALOGE("Error reading AHardwareBuffer from socket: no fd data");
        return INVALID_OPERATION;
    }

    GraphicBuffer* gBuffer = new GraphicBuffer();
    status_t err = gBuffer->unflatten(data, dataLen, fdData, fdCount);
    if (err != NO_ERROR) {
        return err;
    }
    *outBuffer = AHardwareBuffer_from_GraphicBuffer(gBuffer);
    // Ensure the buffer has a positive ref-count.
    AHardwareBuffer_acquire(*outBuffer);

    return NO_ERROR;
}

int AHardwareBuffer_isSupported(const AHardwareBuffer_Desc* desc) {
    if (!desc) return 0;
    if (!AHardwareBuffer_isValidDescription(desc, /*log=*/false)) return 0;

    bool supported = false;
    GraphicBuffer* gBuffer = new GraphicBuffer();
    status_t err = gBuffer->isSupported(desc->width, desc->height, desc->format, desc->layers,
                                        desc->usage, &supported);

    if (err == NO_ERROR) {
        return supported;
    }

    // function isSupported is not implemented on device or an error occurred during HAL
    // query.  Make a trial allocation.
    AHardwareBuffer_Desc trialDesc = *desc;
    trialDesc.width = 4;
    trialDesc.height = desc->format == AHARDWAREBUFFER_FORMAT_BLOB ? 1 : 4;
    if (desc->usage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) {
        trialDesc.layers = desc->layers == 6 ? 6 : 12;
    } else {
        trialDesc.layers = desc->layers == 1 ? 1 : 2;
    }
    AHardwareBuffer* trialBuffer = nullptr;
    int result = AHardwareBuffer_allocate(&trialDesc, &trialBuffer);
    if (result == NO_ERROR) {
        AHardwareBuffer_release(trialBuffer);
        return 1;
    }
    return 0;
}


// ----------------------------------------------------------------------------
// VNDK functions
// ----------------------------------------------------------------------------

const native_handle_t* AHardwareBuffer_getNativeHandle(
        const AHardwareBuffer* buffer) {
    if (!buffer) return nullptr;
    const GraphicBuffer* gbuffer = AHardwareBuffer_to_GraphicBuffer(buffer);
    return gbuffer->handle;
}

int AHardwareBuffer_createFromHandle(const AHardwareBuffer_Desc* desc,
                                     const native_handle_t* handle, int32_t method,
                                     AHardwareBuffer** outBuffer) {
    static_assert(static_cast<int32_t>(AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER) ==
                  static_cast<int32_t>(GraphicBuffer::TAKE_UNREGISTERED_HANDLE));
    static_assert(static_cast<int32_t>(AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE) ==
                  static_cast<int32_t>(GraphicBuffer::CLONE_HANDLE));

    if (!desc || !handle || !outBuffer) return BAD_VALUE;
    if (!(method == AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER ||
          method == AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE))
        return BAD_VALUE;
    if (desc->rfu0 != 0 || desc->rfu1 != 0) return BAD_VALUE;
    if (desc->format == AHARDWAREBUFFER_FORMAT_BLOB && desc->height != 1) return BAD_VALUE;

    const int format = AHardwareBuffer_convertToPixelFormat(desc->format);
    const uint64_t usage = AHardwareBuffer_convertToGrallocUsageBits(desc->usage);
    const auto wrapMethod = static_cast<GraphicBuffer::HandleWrapMethod>(method);
    sp<GraphicBuffer> gbuffer(new GraphicBuffer(handle, wrapMethod, desc->width, desc->height,
                                                format, desc->layers, usage, desc->stride));
    status_t err = gbuffer->initCheck();
    if (err != 0 || gbuffer->handle == 0) return err;

    *outBuffer = AHardwareBuffer_from_GraphicBuffer(gbuffer.get());
    // Ensure the buffer doesn't get destroyed when the sp<> goes away.
    AHardwareBuffer_acquire(*outBuffer);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
// Helpers implementation
// ----------------------------------------------------------------------------

namespace android {

bool AHardwareBuffer_isValidDescription(const AHardwareBuffer_Desc* desc, bool log) {
    if (desc->width == 0 || desc->height == 0 || desc->layers == 0) {
        ALOGE_IF(log, "Width, height and layers must all be nonzero");
        return false;
    }

    if (!AHardwareBuffer_isValidPixelFormat(desc->format)) {
        ALOGE_IF(log, "Invalid AHardwareBuffer pixel format %u (%#x))",
                desc->format, desc->format);
        return false;
    }

    if (desc->rfu0 != 0 || desc->rfu1 != 0) {
        ALOGE_IF(log, "AHardwareBuffer_Desc::rfu fields must be 0");
        return false;
    }

    if (desc->format == AHARDWAREBUFFER_FORMAT_BLOB) {
        if (desc->height != 1 || desc->layers != 1) {
            ALOGE_IF(log, "Height and layers must be 1 for AHARDWAREBUFFER_FORMAT_BLOB");
            return false;
        }
        const uint64_t blobInvalidGpuMask =
            AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
            AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
            AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
            AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;
        if (desc->usage & blobInvalidGpuMask) {
            ALOGE_IF(log, "Invalid GPU usage flag for AHARDWAREBUFFER_FORMAT_BLOB; "
                    "only AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER is allowed");
            return false;
        }
        if (desc->usage & AHARDWAREBUFFER_USAGE_VIDEO_ENCODE) {
            ALOGE_IF(log, "AHARDWAREBUFFER_FORMAT_BLOB cannot be encoded as video");
            return false;
        }
    } else if (AHardwareBuffer_formatIsYuv(desc->format)) {
        if (desc->layers != 1) {
            ALOGE_IF(log, "Layers must be 1 for YUV formats.");
            return false;
        }
        const uint64_t yuvInvalidGpuMask =
            AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
            AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;
        if (desc->usage & yuvInvalidGpuMask) {
            ALOGE_IF(log, "Invalid usage flags specified for YUV format; "
                    "mip-mapping and cube-mapping are not allowed.");
            return false;
        }
    } else {
        if (desc->usage & AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA) {
            ALOGE_IF(log, "AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA requires AHARDWAREBUFFER_FORMAT_BLOB");
            return false;
        }
        if (desc->usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
            ALOGE_IF(log, "AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER requires AHARDWAREBUFFER_FORMAT_BLOB");
            return false;
        }
    }

    if ((desc->usage & (AHARDWAREBUFFER_USAGE_CPU_READ_MASK | AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK)) &&
        (desc->usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT)) {
        ALOGE_IF(log, "AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT requires AHARDWAREBUFFER_USAGE_CPU_READ_NEVER "
              "and AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER");
        return false;
    }

    if (desc->usage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) {
        if (desc->width != desc->height) {
            ALOGE_IF(log, "Cube maps must be square");
            return false;
        }
        if (desc->layers % 6 != 0) {
            ALOGE_IF(log, "Cube map layers must be a multiple of 6");
            return false;
        }
    }
    return true;
}

bool AHardwareBuffer_isValidPixelFormat(uint32_t format) {
    static_assert(HAL_PIXEL_FORMAT_RGBA_8888 == AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RGBX_8888 == AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RGB_565 == AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RGB_888 == AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RGBA_FP16 == AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RGBA_1010102 == AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_BLOB == AHARDWAREBUFFER_FORMAT_BLOB,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_DEPTH_16 == AHARDWAREBUFFER_FORMAT_D16_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_DEPTH_24 == AHARDWAREBUFFER_FORMAT_D24_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_DEPTH_24_STENCIL_8 == AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_DEPTH_32F == AHARDWAREBUFFER_FORMAT_D32_FLOAT,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_DEPTH_32F_STENCIL_8 == AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_STENCIL_8 == AHARDWAREBUFFER_FORMAT_S8_UINT,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_BGRA_8888 == AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_YV12 == AHARDWAREBUFFER_FORMAT_YV12,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_Y8 == AHARDWAREBUFFER_FORMAT_Y8,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_Y16 == AHARDWAREBUFFER_FORMAT_Y16,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RAW16 == AHARDWAREBUFFER_FORMAT_RAW16,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RAW10 == AHARDWAREBUFFER_FORMAT_RAW10,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RAW12 == AHARDWAREBUFFER_FORMAT_RAW12,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_RAW_OPAQUE == AHARDWAREBUFFER_FORMAT_RAW_OPAQUE,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED == AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_YCBCR_420_888 == AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_YCBCR_422_SP == AHARDWAREBUFFER_FORMAT_YCbCr_422_SP,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_YCRCB_420_SP == AHARDWAREBUFFER_FORMAT_YCrCb_420_SP,
            "HAL and AHardwareBuffer pixel format don't match");
    static_assert(HAL_PIXEL_FORMAT_YCBCR_422_I == AHARDWAREBUFFER_FORMAT_YCbCr_422_I,
            "HAL and AHardwareBuffer pixel format don't match");

    switch (format) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
        case AHARDWAREBUFFER_FORMAT_BLOB:
        case AHARDWAREBUFFER_FORMAT_D16_UNORM:
        case AHARDWAREBUFFER_FORMAT_D24_UNORM:
        case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
        case AHARDWAREBUFFER_FORMAT_S8_UINT:
        case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
            // VNDK formats only -- unfortunately we can't differentiate from where we're called
        case AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM:
        case AHARDWAREBUFFER_FORMAT_YV12:
        case AHARDWAREBUFFER_FORMAT_Y8:
        case AHARDWAREBUFFER_FORMAT_Y16:
        case AHARDWAREBUFFER_FORMAT_RAW16:
        case AHARDWAREBUFFER_FORMAT_RAW10:
        case AHARDWAREBUFFER_FORMAT_RAW12:
        case AHARDWAREBUFFER_FORMAT_RAW_OPAQUE:
        case AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED:
        case AHARDWAREBUFFER_FORMAT_YCbCr_422_SP:
        case AHARDWAREBUFFER_FORMAT_YCrCb_420_SP:
        case AHARDWAREBUFFER_FORMAT_YCbCr_422_I:
            return true;

        default:
            return false;
    }
}

bool AHardwareBuffer_formatIsYuv(uint32_t format) {
    switch (format) {
        case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
        case AHARDWAREBUFFER_FORMAT_YV12:
        case AHARDWAREBUFFER_FORMAT_Y8:
        case AHARDWAREBUFFER_FORMAT_Y16:
        case AHARDWAREBUFFER_FORMAT_YCbCr_422_SP:
        case AHARDWAREBUFFER_FORMAT_YCrCb_420_SP:
        case AHARDWAREBUFFER_FORMAT_YCbCr_422_I:
            return true;
        default:
            return false;
    }
}

uint32_t AHardwareBuffer_bytesPerPixel(uint32_t format) {
  switch (format) {
      case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      case AHARDWAREBUFFER_FORMAT_D16_UNORM:
          return 2;
      case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      case AHARDWAREBUFFER_FORMAT_D24_UNORM:
          return 3;
      case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
      case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
      case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
          return 4;
      default:
          return 0;
  }
}

uint32_t AHardwareBuffer_convertFromPixelFormat(uint32_t hal_format) {
    return hal_format;
}

uint32_t AHardwareBuffer_convertToPixelFormat(uint32_t ahardwarebuffer_format) {
    return ahardwarebuffer_format;
}

uint64_t AHardwareBuffer_convertToGrallocUsageBits(uint64_t usage) {
    using android::hardware::graphics::common::V1_1::BufferUsage;
    static_assert(AHARDWAREBUFFER_USAGE_CPU_READ_NEVER == (uint64_t)BufferUsage::CPU_READ_NEVER,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_CPU_READ_RARELY == (uint64_t)BufferUsage::CPU_READ_RARELY,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN == (uint64_t)BufferUsage::CPU_READ_OFTEN,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER == (uint64_t)BufferUsage::CPU_WRITE_NEVER,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY == (uint64_t)BufferUsage::CPU_WRITE_RARELY,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN == (uint64_t)BufferUsage::CPU_WRITE_OFTEN,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE == (uint64_t)BufferUsage::GPU_TEXTURE,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER == (uint64_t)BufferUsage::GPU_RENDER_TARGET,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT == (uint64_t)BufferUsage::PROTECTED,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_VIDEO_ENCODE == (uint64_t)BufferUsage::VIDEO_ENCODER,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER == (uint64_t)BufferUsage::GPU_DATA_BUFFER,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA == (uint64_t)BufferUsage::SENSOR_DIRECT_DATA,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP == (uint64_t)BufferUsage::GPU_CUBE_MAP,
            "gralloc and AHardwareBuffer flags don't match");
    static_assert(AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE == (uint64_t)BufferUsage::GPU_MIPMAP_COMPLETE,
            "gralloc and AHardwareBuffer flags don't match");
    return usage;
}

uint64_t AHardwareBuffer_convertFromGrallocUsageBits(uint64_t usage) {
    return usage;
}

const GraphicBuffer* AHardwareBuffer_to_GraphicBuffer(const AHardwareBuffer* buffer) {
    return GraphicBuffer::fromAHardwareBuffer(buffer);
}

GraphicBuffer* AHardwareBuffer_to_GraphicBuffer(AHardwareBuffer* buffer) {
    return GraphicBuffer::fromAHardwareBuffer(buffer);
}

const ANativeWindowBuffer* AHardwareBuffer_to_ANativeWindowBuffer(const AHardwareBuffer* buffer) {
    return AHardwareBuffer_to_GraphicBuffer(buffer)->getNativeBuffer();
}

ANativeWindowBuffer* AHardwareBuffer_to_ANativeWindowBuffer(AHardwareBuffer* buffer) {
    return AHardwareBuffer_to_GraphicBuffer(buffer)->getNativeBuffer();
}

AHardwareBuffer* AHardwareBuffer_from_GraphicBuffer(GraphicBuffer* buffer) {
    return buffer->toAHardwareBuffer();
}

} // namespace android
