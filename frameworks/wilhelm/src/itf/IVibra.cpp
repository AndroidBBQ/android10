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

/* Vibra implementation */

#include "sles_allinclusive.h"


static SLresult IVibra_Vibrate(SLVibraItf self, SLboolean vibrate)
{
    SL_ENTER_INTERFACE

    IVibra *thiz = (IVibra *) self;
    interface_lock_poke(thiz);
    thiz->mVibrate = SL_BOOLEAN_FALSE != vibrate; // normalize
    interface_unlock_poke(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IVibra_IsVibrating(SLVibraItf self, SLboolean *pVibrating)
{
    SL_ENTER_INTERFACE

    if (NULL == pVibrating) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVibra *thiz = (IVibra *) self;
        interface_lock_peek(thiz);
        SLboolean vibrate = thiz->mVibrate;
        interface_unlock_peek(thiz);
        *pVibrating = vibrate;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVibra_SetFrequency(SLVibraItf self, SLmilliHertz frequency)
{
    SL_ENTER_INTERFACE

    const SLVibraDescriptor *d = Vibra_id_descriptors[0].descriptor;
    if (!d->supportsFrequency) {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else if (!(d->minFrequency <= frequency && frequency <= d->maxFrequency)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVibra *thiz = (IVibra *) self;
        interface_lock_poke(thiz);
        thiz->mFrequency = frequency;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVibra_GetFrequency(SLVibraItf self, SLmilliHertz *pFrequency)
{
    SL_ENTER_INTERFACE

    if (NULL == pFrequency) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVibra *thiz = (IVibra *) self;
        interface_lock_peek(thiz);
        SLmilliHertz frequency = thiz->mFrequency;
        interface_unlock_peek(thiz);
        *pFrequency = frequency;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVibra_SetIntensity(SLVibraItf self, SLpermille intensity)
{
    SL_ENTER_INTERFACE

    const SLVibraDescriptor *d = Vibra_id_descriptors[0].descriptor;
    if (!d->supportsIntensity) {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else if (!(0 <= intensity && intensity <= 1000)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVibra *thiz = (IVibra *) self;
        interface_lock_poke(thiz);
        thiz->mIntensity = intensity;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVibra_GetIntensity(SLVibraItf self, SLpermille *pIntensity)
{
    SL_ENTER_INTERFACE

    if (NULL == pIntensity) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        const SLVibraDescriptor *d = Vibra_id_descriptors[0].descriptor;
        if (!d->supportsIntensity) {
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        } else {
            IVibra *thiz = (IVibra *) self;
            interface_lock_peek(thiz);
            SLpermille intensity = thiz->mIntensity;
            interface_unlock_peek(thiz);
            *pIntensity = intensity;
            result = SL_RESULT_SUCCESS;
        }
    }

    SL_LEAVE_INTERFACE
}


static const struct SLVibraItf_ IVibra_Itf = {
    IVibra_Vibrate,
    IVibra_IsVibrating,
    IVibra_SetFrequency,
    IVibra_GetFrequency,
    IVibra_SetIntensity,
    IVibra_GetIntensity
};

void IVibra_init(void *self)
{
    IVibra *thiz = (IVibra *) self;
    thiz->mItf = &IVibra_Itf;
    thiz->mVibrate = SL_BOOLEAN_FALSE;
    // next 2 values are undefined per spec
    thiz->mFrequency = Vibra_id_descriptors[0].descriptor->minFrequency;
    thiz->mIntensity = 1000;
}
