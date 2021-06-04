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

#define LOG_TAG "APM::HwModule"
//#define LOG_NDEBUG 0

#include "HwModule.h"
#include "IOProfile.h"
#include "AudioGain.h"
#include <policy.h>
#include <system/audio.h>

namespace android {

HwModule::HwModule(const char *name, uint32_t halVersionMajor, uint32_t halVersionMinor)
    : mName(String8(name)),
      mHandle(AUDIO_MODULE_HANDLE_NONE)
{
    setHalVersion(halVersionMajor, halVersionMinor);
}

HwModule::~HwModule()
{
    for (size_t i = 0; i < mOutputProfiles.size(); i++) {
        mOutputProfiles[i]->clearSupportedDevices();
    }
    for (size_t i = 0; i < mInputProfiles.size(); i++) {
        mInputProfiles[i]->clearSupportedDevices();
    }
}

status_t HwModule::addOutputProfile(const String8& name, const audio_config_t *config,
                                    audio_devices_t device, const String8& address)
{
    sp<IOProfile> profile = new OutputProfile(name);

    profile->addAudioProfile(new AudioProfile(config->format, config->channel_mask,
                                              config->sample_rate));

    sp<DeviceDescriptor> devDesc = new DeviceDescriptor(device);
    devDesc->setAddress(address);
    addDynamicDevice(devDesc);
    // Reciprocally attach the device to the module
    devDesc->attach(this);
    profile->addSupportedDevice(devDesc);

    return addOutputProfile(profile);
}

status_t HwModule::addOutputProfile(const sp<IOProfile> &profile)
{
    profile->attach(this);
    mOutputProfiles.add(profile);
    mPorts.add(profile);
    return NO_ERROR;
}

status_t HwModule::addInputProfile(const sp<IOProfile> &profile)
{
    profile->attach(this);
    mInputProfiles.add(profile);
    mPorts.add(profile);
    return NO_ERROR;
}

status_t HwModule::addProfile(const sp<IOProfile> &profile)
{
    switch (profile->getRole()) {
    case AUDIO_PORT_ROLE_SOURCE:
        return addOutputProfile(profile);
    case AUDIO_PORT_ROLE_SINK:
        return addInputProfile(profile);
    case AUDIO_PORT_ROLE_NONE:
        return BAD_VALUE;
    }
    return BAD_VALUE;
}

void HwModule::setProfiles(const IOProfileCollection &profiles)
{
    for (size_t i = 0; i < profiles.size(); i++) {
        addProfile(profiles[i]);
    }
}

status_t HwModule::removeOutputProfile(const String8& name)
{
    for (size_t i = 0; i < mOutputProfiles.size(); i++) {
        if (mOutputProfiles[i]->getName() == name) {
            for (const auto &device : mOutputProfiles[i]->getSupportedDevices()) {
                removeDynamicDevice(device);
            }
            mOutputProfiles.removeAt(i);
            break;
        }
    }

    return NO_ERROR;
}

status_t HwModule::addInputProfile(const String8& name, const audio_config_t *config,
                                   audio_devices_t device, const String8& address)
{
    sp<IOProfile> profile = new InputProfile(name);
    profile->addAudioProfile(new AudioProfile(config->format, config->channel_mask,
                                              config->sample_rate));

    sp<DeviceDescriptor> devDesc = new DeviceDescriptor(device);
    devDesc->setAddress(address);
    addDynamicDevice(devDesc);
    // Reciprocally attach the device to the module
    devDesc->attach(this);
    profile->addSupportedDevice(devDesc);

    ALOGV("addInputProfile() name %s rate %d mask 0x%08x",
          name.string(), config->sample_rate, config->channel_mask);

    return addInputProfile(profile);
}

status_t HwModule::removeInputProfile(const String8& name)
{
    for (size_t i = 0; i < mInputProfiles.size(); i++) {
        if (mInputProfiles[i]->getName() == name) {
            for (const auto &device : mInputProfiles[i]->getSupportedDevices()) {
                removeDynamicDevice(device);
            }
            mInputProfiles.removeAt(i);
            break;
        }
    }

    return NO_ERROR;
}

void HwModule::setDeclaredDevices(const DeviceVector &devices)
{
    mDeclaredDevices = devices;
    for (size_t i = 0; i < devices.size(); i++) {
        mPorts.add(devices[i]);
    }
}

sp<DeviceDescriptor> HwModule::getRouteSinkDevice(const sp<AudioRoute> &route) const
{
    sp<DeviceDescriptor> sinkDevice = 0;
    if (route->getSink()->getType() == AUDIO_PORT_TYPE_DEVICE) {
        sinkDevice = mDeclaredDevices.getDeviceFromTagName(route->getSink()->getTagName());
    }
    return sinkDevice;
}

DeviceVector HwModule::getRouteSourceDevices(const sp<AudioRoute> &route) const
{
    DeviceVector sourceDevices;
    for (const auto& source : route->getSources()) {
        if (source->getType() == AUDIO_PORT_TYPE_DEVICE) {
            sourceDevices.add(mDeclaredDevices.getDeviceFromTagName(source->getTagName()));
        }
    }
    return sourceDevices;
}

void HwModule::setRoutes(const AudioRouteVector &routes)
{
    mRoutes = routes;
    // Now updating the streams (aka IOProfile until now) supported devices
    refreshSupportedDevices();
}

void HwModule::refreshSupportedDevices()
{
    // Now updating the streams (aka IOProfile until now) supported devices
    for (const auto& stream : mInputProfiles) {
        DeviceVector sourceDevices;
        for (const auto& route : stream->getRoutes()) {
            sp<AudioPort> sink = route->getSink();
            if (sink == 0 || stream != sink) {
                ALOGE("%s: Invalid route attached to input stream", __FUNCTION__);
                continue;
            }
            DeviceVector sourceDevicesForRoute = getRouteSourceDevices(route);
            if (sourceDevicesForRoute.isEmpty()) {
                ALOGE("%s: invalid source devices for %s", __FUNCTION__, stream->getName().string());
                continue;
            }
            sourceDevices.add(sourceDevicesForRoute);
        }
        if (sourceDevices.isEmpty()) {
            ALOGE("%s: invalid source devices for %s", __FUNCTION__, stream->getName().string());
            continue;
        }
        stream->setSupportedDevices(sourceDevices);
    }
    for (const auto& stream : mOutputProfiles) {
        DeviceVector sinkDevices;
        for (const auto& route : stream->getRoutes()) {
            sp<AudioPort> source = route->getSources().findByTagName(stream->getTagName());
            if (source == 0 || stream != source) {
                ALOGE("%s: Invalid route attached to output stream", __FUNCTION__);
                continue;
            }
            sp<DeviceDescriptor> sinkDevice = getRouteSinkDevice(route);
            if (sinkDevice == 0) {
                ALOGE("%s: invalid sink device for %s", __FUNCTION__, stream->getName().string());
                continue;
            }
            sinkDevices.add(sinkDevice);
        }
        stream->setSupportedDevices(sinkDevices);
    }
}

void HwModule::setHandle(audio_module_handle_t handle) {
    ALOGW_IF(mHandle != AUDIO_MODULE_HANDLE_NONE,
            "HwModule handle is changing from %d to %d", mHandle, handle);
    mHandle = handle;
}

bool HwModule::supportsPatch(const sp<AudioPort> &srcPort, const sp<AudioPort> &dstPort) const {
    for (const auto &route : mRoutes) {
        if (route->supportsPatch(srcPort, dstPort)) {
            return true;
        }
    }
    return false;
}

void HwModule::dump(String8 *dst) const
{
    dst->appendFormat("  - name: %s\n", getName());
    dst->appendFormat("  - handle: %d\n", mHandle);
    dst->appendFormat("  - version: %u.%u\n", getHalVersionMajor(), getHalVersionMinor());
    if (mOutputProfiles.size()) {
        dst->append("  - outputs:\n");
        for (size_t i = 0; i < mOutputProfiles.size(); i++) {
            dst->appendFormat("    output %zu:\n", i);
            mOutputProfiles[i]->dump(dst);
        }
    }
    if (mInputProfiles.size()) {
        dst->append("  - inputs:\n");
        for (size_t i = 0; i < mInputProfiles.size(); i++) {
            dst->appendFormat("    input %zu:\n", i);
            mInputProfiles[i]->dump(dst);
        }
    }
    mDeclaredDevices.dump(dst, String8("Declared"), 2, true);
    mDynamicDevices.dump(dst, String8("Dynamic"),  2, true);
    mRoutes.dump(dst, 2);
}

sp <HwModule> HwModuleCollection::getModuleFromName(const char *name) const
{
    for (const auto& module : *this) {
        if (strcmp(module->getName(), name) == 0) {
            return module;
        }
    }
    return nullptr;
}

sp <HwModule> HwModuleCollection::getModuleForDeviceTypes(audio_devices_t type,
                                                          audio_format_t encodedFormat) const
{
    for (const auto& module : *this) {
        const auto& profiles = audio_is_output_device(type) ?
                module->getOutputProfiles() : module->getInputProfiles();
        for (const auto& profile : profiles) {
            if (profile->supportsDeviceTypes(type)) {
                if (encodedFormat != AUDIO_FORMAT_DEFAULT) {
                    DeviceVector declaredDevices = module->getDeclaredDevices();
                    sp <DeviceDescriptor> deviceDesc =
                            declaredDevices.getDevice(type, String8(), encodedFormat);
                    if (deviceDesc) {
                        return module;
                    }
                } else {
                    return module;
                }
            }
        }
    }
    return nullptr;
}

sp<HwModule> HwModuleCollection::getModuleForDevice(const sp<DeviceDescriptor> &device,
                                                     audio_format_t encodedFormat) const
{
    return getModuleForDeviceTypes(device->type(), encodedFormat);
}

DeviceVector HwModuleCollection::getAvailableDevicesFromModuleName(
        const char *name, const DeviceVector &availableDevices) const
{
    sp<HwModule> module = getModuleFromName(name);
    if (module == nullptr) {
        return DeviceVector();
    }
    return availableDevices.getDevicesFromHwModule(module->getHandle());
}

sp<DeviceDescriptor> HwModuleCollection::getDeviceDescriptor(const audio_devices_t deviceType,
                                                             const char *address,
                                                             const char *name,
                                                             const audio_format_t encodedFormat,
                                                             bool allowToCreate,
                                                             bool matchAddress) const
{
    String8 devAddress = (address == nullptr || !matchAddress) ? String8("") : String8(address);
    // handle legacy remote submix case where the address was not always specified
    if (device_distinguishes_on_address(deviceType) && (devAddress.length() == 0)) {
        devAddress = String8("0");
    }

    for (const auto& hwModule : *this) {
        DeviceVector moduleDevices = hwModule->getAllDevices();
        auto moduleDevice = moduleDevices.getDevice(deviceType, devAddress, encodedFormat);
        if (moduleDevice) {
            if (encodedFormat != AUDIO_FORMAT_DEFAULT) {
                moduleDevice->setEncodedFormat(encodedFormat);
            }
            if (allowToCreate) {
                moduleDevice->attach(hwModule);
                moduleDevice->setAddress(devAddress);
                moduleDevice->setName(String8(name));
            }
            return moduleDevice;
        }
    }
    if (!allowToCreate) {
        ALOGV("%s: could not find HW module for device %s %04x address %s", __FUNCTION__,
              name, deviceType, address);
        return nullptr;
    }
    return createDevice(deviceType, address, name, encodedFormat);
}

sp<DeviceDescriptor> HwModuleCollection::createDevice(const audio_devices_t type,
                                                      const char *address,
                                                      const char *name,
                                                      const audio_format_t encodedFormat) const
{
    sp<HwModule> hwModule = getModuleForDeviceTypes(type, encodedFormat);
    if (hwModule == 0) {
        ALOGE("%s: could not find HW module for device %04x address %s", __FUNCTION__, type,
              address);
        return nullptr;
    }
    sp<DeviceDescriptor> device = new DeviceDescriptor(type, String8(name));
    device->setName(String8(name));
    device->setAddress(String8(address));
    device->setEncodedFormat(encodedFormat);

  // Add the device to the list of dynamic devices
    hwModule->addDynamicDevice(device);
    // Reciprocally attach the device to the module
    device->attach(hwModule);
    ALOGD("%s: adding dynamic device %s to module %s", __FUNCTION__,
          device->toString().c_str(), hwModule->getName());

    const auto &profiles = (audio_is_output_device(type) ? hwModule->getOutputProfiles() :
                                                             hwModule->getInputProfiles());
    for (const auto &profile : profiles) {
        // Add the device as supported to all profile supporting "weakly" or not the device
        // according to its type
        if (profile->supportsDevice(device, false /*matchAdress*/)) {

            // @todo quid of audio profile? import the profile from device of the same type?
            const auto &isoTypeDeviceForProfile =
                profile->getSupportedDevices().getDevice(type, String8(), AUDIO_FORMAT_DEFAULT);
            device->importAudioPort(isoTypeDeviceForProfile, true /* force */);

            ALOGV("%s: adding device %s to profile %s", __FUNCTION__,
                  device->toString().c_str(), profile->getTagName().c_str());
            profile->addSupportedDevice(device);
        }
    }
    return device;
}

void HwModuleCollection::cleanUpForDevice(const sp<DeviceDescriptor> &device)
{
    for (const auto& hwModule : *this) {
        DeviceVector moduleDevices = hwModule->getAllDevices();
        if (!moduleDevices.contains(device)) {
            continue;
        }

        // removal of remote submix devices associated with a dynamic policy is
        // handled by removeOutputProfile() and removeInputProfile()
        if (audio_is_remote_submix_device(device->type()) && device->address() != "0") {
            continue;
        }

        device->detach();
        // Only remove from dynamic list, not from declared list!!!
        if (!hwModule->getDynamicDevices().contains(device)) {
            return;
        }
        hwModule->removeDynamicDevice(device);
        ALOGV("%s: removed dynamic device %s from module %s", __FUNCTION__,
              device->toString().c_str(), hwModule->getName());

        const IOProfileCollection &profiles = audio_is_output_device(device->type()) ?
                    hwModule->getOutputProfiles() : hwModule->getInputProfiles();
        for (const auto &profile : profiles) {
            // For cleanup, strong match is required
            if (profile->supportsDevice(device, true /*matchAdress*/)) {
                ALOGV("%s: removing device %s from profile %s", __FUNCTION__,
                      device->toString().c_str(), profile->getTagName().c_str());
                profile->removeSupportedDevice(device);
            }
        }
    }
}

void HwModuleCollection::dump(String8 *dst) const
{
    dst->append("\nHW Modules dump:\n");
    for (size_t i = 0; i < size(); i++) {
        dst->appendFormat("- HW Module %zu:\n", i + 1);
        itemAt(i)->dump(dst);
    }
}


} //namespace android
