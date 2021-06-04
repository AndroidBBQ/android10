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

#include "sles_allinclusive.h"

/* Device table (change this when you port!) */

static const SLAudioInputDescriptor AudioInputDescriptor_mic = {
    (SLchar *) "mic",            // deviceName
    SL_DEVCONNECTION_INTEGRATED, // deviceConnection
    SL_DEVSCOPE_ENVIRONMENT,     // deviceScope
    SL_DEVLOCATION_HANDSET,      // deviceLocation
    SL_BOOLEAN_TRUE,             // isForTelephony
    SL_SAMPLINGRATE_44_1,        // minSampleRate
    SL_SAMPLINGRATE_44_1,        // maxSampleRate
    SL_BOOLEAN_TRUE,             // isFreqRangeContinuous
    NULL,                        // samplingRatesSupported
    0,                           // numOfSamplingRatesSupported
    1                            // maxChannels
};

const struct AudioInput_id_descriptor AudioInput_id_descriptors[] = {
    {SL_DEFAULTDEVICEID_AUDIOINPUT, &AudioInputDescriptor_mic},
    {0, NULL}
};

static const SLAudioOutputDescriptor AudioOutputDescriptor_speaker = {
    (SLchar *) "speaker",        // deviceName
    SL_DEVCONNECTION_INTEGRATED, // deviceConnection
    SL_DEVSCOPE_USER,            // deviceScope
    SL_DEVLOCATION_HEADSET,      // deviceLocation
    SL_BOOLEAN_TRUE,             // isForTelephony
    SL_SAMPLINGRATE_44_1,        // minSamplingRate
    SL_SAMPLINGRATE_44_1,        // maxSamplingRate
    SL_BOOLEAN_TRUE,             // isFreqRangeContinuous
    NULL,                        // samplingRatesSupported
    0,                           // numOfSamplingRatesSupported
    2                            // maxChannels
};

static const SLAudioOutputDescriptor AudioOutputDescriptor_headset = {
    (SLchar *) "headset",
    SL_DEVCONNECTION_ATTACHED_WIRED,
    SL_DEVSCOPE_USER,
    SL_DEVLOCATION_HEADSET,
    SL_BOOLEAN_FALSE,
    SL_SAMPLINGRATE_44_1,
    SL_SAMPLINGRATE_44_1,
    SL_BOOLEAN_TRUE,
    NULL,
    0,
    2
};

static const SLAudioOutputDescriptor AudioOutputDescriptor_handsfree = {
    (SLchar *) "handsfree",
    SL_DEVCONNECTION_INTEGRATED,
    SL_DEVSCOPE_ENVIRONMENT,
    SL_DEVLOCATION_HANDSET,
    SL_BOOLEAN_FALSE,
    SL_SAMPLINGRATE_44_1,
    SL_SAMPLINGRATE_44_1,
    SL_BOOLEAN_TRUE,
    NULL,
    0,
    2
};

const struct AudioOutput_id_descriptor AudioOutput_id_descriptors[] = {
    {SL_DEFAULTDEVICEID_AUDIOOUTPUT, &AudioOutputDescriptor_speaker},
    {DEVICE_ID_HEADSET, &AudioOutputDescriptor_headset},
    {DEVICE_ID_HANDSFREE, &AudioOutputDescriptor_handsfree},
    {0, NULL}
};

static const SLLEDDescriptor SLLEDDescriptor_default = {
    32, // ledCount
    0,  // primaryLED
    (SLuint32) ~0   // colorMask
};

const struct LED_id_descriptor LED_id_descriptors[] = {
    {SL_DEFAULTDEVICEID_LED, &SLLEDDescriptor_default},
    {0, NULL}
};

static const SLVibraDescriptor SLVibraDescriptor_default = {
    SL_BOOLEAN_TRUE, // supportsFrequency
    SL_BOOLEAN_TRUE, // supportsIntensity
    20000,           // minFrequency
    100000           // maxFrequency
};

const struct Vibra_id_descriptor Vibra_id_descriptors[] = {
    {SL_DEFAULTDEVICEID_VIBRA, &SLVibraDescriptor_default},
    {0, NULL}
};

// should build this table from Caps table below

static const SLuint32 Codec_IDs[] = {
    SL_AUDIOCODEC_PCM,
    SL_AUDIOCODEC_MP3,
    SL_AUDIOCODEC_AMR,
    SL_AUDIOCODEC_AMRWB,
    SL_AUDIOCODEC_AMRWBPLUS,
    SL_AUDIOCODEC_AAC,
    SL_AUDIOCODEC_WMA,
    SL_AUDIOCODEC_REAL,
    SL_AUDIOCODEC_VORBIS
};

const SLuint32 * const Decoder_IDs = Codec_IDs;
const SLuint32 * const Encoder_IDs = Codec_IDs;

// for ANDROID, must match size and order of kVideoMimeTypes
static const SLuint32 VideoCodecIds[] = {
        XA_VIDEOCODEC_MPEG2,
        XA_VIDEOCODEC_H263,
        XA_VIDEOCODEC_MPEG4,
        XA_VIDEOCODEC_AVC,
#ifndef ANDROID
        XA_VIDEOCODEC_VC1,
#endif
#ifdef ANDROID
        XA_ANDROID_VIDEOCODEC_VP8
#endif
};

const SLuint32 * const VideoDecoderIds = VideoCodecIds;

static const SLmilliHertz SamplingRates_A[] = {
    SL_SAMPLINGRATE_8,
    SL_SAMPLINGRATE_11_025,
    SL_SAMPLINGRATE_12,
    SL_SAMPLINGRATE_16,
    SL_SAMPLINGRATE_22_05,
    SL_SAMPLINGRATE_24,
    SL_SAMPLINGRATE_32,
    SL_SAMPLINGRATE_44_1,
    SL_SAMPLINGRATE_48
};

static const SLAudioCodecDescriptor CodecDescriptor_A = {
    2,                   // maxChannels
    8,                   // minBitsPerSample
    16,                  // maxBitsPerSample
    SL_SAMPLINGRATE_8,   // minSampleRate
    SL_SAMPLINGRATE_48,  // maxSampleRate
    SL_BOOLEAN_FALSE,    // isFreqRangeContinuous
    (SLmilliHertz *) SamplingRates_A,
                         // pSampleRatesSupported;
    sizeof(SamplingRates_A) / sizeof(SamplingRates_A[0]),
                         // numSampleRatesSupported
    1,                   // minBitRate
    (SLuint32) ~0,       // maxBitRate
    SL_BOOLEAN_TRUE,     // isBitrateRangeContinuous
    NULL,                // pBitratesSupported
    0,                   // numBitratesSupported
    SL_AUDIOPROFILE_PCM, // profileSetting
    0                    // modeSetting
};

const CodecDescriptor DecoderDescriptors[] = {
    {SL_AUDIOCODEC_PCM, &CodecDescriptor_A},
    {SL_AUDIOCODEC_MP3, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMR, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMRWB, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMRWBPLUS, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AAC, &CodecDescriptor_A},
    {SL_AUDIOCODEC_WMA, &CodecDescriptor_A},
    {SL_AUDIOCODEC_REAL, &CodecDescriptor_A},
    {SL_AUDIOCODEC_VORBIS, &CodecDescriptor_A},
    {SL_AUDIOCODEC_NULL, NULL}
};

const CodecDescriptor EncoderDescriptors[] = {
    {SL_AUDIOCODEC_PCM, &CodecDescriptor_A},
    {SL_AUDIOCODEC_MP3, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMR, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMRWB, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AMRWBPLUS, &CodecDescriptor_A},
    {SL_AUDIOCODEC_AAC, &CodecDescriptor_A},
    {SL_AUDIOCODEC_WMA, &CodecDescriptor_A},
    {SL_AUDIOCODEC_REAL, &CodecDescriptor_A},
    {SL_AUDIOCODEC_VORBIS, &CodecDescriptor_A},
    {SL_AUDIOCODEC_NULL, NULL}
};


/** \brief Helper shared by decoder and encoder */

SLresult GetCodecCapabilities(SLuint32 codecId, SLuint32 *pIndex,
    SLAudioCodecDescriptor *pDescriptor, const CodecDescriptor *codecDescriptors)
{
    if (NULL == pIndex) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    const CodecDescriptor *cd = codecDescriptors;
    SLuint32 index;
    if (NULL == pDescriptor) {
        for (index = 0 ; NULL != cd->mDescriptor; ++cd) {
            if (cd->mCodecID == codecId) {
                ++index;
            }
        }
        *pIndex = index;
        return SL_RESULT_SUCCESS;
    }
    index = *pIndex;
    for ( ; NULL != cd->mDescriptor; ++cd) {
        if (cd->mCodecID == codecId) {
            if (0 == index) {
                *pDescriptor = *cd->mDescriptor;
#if 0   // Temporary workaround for Khronos bug 6331
                if (0 < pDescriptor->numSampleRatesSupported) {
                    // The malloc is not in the 1.0.1 specification
                    SLmilliHertz *temp = (SLmilliHertz *) malloc(sizeof(SLmilliHertz) *
                        pDescriptor->numSampleRatesSupported);
                    assert(NULL != temp);
                    memcpy(temp, pDescriptor->pSampleRatesSupported, sizeof(SLmilliHertz) *
                        pDescriptor->numSampleRatesSupported);
                    pDescriptor->pSampleRatesSupported = temp;
                } else {
                    pDescriptor->pSampleRatesSupported = NULL;
                }
#endif
                return SL_RESULT_SUCCESS;
            }
            --index;
        }
    }
    return SL_RESULT_PARAMETER_INVALID;
}
