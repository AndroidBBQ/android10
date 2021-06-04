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

#ifndef ANDROID_RS_GRALLOC_CONSUMER_H
#define ANDROID_RS_GRALLOC_CONSUMER_H

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {

class Allocation;
class Context;

/**
 * CpuConsumer is a BufferQueue consumer endpoint that allows direct CPU
 * access to the underlying gralloc buffers provided by BufferQueue. Multiple
 * buffers may be acquired by it at once, to be used concurrently by the
 * CpuConsumer owner. Sets gralloc usage flags to be software-read-only.
 * This queue is synchronous by default.
 */
class GrallocConsumer
{
  public:
    GrallocConsumer(const Context *, Allocation *, uint32_t numAlloc);

    virtual ~GrallocConsumer();
    ANativeWindow* getNativeWindow();
    media_status_t lockNextBuffer(uint32_t idx = 0);
    media_status_t unlockBuffer(uint32_t idx = 0);
    uint32_t getNextAvailableIdx(Allocation *a);
    bool releaseIdx(uint32_t idx);
    bool isActive();
    uint32_t mNumAlloc;

    static void onFrameAvailable(void* obj, AImageReader* reader);

  private:
    media_status_t releaseAcquiredBufferLocked(uint32_t idx);
    // Boolean array to check if a position has been occupied or not.
    bool *isIdxUsed;
    Allocation **mAlloc;

    const Context *mCtx;
    AImageReader* mImgReader;
    ANativeWindow* mNativeWindow;
    AImageReader_ImageListener mReaderCb;
    // Tracking for buffers acquired by the user
    struct AcquiredBuffer {
        AImage *mImg;
        uint8_t *mBufferPointer;

        AcquiredBuffer() :
                mImg(nullptr),
                mBufferPointer(nullptr) {
        }
    };
    AcquiredBuffer *mAcquiredBuffer;
};

} // namespace renderscript
} // namespace android

#endif // ANDROID_RS_GRALLOC_CONSUMER_H
