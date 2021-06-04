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

#include <AudioPolicyManagerObserver.h>
#include <media/AudioProductStrategy.h>
#include <media/AudioVolumeGroup.h>
#include <IVolumeCurves.h>
#include <policy.h>
#include <Volume.h>
#include <HwModule.h>
#include <DeviceDescriptor.h>
#include <system/audio.h>
#include <system/audio_policy.h>
#include <utils/Errors.h>
#include <utils/Vector.h>

namespace android {

using DeviceStrategyMap = std::map<product_strategy_t, DeviceVector>;
using StrategyVector = std::vector<product_strategy_t>;
using VolumeGroupVector = std::vector<volume_group_t>;

/**
 * This interface is dedicated to the policy manager that a Policy Engine shall implement.
 */
class AudioPolicyManagerInterface
{
public:
    /**
     * Checks if the engine was correctly initialized.
     *
     * @return NO_ERROR if initialization has been done correctly, error code otherwise..
     */
    virtual status_t initCheck() = 0;

    /**
     * Sets the Manager observer that allows the engine to retrieve information on collection
     * of devices, streams, HwModules, ...
     *
     * @param[in] observer handle on the manager.
     */
    virtual void setObserver(AudioPolicyManagerObserver *observer) = 0;

    /**
     * Set the Telephony Mode.
     *
     * @param[in] mode: Android Phone state (normal, ringtone, csv, in communication)
     *
     * @return NO_ERROR if Telephony Mode set correctly, error code otherwise.
     */
    virtual status_t setPhoneState(audio_mode_t mode) = 0;

    /**
     * Get the telephony Mode
     *
     * @return the current telephony mode
     */
    virtual audio_mode_t getPhoneState() const = 0;

    /**
     * Set Force Use config for a given usage.
     *
     * @param[in] usage for which a configuration shall be forced.
     * @param[in] config wished to be forced for the given usage.
     *
     * @return NO_ERROR if the Force Use config was set correctly, error code otherwise (e.g. config
     * not allowed a given usage...)
     */
    virtual status_t setForceUse(audio_policy_force_use_t usage,
                                 audio_policy_forced_cfg_t config) = 0;

    /**
     * Get Force Use config for a given usage.
     *
     * @param[in] usage for which a configuration shall be forced.
     *
     * @return config wished to be forced for the given usage.
     */
    virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage) const = 0;

    /**
     * Set the connection state of device(s).
     *
     * @param[in] devDesc for which the state has changed.
     * @param[in] state of availability of this(these) device(s).
     *
     * @return NO_ERROR if devices criterion updated correctly, error code otherwise.
     */
    virtual status_t setDeviceConnectionState(const android::sp<android::DeviceDescriptor> devDesc,
                                              audio_policy_dev_state_t state) = 0;

    /**
     * Get the strategy selected for a given audio attributes.
     *
     * @param[in] audio attributes to get the selected @product_strategy_t followed by.
     *
     * @return @product_strategy_t to be followed.
     */
    virtual product_strategy_t getProductStrategyForAttributes(
            const audio_attributes_t &attr) const = 0;

    /**
     * @brief getOutputDevicesForAttributes retrieves the devices to be used for given
     * audio attributes.
     * @param attributes of the output requesting Device(s) selection
     * @param preferedDevice valid reference if a prefered device is requested, nullptr otherwise.
     * @param fromCache if true, the device is returned from internal cache,
     *                  otherwise it is determined by current state (device connected,phone state,
     *                  force use, a2dp output...)
     * @return vector of selected device descriptors.
     *         Appropriate device for streams handled by the specified audio attributes according
     *         to current phone state, forced states, connected devices...
     *         if fromCache is true, the device is returned from internal cache,
     *         otherwise it is determined by current state (device connected,phone state, force use,
     *         a2dp output...)
     * This allows to:
     *      1 speed up process when the state is stable (when starting or stopping an output)
     *      2 access to either current device selection (fromCache == true) or
     *      "future" device selection (fromCache == false) when called from a context
     *      where conditions are changing (setDeviceConnectionState(), setPhoneState()...) AND
     *      before manager updates its outputs.
     */
    virtual DeviceVector getOutputDevicesForAttributes(
            const audio_attributes_t &attributes,
            const sp<DeviceDescriptor> &preferedDevice = nullptr,
            bool fromCache = false) const = 0;

    /**
     * @brief getOutputDevicesForStream Legacy function retrieving devices from a stream type.
     * @param stream type of the output requesting Device(s) selection
     * @param fromCache if true, the device is returned from internal cache,
     *                  otherwise it is determined by current state (device connected,phone state,
     *                  force use, a2dp output...)
     * @return appropriate device for streams handled by the specified audio attributes according
     *         to current phone state, forced states, connected devices...
     *         if fromCache is true, the device is returned from internal cache,
     *         otherwise it is determined by current state (device connected,phone state, force use,
     *         a2dp output...)
     * This allows to:
     *      1 speed up process when the state is stable (when starting or stopping an output)
     *      2 access to either current device selection (fromCache == true) or
     *      "future" device selection (fromCache == false) when called from a context
     *      where conditions are changing (setDeviceConnectionState(), setPhoneState()...) AND
     *      before manager updates its outputs.
     */
    virtual DeviceVector getOutputDevicesForStream(audio_stream_type_t stream,
                                                   bool fromCache = false) const = 0;

    /**
     * Get the input device selected for given audio attributes.
     *
     * @param[in] attr audio attributes to consider
     * @param[out] mix to be used if a mix has been installed for the given audio attributes.
     * @return selected input device for the audio attributes, may be null if error.
     */
    virtual sp<DeviceDescriptor> getInputDeviceForAttributes(
            const audio_attributes_t &attr, sp<AudioPolicyMix> *mix = nullptr) const = 0;

    /**
     * Get the legacy stream type for a given audio attributes.
     *
     * @param[in] audio attributes to get the associated audio_stream_type_t.
     *
     * @return audio_stream_type_t associated to the attributes.
     */
    virtual audio_stream_type_t getStreamTypeForAttributes(
            const audio_attributes_t &attr) const = 0;

    /**
     * @brief getAttributesForStream get the audio attributes from legacy stream type
     * Attributes returned might only be used to check upon routing decision, not volume decisions.
     * @param stream to consider
     * @return audio attributes matching the legacy stream type
     */
    virtual audio_attributes_t getAttributesForStreamType(audio_stream_type_t stream) const = 0;

    /**
     * @brief getStreamTypesForProductStrategy retrieves the list of legacy stream type following
     * the given product strategy
     * @param ps product strategy to consider
     * @return associated legacy Stream Types vector of the given product strategy
     */
    virtual StreamTypeVector getStreamTypesForProductStrategy(product_strategy_t ps) const = 0;

    /**
     * @brief getAllAttributesForProductStrategy retrieves all the attributes following the given
     * product strategy. Any attributes that "matches" with this one will follow the product
     * strategy.
     * "matching" means the usage shall match if reference attributes has a defined usage, AND
     * content type shall match if reference attributes has a defined content type AND
     * flags shall match if reference attributes has defined flags AND
     * tags shall match if reference attributes has defined tags.
     * @param ps product strategy to consider
     * @return vector of product strategy ids, empty if unknown strategy.
     */
    virtual AttributesVector getAllAttributesForProductStrategy(product_strategy_t ps) const = 0;

    /**
     * @brief getOrderedAudioProductStrategies
     * @return priority ordered product strategies to help the AudioPolicyManager evaluating the
     * device selection per output according to the prioritized strategies.
     */
    virtual StrategyVector getOrderedProductStrategies() const = 0;

    /**
     * @brief updateDeviceSelectionCache. Device selection for AudioAttribute / Streams is cached
     * in the engine in order to speed up process when the audio system is stable.
     * When a device is connected, the android mode is changed, engine is notified and can update
     * the cache.
     * When starting / stopping an output with a stream that can affect notification, the engine
     * needs to update the cache upon this function call.
     */
    virtual void updateDeviceSelectionCache() = 0;

    /**
     * @brief listAudioProductStrategies. Introspection API to retrieve a collection of
     * AudioProductStrategyVector that allows to build AudioAttributes according to a
     * product_strategy which is just an index. It has also a human readable name to help the
     * Car/Oem/AudioManager identiying the use case.
     * @param strategies collection.
     * @return OK if the list has been retrieved, error code otherwise
     */
    virtual status_t listAudioProductStrategies(AudioProductStrategyVector &strategies) const = 0;

    /**
     * @brief getVolumeCurvesForAttributes retrieves the Volume Curves interface for the
     *        requested Audio Attributes.
     * @param attr to be considered
     * @return IVolumeCurves interface pointer if found, nullptr otherwise
     */
    virtual IVolumeCurves *getVolumeCurvesForAttributes(const audio_attributes_t &attr) const = 0;

    /**
     * @brief getVolumeCurvesForStreamType retrieves the Volume Curves interface for the stream
     * @param stream to be considered
     * @return IVolumeCurves interface pointer if found, nullptr otherwise
     */
    virtual IVolumeCurves *getVolumeCurvesForStreamType(audio_stream_type_t stream) const = 0;

    /**
     * @brief getVolumeCurvesForVolumeGroup retrieves the Volume Curves interface for volume group
     * @param group to be considered
     * @return IVolumeCurves interface pointer if found, nullptr otherwise
     */
    virtual IVolumeCurves *getVolumeCurvesForVolumeGroup(volume_group_t group) const = 0;

    /**
     * @brief getVolumeGroups retrieves the collection of volume groups.
     * @return vector of volume groups
     */
    virtual VolumeGroupVector getVolumeGroups() const = 0;

    /**
     * @brief getVolumeGroupForAttributes gets the appropriate volume group to be used for a given
     * Audio Attributes.
     * @param attr to be considered
     * @return volume group associated to the given audio attributes, default group if none
     * applicable, VOLUME_GROUP_NONE if no default group defined.
     */
    virtual volume_group_t getVolumeGroupForAttributes(const audio_attributes_t &attr) const = 0;

    /**
     * @brief getVolumeGroupForStreamType gets the appropriate volume group to be used for a given
     * legacy stream type
     * @param stream type to be considered
     * @return volume group associated to the given stream type, default group if none applicable,
     * VOLUME_GROUP_NONE if no default group defined.
     */
    virtual volume_group_t getVolumeGroupForStreamType(audio_stream_type_t stream) const = 0;

    /**
     * @brief listAudioVolumeGroups introspection API to get the Audio Volume Groups, aka
     * former stream aliases in Audio Service, defining volume curves attached to one or more
     * Audio Attributes.
     * @param groups
     * @return NO_ERROR if the volume groups were retrieved successfully, error code otherwise
     */
    virtual status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups) const = 0;

    virtual void dump(String8 *dst) const = 0;

protected:
    virtual ~AudioPolicyManagerInterface() {}
};

} // namespace android
