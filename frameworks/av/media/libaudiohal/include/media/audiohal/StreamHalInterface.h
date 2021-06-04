/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_STREAM_HAL_INTERFACE_H
#define ANDROID_HARDWARE_STREAM_HAL_INTERFACE_H

#include <vector>

#include <media/audiohal/EffectHalInterface.h>
#include <media/MicrophoneInfo.h>
#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

namespace android {

class StreamHalInterface : public virtual RefBase
{
  public:
    // Return the sampling rate in Hz - eg. 44100.
    virtual status_t getSampleRate(uint32_t *rate) = 0;

    // Return size of input/output buffer in bytes for this stream - eg. 4800.
    virtual status_t getBufferSize(size_t *size) = 0;

    // Return the channel mask.
    virtual status_t getChannelMask(audio_channel_mask_t *mask) = 0;

    // Return the audio format - e.g. AUDIO_FORMAT_PCM_16_BIT.
    virtual status_t getFormat(audio_format_t *format) = 0;

    // Convenience method.
    virtual status_t getAudioProperties(
            uint32_t *sampleRate, audio_channel_mask_t *mask, audio_format_t *format) = 0;

    // Set audio stream parameters.
    virtual status_t setParameters(const String8& kvPairs) = 0;

    // Get audio stream parameters.
    virtual status_t getParameters(const String8& keys, String8 *values) = 0;

    // Return the frame size (number of bytes per sample) of a stream.
    virtual status_t getFrameSize(size_t *size) = 0;

    // Add or remove the effect on the stream.
    virtual status_t addEffect(sp<EffectHalInterface> effect) = 0;
    virtual status_t removeEffect(sp<EffectHalInterface> effect) = 0;

    // Put the audio hardware input/output into standby mode.
    virtual status_t standby() = 0;

    virtual status_t dump(int fd) = 0;

    // Start a stream operating in mmap mode.
    virtual status_t start() = 0;

    // Stop a stream operating in mmap mode.
    virtual status_t stop() = 0;

    // Retrieve information on the data buffer in mmap mode.
    virtual status_t createMmapBuffer(int32_t minSizeFrames,
                                      struct audio_mmap_buffer_info *info) = 0;

    // Get current read/write position in the mmap buffer
    virtual status_t getMmapPosition(struct audio_mmap_position *position) = 0;

    // Set the priority of the thread that interacts with the HAL
    // (must match the priority of the audioflinger's thread that calls 'read' / 'write')
    virtual status_t setHalThreadPriority(int priority) = 0;

  protected:
    // Subclasses can not be constructed directly by clients.
    StreamHalInterface() {}

    // The destructor automatically closes the stream.
    virtual ~StreamHalInterface() {}
};

class StreamOutHalInterfaceCallback : public virtual RefBase {
  public:
    virtual void onWriteReady() {}
    virtual void onDrainReady() {}
    virtual void onError() {}

  protected:
    StreamOutHalInterfaceCallback() {}
    virtual ~StreamOutHalInterfaceCallback() {}
};

class StreamOutHalInterface : public virtual StreamHalInterface {
  public:
    // Return the audio hardware driver estimated latency in milliseconds.
    virtual status_t getLatency(uint32_t *latency) = 0;

    // Use this method in situations where audio mixing is done in the hardware.
    virtual status_t setVolume(float left, float right) = 0;

    // Selects the audio presentation (if available).
    virtual status_t selectPresentation(int presentationId, int programId) = 0;

    // Write audio buffer to driver.
    virtual status_t write(const void *buffer, size_t bytes, size_t *written) = 0;

    // Return the number of audio frames written by the audio dsp to DAC since
    // the output has exited standby.
    virtual status_t getRenderPosition(uint32_t *dspFrames) = 0;

    // Get the local time at which the next write to the audio driver will be presented.
    virtual status_t getNextWriteTimestamp(int64_t *timestamp) = 0;

    // Set the callback for notifying completion of non-blocking write and drain.
    // The callback must be owned by someone else. The output stream does not own it
    // to avoid strong pointer loops.
    virtual status_t setCallback(wp<StreamOutHalInterfaceCallback> callback) = 0;

    // Returns whether pause and resume operations are supported.
    virtual status_t supportsPauseAndResume(bool *supportsPause, bool *supportsResume) = 0;

    // Notifies to the audio driver to resume playback following a pause.
    virtual status_t pause() = 0;

    // Notifies to the audio driver to resume playback following a pause.
    virtual status_t resume() = 0;

    // Returns whether drain operation is supported.
    virtual status_t supportsDrain(bool *supportsDrain) = 0;

    // Requests notification when data buffered by the driver/hardware has been played.
    virtual status_t drain(bool earlyNotify) = 0;

    // Notifies to the audio driver to flush the queued data.
    virtual status_t flush() = 0;

    // Return a recent count of the number of audio frames presented to an external observer.
    virtual status_t getPresentationPosition(uint64_t *frames, struct timespec *timestamp) = 0;

    struct SourceMetadata {
        std::vector<playback_track_metadata_t> tracks;
    };
    /**
     * Called when the metadata of the stream's source has been changed.
     * @param sourceMetadata Description of the audio that is played by the clients.
     */
    virtual status_t updateSourceMetadata(const SourceMetadata& sourceMetadata) = 0;

  protected:
    virtual ~StreamOutHalInterface() {}
};

class StreamInHalInterface : public virtual StreamHalInterface {
  public:
    // Set the input gain for the audio driver.
    virtual status_t setGain(float gain) = 0;

    // Read audio buffer in from driver.
    virtual status_t read(void *buffer, size_t bytes, size_t *read) = 0;

    // Return the amount of input frames lost in the audio driver.
    virtual status_t getInputFramesLost(uint32_t *framesLost) = 0;

    // Return a recent count of the number of audio frames received and
    // the clock time associated with that frame count.
    virtual status_t getCapturePosition(int64_t *frames, int64_t *time) = 0;

    // Get active microphones
    virtual status_t getActiveMicrophones(std::vector<media::MicrophoneInfo> *microphones) = 0;

    // Set direction for capture processing
    virtual status_t setPreferredMicrophoneDirection(audio_microphone_direction_t) = 0;

    // Set zoom factor for capture stream
    virtual status_t setPreferredMicrophoneFieldDimension(float zoom) = 0;

    struct SinkMetadata {
        std::vector<record_track_metadata_t> tracks;
    };
    /**
     * Called when the metadata of the stream's sink has been changed.
     * @param sinkMetadata Description of the audio that is suggested by the clients.
     */
    virtual status_t updateSinkMetadata(const SinkMetadata& sinkMetadata) = 0;

  protected:
    virtual ~StreamInHalInterface() {}
};

} // namespace android

#endif // ANDROID_HARDWARE_STREAM_HAL_INTERFACE_H
