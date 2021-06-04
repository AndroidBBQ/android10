/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <EngineConfig.h>
#include <AudioPolicyManagerInterface.h>
#include <ProductStrategy.h>
#include <VolumeGroup.h>

namespace android {
namespace audio_policy {

class EngineBase : public AudioPolicyManagerInterface
{
public:
    ///
    /// from AudioPolicyManagerInterface
    ///
    android::status_t initCheck() override;

    void setObserver(AudioPolicyManagerObserver *observer) override;

    status_t setPhoneState(audio_mode_t mode) override;

    audio_mode_t getPhoneState() const override { return mPhoneState; }

    status_t setForceUse(audio_policy_force_use_t usage, audio_policy_forced_cfg_t config) override
    {
        mForceUse[usage] = config;
        return NO_ERROR;
    }

    audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage) const override
    {
        return mForceUse[usage];
    }
    android::status_t setDeviceConnectionState(const sp<DeviceDescriptor> /*devDesc*/,
                                               audio_policy_dev_state_t /*state*/) override
    {
        return NO_ERROR;
    }
    product_strategy_t getProductStrategyForAttributes(
            const audio_attributes_t &attr) const override;

    audio_stream_type_t getStreamTypeForAttributes(const audio_attributes_t &attr) const override;

    audio_attributes_t getAttributesForStreamType(audio_stream_type_t stream) const override;

    StreamTypeVector getStreamTypesForProductStrategy(product_strategy_t ps) const override;

    AttributesVector getAllAttributesForProductStrategy(product_strategy_t ps) const override;

    StrategyVector getOrderedProductStrategies() const override;

    status_t listAudioProductStrategies(AudioProductStrategyVector &strategies) const override;

    VolumeCurves *getVolumeCurvesForAttributes(const audio_attributes_t &attr) const override;

    VolumeCurves *getVolumeCurvesForStreamType(audio_stream_type_t stream) const override;

    IVolumeCurves *getVolumeCurvesForVolumeGroup(volume_group_t group) const override
    {
       return mVolumeGroups.find(group) != end(mVolumeGroups) ?
                   mVolumeGroups.at(group)->getVolumeCurves() : nullptr;
    }

    VolumeGroupVector getVolumeGroups() const override;

    volume_group_t getVolumeGroupForAttributes(const audio_attributes_t &attr) const override;

    volume_group_t getVolumeGroupForStreamType(audio_stream_type_t stream) const override;

    status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups) const override;

    void dump(String8 *dst) const override;


    engineConfig::ParsingResult loadAudioPolicyEngineConfig();

    const ProductStrategyMap &getProductStrategies() const { return mProductStrategies; }

    ProductStrategyMap &getProductStrategies() { return mProductStrategies; }

    product_strategy_t getProductStrategyForStream(audio_stream_type_t stream) const;

    product_strategy_t getProductStrategyByName(const std::string &name) const;

    AudioPolicyManagerObserver *getApmObserver() const { return mApmObserver; }

    inline bool isInCall() const
    {
        return is_state_in_call(getPhoneState());
    }

    VolumeSource toVolumeSource(audio_stream_type_t stream) const
    {
        return static_cast<VolumeSource>(getVolumeGroupForStreamType(stream));
    }

    status_t switchVolumeCurve(audio_stream_type_t streamSrc, audio_stream_type_t streamDst);

    status_t restoreOriginVolumeCurve(audio_stream_type_t stream);

 private:
    AudioPolicyManagerObserver *mApmObserver = nullptr;

    ProductStrategyMap mProductStrategies;
    VolumeGroupMap mVolumeGroups;
    audio_mode_t mPhoneState = AUDIO_MODE_NORMAL;  /**< current phone state. */

    /** current forced use configuration. */
    audio_policy_forced_cfg_t mForceUse[AUDIO_POLICY_FORCE_USE_CNT] = {};
};

} // namespace audio_policy
} // namespace android
