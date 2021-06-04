/*
 * Copyright (C) 2014 The Android Open Source Project
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

/* Automatic Gain Control implementation */
#include "sles_allinclusive.h"

#include <system/audio_effects/effect_ns.h>
/**
 * returns true if this interface is not associated with an initialized Noise Suppression effect
 */
static inline bool NO_NOISESUPPRESS(IAndroidNoiseSuppression* v) {
    return (v->mNSEffect == 0);
}

SLresult IAndroidNoiseSuppression_SetEnabled(SLAndroidNoiseSuppressionItf self, SLboolean enabled)
{
    SL_ENTER_INTERFACE

    IAndroidNoiseSuppression *thiz = (IAndroidNoiseSuppression *) self;
    interface_lock_exclusive(thiz);
    thiz->mEnabled = (SLboolean) enabled;
    if (NO_NOISESUPPRESS(thiz)) {
        result = SL_RESULT_CONTROL_LOST;
    } else {
        android::status_t status = thiz->mNSEffect->setEnabled((bool) thiz->mEnabled);
        result = android_fx_statusToResult(status);
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}

SLresult IAndroidNoiseSuppression_IsEnabled(SLAndroidNoiseSuppressionItf self, SLboolean *pEnabled)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnabled) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidNoiseSuppression *thiz = (IAndroidNoiseSuppression *) self;
        interface_lock_exclusive(thiz);
        if (NO_NOISESUPPRESS(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            *pEnabled = (SLboolean) thiz->mNSEffect->getEnabled();
            result = SL_RESULT_SUCCESS;
        }
        interface_unlock_exclusive(thiz);
    }
    SL_LEAVE_INTERFACE
}

static const struct SLAndroidNoiseSuppressionItf_ IAndroidNoiseSuppression_Itf = {
    IAndroidNoiseSuppression_SetEnabled,
    IAndroidNoiseSuppression_IsEnabled
};

void IAndroidNoiseSuppression_init(void *self)
{
    IAndroidNoiseSuppression *thiz = (IAndroidNoiseSuppression *) self;
    thiz->mItf = &IAndroidNoiseSuppression_Itf;
    thiz->mEnabled = SL_BOOLEAN_FALSE;
    memset(&thiz->mNSDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mNSEffect) android::sp<android::AudioEffect>();
}

void IAndroidNoiseSuppression_deinit(void *self)
{
    IAndroidNoiseSuppression *thiz = (IAndroidNoiseSuppression *) self;
    // explicit destructor
    thiz->mNSEffect.~sp();
}

bool IAndroidNoiseSuppression_Expose(void *self)
{
    IAndroidNoiseSuppression *thiz = (IAndroidNoiseSuppression *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_ANDROIDNOISESUPPRESSION, &thiz->mNSDescriptor)) {
        SL_LOGE("Noise Suppression initialization failed.");
        return false;
    }
    return true;
}
