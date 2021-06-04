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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA3_DEPTH_COMPOSITE_STREAM_H
#define ANDROID_SERVERS_CAMERA_CAMERA3_DEPTH_COMPOSITE_STREAM_H

#include "common/DepthPhotoProcessor.h"
#include <dynamic_depth/imaging_model.h>
#include <dynamic_depth/depth_map.h>

#include <gui/CpuConsumer.h>

#include "CompositeStream.h"

using dynamic_depth::DepthMap;
using dynamic_depth::Item;
using dynamic_depth::ImagingModel;

namespace android {

class CameraDeviceClient;
class CameraMetadata;
class Surface;

namespace camera3 {

class DepthCompositeStream : public CompositeStream, public Thread,
        public CpuConsumer::FrameAvailableListener {

public:
    DepthCompositeStream(wp<CameraDeviceBase> device,
            wp<hardware::camera2::ICameraDeviceCallbacks> cb);
    ~DepthCompositeStream() override;

    static bool isDepthCompositeStream(const sp<Surface> &surface);

    // CompositeStream overrides
    status_t createInternalStreams(const std::vector<sp<Surface>>& consumers,
            bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
            camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
            std::vector<int> *surfaceIds, int streamSetId, bool isShared) override;
    status_t deleteInternalStreams() override;
    status_t configureStream() override;
    status_t insertGbp(SurfaceMap* /*out*/outSurfaceMap, Vector<int32_t>* /*out*/outputStreamIds,
            int32_t* /*out*/currentStreamId) override;
    int getStreamId() override { return mBlobStreamId; }

    // CpuConsumer listener implementation
    void onFrameAvailable(const BufferItem& item) override;

    // Return stream information about the internal camera streams
    static status_t getCompositeStreamInfo(const OutputStreamInfo &streamInfo,
            const CameraMetadata& ch, std::vector<OutputStreamInfo>* compositeOutput /*out*/);

protected:

    bool threadLoop() override;
    bool onStreamBufferError(const CaptureResultExtras& resultExtras) override;
    void onResultError(const CaptureResultExtras& resultExtras) override;

private:
    struct InputFrame {
        CpuConsumer::LockedBuffer depthBuffer;
        CpuConsumer::LockedBuffer jpegBuffer;
        CameraMetadata            result;
        bool                      error;
        bool                      errorNotified;
        int64_t                   frameNumber;

        InputFrame() : error(false), errorNotified(false), frameNumber(-1) { }
    };

    // Helper methods
    static void getSupportedDepthSizes(const CameraMetadata& ch,
            std::vector<std::tuple<size_t, size_t>>* depthSizes /*out*/);
    static status_t getMatchingDepthSize(size_t width, size_t height,
            const std::vector<std::tuple<size_t, size_t>>& supporedDepthSizes,
            size_t *depthWidth /*out*/, size_t *depthHeight /*out*/);

    // Dynamic depth processing
    status_t encodeGrayscaleJpeg(size_t width, size_t height, uint8_t *in, void *out,
            const size_t maxOutSize, uint8_t jpegQuality, size_t &actualSize);
    std::unique_ptr<DepthMap> processDepthMapFrame(const CpuConsumer::LockedBuffer &depthMapBuffer,
            size_t maxJpegSize, uint8_t jpegQuality,
            std::vector<std::unique_ptr<Item>>* items /*out*/);
    std::unique_ptr<ImagingModel> getImagingModel();
    status_t processInputFrame(nsecs_t ts, const InputFrame &inputFrame);

    // Buffer/Results handling
    void compilePendingInputLocked();
    void releaseInputFrameLocked(InputFrame *inputFrame /*out*/);
    void releaseInputFramesLocked(int64_t currentTs);

    // Find first complete and valid frame with smallest timestamp
    bool getNextReadyInputLocked(int64_t *currentTs /*inout*/);

    // Find next failing frame number with smallest timestamp and return respective frame number
    int64_t getNextFailingInputLocked(int64_t *currentTs /*inout*/);

    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    static const auto kDepthMapPixelFormat = HAL_PIXEL_FORMAT_Y16;
    static const auto kDepthMapDataSpace = HAL_DATASPACE_DEPTH;
    static const auto kJpegDataSpace = HAL_DATASPACE_V0_JFIF;

    int                  mBlobStreamId, mBlobSurfaceId, mDepthStreamId, mDepthSurfaceId;
    size_t               mBlobWidth, mBlobHeight;
    sp<CpuConsumer>      mBlobConsumer, mDepthConsumer;
    bool                 mDepthBufferAcquired, mBlobBufferAcquired;
    sp<Surface>          mDepthSurface, mBlobSurface, mOutputSurface;
    sp<ProducerListener> mProducerListener;

    ssize_t              mMaxJpegSize;
    std::vector<std::tuple<size_t, size_t>> mSupportedDepthSizes;
    std::vector<float>   mIntrinsicCalibration, mLensDistortion;
    bool                 mIsLogicalCamera;
    void*                mDepthPhotoLibHandle;
    process_depth_photo_frame mDepthPhotoProcess;

    // Keep all incoming Depth buffer timestamps pending further processing.
    std::vector<int64_t> mInputDepthBuffers;

    // Keep all incoming Jpeg/Blob buffer timestamps pending further processing.
    std::vector<int64_t> mInputJpegBuffers;

    // Map of all input frames pending further processing.
    std::unordered_map<int64_t, InputFrame> mPendingInputFrames;
};

}; //namespace camera3
}; //namespace android

#endif
