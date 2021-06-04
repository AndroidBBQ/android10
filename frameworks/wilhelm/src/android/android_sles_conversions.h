/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "math.h"

#include <system/audio.h>

//-----------------------------------------------------------------------------
// OpenSL ES to Android
//----------------------

static inline uint32_t sles_to_android_sampleRate(SLuint32 sampleRateMilliHertz) {
    return (uint32_t)(sampleRateMilliHertz / 1000);
}

static inline audio_format_t sles_to_android_sampleFormat(const SLDataFormat_PCM *df_pcm) {
    if (df_pcm->containerSize != df_pcm->bitsPerSample) {
        return AUDIO_FORMAT_INVALID;
    }
    switch (df_pcm->formatType) {
    case SL_DATAFORMAT_PCM:
        switch (df_pcm->containerSize) {
        case 8:
            return AUDIO_FORMAT_PCM_8_BIT;
        case 16:
            return AUDIO_FORMAT_PCM_16_BIT;
        case 24:
            return AUDIO_FORMAT_PCM_24_BIT_PACKED;
        case 32:
            return AUDIO_FORMAT_PCM_32_BIT;
        default:
            return AUDIO_FORMAT_INVALID;
        }
    case SL_ANDROID_DATAFORMAT_PCM_EX:
        switch (((SLAndroidDataFormat_PCM_EX *)df_pcm)->representation) {
        case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
            switch (df_pcm->containerSize) {
            case 8:
                return AUDIO_FORMAT_PCM_8_BIT;
            default:
                return AUDIO_FORMAT_INVALID;
            }
        case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
            switch (df_pcm->containerSize) {
            case 16:
                return AUDIO_FORMAT_PCM_16_BIT;
            case 24:
                return AUDIO_FORMAT_PCM_24_BIT_PACKED;
            case 32:
                return AUDIO_FORMAT_PCM_32_BIT;
            default:
                return AUDIO_FORMAT_INVALID;
            }
        case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
            switch (df_pcm->containerSize) {
            case 32:
                return AUDIO_FORMAT_PCM_FLOAT;
            default:
                return AUDIO_FORMAT_INVALID;
            }
        default:
            return AUDIO_FORMAT_INVALID;
        }
    default:
        return AUDIO_FORMAT_INVALID;
    }
}


static inline float sles_to_android_amplification(SLmillibel level) {
    // FIXME use the FX Framework conversions
    return pow(10, (float)level/2000);
}
