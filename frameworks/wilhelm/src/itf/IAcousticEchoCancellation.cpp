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

/* Acoustic Echo Cancellation implementation */
#include "sles_allinclusive.h"

#include <system/audio_effects/effect_aec.h>

/**
 * returns true if this interface is not associated with an initialized AEC effect
 */
static inline bool NO_ECHOCANCEL(IAndroidAcousticEchoCancellation* v) {
    return (v->mAECEffect == 0);
}

static SLresult IAndroidAcousticEchoCancellation_SetEnabled(
                                                     SLAndroidAcousticEchoCancellationItf self,
                                                     SLboolean enabled)
{
    SL_ENTER_INTERFACE

    IAndroidAcousticEchoCancellation *thiz = (IAndroidAcousticEchoCancellation *) self;
    interface_lock_exclusive(thiz);
    thiz->mEnabled = (SLboolean) enabled;
    if (NO_ECHOCANCEL(thiz)) {
        result = SL_RESULT_CONTROL_LOST;
    } else {
        android::status_t status = thiz->mAECEffect->setEnabled((bool) thiz->mEnabled);
        result = android_fx_statusToResult(status);
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}

static SLresult IAndroidAcousticEchoCancellation_IsEnabled(
                                                    SLAndroidAcousticEchoCancellationItf self,
                                                    SLboolean *pEnabled)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnabled) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidAcousticEchoCancellation *thiz = (IAndroidAcousticEchoCancellation *) self;
        interface_lock_exclusive(thiz);
        if (NO_ECHOCANCEL(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            *pEnabled = (SLboolean) thiz->mAECEffect->getEnabled();
            result = SL_RESULT_SUCCESS;
        }
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}

static const struct SLAndroidAcousticEchoCancellationItf_ IAndroidAcousticEchoCancellation_Itf = {
    IAndroidAcousticEchoCancellation_SetEnabled,
    IAndroidAcousticEchoCancellation_IsEnabled
};

void IAndroidAcousticEchoCancellation_init(void *self)
{
    IAndroidAcousticEchoCancellation *thiz = (IAndroidAcousticEchoCancellation *) self;
    thiz->mItf = &IAndroidAcousticEchoCancellation_Itf;
    thiz->mEnabled = SL_BOOLEAN_FALSE;
    memset(&thiz->mAECDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mAECEffect) android::sp<android::AudioEffect>();
}

void IAndroidAcousticEchoCancellation_deinit(void *self)
{
    IAndroidAcousticEchoCancellation *thiz = (IAndroidAcousticEchoCancellation *) self;
    // explicit destructor
    thiz->mAECEffect.~sp();
}

bool IAndroidAcousticEchoCancellation_Expose(void *self)
{
    IAndroidAcousticEchoCancellation *thiz = (IAndroidAcousticEchoCancellation *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_ANDROIDACOUSTICECHOCANCELLATION,
                                         &thiz->mAECDescriptor)) {
        SL_LOGE("Acoustic Echo Cancellation initialization failed.");
        return false;
    }
    return true;
}
