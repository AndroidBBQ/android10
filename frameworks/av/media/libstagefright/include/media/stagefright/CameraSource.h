/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef CAMERA_SOURCE_H_

#define CAMERA_SOURCE_H_

#include <deque>
#include <media/MediaSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <camera/android/hardware/ICamera.h>
#include <camera/ICameraRecordingProxy.h>
#include <camera/ICameraRecordingProxyListener.h>
#include <camera/CameraParameters.h>
#include <gui/BufferItemConsumer.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String16.h>
#include <media/hardware/MetadataBufferType.h>

namespace android {

class IMemory;
class Camera;
class Surface;

class CameraSource : public MediaSource, public MediaBufferObserver {
public:
    /**
     * Factory method to create a new CameraSource using the current
     * settings (such as video size, frame rate, color format, etc)
     * from the default camera.
     *
     * @param clientName The package/process name of the client application.
     *    This is used for permissions checking.
     * @return NULL on error.
     */
    static CameraSource *Create(const String16 &clientName);

    /**
     * Factory method to create a new CameraSource.
     *
     * @param camera the video input frame data source. If it is NULL,
     *          we will try to connect to the camera with the given
     *          cameraId.
     *
     * @param cameraId the id of the camera that the source will connect
     *          to if camera is NULL; otherwise ignored.
     * @param clientName the package/process name of the camera-using
     *          application if camera is NULL; otherwise ignored. Used for
     *          permissions checking.
     * @param clientUid the UID of the camera-using application if camera is
     *          NULL; otherwise ignored. Used for permissions checking.
     * @param clientPid the PID of the camera-using application if camera is
     *          NULL; otherwise ignored. Used for permissions checking.
     * @param videoSize the dimension (in pixels) of the video frame
     * @param frameRate the target frames per second
     * @param surface the preview surface for display where preview
     *          frames are sent to
     * @param storeMetaDataInVideoBuffers true to request the camera
     *          source to store meta data in video buffers; false to
     *          request the camera source to store real YUV frame data
     *          in the video buffers. The camera source may not support
     *          storing meta data in video buffers, if so, a request
     *          to do that will NOT be honored. To find out whether
     *          meta data is actually being stored in video buffers
     *          during recording, call isMetaDataStoredInVideoBuffers().
     *
     * @return NULL on error.
     */
    static CameraSource *CreateFromCamera(const sp<hardware::ICamera> &camera,
                                          const sp<ICameraRecordingProxy> &proxy,
                                          int32_t cameraId,
                                          const String16& clientName,
                                          uid_t clientUid,
                                          pid_t clientPid,
                                          Size videoSize,
                                          int32_t frameRate,
                                          const sp<IGraphicBufferProducer>& surface,
                                          bool storeMetaDataInVideoBuffers = true);

    virtual ~CameraSource();

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop() { return reset(); }
    virtual status_t read(
            MediaBufferBase **buffer, const ReadOptions *options = NULL);
    virtual status_t setStopTimeUs(int64_t stopTimeUs);

    /**
     * Check whether a CameraSource object is properly initialized.
     * Must call this method before stop().
     * @return OK if initialization has successfully completed.
     */
    virtual status_t initCheck() const;

    /**
     * Returns the MetaData associated with the CameraSource,
     * including:
     * kKeyColorFormat: YUV color format of the video frames
     * kKeyWidth, kKeyHeight: dimension (in pixels) of the video frames
     * kKeySampleRate: frame rate in frames per second
     * kKeyMIMEType: always fixed to be MEDIA_MIMETYPE_VIDEO_RAW
     */
    virtual sp<MetaData> getFormat();

    /**
     * Tell whether this camera source stores meta data or real YUV
     * frame data in video buffers.
     *
     * @return a valid type if meta data is stored in the video
     *      buffers; kMetadataBufferTypeInvalid if real YUV data is stored in
     *      the video buffers.
     */
    MetadataBufferType metaDataStoredInVideoBuffers() const;

    virtual void signalBufferReturned(MediaBufferBase* buffer);

protected:

    /**
     * The class for listening to BnCameraRecordingProxyListener. This is used to receive video
     * buffers in VIDEO_BUFFER_MODE_DATA_CALLBACK_YUV and VIDEO_BUFFER_MODE_DATA_CALLBACK_METADATA
     * mode. When a frame is available, CameraSource::dataCallbackTimestamp() will be called.
     */
    class ProxyListener: public BnCameraRecordingProxyListener {
    public:
        ProxyListener(const sp<CameraSource>& source);
        virtual void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType,
                const sp<IMemory> &data);
        virtual void recordingFrameHandleCallbackTimestamp(int64_t timestampUs,
                native_handle_t* handle);
        virtual void recordingFrameHandleCallbackTimestampBatch(
                const std::vector<int64_t>& timestampsUs,
                const std::vector<native_handle_t*>& handles);

    private:
        sp<CameraSource> mSource;
    };

    /**
     * The class for listening to BufferQueue's onFrameAvailable. This is used to receive video
     * buffers in VIDEO_BUFFER_MODE_BUFFER_QUEUE mode. When a frame is available,
     * CameraSource::processBufferQueueFrame() will be called.
     */
    class BufferQueueListener : public Thread,  public BufferItemConsumer::FrameAvailableListener {
    public:
        BufferQueueListener(const sp<BufferItemConsumer> &consumer,
                const sp<CameraSource> &cameraSource);
        virtual void onFrameAvailable(const BufferItem& item);
        virtual bool threadLoop();
    private:
        static const nsecs_t kFrameAvailableTimeout = 50000000; // 50ms

        sp<BufferItemConsumer> mConsumer;
        sp<CameraSource> mCameraSource;

        Mutex mLock;
        Condition mFrameAvailableSignal;
        bool mFrameAvailable;
    };

    // isBinderAlive needs linkToDeath to work.
    class DeathNotifier: public IBinder::DeathRecipient {
    public:
        DeathNotifier() {}
        virtual void binderDied(const wp<IBinder>& who);
    };

    enum CameraFlags {
        FLAGS_SET_CAMERA = 1L << 0,
        FLAGS_HOT_CAMERA = 1L << 1,
    };

    int32_t  mCameraFlags;
    Size     mVideoSize;
    int32_t  mNumInputBuffers;
    int32_t  mVideoFrameRate;
    int32_t  mColorFormat;
    int32_t  mEncoderFormat;
    int32_t  mEncoderDataSpace;
    status_t mInitCheck;

    sp<Camera>   mCamera;
    sp<ICameraRecordingProxy>   mCameraRecordingProxy;
    sp<DeathNotifier> mDeathNotifier;
    sp<IGraphicBufferProducer>  mSurface;
    sp<MetaData> mMeta;

    int64_t mStartTimeUs;
    int32_t mNumFramesReceived;
    int64_t mLastFrameTimestampUs;
    bool mStarted;
    bool mEos;
    int32_t mNumFramesEncoded;

    // Time between capture of two frames.
    int64_t mTimeBetweenFrameCaptureUs;

    CameraSource(const sp<hardware::ICamera>& camera, const sp<ICameraRecordingProxy>& proxy,
                 int32_t cameraId, const String16& clientName, uid_t clientUid, pid_t clientPid,
                 Size videoSize, int32_t frameRate,
                 const sp<IGraphicBufferProducer>& surface,
                 bool storeMetaDataInVideoBuffers);

    virtual status_t startCameraRecording();
    virtual void releaseRecordingFrame(const sp<IMemory>& frame);
    virtual void releaseRecordingFrameHandle(native_handle_t* handle);
    // stagefright recorder not using this for now
    virtual void releaseRecordingFrameHandleBatch(const std::vector<native_handle_t*>& handles);

    // Returns true if need to skip the current frame.
    // Called from dataCallbackTimestamp.
    virtual bool skipCurrentFrame(int64_t /*timestampUs*/) {return false;}

    // Callback called when still camera raw data is available.
    virtual void dataCallback(int32_t /*msgType*/, const sp<IMemory>& /*data*/) {}

    virtual void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType,
            const sp<IMemory> &data);

    virtual void recordingFrameHandleCallbackTimestamp(int64_t timestampUs,
            native_handle_t* handle);

    virtual void recordingFrameHandleCallbackTimestampBatch(
            const std::vector<int64_t>& timestampsUs,
            const std::vector<native_handle_t*>& handles);

    // Process a buffer item received in BufferQueueListener.
    virtual void processBufferQueueFrame(BufferItem& buffer);

    void releaseCamera();

private:
    friend struct CameraSourceListener;

    Mutex mLock;
    Condition mFrameAvailableCondition;
    Condition mFrameCompleteCondition;
    List<sp<IMemory> > mFramesReceived;
    List<sp<IMemory> > mFramesBeingEncoded;
    List<int64_t> mFrameTimes;

    int64_t mFirstFrameTimeUs;
    int64_t mStopSystemTimeUs;
    int32_t mNumFramesDropped;
    int32_t mNumGlitches;
    int64_t mGlitchDurationThresholdUs;
    bool mCollectStats;

    // The mode video buffers are received from camera. One of VIDEO_BUFFER_MODE_*.
    int32_t mVideoBufferMode;

    static const uint32_t kDefaultVideoBufferCount = 32;

    /**
     * The following variables are used in VIDEO_BUFFER_MODE_BUFFER_QUEUE mode.
     */
    static const size_t kConsumerBufferCount = 8;
    static const nsecs_t kMemoryBaseAvailableTimeoutNs = 200000000; // 200ms
    // Consumer and producer of the buffer queue between this class and camera.
    sp<BufferItemConsumer> mVideoBufferConsumer;
    sp<IGraphicBufferProducer> mVideoBufferProducer;
    // Memory used to send the buffers to encoder, where sp<IMemory> stores VideoNativeMetadata.
    sp<IMemoryHeap> mMemoryHeapBase;
    List<sp<IMemory>> mMemoryBases;
    // The condition that will be signaled when there is an entry available in mMemoryBases.
    Condition mMemoryBaseAvailableCond;
    // A mapping from ANativeWindowBuffer sent to encoder to BufferItem received from camera.
    // This is protected by mLock.
    KeyedVector<ANativeWindowBuffer*, BufferItem> mReceivedBufferItemMap;
    sp<BufferQueueListener> mBufferQueueListener;

    Mutex mBatchLock; // protecting access to mInflightXXXXX members below
    // Start of members protected by mBatchLock
    std::deque<uint32_t> mInflightBatchSizes;
    std::vector<native_handle_t*> mInflightReturnedHandles;
    std::vector<const sp<IMemory>> mInflightReturnedMemorys;
    // End of members protected by mBatchLock

    void releaseQueuedFrames();
    void releaseOneRecordingFrame(const sp<IMemory>& frame);
    void createVideoBufferMemoryHeap(size_t size, uint32_t bufferCount);

    status_t init(const sp<hardware::ICamera>& camera, const sp<ICameraRecordingProxy>& proxy,
                  int32_t cameraId, const String16& clientName, uid_t clientUid, pid_t clientPid,
                  Size videoSize, int32_t frameRate, bool storeMetaDataInVideoBuffers);

    status_t initWithCameraAccess(
                  const sp<hardware::ICamera>& camera, const sp<ICameraRecordingProxy>& proxy,
                  int32_t cameraId, const String16& clientName, uid_t clientUid, pid_t clientPid,
                  Size videoSize, int32_t frameRate, bool storeMetaDataInVideoBuffers);

    // Initialize the buffer queue used in VIDEO_BUFFER_MODE_BUFFER_QUEUE mode.
    status_t initBufferQueue(uint32_t width, uint32_t height, uint32_t format,
                  android_dataspace dataSpace, uint32_t bufferCount);

    status_t isCameraAvailable(const sp<hardware::ICamera>& camera,
                               const sp<ICameraRecordingProxy>& proxy,
                               int32_t cameraId,
                               const String16& clientName,
                               uid_t clientUid,
                               pid_t clientPid);

    status_t isCameraColorFormatSupported(const CameraParameters& params);
    status_t configureCamera(CameraParameters* params,
                    int32_t width, int32_t height,
                    int32_t frameRate);

    status_t checkVideoSize(const CameraParameters& params,
                    int32_t width, int32_t height);

    status_t checkFrameRate(const CameraParameters& params,
                    int32_t frameRate);

    // Check if this frame should be skipped based on the frame's timestamp in microsecond.
    // mLock must be locked before calling this function.
    bool shouldSkipFrameLocked(int64_t timestampUs);

    void stopCameraRecording();
    status_t reset();

    CameraSource(const CameraSource &);
    CameraSource &operator=(const CameraSource &);
};

}  // namespace android

#endif  // CAMERA_SOURCE_H_
