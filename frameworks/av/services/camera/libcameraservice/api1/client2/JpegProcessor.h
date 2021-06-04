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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_JPEGPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_JPEGPROCESSOR_H

#include <utils/Thread.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <gui/CpuConsumer.h>

#include "camera/CameraMetadata.h"
#include "device3/Camera3StreamBufferListener.h"

namespace android {

class Camera2Client;
class CameraDeviceBase;
class MemoryHeapBase;

namespace camera2 {

class CaptureSequencer;
struct Parameters;

/***
 * Still image capture output image processing
 */
class JpegProcessor:
            public Thread, public CpuConsumer::FrameAvailableListener {
  public:
    JpegProcessor(sp<Camera2Client> client, wp<CaptureSequencer> sequencer);
    ~JpegProcessor();

    // CpuConsumer listener implementation
    void onFrameAvailable(const BufferItem& item);

    status_t updateStream(const Parameters &params);
    status_t deleteStream();
    int getStreamId() const;

    void dump(int fd, const Vector<String16>& args) const;

    static size_t findJpegSize(uint8_t* jpegBuffer, size_t maxSize);

  private:
    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    wp<CameraDeviceBase> mDevice;
    wp<CaptureSequencer> mSequencer;
    int mId;

    mutable Mutex mInputMutex;
    bool mCaptureDone;
    bool mCaptureSuccess;
    Condition mCaptureDoneSignal;

    enum {
        NO_STREAM = -1
    };

    int mCaptureStreamId;
    sp<CpuConsumer>    mCaptureConsumer;
    sp<Surface>        mCaptureWindow;
    sp<MemoryHeapBase> mCaptureHeap;

    virtual bool threadLoop();

    status_t processNewCapture(bool captureSuccess);

};


}; //namespace camera2
}; //namespace android

#endif
