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

#ifndef STAGEFRIGHT_RECORDER_H_

#define STAGEFRIGHT_RECORDER_H_

#include <media/MediaAnalyticsItem.h>
#include <media/MediaRecorderBase.h>
#include <camera/CameraParameters.h>
#include <utils/String8.h>

#include <system/audio.h>

#include <media/hardware/MetadataBufferType.h>

namespace android {

class Camera;
class ICameraRecordingProxy;
class CameraSource;
class CameraSourceTimeLapse;
struct MediaCodecSource;
struct MediaSource;
struct MediaWriter;
class MetaData;
struct AudioSource;
class MediaProfiles;
struct ALooper;

struct StagefrightRecorder : public MediaRecorderBase {
    explicit StagefrightRecorder(const String16 &opPackageName);
    virtual ~StagefrightRecorder();
    virtual status_t init();
    virtual status_t setAudioSource(audio_source_t as);
    virtual status_t setVideoSource(video_source vs);
    virtual status_t setOutputFormat(output_format of);
    virtual status_t setAudioEncoder(audio_encoder ae);
    virtual status_t setVideoEncoder(video_encoder ve);
    virtual status_t setVideoSize(int width, int height);
    virtual status_t setVideoFrameRate(int frames_per_second);
    virtual status_t setCamera(const sp<hardware::ICamera>& camera, const sp<ICameraRecordingProxy>& proxy);
    virtual status_t setPreviewSurface(const sp<IGraphicBufferProducer>& surface);
    virtual status_t setInputSurface(const sp<PersistentSurface>& surface);
    virtual status_t setOutputFile(int fd);
    virtual status_t setNextOutputFile(int fd);
    virtual status_t setParameters(const String8 &params);
    virtual status_t setListener(const sp<IMediaRecorderClient> &listener);
    virtual status_t setClientName(const String16 &clientName);
    virtual status_t prepare();
    virtual status_t start();
    virtual status_t pause();
    virtual status_t resume();
    virtual status_t stop();
    virtual status_t close();
    virtual status_t reset();
    virtual status_t getMaxAmplitude(int *max);
    virtual status_t getMetrics(Parcel *reply);
    virtual status_t dump(int fd, const Vector<String16> &args) const;
    // Querying a SurfaceMediaSourcer
    virtual sp<IGraphicBufferProducer> querySurfaceMediaSource() const;
    virtual status_t setInputDevice(audio_port_handle_t deviceId);
    virtual status_t getRoutedDeviceId(audio_port_handle_t* deviceId);
    virtual void setAudioDeviceCallback(const sp<AudioSystem::AudioDeviceCallback>& callback);
    virtual status_t enableAudioDeviceCallback(bool enabled);
    virtual status_t getActiveMicrophones(std::vector<media::MicrophoneInfo>* activeMicrophones);
    virtual status_t setPreferredMicrophoneDirection(audio_microphone_direction_t direction);
    virtual status_t setPreferredMicrophoneFieldDimension(float zoom);
            status_t getPortId(audio_port_handle_t *portId) const override;

private:
    mutable Mutex mLock;
    sp<hardware::ICamera> mCamera;
    sp<ICameraRecordingProxy> mCameraProxy;
    sp<IGraphicBufferProducer> mPreviewSurface;
    sp<PersistentSurface> mPersistentSurface;
    sp<IMediaRecorderClient> mListener;
    String16 mClientName;
    uid_t mClientUid;
    pid_t mClientPid;
    sp<MediaWriter> mWriter;
    int mOutputFd;
    sp<AudioSource> mAudioSourceNode;

    MediaAnalyticsItem *mAnalyticsItem;
    bool mAnalyticsDirty;
    void flushAndResetMetrics(bool reinitialize);
    void updateMetrics();

    audio_source_t mAudioSource;
    video_source mVideoSource;
    output_format mOutputFormat;
    audio_encoder mAudioEncoder;
    video_encoder mVideoEncoder;
    bool mUse64BitFileOffset;
    int32_t mVideoWidth, mVideoHeight;
    int32_t mFrameRate;
    int32_t mVideoBitRate;
    int32_t mAudioBitRate;
    int32_t mAudioChannels;
    int32_t mSampleRate;
    int32_t mInterleaveDurationUs;
    int32_t mIFramesIntervalSec;
    int32_t mCameraId;
    int32_t mVideoEncoderProfile;
    int32_t mVideoEncoderLevel;
    int32_t mMovieTimeScale;
    int32_t mVideoTimeScale;
    int32_t mAudioTimeScale;
    int64_t mMaxFileSizeBytes;
    int64_t mMaxFileDurationUs;
    int64_t mTrackEveryTimeDurationUs;
    int32_t mRotationDegrees;  // Clockwise
    int32_t mLatitudex10000;
    int32_t mLongitudex10000;
    int32_t mStartTimeOffsetMs;
    int32_t mTotalBitRate;

    int64_t mDurationRecordedUs;
    int64_t mStartedRecordingUs;
    int64_t mDurationPausedUs;
    int32_t mNPauses;

    bool mCaptureFpsEnable;
    double mCaptureFps;
    int64_t mTimeBetweenCaptureUs;
    sp<CameraSourceTimeLapse> mCameraSourceTimeLapse;

    String8 mParams;

    MetadataBufferType mMetaDataStoredInVideoBuffers;
    MediaProfiles *mEncoderProfiles;

    int64_t mPauseStartTimeUs;
    int64_t mTotalPausedDurationUs;
    sp<MediaCodecSource> mAudioEncoderSource;
    sp<MediaCodecSource> mVideoEncoderSource;

    bool mStarted;
    // Needed when GLFrames are encoded.
    // An <IGraphicBufferProducer> pointer
    // will be sent to the client side using which the
    // frame buffers will be queued and dequeued
    sp<IGraphicBufferProducer> mGraphicBufferProducer;
    sp<ALooper> mLooper;

    audio_port_handle_t mSelectedDeviceId;
    bool mDeviceCallbackEnabled;
    wp<AudioSystem::AudioDeviceCallback> mAudioDeviceCallback;

    audio_microphone_direction_t mSelectedMicDirection;
    float mSelectedMicFieldDimension;

    static const int kMaxHighSpeedFps = 1000;

    status_t prepareInternal();
    status_t setupMPEG4orWEBMRecording();
    void setupMPEG4orWEBMMetaData(sp<MetaData> *meta);
    status_t setupAMRRecording();
    status_t setupAACRecording();
    status_t setupOggRecording();
    status_t setupRawAudioRecording();
    status_t setupRTPRecording();
    status_t setupMPEG2TSRecording();
    sp<MediaCodecSource> createAudioSource();
    status_t checkVideoEncoderCapabilities();
    status_t checkAudioEncoderCapabilities();
    // Generic MediaSource set-up. Returns the appropriate
    // source (CameraSource or SurfaceMediaSource)
    // depending on the videosource type
    status_t setupMediaSource(sp<MediaSource> *mediaSource);
    status_t setupCameraSource(sp<CameraSource> *cameraSource);
    status_t setupAudioEncoder(const sp<MediaWriter>& writer);
    status_t setupVideoEncoder(const sp<MediaSource>& cameraSource, sp<MediaCodecSource> *source);

    // Encoding parameter handling utilities
    status_t setParameter(const String8 &key, const String8 &value);
    status_t setParamAudioEncodingBitRate(int32_t bitRate);
    status_t setParamAudioNumberOfChannels(int32_t channles);
    status_t setParamAudioSamplingRate(int32_t sampleRate);
    status_t setParamAudioTimeScale(int32_t timeScale);
    status_t setParamCaptureFpsEnable(int32_t timeLapseEnable);
    status_t setParamCaptureFps(double fps);
    status_t setParamVideoEncodingBitRate(int32_t bitRate);
    status_t setParamVideoIFramesInterval(int32_t seconds);
    status_t setParamVideoEncoderProfile(int32_t profile);
    status_t setParamVideoEncoderLevel(int32_t level);
    status_t setParamVideoCameraId(int32_t cameraId);
    status_t setParamVideoTimeScale(int32_t timeScale);
    status_t setParamVideoRotation(int32_t degrees);
    status_t setParamTrackTimeStatus(int64_t timeDurationUs);
    status_t setParamInterleaveDuration(int32_t durationUs);
    status_t setParam64BitFileOffset(bool use64BitFileOffset);
    status_t setParamMaxFileDurationUs(int64_t timeUs);
    status_t setParamMaxFileSizeBytes(int64_t bytes);
    status_t setParamMovieTimeScale(int32_t timeScale);
    status_t setParamGeoDataLongitude(int64_t longitudex10000);
    status_t setParamGeoDataLatitude(int64_t latitudex10000);
    void clipVideoBitRate();
    void clipVideoFrameRate();
    void clipVideoFrameWidth();
    void clipVideoFrameHeight();
    void clipAudioBitRate();
    void clipAudioSampleRate();
    void clipNumberOfAudioChannels();
    void setDefaultProfileIfNecessary();
    void setDefaultVideoEncoderIfNecessary();


    StagefrightRecorder(const StagefrightRecorder &);
    StagefrightRecorder &operator=(const StagefrightRecorder &);
};

}  // namespace android

#endif  // STAGEFRIGHT_RECORDER_H_
