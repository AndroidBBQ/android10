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

#define LOG_TAG "APM::AudioPolicyEngine/InputSource"

#include "InputSource.h"

using std::string;

namespace android {
namespace audio_policy {

status_t Element<audio_source_t>::setIdentifier(audio_source_t identifier)
{
    if (identifier > AUDIO_SOURCE_MAX && identifier != AUDIO_SOURCE_HOTWORD
        && identifier != AUDIO_SOURCE_FM_TUNER && identifier != AUDIO_SOURCE_ECHO_REFERENCE) {
        return BAD_VALUE;
    }
    mIdentifier = identifier;
    ALOGD("%s: InputSource %s identifier 0x%X", __FUNCTION__, getName().c_str(), identifier);
    return NO_ERROR;
}

/**
* Set the device associated to this source.
* It checks if the input device is valid.
*
* @param[in] devices selected for the given input source.
* @tparam audio_devices_t: Applicable input device for this input source.
*
* @return NO_ERROR if the device is either valid or none, error code otherwise.
*/
template <>
status_t Element<audio_source_t>::set(audio_devices_t devices)
{
    if (devices != AUDIO_DEVICE_NONE) {
        devices |= AUDIO_DEVICE_BIT_IN;
    }
    if (!audio_is_input_device(devices)) {
        ALOGE("%s: trying to set an invalid device 0x%X for input source %s",
              __FUNCTION__, devices, getName().c_str());
        return BAD_VALUE;
    }
    ALOGD("%s: 0x%X for input source %s", __FUNCTION__, devices, getName().c_str());
    mApplicableDevices = devices;
    return NO_ERROR;
}

template <>
audio_devices_t Element<audio_source_t>::get<audio_devices_t>() const
{
    ALOGV("%s: 0x%X for inputSource %s", __FUNCTION__, mApplicableDevices, getName().c_str());
    return mApplicableDevices;
}
} // namespace audio_policy
} // namespace android


