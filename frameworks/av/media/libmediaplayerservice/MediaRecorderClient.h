/*
 **
 ** Copyright 2008, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_MEDIARECORDERCLIENT_H
#define ANDROID_MEDIARECORDERCLIENT_H

#include "DeathNotifier.h"

#include <media/AudioSystem.h>
#include <media/IMediaRecorder.h>

#include <vector>

namespace android {

struct MediaRecorderBase;
class MediaPlayerService;
class ICameraRecordingProxy;

class MediaRecorderClient : public BnMediaRecorder
{
    class AudioDeviceUpdatedNotifier: public AudioSystem::AudioDeviceCallback
    {
    public:
        AudioDeviceUpdatedNotifier(const sp<IMediaRecorderClient>& listener);
        virtual ~AudioDeviceUpdatedNotifier();
        virtual void onAudioDeviceUpdate(
                audio_io_handle_t audioIo,
                audio_port_handle_t deviceId);
    private:
        wp<IMediaRecorderClient> mListener;
    };

public:
    virtual     status_t   setCamera(const sp<hardware::ICamera>& camera,
                                    const sp<ICameraRecordingProxy>& proxy);
    virtual     status_t   setPreviewSurface(const sp<IGraphicBufferProducer>& surface);
    virtual     status_t   setVideoSource(int vs);
    virtual     status_t   setAudioSource(int as);
    virtual     status_t   setOutputFormat(int of);
    virtual     status_t   setVideoEncoder(int ve);
    virtual     status_t   setAudioEncoder(int ae);
    virtual     status_t   setOutputFile(int fd);
    virtual     status_t   setNextOutputFile(int fd);
    virtual     status_t   setVideoSize(int width, int height);
    virtual     status_t   setVideoFrameRate(int frames_per_second);
    virtual     status_t   setParameters(const String8& params);
    virtual     status_t   setListener(
                              const sp<IMediaRecorderClient>& listener);
    virtual     status_t   setClientName(const String16& clientName);
    virtual     status_t   prepare();
    virtual     status_t   getMaxAmplitude(int* max);
    virtual     status_t   getMetrics(Parcel* reply);
    virtual     status_t   start();
    virtual     status_t   stop();
    virtual     status_t   reset();
    virtual     status_t   pause();
    virtual     status_t   resume();
    virtual     status_t   init();
    virtual     status_t   close();
    virtual     status_t   release();
    virtual     status_t   dump(int fd, const Vector<String16>& args);
    virtual     status_t   setInputSurface(const sp<PersistentSurface>& surface);
    virtual     sp<IGraphicBufferProducer> querySurfaceMediaSource();
    virtual     status_t   setInputDevice(audio_port_handle_t deviceId);
    virtual     status_t   getRoutedDeviceId(audio_port_handle_t* deviceId);
    virtual     status_t   enableAudioDeviceCallback(bool enabled);
    virtual     status_t   getActiveMicrophones(
                              std::vector<media::MicrophoneInfo>* activeMicrophones);
    virtual     status_t   setPreferredMicrophoneDirection(audio_microphone_direction_t direction);
    virtual     status_t   setPreferredMicrophoneFieldDimension(float zoom);
                status_t   getPortId(audio_port_handle_t *portId) override;

private:
    friend class           MediaPlayerService;  // for accessing private constructor

                           MediaRecorderClient(
                                   const sp<MediaPlayerService>& service,
                                                               pid_t pid,
                                                               const String16& opPackageName);
    virtual                ~MediaRecorderClient();

    std::vector<DeathNotifier> mDeathNotifiers;
    sp<AudioDeviceUpdatedNotifier> mAudioDeviceUpdatedNotifier;

    pid_t                  mPid;
    Mutex                  mLock;
    MediaRecorderBase      *mRecorder;
    sp<MediaPlayerService> mMediaPlayerService;
};

}; // namespace android

#endif // ANDROID_MEDIARECORDERCLIENT_H
