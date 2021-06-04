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

/* Pitch implementation */

#include "sles_allinclusive.h"


static SLresult IPitch_SetPitch(SLPitchItf self, SLpermille pitch)
{
    SL_ENTER_INTERFACE

    IPitch *thiz = (IPitch *) self;
    // const, so no lock needed
    if (!(thiz->mMinPitch <= pitch && pitch <= thiz->mMaxPitch)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_poke(thiz);
        thiz->mPitch = pitch;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPitch_GetPitch(SLPitchItf self, SLpermille *pPitch)
{
    SL_ENTER_INTERFACE

    if (NULL == pPitch) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPitch *thiz = (IPitch *) self;
        interface_lock_peek(thiz);
        SLpermille pitch = thiz->mPitch;
        interface_unlock_peek(thiz);
        *pPitch = pitch;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPitch_GetPitchCapabilities(SLPitchItf self,
    SLpermille *pMinPitch, SLpermille *pMaxPitch)
{
    SL_ENTER_INTERFACE

    // per spec, each is optional, and does not require that at least one must be non-NULL
#if 0
    if (NULL == pMinPitch && NULL == pMaxPitch)
        result = SL_RESULT_PARAMETER_INVALID;
#endif
    IPitch *thiz = (IPitch *) self;
    // const, so no lock needed
    SLpermille minPitch = thiz->mMinPitch;
    SLpermille maxPitch = thiz->mMaxPitch;
    if (NULL != pMinPitch)
        *pMinPitch = minPitch;
    if (NULL != pMaxPitch)
        *pMaxPitch = maxPitch;
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SLPitchItf_ IPitch_Itf = {
    IPitch_SetPitch,
    IPitch_GetPitch,
    IPitch_GetPitchCapabilities
};

void IPitch_init(void *self)
{
    IPitch *thiz = (IPitch *) self;
    thiz->mItf = &IPitch_Itf;
    thiz->mPitch = 1000;
    // const
    thiz->mMinPitch = -500;
    thiz->mMaxPitch = 2000;
}
