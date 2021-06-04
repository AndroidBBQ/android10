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

#include "DeviceDescriptor.h"
#include "AudioRoute.h"
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <system/audio.h>
#include <cutils/config_utils.h>
#include <string>

namespace android {

class IOProfile;
class InputProfile;
class OutputProfile;

typedef Vector<sp<IOProfile> > InputProfileCollection;
typedef Vector<sp<IOProfile> > OutputProfileCollection;
typedef Vector<sp<IOProfile> > IOProfileCollection;

class HwModule : public RefBase
{
public:
    explicit HwModule(const char *name, uint32_t halVersionMajor = 0, uint32_t halVersionMinor = 0);
    ~HwModule();

    const char *getName() const { return mName.string(); }

    const DeviceVector &getDeclaredDevices() const { return mDeclaredDevices; }
    void setDeclaredDevices(const DeviceVector &devices);
    DeviceVector getAllDevices() const
    {
        DeviceVector devices = mDeclaredDevices;
        devices.merge(mDynamicDevices);
        return devices;
    }
    void addDynamicDevice(const sp<DeviceDescriptor> &device)
    {
        mDynamicDevices.add(device);
    }

    bool removeDynamicDevice(const sp<DeviceDescriptor> &device)
    {
        return mDynamicDevices.remove(device) >= 0;
    }
    DeviceVector getDynamicDevices() const { return mDynamicDevices; }

    const InputProfileCollection &getInputProfiles() const { return mInputProfiles; }
    const OutputProfileCollection &getOutputProfiles() const { return mOutputProfiles; }

    void setProfiles(const IOProfileCollection &profiles);

    void setHalVersion(uint32_t major, uint32_t minor) {
        mHalVersion = (major << 8) | (minor & 0xff);
    }
    uint32_t getHalVersionMajor() const { return mHalVersion >> 8; }
    uint32_t getHalVersionMinor() const { return mHalVersion & 0xff; }

    sp<DeviceDescriptor> getRouteSinkDevice(const sp<AudioRoute> &route) const;
    DeviceVector getRouteSourceDevices(const sp<AudioRoute> &route) const;
    void setRoutes(const AudioRouteVector &routes);

    status_t addOutputProfile(const sp<IOProfile> &profile);
    status_t addInputProfile(const sp<IOProfile> &profile);
    status_t addProfile(const sp<IOProfile> &profile);

    status_t addOutputProfile(const String8& name, const audio_config_t *config,
            audio_devices_t device, const String8& address);
    status_t removeOutputProfile(const String8& name);
    status_t addInputProfile(const String8& name, const audio_config_t *config,
            audio_devices_t device, const String8& address);
    status_t removeInputProfile(const String8& name);

    audio_module_handle_t getHandle() const { return mHandle; }
    void setHandle(audio_module_handle_t handle);

    sp<AudioPort> findPortByTagName(const String8 &tagName) const
    {
        return mPorts.findByTagName(tagName);
    }

    /**
     * @brief supportsPatch checks if an audio patch between 2 ports beloging to this HwModule
     * is supported by a HwModule. The ports and the route shall be declared in the
     * audio_policy_configuration.xml file.
     * @param srcPort (aka the source) to be considered
     * @param dstPort (aka the sink) to be considered
     * @return true if the HwModule supports the connection between the sink and the source,
     * false otherwise
     */
    bool supportsPatch(const sp<AudioPort> &srcPort, const sp<AudioPort> &dstPort) const;

    // TODO remove from here (split serialization)
    void dump(String8 *dst) const;

private:
    void refreshSupportedDevices();

    const String8 mName; // base name of the audio HW module (primary, a2dp ...)
    audio_module_handle_t mHandle;
    OutputProfileCollection mOutputProfiles; // output profiles exposed by this module
    InputProfileCollection mInputProfiles;  // input profiles exposed by this module
    uint32_t mHalVersion; // audio HAL API version
    DeviceVector mDeclaredDevices; // devices declared in audio_policy configuration file.
    DeviceVector mDynamicDevices; /**< devices that can be added/removed at runtime (e.g. rsbumix)*/
    AudioRouteVector mRoutes;
    AudioPortVector mPorts;
};

class HwModuleCollection : public Vector<sp<HwModule> >
{
public:
    sp<HwModule> getModuleFromName(const char *name) const;

    sp<HwModule> getModuleForDeviceTypes(audio_devices_t device,
                                         audio_format_t encodedFormat) const;

    sp<HwModule> getModuleForDevice(const sp<DeviceDescriptor> &device,
                                    audio_format_t encodedFormat) const;

    DeviceVector getAvailableDevicesFromModuleName(const char *name,
                                                   const DeviceVector &availableDevices) const;

    /**
     * @brief getDeviceDescriptor returns a device descriptor associated to the device type and
     * device address (if matchAddress is true).
     * It may loop twice on all modules to check if allowToCreate is true
     *      -first loop will check if the device is found on a module since declared in the list
     * of device port in configuration file
     *      -(allowToCreate is true)second loop will check if the device is weakly supported by one
     * or more profiles on a given module and will add as a supported device for this module.
     *       The device will also be added to the dynamic list of device of this module
     * @param type of the device requested
     * @param address of the device requested
     * @param name of the device that requested
     * @param encodedFormat if not AUDIO_FORMAT_DEFAULT, must match one supported format
     * @param matchAddress true if a strong match is required
     * @param allowToCreate true if allowed to create dynamic device (e.g. hdmi, usb...)
     * @return device descriptor associated to the type (and address if matchAddress is true)
     */
    sp<DeviceDescriptor> getDeviceDescriptor(const audio_devices_t type,
                                             const char *address,
                                             const char *name,
                                             audio_format_t encodedFormat,
                                             bool allowToCreate = false,
                                             bool matchAddress = true) const;

    /**
     * @brief createDevice creates a new device from the type and address given. It checks that
     * according to the device type, a module is supporting this device (weak check).
     * This concerns only dynamic device, aka device with a specific address and not
     * already supported by module/underlying profiles.
     * @param type of the device to be created
     * @param address of the device to be created
     * @param name of the device to be created
     * @return device descriptor if a module is supporting this type, nullptr otherwise.
     */
    sp<DeviceDescriptor> createDevice(const audio_devices_t type,
                                      const char *address,
                                      const char *name,
                                      const audio_format_t encodedFormat) const;

    /**
     * @brief cleanUpForDevice: loop on all profiles of all modules to remove device from
     * the list of supported device. If this device is a dynamic device (aka a device not in the
     * xml file with a runtime address), it is also removed from the module collection of dynamic
     * devices.
     * @param device that has been disconnected
     */
    void cleanUpForDevice(const sp<DeviceDescriptor> &device);

    void dump(String8 *dst) const;
};

} // namespace android
