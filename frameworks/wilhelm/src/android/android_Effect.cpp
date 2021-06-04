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


#include "sles_allinclusive.h"
#include "math.h"
#include "utils/RefBase.h"
#include "utils/String16.h"

#include <system/audio_effects/effect_bassboost.h>
#include <system/audio_effects/effect_equalizer.h>
#include <system/audio_effects/effect_environmentalreverb.h>
#include <system/audio_effects/effect_presetreverb.h>
#include <system/audio_effects/effect_virtualizer.h>

#include <system/audio_effects/effect_aec.h>
#include <system/audio_effects/effect_agc.h>
#include <system/audio_effects/effect_ns.h>

#include <system/audio.h>

static const int EQUALIZER_PARAM_SIZE_MAX = sizeof(effect_param_t) + 2 * sizeof(int32_t)
        + EFFECT_STRING_LEN_MAX;

static const int BASSBOOST_PARAM_SIZE_MAX = sizeof(effect_param_t) + 2 * sizeof(int32_t);

static const int VIRTUALIZER_PARAM_SIZE_MAX = sizeof(effect_param_t) + 2 * sizeof(int32_t);

static const int ENVREVERB_PARAM_SIZE_MAX_SINGLE = sizeof(effect_param_t) + 2 * sizeof(int32_t);

static const int ENVREVERB_PARAM_SIZE_MAX_ALL = sizeof(effect_param_t) + sizeof(int32_t)
        + sizeof(s_reverb_settings);

static const int PRESETREVERB_PARAM_SIZE_MAX = sizeof(effect_param_t) + 2 * sizeof(int32_t);

static inline SLuint32 KEY_FROM_GUID(SLInterfaceID pUuid) {
    return pUuid->time_low;
}


//-----------------------------------------------------------------------------
static
uint32_t eq_paramSize(int32_t param) {
    uint32_t size;

    switch (param) {
    case EQ_PARAM_NUM_BANDS:
    case EQ_PARAM_LEVEL_RANGE:
    case EQ_PARAM_CUR_PRESET:
    case EQ_PARAM_GET_NUM_OF_PRESETS:
        size = sizeof(int32_t);
        break;
    case EQ_PARAM_BAND_LEVEL:
    case EQ_PARAM_CENTER_FREQ:
    case EQ_PARAM_BAND_FREQ_RANGE:
    case EQ_PARAM_GET_BAND:
    case EQ_PARAM_GET_PRESET_NAME:
        size = 2 * sizeof(int32_t);
        break;
    default:
        size = 2 * sizeof(int32_t);
        SL_LOGE("Trying to use an unknown EQ parameter %d", param);
        break;
    }
    return size;
}

static
uint32_t eq_valueSize(int32_t param) {
    uint32_t size;

    switch (param) {
    case EQ_PARAM_NUM_BANDS:
    case EQ_PARAM_CUR_PRESET:
    case EQ_PARAM_GET_NUM_OF_PRESETS:
    case EQ_PARAM_BAND_LEVEL:
    case EQ_PARAM_GET_BAND:
        size = sizeof(int16_t);
        break;
    case EQ_PARAM_LEVEL_RANGE:
        size = 2 * sizeof(int16_t);
        break;
    case EQ_PARAM_CENTER_FREQ:
        size = sizeof(int32_t);
        break;
    case EQ_PARAM_BAND_FREQ_RANGE:
        size = 2 * sizeof(int32_t);
        break;
    case EQ_PARAM_GET_PRESET_NAME:
        size = EFFECT_STRING_LEN_MAX;
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown EQ parameter %d", param);
        break;
    }
    return size;
}

//-----------------------------------------------------------------------------
/**
 * returns the size in bytes of the value of each bass boost parameter
 */
static
uint32_t bb_valueSize(int32_t param) {
    uint32_t size;

    switch (param) {
    case BASSBOOST_PARAM_STRENGTH_SUPPORTED:
        size = sizeof(int32_t);
        break;
    case BASSBOOST_PARAM_STRENGTH:
        size = sizeof(int16_t);
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown BassBoost parameter %d", param);
        break;
    }

    return size;
}

//-----------------------------------------------------------------------------
/**
 * returns the size in bytes of the value of each virtualizer parameter
 */
static
uint32_t virt_valueSize(int32_t param) {
    uint32_t size;

    switch (param) {
    case VIRTUALIZER_PARAM_STRENGTH_SUPPORTED:
        size = sizeof(int32_t);
        break;
    case VIRTUALIZER_PARAM_STRENGTH:
        size = sizeof(int16_t);
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown Virtualizer parameter %d", param);
        break;
    }

    return size;
}

//-----------------------------------------------------------------------------
/**
 * returns the size in bytes of the value of each environmental reverb parameter
 */
static
uint32_t erev_valueSize(int32_t param) {
    uint32_t size;

    switch (param) {
    case REVERB_PARAM_ROOM_LEVEL:
    case REVERB_PARAM_ROOM_HF_LEVEL:
    case REVERB_PARAM_REFLECTIONS_LEVEL:
    case REVERB_PARAM_REVERB_LEVEL:
        size = sizeof(int16_t); // millibel
        break;
    case REVERB_PARAM_DECAY_TIME:
    case REVERB_PARAM_REFLECTIONS_DELAY:
    case REVERB_PARAM_REVERB_DELAY:
        size = sizeof(uint32_t); // milliseconds
        break;
    case REVERB_PARAM_DECAY_HF_RATIO:
    case REVERB_PARAM_DIFFUSION:
    case REVERB_PARAM_DENSITY:
        size = sizeof(int16_t); // permille
        break;
    case REVERB_PARAM_PROPERTIES:
        size = sizeof(s_reverb_settings); // struct of all reverb properties
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown Environmental Reverb parameter %d", param);
        break;
    }

    return size;
}

//-----------------------------------------------------------------------------
android::status_t android_eq_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, int32_t param2, void *pValue)
{
     android::status_t status;
     uint32_t buf32[(EQUALIZER_PARAM_SIZE_MAX - 1) / sizeof(uint32_t) + 1];
     effect_param_t *p = (effect_param_t *)buf32;

     p->psize = eq_paramSize(param);
     *(int32_t *)p->data = param;
     if (p->psize == 2 * sizeof(int32_t)) {
         *((int32_t *)p->data + 1) = param2;
     }
     p->vsize = eq_valueSize(param);
     status = pFx->getParameter(p);
     if (android::NO_ERROR == status) {
         status = p->status;
         if (android::NO_ERROR == status) {
             memcpy(pValue, p->data + p->psize, p->vsize);
         }
     }

     return status;
 }


//-----------------------------------------------------------------------------
android::status_t android_eq_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, int32_t param2, void *pValue)
{
    android::status_t status;
    uint32_t buf32[(EQUALIZER_PARAM_SIZE_MAX - 1) / sizeof(uint32_t) + 1];
    effect_param_t *p = (effect_param_t *)buf32;

    p->psize = eq_paramSize(param);
    *(int32_t *)p->data = param;
    if (p->psize == 2 * sizeof(int32_t)) {
        *((int32_t *)p->data + 1) = param2;
    }
    p->vsize = eq_valueSize(param);
    memcpy(p->data + p->psize, pValue, p->vsize);
    status = pFx->setParameter(p);
    if (android::NO_ERROR == status) {
        status = p->status;
    }

    return status;
}

//-----------------------------------------------------------------------------
android::status_t android_bb_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    return android_fx_setParam(pFx, param, BASSBOOST_PARAM_SIZE_MAX,
            pValue, bb_valueSize(param));
}

//-----------------------------------------------------------------------------
android::status_t android_bb_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    return android_fx_getParam(pFx, param, BASSBOOST_PARAM_SIZE_MAX,
            pValue, bb_valueSize(param));
}

//-----------------------------------------------------------------------------
void android_bb_init(audio_session_t sessionId, IBassBoost* ibb) {
    SL_LOGV("session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, ibb->mBassBoostEffect,
            &ibb->mBassBoostDescriptor.type))
    {
        SL_LOGE("BassBoost effect initialization failed");
        return;
    }

    // initialize strength
    int16_t strength;
    if (android::NO_ERROR == android_bb_getParam(ibb->mBassBoostEffect,
            BASSBOOST_PARAM_STRENGTH, &strength)) {
        ibb->mStrength = (SLpermille) strength;
    }
}


//-----------------------------------------------------------------------------
void android_eq_init(audio_session_t sessionId, IEqualizer* ieq) {
    SL_LOGV("android_eq_init on session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, ieq->mEqEffect, &ieq->mEqDescriptor.type)) {
        SL_LOGE("Equalizer effect initialization failed");
        return;
    }

    // initialize number of bands, band level range, and number of presets
    uint16_t num = 0;
    if (android::NO_ERROR == android_eq_getParam(ieq->mEqEffect, EQ_PARAM_NUM_BANDS, 0, &num)) {
        ieq->mNumBands = num;
    }
    int16_t range[2] = {0, 0};
    if (android::NO_ERROR == android_eq_getParam(ieq->mEqEffect, EQ_PARAM_LEVEL_RANGE, 0, range)) {
        ieq->mBandLevelRangeMin = range[0];
        ieq->mBandLevelRangeMax = range[1];
    }

    SL_LOGV(" EQ init: num bands = %u, band range=[%d %d]mB", num, range[0], range[1]);

    // FIXME don't store presets names, they can be queried each time they're needed
    // initialize preset number and names, store in IEngine
    uint16_t numPresets = 0;
    if (android::NO_ERROR == android_eq_getParam(ieq->mEqEffect,
            EQ_PARAM_GET_NUM_OF_PRESETS, 0, &numPresets)) {
        ieq->mThis->mEngine->mEqNumPresets = numPresets;
        ieq->mNumPresets = numPresets;
    }

    object_lock_exclusive(&ieq->mThis->mEngine->mObject);
    char name[EFFECT_STRING_LEN_MAX];
    if ((0 < numPresets) && (NULL == ieq->mThis->mEngine->mEqPresetNames)) {
        ieq->mThis->mEngine->mEqPresetNames = (char **)new char *[numPresets];
        for(uint32_t i = 0 ; i < numPresets ; i++) {
            if (android::NO_ERROR == android_eq_getParam(ieq->mEqEffect,
                    EQ_PARAM_GET_PRESET_NAME, i, name)) {
                ieq->mThis->mEngine->mEqPresetNames[i] = new char[strlen(name) + 1];
                strcpy(ieq->mThis->mEngine->mEqPresetNames[i], name);
                SL_LOGV(" EQ init: presets = %u is %s", i, ieq->mThis->mEngine->mEqPresetNames[i]);
            }
        }
    }
    object_unlock_exclusive(&ieq->mThis->mEngine->mObject);

}


//-----------------------------------------------------------------------------
void android_virt_init(audio_session_t sessionId, IVirtualizer* ivi) {
    SL_LOGV("android_virt_init on session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, ivi->mVirtualizerEffect,
            &ivi->mVirtualizerDescriptor.type)) {
        SL_LOGE("Virtualizer effect initialization failed");
        return;
    }

    // initialize strength
    int16_t strength;
    if (android::NO_ERROR == android_virt_getParam(ivi->mVirtualizerEffect,
            VIRTUALIZER_PARAM_STRENGTH, &strength)) {
        ivi->mStrength = (SLpermille) strength;
    }
}

//-----------------------------------------------------------------------------
android::status_t android_virt_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    return android_fx_setParam(pFx, param, VIRTUALIZER_PARAM_SIZE_MAX,
            pValue, virt_valueSize(param));
}

//-----------------------------------------------------------------------------
android::status_t android_virt_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    return android_fx_getParam(pFx, param, VIRTUALIZER_PARAM_SIZE_MAX,
            pValue, virt_valueSize(param));
}


//-----------------------------------------------------------------------------
void android_prev_init(IPresetReverb* ipr) {
    SL_LOGV("session is implicitly %d (aux effect)", AUDIO_SESSION_OUTPUT_MIX);

    if (!android_fx_initEffectObj(AUDIO_SESSION_OUTPUT_MIX /*sessionId*/,
            ipr->mPresetReverbEffect, &ipr->mPresetReverbDescriptor.type)) {
        SL_LOGE("PresetReverb effect initialization failed");
        return;
    }

    // initialize preset
    uint16_t preset;
    if (android::NO_ERROR == android_prev_getPreset(ipr->mPresetReverbEffect, &preset)) {
        ipr->mPreset = preset;
        // enable the effect if it has a preset loaded
        ipr->mPresetReverbEffect->setEnabled(SL_REVERBPRESET_NONE != preset);
    }
}

//-----------------------------------------------------------------------------
android::status_t android_prev_setPreset(const android::sp<android::AudioEffect>& pFx,
        uint16_t preset) {
    android::status_t status = android_fx_setParam(pFx, REVERB_PARAM_PRESET,
            PRESETREVERB_PARAM_SIZE_MAX, &preset, sizeof(uint16_t));
    // enable the effect if the preset is different from SL_REVERBPRESET_NONE
    pFx->setEnabled(SL_REVERBPRESET_NONE != preset);
    return status;
}

//-----------------------------------------------------------------------------
android::status_t android_prev_getPreset(const android::sp<android::AudioEffect>& pFx,
        uint16_t* preset) {
    return android_fx_getParam(pFx, REVERB_PARAM_PRESET, PRESETREVERB_PARAM_SIZE_MAX, preset,
            sizeof(uint16_t));
}


//-----------------------------------------------------------------------------
void android_erev_init(IEnvironmentalReverb* ier) {
    SL_LOGV("session is implicitly %d (aux effect)", AUDIO_SESSION_OUTPUT_MIX);

    if (!android_fx_initEffectObj(AUDIO_SESSION_OUTPUT_MIX /*sessionId*/,
            ier->mEnvironmentalReverbEffect, &ier->mEnvironmentalReverbDescriptor.type)) {
        SL_LOGE("EnvironmentalReverb effect initialization failed");
        return;
    }

    // enable env reverb: other SL ES effects have an explicit SetEnabled() function, and the
    //  preset reverb state depends on the selected preset.
    ier->mEnvironmentalReverbEffect->setEnabled(true);

    // initialize reverb properties
    SLEnvironmentalReverbSettings properties;
    if (android::NO_ERROR == android_erev_getParam(ier->mEnvironmentalReverbEffect,
            REVERB_PARAM_PROPERTIES, &properties)) {
        ier->mProperties = properties;
    }
}

//-----------------------------------------------------------------------------
android::status_t android_erev_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    // given the size difference between a single reverb property and the whole set of reverb
    // properties, select which max size to pass to avoid allocating too much memory
    if (param == REVERB_PARAM_PROPERTIES) {
        return android_fx_setParam(pFx, param, ENVREVERB_PARAM_SIZE_MAX_ALL,
                pValue, erev_valueSize(param));
    } else {
        return android_fx_setParam(pFx, param, ENVREVERB_PARAM_SIZE_MAX_SINGLE,
                pValue, erev_valueSize(param));
    }
}

//-----------------------------------------------------------------------------
android::status_t android_erev_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {

    // given the size difference between a single reverb property and the whole set of reverb
    // properties, select which max size to pass to avoid allocating too much memory
    if (param == REVERB_PARAM_PROPERTIES) {
        return android_fx_getParam(pFx, param, ENVREVERB_PARAM_SIZE_MAX_ALL,
                pValue, erev_valueSize(param));
    } else {
        return android_fx_getParam(pFx, param, ENVREVERB_PARAM_SIZE_MAX_SINGLE,
                pValue, erev_valueSize(param));
    }
}

//-----------------------------------------------------------------------------
void android_aec_init(audio_session_t sessionId, IAndroidAcousticEchoCancellation* iaec) {
    SL_LOGV("android_aec_init on session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, iaec->mAECEffect,
            &iaec->mAECDescriptor.type)) {
        SL_LOGE("AEC effect initialization failed");
        return;
    }
}

//-----------------------------------------------------------------------------
void android_agc_init(audio_session_t sessionId, IAndroidAutomaticGainControl* iagc) {
    SL_LOGV("android_agc_init on session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, iagc->mAGCEffect,
            &iagc->mAGCDescriptor.type)) {
        SL_LOGE("AGC effect initialization failed");
        return;
    }
}

//-----------------------------------------------------------------------------
void android_ns_init(audio_session_t sessionId, IAndroidNoiseSuppression* ins) {
    SL_LOGV("android_ns_init on session %d", sessionId);

    if (!android_fx_initEffectObj(sessionId, ins->mNSEffect,
            &ins->mNSDescriptor.type)) {
        SL_LOGE("NS effect initialization failed");
        return;
    }
}

//-----------------------------------------------------------------------------
/**
 * pre-condition:
 *    ap != NULL
 *    for media players:
 *      ap->mAPlayer != 0
 *      ap->mTrackPlayer->mAudioTrack == 0
 *    for buffer queue players:
 *      ap->mAPlayer == 0
 *      ap->mTrackPlayer->mAudioTrack != 0 is optional; if no track yet then the setting is deferred
 */
android::status_t android_fxSend_attach(CAudioPlayer* ap, bool attach,
        const android::sp<android::AudioEffect>& pFx, SLmillibel sendLevel) {

    if (pFx == 0) {
        return android::INVALID_OPERATION;
    }

    // There are 3 cases:
    //  mAPlayer != 0 && mAudioTrack == 0 means playing decoded audio
    //  mAPlayer == 0 && mAudioTrack != 0 means playing PCM audio
    //  mAPlayer == 0 && mAudioTrack == 0 means player not fully configured yet
    // The asserts document and verify this.
    if (ap->mAPlayer != 0) {
        assert(ap->mTrackPlayer->mAudioTrack == 0);
        if (attach) {
            ap->mAPlayer->attachAuxEffect(pFx->id());
            ap->mAPlayer->setAuxEffectSendLevel( sles_to_android_amplification(sendLevel) );
        } else {
            ap->mAPlayer->attachAuxEffect(0);
        }
        return android::NO_ERROR;
    }

    if (ap->mTrackPlayer->mAudioTrack == 0) {
        // the player doesn't have an AudioTrack at the moment, so store this info to use it
        // when the AudioTrack becomes available
        if (attach) {
            ap->mAuxEffect = pFx;
        } else {
            ap->mAuxEffect.clear();
        }
        // we keep track of the send level, independently of the current audio player level
        ap->mAuxSendLevel = sendLevel - ap->mVolume.mLevel;
        return android::NO_ERROR;
    }

    if (attach) {
        android::status_t status = ap->mTrackPlayer->mAudioTrack->attachAuxEffect(pFx->id());
        //SL_LOGV("attachAuxEffect(%d) returned %d", pFx->id(), status);
        if (android::NO_ERROR == status) {
            status =
                ap->mTrackPlayer->mAudioTrack->setAuxEffectSendLevel(
                        sles_to_android_amplification(sendLevel) );
        }
        return status;
    } else {
        return ap->mTrackPlayer->mAudioTrack->attachAuxEffect(0);
    }
}

//-----------------------------------------------------------------------------
/**
 * pre-condition:
 *    ap != NULL
 *    ap->mOutputMix != NULL
 */
SLresult android_fxSend_attachToAux(CAudioPlayer* ap, SLInterfaceID pUuid, SLboolean attach,
        SLmillibel sendLevel) {
    COutputMix *outputMix = CAudioPlayer_GetOutputMix(ap);
    ssize_t index = outputMix->mAndroidEffect.mEffects->indexOfKey(KEY_FROM_GUID(pUuid));

    if (0 > index) {
        SL_LOGE("invalid effect ID: no such effect attached to the OutputMix");
        return SL_RESULT_PARAMETER_INVALID;
    }

    android::sp<android::AudioEffect> pFx =
                          outputMix->mAndroidEffect.mEffects->valueAt(index);
    if (pFx == 0) {
        return SL_RESULT_RESOURCE_ERROR;
    }
    if (android::NO_ERROR == android_fxSend_attach( ap, (bool) attach, pFx, sendLevel) ) {
        return SL_RESULT_SUCCESS;
    } else {
        return SL_RESULT_RESOURCE_ERROR;
    }

}

//-----------------------------------------------------------------------------
/**
 * pre-condition:
 *    ap != NULL
 *    for media players:
 *      ap->mAPlayer != 0
 *      ap->mTrackPlayer->mAudioTrack == 0
 *    for buffer queue players:
 *      ap->mAPlayer == 0
 *      ap->mTrackPlayer->mAudioTrack != 0 is optional; if no track yet then the setting is deferred
 */
android::status_t android_fxSend_setSendLevel(CAudioPlayer* ap, SLmillibel sendLevel) {
    // we keep track of the send level, independently of the current audio player level
    ap->mAuxSendLevel = sendLevel - ap->mVolume.mLevel;

    if (ap->mAPlayer != 0) {
        assert(ap->mTrackPlayer->mAudioTrack == 0);
        ap->mAPlayer->setAuxEffectSendLevel( sles_to_android_amplification(sendLevel) );
        return android::NO_ERROR;
    }

    if (ap->mTrackPlayer->mAudioTrack == 0) {
        return android::NO_ERROR;
    }

    return ap->mTrackPlayer->mAudioTrack->setAuxEffectSendLevel(
            sles_to_android_amplification(sendLevel) );
}

//-----------------------------------------------------------------------------
android::status_t android_fx_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, uint32_t paramSizeMax, void *pValue, uint32_t valueSize)
{

    android::status_t status;
    uint32_t buf32[(paramSizeMax - 1) / sizeof(uint32_t) + 1];
    effect_param_t *p = (effect_param_t *)buf32;

    p->psize = sizeof(int32_t);
    *(int32_t *)p->data = param;
    p->vsize = valueSize;
    memcpy(p->data + p->psize, pValue, p->vsize);
    status = pFx->setParameter(p);
    if (android::NO_ERROR == status) {
        status = p->status;
    }
    return status;
}


//-----------------------------------------------------------------------------
android::status_t android_fx_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, uint32_t paramSizeMax, void *pValue, uint32_t valueSize)
{
    android::status_t status;
    uint32_t buf32[(paramSizeMax - 1) / sizeof(uint32_t) + 1];
    effect_param_t *p = (effect_param_t *)buf32;

    p->psize = sizeof(int32_t);
    *(int32_t *)p->data = param;
    p->vsize = valueSize;
    status = pFx->getParameter(p);
    if (android::NO_ERROR == status) {
        status = p->status;
        if (android::NO_ERROR == status) {
            memcpy(pValue, p->data + p->psize, p->vsize);
        }
    }

    return status;
}


//-----------------------------------------------------------------------------
SLresult android_fx_statusToResult(android::status_t status) {

    if ((android::INVALID_OPERATION == status) || (android::DEAD_OBJECT == status)) {
        return SL_RESULT_CONTROL_LOST;
    } else {
        return SL_RESULT_SUCCESS;
    }
}


//-----------------------------------------------------------------------------
bool android_fx_initEffectObj(audio_session_t sessionId, android::sp<android::AudioEffect>& effect,
        const effect_uuid_t *type) {
    //SL_LOGV("android_fx_initEffectObj on session %d", sessionId);

    effect = new android::AudioEffect(type, android::String16(), EFFECT_UUID_NULL,
            0,// priority
            0,// effect callback
            0,// callback data
            sessionId,// session ID
            0 );// output

    android::status_t status = effect->initCheck();
    if (android::NO_ERROR != status) {
        effect.clear();
        SL_LOGE("Effect initCheck() returned %d", status);
        return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
bool android_fx_initEffectDescriptor(const SLInterfaceID effectId,
        effect_descriptor_t* fxDescrLoc) {
    uint32_t numEffects = 0;
    effect_descriptor_t descriptor;
    bool foundEffect = false;

    // any effects?
    android::status_t res = android::AudioEffect::queryNumberEffects(&numEffects);
    if (android::NO_ERROR != res) {
        SL_LOGE("unable to find any effects.");
        goto effectError;
    }

    // request effect in the effects?
    for (uint32_t i=0 ; i < numEffects ; i++) {
        res = android::AudioEffect::queryEffect(i, &descriptor);
        if ((android::NO_ERROR == res) &&
                (0 == memcmp(effectId, &descriptor.type, sizeof(effect_uuid_t)))) {
            SL_LOGV("found effect %d %s", i, descriptor.name);
            foundEffect = true;
            break;
        }
    }
    if (foundEffect) {
        memcpy(fxDescrLoc, &descriptor, sizeof(effect_descriptor_t));
    } else {
        SL_LOGE("unable to find an implementation for the requested effect.");
        goto effectError;
    }

    return true;

effectError:
    // the requested effect wasn't found
    memset(fxDescrLoc, 0, sizeof(effect_descriptor_t));

    return false;
}

//-----------------------------------------------------------------------------
SLresult android_genericFx_queryNumEffects(SLuint32 *pNumSupportedAudioEffects) {

    if (NULL == pNumSupportedAudioEffects) {
        return SL_RESULT_PARAMETER_INVALID;
    }

    android::status_t status =
            android::AudioEffect::queryNumberEffects((uint32_t*)pNumSupportedAudioEffects);

    SLresult result = SL_RESULT_SUCCESS;
    switch (status) {
        case android::NO_ERROR:
            result = SL_RESULT_SUCCESS;
            break;
        case android::PERMISSION_DENIED:
            result = SL_RESULT_PERMISSION_DENIED;
            break;
        case android::NO_INIT:
            result = SL_RESULT_RESOURCE_ERROR;
            break;
        case android::BAD_VALUE:
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        default:
            result = SL_RESULT_INTERNAL_ERROR;
            SL_LOGE("received invalid status %d from AudioEffect::queryNumberEffects()", status);
            break;
    }
    return result;
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_queryEffect(SLuint32 index, effect_descriptor_t* pDescriptor) {

    if (NULL == pDescriptor) {
        return SL_RESULT_PARAMETER_INVALID;
    }

    android::status_t status =
                android::AudioEffect::queryEffect(index, pDescriptor);

    SLresult result = SL_RESULT_SUCCESS;
    if (android::NO_ERROR != status) {
        switch (status) {
        case android::PERMISSION_DENIED:
            result = SL_RESULT_PERMISSION_DENIED;
            break;
        case android::NO_INIT:
        case android::INVALID_OPERATION:
            result = SL_RESULT_RESOURCE_ERROR;
            break;
        case android::BAD_VALUE:
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        default:
            result = SL_RESULT_INTERNAL_ERROR;
            SL_LOGE("received invalid status %d from AudioEffect::queryNumberEffects()", status);
            break;
        }
        // an error occurred, reset the effect descriptor
        memset(pDescriptor, 0, sizeof(effect_descriptor_t));
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_createEffect(IAndroidEffect* iae, SLInterfaceID pUuid,
        audio_session_t sessionId)
{

    SLresult result = SL_RESULT_SUCCESS;

    // does this effect already exist?
    if (0 <= iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid))) {
        return result;
    }

    // create new effect
    android::sp<android::AudioEffect> pFx = new android::AudioEffect(
            NULL, // not using type to create effect
            android::String16(),
            (const effect_uuid_t*)pUuid,
            0,// priority
            0,// effect callback
            0,// callback data
            sessionId,
            0 );// output

    // verify effect was successfully created before storing it
    android::status_t status = pFx->initCheck();
    if (android::NO_ERROR != status) {
        SL_LOGE("AudioEffect initCheck() returned %d, effect will not be stored", status);
        result = SL_RESULT_RESOURCE_ERROR;
    } else {
        SL_LOGV("AudioEffect successfully created on session %d", sessionId);
        iae->mEffects->add(KEY_FROM_GUID(pUuid), pFx);
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_releaseEffect(IAndroidEffect* iae, SLInterfaceID pUuid) {

    ssize_t index = iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid));

    if (0 > index) {
        return SL_RESULT_PARAMETER_INVALID;
    } else {
        iae->mEffects->removeItem(index);
        return SL_RESULT_SUCCESS;
    }
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_setEnabled(IAndroidEffect* iae, SLInterfaceID pUuid, SLboolean enabled) {

    ssize_t index = iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid));

    if (0 > index) {
        return SL_RESULT_PARAMETER_INVALID;
    } else {
        android::sp<android::AudioEffect> pFx = iae->mEffects->valueAt(index);
        android::status_t status = pFx->setEnabled(SL_BOOLEAN_TRUE == enabled);
        return android_fx_statusToResult(status);
    }
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_isEnabled(IAndroidEffect* iae, SLInterfaceID pUuid, SLboolean *pEnabled)
{
    ssize_t index = iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid));

    if (0 > index) {
        return SL_RESULT_PARAMETER_INVALID;
    } else {
        android::sp<android::AudioEffect> pFx = iae->mEffects->valueAt(index);
        *pEnabled = (SLboolean) pFx->getEnabled();
        return SL_RESULT_SUCCESS;
    }
}


//-----------------------------------------------------------------------------
SLresult android_genericFx_sendCommand(IAndroidEffect* iae, SLInterfaceID pUuid,
        SLuint32 command, SLuint32 commandSize, void* pCommandData,
        SLuint32 *replySize, void *pReplyData) {

    ssize_t index = iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid));

    if (0 > index) {
        return SL_RESULT_PARAMETER_INVALID;
    } else {
        android::sp<android::AudioEffect> pFx = iae->mEffects->valueAt(index);
        android::status_t status = pFx->command(
                (uint32_t) command,
                (uint32_t) commandSize,
                pCommandData,
                (uint32_t*)replySize,
                pReplyData);
        if (android::BAD_VALUE == status) {
                return SL_RESULT_PARAMETER_INVALID;
        } else {
            return SL_RESULT_SUCCESS;
        }
    }
}

//-----------------------------------------------------------------------------
/**
 * returns true if the given effect id is present in the AndroidEffect interface
 */
bool android_genericFx_hasEffect(IAndroidEffect* iae, SLInterfaceID pUuid) {
    return( 0 <= iae->mEffects->indexOfKey(KEY_FROM_GUID(pUuid)));
}

//-----------------------------------------------------------------------------
static const int AEC_PARAM_SIZE_MAX = sizeof(effect_param_t) + (2 * sizeof(int32_t));
/**
 * returns the size in bytes of the value of each acoustic echo cancellation parameter
 */
uint32_t aec_valueSize(int32_t param) {
    uint32_t size;
    switch (param) {
    case AEC_PARAM_ECHO_DELAY:
        size = sizeof(int32_t);
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown Acoustic Echo Cancellation parameter %d", param);
        break;
    }

    return size;
}

android::status_t android_aec_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {
    return android_fx_setParam(pFx, param, AEC_PARAM_SIZE_MAX,
            pValue, aec_valueSize(param));
}

android::status_t android_aec_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {
    return android_fx_getParam(pFx, param, AEC_PARAM_SIZE_MAX,
            pValue, aec_valueSize(param));
}

//-----------------------------------------------------------------------------
static const int AGC_PARAM_SIZE_MAX = sizeof(effect_param_t) + (2 * sizeof(int16_t)) + sizeof(bool);
/**
 * returns the size in bytes of the value of each automatic gain control parameter
 */
uint32_t agc_valueSize(int32_t param) {
    uint32_t size;
    switch (param) {
    case AGC_PARAM_TARGET_LEVEL:
    case AGC_PARAM_COMP_GAIN:
        size = sizeof(int16_t);
        break;
    case AGC_PARAM_LIMITER_ENA:
        size = sizeof(bool);
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown Automatic Gain Control parameter %d", param);
        break;
    }

    return size;
}

android::status_t android_agc_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {
    return android_fx_setParam(pFx, param, AGC_PARAM_SIZE_MAX,
            pValue, agc_valueSize(param));
}

android::status_t android_agc_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue) {
    return android_fx_getParam(pFx, param, AGC_PARAM_SIZE_MAX,
            pValue, agc_valueSize(param));
}

//-----------------------------------------------------------------------------
static const int NS_PARAM_SIZE_MAX = sizeof(effect_param_t) + 2 * sizeof(int32_t);
/**
 * returns the size in bytes of the value of each noise suppression parameter
 */
uint32_t ns_valueSize(int32_t param) {
    uint32_t size;
    switch (param) {
    case NS_PARAM_LEVEL:
        size = sizeof(int32_t);
        break;
    default:
        size = sizeof(int32_t);
        SL_LOGE("Trying to access an unknown Noise suppression parameter %d", param);
        break;
    }

    return size;
}

android::status_t android_ns_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue)
{
    return android_fx_setParam(pFx, param, NS_PARAM_SIZE_MAX,
            pValue, ns_valueSize(param));
}

android::status_t android_ns_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue)
{
    return android_fx_getParam(pFx, param, NS_PARAM_SIZE_MAX,
            pValue, ns_valueSize(param));
}
