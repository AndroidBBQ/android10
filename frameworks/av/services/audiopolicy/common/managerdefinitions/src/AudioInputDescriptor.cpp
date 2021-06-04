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

#define LOG_TAG "APM::AudioInputDescriptor"
//#define LOG_NDEBUG 0

#include <audiomanager/AudioManager.h>
#include <media/AudioPolicy.h>
#include <policy.h>
#include <AudioPolicyInterface.h>
#include "AudioInputDescriptor.h"
#include "AudioGain.h"
#include "AudioPolicyMix.h"
#include "HwModule.h"

namespace android {

AudioInputDescriptor::AudioInputDescriptor(const sp<IOProfile>& profile,
                                           AudioPolicyClientInterface *clientInterface)
    : mProfile(profile)
    ,  mClientInterface(clientInterface)
{
    if (profile != NULL) {
        profile->pickAudioProfile(mSamplingRate, mChannelMask, mFormat);
        if (profile->mGains.size() > 0) {
            profile->mGains[0]->getDefaultConfig(&mGain);
        }
    }
}

audio_module_handle_t AudioInputDescriptor::getModuleHandle() const
{
    if (mProfile == 0) {
        return AUDIO_MODULE_HANDLE_NONE;
    }
    return mProfile->getModuleHandle();
}

audio_port_handle_t AudioInputDescriptor::getId() const
{
    return mId;
}

audio_source_t AudioInputDescriptor::source() const
{
    return getHighestPriorityAttributes().source;
}

void AudioInputDescriptor::toAudioPortConfig(struct audio_port_config *dstConfig,
                                             const struct audio_port_config *srcConfig) const
{
    ALOG_ASSERT(mProfile != 0,
                "toAudioPortConfig() called on input with null profile %d", mIoHandle);
    dstConfig->config_mask = AUDIO_PORT_CONFIG_SAMPLE_RATE|AUDIO_PORT_CONFIG_CHANNEL_MASK|
                            AUDIO_PORT_CONFIG_FORMAT|AUDIO_PORT_CONFIG_GAIN;
    if (srcConfig != NULL) {
        dstConfig->config_mask |= srcConfig->config_mask;
    }

    AudioPortConfig::toAudioPortConfig(dstConfig, srcConfig);

    dstConfig->id = mId;
    dstConfig->role = AUDIO_PORT_ROLE_SINK;
    dstConfig->type = AUDIO_PORT_TYPE_MIX;
    dstConfig->ext.mix.hw_module = getModuleHandle();
    dstConfig->ext.mix.handle = mIoHandle;
    dstConfig->ext.mix.usecase.source = source();
}

void AudioInputDescriptor::toAudioPort(struct audio_port *port) const
{
    ALOG_ASSERT(mProfile != 0, "toAudioPort() called on input with null profile %d", mIoHandle);

    mProfile->toAudioPort(port);
    port->id = mId;
    toAudioPortConfig(&port->active_config);
    port->ext.mix.hw_module = getModuleHandle();
    port->ext.mix.handle = mIoHandle;
    port->ext.mix.latency_class = AUDIO_LATENCY_NORMAL;
}

void AudioInputDescriptor::setPreemptedSessions(const SortedVector<audio_session_t>& sessions)
{
    mPreemptedSessions = sessions;
}

SortedVector<audio_session_t> AudioInputDescriptor::getPreemptedSessions() const
{
    return mPreemptedSessions;
}

bool AudioInputDescriptor::hasPreemptedSession(audio_session_t session) const
{
    return (mPreemptedSessions.indexOf(session) >= 0);
}

void AudioInputDescriptor::clearPreemptedSessions()
{
    mPreemptedSessions.clear();
}

bool AudioInputDescriptor::isSourceActive(audio_source_t source) const
{
    for (const auto &client : getClientIterable()) {
        if (client->active() &&
            ((client->source() == source) ||
                ((source == AUDIO_SOURCE_VOICE_RECOGNITION) &&
                    (client->source() == AUDIO_SOURCE_HOTWORD) &&
                    client->isSoundTrigger()))) {
            return true;
        }
    }
    return false;
}

audio_attributes_t AudioInputDescriptor::getHighestPriorityAttributes() const
{
    audio_attributes_t attributes = { .source = AUDIO_SOURCE_DEFAULT };
    sp<RecordClientDescriptor> topClient = getHighestPriorityClient();
    return topClient ? topClient->attributes() : attributes;
}

sp<RecordClientDescriptor> AudioInputDescriptor::getHighestPriorityClient() const
{
    sp<RecordClientDescriptor> topClient;

    for (bool activeOnly : { true, false }) {
        int32_t topPriority = -1;
        app_state_t topState = APP_STATE_IDLE;
        for (const auto &client : getClientIterable()) {
            if (activeOnly && !client->active()) {
              continue;
            }
            app_state_t curState = client->appState();
            if (curState >= topState) {
                int32_t curPriority = source_priority(client->source());
                if (curPriority >= topPriority) {
                    topClient = client;
                    topPriority = curPriority;
                }
                topState = curState;
            }
        }
        if (topClient != nullptr) {
            break;
        }
    }
    return topClient;
}

bool AudioInputDescriptor::isSoundTrigger() const {
    // sound trigger and non sound trigger clients are not mixed on a given input
    // so check only first client
    if (getClientCount() == 0) {
        return false;
    }
    return getClientIterable().begin()->isSoundTrigger();
}

audio_patch_handle_t AudioInputDescriptor::getPatchHandle() const
{
    return mPatchHandle;
}

void AudioInputDescriptor::setPatchHandle(audio_patch_handle_t handle)
{
    mPatchHandle = handle;
    for (const auto &client : getClientIterable()) {
        if (client->active()) {
            updateClientRecordingConfiguration(
                    client->isLowLevel() ? RECORD_CONFIG_EVENT_START : RECORD_CONFIG_EVENT_UPDATE,
                    client);
        }
    }
}

audio_config_base_t AudioInputDescriptor::getConfig() const
{
    const audio_config_base_t config = { .sample_rate = mSamplingRate, .channel_mask = mChannelMask,
            .format = mFormat };
    return config;
}

status_t AudioInputDescriptor::open(const audio_config_t *config,
                                       const sp<DeviceDescriptor> &device,
                                       audio_source_t source,
                                       audio_input_flags_t flags,
                                       audio_io_handle_t *input)
{
    audio_config_t lConfig;
    if (config == nullptr) {
        lConfig = AUDIO_CONFIG_INITIALIZER;
        lConfig.sample_rate = mSamplingRate;
        lConfig.channel_mask = mChannelMask;
        lConfig.format = mFormat;
    } else {
        lConfig = *config;
    }

    mDevice = device;

    ALOGV("opening input for device %s profile %p name %s",
          mDevice->toString().c_str(), mProfile.get(), mProfile->getName().string());

    audio_devices_t deviceType = mDevice->type();

    status_t status = mClientInterface->openInput(mProfile->getModuleHandle(),
                                                  input,
                                                  &lConfig,
                                                  &deviceType,
                                                  mDevice->address(),
                                                  source,
                                                  flags);
    LOG_ALWAYS_FATAL_IF(mDevice->type() != deviceType,
                        "%s openInput returned device %08x when given device %08x",
                        __FUNCTION__, mDevice->type(), deviceType);

    if (status == NO_ERROR) {
        LOG_ALWAYS_FATAL_IF(*input == AUDIO_IO_HANDLE_NONE,
                            "%s openInput returned input handle %d for device %s",
                            __FUNCTION__, *input, mDevice->toString().c_str());
        mSamplingRate = lConfig.sample_rate;
        mChannelMask = lConfig.channel_mask;
        mFormat = lConfig.format;
        mId = AudioPort::getNextUniqueId();
        mIoHandle = *input;
        mProfile->curOpenCount++;
    }

    return status;
}

status_t AudioInputDescriptor::start()
{
    if (!isActive()) {
        if (!mProfile->canStartNewIo()) {
            ALOGI("%s mProfile->curActiveCount %d", __func__, mProfile->curActiveCount);
            return INVALID_OPERATION;
        }
        mProfile->curActiveCount++;
    }
    return NO_ERROR;
}

void AudioInputDescriptor::stop()
{
    if (!isActive()) {
        LOG_ALWAYS_FATAL_IF(mProfile->curActiveCount < 1,
                            "%s invalid profile active count %u",
                            __func__, mProfile->curActiveCount);
        mProfile->curActiveCount--;
    }
}

void AudioInputDescriptor::close()
{
    if (mIoHandle != AUDIO_IO_HANDLE_NONE) {
        // clean up active clients if any (can happen if close() is called to force
        // clients to reconnect
        for (const auto &client : getClientIterable()) {
            if (client->active()) {
                ALOGW("%s client with port ID %d still active on input %d",
                    __func__, client->portId(), mId);
                setClientActive(client, false);
                stop();
            }
        }

        mClientInterface->closeInput(mIoHandle);
        LOG_ALWAYS_FATAL_IF(mProfile->curOpenCount < 1, "%s profile open count %u",
                            __FUNCTION__, mProfile->curOpenCount);

        mProfile->curOpenCount--;
        LOG_ALWAYS_FATAL_IF(mProfile->curOpenCount <  mProfile->curActiveCount,
                "%s(%d): mProfile->curOpenCount %d < mProfile->curActiveCount %d.",
                __func__, mId, mProfile->curOpenCount, mProfile->curActiveCount);
        mIoHandle = AUDIO_IO_HANDLE_NONE;
    }
}

void AudioInputDescriptor::addClient(const sp<RecordClientDescriptor> &client) {
    ClientMapHandler<RecordClientDescriptor>::addClient(client);

    for (size_t i = 0; i < mEnabledEffects.size(); i++) {
        if (mEnabledEffects.valueAt(i)->mSession == client->session()) {
            client->trackEffectEnabled(mEnabledEffects.valueAt(i), true);
        }
    }
}

void AudioInputDescriptor::setClientActive(const sp<RecordClientDescriptor>& client, bool active)
{
    LOG_ALWAYS_FATAL_IF(getClient(client->portId()) == nullptr,
        "%s(%d) does not exist on input descriptor", __func__, client->portId());
    if (active == client->active()) {
        return;
    }

    // Handle non-client-specific activity ref count
    int32_t oldGlobalActiveCount = mGlobalActiveCount;
    if (!active && mGlobalActiveCount < 1) {
        LOG_ALWAYS_FATAL("%s(%d) invalid deactivation with globalActiveCount %d",
               __func__, client->portId(), mGlobalActiveCount);
        // mGlobalActiveCount = 1;
    }
    const int delta = active ? 1 : -1;
    mGlobalActiveCount += delta;

    sp<AudioPolicyMix> policyMix = mPolicyMix.promote();
    if ((oldGlobalActiveCount == 0) && (mGlobalActiveCount > 0)) {
        if ((policyMix != NULL) && ((policyMix->mCbFlags & AudioMix::kCbFlagNotifyActivity) != 0))
        {
            mClientInterface->onDynamicPolicyMixStateUpdate(policyMix->mDeviceAddress,
                                                            MIX_STATE_MIXING);
        }
    } else if ((oldGlobalActiveCount > 0) && (mGlobalActiveCount == 0)) {
        if ((policyMix != NULL) && ((policyMix->mCbFlags & AudioMix::kCbFlagNotifyActivity) != 0))
        {
            mClientInterface->onDynamicPolicyMixStateUpdate(policyMix->mDeviceAddress,
                                                            MIX_STATE_IDLE);
        }
    }

    client->setActive(active);

    checkSuspendEffects();

    int event = active ? RECORD_CONFIG_EVENT_START : RECORD_CONFIG_EVENT_STOP;
    updateClientRecordingConfiguration(event, client);
}

void AudioInputDescriptor::updateClientRecordingConfiguration(
    int event, const sp<RecordClientDescriptor>& client)
{
    ALOGV("%s riid %d uid %d port %d session %d event %d",
            __func__, client->riid(), client->uid(), client->portId(), client->session(), event);
    // do not send callback if starting and no device is selected yet to avoid
    // double callbacks from startInput() before and after the device is selected
    // "start" and "stop" events for "high level" clients (AudioRecord) are sent by the client side
    if ((event == RECORD_CONFIG_EVENT_START && mPatchHandle == AUDIO_PATCH_HANDLE_NONE)
            || (!client->isLowLevel()
                    && (event == RECORD_CONFIG_EVENT_START || event == RECORD_CONFIG_EVENT_STOP))) {
        return;
    }

    const audio_config_base_t sessionConfig = client->config();
    const record_client_info_t recordClientInfo{client->riid(), client->uid(), client->session(),
                                                client->source(), client->portId(),
                                                client->isSilenced()};
    const audio_config_base_t config = getConfig();

    std::vector<effect_descriptor_t> clientEffects;
    EffectDescriptorCollection effectsList = client->getEnabledEffects();
    for (size_t i = 0; i < effectsList.size(); i++) {
        clientEffects.push_back(effectsList.valueAt(i)->mDesc);
    }

    std::vector<effect_descriptor_t> effects;
    effectsList = getEnabledEffects();
    for (size_t i = 0; i < effectsList.size(); i++) {
        effects.push_back(effectsList.valueAt(i)->mDesc);
    }

    mClientInterface->onRecordingConfigurationUpdate(event, &recordClientInfo, &sessionConfig,
                                                     clientEffects, &config, effects,
                                                     mPatchHandle, source());
}

RecordClientVector AudioInputDescriptor::getClientsForSession(
    audio_session_t session)
{
    RecordClientVector clients;
    for (const auto &client : getClientIterable()) {
        if (client->session() == session) {
            clients.push_back(client);
        }
    }
    return clients;
}

RecordClientVector AudioInputDescriptor::clientsList(bool activeOnly, audio_source_t source,
                                                     bool preferredDeviceOnly) const
{
    RecordClientVector clients;
    for (const auto &client : getClientIterable()) {
        if ((!activeOnly || client->active())
            && (source == AUDIO_SOURCE_DEFAULT || source == client->source())
            && (!preferredDeviceOnly || client->hasPreferredDevice())) {
            clients.push_back(client);
        }
    }
    return clients;
}

void AudioInputDescriptor::trackEffectEnabled(const sp<EffectDescriptor> &effect,
                                              bool enabled)
{
    if (enabled) {
        mEnabledEffects.replaceValueFor(effect->mId, effect);
    } else {
        mEnabledEffects.removeItem(effect->mId);
        // always exit from suspend when disabling an effect as only enabled effects
        // are managed by checkSuspendEffects()
        if (effect->mSuspended) {
            effect->mSuspended = false;
            mClientInterface->setEffectSuspended(effect->mId, effect->mSession, effect->mSuspended);
        }
    }

    RecordClientVector clients = getClientsForSession((audio_session_t)effect->mSession);
    RecordClientVector updatedClients;

    for (const auto& client : clients) {
        sp<EffectDescriptor> clientEffect = client->getEnabledEffects().getEffect(effect->mId);
        bool changed = (enabled && clientEffect == nullptr)
                || (!enabled && clientEffect != nullptr);
        client->trackEffectEnabled(effect, enabled);
        if (changed && client->active()) {
            updatedClients.push_back(client);
        }
    }

    checkSuspendEffects();

    for (const auto& client : updatedClients) {
        updateClientRecordingConfiguration(RECORD_CONFIG_EVENT_UPDATE, client);
    }
}

EffectDescriptorCollection AudioInputDescriptor::getEnabledEffects() const
{
    // report effects for highest priority active source as applied to all clients
    EffectDescriptorCollection enabledEffects;
    sp<RecordClientDescriptor> topClient = getHighestPriorityClient();
    if (topClient != nullptr) {
        enabledEffects = topClient->getEnabledEffects();
    }
    return enabledEffects;
}

void AudioInputDescriptor::setAppState(uid_t uid, app_state_t state)
{
    RecordClientVector clients = clientsList(false /*activeOnly*/);
    RecordClientVector updatedClients;

    for (const auto& client : clients) {
        if (uid == client->uid()) {
            bool wasSilenced = client->isSilenced();
            client->setAppState(state);
            if (client->active() && wasSilenced != client->isSilenced()) {
                updatedClients.push_back(client);
            }
        }
    }

    checkSuspendEffects();

    for (const auto& client : updatedClients) {
        updateClientRecordingConfiguration(RECORD_CONFIG_EVENT_UPDATE, client);
    }
}

void AudioInputDescriptor::checkSuspendEffects()
{
    sp<RecordClientDescriptor> topClient = getHighestPriorityClient();
    if (topClient == nullptr) {
        return;
    }

    for (size_t i = 0; i < mEnabledEffects.size(); i++) {
        sp<EffectDescriptor> effect = mEnabledEffects.valueAt(i);
        if (effect->mSession == topClient->session()) {
            if (effect->mSuspended) {
                effect->mSuspended = false;
                mClientInterface->setEffectSuspended(effect->mId,
                                                     effect->mSession,
                                                     effect->mSuspended);
            }
        } else if (!effect->mSuspended) {
            effect->mSuspended = true;
            mClientInterface->setEffectSuspended(effect->mId,
                                                 effect->mSession,
                                                 effect->mSuspended);
        }
    }
}

void AudioInputDescriptor::dump(String8 *dst) const
{
    dst->appendFormat(" ID: %d\n", getId());
    dst->appendFormat(" Sampling rate: %d\n", mSamplingRate);
    dst->appendFormat(" Format: %d\n", mFormat);
    dst->appendFormat(" Channels: %08x\n", mChannelMask);
    dst->appendFormat(" Devices %s\n", mDevice->toString().c_str());
    mEnabledEffects.dump(dst, 1 /*spaces*/, false /*verbose*/);
    dst->append(" AudioRecord Clients:\n");
    ClientMapHandler<RecordClientDescriptor>::dump(dst);
    dst->append("\n");
}

bool AudioInputCollection::isSourceActive(audio_source_t source) const
{
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioInputDescriptor>  inputDescriptor = valueAt(i);
        if (inputDescriptor->isSourceActive(source)) {
            return true;
        }
    }
    return false;
}

sp<AudioInputDescriptor> AudioInputCollection::getInputFromId(audio_port_handle_t id) const
{
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioInputDescriptor> inputDescriptor = valueAt(i);
        if (inputDescriptor->getId() == id) {
            return inputDescriptor;
        }
    }
    return NULL;
}

uint32_t AudioInputCollection::activeInputsCountOnDevices(const DeviceVector &devices) const
{
    uint32_t count = 0;
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioInputDescriptor>  inputDescriptor = valueAt(i);
        if (inputDescriptor->isActive() &&
                (devices.isEmpty() || devices.contains(inputDescriptor->getDevice()))) {
            count++;
        }
    }
    return count;
}

Vector<sp <AudioInputDescriptor> > AudioInputCollection::getActiveInputs()
{
    Vector<sp <AudioInputDescriptor> > activeInputs;

    for (size_t i = 0; i < size(); i++) {
        const sp<AudioInputDescriptor>  inputDescriptor = valueAt(i);
        if (inputDescriptor->isActive()) {
            activeInputs.add(inputDescriptor);
        }
    }
    return activeInputs;
}

sp<AudioInputDescriptor> AudioInputCollection::getInputForClient(audio_port_handle_t portId)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioInputDescriptor> inputDesc = valueAt(i);
        if (inputDesc->getClient(portId) != nullptr) {
            return inputDesc;
        }
    }
    return 0;
}

void AudioInputCollection::trackEffectEnabled(const sp<EffectDescriptor> &effect,
                                            bool enabled)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioInputDescriptor> inputDesc = valueAt(i);
        if (inputDesc->mIoHandle == effect->mIo) {
            return inputDesc->trackEffectEnabled(effect, enabled);
        }
    }
}

void AudioInputCollection::clearSessionRoutesForDevice(
    const sp<DeviceDescriptor> &disconnectedDevice)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioInputDescriptor> inputDesc = valueAt(i);
        for (const auto& client : inputDesc->getClientIterable()) {
            if (client->preferredDeviceId() == disconnectedDevice->getId()) {
                client->setPreferredDeviceId(AUDIO_PORT_HANDLE_NONE);
            }
        }
    }
}

void AudioInputCollection::dump(String8 *dst) const
{
    dst->append("\nInputs dump:\n");
    for (size_t i = 0; i < size(); i++) {
        dst->appendFormat("- Input %d dump:\n", keyAt(i));
        valueAt(i)->dump(dst);
    }
}

}; //namespace android
