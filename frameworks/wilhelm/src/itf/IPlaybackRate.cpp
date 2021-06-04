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

/* PlaybackRate implementation */

#include "sles_allinclusive.h"


static SLresult IPlaybackRate_SetRate(SLPlaybackRateItf self, SLpermille rate)
{
    SL_ENTER_INTERFACE

    IPlaybackRate *thiz = (IPlaybackRate *) self;
    // const, so no lock needed
    if (!(thiz->mMinRate <= rate && rate <= thiz->mMaxRate)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_exclusive(thiz);
#ifdef ANDROID
        CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                (CAudioPlayer *) thiz->mThis : NULL;
        if (NULL != ap) {
            result = android_audioPlayer_setPlaybackRateAndConstraints(ap, rate, thiz->mProperties);
        } else {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        }
#else
        result = SL_RESULT_SUCCESS;
#endif
        if (SL_RESULT_SUCCESS == result) {
            thiz->mRate = rate;
        }
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlaybackRate_GetRate(SLPlaybackRateItf self, SLpermille *pRate)
{
    SL_ENTER_INTERFACE

    if (NULL == pRate) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlaybackRate *thiz = (IPlaybackRate *) self;
        interface_lock_shared(thiz);
        SLpermille rate = thiz->mRate;
        interface_unlock_shared(thiz);
        *pRate = rate;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlaybackRate_SetPropertyConstraints(SLPlaybackRateItf self, SLuint32 constraints)
{
    SL_ENTER_INTERFACE

    IPlaybackRate *thiz = (IPlaybackRate *) self;
    if (constraints & ~(SL_RATEPROP_SILENTAUDIO | SL_RATEPROP_STAGGEREDAUDIO |
            SL_RATEPROP_NOPITCHCORAUDIO | SL_RATEPROP_PITCHCORAUDIO)) {
        result = SL_RESULT_PARAMETER_INVALID;
    // const, so no lock needed
    } else if (!(thiz->mCapabilities & constraints)) {
        result = SL_RESULT_FEATURE_UNSUPPORTED;
    } else {
        interface_lock_exclusive(thiz);
#ifdef ANDROID
        // verify property support before storing
        CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                (CAudioPlayer *) thiz->mThis : NULL;
        if (NULL != ap) {
            result = android_audioPlayer_setPlaybackRateAndConstraints(ap, thiz->mRate,
                    constraints);
        } else {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        }
#else
        result = SL_RESULT_SUCCESS;
#endif
        if (result == SL_RESULT_SUCCESS) {
            thiz->mProperties = constraints;
        }
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlaybackRate_GetProperties(SLPlaybackRateItf self, SLuint32 *pProperties)
{
    SL_ENTER_INTERFACE

    if (NULL == pProperties) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlaybackRate *thiz = (IPlaybackRate *) self;
        interface_lock_shared(thiz);
        SLuint32 properties = thiz->mProperties;
        interface_unlock_shared(thiz);
        *pProperties = properties;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlaybackRate_GetCapabilitiesOfRate(SLPlaybackRateItf self,
    SLpermille rate, SLuint32 *pCapabilities)
{
    SL_ENTER_INTERFACE

    if (NULL == pCapabilities) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlaybackRate *thiz = (IPlaybackRate *) self;
        SLuint32 capabilities;
        // const, so no lock needed
        if (!(thiz->mMinRate <= rate && rate <= thiz->mMaxRate)) {
            capabilities = 0;
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            capabilities = thiz->mCapabilities;
            result = SL_RESULT_SUCCESS;
        }
        *pCapabilities = capabilities;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlaybackRate_GetRateRange(SLPlaybackRateItf self, SLuint8 index,
    SLpermille *pMinRate, SLpermille *pMaxRate, SLpermille *pStepSize, SLuint32 *pCapabilities)
{
    SL_ENTER_INTERFACE

    // only one range
    if (NULL == pMinRate || NULL == pMaxRate || NULL == pStepSize || NULL == pCapabilities ||
        (0 < index)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlaybackRate *thiz = (IPlaybackRate *) self;
        // const, so no lock needed
        SLpermille minRate = thiz->mMinRate;
        SLpermille maxRate = thiz->mMaxRate;
        SLpermille stepSize = thiz->mStepSize;
        SLuint32 capabilities = thiz->mCapabilities;
        *pMinRate = minRate;
        *pMaxRate = maxRate;
        *pStepSize = stepSize;
        *pCapabilities = capabilities;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLPlaybackRateItf_ IPlaybackRate_Itf = {
    IPlaybackRate_SetRate,
    IPlaybackRate_GetRate,
    IPlaybackRate_SetPropertyConstraints,
    IPlaybackRate_GetProperties,
    IPlaybackRate_GetCapabilitiesOfRate,
    IPlaybackRate_GetRateRange
};

void IPlaybackRate_init(void *self)
{
    IPlaybackRate *thiz = (IPlaybackRate *) self;
    thiz->mItf = &IPlaybackRate_Itf;
    thiz->mProperties = SL_RATEPROP_NOPITCHCORAUDIO;
    thiz->mRate = 1000;
    // const after initialization; these are default values which may be overwritten
    // during object creation but will not be modified after that
    // (e.g. for an Android AudioPlayer, see sles_to_android_audioPlayerCreate)
    thiz->mMinRate = 1000;
    thiz->mMaxRate = 1000;
    thiz->mStepSize = 0;
    thiz->mCapabilities = SL_RATEPROP_NOPITCHCORAUDIO;
}
