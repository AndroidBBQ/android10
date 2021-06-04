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

#include <inttypes.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkImage"

#include "NdkImagePriv.h"
#include "NdkImageReaderPriv.h"

#include <android_media_Utils.h>
#include <private/android/AHardwareBufferHelpers.h>
#include <utils/Log.h>
#include "hardware/camera3.h"

using namespace android;

#define ALIGN(x, mask) ( ((x) + (mask) - 1) & ~((mask) - 1) )

AImage::AImage(AImageReader* reader, int32_t format, uint64_t usage, BufferItem* buffer,
        int64_t timestamp, int32_t width, int32_t height, int32_t numPlanes) :
        mReader(reader), mFormat(format), mUsage(usage), mBuffer(buffer), mLockedBuffer(nullptr),
        mTimestamp(timestamp), mWidth(width), mHeight(height), mNumPlanes(numPlanes) {
    LOG_FATAL_IF(reader == nullptr, "AImageReader shouldn't be null while creating AImage");
}

AImage::~AImage() {
    Mutex::Autolock _l(mLock);
    if (!mIsClosed) {
        LOG_ALWAYS_FATAL(
                "Error: AImage %p is deleted before returning buffer to AImageReader!", this);
    }
}

bool
AImage::isClosed() const {
    Mutex::Autolock _l(mLock);
    return mIsClosed;
}

void
AImage::close(int releaseFenceFd) {
    Mutex::Autolock _l(mLock);
    if (mIsClosed) {
        return;
    }
    if (!mReader->mIsClosed) {
        mReader->releaseImageLocked(this, releaseFenceFd);
    }
    // Should have been set to nullptr in releaseImageLocked
    // Set to nullptr here for extra safety only
    mBuffer = nullptr;
    mLockedBuffer = nullptr;
    mIsClosed = true;
}

void
AImage::free() {
    if (!isClosed()) {
        ALOGE("Cannot free AImage before close!");
        return;
    }
    delete this;
}

void
AImage::lockReader() const {
    mReader->mLock.lock();
}

void
AImage::unlockReader() const {
    mReader->mLock.unlock();
}

media_status_t
AImage::getWidth(int32_t* width) const {
    if (width == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *width = -1;
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    *width = mWidth;
    return AMEDIA_OK;
}

media_status_t
AImage::getHeight(int32_t* height) const {
    if (height == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *height = -1;
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    *height = mHeight;
    return AMEDIA_OK;
}

media_status_t
AImage::getFormat(int32_t* format) const {
    if (format == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *format = -1;
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    *format = mFormat;
    return AMEDIA_OK;
}

media_status_t
AImage::getNumPlanes(int32_t* numPlanes) const {
    if (numPlanes == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *numPlanes = -1;
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    *numPlanes = mNumPlanes;
    return AMEDIA_OK;
}

media_status_t
AImage::getTimestamp(int64_t* timestamp) const {
    if (timestamp == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *timestamp = -1;
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    *timestamp = mTimestamp;
    return AMEDIA_OK;
}

media_status_t AImage::lockImage() {
    if (mBuffer == nullptr || mBuffer->mGraphicBuffer == nullptr) {
        LOG_ALWAYS_FATAL("%s: AImage %p has no buffer.", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    if ((mUsage & AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN) == 0) {
        ALOGE("%s: AImage %p does not have any software read usage bits set, usage=%" PRIu64 "",
              __FUNCTION__, this, mUsage);
        return AMEDIA_IMGREADER_CANNOT_LOCK_IMAGE;
    }

    if (mLockedBuffer != nullptr) {
        // Return immediately if the image has already been locked.
        return AMEDIA_OK;
    }

    auto lockedBuffer = std::make_unique<CpuConsumer::LockedBuffer>();

    uint64_t grallocUsage = AHardwareBuffer_convertToGrallocUsageBits(mUsage);

    status_t ret =
            lockImageFromBuffer(mBuffer, grallocUsage, mBuffer->mFence->dup(), lockedBuffer.get());
    if (ret != OK) {
        ALOGE("%s: AImage %p failed to lock, error=%d", __FUNCTION__, this, ret);
        return AMEDIA_IMGREADER_CANNOT_LOCK_IMAGE;
    }

    ALOGV("%s: Successfully locked the image %p.", __FUNCTION__, this);
    mLockedBuffer = std::move(lockedBuffer);

    return AMEDIA_OK;
}

media_status_t AImage::unlockImageIfLocked(int* fenceFd) {
    if (fenceFd == nullptr) {
        LOG_ALWAYS_FATAL("%s: fenceFd cannot be null.", __FUNCTION__);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (mBuffer == nullptr || mBuffer->mGraphicBuffer == nullptr) {
        LOG_ALWAYS_FATAL("%s: AImage %p has no buffer.", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    if (mLockedBuffer == nullptr) {
        // This image hasn't been locked yet, no need to unlock.
        *fenceFd = -1;
        return AMEDIA_OK;
    }

    // No fence by default.
    int releaseFenceFd = -1;
    status_t res = mBuffer->mGraphicBuffer->unlockAsync(&releaseFenceFd);
    if (res != OK) {
        ALOGE("%s unlock buffer failed on iamge %p.", __FUNCTION__, this);
        *fenceFd = -1;
        return AMEDIA_IMGREADER_CANNOT_UNLOCK_IMAGE;
    }

    *fenceFd = releaseFenceFd;
    return AMEDIA_OK;
}

media_status_t
AImage::getPlanePixelStride(int planeIdx, /*out*/int32_t* pixelStride) const {
    if (mLockedBuffer == nullptr) {
        ALOGE("%s: buffer not locked.", __FUNCTION__);
        return AMEDIA_IMGREADER_IMAGE_NOT_LOCKED;
    }

    if (planeIdx < 0 || planeIdx >= mNumPlanes) {
        ALOGE("Error: planeIdx %d out of bound [0,%d]",
                planeIdx, mNumPlanes - 1);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (pixelStride == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    int32_t fmt = mLockedBuffer->flexFormat;
    switch (fmt) {
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            *pixelStride = (planeIdx == 0) ? 1 : mLockedBuffer->chromaStep;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            *pixelStride = (planeIdx == 0) ? 1 : 2;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_Y8:
            *pixelStride = 1;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_YV12:
            *pixelStride = 1;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW16:
        case HAL_PIXEL_FORMAT_RGB_565:
            // Single plane 16bpp data.
            *pixelStride = 2;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
            *pixelStride = 4;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RGB_888:
            // Single plane, 24bpp.
            *pixelStride = 3;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_RAW10:
        case HAL_PIXEL_FORMAT_RAW12:
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            // Blob is used for JPEG data, RAW10 and RAW12 is used for 10-bit and 12-bit raw data,
            // those are single plane data without pixel stride defined
            return AMEDIA_ERROR_UNSUPPORTED;
        default:
            ALOGE("Pixel format: 0x%x is unsupported", fmt);
            return AMEDIA_ERROR_UNSUPPORTED;
    }
}

media_status_t
AImage::getPlaneRowStride(int planeIdx, /*out*/int32_t* rowStride) const {
    if (mLockedBuffer == nullptr) {
        ALOGE("%s: buffer not locked.", __FUNCTION__);
        return AMEDIA_IMGREADER_IMAGE_NOT_LOCKED;
    }

    if (planeIdx < 0 || planeIdx >= mNumPlanes) {
        ALOGE("Error: planeIdx %d out of bound [0,%d]",
                planeIdx, mNumPlanes - 1);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (rowStride == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    int32_t fmt = mLockedBuffer->flexFormat;
    switch (fmt) {
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            *rowStride = (planeIdx == 0) ? mLockedBuffer->stride
                                         : mLockedBuffer->chromaStride;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            *rowStride = mLockedBuffer->width;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_YV12:
            if (mLockedBuffer->stride % 16) {
                ALOGE("Stride %d is not 16 pixel aligned!", mLockedBuffer->stride);
                return AMEDIA_ERROR_UNKNOWN;
            }
            *rowStride = (planeIdx == 0) ? mLockedBuffer->stride
                                         : ALIGN(mLockedBuffer->stride / 2, 16);
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RAW10:
        case HAL_PIXEL_FORMAT_RAW12:
            // RAW10 and RAW12 are used for 10-bit and 12-bit raw data, they are single plane
            *rowStride = mLockedBuffer->stride;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_Y8:
            if (mLockedBuffer->stride % 16) {
                ALOGE("Stride %d is not 16 pixel aligned!",
                      mLockedBuffer->stride);
                return AMEDIA_ERROR_UNKNOWN;
            }
            *rowStride = mLockedBuffer->stride;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW16:
            // In native side, strides are specified in pixels, not in bytes.
            // Single plane 16bpp bayer data. even width/height,
            // row stride multiple of 16 pixels (32 bytes)
            if (mLockedBuffer->stride % 16) {
                ALOGE("Stride %d is not 16 pixel aligned!",
                      mLockedBuffer->stride);
                return AMEDIA_ERROR_UNKNOWN;
            }
            *rowStride = mLockedBuffer->stride * 2;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RGB_565:
            *rowStride = mLockedBuffer->stride * 2;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
            *rowStride = mLockedBuffer->stride * 4;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_RGB_888:
            // Single plane, 24bpp.
            *rowStride = mLockedBuffer->stride * 3;
            return AMEDIA_OK;
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            // Blob is used for JPEG/Raw opaque data. It is single plane and has 0 row stride and
            // no row stride defined
            return AMEDIA_ERROR_UNSUPPORTED;
        default:
            ALOGE("%s Pixel format: 0x%x is unsupported", __FUNCTION__, fmt);
          return AMEDIA_ERROR_UNSUPPORTED;
    }
}

uint32_t
AImage::getJpegSize() const {
    if (mLockedBuffer == nullptr) {
        LOG_ALWAYS_FATAL("Error: buffer is null");
    }

    uint32_t size = 0;
    uint32_t width = mLockedBuffer->width;
    uint8_t* jpegBuffer = mLockedBuffer->data;

    // First check for JPEG transport header at the end of the buffer
    uint8_t* header = jpegBuffer + (width - sizeof(struct camera3_jpeg_blob));
    struct camera3_jpeg_blob* blob = (struct camera3_jpeg_blob*)(header);
    if (blob->jpeg_blob_id == CAMERA3_JPEG_BLOB_ID) {
        size = blob->jpeg_size;
        ALOGV("%s: Jpeg size = %d", __FUNCTION__, size);
    }

    // failed to find size, default to whole buffer
    if (size == 0) {
        /*
         * This is a problem because not including the JPEG header
         * means that in certain rare situations a regular JPEG blob
         * will be misidentified as having a header, in which case
         * we will get a garbage size value.
         */
        ALOGW("%s: No JPEG header detected, defaulting to size=width=%d",
                __FUNCTION__, width);
        size = width;
    }

    return size;
}

media_status_t
AImage::getPlaneData(int planeIdx,/*out*/uint8_t** data, /*out*/int* dataLength) const {
    if (mLockedBuffer == nullptr) {
        ALOGE("%s: buffer not locked.", __FUNCTION__);
        return AMEDIA_IMGREADER_IMAGE_NOT_LOCKED;
    }

    if (planeIdx < 0 || planeIdx >= mNumPlanes) {
        ALOGE("Error: planeIdx %d out of bound [0,%d]",
                planeIdx, mNumPlanes - 1);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (data == nullptr || dataLength == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (isClosed()) {
        ALOGE("%s: image %p has been closed!", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    uint32_t dataSize, ySize, cSize, cStride;
    uint8_t* cb = nullptr;
    uint8_t* cr = nullptr;
    uint8_t* pData = nullptr;
    int bytesPerPixel = 0;
    int32_t fmt = mLockedBuffer->flexFormat;

    switch (fmt) {
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            pData = (planeIdx == 0) ? mLockedBuffer->data
                                    : (planeIdx == 1) ? mLockedBuffer->dataCb
                                                      : mLockedBuffer->dataCr;
            // only map until last pixel
            if (planeIdx == 0) {
                dataSize = mLockedBuffer->stride * (mLockedBuffer->height - 1) +
                           mLockedBuffer->width;
            } else {
                dataSize =
                    mLockedBuffer->chromaStride *
                        (mLockedBuffer->height / 2 - 1) +
                    mLockedBuffer->chromaStep * (mLockedBuffer->width / 2 - 1) +
                    1;
            }
            break;
        // NV21
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            cr = mLockedBuffer->data +
                 (mLockedBuffer->stride * mLockedBuffer->height);
            cb = cr + 1;
            // only map until last pixel
            ySize = mLockedBuffer->width * (mLockedBuffer->height - 1) +
                    mLockedBuffer->width;
            cSize = mLockedBuffer->width * (mLockedBuffer->height / 2 - 1) +
                    mLockedBuffer->width - 1;
            pData = (planeIdx == 0) ? mLockedBuffer->data
                                    : (planeIdx == 1) ? cb : cr;
            dataSize = (planeIdx == 0) ? ySize : cSize;
            break;
        case HAL_PIXEL_FORMAT_YV12:
            // Y and C stride need to be 16 pixel aligned.
            if (mLockedBuffer->stride % 16) {
                ALOGE("Stride %d is not 16 pixel aligned!",
                      mLockedBuffer->stride);
                return AMEDIA_ERROR_UNKNOWN;
            }

            ySize = mLockedBuffer->stride * mLockedBuffer->height;
            cStride = ALIGN(mLockedBuffer->stride / 2, 16);
            cr = mLockedBuffer->data + ySize;
            cSize = cStride * mLockedBuffer->height / 2;
            cb = cr + cSize;

            pData = (planeIdx == 0) ? mLockedBuffer->data
                                    : (planeIdx == 1) ? cb : cr;
            dataSize = (planeIdx == 0) ? ySize : cSize;
            break;
        case HAL_PIXEL_FORMAT_Y8:
            // Single plane, 8bpp.

            pData = mLockedBuffer->data;
            dataSize = mLockedBuffer->stride * mLockedBuffer->height;
            break;
        case HAL_PIXEL_FORMAT_Y16:
            bytesPerPixel = 2;

            pData = mLockedBuffer->data;
            dataSize =
                mLockedBuffer->stride * mLockedBuffer->height * bytesPerPixel;
            break;
        case HAL_PIXEL_FORMAT_BLOB:
            // Used for JPEG data, height must be 1, width == size, single plane.
            if (mLockedBuffer->height != 1) {
                ALOGE("Jpeg should have height value one but got %d",
                      mLockedBuffer->height);
                return AMEDIA_ERROR_UNKNOWN;
            }

            pData = mLockedBuffer->data;
            dataSize = getJpegSize();
            break;
        case HAL_PIXEL_FORMAT_RAW16:
            // Single plane 16bpp bayer data.
            bytesPerPixel = 2;
            pData = mLockedBuffer->data;
            dataSize =
                mLockedBuffer->stride * mLockedBuffer->height * bytesPerPixel;
            break;
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            // Used for RAW_OPAQUE data, height must be 1, width == size, single plane.
            if (mLockedBuffer->height != 1) {
                ALOGE("RAW_OPAQUE should have height value one but got %d",
                      mLockedBuffer->height);
                return AMEDIA_ERROR_UNKNOWN;
            }
            pData = mLockedBuffer->data;
            dataSize = mLockedBuffer->width;
            break;
        case HAL_PIXEL_FORMAT_RAW10:
            // Single plane 10bpp bayer data.
            if (mLockedBuffer->width % 4) {
                ALOGE("Width is not multiple of 4 %d", mLockedBuffer->width);
                return AMEDIA_ERROR_UNKNOWN;
            }
            if (mLockedBuffer->height % 2) {
                ALOGE("Height is not multiple of 2 %d", mLockedBuffer->height);
                return AMEDIA_ERROR_UNKNOWN;
            }
            if (mLockedBuffer->stride < (mLockedBuffer->width * 10 / 8)) {
                ALOGE("stride (%d) should be at least %d",
                        mLockedBuffer->stride, mLockedBuffer->width * 10 / 8);
                return AMEDIA_ERROR_UNKNOWN;
            }
            pData = mLockedBuffer->data;
            dataSize = mLockedBuffer->stride * mLockedBuffer->height;
            break;
        case HAL_PIXEL_FORMAT_RAW12:
            // Single plane 10bpp bayer data.
            if (mLockedBuffer->width % 4) {
                ALOGE("Width is not multiple of 4 %d", mLockedBuffer->width);
                return AMEDIA_ERROR_UNKNOWN;
            }
            if (mLockedBuffer->height % 2) {
                ALOGE("Height is not multiple of 2 %d", mLockedBuffer->height);
                return AMEDIA_ERROR_UNKNOWN;
            }
            if (mLockedBuffer->stride < (mLockedBuffer->width * 12 / 8)) {
                ALOGE("stride (%d) should be at least %d",
                        mLockedBuffer->stride, mLockedBuffer->width * 12 / 8);
                return AMEDIA_ERROR_UNKNOWN;
            }
            pData = mLockedBuffer->data;
            dataSize = mLockedBuffer->stride * mLockedBuffer->height;
            break;
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
            // Single plane, 32bpp.
            bytesPerPixel = 4;
            pData = mLockedBuffer->data;
            dataSize =
                mLockedBuffer->stride * mLockedBuffer->height * bytesPerPixel;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            // Single plane, 16bpp.
            bytesPerPixel = 2;
            pData = mLockedBuffer->data;
            dataSize =
                mLockedBuffer->stride * mLockedBuffer->height * bytesPerPixel;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            // Single plane, 24bpp.
            bytesPerPixel = 3;
            pData = mLockedBuffer->data;
            dataSize = mLockedBuffer->stride * mLockedBuffer->height * bytesPerPixel;
            break;
        default:
            ALOGE("Pixel format: 0x%x is unsupported", fmt);
            return AMEDIA_ERROR_UNSUPPORTED;
    }

    *data = pData;
    *dataLength = dataSize;
    return AMEDIA_OK;
}

media_status_t
AImage::getHardwareBuffer(/*out*/AHardwareBuffer** buffer) const {
    if (mBuffer == nullptr || mBuffer->mGraphicBuffer == nullptr) {
        ALOGE("%s: AImage %p has no buffer.", __FUNCTION__, this);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    // TODO(jwcai) Someone from Android graphics team stating this should just be a static_cast.
    *buffer = reinterpret_cast<AHardwareBuffer*>(mBuffer->mGraphicBuffer.get());
    return AMEDIA_OK;
}

EXPORT
void AImage_delete(AImage* image) {
    ALOGV("%s", __FUNCTION__);
    AImage_deleteAsync(image, -1);
    return;
}

EXPORT
void AImage_deleteAsync(AImage* image, int releaseFenceFd) {
    ALOGV("%s", __FUNCTION__);
    if (image != nullptr) {
        image->lockReader();
        image->close(releaseFenceFd);
        image->unlockReader();
        if (!image->isClosed()) {
            LOG_ALWAYS_FATAL("Image close failed!");
        }
        image->free();
    }
    return;
}

EXPORT
media_status_t AImage_getWidth(const AImage* image, /*out*/int32_t* width) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || width == nullptr) {
        ALOGE("%s: bad argument. image %p width %p",
                __FUNCTION__, image, width);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getWidth(width);
}

EXPORT
media_status_t AImage_getHeight(const AImage* image, /*out*/int32_t* height) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || height == nullptr) {
        ALOGE("%s: bad argument. image %p height %p",
                __FUNCTION__, image, height);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getHeight(height);
}

EXPORT
media_status_t AImage_getFormat(const AImage* image, /*out*/int32_t* format) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || format == nullptr) {
        ALOGE("%s: bad argument. image %p format %p",
                __FUNCTION__, image, format);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getFormat(format);
}

EXPORT
media_status_t AImage_getCropRect(const AImage* image, /*out*/AImageCropRect* rect) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || rect == nullptr) {
        ALOGE("%s: bad argument. image %p rect %p",
                __FUNCTION__, image, rect);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    // For now AImage only supports camera outputs where cropRect is always full window
    int32_t width = -1;
    media_status_t ret = image->getWidth(&width);
    if (ret != AMEDIA_OK) {
        return ret;
    }
    int32_t height = -1;
    ret = image->getHeight(&height);
    if (ret != AMEDIA_OK) {
        return ret;
    }
    rect->left = 0;
    rect->top = 0;
    rect->right = width;
    rect->bottom = height;
    return AMEDIA_OK;
}

EXPORT
media_status_t AImage_getTimestamp(const AImage* image, /*out*/int64_t* timestampNs) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || timestampNs == nullptr) {
        ALOGE("%s: bad argument. image %p timestampNs %p",
                __FUNCTION__, image, timestampNs);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getTimestamp(timestampNs);
}

EXPORT
media_status_t AImage_getNumberOfPlanes(const AImage* image, /*out*/int32_t* numPlanes) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || numPlanes == nullptr) {
        ALOGE("%s: bad argument. image %p numPlanes %p",
                __FUNCTION__, image, numPlanes);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getNumPlanes(numPlanes);
}

EXPORT
media_status_t AImage_getPlanePixelStride(
        const AImage* image, int planeIdx, /*out*/int32_t* pixelStride) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || pixelStride == nullptr) {
        ALOGE("%s: bad argument. image %p pixelStride %p",
                __FUNCTION__, image, pixelStride);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    media_status_t ret = const_cast<AImage*>(image)->lockImage();
    if (ret != AMEDIA_OK) {
        ALOGE("%s: failed to lock buffer for CPU access. image %p, error=%d.",
              __FUNCTION__, image, ret);
        return ret;
    }
    return image->getPlanePixelStride(planeIdx, pixelStride);
}

EXPORT
media_status_t AImage_getPlaneRowStride(
        const AImage* image, int planeIdx, /*out*/int32_t* rowStride) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || rowStride == nullptr) {
        ALOGE("%s: bad argument. image %p rowStride %p",
                __FUNCTION__, image, rowStride);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    media_status_t ret = const_cast<AImage*>(image)->lockImage();
    if (ret != AMEDIA_OK) {
        ALOGE("%s: failed to lock buffer for CPU access. image %p, error=%d.",
              __FUNCTION__, image, ret);
        return ret;
    }
    return image->getPlaneRowStride(planeIdx, rowStride);
}

EXPORT
media_status_t AImage_getPlaneData(
        const AImage* image, int planeIdx,
        /*out*/uint8_t** data, /*out*/int* dataLength) {
    ALOGV("%s", __FUNCTION__);
    if (image == nullptr || data == nullptr || dataLength == nullptr) {
        ALOGE("%s: bad argument. image %p data %p dataLength %p",
                __FUNCTION__, image, data, dataLength);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    media_status_t ret = const_cast<AImage*>(image)->lockImage();
    if (ret != AMEDIA_OK) {
        ALOGE("%s: failed to lock buffer for CPU access. image %p, error=%d.",
              __FUNCTION__, image, ret);
        return ret;
    }
    return image->getPlaneData(planeIdx, data, dataLength);
}

EXPORT
media_status_t AImage_getHardwareBuffer(
    const AImage* image, /*out*/AHardwareBuffer** buffer) {
    ALOGV("%s", __FUNCTION__);

    if (image == nullptr || buffer == nullptr) {
        ALOGE("%s: bad argument. image %p buffer %p", __FUNCTION__, image, buffer);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return image->getHardwareBuffer(buffer);
}
