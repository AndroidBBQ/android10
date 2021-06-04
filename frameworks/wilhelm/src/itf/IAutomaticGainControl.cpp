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

#include <system/audio_effects/effect_agc.h>

/**
 * returns true if this interface is not associated with an initialized AGC effect
 */
static inline bool NO_AUTOGAIN(IAndroidAutomaticGainControl* v) {
    return (v->mAGCEffect == 0);
}

SLresult IAndroidAutomaticGainControl_SetEnabled(SLAndroidAutomaticGainControlItf self,
                                                 SLboolean enabled)
{
    SL_ENTER_INTERFACE

    IAndroidAutomaticGainControl *thiz = (IAndroidAutomaticGainControl *) self;
    interface_lock_exclusive(thiz);
    thiz->mEnabled = (SLboolean) enabled;
    if (NO_AUTOGAIN(thiz)) {
        result = SL_RESULT_CONTROL_LOST;
    } else {
        android::status_t status = thiz->mAGCEffect->setEnabled((bool) thiz->mEnabled);
        result = android_fx_statusToResult(status);
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}

SLresult IAndroidAutomaticGainControl_IsEnabled(SLAndroidAutomaticGainControlItf self,
                                                SLboolean *pEnabled)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnabled) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidAutomaticGainControl *thiz = (IAndroidAutomaticGainControl *) self;
        interface_lock_exclusive(thiz);
        if (NO_AUTOGAIN(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            *pEnabled = (SLboolean) thiz->mAGCEffect->getEnabled();
            result = SL_RESULT_SUCCESS;
        }
        interface_unlock_exclusive(thiz);
    }
    SL_LEAVE_INTERFACE
}

static const struct SLAndroidAutomaticGainControlItf_ IAndroidAutomaticGainControl_Itf = {
    IAndroidAutomaticGainControl_SetEnabled,
    IAndroidAutomaticGainControl_IsEnabled
};

void IAndroidAutomaticGainControl_init(void *self)
{
    IAndroidAutomaticGainControl *thiz = (IAndroidAutomaticGainControl *) self;
    thiz->mItf = &IAndroidAutomaticGainControl_Itf;
    thiz->mEnabled = SL_BOOLEAN_FALSE;
    memset(&thiz->mAGCDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mAGCEffect) android::sp<android::AudioEffect>();
}

void IAndroidAutomaticGainControl_deinit(void *self)
{
    IAndroidAutomaticGainControl *thiz = (IAndroidAutomaticGainControl *) self;
    // explicit destructor
    thiz->mAGCEffect.~sp();
}

bool IAndroidAutomaticGainControl_Expose(void *self)
{
    IAndroidAutomaticGainControl *thiz = (IAndroidAutomaticGainControl *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_ANDROIDAUTOMATICGAINCONTROL,
                                         &thiz->mAGCDescriptor)) {
        SL_LOGE("Automatic Gain Control initialization failed.");
        return false;
    }
    return true;
}
