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

/* Android Effect Capabilities implementation */

#include "sles_allinclusive.h"


static SLresult IAndroidEffectCapabilities_QueryNumEffects(SLAndroidEffectCapabilitiesItf self,
        SLuint32 * pNumSupportedAudioEffects) {

    SL_ENTER_INTERFACE

    if (NULL == pNumSupportedAudioEffects) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidEffectCapabilities *thiz = (IAndroidEffectCapabilities *) self;
        interface_lock_shared(thiz);

        *pNumSupportedAudioEffects = thiz->mNumFx;
        result = SL_RESULT_SUCCESS;

        interface_unlock_shared(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidEffectCapabilities_QueryEffect(SLAndroidEffectCapabilitiesItf self,
        SLuint32 index, SLInterfaceID *pEffectType, SLInterfaceID *pEffectImplementation,
        SLchar * pName, SLuint16 *pNameSize) {

    SL_ENTER_INTERFACE

    IAndroidEffectCapabilities *thiz = (IAndroidEffectCapabilities *) self;
    if (index > thiz->mNumFx) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_shared(thiz);
        if (NULL != pEffectType) {
            *pEffectType = (SLInterfaceID) &thiz->mFxDescriptors[index].type;
        }
        if (NULL != pEffectImplementation) {
            *pEffectImplementation = (SLInterfaceID) &thiz->mFxDescriptors[index].uuid;
        }
        if ((NULL != pName) && (0 < *pNameSize)) {
            int len = strlen(thiz->mFxDescriptors[index].name);
            strncpy((char*)pName, thiz->mFxDescriptors[index].name,
                    *pNameSize > len ? len : *pNameSize );
            *pNameSize = len;
        }
        interface_unlock_shared(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLAndroidEffectCapabilitiesItf_ IAndroidEffectCapabilities_Itf = {
        IAndroidEffectCapabilities_QueryNumEffects,
        IAndroidEffectCapabilities_QueryEffect
};

void IAndroidEffectCapabilities_init(void *self)
{
    IAndroidEffectCapabilities *thiz = (IAndroidEffectCapabilities *) self;
    thiz->mItf = &IAndroidEffectCapabilities_Itf;

    // This is the default initialization; fields will be updated when interface is exposed
    thiz->mNumFx = 0;
    thiz->mFxDescriptors = NULL;
}

bool IAndroidEffectCapabilities_Expose(void *self)
{
    IAndroidEffectCapabilities *thiz = (IAndroidEffectCapabilities *) self;
    SLuint32 numEffects = 0;
    SLresult result = android_genericFx_queryNumEffects(&numEffects);
    if (SL_RESULT_SUCCESS != result) {
        SL_LOGE("android_genericFx_queryNumEffects %u", result);
        return false;
    }
    thiz->mNumFx = numEffects;
    SL_LOGV("Effect Capabilities has %d effects", thiz->mNumFx);
    if (thiz->mNumFx > 0) {
        thiz->mFxDescriptors = (effect_descriptor_t*) new effect_descriptor_t[thiz->mNumFx];
        for (SLuint32 i = 0 ; i < thiz->mNumFx ; i++) {
            SLresult result2;
            result2 = android_genericFx_queryEffect(i, &thiz->mFxDescriptors[i]);
            if (SL_RESULT_SUCCESS != result2) {
                SL_LOGE("Error (SLresult is %d) querying effect %d", result2, i);
                // Remember the first failing result code, but keep going
                if (SL_RESULT_SUCCESS == result) {
                    result = result2;
                }
            } else {
                SL_LOGV("effect %d: type=%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x name=%s",
                        i,
                        thiz->mFxDescriptors[i].type.timeLow,
                        thiz->mFxDescriptors[i].type.timeMid,
                        thiz->mFxDescriptors[i].type.timeHiAndVersion,
                        thiz->mFxDescriptors[i].type.clockSeq,
                        thiz->mFxDescriptors[i].type.node[0],
                        thiz->mFxDescriptors[i].type.node[1],
                        thiz->mFxDescriptors[i].type.node[2],
                        thiz->mFxDescriptors[i].type.node[3],
                        thiz->mFxDescriptors[i].type.node[4],
                        thiz->mFxDescriptors[i].type.node[5],
                        thiz->mFxDescriptors[i].name);
            }
        }
    }
    return SL_RESULT_SUCCESS == result;
}

void IAndroidEffectCapabilities_deinit(void *self)
{
    IAndroidEffectCapabilities *thiz = (IAndroidEffectCapabilities *) self;
    // free effect library data
    if (NULL != thiz->mFxDescriptors) {
        delete[] thiz->mFxDescriptors;
        thiz->mFxDescriptors = NULL;
    }
}
