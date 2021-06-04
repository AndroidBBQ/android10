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

#ifndef ANDROID_HARDWARE_STREAM_HAL_LOCAL_H
#define ANDROID_HARDWARE_STREAM_HAL_LOCAL_H

#include <media/audiohal/StreamHalInterface.h>
#include "StreamPowerLog.h"

namespace android {
namespace CPP_VERSION {

class DeviceHalLocal;

class StreamHalLocal : public virtual StreamHalInterface
{
  public:
    // Return the sampling rate in Hz - eg. 44100.
    virtual status_t getSampleRate(uint32_t *rate);

    // Return size of input/output buffer in bytes for this stream - eg. 4800.
    virtual status_t getBufferSize(size_t *size);

    // Return the channel mask.
    virtual status_t getChannelMask(audio_channel_mask_t *mask);

    // Return the audio format - e.g. AUDIO_FORMAT_PCM_16_BIT.
    virtual status_t getFormat(audio_format_t *format);

    // Convenience method.
    virtual status_t getAudioProperties(
            uint32_t *sampleRate, audio_channel_mask_t *mask, audio_format_t *format);

    // Set audio stream parameters.
    virtual status_t setParameters(const String8& kvPairs);

    // Get audio stream parameters.
    virtual status_t getParameters(const String8& keys, String8 *values);

    // Add or remove the effect on the stream.
    virtual status_t addEffect(sp<EffectHalInterface> effect);
    virtual status_t removeEffect(sp<EffectHalInterface> effect);

    // Put the audio hardware input/output into standby mode.
    virtual status_t standby();

    virtual status_t dump(int fd);

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
    virtual status_t setHalThreadPriority(int priority);

  protected:
    // Subclasses can not be constructed directly by clients.
    StreamHalLocal(audio_stream_t *stream, sp<DeviceHalLocal> device);

    // The destructor automatically closes the stream.
    virtual ~StreamHalLocal();

    sp<DeviceHalLocal> mDevice;

    // mStreamPowerLog is used for audio signal power logging.
    StreamPowerLog mStreamPowerLog;

  private:
    audio_stream_t *mStream;
};

class StreamOutHalLocal : public StreamOutHalInterface, public StreamHalLocal {
  public:
    // Return the frame size (number of bytes per sample) of a stream.
    virtual status_t getFrameSize(size_t *size);

    // Return the audio hardware driver estimated latency in milliseconds.
    virtual status_t getLatency(uint32_t *latency);

    // Use this method in situations where audio mixing is done in the hardware.
    virtual status_t setVolume(float left, float right);

    // Selects the audio presentation (if available).
    virtual status_t selectPresentation(int presentationId, int programId);

    // Write audio buffer to driver.
    virtual status_t write(const void *buffer, size_t bytes, size_t *written);

    // Return the number of audio frames written by the audio dsp to DAC since
    // the output has exited standby.
    virtual status_t getRenderPosition(uint32_t *dspFrames);

    // Get the local time at which the next write to the audio driver will be presented.
    virtual status_t getNextWriteTimestamp(int64_t *timestamp);

    // Set the callback for notifying completion of non-blocking write and drain.
    virtual status_t setCallback(wp<StreamOutHalInterfaceCallback> callback);

    // Returns whether pause and resume operations are supported.
    virtual status_t supportsPauseAndResume(bool *supportsPause, bool *supportsResume);

    // Notifies to the audio driver to resume playback following a pause.
    virtual status_t pause();

    // Notifies to the audio driver to resume playback following a pause.
    virtual status_t resume();

    // Returns whether drain operation is supported.
    virtual status_t supportsDrain(bool *supportsDrain);

    // Requests notification when data buffered by the driver/hardware has been played.
    virtual status_t drain(bool earlyNotify);

    // Notifies to the audio driver to flush the queued data.
    virtual status_t flush();

    // Return a recent count of the number of audio frames presented to an external observer.
    virtual status_t getPresentationPosition(uint64_t *frames, struct timespec *timestamp);

    // Start a stream operating in mmap mode.
    virtual status_t start();

    // Stop a stream operating in mmap mode.
    virtual status_t stop();

    // Retrieve information on the data buffer in mmap mode.
    virtual status_t createMmapBuffer(int32_t minSizeFrames,
                                      struct audio_mmap_buffer_info *info);

    // Get current read/write position in the mmap buffer
    virtual status_t getMmapPosition(struct audio_mmap_position *position);

    // Called when the metadata of the stream's source has been changed.
    status_t updateSourceMetadata(const SourceMetadata& sourceMetadata) override;

  private:
    audio_stream_out_t *mStream;
    wp<StreamOutHalInterfaceCallback> mCallback;

    friend class DeviceHalLocal;

    // Can not be constructed directly by clients.
    StreamOutHalLocal(audio_stream_out_t *stream, sp<DeviceHalLocal> device);

    virtual ~StreamOutHalLocal();

    static int asyncCallback(stream_callback_event_t event, void *param, void *cookie);
};

class StreamInHalLocal : public StreamInHalInterface, public StreamHalLocal {
  public:
    // Return the frame size (number of bytes per sample) of a stream.
    virtual status_t getFrameSize(size_t *size);

    // Set the input gain for the audio driver.
    virtual status_t setGain(float gain);

    // Read audio buffer in from driver.
    virtual status_t read(void *buffer, size_t bytes, size_t *read);

    // Return the amount of input frames lost in the audio driver.
    virtual status_t getInputFramesLost(uint32_t *framesLost);

    // Return a recent count of the number of audio frames received and
    // the clock time associated with that frame count.
    virtual status_t getCapturePosition(int64_t *frames, int64_t *time);

    // Start a stream operating in mmap mode.
    virtual status_t start();

    // Stop a stream operating in mmap mode.
    virtual status_t stop();

    // Retrieve information on the data buffer in mmap mode.
    virtual status_t createMmapBuffer(int32_t minSizeFrames,
                                      struct audio_mmap_buffer_info *info);

    // Get current read/write position in the mmap buffer
    virtual status_t getMmapPosition(struct audio_mmap_position *position);

    // Get active microphones
    virtual status_t getActiveMicrophones(std::vector<media::MicrophoneInfo> *microphones);

    // Sets microphone direction (for processing)
    virtual status_t setPreferredMicrophoneDirection(audio_microphone_direction_t direction);

    // Sets microphone zoom (for processing)
    virtual status_t setPreferredMicrophoneFieldDimension(float zoom);

    // Called when the metadata of the stream's sink has been changed.
    status_t updateSinkMetadata(const SinkMetadata& sinkMetadata) override;

  private:
    audio_stream_in_t *mStream;

    friend class DeviceHalLocal;

    // Can not be constructed directly by clients.
    StreamInHalLocal(audio_stream_in_t *stream, sp<DeviceHalLocal> device);

    virtual ~StreamInHalLocal();
};

} // namespace CPP_VERSION
} // namespace android

#endif // ANDROID_HARDWARE_STREAM_HAL_LOCAL_H
