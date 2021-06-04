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

#ifndef ANDROID_CHANNELS_H_
#define ANDROID_CHANNELS_H_

#include <system/audio.h>
#include <SLES/OpenSLES.h>

#define SL_ANDROID_SPEAKER_COUNT_MAX            FCC_8
#define SL_ANDROID_INDEXED_SPEAKER_MASK_ALL     ((SLuint32) \
                                                ((1 << SL_ANDROID_SPEAKER_COUNT_MAX) - 1))
#define SL_ANDROID_POSITIONAL_SPEAKER_MASK_ALL  \
        ( SL_SPEAKER_FRONT_LEFT \
        | SL_SPEAKER_FRONT_RIGHT \
        | SL_SPEAKER_FRONT_CENTER \
        | SL_SPEAKER_LOW_FREQUENCY \
        | SL_SPEAKER_BACK_LEFT \
        | SL_SPEAKER_BACK_RIGHT \
        | SL_SPEAKER_FRONT_LEFT_OF_CENTER \
        | SL_SPEAKER_FRONT_RIGHT_OF_CENTER \
        | SL_SPEAKER_BACK_CENTER \
        | SL_SPEAKER_SIDE_LEFT \
        | SL_SPEAKER_SIDE_RIGHT \
        | SL_SPEAKER_TOP_CENTER \
        | SL_SPEAKER_TOP_FRONT_LEFT \
        | SL_SPEAKER_TOP_FRONT_CENTER \
        | SL_SPEAKER_TOP_FRONT_RIGHT \
        | SL_SPEAKER_TOP_BACK_LEFT \
        | SL_SPEAKER_TOP_BACK_CENTER \
        | SL_SPEAKER_TOP_BACK_RIGHT )

// Channel count and channel mask definitions

#define SL_ANDROID_SPEAKER_QUAD (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT \
        | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT)

#define SL_ANDROID_SPEAKER_5DOT1 (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT \
        | SL_SPEAKER_FRONT_CENTER  | SL_SPEAKER_LOW_FREQUENCY| SL_SPEAKER_BACK_LEFT \
        | SL_SPEAKER_BACK_RIGHT)

#define SL_ANDROID_SPEAKER_7DOT1 (SL_ANDROID_SPEAKER_5DOT1 | SL_SPEAKER_SIDE_LEFT \
        | SL_SPEAKER_SIDE_RIGHT)

extern SLuint32 sles_channel_out_mask_from_count(unsigned channelCount);
extern SLuint32 sles_channel_in_mask_from_count(unsigned channelCount);
extern SLuint32 sles_channel_count_from_mask(SLuint32 mask);
extern SLboolean sles_is_channel_mask_valid(SLuint32 mask);

extern audio_channel_representation_t sles_to_audio_channel_mask_representation(SLuint32 mask);
extern audio_channel_mask_t sles_to_audio_output_channel_mask(SLuint32 mask);
extern audio_channel_mask_t sles_to_audio_input_channel_mask(SLuint32 mask);

#define SL_ANDROID_UNKNOWN_CHANNELMASK 0

#endif // ANDROID_CHANNELS_H_
