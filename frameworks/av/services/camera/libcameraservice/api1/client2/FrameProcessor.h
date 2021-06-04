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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_FRAMEPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_FRAMEPROCESSOR_H

#include <utils/Thread.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <camera/CameraMetadata.h>

#include "common/FrameProcessorBase.h"

struct camera_frame_metadata;

namespace android {

class Camera2Client;

namespace camera2 {

/* Output frame metadata processing thread.  This thread waits for new
 * frames from the device, and analyzes them as necessary.
 */
class FrameProcessor : public FrameProcessorBase {
  public:
    FrameProcessor(wp<CameraDeviceBase> device, sp<Camera2Client> client);
    ~FrameProcessor();

  private:
    static const int32_t NOT_SET = -1;

    wp<Camera2Client> mClient;

    bool mSynthesize3ANotify;

    int mLastFrameNumberOfFaces;

    void processNewFrames(const sp<Camera2Client> &client);

    virtual bool processSingleFrame(CaptureResult &frame,
                                    const sp<CameraDeviceBase> &device);

    status_t processFaceDetect(const CameraMetadata &frame,
            const sp<Camera2Client> &client);

    // Send 3A state change notifications to client based on frame metadata
    status_t process3aState(const CaptureResult &frame,
            const sp<Camera2Client> &client);

    // Helper for process3aState
    template<typename Src, typename T>
    bool updatePendingState(const CameraMetadata& result, int32_t tag, T* value,
            int32_t frameNumber, int cameraId);


    struct AlgState {
        // TODO: also track AE mode
        camera_metadata_enum_android_control_af_mode   afMode;
        camera_metadata_enum_android_control_awb_mode  awbMode;

        camera_metadata_enum_android_control_ae_state  aeState;
        camera_metadata_enum_android_control_af_state  afState;
        camera_metadata_enum_android_control_awb_state awbState;

        int32_t                                        afTriggerId;
        int32_t                                        aeTriggerId;

        // These defaults need to match those in Parameters.cpp
        AlgState() :
                afMode((camera_metadata_enum_android_control_af_mode)NOT_SET),
                awbMode((camera_metadata_enum_android_control_awb_mode)NOT_SET),
                aeState((camera_metadata_enum_android_control_ae_state)NOT_SET),
                afState((camera_metadata_enum_android_control_af_state)NOT_SET),
                awbState((camera_metadata_enum_android_control_awb_state)NOT_SET),
                afTriggerId(NOT_SET),
                aeTriggerId(NOT_SET) {
        }
    };

    AlgState m3aState;

    // frame number -> pending 3A states that not all data are received yet.
    KeyedVector<int32_t, AlgState> mPending3AStates;

    // Whether the partial result is enabled for this device
    bool mUsePartialResult;

    // Track most recent frame number for which 3A notifications were sent for.
    // Used to filter against sending 3A notifications for the same frame
    // several times.
    int32_t mLast3AFrameNumber, mLastAEFrameNumber, mLastAFrameNumber, mLastAWBFrameNumber;
    // Emit FaceDetection event to java if faces changed
    void callbackFaceDetection(const sp<Camera2Client>& client,
                               const camera_frame_metadata &metadata);
};


}; //namespace camera2
}; //namespace android

#endif
