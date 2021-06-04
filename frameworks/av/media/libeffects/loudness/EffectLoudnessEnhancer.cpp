/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_TAG "EffectLE"
//#define LOG_NDEBUG 0

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <new>

#include <log/log.h>

#include <audio_effects/effect_loudnessenhancer.h>
#include "dsp/core/dynamic_range_compression.h"

// BUILD_FLOAT targets building a float effect instead of the legacy int16_t effect.
#define BUILD_FLOAT

#ifdef BUILD_FLOAT

static constexpr audio_format_t kProcessFormat = AUDIO_FORMAT_PCM_FLOAT;

#else

static constexpr audio_format_t kProcessFormat = AUDIO_FORMAT_PCM_16_BIT;

static inline int16_t clamp16(int32_t sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

#endif // BUILD_FLOAT

extern "C" {

// effect_handle_t interface implementation for LE effect
extern const struct effect_interface_s gLEInterface;

// AOSP Loudness Enhancer UUID: fa415329-2034-4bea-b5dc-5b381c8d1e2c
const effect_descriptor_t gLEDescriptor = {
        {0xfe3199be, 0xaed0, 0x413f, 0x87bb, {0x11, 0x26, 0x0e, 0xb6, 0x3c, 0xf1}}, // type
        {0xfa415329, 0x2034, 0x4bea, 0xb5dc, {0x5b, 0x38, 0x1c, 0x8d, 0x1e, 0x2c}}, // uuid
        EFFECT_CONTROL_API_VERSION,
        (EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_FIRST),
        0, // TODO
        1,
        "Loudness Enhancer",
        "The Android Open Source Project",
};

enum le_state_e {
    LOUDNESS_ENHANCER_STATE_UNINITIALIZED,
    LOUDNESS_ENHANCER_STATE_INITIALIZED,
    LOUDNESS_ENHANCER_STATE_ACTIVE,
};

struct LoudnessEnhancerContext {
    const struct effect_interface_s *mItfe;
    effect_config_t mConfig;
    uint8_t mState;
    int32_t mTargetGainmB;// target gain in mB
    // in this implementation, there is no coupling between the compression on the left and right
    // channels
    le_fx::AdaptiveDynamicRangeCompression* mCompressor;
};

//
//--- Local functions (not directly used by effect interface)
//

void LE_reset(LoudnessEnhancerContext *pContext)
{
    ALOGV("  > LE_reset(%p)", pContext);

    if (pContext->mCompressor != NULL) {
        float targetAmp = pow(10, pContext->mTargetGainmB/2000.0f); // mB to linear amplification
        ALOGV("LE_reset(): Target gain=%dmB <=> factor=%.2fX", pContext->mTargetGainmB, targetAmp);
        pContext->mCompressor->Initialize(targetAmp, pContext->mConfig.inputCfg.samplingRate);
    } else {
        ALOGE("LE_reset(%p): null compressors, can't apply target gain", pContext);
    }
}

//----------------------------------------------------------------------------
// LE_setConfig()
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

int LE_setConfig(LoudnessEnhancerContext *pContext, effect_config_t *pConfig)
{
    ALOGV("LE_setConfig(%p)", pContext);

    if (pConfig->inputCfg.samplingRate != pConfig->outputCfg.samplingRate) return -EINVAL;
    if (pConfig->inputCfg.channels != pConfig->outputCfg.channels) return -EINVAL;
    if (pConfig->inputCfg.format != pConfig->outputCfg.format) return -EINVAL;
    if (pConfig->inputCfg.channels != AUDIO_CHANNEL_OUT_STEREO) return -EINVAL;
    if (pConfig->outputCfg.accessMode != EFFECT_BUFFER_ACCESS_WRITE &&
            pConfig->outputCfg.accessMode != EFFECT_BUFFER_ACCESS_ACCUMULATE) return -EINVAL;
    if (pConfig->inputCfg.format != kProcessFormat) return -EINVAL;

    pContext->mConfig = *pConfig;

    LE_reset(pContext);

    return 0;
}


//----------------------------------------------------------------------------
// LE_getConfig()
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

void LE_getConfig(LoudnessEnhancerContext *pContext, effect_config_t *pConfig)
{
    *pConfig = pContext->mConfig;
}


//----------------------------------------------------------------------------
// LE_init()
//----------------------------------------------------------------------------
// Purpose: Initialize engine with default configuration.
//
// Inputs:
//  pContext:   effect engine context
//
// Outputs:
//
//----------------------------------------------------------------------------

int LE_init(LoudnessEnhancerContext *pContext)
{
    ALOGV("LE_init(%p)", pContext);

    pContext->mConfig.inputCfg.accessMode = EFFECT_BUFFER_ACCESS_READ;
    pContext->mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
    pContext->mConfig.inputCfg.format = kProcessFormat;
    pContext->mConfig.inputCfg.samplingRate = 44100;
    pContext->mConfig.inputCfg.bufferProvider.getBuffer = NULL;
    pContext->mConfig.inputCfg.bufferProvider.releaseBuffer = NULL;
    pContext->mConfig.inputCfg.bufferProvider.cookie = NULL;
    pContext->mConfig.inputCfg.mask = EFFECT_CONFIG_ALL;
    pContext->mConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_ACCUMULATE;
    pContext->mConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
    pContext->mConfig.outputCfg.format = kProcessFormat;
    pContext->mConfig.outputCfg.samplingRate = 44100;
    pContext->mConfig.outputCfg.bufferProvider.getBuffer = NULL;
    pContext->mConfig.outputCfg.bufferProvider.releaseBuffer = NULL;
    pContext->mConfig.outputCfg.bufferProvider.cookie = NULL;
    pContext->mConfig.outputCfg.mask = EFFECT_CONFIG_ALL;

    pContext->mTargetGainmB = LOUDNESS_ENHANCER_DEFAULT_TARGET_GAIN_MB;
    float targetAmp = pow(10, pContext->mTargetGainmB/2000.0f); // mB to linear amplification
    ALOGV("LE_init(): Target gain=%dmB <=> factor=%.2fX", pContext->mTargetGainmB, targetAmp);

    if (pContext->mCompressor == NULL) {
        pContext->mCompressor = new le_fx::AdaptiveDynamicRangeCompression();
        pContext->mCompressor->Initialize(targetAmp, pContext->mConfig.inputCfg.samplingRate);
    }

    LE_setConfig(pContext, &pContext->mConfig);

    return 0;
}

//
//--- Effect Library Interface Implementation
//

int LELib_Create(const effect_uuid_t *uuid,
                         int32_t sessionId __unused,
                         int32_t ioId __unused,
                         effect_handle_t *pHandle) {
    ALOGV("LELib_Create()");
    int ret;

    if (pHandle == NULL || uuid == NULL) {
        return -EINVAL;
    }

    if (memcmp(uuid, &gLEDescriptor.uuid, sizeof(effect_uuid_t)) != 0) {
        return -EINVAL;
    }

    LoudnessEnhancerContext *pContext = new LoudnessEnhancerContext;

    pContext->mItfe = &gLEInterface;
    pContext->mState = LOUDNESS_ENHANCER_STATE_UNINITIALIZED;

    pContext->mCompressor = NULL;
    ret = LE_init(pContext);
    if (ret < 0) {
        ALOGW("LELib_Create() init failed");
        delete pContext;
        return ret;
    }

    *pHandle = (effect_handle_t)pContext;

    pContext->mState = LOUDNESS_ENHANCER_STATE_INITIALIZED;

    ALOGV("  LELib_Create context is %p", pContext);

    return 0;

}

int LELib_Release(effect_handle_t handle) {
    LoudnessEnhancerContext * pContext = (LoudnessEnhancerContext *)handle;

    ALOGV("LELib_Release %p", handle);
    if (pContext == NULL) {
        return -EINVAL;
    }
    pContext->mState = LOUDNESS_ENHANCER_STATE_UNINITIALIZED;
    if (pContext->mCompressor != NULL) {
        delete pContext->mCompressor;
        pContext->mCompressor = NULL;
    }
    delete pContext;

    return 0;
}

int LELib_GetDescriptor(const effect_uuid_t *uuid,
                                effect_descriptor_t *pDescriptor) {

    if (pDescriptor == NULL || uuid == NULL){
        ALOGV("LELib_GetDescriptor() called with NULL pointer");
        return -EINVAL;
    }

    if (memcmp(uuid, &gLEDescriptor.uuid, sizeof(effect_uuid_t)) == 0) {
        *pDescriptor = gLEDescriptor;
        return 0;
    }

    return  -EINVAL;
} /* end LELib_GetDescriptor */

//
//--- Effect Control Interface Implementation
//
int LE_process(
        effect_handle_t self, audio_buffer_t *inBuffer, audio_buffer_t *outBuffer)
{
    LoudnessEnhancerContext * pContext = (LoudnessEnhancerContext *)self;

    if (pContext == NULL) {
        return -EINVAL;
    }

    if (inBuffer == NULL || inBuffer->raw == NULL ||
        outBuffer == NULL || outBuffer->raw == NULL ||
        inBuffer->frameCount != outBuffer->frameCount ||
        inBuffer->frameCount == 0) {
        return -EINVAL;
    }

    //ALOGV("LE about to process %d samples", inBuffer->frameCount);
    uint16_t inIdx;
#ifdef BUILD_FLOAT
    constexpr float scale = 1 << 15; // power of 2 is lossless conversion to int16_t range
    constexpr float inverseScale = 1.f / scale;
    const float inputAmp = pow(10, pContext->mTargetGainmB/2000.0f) * scale;
#else
    float inputAmp = pow(10, pContext->mTargetGainmB/2000.0f);
#endif
    float leftSample, rightSample;
    for (inIdx = 0 ; inIdx < inBuffer->frameCount ; inIdx++) {
        // makeup gain is applied on the input of the compressor
#ifdef BUILD_FLOAT
        leftSample  = inputAmp * inBuffer->f32[2*inIdx];
        rightSample = inputAmp * inBuffer->f32[2*inIdx +1];
        pContext->mCompressor->Compress(&leftSample, &rightSample);
        inBuffer->f32[2*inIdx]    = leftSample * inverseScale;
        inBuffer->f32[2*inIdx +1] = rightSample * inverseScale;
#else
        leftSample  = inputAmp * (float)inBuffer->s16[2*inIdx];
        rightSample = inputAmp * (float)inBuffer->s16[2*inIdx +1];
        pContext->mCompressor->Compress(&leftSample, &rightSample);
        inBuffer->s16[2*inIdx]    = (int16_t) leftSample;
        inBuffer->s16[2*inIdx +1] = (int16_t) rightSample;
#endif // BUILD_FLOAT
    }

    if (inBuffer->raw != outBuffer->raw) {
#ifdef BUILD_FLOAT
        if (pContext->mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
            for (size_t i = 0; i < outBuffer->frameCount*2; i++) {
                outBuffer->f32[i] += inBuffer->f32[i];
            }
        } else {
            memcpy(outBuffer->raw, inBuffer->raw, outBuffer->frameCount * 2 * sizeof(float));
        }
#else
        if (pContext->mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
            for (size_t i = 0; i < outBuffer->frameCount*2; i++) {
                outBuffer->s16[i] = clamp16(outBuffer->s16[i] + inBuffer->s16[i]);
            }
        } else {
            memcpy(outBuffer->raw, inBuffer->raw, outBuffer->frameCount * 2 * sizeof(int16_t));
        }
#endif // BUILD_FLOAT
    }
    if (pContext->mState != LOUDNESS_ENHANCER_STATE_ACTIVE) {
        return -ENODATA;
    }
    return 0;
}

int LE_command(effect_handle_t self, uint32_t cmdCode, uint32_t cmdSize,
        void *pCmdData, uint32_t *replySize, void *pReplyData) {

    LoudnessEnhancerContext * pContext = (LoudnessEnhancerContext *)self;

    if (pContext == NULL || pContext->mState == LOUDNESS_ENHANCER_STATE_UNINITIALIZED) {
        return -EINVAL;
    }

//    ALOGV("LE_command command %d cmdSize %d",cmdCode, cmdSize);
    switch (cmdCode) {
    case EFFECT_CMD_INIT:
        if (pReplyData == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        *(int *) pReplyData = LE_init(pContext);
        break;
    case EFFECT_CMD_SET_CONFIG:
        if (pCmdData == NULL || cmdSize != sizeof(effect_config_t)
                || pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        *(int *) pReplyData = LE_setConfig(pContext,
                (effect_config_t *) pCmdData);
        break;
    case EFFECT_CMD_GET_CONFIG:
        if (pReplyData == NULL ||
            *replySize != sizeof(effect_config_t)) {
            return -EINVAL;
        }
        LE_getConfig(pContext, (effect_config_t *)pReplyData);
        break;
    case EFFECT_CMD_RESET:
        LE_reset(pContext);
        break;
    case EFFECT_CMD_ENABLE:
        if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        if (pContext->mState != LOUDNESS_ENHANCER_STATE_INITIALIZED) {
            return -ENOSYS;
        }
        pContext->mState = LOUDNESS_ENHANCER_STATE_ACTIVE;
        ALOGV("EFFECT_CMD_ENABLE() OK");
        *(int *)pReplyData = 0;
        break;
    case EFFECT_CMD_DISABLE:
        if (pReplyData == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        if (pContext->mState != LOUDNESS_ENHANCER_STATE_ACTIVE) {
            return -ENOSYS;
        }
        pContext->mState = LOUDNESS_ENHANCER_STATE_INITIALIZED;
        ALOGV("EFFECT_CMD_DISABLE() OK");
        *(int *)pReplyData = 0;
        break;
    case EFFECT_CMD_GET_PARAM: {
        if (pCmdData == NULL ||
            cmdSize != (int)(sizeof(effect_param_t) + sizeof(uint32_t)) ||
            pReplyData == NULL || replySize == NULL ||
            *replySize < (int)(sizeof(effect_param_t) + sizeof(uint32_t) + sizeof(uint32_t))) {
            return -EINVAL;
        }
        memcpy(pReplyData, pCmdData, sizeof(effect_param_t) + sizeof(uint32_t));
        effect_param_t *p = (effect_param_t *)pReplyData;
        p->status = 0;
        *replySize = sizeof(effect_param_t) + sizeof(uint32_t);
        if (p->psize != sizeof(uint32_t)) {
            p->status = -EINVAL;
            break;
        }
        switch (*(uint32_t *)p->data) {
        case LOUDNESS_ENHANCER_PARAM_TARGET_GAIN_MB:
            ALOGV("get target gain(mB) = %d", pContext->mTargetGainmB);
            *((int32_t *)p->data + 1) = pContext->mTargetGainmB;
            p->vsize = sizeof(int32_t);
            *replySize += sizeof(int32_t);
            break;
        default:
            p->status = -EINVAL;
        }
        } break;
    case EFFECT_CMD_SET_PARAM: {
        if (pCmdData == NULL ||
            cmdSize != (int)(sizeof(effect_param_t) + sizeof(uint32_t) + sizeof(uint32_t)) ||
            pReplyData == NULL || replySize == NULL || *replySize != sizeof(int32_t)) {
            return -EINVAL;
        }
        *(int32_t *)pReplyData = 0;
        effect_param_t *p = (effect_param_t *)pCmdData;
        if (p->psize != sizeof(uint32_t) || p->vsize != sizeof(uint32_t)) {
            *(int32_t *)pReplyData = -EINVAL;
            break;
        }
        switch (*(uint32_t *)p->data) {
        case LOUDNESS_ENHANCER_PARAM_TARGET_GAIN_MB:
            pContext->mTargetGainmB = *((int32_t *)p->data + 1);
            ALOGV("set target gain(mB) = %d", pContext->mTargetGainmB);
            LE_reset(pContext); // apply parameter update
            break;
        default:
            *(int32_t *)pReplyData = -EINVAL;
        }
        } break;
    case EFFECT_CMD_SET_DEVICE:
    case EFFECT_CMD_SET_VOLUME:
    case EFFECT_CMD_SET_AUDIO_MODE:
        break;

    default:
        ALOGW("LE_command invalid command %d",cmdCode);
        return -EINVAL;
    }

    return 0;
}

/* Effect Control Interface Implementation: get_descriptor */
int LE_getDescriptor(effect_handle_t   self,
                                    effect_descriptor_t *pDescriptor)
{
    LoudnessEnhancerContext * pContext = (LoudnessEnhancerContext *) self;

    if (pContext == NULL || pDescriptor == NULL) {
        ALOGV("LE_getDescriptor() invalid param");
        return -EINVAL;
    }

    *pDescriptor = gLEDescriptor;

    return 0;
}   /* end LE_getDescriptor */

// effect_handle_t interface implementation for DRC effect
const struct effect_interface_s gLEInterface = {
        LE_process,
        LE_command,
        LE_getDescriptor,
        NULL,
};

// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
audio_effect_library_t AUDIO_EFFECT_LIBRARY_INFO_SYM = {
    .tag = AUDIO_EFFECT_LIBRARY_TAG,
    .version = EFFECT_LIBRARY_API_VERSION,
    .name = "Loudness Enhancer Library",
    .implementor = "The Android Open Source Project",
    .create_effect = LELib_Create,
    .release_effect = LELib_Release,
    .get_descriptor = LELib_GetDescriptor,
};

}; // extern "C"

