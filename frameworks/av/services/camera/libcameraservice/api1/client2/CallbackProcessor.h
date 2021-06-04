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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_CALLBACKPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_CALLBACKPROCESSOR_H

#include <utils/Thread.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <gui/CpuConsumer.h>

#include "api1/client2/Camera2Heap.h"

namespace android {

class Camera2Client;
class CameraDeviceBase;

namespace camera2 {

struct Parameters;

/***
 * Still image capture output image processing
 */
class CallbackProcessor:
            public Thread, public CpuConsumer::FrameAvailableListener {
  public:
    explicit CallbackProcessor(sp<Camera2Client> client);
    ~CallbackProcessor();

    void onFrameAvailable(const BufferItem& item);

    // Set to NULL to disable the direct-to-app callback window
    status_t setCallbackWindow(const sp<Surface>& callbackWindow);
    status_t updateStream(const Parameters &params);
    status_t deleteStream();
    int getStreamId() const;

    void dump(int fd, const Vector<String16>& args) const;
  private:
    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    wp<Camera2Client> mClient;
    wp<CameraDeviceBase> mDevice;
    int mId;

    mutable Mutex mInputMutex;
    bool mCallbackAvailable;
    Condition mCallbackAvailableSignal;

    enum {
        NO_STREAM = -1
    };

    // True if mCallbackWindow is a remote consumer, false if just the local
    // mCallbackConsumer
    bool mCallbackToApp;
    int mCallbackStreamId;
    static const size_t kCallbackHeapCount = 6;
    sp<CpuConsumer>    mCallbackConsumer;
    sp<Surface>        mCallbackWindow;
    sp<Camera2Heap>    mCallbackHeap;
    size_t mCallbackHeapHead, mCallbackHeapFree;

    virtual bool threadLoop();

    status_t processNewCallback(sp<Camera2Client> &client);
    // Used when shutting down
    status_t discardNewCallback();

    // Convert from flexible YUV to NV21 or YV12
    status_t convertFromFlexibleYuv(int32_t previewFormat,
            uint8_t *dst,
            const CpuConsumer::LockedBuffer &src,
            uint32_t dstYStride,
            uint32_t dstCStride) const;
};


}; //namespace camera2
}; //namespace android

#endif
