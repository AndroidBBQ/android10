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

#ifndef ANDROID_HARDWARE_DEVICE_HAL_INTERFACE_H
#define ANDROID_HARDWARE_DEVICE_HAL_INTERFACE_H

#include <media/MicrophoneInfo.h>
#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

namespace android {

class StreamInHalInterface;
class StreamOutHalInterface;

class DeviceHalInterface : public RefBase
{
  public:
    // Sets the value of 'devices' to a bitmask of 1 or more values of audio_devices_t.
    virtual status_t getSupportedDevices(uint32_t *devices) = 0;

    // Check to see if the audio hardware interface has been initialized.
    virtual status_t initCheck() = 0;

    // Set the audio volume of a voice call. Range is between 0.0 and 1.0.
    virtual status_t setVoiceVolume(float volume) = 0;

    // Set the audio volume for all audio activities other than voice call.
    virtual status_t setMasterVolume(float volume) = 0;

    // Get the current master volume value for the HAL.
    virtual status_t getMasterVolume(float *volume) = 0;

    // Called when the audio mode changes.
    virtual status_t setMode(audio_mode_t mode) = 0;

    // Muting control.
    virtual status_t setMicMute(bool state) = 0;
    virtual status_t getMicMute(bool *state) = 0;
    virtual status_t setMasterMute(bool state) = 0;
    virtual status_t getMasterMute(bool *state) = 0;

    // Set global audio parameters.
    virtual status_t setParameters(const String8& kvPairs) = 0;

    // Get global audio parameters.
    virtual status_t getParameters(const String8& keys, String8 *values) = 0;

    // Returns audio input buffer size according to parameters passed.
    virtual status_t getInputBufferSize(const struct audio_config *config,
            size_t *size) = 0;

    // Creates and opens the audio hardware output stream. The stream is closed
    // by releasing all references to the returned object.
    virtual status_t openOutputStream(
            audio_io_handle_t handle,
            audio_devices_t devices,
            audio_output_flags_t flags,
            struct audio_config *config,
            const char *address,
            sp<StreamOutHalInterface> *outStream) = 0;

    // Creates and opens the audio hardware input stream. The stream is closed
    // by releasing all references to the returned object.
    virtual status_t openInputStream(
            audio_io_handle_t handle,
            audio_devices_t devices,
            struct audio_config *config,
            audio_input_flags_t flags,
            const char *address,
            audio_source_t source,
            audio_devices_t outputDevice,
            const char *outputDeviceAddress,
            sp<StreamInHalInterface> *inStream) = 0;

    // Returns whether createAudioPatch and releaseAudioPatch operations are supported.
    virtual status_t supportsAudioPatches(bool *supportsPatches) = 0;

    // Creates an audio patch between several source and sink ports.
    virtual status_t createAudioPatch(
            unsigned int num_sources,
            const struct audio_port_config *sources,
            unsigned int num_sinks,
            const struct audio_port_config *sinks,
            audio_patch_handle_t *patch) = 0;

    // Releases an audio patch.
    virtual status_t releaseAudioPatch(audio_patch_handle_t patch) = 0;

    // Fills the list of supported attributes for a given audio port.
    virtual status_t getAudioPort(struct audio_port *port) = 0;

    // Set audio port configuration.
    virtual status_t setAudioPortConfig(const struct audio_port_config *config) = 0;

    // List microphones
    virtual status_t getMicrophones(std::vector<media::MicrophoneInfo> *microphones) = 0;

    virtual status_t dump(int fd) = 0;

  protected:
    // Subclasses can not be constructed directly by clients.
    DeviceHalInterface() {}

    // The destructor automatically closes the device.
    virtual ~DeviceHalInterface() {}
};

} // namespace android

#endif // ANDROID_HARDWARE_DEVICE_HAL_INTERFACE_H
