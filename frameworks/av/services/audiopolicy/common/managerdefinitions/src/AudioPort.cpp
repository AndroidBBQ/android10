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

#define LOG_TAG "APM::AudioPort"
//#define LOG_NDEBUG 0
#include "TypeConverter.h"
#include "AudioPort.h"
#include "HwModule.h"
#include "AudioGain.h"
#include <policy.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

namespace android {

// --- AudioPort class implementation
void AudioPort::attach(const sp<HwModule>& module)
{
    ALOGV("%s: attaching module %s to port %s", __FUNCTION__, getModuleName(), mName.string());
    mModule = module;
}

void AudioPort::detach()
{
    mModule = nullptr;
}

// Note that is a different namespace than AudioFlinger unique IDs
audio_port_handle_t AudioPort::getNextUniqueId()
{
    return getNextHandle();
}

audio_module_handle_t AudioPort::getModuleHandle() const
{
    return mModule != 0 ? mModule->getHandle() : AUDIO_MODULE_HANDLE_NONE;
}

uint32_t AudioPort::getModuleVersionMajor() const
{
    return mModule != 0 ? mModule->getHalVersionMajor() : 0;
}

const char *AudioPort::getModuleName() const
{
    return mModule != 0 ? mModule->getName() : "invalid module";
}

void AudioPort::toAudioPort(struct audio_port *port) const
{
    // TODO: update this function once audio_port structure reflects the new profile definition.
    // For compatibility reason: flatening the AudioProfile into audio_port structure.
    SortedVector<audio_format_t> flatenedFormats;
    SampleRateVector flatenedRates;
    ChannelsVector flatenedChannels;
    for (const auto& profile : mProfiles) {
        if (profile->isValid()) {
            audio_format_t formatToExport = profile->getFormat();
            const SampleRateVector &ratesToExport = profile->getSampleRates();
            const ChannelsVector &channelsToExport = profile->getChannels();

            if (flatenedFormats.indexOf(formatToExport) < 0) {
                flatenedFormats.add(formatToExport);
            }
            for (size_t rateIndex = 0; rateIndex < ratesToExport.size(); rateIndex++) {
                uint32_t rate = ratesToExport[rateIndex];
                if (flatenedRates.indexOf(rate) < 0) {
                    flatenedRates.add(rate);
                }
            }
            for (size_t chanIndex = 0; chanIndex < channelsToExport.size(); chanIndex++) {
                audio_channel_mask_t channels = channelsToExport[chanIndex];
                if (flatenedChannels.indexOf(channels) < 0) {
                    flatenedChannels.add(channels);
                }
            }
            if (flatenedRates.size() > AUDIO_PORT_MAX_SAMPLING_RATES ||
                    flatenedChannels.size() > AUDIO_PORT_MAX_CHANNEL_MASKS ||
                    flatenedFormats.size() > AUDIO_PORT_MAX_FORMATS) {
                ALOGE("%s: bailing out: cannot export profiles to port config", __FUNCTION__);
                return;
            }
        }
    }
    port->role = mRole;
    port->type = mType;
    strlcpy(port->name, mName, AUDIO_PORT_MAX_NAME_LEN);
    port->num_sample_rates = flatenedRates.size();
    port->num_channel_masks = flatenedChannels.size();
    port->num_formats = flatenedFormats.size();
    for (size_t i = 0; i < flatenedRates.size(); i++) {
        port->sample_rates[i] = flatenedRates[i];
    }
    for (size_t i = 0; i < flatenedChannels.size(); i++) {
        port->channel_masks[i] = flatenedChannels[i];
    }
    for (size_t i = 0; i < flatenedFormats.size(); i++) {
        port->formats[i] = flatenedFormats[i];
    }

    ALOGV("AudioPort::toAudioPort() num gains %zu", mGains.size());

    uint32_t i;
    for (i = 0; i < mGains.size() && i < AUDIO_PORT_MAX_GAINS; i++) {
        port->gains[i] = mGains[i]->getGain();
    }
    port->num_gains = i;
}

void AudioPort::importAudioPort(const sp<AudioPort>& port, bool force __unused)
{
    for (const auto& profileToImport : port->mProfiles) {
        if (profileToImport->isValid()) {
            // Import only valid port, i.e. valid format, non empty rates and channels masks
            bool hasSameProfile = false;
            for (const auto& profile : mProfiles) {
                if (*profile == *profileToImport) {
                    // never import a profile twice
                    hasSameProfile = true;
                    break;
                }
            }
            if (hasSameProfile) { // never import a same profile twice
                continue;
            }
            addAudioProfile(profileToImport);
        }
    }
}

status_t AudioPort::checkExactAudioProfile(const struct audio_port_config *config) const
{
    status_t status = NO_ERROR;
    auto config_mask = config->config_mask;
    if (config_mask & AUDIO_PORT_CONFIG_GAIN) {
        config_mask &= ~AUDIO_PORT_CONFIG_GAIN;
        status = checkGain(&config->gain, config->gain.index);
        if (status != NO_ERROR) {
            return status;
        }
    }
    if (config_mask != 0) {
        // TODO should we check sample_rate / channel_mask / format separately?
        status = mProfiles.checkExactProfile(config->sample_rate,
                                             config->channel_mask,
                                             config->format);
    }
    return status;
}

void AudioPort::pickSamplingRate(uint32_t &pickedRate,const SampleRateVector &samplingRates) const
{
    pickedRate = 0;
    // For direct outputs, pick minimum sampling rate: this helps ensuring that the
    // channel count / sampling rate combination chosen will be supported by the connected
    // sink
    if (isDirectOutput()) {
        uint32_t samplingRate = UINT_MAX;
        for (size_t i = 0; i < samplingRates.size(); i ++) {
            if ((samplingRates[i] < samplingRate) && (samplingRates[i] > 0)) {
                samplingRate = samplingRates[i];
            }
        }
        pickedRate = (samplingRate == UINT_MAX) ? 0 : samplingRate;
    } else {
        uint32_t maxRate = SAMPLE_RATE_HZ_MAX;

        // For mixed output and inputs, use max mixer sampling rates. Do not
        // limit sampling rate otherwise
        // For inputs, also see checkCompatibleSamplingRate().
        if (mType != AUDIO_PORT_TYPE_MIX) {
            maxRate = UINT_MAX;
        }
        // TODO: should mSamplingRates[] be ordered in terms of our preference
        // and we return the first (and hence most preferred) match?  This is of concern if
        // we want to choose 96kHz over 192kHz for USB driver stability or resource constraints.
        for (size_t i = 0; i < samplingRates.size(); i ++) {
            if ((samplingRates[i] > pickedRate) && (samplingRates[i] <= maxRate)) {
                pickedRate = samplingRates[i];
            }
        }
    }
}

void AudioPort::pickChannelMask(audio_channel_mask_t &pickedChannelMask,
                                const ChannelsVector &channelMasks) const
{
    pickedChannelMask = AUDIO_CHANNEL_NONE;
    // For direct outputs, pick minimum channel count: this helps ensuring that the
    // channel count / sampling rate combination chosen will be supported by the connected
    // sink
    if (isDirectOutput()) {
        uint32_t channelCount = UINT_MAX;
        for (size_t i = 0; i < channelMasks.size(); i ++) {
            uint32_t cnlCount;
            if (useInputChannelMask()) {
                cnlCount = audio_channel_count_from_in_mask(channelMasks[i]);
            } else {
                cnlCount = audio_channel_count_from_out_mask(channelMasks[i]);
            }
            if ((cnlCount < channelCount) && (cnlCount > 0)) {
                pickedChannelMask = channelMasks[i];
                channelCount = cnlCount;
            }
        }
    } else {
        uint32_t channelCount = 0;
        uint32_t maxCount = MAX_MIXER_CHANNEL_COUNT;

        // For mixed output and inputs, use max mixer channel count. Do not
        // limit channel count otherwise
        if (mType != AUDIO_PORT_TYPE_MIX) {
            maxCount = UINT_MAX;
        }
        for (size_t i = 0; i < channelMasks.size(); i ++) {
            uint32_t cnlCount;
            if (useInputChannelMask()) {
                cnlCount = audio_channel_count_from_in_mask(channelMasks[i]);
            } else {
                cnlCount = audio_channel_count_from_out_mask(channelMasks[i]);
            }
            if ((cnlCount > channelCount) && (cnlCount <= maxCount)) {
                pickedChannelMask = channelMasks[i];
                channelCount = cnlCount;
            }
        }
    }
}

/* format in order of increasing preference */
const audio_format_t AudioPort::sPcmFormatCompareTable[] = {
        AUDIO_FORMAT_DEFAULT,
        AUDIO_FORMAT_PCM_16_BIT,
        AUDIO_FORMAT_PCM_8_24_BIT,
        AUDIO_FORMAT_PCM_24_BIT_PACKED,
        AUDIO_FORMAT_PCM_32_BIT,
        AUDIO_FORMAT_PCM_FLOAT,
};

int AudioPort::compareFormats(audio_format_t format1, audio_format_t format2)
{
    // NOTE: AUDIO_FORMAT_INVALID is also considered not PCM and will be compared equal to any
    // compressed format and better than any PCM format. This is by design of pickFormat()
    if (!audio_is_linear_pcm(format1)) {
        if (!audio_is_linear_pcm(format2)) {
            return 0;
        }
        return 1;
    }
    if (!audio_is_linear_pcm(format2)) {
        return -1;
    }

    int index1 = -1, index2 = -1;
    for (size_t i = 0;
            (i < ARRAY_SIZE(sPcmFormatCompareTable)) && ((index1 == -1) || (index2 == -1));
            i ++) {
        if (sPcmFormatCompareTable[i] == format1) {
            index1 = i;
        }
        if (sPcmFormatCompareTable[i] == format2) {
            index2 = i;
        }
    }
    // format1 not found => index1 < 0 => format2 > format1
    // format2 not found => index2 < 0 => format2 < format1
    return index1 - index2;
}

uint32_t AudioPort::formatDistance(audio_format_t format1, audio_format_t format2)
{
    if (format1 == format2) {
        return 0;
    }
    if (format1 == AUDIO_FORMAT_INVALID || format2 == AUDIO_FORMAT_INVALID) {
        return kFormatDistanceMax;
    }
    int diffBytes = (int)audio_bytes_per_sample(format1) -
            audio_bytes_per_sample(format2);

    return abs(diffBytes);
}

bool AudioPort::isBetterFormatMatch(audio_format_t newFormat,
                                    audio_format_t currentFormat,
                                    audio_format_t targetFormat)
{
    return formatDistance(newFormat, targetFormat) < formatDistance(currentFormat, targetFormat);
}

void AudioPort::pickAudioProfile(uint32_t &samplingRate,
                                 audio_channel_mask_t &channelMask,
                                 audio_format_t &format) const
{
    format = AUDIO_FORMAT_DEFAULT;
    samplingRate = 0;
    channelMask = AUDIO_CHANNEL_NONE;

    // special case for uninitialized dynamic profile
    if (!mProfiles.hasValidProfile()) {
        return;
    }
    audio_format_t bestFormat = sPcmFormatCompareTable[ARRAY_SIZE(sPcmFormatCompareTable) - 1];
    // For mixed output and inputs, use best mixer output format.
    // Do not limit format otherwise
    if ((mType != AUDIO_PORT_TYPE_MIX) || isDirectOutput()) {
        bestFormat = AUDIO_FORMAT_INVALID;
    }

    for (size_t i = 0; i < mProfiles.size(); i ++) {
        if (!mProfiles[i]->isValid()) {
            continue;
        }
        audio_format_t formatToCompare = mProfiles[i]->getFormat();
        if ((compareFormats(formatToCompare, format) > 0) &&
                (compareFormats(formatToCompare, bestFormat) <= 0)) {
            uint32_t pickedSamplingRate = 0;
            audio_channel_mask_t pickedChannelMask = AUDIO_CHANNEL_NONE;
            pickChannelMask(pickedChannelMask, mProfiles[i]->getChannels());
            pickSamplingRate(pickedSamplingRate, mProfiles[i]->getSampleRates());

            if (formatToCompare != AUDIO_FORMAT_DEFAULT && pickedChannelMask != AUDIO_CHANNEL_NONE
                    && pickedSamplingRate != 0) {
                format = formatToCompare;
                channelMask = pickedChannelMask;
                samplingRate = pickedSamplingRate;
                // TODO: shall we return on the first one or still trying to pick a better Profile?
            }
        }
    }
    ALOGV("%s Port[nm:%s] profile rate=%d, format=%d, channels=%d", __FUNCTION__, mName.string(),
          samplingRate, channelMask, format);
}

status_t AudioPort::checkGain(const struct audio_gain_config *gainConfig, int index) const
{
    if (index < 0 || (size_t)index >= mGains.size()) {
        return BAD_VALUE;
    }
    return mGains[index]->checkConfig(gainConfig);
}

void AudioPort::dump(String8 *dst, int spaces, bool verbose) const
{
    if (!mName.isEmpty()) {
        dst->appendFormat("%*s- name: %s\n", spaces, "", mName.string());
    }
    if (verbose) {
        mProfiles.dump(dst, spaces);

        if (mGains.size() != 0) {
            dst->appendFormat("%*s- gains:\n", spaces, "");
            for (size_t i = 0; i < mGains.size(); i++) {
                mGains[i]->dump(dst, spaces + 2, i);
            }
        }
    }
}

void AudioPort::log(const char* indent) const
{
    ALOGI("%s Port[nm:%s, type:%d, role:%d]", indent, mName.string(), mType, mRole);
}

// --- AudioPortConfig class implementation

status_t AudioPortConfig::applyAudioPortConfig(const struct audio_port_config *config,
                                               struct audio_port_config *backupConfig)
{
    struct audio_port_config localBackupConfig = { .config_mask = config->config_mask };
    status_t status = NO_ERROR;

    toAudioPortConfig(&localBackupConfig);

    sp<AudioPort> audioport = getAudioPort();
    if (audioport == 0) {
        status = NO_INIT;
        goto exit;
    }
    status = audioport->checkExactAudioProfile(config);
    if (status != NO_ERROR) {
        goto exit;
    }
    if (config->config_mask & AUDIO_PORT_CONFIG_SAMPLE_RATE) {
        mSamplingRate = config->sample_rate;
    }
    if (config->config_mask & AUDIO_PORT_CONFIG_CHANNEL_MASK) {
        mChannelMask = config->channel_mask;
    }
    if (config->config_mask & AUDIO_PORT_CONFIG_FORMAT) {
        mFormat = config->format;
    }
    if (config->config_mask & AUDIO_PORT_CONFIG_GAIN) {
        mGain = config->gain;
    }
    if (config->config_mask & AUDIO_PORT_CONFIG_FLAGS) {
        mFlags = config->flags;
    }

exit:
    if (status != NO_ERROR) {
        applyAudioPortConfig(&localBackupConfig);
    }
    if (backupConfig != NULL) {
        *backupConfig = localBackupConfig;
    }
    return status;
}

namespace {

template<typename T>
void updateField(
        const T& portConfigField, T audio_port_config::*port_config_field,
        struct audio_port_config *dstConfig, const struct audio_port_config *srcConfig,
        unsigned int configMask, T defaultValue)
{
    if (dstConfig->config_mask & configMask) {
        if ((srcConfig != nullptr) && (srcConfig->config_mask & configMask)) {
            dstConfig->*port_config_field = srcConfig->*port_config_field;
        } else {
            dstConfig->*port_config_field = portConfigField;
        }
    } else {
        dstConfig->*port_config_field = defaultValue;
    }
}

} // namespace

void AudioPortConfig::toAudioPortConfig(struct audio_port_config *dstConfig,
                                        const struct audio_port_config *srcConfig) const
{
    updateField(mSamplingRate, &audio_port_config::sample_rate,
            dstConfig, srcConfig, AUDIO_PORT_CONFIG_SAMPLE_RATE, 0u);
    updateField(mChannelMask, &audio_port_config::channel_mask,
            dstConfig, srcConfig, AUDIO_PORT_CONFIG_CHANNEL_MASK,
            (audio_channel_mask_t)AUDIO_CHANNEL_NONE);
    updateField(mFormat, &audio_port_config::format,
            dstConfig, srcConfig, AUDIO_PORT_CONFIG_FORMAT, AUDIO_FORMAT_INVALID);

    sp<AudioPort> audioport = getAudioPort();
    if ((dstConfig->config_mask & AUDIO_PORT_CONFIG_GAIN) && audioport != NULL) {
        dstConfig->gain = mGain;
        if ((srcConfig != NULL) && (srcConfig->config_mask & AUDIO_PORT_CONFIG_GAIN)
                && audioport->checkGain(&srcConfig->gain, srcConfig->gain.index) == OK) {
            dstConfig->gain = srcConfig->gain;
        }
    } else {
        dstConfig->gain.index = -1;
    }
    if (dstConfig->gain.index != -1) {
        dstConfig->config_mask |= AUDIO_PORT_CONFIG_GAIN;
    } else {
        dstConfig->config_mask &= ~AUDIO_PORT_CONFIG_GAIN;
    }

    updateField(mFlags, &audio_port_config::flags,
            dstConfig, srcConfig, AUDIO_PORT_CONFIG_FLAGS, { AUDIO_INPUT_FLAG_NONE });
}

bool AudioPortConfig::hasGainController(bool canUseForVolume) const
{
    sp<AudioPort> audioport = getAudioPort();
    if (audioport == nullptr) {
        return false;
    }
    return canUseForVolume ? audioport->getGains().canUseForVolume()
                           : audioport->getGains().size() > 0;
}

} // namespace android
