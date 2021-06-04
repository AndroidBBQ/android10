/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA3_HEIC_COMPOSITE_STREAM_H
#define ANDROID_SERVERS_CAMERA_CAMERA3_HEIC_COMPOSITE_STREAM_H

#include <queue>

#include <gui/IProducerListener.h>
#include <gui/CpuConsumer.h>

#include <media/hardware/VideoAPI.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaMuxer.h>

#include "CompositeStream.h"

namespace android {
namespace camera3 {

class HeicCompositeStream : public CompositeStream, public Thread,
        public CpuConsumer::FrameAvailableListener {
public:
    HeicCompositeStream(wp<CameraDeviceBase> device,
            wp<hardware::camera2::ICameraDeviceCallbacks> cb);
    ~HeicCompositeStream() override;

    static bool isHeicCompositeStream(const sp<Surface> &surface);

    status_t createInternalStreams(const std::vector<sp<Surface>>& consumers,
            bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
            camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
            std::vector<int> *surfaceIds, int streamSetId, bool isShared) override;

    status_t deleteInternalStreams() override;

    status_t configureStream() override;

    status_t insertGbp(SurfaceMap* /*out*/outSurfaceMap, Vector<int32_t>* /*out*/outputStreamIds,
            int32_t* /*out*/currentStreamId) override;

    void onShutter(const CaptureResultExtras& resultExtras, nsecs_t timestamp) override;

    int getStreamId() override { return mMainImageStreamId; }

    // Use onShutter to keep track of frame number <-> timestamp mapping.
    void onBufferReleased(const BufferInfo& bufferInfo) override;
    void onBufferRequestForFrameNumber(uint64_t frameNumber, int streamId,
            const CameraMetadata& settings) override;

    // CpuConsumer listener implementation
    void onFrameAvailable(const BufferItem& item) override;

    // Return stream information about the internal camera streams
    static status_t getCompositeStreamInfo(const OutputStreamInfo &streamInfo,
            const CameraMetadata& ch, std::vector<OutputStreamInfo>* compositeOutput /*out*/);

    static bool isSizeSupportedByHeifEncoder(int32_t width, int32_t height,
            bool* useHeic, bool* useGrid, int64_t* stall, AString* hevcName = nullptr);
    static bool isInMemoryTempFileSupported();
protected:

    bool threadLoop() override;
    bool onStreamBufferError(const CaptureResultExtras& resultExtras) override;
    void onResultError(const CaptureResultExtras& resultExtras) override;

private:
    //
    // HEIC/HEVC Codec related structures, utility functions, and callbacks
    //
    struct CodecOutputBufferInfo {
        int32_t index;
        int32_t offset;
        int32_t size;
        int64_t timeUs;
        uint32_t flags;
    };

    struct CodecInputBufferInfo {
        int32_t index;
        int64_t timeUs;
        size_t tileIndex;
    };

    class CodecCallbackHandler : public AHandler {
    public:
        explicit CodecCallbackHandler(wp<HeicCompositeStream> parent) {
            mParent = parent;
        }
        virtual void onMessageReceived(const sp<AMessage> &msg);
    private:
        wp<HeicCompositeStream> mParent;
    };

    enum {
        kWhatCallbackNotify,
    };

    bool              mUseHeic;
    sp<MediaCodec>    mCodec;
    sp<ALooper>       mCodecLooper, mCallbackLooper;
    sp<CodecCallbackHandler> mCodecCallbackHandler;
    sp<AMessage>      mAsyncNotify;
    sp<AMessage>      mFormat;
    size_t            mNumOutputTiles;

    int32_t           mOutputWidth, mOutputHeight;
    size_t            mMaxHeicBufferSize;
    int32_t           mGridWidth, mGridHeight;
    size_t            mGridRows, mGridCols;
    bool              mUseGrid; // Whether to use framework YUV frame tiling.

    static const int64_t kNoFrameDropMaxPtsGap = -1000000;
    static const int32_t kNoGridOpRate = 30;
    static const int32_t kGridOpRate = 120;

    void onHeicOutputFrameAvailable(const CodecOutputBufferInfo& bufferInfo);
    void onHeicInputFrameAvailable(int32_t index);  // Only called for YUV input mode.
    void onHeicFormatChanged(sp<AMessage>& newFormat);
    void onHeicCodecError();

    status_t initializeCodec(uint32_t width, uint32_t height,
            const sp<CameraDeviceBase>& cameraDevice);
    void deinitCodec();

    //
    // Composite stream related structures, utility functions and callbacks.
    //
    struct InputFrame {
        int32_t                   orientation;
        int32_t                   quality;

        CpuConsumer::LockedBuffer          appSegmentBuffer;
        std::vector<CodecOutputBufferInfo> codecOutputBuffers;
        std::unique_ptr<CameraMetadata>    result;

        // Fields that are only applicable to HEVC tiling.
        CpuConsumer::LockedBuffer          yuvBuffer;
        std::vector<CodecInputBufferInfo>  codecInputBuffers;

        bool                      error;
        bool                      errorNotified;
        int64_t                   frameNumber;

        sp<AMessage>              format;
        sp<MediaMuxer>            muxer;
        int                       fenceFd;
        int                       fileFd;
        ssize_t                   trackIndex;
        ANativeWindowBuffer       *anb;

        bool                      appSegmentWritten;
        size_t                    pendingOutputTiles;
        size_t                    codecInputCounter;

        InputFrame() : orientation(0), quality(kDefaultJpegQuality), error(false),
                       errorNotified(false), frameNumber(-1), fenceFd(-1), fileFd(-1),
                       trackIndex(-1), anb(nullptr), appSegmentWritten(false),
                       pendingOutputTiles(0), codecInputCounter(0) { }
    };

    void compilePendingInputLocked();
    // Find first complete and valid frame with smallest timestamp
    bool getNextReadyInputLocked(int64_t *currentTs /*out*/);
    // Find next failing frame number with smallest timestamp and return respective frame number
    int64_t getNextFailingInputLocked(int64_t *currentTs /*out*/);

    status_t processInputFrame(nsecs_t timestamp, InputFrame &inputFrame);
    status_t processCodecInputFrame(InputFrame &inputFrame);
    status_t startMuxerForInputFrame(nsecs_t timestamp, InputFrame &inputFrame);
    status_t processAppSegment(nsecs_t timestamp, InputFrame &inputFrame);
    status_t processOneCodecOutputFrame(nsecs_t timestamp, InputFrame &inputFrame);
    status_t processCompletedInputFrame(nsecs_t timestamp, InputFrame &inputFrame);

    void releaseInputFrameLocked(InputFrame *inputFrame /*out*/);
    void releaseInputFramesLocked();

    size_t findAppSegmentsSize(const uint8_t* appSegmentBuffer, size_t maxSize,
            size_t* app1SegmentSize);
    int64_t findTimestampInNsLocked(int64_t timeInUs);
    status_t copyOneYuvTile(sp<MediaCodecBuffer>& codecBuffer,
            const CpuConsumer::LockedBuffer& yuvBuffer,
            size_t top, size_t left, size_t width, size_t height);
    void initCopyRowFunction(int32_t width);
    static size_t calcAppSegmentMaxSize(const CameraMetadata& info);

    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    static const int32_t kDefaultJpegQuality = 99;
    static const auto kJpegDataSpace = HAL_DATASPACE_V0_JFIF;
    static const android_dataspace kAppSegmentDataSpace =
            static_cast<android_dataspace>(HAL_DATASPACE_JPEG_APP_SEGMENTS);
    static const android_dataspace kHeifDataSpace =
            static_cast<android_dataspace>(HAL_DATASPACE_HEIF);
    // Use the limit of pipeline depth in the API sepc as maximum number of acquired
    // app segment buffers.
    static const uint32_t kMaxAcquiredAppSegment = 8;

    int               mAppSegmentStreamId, mAppSegmentSurfaceId;
    sp<CpuConsumer>   mAppSegmentConsumer;
    sp<Surface>       mAppSegmentSurface;
    size_t            mAppSegmentMaxSize;
    CameraMetadata    mStaticInfo;

    int               mMainImageStreamId, mMainImageSurfaceId;
    sp<Surface>       mMainImageSurface;
    sp<CpuConsumer>   mMainImageConsumer; // Only applicable for HEVC codec.
    bool              mYuvBufferAcquired; // Only applicable to HEVC codec

    static const int32_t kMaxOutputSurfaceProducerCount = 1;
    sp<Surface>       mOutputSurface;
    sp<ProducerListener> mProducerListener;
    int32_t           mDequeuedOutputBufferCnt;

    // Map from frame number to JPEG setting of orientation+quality
    std::map<int64_t, std::pair<int32_t, int32_t>> mSettingsByFrameNumber;
    // Map from timestamp to JPEG setting of orientation+quality
    std::map<int64_t, std::pair<int32_t, int32_t>> mSettingsByTimestamp;

    // Keep all incoming APP segment Blob buffer pending further processing.
    std::vector<int64_t> mInputAppSegmentBuffers;
    int32_t           mLockedAppSegmentBufferCnt;

    // Keep all incoming HEIC blob buffer pending further processing.
    std::vector<CodecOutputBufferInfo> mCodecOutputBuffers;
    std::queue<int64_t> mCodecOutputBufferTimestamps;
    size_t mCodecOutputCounter;

    // Keep all incoming Yuv buffer pending tiling and encoding (for HEVC YUV tiling only)
    std::vector<int64_t> mInputYuvBuffers;
    // Keep all codec input buffers ready to be filled out (for HEVC YUV tiling only)
    std::vector<int32_t> mCodecInputBuffers;

    // Artificial strictly incremental YUV grid timestamp to make encoder happy.
    int64_t mGridTimestampUs;

    // In most common use case, entries are accessed in order.
    std::map<int64_t, InputFrame> mPendingInputFrames;

    // Function pointer of libyuv row copy.
    void (*mFnCopyRow)(const uint8_t* src, uint8_t* dst, int width);
};

}; // namespace camera3
}; // namespace android

#endif //ANDROID_SERVERS_CAMERA_CAMERA3_HEIC_COMPOSITE_STREAM_H
