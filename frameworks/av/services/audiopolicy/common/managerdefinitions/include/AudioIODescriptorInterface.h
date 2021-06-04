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

#pragma once

#include "DeviceDescriptor.h"

namespace android {

/**
 * Interface for I/O descriptors to implement so information about their context
 * can be queried and updated.
 */
class AudioIODescriptorInterface
{
public:
    virtual ~AudioIODescriptorInterface() {};

    virtual audio_config_base_t getConfig() const = 0;

    virtual audio_patch_handle_t getPatchHandle() const = 0;

    virtual void setPatchHandle(audio_patch_handle_t handle) = 0;
};

template <class IoDescriptor, class Filter>
sp<DeviceDescriptor> findPreferredDevice(
        IoDescriptor& desc, Filter filter, bool& active, const DeviceVector& devices)
{
    auto activeClients = desc->clientsList(true /*activeOnly*/);
    auto activeClientsWithRoute =
        desc->clientsList(true /*activeOnly*/, filter, true /*preferredDevice*/);
    active = activeClients.size() > 0;
    if (active && activeClients.size() == activeClientsWithRoute.size()) {
        return devices.getDeviceFromId(activeClientsWithRoute[0]->preferredDeviceId());
    }
    return nullptr;
}

template <class IoCollection, class Filter>
sp<DeviceDescriptor> findPreferredDevice(
        IoCollection& ioCollection, Filter filter, const DeviceVector& devices)
{
    sp<DeviceDescriptor> device;
    for (size_t i = 0; i < ioCollection.size(); i++) {
        auto desc = ioCollection.valueAt(i);
        bool active;
        sp<DeviceDescriptor> curDevice = findPreferredDevice(desc, filter, active, devices);
        if (active && curDevice == nullptr) {
            return nullptr;
        } else if (curDevice != nullptr) {
            device = curDevice;
        }
    }
    return device;
}

} // namespace android
