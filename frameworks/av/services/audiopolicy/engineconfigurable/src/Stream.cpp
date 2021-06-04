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

#define LOG_TAG "APM::AudioPolicyEngine/Stream"

#include "Stream.h"
#include <system/audio.h>

using std::string;

namespace android {
namespace audio_policy {

status_t Element<audio_stream_type_t>::setIdentifier(audio_stream_type_t identifier)
{
    if (identifier > AUDIO_STREAM_CNT) {
        return BAD_VALUE;
    }
    mIdentifier = identifier;
    ALOGD("%s: Stream %s identifier 0x%X", __FUNCTION__, getName().c_str(), identifier);
    return NO_ERROR;
}

template <>
status_t Element<audio_stream_type_t>::set<audio_stream_type_t>(audio_stream_type_t volumeProfile)
{
    if (volumeProfile >= AUDIO_STREAM_CNT) {
        return BAD_VALUE;
    }
    mVolumeProfile = volumeProfile;
    ALOGD("%s: 0x%X for Stream %s", __FUNCTION__, mVolumeProfile, getName().c_str());
    return NO_ERROR;
}

template <>
audio_stream_type_t Element<audio_stream_type_t>::get<audio_stream_type_t>() const
{
    ALOGV("%s: 0x%X for Stream %s", __FUNCTION__, mVolumeProfile, getName().c_str());
    return mVolumeProfile;
}

} // namespace audio_policy
} // namespace android

