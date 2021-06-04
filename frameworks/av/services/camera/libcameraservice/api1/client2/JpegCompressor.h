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


/**
 * This class simulates a hardware JPEG compressor.  It receives image buffers
 * in RGBA_8888 format, processes them in a worker thread, and then pushes them
 * out to their destination stream.
 */

#ifndef ANDROID_SERVERS_CAMERA_JPEGCOMPRESSOR_H
#define ANDROID_SERVERS_CAMERA_JPEGCOMPRESSOR_H

#include "utils/Thread.h"
#include "utils/Mutex.h"
#include "utils/Timers.h"
#include "utils/Vector.h"
//#include "Base.h"
#include <stdio.h>
#include <gui/CpuConsumer.h>

extern "C" {
#include <jpeglib.h>
}


namespace android {
namespace camera2 {

class JpegCompressor: private Thread, public virtual RefBase {
  public:

    JpegCompressor();
    ~JpegCompressor();

    // Start compressing COMPRESSED format buffers; JpegCompressor takes
    // ownership of the Buffers vector.
    status_t start(const Vector<CpuConsumer::LockedBuffer*>& buffers,
            nsecs_t captureTime);

    status_t cancel();

    bool isBusy();
    bool isStreamInUse(uint32_t id);

    bool waitForDone(nsecs_t timeout);

    // TODO: Measure this
    static const size_t kMaxJpegSize = 300000;

  private:
    Mutex mBusyMutex;
    Mutex mMutex;
    bool mIsBusy;
    Condition mDone;
    nsecs_t mCaptureTime;

    Vector<CpuConsumer::LockedBuffer*> mBuffers;
    CpuConsumer::LockedBuffer *mJpegBuffer;
    CpuConsumer::LockedBuffer *mAuxBuffer;

    jpeg_compress_struct mCInfo;

    struct JpegError : public jpeg_error_mgr {
        JpegCompressor *parent;
    };
    j_common_ptr mJpegErrorInfo;

    struct JpegDestination : public jpeg_destination_mgr {
        JpegCompressor *parent;
    };

    static void jpegErrorHandler(j_common_ptr cinfo);

    static void jpegInitDestination(j_compress_ptr cinfo);
    static boolean jpegEmptyOutputBuffer(j_compress_ptr cinfo);
    static void jpegTermDestination(j_compress_ptr cinfo);

    bool checkError(const char *msg);
    void cleanUp();

    /**
     * Inherited Thread virtual overrides
     */
  private:
    virtual status_t readyToRun();
    virtual bool threadLoop();
};

}; // namespace camera2
}; // namespace android

#endif
