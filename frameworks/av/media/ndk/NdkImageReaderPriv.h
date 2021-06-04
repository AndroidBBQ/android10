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

#ifndef _NDK_IMAGE_READER_PRIV_H
#define _NDK_IMAGE_READER_PRIV_H

#include <inttypes.h>

#include <media/NdkImageReader.h>

#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>

#include <gui/BufferItem.h>
#include <gui/BufferItemConsumer.h>
#include <gui/Surface.h>

#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

using namespace android;

namespace {
    enum {
        IMAGE_READER_MAX_NUM_PLANES = 3,
    };

    enum {
        ACQUIRE_SUCCESS = 0,
        ACQUIRE_NO_BUFFERS = 1,
        ACQUIRE_MAX_IMAGES = 2,
    };
}

struct AImageReader : public RefBase {
  public:
    static bool isSupportedFormatAndUsage(int32_t format, uint64_t usage0);
    static int getNumPlanesForFormat(int32_t format);

    AImageReader(int32_t width,
                 int32_t height,
                 int32_t format,
                 uint64_t usage,
                 int32_t maxImages);
    ~AImageReader();

    // Inintialize AImageReader, uninitialized or failed to initialize AImageReader
    // should never be passed to application
    media_status_t init();

    media_status_t setImageListener(AImageReader_ImageListener* listener);
    media_status_t setBufferRemovedListener(AImageReader_BufferRemovedListener* listener);

    media_status_t acquireNextImage(/*out*/AImage** image, /*out*/int* fenceFd);
    media_status_t acquireLatestImage(/*out*/AImage** image, /*out*/int* fenceFd);

    media_status_t getWindowNativeHandle(/*out*/native_handle_t **handle);

    ANativeWindow* getWindow()    const { return mWindow.get(); };
    int32_t        getWidth()     const { return mWidth; };
    int32_t        getHeight()    const { return mHeight; };
    int32_t        getFormat()    const { return mFormat; };
    int32_t        getMaxImages() const { return mMaxImages; };
    void           close();

  private:

    friend struct AImage; // for grabing reader lock

    BufferItem* getBufferItemLocked();
    void returnBufferItemLocked(BufferItem* buffer);

    // Called by AImageReader_acquireXXX to acquire a Buffer and setup AImage.
    media_status_t acquireImageLocked(/*out*/AImage** image, /*out*/int* fenceFd);

    // Called by AImage/~AImageReader to close image. Caller is responsible to grab AImage::mLock
    void releaseImageLocked(AImage* image, int releaseFenceFd, bool clearCache = true);

    static int getBufferWidth(BufferItem* buffer);
    static int getBufferHeight(BufferItem* buffer);

    media_status_t setImageListenerLocked(AImageReader_ImageListener* listener);
    media_status_t setBufferRemovedListenerLocked(AImageReader_BufferRemovedListener* listener);

    // definition of handler and message
    enum {
        kWhatBufferRemoved,
        kWhatImageAvailable,
    };
    static const char* kCallbackFpKey;
    static const char* kContextKey;
    static const char* kGraphicBufferKey;
    class CallbackHandler : public AHandler {
      public:
        CallbackHandler(AImageReader* reader) : mReader(reader) {}
        void onMessageReceived(const sp<AMessage> &msg) override;
      private:
        AImageReader* mReader;
    };
    sp<CallbackHandler> mHandler;
    sp<ALooper>         mCbLooper; // Looper thread where callbacks actually happen on
    List<BufferItem*>   mBuffers;

    const int32_t mWidth;
    const int32_t mHeight;
    const int32_t mFormat;
    const uint64_t mUsage;  // AHARDWAREBUFFER_USAGE_* flags.
    const int32_t mMaxImages;

    // TODO(jwcai) Seems completely unused in AImageReader class.
    const int32_t mNumPlanes;

    struct FrameListener : public ConsumerBase::FrameAvailableListener {
      public:
        explicit FrameListener(AImageReader* parent) : mReader(parent) {}

        void onFrameAvailable(const BufferItem& item) override;

        media_status_t setImageListener(AImageReader_ImageListener* listener);

      private:
        AImageReader_ImageListener mListener = {nullptr, nullptr};
        const wp<AImageReader>     mReader;
        Mutex                      mLock;
    };
    sp<FrameListener> mFrameListener;

    struct BufferRemovedListener : public BufferItemConsumer::BufferFreedListener {
      public:
        explicit BufferRemovedListener(AImageReader* parent) : mReader(parent) {}

        void onBufferFreed(const wp<GraphicBuffer>& graphicBuffer) override;

        media_status_t setBufferRemovedListener(AImageReader_BufferRemovedListener* listener);

       private:
        AImageReader_BufferRemovedListener mListener = {nullptr, nullptr};
        const wp<AImageReader>     mReader;
        Mutex                      mLock;
    };
    sp<BufferRemovedListener> mBufferRemovedListener;

    int mHalFormat;
    android_dataspace mHalDataSpace;
    uint64_t mHalUsage;

    sp<IGraphicBufferProducer> mProducer;
    sp<Surface>                mSurface;
    sp<BufferItemConsumer>     mBufferItemConsumer;
    sp<ANativeWindow>          mWindow;
    native_handle_t*           mWindowHandle = nullptr;

    List<AImage*>              mAcquiredImages;
    bool                       mIsClosed = false;

    Mutex                      mLock;
};

#endif // _NDK_IMAGE_READER_PRIV_H
