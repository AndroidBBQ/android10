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

/**************************************************************************************************
 * Equalizer functions
 ****************************/
extern void android_eq_init(audio_session_t sessionId, IEqualizer* ieq);

extern android::status_t android_eq_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, int32_t param2, void *pValue);

extern android::status_t android_eq_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, int32_t param2, void *pValue);

/**************************************************************************************************
 * BassBoost functions
 ****************************/
extern void android_bb_init(audio_session_t sessionId, IBassBoost* ibb);

extern android::status_t android_bb_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

extern android::status_t android_bb_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

/**************************************************************************************************
 * Virtualizer functions
 ****************************/
extern void android_virt_init(audio_session_t sessionId, IVirtualizer* ivi);

extern android::status_t android_virt_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

extern android::status_t android_virt_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

/**************************************************************************************************
 * PresetReverb functions
 ****************************/
extern void android_prev_init(IPresetReverb* ipr);

extern android::status_t android_prev_setPreset(const android::sp<android::AudioEffect>& pFx,
        uint16_t preset);

extern android::status_t android_prev_getPreset(const android::sp<android::AudioEffect>& pFx,
        uint16_t* preset);

/**************************************************************************************************
 * EnvironmentalReverb functions
 ****************************/
extern void android_erev_init(IEnvironmentalReverb* ier);

extern android::status_t android_erev_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

extern android::status_t android_erev_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, void *pValue);

/**************************************************************************************************
 * AEC functions
 ****************************/
extern void android_aec_init(audio_session_t sessionId, IAndroidAcousticEchoCancellation* iaec);

/**************************************************************************************************
 * AGC functions
 ****************************/
extern void android_agc_init(audio_session_t sessionId, IAndroidAutomaticGainControl* iagc);

/**************************************************************************************************
 * NS functions
 ****************************/
extern void android_ns_init(audio_session_t sessionId, IAndroidNoiseSuppression* ins);

/**************************************************************************************************
 * Generic Effect functions
 ****************************/
extern SLresult android_genericFx_queryNumEffects(SLuint32 *pNumSupportedAudioEffects);

extern SLresult android_genericFx_queryEffect(SLuint32 index, effect_descriptor_t* pDescriptor);

extern SLresult android_genericFx_createEffect(IAndroidEffect* iae, SLInterfaceID pUuid,
        audio_session_t sessionId);

extern SLresult android_genericFx_releaseEffect(IAndroidEffect* iae, SLInterfaceID pUuid);

extern SLresult android_genericFx_setEnabled(IAndroidEffect* iae, SLInterfaceID pUuid,
        SLboolean enabled);

extern SLresult android_genericFx_isEnabled(IAndroidEffect* iae, SLInterfaceID pUuid,
        SLboolean *pEnabled);

extern SLresult android_genericFx_sendCommand(IAndroidEffect* iae, SLInterfaceID pUuid,
        SLuint32 command, SLuint32 commandSize, void* pCommandData,
        SLuint32 *replySize, void *pReplyData);

extern bool android_genericFx_hasEffect(IAndroidEffect* iae, SLInterfaceID pUuid);


/**************************************************************************************************
 * EffectSend functions
 ****************************/
/**
 * sendLevel is the total energy going to the send bus. This implies that the volume attenuation
 *   should be combined with the send level for the aux level to follow volume changes.
 */
extern android::status_t android_fxSend_attach(CAudioPlayer* ap, bool attach,
        const android::sp<android::AudioEffect>& pFx, SLmillibel sendLevel);

/**
 * sendLevel is the total energy going to the send bus. This implies that the volume attenuation
 *   should be combined with the send level for the aux level to follow volume changes.
 *   This one is used by Android-specific APIs, not portable Khronos APIs.
 */
extern SLresult android_fxSend_attachToAux(CAudioPlayer* ap, SLInterfaceID pUuid,
        SLboolean attach, SLmillibel sendLevel);

/**
 * sendLevel is the total energy going to the send bus. This implies that the volume attenuation
 *   should be combined with the send level for the aux level to follow volume changes.
 */
extern android::status_t android_fxSend_setSendLevel(CAudioPlayer* ap, SLmillibel sendLevel);

/**************************************************************************************************
 * Effect-agnostic functions
 ****************************/
extern android::status_t android_fx_setParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, uint32_t paramSizeMax, void *pValue, uint32_t valueSize);

extern android::status_t android_fx_getParam(const android::sp<android::AudioEffect>& pFx,
        int32_t param, uint32_t paramSizeMax, void *pValue, uint32_t valueSize);

extern SLresult android_fx_statusToResult(android::status_t status);

extern bool android_fx_initEffectObj(audio_session_t sessionId,
        android::sp<android::AudioEffect>& effect,
        const effect_uuid_t *type);

extern bool android_fx_initEffectDescriptor(const SLInterfaceID effectId,
        effect_descriptor_t* fxDescrLoc);
