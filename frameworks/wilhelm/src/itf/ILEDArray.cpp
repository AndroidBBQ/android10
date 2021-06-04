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

/* LEDArray implementation */

#include "sles_allinclusive.h"


static SLresult ILEDArray_ActivateLEDArray(SLLEDArrayItf self, SLuint32 lightMask)
{
    SL_ENTER_INTERFACE

    ILEDArray *thiz = (ILEDArray *) self;
    interface_lock_poke(thiz);
    thiz->mLightMask = lightMask;
    interface_unlock_poke(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult ILEDArray_IsLEDArrayActivated(SLLEDArrayItf self, SLuint32 *pLightMask)
{
    SL_ENTER_INTERFACE

    if (NULL == pLightMask) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        ILEDArray *thiz = (ILEDArray *) self;
        interface_lock_peek(thiz);
        SLuint32 lightMask = thiz->mLightMask;
        interface_unlock_peek(thiz);
        *pLightMask = lightMask;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult ILEDArray_SetColor(SLLEDArrayItf self, SLuint8 index, const SLHSL *pColor)
{
    SL_ENTER_INTERFACE

    result = SL_RESULT_PARAMETER_INVALID;
    do {
        if (!(index < MAX_LED_COUNT) || NULL == pColor)
            break;
        SLHSL color = *pColor;
        if (!(0 <= color.hue && color.hue <= 360000))
            break;
        if (!(0 <= color.saturation && color.saturation <= 1000))
            break;
        if (!(0 <= color.lightness && color.lightness <= 1000))
            break;
        ILEDArray *thiz = (ILEDArray *) self;
        // can't use poke because struct copy might not be atomic
        interface_lock_exclusive(thiz);
        thiz->mColors[index] = color;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    } while (0);

    SL_LEAVE_INTERFACE
}


static SLresult ILEDArray_GetColor(SLLEDArrayItf self, SLuint8 index, SLHSL *pColor)
{
    SL_ENTER_INTERFACE

    if (!(index < MAX_LED_COUNT) || NULL == pColor) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        ILEDArray *thiz = (ILEDArray *) self;
        // can't use peek because struct copy might not be atomic
        interface_lock_shared(thiz);
        SLHSL color = thiz->mColors[index];
        interface_unlock_shared(thiz);
        *pColor = color;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLLEDArrayItf_ ILEDArray_Itf = {
    ILEDArray_ActivateLEDArray,
    ILEDArray_IsLEDArrayActivated,
    ILEDArray_SetColor,
    ILEDArray_GetColor
};

void ILEDArray_init(void *self)
{
    ILEDArray *thiz = (ILEDArray *) self;
    thiz->mItf = &ILEDArray_Itf;
    thiz->mLightMask = 0;
    SLHSL *color = thiz->mColors;
    SLuint8 index;
    for (index = 0; index < MAX_LED_COUNT; ++index, ++color) {
        color->hue = 0; // red, but per specification 1.0.1 pg. 259: "Default color is undefined."
        color->saturation = 1000;
        color->lightness = 1000;
    }
    // const
    thiz->mCount = MAX_LED_COUNT;
}
