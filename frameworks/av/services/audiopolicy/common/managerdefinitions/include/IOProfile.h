/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#include "AudioPort.h"
#include "DeviceDescriptor.h"
#include <utils/String8.h>
#include <system/audio.h>

namespace android {

class HwModule;

// the IOProfile class describes the capabilities of an output or input stream.
// It is currently assumed that all combination of listed parameters are supported.
// It is used by the policy manager to determine if an output or input is suitable for
// a given use case,  open/close it accordingly and connect/disconnect audio tracks
// to/from it.
class IOProfile : public AudioPort
{
public:
    IOProfile(const String8 &name, audio_port_role_t role)
        : AudioPort(name, AUDIO_PORT_TYPE_MIX, role),
          maxOpenCount(1),
          curOpenCount(0),
          maxActiveCount(1),
          curActiveCount(0) {}

    // For a Profile aka MixPort, tag name and name are equivalent.
    virtual const String8 getTagName() const { return getName(); }

    // FIXME: this is needed because shared MMAP stream clients use the same audio session.
    // Once capture clients are tracked individually and not per session this can be removed
    // MMAP no IRQ input streams do not have the default limitation of one active client
    // max as they can be used in shared mode by the same application.
    // NOTE: this works for explicit values set in audio_policy_configuration.xml because
    // flags are parsed before maxActiveCount by the serializer.
    void setFlags(uint32_t flags) override
    {
        AudioPort::setFlags(flags);
        if (getRole() == AUDIO_PORT_ROLE_SINK && (flags & AUDIO_INPUT_FLAG_MMAP_NOIRQ) != 0) {
            maxActiveCount = 0;
        }
    }

    /**
     * @brief isCompatibleProfile: This method is used for input and direct output,
     * and is not used for other output.
     * Checks if the IO profile is compatible with specified parameters.
     * For input, flags is interpreted as audio_input_flags_t.
     * TODO: merge audio_output_flags_t and audio_input_flags_t.
     *
     * @param devices vector of devices to be checked for compatibility
     * @param samplingRate to be checked for compatibility. Must be specified
     * @param updatedSamplingRate if non-NULL, it is assigned the actual sample rate.
     * @param format to be checked for compatibility. Must be specified
     * @param updatedFormat if non-NULL, it is assigned the actual format
     * @param channelMask to be checked for compatibility. Must be specified
     * @param updatedChannelMask if non-NULL, it is assigned the actual channel mask
     * @param flags to be checked for compatibility
     * @param exactMatchRequiredForInputFlags true if exact match is required on flags
     * @return true if the profile is compatible, false otherwise.
     */
    bool isCompatibleProfile(const DeviceVector &devices,
                             uint32_t samplingRate,
                             uint32_t *updatedSamplingRate,
                             audio_format_t format,
                             audio_format_t *updatedFormat,
                             audio_channel_mask_t channelMask,
                             audio_channel_mask_t *updatedChannelMask,
                             // FIXME parameter type
                             uint32_t flags,
                             bool exactMatchRequiredForInputFlags = false) const;

    void dump(String8 *dst) const;
    void log();

    bool hasSupportedDevices() const { return !mSupportedDevices.isEmpty(); }

    bool supportsDeviceTypes(audio_devices_t device) const
    {
        if (audio_is_output_devices(device)) {
            if (deviceSupportsEncodedFormats(device)) {
                return mSupportedDevices.types() & device;
            }
            return false;
        }
        return mSupportedDevices.types() & (device & ~AUDIO_DEVICE_BIT_IN);
    }

    /**
     * @brief supportsDevice
     * @param device to be checked against
     *        forceCheckOnAddress if true, check on type and address whatever the type, otherwise
     *        the address enforcement is limited to "offical devices" that distinguishe on address
     * @return true if the device is supported by type (for non bus / remote submix devices),
     *         true if the device is supported (both type and address) for bus / remote submix
     *         false otherwise
     */
    bool supportsDevice(const sp<DeviceDescriptor> &device, bool forceCheckOnAddress = false) const
    {
        if (!device_distinguishes_on_address(device->type()) && !forceCheckOnAddress) {
            return supportsDeviceTypes(device->type());
        }
        return mSupportedDevices.contains(device);
    }

    bool deviceSupportsEncodedFormats(audio_devices_t device) const
    {
        if (device == AUDIO_DEVICE_NONE) {
            return true; // required for isOffloadSupported() check
        }
        DeviceVector deviceList =
            mSupportedDevices.getDevicesFromTypeMask(device);
        if (!deviceList.empty()) {
            return deviceList.itemAt(0)->hasCurrentEncodedFormat();
        }
        return false;
    }

    void clearSupportedDevices() { mSupportedDevices.clear(); }
    void addSupportedDevice(const sp<DeviceDescriptor> &device)
    {
        mSupportedDevices.add(device);
    }
    void removeSupportedDevice(const sp<DeviceDescriptor> &device)
    {
        mSupportedDevices.remove(device);
    }
    void setSupportedDevices(const DeviceVector &devices)
    {
        mSupportedDevices = devices;
    }

    const DeviceVector &getSupportedDevices() const { return mSupportedDevices; }

    bool canOpenNewIo() {
        if (maxOpenCount == 0 || curOpenCount < maxOpenCount) {
            return true;
        }
        return false;
    }

    bool canStartNewIo() {
        if (maxActiveCount == 0 || curActiveCount < maxActiveCount) {
            return true;
        }
        return false;
    }

    // Maximum number of input or output streams that can be simultaneously opened for this profile.
    // By convention 0 means no limit. To respect legacy behavior, initialized to 1 for output
    // profiles and 0 for input profiles
    uint32_t     maxOpenCount;
    // Number of streams currently opened for this profile.
    uint32_t     curOpenCount;
    // Maximum number of input or output streams that can be simultaneously active for this profile.
    // By convention 0 means no limit. To respect legacy behavior, initialized to 0 for output
    // profiles and 1 for input profiles
    uint32_t     maxActiveCount;
    // Number of streams currently active for this profile. This is not the number of active clients
    // (AudioTrack or AudioRecord) but the number of active HAL streams.
    uint32_t     curActiveCount;

private:
    DeviceVector mSupportedDevices; // supported devices: this input/output can be routed from/to
};

class InputProfile : public IOProfile
{
public:
    explicit InputProfile(const String8 &name) : IOProfile(name, AUDIO_PORT_ROLE_SINK) {}
};

class OutputProfile : public IOProfile
{
public:
    explicit OutputProfile(const String8 &name) : IOProfile(name, AUDIO_PORT_ROLE_SOURCE) {}
};

} // namespace android
