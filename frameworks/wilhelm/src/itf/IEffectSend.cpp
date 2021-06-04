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

/* EffectSend implementation */

#include "sles_allinclusive.h"


/** \brief Maps AUX index to OutputMix interface index */

static const unsigned char AUX_to_MPH[AUX_MAX] = {
    MPH_ENVIRONMENTALREVERB,
    MPH_PRESETREVERB
};


/** \brief This is a private function that validates the effect interface specified by the
 *  application when it calls EnableEffectSend, IsEnabled, SetSendLevel, or GetSendLevel.
 *  For the interface to be valid, it has to satisfy these requirements:
 *   - object is an audio player (MIDI player is not supported yet)
 *   - audio sink is an output mix
 *   - interface was exposed at object creation time or by DynamicInterface::AddInterface
 *   - interface was "gotten" with Object::GetInterface
 */

static struct EnableLevel *getEnableLevel(IEffectSend *thiz, const void *pAuxEffect)
{
    // Make sure this effect send is on an audio player, not a MIDI player
    CAudioPlayer *audioPlayer = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
        (CAudioPlayer *) thiz->mThis : NULL;
    if (NULL == audioPlayer) {
        return NULL;
    }
    // Get the output mix for this player
    COutputMix *outputMix = CAudioPlayer_GetOutputMix(audioPlayer);
    unsigned aux;
    if (pAuxEffect == &outputMix->mEnvironmentalReverb.mItf) {
        aux = AUX_ENVIRONMENTALREVERB;
    } else if (pAuxEffect == &outputMix->mPresetReverb.mItf) {
        aux = AUX_PRESETREVERB;
    } else {
        SL_LOGE("EffectSend on unknown aux effect %p", pAuxEffect);
        return NULL;
    }
    assert(aux < AUX_MAX);
    // Validate that the application has a valid interface for the effect.  The interface must have
    // been exposed at object creation time or by DynamicInterface::AddInterface, and it also must
    // have been "gotten" with Object::GetInterface.
    unsigned MPH = AUX_to_MPH[aux];
    int index = MPH_to_OutputMix[MPH];
    if (0 > index) {
        SL_LOGE("EffectSend aux=%u MPH=%u", aux, MPH);
        return NULL;
    }
    unsigned mask = 1 << index;
    object_lock_shared(&outputMix->mObject);
    SLuint32 state = outputMix->mObject.mInterfaceStates[index];
    mask &= outputMix->mObject.mGottenMask;
    object_unlock_shared(&outputMix->mObject);
    switch (state) {
    case INTERFACE_EXPOSED:
    case INTERFACE_ADDED:
    case INTERFACE_SUSPENDED:
    case INTERFACE_SUSPENDING:
    case INTERFACE_RESUMING_1:
    case INTERFACE_RESUMING_2:
        if (mask) {
            return &thiz->mEnableLevels[aux];
        }
        SL_LOGE("EffectSend no GetInterface yet");
        break;
    default:
        SL_LOGE("EffectSend invalid interface state %u", state);
        break;
    }
    return NULL;
}

#if defined(ANDROID)
/** \brief This is a private function that translates an Android effect framework status code
 *  to the SL ES result code used in the EnableEffectSend() function of the SLEffectSendItf
 *  interface.
 */
static SLresult translateEnableFxSendError(android::status_t status) {
    switch (status) {
    case android::NO_ERROR:
        return SL_RESULT_SUCCESS;
    case android::INVALID_OPERATION:
    case android::BAD_VALUE:
    default:
        SL_LOGE("EffectSend status %u", status);
        return SL_RESULT_RESOURCE_ERROR;
    }
}
#endif


static SLresult IEffectSend_EnableEffectSend(SLEffectSendItf self,
    const void *pAuxEffect, SLboolean enable, SLmillibel initialLevel)
{
    SL_ENTER_INTERFACE

    //if (!((SL_MILLIBEL_MIN <= initialLevel) && (initialLevel <= 0))) {
    // comparison (SL_MILLIBEL_MIN <= initialLevel) is always true due to range of SLmillibel
    if (!(initialLevel <= 0)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        struct EnableLevel *enableLevel = getEnableLevel(thiz, pAuxEffect);
        if (NULL == enableLevel) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            interface_lock_exclusive(thiz);
            enableLevel->mEnable = SL_BOOLEAN_FALSE != enable; // normalize
            enableLevel->mSendLevel = initialLevel;
#if !defined(ANDROID)
            result = SL_RESULT_SUCCESS;
#else
            // TODO do not repeat querying of CAudioPlayer, done inside getEnableLevel()
            CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                    (CAudioPlayer *) thiz->mThis : NULL;
            // note that if this was a MIDI player, getEnableLevel would have returned NULL
            assert(NULL != ap);
            // check which effect the send is attached to, attach and set level
            COutputMix *outputMix = CAudioPlayer_GetOutputMix(ap);
            // the initial send level set here is the total energy on the aux bus,
            //  so it must take into account the player volume level
            if (pAuxEffect == &outputMix->mPresetReverb.mItf) {
                result = translateEnableFxSendError(android_fxSend_attach(ap, (bool) enable,
                    outputMix->mPresetReverb.mPresetReverbEffect,
                    initialLevel + ap->mVolume.mLevel));
            } else if (pAuxEffect == &outputMix->mEnvironmentalReverb.mItf) {
                result = translateEnableFxSendError(android_fxSend_attach(ap, (bool) enable,
                    outputMix->mEnvironmentalReverb.mEnvironmentalReverbEffect,
                    initialLevel + ap->mVolume.mLevel));
            } else {
                SL_LOGE("EffectSend unknown aux effect %p", pAuxEffect);
                result = SL_RESULT_PARAMETER_INVALID;
            }
#endif
            interface_unlock_exclusive(thiz);
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEffectSend_IsEnabled(SLEffectSendItf self,
    const void *pAuxEffect, SLboolean *pEnable)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnable) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        struct EnableLevel *enableLevel = getEnableLevel(thiz, pAuxEffect);
        if (NULL == enableLevel) {
            *pEnable = SL_BOOLEAN_FALSE;
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            interface_lock_shared(thiz);
            SLboolean enable = enableLevel->mEnable;
            interface_unlock_shared(thiz);
            *pEnable = enable;
            result = SL_RESULT_SUCCESS;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEffectSend_SetDirectLevel(SLEffectSendItf self, SLmillibel directLevel)
{
    SL_ENTER_INTERFACE

    //if (!((SL_MILLIBEL_MIN <= directLevel) && (directLevel <= 0))) {
    // comparison (SL_MILLIBEL_MIN <= directLevel) is always true due to range of SLmillibel
    if (!(directLevel <= 0)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        interface_lock_exclusive(thiz);
        CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                (CAudioPlayer *) thiz->mThis : NULL;
        if (NULL != ap) {
            SLmillibel oldDirectLevel = ap->mDirectLevel;
            if (oldDirectLevel != directLevel) {
                ap->mDirectLevel = directLevel;
#if defined(ANDROID)
                ap->mAmplFromDirectLevel = sles_to_android_amplification(directLevel);
                interface_unlock_exclusive_attributes(thiz, ATTR_GAIN);
#else
                interface_unlock_exclusive(thiz);
#endif
            } else {
                interface_unlock_exclusive(thiz);
            }
        } else {
            // MIDI player is silently not supported
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEffectSend_GetDirectLevel(SLEffectSendItf self, SLmillibel *pDirectLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pDirectLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        interface_lock_shared(thiz);
        CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                (CAudioPlayer *) thiz->mThis : NULL;
        if (NULL != ap) {
            *pDirectLevel = ap->mDirectLevel;
        } else {
            // MIDI player is silently not supported
            *pDirectLevel = 0;
        }
        interface_unlock_shared(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEffectSend_SetSendLevel(SLEffectSendItf self, const void *pAuxEffect,
    SLmillibel sendLevel)
{
    SL_ENTER_INTERFACE

    //if (!((SL_MILLIBEL_MIN <= sendLevel) && (sendLevel <= 0))) {
    // comparison (SL_MILLIBEL_MIN <= sendLevel) is always true due to range of SLmillibel
    if (!(sendLevel <= 0)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        struct EnableLevel *enableLevel = getEnableLevel(thiz, pAuxEffect);
        if (NULL == enableLevel) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            result = SL_RESULT_SUCCESS;
            // EnableEffectSend is exclusive, so this has to be also
            interface_lock_exclusive(thiz);
            enableLevel->mSendLevel = sendLevel;
#if defined(ANDROID)
            CAudioPlayer *ap = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
                    (CAudioPlayer *) thiz->mThis : NULL;
            if (NULL != ap) {
                // the send level set here is the total energy on the aux bus, so it must take
                // into account the player volume level
                result = android_fxSend_setSendLevel(ap, sendLevel + ap->mVolume.mLevel);
            }
#endif
            interface_unlock_exclusive(thiz);

        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEffectSend_GetSendLevel(SLEffectSendItf self, const void *pAuxEffect,
    SLmillibel *pSendLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pSendLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEffectSend *thiz = (IEffectSend *) self;
        struct EnableLevel *enableLevel = getEnableLevel(thiz, pAuxEffect);
        if (NULL == enableLevel) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            interface_lock_shared(thiz);
            SLmillibel sendLevel = enableLevel->mSendLevel;
            interface_unlock_shared(thiz);
            *pSendLevel = sendLevel;
            result = SL_RESULT_SUCCESS;
        }
    }

    SL_LEAVE_INTERFACE
}


static const struct SLEffectSendItf_ IEffectSend_Itf = {
    IEffectSend_EnableEffectSend,
    IEffectSend_IsEnabled,
    IEffectSend_SetDirectLevel,
    IEffectSend_GetDirectLevel,
    IEffectSend_SetSendLevel,
    IEffectSend_GetSendLevel
};

void IEffectSend_init(void *self)
{
    IEffectSend *thiz = (IEffectSend *) self;
    thiz->mItf = &IEffectSend_Itf;
    struct EnableLevel *enableLevel = thiz->mEnableLevels;
    unsigned aux;
    for (aux = 0; aux < AUX_MAX; ++aux, ++enableLevel) {
        enableLevel->mEnable = SL_BOOLEAN_FALSE;
        enableLevel->mSendLevel = SL_MILLIBEL_MIN;
    }
}
