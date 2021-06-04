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

/* Android Effect implementation */

#include "sles_allinclusive.h"

#include <system/audio.h>

static SLresult IAndroidEffect_CreateEffect(SLAndroidEffectItf self,
        SLInterfaceID effectImplementationId) {

    SL_ENTER_INTERFACE

    IAndroidEffect *thiz = (IAndroidEffect *) self;
    if (SL_OBJECTID_AUDIOPLAYER == IObjectToObjectID(thiz->mThis)) {
        CAudioPlayer *ap = (CAudioPlayer *)thiz->mThis;
        if (ap->mTrackPlayer->mAudioTrack != 0) {
            result = android_genericFx_createEffect(thiz, effectImplementationId, ap->mSessionId);
        } else {
            result = SL_RESULT_RESOURCE_ERROR;
        }
    } else if (SL_OBJECTID_OUTPUTMIX == IObjectToObjectID(thiz->mThis)) {
        result = android_genericFx_createEffect(thiz, effectImplementationId,
                AUDIO_SESSION_OUTPUT_MIX);
    } else {
        // the interface itself is invalid because it is not attached to an AudioPlayer or
        // an OutputMix
        result = SL_RESULT_PARAMETER_INVALID;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidEffect_ReleaseEffect(SLAndroidEffectItf self,
        SLInterfaceID effectImplementationId) {

    SL_ENTER_INTERFACE

    IAndroidEffect *thiz = (IAndroidEffect *) self;
    result = android_genericFx_releaseEffect(thiz, effectImplementationId);

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidEffect_SetEnabled(SLAndroidEffectItf self,
        SLInterfaceID effectImplementationId, SLboolean enabled) {

    SL_ENTER_INTERFACE

    IAndroidEffect *thiz = (IAndroidEffect *) self;
    result = android_genericFx_setEnabled(thiz, effectImplementationId, enabled);

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidEffect_IsEnabled(SLAndroidEffectItf self,
        SLInterfaceID effectImplementationId, SLboolean * pEnabled) {

    SL_ENTER_INTERFACE

    IAndroidEffect *thiz = (IAndroidEffect *) self;
    result = android_genericFx_isEnabled(thiz, effectImplementationId, pEnabled);

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidEffect_SendCommand(SLAndroidEffectItf self,
        SLInterfaceID effectImplementationId, SLuint32 command, SLuint32 commandSize,
        void* pCommand, SLuint32 *replySize, void *pReply) {

    SL_ENTER_INTERFACE

    IAndroidEffect *thiz = (IAndroidEffect *) self;
    result = android_genericFx_sendCommand(thiz, effectImplementationId, command, commandSize,
            pCommand, replySize, pReply);

    SL_LEAVE_INTERFACE
}


static const struct SLAndroidEffectItf_ IAndroidEffect_Itf = {
        IAndroidEffect_CreateEffect,
        IAndroidEffect_ReleaseEffect,
        IAndroidEffect_SetEnabled,
        IAndroidEffect_IsEnabled,
        IAndroidEffect_SendCommand
};

void IAndroidEffect_init(void *self)
{
    IAndroidEffect *thiz = (IAndroidEffect *) self;
    thiz->mItf = &IAndroidEffect_Itf;
    thiz->mEffects =
      new android::KeyedVector<SLuint32, android::sp<android::AudioEffect> >();
}

void IAndroidEffect_deinit(void *self)
{
    IAndroidEffect *thiz = (IAndroidEffect *) self;
    if (NULL != thiz->mEffects) {
        if (!thiz->mEffects->isEmpty()) {
            thiz->mEffects->clear();
        }
        delete thiz->mEffects;
        thiz->mEffects = NULL;
    }
}
