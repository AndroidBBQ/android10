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

#include "EngineBase.h"
#include "AudioPolicyManagerInterface.h"
#include <AudioGain.h>
#include <policy.h>

namespace android
{

class AudioPolicyManagerObserver;

namespace audio_policy
{

enum legacy_strategy {
    STRATEGY_NONE = -1,
    STRATEGY_MEDIA,
    STRATEGY_PHONE,
    STRATEGY_SONIFICATION,
    STRATEGY_SONIFICATION_RESPECTFUL,
    STRATEGY_DTMF,
    STRATEGY_ENFORCED_AUDIBLE,
    STRATEGY_TRANSMITTED_THROUGH_SPEAKER,
    STRATEGY_ACCESSIBILITY,
    STRATEGY_REROUTING,
};

class Engine : public EngineBase
{
public:
    Engine();
    virtual ~Engine() = default;

    template <class RequestedInterface>
    RequestedInterface *queryInterface();

private:
    ///
    /// from EngineBase, so from AudioPolicyManagerInterface
    ///
    status_t setForceUse(audio_policy_force_use_t usage,
                         audio_policy_forced_cfg_t config) override;

    DeviceVector getOutputDevicesForAttributes(const audio_attributes_t &attr,
                                               const sp<DeviceDescriptor> &preferedDevice = nullptr,
                                               bool fromCache = false) const override;

    DeviceVector getOutputDevicesForStream(audio_stream_type_t stream,
                                           bool fromCache = false) const override;

    sp<DeviceDescriptor> getInputDeviceForAttributes(
            const audio_attributes_t &attr, sp<AudioPolicyMix> *mix = nullptr) const override;

    void updateDeviceSelectionCache() override;

private:
    /* Copy facilities are put private to disable copy. */
    Engine(const Engine &object);
    Engine &operator=(const Engine &object);

    status_t setDefaultDevice(audio_devices_t device);

    audio_devices_t getDeviceForStrategyInt(legacy_strategy strategy,
                                            DeviceVector availableOutputDevices,
                                            DeviceVector availableInputDevices,
                                            const SwAudioOutputCollection &outputs,
                                            uint32_t outputDeviceTypesToIgnore) const;

    DeviceVector getDevicesForProductStrategy(product_strategy_t strategy) const;

    audio_devices_t getDeviceForInputSource(audio_source_t inputSource) const;

    DeviceStrategyMap mDevicesForStrategies;

    std::map<product_strategy_t, legacy_strategy> mLegacyStrategyMap;
};
} // namespace audio_policy
} // namespace android

