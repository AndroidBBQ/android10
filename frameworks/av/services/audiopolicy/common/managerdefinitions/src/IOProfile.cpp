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

#define LOG_TAG "APM::IOProfile"
//#define LOG_NDEBUG 0

#include <system/audio-base.h>
#include "IOProfile.h"
#include "HwModule.h"
#include "AudioGain.h"
#include "TypeConverter.h"

namespace android {

bool IOProfile::isCompatibleProfile(const DeviceVector &devices,
                                    uint32_t samplingRate,
                                    uint32_t *updatedSamplingRate,
                                    audio_format_t format,
                                    audio_format_t *updatedFormat,
                                    audio_channel_mask_t channelMask,
                                    audio_channel_mask_t *updatedChannelMask,
                                    // FIXME type punning here
                                    uint32_t flags,
                                    bool exactMatchRequiredForInputFlags) const
{
    const bool isPlaybackThread =
            getType() == AUDIO_PORT_TYPE_MIX && getRole() == AUDIO_PORT_ROLE_SOURCE;
    const bool isRecordThread =
            getType() == AUDIO_PORT_TYPE_MIX && getRole() == AUDIO_PORT_ROLE_SINK;
    ALOG_ASSERT(isPlaybackThread != isRecordThread);

    if (!devices.isEmpty()) {
        if (!mSupportedDevices.containsAllDevices(devices)) {
            return false;
        }
    }

    if (!audio_is_valid_format(format) ||
            (isPlaybackThread && (samplingRate == 0 || !audio_is_output_channel(channelMask))) ||
            (isRecordThread && (!audio_is_input_channel(channelMask)))) {
         return false;
    }

    audio_format_t myUpdatedFormat = format;
    audio_channel_mask_t myUpdatedChannelMask = channelMask;
    uint32_t myUpdatedSamplingRate = samplingRate;
    const struct audio_port_config config = {
        .config_mask = AUDIO_PORT_CONFIG_ALL & ~AUDIO_PORT_CONFIG_GAIN,
        .sample_rate = samplingRate,
        .channel_mask = channelMask,
        .format = format,
    };
    if (isRecordThread)
    {
        if ((flags & AUDIO_INPUT_FLAG_MMAP_NOIRQ) != 0) {
            if (checkExactAudioProfile(&config) != NO_ERROR) {
                return false;
            }
        } else if (checkCompatibleAudioProfile(
                myUpdatedSamplingRate, myUpdatedChannelMask, myUpdatedFormat) != NO_ERROR) {
            return false;
        }
    } else {
        if (checkExactAudioProfile(&config) != NO_ERROR) {
            return false;
        }
    }

    if (isPlaybackThread && (getFlags() & flags) != flags) {
        return false;
    }
    // The only input flag that is allowed to be different is the fast flag.
    // An existing fast stream is compatible with a normal track request.
    // An existing normal stream is compatible with a fast track request,
    // but the fast request will be denied by AudioFlinger and converted to normal track.
    if (isRecordThread && ((getFlags() ^ flags) &
            ~(exactMatchRequiredForInputFlags ? AUDIO_INPUT_FLAG_NONE : AUDIO_INPUT_FLAG_FAST))) {
        return false;
    }

    if (updatedSamplingRate != NULL) {
        *updatedSamplingRate = myUpdatedSamplingRate;
    }
    if (updatedFormat != NULL) {
        *updatedFormat = myUpdatedFormat;
    }
    if (updatedChannelMask != NULL) {
        *updatedChannelMask = myUpdatedChannelMask;
    }
    return true;
}

void IOProfile::dump(String8 *dst) const
{
    AudioPort::dump(dst, 4);

    dst->appendFormat("    - flags: 0x%04x", getFlags());
    std::string flagsLiteral;
    if (getRole() == AUDIO_PORT_ROLE_SINK) {
        InputFlagConverter::maskToString(getFlags(), flagsLiteral);
    } else if (getRole() == AUDIO_PORT_ROLE_SOURCE) {
        OutputFlagConverter::maskToString(getFlags(), flagsLiteral);
    }
    if (!flagsLiteral.empty()) {
        dst->appendFormat(" (%s)", flagsLiteral.c_str());
    }
    dst->append("\n");
    mSupportedDevices.dump(dst, String8("Supported"), 4, false);
    dst->appendFormat("\n    - maxOpenCount: %u - curOpenCount: %u\n",
             maxOpenCount, curOpenCount);
    dst->appendFormat("    - maxActiveCount: %u - curActiveCount: %u\n",
             maxActiveCount, curActiveCount);
}

void IOProfile::log()
{
    // @TODO: forward log to AudioPort
}

} // namespace android
