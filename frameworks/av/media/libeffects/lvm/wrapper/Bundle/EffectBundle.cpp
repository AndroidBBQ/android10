/*
 * Copyright (C) 2010-2010 NXP Software
 * Copyright (C) 2009 The Android Open Source Project
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
#ifndef LVM_FLOAT
typedef float LVM_FLOAT;
#endif
#define LOG_TAG "Bundle"
#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array)[0])
//#define LOG_NDEBUG 0

#include <assert.h>
#include <inttypes.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#include <audio_utils/primitives.h>
#include <log/log.h>

#include "EffectBundle.h"
#include "math.h"

// effect_handle_t interface implementation for bass boost
extern "C" const struct effect_interface_s gLvmEffectInterface;

// Turn on VERY_VERY_VERBOSE_LOGGING to log parameter get and set for effects.

//#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while (false)
#endif

#define LVM_ERROR_CHECK(LvmStatus, callingFunc, calledFunc){\
        if ((LvmStatus) == LVM_NULLADDRESS){\
            ALOGV("\tLVM_ERROR : Parameter error - "\
                    "null pointer returned by %s in %s\n\n\n\n", callingFunc, calledFunc);\
        }\
        if ((LvmStatus) == LVM_ALIGNMENTERROR){\
            ALOGV("\tLVM_ERROR : Parameter error - "\
                    "bad alignment returned by %s in %s\n\n\n\n", callingFunc, calledFunc);\
        }\
        if ((LvmStatus) == LVM_INVALIDNUMSAMPLES){\
            ALOGV("\tLVM_ERROR : Parameter error - "\
                    "bad number of samples returned by %s in %s\n\n\n\n", callingFunc, calledFunc);\
        }\
        if ((LvmStatus) == LVM_OUTOFRANGE){\
            ALOGV("\tLVM_ERROR : Parameter error - "\
                    "out of range returned by %s in %s\n", callingFunc, calledFunc);\
        }\
    }

// Namespaces
namespace android {
namespace {

// Flag to allow a one time init of global memory, only happens on first call ever
int LvmInitFlag = LVM_FALSE;
SessionContext GlobalSessionMemory[LVM_MAX_SESSIONS];
int SessionIndex[LVM_MAX_SESSIONS];

/* local functions */
#define CHECK_ARG(cond) {                     \
    if (!(cond)) {                            \
        ALOGV("\tLVM_ERROR : Invalid argument: "#cond);      \
        return -EINVAL;                       \
    }                                         \
}


// NXP SW BassBoost UUID
const effect_descriptor_t gBassBoostDescriptor = {
        {0x0634f220, 0xddd4, 0x11db, 0xa0fc, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b }},
        {0x8631f300, 0x72e2, 0x11df, 0xb57e, {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}}, // uuid
        EFFECT_CONTROL_API_VERSION,
        (EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_FIRST | EFFECT_FLAG_DEVICE_IND
        | EFFECT_FLAG_VOLUME_CTRL),
        BASS_BOOST_CUP_LOAD_ARM9E,
        BUNDLE_MEM_USAGE,
        "Dynamic Bass Boost",
        "NXP Software Ltd.",
};

// NXP SW Virtualizer UUID
const effect_descriptor_t gVirtualizerDescriptor = {
        {0x37cc2c00, 0xdddd, 0x11db, 0x8577, {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}},
        {0x1d4033c0, 0x8557, 0x11df, 0x9f2d, {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}},
        EFFECT_CONTROL_API_VERSION,
        (EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_LAST | EFFECT_FLAG_DEVICE_IND
        | EFFECT_FLAG_VOLUME_CTRL),
        VIRTUALIZER_CUP_LOAD_ARM9E,
        BUNDLE_MEM_USAGE,
        "Virtualizer",
        "NXP Software Ltd.",
};

// NXP SW Equalizer UUID
const effect_descriptor_t gEqualizerDescriptor = {
        {0x0bed4300, 0xddd6, 0x11db, 0x8f34, {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}}, // type
        {0xce772f20, 0x847d, 0x11df, 0xbb17, {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}}, // uuid Eq NXP
        EFFECT_CONTROL_API_VERSION,
        (EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_FIRST | EFFECT_FLAG_VOLUME_CTRL),
        EQUALIZER_CUP_LOAD_ARM9E,
        BUNDLE_MEM_USAGE,
        "Equalizer",
        "NXP Software Ltd.",
};

// NXP SW Volume UUID
const effect_descriptor_t gVolumeDescriptor = {
        {0x09e8ede0, 0xddde, 0x11db, 0xb4f6, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b }},
        {0x119341a0, 0x8469, 0x11df, 0x81f9, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b }}, //uuid VOL NXP
        EFFECT_CONTROL_API_VERSION,
        (EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_LAST | EFFECT_FLAG_VOLUME_CTRL),
        VOLUME_CUP_LOAD_ARM9E,
        BUNDLE_MEM_USAGE,
        "Volume",
        "NXP Software Ltd.",
};

//--- local function prototypes
void LvmGlobalBundle_init      (void);
int  LvmBundle_init            (EffectContext *pContext);
int  LvmEffect_enable          (EffectContext *pContext);
int  LvmEffect_disable         (EffectContext *pContext);
void LvmEffect_free            (EffectContext *pContext);
int  Effect_setConfig          (EffectContext *pContext, effect_config_t *pConfig);
void Effect_getConfig          (EffectContext *pContext, effect_config_t *pConfig);
int  BassBoost_setParameter    (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t       valueSize,
                                void          *pValue);
int  BassBoost_getParameter    (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t      *pValueSize,
                                void          *pValue);
int  Virtualizer_setParameter  (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t       valueSize,
                                void          *pValue);
int  Virtualizer_getParameter  (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t      *pValueSize,
                                void          *pValue);
int  Equalizer_setParameter    (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t       valueSize,
                                void          *pValue);
int  Equalizer_getParameter    (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t      *pValueSize,
                                void          *pValue);
int  Volume_setParameter       (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t       valueSize,
                                void          *pValue);
int  Volume_getParameter       (EffectContext *pContext,
                                uint32_t       paramSize,
                                void          *pParam,
                                uint32_t      *pValueSize,
                                void          *pValue);
int Effect_setEnabled(EffectContext *pContext, bool enabled);

/* Effect Library Interface Implementation */

extern "C" int EffectCreate(const effect_uuid_t *uuid,
                            int32_t             sessionId,
                            int32_t             ioId __unused,
                            effect_handle_t  *pHandle){
    int ret = 0;
    int sessionNo = -1;
    int i;
    EffectContext *pContext = NULL;
    bool newBundle = false;
    SessionContext *pSessionContext;

    ALOGV("\n\tEffectCreate start session %d", sessionId);

    if (pHandle == NULL || uuid == NULL){
        ALOGV("\tLVM_ERROR : EffectCreate() called with NULL pointer");
        ret = -EINVAL;
        goto exit;
    }

    if(LvmInitFlag == LVM_FALSE){
        LvmInitFlag = LVM_TRUE;
        ALOGV("\tEffectCreate - Initializing all global memory");
        LvmGlobalBundle_init();
    }

    // Find sessionNo: if one already exists for the sessionId use it,
    // otherwise choose the first available empty slot.
    for(i=0; i<LVM_MAX_SESSIONS; i++){
        if (SessionIndex[i] == sessionId) {
            sessionNo = i;
            break;
        }
        if (sessionNo < 0 && SessionIndex[i] == LVM_UNUSED_SESSION) {
            sessionNo = i;
            // do not break; allow loop to continue to search for a sessionId match.
        }
    }
    if (sessionNo < 0) {
        ALOGV("\tLVM_ERROR : Cannot find memory to allocate for current session");
        ret = -EINVAL;
        goto exit;
    }

    SessionIndex[sessionNo] = sessionId;
    ALOGV("\tEffectCreate: Allocating sessionNo %d for sessionId %d\n", sessionNo, sessionId);

    pContext = new EffectContext;

    // If this is the first create in this session
    if(GlobalSessionMemory[sessionNo].bBundledEffectsEnabled == LVM_FALSE){
        ALOGV("\tEffectCreate - This is the first effect in current sessionId %d sessionNo %d",
                sessionId, sessionNo);

        GlobalSessionMemory[sessionNo].bBundledEffectsEnabled = LVM_TRUE;
        GlobalSessionMemory[sessionNo].pBundledContext        = new BundledEffectContext;
        newBundle = true;

        pContext->pBundledContext = GlobalSessionMemory[sessionNo].pBundledContext;
        pContext->pBundledContext->SessionNo                = sessionNo;
        pContext->pBundledContext->SessionId                = sessionId;
        pContext->pBundledContext->hInstance                = NULL;
        pContext->pBundledContext->bVolumeEnabled           = LVM_FALSE;
        pContext->pBundledContext->bEqualizerEnabled        = LVM_FALSE;
        pContext->pBundledContext->bBassEnabled             = LVM_FALSE;
        pContext->pBundledContext->bBassTempDisabled        = LVM_FALSE;
        pContext->pBundledContext->bVirtualizerEnabled      = LVM_FALSE;
        pContext->pBundledContext->bVirtualizerTempDisabled = LVM_FALSE;
        pContext->pBundledContext->nOutputDevice            = AUDIO_DEVICE_NONE;
        pContext->pBundledContext->nVirtualizerForcedDevice = AUDIO_DEVICE_NONE;
        pContext->pBundledContext->NumberEffectsEnabled     = 0;
        pContext->pBundledContext->NumberEffectsCalled      = 0;
        pContext->pBundledContext->firstVolume              = LVM_TRUE;
        pContext->pBundledContext->volume                   = 0;

        #ifdef LVM_PCM
        char fileName[256];
        snprintf(fileName, 256, "/data/tmp/bundle_%p_pcm_in.pcm", pContext->pBundledContext);
        pContext->pBundledContext->PcmInPtr = fopen(fileName, "w");
        if (pContext->pBundledContext->PcmInPtr == NULL) {
            ALOGV("cannot open %s", fileName);
            ret = -EINVAL;
            goto exit;
        }

        snprintf(fileName, 256, "/data/tmp/bundle_%p_pcm_out.pcm", pContext->pBundledContext);
        pContext->pBundledContext->PcmOutPtr = fopen(fileName, "w");
        if (pContext->pBundledContext->PcmOutPtr == NULL) {
            ALOGV("cannot open %s", fileName);
            fclose(pContext->pBundledContext->PcmInPtr);
           pContext->pBundledContext->PcmInPtr = NULL;
           ret = -EINVAL;
           goto exit;
        }
        #endif

        /* Saved strength is used to return the exact strength that was used in the set to the get
         * because we map the original strength range of 0:1000 to 1:15, and this will avoid
         * quantisation like effect when returning
         */
        pContext->pBundledContext->BassStrengthSaved        = 0;
        pContext->pBundledContext->VirtStrengthSaved        = 0;
        pContext->pBundledContext->CurPreset                = PRESET_CUSTOM;
        pContext->pBundledContext->levelSaved               = 0;
        pContext->pBundledContext->bMuteEnabled             = LVM_FALSE;
        pContext->pBundledContext->bStereoPositionEnabled   = LVM_FALSE;
        pContext->pBundledContext->positionSaved            = 0;
        pContext->pBundledContext->workBuffer               = NULL;
        pContext->pBundledContext->frameCount               = -1;
        pContext->pBundledContext->SamplesToExitCountVirt   = 0;
        pContext->pBundledContext->SamplesToExitCountBb     = 0;
        pContext->pBundledContext->SamplesToExitCountEq     = 0;
#if defined(BUILD_FLOAT) && !defined(NATIVE_FLOAT_BUFFER)
        pContext->pBundledContext->pInputBuffer             = NULL;
        pContext->pBundledContext->pOutputBuffer            = NULL;
#endif
        for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
            pContext->pBundledContext->bandGaindB[i] = EQNB_5BandSoftPresets[i];
        }
        pContext->pBundledContext->effectProcessCalled      = 0;
        pContext->pBundledContext->effectInDrain            = 0;

        ALOGV("\tEffectCreate - Calling LvmBundle_init");
        ret = LvmBundle_init(pContext);

        if (ret < 0){
            ALOGV("\tLVM_ERROR : EffectCreate() Bundle init failed");
            goto exit;
        }
    }
    else{
        ALOGV("\tEffectCreate - Assigning memory for previously created effect on sessionNo %d",
                sessionNo);
        pContext->pBundledContext =
                GlobalSessionMemory[sessionNo].pBundledContext;
    }
    ALOGV("\tEffectCreate - pBundledContext is %p", pContext->pBundledContext);

    pSessionContext = &GlobalSessionMemory[pContext->pBundledContext->SessionNo];

    // Create each Effect
    if (memcmp(uuid, &gBassBoostDescriptor.uuid, sizeof(effect_uuid_t)) == 0){
        // Create Bass Boost
        ALOGV("\tEffectCreate - Effect to be created is LVM_BASS_BOOST");
        pSessionContext->bBassInstantiated = LVM_TRUE;
        pContext->pBundledContext->SamplesToExitCountBb = 0;

        pContext->itfe       = &gLvmEffectInterface;
        pContext->EffectType = LVM_BASS_BOOST;
    } else if (memcmp(uuid, &gVirtualizerDescriptor.uuid, sizeof(effect_uuid_t)) == 0){
        // Create Virtualizer
        ALOGV("\tEffectCreate - Effect to be created is LVM_VIRTUALIZER");
        pSessionContext->bVirtualizerInstantiated=LVM_TRUE;
        pContext->pBundledContext->SamplesToExitCountVirt = 0;

        pContext->itfe       = &gLvmEffectInterface;
        pContext->EffectType = LVM_VIRTUALIZER;
    } else if (memcmp(uuid, &gEqualizerDescriptor.uuid, sizeof(effect_uuid_t)) == 0){
        // Create Equalizer
        ALOGV("\tEffectCreate - Effect to be created is LVM_EQUALIZER");
        pSessionContext->bEqualizerInstantiated = LVM_TRUE;
        pContext->pBundledContext->SamplesToExitCountEq = 0;

        pContext->itfe       = &gLvmEffectInterface;
        pContext->EffectType = LVM_EQUALIZER;
    } else if (memcmp(uuid, &gVolumeDescriptor.uuid, sizeof(effect_uuid_t)) == 0){
        // Create Volume
        ALOGV("\tEffectCreate - Effect to be created is LVM_VOLUME");
        pSessionContext->bVolumeInstantiated = LVM_TRUE;

        pContext->itfe       = &gLvmEffectInterface;
        pContext->EffectType = LVM_VOLUME;
    }
    else{
        ALOGV("\tLVM_ERROR : EffectCreate() invalid UUID");
        ret = -EINVAL;
        goto exit;
    }

exit:
    if (ret != 0) {
        if (pContext != NULL) {
            if (newBundle) {
                GlobalSessionMemory[sessionNo].bBundledEffectsEnabled = LVM_FALSE;
                SessionIndex[sessionNo] = LVM_UNUSED_SESSION;
                delete pContext->pBundledContext;
            }
            delete pContext;
        }
        if (pHandle != NULL)
          *pHandle = (effect_handle_t)NULL;
    } else {
      if (pHandle != NULL)
        *pHandle = (effect_handle_t)pContext;
    }
    ALOGV("\tEffectCreate end..\n\n");
    return ret;
} /* end EffectCreate */

extern "C" int EffectRelease(effect_handle_t handle){
    ALOGV("\n\tEffectRelease start %p", handle);
    EffectContext * pContext = (EffectContext *)handle;

    ALOGV("\tEffectRelease start handle: %p, context %p", handle, pContext->pBundledContext);
    if (pContext == NULL){
        ALOGV("\tLVM_ERROR : EffectRelease called with NULL pointer");
        return -EINVAL;
    }

    SessionContext *pSessionContext = &GlobalSessionMemory[pContext->pBundledContext->SessionNo];

    // Clear the instantiated flag for the effect
    // protect agains the case where an effect is un-instantiated without being disabled

    int &effectInDrain = pContext->pBundledContext->effectInDrain;
    if(pContext->EffectType == LVM_BASS_BOOST) {
        ALOGV("\tEffectRelease LVM_BASS_BOOST Clearing global intstantiated flag");
        pSessionContext->bBassInstantiated = LVM_FALSE;
        if(pContext->pBundledContext->SamplesToExitCountBb > 0){
            pContext->pBundledContext->NumberEffectsEnabled--;
        }
        pContext->pBundledContext->SamplesToExitCountBb = 0;
    } else if(pContext->EffectType == LVM_VIRTUALIZER) {
        ALOGV("\tEffectRelease LVM_VIRTUALIZER Clearing global intstantiated flag");
        pSessionContext->bVirtualizerInstantiated = LVM_FALSE;
        if(pContext->pBundledContext->SamplesToExitCountVirt > 0){
            pContext->pBundledContext->NumberEffectsEnabled--;
        }
        pContext->pBundledContext->SamplesToExitCountVirt = 0;
    } else if(pContext->EffectType == LVM_EQUALIZER) {
        ALOGV("\tEffectRelease LVM_EQUALIZER Clearing global intstantiated flag");
        pSessionContext->bEqualizerInstantiated =LVM_FALSE;
        if(pContext->pBundledContext->SamplesToExitCountEq > 0){
            pContext->pBundledContext->NumberEffectsEnabled--;
        }
        pContext->pBundledContext->SamplesToExitCountEq = 0;
    } else if(pContext->EffectType == LVM_VOLUME) {
        ALOGV("\tEffectRelease LVM_VOLUME Clearing global intstantiated flag");
        pSessionContext->bVolumeInstantiated = LVM_FALSE;
        // There is no samplesToExitCount for volume so we also use the drain flag to check
        // if we should decrement the effects enabled.
        if (pContext->pBundledContext->bVolumeEnabled == LVM_TRUE
                || (effectInDrain & 1 << LVM_VOLUME) != 0) {
            pContext->pBundledContext->NumberEffectsEnabled--;
        }
    } else {
        ALOGV("\tLVM_ERROR : EffectRelease : Unsupported effect\n\n\n\n\n\n\n");
    }
    effectInDrain &= ~(1 << pContext->EffectType); // no need to drain if released

    // Disable effect, in this case ignore errors (return codes)
    // if an effect has already been disabled
    Effect_setEnabled(pContext, LVM_FALSE);

    // if all effects are no longer instantiaed free the lvm memory and delete BundledEffectContext
    if ((pSessionContext->bBassInstantiated == LVM_FALSE) &&
            (pSessionContext->bVolumeInstantiated == LVM_FALSE) &&
            (pSessionContext->bEqualizerInstantiated ==LVM_FALSE) &&
            (pSessionContext->bVirtualizerInstantiated==LVM_FALSE))
    {
#ifdef LVM_PCM
        if (pContext->pBundledContext->PcmInPtr != NULL) {
            fclose(pContext->pBundledContext->PcmInPtr);
            pContext->pBundledContext->PcmInPtr = NULL;
        }
        if (pContext->pBundledContext->PcmOutPtr != NULL) {
            fclose(pContext->pBundledContext->PcmOutPtr);
            pContext->pBundledContext->PcmOutPtr = NULL;
        }
#endif


        // Clear the SessionIndex
        for(int i=0; i<LVM_MAX_SESSIONS; i++){
            if(SessionIndex[i] == pContext->pBundledContext->SessionId){
                SessionIndex[i] = LVM_UNUSED_SESSION;
                ALOGV("\tEffectRelease: Clearing SessionIndex SessionNo %d for SessionId %d\n",
                        i, pContext->pBundledContext->SessionId);
                break;
            }
        }

        ALOGV("\tEffectRelease: All effects are no longer instantiated\n");
        pSessionContext->bBundledEffectsEnabled = LVM_FALSE;
        pSessionContext->pBundledContext = LVM_NULL;
        ALOGV("\tEffectRelease: Freeing LVM Bundle memory\n");
        LvmEffect_free(pContext);
        ALOGV("\tEffectRelease: Deleting LVM Bundle context %p\n", pContext->pBundledContext);
        if (pContext->pBundledContext->workBuffer != NULL) {
            free(pContext->pBundledContext->workBuffer);
        }
#if defined(BUILD_FLOAT) && !defined(NATIVE_FLOAT_BUFFER)
        free(pContext->pBundledContext->pInputBuffer);
        free(pContext->pBundledContext->pOutputBuffer);
#endif
        delete pContext->pBundledContext;
        pContext->pBundledContext = LVM_NULL;
    }
    // free the effect context for current effect
    delete pContext;

    ALOGV("\tEffectRelease end\n");
    return 0;

} /* end EffectRelease */

extern "C" int EffectGetDescriptor(const effect_uuid_t *uuid,
                                   effect_descriptor_t *pDescriptor) {
    const effect_descriptor_t *desc = NULL;

    if (pDescriptor == NULL || uuid == NULL){
        ALOGV("EffectGetDescriptor() called with NULL pointer");
        return -EINVAL;
    }

    if (memcmp(uuid, &gBassBoostDescriptor.uuid, sizeof(effect_uuid_t)) == 0) {
        desc = &gBassBoostDescriptor;
    } else if (memcmp(uuid, &gVirtualizerDescriptor.uuid, sizeof(effect_uuid_t)) == 0) {
        desc = &gVirtualizerDescriptor;
    } else if (memcmp(uuid, &gEqualizerDescriptor.uuid, sizeof(effect_uuid_t)) == 0) {
        desc = &gEqualizerDescriptor;
    } else if (memcmp(uuid, &gVolumeDescriptor.uuid, sizeof(effect_uuid_t)) == 0) {
        desc = &gVolumeDescriptor;
    }

    if (desc == NULL) {
        return  -EINVAL;
    }

    *pDescriptor = *desc;

    return 0;
} /* end EffectGetDescriptor */

void LvmGlobalBundle_init(){
    ALOGV("\tLvmGlobalBundle_init start");
    for(int i=0; i<LVM_MAX_SESSIONS; i++){
        GlobalSessionMemory[i].bBundledEffectsEnabled   = LVM_FALSE;
        GlobalSessionMemory[i].bVolumeInstantiated      = LVM_FALSE;
        GlobalSessionMemory[i].bEqualizerInstantiated   = LVM_FALSE;
        GlobalSessionMemory[i].bBassInstantiated        = LVM_FALSE;
        GlobalSessionMemory[i].bVirtualizerInstantiated = LVM_FALSE;
        GlobalSessionMemory[i].pBundledContext          = LVM_NULL;

        SessionIndex[i] = LVM_UNUSED_SESSION;
    }
    return;
}
//----------------------------------------------------------------------------
// LvmBundle_init()
//----------------------------------------------------------------------------
// Purpose: Initialize engine with default configuration, creates instance
// with all effects disabled.
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------

int LvmBundle_init(EffectContext *pContext){
    ALOGV("\tLvmBundle_init start");

    pContext->config.inputCfg.accessMode                    = EFFECT_BUFFER_ACCESS_READ;
    pContext->config.inputCfg.channels                      = AUDIO_CHANNEL_OUT_STEREO;
    pContext->config.inputCfg.format                        = EFFECT_BUFFER_FORMAT;
    pContext->config.inputCfg.samplingRate                  = 44100;
    pContext->config.inputCfg.bufferProvider.getBuffer      = NULL;
    pContext->config.inputCfg.bufferProvider.releaseBuffer  = NULL;
    pContext->config.inputCfg.bufferProvider.cookie         = NULL;
    pContext->config.inputCfg.mask                          = EFFECT_CONFIG_ALL;
    pContext->config.outputCfg.accessMode                   = EFFECT_BUFFER_ACCESS_ACCUMULATE;
    pContext->config.outputCfg.channels                     = AUDIO_CHANNEL_OUT_STEREO;
    pContext->config.outputCfg.format                       = EFFECT_BUFFER_FORMAT;
    pContext->config.outputCfg.samplingRate                 = 44100;
    pContext->config.outputCfg.bufferProvider.getBuffer     = NULL;
    pContext->config.outputCfg.bufferProvider.releaseBuffer = NULL;
    pContext->config.outputCfg.bufferProvider.cookie        = NULL;
    pContext->config.outputCfg.mask                         = EFFECT_CONFIG_ALL;

    CHECK_ARG(pContext != NULL);

    if (pContext->pBundledContext->hInstance != NULL){
        ALOGV("\tLvmBundle_init pContext->pBassBoost != NULL "
                "-> Calling pContext->pBassBoost->free()");

        LvmEffect_free(pContext);

        ALOGV("\tLvmBundle_init pContext->pBassBoost != NULL "
                "-> Called pContext->pBassBoost->free()");
    }

    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;          /* Function call status */
    LVM_ControlParams_t     params;                         /* Control Parameters */
    LVM_InstParams_t        InstParams;                     /* Instance parameters */
    LVM_EQNB_BandDef_t      BandDefs[MAX_NUM_BANDS];        /* Equaliser band definitions */
    LVM_HeadroomParams_t    HeadroomParams;                 /* Headroom parameters */
    LVM_HeadroomBandDef_t   HeadroomBandDef[LVM_HEADROOM_MAX_NBANDS];
    LVM_MemTab_t            MemTab;                         /* Memory allocation table */
    bool                    bMallocFailure = LVM_FALSE;

    /* Set the capabilities */
    InstParams.BufferMode       = LVM_UNMANAGED_BUFFERS;
    InstParams.MaxBlockSize     = MAX_CALL_SIZE;
    InstParams.EQNB_NumBands    = MAX_NUM_BANDS;
    InstParams.PSA_Included     = LVM_PSA_ON;

    /* Allocate memory, forcing alignment */
    LvmStatus = LVM_GetMemoryTable(LVM_NULL,
                                  &MemTab,
                                  &InstParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetMemoryTable", "LvmBundle_init")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    ALOGV("\tCreateInstance Succesfully called LVM_GetMemoryTable\n");

    /* Allocate memory */
    for (int i=0; i<LVM_NR_MEMORY_REGIONS; i++){
        if (MemTab.Region[i].Size != 0){
            MemTab.Region[i].pBaseAddress = malloc(MemTab.Region[i].Size);

            if (MemTab.Region[i].pBaseAddress == LVM_NULL){
                ALOGV("\tLVM_ERROR :LvmBundle_init CreateInstance Failed to allocate %" PRIu32
                        " bytes for region %u\n", MemTab.Region[i].Size, i );
                bMallocFailure = LVM_TRUE;
            }else{
                ALOGV("\tLvmBundle_init CreateInstance allocated %" PRIu32
                        " bytes for region %u at %p\n",
                        MemTab.Region[i].Size, i, MemTab.Region[i].pBaseAddress);
            }
        }
    }

    /* If one or more of the memory regions failed to allocate, free the regions that were
     * succesfully allocated and return with an error
     */
    if(bMallocFailure == LVM_TRUE){
        for (int i=0; i<LVM_NR_MEMORY_REGIONS; i++){
            if (MemTab.Region[i].pBaseAddress == LVM_NULL){
                ALOGV("\tLVM_ERROR :LvmBundle_init CreateInstance Failed to allocate %" PRIu32
                        " bytes for region %u Not freeing\n", MemTab.Region[i].Size, i );
            }else{
                ALOGV("\tLVM_ERROR :LvmBundle_init CreateInstance Failed: but allocated %" PRIu32
                     " bytes for region %u at %p- free\n",
                     MemTab.Region[i].Size, i, MemTab.Region[i].pBaseAddress);
                free(MemTab.Region[i].pBaseAddress);
            }
        }
        return -EINVAL;
    }
    ALOGV("\tLvmBundle_init CreateInstance Succesfully malloc'd memory\n");

    /* Initialise */
    pContext->pBundledContext->hInstance = LVM_NULL;

    /* Init sets the instance handle */
    LvmStatus = LVM_GetInstanceHandle(&pContext->pBundledContext->hInstance,
                                      &MemTab,
                                      &InstParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetInstanceHandle", "LvmBundle_init")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    ALOGV("\tLvmBundle_init CreateInstance Succesfully called LVM_GetInstanceHandle\n");

    /* Set the initial process parameters */
    /* General parameters */
    params.OperatingMode          = LVM_MODE_ON;
    params.SampleRate             = LVM_FS_44100;
    params.SourceFormat           = LVM_STEREO;
    params.SpeakerType            = LVM_HEADPHONES;

    pContext->pBundledContext->SampleRate = LVM_FS_44100;
#ifdef SUPPORT_MC
    pContext->pBundledContext->ChMask = AUDIO_CHANNEL_OUT_STEREO;
#endif

    /* Concert Sound parameters */
    params.VirtualizerOperatingMode   = LVM_MODE_OFF;
    params.VirtualizerType            = LVM_CONCERTSOUND;
    params.VirtualizerReverbLevel     = 100;
    params.CS_EffectLevel             = LVM_CS_EFFECT_NONE;

    /* N-Band Equaliser parameters */
    params.EQNB_OperatingMode     = LVM_EQNB_OFF;
    params.EQNB_NBands            = FIVEBAND_NUMBANDS;
    params.pEQNB_BandDefinition   = &BandDefs[0];

    for (int i=0; i<FIVEBAND_NUMBANDS; i++)
    {
        BandDefs[i].Frequency = EQNB_5BandPresetsFrequencies[i];
        BandDefs[i].QFactor   = EQNB_5BandPresetsQFactors[i];
        BandDefs[i].Gain      = EQNB_5BandSoftPresets[i];
    }

    /* Volume Control parameters */
    params.VC_EffectLevel         = 0;
    params.VC_Balance             = 0;

    /* Treble Enhancement parameters */
    params.TE_OperatingMode       = LVM_TE_OFF;
    params.TE_EffectLevel         = 0;

    /* PSA Control parameters */
    params.PSA_Enable             = LVM_PSA_OFF;
    params.PSA_PeakDecayRate      = (LVM_PSA_DecaySpeed_en)0;

    /* Bass Enhancement parameters */
    params.BE_OperatingMode       = LVM_BE_OFF;
    params.BE_EffectLevel         = 0;
    params.BE_CentreFreq          = LVM_BE_CENTRE_90Hz;
    params.BE_HPF                 = LVM_BE_HPF_ON;

    /* PSA Control parameters */
    params.PSA_Enable             = LVM_PSA_OFF;
    params.PSA_PeakDecayRate      = LVM_PSA_SPEED_MEDIUM;

    /* TE Control parameters */
    params.TE_OperatingMode       = LVM_TE_OFF;
    params.TE_EffectLevel         = 0;

#ifdef SUPPORT_MC
    params.NrChannels             =
        audio_channel_count_from_out_mask(AUDIO_CHANNEL_OUT_STEREO);
    params.ChMask                 = AUDIO_CHANNEL_OUT_STEREO;
#endif
    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance,
                                         &params);

    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "LvmBundle_init")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    ALOGV("\tLvmBundle_init CreateInstance Succesfully called LVM_SetControlParameters\n");

    /* Set the headroom parameters */
    HeadroomBandDef[0].Limit_Low          = 20;
    HeadroomBandDef[0].Limit_High         = 4999;
    HeadroomBandDef[0].Headroom_Offset    = 0;
    HeadroomBandDef[1].Limit_Low          = 5000;
    HeadroomBandDef[1].Limit_High         = 24000;
    HeadroomBandDef[1].Headroom_Offset    = 0;
    HeadroomParams.pHeadroomDefinition    = &HeadroomBandDef[0];
    HeadroomParams.Headroom_OperatingMode = LVM_HEADROOM_ON;
    HeadroomParams.NHeadroomBands         = 2;

    LvmStatus = LVM_SetHeadroomParams(pContext->pBundledContext->hInstance,
                                      &HeadroomParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_SetHeadroomParams", "LvmBundle_init")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    ALOGV("\tLvmBundle_init CreateInstance Succesfully called LVM_SetHeadroomParams\n");
    ALOGV("\tLvmBundle_init End");
    return 0;
}   /* end LvmBundle_init */

//----------------------------------------------------------------------------
// LvmBundle_process()
//----------------------------------------------------------------------------
// Purpose:
// Apply LVM Bundle effects
//
// Inputs:
//  pIn:        pointer to stereo float or 16 bit input data
//  pOut:       pointer to stereo float or 16 bit output data
//  frameCount: Frames to process
//  pContext:   effect engine context
//  strength    strength to be applied
//
//  Outputs:
//  pOut:       pointer to updated stereo 16 bit output data
//
//----------------------------------------------------------------------------
#ifdef BUILD_FLOAT
int LvmBundle_process(effect_buffer_t  *pIn,
                      effect_buffer_t  *pOut,
                      int              frameCount,
                      EffectContext    *pContext){

    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    effect_buffer_t         *pOutTmp;
    const LVM_INT32 NrChannels =
        audio_channel_count_from_out_mask(pContext->config.inputCfg.channels);
#ifndef NATIVE_FLOAT_BUFFER
    if (pContext->pBundledContext->pInputBuffer == nullptr ||
            pContext->pBundledContext->frameCount < frameCount) {
        free(pContext->pBundledContext->pInputBuffer);
        pContext->pBundledContext->pInputBuffer =
                (LVM_FLOAT *)calloc(frameCount, sizeof(LVM_FLOAT) * NrChannels);
    }

    if (pContext->pBundledContext->pOutputBuffer == nullptr ||
            pContext->pBundledContext->frameCount < frameCount) {
        free(pContext->pBundledContext->pOutputBuffer);
        pContext->pBundledContext->pOutputBuffer =
                (LVM_FLOAT *)calloc(frameCount, sizeof(LVM_FLOAT) * NrChannels);
    }

    if (pContext->pBundledContext->pInputBuffer == nullptr ||
            pContext->pBundledContext->pOutputBuffer == nullptr) {
        ALOGE("LVM_ERROR : LvmBundle_process memory allocation for float buffer's failed");
        return -EINVAL;
    }

    LVM_FLOAT * const pInputBuff = pContext->pBundledContext->pInputBuffer;
    LVM_FLOAT * const pOutputBuff = pContext->pBundledContext->pOutputBuffer;
#endif

    if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_WRITE){
        pOutTmp = pOut;
    } else if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE){
        if (pContext->pBundledContext->frameCount != frameCount) {
            if (pContext->pBundledContext->workBuffer != NULL) {
                free(pContext->pBundledContext->workBuffer);
            }
            pContext->pBundledContext->workBuffer =
                    (effect_buffer_t *)calloc(frameCount, sizeof(effect_buffer_t) * NrChannels);
            if (pContext->pBundledContext->workBuffer == NULL) {
                return -ENOMEM;
            }
            pContext->pBundledContext->frameCount = frameCount;
        }
        pOutTmp = pContext->pBundledContext->workBuffer;
    } else {
        ALOGV("LVM_ERROR : LvmBundle_process invalid access mode");
        return -EINVAL;
    }

#ifdef LVM_PCM
    fwrite(pIn,
           frameCount * sizeof(effect_buffer_t) * NrChannels,
           1,
           pContext->pBundledContext->PcmInPtr);
    fflush(pContext->pBundledContext->PcmInPtr);
#endif

#ifndef NATIVE_FLOAT_BUFFER
    /* Converting input data from fixed point to float point */
    memcpy_to_float_from_i16(pInputBuff, pIn, frameCount * NrChannels);

    /* Process the samples */
    LvmStatus = LVM_Process(pContext->pBundledContext->hInstance, /* Instance handle */
                            pInputBuff,                           /* Input buffer */
                            pOutputBuff,                          /* Output buffer */
                            (LVM_UINT16)frameCount,               /* Number of samples to read */
                            0);                                   /* Audio Time */

    /* Converting output data from float point to fixed point */
    memcpy_to_i16_from_float(pOutTmp, pOutputBuff, frameCount * NrChannels);

#else
    /* Process the samples */
    LvmStatus = LVM_Process(pContext->pBundledContext->hInstance, /* Instance handle */
                            pIn,                                  /* Input buffer */
                            pOutTmp,                              /* Output buffer */
                            (LVM_UINT16)frameCount,               /* Number of samples to read */
                            0);                                   /* Audio Time */
#endif
    LVM_ERROR_CHECK(LvmStatus, "LVM_Process", "LvmBundle_process")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

#ifdef LVM_PCM
    fwrite(pOutTmp,
           frameCount * sizeof(effect_buffer_t) * NrChannels,
           1,
           pContext->pBundledContext->PcmOutPtr);
    fflush(pContext->pBundledContext->PcmOutPtr);
#endif

    if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE){
        for (int i = 0; i < frameCount * NrChannels; i++) {
#ifndef NATIVE_FLOAT_BUFFER
            pOut[i] = clamp16((LVM_INT32)pOut[i] + (LVM_INT32)pOutTmp[i]);
#else
            pOut[i] = pOut[i] + pOutTmp[i];
#endif
        }
    }
    return 0;
}    /* end LvmBundle_process */

#else // BUILD_FLOAT

int LvmBundle_process(LVM_INT16        *pIn,
                      LVM_INT16        *pOut,
                      int              frameCount,
                      EffectContext    *pContext) {

    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    LVM_INT16               *pOutTmp;

    if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_WRITE){
        pOutTmp = pOut;
    } else if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE){
        if (pContext->pBundledContext->frameCount != frameCount) {
            if (pContext->pBundledContext->workBuffer != NULL) {
                free(pContext->pBundledContext->workBuffer);
            }
            pContext->pBundledContext->workBuffer =
                    (effect_buffer_t *)calloc(frameCount, sizeof(effect_buffer_t) * FCC_2);
            if (pContext->pBundledContext->workBuffer == NULL) {
                return -ENOMEM;
            }
            pContext->pBundledContext->frameCount = frameCount;
        }
        pOutTmp = pContext->pBundledContext->workBuffer;
    } else {
        ALOGV("LVM_ERROR : LvmBundle_process invalid access mode");
        return -EINVAL;
    }

#ifdef LVM_PCM
    fwrite(pIn, frameCount * sizeof(*pIn) * FCC_2,
            1 /* nmemb */, pContext->pBundledContext->PcmInPtr);
    fflush(pContext->pBundledContext->PcmInPtr);
#endif

    //ALOGV("Calling LVM_Process");

    /* Process the samples */
    LvmStatus = LVM_Process(pContext->pBundledContext->hInstance, /* Instance handle */
                            pIn,                                  /* Input buffer */
                            pOutTmp,                              /* Output buffer */
                            (LVM_UINT16)frameCount,               /* Number of samples to read */
                            0);                                   /* Audio Time */

    LVM_ERROR_CHECK(LvmStatus, "LVM_Process", "LvmBundle_process")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

#ifdef LVM_PCM
    fwrite(pOutTmp, frameCount * sizeof(*pOutTmp) * FCC_2,
            1 /* nmemb */, pContext->pBundledContext->PcmOutPtr);
    fflush(pContext->pBundledContext->PcmOutPtr);
#endif

    if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE){
        for (int i=0; i<frameCount*2; i++){
            pOut[i] = clamp16((LVM_INT32)pOut[i] + (LVM_INT32)pOutTmp[i]);
        }
    }
    return 0;
}    /* end LvmBundle_process */

#endif // BUILD_FLOAT

//----------------------------------------------------------------------------
// EqualizerUpdateActiveParams()
//----------------------------------------------------------------------------
// Purpose: Update ActiveParams for Equalizer
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------
void EqualizerUpdateActiveParams(EffectContext *pContext) {
    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "EqualizerUpdateActiveParams")
    //ALOGV("\tEqualizerUpdateActiveParams Succesfully returned from LVM_GetControlParameters\n");
    //ALOGV("\tEqualizerUpdateActiveParams just Got -> %d\n",
    //          ActiveParams.pEQNB_BandDefinition[band].Gain);


    for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
           ActiveParams.pEQNB_BandDefinition[i].Frequency = EQNB_5BandPresetsFrequencies[i];
           ActiveParams.pEQNB_BandDefinition[i].QFactor   = EQNB_5BandPresetsQFactors[i];
           ActiveParams.pEQNB_BandDefinition[i].Gain = pContext->pBundledContext->bandGaindB[i];
       }

    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "EqualizerUpdateActiveParams")
    //ALOGV("\tEqualizerUpdateActiveParams just Set -> %d\n",
    //          ActiveParams.pEQNB_BandDefinition[band].Gain);

}

//----------------------------------------------------------------------------
// LvmEffect_limitLevel()
//----------------------------------------------------------------------------
// Purpose: limit the overall level to a value less than 0 dB preserving
//          the overall EQ band gain and BassBoost relative levels.
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------
void LvmEffect_limitLevel(EffectContext *pContext) {
    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "LvmEffect_limitLevel")
    //ALOGV("\tLvmEffect_limitLevel Succesfully returned from LVM_GetControlParameters\n");
    //ALOGV("\tLvmEffect_limitLevel just Got -> %d\n",
    //          ActiveParams.pEQNB_BandDefinition[band].Gain);

    int gainCorrection = 0;
    //Count the energy contribution per band for EQ and BassBoost only if they are active.
    float energyContribution = 0;
    float energyCross = 0;
    float energyBassBoost = 0;
    float crossCorrection = 0;

    bool eqEnabled = pContext->pBundledContext->bEqualizerEnabled == LVM_TRUE;
    bool bbEnabled = pContext->pBundledContext->bBassEnabled == LVM_TRUE;
    bool viEnabled = pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE;

    //EQ contribution
    if (eqEnabled) {
        for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
            float bandFactor = pContext->pBundledContext->bandGaindB[i]/15.0;
            float bandCoefficient = LimitLevel_bandEnergyCoefficient[i];
            float bandEnergy = bandFactor * bandCoefficient * bandCoefficient;
            if (bandEnergy > 0)
                energyContribution += bandEnergy;
        }

        //cross EQ coefficients
        float bandFactorSum = 0;
        for (int i = 0; i < FIVEBAND_NUMBANDS-1; i++) {
            float bandFactor1 = pContext->pBundledContext->bandGaindB[i]/15.0;
            float bandFactor2 = pContext->pBundledContext->bandGaindB[i+1]/15.0;

            if (bandFactor1 > 0 && bandFactor2 > 0) {
                float crossEnergy = bandFactor1 * bandFactor2 *
                        LimitLevel_bandEnergyCrossCoefficient[i];
                bandFactorSum += bandFactor1 * bandFactor2;

                if (crossEnergy > 0)
                    energyCross += crossEnergy;
            }
        }
        bandFactorSum -= 1.0;
        if (bandFactorSum > 0)
          crossCorrection = bandFactorSum * 0.7;
    }

    //BassBoost contribution
    if (bbEnabled) {
        float boostFactor = (pContext->pBundledContext->BassStrengthSaved)/1000.0;
        float boostCoefficient = LimitLevel_bassBoostEnergyCoefficient;

        energyContribution += boostFactor * boostCoefficient * boostCoefficient;

        if (eqEnabled) {
            for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
                float bandFactor = pContext->pBundledContext->bandGaindB[i]/15.0;
                float bandCrossCoefficient = LimitLevel_bassBoostEnergyCrossCoefficient[i];
                float bandEnergy = boostFactor * bandFactor *
                    bandCrossCoefficient;
                if (bandEnergy > 0)
                  energyBassBoost += bandEnergy;
            }
        }
    }

    //Virtualizer contribution
    if (viEnabled) {
        energyContribution += LimitLevel_virtualizerContribution *
                LimitLevel_virtualizerContribution;
    }

    double totalEnergyEstimation = sqrt(energyContribution + energyCross + energyBassBoost) -
            crossCorrection;
    ALOGV(" TOTAL energy estimation: %0.2f dB", totalEnergyEstimation);

    //roundoff
    int maxLevelRound = (int)(totalEnergyEstimation + 0.99);
    if (maxLevelRound + pContext->pBundledContext->volume > 0) {
        gainCorrection = maxLevelRound + pContext->pBundledContext->volume;
    }

    ActiveParams.VC_EffectLevel  = pContext->pBundledContext->volume - gainCorrection;
    if (ActiveParams.VC_EffectLevel < -96) {
        ActiveParams.VC_EffectLevel = -96;
    }
    ALOGV("\tVol:%d, GainCorrection: %d, Actual vol: %d", pContext->pBundledContext->volume,
            gainCorrection, ActiveParams.VC_EffectLevel);

    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "LvmEffect_limitLevel")

    ALOGV("LVM_SetControlParameters return:%d", (int)LvmStatus);
    //ALOGV("\tLvmEffect_limitLevel just Set -> %d\n",
    //          ActiveParams.pEQNB_BandDefinition[band].Gain);

    //ALOGV("\tLvmEffect_limitLevel just set (-96dB -> 0dB) -> %d\n",ActiveParams.VC_EffectLevel );
    if (pContext->pBundledContext->firstVolume == LVM_TRUE){
        LvmStatus = LVM_SetVolumeNoSmoothing(pContext->pBundledContext->hInstance, &ActiveParams);
        LVM_ERROR_CHECK(LvmStatus, "LVM_SetVolumeNoSmoothing", "LvmBundle_process")
        ALOGV("\tLVM_VOLUME: Disabling Smoothing for first volume change to remove spikes/clicks");
        pContext->pBundledContext->firstVolume = LVM_FALSE;
    }
}

//----------------------------------------------------------------------------
// LvmEffect_enable()
//----------------------------------------------------------------------------
// Purpose: Enable the effect in the bundle
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------

int LvmEffect_enable(EffectContext *pContext){
    //ALOGV("\tLvmEffect_enable start");

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "LvmEffect_enable")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;
    //ALOGV("\tLvmEffect_enable Succesfully called LVM_GetControlParameters\n");

    if(pContext->EffectType == LVM_BASS_BOOST) {
        ALOGV("\tLvmEffect_enable : Enabling LVM_BASS_BOOST");
        ActiveParams.BE_OperatingMode       = LVM_BE_ON;
    }
    if(pContext->EffectType == LVM_VIRTUALIZER) {
        ALOGV("\tLvmEffect_enable : Enabling LVM_VIRTUALIZER");
        ActiveParams.VirtualizerOperatingMode   = LVM_MODE_ON;
    }
    if(pContext->EffectType == LVM_EQUALIZER) {
        ALOGV("\tLvmEffect_enable : Enabling LVM_EQUALIZER");
        ActiveParams.EQNB_OperatingMode     = LVM_EQNB_ON;
    }
    if(pContext->EffectType == LVM_VOLUME) {
        ALOGV("\tLvmEffect_enable : Enabling LVM_VOLUME");
    }

    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "LvmEffect_enable")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tLvmEffect_enable Succesfully called LVM_SetControlParameters\n");
    //ALOGV("\tLvmEffect_enable end");
    LvmEffect_limitLevel(pContext);
    return 0;
}

//----------------------------------------------------------------------------
// LvmEffect_disable()
//----------------------------------------------------------------------------
// Purpose: Disable the effect in the bundle
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------

int LvmEffect_disable(EffectContext *pContext){
    //ALOGV("\tLvmEffect_disable start");

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "LvmEffect_disable")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;
    //ALOGV("\tLvmEffect_disable Succesfully called LVM_GetControlParameters\n");

    if(pContext->EffectType == LVM_BASS_BOOST) {
        ALOGV("\tLvmEffect_disable : Disabling LVM_BASS_BOOST");
        ActiveParams.BE_OperatingMode       = LVM_BE_OFF;
    }
    if(pContext->EffectType == LVM_VIRTUALIZER) {
        ALOGV("\tLvmEffect_disable : Disabling LVM_VIRTUALIZER");
        ActiveParams.VirtualizerOperatingMode   = LVM_MODE_OFF;
    }
    if(pContext->EffectType == LVM_EQUALIZER) {
        ALOGV("\tLvmEffect_disable : Disabling LVM_EQUALIZER");
        ActiveParams.EQNB_OperatingMode     = LVM_EQNB_OFF;
    }
    if(pContext->EffectType == LVM_VOLUME) {
        ALOGV("\tLvmEffect_disable : Disabling LVM_VOLUME");
    }

    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "LvmEffect_disable")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tLvmEffect_disable Succesfully called LVM_SetControlParameters\n");
    //ALOGV("\tLvmEffect_disable end");
    LvmEffect_limitLevel(pContext);
    return 0;
}

//----------------------------------------------------------------------------
// LvmEffect_free()
//----------------------------------------------------------------------------
// Purpose: Free all memory associated with the Bundle.
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------

void LvmEffect_free(EffectContext *pContext){
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;         /* Function call status */
    LVM_MemTab_t            MemTab;

    /* Free the algorithm memory */
    LvmStatus = LVM_GetMemoryTable(pContext->pBundledContext->hInstance,
                                   &MemTab,
                                   LVM_NULL);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetMemoryTable", "LvmEffect_free")

    for (int i=0; i<LVM_NR_MEMORY_REGIONS; i++){
        if (MemTab.Region[i].Size != 0){
            if (MemTab.Region[i].pBaseAddress != NULL){
                free(MemTab.Region[i].pBaseAddress);
            }else{
                ALOGV("\tLVM_ERROR : LvmEffect_free - trying to free with NULL pointer %" PRIu32
                        " bytes for region %u at %p ERROR\n",
                        MemTab.Region[i].Size, i, MemTab.Region[i].pBaseAddress);
            }
        }
    }
}    /* end LvmEffect_free */

//----------------------------------------------------------------------------
// Effect_setConfig()
//----------------------------------------------------------------------------
// Purpose: Set input and output audio configuration.
//
// Inputs:
//  pContext:   effect engine context
//  pConfig:    pointer to effect_config_t structure holding input and output
//      configuration parameters
//
// Outputs:
//
//----------------------------------------------------------------------------

int Effect_setConfig(EffectContext *pContext, effect_config_t *pConfig){
    LVM_Fs_en   SampleRate;
    //ALOGV("\tEffect_setConfig start");

    CHECK_ARG(pContext != NULL);
    CHECK_ARG(pConfig != NULL);

    CHECK_ARG(pConfig->inputCfg.samplingRate == pConfig->outputCfg.samplingRate);
    CHECK_ARG(pConfig->inputCfg.channels == pConfig->outputCfg.channels);
    CHECK_ARG(pConfig->inputCfg.format == pConfig->outputCfg.format);
#ifdef SUPPORT_MC
    CHECK_ARG(audio_channel_count_from_out_mask(pConfig->inputCfg.channels) <= LVM_MAX_CHANNELS);
#else
    CHECK_ARG(pConfig->inputCfg.channels == AUDIO_CHANNEL_OUT_STEREO);
#endif
    CHECK_ARG(pConfig->outputCfg.accessMode == EFFECT_BUFFER_ACCESS_WRITE
              || pConfig->outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE);
    CHECK_ARG(pConfig->inputCfg.format == EFFECT_BUFFER_FORMAT);
    pContext->config = *pConfig;
    const LVM_INT16 NrChannels = audio_channel_count_from_out_mask(pConfig->inputCfg.channels);

    switch (pConfig->inputCfg.samplingRate) {
    case 8000:
        SampleRate = LVM_FS_8000;
        pContext->pBundledContext->SamplesPerSecond = 8000 * NrChannels;
        break;
    case 16000:
        SampleRate = LVM_FS_16000;
        pContext->pBundledContext->SamplesPerSecond = 16000 * NrChannels;
        break;
    case 22050:
        SampleRate = LVM_FS_22050;
        pContext->pBundledContext->SamplesPerSecond = 22050 * NrChannels;
        break;
    case 32000:
        SampleRate = LVM_FS_32000;
        pContext->pBundledContext->SamplesPerSecond = 32000 * NrChannels;
        break;
    case 44100:
        SampleRate = LVM_FS_44100;
        pContext->pBundledContext->SamplesPerSecond = 44100 * NrChannels;
        break;
    case 48000:
        SampleRate = LVM_FS_48000;
        pContext->pBundledContext->SamplesPerSecond = 48000 * NrChannels;
        break;
#if defined(BUILD_FLOAT) && defined(HIGHER_FS)
    case 88200:
        SampleRate = LVM_FS_88200;
        pContext->pBundledContext->SamplesPerSecond = 88200 * NrChannels;
        break;
    case 96000:
        SampleRate = LVM_FS_96000;
        pContext->pBundledContext->SamplesPerSecond = 96000 * NrChannels;
        break;
    case 176400:
        SampleRate = LVM_FS_176400;
        pContext->pBundledContext->SamplesPerSecond = 176400 * NrChannels;
        break;
    case 192000:
        SampleRate = LVM_FS_192000;
        pContext->pBundledContext->SamplesPerSecond = 192000 * NrChannels;
        break;
#endif
    default:
        ALOGV("\tEffect_setConfig invalid sampling rate %d", pConfig->inputCfg.samplingRate);
        return -EINVAL;
    }

#ifdef SUPPORT_MC
    if (pContext->pBundledContext->SampleRate != SampleRate ||
        pContext->pBundledContext->ChMask != pConfig->inputCfg.channels) {
#else
    if(pContext->pBundledContext->SampleRate != SampleRate){
#endif

        LVM_ControlParams_t     ActiveParams;
        LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;

        ALOGV("\tEffect_setConfig change sampling rate to %d", SampleRate);

        /* Get the current settings */
        LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

        LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "Effect_setConfig")
        if(LvmStatus != LVM_SUCCESS) return -EINVAL;

        ActiveParams.SampleRate = SampleRate;

#ifdef SUPPORT_MC
        ActiveParams.NrChannels = NrChannels;
        ActiveParams.ChMask = pConfig->inputCfg.channels;
#endif

        LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);

        LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "Effect_setConfig")
        ALOGV("\tEffect_setConfig Succesfully called LVM_SetControlParameters\n");
        pContext->pBundledContext->SampleRate = SampleRate;
#ifdef SUPPORT_MC
        pContext->pBundledContext->ChMask = pConfig->inputCfg.channels;
#endif

        LvmEffect_limitLevel(pContext);

    }else{
        //ALOGV("\tEffect_setConfig keep sampling rate at %d", SampleRate);
    }

    //ALOGV("\tEffect_setConfig End....");
    return 0;
}   /* end Effect_setConfig */

//----------------------------------------------------------------------------
// Effect_getConfig()
//----------------------------------------------------------------------------
// Purpose: Get input and output audio configuration.
//
// Inputs:
//  pContext:   effect engine context
//  pConfig:    pointer to effect_config_t structure holding input and output
//      configuration parameters
//
// Outputs:
//
//----------------------------------------------------------------------------

void Effect_getConfig(EffectContext *pContext, effect_config_t *pConfig)
{
    *pConfig = pContext->config;
}   /* end Effect_getConfig */

//----------------------------------------------------------------------------
// BassGetStrength()
//----------------------------------------------------------------------------
// Purpose:
// get the effect strength currently being used, what is actually returned is the strengh that was
// previously used in the set, this is because the app uses a strength in the range 0-1000 while
// the bassboost uses 1-15, so to avoid a quantisation the original set value is used. However the
// actual used value is checked to make sure it corresponds to the one being returned
//
// Inputs:
//  pContext:   effect engine context
//
//----------------------------------------------------------------------------

uint32_t BassGetStrength(EffectContext *pContext){
    //ALOGV("\tBassGetStrength() (0-1000) -> %d\n", pContext->pBundledContext->BassStrengthSaved);

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "BassGetStrength")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tBassGetStrength Succesfully returned from LVM_GetControlParameters\n");

    /* Check that the strength returned matches the strength that was set earlier */
    if(ActiveParams.BE_EffectLevel !=
       (LVM_INT16)((15*pContext->pBundledContext->BassStrengthSaved)/1000)){
        ALOGV("\tLVM_ERROR : BassGetStrength module strength does not match savedStrength %d %d\n",
                ActiveParams.BE_EffectLevel, pContext->pBundledContext->BassStrengthSaved);
        return -EINVAL;
    }

    //ALOGV("\tBassGetStrength() (0-15)   -> %d\n", ActiveParams.BE_EffectLevel );
    //ALOGV("\tBassGetStrength() (saved)  -> %d\n", pContext->pBundledContext->BassStrengthSaved );
    return pContext->pBundledContext->BassStrengthSaved;
}    /* end BassGetStrength */

//----------------------------------------------------------------------------
// BassSetStrength()
//----------------------------------------------------------------------------
// Purpose:
// Apply the strength to the BassBosst. Must first be converted from the range 0-1000 to 1-15
//
// Inputs:
//  pContext:   effect engine context
//  strength    strength to be applied
//
//----------------------------------------------------------------------------

void BassSetStrength(EffectContext *pContext, uint32_t strength){
    //ALOGV("\tBassSetStrength(%d)", strength);

    pContext->pBundledContext->BassStrengthSaved = (int)strength;

    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "BassSetStrength")
    //ALOGV("\tBassSetStrength Succesfully returned from LVM_GetControlParameters\n");

    /* Bass Enhancement parameters */
    ActiveParams.BE_EffectLevel    = (LVM_INT16)((15*strength)/1000);
    ActiveParams.BE_CentreFreq     = LVM_BE_CENTRE_90Hz;

    //ALOGV("\tBassSetStrength() (0-15)   -> %d\n", ActiveParams.BE_EffectLevel );

    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "BassSetStrength")
    //ALOGV("\tBassSetStrength Succesfully called LVM_SetControlParameters\n");

    LvmEffect_limitLevel(pContext);
}    /* end BassSetStrength */

//----------------------------------------------------------------------------
// VirtualizerGetStrength()
//----------------------------------------------------------------------------
// Purpose:
// get the effect strength currently being used, what is actually returned is the strengh that was
// previously used in the set, this is because the app uses a strength in the range 0-1000 while
// the Virtualizer uses 1-100, so to avoid a quantisation the original set value is used.However the
// actual used value is checked to make sure it corresponds to the one being returned
//
// Inputs:
//  pContext:   effect engine context
//
//----------------------------------------------------------------------------

uint32_t VirtualizerGetStrength(EffectContext *pContext){
    //ALOGV("\tVirtualizerGetStrength (0-1000) -> %d\n",pContext->pBundledContext->VirtStrengthSaved);

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */

    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VirtualizerGetStrength")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tVirtualizerGetStrength Succesfully returned from LVM_GetControlParameters\n");
    //ALOGV("\tVirtualizerGetStrength() (0-100)   -> %d\n", ActiveParams.VirtualizerReverbLevel*10);
    return pContext->pBundledContext->VirtStrengthSaved;
}    /* end getStrength */

//----------------------------------------------------------------------------
// VirtualizerSetStrength()
//----------------------------------------------------------------------------
// Purpose:
// Apply the strength to the Virtualizer. Must first be converted from the range 0-1000 to 1-15
//
// Inputs:
//  pContext:   effect engine context
//  strength    strength to be applied
//
//----------------------------------------------------------------------------

void VirtualizerSetStrength(EffectContext *pContext, uint32_t strength){
    //ALOGV("\tVirtualizerSetStrength(%d)", strength);
    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */

    pContext->pBundledContext->VirtStrengthSaved = (int)strength;

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,&ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VirtualizerSetStrength")
    //ALOGV("\tVirtualizerSetStrength Succesfully returned from LVM_GetControlParameters\n");

    /* Virtualizer parameters */
    ActiveParams.CS_EffectLevel             = (int)((strength*32767)/1000);

    ALOGV("\tVirtualizerSetStrength() (0-1000)   -> %d\n", strength );
    ALOGV("\tVirtualizerSetStrength() (0- 100)   -> %d\n", ActiveParams.CS_EffectLevel );

    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "VirtualizerSetStrength")
    //ALOGV("\tVirtualizerSetStrength Succesfully called LVM_SetControlParameters\n\n");
    LvmEffect_limitLevel(pContext);
}    /* end setStrength */

//----------------------------------------------------------------------------
// VirtualizerIsDeviceSupported()
//----------------------------------------------------------------------------
// Purpose:
// Check if an audio device type is supported by this implementation
//
// Inputs:
//  deviceType   the type of device that affects the processing (e.g. for binaural vs transaural)
// Output:
//  -EINVAL      if the configuration is not supported or it is unknown
//  0            if the configuration is supported
//----------------------------------------------------------------------------
int VirtualizerIsDeviceSupported(audio_devices_t deviceType) {
    switch (deviceType) {
    case AUDIO_DEVICE_OUT_WIRED_HEADSET:
    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
    case AUDIO_DEVICE_OUT_USB_HEADSET:
    // case AUDIO_DEVICE_OUT_USB_DEVICE:  // For USB testing of the virtualizer only.
        return 0;
    default :
        return -EINVAL;
    }
}

//----------------------------------------------------------------------------
// VirtualizerIsConfigurationSupported()
//----------------------------------------------------------------------------
// Purpose:
// Check if a channel mask + audio device type is supported by this implementation
//
// Inputs:
//  channelMask  the channel mask of the input to virtualize
//  deviceType   the type of device that affects the processing (e.g. for binaural vs transaural)
// Output:
//  -EINVAL      if the configuration is not supported or it is unknown
//  0            if the configuration is supported
//----------------------------------------------------------------------------
int VirtualizerIsConfigurationSupported(audio_channel_mask_t channelMask,
        audio_devices_t deviceType) {
    uint32_t channelCount = audio_channel_count_from_out_mask(channelMask);
    if (channelCount < 1 || channelCount > FCC_2) { // TODO: update to 8 channels when supported.
        return -EINVAL;
    }
    return VirtualizerIsDeviceSupported(deviceType);
}

//----------------------------------------------------------------------------
// VirtualizerForceVirtualizationMode()
//----------------------------------------------------------------------------
// Purpose:
// Force the virtualization mode to that of the given audio device
//
// Inputs:
//  pContext     effect engine context
//  forcedDevice the type of device whose virtualization mode we'll always use
// Output:
//  -EINVAL      if the device is not supported or is unknown
//  0            if the device is supported and the virtualization mode forced
//
//----------------------------------------------------------------------------
int VirtualizerForceVirtualizationMode(EffectContext *pContext, audio_devices_t forcedDevice) {
    ALOGV("VirtualizerForceVirtualizationMode: forcedDev=0x%x enabled=%d tmpDisabled=%d",
            forcedDevice, pContext->pBundledContext->bVirtualizerEnabled,
            pContext->pBundledContext->bVirtualizerTempDisabled);
    int status = 0;
    bool useVirtualizer = false;

    if (VirtualizerIsDeviceSupported(forcedDevice) != 0) {
        if (forcedDevice != AUDIO_DEVICE_NONE) {
            //forced device is not supported, make it behave as a reset of forced mode
            forcedDevice = AUDIO_DEVICE_NONE;
            // but return an error
            status = -EINVAL;
        }
    }

    if (forcedDevice == AUDIO_DEVICE_NONE) {
        // disabling forced virtualization mode:
        // verify whether the virtualization should be enabled or disabled
        if (VirtualizerIsDeviceSupported(pContext->pBundledContext->nOutputDevice) == 0) {
            useVirtualizer = (pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE);
        }
        pContext->pBundledContext->nVirtualizerForcedDevice = AUDIO_DEVICE_NONE;
    } else {
        // forcing virtualization mode: here we already know the device is supported
        pContext->pBundledContext->nVirtualizerForcedDevice = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
        // only enable for a supported mode, when the effect is enabled
        useVirtualizer = (pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE);
    }

    if (useVirtualizer) {
        if (pContext->pBundledContext->bVirtualizerTempDisabled == LVM_TRUE) {
            ALOGV("\tVirtualizerForceVirtualizationMode re-enable LVM_VIRTUALIZER");
            android::LvmEffect_enable(pContext);
            pContext->pBundledContext->bVirtualizerTempDisabled = LVM_FALSE;
        } else {
            ALOGV("\tVirtualizerForceVirtualizationMode leaving LVM_VIRTUALIZER enabled");
        }
    } else {
        if (pContext->pBundledContext->bVirtualizerTempDisabled == LVM_FALSE) {
            ALOGV("\tVirtualizerForceVirtualizationMode disable LVM_VIRTUALIZER");
            android::LvmEffect_disable(pContext);
            pContext->pBundledContext->bVirtualizerTempDisabled = LVM_TRUE;
        } else {
            ALOGV("\tVirtualizerForceVirtualizationMode leaving LVM_VIRTUALIZER disabled");
        }
    }

    ALOGV("\tafter VirtualizerForceVirtualizationMode: enabled=%d tmpDisabled=%d",
            pContext->pBundledContext->bVirtualizerEnabled,
            pContext->pBundledContext->bVirtualizerTempDisabled);

    return status;
}
//----------------------------------------------------------------------------
// VirtualizerGetSpeakerAngles()
//----------------------------------------------------------------------------
// Purpose:
// Get the virtual speaker angles for a channel mask + audio device type
// configuration which is guaranteed to be supported by this implementation
//
// Inputs:
//  channelMask:   the channel mask of the input to virtualize
//  deviceType     the type of device that affects the processing (e.g. for binaural vs transaural)
// Input/Output:
//  pSpeakerAngles the array of integer where each speaker angle is written as a triplet in the
//                 following format:
//                    int32_t a bit mask with a single value selected for each speaker, following
//                            the convention of the audio_channel_mask_t type
//                    int32_t a value in degrees expressing the speaker azimuth, where 0 is in front
//                            of the user, 180 behind, -90 to the left, 90 to the right of the user
//                    int32_t a value in degrees expressing the speaker elevation, where 0 is the
//                            horizontal plane, +90 is directly above the user, -90 below
//
//----------------------------------------------------------------------------
void VirtualizerGetSpeakerAngles(audio_channel_mask_t channelMask,
        audio_devices_t deviceType __unused, int32_t *pSpeakerAngles) {
    // the channel count is guaranteed to be 1 or 2
    // the device is guaranteed to be of type headphone
    // this virtualizer is always using 2 virtual speakers at -90 and 90deg of azimuth, 0deg of
    // elevation but the return information is sized for nbChannels * 3, so we have to consider
    // the (false here) case of a single channel, and return only 3 fields.
    if (audio_channel_count_from_out_mask(channelMask) == 1) {
        *pSpeakerAngles++ = (int32_t) AUDIO_CHANNEL_OUT_MONO; // same as FRONT_LEFT
        *pSpeakerAngles++ = 0; // azimuth
        *pSpeakerAngles = 0; // elevation
    } else {
        *pSpeakerAngles++ = (int32_t) AUDIO_CHANNEL_OUT_FRONT_LEFT;
        *pSpeakerAngles++ = -90; // azimuth
        *pSpeakerAngles++ = 0;   // elevation
        *pSpeakerAngles++ = (int32_t) AUDIO_CHANNEL_OUT_FRONT_RIGHT;
        *pSpeakerAngles++ = 90;  // azimuth
        *pSpeakerAngles   = 0;   // elevation
    }
}

//----------------------------------------------------------------------------
// VirtualizerGetVirtualizationMode()
//----------------------------------------------------------------------------
// Purpose:
// Retrieve the current device whose processing mode is used by this effect
//
// Output:
//   AUDIO_DEVICE_NONE if the effect is not virtualizing
//   or the device type if the effect is virtualizing
//----------------------------------------------------------------------------
audio_devices_t VirtualizerGetVirtualizationMode(EffectContext *pContext) {
    audio_devices_t virtDevice = AUDIO_DEVICE_NONE;
    if ((pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE)
            && (pContext->pBundledContext->bVirtualizerTempDisabled == LVM_FALSE)) {
        if (pContext->pBundledContext->nVirtualizerForcedDevice != AUDIO_DEVICE_NONE) {
            // virtualization mode is forced, return that device
            virtDevice = pContext->pBundledContext->nVirtualizerForcedDevice;
        } else {
            // no forced mode, return the current device
            virtDevice = pContext->pBundledContext->nOutputDevice;
        }
    }
    ALOGV("VirtualizerGetVirtualizationMode() returning 0x%x", virtDevice);
    return virtDevice;
}

//----------------------------------------------------------------------------
// EqualizerGetBandLevel()
//----------------------------------------------------------------------------
// Purpose: Retrieve the gain currently being used for the band passed in
//
// Inputs:
//  band:       band number
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------
int32_t EqualizerGetBandLevel(EffectContext *pContext, int32_t band){
    //ALOGV("\tEqualizerGetBandLevel -> %d\n", pContext->pBundledContext->bandGaindB[band] );
    return pContext->pBundledContext->bandGaindB[band] * 100;
}

//----------------------------------------------------------------------------
// EqualizerSetBandLevel()
//----------------------------------------------------------------------------
// Purpose:
//  Sets gain value for the given band.
//
// Inputs:
//  band:       band number
//  Gain:       Gain to be applied in millibels
//  pContext:   effect engine context
//
// Outputs:
//
//---------------------------------------------------------------------------
void EqualizerSetBandLevel(EffectContext *pContext, int band, short Gain){
    int gainRounded;
    if(Gain > 0){
        gainRounded = (int)((Gain+50)/100);
    }else{
        gainRounded = (int)((Gain-50)/100);
    }
    //ALOGV("\tEqualizerSetBandLevel(%d)->(%d)", Gain, gainRounded);
    pContext->pBundledContext->bandGaindB[band] = gainRounded;
    pContext->pBundledContext->CurPreset = PRESET_CUSTOM;

    EqualizerUpdateActiveParams(pContext);
    LvmEffect_limitLevel(pContext);
}

//----------------------------------------------------------------------------
// EqualizerGetCentreFrequency()
//----------------------------------------------------------------------------
// Purpose: Retrieve the frequency being used for the band passed in
//
// Inputs:
//  band:       band number
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------
int32_t EqualizerGetCentreFrequency(EffectContext *pContext, int32_t band){
    int32_t Frequency =0;

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    LVM_EQNB_BandDef_t      *BandDef;
    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,
                                         &ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "EqualizerGetCentreFrequency")

    BandDef   = ActiveParams.pEQNB_BandDefinition;
    Frequency = (int32_t)BandDef[band].Frequency*1000;     // Convert to millibels

    //ALOGV("\tEqualizerGetCentreFrequency -> %d\n", Frequency );
    //ALOGV("\tEqualizerGetCentreFrequency Succesfully returned from LVM_GetControlParameters\n");
    return Frequency;
}

//----------------------------------------------------------------------------
// EqualizerGetBandFreqRange(
//----------------------------------------------------------------------------
// Purpose:
//
// Gets lower and upper boundaries of a band.
// For the high shelf, the low bound is the band frequency and the high
// bound is Nyquist.
// For the peaking filters, they are the gain[dB]/2 points.
//
// Inputs:
//  band:       band number
//  pContext:   effect engine context
//
// Outputs:
//  pLow:       lower band range
//  pLow:       upper band range
//----------------------------------------------------------------------------
int32_t EqualizerGetBandFreqRange(EffectContext *pContext __unused, int32_t band, uint32_t *pLow,
                                  uint32_t *pHi){
    *pLow = bandFreqRange[band][0];
    *pHi  = bandFreqRange[band][1];
    return 0;
}

//----------------------------------------------------------------------------
// EqualizerGetBand(
//----------------------------------------------------------------------------
// Purpose:
//
// Returns the band with the maximum influence on a given frequency.
// Result is unaffected by whether EQ is enabled or not, or by whether
// changes have been committed or not.
//
// Inputs:
//  targetFreq   The target frequency, in millihertz.
//  pContext:    effect engine context
//
// Outputs:
//  pLow:       lower band range
//  pLow:       upper band range
//----------------------------------------------------------------------------
int32_t EqualizerGetBand(EffectContext *pContext __unused, uint32_t targetFreq){
    int band = 0;

    if(targetFreq < bandFreqRange[0][0]){
        return -EINVAL;
    }else if(targetFreq == bandFreqRange[0][0]){
        return 0;
    }
    for(int i=0; i<FIVEBAND_NUMBANDS;i++){
        if((targetFreq > bandFreqRange[i][0])&&(targetFreq <= bandFreqRange[i][1])){
            band = i;
        }
    }
    return band;
}

//----------------------------------------------------------------------------
// EqualizerGetPreset(
//----------------------------------------------------------------------------
// Purpose:
//
// Gets the currently set preset ID.
// Will return PRESET_CUSTOM in case the EQ parameters have been modified
// manually since a preset was set.
//
// Inputs:
//  pContext:    effect engine context
//
//----------------------------------------------------------------------------
int32_t EqualizerGetPreset(EffectContext *pContext){
    return pContext->pBundledContext->CurPreset;
}

//----------------------------------------------------------------------------
// EqualizerSetPreset(
//----------------------------------------------------------------------------
// Purpose:
//
// Sets the current preset by ID.
// All the band parameters will be overridden.
//
// Inputs:
//  pContext:    effect engine context
//  preset       The preset ID.
//
//----------------------------------------------------------------------------
void EqualizerSetPreset(EffectContext *pContext, int preset){

    //ALOGV("\tEqualizerSetPreset(%d)", preset);
    pContext->pBundledContext->CurPreset = preset;

    //ActiveParams.pEQNB_BandDefinition = &BandDefs[0];
    for (int i=0; i<FIVEBAND_NUMBANDS; i++)
    {
        pContext->pBundledContext->bandGaindB[i] =
                EQNB_5BandSoftPresets[i + preset * FIVEBAND_NUMBANDS];
    }

    EqualizerUpdateActiveParams(pContext);
    LvmEffect_limitLevel(pContext);

    //ALOGV("\tEqualizerSetPreset Succesfully called LVM_SetControlParameters\n");
    return;
}

int32_t EqualizerGetNumPresets(){
    return sizeof(gEqualizerPresets) / sizeof(PresetConfig);
}

//----------------------------------------------------------------------------
// EqualizerGetPresetName(
//----------------------------------------------------------------------------
// Purpose:
// Gets a human-readable name for a preset ID. Will return "Custom" if
// PRESET_CUSTOM is passed.
//
// Inputs:
// preset       The preset ID. Must be less than number of presets.
//
//-------------------------------------------------------------------------
const char * EqualizerGetPresetName(int32_t preset){
    //ALOGV("\tEqualizerGetPresetName start(%d)", preset);
    if (preset == PRESET_CUSTOM) {
        return "Custom";
    } else {
        return gEqualizerPresets[preset].name;
    }
    //ALOGV("\tEqualizerGetPresetName end(%d)", preset);
    return 0;
}

//----------------------------------------------------------------------------
// VolumeSetVolumeLevel()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:   effect engine context
//  level       level to be applied
//
//----------------------------------------------------------------------------

int VolumeSetVolumeLevel(EffectContext *pContext, int16_t level){

    if (level > 0 || level < -9600) {
        return -EINVAL;
    }

    if (pContext->pBundledContext->bMuteEnabled == LVM_TRUE) {
        pContext->pBundledContext->levelSaved = level / 100;
    } else {
        pContext->pBundledContext->volume = level / 100;
    }

    LvmEffect_limitLevel(pContext);

    return 0;
}    /* end VolumeSetVolumeLevel */

//----------------------------------------------------------------------------
// VolumeGetVolumeLevel()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:   effect engine context
//
//----------------------------------------------------------------------------

int VolumeGetVolumeLevel(EffectContext *pContext, int16_t *level){

    if (pContext->pBundledContext->bMuteEnabled == LVM_TRUE) {
        *level = pContext->pBundledContext->levelSaved * 100;
    } else {
        *level = pContext->pBundledContext->volume * 100;
    }
    return 0;
}    /* end VolumeGetVolumeLevel */

//----------------------------------------------------------------------------
// VolumeSetMute()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:   effect engine context
//  mute:       enable/disable flag
//
//----------------------------------------------------------------------------

int32_t VolumeSetMute(EffectContext *pContext, uint32_t mute){
    //ALOGV("\tVolumeSetMute start(%d)", mute);

    pContext->pBundledContext->bMuteEnabled = mute;

    /* Set appropriate volume level */
    if(pContext->pBundledContext->bMuteEnabled == LVM_TRUE){
        pContext->pBundledContext->levelSaved = pContext->pBundledContext->volume;
        pContext->pBundledContext->volume = -96;
    }else{
        pContext->pBundledContext->volume = pContext->pBundledContext->levelSaved;
    }

    LvmEffect_limitLevel(pContext);

    return 0;
}    /* end setMute */

//----------------------------------------------------------------------------
// VolumeGetMute()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:   effect engine context
//
// Ourputs:
//  mute:       enable/disable flag
//----------------------------------------------------------------------------

int32_t VolumeGetMute(EffectContext *pContext, uint32_t *mute){
    //ALOGV("\tVolumeGetMute start");
    if((pContext->pBundledContext->bMuteEnabled == LVM_FALSE)||
       (pContext->pBundledContext->bMuteEnabled == LVM_TRUE)){
        *mute = pContext->pBundledContext->bMuteEnabled;
        return 0;
    }else{
        ALOGV("\tLVM_ERROR : VolumeGetMute read an invalid value from context %d",
              pContext->pBundledContext->bMuteEnabled);
        return -EINVAL;
    }
    //ALOGV("\tVolumeGetMute end");
}    /* end getMute */

int16_t VolumeConvertStereoPosition(int16_t position){
    int16_t convertedPosition = 0;

    convertedPosition = (int16_t)(((float)position/1000)*96);
    return convertedPosition;

}

//----------------------------------------------------------------------------
// VolumeSetStereoPosition()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:       effect engine context
//  position:       stereo position
//
// Outputs:
//----------------------------------------------------------------------------

int VolumeSetStereoPosition(EffectContext *pContext, int16_t position){

    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */
    LVM_INT16               Balance = 0;



    pContext->pBundledContext->positionSaved = position;
    Balance = VolumeConvertStereoPosition(pContext->pBundledContext->positionSaved);

    //ALOGV("\tVolumeSetStereoPosition start pContext->pBundledContext->positionSaved = %d",
    //pContext->pBundledContext->positionSaved);

    if(pContext->pBundledContext->bStereoPositionEnabled == LVM_TRUE){

        //ALOGV("\tVolumeSetStereoPosition Position to be set is %d %d\n", position, Balance);
        pContext->pBundledContext->positionSaved = position;
        /* Get the current settings */
        LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
        LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VolumeSetStereoPosition")
        if(LvmStatus != LVM_SUCCESS) return -EINVAL;
        //ALOGV("\tVolumeSetStereoPosition Succesfully returned from LVM_GetControlParameters got:"
        //     " %d\n", ActiveParams.VC_Balance);

        /* Volume parameters */
        ActiveParams.VC_Balance  = Balance;
        //ALOGV("\tVolumeSetStereoPosition() (-96dB -> +96dB)   -> %d\n", ActiveParams.VC_Balance );

        /* Activate the initial settings */
        LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
        LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "VolumeSetStereoPosition")
        if(LvmStatus != LVM_SUCCESS) return -EINVAL;

        //ALOGV("\tVolumeSetStereoPosition Succesfully called LVM_SetControlParameters\n");

        /* Get the current settings */
        LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
        LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VolumeSetStereoPosition")
        if(LvmStatus != LVM_SUCCESS) return -EINVAL;
        //ALOGV("\tVolumeSetStereoPosition Succesfully returned from LVM_GetControlParameters got: "
        //     "%d\n", ActiveParams.VC_Balance);
    }
    else{
        //ALOGV("\tVolumeSetStereoPosition Position attempting to set, but not enabled %d %d\n",
        //position, Balance);
    }
    //ALOGV("\tVolumeSetStereoPosition end pContext->pBundledContext->positionSaved = %d\n",
    //pContext->pBundledContext->positionSaved);
    return 0;
}    /* end VolumeSetStereoPosition */


//----------------------------------------------------------------------------
// VolumeGetStereoPosition()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:       effect engine context
//
// Outputs:
//  position:       stereo position
//----------------------------------------------------------------------------

int32_t VolumeGetStereoPosition(EffectContext *pContext, int16_t *position){
    //ALOGV("\tVolumeGetStereoPosition start");

    LVM_ControlParams_t     ActiveParams;                           /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus = LVM_SUCCESS;                /* Function call status */
    LVM_INT16               balance;

    //ALOGV("\tVolumeGetStereoPosition start pContext->pBundledContext->positionSaved = %d",
    //pContext->pBundledContext->positionSaved);

    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VolumeGetStereoPosition")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tVolumeGetStereoPosition -> %d\n", ActiveParams.VC_Balance);
    //ALOGV("\tVolumeGetStereoPosition Succesfully returned from LVM_GetControlParameters\n");

    balance = VolumeConvertStereoPosition(pContext->pBundledContext->positionSaved);

    if(pContext->pBundledContext->bStereoPositionEnabled == LVM_TRUE){
        if(balance != ActiveParams.VC_Balance){
            return -EINVAL;
        }
    }
    *position = (LVM_INT16)pContext->pBundledContext->positionSaved;     // Convert dB to millibels
    //ALOGV("\tVolumeGetStereoPosition end returning pContext->pBundledContext->positionSaved =%d\n",
    //pContext->pBundledContext->positionSaved);
    return 0;
}    /* end VolumeGetStereoPosition */

//----------------------------------------------------------------------------
// VolumeEnableStereoPosition()
//----------------------------------------------------------------------------
// Purpose:
//
// Inputs:
//  pContext:   effect engine context
//  mute:       enable/disable flag
//
//----------------------------------------------------------------------------

int32_t VolumeEnableStereoPosition(EffectContext *pContext, uint32_t enabled){
    //ALOGV("\tVolumeEnableStereoPosition start()");

    pContext->pBundledContext->bStereoPositionEnabled = enabled;

    LVM_ControlParams_t     ActiveParams;              /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;     /* Function call status */

    /* Get the current settings */
    LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VolumeEnableStereoPosition")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tVolumeEnableStereoPosition Succesfully returned from LVM_GetControlParameters\n");
    //ALOGV("\tVolumeEnableStereoPosition to %d, position was %d\n",
    //     enabled, ActiveParams.VC_Balance );

    /* Set appropriate stereo position */
    if(pContext->pBundledContext->bStereoPositionEnabled == LVM_FALSE){
        ActiveParams.VC_Balance = 0;
    }else{
        ActiveParams.VC_Balance  =
                            VolumeConvertStereoPosition(pContext->pBundledContext->positionSaved);
    }

    /* Activate the initial settings */
    LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "VolumeEnableStereoPosition")
    if(LvmStatus != LVM_SUCCESS) return -EINVAL;

    //ALOGV("\tVolumeEnableStereoPosition Succesfully called LVM_SetControlParameters\n");
    //ALOGV("\tVolumeEnableStereoPosition end()\n");
    return 0;
}    /* end VolumeEnableStereoPosition */

//----------------------------------------------------------------------------
// BassBoost_getParameter()
//----------------------------------------------------------------------------
// Purpose:
// Get a BassBoost parameter
//
// Inputs:
//  pBassBoost       - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to variable to hold retrieved value
//  pValueSize       - pointer to value size: maximum size as input
//
// Outputs:
//  *pValue updated with parameter value
//  *pValueSize updated with actual value size
//
//
// Side Effects:
//
//----------------------------------------------------------------------------

int BassBoost_getParameter(EffectContext *pContext,
                           uint32_t       paramSize,
                           void          *pParam,
                           uint32_t      *pValueSize,
                           void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case BASSBOOST_PARAM_STRENGTH_SUPPORTED:
            if (*pValueSize != sizeof(uint32_t)) {  // legacy: check equality here.
                ALOGV("%s BASSBOOST_PARAM_STRENGTH_SUPPORTED invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            *(uint32_t *)pValue = 1;
            ALOGVV("%s BASSBOOST_PARAM_STRENGTH_SUPPORTED %u", __func__, *(uint32_t *)pValue);
            break;

        case BASSBOOST_PARAM_STRENGTH:
            if (*pValueSize != sizeof(int16_t)) {  // legacy: check equality here.
                ALOGV("%s BASSBOOST_PARAM_STRENGTH invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            *(int16_t *)pValue = BassGetStrength(pContext);
            ALOGVV("%s BASSBOOST_PARAM_STRENGTH %d", __func__, *(int16_t *)pValue);
            break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end BassBoost_getParameter */

//----------------------------------------------------------------------------
// BassBoost_setParameter()
//----------------------------------------------------------------------------
// Purpose:
// Set a BassBoost parameter
//
// Inputs:
//  pBassBoost       - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to value
//
// Outputs:
//
//----------------------------------------------------------------------------

int BassBoost_setParameter(EffectContext *pContext,
                           uint32_t       paramSize,
                           void          *pParam,
                           uint32_t       valueSize,
                           void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize != sizeof(int32_t)) {  // legacy: check equality here.
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case BASSBOOST_PARAM_STRENGTH: {
            if (valueSize < sizeof(int16_t)) {
                ALOGV("%s BASSBOOST_PARAM_STRENGTH invalid valueSize: %u", __func__, valueSize);
                status = -EINVAL;
                break;
            }

            const int16_t strength = *(int16_t *)pValue;
            ALOGVV("%s BASSBOOST_PARAM_STRENGTH %d", __func__, strength);
            ALOGVV("%s BASSBOOST_PARAM_STRENGTH Calling BassSetStrength", __func__);
            BassSetStrength(pContext, (int32_t)strength);
            ALOGVV("%s BASSBOOST_PARAM_STRENGTH Called BassSetStrength", __func__);
        } break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end BassBoost_setParameter */

//----------------------------------------------------------------------------
// Virtualizer_getParameter()
//----------------------------------------------------------------------------
// Purpose:
// Get a Virtualizer parameter
//
// Inputs:
//  pVirtualizer     - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to variable to hold retrieved value
//  pValueSize       - pointer to value size: maximum size as input
//
// Outputs:
//  *pValue updated with parameter value
//  *pValueSize updated with actual value size
//
//
// Side Effects:
//
//----------------------------------------------------------------------------

int Virtualizer_getParameter(EffectContext *pContext,
                             uint32_t       paramSize,
                             void          *pParam,
                             uint32_t      *pValueSize,
                             void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case VIRTUALIZER_PARAM_STRENGTH_SUPPORTED:
            if (*pValueSize != sizeof(uint32_t)) { // legacy: check equality here.
                ALOGV("%s VIRTUALIZER_PARAM_STRENGTH_SUPPORTED invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            *(uint32_t *)pValue = 1;
            ALOGVV("%s VIRTUALIZER_PARAM_STRENGTH_SUPPORTED %d", __func__, *(uint32_t *)pValue);
            break;

        case VIRTUALIZER_PARAM_STRENGTH:
            if (*pValueSize != sizeof(int16_t)) { // legacy: check equality here.
                ALOGV("%s VIRTUALIZER_PARAM_STRENGTH invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            *(int16_t *)pValue = VirtualizerGetStrength(pContext);

            ALOGVV("%s VIRTUALIZER_PARAM_STRENGTH %d", __func__, *(int16_t *)pValue);
            break;

        case VIRTUALIZER_PARAM_VIRTUAL_SPEAKER_ANGLES: {
            if (paramSize < 3 * sizeof(int32_t)) {
                ALOGV("%s VIRTUALIZER_PARAM_SPEAKER_ANGLES invalid paramSize: %u",
                        __func__, paramSize);
                status = -EINVAL;
                break;
            }

            const audio_channel_mask_t channelMask = (audio_channel_mask_t) params[1];
            const audio_devices_t deviceType = (audio_devices_t) params[2];
            const uint32_t nbChannels = audio_channel_count_from_out_mask(channelMask);
            const uint32_t valueSizeRequired = 3 * nbChannels * sizeof(int32_t);
            if (*pValueSize < valueSizeRequired) {
                ALOGV("%s VIRTUALIZER_PARAM_SPEAKER_ANGLES invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            *pValueSize = valueSizeRequired;

            // verify the configuration is supported
            status = VirtualizerIsConfigurationSupported(channelMask, deviceType);
            if (status == 0) {
                ALOGV("%s VIRTUALIZER_PARAM_VIRTUAL_SPEAKER_ANGLES mask=0x%x device=0x%x",
                        __func__, channelMask, deviceType);
                // configuration is supported, get the angles
                VirtualizerGetSpeakerAngles(channelMask, deviceType, (int32_t *)pValue);
            }
        } break;

        case VIRTUALIZER_PARAM_VIRTUALIZATION_MODE:
            if (*pValueSize != sizeof(uint32_t)) { // legacy: check equality here.
                ALOGV("%s VIRTUALIZER_PARAM_VIRTUALIZATION_MODE invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            *(uint32_t *)pValue = (uint32_t) VirtualizerGetVirtualizationMode(pContext);
            break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Virtualizer_getParameter */

//----------------------------------------------------------------------------
// Virtualizer_setParameter()
//----------------------------------------------------------------------------
// Purpose:
// Set a Virtualizer parameter
//
// Inputs:
//  pVirtualizer     - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to value
//
// Outputs:
//
//----------------------------------------------------------------------------

int Virtualizer_setParameter(EffectContext *pContext,
                             uint32_t       paramSize,
                             void          *pParam,
                             uint32_t       valueSize,
                             void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize != sizeof(int32_t)) { // legacy: check equality here.
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case VIRTUALIZER_PARAM_STRENGTH: {
            if (valueSize < sizeof(int16_t)) {
                ALOGV("%s VIRTUALIZER_PARAM_STRENGTH invalid valueSize: %u", __func__, valueSize);
                status = -EINVAL;
                break;
            }

            const int16_t strength = *(int16_t *)pValue;
            ALOGVV("%s VIRTUALIZER_PARAM_STRENGTH %d", __func__, strength);
            ALOGVV("%s VIRTUALIZER_PARAM_STRENGTH Calling VirtualizerSetStrength", __func__);
            VirtualizerSetStrength(pContext, (int32_t)strength);
            ALOGVV("%s VIRTUALIZER_PARAM_STRENGTH Called VirtualizerSetStrength", __func__);
        } break;

        case VIRTUALIZER_PARAM_FORCE_VIRTUALIZATION_MODE: {
            if (valueSize < sizeof(int32_t)) {
                ALOGV("%s VIRTUALIZER_PARAM_FORCE_VIRTUALIZATION_MODE invalid valueSize: %u",
                        __func__, valueSize);
                android_errorWriteLog(0x534e4554, "64478003");
                status = -EINVAL;
                break;
            }

            const audio_devices_t deviceType = (audio_devices_t)*(int32_t *)pValue;
            status = VirtualizerForceVirtualizationMode(pContext, deviceType);
            ALOGVV("%s VIRTUALIZER_PARAM_FORCE_VIRTUALIZATION_MODE device=%#x result=%d",
                    __func__, deviceType, status);
        } break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Virtualizer_setParameter */

//----------------------------------------------------------------------------
// Equalizer_getParameter()
//----------------------------------------------------------------------------
// Purpose:
// Get a Equalizer parameter
//
// Inputs:
//  pEqualizer       - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to variable to hold retrieved value
//  pValueSize       - pointer to value size: maximum size as input
//
// Outputs:
//  *pValue updated with parameter value
//  *pValueSize updated with actual value size
//
//
// Side Effects:
//
//----------------------------------------------------------------------------
int Equalizer_getParameter(EffectContext *pContext,
                           uint32_t       paramSize,
                           void          *pParam,
                           uint32_t      *pValueSize,
                           void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
    case EQ_PARAM_NUM_BANDS:
        if (*pValueSize < sizeof(uint16_t)) {
            ALOGV("%s EQ_PARAM_NUM_BANDS invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(uint16_t);

        *(uint16_t *)pValue = (uint16_t)FIVEBAND_NUMBANDS;
        ALOGVV("%s EQ_PARAM_NUM_BANDS %u", __func__, *(uint16_t *)pValue);
        break;

    case EQ_PARAM_CUR_PRESET:
        if (*pValueSize < sizeof(uint16_t)) {
            ALOGV("%s EQ_PARAM_CUR_PRESET invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(uint16_t);

        *(uint16_t *)pValue = (uint16_t)EqualizerGetPreset(pContext);
        ALOGVV("%s EQ_PARAM_CUR_PRESET %u", __func__, *(uint16_t *)pValue);
        break;

    case EQ_PARAM_GET_NUM_OF_PRESETS:
        if (*pValueSize < sizeof(uint16_t)) {
            ALOGV("%s EQ_PARAM_GET_NUM_OF_PRESETS invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(uint16_t);

        *(uint16_t *)pValue = (uint16_t)EqualizerGetNumPresets();
        ALOGVV("%s EQ_PARAM_GET_NUM_OF_PRESETS %u", __func__, *(uint16_t *)pValue);
        break;

    case EQ_PARAM_GET_BAND: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_GET_BAND invalid paramSize: %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (*pValueSize < sizeof(uint16_t)) {
            ALOGV("%s EQ_PARAM_GET_BAND invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(uint16_t);

        const int32_t frequency = params[1];
        *(uint16_t *)pValue = (uint16_t)EqualizerGetBand(pContext, frequency);
        ALOGVV("%s EQ_PARAM_GET_BAND frequency %d, band %u",
                __func__, frequency, *(uint16_t *)pValue);
    } break;

    case EQ_PARAM_BAND_LEVEL: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_BAND_LEVEL invalid paramSize %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (*pValueSize < sizeof(int16_t)) {
            ALOGV("%s EQ_PARAM_BAND_LEVEL invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(int16_t);

        const int32_t band = params[1];
        if (band < 0 || band >= FIVEBAND_NUMBANDS) {
            if (band < 0) {
                android_errorWriteLog(0x534e4554, "32438598");
                ALOGW("%s EQ_PARAM_BAND_LEVEL invalid band %d", __func__, band);
            }
            status = -EINVAL;
            break;
        }
        *(int16_t *)pValue = (int16_t)EqualizerGetBandLevel(pContext, band);
        ALOGVV("%s EQ_PARAM_BAND_LEVEL band %d, level %d",
                __func__, band, *(int16_t *)pValue);
    } break;

    case EQ_PARAM_LEVEL_RANGE:
        if (*pValueSize < 2 * sizeof(int16_t)) {
            ALOGV("%s EQ_PARAM_LEVEL_RANGE invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = 2 * sizeof(int16_t);

        *(int16_t *)pValue = -1500;
        *((int16_t *)pValue + 1) = 1500;
        ALOGVV("%s EQ_PARAM_LEVEL_RANGE min %d, max %d",
                __func__, *(int16_t *)pValue, *((int16_t *)pValue + 1));
        break;

    case EQ_PARAM_BAND_FREQ_RANGE: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_BAND_FREQ_RANGE invalid paramSize: %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (*pValueSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_BAND_FREQ_RANGE invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = 2 * sizeof(int32_t);

        const int32_t band = params[1];
        if (band < 0 || band >= FIVEBAND_NUMBANDS) {
            if (band < 0) {
                android_errorWriteLog(0x534e4554, "32247948");
                ALOGW("%s EQ_PARAM_BAND_FREQ_RANGE invalid band %d",
                        __func__, band);
            }
            status = -EINVAL;
            break;
        }
        EqualizerGetBandFreqRange(pContext, band, (uint32_t *)pValue, ((uint32_t *)pValue + 1));
        ALOGVV("%s EQ_PARAM_BAND_FREQ_RANGE band %d, min %d, max %d",
                __func__, band, *(int32_t *)pValue, *((int32_t *)pValue + 1));

    } break;

    case EQ_PARAM_CENTER_FREQ: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_CENTER_FREQ invalid paramSize: %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (*pValueSize < sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_CENTER_FREQ invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = sizeof(int32_t);

        const int32_t band = params[1];
        if (band < 0 || band >= FIVEBAND_NUMBANDS) {
            status = -EINVAL;
            if (band < 0) {
                android_errorWriteLog(0x534e4554, "32436341");
                ALOGW("%s EQ_PARAM_CENTER_FREQ invalid band %d", __func__, band);
            }
            break;
        }
        *(int32_t *)pValue = EqualizerGetCentreFrequency(pContext, band);
        ALOGVV("%s EQ_PARAM_CENTER_FREQ band %d, frequency %d",
                __func__, band, *(int32_t *)pValue);
    } break;

    case EQ_PARAM_GET_PRESET_NAME: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_PRESET_NAME invalid paramSize: %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (*pValueSize < 1) {
            android_errorWriteLog(0x534e4554, "37536407");
            status = -EINVAL;
            break;
        }

        const int32_t preset = params[1];
        if ((preset < 0 && preset != PRESET_CUSTOM) ||  preset >= EqualizerGetNumPresets()) {
            if (preset < 0) {
                android_errorWriteLog(0x534e4554, "32448258");
                ALOGE("%s EQ_PARAM_GET_PRESET_NAME preset %d", __func__, preset);
            }
            status = -EINVAL;
            break;
        }

        char * const name = (char *)pValue;
        strncpy(name, EqualizerGetPresetName(preset), *pValueSize - 1);
        name[*pValueSize - 1] = 0;
        *pValueSize = strlen(name) + 1;
        ALOGVV("%s EQ_PARAM_GET_PRESET_NAME preset %d, name %s len %d",
                __func__, preset, gEqualizerPresets[preset].name, *pValueSize);

    } break;

    case EQ_PARAM_PROPERTIES: {
        constexpr uint32_t requiredValueSize = (2 + FIVEBAND_NUMBANDS) * sizeof(uint16_t);
        if (*pValueSize < requiredValueSize) {
            ALOGV("%s EQ_PARAM_PROPERTIES invalid *pValueSize %u", __func__, *pValueSize);
            status = -EINVAL;
            break;
        }
        *pValueSize = requiredValueSize;

        int16_t *p = (int16_t *)pValue;
        ALOGV("%s EQ_PARAM_PROPERTIES", __func__);
        p[0] = (int16_t)EqualizerGetPreset(pContext);
        p[1] = (int16_t)FIVEBAND_NUMBANDS;
        for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
            p[2 + i] = (int16_t)EqualizerGetBandLevel(pContext, i);
        }
    } break;

    default:
        ALOGV("%s invalid param %d", __func__, params[0]);
        status = -EINVAL;
        break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Equalizer_getParameter */

//----------------------------------------------------------------------------
// Equalizer_setParameter()
//----------------------------------------------------------------------------
// Purpose:
// Set a Equalizer parameter
//
// Inputs:
//  pEqualizer    - handle to instance data
//  pParam        - pointer to parameter
//  valueSize     - value size
//  pValue        - pointer to value

//
// Outputs:
//
//----------------------------------------------------------------------------
int Equalizer_setParameter(EffectContext *pContext,
                           uint32_t       paramSize,
                           void          *pParam,
                           uint32_t       valueSize,
                           void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
    case EQ_PARAM_CUR_PRESET: {
        if (valueSize < sizeof(int16_t)) {
            ALOGV("%s EQ_PARAM_CUR_PRESET invalid valueSize %u", __func__, valueSize);
            status = -EINVAL;
            break;
        }
        const int32_t preset = (int32_t)*(uint16_t *)pValue;

        ALOGVV("%s EQ_PARAM_CUR_PRESET %d", __func__, preset);
        if (preset >= EqualizerGetNumPresets() || preset < 0) {
            ALOGV("%s EQ_PARAM_CUR_PRESET invalid preset %d", __func__, preset);
            status = -EINVAL;
            break;
        }
        EqualizerSetPreset(pContext, preset);
    } break;

    case EQ_PARAM_BAND_LEVEL: {
        if (paramSize < 2 * sizeof(int32_t)) {
            ALOGV("%s EQ_PARAM_BAND_LEVEL invalid paramSize: %u", __func__, paramSize);
            status = -EINVAL;
            break;
        }
        if (valueSize < sizeof(int16_t)) {
            ALOGV("%s EQ_PARAM_BAND_LEVEL invalid valueSize %u", __func__, valueSize);
            status = -EINVAL;
            break;
        }
        const int32_t band =  params[1];
        const int32_t level = (int32_t)*(int16_t *)pValue;
        ALOGVV("%s EQ_PARAM_BAND_LEVEL band %d, level %d", __func__, band, level);
        if (band < 0 || band >= FIVEBAND_NUMBANDS) {
            if (band < 0) {
                android_errorWriteLog(0x534e4554, "32095626");
                ALOGE("%s EQ_PARAM_BAND_LEVEL invalid band %d", __func__, band);
            }
            status = -EINVAL;
            break;
        }
        EqualizerSetBandLevel(pContext, band, level);
    } break;

    case EQ_PARAM_PROPERTIES: {
        ALOGVV("%s EQ_PARAM_PROPERTIES", __func__);
        if (valueSize < sizeof(int16_t)) {
            ALOGV("%s EQ_PARAM_PROPERTIES invalid valueSize %u", __func__, valueSize);
            status = -EINVAL;
            break;
        }
        int16_t *p = (int16_t *)pValue;
        if ((int)p[0] >= EqualizerGetNumPresets()) {
            ALOGV("%s EQ_PARAM_PROPERTIES invalid preset %d", __func__, (int)p[0]);
            status = -EINVAL;
            break;
        }
        if (p[0] >= 0) {
            EqualizerSetPreset(pContext, (int)p[0]);
        } else {
            constexpr uint32_t valueSizeRequired = (2 + FIVEBAND_NUMBANDS) * sizeof(int16_t);
            if (valueSize < valueSizeRequired) {
              android_errorWriteLog(0x534e4554, "37563371");
              ALOGE("%s EQ_PARAM_PROPERTIES invalid valueSize %u < %u",
                      __func__, valueSize, valueSizeRequired);
              status = -EINVAL;
              break;
            }
            if ((int)p[1] != FIVEBAND_NUMBANDS) {
                ALOGV("%s EQ_PARAM_PROPERTIES invalid bands %d", __func__, (int)p[1]);
                status = -EINVAL;
                break;
            }
            for (int i = 0; i < FIVEBAND_NUMBANDS; i++) {
                EqualizerSetBandLevel(pContext, i, (int)p[2 + i]);
            }
        }
    } break;

    default:
        ALOGV("%s invalid param %d", __func__, params[0]);
        status = -EINVAL;
        break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Equalizer_setParameter */

//----------------------------------------------------------------------------
// Volume_getParameter()
//----------------------------------------------------------------------------
// Purpose:
// Get a Volume parameter
//
// Inputs:
//  pVolume          - handle to instance data
//  pParam           - pointer to parameter
//  pValue           - pointer to variable to hold retrieved value
//  pValueSize       - pointer to value size: maximum size as input
//
// Outputs:
//  *pValue updated with parameter value
//  *pValueSize updated with actual value size
//
//
// Side Effects:
//
//----------------------------------------------------------------------------

int Volume_getParameter(EffectContext *pContext,
                        uint32_t       paramSize,
                        void          *pParam,
                        uint32_t      *pValueSize,
                        void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case VOLUME_PARAM_LEVEL:
            if (*pValueSize != sizeof(int16_t)) { // legacy: check equality here.
                ALOGV("%s VOLUME_PARAM_LEVEL invalid *pValueSize %u", __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            status = VolumeGetVolumeLevel(pContext, (int16_t *)(pValue));
            ALOGVV("%s VOLUME_PARAM_LEVEL %d", __func__, *(int16_t *)pValue);
            break;

        case VOLUME_PARAM_MAXLEVEL:
            if (*pValueSize != sizeof(int16_t)) { // legacy: check equality here.
                ALOGV("%s VOLUME_PARAM_MAXLEVEL invalid *pValueSize %u", __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            // in millibel
            *(int16_t *)pValue = 0;
            ALOGVV("%s VOLUME_PARAM_MAXLEVEL %d", __func__, *(int16_t *)pValue);
            break;

        case VOLUME_PARAM_STEREOPOSITION:
            if (*pValueSize != sizeof(int16_t)) { // legacy: check equality here.
                ALOGV("%s VOLUME_PARAM_STEREOPOSITION invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            // no need to set *pValueSize

            VolumeGetStereoPosition(pContext, (int16_t *)pValue);
            ALOGVV("%s VOLUME_PARAM_STEREOPOSITION %d", __func__, *(int16_t *)pValue);
            break;

        case VOLUME_PARAM_MUTE:
            if (*pValueSize < sizeof(uint32_t)) {
                ALOGV("%s VOLUME_PARAM_MUTE invalid *pValueSize %u", __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            *pValueSize = sizeof(uint32_t);

            status = VolumeGetMute(pContext, (uint32_t *)pValue);
            ALOGV("%s VOLUME_PARAM_MUTE %u", __func__, *(uint32_t *)pValue);
            break;

        case VOLUME_PARAM_ENABLESTEREOPOSITION:
            if (*pValueSize < sizeof(int32_t)) {
                ALOGV("%s VOLUME_PARAM_ENABLESTEREOPOSITION invalid *pValueSize %u",
                        __func__, *pValueSize);
                status = -EINVAL;
                break;
            }
            *pValueSize = sizeof(int32_t);

            *(int32_t *)pValue = pContext->pBundledContext->bStereoPositionEnabled;
            ALOGVV("%s VOLUME_PARAM_ENABLESTEREOPOSITION %d", __func__, *(int32_t *)pValue);

            break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Volume_getParameter */


//----------------------------------------------------------------------------
// Volume_setParameter()
//----------------------------------------------------------------------------
// Purpose:
// Set a Volume parameter
//
// Inputs:
//  pVolume       - handle to instance data
//  pParam        - pointer to parameter
//  pValue        - pointer to value
//
// Outputs:
//
//----------------------------------------------------------------------------

int Volume_setParameter(EffectContext *pContext,
                        uint32_t       paramSize,
                        void          *pParam,
                        uint32_t       valueSize,
                        void          *pValue) {
    int status = 0;
    int32_t *params = (int32_t *)pParam;

    ALOGVV("%s start", __func__);

    if (paramSize < sizeof(int32_t)) {
        ALOGV("%s invalid paramSize: %u", __func__, paramSize);
        return -EINVAL;
    }
    switch (params[0]) {
        case VOLUME_PARAM_LEVEL: {
            if (valueSize < sizeof(int16_t)) {
                ALOGV("%s VOLUME_PARAM_LEVEL invalid valueSize %u", __func__, valueSize);
                status = -EINVAL;
                break;
            }

            const int16_t level = *(int16_t *)pValue;
            ALOGVV("%s VOLUME_PARAM_LEVEL %d", __func__, level);
            ALOGVV("%s VOLUME_PARAM_LEVEL Calling VolumeSetVolumeLevel", __func__);
            status = VolumeSetVolumeLevel(pContext, level);
            ALOGVV("%s VOLUME_PARAM_LEVEL Called VolumeSetVolumeLevel", __func__);
        } break;

        case VOLUME_PARAM_MUTE: {
            if (valueSize < sizeof(uint32_t)) {
                ALOGV("%s VOLUME_PARAM_MUTE invalid valueSize %u", __func__, valueSize);
                android_errorWriteLog(0x534e4554, "64477217");
                status = -EINVAL;
                break;
            }

            const uint32_t mute = *(uint32_t *)pValue;
            ALOGVV("%s VOLUME_PARAM_MUTE %d", __func__, mute);
            ALOGVV("%s VOLUME_PARAM_MUTE Calling VolumeSetMute", __func__);
            status = VolumeSetMute(pContext, mute);
            ALOGVV("%s VOLUME_PARAM_MUTE Called VolumeSetMute", __func__);
        } break;

        case VOLUME_PARAM_ENABLESTEREOPOSITION: {
            if (valueSize < sizeof(uint32_t)) {
                ALOGV("%s VOLUME_PARAM_ENABLESTEREOPOSITION invalid valueSize %u",
                        __func__, valueSize);
                status = -EINVAL;
                break;
            }

            const uint32_t positionEnabled = *(uint32_t *)pValue;
            status = VolumeEnableStereoPosition(pContext, positionEnabled)
                    ?: VolumeSetStereoPosition(pContext, pContext->pBundledContext->positionSaved);
            ALOGVV("%s VOLUME_PARAM_ENABLESTEREOPOSITION called", __func__);
        } break;

        case VOLUME_PARAM_STEREOPOSITION: {
            if (valueSize < sizeof(int16_t)) {
                ALOGV("%s VOLUME_PARAM_STEREOPOSITION invalid valueSize %u", __func__, valueSize);
                status = -EINVAL;
                break;
            }

            const int16_t position = *(int16_t *)pValue;
            ALOGVV("%s VOLUME_PARAM_STEREOPOSITION %d", __func__, position);
            ALOGVV("%s VOLUME_PARAM_STEREOPOSITION Calling VolumeSetStereoPosition",
                    __func__);
            status = VolumeSetStereoPosition(pContext, position);
            ALOGVV("%s VOLUME_PARAM_STEREOPOSITION Called VolumeSetStereoPosition",
                    __func__);
        } break;

        default:
            ALOGV("%s invalid param %d", __func__, params[0]);
            status = -EINVAL;
            break;
    }

    ALOGVV("%s end param: %d, status: %d", __func__, params[0], status);
    return status;
} /* end Volume_setParameter */

/****************************************************************************************
 * Name : LVC_ToDB_s32Tos16()
 *  Input       : Signed 32-bit integer
 *  Output      : Signed 16-bit integer
 *                  MSB (16) = sign bit
 *                  (15->05) = integer part
 *                  (04->01) = decimal part
 *  Returns     : Db value with respect to full scale
 *  Description :
 *  Remarks     :
 ****************************************************************************************/

LVM_INT16 LVC_ToDB_s32Tos16(LVM_INT32 Lin_fix)
{
    LVM_INT16   db_fix;
    LVM_INT16   Shift;
    LVM_INT16   SmallRemainder;
    LVM_UINT32  Remainder = (LVM_UINT32)Lin_fix;

    /* Count leading bits, 1 cycle in assembly*/
    for (Shift = 0; Shift<32; Shift++)
    {
        if ((Remainder & 0x80000000U)!=0)
        {
            break;
        }
        Remainder = Remainder << 1;
    }

    /*
     * Based on the approximation equation (for Q11.4 format):
     *
     * dB = -96 * Shift + 16 * (8 * Remainder - 2 * Remainder^2)
     */
    db_fix    = (LVM_INT16)(-96 * Shift);               /* Six dB steps in Q11.4 format*/
    SmallRemainder = (LVM_INT16)((Remainder & 0x7fffffff) >> 24);
    db_fix = (LVM_INT16)(db_fix + SmallRemainder );
    SmallRemainder = (LVM_INT16)(SmallRemainder * SmallRemainder);
    db_fix = (LVM_INT16)(db_fix - (LVM_INT16)((LVM_UINT16)SmallRemainder >> 9));

    /* Correct for small offset */
    db_fix = (LVM_INT16)(db_fix - 5);

    return db_fix;
}

//----------------------------------------------------------------------------
// Effect_setEnabled()
//----------------------------------------------------------------------------
// Purpose:
// Enable or disable effect
//
// Inputs:
//  pContext      - pointer to effect context
//  enabled       - true if enabling the effect, false otherwise
//
// Outputs:
//
//----------------------------------------------------------------------------

int Effect_setEnabled(EffectContext *pContext, bool enabled)
{
    ALOGV("%s effectType %d, enabled %d, currently enabled %d", __func__,
            pContext->EffectType, enabled, pContext->pBundledContext->NumberEffectsEnabled);
    int &effectInDrain = pContext->pBundledContext->effectInDrain;
    if (enabled) {
        // Bass boost or Virtualizer can be temporarily disabled if playing over device speaker due
        // to their nature.
        bool tempDisabled = false;
        switch (pContext->EffectType) {
            case LVM_BASS_BOOST:
                if (pContext->pBundledContext->bBassEnabled == LVM_TRUE) {
                     ALOGV("\tEffect_setEnabled() LVM_BASS_BOOST is already enabled");
                     return -EINVAL;
                }
                if(pContext->pBundledContext->SamplesToExitCountBb <= 0){
                    pContext->pBundledContext->NumberEffectsEnabled++;
                }
                effectInDrain &= ~(1 << LVM_BASS_BOOST);
                pContext->pBundledContext->SamplesToExitCountBb =
                     (LVM_INT32)(pContext->pBundledContext->SamplesPerSecond*0.1);
                pContext->pBundledContext->bBassEnabled = LVM_TRUE;
                tempDisabled = pContext->pBundledContext->bBassTempDisabled;
                break;
            case LVM_EQUALIZER:
                if (pContext->pBundledContext->bEqualizerEnabled == LVM_TRUE) {
                    ALOGV("\tEffect_setEnabled() LVM_EQUALIZER is already enabled");
                    return -EINVAL;
                }
                if(pContext->pBundledContext->SamplesToExitCountEq <= 0){
                    pContext->pBundledContext->NumberEffectsEnabled++;
                }
                effectInDrain &= ~(1 << LVM_EQUALIZER);
                pContext->pBundledContext->SamplesToExitCountEq =
                     (LVM_INT32)(pContext->pBundledContext->SamplesPerSecond*0.1);
                pContext->pBundledContext->bEqualizerEnabled = LVM_TRUE;
                break;
            case LVM_VIRTUALIZER:
                if (pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE) {
                    ALOGV("\tEffect_setEnabled() LVM_VIRTUALIZER is already enabled");
                    return -EINVAL;
                }
                if(pContext->pBundledContext->SamplesToExitCountVirt <= 0){
                    pContext->pBundledContext->NumberEffectsEnabled++;
                }
                effectInDrain &= ~(1 << LVM_VIRTUALIZER);
                pContext->pBundledContext->SamplesToExitCountVirt =
                     (LVM_INT32)(pContext->pBundledContext->SamplesPerSecond*0.1);
                pContext->pBundledContext->bVirtualizerEnabled = LVM_TRUE;
                tempDisabled = pContext->pBundledContext->bVirtualizerTempDisabled;
                break;
            case LVM_VOLUME:
                if (pContext->pBundledContext->bVolumeEnabled == LVM_TRUE) {
                    ALOGV("\tEffect_setEnabled() LVM_VOLUME is already enabled");
                    return -EINVAL;
                }
                if ((effectInDrain & 1 << LVM_VOLUME) == 0) {
                    pContext->pBundledContext->NumberEffectsEnabled++;
                }
                effectInDrain &= ~(1 << LVM_VOLUME);
                pContext->pBundledContext->bVolumeEnabled = LVM_TRUE;
                break;
            default:
                ALOGV("\tEffect_setEnabled() invalid effect type");
                return -EINVAL;
        }
        if (!tempDisabled) {
            LvmEffect_enable(pContext);
        }
    } else {
        switch (pContext->EffectType) {
            case LVM_BASS_BOOST:
                if (pContext->pBundledContext->bBassEnabled == LVM_FALSE) {
                    ALOGV("\tEffect_setEnabled() LVM_BASS_BOOST is already disabled");
                    return -EINVAL;
                }
                pContext->pBundledContext->bBassEnabled = LVM_FALSE;
                effectInDrain |= 1 << LVM_BASS_BOOST;
                break;
            case LVM_EQUALIZER:
                if (pContext->pBundledContext->bEqualizerEnabled == LVM_FALSE) {
                    ALOGV("\tEffect_setEnabled() LVM_EQUALIZER is already disabled");
                    return -EINVAL;
                }
                pContext->pBundledContext->bEqualizerEnabled = LVM_FALSE;
                effectInDrain |= 1 << LVM_EQUALIZER;
                break;
            case LVM_VIRTUALIZER:
                if (pContext->pBundledContext->bVirtualizerEnabled == LVM_FALSE) {
                    ALOGV("\tEffect_setEnabled() LVM_VIRTUALIZER is already disabled");
                    return -EINVAL;
                }
                pContext->pBundledContext->bVirtualizerEnabled = LVM_FALSE;
                effectInDrain |= 1 << LVM_VIRTUALIZER;
                break;
            case LVM_VOLUME:
                if (pContext->pBundledContext->bVolumeEnabled == LVM_FALSE) {
                    ALOGV("\tEffect_setEnabled() LVM_VOLUME is already disabled");
                    return -EINVAL;
                }
                pContext->pBundledContext->bVolumeEnabled = LVM_FALSE;
                effectInDrain |= 1 << LVM_VOLUME;
                break;
            default:
                ALOGV("\tEffect_setEnabled() invalid effect type");
                return -EINVAL;
        }
        LvmEffect_disable(pContext);
    }

    return 0;
}

//----------------------------------------------------------------------------
// LVC_Convert_VolToDb()
//----------------------------------------------------------------------------
// Purpose:
// Convery volume in Q24 to dB
//
// Inputs:
//  vol:   Q.24 volume dB
//
//-----------------------------------------------------------------------

int16_t LVC_Convert_VolToDb(uint32_t vol){
    int16_t  dB;

    dB = LVC_ToDB_s32Tos16(vol <<7);
    dB = (dB +8)>>4;
    dB = (dB <-96) ? -96 : dB ;

    return dB;
}

} // namespace
} // namespace

extern "C" {
/* Effect Control Interface Implementation: Process */
int Effect_process(effect_handle_t     self,
                              audio_buffer_t         *inBuffer,
                              audio_buffer_t         *outBuffer){
    EffectContext * pContext = (EffectContext *) self;
    int    status = 0;
    int    processStatus = 0;
    const int NrChannels = audio_channel_count_from_out_mask(pContext->config.inputCfg.channels);

//ALOGV("\tEffect_process Start : Enabled = %d     Called = %d (%8d %8d %8d)",
//pContext->pBundledContext->NumberEffectsEnabled,pContext->pBundledContext->NumberEffectsCalled,
//    pContext->pBundledContext->SamplesToExitCountBb,
//    pContext->pBundledContext->SamplesToExitCountVirt,
//    pContext->pBundledContext->SamplesToExitCountEq);

    if (pContext == NULL){
        ALOGV("\tLVM_ERROR : Effect_process() ERROR pContext == NULL");
        return -EINVAL;
    }

    //if(pContext->EffectType == LVM_BASS_BOOST){
    //  ALOGV("\tEffect_process: Effect type is BASS_BOOST");
    //}else if(pContext->EffectType == LVM_EQUALIZER){
    //  ALOGV("\tEffect_process: Effect type is LVM_EQUALIZER");
    //}else if(pContext->EffectType == LVM_VIRTUALIZER){
    //  ALOGV("\tEffect_process: Effect type is LVM_VIRTUALIZER");
    //}

    if (inBuffer == NULL  || inBuffer->raw == NULL  ||
            outBuffer == NULL || outBuffer->raw == NULL ||
            inBuffer->frameCount != outBuffer->frameCount){
        ALOGV("\tLVM_ERROR : Effect_process() ERROR NULL INPUT POINTER OR FRAME COUNT IS WRONG");
        return -EINVAL;
    }

    int &effectProcessCalled = pContext->pBundledContext->effectProcessCalled;
    int &effectInDrain = pContext->pBundledContext->effectInDrain;
    if ((effectProcessCalled & 1 << pContext->EffectType) != 0) {
        ALOGW("Effect %d already called", pContext->EffectType);
        const int undrainedEffects = effectInDrain & ~effectProcessCalled;
        if ((undrainedEffects & 1 << LVM_BASS_BOOST) != 0) {
            ALOGW("Draining BASS_BOOST");
            pContext->pBundledContext->SamplesToExitCountBb = 0;
            --pContext->pBundledContext->NumberEffectsEnabled;
            effectInDrain &= ~(1 << LVM_BASS_BOOST);
        }
        if ((undrainedEffects & 1 << LVM_EQUALIZER) != 0) {
            ALOGW("Draining EQUALIZER");
            pContext->pBundledContext->SamplesToExitCountEq = 0;
            --pContext->pBundledContext->NumberEffectsEnabled;
            effectInDrain &= ~(1 << LVM_EQUALIZER);
        }
        if ((undrainedEffects & 1 << LVM_VIRTUALIZER) != 0) {
            ALOGW("Draining VIRTUALIZER");
            pContext->pBundledContext->SamplesToExitCountVirt = 0;
            --pContext->pBundledContext->NumberEffectsEnabled;
            effectInDrain &= ~(1 << LVM_VIRTUALIZER);
        }
        if ((undrainedEffects & 1 << LVM_VOLUME) != 0) {
            ALOGW("Draining VOLUME");
            --pContext->pBundledContext->NumberEffectsEnabled;
            effectInDrain &= ~(1 << LVM_VOLUME);
        }
    }
    effectProcessCalled |= 1 << pContext->EffectType;

    if ((pContext->pBundledContext->bBassEnabled == LVM_FALSE)&&
        (pContext->EffectType == LVM_BASS_BOOST)){
        //ALOGV("\tEffect_process() LVM_BASS_BOOST Effect is not enabled");
        if(pContext->pBundledContext->SamplesToExitCountBb > 0){
            pContext->pBundledContext->SamplesToExitCountBb -= outBuffer->frameCount * NrChannels;
            //ALOGV("\tEffect_process: Waiting to turn off BASS_BOOST, %d samples left",
            //    pContext->pBundledContext->SamplesToExitCountBb);
        }
        if (pContext->pBundledContext->SamplesToExitCountBb <= 0) {
            status = -ENODATA;
            if ((effectInDrain & 1 << LVM_BASS_BOOST) != 0) {
                pContext->pBundledContext->NumberEffectsEnabled--;
                effectInDrain &= ~(1 << LVM_BASS_BOOST);
            }
            ALOGV("\tEffect_process() this is the last frame for LVM_BASS_BOOST");
        }
    }
    if ((pContext->pBundledContext->bVolumeEnabled == LVM_FALSE)&&
        (pContext->EffectType == LVM_VOLUME)){
        //ALOGV("\tEffect_process() LVM_VOLUME Effect is not enabled");
        status = -ENODATA;
        if ((effectInDrain & 1 << LVM_VOLUME) != 0) {
            pContext->pBundledContext->NumberEffectsEnabled--;
            effectInDrain &= ~(1 << LVM_VOLUME);
        }
    }
    if ((pContext->pBundledContext->bEqualizerEnabled == LVM_FALSE)&&
        (pContext->EffectType == LVM_EQUALIZER)){
        //ALOGV("\tEffect_process() LVM_EQUALIZER Effect is not enabled");
        if(pContext->pBundledContext->SamplesToExitCountEq > 0){
            pContext->pBundledContext->SamplesToExitCountEq -= outBuffer->frameCount * NrChannels;
            //ALOGV("\tEffect_process: Waiting to turn off EQUALIZER, %d samples left",
            //    pContext->pBundledContext->SamplesToExitCountEq);
        }
        if (pContext->pBundledContext->SamplesToExitCountEq <= 0) {
            status = -ENODATA;
            if ((effectInDrain & 1 << LVM_EQUALIZER) != 0) {
                pContext->pBundledContext->NumberEffectsEnabled--;
                effectInDrain &= ~(1 << LVM_EQUALIZER);
            }
            ALOGV("\tEffect_process() this is the last frame for LVM_EQUALIZER");
        }
    }
    if ((pContext->pBundledContext->bVirtualizerEnabled == LVM_FALSE)&&
        (pContext->EffectType == LVM_VIRTUALIZER)){
        //ALOGV("\tEffect_process() LVM_VIRTUALIZER Effect is not enabled");
        if(pContext->pBundledContext->SamplesToExitCountVirt > 0){
            pContext->pBundledContext->SamplesToExitCountVirt -=
                outBuffer->frameCount * NrChannels;
            //ALOGV("\tEffect_process: Waiting for to turn off VIRTUALIZER, %d samples left",
            //    pContext->pBundledContext->SamplesToExitCountVirt);
        }
        if (pContext->pBundledContext->SamplesToExitCountVirt <= 0) {
            status = -ENODATA;
            if ((effectInDrain & 1 << LVM_VIRTUALIZER) != 0) {
                pContext->pBundledContext->NumberEffectsEnabled--;
                effectInDrain &= ~(1 << LVM_VIRTUALIZER);
            }
            ALOGV("\tEffect_process() this is the last frame for LVM_VIRTUALIZER");
        }
    }

    if(status != -ENODATA){
        pContext->pBundledContext->NumberEffectsCalled++;
    }

    if (pContext->pBundledContext->NumberEffectsCalled >=
            pContext->pBundledContext->NumberEffectsEnabled) {

        // We expect the # effects called to be equal to # effects enabled in sequence (including
        // draining effects).  Warn if this is not the case due to inconsistent calls.
        ALOGW_IF(pContext->pBundledContext->NumberEffectsCalled >
                pContext->pBundledContext->NumberEffectsEnabled,
                "%s Number of effects called %d is greater than number of effects enabled %d",
                __func__, pContext->pBundledContext->NumberEffectsCalled,
                pContext->pBundledContext->NumberEffectsEnabled);
        effectProcessCalled = 0; // reset our consistency check.

        //ALOGV("\tEffect_process     Calling process with %d effects enabled, %d called: Effect %d",
        //pContext->pBundledContext->NumberEffectsEnabled,
        //pContext->pBundledContext->NumberEffectsCalled, pContext->EffectType);

        if (status == -ENODATA){
            ALOGV("\tEffect_process() processing last frame");
        }
        pContext->pBundledContext->NumberEffectsCalled = 0;
        /* Process all the available frames, block processing is
           handled internalLY by the LVM bundle */
#ifdef NATIVE_FLOAT_BUFFER
        processStatus = android::LvmBundle_process(inBuffer->f32,
                                                   outBuffer->f32,
                                                   outBuffer->frameCount,
                                                   pContext);
#else
        processStatus = android::LvmBundle_process(inBuffer->s16,
                                                   outBuffer->s16,
                                                   outBuffer->frameCount,
                                                   pContext);
#endif
        if (processStatus != 0){
            ALOGV("\tLVM_ERROR : LvmBundle_process returned error %d", processStatus);
            if (status == 0) {
                status = processStatus;
            }
            return status;
        }
    } else {
        //ALOGV("\tEffect_process Not Calling process with %d effects enabled, %d called: Effect %d",
        //pContext->pBundledContext->NumberEffectsEnabled,
        //pContext->pBundledContext->NumberEffectsCalled, pContext->EffectType);

        if (pContext->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
            for (size_t i = 0; i < outBuffer->frameCount * NrChannels; ++i) {
#ifdef NATIVE_FLOAT_BUFFER
                outBuffer->f32[i] += inBuffer->f32[i];
#else
                outBuffer->s16[i] = clamp16((LVM_INT32)outBuffer->s16[i] + inBuffer->s16[i]);
#endif
            }
        } else if (outBuffer->raw != inBuffer->raw) {
            memcpy(outBuffer->raw,
                    inBuffer->raw,
                    outBuffer->frameCount * sizeof(effect_buffer_t) * FCC_2);
        }
    }

    return status;
}   /* end Effect_process */

// The value offset of an effect parameter is computed by rounding up
// the parameter size to the next 32 bit alignment.
static inline uint32_t computeParamVOffset(const effect_param_t *p) {
    return ((p->psize + sizeof(int32_t) - 1) / sizeof(int32_t)) *
            sizeof(int32_t);
}

/* Effect Control Interface Implementation: Command */
int Effect_command(effect_handle_t  self,
                              uint32_t            cmdCode,
                              uint32_t            cmdSize,
                              void                *pCmdData,
                              uint32_t            *replySize,
                              void                *pReplyData){
    EffectContext * pContext = (EffectContext *) self;

    //ALOGV("\t\nEffect_command start");

    if(pContext->EffectType == LVM_BASS_BOOST){
        //ALOGV("\tEffect_command setting command for LVM_BASS_BOOST");
    }
    if(pContext->EffectType == LVM_VIRTUALIZER){
        //ALOGV("\tEffect_command setting command for LVM_VIRTUALIZER");
    }
    if(pContext->EffectType == LVM_EQUALIZER){
        //ALOGV("\tEffect_command setting command for LVM_EQUALIZER");
    }
    if(pContext->EffectType == LVM_VOLUME){
        //ALOGV("\tEffect_command setting command for LVM_VOLUME");
    }

    if (pContext == NULL){
        ALOGV("\tLVM_ERROR : Effect_command ERROR pContext == NULL");
        return -EINVAL;
    }

    //ALOGV("\tEffect_command INPUTS are: command %d cmdSize %d",cmdCode, cmdSize);

    // Incase we disable an effect, next time process is
    // called the number of effect called could be greater
    // pContext->pBundledContext->NumberEffectsCalled = 0;

    //ALOGV("\tEffect_command NumberEffectsCalled = %d, NumberEffectsEnabled = %d",
    //        pContext->pBundledContext->NumberEffectsCalled,
    //        pContext->pBundledContext->NumberEffectsEnabled);

    switch (cmdCode){
        case EFFECT_CMD_INIT:
            if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)){
                ALOGV("\tLVM_ERROR, EFFECT_CMD_INIT: ERROR for effect type %d",
                        pContext->EffectType);
                return -EINVAL;
            }
            *(int *) pReplyData = 0;
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_INIT start");
            if(pContext->EffectType == LVM_BASS_BOOST){
                //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_INIT for LVM_BASS_BOOST");
                android::BassSetStrength(pContext, 0);
            }
            if(pContext->EffectType == LVM_VIRTUALIZER){
                //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_INIT for LVM_VIRTUALIZER");
                android::VirtualizerSetStrength(pContext, 0);
            }
            if(pContext->EffectType == LVM_EQUALIZER){
                //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_INIT for LVM_EQUALIZER");
                android::EqualizerSetPreset(pContext, 0);
            }
            if(pContext->EffectType == LVM_VOLUME){
                //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_INIT for LVM_VOLUME");
                *(int *) pReplyData = android::VolumeSetVolumeLevel(pContext, 0);
            }
            break;

        case EFFECT_CMD_SET_CONFIG:
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_CONFIG start");
            if (pCmdData    == NULL || cmdSize     != sizeof(effect_config_t) ||
                    pReplyData  == NULL || replySize == NULL || *replySize  != sizeof(int)) {
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: "
                        "EFFECT_CMD_SET_CONFIG: ERROR");
                return -EINVAL;
            }
            *(int *) pReplyData = android::Effect_setConfig(pContext, (effect_config_t *) pCmdData);
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_CONFIG end");
            break;

        case EFFECT_CMD_GET_CONFIG:
            if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(effect_config_t)) {
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: "
                        "EFFECT_CMD_GET_CONFIG: ERROR");
                return -EINVAL;
            }

            android::Effect_getConfig(pContext, (effect_config_t *)pReplyData);
            break;

        case EFFECT_CMD_RESET:
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_RESET start");
            android::Effect_setConfig(pContext, &pContext->config);
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_RESET end");
            break;

        case EFFECT_CMD_GET_PARAM:{
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_GET_PARAM start");

            effect_param_t *p = (effect_param_t *)pCmdData;
            if (pCmdData == NULL || cmdSize < sizeof(effect_param_t) ||
                    cmdSize < (sizeof(effect_param_t) + p->psize) ||
                    pReplyData == NULL || replySize == NULL ||
                    *replySize < (sizeof(effect_param_t) + p->psize)) {
                ALOGV("\tLVM_ERROR : EFFECT_CMD_GET_PARAM: ERROR");
                return -EINVAL;
            }
            if (EFFECT_PARAM_SIZE_MAX - sizeof(effect_param_t) < (size_t)p->psize) {
                android_errorWriteLog(0x534e4554, "26347509");
                ALOGV("\tLVM_ERROR : EFFECT_CMD_GET_PARAM: psize too big");
                return -EINVAL;
            }
            const uint32_t paddedParamSize = computeParamVOffset(p);
            if ((EFFECT_PARAM_SIZE_MAX - sizeof(effect_param_t) < paddedParamSize) ||
                (EFFECT_PARAM_SIZE_MAX - sizeof(effect_param_t) - paddedParamSize <
                    p->vsize)) {
                ALOGV("\tLVM_ERROR : EFFECT_CMD_GET_PARAM: padded_psize or vsize too big");
                return -EINVAL;
            }
            uint32_t expectedReplySize = sizeof(effect_param_t) + paddedParamSize + p->vsize;
            if (*replySize < expectedReplySize) {
                ALOGV("\tLVM_ERROR : EFFECT_CMD_GET_PARAM: min. replySize %u, got %u bytes",
                        expectedReplySize, *replySize);
                android_errorWriteLog(0x534e4554, "32705438");
                return -EINVAL;
            }

            memcpy(pReplyData, pCmdData, sizeof(effect_param_t) + p->psize);

            p = (effect_param_t *)pReplyData;

            uint32_t voffset = paddedParamSize;
            if(pContext->EffectType == LVM_BASS_BOOST){
                p->status = android::BassBoost_getParameter(pContext,
                                                            p->psize,
                                                            p->data,
                                                            &p->vsize,
                                                            p->data + voffset);
                //ALOGV("\tBassBoost_command EFFECT_CMD_GET_PARAM "
                //        "*pCmdData %d, *replySize %d, *pReplyData %d ",
                //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
                //        *replySize,
                //        *(int16_t *)((char *)pReplyData + sizeof(effect_param_t) + voffset));
            }

            if(pContext->EffectType == LVM_VIRTUALIZER){
                p->status = android::Virtualizer_getParameter(pContext,
                                                              p->psize,
                                                              (void *)p->data,
                                                              &p->vsize,
                                                              p->data + voffset);

                //ALOGV("\tVirtualizer_command EFFECT_CMD_GET_PARAM "
                //        "*pCmdData %d, *replySize %d, *pReplyData %d ",
                //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
                //        *replySize,
                //        *(int16_t *)((char *)pReplyData + sizeof(effect_param_t) + voffset));
            }
            if(pContext->EffectType == LVM_EQUALIZER){
                //ALOGV("\tEqualizer_command cmdCode Case: "
                //        "EFFECT_CMD_GET_PARAM start");
                p->status = android::Equalizer_getParameter(pContext,
                                                            p->psize,
                                                            p->data,
                                                            &p->vsize,
                                                            p->data + voffset);

                //ALOGV("\tEqualizer_command EFFECT_CMD_GET_PARAM *pCmdData %d, *replySize %d, "
                //       "*pReplyData %08x %08x",
                //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)), *replySize,
                //        *(int32_t *)((char *)pReplyData + sizeof(effect_param_t) + voffset),
                //        *(int32_t *)((char *)pReplyData + sizeof(effect_param_t) + voffset +
                //        sizeof(int32_t)));
            }
            if(pContext->EffectType == LVM_VOLUME){
                //ALOGV("\tVolume_command cmdCode Case: EFFECT_CMD_GET_PARAM start");
                p->status = android::Volume_getParameter(pContext,
                                                         p->psize,
                                                         (void *)p->data,
                                                         &p->vsize,
                                                         p->data + voffset);

                //ALOGV("\tVolume_command EFFECT_CMD_GET_PARAM "
                //        "*pCmdData %d, *replySize %d, *pReplyData %d ",
                //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
                //        *replySize,
                //        *(int16_t *)((char *)pReplyData + sizeof(effect_param_t) + voffset));
            }
            *replySize = sizeof(effect_param_t) + voffset + p->vsize;

            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_GET_PARAM end");
        } break;
        case EFFECT_CMD_SET_PARAM:{
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_PARAM start");
            if(pContext->EffectType == LVM_BASS_BOOST){
                //ALOGV("\tBassBoost_command EFFECT_CMD_SET_PARAM param %d, *replySize %d, value %d",
                //       *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
                //       *replySize,
                //       *(int16_t *)((char *)pCmdData + sizeof(effect_param_t) + sizeof(int32_t)));

                if (pCmdData   == NULL ||
                        cmdSize    != (sizeof(effect_param_t) + sizeof(int32_t) +sizeof(int16_t)) ||
                        pReplyData == NULL || replySize == NULL || *replySize != sizeof(int32_t)) {
                    ALOGV("\tLVM_ERROR : BassBoost_command cmdCode Case: "
                            "EFFECT_CMD_SET_PARAM: ERROR");
                    return -EINVAL;
                }

                effect_param_t * const p = (effect_param_t *) pCmdData;
                const uint32_t voffset = computeParamVOffset(p);

                //ALOGV("\tnBassBoost_command cmdSize is %d\n"
                //        "\tsizeof(effect_param_t) is  %d\n"
                //        "\tp->psize is %d\n"
                //        "\tp->vsize is %d"
                //        "\n",
                //        cmdSize, sizeof(effect_param_t), p->psize, p->vsize );

                *(int *)pReplyData = android::BassBoost_setParameter(pContext,
                                                                     p->psize,
                                                                     (void *)p->data,
                                                                     p->vsize,
                                                                     p->data + voffset);
            }
            if(pContext->EffectType == LVM_VIRTUALIZER){
              // Warning this log will fail to properly read an int32_t value, assumes int16_t
              //ALOGV("\tVirtualizer_command EFFECT_CMD_SET_PARAM param %d, *replySize %d, value %d",
              //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
              //        *replySize,
              //        *(int16_t *)((char *)pCmdData + sizeof(effect_param_t) + sizeof(int32_t)));

                if (pCmdData   == NULL ||
                        // legal parameters are int16_t or int32_t
                        cmdSize    > (sizeof(effect_param_t) + sizeof(int32_t) +sizeof(int32_t)) ||
                        cmdSize    < (sizeof(effect_param_t) + sizeof(int32_t) +sizeof(int16_t)) ||
                        pReplyData == NULL || replySize == NULL || *replySize != sizeof(int32_t)) {
                    ALOGV("\tLVM_ERROR : Virtualizer_command cmdCode Case: "
                            "EFFECT_CMD_SET_PARAM: ERROR");
                    return -EINVAL;
                }

                effect_param_t * const p = (effect_param_t *) pCmdData;
                const uint32_t voffset = computeParamVOffset(p);

                //ALOGV("\tnVirtualizer_command cmdSize is %d\n"
                //        "\tsizeof(effect_param_t) is  %d\n"
                //        "\tp->psize is %d\n"
                //        "\tp->vsize is %d"
                //        "\n",
                //        cmdSize, sizeof(effect_param_t), p->psize, p->vsize );

                *(int *)pReplyData = android::Virtualizer_setParameter(pContext,
                                                                       p->psize,
                                                                       (void *)p->data,
                                                                       p->vsize,
                                                                       p->data + voffset);
            }
            if(pContext->EffectType == LVM_EQUALIZER){
               //ALOGV("\tEqualizer_command cmdCode Case: "
               //        "EFFECT_CMD_SET_PARAM start");
               //ALOGV("\tEqualizer_command EFFECT_CMD_SET_PARAM param %d, *replySize %d, value %d ",
               //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
               //        *replySize,
               //        *(int16_t *)((char *)pCmdData + sizeof(effect_param_t) + sizeof(int32_t)));

                if (pCmdData == NULL || cmdSize < (sizeof(effect_param_t) + sizeof(int32_t)) ||
                        pReplyData == NULL || replySize == NULL || *replySize != sizeof(int32_t)) {
                    ALOGV("\tLVM_ERROR : Equalizer_command cmdCode Case: "
                            "EFFECT_CMD_SET_PARAM: ERROR");
                    return -EINVAL;
                }

                effect_param_t * const p = (effect_param_t *) pCmdData;
                const uint32_t voffset = computeParamVOffset(p);

                *(int *)pReplyData = android::Equalizer_setParameter(pContext,
                                                                     p->psize,
                                                                     (void *)p->data,
                                                                     p->vsize,
                                                                     p->data + voffset);
            }
            if(pContext->EffectType == LVM_VOLUME){
                //ALOGV("\tVolume_command cmdCode Case: EFFECT_CMD_SET_PARAM start");
                //ALOGV("\tVolume_command EFFECT_CMD_SET_PARAM param %d, *replySize %d, value %d ",
                //        *(int32_t *)((char *)pCmdData + sizeof(effect_param_t)),
                //        *replySize,
                //        *(int16_t *)((char *)pCmdData + sizeof(effect_param_t) +sizeof(int32_t)));

                if (pCmdData   == NULL ||
                        cmdSize    < (sizeof(effect_param_t) + sizeof(int32_t)) ||
                        pReplyData == NULL || replySize == NULL ||
                        *replySize != sizeof(int32_t)) {
                    ALOGV("\tLVM_ERROR : Volume_command cmdCode Case: "
                            "EFFECT_CMD_SET_PARAM: ERROR");
                    return -EINVAL;
                }

                effect_param_t * const p = (effect_param_t *) pCmdData;
                const uint32_t voffset = computeParamVOffset(p);

                *(int *)pReplyData = android::Volume_setParameter(pContext,
                                                                  p->psize,
                                                                  (void *)p->data,
                                                                  p->vsize,
                                                                  p->data + voffset);
            }
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_PARAM end");
        } break;

        case EFFECT_CMD_ENABLE:
            ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_ENABLE start");
            if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: EFFECT_CMD_ENABLE: ERROR");
                return -EINVAL;
            }

            *(int *)pReplyData = android::Effect_setEnabled(pContext, LVM_TRUE);
            break;

        case EFFECT_CMD_DISABLE:
            //ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_DISABLE start");
            if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: EFFECT_CMD_DISABLE: ERROR");
                return -EINVAL;
            }
            *(int *)pReplyData = android::Effect_setEnabled(pContext, LVM_FALSE);
            break;

        case EFFECT_CMD_SET_DEVICE:
        {
            ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_DEVICE start");
            if (pCmdData   == NULL){
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: EFFECT_CMD_SET_DEVICE: ERROR");
                return -EINVAL;
            }

            uint32_t device = *(uint32_t *)pCmdData;
            pContext->pBundledContext->nOutputDevice = (audio_devices_t) device;

            if (pContext->EffectType == LVM_BASS_BOOST) {
                if((device == AUDIO_DEVICE_OUT_SPEAKER) ||
                        (device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) ||
                        (device == AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER)){
                    ALOGV("\tEFFECT_CMD_SET_DEVICE device is invalid for LVM_BASS_BOOST %d",
                          *(int32_t *)pCmdData);
                    ALOGV("\tEFFECT_CMD_SET_DEVICE temporary disable LVM_BAS_BOOST");

                    // If a device doesnt support bassboost the effect must be temporarily disabled
                    // the effect must still report its original state as this can only be changed
                    // by the ENABLE/DISABLE command

                    if (pContext->pBundledContext->bBassEnabled == LVM_TRUE) {
                        ALOGV("\tEFFECT_CMD_SET_DEVICE disable LVM_BASS_BOOST %d",
                             *(int32_t *)pCmdData);
                        android::LvmEffect_disable(pContext);
                    }
                    pContext->pBundledContext->bBassTempDisabled = LVM_TRUE;
                } else {
                    ALOGV("\tEFFECT_CMD_SET_DEVICE device is valid for LVM_BASS_BOOST %d",
                         *(int32_t *)pCmdData);

                    // If a device supports bassboost and the effect has been temporarily disabled
                    // previously then re-enable it

                    if (pContext->pBundledContext->bBassEnabled == LVM_TRUE) {
                        ALOGV("\tEFFECT_CMD_SET_DEVICE re-enable LVM_BASS_BOOST %d",
                             *(int32_t *)pCmdData);
                        android::LvmEffect_enable(pContext);
                    }
                    pContext->pBundledContext->bBassTempDisabled = LVM_FALSE;
                }
            }
            if (pContext->EffectType == LVM_VIRTUALIZER) {
                if (pContext->pBundledContext->nVirtualizerForcedDevice == AUDIO_DEVICE_NONE) {
                    // default case unless configuration is forced
                    if (android::VirtualizerIsDeviceSupported(device) != 0) {
                        ALOGV("\tEFFECT_CMD_SET_DEVICE device is invalid for LVM_VIRTUALIZER %d",
                                *(int32_t *)pCmdData);
                        ALOGV("\tEFFECT_CMD_SET_DEVICE temporary disable LVM_VIRTUALIZER");

                        //If a device doesnt support virtualizer the effect must be temporarily
                        // disabled the effect must still report its original state as this can
                        // only be changed by the ENABLE/DISABLE command

                        if (pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE) {
                            ALOGV("\tEFFECT_CMD_SET_DEVICE disable LVM_VIRTUALIZER %d",
                                    *(int32_t *)pCmdData);
                            android::LvmEffect_disable(pContext);
                        }
                        pContext->pBundledContext->bVirtualizerTempDisabled = LVM_TRUE;
                    } else {
                        ALOGV("\tEFFECT_CMD_SET_DEVICE device is valid for LVM_VIRTUALIZER %d",
                                *(int32_t *)pCmdData);

                        // If a device supports virtualizer and the effect has been temporarily
                        // disabled previously then re-enable it

                        if(pContext->pBundledContext->bVirtualizerEnabled == LVM_TRUE){
                            ALOGV("\tEFFECT_CMD_SET_DEVICE re-enable LVM_VIRTUALIZER %d",
                                    *(int32_t *)pCmdData);
                            android::LvmEffect_enable(pContext);
                        }
                        pContext->pBundledContext->bVirtualizerTempDisabled = LVM_FALSE;
                    }
                } // else virtualization mode is forced to a certain device, nothing to do
            }
            ALOGV("\tEffect_command cmdCode Case: EFFECT_CMD_SET_DEVICE end");
            break;
        }
        case EFFECT_CMD_SET_VOLUME:
        {
            uint32_t leftVolume, rightVolume;
            int16_t  leftdB, rightdB;
            int16_t  maxdB, pandB;
            int32_t  vol_ret[2] = {1<<24,1<<24}; // Apply no volume
            LVM_ControlParams_t     ActiveParams;           /* Current control Parameters */
            LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS;  /* Function call status */

            // if pReplyData is NULL, VOL_CTRL is delegated to another effect
            if(pReplyData == LVM_NULL){
                break;
            }

            if (pCmdData == NULL || cmdSize != 2 * sizeof(uint32_t) || pReplyData == NULL ||
                    replySize == NULL || *replySize < 2*sizeof(int32_t)) {
                ALOGV("\tLVM_ERROR : Effect_command cmdCode Case: "
                        "EFFECT_CMD_SET_VOLUME: ERROR");
                return -EINVAL;
            }

            leftVolume  = ((*(uint32_t *)pCmdData));
            rightVolume = ((*((uint32_t *)pCmdData + 1)));

            if(leftVolume == 0x1000000){
                leftVolume -= 1;
            }
            if(rightVolume == 0x1000000){
                rightVolume -= 1;
            }

            // Convert volume to dB
            leftdB  = android::LVC_Convert_VolToDb(leftVolume);
            rightdB = android::LVC_Convert_VolToDb(rightVolume);

            pandB = rightdB - leftdB;

            // Calculate max volume in dB
            maxdB = leftdB;
            if(rightdB > maxdB){
                maxdB = rightdB;
            }
            //ALOGV("\tEFFECT_CMD_SET_VOLUME Session: %d, SessionID: %d VOLUME is %d dB, "
            //      "effect is %d",
            //pContext->pBundledContext->SessionNo, pContext->pBundledContext->SessionId,
            //(int32_t)maxdB, pContext->EffectType);
            //ALOGV("\tEFFECT_CMD_SET_VOLUME: Left is %d, Right is %d", leftVolume, rightVolume);
            //ALOGV("\tEFFECT_CMD_SET_VOLUME: Left %ddB, Right %ddB, Position %ddB",
            //        leftdB, rightdB, pandB);

            memcpy(pReplyData, vol_ret, sizeof(int32_t)*2);
            android::VolumeSetVolumeLevel(pContext, (int16_t)(maxdB*100));

            /* Get the current settings */
            LvmStatus =LVM_GetControlParameters(pContext->pBundledContext->hInstance,&ActiveParams);
            LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VolumeSetStereoPosition")
            if(LvmStatus != LVM_SUCCESS) return -EINVAL;

            /* Volume parameters */
            ActiveParams.VC_Balance  = pandB;
            ALOGV("\t\tVolumeSetStereoPosition() (-96dB -> +96dB)-> %d\n", ActiveParams.VC_Balance );

            /* Activate the initial settings */
            LvmStatus =LVM_SetControlParameters(pContext->pBundledContext->hInstance,&ActiveParams);
            LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "VolumeSetStereoPosition")
            if(LvmStatus != LVM_SUCCESS) return -EINVAL;
            break;
         }
        case EFFECT_CMD_SET_AUDIO_MODE:
            break;
        default:
            return -EINVAL;
    }

    //ALOGV("\tEffect_command end...\n\n");
    return 0;
}    /* end Effect_command */

/* Effect Control Interface Implementation: get_descriptor */
int Effect_getDescriptor(effect_handle_t   self,
                                    effect_descriptor_t *pDescriptor)
{
    EffectContext * pContext = (EffectContext *) self;
    const effect_descriptor_t *desc;

    if (pContext == NULL || pDescriptor == NULL) {
        ALOGV("Effect_getDescriptor() invalid param");
        return -EINVAL;
    }

    switch(pContext->EffectType) {
        case LVM_BASS_BOOST:
            desc = &android::gBassBoostDescriptor;
            break;
        case LVM_VIRTUALIZER:
            desc = &android::gVirtualizerDescriptor;
            break;
        case LVM_EQUALIZER:
            desc = &android::gEqualizerDescriptor;
            break;
        case LVM_VOLUME:
            desc = &android::gVolumeDescriptor;
            break;
        default:
            return -EINVAL;
    }

    *pDescriptor = *desc;

    return 0;
}   /* end Effect_getDescriptor */

// effect_handle_t interface implementation for effect
const struct effect_interface_s gLvmEffectInterface = {
    Effect_process,
    Effect_command,
    Effect_getDescriptor,
    NULL,
};    /* end gLvmEffectInterface */

// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
audio_effect_library_t AUDIO_EFFECT_LIBRARY_INFO_SYM = {
    .tag = AUDIO_EFFECT_LIBRARY_TAG,
    .version = EFFECT_LIBRARY_API_VERSION,
    .name = "Effect Bundle Library",
    .implementor = "NXP Software Ltd.",
    .create_effect = android::EffectCreate,
    .release_effect = android::EffectRelease,
    .get_descriptor = android::EffectGetDescriptor,
};

}
