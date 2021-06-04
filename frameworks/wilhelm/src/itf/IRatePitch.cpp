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

/* RatePitch implementation */

#include "sles_allinclusive.h"


static SLresult IRatePitch_SetRate(SLRatePitchItf self, SLpermille rate)
{
    SL_ENTER_INTERFACE

    IRatePitch *thiz = (IRatePitch *) self;
    if (!(thiz->mMinRate <= rate && rate <= thiz->mMaxRate)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_poke(thiz);
        thiz->mRate = rate;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRatePitch_GetRate(SLRatePitchItf self, SLpermille *pRate)
{
    SL_ENTER_INTERFACE

    if (NULL == pRate) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRatePitch *thiz = (IRatePitch *) self;
        interface_lock_peek(thiz);
        SLpermille rate = thiz->mRate;
        interface_unlock_peek(thiz);
        *pRate = rate;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRatePitch_GetRatePitchCapabilities(SLRatePitchItf self,
    SLpermille *pMinRate, SLpermille *pMaxRate)
{
    SL_ENTER_INTERFACE

    // per spec, each is optional, and does not require that at least one must be non-NULL
#if 0
    if (NULL == pMinRate && NULL == pMaxRate)
        result = SL_RESULT_PARAMETER_INVALID;
#endif
    IRatePitch *thiz = (IRatePitch *) self;
    // const, so no lock required
    SLpermille minRate = thiz->mMinRate;
    SLpermille maxRate = thiz->mMaxRate;
    if (NULL != pMinRate)
        *pMinRate = minRate;
    if (NULL != pMaxRate)
        *pMaxRate = maxRate;
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SLRatePitchItf_ IRatePitch_Itf = {
    IRatePitch_SetRate,
    IRatePitch_GetRate,
    IRatePitch_GetRatePitchCapabilities
};

void IRatePitch_init(void *self)
{
    IRatePitch *thiz = (IRatePitch *) self;
    thiz->mItf = &IRatePitch_Itf;
    thiz->mRate = 1000;
    // const
    thiz->mMinRate = 500;
    thiz->mMaxRate = 2000;
}
