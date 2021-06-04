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

/* EngineCapabilities implementation */

#include "sles_allinclusive.h"


static SLresult IEngineCapabilities_QuerySupportedProfiles(
    SLEngineCapabilitiesItf self, SLuint16 *pProfilesSupported)
{
    SL_ENTER_INTERFACE

    if (NULL == pProfilesSupported) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        // The generic implementation doesn't implement any of the profiles, they shouldn't be
        // declared as supported. Also exclude the fake profiles BASE and OPTIONAL.
        *pProfilesSupported = USE_PROFILES &
                (USE_PROFILES_GAME | USE_PROFILES_MUSIC | USE_PROFILES_PHONE);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngineCapabilities_QueryAvailableVoices(SLEngineCapabilitiesItf self,
    SLuint16 voiceType, SLint16 *pNumMaxVoices, SLboolean *pIsAbsoluteMax, SLint16 *pNumFreeVoices)
{
    SL_ENTER_INTERFACE

    switch (voiceType) {
    case SL_VOICETYPE_2D_AUDIO:
    case SL_VOICETYPE_MIDI:
    case SL_VOICETYPE_3D_AUDIO:
    case SL_VOICETYPE_3D_MIDIOUTPUT:
        if (NULL != pNumMaxVoices)
            *pNumMaxVoices = MAX_INSTANCE - 2;
        if (NULL != pIsAbsoluteMax)
            *pIsAbsoluteMax = SL_BOOLEAN_TRUE;
        if (NULL != pNumFreeVoices)
            *pNumFreeVoices = MAX_INSTANCE - 2;
        result = SL_RESULT_SUCCESS;
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngineCapabilities_QueryNumberOfMIDISynthesizers(
    SLEngineCapabilitiesItf self, SLint16 *pNum)
{
    SL_ENTER_INTERFACE

    if (NULL == pNum) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pNum = 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngineCapabilities_QueryAPIVersion(SLEngineCapabilitiesItf self,
    SLint16 *pMajor, SLint16 *pMinor, SLint16 *pStep)
{
    SL_ENTER_INTERFACE

    if (!(NULL != pMajor && NULL != pMinor && NULL != pStep)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pMajor = 1;
        *pMinor = 0;
        *pStep = 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


SLresult IEngineCapabilities_QueryLEDCapabilities(SLEngineCapabilitiesItf self,
    SLuint32 *pIndex, SLuint32 *pLEDDeviceID, SLLEDDescriptor *pDescriptor)
{
    SL_ENTER_INTERFACE

    IEngineCapabilities *thiz = (IEngineCapabilities *) self;
    const struct LED_id_descriptor *id_descriptor;
    SLuint32 index;
    if (NULL != pIndex) {
        result = SL_RESULT_SUCCESS;
        if (NULL != pLEDDeviceID || NULL != pDescriptor) {
            index = *pIndex;
            if (index >= thiz->mMaxIndexLED) {
                result = SL_RESULT_PARAMETER_INVALID;
            } else {
                id_descriptor = &LED_id_descriptors[index];
                if (NULL != pLEDDeviceID)
                    *pLEDDeviceID = id_descriptor->id;
                if (NULL != pDescriptor)
                    *pDescriptor = *id_descriptor->descriptor;
            }
        }
        *pIndex = thiz->mMaxIndexLED;
    } else {
        result = SL_RESULT_PARAMETER_INVALID;
        if (NULL != pLEDDeviceID && NULL != pDescriptor) {
            SLuint32 id = *pLEDDeviceID;
            for (index = 0; index < thiz->mMaxIndexLED; ++index) {
                id_descriptor = &LED_id_descriptors[index];
                if (id == id_descriptor->id) {
                    *pDescriptor = *id_descriptor->descriptor;
                    result = SL_RESULT_SUCCESS;
                    break;
                }
            }
        }
    }

    SL_LEAVE_INTERFACE
}


SLresult IEngineCapabilities_QueryVibraCapabilities(SLEngineCapabilitiesItf self,
    SLuint32 *pIndex, SLuint32 *pVibraDeviceID, SLVibraDescriptor *pDescriptor)
{
    SL_ENTER_INTERFACE

    IEngineCapabilities *thiz = (IEngineCapabilities *) self;
    const struct Vibra_id_descriptor *id_descriptor;
    SLuint32 index;
    if (NULL != pIndex) {
        result = SL_RESULT_SUCCESS;
        if (NULL != pVibraDeviceID || NULL != pDescriptor) {
            index = *pIndex;
            if (index >= thiz->mMaxIndexVibra) {
                result = SL_RESULT_PARAMETER_INVALID;
            } else {
                id_descriptor = &Vibra_id_descriptors[index];
                if (NULL != pVibraDeviceID)
                    *pVibraDeviceID = id_descriptor->id;
                if (NULL != pDescriptor)
                    *pDescriptor = *id_descriptor->descriptor;
            }
        }
        *pIndex = thiz->mMaxIndexVibra;
    } else {
        result = SL_RESULT_PARAMETER_INVALID;
        if (NULL != pVibraDeviceID && NULL != pDescriptor) {
            SLuint32 id = *pVibraDeviceID;
            for (index = 0; index < thiz->mMaxIndexVibra; ++index) {
                id_descriptor = &Vibra_id_descriptors[index];
                if (id == id_descriptor->id) {
                    *pDescriptor = *id_descriptor->descriptor;
                    result = SL_RESULT_SUCCESS;
                    break;
                }
            }
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngineCapabilities_IsThreadSafe(SLEngineCapabilitiesItf self,
    SLboolean *pIsThreadSafe)
{
    SL_ENTER_INTERFACE

    if (NULL == pIsThreadSafe) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEngineCapabilities *thiz = (IEngineCapabilities *) self;
        *pIsThreadSafe = thiz->mThreadSafe;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLEngineCapabilitiesItf_ IEngineCapabilities_Itf = {
    IEngineCapabilities_QuerySupportedProfiles,
    IEngineCapabilities_QueryAvailableVoices,
    IEngineCapabilities_QueryNumberOfMIDISynthesizers,
    IEngineCapabilities_QueryAPIVersion,
    IEngineCapabilities_QueryLEDCapabilities,
    IEngineCapabilities_QueryVibraCapabilities,
    IEngineCapabilities_IsThreadSafe
};

void IEngineCapabilities_init(void *self)
{
    IEngineCapabilities *thiz = (IEngineCapabilities *) self;
    thiz->mItf = &IEngineCapabilities_Itf;
    // mThreadSafe is initialized in slCreateEngine
    const struct LED_id_descriptor *id_descriptor_LED = LED_id_descriptors;
    while (NULL != id_descriptor_LED->descriptor)
        ++id_descriptor_LED;
    thiz->mMaxIndexLED = id_descriptor_LED - LED_id_descriptors;
    const struct Vibra_id_descriptor *id_descriptor_Vibra = Vibra_id_descriptors;
    while (NULL != id_descriptor_Vibra->descriptor)
        ++id_descriptor_Vibra;
    thiz->mMaxIndexVibra = id_descriptor_Vibra - Vibra_id_descriptors;
}
