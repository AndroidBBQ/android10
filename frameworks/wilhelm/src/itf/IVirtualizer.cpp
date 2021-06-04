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

/* Virtualizer implementation */

#include "sles_allinclusive.h"
#ifdef ANDROID
#include <system/audio_effects/effect_virtualizer.h>
#endif

#define VIRTUALIZER_STRENGTH_MIN 0
#define VIRTUALIZER_STRENGTH_MAX 1000


#if defined(ANDROID)
/**
 * returns true if this interface is not associated with an initialized Virtualizer effect
 */
static inline bool NO_VIRTUALIZER(IVirtualizer* v) {
    return (v->mVirtualizerEffect == 0);
}
#endif


static SLresult IVirtualizer_SetEnabled(SLVirtualizerItf self, SLboolean enabled)
{
    SL_ENTER_INTERFACE

    IVirtualizer *thiz = (IVirtualizer *) self;
    interface_lock_exclusive(thiz);
    thiz->mEnabled = (SLboolean) enabled;
#if !defined(ANDROID)
    result = SL_RESULT_SUCCESS;
#else
    if (NO_VIRTUALIZER(thiz)) {
        result = SL_RESULT_CONTROL_LOST;
    } else {
        android::status_t status =
                thiz->mVirtualizerEffect->setEnabled((bool) thiz->mEnabled);
        result = android_fx_statusToResult(status);
    }
#endif
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE

}


static SLresult IVirtualizer_IsEnabled(SLVirtualizerItf self, SLboolean *pEnabled)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnabled) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVirtualizer *thiz = (IVirtualizer *) self;
        interface_lock_exclusive(thiz);
#if !defined(ANDROID)
        SLboolean enabled = thiz->mEnabled;
        *pEnabled = enabled;
        result = SL_RESULT_SUCCESS;
#else
        if (NO_VIRTUALIZER(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            *pEnabled = (SLboolean) thiz->mVirtualizerEffect->getEnabled();
            result = SL_RESULT_SUCCESS;
        }
#endif
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVirtualizer_SetStrength(SLVirtualizerItf self, SLpermille strength)
{
    SL_ENTER_INTERFACE

    if ((VIRTUALIZER_STRENGTH_MIN > strength) || (VIRTUALIZER_STRENGTH_MAX < strength)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVirtualizer *thiz = (IVirtualizer *) self;
        interface_lock_exclusive(thiz);
#if !defined(ANDROID)
        thiz->mStrength = strength;
        result = SL_RESULT_SUCCESS;
#else
        if (NO_VIRTUALIZER(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status = android_virt_setParam(thiz->mVirtualizerEffect,
                    VIRTUALIZER_PARAM_STRENGTH, &strength);
            result = android_fx_statusToResult(status);
        }
#endif
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVirtualizer_GetRoundedStrength(SLVirtualizerItf self, SLpermille *pStrength)
{
    SL_ENTER_INTERFACE

    if (NULL == pStrength) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVirtualizer *thiz = (IVirtualizer *) self;
        interface_lock_exclusive(thiz);
        SLpermille strength = thiz->mStrength;;
#if !defined(ANDROID)
        result = SL_RESULT_SUCCESS;
#else
        if (NO_VIRTUALIZER(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status = android_virt_getParam(thiz->mVirtualizerEffect,
                           VIRTUALIZER_PARAM_STRENGTH, &strength);
            result = android_fx_statusToResult(status);
        }
#endif
        interface_unlock_exclusive(thiz);
        *pStrength = strength;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVirtualizer_IsStrengthSupported(SLVirtualizerItf self, SLboolean *pSupported)
{
    SL_ENTER_INTERFACE

    if (NULL == pSupported) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
#if !defined(ANDROID)
        *pSupported = SL_BOOLEAN_TRUE;
        result = SL_RESULT_SUCCESS;
#else
        IVirtualizer *thiz = (IVirtualizer *) self;
        int32_t supported = 0;
        interface_lock_exclusive(thiz);
        if (NO_VIRTUALIZER(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status =
                android_virt_getParam(thiz->mVirtualizerEffect,
                        VIRTUALIZER_PARAM_STRENGTH_SUPPORTED, &supported);
            result = android_fx_statusToResult(status);
        }
        interface_unlock_exclusive(thiz);
        *pSupported = (SLboolean) (supported != 0);
#endif
    }

    SL_LEAVE_INTERFACE
}


static const struct SLVirtualizerItf_ IVirtualizer_Itf = {
    IVirtualizer_SetEnabled,
    IVirtualizer_IsEnabled,
    IVirtualizer_SetStrength,
    IVirtualizer_GetRoundedStrength,
    IVirtualizer_IsStrengthSupported
};

void IVirtualizer_init(void *self)
{
    IVirtualizer *thiz = (IVirtualizer *) self;
    thiz->mItf = &IVirtualizer_Itf;
    thiz->mEnabled = SL_BOOLEAN_FALSE;
    thiz->mStrength = 0;
#if defined(ANDROID)
    memset(&thiz->mVirtualizerDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mVirtualizerEffect) android::sp<android::AudioEffect>();
#endif
}

void IVirtualizer_deinit(void *self)
{
#if defined(ANDROID)
    IVirtualizer *thiz = (IVirtualizer *) self;
    // explicit destructor
    thiz->mVirtualizerEffect.~sp();
#endif
}

bool IVirtualizer_Expose(void *self)
{
#if defined(ANDROID)
    IVirtualizer *thiz = (IVirtualizer *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_VIRTUALIZER, &thiz->mVirtualizerDescriptor)) {
        SL_LOGE("Virtualizer initialization failed.");
        return false;
    }
#endif
    return true;
}
