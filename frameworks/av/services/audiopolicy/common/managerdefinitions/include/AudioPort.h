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

#include "AudioCollections.h"
#include "AudioProfile.h"
#include "AudioGain.h"
#include "HandleGenerator.h"
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <system/audio.h>
#include <cutils/config_utils.h>

namespace android {

class HwModule;
class AudioRoute;

class AudioPort : public virtual RefBase, private HandleGenerator<audio_port_handle_t>
{
public:
    AudioPort(const String8& name, audio_port_type_t type,  audio_port_role_t role) :
        mName(name), mType(type), mRole(role), mFlags(AUDIO_OUTPUT_FLAG_NONE) {}

    virtual ~AudioPort() {}

    void setName(const String8 &name) { mName = name; }
    const String8 &getName() const { return mName; }

    audio_port_type_t getType() const { return mType; }
    audio_port_role_t getRole() const { return mRole; }

    virtual const String8 getTagName() const = 0;

    void setGains(const AudioGains &gains) { mGains = gains; }
    const AudioGains &getGains() const { return mGains; }

    virtual void setFlags(uint32_t flags)
    {
        //force direct flag if offload flag is set: offloading implies a direct output stream
        // and all common behaviors are driven by checking only the direct flag
        // this should normally be set appropriately in the policy configuration file
        if (mRole == AUDIO_PORT_ROLE_SOURCE && (flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) != 0) {
            flags |= AUDIO_OUTPUT_FLAG_DIRECT;
        }
        mFlags = flags;
    }
    uint32_t getFlags() const { return mFlags; }

    virtual void attach(const sp<HwModule>& module);
    virtual void detach();
    bool isAttached() { return mModule != 0; }

    // Audio port IDs are in a different namespace than AudioFlinger unique IDs
    static audio_port_handle_t getNextUniqueId();

    virtual void toAudioPort(struct audio_port *port) const;

    virtual void importAudioPort(const sp<AudioPort>& port, bool force = false);

    void addAudioProfile(const sp<AudioProfile> &profile) { mProfiles.add(profile); }

    void setAudioProfiles(const AudioProfileVector &profiles) { mProfiles = profiles; }
    AudioProfileVector &getAudioProfiles() { return mProfiles; }

    bool hasValidAudioProfile() const { return mProfiles.hasValidProfile(); }

    bool hasDynamicAudioProfile() const { return mProfiles.hasDynamicProfile(); }

    // searches for an exact match
    virtual status_t checkExactAudioProfile(const struct audio_port_config *config) const;

    // searches for a compatible match, currently implemented for input
    // parameters are input|output, returned value is the best match.
    status_t checkCompatibleAudioProfile(uint32_t &samplingRate,
                                         audio_channel_mask_t &channelMask,
                                         audio_format_t &format) const
    {
        return mProfiles.checkCompatibleProfile(samplingRate, channelMask, format, mType, mRole);
    }

    void clearAudioProfiles() { return mProfiles.clearProfiles(); }

    status_t checkGain(const struct audio_gain_config *gainConfig, int index) const;

    void pickAudioProfile(uint32_t &samplingRate,
                          audio_channel_mask_t &channelMask,
                          audio_format_t &format) const;

    static const audio_format_t sPcmFormatCompareTable[];

    static int compareFormats(audio_format_t format1, audio_format_t format2);

    // Used to select an audio HAL output stream with a sample format providing the
    // less degradation for a given AudioTrack sample format.
    static bool isBetterFormatMatch(audio_format_t newFormat,
                                        audio_format_t currentFormat,
                                        audio_format_t targetFormat);
    static uint32_t formatDistance(audio_format_t format1,
                                   audio_format_t format2);
    static const uint32_t kFormatDistanceMax = 4;

    audio_module_handle_t getModuleHandle() const;
    uint32_t getModuleVersionMajor() const;
    const char *getModuleName() const;
    sp<HwModule> getModule() const { return mModule; }

    bool useInputChannelMask() const
    {
        return ((mType == AUDIO_PORT_TYPE_DEVICE) && (mRole == AUDIO_PORT_ROLE_SOURCE)) ||
                ((mType == AUDIO_PORT_TYPE_MIX) && (mRole == AUDIO_PORT_ROLE_SINK));
    }

    inline bool isDirectOutput() const
    {
        return (mType == AUDIO_PORT_TYPE_MIX) && (mRole == AUDIO_PORT_ROLE_SOURCE) &&
                (mFlags & (AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD));
    }

    void addRoute(const sp<AudioRoute> &route) { mRoutes.add(route); }
    const AudioRouteVector &getRoutes() const { return mRoutes; }

    void dump(String8 *dst, int spaces, bool verbose = true) const;

    void log(const char* indent) const;

    AudioGains mGains; // gain controllers

private:
    void pickChannelMask(audio_channel_mask_t &channelMask, const ChannelsVector &channelMasks) const;
    void pickSamplingRate(uint32_t &rate,const SampleRateVector &samplingRates) const;

    sp<HwModule> mModule;                 // audio HW module exposing this I/O stream
    String8  mName;
    audio_port_type_t mType;
    audio_port_role_t mRole;
    uint32_t mFlags; // attribute flags mask (e.g primary output, direct output...).
    AudioProfileVector mProfiles; // AudioProfiles supported by this port (format, Rates, Channels)
    AudioRouteVector mRoutes; // Routes involving this port
};

class AudioPortConfig : public virtual RefBase
{
public:
    status_t applyAudioPortConfig(const struct audio_port_config *config,
                                  struct audio_port_config *backupConfig = NULL);
    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
                                   const struct audio_port_config *srcConfig = NULL) const = 0;
    virtual sp<AudioPort> getAudioPort() const = 0;
    virtual bool hasSameHwModuleAs(const sp<AudioPortConfig>& other) const {
        return (other != 0) && (other->getAudioPort() != 0) && (getAudioPort() != 0) &&
                (other->getAudioPort()->getModuleHandle() == getAudioPort()->getModuleHandle());
    }
    bool hasGainController(bool canUseForVolume = false) const;

    unsigned int mSamplingRate = 0u;
    audio_format_t mFormat = AUDIO_FORMAT_INVALID;
    audio_channel_mask_t mChannelMask = AUDIO_CHANNEL_NONE;
    struct audio_gain_config mGain = { .index = -1 };
    union audio_io_flags mFlags = { AUDIO_INPUT_FLAG_NONE };
};

} // namespace android
