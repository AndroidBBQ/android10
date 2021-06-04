/*
 * Copyright (C) 2012 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "CpuConsumer"
//#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <gui/CpuConsumer.h>

#include <gui/BufferItem.h>
#include <utils/Log.h>

#define CC_LOGV(x, ...) ALOGV("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define CC_LOGD(x, ...) ALOGD("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define CC_LOGI(x, ...) ALOGI("[%s] " x, mName.string(), ##__VA_ARGS__)
#define CC_LOGW(x, ...) ALOGW("[%s] " x, mName.string(), ##__VA_ARGS__)
#define CC_LOGE(x, ...) ALOGE("[%s] " x, mName.string(), ##__VA_ARGS__)

namespace android {

CpuConsumer::CpuConsumer(const sp<IGraphicBufferConsumer>& bq,
        size_t maxLockedBuffers, bool controlledByApp) :
    ConsumerBase(bq, controlledByApp),
    mMaxLockedBuffers(maxLockedBuffers),
    mCurrentLockedBuffers(0)
{
    // Create tracking entries for locked buffers
    mAcquiredBuffers.insertAt(0, maxLockedBuffers);

    mConsumer->setConsumerUsageBits(GRALLOC_USAGE_SW_READ_OFTEN);
    mConsumer->setMaxAcquiredBufferCount(static_cast<int32_t>(maxLockedBuffers));
}

size_t CpuConsumer::findAcquiredBufferLocked(uintptr_t id) const {
    for (size_t i = 0; i < mMaxLockedBuffers; i++) {
        const auto& ab = mAcquiredBuffers[i];
        // note that this finds AcquiredBuffer::kUnusedId as well
        if (ab.mLockedBufferId == id) {
            return i;
        }
    }
    return mMaxLockedBuffers; // an invalid index
}

static uintptr_t getLockedBufferId(const CpuConsumer::LockedBuffer& buffer) {
    return reinterpret_cast<uintptr_t>(buffer.data);
}

static bool isPossiblyYUV(PixelFormat format) {
    switch (static_cast<int>(format)) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_FP16:
        case HAL_PIXEL_FORMAT_RGBA_1010102:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_Y8:
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW16:
        case HAL_PIXEL_FORMAT_RAW10:
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
            return false;

        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        default:
            return true;
    }
}

status_t CpuConsumer::lockBufferItem(const BufferItem& item, LockedBuffer* outBuffer) const {
    android_ycbcr ycbcr = android_ycbcr();

    PixelFormat format = item.mGraphicBuffer->getPixelFormat();
    PixelFormat flexFormat = format;
    if (isPossiblyYUV(format)) {
        int fenceFd = item.mFence.get() ? item.mFence->dup() : -1;
        status_t err = item.mGraphicBuffer->lockAsyncYCbCr(GraphicBuffer::USAGE_SW_READ_OFTEN,
                                                           item.mCrop, &ycbcr, fenceFd);
        if (err == OK) {
            flexFormat = HAL_PIXEL_FORMAT_YCbCr_420_888;
            if (format != HAL_PIXEL_FORMAT_YCbCr_420_888) {
                CC_LOGV("locking buffer of format %#x as flex YUV", format);
            }
        } else if (format == HAL_PIXEL_FORMAT_YCbCr_420_888) {
            CC_LOGE("Unable to lock YCbCr buffer for CPU reading: %s (%d)", strerror(-err), err);
            return err;
        }
    }

    if (ycbcr.y != nullptr) {
        outBuffer->data = reinterpret_cast<uint8_t*>(ycbcr.y);
        outBuffer->stride = static_cast<uint32_t>(ycbcr.ystride);
        outBuffer->dataCb = reinterpret_cast<uint8_t*>(ycbcr.cb);
        outBuffer->dataCr = reinterpret_cast<uint8_t*>(ycbcr.cr);
        outBuffer->chromaStride = static_cast<uint32_t>(ycbcr.cstride);
        outBuffer->chromaStep = static_cast<uint32_t>(ycbcr.chroma_step);
    } else {
        // not flexible YUV; try lockAsync
        void* bufferPointer = nullptr;
        int fenceFd = item.mFence.get() ? item.mFence->dup() : -1;
        status_t err = item.mGraphicBuffer->lockAsync(GraphicBuffer::USAGE_SW_READ_OFTEN,
                                                      item.mCrop, &bufferPointer, fenceFd);
        if (err != OK) {
            CC_LOGE("Unable to lock buffer for CPU reading: %s (%d)", strerror(-err), err);
            return err;
        }

        outBuffer->data = reinterpret_cast<uint8_t*>(bufferPointer);
        outBuffer->stride = item.mGraphicBuffer->getStride();
        outBuffer->dataCb = nullptr;
        outBuffer->dataCr = nullptr;
        outBuffer->chromaStride = 0;
        outBuffer->chromaStep = 0;
    }

    outBuffer->width = item.mGraphicBuffer->getWidth();
    outBuffer->height = item.mGraphicBuffer->getHeight();
    outBuffer->format = format;
    outBuffer->flexFormat = flexFormat;

    outBuffer->crop = item.mCrop;
    outBuffer->transform = item.mTransform;
    outBuffer->scalingMode = item.mScalingMode;
    outBuffer->timestamp = item.mTimestamp;
    outBuffer->dataSpace = item.mDataSpace;
    outBuffer->frameNumber = item.mFrameNumber;

    return OK;
}

status_t CpuConsumer::lockNextBuffer(LockedBuffer *nativeBuffer) {
    status_t err;

    if (!nativeBuffer) return BAD_VALUE;

    Mutex::Autolock _l(mMutex);

    if (mCurrentLockedBuffers == mMaxLockedBuffers) {
        CC_LOGW("Max buffers have been locked (%zd), cannot lock anymore.",
                mMaxLockedBuffers);
        return NOT_ENOUGH_DATA;
    }

    BufferItem b;
    err = acquireBufferLocked(&b, 0);
    if (err != OK) {
        if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
            return BAD_VALUE;
        } else {
            CC_LOGE("Error acquiring buffer: %s (%d)", strerror(err), err);
            return err;
        }
    }

    if (b.mGraphicBuffer == nullptr) {
        b.mGraphicBuffer = mSlots[b.mSlot].mGraphicBuffer;
    }

    err = lockBufferItem(b, nativeBuffer);
    if (err != OK) {
        return err;
    }

    // find an unused AcquiredBuffer
    size_t lockedIdx = findAcquiredBufferLocked(AcquiredBuffer::kUnusedId);
    ALOG_ASSERT(lockedIdx < mMaxLockedBuffers);
    AcquiredBuffer& ab = mAcquiredBuffers.editItemAt(lockedIdx);

    ab.mSlot = b.mSlot;
    ab.mGraphicBuffer = b.mGraphicBuffer;
    ab.mLockedBufferId = getLockedBufferId(*nativeBuffer);

    mCurrentLockedBuffers++;

    return OK;
}

status_t CpuConsumer::unlockBuffer(const LockedBuffer &nativeBuffer) {
    Mutex::Autolock _l(mMutex);

    uintptr_t id = getLockedBufferId(nativeBuffer);
    size_t lockedIdx =
        (id != AcquiredBuffer::kUnusedId) ? findAcquiredBufferLocked(id) : mMaxLockedBuffers;
    if (lockedIdx == mMaxLockedBuffers) {
        CC_LOGE("%s: Can't find buffer to free", __FUNCTION__);
        return BAD_VALUE;
    }

    AcquiredBuffer& ab = mAcquiredBuffers.editItemAt(lockedIdx);

    int fenceFd = -1;
    status_t err = ab.mGraphicBuffer->unlockAsync(&fenceFd);
    if (err != OK) {
        CC_LOGE("%s: Unable to unlock graphic buffer %zd", __FUNCTION__,
                lockedIdx);
        return err;
    }

    sp<Fence> fence(fenceFd >= 0 ? new Fence(fenceFd) : Fence::NO_FENCE);
    addReleaseFenceLocked(ab.mSlot, ab.mGraphicBuffer, fence);
    releaseBufferLocked(ab.mSlot, ab.mGraphicBuffer);

    ab.reset();

    mCurrentLockedBuffers--;

    return OK;
}

} // namespace android
