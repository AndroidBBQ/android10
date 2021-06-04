/*
**
** Copyright 2007, The Android Open Source Project
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

#ifndef ANDROID_AUDIO_HW_DEVICE_H
#define ANDROID_AUDIO_HW_DEVICE_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <media/audiohal/DeviceHalInterface.h>
#include <utils/Errors.h>
#include <system/audio.h>

namespace android {

class AudioStreamOut;

class AudioHwDevice {
public:
    enum Flags {
        AHWD_CAN_SET_MASTER_VOLUME  = 0x1,
        AHWD_CAN_SET_MASTER_MUTE    = 0x2,
        // Means that this isn't a terminal module, and software patches
        // are used to transport audio data further.
        AHWD_IS_INSERT              = 0x4,
    };

    AudioHwDevice(audio_module_handle_t handle,
                  const char *moduleName,
                  sp<DeviceHalInterface> hwDevice,
                  Flags flags)
        : mHandle(handle)
        , mModuleName(strdup(moduleName))
        , mHwDevice(hwDevice)
        , mFlags(flags) { }
    virtual ~AudioHwDevice() { free((void *)mModuleName); }

    bool canSetMasterVolume() const {
        return (0 != (mFlags & AHWD_CAN_SET_MASTER_VOLUME));
    }

    bool canSetMasterMute() const {
        return (0 != (mFlags & AHWD_CAN_SET_MASTER_MUTE));
    }

    bool isInsert() const {
        return (0 != (mFlags & AHWD_IS_INSERT));
    }

    audio_module_handle_t handle() const { return mHandle; }
    const char *moduleName() const { return mModuleName; }
    sp<DeviceHalInterface> hwDevice() const { return mHwDevice; }

    /** This method creates and opens the audio hardware output stream.
     * The "address" parameter qualifies the "devices" audio device type if needed.
     * The format format depends on the device type:
     * - Bluetooth devices use the MAC address of the device in the form "00:11:22:AA:BB:CC"
     * - USB devices use the ALSA card and device numbers in the form  "card=X;device=Y"
     * - Other devices may use a number or any other string.
     */
    status_t openOutputStream(
            AudioStreamOut **ppStreamOut,
            audio_io_handle_t handle,
            audio_devices_t devices,
            audio_output_flags_t flags,
            struct audio_config *config,
            const char *address);

    bool supportsAudioPatches() const;

private:
    const audio_module_handle_t mHandle;
    const char * const          mModuleName;
    sp<DeviceHalInterface>      mHwDevice;
    const Flags                 mFlags;
};

} // namespace android

#endif // ANDROID_AUDIO_HW_DEVICE_H
