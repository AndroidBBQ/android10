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

/* AudioDecoderCapabilities implementation */

#include "sles_allinclusive.h"


static SLresult IAudioDecoderCapabilities_GetAudioDecoders(SLAudioDecoderCapabilitiesItf self,
    SLuint32 *pNumDecoders, SLuint32 *pDecoderIds)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumDecoders) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        result = SL_RESULT_SUCCESS;
        if (NULL != pDecoderIds) {
            SLuint32 numDecoders = *pNumDecoders;
            if (numDecoders > MAX_DECODERS) {
                numDecoders = MAX_DECODERS;
            } else if (numDecoders < MAX_DECODERS) {
                // FIXME starting in 1.1 this will be SL_RESULT_BUFFER_INSUFFICIENT
                result = SL_RESULT_PARAMETER_INVALID;
            }
            memcpy(pDecoderIds, Decoder_IDs, numDecoders * sizeof(SLuint32));
        }
        *pNumDecoders = MAX_DECODERS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAudioDecoderCapabilities_GetAudioDecoderCapabilities(
    SLAudioDecoderCapabilitiesItf self, SLuint32 decoderId, SLuint32 *pIndex,
    SLAudioCodecDescriptor *pDescriptor)
{
    SL_ENTER_INTERFACE

    result = GetCodecCapabilities(decoderId, pIndex, pDescriptor, DecoderDescriptors);

    SL_LEAVE_INTERFACE
}


static const struct SLAudioDecoderCapabilitiesItf_ IAudioDecoderCapabilities_Itf = {
    IAudioDecoderCapabilities_GetAudioDecoders,
    IAudioDecoderCapabilities_GetAudioDecoderCapabilities
};

void IAudioDecoderCapabilities_init(void *self)
{
    IAudioDecoderCapabilities *thiz = (IAudioDecoderCapabilities *) self;
    thiz->mItf = &IAudioDecoderCapabilities_Itf;
}
