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
#define LOG_TAG "NdkImageReader"

#include "NdkImagePriv.h"
#include "NdkImageReaderPriv.h"
#include <private/media/NdkImage.h>

#include <cutils/atomic.h>
#include <utils/Log.h>
#include <android_media_Utils.h>
#include <ui/PublicFormat.h>
#include <private/android/AHardwareBufferHelpers.h>
#include <grallocusage/GrallocUsageConversion.h>
#include <gui/bufferqueue/1.0/WGraphicBufferProducer.h>

using namespace android;

namespace {
    // Get an ID that's unique within this process.
    static int32_t createProcessUniqueId() {
        static volatile int32_t globalCounter = 0;
        return android_atomic_inc(&globalCounter);
    }
}

const char* AImageReader::kCallbackFpKey = "Callback";
const char* AImageReader::kContextKey    = "Context";
const char* AImageReader::kGraphicBufferKey = "GraphicBuffer";

static constexpr int kWindowHalTokenSizeMax = 256;

static native_handle_t *convertHalTokenToNativeHandle(const HalToken &halToken);

bool
AImageReader::isSupportedFormatAndUsage(int32_t format, uint64_t usage) {
    // Check whether usage has either CPU_READ_OFTEN or CPU_READ set. Note that check against
    // AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN (0x6) is sufficient as it implies
    // AHARDWAREBUFFER_USAGE_CPU_READ (0x2).
    bool hasCpuUsage = usage & AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN;
    switch (format) {
        case AIMAGE_FORMAT_RGBA_8888:
        case AIMAGE_FORMAT_RGBX_8888:
        case AIMAGE_FORMAT_RGB_888:
        case AIMAGE_FORMAT_RGB_565:
        case AIMAGE_FORMAT_RGBA_FP16:
        case AIMAGE_FORMAT_YUV_420_888:
        case AIMAGE_FORMAT_JPEG:
        case AIMAGE_FORMAT_RAW16:
        case AIMAGE_FORMAT_RAW_DEPTH:
        case AIMAGE_FORMAT_RAW_PRIVATE:
        case AIMAGE_FORMAT_RAW10:
        case AIMAGE_FORMAT_RAW12:
        case AIMAGE_FORMAT_DEPTH16:
        case AIMAGE_FORMAT_DEPTH_POINT_CLOUD:
        case AIMAGE_FORMAT_Y8:
        case AIMAGE_FORMAT_HEIC:
        case AIMAGE_FORMAT_DEPTH_JPEG:
            return true;
        case AIMAGE_FORMAT_PRIVATE:
            // For private format, cpu usage is prohibited.
            return !hasCpuUsage;
        default:
            return false;
    }
}

int
AImageReader::getNumPlanesForFormat(int32_t format) {
    switch (format) {
        case AIMAGE_FORMAT_YUV_420_888:
            return 3;
        case AIMAGE_FORMAT_RGBA_8888:
        case AIMAGE_FORMAT_RGBX_8888:
        case AIMAGE_FORMAT_RGB_888:
        case AIMAGE_FORMAT_RGB_565:
        case AIMAGE_FORMAT_RGBA_FP16:
        case AIMAGE_FORMAT_JPEG:
        case AIMAGE_FORMAT_RAW16:
        case AIMAGE_FORMAT_RAW_DEPTH:
        case AIMAGE_FORMAT_RAW_PRIVATE:
        case AIMAGE_FORMAT_RAW10:
        case AIMAGE_FORMAT_RAW12:
        case AIMAGE_FORMAT_DEPTH16:
        case AIMAGE_FORMAT_DEPTH_POINT_CLOUD:
        case AIMAGE_FORMAT_Y8:
        case AIMAGE_FORMAT_HEIC:
        case AIMAGE_FORMAT_DEPTH_JPEG:
            return 1;
        case AIMAGE_FORMAT_PRIVATE:
            return 0;
        default:
            return -1;
    }
}

void
AImageReader::FrameListener::onFrameAvailable(const BufferItem& /*item*/) {
    sp<AImageReader> reader = mReader.promote();
    if (reader == nullptr) {
        ALOGW("A frame is available after AImageReader closed!");
        return; // reader has been closed
    }
    Mutex::Autolock _l(mLock);
    if (mListener.onImageAvailable == nullptr) {
        return; // No callback registered
    }

    sp<AMessage> msg = new AMessage(AImageReader::kWhatImageAvailable, reader->mHandler);
    msg->setPointer(AImageReader::kCallbackFpKey, (void *) mListener.onImageAvailable);
    msg->setPointer(AImageReader::kContextKey, mListener.context);
    msg->post();
}

media_status_t
AImageReader::FrameListener::setImageListener(AImageReader_ImageListener* listener) {
    Mutex::Autolock _l(mLock);
    if (listener == nullptr) {
        mListener.context = nullptr;
        mListener.onImageAvailable = nullptr;
    } else {
        mListener = *listener;
    }
    return AMEDIA_OK;
}

void
AImageReader::BufferRemovedListener::onBufferFreed(const wp<GraphicBuffer>& graphicBuffer) {
    sp<AImageReader> reader = mReader.promote();
    if (reader == nullptr) {
        ALOGW("A frame is available after AImageReader closed!");
        return; // reader has been closed
    }
    Mutex::Autolock _l(mLock);
    if (mListener.onBufferRemoved == nullptr) {
        return; // No callback registered
    }

    sp<GraphicBuffer> gBuffer = graphicBuffer.promote();
    if (gBuffer == nullptr) {
        ALOGW("A buffer being freed has gone away!");
        return; // buffer is already destroyed
    }

    sp<AMessage> msg = new AMessage(AImageReader::kWhatBufferRemoved, reader->mHandler);
    msg->setPointer(
        AImageReader::kCallbackFpKey, (void*) mListener.onBufferRemoved);
    msg->setPointer(AImageReader::kContextKey, mListener.context);
    msg->setObject(AImageReader::kGraphicBufferKey, gBuffer);
    msg->post();
}

media_status_t
AImageReader::BufferRemovedListener::setBufferRemovedListener(
    AImageReader_BufferRemovedListener* listener) {
    Mutex::Autolock _l(mLock);
    if (listener == nullptr) {
        mListener.context = nullptr;
        mListener.onBufferRemoved = nullptr;
    } else {
        mListener = *listener;
    }
    return AMEDIA_OK;
}

media_status_t
AImageReader::setImageListenerLocked(AImageReader_ImageListener* listener) {
    return mFrameListener->setImageListener(listener);
}

media_status_t
AImageReader::setImageListener(AImageReader_ImageListener* listener) {
    Mutex::Autolock _l(mLock);
    return setImageListenerLocked(listener);
}

media_status_t
AImageReader::setBufferRemovedListenerLocked(AImageReader_BufferRemovedListener* listener) {
    return mBufferRemovedListener->setBufferRemovedListener(listener);
}

media_status_t
AImageReader::setBufferRemovedListener(AImageReader_BufferRemovedListener* listener) {
    Mutex::Autolock _l(mLock);
    return setBufferRemovedListenerLocked(listener);
}

void AImageReader::CallbackHandler::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatBufferRemoved:
        {
            AImageReader_BufferRemovedCallback onBufferRemoved;
            void* context;
            bool found = msg->findPointer(kCallbackFpKey, (void**) &onBufferRemoved);
            if (!found || onBufferRemoved == nullptr) {
                ALOGE("%s: Cannot find onBufferRemoved callback fp!", __FUNCTION__);
                return;
            }
            found = msg->findPointer(kContextKey, &context);
            if (!found) {
                ALOGE("%s: Cannot find callback context!", __FUNCTION__);
                return;
            }
            sp<RefBase> bufferToFree;
            found = msg->findObject(kGraphicBufferKey, &bufferToFree);
            if (!found || bufferToFree == nullptr) {
                ALOGE("%s: Cannot find the buffer to free!", __FUNCTION__);
                return;
            }

            // TODO(jwcai) Someone from Android graphics team stating this should just be a
            // static_cast.
            AHardwareBuffer* outBuffer = reinterpret_cast<AHardwareBuffer*>(bufferToFree.get());

            // At this point, bufferToFree holds the last reference to the GraphicBuffer owned by
            // this AImageReader, and the reference will be gone once this function returns.
            (*onBufferRemoved)(context, mReader, outBuffer);
            break;
        }
        case kWhatImageAvailable:
        {
            AImageReader_ImageCallback onImageAvailable;
            void* context;
            bool found = msg->findPointer(kCallbackFpKey, (void**) &onImageAvailable);
            if (!found || onImageAvailable == nullptr) {
                ALOGE("%s: Cannot find onImageAvailable callback fp!", __FUNCTION__);
                return;
            }
            found = msg->findPointer(kContextKey, &context);
            if (!found) {
                ALOGE("%s: Cannot find callback context!", __FUNCTION__);
                return;
            }
            (*onImageAvailable)(context, mReader);
            break;
        }
        default:
            ALOGE("%s: unknown message type %d", __FUNCTION__, msg->what());
            break;
    }
}

AImageReader::AImageReader(int32_t width,
                           int32_t height,
                           int32_t format,
                           uint64_t usage,
                           int32_t maxImages)
    : mWidth(width),
      mHeight(height),
      mFormat(format),
      mUsage(usage),
      mMaxImages(maxImages),
      mNumPlanes(getNumPlanesForFormat(format)),
      mFrameListener(new FrameListener(this)),
      mBufferRemovedListener(new BufferRemovedListener(this)) {}

AImageReader::~AImageReader() {
    Mutex::Autolock _l(mLock);
    LOG_FATAL_IF("AImageReader not closed before destruction", mIsClosed != true);
}

media_status_t
AImageReader::init() {
    PublicFormat publicFormat = static_cast<PublicFormat>(mFormat);
    mHalFormat = mapPublicFormatToHalFormat(publicFormat);
    mHalDataSpace = mapPublicFormatToHalDataspace(publicFormat);
    mHalUsage = AHardwareBuffer_convertToGrallocUsageBits(mUsage);

    sp<IGraphicBufferProducer> gbProducer;
    sp<IGraphicBufferConsumer> gbConsumer;
    BufferQueue::createBufferQueue(&gbProducer, &gbConsumer);

    String8 consumerName = String8::format("ImageReader-%dx%df%xu%" PRIu64 "m%d-%d-%d",
            mWidth, mHeight, mFormat, mUsage, mMaxImages, getpid(),
            createProcessUniqueId());

    mBufferItemConsumer =
            new BufferItemConsumer(gbConsumer, mHalUsage, mMaxImages, /*controlledByApp*/ true);
    if (mBufferItemConsumer == nullptr) {
        ALOGE("Failed to allocate BufferItemConsumer");
        return AMEDIA_ERROR_UNKNOWN;
    }

    mProducer = gbProducer;
    mBufferItemConsumer->setName(consumerName);
    mBufferItemConsumer->setFrameAvailableListener(mFrameListener);
    mBufferItemConsumer->setBufferFreedListener(mBufferRemovedListener);

    status_t res;
    res = mBufferItemConsumer->setDefaultBufferSize(mWidth, mHeight);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer size");
        return AMEDIA_ERROR_UNKNOWN;
    }
    res = mBufferItemConsumer->setDefaultBufferFormat(mHalFormat);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer format");
        return AMEDIA_ERROR_UNKNOWN;
    }
    res = mBufferItemConsumer->setDefaultBufferDataSpace(mHalDataSpace);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer dataSpace");
        return AMEDIA_ERROR_UNKNOWN;
    }
    if (mUsage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) {
        gbConsumer->setConsumerIsProtected(true);
    }

    mSurface = new Surface(mProducer, /*controlledByApp*/true);
    if (mSurface == nullptr) {
        ALOGE("Failed to create surface");
        return AMEDIA_ERROR_UNKNOWN;
    }
    mWindow = static_cast<ANativeWindow*>(mSurface.get());

    for (int i = 0; i < mMaxImages; i++) {
        BufferItem* buffer = new BufferItem;
        mBuffers.push_back(buffer);
    }

    mCbLooper = new ALooper;
    mCbLooper->setName(consumerName.string());
    res = mCbLooper->start(
            /*runOnCallingThread*/false,
            /*canCallJava*/       true,
            PRIORITY_DEFAULT);
    if (res != OK) {
        ALOGE("Failed to start the looper");
        return AMEDIA_ERROR_UNKNOWN;
    }
    mHandler = new CallbackHandler(this);
    mCbLooper->registerHandler(mHandler);

    return AMEDIA_OK;
}

void AImageReader::close() {
    Mutex::Autolock _l(mLock);
    if (mIsClosed) {
        return;
    }
    mIsClosed = true;
    AImageReader_ImageListener nullListener = {nullptr, nullptr};
    setImageListenerLocked(&nullListener);

    AImageReader_BufferRemovedListener nullBufferRemovedListener = {nullptr, nullptr};
    setBufferRemovedListenerLocked(&nullBufferRemovedListener);

    if (mCbLooper != nullptr) {
        mCbLooper->unregisterHandler(mHandler->id());
        mCbLooper->stop();
    }
    mCbLooper.clear();
    mHandler.clear();

    // Close all previously acquired images
    for (auto it = mAcquiredImages.begin();
              it != mAcquiredImages.end(); it++) {
        AImage* image = *it;
        Mutex::Autolock _l(image->mLock);
        // Do not alter mAcquiredImages while we are iterating on it
        releaseImageLocked(image, /*releaseFenceFd*/-1, /*clearCache*/false);
    }
    mAcquiredImages.clear();

    // Delete Buffer Items
    for (auto it = mBuffers.begin();
              it != mBuffers.end(); it++) {
        delete *it;
    }

    if (mBufferItemConsumer != nullptr) {
        mBufferItemConsumer->abandon();
        mBufferItemConsumer->setFrameAvailableListener(nullptr);
    }

    if (mWindowHandle != nullptr) {
        int size = mWindowHandle->data[0];
        hidl_vec<uint8_t> halToken;
        halToken.setToExternal(
            reinterpret_cast<uint8_t *>(&mWindowHandle->data[1]), size);
        deleteHalToken(halToken);
        native_handle_delete(mWindowHandle);
    }
}

media_status_t
AImageReader::acquireImageLocked(/*out*/AImage** image, /*out*/int* acquireFenceFd) {
    *image = nullptr;
    BufferItem* buffer = getBufferItemLocked();
    if (buffer == nullptr) {
        ALOGW("Unable to acquire a lockedBuffer, very likely client tries to lock more than"
            " maxImages buffers");
        return AMEDIA_IMGREADER_MAX_IMAGES_ACQUIRED;
    }

    // When the output paramter fence is not NULL, we are acquiring the image asynchronously.
    bool waitForFence = acquireFenceFd == nullptr;
    status_t res = mBufferItemConsumer->acquireBuffer(buffer, 0, waitForFence);

    if (res != NO_ERROR) {
        returnBufferItemLocked(buffer);
        if (res != BufferQueue::NO_BUFFER_AVAILABLE) {
            if (res == INVALID_OPERATION) {
                return AMEDIA_IMGREADER_MAX_IMAGES_ACQUIRED;
            } else {
                ALOGE("%s: Acquire image failed with some unknown error: %s (%d)",
                      __FUNCTION__, strerror(-res), res);
                return AMEDIA_ERROR_UNKNOWN;
            }
        }
        return AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE;
    }

    const int bufferWidth = getBufferWidth(buffer);
    const int bufferHeight = getBufferHeight(buffer);
    const int bufferFmt = buffer->mGraphicBuffer->getPixelFormat();
    const int bufferUsage = buffer->mGraphicBuffer->getUsage();

    const int readerWidth = mWidth;
    const int readerHeight = mHeight;
    const int readerFmt = mHalFormat;
    const int readerUsage = mHalUsage;

    // Check if the producer buffer configurations match what AImageReader configured. Add some
    // extra checks for non-opaque formats.
    if (!isFormatOpaque(readerFmt)) {
        // Check if the left-top corner of the crop rect is origin, we currently assume this point
        // is zero, will revisit this once this assumption turns out problematic.
        Point lt = buffer->mCrop.leftTop();
        if (lt.x != 0 || lt.y != 0) {
            ALOGE("Crop left top corner [%d, %d] not at origin", lt.x, lt.y);
            return AMEDIA_ERROR_UNKNOWN;
        }

        // Check if the producer buffer configurations match what ImageReader configured.
        ALOGV_IF(readerWidth != bufferWidth || readerHeight != bufferHeight,
                "%s: Buffer size: %dx%d, doesn't match AImageReader configured size: %dx%d",
                __FUNCTION__, bufferWidth, bufferHeight, readerWidth, readerHeight);

        // Check if the buffer usage is a super set of reader's usage bits, aka all usage bits that
        // ImageReader requested has been supported from the producer side.
        ALOGD_IF((readerUsage | bufferUsage) != bufferUsage,
                "%s: Producer buffer usage: %x, doesn't cover all usage bits AImageReader "
                "configured: %x",
                __FUNCTION__, bufferUsage, readerUsage);

        if (readerFmt != bufferFmt) {
            if (readerFmt == HAL_PIXEL_FORMAT_YCbCr_420_888 && isPossiblyYUV(bufferFmt)) {
                // Special casing for when producer switches to a format compatible with flexible
                // YUV.
                mHalFormat = bufferFmt;
                ALOGD("%s: Overriding buffer format YUV_420_888 to 0x%x.", __FUNCTION__, bufferFmt);
            } else {
                // Return the buffer to the queue. No need to provide fence, as this buffer wasn't
                // used anywhere yet.
                mBufferItemConsumer->releaseBuffer(*buffer);
                returnBufferItemLocked(buffer);

                ALOGE("%s: Output buffer format: 0x%x, ImageReader configured format: 0x%x",
                        __FUNCTION__, bufferFmt, readerFmt);

                return AMEDIA_ERROR_UNKNOWN;
            }
        }
    }

    if (mHalFormat == HAL_PIXEL_FORMAT_BLOB) {
        *image = new AImage(this, mFormat, mUsage, buffer, buffer->mTimestamp,
                readerWidth, readerHeight, mNumPlanes);
    } else {
        *image = new AImage(this, mFormat, mUsage, buffer, buffer->mTimestamp,
                bufferWidth, bufferHeight, mNumPlanes);
    }
    mAcquiredImages.push_back(*image);

    // When the output paramter fence is not NULL, we are acquiring the image asynchronously.
    if (acquireFenceFd != nullptr) {
        *acquireFenceFd = buffer->mFence->dup();
    }

    return AMEDIA_OK;
}

BufferItem*
AImageReader::getBufferItemLocked() {
    if (mBuffers.empty()) {
        return nullptr;
    }
    // Return a BufferItem pointer and remove it from the list
    auto it = mBuffers.begin();
    BufferItem* buffer = *it;
    mBuffers.erase(it);
    return buffer;
}

void
AImageReader::returnBufferItemLocked(BufferItem* buffer) {
    mBuffers.push_back(buffer);
}

void
AImageReader::releaseImageLocked(AImage* image, int releaseFenceFd, bool clearCache) {
    BufferItem* buffer = image->mBuffer;
    if (buffer == nullptr) {
        // This should not happen, but is not fatal
        ALOGW("AImage %p has no buffer!", image);
        return;
    }

    int unlockFenceFd = -1;
    media_status_t ret = image->unlockImageIfLocked(&unlockFenceFd);
    if (ret < 0) {
        ALOGW("%s: AImage %p is cannot be unlocked.", __FUNCTION__, image);
        return;
    }

    sp<Fence> unlockFence = unlockFenceFd > 0 ? new Fence(unlockFenceFd) : Fence::NO_FENCE;
    sp<Fence> releaseFence = releaseFenceFd > 0 ? new Fence(releaseFenceFd) : Fence::NO_FENCE;
    sp<Fence> bufferFence = Fence::merge("AImageReader", unlockFence, releaseFence);
    mBufferItemConsumer->releaseBuffer(*buffer, bufferFence);
    returnBufferItemLocked(buffer);
    image->mBuffer = nullptr;
    image->mLockedBuffer = nullptr;
    image->mIsClosed = true;

    if (!clearCache) {
        return;
    }

    bool found = false;
    // cleanup acquired image list
    for (auto it = mAcquiredImages.begin();
              it != mAcquiredImages.end(); it++) {
        AImage* readerCopy = *it;
        if (readerCopy == image) {
            found = true;
            mAcquiredImages.erase(it);
            break;
        }
    }
    if (!found) {
        ALOGE("Error: AImage %p is not generated by AImageReader %p",
                image, this);
    }
}

media_status_t AImageReader::getWindowNativeHandle(native_handle **handle) {
    if (mWindowHandle != nullptr) {
        *handle = mWindowHandle;
        return AMEDIA_OK;
    }
    sp<HGraphicBufferProducer> hgbp =
        new TWGraphicBufferProducer<HGraphicBufferProducer>(mProducer);
    HalToken halToken;
    if (!createHalToken(hgbp, &halToken)) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    mWindowHandle = convertHalTokenToNativeHandle(halToken);
    if (!mWindowHandle) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    *handle = mWindowHandle;
    return AMEDIA_OK;
}

int
AImageReader::getBufferWidth(BufferItem* buffer) {
    if (buffer == NULL) return -1;

    if (!buffer->mCrop.isEmpty()) {
        return buffer->mCrop.getWidth();
    }

    return buffer->mGraphicBuffer->getWidth();
}

int
AImageReader::getBufferHeight(BufferItem* buffer) {
    if (buffer == NULL) return -1;

    if (!buffer->mCrop.isEmpty()) {
        return buffer->mCrop.getHeight();
    }

    return buffer->mGraphicBuffer->getHeight();
}

media_status_t
AImageReader::acquireNextImage(/*out*/AImage** image, /*out*/int* acquireFenceFd) {
    Mutex::Autolock _l(mLock);
    return acquireImageLocked(image, acquireFenceFd);
}

media_status_t
AImageReader::acquireLatestImage(/*out*/AImage** image, /*out*/int* acquireFenceFd) {
    if (image == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    Mutex::Autolock _l(mLock);
    *image = nullptr;
    AImage* prevImage = nullptr;
    AImage* nextImage = nullptr;
    media_status_t ret = acquireImageLocked(&prevImage, acquireFenceFd);
    if (prevImage == nullptr) {
        return ret;
    }
    for (;;) {
        ret = acquireImageLocked(&nextImage, acquireFenceFd);
        if (nextImage == nullptr) {
            *image = prevImage;
            return AMEDIA_OK;
        }

        if (acquireFenceFd == nullptr) {
            // No need for release fence here since the prevImage is unused and acquireImageLocked
            // has already waited for acquired fence to be signaled.
            prevImage->close();
        } else {
            // Use the acquire fence as release fence, so that producer can wait before trying to
            // refill the buffer.
            prevImage->close(*acquireFenceFd);
        }
        prevImage->free();
        prevImage = nextImage;
        nextImage = nullptr;
    }
}

static native_handle_t *convertHalTokenToNativeHandle(
        const HalToken &halToken) {
    // We attempt to store halToken in the ints of the native_handle_t after its
    // size. The first int stores the size of the token. We store this in an int
    // to avoid alignment issues where size_t and int do not have the same
    // alignment.
    size_t nhDataByteSize = halToken.size();
    if (nhDataByteSize > kWindowHalTokenSizeMax) {
        // The size of the token isn't reasonable..
        return nullptr;
    }
    size_t numInts = ceil(nhDataByteSize / sizeof(int)) + 1;

    // We don't check for overflow, whether numInts can fit in an int, since we
    // expect kWindowHalTokenSizeMax to be a reasonable limit.
    // create a native_handle_t with 0 numFds and numInts number of ints.
    native_handle_t *nh =
        native_handle_create(0, numInts);
    if (!nh) {
        return nullptr;
    }
    // Store the size of the token in the first int.
    nh->data[0] = nhDataByteSize;
    memcpy(&(nh->data[1]), halToken.data(), nhDataByteSize);
    return nh;
}

EXPORT
media_status_t AImageReader_new(
        int32_t width, int32_t height, int32_t format, int32_t maxImages,
        /*out*/AImageReader** reader) {
    ALOGV("%s", __FUNCTION__);
    return AImageReader_newWithUsage(
            width, height, format, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, maxImages, reader);
}

extern "C" {

EXPORT
media_status_t AImageReader_getWindowNativeHandle(
        AImageReader *reader, /*out*/native_handle_t **handle) {
    if (reader == nullptr || handle == nullptr) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return reader->getWindowNativeHandle(handle);
}

} //extern "C"

EXPORT
media_status_t AImageReader_newWithUsage(
        int32_t width, int32_t height, int32_t format, uint64_t usage,
        int32_t maxImages, /*out*/ AImageReader** reader) {
    ALOGV("%s", __FUNCTION__);

    if (width < 1 || height < 1) {
        ALOGE("%s: image dimension must be positive: w:%d h:%d",
                __FUNCTION__, width, height);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (maxImages < 1) {
        ALOGE("%s: max outstanding image count must be at least 1 (%d)",
                __FUNCTION__, maxImages);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (maxImages > BufferQueueDefs::NUM_BUFFER_SLOTS) {
        ALOGE("%s: max outstanding image count (%d) cannot be larget than %d.",
              __FUNCTION__, maxImages, BufferQueueDefs::NUM_BUFFER_SLOTS);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (!AImageReader::isSupportedFormatAndUsage(format, usage)) {
        ALOGE("%s: format %d is not supported with usage 0x%" PRIx64 " by AImageReader",
                __FUNCTION__, format, usage);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    if (reader == nullptr) {
        ALOGE("%s: reader argument is null", __FUNCTION__);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    AImageReader* tmpReader = new AImageReader(
        width, height, format, usage, maxImages);
    if (tmpReader == nullptr) {
        ALOGE("%s: AImageReader allocation failed", __FUNCTION__);
        return AMEDIA_ERROR_UNKNOWN;
    }
    media_status_t ret = tmpReader->init();
    if (ret != AMEDIA_OK) {
        ALOGE("%s: AImageReader initialization failed!", __FUNCTION__);
        delete tmpReader;
        return ret;
    }
    *reader = tmpReader;
    (*reader)->incStrong((void*) AImageReader_new);
    return AMEDIA_OK;
}

EXPORT
void AImageReader_delete(AImageReader* reader) {
    ALOGV("%s", __FUNCTION__);
    if (reader != nullptr) {
        reader->close();
        reader->decStrong((void*) AImageReader_delete);
    }
    return;
}

EXPORT
media_status_t AImageReader_getWindow(AImageReader* reader, /*out*/ANativeWindow** window) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || window == nullptr) {
        ALOGE("%s: invalid argument. reader %p, window %p",
                __FUNCTION__, reader, window);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *window = reader->getWindow();
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_getWidth(const AImageReader* reader, /*out*/int32_t* width) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || width == nullptr) {
        ALOGE("%s: invalid argument. reader %p, width %p",
                __FUNCTION__, reader, width);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *width = reader->getWidth();
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_getHeight(const AImageReader* reader, /*out*/int32_t* height) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || height == nullptr) {
        ALOGE("%s: invalid argument. reader %p, height %p",
                __FUNCTION__, reader, height);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *height = reader->getHeight();
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_getFormat(const AImageReader* reader, /*out*/int32_t* format) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || format == nullptr) {
        ALOGE("%s: invalid argument. reader %p, format %p",
                __FUNCTION__, reader, format);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *format = reader->getFormat();
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_getMaxImages(const AImageReader* reader, /*out*/int32_t* maxImages) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || maxImages == nullptr) {
        ALOGE("%s: invalid argument. reader %p, maxImages %p",
                __FUNCTION__, reader, maxImages);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *maxImages = reader->getMaxImages();
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_acquireNextImage(AImageReader* reader, /*out*/AImage** image) {
    ALOGV("%s", __FUNCTION__);
    return AImageReader_acquireNextImageAsync(reader, image, nullptr);
}

EXPORT
media_status_t AImageReader_acquireLatestImage(AImageReader* reader, /*out*/AImage** image) {
    ALOGV("%s", __FUNCTION__);
    return AImageReader_acquireLatestImageAsync(reader, image, nullptr);
}

EXPORT
media_status_t AImageReader_acquireNextImageAsync(
    AImageReader* reader, /*out*/AImage** image, /*out*/int* acquireFenceFd) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || image == nullptr) {
        ALOGE("%s: invalid argument. reader %p, image %p",
                __FUNCTION__, reader, image);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return reader->acquireNextImage(image, acquireFenceFd);
}

EXPORT
media_status_t AImageReader_acquireLatestImageAsync(
    AImageReader* reader, /*out*/AImage** image, /*out*/int* acquireFenceFd) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr || image == nullptr) {
        ALOGE("%s: invalid argument. reader %p, image %p",
                __FUNCTION__, reader, image);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return reader->acquireLatestImage(image, acquireFenceFd);
}

EXPORT
media_status_t AImageReader_setImageListener(
        AImageReader* reader, AImageReader_ImageListener* listener) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr) {
        ALOGE("%s: invalid argument! reader %p", __FUNCTION__, reader);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    reader->setImageListener(listener);
    return AMEDIA_OK;
}

EXPORT
media_status_t AImageReader_setBufferRemovedListener(
    AImageReader* reader, AImageReader_BufferRemovedListener* listener) {
    ALOGV("%s", __FUNCTION__);
    if (reader == nullptr) {
        ALOGE("%s: invalid argument! reader %p", __FUNCTION__, reader);
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    reader->setBufferRemovedListener(listener);
    return AMEDIA_OK;
}
