/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "rsAllocation.h"
#include "rsContext.h"
#include "rsGrallocConsumer.h"
#include "rs_hal.h"

namespace android {
namespace renderscript {

GrallocConsumer::GrallocConsumer (const Context *rsc, Allocation *a, uint32_t numAlloc)
{
    mCtx = rsc;
    mAlloc = new Allocation *[numAlloc];
    mAcquiredBuffer = new AcquiredBuffer[numAlloc];
    isIdxUsed = new bool[numAlloc];

    mAlloc[0] = a;
    isIdxUsed[0] = true;
    mNumAlloc = numAlloc;

    uint32_t width  = a->mHal.drvState.lod[0].dimX;
    uint32_t height = a->mHal.drvState.lod[0].dimY;
    if (height < 1) height = 1;

    int32_t format = AIMAGE_FORMAT_RGBA_8888;
    if (a->mHal.state.yuv) {
        format = AIMAGE_FORMAT_YUV_420_888;
    }

    // GRALLOC_USAGE_RENDERSCRIPT
    const uint64_t USAGE_RENDERSCRIPT = 0x00100000U;
    uint64_t usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | USAGE_RENDERSCRIPT;
    media_status_t ret = AImageReader_newWithUsage(
            width, height, format, usage,
            mNumAlloc, &mImgReader);
    if (ret != AMEDIA_OK || mImgReader == nullptr) {
        ALOGE("Error creating image reader. ret %d", ret);
    }

    ret = AImageReader_getWindow(mImgReader, &mNativeWindow);
    if (ret != AMEDIA_OK || mNativeWindow == nullptr) {
        ALOGE("Error creating native window. ret %d", ret);
    }

    mReaderCb = {this, GrallocConsumer::onFrameAvailable};
    ret = AImageReader_setImageListener(mImgReader, &mReaderCb);

    for (uint32_t i = 1; i < numAlloc; i++) {
        isIdxUsed[i] = false;
    }
}

GrallocConsumer::~GrallocConsumer() {
    AImageReader_delete(mImgReader);
    delete[] mAlloc;
    delete[] mAcquiredBuffer;
    delete[] isIdxUsed;
}

void GrallocConsumer::onFrameAvailable(void* obj, AImageReader* reader) {
    GrallocConsumer* consumer = (GrallocConsumer *) obj;
    for (uint32_t i = 0; i < consumer->mNumAlloc; i++) {
        if (consumer->mAlloc[i] != nullptr) {
            intptr_t ip = (intptr_t)(consumer->mAlloc[i]);
            consumer->mCtx->sendMessageToClient(&ip,
                RS_MESSAGE_TO_CLIENT_NEW_BUFFER, 0, sizeof(ip), true);
        }
    }
}

ANativeWindow* GrallocConsumer::getNativeWindow() {
    return mNativeWindow;
}

media_status_t GrallocConsumer::lockNextBuffer(uint32_t idx) {
    media_status_t ret;

    if (idx >= mNumAlloc) {
        ALOGE("Invalid buffer index: %d", idx);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (mAcquiredBuffer[idx].mImg != nullptr) {
        ret = unlockBuffer(idx);
        if (ret != AMEDIA_OK) {
            return ret;
        }
    }

    ret = AImageReader_acquireNextImage(mImgReader, &(mAcquiredBuffer[idx].mImg));
    if (ret != AMEDIA_OK || mAcquiredBuffer[idx].mImg == nullptr) {
        ALOGE("%s: acquire image from reader %p failed! ret: %d, img %p",
                __FUNCTION__, mImgReader, ret, mAcquiredBuffer[idx].mImg);
        return ret;
    }

    AImage *img = mAcquiredBuffer[idx].mImg;
    int32_t format = -1;
    ret = AImage_getFormat(img, &format);
    if (ret != AMEDIA_OK || format == -1) {
        ALOGE("%s: get format for image %p failed! ret: %d, format %d",
                 __FUNCTION__, img, ret, format);
        return ret;
    }

    if (format != AIMAGE_FORMAT_YUV_420_888 && format != AIMAGE_FORMAT_RGBA_8888) {
        ALOGE("Format %d not supported", format);
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    uint8_t *data = nullptr;
    int dataLength = 0;
    ret =  AImage_getPlaneData(img, 0, &data, &dataLength);
    if (ret != AMEDIA_OK || data == nullptr || dataLength <= 0) {
        ALOGE("%s: get data for image %p failed! ret: %d, data %p, len %d",
                __FUNCTION__, img, ret, data, dataLength);
        return ret;
    }

    int64_t timestamp = -1;
    ret = AImage_getTimestamp(img, &timestamp);
    if (ret != AMEDIA_OK || timestamp == -1) {
        ALOGE("%s: get timestamp for image %p failed! ret: %d",
                __FUNCTION__, img, ret);
        return ret;
    }

    int32_t rowstride = -1;
    ret = AImage_getPlaneRowStride(img, 0, &rowstride);
    if (ret != AMEDIA_OK || rowstride == -1) {
        ALOGE("%s: get row stride for image %p failed! ret: %d, rowstride %d",
                __FUNCTION__, img, ret, rowstride);
        return ret;
    }

    AHardwareBuffer *hardwareBuffer = nullptr;
    ret =  AImage_getHardwareBuffer(img, &hardwareBuffer);
    if (ret != AMEDIA_OK || hardwareBuffer == nullptr) {
        ALOGE("%s: get hardware buffer for image %p failed! ret: %d",
                __FUNCTION__, img, ret);
        return ret;
    }

    mAcquiredBuffer[idx].mBufferPointer = data;

    mAlloc[idx]->mHal.drvState.lod[0].mallocPtr = data;
    mAlloc[idx]->mHal.drvState.lod[0].stride = rowstride;
    mAlloc[idx]->mHal.state.nativeBuffer = hardwareBuffer;
    mAlloc[idx]->mHal.state.timestamp = timestamp;

    if (format == AIMAGE_FORMAT_YUV_420_888) {
        const int yWidth = mAlloc[idx]->mHal.drvState.lod[0].dimX;
        const int yHeight = mAlloc[idx]->mHal.drvState.lod[0].dimY;

        const int cWidth = yWidth / 2;
        const int cHeight = yHeight / 2;

        uint8_t *uData = nullptr;
        int uDataLength = 0;
        ret =  AImage_getPlaneData(img, 1, &uData, &uDataLength);
        if (ret != AMEDIA_OK || uData == nullptr || uDataLength <= 0) {
            ALOGE("%s: get U data for image %p failed! ret: %d, data %p, len %d",
                    __FUNCTION__, img, ret, uData, uDataLength);
            return ret;
        }

        uint8_t *vData = nullptr;
        int vDataLength = 0;
        ret =  AImage_getPlaneData(img, 2, &vData, &vDataLength);
        if (ret != AMEDIA_OK || vData == nullptr || vDataLength <= 0) {
            ALOGE("%s: get V data for image %p failed! ret: %d, data %p, len %d",
                    __FUNCTION__, img, ret, vData, vDataLength);
            return ret;
        }

        int32_t uRowStride = -1;
        ret = AImage_getPlaneRowStride(img, 1, &uRowStride);
        if (ret != AMEDIA_OK || uRowStride == -1) {
            ALOGE("%s: get U row stride for image %p failed! ret: %d, uRowStride %d",
                    __FUNCTION__, img, ret, uRowStride);
            return ret;
        }

        int32_t vRowStride = -1;
        ret = AImage_getPlaneRowStride(img, 2, &vRowStride);
        if (ret != AMEDIA_OK || vRowStride == -1) {
            ALOGE("%s: get V row stride for image %p failed! ret: %d, vRowStride %d",
                    __FUNCTION__, img, ret, vRowStride);
            return ret;
        }

        int32_t uPixStride = -1;
        ret = AImage_getPlanePixelStride(img, 1, &uPixStride);
        if (ret != AMEDIA_OK || uPixStride == -1) {
            ALOGE("%s: get U pixel stride for image %p failed! ret: %d, uPixStride %d",
                    __FUNCTION__, img, ret, uPixStride);
            return ret;
        }

        mAlloc[idx]->mHal.drvState.lod[1].dimX = cWidth;
        mAlloc[idx]->mHal.drvState.lod[1].dimY = cHeight;
        mAlloc[idx]->mHal.drvState.lod[2].dimX = cWidth;
        mAlloc[idx]->mHal.drvState.lod[2].dimY = cHeight;

        mAlloc[idx]->mHal.drvState.lod[1].mallocPtr = uData;
        mAlloc[idx]->mHal.drvState.lod[2].mallocPtr = vData;

        mAlloc[idx]->mHal.drvState.lod[1].stride = uRowStride;
        mAlloc[idx]->mHal.drvState.lod[2].stride = vRowStride;

        mAlloc[idx]->mHal.drvState.yuv.shift = 1;
        mAlloc[idx]->mHal.drvState.yuv.step = uPixStride;
        mAlloc[idx]->mHal.drvState.lodCount = 3;
    }

    return AMEDIA_OK;
}

media_status_t GrallocConsumer::unlockBuffer(uint32_t idx) {
    media_status_t ret;

    if (idx >= mNumAlloc) {
        ALOGE("Invalid buffer index: %d", idx);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    if (mAcquiredBuffer[idx].mImg == nullptr) {
       return AMEDIA_OK;
    }

    AImage_delete(mAcquiredBuffer[idx].mImg);
    mAcquiredBuffer[idx].mImg = nullptr;
    return AMEDIA_OK;
}

uint32_t GrallocConsumer::getNextAvailableIdx(Allocation *a) {
    for (uint32_t i = 0; i < mNumAlloc; i++) {
        if (isIdxUsed[i] == false) {
            mAlloc[i] = a;
            isIdxUsed[i] = true;
            return i;
        }
    }
    return mNumAlloc;
}

bool GrallocConsumer::releaseIdx(uint32_t idx) {
    if (idx >= mNumAlloc) {
        ALOGE("Invalid buffer index: %d", idx);
        return false;
    }
    if (isIdxUsed[idx] == false) {
        ALOGV("Buffer index already released: %d", idx);
        return true;
    }
    media_status_t ret;
    ret = unlockBuffer(idx);
    if (ret != OK) {
        ALOGE("Unable to unlock graphic buffer");
        return false;
    }
    mAlloc[idx] = nullptr;
    isIdxUsed[idx] = false;
    return true;
}

bool GrallocConsumer::isActive() {
    for (uint32_t i = 0; i < mNumAlloc; i++) {
        if (isIdxUsed[i]) {
            return true;
        }
    }
    return false;
}

} // namespace renderscript
} // namespace android
