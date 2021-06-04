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

#ifndef AUDIO_SOURCE_H_

#define AUDIO_SOURCE_H_

#include <media/AudioRecord.h>
#include <media/AudioSystem.h>
#include <media/MediaSource.h>
#include <media/MicrophoneInfo.h>
#include <media/stagefright/MediaBuffer.h>
#include <utils/List.h>

#include <system/audio.h>

#include <vector>

namespace android {

class AudioRecord;

struct AudioSource : public MediaSource, public MediaBufferObserver {
    // Note that the "channels" parameter _is_ the number of channels,
    // _not_ a bitmask of audio_channels_t constants.
    AudioSource(
            audio_source_t inputSource,
            const String16 &opPackageName,
            uint32_t sampleRate,
            uint32_t channels,
            uint32_t outSampleRate = 0,
            uid_t uid = -1,
            pid_t pid = -1,
            audio_port_handle_t selectedDeviceId = AUDIO_PORT_HANDLE_NONE,
            audio_microphone_direction_t selectedMicDirection = MIC_DIRECTION_UNSPECIFIED,
            float selectedMicFieldDimension = MIC_FIELD_DIMENSION_NORMAL);

    status_t initCheck() const;

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop() { return reset(); }
    virtual sp<MetaData> getFormat();

    // Returns the maximum amplitude since last call.
    int16_t getMaxAmplitude();

    virtual status_t read(
            MediaBufferBase **buffer, const ReadOptions *options = NULL);
    virtual status_t setStopTimeUs(int64_t stopTimeUs);

    status_t dataCallback(const AudioRecord::Buffer& buffer);
    virtual void signalBufferReturned(MediaBufferBase *buffer);

    status_t setInputDevice(audio_port_handle_t deviceId);
    status_t getRoutedDeviceId(audio_port_handle_t* deviceId);
    status_t addAudioDeviceCallback(const sp<AudioSystem::AudioDeviceCallback>& callback);
    status_t removeAudioDeviceCallback(const sp<AudioSystem::AudioDeviceCallback>& callback);

    status_t getActiveMicrophones(std::vector<media::MicrophoneInfo>* activeMicrophones);
    status_t setPreferredMicrophoneDirection(audio_microphone_direction_t direction);
    status_t setPreferredMicrophoneFieldDimension(float zoom);

    status_t getPortId(audio_port_handle_t *portId) const;

protected:
    virtual ~AudioSource();

private:
    enum {
        kMaxBufferSize = 2048,

        // After the initial mute, we raise the volume linearly
        // over kAutoRampDurationUs.
        kAutoRampDurationUs = 300000,

        // This is the initial mute duration to suppress
        // the video recording signal tone
        kAutoRampStartUs = 0,
    };

    Mutex mLock;
    Condition mFrameAvailableCondition;
    Condition mFrameEncodingCompletionCondition;

    sp<AudioRecord> mRecord;
    status_t mInitCheck;
    bool mStarted;
    int32_t mSampleRate;
    int32_t mOutSampleRate;

    bool mTrackMaxAmplitude;
    int64_t mStartTimeUs;
    int64_t mStopSystemTimeUs;
    int64_t mLastFrameTimestampUs;
    int16_t mMaxAmplitude;
    int64_t mPrevSampleTimeUs;
    int64_t mInitialReadTimeUs;
    int64_t mNumFramesReceived;
    int64_t mNumFramesSkipped;
    int64_t mNumFramesLost;
    int64_t mNumClientOwnedBuffers;
    bool mNoMoreFramesToRead;

    List<MediaBuffer * > mBuffersReceived;

    void trackMaxAmplitude(int16_t *data, int nSamples);

    // This is used to raise the volume from mute to the
    // actual level linearly.
    void rampVolume(
        int32_t startFrame, int32_t rampDurationFrames,
        uint8_t *data,   size_t bytes);

    void queueInputBuffer_l(MediaBuffer *buffer, int64_t timeUs);
    void releaseQueuedFrames_l();
    void waitOutstandingEncodingFrames_l();
    status_t reset();

    AudioSource(const AudioSource &);
    AudioSource &operator=(const AudioSource &);
};

}  // namespace android

#endif  // AUDIO_SOURCE_H_
