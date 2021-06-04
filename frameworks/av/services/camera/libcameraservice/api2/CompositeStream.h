/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA3_COMPOSITE_STREAM_H
#define ANDROID_SERVERS_CAMERA_CAMERA3_COMPOSITE_STREAM_H

#include <set>
#include <unordered_map>

#include <android/hardware/camera2/ICameraDeviceCallbacks.h>
#include <camera/CameraMetadata.h>
#include <camera/camera2/OutputConfiguration.h>
#include <gui/IProducerListener.h>
#include "common/CameraDeviceBase.h"
#include "device3/Camera3StreamInterface.h"

namespace android {

class CameraDeviceClient;
class CameraMetadata;
class Surface;

namespace camera3 {

class CompositeStream : public camera3::Camera3StreamBufferListener {

public:
    CompositeStream(wp<CameraDeviceBase> device, wp<hardware::camera2::ICameraDeviceCallbacks> cb);
    virtual ~CompositeStream() {}

    status_t createStream(const std::vector<sp<Surface>>& consumers,
            bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
            camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
            std::vector<int> *surfaceIds, int streamSetId, bool isShared);

    status_t deleteStream();

    // Create and register all internal camera streams.
    virtual status_t createInternalStreams(const std::vector<sp<Surface>>& consumers,
            bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
            camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
            std::vector<int> *surfaceIds, int streamSetId, bool isShared) = 0;

    // Release all internal streams and corresponding resources.
    virtual status_t deleteInternalStreams() = 0;

    // Stream configuration completed.
    virtual status_t configureStream() = 0;

    // Insert the internal composite stream id in the user capture request.
    virtual status_t insertGbp(SurfaceMap* /*out*/outSurfaceMap,
            Vector<int32_t>* /*out*/outputStreamIds, int32_t* /*out*/currentStreamId) = 0;

    // Return composite stream id.
    virtual int getStreamId() = 0;

    // Notify when shutter notify is triggered
    virtual void onShutter(const CaptureResultExtras& /*resultExtras*/, nsecs_t /*timestamp*/) {}

    void onResultAvailable(const CaptureResult& result);
    bool onError(int32_t errorCode, const CaptureResultExtras& resultExtras);

    // Camera3StreamBufferListener implementation
    void onBufferAcquired(const BufferInfo& /*bufferInfo*/) override { /*Empty for now */ }
    void onBufferReleased(const BufferInfo& bufferInfo) override;
    void onBufferRequestForFrameNumber(uint64_t frameNumber, int streamId,
            const CameraMetadata& settings) override;

protected:
    struct ProducerListener : public BnProducerListener {
        // ProducerListener impementation
        void onBufferReleased() override { /*No impl. for now*/ };
    };

    status_t registerCompositeStreamListener(int32_t streamId);
    void eraseResult(int64_t frameNumber);
    void flagAnErrorFrameNumber(int64_t frameNumber);
    void notifyError(int64_t frameNumber);

    // Subclasses should check for buffer errors from internal streams and return 'true' in
    // case the error notification should remain within camera service.
    virtual bool onStreamBufferError(const CaptureResultExtras& resultExtras) = 0;

    // Subclasses can decide how to handle result errors depending on whether or not the
    // internal processing needs result data.
    virtual void onResultError(const CaptureResultExtras& resultExtras) = 0;

    // Device and/or service is in unrecoverable error state.
    // Composite streams should behave accordingly.
    void enableErrorState();

    wp<CameraDeviceBase>   mDevice;
    wp<hardware::camera2::ICameraDeviceCallbacks> mRemoteCallback;

    mutable Mutex          mMutex;
    Condition              mInputReadyCondition;
    int32_t                mNumPartialResults;
    bool                   mErrorState;

    // Frame number to capture result map of partial pending request results.
    std::unordered_map<uint64_t, CameraMetadata> mPendingCaptureResults;

    // Timestamp to capture (frame number, result) map of completed pending request results.
    std::unordered_map<int64_t, std::tuple<int64_t, CameraMetadata>> mCaptureResults;

    // Frame number to timestamp map
    std::unordered_map<int64_t, int64_t> mFrameNumberMap;

    // Keeps a set buffer/result frame numbers for any errors detected during processing.
    std::set<int64_t> mErrorFrameNumbers;

};

}; //namespace camera3
}; //namespace android

#endif
