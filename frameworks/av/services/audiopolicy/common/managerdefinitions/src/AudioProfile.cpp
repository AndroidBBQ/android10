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

#include <algorithm>
#include <set>
#include <string>

#define LOG_TAG "APM::AudioProfile"
//#define LOG_NDEBUG 0

#include <media/AudioResamplerPublic.h>
#include <utils/Errors.h>

#include "AudioGain.h"
#include "AudioPort.h"
#include "AudioProfile.h"
#include "HwModule.h"
#include "TypeConverter.h"

namespace android {

ChannelsVector ChannelsVector::asInMask() const
{
    ChannelsVector inMaskVector;
    for (const auto& channel : *this) {
        if (audio_channel_mask_out_to_in(channel) != AUDIO_CHANNEL_INVALID) {
            inMaskVector.add(audio_channel_mask_out_to_in(channel));
        }
    }
    return inMaskVector;
}

ChannelsVector ChannelsVector::asOutMask() const
{
    ChannelsVector outMaskVector;
    for (const auto& channel : *this) {
        if (audio_channel_mask_in_to_out(channel) != AUDIO_CHANNEL_INVALID) {
            outMaskVector.add(audio_channel_mask_in_to_out(channel));
        }
    }
    return outMaskVector;
}

bool operator == (const AudioProfile &left, const AudioProfile &compareTo)
{
    return (left.getFormat() == compareTo.getFormat()) &&
            (left.getChannels() == compareTo.getChannels()) &&
            (left.getSampleRates() == compareTo.getSampleRates());
}

static AudioProfile* createFullDynamicImpl()
{
    AudioProfile* dynamicProfile = new AudioProfile(gDynamicFormat,
            ChannelsVector(), SampleRateVector());
    dynamicProfile->setDynamicFormat(true);
    dynamicProfile->setDynamicChannels(true);
    dynamicProfile->setDynamicRate(true);
    return dynamicProfile;
}

// static
sp<AudioProfile> AudioProfile::createFullDynamic()
{
    static sp<AudioProfile> dynamicProfile = createFullDynamicImpl();
    return dynamicProfile;
}

AudioProfile::AudioProfile(audio_format_t format,
                           audio_channel_mask_t channelMasks,
                           uint32_t samplingRate) :
        mName(String8("")),
        mFormat(format)
{
    mChannelMasks.add(channelMasks);
    mSamplingRates.add(samplingRate);
}

AudioProfile::AudioProfile(audio_format_t format,
                           const ChannelsVector &channelMasks,
                           const SampleRateVector &samplingRateCollection) :
        mName(String8("")),
        mFormat(format),
        mChannelMasks(channelMasks),
        mSamplingRates(samplingRateCollection) {}

void AudioProfile::setChannels(const ChannelsVector &channelMasks)
{
    if (mIsDynamicChannels) {
        mChannelMasks = channelMasks;
    }
}

void AudioProfile::setSampleRates(const SampleRateVector &sampleRates)
{
    if (mIsDynamicRate) {
        mSamplingRates = sampleRates;
    }
}

void AudioProfile::clear()
{
    if (mIsDynamicChannels) {
        mChannelMasks.clear();
    }
    if (mIsDynamicRate) {
        mSamplingRates.clear();
    }
}

status_t AudioProfile::checkExact(uint32_t samplingRate, audio_channel_mask_t channelMask,
                                  audio_format_t format) const
{
    if (audio_formats_match(format, mFormat) &&
            supportsChannels(channelMask) &&
            supportsRate(samplingRate)) {
        return NO_ERROR;
    }
    return BAD_VALUE;
}

status_t AudioProfile::checkCompatibleSamplingRate(uint32_t samplingRate,
                                                   uint32_t &updatedSamplingRate) const
{
    ALOG_ASSERT(samplingRate > 0);

    if (mSamplingRates.isEmpty()) {
        updatedSamplingRate = samplingRate;
        return NO_ERROR;
    }

    // Search for the closest supported sampling rate that is above (preferred)
    // or below (acceptable) the desired sampling rate, within a permitted ratio.
    // The sampling rates are sorted in ascending order.
    size_t orderOfDesiredRate = mSamplingRates.orderOf(samplingRate);

    // Prefer to down-sample from a higher sampling rate, as we get the desired frequency spectrum.
    if (orderOfDesiredRate < mSamplingRates.size()) {
        uint32_t candidate = mSamplingRates[orderOfDesiredRate];
        if (candidate / AUDIO_RESAMPLER_DOWN_RATIO_MAX <= samplingRate) {
            updatedSamplingRate = candidate;
            return NO_ERROR;
        }
    }
    // But if we have to up-sample from a lower sampling rate, that's OK.
    if (orderOfDesiredRate != 0) {
        uint32_t candidate = mSamplingRates[orderOfDesiredRate - 1];
        if (candidate * AUDIO_RESAMPLER_UP_RATIO_MAX >= samplingRate) {
            updatedSamplingRate = candidate;
            return NO_ERROR;
        }
    }
    // leave updatedSamplingRate unmodified
    return BAD_VALUE;
}

status_t AudioProfile::checkCompatibleChannelMask(audio_channel_mask_t channelMask,
                                                  audio_channel_mask_t &updatedChannelMask,
                                                  audio_port_type_t portType,
                                                  audio_port_role_t portRole) const
{
    if (mChannelMasks.isEmpty()) {
        updatedChannelMask = channelMask;
        return NO_ERROR;
    }
    const bool isRecordThread = portType == AUDIO_PORT_TYPE_MIX && portRole == AUDIO_PORT_ROLE_SINK;
    const bool isIndex = audio_channel_mask_get_representation(channelMask)
            == AUDIO_CHANNEL_REPRESENTATION_INDEX;
    const uint32_t channelCount = audio_channel_count_from_in_mask(channelMask);
    int bestMatch = 0;
    for (size_t i = 0; i < mChannelMasks.size(); i ++) {
        audio_channel_mask_t supported = mChannelMasks[i];
        if (supported == channelMask) {
            // Exact matches always taken.
            updatedChannelMask = channelMask;
            return NO_ERROR;
        }

        // AUDIO_CHANNEL_NONE (value: 0) is used for dynamic channel support
        if (isRecordThread && supported != AUDIO_CHANNEL_NONE) {
            // Approximate (best) match:
            // The match score measures how well the supported channel mask matches the
            // desired mask, where increasing-is-better.
            //
            // TODO: Some tweaks may be needed.
            // Should be a static function of the data processing library.
            //
            // In priority:
            // match score = 1000 if legacy channel conversion equivalent (always prefer this)
            // OR
            // match score += 100 if the channel mask representations match
            // match score += number of channels matched.
            // match score += 100 if the channel mask representations DO NOT match
            //   but the profile has positional channel mask and less than 2 channels.
            //   This is for audio HAL convention to not list index masks for less than 2 channels
            //
            // If there are no matched channels, the mask may still be accepted
            // but the playback or record will be silent.
            const bool isSupportedIndex = (audio_channel_mask_get_representation(supported)
                    == AUDIO_CHANNEL_REPRESENTATION_INDEX);
            const uint32_t supportedChannelCount = audio_channel_count_from_in_mask(supported);
            int match;
            if (isIndex && isSupportedIndex) {
                // index equivalence
                match = 100 + __builtin_popcount(
                        audio_channel_mask_get_bits(channelMask)
                            & audio_channel_mask_get_bits(supported));
            } else if (isIndex && !isSupportedIndex) {
                const uint32_t equivalentBits = (1 << supportedChannelCount) - 1 ;
                match = __builtin_popcount(
                        audio_channel_mask_get_bits(channelMask) & equivalentBits);
                if (supportedChannelCount <= FCC_2) {
                    match += 100;
                }
            } else if (!isIndex && isSupportedIndex) {
                const uint32_t equivalentBits = (1 << channelCount) - 1;
                match = __builtin_popcount(
                        equivalentBits & audio_channel_mask_get_bits(supported));
            } else {
                // positional equivalence
                match = 100 + __builtin_popcount(
                        audio_channel_mask_get_bits(channelMask)
                            & audio_channel_mask_get_bits(supported));
                switch (supported) {
                case AUDIO_CHANNEL_IN_FRONT_BACK:
                case AUDIO_CHANNEL_IN_STEREO:
                    if (channelMask == AUDIO_CHANNEL_IN_MONO) {
                        match = 1000;
                    }
                    break;
                case AUDIO_CHANNEL_IN_MONO:
                    if (channelMask == AUDIO_CHANNEL_IN_FRONT_BACK
                            || channelMask == AUDIO_CHANNEL_IN_STEREO) {
                        match = 1000;
                    }
                    break;
                default:
                    break;
                }
            }
            if (match > bestMatch) {
                bestMatch = match;
                updatedChannelMask = supported;
            }
        }
    }
    return bestMatch > 0 ? NO_ERROR : BAD_VALUE;
}

void AudioProfile::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("%s%s%s\n", mIsDynamicFormat ? "[dynamic format]" : "",
             mIsDynamicChannels ? "[dynamic channels]" : "",
             mIsDynamicRate ? "[dynamic rates]" : "");
    if (mName.length() != 0) {
        dst->appendFormat("%*s- name: %s\n", spaces, "", mName.string());
    }
    std::string formatLiteral;
    if (FormatConverter::toString(mFormat, formatLiteral)) {
        dst->appendFormat("%*s- format: %s\n", spaces, "", formatLiteral.c_str());
    }
    if (!mSamplingRates.isEmpty()) {
        dst->appendFormat("%*s- sampling rates:", spaces, "");
        for (size_t i = 0; i < mSamplingRates.size(); i++) {
            dst->appendFormat("%d", mSamplingRates[i]);
            dst->append(i == (mSamplingRates.size() - 1) ? "" : ", ");
        }
        dst->append("\n");
    }

    if (!mChannelMasks.isEmpty()) {
        dst->appendFormat("%*s- channel masks:", spaces, "");
        for (size_t i = 0; i < mChannelMasks.size(); i++) {
            dst->appendFormat("0x%04x", mChannelMasks[i]);
            dst->append(i == (mChannelMasks.size() - 1) ? "" : ", ");
        }
        dst->append("\n");
    }
}

ssize_t AudioProfileVector::add(const sp<AudioProfile> &profile)
{
    ssize_t index = Vector::add(profile);
    // we sort from worst to best, so that AUDIO_FORMAT_DEFAULT is always the first entry.
    // TODO: compareFormats could be a lambda to convert between pointer-to-format to format:
    // [](const audio_format_t *format1, const audio_format_t *format2) {
    //     return compareFormats(*format1, *format2);
    // }
    sort(compareFormats);
    return index;
}

ssize_t AudioProfileVector::addProfileFromHal(const sp<AudioProfile> &profileToAdd)
{
    // Check valid profile to add:
    if (!profileToAdd->hasValidFormat()) {
        return -1;
    }
    if (!profileToAdd->hasValidChannels() && !profileToAdd->hasValidRates()) {
        FormatVector formats;
        formats.add(profileToAdd->getFormat());
        setFormats(FormatVector(formats));
        return 0;
    }
    if (!profileToAdd->hasValidChannels() && profileToAdd->hasValidRates()) {
        setSampleRatesFor(profileToAdd->getSampleRates(), profileToAdd->getFormat());
        return 0;
    }
    if (profileToAdd->hasValidChannels() && !profileToAdd->hasValidRates()) {
        setChannelsFor(profileToAdd->getChannels(), profileToAdd->getFormat());
        return 0;
    }
    // Go through the list of profile to avoid duplicates
    for (size_t profileIndex = 0; profileIndex < size(); profileIndex++) {
        const sp<AudioProfile> &profile = itemAt(profileIndex);
        if (profile->isValid() && profile == profileToAdd) {
            // Nothing to do
            return profileIndex;
        }
    }
    profileToAdd->setDynamicFormat(true); // set the format as dynamic to allow removal
    return add(profileToAdd);
}

status_t AudioProfileVector::checkExactProfile(uint32_t samplingRate,
                                               audio_channel_mask_t channelMask,
                                               audio_format_t format) const
{
    if (isEmpty()) {
        return NO_ERROR;
    }

    for (const auto& profile : *this) {
        if (profile->checkExact(samplingRate, channelMask, format) == NO_ERROR) {
            return NO_ERROR;
        }
    }
    return BAD_VALUE;
}

status_t AudioProfileVector::checkCompatibleProfile(uint32_t &samplingRate,
                                                    audio_channel_mask_t &channelMask,
                                                    audio_format_t &format,
                                                    audio_port_type_t portType,
                                                    audio_port_role_t portRole) const
{
    if (isEmpty()) {
        return NO_ERROR;
    }

    const bool checkInexact = // when port is input and format is linear pcm
            portType == AUDIO_PORT_TYPE_MIX && portRole == AUDIO_PORT_ROLE_SINK
            && audio_is_linear_pcm(format);

    // iterate from best format to worst format (reverse order)
    for (ssize_t i = size() - 1; i >= 0 ; --i) {
        const sp<AudioProfile> profile = itemAt(i);
        audio_format_t formatToCompare = profile->getFormat();
        if (formatToCompare == format ||
                (checkInexact
                        && formatToCompare != AUDIO_FORMAT_DEFAULT
                        && audio_is_linear_pcm(formatToCompare))) {
            // Compatible profile has been found, checks if this profile has compatible
            // rate and channels as well
            audio_channel_mask_t updatedChannels;
            uint32_t updatedRate;
            if (profile->checkCompatibleChannelMask(channelMask, updatedChannels,
                                                    portType, portRole) == NO_ERROR &&
                    profile->checkCompatibleSamplingRate(samplingRate, updatedRate) == NO_ERROR) {
                // for inexact checks we take the first linear pcm format due to sorting.
                format = formatToCompare;
                channelMask = updatedChannels;
                samplingRate = updatedRate;
                return NO_ERROR;
            }
        }
    }
    return BAD_VALUE;
}

void AudioProfileVector::clearProfiles()
{
    for (size_t i = size(); i != 0; ) {
        sp<AudioProfile> profile = itemAt(--i);
        if (profile->isDynamicFormat() && profile->hasValidFormat()) {
            removeAt(i);
            continue;
        }
        profile->clear();
    }
}

// Returns an intersection between two possibly unsorted vectors and the contents of 'order'.
// The result is ordered according to 'order'.
template<typename T, typename Order>
std::vector<typename T::value_type> intersectFilterAndOrder(
        const T& input1, const T& input2, const Order& order)
{
    std::set<typename T::value_type> set1{input1.begin(), input1.end()};
    std::set<typename T::value_type> set2{input2.begin(), input2.end()};
    std::set<typename T::value_type> common;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
            std::inserter(common, common.begin()));
    std::vector<typename T::value_type> result;
    for (const auto& e : order) {
        if (common.find(e) != common.end()) result.push_back(e);
    }
    return result;
}

// Intersect two possibly unsorted vectors, return common elements according to 'comp' ordering.
// 'comp' is a comparator function.
template<typename T, typename Compare>
std::vector<typename T::value_type> intersectAndOrder(
        const T& input1, const T& input2, Compare comp)
{
    std::set<typename T::value_type, Compare> set1{input1.begin(), input1.end(), comp};
    std::set<typename T::value_type, Compare> set2{input2.begin(), input2.end(), comp};
    std::vector<typename T::value_type> result;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
            std::back_inserter(result), comp);
    return result;
}

status_t AudioProfileVector::findBestMatchingOutputConfig(const AudioProfileVector& outputProfiles,
            const std::vector<audio_format_t>& preferredFormats,
            const std::vector<audio_channel_mask_t>& preferredOutputChannels,
            bool preferHigherSamplingRates,
            audio_config_base *bestOutputConfig) const
{
    auto formats = intersectFilterAndOrder(getSupportedFormats(),
            outputProfiles.getSupportedFormats(), preferredFormats);
    // Pick the best compatible profile.
    for (const auto& f : formats) {
        sp<AudioProfile> inputProfile = getFirstValidProfileFor(f);
        sp<AudioProfile> outputProfile = outputProfiles.getFirstValidProfileFor(f);
        if (inputProfile == nullptr || outputProfile == nullptr) {
            continue;
        }
        auto channels = intersectFilterAndOrder(inputProfile->getChannels().asOutMask(),
                outputProfile->getChannels(), preferredOutputChannels);
        if (channels.empty()) {
            continue;
        }
        auto sampleRates = preferHigherSamplingRates ?
                intersectAndOrder(inputProfile->getSampleRates(), outputProfile->getSampleRates(),
                        std::greater<typename SampleRateVector::value_type>()) :
                intersectAndOrder(inputProfile->getSampleRates(), outputProfile->getSampleRates(),
                        std::less<typename SampleRateVector::value_type>());
        if (sampleRates.empty()) {
            continue;
        }
        ALOGD("%s() found channel mask %#x and sample rate %d for format %#x.",
                __func__, *channels.begin(), *sampleRates.begin(), f);
        bestOutputConfig->format = f;
        bestOutputConfig->sample_rate = *sampleRates.begin();
        bestOutputConfig->channel_mask = *channels.begin();
        return NO_ERROR;
    }
    return BAD_VALUE;
}

sp<AudioProfile> AudioProfileVector::getFirstValidProfile() const
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->isValid()) {
            return itemAt(i);
        }
    }
    return 0;
}

sp<AudioProfile> AudioProfileVector::getFirstValidProfileFor(audio_format_t format) const
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->isValid() && itemAt(i)->getFormat() == format) {
            return itemAt(i);
        }
    }
    return 0;
}

FormatVector AudioProfileVector::getSupportedFormats() const
{
    FormatVector supportedFormats;
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->hasValidFormat()) {
            supportedFormats.add(itemAt(i)->getFormat());
        }
    }
    return supportedFormats;
}

bool AudioProfileVector::hasDynamicChannelsFor(audio_format_t format) const
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioProfile> profile = itemAt(i);
        if (profile->getFormat() == format && profile->isDynamicChannels()) {
            return true;
        }
    }
    return false;
}

bool AudioProfileVector::hasDynamicProfile() const
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->isDynamic()) {
            return true;
        }
    }
    return false;
}

bool AudioProfileVector::hasDynamicRateFor(audio_format_t format) const
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioProfile> profile = itemAt(i);
        if (profile->getFormat() == format && profile->isDynamicRate()) {
            return true;
        }
    }
    return false;
}

void AudioProfileVector::setFormats(const FormatVector &formats)
{
    // Only allow to change the format of dynamic profile
    sp<AudioProfile> dynamicFormatProfile = getProfileFor(gDynamicFormat);
    if (dynamicFormatProfile == 0) {
        return;
    }
    for (size_t i = 0; i < formats.size(); i++) {
        sp<AudioProfile> profile = new AudioProfile(formats[i],
                dynamicFormatProfile->getChannels(),
                dynamicFormatProfile->getSampleRates());
        profile->setDynamicFormat(true);
        profile->setDynamicChannels(dynamicFormatProfile->isDynamicChannels());
        profile->setDynamicRate(dynamicFormatProfile->isDynamicRate());
        add(profile);
    }
}

void AudioProfileVector::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("%*s- Profiles:\n", spaces, "");
    for (size_t i = 0; i < size(); i++) {
        dst->appendFormat("%*sProfile %zu:", spaces + 4, "", i);
        itemAt(i)->dump(dst, spaces + 8);
    }
}

sp<AudioProfile> AudioProfileVector::getProfileFor(audio_format_t format) const
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->getFormat() == format) {
            return itemAt(i);
        }
    }
    return 0;
}

void AudioProfileVector::setSampleRatesFor(
        const SampleRateVector &sampleRates, audio_format_t format)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioProfile> profile = itemAt(i);
        if (profile->getFormat() == format && profile->isDynamicRate()) {
            if (profile->hasValidRates()) {
                // Need to create a new profile with same format
                sp<AudioProfile> profileToAdd = new AudioProfile(format, profile->getChannels(),
                        sampleRates);
                profileToAdd->setDynamicFormat(true); // need to set to allow cleaning
                add(profileToAdd);
            } else {
                profile->setSampleRates(sampleRates);
            }
            return;
        }
    }
}

void AudioProfileVector::setChannelsFor(const ChannelsVector &channelMasks, audio_format_t format)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioProfile> profile = itemAt(i);
        if (profile->getFormat() == format && profile->isDynamicChannels()) {
            if (profile->hasValidChannels()) {
                // Need to create a new profile with same format
                sp<AudioProfile> profileToAdd = new AudioProfile(format, channelMasks,
                        profile->getSampleRates());
                profileToAdd->setDynamicFormat(true); // need to set to allow cleaning
                add(profileToAdd);
            } else {
                profile->setChannels(channelMasks);
            }
            return;
        }
    }
}

// static
int AudioProfileVector::compareFormats(const sp<AudioProfile> *profile1,
                                       const sp<AudioProfile> *profile2)
{
    return AudioPort::compareFormats((*profile1)->getFormat(), (*profile2)->getFormat());
}

} // namespace android
