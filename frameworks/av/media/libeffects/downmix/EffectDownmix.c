/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "EffectDownmix"
//#define LOG_NDEBUG 0

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#include "EffectDownmix.h"

// Do not submit with DOWNMIX_TEST_CHANNEL_INDEX defined, strictly for testing
//#define DOWNMIX_TEST_CHANNEL_INDEX 0
// Do not submit with DOWNMIX_ALWAYS_USE_GENERIC_DOWNMIXER defined, strictly for testing
//#define DOWNMIX_ALWAYS_USE_GENERIC_DOWNMIXER 0

#ifdef BUILD_FLOAT
#define MINUS_3_DB_IN_FLOAT 0.70710678f // -3dB = 0.70710678f
const audio_format_t gTargetFormat = AUDIO_FORMAT_PCM_FLOAT;
#else
#define MINUS_3_DB_IN_Q19_12 2896 // -3dB = 0.707 * 2^12 = 2896
const audio_format_t gTargetFormat = AUDIO_FORMAT_PCM_16_BIT;
#endif

// subset of possible audio_channel_mask_t values, and AUDIO_CHANNEL_OUT_* renamed to CHANNEL_MASK_*
typedef enum {
    CHANNEL_MASK_QUAD_BACK = AUDIO_CHANNEL_OUT_QUAD_BACK,
    CHANNEL_MASK_QUAD_SIDE = AUDIO_CHANNEL_OUT_QUAD_SIDE,
    CHANNEL_MASK_5POINT1_BACK = AUDIO_CHANNEL_OUT_5POINT1_BACK,
    CHANNEL_MASK_5POINT1_SIDE = AUDIO_CHANNEL_OUT_5POINT1_SIDE,
    CHANNEL_MASK_7POINT1 = AUDIO_CHANNEL_OUT_7POINT1,
} downmix_input_channel_mask_t;

// effect_handle_t interface implementation for downmix effect
const struct effect_interface_s gDownmixInterface = {
        Downmix_Process,
        Downmix_Command,
        Downmix_GetDescriptor,
        NULL /* no process_reverse function, no reference stream needed */
};

// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
audio_effect_library_t AUDIO_EFFECT_LIBRARY_INFO_SYM = {
    .tag = AUDIO_EFFECT_LIBRARY_TAG,
    .version = EFFECT_LIBRARY_API_VERSION,
    .name = "Downmix Library",
    .implementor = "The Android Open Source Project",
    .create_effect = DownmixLib_Create,
    .release_effect = DownmixLib_Release,
    .get_descriptor = DownmixLib_GetDescriptor,
};


// AOSP insert downmix UUID: 93f04452-e4fe-41cc-91f9-e475b6d1d69f
static const effect_descriptor_t gDownmixDescriptor = {
        EFFECT_UIID_DOWNMIX__, //type
        {0x93f04452, 0xe4fe, 0x41cc, 0x91f9, {0xe4, 0x75, 0xb6, 0xd1, 0xd6, 0x9f}}, // uuid
        EFFECT_CONTROL_API_VERSION,
        EFFECT_FLAG_TYPE_INSERT | EFFECT_FLAG_INSERT_FIRST,
        0, //FIXME what value should be reported? // cpu load
        0, //FIXME what value should be reported? // memory usage
        "Multichannel Downmix To Stereo", // human readable effect name
        "The Android Open Source Project" // human readable effect implementor name
};

// gDescriptors contains pointers to all defined effect descriptor in this library
static const effect_descriptor_t * const gDescriptors[] = {
        &gDownmixDescriptor
};

// number of effects in this library
const int kNbEffects = sizeof(gDescriptors) / sizeof(const effect_descriptor_t *);
#ifdef BUILD_FLOAT
static LVM_FLOAT clamp_float(LVM_FLOAT a) {
    if (a > 1.0f) {
        return 1.0f;
    }
    else if (a < -1.0f) {
        return -1.0f;
    }
    else {
        return a;
    }
}
#endif
/*----------------------------------------------------------------------------
 * Test code
 *--------------------------------------------------------------------------*/
#ifdef DOWNMIX_TEST_CHANNEL_INDEX
// strictly for testing, logs the indices of the channels for a given mask,
// uses the same code as Downmix_foldGeneric()
void Downmix_testIndexComputation(uint32_t mask) {
    ALOGI("Testing index computation for 0x%" PRIx32 ":", mask);
    // check against unsupported channels
    if (mask & kUnsupported) {
        ALOGE("Unsupported channels (top or front left/right of center)");
        return;
    }
    // verify has FL/FR
    if ((mask & AUDIO_CHANNEL_OUT_STEREO) != AUDIO_CHANNEL_OUT_STEREO) {
        ALOGE("Front channels must be present");
        return;
    }
    // verify uses SIDE as a pair (ok if not using SIDE at all)
    bool hasSides = false;
    if ((mask & kSides) != 0) {
        if ((mask & kSides) != kSides) {
            ALOGE("Side channels must be used as a pair");
            return;
        }
        hasSides = true;
    }
    // verify uses BACK as a pair (ok if not using BACK at all)
    bool hasBacks = false;
    if ((mask & kBacks) != 0) {
        if ((mask & kBacks) != kBacks) {
            ALOGE("Back channels must be used as a pair");
            return;
        }
        hasBacks = true;
    }

    const int numChan = audio_channel_count_from_out_mask(mask);
    const bool hasFC = ((mask & AUDIO_CHANNEL_OUT_FRONT_CENTER) == AUDIO_CHANNEL_OUT_FRONT_CENTER);
    const bool hasLFE =
            ((mask & AUDIO_CHANNEL_OUT_LOW_FREQUENCY) == AUDIO_CHANNEL_OUT_LOW_FREQUENCY);
    const bool hasBC = ((mask & AUDIO_CHANNEL_OUT_BACK_CENTER) == AUDIO_CHANNEL_OUT_BACK_CENTER);
    // compute at what index each channel is: samples will be in the following order:
    //   FL FR FC LFE BL BR BC SL SR
    // when a channel is not present, its index is set to the same as the index of the preceding
    // channel
    const int indexFC  = hasFC    ? 2            : 1;        // front center
    const int indexLFE = hasLFE   ? indexFC + 1  : indexFC;  // low frequency
    const int indexBL  = hasBacks ? indexLFE + 1 : indexLFE; // back left
    const int indexBR  = hasBacks ? indexBL + 1  : indexBL;  // back right
    const int indexBC  = hasBC    ? indexBR + 1  : indexBR;  // back center
    const int indexSL  = hasSides ? indexBC + 1  : indexBC;  // side left
    const int indexSR  = hasSides ? indexSL + 1  : indexSL;  // side right

    ALOGI("  FL FR FC LFE BL BR BC SL SR");
    ALOGI("   %d  %d  %d   %d  %d  %d  %d  %d  %d",
            0, 1, indexFC, indexLFE, indexBL, indexBR, indexBC, indexSL, indexSR);
}
#endif

static bool Downmix_validChannelMask(uint32_t mask)
{
    if (!mask) {
        return false;
    }
    // check against unsupported channels
    if (mask & kUnsupported) {
        ALOGE("Unsupported channels (top or front left/right of center)");
        return false;
    }
    // verify has FL/FR
    if ((mask & AUDIO_CHANNEL_OUT_STEREO) != AUDIO_CHANNEL_OUT_STEREO) {
        ALOGE("Front channels must be present");
        return false;
    }
    // verify uses SIDE as a pair (ok if not using SIDE at all)
    if ((mask & kSides) != 0) {
        if ((mask & kSides) != kSides) {
            ALOGE("Side channels must be used as a pair");
            return false;
        }
    }
    // verify uses BACK as a pair (ok if not using BACK at all)
    if ((mask & kBacks) != 0) {
        if ((mask & kBacks) != kBacks) {
            ALOGE("Back channels must be used as a pair");
            return false;
        }
    }
    return true;
}

/*----------------------------------------------------------------------------
 * Effect API implementation
 *--------------------------------------------------------------------------*/

/*--- Effect Library Interface Implementation ---*/

int32_t DownmixLib_Create(const effect_uuid_t *uuid,
        int32_t sessionId __unused,
        int32_t ioId __unused,
        effect_handle_t *pHandle) {
    int ret;
    int i;
    downmix_module_t *module;
    const effect_descriptor_t *desc;

    ALOGV("DownmixLib_Create()");

#ifdef DOWNMIX_TEST_CHANNEL_INDEX
    // should work (won't log an error)
    ALOGI("DOWNMIX_TEST_CHANNEL_INDEX: should work:");
    Downmix_testIndexComputation(AUDIO_CHANNEL_OUT_FRONT_LEFT | AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                    AUDIO_CHANNEL_OUT_LOW_FREQUENCY | AUDIO_CHANNEL_OUT_BACK_CENTER);
    Downmix_testIndexComputation(CHANNEL_MASK_QUAD_SIDE | CHANNEL_MASK_QUAD_BACK);
    Downmix_testIndexComputation(CHANNEL_MASK_5POINT1_SIDE | AUDIO_CHANNEL_OUT_BACK_CENTER);
    Downmix_testIndexComputation(CHANNEL_MASK_5POINT1_BACK | AUDIO_CHANNEL_OUT_BACK_CENTER);
    // shouldn't work (will log an error, won't display channel indices)
    ALOGI("DOWNMIX_TEST_CHANNEL_INDEX: should NOT work:");
    Downmix_testIndexComputation(AUDIO_CHANNEL_OUT_FRONT_LEFT | AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                        AUDIO_CHANNEL_OUT_LOW_FREQUENCY | AUDIO_CHANNEL_OUT_BACK_LEFT);
    Downmix_testIndexComputation(AUDIO_CHANNEL_OUT_FRONT_LEFT | AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                            AUDIO_CHANNEL_OUT_LOW_FREQUENCY | AUDIO_CHANNEL_OUT_SIDE_LEFT);
    Downmix_testIndexComputation(AUDIO_CHANNEL_OUT_FRONT_LEFT |
                        AUDIO_CHANNEL_OUT_BACK_LEFT | AUDIO_CHANNEL_OUT_BACK_RIGHT);
    Downmix_testIndexComputation(AUDIO_CHANNEL_OUT_FRONT_LEFT |
                            AUDIO_CHANNEL_OUT_SIDE_LEFT | AUDIO_CHANNEL_OUT_SIDE_RIGHT);
#endif

    if (pHandle == NULL || uuid == NULL) {
        return -EINVAL;
    }

    for (i = 0 ; i < kNbEffects ; i++) {
        desc = gDescriptors[i];
        if (memcmp(uuid, &desc->uuid, sizeof(effect_uuid_t)) == 0) {
            break;
        }
    }

    if (i == kNbEffects) {
        return -ENOENT;
    }

    module = malloc(sizeof(downmix_module_t));

    module->itfe = &gDownmixInterface;

    module->context.state = DOWNMIX_STATE_UNINITIALIZED;

    ret = Downmix_Init(module);
    if (ret < 0) {
        ALOGW("DownmixLib_Create() init failed");
        free(module);
        return ret;
    }

    *pHandle = (effect_handle_t) module;

    ALOGV("DownmixLib_Create() %p , size %zu", module, sizeof(downmix_module_t));

    return 0;
}


int32_t DownmixLib_Release(effect_handle_t handle) {
    downmix_module_t *pDwmModule = (downmix_module_t *)handle;

    ALOGV("DownmixLib_Release() %p", handle);
    if (handle == NULL) {
        return -EINVAL;
    }

    pDwmModule->context.state = DOWNMIX_STATE_UNINITIALIZED;

    free(pDwmModule);
    return 0;
}


int32_t DownmixLib_GetDescriptor(const effect_uuid_t *uuid, effect_descriptor_t *pDescriptor) {
    ALOGV("DownmixLib_GetDescriptor()");
    int i;

    if (pDescriptor == NULL || uuid == NULL){
        ALOGE("DownmixLib_Create() called with NULL pointer");
        return -EINVAL;
    }
    ALOGV("DownmixLib_GetDescriptor() nb effects=%d", kNbEffects);
    for (i = 0; i < kNbEffects; i++) {
        ALOGV("DownmixLib_GetDescriptor() i=%d", i);
        if (memcmp(uuid, &gDescriptors[i]->uuid, sizeof(effect_uuid_t)) == 0) {
            memcpy(pDescriptor, gDescriptors[i], sizeof(effect_descriptor_t));
            ALOGV("EffectGetDescriptor - UUID matched downmix type %d, UUID = %" PRIx32,
                 i, gDescriptors[i]->uuid.timeLow);
            return 0;
        }
    }

    return -EINVAL;
}

#ifndef BUILD_FLOAT
/*--- Effect Control Interface Implementation ---*/

static int Downmix_Process(effect_handle_t self,
        audio_buffer_t *inBuffer, audio_buffer_t *outBuffer) {

    downmix_object_t *pDownmixer;
    int16_t *pSrc, *pDst;
    downmix_module_t *pDwmModule = (downmix_module_t *)self;

    if (pDwmModule == NULL) {
        return -EINVAL;
    }

    if (inBuffer == NULL || inBuffer->raw == NULL ||
        outBuffer == NULL || outBuffer->raw == NULL ||
        inBuffer->frameCount != outBuffer->frameCount) {
        return -EINVAL;
    }

    pDownmixer = (downmix_object_t*) &pDwmModule->context;

    if (pDownmixer->state == DOWNMIX_STATE_UNINITIALIZED) {
        ALOGE("Downmix_Process error: trying to use an uninitialized downmixer");
        return -EINVAL;
    } else if (pDownmixer->state == DOWNMIX_STATE_INITIALIZED) {
        ALOGE("Downmix_Process error: trying to use a non-configured downmixer");
        return -ENODATA;
    }

    pSrc = inBuffer->s16;
    pDst = outBuffer->s16;
    size_t numFrames = outBuffer->frameCount;

    const bool accumulate =
            (pDwmModule->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE);
    const uint32_t downmixInputChannelMask = pDwmModule->config.inputCfg.channels;

    switch(pDownmixer->type) {

      case DOWNMIX_TYPE_STRIP:
          if (accumulate) {
              while (numFrames) {
                  pDst[0] = clamp16(pDst[0] + pSrc[0]);
                  pDst[1] = clamp16(pDst[1] + pSrc[1]);
                  pSrc += pDownmixer->input_channel_count;
                  pDst += 2;
                  numFrames--;
              }
          } else {
              while (numFrames) {
                  pDst[0] = pSrc[0];
                  pDst[1] = pSrc[1];
                  pSrc += pDownmixer->input_channel_count;
                  pDst += 2;
                  numFrames--;
              }
          }
          break;

      case DOWNMIX_TYPE_FOLD:
#ifdef DOWNMIX_ALWAYS_USE_GENERIC_DOWNMIXER
          // bypass the optimized downmix routines for the common formats
          if (!Downmix_foldGeneric(
                  downmixInputChannelMask, pSrc, pDst, numFrames, accumulate)) {
              ALOGE("Multichannel configuration 0x%" PRIx32 " is not supported", downmixInputChannelMask);
              return -EINVAL;
          }
          break;
#endif
        // optimize for the common formats
        switch((downmix_input_channel_mask_t)downmixInputChannelMask) {
        case CHANNEL_MASK_QUAD_BACK:
        case CHANNEL_MASK_QUAD_SIDE:
            Downmix_foldFromQuad(pSrc, pDst, numFrames, accumulate);
            break;
        case CHANNEL_MASK_5POINT1_BACK:
        case CHANNEL_MASK_5POINT1_SIDE:
            Downmix_foldFrom5Point1(pSrc, pDst, numFrames, accumulate);
            break;
        case CHANNEL_MASK_7POINT1:
            Downmix_foldFrom7Point1(pSrc, pDst, numFrames, accumulate);
            break;
        default:
            if (!Downmix_foldGeneric(
                    downmixInputChannelMask, pSrc, pDst, numFrames, accumulate)) {
                ALOGE("Multichannel configuration 0x%" PRIx32 " is not supported", downmixInputChannelMask);
                return -EINVAL;
            }
            break;
        }
        break;

      default:
        return -EINVAL;
    }

    return 0;
}
#else /*BUILD_FLOAT*/
/*--- Effect Control Interface Implementation ---*/

static int Downmix_Process(effect_handle_t self,
        audio_buffer_t *inBuffer, audio_buffer_t *outBuffer) {

    downmix_object_t *pDownmixer;
    LVM_FLOAT *pSrc, *pDst;
    downmix_module_t *pDwmModule = (downmix_module_t *)self;

    if (pDwmModule == NULL) {
        return -EINVAL;
    }

    if (inBuffer == NULL || inBuffer->raw == NULL ||
        outBuffer == NULL || outBuffer->raw == NULL ||
        inBuffer->frameCount != outBuffer->frameCount) {
        return -EINVAL;
    }

    pDownmixer = (downmix_object_t*) &pDwmModule->context;

    if (pDownmixer->state == DOWNMIX_STATE_UNINITIALIZED) {
        ALOGE("Downmix_Process error: trying to use an uninitialized downmixer");
        return -EINVAL;
    } else if (pDownmixer->state == DOWNMIX_STATE_INITIALIZED) {
        ALOGE("Downmix_Process error: trying to use a non-configured downmixer");
        return -ENODATA;
    }

    pSrc = (LVM_FLOAT *) inBuffer->s16;
    pDst = (LVM_FLOAT *) outBuffer->s16;
    size_t numFrames = outBuffer->frameCount;

    const bool accumulate =
            (pDwmModule->config.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE);
    const uint32_t downmixInputChannelMask = pDwmModule->config.inputCfg.channels;

    switch(pDownmixer->type) {

      case DOWNMIX_TYPE_STRIP:
          if (accumulate) {
              while (numFrames) {
                  pDst[0] = clamp_float(pDst[0] + pSrc[0]);
                  pDst[1] = clamp_float(pDst[1] + pSrc[1]);
                  pSrc += pDownmixer->input_channel_count;
                  pDst += 2;
                  numFrames--;
              }
          } else {
              while (numFrames) {
                  pDst[0] = pSrc[0];
                  pDst[1] = pSrc[1];
                  pSrc += pDownmixer->input_channel_count;
                  pDst += 2;
                  numFrames--;
              }
          }
          break;

      case DOWNMIX_TYPE_FOLD:
#ifdef DOWNMIX_ALWAYS_USE_GENERIC_DOWNMIXER
          // bypass the optimized downmix routines for the common formats
          if (!Downmix_foldGeneric(
                  downmixInputChannelMask, pSrc, pDst, numFrames, accumulate)) {
              ALOGE("Multichannel configuration 0x%" PRIx32 " is not supported",
                    downmixInputChannelMask);
              return -EINVAL;
          }
          break;
#endif
        // optimize for the common formats
        switch((downmix_input_channel_mask_t)downmixInputChannelMask) {
        case CHANNEL_MASK_QUAD_BACK:
        case CHANNEL_MASK_QUAD_SIDE:
            Downmix_foldFromQuad(pSrc, pDst, numFrames, accumulate);
            break;
        case CHANNEL_MASK_5POINT1_BACK:
        case CHANNEL_MASK_5POINT1_SIDE:
            Downmix_foldFrom5Point1(pSrc, pDst, numFrames, accumulate);
            break;
        case CHANNEL_MASK_7POINT1:
            Downmix_foldFrom7Point1(pSrc, pDst, numFrames, accumulate);
            break;
        default:
            if (!Downmix_foldGeneric(
                    downmixInputChannelMask, pSrc, pDst, numFrames, accumulate)) {
                ALOGE("Multichannel configuration 0x%" PRIx32 " is not supported",
                      downmixInputChannelMask);
                return -EINVAL;
            }
            break;
        }
        break;

      default:
        return -EINVAL;
    }

    return 0;
}
#endif

static int Downmix_Command(effect_handle_t self, uint32_t cmdCode, uint32_t cmdSize,
        void *pCmdData, uint32_t *replySize, void *pReplyData) {

    downmix_module_t *pDwmModule = (downmix_module_t *) self;
    downmix_object_t *pDownmixer;

    if (pDwmModule == NULL || pDwmModule->context.state == DOWNMIX_STATE_UNINITIALIZED) {
        return -EINVAL;
    }

    pDownmixer = (downmix_object_t*) &pDwmModule->context;

    ALOGV("Downmix_Command command %" PRIu32 " cmdSize %" PRIu32, cmdCode, cmdSize);

    switch (cmdCode) {
    case EFFECT_CMD_INIT:
        if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        *(int *) pReplyData = Downmix_Init(pDwmModule);
        break;

    case EFFECT_CMD_SET_CONFIG:
        if (pCmdData == NULL || cmdSize != sizeof(effect_config_t)
                || pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        *(int *) pReplyData = Downmix_Configure(pDwmModule,
                (effect_config_t *)pCmdData, false);
        break;

    case EFFECT_CMD_RESET:
        Downmix_Reset(pDownmixer, false);
        break;

    case EFFECT_CMD_GET_PARAM:
        ALOGV("Downmix_Command EFFECT_CMD_GET_PARAM pCmdData %p, *replySize %" PRIu32 ", pReplyData: %p",
                pCmdData, *replySize, pReplyData);
        if (pCmdData == NULL || cmdSize < (int)(sizeof(effect_param_t) + sizeof(int32_t)) ||
                pReplyData == NULL || replySize == NULL ||
                *replySize < (int) sizeof(effect_param_t) + 2 * sizeof(int32_t)) {
            return -EINVAL;
        }
        effect_param_t *rep = (effect_param_t *) pReplyData;
        memcpy(pReplyData, pCmdData, sizeof(effect_param_t) + sizeof(int32_t));
        ALOGV("Downmix_Command EFFECT_CMD_GET_PARAM param %" PRId32 ", replySize %" PRIu32,
                *(int32_t *)rep->data, rep->vsize);
        rep->status = Downmix_getParameter(pDownmixer, *(int32_t *)rep->data, &rep->vsize,
                rep->data + sizeof(int32_t));
        *replySize = sizeof(effect_param_t) + sizeof(int32_t) + rep->vsize;
        break;

    case EFFECT_CMD_SET_PARAM:
        ALOGV("Downmix_Command EFFECT_CMD_SET_PARAM cmdSize %d pCmdData %p, *replySize %" PRIu32
                ", pReplyData %p", cmdSize, pCmdData, *replySize, pReplyData);
        if (pCmdData == NULL || (cmdSize < (int)(sizeof(effect_param_t) + sizeof(int32_t)))
                || pReplyData == NULL || replySize == NULL || *replySize != (int)sizeof(int32_t)) {
            return -EINVAL;
        }
        effect_param_t *cmd = (effect_param_t *) pCmdData;
        if (cmd->psize != sizeof(int32_t)) {
            android_errorWriteLog(0x534e4554, "63662938");
            return -EINVAL;
        }
        *(int *)pReplyData = Downmix_setParameter(pDownmixer, *(int32_t *)cmd->data,
                cmd->vsize, cmd->data + sizeof(int32_t));
        break;

    case EFFECT_CMD_SET_PARAM_DEFERRED:
        //FIXME implement
        ALOGW("Downmix_Command command EFFECT_CMD_SET_PARAM_DEFERRED not supported, FIXME");
        break;

    case EFFECT_CMD_SET_PARAM_COMMIT:
        //FIXME implement
        ALOGW("Downmix_Command command EFFECT_CMD_SET_PARAM_COMMIT not supported, FIXME");
        break;

    case EFFECT_CMD_ENABLE:
        if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        if (pDownmixer->state != DOWNMIX_STATE_INITIALIZED) {
            return -ENOSYS;
        }
        pDownmixer->state = DOWNMIX_STATE_ACTIVE;
        ALOGV("EFFECT_CMD_ENABLE() OK");
        *(int *)pReplyData = 0;
        break;

    case EFFECT_CMD_DISABLE:
        if (pReplyData == NULL || replySize == NULL || *replySize != sizeof(int)) {
            return -EINVAL;
        }
        if (pDownmixer->state != DOWNMIX_STATE_ACTIVE) {
            return -ENOSYS;
        }
        pDownmixer->state = DOWNMIX_STATE_INITIALIZED;
        ALOGV("EFFECT_CMD_DISABLE() OK");
        *(int *)pReplyData = 0;
        break;

    case EFFECT_CMD_SET_DEVICE:
        if (pCmdData == NULL || cmdSize != (int)sizeof(uint32_t)) {
            return -EINVAL;
        }
        // FIXME change type if playing on headset vs speaker
        ALOGV("Downmix_Command EFFECT_CMD_SET_DEVICE: 0x%08" PRIx32, *(uint32_t *)pCmdData);
        break;

    case EFFECT_CMD_SET_VOLUME: {
        // audio output is always stereo => 2 channel volumes
        if (pCmdData == NULL || cmdSize != (int)sizeof(uint32_t) * 2) {
            return -EINVAL;
        }
        // FIXME change volume
        ALOGW("Downmix_Command command EFFECT_CMD_SET_VOLUME not supported, FIXME");
        float left = (float)(*(uint32_t *)pCmdData) / (1 << 24);
        float right = (float)(*((uint32_t *)pCmdData + 1)) / (1 << 24);
        ALOGV("Downmix_Command EFFECT_CMD_SET_VOLUME: left %f, right %f ", left, right);
        break;
    }

    case EFFECT_CMD_SET_AUDIO_MODE:
        if (pCmdData == NULL || cmdSize != (int)sizeof(uint32_t)) {
            return -EINVAL;
        }
        ALOGV("Downmix_Command EFFECT_CMD_SET_AUDIO_MODE: %" PRIu32, *(uint32_t *)pCmdData);
        break;

    case EFFECT_CMD_SET_CONFIG_REVERSE:
    case EFFECT_CMD_SET_INPUT_DEVICE:
        // these commands are ignored by a downmix effect
        break;

    default:
        ALOGW("Downmix_Command invalid command %" PRIu32, cmdCode);
        return -EINVAL;
    }

    return 0;
}


int Downmix_GetDescriptor(effect_handle_t self, effect_descriptor_t *pDescriptor)
{
    downmix_module_t *pDwnmxModule = (downmix_module_t *) self;

    if (pDwnmxModule == NULL ||
            pDwnmxModule->context.state == DOWNMIX_STATE_UNINITIALIZED) {
        return -EINVAL;
    }

    memcpy(pDescriptor, &gDownmixDescriptor, sizeof(effect_descriptor_t));

    return 0;
}


/*----------------------------------------------------------------------------
 * Downmix internal functions
 *--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Downmix_Init()
 *----------------------------------------------------------------------------
 * Purpose:
 * Initialize downmix context and apply default parameters
 *
 * Inputs:
 *  pDwmModule    pointer to downmix effect module
 *
 * Outputs:
 *
 * Returns:
 *  0             indicates success
 *
 * Side Effects:
 *  updates:
 *           pDwmModule->context.type
 *           pDwmModule->context.apply_volume_correction
 *           pDwmModule->config.inputCfg
 *           pDwmModule->config.outputCfg
 *           pDwmModule->config.inputCfg.samplingRate
 *           pDwmModule->config.outputCfg.samplingRate
 *           pDwmModule->context.state
 *  doesn't set:
 *           pDwmModule->itfe
 *
 *----------------------------------------------------------------------------
 */

int Downmix_Init(downmix_module_t *pDwmModule) {

    ALOGV("Downmix_Init module %p", pDwmModule);
    int ret = 0;

    memset(&pDwmModule->context, 0, sizeof(downmix_object_t));

    pDwmModule->config.inputCfg.accessMode = EFFECT_BUFFER_ACCESS_READ;
    pDwmModule->config.inputCfg.format = gTargetFormat;
    pDwmModule->config.inputCfg.channels = AUDIO_CHANNEL_OUT_7POINT1;
    pDwmModule->config.inputCfg.bufferProvider.getBuffer = NULL;
    pDwmModule->config.inputCfg.bufferProvider.releaseBuffer = NULL;
    pDwmModule->config.inputCfg.bufferProvider.cookie = NULL;
    pDwmModule->config.inputCfg.mask = EFFECT_CONFIG_ALL;

    pDwmModule->config.inputCfg.samplingRate = 44100;
    pDwmModule->config.outputCfg.samplingRate = pDwmModule->config.inputCfg.samplingRate;

    // set a default value for the access mode, but should be overwritten by caller
    pDwmModule->config.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_ACCUMULATE;
    pDwmModule->config.outputCfg.format = gTargetFormat;
    pDwmModule->config.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
    pDwmModule->config.outputCfg.bufferProvider.getBuffer = NULL;
    pDwmModule->config.outputCfg.bufferProvider.releaseBuffer = NULL;
    pDwmModule->config.outputCfg.bufferProvider.cookie = NULL;
    pDwmModule->config.outputCfg.mask = EFFECT_CONFIG_ALL;

    ret = Downmix_Configure(pDwmModule, &pDwmModule->config, true);
    if (ret != 0) {
        ALOGV("Downmix_Init error %d on module %p", ret, pDwmModule);
    } else {
        pDwmModule->context.state = DOWNMIX_STATE_INITIALIZED;
    }

    return ret;
}


/*----------------------------------------------------------------------------
 * Downmix_Configure()
 *----------------------------------------------------------------------------
 * Purpose:
 *  Set input and output audio configuration.
 *
 * Inputs:
 *  pDwmModule  pointer to downmix effect module
 *  pConfig     pointer to effect_config_t structure containing input
 *                  and output audio parameters configuration
 *  init        true if called from init function
 *
 * Outputs:
 *
 * Returns:
 *  0           indicates success
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
 */

int Downmix_Configure(downmix_module_t *pDwmModule, effect_config_t *pConfig, bool init) {

    downmix_object_t *pDownmixer = &pDwmModule->context;

    // Check configuration compatibility with build options, and effect capabilities
    if (pConfig->inputCfg.samplingRate != pConfig->outputCfg.samplingRate
        || pConfig->outputCfg.channels != DOWNMIX_OUTPUT_CHANNELS
        || pConfig->inputCfg.format != gTargetFormat
        || pConfig->outputCfg.format != gTargetFormat) {
        ALOGE("Downmix_Configure error: invalid config");
        return -EINVAL;
    }

    if (&pDwmModule->config != pConfig) {
        memcpy(&pDwmModule->config, pConfig, sizeof(effect_config_t));
    }

    if (init) {
        pDownmixer->type = DOWNMIX_TYPE_FOLD;
        pDownmixer->apply_volume_correction = false;
        pDownmixer->input_channel_count = 8; // matches default input of AUDIO_CHANNEL_OUT_7POINT1
    } else {
        // when configuring the effect, do not allow a blank or unsupported channel mask
        if (!Downmix_validChannelMask(pConfig->inputCfg.channels)) {
            ALOGE("Downmix_Configure error: input channel mask(0x%x) not supported",
                                                        pConfig->inputCfg.channels);
            return -EINVAL;
        }
        pDownmixer->input_channel_count =
                audio_channel_count_from_out_mask(pConfig->inputCfg.channels);
    }

    Downmix_Reset(pDownmixer, init);

    return 0;
}


/*----------------------------------------------------------------------------
 * Downmix_Reset()
 *----------------------------------------------------------------------------
 * Purpose:
 *  Reset internal states.
 *
 * Inputs:
 *  pDownmixer   pointer to downmix context
 *  init         true if called from init function
 *
 * Outputs:
*
 * Returns:
 *  0            indicates success
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
 */

int Downmix_Reset(downmix_object_t *pDownmixer __unused, bool init __unused) {
    // nothing to do here
    return 0;
}


/*----------------------------------------------------------------------------
 * Downmix_setParameter()
 *----------------------------------------------------------------------------
 * Purpose:
 * Set a Downmix parameter
 *
 * Inputs:
 *  pDownmixer    handle to instance data
 *  param         parameter
 *  pValue        pointer to parameter value
 *  size          value size
 *
 * Outputs:
 *
 * Returns:
 *  0             indicates success
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
 */
int Downmix_setParameter(downmix_object_t *pDownmixer, int32_t param, uint32_t size, void *pValue) {

    int16_t value16;
    ALOGV("Downmix_setParameter, context %p, param %" PRId32 ", value16 %" PRId16 ", value32 %" PRId32,
            pDownmixer, param, *(int16_t *)pValue, *(int32_t *)pValue);

    switch (param) {

      case DOWNMIX_PARAM_TYPE:
        if (size != sizeof(downmix_type_t)) {
            ALOGE("Downmix_setParameter(DOWNMIX_PARAM_TYPE) invalid size %" PRIu32 ", should be %zu",
                    size, sizeof(downmix_type_t));
            return -EINVAL;
        }
        value16 = *(int16_t *)pValue;
        ALOGV("set DOWNMIX_PARAM_TYPE, type %" PRId16, value16);
        if (!((value16 > DOWNMIX_TYPE_INVALID) && (value16 <= DOWNMIX_TYPE_LAST))) {
            ALOGE("Downmix_setParameter invalid DOWNMIX_PARAM_TYPE value %" PRId16, value16);
            return -EINVAL;
        } else {
            pDownmixer->type = (downmix_type_t) value16;
        break;

      default:
        ALOGE("Downmix_setParameter unknown parameter %" PRId32, param);
        return -EINVAL;
    }
}

    return 0;
} /* end Downmix_setParameter */


/*----------------------------------------------------------------------------
 * Downmix_getParameter()
 *----------------------------------------------------------------------------
 * Purpose:
 * Get a Downmix parameter
 *
 * Inputs:
 *  pDownmixer    handle to instance data
 *  param         parameter
 *  pValue        pointer to variable to hold retrieved value
 *  pSize         pointer to value size: maximum size as input
 *
 * Outputs:
 *  *pValue updated with parameter value
 *  *pSize updated with actual value size
 *
 * Returns:
 *  0             indicates success
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
 */
int Downmix_getParameter(downmix_object_t *pDownmixer, int32_t param, uint32_t *pSize, void *pValue) {
    int16_t *pValue16;

    switch (param) {

    case DOWNMIX_PARAM_TYPE:
      if (*pSize < sizeof(int16_t)) {
          ALOGE("Downmix_getParameter invalid parameter size %" PRIu32 " for DOWNMIX_PARAM_TYPE", *pSize);
          return -EINVAL;
      }
      pValue16 = (int16_t *)pValue;
      *pValue16 = (int16_t) pDownmixer->type;
      *pSize = sizeof(int16_t);
      ALOGV("Downmix_getParameter DOWNMIX_PARAM_TYPE is %" PRId16, *pValue16);
      break;

    default:
      ALOGE("Downmix_getParameter unknown parameter %" PRId16, param);
      return -EINVAL;
    }

    return 0;
} /* end Downmix_getParameter */


/*----------------------------------------------------------------------------
 * Downmix_foldFromQuad()
 *----------------------------------------------------------------------------
 * Purpose:
 * downmix a quad signal to stereo
 *
 * Inputs:
 *  pSrc       quad audio samples to downmix
 *  numFrames  the number of quad frames to downmix
 *  accumulate whether to mix (when true) the result of the downmix with the contents of pDst,
 *               or overwrite pDst (when false)
 *
 * Outputs:
 *  pDst       downmixed stereo audio samples
 *
 *----------------------------------------------------------------------------
 */
#ifndef BUILD_FLOAT
void Downmix_foldFromQuad(int16_t *pSrc, int16_t*pDst, size_t numFrames, bool accumulate) {
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is RL
    // sample at index 3 is RR
    if (accumulate) {
        while (numFrames) {
            // FL + RL
            pDst[0] = clamp16(pDst[0] + ((pSrc[0] + pSrc[2]) >> 1));
            // FR + RR
            pDst[1] = clamp16(pDst[1] + ((pSrc[1] + pSrc[3]) >> 1));
            pSrc += 4;
            pDst += 2;
            numFrames--;
        }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // FL + RL
            pDst[0] = clamp16((pSrc[0] + pSrc[2]) >> 1);
            // FR + RR
            pDst[1] = clamp16((pSrc[1] + pSrc[3]) >> 1);
            pSrc += 4;
            pDst += 2;
            numFrames--;
        }
    }
}
#else
void Downmix_foldFromQuad(LVM_FLOAT *pSrc, LVM_FLOAT *pDst, size_t numFrames, bool accumulate) {
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is RL
    // sample at index 3 is RR
    if (accumulate) {
        while (numFrames) {
            // FL + RL
            pDst[0] = clamp_float(pDst[0] + ((pSrc[0] + pSrc[2]) / 2.0f));
            // FR + RR
            pDst[1] = clamp_float(pDst[1] + ((pSrc[1] + pSrc[3]) / 2.0f));
            pSrc += 4;
            pDst += 2;
            numFrames--;
        }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // FL + RL
            pDst[0] = clamp_float((pSrc[0] + pSrc[2]) / 2.0f);
            // FR + RR
            pDst[1] = clamp_float((pSrc[1] + pSrc[3]) / 2.0f);
            pSrc += 4;
            pDst += 2;
            numFrames--;
        }
    }
}
#endif

/*----------------------------------------------------------------------------
 * Downmix_foldFrom5Point1()
 *----------------------------------------------------------------------------
 * Purpose:
 * downmix a 5.1 signal to stereo
 *
 * Inputs:
 *  pSrc       5.1 audio samples to downmix
 *  numFrames  the number of 5.1 frames to downmix
 *  accumulate whether to mix (when true) the result of the downmix with the contents of pDst,
 *               or overwrite pDst (when false)
 *
 * Outputs:
 *  pDst       downmixed stereo audio samples
 *
 *----------------------------------------------------------------------------
 */
#ifndef BUILD_FLOAT
void Downmix_foldFrom5Point1(int16_t *pSrc, int16_t*pDst, size_t numFrames, bool accumulate) {
    int32_t lt, rt, centerPlusLfeContrib; // samples in Q19.12 format
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is FC
    // sample at index 3 is LFE
    // sample at index 4 is RL
    // sample at index 5 is RR
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_Q19_12)
                    + (pSrc[3] * MINUS_3_DB_IN_Q19_12);
            // FL + centerPlusLfeContrib + RL
            lt = (pSrc[0] << 12) + centerPlusLfeContrib + (pSrc[4] << 12);
            // FR + centerPlusLfeContrib + RR
            rt = (pSrc[1] << 12) + centerPlusLfeContrib + (pSrc[5] << 12);
            // accumulate in destination
            pDst[0] = clamp16(pDst[0] + (lt >> 13));
            pDst[1] = clamp16(pDst[1] + (rt >> 13));
            pSrc += 6;
            pDst += 2;
            numFrames--;
        }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_Q19_12)
                    + (pSrc[3] * MINUS_3_DB_IN_Q19_12);
            // FL + centerPlusLfeContrib + RL
            lt = (pSrc[0] << 12) + centerPlusLfeContrib + (pSrc[4] << 12);
            // FR + centerPlusLfeContrib + RR
            rt = (pSrc[1] << 12) + centerPlusLfeContrib + (pSrc[5] << 12);
            // store in destination
            pDst[0] = clamp16(lt >> 13); // differs from when accumulate is true above
            pDst[1] = clamp16(rt >> 13); // differs from when accumulate is true above
            pSrc += 6;
            pDst += 2;
            numFrames--;
        }
    }
}
#else
void Downmix_foldFrom5Point1(LVM_FLOAT *pSrc, LVM_FLOAT *pDst, size_t numFrames, bool accumulate) {
    LVM_FLOAT lt, rt, centerPlusLfeContrib; // samples in Q19.12 format
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is FC
    // sample at index 3 is LFE
    // sample at index 4 is RL
    // sample at index 5 is RR
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_FLOAT)
                    + (pSrc[3] * MINUS_3_DB_IN_FLOAT);
            // FL + centerPlusLfeContrib + RL
            lt = pSrc[0] + centerPlusLfeContrib + pSrc[4];
            // FR + centerPlusLfeContrib + RR
            rt = pSrc[1] + centerPlusLfeContrib + pSrc[5];
            // accumulate in destination
            pDst[0] = clamp_float(pDst[0] + (lt / 2.0f));
            pDst[1] = clamp_float(pDst[1] + (rt / 2.0f));
            pSrc += 6;
            pDst += 2;
            numFrames--;
        }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_FLOAT)
                    + (pSrc[3] * MINUS_3_DB_IN_FLOAT);
            // FL + centerPlusLfeContrib + RL
            lt = pSrc[0] + centerPlusLfeContrib + pSrc[4];
            // FR + centerPlusLfeContrib + RR
            rt = pSrc[1] + centerPlusLfeContrib + pSrc[5];
            // store in destination
            pDst[0] = clamp_float(lt / 2.0f); // differs from when accumulate is true above
            pDst[1] = clamp_float(rt / 2.0f); // differs from when accumulate is true above
            pSrc += 6;
            pDst += 2;
            numFrames--;
        }
    }
}
#endif

/*----------------------------------------------------------------------------
 * Downmix_foldFrom7Point1()
 *----------------------------------------------------------------------------
 * Purpose:
 * downmix a 7.1 signal to stereo
 *
 * Inputs:
 *  pSrc       7.1 audio samples to downmix
 *  numFrames  the number of 7.1 frames to downmix
 *  accumulate whether to mix (when true) the result of the downmix with the contents of pDst,
 *               or overwrite pDst (when false)
 *
 * Outputs:
 *  pDst       downmixed stereo audio samples
 *
 *----------------------------------------------------------------------------
 */
#ifndef BUILD_FLOAT
void Downmix_foldFrom7Point1(int16_t *pSrc, int16_t*pDst, size_t numFrames, bool accumulate) {
    int32_t lt, rt, centerPlusLfeContrib; // samples in Q19.12 format
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is FC
    // sample at index 3 is LFE
    // sample at index 4 is RL
    // sample at index 5 is RR
    // sample at index 6 is SL
    // sample at index 7 is SR
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_Q19_12)
                    + (pSrc[3] * MINUS_3_DB_IN_Q19_12);
            // FL + centerPlusLfeContrib + SL + RL
            lt = (pSrc[0] << 12) + centerPlusLfeContrib + (pSrc[6] << 12) + (pSrc[4] << 12);
            // FR + centerPlusLfeContrib + SR + RR
            rt = (pSrc[1] << 12) + centerPlusLfeContrib + (pSrc[7] << 12) + (pSrc[5] << 12);
            //accumulate in destination
            pDst[0] = clamp16(pDst[0] + (lt >> 13));
            pDst[1] = clamp16(pDst[1] + (rt >> 13));
            pSrc += 8;
            pDst += 2;
            numFrames--;
    }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_Q19_12)
                    + (pSrc[3] * MINUS_3_DB_IN_Q19_12);
            // FL + centerPlusLfeContrib + SL + RL
            lt = (pSrc[0] << 12) + centerPlusLfeContrib + (pSrc[6] << 12) + (pSrc[4] << 12);
            // FR + centerPlusLfeContrib + SR + RR
            rt = (pSrc[1] << 12) + centerPlusLfeContrib + (pSrc[7] << 12) + (pSrc[5] << 12);
            // store in destination
            pDst[0] = clamp16(lt >> 13); // differs from when accumulate is true above
            pDst[1] = clamp16(rt >> 13); // differs from when accumulate is true above
            pSrc += 8;
            pDst += 2;
            numFrames--;
        }
    }
}
#else
void Downmix_foldFrom7Point1(LVM_FLOAT *pSrc, LVM_FLOAT *pDst, size_t numFrames, bool accumulate) {
    LVM_FLOAT lt, rt, centerPlusLfeContrib; // samples in Q19.12 format
    // sample at index 0 is FL
    // sample at index 1 is FR
    // sample at index 2 is FC
    // sample at index 3 is LFE
    // sample at index 4 is RL
    // sample at index 5 is RR
    // sample at index 6 is SL
    // sample at index 7 is SR
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_FLOAT)
                    + (pSrc[3] * MINUS_3_DB_IN_FLOAT);
            // FL + centerPlusLfeContrib + SL + RL
            lt = pSrc[0] + centerPlusLfeContrib + pSrc[6] + pSrc[4];
            // FR + centerPlusLfeContrib + SR + RR
            rt = pSrc[1] + centerPlusLfeContrib + pSrc[7] + pSrc[5];
            //accumulate in destination
            pDst[0] = clamp_float(pDst[0] + (lt / 2.0f));
            pDst[1] = clamp_float(pDst[1] + (rt / 2.0f));
            pSrc += 8;
            pDst += 2;
            numFrames--;
        }
    } else { // same code as above but without adding and clamping pDst[i] to itself
        while (numFrames) {
            // centerPlusLfeContrib = FC(-3dB) + LFE(-3dB)
            centerPlusLfeContrib = (pSrc[2] * MINUS_3_DB_IN_FLOAT)
                    + (pSrc[3] * MINUS_3_DB_IN_FLOAT);
            // FL + centerPlusLfeContrib + SL + RL
            lt = pSrc[0] + centerPlusLfeContrib + pSrc[6] + pSrc[4];
            // FR + centerPlusLfeContrib + SR + RR
            rt = pSrc[1] + centerPlusLfeContrib + pSrc[7] + pSrc[5];
            // store in destination
            pDst[0] = clamp_float(lt / 2.0f); // differs from when accumulate is true above
            pDst[1] = clamp_float(rt / 2.0f); // differs from when accumulate is true above
            pSrc += 8;
            pDst += 2;
            numFrames--;
        }
    }
}
#endif
/*----------------------------------------------------------------------------
 * Downmix_foldGeneric()
 *----------------------------------------------------------------------------
 * Purpose:
 * downmix to stereo a multichannel signal whose format is:
 *  - has FL/FR
 *  - if using AUDIO_CHANNEL_OUT_SIDE*, it contains both left and right
 *  - if using AUDIO_CHANNEL_OUT_BACK*, it contains both left and right
 *  - doesn't use any of the AUDIO_CHANNEL_OUT_TOP* channels
 *  - doesn't use any of the AUDIO_CHANNEL_OUT_FRONT_*_OF_CENTER channels
 * Only handles channel masks not enumerated in downmix_input_channel_mask_t
 *
 * Inputs:
 *  mask       the channel mask of pSrc
 *  pSrc       multichannel audio buffer to downmix
 *  numFrames  the number of multichannel frames to downmix
 *  accumulate whether to mix (when true) the result of the downmix with the contents of pDst,
 *               or overwrite pDst (when false)
 *
 * Outputs:
 *  pDst       downmixed stereo audio samples
 *
 * Returns: false if multichannel format is not supported
 *
 *----------------------------------------------------------------------------
 */
#ifndef BUILD_FLOAT
bool Downmix_foldGeneric(
        uint32_t mask, int16_t *pSrc, int16_t*pDst, size_t numFrames, bool accumulate) {

    if (!Downmix_validChannelMask(mask)) {
        return false;
    }

    const bool hasSides = (mask & kSides) != 0;
    const bool hasBacks = (mask & kBacks) != 0;

    const int numChan = audio_channel_count_from_out_mask(mask);
    const bool hasFC = ((mask & AUDIO_CHANNEL_OUT_FRONT_CENTER) == AUDIO_CHANNEL_OUT_FRONT_CENTER);
    const bool hasLFE =
            ((mask & AUDIO_CHANNEL_OUT_LOW_FREQUENCY) == AUDIO_CHANNEL_OUT_LOW_FREQUENCY);
    const bool hasBC = ((mask & AUDIO_CHANNEL_OUT_BACK_CENTER) == AUDIO_CHANNEL_OUT_BACK_CENTER);
    // compute at what index each channel is: samples will be in the following order:
    //   FL FR FC LFE BL BR BC SL SR
    // when a channel is not present, its index is set to the same as the index of the preceding
    // channel
    const int indexFC  = hasFC    ? 2            : 1;        // front center
    const int indexLFE = hasLFE   ? indexFC + 1  : indexFC;  // low frequency
    const int indexBL  = hasBacks ? indexLFE + 1 : indexLFE; // back left
    const int indexBR  = hasBacks ? indexBL + 1  : indexBL;  // back right
    const int indexBC  = hasBC    ? indexBR + 1  : indexBR;  // back center
    const int indexSL  = hasSides ? indexBC + 1  : indexBC;  // side left
    const int indexSR  = hasSides ? indexSL + 1  : indexSL;  // side right

    int32_t lt, rt, centersLfeContrib; // samples in Q19.12 format
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // compute contribution of FC, BC and LFE
            centersLfeContrib = 0;
            if (hasFC)  { centersLfeContrib += pSrc[indexFC]; }
            if (hasLFE) { centersLfeContrib += pSrc[indexLFE]; }
            if (hasBC)  { centersLfeContrib += pSrc[indexBC]; }
            centersLfeContrib *= MINUS_3_DB_IN_Q19_12;
            // always has FL/FR
            lt = (pSrc[0] << 12);
            rt = (pSrc[1] << 12);
            // mix in sides and backs
            if (hasSides) {
                lt += pSrc[indexSL] << 12;
                rt += pSrc[indexSR] << 12;
            }
            if (hasBacks) {
                lt += pSrc[indexBL] << 12;
                rt += pSrc[indexBR] << 12;
            }
            lt += centersLfeContrib;
            rt += centersLfeContrib;
            // accumulate in destination
            pDst[0] = clamp16(pDst[0] + (lt >> 13));
            pDst[1] = clamp16(pDst[1] + (rt >> 13));
            pSrc += numChan;
            pDst += 2;
            numFrames--;
        }
    } else {
        while (numFrames) {
            // compute contribution of FC, BC and LFE
            centersLfeContrib = 0;
            if (hasFC)  { centersLfeContrib += pSrc[indexFC]; }
            if (hasLFE) { centersLfeContrib += pSrc[indexLFE]; }
            if (hasBC)  { centersLfeContrib += pSrc[indexBC]; }
            centersLfeContrib *= MINUS_3_DB_IN_Q19_12;
            // always has FL/FR
            lt = (pSrc[0] << 12);
            rt = (pSrc[1] << 12);
            // mix in sides and backs
            if (hasSides) {
                lt += pSrc[indexSL] << 12;
                rt += pSrc[indexSR] << 12;
            }
            if (hasBacks) {
                lt += pSrc[indexBL] << 12;
                rt += pSrc[indexBR] << 12;
            }
            lt += centersLfeContrib;
            rt += centersLfeContrib;
            // store in destination
            pDst[0] = clamp16(lt >> 13); // differs from when accumulate is true above
            pDst[1] = clamp16(rt >> 13); // differs from when accumulate is true above
            pSrc += numChan;
            pDst += 2;
            numFrames--;
        }
    }
    return true;
}
#else
bool Downmix_foldGeneric(
        uint32_t mask, LVM_FLOAT *pSrc, LVM_FLOAT *pDst, size_t numFrames, bool accumulate) {

    if (!Downmix_validChannelMask(mask)) {
        return false;
    }

    const bool hasSides = (mask & kSides) != 0;
    const bool hasBacks = (mask & kBacks) != 0;

    const int numChan = audio_channel_count_from_out_mask(mask);
    const bool hasFC = ((mask & AUDIO_CHANNEL_OUT_FRONT_CENTER) == AUDIO_CHANNEL_OUT_FRONT_CENTER);
    const bool hasLFE =
            ((mask & AUDIO_CHANNEL_OUT_LOW_FREQUENCY) == AUDIO_CHANNEL_OUT_LOW_FREQUENCY);
    const bool hasBC = ((mask & AUDIO_CHANNEL_OUT_BACK_CENTER) == AUDIO_CHANNEL_OUT_BACK_CENTER);
    // compute at what index each channel is: samples will be in the following order:
    //   FL FR FC LFE BL BR BC SL SR
    // when a channel is not present, its index is set to the same as the index of the preceding
    // channel
    const int indexFC  = hasFC    ? 2            : 1;        // front center
    const int indexLFE = hasLFE   ? indexFC + 1  : indexFC;  // low frequency
    const int indexBL  = hasBacks ? indexLFE + 1 : indexLFE; // back left
    const int indexBR  = hasBacks ? indexBL + 1  : indexBL;  // back right
    const int indexBC  = hasBC    ? indexBR + 1  : indexBR;  // back center
    const int indexSL  = hasSides ? indexBC + 1  : indexBC;  // side left
    const int indexSR  = hasSides ? indexSL + 1  : indexSL;  // side right

    LVM_FLOAT lt, rt, centersLfeContrib;
    // code is mostly duplicated between the two values of accumulate to avoid repeating the test
    // for every sample
    if (accumulate) {
        while (numFrames) {
            // compute contribution of FC, BC and LFE
            centersLfeContrib = 0;
            if (hasFC)  { centersLfeContrib += pSrc[indexFC]; }
            if (hasLFE) { centersLfeContrib += pSrc[indexLFE]; }
            if (hasBC)  { centersLfeContrib += pSrc[indexBC]; }
            centersLfeContrib *= MINUS_3_DB_IN_FLOAT;
            // always has FL/FR
            lt = pSrc[0];
            rt = pSrc[1];
            // mix in sides and backs
            if (hasSides) {
                lt += pSrc[indexSL];
                rt += pSrc[indexSR];
            }
            if (hasBacks) {
                lt += pSrc[indexBL];
                rt += pSrc[indexBR];
            }
            lt += centersLfeContrib;
            rt += centersLfeContrib;
            // accumulate in destination
            pDst[0] = clamp_float(pDst[0] + (lt / 2.0f));
            pDst[1] = clamp_float(pDst[1] + (rt / 2.0f));
            pSrc += numChan;
            pDst += 2;
            numFrames--;
        }
    } else {
        while (numFrames) {
            // compute contribution of FC, BC and LFE
            centersLfeContrib = 0;
            if (hasFC)  { centersLfeContrib += pSrc[indexFC]; }
            if (hasLFE) { centersLfeContrib += pSrc[indexLFE]; }
            if (hasBC)  { centersLfeContrib += pSrc[indexBC]; }
            centersLfeContrib *= MINUS_3_DB_IN_FLOAT;
            // always has FL/FR
            lt = pSrc[0];
            rt = pSrc[1];
            // mix in sides and backs
            if (hasSides) {
                lt += pSrc[indexSL];
                rt += pSrc[indexSR];
            }
            if (hasBacks) {
                lt += pSrc[indexBL];
                rt += pSrc[indexBR];
            }
            lt += centersLfeContrib;
            rt += centersLfeContrib;
            // store in destination
            pDst[0] = clamp_float(lt / 2.0f); // differs from when accumulate is true above
            pDst[1] = clamp_float(rt / 2.0f); // differs from when accumulate is true above
            pSrc += numChan;
            pDst += 2;
            numFrames--;
        }
    }
    return true;
}
#endif
