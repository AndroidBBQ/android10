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

#ifndef ANDROID_HARDWARE_DEVICE_HAL_HIDL_H
#define ANDROID_HARDWARE_DEVICE_HAL_HIDL_H

#include PATH(android/hardware/audio/FILE_VERSION/IDevice.h)
#include PATH(android/hardware/audio/FILE_VERSION/IPrimaryDevice.h)
#include <media/audiohal/DeviceHalInterface.h>

#include "ConversionHelperHidl.h"

using ::android::hardware::audio::CPP_VERSION::IDevice;
using ::android::hardware::audio::CPP_VERSION::IPrimaryDevice;
using ::android::hardware::Return;

namespace android {
namespace CPP_VERSION {

class DeviceHalHidl : public DeviceHalInterface, public ConversionHelperHidl
{
  public:
    // Sets the value of 'devices' to a bitmask of 1 or more values of audio_devices_t.
    virtual status_t getSupportedDevices(uint32_t *devices);

    // Check to see if the audio hardware interface has been initialized.
    virtual status_t initCheck();

    // Set the audio volume of a voice call. Range is between 0.0 and 1.0.
    virtual status_t setVoiceVolume(float volume);

    // Set the audio volume for all audio activities other than voice call.
    virtual status_t setMasterVolume(float volume);

    // Get the current master volume value for the HAL.
    virtual status_t getMasterVolume(float *volume);

    // Called when the audio mode changes.
    virtual status_t setMode(audio_mode_t mode);

    // Muting control.
    virtual status_t setMicMute(bool state);
    virtual status_t getMicMute(bool *state);
    virtual status_t setMasterMute(bool state);
    virtual status_t getMasterMute(bool *state);

    // Set global audio parameters.
    virtual status_t setParameters(const String8& kvPairs);

    // Get global audio parameters.
    virtual status_t getParameters(const String8& keys, String8 *values);

    // Returns audio input buffer size according to parameters passed.
    virtual status_t getInputBufferSize(const struct audio_config *config,
            size_t *size);

    // Creates and opens the audio hardware output stream. The stream is closed
    // by releasing all references to the returned object.
    virtual status_t openOutputStream(
            audio_io_handle_t handle,
            audio_devices_t devices,
            audio_output_flags_t flags,
            struct audio_config *config,
            const char *address,
            sp<StreamOutHalInterface> *outStream);

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
            sp<StreamInHalInterface> *inStream);

    // Returns whether createAudioPatch and releaseAudioPatch operations are supported.
    virtual status_t supportsAudioPatches(bool *supportsPatches);

    // Creates an audio patch between several source and sink ports.
    virtual status_t createAudioPatch(
            unsigned int num_sources,
            const struct audio_port_config *sources,
            unsigned int num_sinks,
            const struct audio_port_config *sinks,
            audio_patch_handle_t *patch);

    // Releases an audio patch.
    virtual status_t releaseAudioPatch(audio_patch_handle_t patch);

    // Fills the list of supported attributes for a given audio port.
    virtual status_t getAudioPort(struct audio_port *port);

    // Set audio port configuration.
    virtual status_t setAudioPortConfig(const struct audio_port_config *config);

    // List microphones
    virtual status_t getMicrophones(std::vector<media::MicrophoneInfo> *microphones);

    virtual status_t dump(int fd);

  private:
    friend class DevicesFactoryHalHidl;
    sp<IDevice> mDevice;
    sp<IPrimaryDevice> mPrimaryDevice;  // Null if it's not a primary device.

    // Can not be constructed directly by clients.
    explicit DeviceHalHidl(const sp<IDevice>& device);

    // The destructor automatically closes the device.
    virtual ~DeviceHalHidl();
};

} // namespace CPP_VERSION
} // namespace android

#endif // ANDROID_HARDWARE_DEVICE_HAL_HIDL_H
