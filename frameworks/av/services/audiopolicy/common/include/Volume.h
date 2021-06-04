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

#include <media/AudioCommonTypes.h>
#include <system/audio.h>
#include <utils/Log.h>
#include <math.h>

namespace android {

/**
 * VolumeSource is the discriminent for volume management on an output.
 * It used to be the stream type by legacy, it may be host volume group or a volume curves if
 * we allow to have more than one curve per volume group (mandatory to get rid of AudioServer
 * stream aliases.
 */
enum VolumeSource : std::underlying_type<volume_group_t>::type;
static const VolumeSource VOLUME_SOURCE_NONE = static_cast<VolumeSource>(VOLUME_GROUP_NONE);

} // namespace android

// Absolute min volume in dB (can be represented in single precision normal float value)
#define VOLUME_MIN_DB (-758)

class VolumeCurvePoint
{
public:
    int mIndex;
    float mDBAttenuation;
};

/**
 * device categories used for volume curve management.
 */
enum device_category {
    DEVICE_CATEGORY_HEADSET,
    DEVICE_CATEGORY_SPEAKER,
    DEVICE_CATEGORY_EARPIECE,
    DEVICE_CATEGORY_EXT_MEDIA,
    DEVICE_CATEGORY_HEARING_AID,
    DEVICE_CATEGORY_CNT
};

class Volume
{
public:
    /**
     * 4 points to define the volume attenuation curve, each characterized by the volume
     * index (from 0 to 100) at which they apply, and the attenuation in dB at that index.
     * we use 100 steps to avoid rounding errors when computing the volume in volIndexToDb()
     *
     * @todo shall become configurable
     */
    enum {
        VOLMIN = 0,
        VOLKNEE1 = 1,
        VOLKNEE2 = 2,
        VOLMAX = 3,

        VOLCNT = 4
    };

    /**
     * extract one device relevant for volume control from multiple device selection
     *
     * @param[in] device for which the volume category is associated
     *
     * @return subset of device required to limit the number of volume category per device
     */
    static audio_devices_t getDeviceForVolume(audio_devices_t device)
    {
        if (device == AUDIO_DEVICE_NONE) {
            // this happens when forcing a route update and no track is active on an output.
            // In this case the returned category is not important.
            device =  AUDIO_DEVICE_OUT_SPEAKER;
        } else if (popcount(device) > 1) {
            // Multiple device selection is either:
            //  - speaker + one other device: give priority to speaker in this case.
            //  - one A2DP device + another device: happens with duplicated output. In this case
            // retain the device on the A2DP output as the other must not correspond to an active
            // selection if not the speaker.
            //  - HDMI-CEC system audio mode only output: give priority to available item in order.
            if (device & AUDIO_DEVICE_OUT_SPEAKER) {
                device = AUDIO_DEVICE_OUT_SPEAKER;
            } else if (device & AUDIO_DEVICE_OUT_SPEAKER_SAFE) {
                device = AUDIO_DEVICE_OUT_SPEAKER_SAFE;
            } else if (device & AUDIO_DEVICE_OUT_HDMI_ARC) {
                device = AUDIO_DEVICE_OUT_HDMI_ARC;
            } else if (device & AUDIO_DEVICE_OUT_AUX_LINE) {
                device = AUDIO_DEVICE_OUT_AUX_LINE;
            } else if (device & AUDIO_DEVICE_OUT_SPDIF) {
                device = AUDIO_DEVICE_OUT_SPDIF;
            } else {
                device = (audio_devices_t)(device & AUDIO_DEVICE_OUT_ALL_A2DP);
            }
        }

        /*SPEAKER_SAFE is an alias of SPEAKER for purposes of volume control*/
        if (device == AUDIO_DEVICE_OUT_SPEAKER_SAFE)
            device = AUDIO_DEVICE_OUT_SPEAKER;

        ALOGW_IF(popcount(device) != 1,
                 "getDeviceForVolume() invalid device combination: %08x",
                 device);

        return device;
    }

    /**
     * returns the category the device belongs to with regard to volume curve management
     *
     * @param[in] device to check upon the category to whom it belongs to.
     *
     * @return device category.
     */
    static device_category getDeviceCategory(audio_devices_t device)
    {
        switch(getDeviceForVolume(device)) {
        case AUDIO_DEVICE_OUT_EARPIECE:
            return DEVICE_CATEGORY_EARPIECE;
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
        case AUDIO_DEVICE_OUT_USB_HEADSET:
            return DEVICE_CATEGORY_HEADSET;
        case AUDIO_DEVICE_OUT_HEARING_AID:
            return DEVICE_CATEGORY_HEARING_AID;
        case AUDIO_DEVICE_OUT_LINE:
        case AUDIO_DEVICE_OUT_AUX_DIGITAL:
        case AUDIO_DEVICE_OUT_USB_DEVICE:
            return DEVICE_CATEGORY_EXT_MEDIA;
        case AUDIO_DEVICE_OUT_SPEAKER:
        case AUDIO_DEVICE_OUT_SPEAKER_SAFE:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
        case AUDIO_DEVICE_OUT_USB_ACCESSORY:
        case AUDIO_DEVICE_OUT_REMOTE_SUBMIX:
        default:
            return DEVICE_CATEGORY_SPEAKER;
        }
    }

    static inline float DbToAmpl(float decibels)
    {
        if (decibels <= VOLUME_MIN_DB) {
            return 0.0f;
        }
        return exp( decibels * 0.115129f); // exp( dB * ln(10) / 20 )
    }

    static inline float AmplToDb(float amplification)
    {
        if (amplification == 0) {
            return VOLUME_MIN_DB;
        }
        return 20 * log10(amplification);
    }

};
