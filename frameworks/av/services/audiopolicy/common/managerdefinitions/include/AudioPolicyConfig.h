/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <unordered_map>
#include <unordered_set>

#include <AudioGain.h>
#include <AudioPort.h>
#include <AudioPatch.h>
#include <DeviceDescriptor.h>
#include <IOProfile.h>
#include <HwModule.h>
#include <AudioInputDescriptor.h>
#include <AudioOutputDescriptor.h>
#include <AudioPolicyMix.h>
#include <EffectDescriptor.h>
#include <SoundTriggerSession.h>

namespace android {

class AudioPolicyConfig
{
public:
    AudioPolicyConfig(HwModuleCollection &hwModules,
                      DeviceVector &availableOutputDevices,
                      DeviceVector &availableInputDevices,
                      sp<DeviceDescriptor> &defaultOutputDevice)
        : mHwModules(hwModules),
          mAvailableOutputDevices(availableOutputDevices),
          mAvailableInputDevices(availableInputDevices),
          mDefaultOutputDevice(defaultOutputDevice),
          mIsSpeakerDrcEnabled(false)
    {}

    const std::string& getSource() const {
        return mSource;
    }

    void setSource(const std::string& file) {
        mSource = file;
    }

    void setHwModules(const HwModuleCollection &hwModules)
    {
        mHwModules = hwModules;
    }

    void addAvailableDevice(const sp<DeviceDescriptor> &availableDevice)
    {
        if (audio_is_output_device(availableDevice->type())) {
            mAvailableOutputDevices.add(availableDevice);
        } else if (audio_is_input_device(availableDevice->type())) {
            mAvailableInputDevices.add(availableDevice);
        }
    }

    void addAvailableInputDevices(const DeviceVector &availableInputDevices)
    {
        mAvailableInputDevices.add(availableInputDevices);
    }

    void addAvailableOutputDevices(const DeviceVector &availableOutputDevices)
    {
        mAvailableOutputDevices.add(availableOutputDevices);
    }

    bool isSpeakerDrcEnabled() const { return mIsSpeakerDrcEnabled; }

    void setSpeakerDrcEnabled(bool isSpeakerDrcEnabled)
    {
        mIsSpeakerDrcEnabled = isSpeakerDrcEnabled;
    }

    const HwModuleCollection getHwModules() const { return mHwModules; }

    const DeviceVector &getAvailableInputDevices() const
    {
        return mAvailableInputDevices;
    }

    const DeviceVector &getAvailableOutputDevices() const
    {
        return mAvailableOutputDevices;
    }

    void setDefaultOutputDevice(const sp<DeviceDescriptor> &defaultDevice)
    {
        mDefaultOutputDevice = defaultDevice;
    }

    const sp<DeviceDescriptor> &getDefaultOutputDevice() const { return mDefaultOutputDevice; }

    void setDefault(void)
    {
        mSource = "AudioPolicyConfig::setDefault";
        mDefaultOutputDevice = new DeviceDescriptor(AUDIO_DEVICE_OUT_SPEAKER);
        mDefaultOutputDevice->addAudioProfile(AudioProfile::createFullDynamic());
        sp<DeviceDescriptor> defaultInputDevice = new DeviceDescriptor(AUDIO_DEVICE_IN_BUILTIN_MIC);
        defaultInputDevice->addAudioProfile(AudioProfile::createFullDynamic());
        sp<AudioProfile> micProfile = new AudioProfile(
                AUDIO_FORMAT_PCM_16_BIT, AUDIO_CHANNEL_IN_MONO, 8000);
        defaultInputDevice->addAudioProfile(micProfile);
        mAvailableOutputDevices.add(mDefaultOutputDevice);
        mAvailableInputDevices.add(defaultInputDevice);

        sp<HwModule> module = new HwModule(AUDIO_HARDWARE_MODULE_ID_PRIMARY, 2 /*halVersionMajor*/);
        mHwModules.add(module);
        mDefaultOutputDevice->attach(module);
        defaultInputDevice->attach(module);

        sp<OutputProfile> outProfile = new OutputProfile(String8("primary"));
        outProfile->addAudioProfile(
                new AudioProfile(AUDIO_FORMAT_PCM_16_BIT, AUDIO_CHANNEL_OUT_STEREO, 44100));
        outProfile->addSupportedDevice(mDefaultOutputDevice);
        outProfile->setFlags(AUDIO_OUTPUT_FLAG_PRIMARY);
        module->addOutputProfile(outProfile);

        sp<InputProfile> inProfile = new InputProfile(String8("primary"));
        inProfile->addAudioProfile(micProfile);
        inProfile->addSupportedDevice(defaultInputDevice);
        module->addInputProfile(inProfile);

        setDefaultSurroundFormats();
    }

    // Surround formats, with an optional list of subformats that are equivalent from users' POV.
    using SurroundFormats = std::unordered_map<audio_format_t, std::unordered_set<audio_format_t>>;

    const SurroundFormats &getSurroundFormats() const
    {
        return mSurroundFormats;
    }

    void setSurroundFormats(const SurroundFormats &surroundFormats)
    {
        mSurroundFormats = surroundFormats;
    }

    void setDefaultSurroundFormats()
    {
        mSurroundFormats = {
            {AUDIO_FORMAT_AC3, {}},
            {AUDIO_FORMAT_E_AC3, {}},
            {AUDIO_FORMAT_DTS, {}},
            {AUDIO_FORMAT_DTS_HD, {}},
            {AUDIO_FORMAT_AAC_LC, {
                    AUDIO_FORMAT_AAC_HE_V1, AUDIO_FORMAT_AAC_HE_V2, AUDIO_FORMAT_AAC_ELD,
                    AUDIO_FORMAT_AAC_XHE}},
            {AUDIO_FORMAT_DOLBY_TRUEHD, {}},
            {AUDIO_FORMAT_E_AC3_JOC, {}},
            {AUDIO_FORMAT_AC4, {}}};
    }

private:
    std::string mSource;
    HwModuleCollection &mHwModules; /**< Collection of Module, with Profiles, i.e. Mix Ports. */
    DeviceVector &mAvailableOutputDevices;
    DeviceVector &mAvailableInputDevices;
    sp<DeviceDescriptor> &mDefaultOutputDevice;
    // TODO: remove when legacy conf file is removed. true on devices that use DRC on the
    // DEVICE_CATEGORY_SPEAKER path to boost soft sounds, used to adjust volume curves accordingly.
    // Note: remove also speaker_drc_enabled from global configuration of XML config file.
    bool mIsSpeakerDrcEnabled;
    SurroundFormats mSurroundFormats;
};

} // namespace android
