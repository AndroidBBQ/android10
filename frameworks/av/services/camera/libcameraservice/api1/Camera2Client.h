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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2CLIENT_H
#define ANDROID_SERVERS_CAMERA_CAMERA2CLIENT_H

#include "CameraService.h"
#include "common/CameraDeviceBase.h"
#include "common/Camera2ClientBase.h"
#include "api1/client2/Parameters.h"
#include "api1/client2/FrameProcessor.h"
//#include "api1/client2/StreamingProcessor.h"
//#include "api1/client2/JpegProcessor.h"
//#include "api1/client2/ZslProcessor.h"
//#include "api1/client2/CaptureSequencer.h"
//#include "api1/client2/CallbackProcessor.h"

namespace android {

namespace camera2 {

class StreamingProcessor;
class JpegProcessor;
class ZslProcessor;
class CaptureSequencer;
class CallbackProcessor;

}

class IMemory;
/**
 * Interface between android.hardware.Camera API and Camera HAL device for versions
 * CAMERA_DEVICE_API_VERSION_3_0 and above.
 */
class Camera2Client :
        public Camera2ClientBase<CameraService::Client>
{
public:
    /**
     * ICamera interface (see ICamera for details)
     */

    virtual binder::Status  disconnect();
    virtual status_t        connect(const sp<hardware::ICameraClient>& client);
    virtual status_t        lock();
    virtual status_t        unlock();
    virtual status_t        setPreviewTarget(
        const sp<IGraphicBufferProducer>& bufferProducer);
    virtual void            setPreviewCallbackFlag(int flag);
    virtual status_t        setPreviewCallbackTarget(
        const sp<IGraphicBufferProducer>& callbackProducer);

    virtual status_t        startPreview();
    virtual void            stopPreview();
    virtual bool            previewEnabled();
    virtual status_t        setVideoBufferMode(int32_t videoBufferMode);
    virtual status_t        startRecording();
    virtual void            stopRecording();
    virtual bool            recordingEnabled();
    virtual void            releaseRecordingFrame(const sp<IMemory>& mem);
    virtual void            releaseRecordingFrameHandle(native_handle_t *handle);
    virtual void            releaseRecordingFrameHandleBatch(
                                    const std::vector<native_handle_t*>& handles);
    virtual status_t        autoFocus();
    virtual status_t        cancelAutoFocus();
    virtual status_t        takePicture(int msgType);
    virtual status_t        setParameters(const String8& params);
    virtual String8         getParameters() const;
    virtual status_t        sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    virtual void            notifyError(int32_t errorCode,
                                        const CaptureResultExtras& resultExtras);
    virtual status_t        setVideoTarget(const sp<IGraphicBufferProducer>& bufferProducer);

    /**
     * Interface used by CameraService
     */

    Camera2Client(const sp<CameraService>& cameraService,
            const sp<hardware::ICameraClient>& cameraClient,
            const String16& clientPackageName,
            const String8& cameraDeviceId,
            int api1CameraId,
            int cameraFacing,
            int clientPid,
            uid_t clientUid,
            int servicePid);

    virtual ~Camera2Client();

    virtual status_t initialize(sp<CameraProviderManager> manager,
            const String8& monitorTags) override;

    virtual status_t dump(int fd, const Vector<String16>& args);

    virtual status_t dumpClient(int fd, const Vector<String16>& args);

    /**
     * Interface used by CameraDeviceBase
     */

    virtual void notifyAutoFocus(uint8_t newState, int triggerId);
    virtual void notifyAutoExposure(uint8_t newState, int triggerId);
    virtual void notifyShutter(const CaptureResultExtras& resultExtras,
                               nsecs_t timestamp);

    /**
     * Interface used by independent components of Camera2Client.
     */

    camera2::SharedParameters& getParameters();

    int getPreviewStreamId() const;
    int getCaptureStreamId() const;
    int getCallbackStreamId() const;
    int getRecordingStreamId() const;
    int getZslStreamId() const;

    status_t registerFrameListener(int32_t minId, int32_t maxId,
            const wp<camera2::FrameProcessor::FilteredListener>& listener,
            bool sendPartials = true);
    status_t removeFrameListener(int32_t minId, int32_t maxId,
            const wp<camera2::FrameProcessor::FilteredListener>& listener);

    status_t stopStream();

    // For the slowJpegMode to create jpeg stream when precapture sequence is done
    status_t createJpegStreamL(camera2::Parameters &params);

    static size_t calculateBufferSize(int width, int height,
            int format, int stride);

    static const int32_t kPreviewRequestIdStart = 10000000;
    static const int32_t kPreviewRequestIdEnd   = 20000000;

    static const int32_t kRecordingRequestIdStart  = 20000000;
    static const int32_t kRecordingRequestIdEnd    = 30000000;

    static const int32_t kCaptureRequestIdStart = 30000000;
    static const int32_t kCaptureRequestIdEnd   = 40000000;

    // Constant strings for ATRACE logging
    static const char* kAutofocusLabel;
    static const char* kTakepictureLabel;

    // Used with stream IDs
    static const int NO_STREAM = -1;

private:
    /** ICamera interface-related private members */
    typedef camera2::Parameters Parameters;

    status_t setPreviewWindowL(const sp<IBinder>& binder,
            const sp<Surface>& window);
    status_t startPreviewL(Parameters &params, bool restart);
    void     stopPreviewL();
    status_t startRecordingL(Parameters &params, bool restart);
    bool     recordingEnabledL();

    // Individual commands for sendCommand()
    status_t commandStartSmoothZoomL();
    status_t commandStopSmoothZoomL();
    status_t commandSetDisplayOrientationL(int degrees);
    status_t commandEnableShutterSoundL(bool enable);
    status_t commandPlayRecordingSoundL();
    status_t commandStartFaceDetectionL(int type);
    status_t commandStopFaceDetectionL(Parameters &params);
    status_t commandEnableFocusMoveMsgL(bool enable);
    status_t commandPingL();
    status_t commandSetVideoBufferCountL(size_t count);
    status_t commandSetVideoFormatL(int format, android_dataspace dataSpace);

    // Current camera device configuration
    camera2::SharedParameters mParameters;

    /** Camera device-related private members */

    void     setPreviewCallbackFlagL(Parameters &params, int flag);
    status_t updateRequests(Parameters &params);

    template <typename ProcessorT>
    status_t updateProcessorStream(sp<ProcessorT> processor, Parameters params);
    template <typename ProcessorT,
              status_t (ProcessorT::*updateStreamF)(const Parameters &)>
    status_t updateProcessorStream(sp<ProcessorT> processor, Parameters params);

    sp<camera2::FrameProcessor> mFrameProcessor;

    /* Preview/Recording related members */

    sp<IBinder> mPreviewSurface;
    sp<IBinder> mVideoSurface;
    sp<camera2::StreamingProcessor> mStreamingProcessor;

    /** Preview callback related members */

    sp<camera2::CallbackProcessor> mCallbackProcessor;

    /* Still image capture related members */

    sp<camera2::CaptureSequencer> mCaptureSequencer;
    sp<camera2::JpegProcessor> mJpegProcessor;
    sp<camera2::ZslProcessor> mZslProcessor;

    /** Utility members */
    bool mLegacyMode;

    // Wait until the camera device has received the latest control settings
    status_t syncWithDevice();

    // Video snapshot jpeg size overriding helper function
    status_t overrideVideoSnapshotSize(Parameters &params);

    template<typename TProviderPtr>
    status_t initializeImpl(TProviderPtr providerPtr, const String8& monitorTags);

    bool isZslEnabledInStillTemplate();
};

}; // namespace android

#endif
