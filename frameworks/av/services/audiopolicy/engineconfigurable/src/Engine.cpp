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

#define LOG_TAG "APM::AudioPolicyEngine"
//#define LOG_NDEBUG 0

//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include "Engine.h"
#include "Stream.h"
#include "InputSource.h"

#include <EngineConfig.h>
#include <policy.h>
#include <AudioIODescriptorInterface.h>
#include <ParameterManagerWrapper.h>

using std::string;
using std::map;

namespace android {
namespace audio_policy {

template <>
StreamCollection &Engine::getCollection<audio_stream_type_t>()
{
    return mStreamCollection;
}
template <>
InputSourceCollection &Engine::getCollection<audio_source_t>()
{
    return mInputSourceCollection;
}

template <>
const StreamCollection &Engine::getCollection<audio_stream_type_t>() const
{
    return mStreamCollection;
}
template <>
const InputSourceCollection &Engine::getCollection<audio_source_t>() const
{
    return mInputSourceCollection;
}

Engine::Engine() : mPolicyParameterMgr(new ParameterManagerWrapper())
{
    status_t loadResult = loadAudioPolicyEngineConfig();
    if (loadResult < 0) {
        ALOGE("Policy Engine configuration is invalid.");
    }
}

Engine::~Engine()
{
    mStreamCollection.clear();
    mInputSourceCollection.clear();
}

status_t Engine::initCheck()
{
    if (mPolicyParameterMgr == nullptr || mPolicyParameterMgr->start() != NO_ERROR) {
        ALOGE("%s: could not start Policy PFW", __FUNCTION__);
        return NO_INIT;
    }
    return EngineBase::initCheck();
}

template <typename Key>
Element<Key> *Engine::getFromCollection(const Key &key) const
{
    const Collection<Key> collection = getCollection<Key>();
    return collection.get(key);
}

template <typename Key>
status_t Engine::add(const std::string &name, const Key &key)
{
    Collection<Key> &collection = getCollection<Key>();
    return collection.add(name, key);
}

template <typename Property, typename Key>
Property Engine::getPropertyForKey(Key key) const
{
    Element<Key> *element = getFromCollection<Key>(key);
    if (element == NULL) {
        ALOGE("%s: Element not found within collection", __FUNCTION__);
        return static_cast<Property>(0);
    }
    return element->template get<Property>();
}

bool Engine::setVolumeProfileForStream(const audio_stream_type_t &stream,
                                       const audio_stream_type_t &profile)
{
    if (setPropertyForKey<audio_stream_type_t, audio_stream_type_t>(stream, profile)) {
        switchVolumeCurve(profile, stream);
        return true;
    }
    return false;
}

template <typename Property, typename Key>
bool Engine::setPropertyForKey(const Property &property, const Key &key)
{
    Element<Key> *element = getFromCollection<Key>(key);
    if (element == NULL) {
        ALOGE("%s: Element not found within collection", __FUNCTION__);
        return BAD_VALUE;
    }
    return element->template set<Property>(property) == NO_ERROR;
}

status_t Engine::setPhoneState(audio_mode_t mode)
{
    status_t status = mPolicyParameterMgr->setPhoneState(mode);
    if (status != NO_ERROR) {
        return status;
    }
    return EngineBase::setPhoneState(mode);
}

audio_mode_t Engine::getPhoneState() const
{
    return mPolicyParameterMgr->getPhoneState();
}

status_t Engine::setForceUse(audio_policy_force_use_t usage,
                                      audio_policy_forced_cfg_t config)
{
    status_t status = mPolicyParameterMgr->setForceUse(usage, config);
    if (status != NO_ERROR) {
        return status;
    }
    return EngineBase::setForceUse(usage, config);
}

audio_policy_forced_cfg_t Engine::getForceUse(audio_policy_force_use_t usage) const
{
    return mPolicyParameterMgr->getForceUse(usage);
}

status_t Engine::setDeviceConnectionState(const sp<DeviceDescriptor> devDesc,
                                          audio_policy_dev_state_t state)
{
    mPolicyParameterMgr->setDeviceConnectionState(devDesc, state);

    if (audio_is_output_device(devDesc->type())) {
        return mPolicyParameterMgr->setAvailableOutputDevices(
                    getApmObserver()->getAvailableOutputDevices().types());
    } else if (audio_is_input_device(devDesc->type())) {
        return mPolicyParameterMgr->setAvailableInputDevices(
                    getApmObserver()->getAvailableInputDevices().types());
    }
    return BAD_TYPE;
}

status_t Engine::loadAudioPolicyEngineConfig()
{
    auto result = EngineBase::loadAudioPolicyEngineConfig();

    // Custom XML Parsing
    auto loadCriteria= [this](const auto& configCriteria, const auto& configCriterionTypes) {
        for (auto& criterion : configCriteria) {
            engineConfig::CriterionType criterionType;
            for (auto &configCriterionType : configCriterionTypes) {
                if (configCriterionType.name == criterion.typeName) {
                    criterionType = configCriterionType;
                    break;
                }
            }
            ALOG_ASSERT(not criterionType.name.empty(), "Invalid criterion type for %s",
                        criterion.name.c_str());
            mPolicyParameterMgr->addCriterion(criterion.name, criterionType.isInclusive,
                                              criterionType.valuePairs,
                                              criterion.defaultLiteralValue);
        }
    };

    loadCriteria(result.parsedConfig->criteria, result.parsedConfig->criterionTypes);
    return result.nbSkippedElement == 0? NO_ERROR : BAD_VALUE;
}

DeviceVector Engine::getDevicesForProductStrategy(product_strategy_t ps) const
{
    const auto productStrategies = getProductStrategies();
    if (productStrategies.find(ps) == productStrategies.end()) {
        ALOGE("%s: Trying to get device on invalid strategy %d", __FUNCTION__, ps);
        return {};
    }
    const DeviceVector availableOutputDevices = getApmObserver()->getAvailableOutputDevices();
    const SwAudioOutputCollection &outputs = getApmObserver()->getOutputs();
    uint32_t availableOutputDevicesType = availableOutputDevices.types();

    /** This is the only case handled programmatically because the PFW is unable to know the
     * activity of streams.
     *
     * -While media is playing on a remote device, use the the sonification behavior.
     * Note that we test this usecase before testing if media is playing because
     * the isStreamActive() method only informs about the activity of a stream, not
     * if it's for local playback. Note also that we use the same delay between both tests
     *
     * -When media is not playing anymore, fall back on the sonification behavior
     */
    audio_devices_t devices = AUDIO_DEVICE_NONE;
    if (ps == getProductStrategyForStream(AUDIO_STREAM_NOTIFICATION) &&
            !is_state_in_call(getPhoneState()) &&
            !outputs.isActiveRemotely(toVolumeSource(AUDIO_STREAM_MUSIC),
                                      SONIFICATION_RESPECTFUL_AFTER_MUSIC_DELAY) &&
            outputs.isActive(toVolumeSource(AUDIO_STREAM_MUSIC),
                             SONIFICATION_RESPECTFUL_AFTER_MUSIC_DELAY)) {
        product_strategy_t strategyForMedia =
                getProductStrategyForStream(AUDIO_STREAM_MUSIC);
        devices = productStrategies.getDeviceTypesForProductStrategy(strategyForMedia);
    } else if (ps == getProductStrategyForStream(AUDIO_STREAM_ACCESSIBILITY) &&
        (outputs.isActive(toVolumeSource(AUDIO_STREAM_RING)) ||
         outputs.isActive(toVolumeSource(AUDIO_STREAM_ALARM)))) {
            // do not route accessibility prompts to a digital output currently configured with a
            // compressed format as they would likely not be mixed and dropped.
            // Device For Sonification conf file has HDMI, SPDIF and HDMI ARC unreacheable.
        product_strategy_t strategyNotification = getProductStrategyForStream(AUDIO_STREAM_RING);
        devices = productStrategies.getDeviceTypesForProductStrategy(strategyNotification);
    } else {
        devices = productStrategies.getDeviceTypesForProductStrategy(ps);
    }
    if (devices == AUDIO_DEVICE_NONE ||
            (devices & availableOutputDevicesType) == AUDIO_DEVICE_NONE) {
        devices = getApmObserver()->getDefaultOutputDevice()->type();
        ALOGE_IF(devices == AUDIO_DEVICE_NONE, "%s: no valid default device defined", __FUNCTION__);
        return DeviceVector(getApmObserver()->getDefaultOutputDevice());
    }
    if (/*device_distinguishes_on_address(devices)*/ devices == AUDIO_DEVICE_OUT_BUS) {
        // We do expect only one device for these types of devices
        // Criterion device address garantee this one is available
        // If this criterion is not wished, need to ensure this device is available
        const String8 address(productStrategies.getDeviceAddressForProductStrategy(ps).c_str());
        ALOGV("%s:device 0x%x %s %d", __FUNCTION__, devices, address.c_str(), ps);
        return DeviceVector(availableOutputDevices.getDevice(devices,
                                                             address,
                                                             AUDIO_FORMAT_DEFAULT));
    }
    ALOGV("%s:device 0x%x %d", __FUNCTION__, devices, ps);
    return availableOutputDevices.getDevicesFromTypeMask(devices);
}

DeviceVector Engine::getOutputDevicesForAttributes(const audio_attributes_t &attributes,
                                                   const sp<DeviceDescriptor> &preferredDevice,
                                                   bool fromCache) const
{
    // First check for explict routing device
    if (preferredDevice != nullptr) {
        ALOGV("%s explicit Routing on device %s", __func__, preferredDevice->toString().c_str());
        return DeviceVector(preferredDevice);
    }
    product_strategy_t strategy = getProductStrategyForAttributes(attributes);
    const DeviceVector availableOutputDevices = getApmObserver()->getAvailableOutputDevices();
    const SwAudioOutputCollection &outputs = getApmObserver()->getOutputs();
    //
    // @TODO: what is the priority of explicit routing? Shall it be considered first as it used to
    // be by APM?
    //
    // Honor explicit routing requests only if all active clients have a preferred route in which
    // case the last active client route is used
    sp<DeviceDescriptor> device = findPreferredDevice(outputs, strategy, availableOutputDevices);
    if (device != nullptr) {
        return DeviceVector(device);
    }

    return fromCache? mDevicesForStrategies.at(strategy) : getDevicesForProductStrategy(strategy);
}

DeviceVector Engine::getOutputDevicesForStream(audio_stream_type_t stream, bool fromCache) const
{
    auto attributes = EngineBase::getAttributesForStreamType(stream);
    return getOutputDevicesForAttributes(attributes, nullptr, fromCache);
}

sp<DeviceDescriptor> Engine::getInputDeviceForAttributes(const audio_attributes_t &attr,
                                                         sp<AudioPolicyMix> *mix) const
{
    const auto &policyMixes = getApmObserver()->getAudioPolicyMixCollection();
    const auto availableInputDevices = getApmObserver()->getAvailableInputDevices();
    const auto &inputs = getApmObserver()->getInputs();
    std::string address;
    //
    // Explicit Routing ??? what is the priority of explicit routing? Shall it be considered
    // first as it used to be by APM?
    //
    // Honor explicit routing requests only if all active clients have a preferred route in which
    // case the last active client route is used
    sp<DeviceDescriptor> device =
            findPreferredDevice(inputs, attr.source, availableInputDevices);
    if (device != nullptr) {
        return device;
    }

    device = policyMixes.getDeviceAndMixForInputSource(attr.source, availableInputDevices, mix);
    if (device != nullptr) {
        return device;
    }

    audio_devices_t deviceType = getPropertyForKey<audio_devices_t, audio_source_t>(attr.source);

    if (audio_is_remote_submix_device(deviceType)) {
        address = "0";
        std::size_t pos;
        std::string tags { attr.tags };
        if ((pos = tags.find("addr=")) != std::string::npos) {
            address = tags.substr(pos + std::strlen("addr="));
        }
    }
    return availableInputDevices.getDevice(deviceType, String8(address.c_str()), AUDIO_FORMAT_DEFAULT);
}

void Engine::updateDeviceSelectionCache()
{
    for (const auto &iter : getProductStrategies()) {
        const auto &strategy = iter.second;
        mDevicesForStrategies[strategy->getId()] = getDevicesForProductStrategy(strategy->getId());
    }
}

void Engine::setDeviceAddressForProductStrategy(product_strategy_t strategy,
                                                const std::string &address)
{
    if (getProductStrategies().find(strategy) == getProductStrategies().end()) {
        ALOGE("%s: Trying to set address %s on invalid strategy %d", __FUNCTION__, address.c_str(),
              strategy);
        return;
    }
    getProductStrategies().at(strategy)->setDeviceAddress(address);
}

bool Engine::setDeviceTypesForProductStrategy(product_strategy_t strategy, audio_devices_t devices)
{
    if (getProductStrategies().find(strategy) == getProductStrategies().end()) {
        ALOGE("%s: set device %d on invalid strategy %d", __FUNCTION__, devices, strategy);
        return false;
    }
    getProductStrategies().at(strategy)->setDeviceTypes(devices);
    return true;
}

template <>
AudioPolicyManagerInterface *Engine::queryInterface()
{
    return this;
}

template <>
AudioPolicyPluginInterface *Engine::queryInterface()
{
    return this;
}

} // namespace audio_policy
} // namespace android


