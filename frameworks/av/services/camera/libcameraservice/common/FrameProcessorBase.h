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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_PROFRAMEPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_PROFRAMEPROCESSOR_H

#include <utils/Thread.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <camera/CameraMetadata.h>
#include <camera/CaptureResult.h>

namespace android {

class CameraDeviceBase;

namespace camera2 {

/* Output frame metadata processing thread.  This thread waits for new
 * frames from the device, and analyzes them as necessary.
 */
class FrameProcessorBase: public Thread {
  public:
    explicit FrameProcessorBase(wp<CameraDeviceBase> device);
    virtual ~FrameProcessorBase();

    struct FilteredListener: virtual public RefBase {
        virtual void onResultAvailable(const CaptureResult &result) = 0;
    };

    // Register a listener for a range of IDs [minId, maxId). Multiple listeners
    // can be listening to the same range. Registering the same listener with
    // the same range of IDs has no effect.
    // sendPartials controls whether partial results will be sent.
    status_t registerListener(int32_t minId, int32_t maxId,
                              const wp<FilteredListener>& listener,
                              bool sendPartials = true);
    status_t removeListener(int32_t minId, int32_t maxId,
                            const wp<FilteredListener>& listener);

    void dump(int fd, const Vector<String16>& args);
  protected:
    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    wp<CameraDeviceBase> mDevice;

    virtual bool threadLoop();

    Mutex mInputMutex;
    Mutex mLastFrameMutex;

    struct RangeListener {
        int32_t minId;
        int32_t maxId;
        wp<FilteredListener> listener;
        bool sendPartials;
    };
    List<RangeListener> mRangeListeners;

    // Number of partial result the HAL will potentially send.
    int32_t mNumPartialResults;

    void processNewFrames(const sp<CameraDeviceBase> &device);

    virtual bool processSingleFrame(CaptureResult &result,
                                    const sp<CameraDeviceBase> &device);

    status_t processListeners(const CaptureResult &result,
                              const sp<CameraDeviceBase> &device);

    CameraMetadata mLastFrame;
    std::vector<PhysicalCaptureResultInfo> mLastPhysicalFrames;

};


}; //namespace camera2
}; //namespace android

#endif
