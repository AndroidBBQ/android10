/*
 * Copyright (C) 2011 The Android Open Source Project
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

/* VideoDecoderCapabilities implementation */

#include "sles_allinclusive.h"
#ifdef ANDROID
#include "android/VideoCodec_to_android.h"
#endif


static XAresult IVideoDecoderCapabilities_GetVideoDecoders(XAVideoDecoderCapabilitiesItf self,
    XAuint32 *pNumDecoders, XAuint32 *pDecoderIds)
{
    XA_ENTER_INTERFACE

    if (NULL == pNumDecoders) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {
        if (NULL == pDecoderIds) {
            // If pDecoderIds is NULL, pNumDecoders returns the number of decoders available.
#ifdef ANDROID
            *pNumDecoders = android::android_videoCodec_getNbDecoders();
#else
            *pNumDecoders = kMaxVideoDecoders;
#endif

        } else {
            // If pDecodersIds is non-NULL, as an input pNumDecoders specifies the size of the
            // pDecoderIds array and as an output it specifies the number of decoder IDs available
            // within the pDecoderIds array.
            XAuint32 numDecoders = *pNumDecoders;
#ifdef ANDROID
            const XAuint32 androidNbDecoders = android::android_videoCodec_getNbDecoders();
            if (androidNbDecoders < numDecoders) {
                *pNumDecoders = numDecoders = androidNbDecoders;
            }
            android::android_videoCodec_getDecoderIds(numDecoders, pDecoderIds);
#else
            if (kMaxVideoDecoders < numDecoders) {
                *pNumDecoders = numDecoders = kMaxVideoDecoders;
            }
            memcpy(pDecoderIds, VideoDecoderIds, numDecoders * sizeof(XAuint32));
#endif
        }
        result = XA_RESULT_SUCCESS;
    }

    XA_LEAVE_INTERFACE
}


static XAresult IVideoDecoderCapabilities_GetVideoDecoderCapabilities(
    XAVideoDecoderCapabilitiesItf self, XAuint32 decoderId, XAuint32 *pIndex,
    XAVideoCodecDescriptor *pDescriptor)
{
    XA_ENTER_INTERFACE

    if (NULL == pIndex) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {
        if (NULL == pDescriptor) {
            // pIndex returns the number of video decoders capability descriptions.
#ifdef ANDROID
            result = android::android_videoCodec_getProfileLevelCombinationNb(decoderId, pIndex);
#else
            // Generic implementation has zero profile/level combinations for all codecs,
            // but this is not allowed per spec:
            //    "Each decoder must support at least one profile/mode pair
            //    and therefore have at least one Codec Descriptor."
            *pIndex = 0;
            SL_LOGE("Generic implementation has no video decoder capabilities");
            result = XA_RESULT_PARAMETER_INVALID;
#endif
        } else {
            // pIndex is an incrementing value used to enumerate capability descriptions.
#ifdef ANDROID
            result = android::android_videoCodec_getProfileLevelCombination(decoderId, *pIndex,
                    pDescriptor);
#else
            // For the generic implementation, any index >= 0 is out of range
#if 1   // not sure if this is needed, it's not being done for the Android case
            pDescriptor->codecId = decoderId;
#endif
            SL_LOGE("Generic implementation has no video decoder capabilities");
            result = XA_RESULT_PARAMETER_INVALID;
#endif
        }
    }

    XA_LEAVE_INTERFACE
}


static const struct XAVideoDecoderCapabilitiesItf_ IVideoDecoderCapabilities_Itf = {
    IVideoDecoderCapabilities_GetVideoDecoders,
    IVideoDecoderCapabilities_GetVideoDecoderCapabilities
};

void IVideoDecoderCapabilities_init(void *self)
{
    IVideoDecoderCapabilities *thiz = (IVideoDecoderCapabilities *) self;
    thiz->mItf = &IVideoDecoderCapabilities_Itf;
}


bool IVideoDecoderCapabilities_expose(void *self)
{
#ifdef ANDROID
    // This is an Engine object interface, so we allocate the associated resources every time
    //   the interface is exposed on the Engine object and free them when the object is about
    //   to be destroyed (see IVideoDecoderCapabilities_deinit), not just once during the
    //   lifetime of the process.
    return android::android_videoCodec_expose();
#else
    return false;
#endif
}


void IVideoDecoderCapabilities_deinit(void *self)
{
    SL_LOGV("IVideoDecoderCapabilities_deinit()");
#ifdef ANDROID
    android::android_videoCodec_deinit();
#endif
}
