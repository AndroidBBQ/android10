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

#include "sles_allinclusive.h"
#include <system/audio.h>
#include <SLES/OpenSLES_Android.h>
#include "channels.h"


/*
 * Return the default OpenSL ES output channel mask (as used in SLDataFormat_PCM.channelMask)
 * for the specified channel count.
 *
 * OpenSL ES makes no distinction between input and output channel masks, but
 * Android does. This is the OUTPUT version of this function.
 */
SLuint32 sles_channel_out_mask_from_count(unsigned channelCount)
{
    // FIXME channel mask is not yet implemented by Stagefright, so use a reasonable default
    //       that is computed from the channel count
    if (channelCount > FCC_8) {
        return SL_ANDROID_UNKNOWN_CHANNELMASK;
    }
    switch (channelCount) {
    case 1:
        // see explanation in data.c re: default channel mask for mono
        return SL_SPEAKER_FRONT_LEFT;
    case 2:
        return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    // Android-specific
    case 3:
        return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER;
    case 4:
        return SL_ANDROID_SPEAKER_QUAD;
    case 5:
        return SL_ANDROID_SPEAKER_QUAD | SL_SPEAKER_FRONT_CENTER;
    case 6:
        return SL_ANDROID_SPEAKER_5DOT1;
    case 7:
        return SL_ANDROID_SPEAKER_5DOT1 | SL_SPEAKER_BACK_CENTER;
    case 8:
        return SL_ANDROID_SPEAKER_7DOT1;
    // FIXME FCC_8
    default:
        return SL_ANDROID_UNKNOWN_CHANNELMASK;
    }
}

/*
 * Return the default OpenSL ES input channel mask (as used in SLDataFormat_PCM.channelMask)
 * for the specified channel count.
 *
 * OpenSL ES makes no distinction between input and output channel masks, but
 * Android does. This is the INPUT version of this function.
 */
SLuint32 sles_channel_in_mask_from_count(unsigned channelCount) {
    switch (channelCount) {
        case 1:
            return SL_SPEAKER_FRONT_LEFT;
        case 2:
            return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        default: {
            if (channelCount > AUDIO_CHANNEL_COUNT_MAX) {
                return SL_ANDROID_UNKNOWN_CHANNELMASK;
            } else {
                SLuint32 bitfield = (1 << channelCount) - 1;
                return SL_ANDROID_MAKE_INDEXED_CHANNEL_MASK(bitfield);
            }
        }

    }
}

/*
 * Get the number of active channels in an OpenSL ES channel mask.
 *
 * This function is valid for both input and output
 * masks.
 */
SLuint32 sles_channel_count_from_mask(SLuint32 mask) {
    audio_channel_representation_t rep
        = sles_to_audio_channel_mask_representation(mask);

    if (rep == AUDIO_CHANNEL_REPRESENTATION_INDEX) {
        mask &= SL_ANDROID_INDEXED_SPEAKER_MASK_ALL;
        return popcount(mask);
    } else if (rep == AUDIO_CHANNEL_REPRESENTATION_POSITION){
        mask &= SL_ANDROID_POSITIONAL_SPEAKER_MASK_ALL;
        return popcount(mask);
    } else {
        return 0;
    }
}

/*
 * Helper to determine whether a channel mask is indexed or not.
 *
 * This is the OpenSL ES analog to audio_channel_mask_get_representation().
 */
audio_channel_representation_t sles_to_audio_channel_mask_representation(SLuint32 mask) {
    if (mask & SL_ANDROID_SPEAKER_NON_POSITIONAL) {
        return AUDIO_CHANNEL_REPRESENTATION_INDEX;
    } else {
        return AUDIO_CHANNEL_REPRESENTATION_POSITION;
    }
}

// helper struct for the two static arrays which follow.
struct channel_map {
    SLuint32 sles;
    audio_channel_mask_t android;
};

// In practice this map is unnecessary, because the SL definitions just
// happen to match the android definitions perfectly, but we can't rely
// on that fact since the two sets of definitions have different API
// contracts.
static const struct channel_map output_map[] = {
    { SL_SPEAKER_FRONT_LEFT, AUDIO_CHANNEL_OUT_FRONT_LEFT },
    { SL_SPEAKER_FRONT_RIGHT, AUDIO_CHANNEL_OUT_FRONT_RIGHT },
    { SL_SPEAKER_FRONT_CENTER, AUDIO_CHANNEL_OUT_FRONT_CENTER },
    { SL_SPEAKER_LOW_FREQUENCY, AUDIO_CHANNEL_OUT_LOW_FREQUENCY },
    { SL_SPEAKER_BACK_LEFT, AUDIO_CHANNEL_OUT_BACK_LEFT },
    { SL_SPEAKER_BACK_RIGHT, AUDIO_CHANNEL_OUT_BACK_RIGHT },
    { SL_SPEAKER_FRONT_LEFT_OF_CENTER, AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER },
    { SL_SPEAKER_FRONT_RIGHT_OF_CENTER, AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER },
    { SL_SPEAKER_BACK_CENTER, AUDIO_CHANNEL_OUT_BACK_CENTER },
    { SL_SPEAKER_SIDE_LEFT, AUDIO_CHANNEL_OUT_SIDE_LEFT },
    { SL_SPEAKER_SIDE_RIGHT, AUDIO_CHANNEL_OUT_SIDE_RIGHT },
    { SL_SPEAKER_TOP_CENTER, AUDIO_CHANNEL_OUT_TOP_CENTER },
    { SL_SPEAKER_TOP_FRONT_LEFT, AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT },
    { SL_SPEAKER_TOP_FRONT_CENTER, AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER },
    { SL_SPEAKER_TOP_FRONT_RIGHT, AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT },
    { SL_SPEAKER_TOP_BACK_LEFT, AUDIO_CHANNEL_OUT_TOP_BACK_LEFT },
    { SL_SPEAKER_TOP_BACK_CENTER, AUDIO_CHANNEL_OUT_TOP_BACK_CENTER },
    { SL_SPEAKER_TOP_BACK_RIGHT, AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT },
};
static const unsigned int nOutputChannelMappings = sizeof(output_map) / sizeof(output_map[0]);

// This map is quite sparse, because there really isn't a reasonable mapping
// between most of the SL_SPEAKER bits and the android input map. It's probably
// best to use channel indices instead.
static const struct channel_map input_map[] = {
        { SL_SPEAKER_FRONT_LEFT, AUDIO_CHANNEL_IN_LEFT },
        { SL_SPEAKER_FRONT_RIGHT, AUDIO_CHANNEL_IN_RIGHT },
};
static const unsigned int nInputChannelMappings = sizeof(input_map) / sizeof(input_map[0]);

// Core channel mask mapper; implementation common to both input and output
static audio_channel_mask_t sles_to_android_mask_helper(
        SLuint32 mask,
        const struct channel_map* map,
        unsigned int nMappings) {
    if (!sles_is_channel_mask_valid(mask)) {
        SL_LOGW("Channel mask %#x is invalid because it uses bits that are undefined.", mask);
        return AUDIO_CHANNEL_INVALID;
    }

    // determine whether this mask uses positional or indexed representation
    audio_channel_representation_t rep = sles_to_audio_channel_mask_representation(mask);

    uint32_t bitsOut = 0;
    uint32_t bitsIn = mask;
    if (rep == AUDIO_CHANNEL_REPRESENTATION_INDEX) {
        // Indexed masks need no mapping
        bitsIn &= SL_ANDROID_INDEXED_SPEAKER_MASK_ALL;
        bitsOut = bitsIn;
    } else if (rep == AUDIO_CHANNEL_REPRESENTATION_POSITION){
        // positional masks get mapped from OpenSLES speaker definitions
        // to the channel definitions we use internally.
        bitsIn &= SL_ANDROID_POSITIONAL_SPEAKER_MASK_ALL;
        for (unsigned int i = 0; i < nMappings; ++i) {
            if (bitsIn & map[i].sles) {
                bitsOut |= map[i].android;
            }
        }
    } else {
        SL_LOGE("Unrecognized channel representation %#x", rep);
    }

    uint32_t result = audio_channel_mask_from_representation_and_bits(
            rep,
            bitsOut);

    if (popcount(bitsIn) != popcount(bitsOut)) {
        // At this point mask has already been stripped of the
        // representation bitsOut, so its bitcount should equal the number
        // of channels requested. If the bitcount of 'bitsOut' isn't
        // the same, then we're unable to provide the number of
        // channels that the app requested. That will cause an
        // error downstream if the app doesn't correct it, so
        // issue a warning here.
        SL_LOGW("Conversion from OpenSL ES %s channel mask %#x to Android mask %#x %s channels",
                (rep == AUDIO_CHANNEL_REPRESENTATION_POSITION) ? "positional" : "indexed",
                mask,
                result,
                (popcount(bitsIn) < popcount(bitsOut)) ? "gains" : "loses");
    }

    return result;
}

/*
 * Return an android output channel mask, as used in the AudioTrack constructor.
 */
audio_channel_mask_t sles_to_audio_output_channel_mask(SLuint32 mask) {
    return sles_to_android_mask_helper(mask, output_map, nOutputChannelMappings);
}

/*
 * Return an android input channel mask, as used in the AudioRecord constructor.
 */
audio_channel_mask_t sles_to_audio_input_channel_mask(SLuint32 mask) {
    return sles_to_android_mask_helper(mask, input_map, nInputChannelMappings);
}

/*
 * Check the mask for undefined bits (that is, set bits that don't correspond to a channel).
 *
 * Returns SL_BOOLEAN_TRUE if no undefined bits are set; SL_BOOLEAN_FALSE otherwise.
 */
SLboolean sles_is_channel_mask_valid(SLuint32 mask) {
    SLuint32 undefinedMask;
    if (sles_to_audio_channel_mask_representation(mask) == AUDIO_CHANNEL_REPRESENTATION_POSITION) {
        undefinedMask =  ~SL_ANDROID_POSITIONAL_SPEAKER_MASK_ALL;
    } else {
        undefinedMask
                = ~(SL_ANDROID_MAKE_INDEXED_CHANNEL_MASK(SL_ANDROID_INDEXED_SPEAKER_MASK_ALL));
    }
    return (mask & undefinedMask) ? SL_BOOLEAN_FALSE : SL_BOOLEAN_TRUE;
}
