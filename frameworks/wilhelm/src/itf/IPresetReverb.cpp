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

/* PresetReverb implementation */

#include "sles_allinclusive.h"

#if defined(ANDROID)
/**
 * returns true if this interface is not associated with an initialized PresetReverb effect
 */
static inline bool NO_PRESETREVERB(IPresetReverb* ipr) {
    return (ipr->mPresetReverbEffect == 0);
}
#endif

static SLresult IPresetReverb_SetPreset(SLPresetReverbItf self, SLuint16 preset)
{
    SL_ENTER_INTERFACE

    IPresetReverb *thiz = (IPresetReverb *) self;
    switch (preset) {
    case SL_REVERBPRESET_NONE:
    case SL_REVERBPRESET_SMALLROOM:
    case SL_REVERBPRESET_MEDIUMROOM:
    case SL_REVERBPRESET_LARGEROOM:
    case SL_REVERBPRESET_MEDIUMHALL:
    case SL_REVERBPRESET_LARGEHALL:
    case SL_REVERBPRESET_PLATE:
        interface_lock_exclusive(thiz);
        thiz->mPreset = preset;
#if !defined(ANDROID)
        result = SL_RESULT_SUCCESS;
#else
        if (NO_PRESETREVERB(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status = android_prev_setPreset(thiz->mPresetReverbEffect, preset);
            result = android_fx_statusToResult(status);
        }
#endif
        interface_unlock_exclusive(thiz);
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}

static SLresult IPresetReverb_GetPreset(SLPresetReverbItf self, SLuint16 *pPreset)
{
    SL_ENTER_INTERFACE

    if (NULL == pPreset) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPresetReverb *thiz = (IPresetReverb *) self;
        interface_lock_shared(thiz);
        SLuint16 preset = SL_REVERBPRESET_NONE;
#if 1 // !defined(ANDROID)
        preset = thiz->mPreset;
        result = SL_RESULT_SUCCESS;
#else
        if (NO_PRESETREVERB(thiz)) {
            preset = thiz->mPreset;
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status = android_prev_getPreset(thiz->mPresetReverbEffect, &preset);
            result = android_fx_statusToResult(status);
        }
        // OpenSL ES 1.0.1 spec and conformance test do not permit SL_RESULT_CONTROL_LOST
        if (SL_RESULT_CONTROL_LOST == result) {
            result = SL_RESULT_SUCCESS;
        }
#endif
        interface_unlock_shared(thiz);
        *pPreset = preset;
    }

    SL_LEAVE_INTERFACE
}

static const struct SLPresetReverbItf_ IPresetReverb_Itf = {
    IPresetReverb_SetPreset,
    IPresetReverb_GetPreset
};

void IPresetReverb_init(void *self)
{
    IPresetReverb *thiz = (IPresetReverb *) self;
    thiz->mItf = &IPresetReverb_Itf;
    thiz->mPreset = SL_REVERBPRESET_NONE;
#if defined(ANDROID)
    memset(&thiz->mPresetReverbDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mPresetReverbEffect) android::sp<android::AudioEffect>();
#endif
}

void IPresetReverb_deinit(void *self)
{
#if defined(ANDROID)
    IPresetReverb *thiz = (IPresetReverb *) self;
    // explicit destructor
    thiz->mPresetReverbEffect.~sp();
#endif
}

bool IPresetReverb_Expose(void *self)
{
#if defined(ANDROID)
    IPresetReverb *thiz = (IPresetReverb *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_PRESETREVERB, &thiz->mPresetReverbDescriptor)) {
        SL_LOGE("PresetReverb initialization failed.");
        return false;
    }
#endif
    return true;
}
