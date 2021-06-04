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

#define LOG_TAG "APM_AudioPolicyMix"
//#define LOG_NDEBUG 0

#include "AudioPolicyMix.h"
#include "TypeConverter.h"
#include "HwModule.h"
#include "AudioPort.h"
#include "IOProfile.h"
#include "AudioGain.h"
#include <AudioOutputDescriptor.h>

namespace android {

void AudioPolicyMix::dump(String8 *dst, int spaces, int index) const
{
    dst->appendFormat("%*sAudio Policy Mix %d:\n", spaces, "", index + 1);
    std::string mixTypeLiteral;
    if (!MixTypeConverter::toString(mMixType, mixTypeLiteral)) {
        ALOGE("%s: failed to convert mix type %d", __FUNCTION__, mMixType);
        return;
    }
    dst->appendFormat("%*s- mix type: %s\n", spaces, "", mixTypeLiteral.c_str());

    std::string routeFlagLiteral;
    RouteFlagTypeConverter::maskToString(mRouteFlags, routeFlagLiteral);
    dst->appendFormat("%*s- Route Flags: %s\n", spaces, "", routeFlagLiteral.c_str());

    dst->appendFormat("%*s- device type: %s\n", spaces, "", toString(mDeviceType).c_str());

    dst->appendFormat("%*s- device address: %s\n", spaces, "", mDeviceAddress.string());

    int indexCriterion = 0;
    for (const auto &criterion : mCriteria) {
        dst->appendFormat("%*s- Criterion %d: ", spaces + 2, "", indexCriterion++);

        std::string ruleType, ruleValue;
        bool unknownRule = !RuleTypeConverter::toString(criterion.mRule, ruleType);
        switch (criterion.mRule & ~RULE_EXCLUSION_MASK) { // no need to match RULE_EXCLUDE_...
        case RULE_MATCH_ATTRIBUTE_USAGE:
            UsageTypeConverter::toString(criterion.mValue.mUsage, ruleValue);
            break;
        case RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET:
            SourceTypeConverter::toString(criterion.mValue.mSource, ruleValue);
            break;
        case RULE_MATCH_UID:
            ruleValue = std::to_string(criterion.mValue.mUid);
            break;
        default:
            unknownRule = true;
        }

        if (!unknownRule) {
            dst->appendFormat("%s %s\n", ruleType.c_str(), ruleValue.c_str());
        } else {
            dst->appendFormat("Unknown rule type value 0x%x\n", criterion.mRule);
        }
    }
}

status_t AudioPolicyMixCollection::registerMix(AudioMix mix, sp<SwAudioOutputDescriptor> desc)
{
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioPolicyMix>& registeredMix = itemAt(i);
        if (mix.mDeviceType == registeredMix->mDeviceType
                && mix.mDeviceAddress.compare(registeredMix->mDeviceAddress) == 0) {
            ALOGE("registerMix(): mix already registered for dev=0x%x addr=%s",
                    mix.mDeviceType, mix.mDeviceAddress.string());
            return BAD_VALUE;
        }
    }
    sp<AudioPolicyMix> policyMix = new AudioPolicyMix(mix);
    add(policyMix);
    ALOGD("registerMix(): adding mix for dev=0x%x addr=%s",
            policyMix->mDeviceType, policyMix->mDeviceAddress.string());

    if (desc != 0) {
        desc->mPolicyMix = policyMix;
        policyMix->setOutput(desc);
    }
    return NO_ERROR;
}

status_t AudioPolicyMixCollection::unregisterMix(const AudioMix& mix)
{
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioPolicyMix>& registeredMix = itemAt(i);
        if (mix.mDeviceType == registeredMix->mDeviceType
                && mix.mDeviceAddress.compare(registeredMix->mDeviceAddress) == 0) {
            ALOGD("unregisterMix(): removing mix for dev=0x%x addr=%s",
                    mix.mDeviceType, mix.mDeviceAddress.string());
            removeAt(i);
            return NO_ERROR;
        }
    }

    ALOGE("unregisterMix(): mix not registered for dev=0x%x addr=%s",
            mix.mDeviceType, mix.mDeviceAddress.string());
    return BAD_VALUE;
}

status_t AudioPolicyMixCollection::getAudioPolicyMix(audio_devices_t deviceType,
        const String8& address, sp<AudioPolicyMix> &policyMix) const
{

    ALOGV("getAudioPolicyMix() for dev=0x%x addr=%s", deviceType, address.string());
    for (ssize_t i = 0; i < size(); i++) {
        // Workaround: when an in audio policy is registered, it opens an output
        // that tries to find the audio policy, thus the device must be ignored.
        if (itemAt(i)->mDeviceAddress.compare(address) == 0) {
            policyMix = itemAt(i);
            ALOGV("getAudioPolicyMix: found mix %zu match (devType=0x%x addr=%s)",
                    i, deviceType, address.string());
            return NO_ERROR;
        }
    }

    ALOGE("getAudioPolicyMix(): mix not registered for dev=0x%x addr=%s",
            deviceType, address.string());
    return BAD_VALUE;
}

void AudioPolicyMixCollection::closeOutput(sp<SwAudioOutputDescriptor> &desc)
{
    for (size_t i = 0; i < size(); i++) {
        sp<AudioPolicyMix> policyMix = itemAt(i);
        if (policyMix->getOutput() == desc) {
            policyMix->clearOutput();
        }
    }
}

status_t AudioPolicyMixCollection::getOutputForAttr(
        const audio_attributes_t& attributes, uid_t uid,
        audio_output_flags_t flags,
        sp<SwAudioOutputDescriptor> &primaryDesc,
        std::vector<sp<SwAudioOutputDescriptor>> *secondaryDescs)
{
    ALOGV("getOutputForAttr() querying %zu mixes:", size());
    primaryDesc = 0;
    for (size_t i = 0; i < size(); i++) {
        sp<AudioPolicyMix> policyMix = itemAt(i);
        const bool primaryOutputMix = !is_mix_loopback_render(policyMix->mRouteFlags);
        if (!primaryOutputMix && (flags & AUDIO_OUTPUT_FLAG_MMAP_NOIRQ)) {
            // AAudio does not support MMAP_NO_IRQ loopback render, and there is no way with
            // the current MmapStreamInterface::start to reject a specific client added to a shared
            // mmap stream.
            // As a result all MMAP_NOIRQ requests have to be rejected when an loopback render
            // policy is present. That ensures no shared mmap stream is used when an loopback
            // render policy is registered.
            ALOGD("%s: Rejecting MMAP_NOIRQ request due to LOOPBACK|RENDER mix present.", __func__);
            return INVALID_OPERATION;
        }

        sp<SwAudioOutputDescriptor> policyDesc = policyMix->getOutput();
        if (!policyDesc) {
            ALOGV("%s: Skiping %zu: Mix has no output", __func__, i);
            continue;
        }

        if (primaryOutputMix && primaryDesc != 0) {
            ALOGV("%s: Skiping %zu: Primary output already found", __func__, i);
            continue; // Primary output already found
        }

        switch (mixMatch(policyMix.get(), i, attributes, uid)) {
            case MixMatchStatus::INVALID_MIX:
                // The mix has contradictory rules, ignore it
                // TODO: reject invalid mix at registration
                continue;
            case MixMatchStatus::NO_MATCH:
                ALOGV("%s: Mix %zu: does not match", __func__, i);
                continue; // skip the mix
            case MixMatchStatus::MATCH:;
        }

        policyDesc->mPolicyMix = policyMix;
        if (primaryOutputMix) {
            primaryDesc = policyDesc;
            ALOGV("%s: Mix %zu: set primary desc", __func__, i);
        } else {
            if (policyDesc->mIoHandle == AUDIO_IO_HANDLE_NONE) {
                ALOGV("%s: Mix %zu ignored as secondaryOutput because not opened yet", __func__, i);
            } else {
                ALOGV("%s: Add a secondary desc %zu", __func__, i);
                secondaryDescs->push_back(policyDesc);
            }
        }
    }
    return NO_ERROR;
}

AudioPolicyMixCollection::MixMatchStatus AudioPolicyMixCollection::mixMatch(
        const AudioMix* mix, size_t mixIndex, const audio_attributes_t& attributes, uid_t uid) {

    if (mix->mMixType == MIX_TYPE_PLAYERS) {
        // Loopback render mixes are created from a public API and thus restricted
        // to non sensible audio that have not opted out.
        if (is_mix_loopback_render(mix->mRouteFlags)) {
            auto hasFlag = [](auto flags, auto flag) { return (flags & flag) == flag; };
            if (hasFlag(attributes.flags, AUDIO_FLAG_NO_SYSTEM_CAPTURE)) {
                return MixMatchStatus::NO_MATCH;
            }
            if (!mix->mAllowPrivilegedPlaybackCapture &&
                hasFlag(attributes.flags, AUDIO_FLAG_NO_MEDIA_PROJECTION)) {
                return MixMatchStatus::NO_MATCH;
            }
            if (!(attributes.usage == AUDIO_USAGE_UNKNOWN ||
                  attributes.usage == AUDIO_USAGE_MEDIA ||
                  attributes.usage == AUDIO_USAGE_GAME)) {
                return MixMatchStatus::NO_MATCH;
            }
        }
        // TODO if adding more player rules (currently only 2), make rule handling "generic"
        //      as there is no difference in the treatment of usage- or uid-based rules
        bool hasUsageMatchRules = false;
        bool hasUsageExcludeRules = false;
        bool usageMatchFound = false;
        bool usageExclusionFound = false;

        bool hasUidMatchRules = false;
        bool hasUidExcludeRules = false;
        bool uidMatchFound = false;
        bool uidExclusionFound = false;

        bool hasAddrMatch = false;

        // iterate over all mix criteria to list what rules this mix contains
        for (size_t j = 0; j < mix->mCriteria.size(); j++) {
            ALOGV(" getOutputForAttr: mix %zu: inspecting mix criteria %zu of %zu",
                    mixIndex, j, mix->mCriteria.size());

            // if there is an address match, prioritize that match
            if (strncmp(attributes.tags, "addr=", strlen("addr=")) == 0 &&
                    strncmp(attributes.tags + strlen("addr="),
                            mix->mDeviceAddress.string(),
                            AUDIO_ATTRIBUTES_TAGS_MAX_SIZE - strlen("addr=") - 1) == 0) {
                hasAddrMatch = true;
                break;
            }

            switch (mix->mCriteria[j].mRule) {
            case RULE_MATCH_ATTRIBUTE_USAGE:
                ALOGV("\tmix has RULE_MATCH_ATTRIBUTE_USAGE for usage %d",
                                            mix->mCriteria[j].mValue.mUsage);
                hasUsageMatchRules = true;
                if (mix->mCriteria[j].mValue.mUsage == attributes.usage) {
                    // found one match against all allowed usages
                    usageMatchFound = true;
                }
                break;
            case RULE_EXCLUDE_ATTRIBUTE_USAGE:
                ALOGV("\tmix has RULE_EXCLUDE_ATTRIBUTE_USAGE for usage %d",
                        mix->mCriteria[j].mValue.mUsage);
                hasUsageExcludeRules = true;
                if (mix->mCriteria[j].mValue.mUsage == attributes.usage) {
                    // found this usage is to be excluded
                    usageExclusionFound = true;
                }
                break;
            case RULE_MATCH_UID:
                ALOGV("\tmix has RULE_MATCH_UID for uid %d", mix->mCriteria[j].mValue.mUid);
                hasUidMatchRules = true;
                if (mix->mCriteria[j].mValue.mUid == uid) {
                    // found one UID match against all allowed UIDs
                    uidMatchFound = true;
                }
                break;
            case RULE_EXCLUDE_UID:
                ALOGV("\tmix has RULE_EXCLUDE_UID for uid %d", mix->mCriteria[j].mValue.mUid);
                hasUidExcludeRules = true;
                if (mix->mCriteria[j].mValue.mUid == uid) {
                    // found this UID is to be excluded
                    uidExclusionFound = true;
                }
                break;
            default:
                break;
            }

            // consistency checks: for each "dimension" of rules (usage, uid...), we can
            // only have MATCH rules, or EXCLUDE rules in each dimension, not a combination
            if (hasUsageMatchRules && hasUsageExcludeRules) {
                ALOGE("getOutputForAttr: invalid combination of RULE_MATCH_ATTRIBUTE_USAGE"
                        " and RULE_EXCLUDE_ATTRIBUTE_USAGE in mix %zu", mixIndex);
                return MixMatchStatus::INVALID_MIX;
            }
            if (hasUidMatchRules && hasUidExcludeRules) {
                ALOGE("getOutputForAttr: invalid combination of RULE_MATCH_UID"
                        " and RULE_EXCLUDE_UID in mix %zu", mixIndex);
                return MixMatchStatus::INVALID_MIX;
            }

            if ((hasUsageExcludeRules && usageExclusionFound)
                    || (hasUidExcludeRules && uidExclusionFound)) {
                break; // stop iterating on criteria because an exclusion was found (will fail)
            }

        }//iterate on mix criteria

        // determine if exiting on success (or implicit failure as desc is 0)
        if (hasAddrMatch ||
                !((hasUsageExcludeRules && usageExclusionFound) ||
                  (hasUsageMatchRules && !usageMatchFound)  ||
                  (hasUidExcludeRules && uidExclusionFound) ||
                  (hasUidMatchRules && !uidMatchFound))) {
            ALOGV("\tgetOutputForAttr will use mix %zu", mixIndex);
            return MixMatchStatus::MATCH;
        }

    } else if (mix->mMixType == MIX_TYPE_RECORDERS) {
        if (attributes.usage == AUDIO_USAGE_VIRTUAL_SOURCE &&
                strncmp(attributes.tags, "addr=", strlen("addr=")) == 0 &&
                strncmp(attributes.tags + strlen("addr="),
                        mix->mDeviceAddress.string(),
                        AUDIO_ATTRIBUTES_TAGS_MAX_SIZE - strlen("addr=") - 1) == 0) {
            return MixMatchStatus::MATCH;
        }
    }
    return MixMatchStatus::NO_MATCH;
}

sp<DeviceDescriptor> AudioPolicyMixCollection::getDeviceAndMixForOutput(
        const sp<SwAudioOutputDescriptor> &output,
        const DeviceVector &availableOutputDevices)
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->getOutput() == output) {
            // This Desc is involved in a Mix, which has the highest prio
            audio_devices_t deviceType = itemAt(i)->mDeviceType;
            String8 address = itemAt(i)->mDeviceAddress;
            ALOGV("%s: device (0x%x, addr=%s) forced by mix",
                  __FUNCTION__, deviceType, address.c_str());
            return availableOutputDevices.getDevice(deviceType, address, AUDIO_FORMAT_DEFAULT);
        }
    }
    return nullptr;
}

sp<DeviceDescriptor> AudioPolicyMixCollection::getDeviceAndMixForInputSource(
        audio_source_t inputSource,
        const DeviceVector &availDevices,
        sp<AudioPolicyMix> *policyMix) const
{
    for (size_t i = 0; i < size(); i++) {
        AudioPolicyMix *mix = itemAt(i).get();
        if (mix->mMixType != MIX_TYPE_RECORDERS) {
            continue;
        }
        for (size_t j = 0; j < mix->mCriteria.size(); j++) {
            if ((RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET == mix->mCriteria[j].mRule &&
                    mix->mCriteria[j].mValue.mSource == inputSource) ||
               (RULE_EXCLUDE_ATTRIBUTE_CAPTURE_PRESET == mix->mCriteria[j].mRule &&
                    mix->mCriteria[j].mValue.mSource != inputSource)) {
                // assuming PolicyMix only for remote submix for input
                // so mix->mDeviceType can only be AUDIO_DEVICE_OUT_REMOTE_SUBMIX
                audio_devices_t device = AUDIO_DEVICE_IN_REMOTE_SUBMIX;
                auto mixDevice =
                        availDevices.getDevice(device, mix->mDeviceAddress, AUDIO_FORMAT_DEFAULT);
                if (mixDevice != nullptr) {
                    if (policyMix != nullptr) {
                        *policyMix = mix;
                    }
                    return mixDevice;
                }
                break;
            }
        }
    }
    return nullptr;
}

status_t AudioPolicyMixCollection::getInputMixForAttr(
        audio_attributes_t attr, sp<AudioPolicyMix> *policyMix)
{
    if (strncmp(attr.tags, "addr=", strlen("addr=")) != 0) {
        return BAD_VALUE;
    }
    String8 address(attr.tags + strlen("addr="));

#ifdef LOG_NDEBUG
    ALOGV("getInputMixForAttr looking for address %s for source %d\n  mixes available:",
            address.string(), attr.source);
    for (size_t i = 0; i < size(); i++) {
        const sp<AudioPolicyMix> audioPolicyMix = itemAt(i);
        ALOGV("\tmix %zu address=%s", i, audioPolicyMix->mDeviceAddress.string());
    }
#endif

    size_t index;
    for (index = 0; index < size(); index++) {
        const sp<AudioPolicyMix>& registeredMix = itemAt(index);
        if (registeredMix->mDeviceAddress.compare(address) == 0) {
            ALOGD("getInputMixForAttr found addr=%s dev=0x%x",
                    registeredMix->mDeviceAddress.string(), registeredMix->mDeviceType);
            break;
        }
    }
    if (index == size()) {
        ALOGW("getInputMixForAttr() no policy for address %s", address.string());
        return BAD_VALUE;
    }
    const sp<AudioPolicyMix> audioPolicyMix = itemAt(index);

    if (audioPolicyMix->mMixType != MIX_TYPE_PLAYERS) {
        ALOGW("getInputMixForAttr() bad policy mix type for address %s", address.string());
        return BAD_VALUE;
    }
    if (policyMix != nullptr) {
        *policyMix = audioPolicyMix;
    }
    return NO_ERROR;
}

status_t AudioPolicyMixCollection::setUidDeviceAffinities(uid_t uid,
        const Vector<AudioDeviceTypeAddr>& devices) {
    // verify feasibility: for each player mix: if it already contains a
    //    "match uid" rule for this uid, return an error
    //    (adding a uid-device affinity would result in contradictory rules)
    for (size_t i = 0; i < size(); i++) {
        const AudioPolicyMix* mix = itemAt(i).get();
        if (!mix->isDeviceAffinityCompatible()) {
            continue;
        }
        if (mix->hasUidRule(true /*match*/, uid)) {
            return INVALID_OPERATION;
        }
    }

    // remove existing rules for this uid
    removeUidDeviceAffinities(uid);

    // for each player mix:
    //   IF    device is not a target for the mix,
    //     AND it doesn't have a "match uid" rule
    //   THEN add a rule to exclude the uid
    for (size_t i = 0; i < size(); i++) {
        const AudioPolicyMix *mix = itemAt(i).get();
        if (!mix->isDeviceAffinityCompatible()) {
            continue;
        }
        // check if this mix goes to a device in the list of devices
        bool deviceMatch = false;
        for (size_t j = 0; j < devices.size(); j++) {
            if (devices[j].mType == mix->mDeviceType
                    && devices[j].mAddress == mix->mDeviceAddress) {
                deviceMatch = true;
                break;
            }
        }
        if (!deviceMatch && !mix->hasMatchUidRule()) {
            // this mix doesn't go to one of the listed devices for the given uid,
            // and it's not already restricting the mix on a uid,
            // modify its rules to exclude the uid
            if (!mix->hasUidRule(false /*match*/, uid)) {
                // no need to do it again if uid is already excluded
                mix->setExcludeUid(uid);
            }
        }
    }

    return NO_ERROR;
}

status_t AudioPolicyMixCollection::removeUidDeviceAffinities(uid_t uid) {
    // for each player mix: remove existing rules that match or exclude this uid
    for (size_t i = 0; i < size(); i++) {
        bool foundUidRule = false;
        const AudioPolicyMix *mix = itemAt(i).get();
        if (!mix->isDeviceAffinityCompatible()) {
            continue;
        }
        std::vector<size_t> criteriaToRemove;
        for (size_t j = 0; j < mix->mCriteria.size(); j++) {
            const uint32_t rule = mix->mCriteria[j].mRule;
            // is this rule excluding the uid? (not considering uid match rules
            // as those are not used for uid-device affinity)
            if (rule == RULE_EXCLUDE_UID
                    && uid == mix->mCriteria[j].mValue.mUid) {
                foundUidRule = true;
                criteriaToRemove.insert(criteriaToRemove.begin(), j);
            }
        }
        if (foundUidRule) {
            for (size_t j = 0; j < criteriaToRemove.size(); j++) {
                mix->mCriteria.removeAt(criteriaToRemove[j]);
            }
        }
    }
    return NO_ERROR;
}

status_t AudioPolicyMixCollection::getDevicesForUid(uid_t uid,
        Vector<AudioDeviceTypeAddr>& devices) const {
    // for each player mix: find rules that don't exclude this uid, and add the device to the list
    for (size_t i = 0; i < size(); i++) {
        bool ruleAllowsUid = true;
        const AudioPolicyMix *mix = itemAt(i).get();
        if (mix->mMixType != MIX_TYPE_PLAYERS) {
            continue;
        }
        for (size_t j = 0; j < mix->mCriteria.size(); j++) {
            const uint32_t rule = mix->mCriteria[j].mRule;
            if (rule == RULE_EXCLUDE_UID
                    && uid == mix->mCriteria[j].mValue.mUid) {
                ruleAllowsUid = false;
                break;
            }
        }
        if (ruleAllowsUid) {
            devices.add(AudioDeviceTypeAddr(mix->mDeviceType, mix->mDeviceAddress));
        }
    }
    return NO_ERROR;
}

void AudioPolicyMixCollection::dump(String8 *dst) const
{
    dst->append("\nAudio Policy Mix:\n");
    for (size_t i = 0; i < size(); i++) {
        itemAt(i)->dump(dst, 2, i);
    }
}

}; //namespace android
