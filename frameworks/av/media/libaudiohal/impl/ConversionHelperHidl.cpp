/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <string.h>

#define LOG_TAG "HalHidl"
#include <media/AudioParameter.h>
#include <utils/Log.h>

#include "ConversionHelperHidl.h"

namespace android {
namespace CPP_VERSION {

using namespace ::android::hardware::audio::common::CPP_VERSION;
using namespace ::android::hardware::audio::CPP_VERSION;

// static
status_t ConversionHelperHidl::keysFromHal(const String8& keys, hidl_vec<hidl_string> *hidlKeys) {
    AudioParameter halKeys(keys);
    if (halKeys.size() == 0) return BAD_VALUE;
    hidlKeys->resize(halKeys.size());
    //FIXME:  keyStreamSupportedChannels and keyStreamSupportedSamplingRates come with a
    // "keyFormat=<value>" pair. We need to transform it into a single key string so that it is
    // carried over to the legacy HAL via HIDL.
    String8 value;
    bool keepFormatValue = halKeys.size() == 2 &&
         (halKeys.get(String8(AudioParameter::keyStreamSupportedChannels), value) == NO_ERROR ||
         halKeys.get(String8(AudioParameter::keyStreamSupportedSamplingRates), value) == NO_ERROR);

    for (size_t i = 0; i < halKeys.size(); ++i) {
        String8 key;
        status_t status = halKeys.getAt(i, key);
        if (status != OK) return status;
        if (keepFormatValue && key == AudioParameter::keyFormat) {
            AudioParameter formatParam;
            halKeys.getAt(i, key, value);
            formatParam.add(key, value);
            key = formatParam.toString();
        }
        (*hidlKeys)[i] = key.string();
    }
    return OK;
}

// static
status_t ConversionHelperHidl::parametersFromHal(
        const String8& kvPairs, hidl_vec<ParameterValue> *hidlParams) {
    AudioParameter params(kvPairs);
    if (params.size() == 0) return BAD_VALUE;
    hidlParams->resize(params.size());
    for (size_t i = 0; i < params.size(); ++i) {
        String8 key, value;
        status_t status = params.getAt(i, key, value);
        if (status != OK) return status;
        (*hidlParams)[i].key = key.string();
        (*hidlParams)[i].value = value.string();
    }
    return OK;
}

// static
void ConversionHelperHidl::parametersToHal(
        const hidl_vec<ParameterValue>& parameters, String8 *values) {
    AudioParameter params;
    for (size_t i = 0; i < parameters.size(); ++i) {
        params.add(String8(parameters[i].key.c_str()), String8(parameters[i].value.c_str()));
    }
    values->setTo(params.toString());
}

ConversionHelperHidl::ConversionHelperHidl(const char* className)
        : mClassName(className) {
}

// static
status_t ConversionHelperHidl::analyzeResult(const Result& result) {
    switch (result) {
        case Result::OK: return OK;
        case Result::INVALID_ARGUMENTS: return BAD_VALUE;
        case Result::INVALID_STATE: return NOT_ENOUGH_DATA;
        case Result::NOT_INITIALIZED: return NO_INIT;
        case Result::NOT_SUPPORTED: return INVALID_OPERATION;
        default: return NO_INIT;
    }
}

void ConversionHelperHidl::emitError(const char* funcName, const char* description) {
    ALOGE("%s %p %s: %s (from rpc)", mClassName, this, funcName, description);
}

#if MAJOR_VERSION >= 4
// TODO: Use the same implementation in the hal when it moves to a util library.
static std::string deviceAddressToHal(const DeviceAddress& address) {
    // HAL assumes that the address is NUL-terminated.
    char halAddress[AUDIO_DEVICE_MAX_ADDRESS_LEN];
    memset(halAddress, 0, sizeof(halAddress));
    audio_devices_t halDevice = static_cast<audio_devices_t>(address.device);
    const bool isInput = (halDevice & AUDIO_DEVICE_BIT_IN) != 0;
    if (isInput) halDevice &= ~AUDIO_DEVICE_BIT_IN;
    if ((!isInput && (halDevice & AUDIO_DEVICE_OUT_ALL_A2DP) != 0) ||
        (isInput && (halDevice & AUDIO_DEVICE_IN_BLUETOOTH_A2DP) != 0)) {
        snprintf(halAddress, sizeof(halAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
                 address.address.mac[0], address.address.mac[1], address.address.mac[2],
                 address.address.mac[3], address.address.mac[4], address.address.mac[5]);
    } else if ((!isInput && (halDevice & AUDIO_DEVICE_OUT_IP) != 0) ||
               (isInput && (halDevice & AUDIO_DEVICE_IN_IP) != 0)) {
        snprintf(halAddress, sizeof(halAddress), "%d.%d.%d.%d", address.address.ipv4[0],
                 address.address.ipv4[1], address.address.ipv4[2], address.address.ipv4[3]);
    } else if ((!isInput && (halDevice & AUDIO_DEVICE_OUT_ALL_USB) != 0) ||
               (isInput && (halDevice & AUDIO_DEVICE_IN_ALL_USB) != 0)) {
        snprintf(halAddress, sizeof(halAddress), "card=%d;device=%d", address.address.alsa.card,
                 address.address.alsa.device);
    } else if ((!isInput && (halDevice & AUDIO_DEVICE_OUT_BUS) != 0) ||
               (isInput && (halDevice & AUDIO_DEVICE_IN_BUS) != 0)) {
        snprintf(halAddress, sizeof(halAddress), "%s", address.busAddress.c_str());
    } else if ((!isInput && (halDevice & AUDIO_DEVICE_OUT_REMOTE_SUBMIX)) != 0 ||
               (isInput && (halDevice & AUDIO_DEVICE_IN_REMOTE_SUBMIX) != 0)) {
        snprintf(halAddress, sizeof(halAddress), "%s", address.rSubmixAddress.c_str());
    } else {
        snprintf(halAddress, sizeof(halAddress), "%s", address.busAddress.c_str());
    }
    return halAddress;
}

//local conversion helpers

static audio_microphone_channel_mapping_t  channelMappingToHal(AudioMicrophoneChannelMapping mapping) {
    switch (mapping) {
        case AudioMicrophoneChannelMapping::UNUSED:
            return AUDIO_MICROPHONE_CHANNEL_MAPPING_UNUSED;
        case AudioMicrophoneChannelMapping::DIRECT:
            return AUDIO_MICROPHONE_CHANNEL_MAPPING_DIRECT;
        case AudioMicrophoneChannelMapping::PROCESSED:
            return AUDIO_MICROPHONE_CHANNEL_MAPPING_PROCESSED;
        default:
            LOG_ALWAYS_FATAL("Unknown channelMappingToHal conversion %d", mapping);
    }
}

static audio_microphone_location_t locationToHal(AudioMicrophoneLocation location) {
    switch (location) {
        case AudioMicrophoneLocation::UNKNOWN:
            return AUDIO_MICROPHONE_LOCATION_UNKNOWN;
        case AudioMicrophoneLocation::MAINBODY:
            return AUDIO_MICROPHONE_LOCATION_MAINBODY;
        case AudioMicrophoneLocation::MAINBODY_MOVABLE:
            return AUDIO_MICROPHONE_LOCATION_MAINBODY_MOVABLE;
        case AudioMicrophoneLocation::PERIPHERAL:
            return AUDIO_MICROPHONE_LOCATION_PERIPHERAL;
        default:
            LOG_ALWAYS_FATAL("Unknown locationToHal conversion %d", location);
    }
}
static audio_microphone_directionality_t directionalityToHal(AudioMicrophoneDirectionality dir) {
    switch (dir) {
        case AudioMicrophoneDirectionality::UNKNOWN:
            return AUDIO_MICROPHONE_DIRECTIONALITY_UNKNOWN;
        case AudioMicrophoneDirectionality::OMNI:
            return AUDIO_MICROPHONE_DIRECTIONALITY_OMNI;
        case AudioMicrophoneDirectionality::BI_DIRECTIONAL:
            return AUDIO_MICROPHONE_DIRECTIONALITY_BI_DIRECTIONAL;
        case AudioMicrophoneDirectionality::CARDIOID:
            return AUDIO_MICROPHONE_DIRECTIONALITY_CARDIOID;
        case AudioMicrophoneDirectionality::HYPER_CARDIOID:
            return AUDIO_MICROPHONE_DIRECTIONALITY_HYPER_CARDIOID;
        case AudioMicrophoneDirectionality::SUPER_CARDIOID:
            return AUDIO_MICROPHONE_DIRECTIONALITY_SUPER_CARDIOID;
        default:
            LOG_ALWAYS_FATAL("Unknown directionalityToHal conversion %d", dir);
    }
}

void microphoneInfoToHal(const MicrophoneInfo& src,
                         audio_microphone_characteristic_t *pDst) {
    if (pDst != NULL) {
        snprintf(pDst->device_id, sizeof(pDst->device_id),
                 "%s", src.deviceId.c_str());
        pDst->device = static_cast<audio_devices_t>(src.deviceAddress.device);
        snprintf(pDst->address, sizeof(pDst->address),
                 "%s", deviceAddressToHal(src.deviceAddress).c_str());
        if (src.channelMapping.size() > AUDIO_CHANNEL_COUNT_MAX) {
            ALOGW("microphoneInfoToStruct found %zu channelMapping elements. Max expected is %d",
                  src.channelMapping.size(), AUDIO_CHANNEL_COUNT_MAX);
        }
        size_t ch;
        for (ch = 0; ch < src.channelMapping.size() && ch < AUDIO_CHANNEL_COUNT_MAX; ch++) {
            pDst->channel_mapping[ch] = channelMappingToHal(src.channelMapping[ch]);
        }
        for (; ch < AUDIO_CHANNEL_COUNT_MAX; ch++) {
            pDst->channel_mapping[ch] = AUDIO_MICROPHONE_CHANNEL_MAPPING_UNUSED;
        }
        pDst->location = locationToHal(src.location);
        pDst->group = (audio_microphone_group_t)src.group;
        pDst->index_in_the_group = (unsigned int)src.indexInTheGroup;
        pDst->sensitivity = src.sensitivity;
        pDst->max_spl = src.maxSpl;
        pDst->min_spl = src.minSpl;
        pDst->directionality = directionalityToHal(src.directionality);
        pDst->num_frequency_responses = (unsigned int)src.frequencyResponse.size();
        if (pDst->num_frequency_responses > AUDIO_MICROPHONE_MAX_FREQUENCY_RESPONSES) {
            ALOGW("microphoneInfoToStruct found %d frequency responses. Max expected is %d",
                  pDst->num_frequency_responses, AUDIO_MICROPHONE_MAX_FREQUENCY_RESPONSES);
            pDst->num_frequency_responses = AUDIO_MICROPHONE_MAX_FREQUENCY_RESPONSES;
        }
        for (size_t k = 0; k < pDst->num_frequency_responses; k++) {
            pDst->frequency_responses[0][k] = src.frequencyResponse[k].frequency;
            pDst->frequency_responses[1][k] = src.frequencyResponse[k].level;
        }
        pDst->geometric_location.x = src.position.x;
        pDst->geometric_location.y = src.position.y;
        pDst->geometric_location.z = src.position.z;
        pDst->orientation.x = src.orientation.x;
        pDst->orientation.y = src.orientation.y;
        pDst->orientation.z = src.orientation.z;
    }
}
#endif

}  // namespace CPP_VERSION
}  // namespace android
