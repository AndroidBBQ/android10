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

#define DEVICE_ID_HEADSET 1
#define DEVICE_ID_HANDSFREE 2

extern const struct AudioInput_id_descriptor {
    SLuint32 id;
    const SLAudioInputDescriptor *descriptor;
} AudioInput_id_descriptors[];

extern const struct AudioOutput_id_descriptor {
    SLuint32 id;
    const SLAudioOutputDescriptor *descriptor;
} AudioOutput_id_descriptors[];

extern const struct LED_id_descriptor {
    SLuint32 id;
    const SLLEDDescriptor *descriptor;
} LED_id_descriptors[];

extern const struct Vibra_id_descriptor {
    SLuint32 id;
    const SLVibraDescriptor *descriptor;
} Vibra_id_descriptors[];

// These are not in 1.0.1 header file
#define SL_AUDIOCODEC_NULL   0
#define SL_AUDIOCODEC_VORBIS 9

/** \brief Associates a codec ID with a corresponding codec descriptor */

typedef struct {
    SLuint32 mCodecID;  ///< The codec ID
    const SLAudioCodecDescriptor *mDescriptor;  ///< The corresponding descriptor
} CodecDescriptor;

#define MAX_DECODERS 9 ///< (sizeof(Decoder_IDs) / sizeof(Decoder_IDs[0]))
#define MAX_ENCODERS 9 ///< (sizeof(Encoder_IDs) / sizeof(Encoder_IDs[0]))

extern const XAuint32 * const VideoDecoderIds;
#ifndef ANDROID
extern const XAuint32 kMaxVideoDecoders;
#endif

// For now, but encoders might be different than decoders later
extern const SLuint32 * const Decoder_IDs, * const Encoder_IDs;

extern const CodecDescriptor DecoderDescriptors[], EncoderDescriptors[];

extern SLresult GetCodecCapabilities(SLuint32 decoderId, SLuint32 *pIndex,
    SLAudioCodecDescriptor *pDescriptor,
    const CodecDescriptor *codecDescriptors);
