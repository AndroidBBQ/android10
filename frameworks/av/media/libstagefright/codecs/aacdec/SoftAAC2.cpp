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

//#define LOG_NDEBUG 0
#define LOG_TAG "SoftAAC2"
#include <utils/Log.h>

#include "SoftAAC2.h"
#include <OMX_AudioExt.h>
#include <OMX_IndexExt.h>

#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/misc.h>

#include <math.h>

#define FILEREAD_MAX_LAYERS 2

#define DRC_DEFAULT_MOBILE_REF_LEVEL 64  /* 64*-0.25dB = -16 dB below full scale for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_CUT   127 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_BOOST 127 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_HEAVY 1   /* switch for heavy compression for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_EFFECT 3  /* MPEG-D DRC effect type; 3 => Limited playback range */
#define DRC_DEFAULT_MOBILE_ENC_LEVEL (-1) /* encoder target level; -1 => the value is unknown, otherwise dB step value (e.g. 64 for -16 dB) */
#define MAX_CHANNEL_COUNT            8  /* maximum number of audio channels that can be decoded */
// names of properties that can be used to override the default DRC settings
#define PROP_DRC_OVERRIDE_REF_LEVEL  "aac_drc_reference_level"
#define PROP_DRC_OVERRIDE_CUT        "aac_drc_cut"
#define PROP_DRC_OVERRIDE_BOOST      "aac_drc_boost"
#define PROP_DRC_OVERRIDE_HEAVY      "aac_drc_heavy"
#define PROP_DRC_OVERRIDE_ENC_LEVEL "aac_drc_enc_target_level"
#define PROP_DRC_OVERRIDE_EFFECT     "ro.aac_drc_effect_type"

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

static const OMX_U32 kSupportedProfiles[] = {
    OMX_AUDIO_AACObjectLC,
    OMX_AUDIO_AACObjectHE,
    OMX_AUDIO_AACObjectHE_PS,
    OMX_AUDIO_AACObjectLD,
    OMX_AUDIO_AACObjectELD,
    OMX_AUDIO_AACObjectER_Scalable,
    OMX_AUDIO_AACObjectXHE,
};

SoftAAC2::SoftAAC2(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SimpleSoftOMXComponent(name, callbacks, appData, component),
      mAACDecoder(NULL),
      mStreamInfo(NULL),
      mIsADTS(false),
      mInputBufferCount(0),
      mOutputBufferCount(0),
      mSignalledError(false),
      mLastInHeader(NULL),
      mOutputPortSettingsChange(NONE) {
    initPorts();
    CHECK_EQ(initDecoder(), (status_t)OK);
}

SoftAAC2::~SoftAAC2() {
    aacDecoder_Close(mAACDecoder);
    delete[] mOutputDelayRingBuffer;
}

void SoftAAC2::initPorts() {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    def.nPortIndex = 0;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = kNumInputBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = 8192;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.audio.cMIMEType = const_cast<char *>("audio/aac");
    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingAAC;

    addPort(def);

    def.nPortIndex = 1;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = kNumOutputBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = 4096 * MAX_CHANNEL_COUNT;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 2;

    def.format.audio.cMIMEType = const_cast<char *>("audio/raw");
    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    addPort(def);
}

status_t SoftAAC2::initDecoder() {
    ALOGV("initDecoder()");
    status_t status = UNKNOWN_ERROR;
    mAACDecoder = aacDecoder_Open(TT_MP4_ADIF, /* num layers */ 1);
    if (mAACDecoder != NULL) {
        mStreamInfo = aacDecoder_GetStreamInfo(mAACDecoder);
        if (mStreamInfo != NULL) {
            status = OK;
        }
    }

    mEndOfInput = false;
    mEndOfOutput = false;
    mOutputDelayCompensated = 0;
    mOutputDelayRingBufferSize = 2048 * MAX_CHANNEL_COUNT * kNumDelayBlocksMax;
    mOutputDelayRingBuffer = new int16_t[mOutputDelayRingBufferSize];
    mOutputDelayRingBufferWritePos = 0;
    mOutputDelayRingBufferReadPos = 0;
    mOutputDelayRingBufferFilled = 0;

    if (mAACDecoder == NULL) {
        ALOGE("AAC decoder is null. TODO: Can not call aacDecoder_SetParam in the following code");
    }

    //aacDecoder_SetParam(mAACDecoder, AAC_PCM_LIMITER_ENABLE, 0);

    //init DRC wrapper
    mDrcWrap.setDecoderHandle(mAACDecoder);
    mDrcWrap.submitStreamData(mStreamInfo);

    // for streams that contain metadata, use the mobile profile DRC settings unless overridden by platform properties
    // TODO: change the DRC settings depending on audio output device type (HDMI, loadspeaker, headphone)
    char value[PROPERTY_VALUE_MAX];
    //  DRC_PRES_MODE_WRAP_DESIRED_TARGET
    if (property_get(PROP_DRC_OVERRIDE_REF_LEVEL, value, NULL)) {
        unsigned refLevel = atoi(value);
        ALOGV("AAC decoder using desired DRC target reference level of %d instead of %d", refLevel,
                DRC_DEFAULT_MOBILE_REF_LEVEL);
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_TARGET, refLevel);
    } else {
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_TARGET, DRC_DEFAULT_MOBILE_REF_LEVEL);
    }
    //  DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR
    if (property_get(PROP_DRC_OVERRIDE_CUT, value, NULL)) {
        unsigned cut = atoi(value);
        ALOGV("AAC decoder using desired DRC attenuation factor of %d instead of %d", cut,
                DRC_DEFAULT_MOBILE_DRC_CUT);
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR, cut);
    } else {
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR, DRC_DEFAULT_MOBILE_DRC_CUT);
    }
    //  DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR
    if (property_get(PROP_DRC_OVERRIDE_BOOST, value, NULL)) {
        unsigned boost = atoi(value);
        ALOGV("AAC decoder using desired DRC boost factor of %d instead of %d", boost,
                DRC_DEFAULT_MOBILE_DRC_BOOST);
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR, boost);
    } else {
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR, DRC_DEFAULT_MOBILE_DRC_BOOST);
    }
    //  DRC_PRES_MODE_WRAP_DESIRED_HEAVY
    if (property_get(PROP_DRC_OVERRIDE_HEAVY, value, NULL)) {
        unsigned heavy = atoi(value);
        ALOGV("AAC decoder using desried DRC heavy compression switch of %d instead of %d", heavy,
                DRC_DEFAULT_MOBILE_DRC_HEAVY);
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_HEAVY, heavy);
    } else {
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_HEAVY, DRC_DEFAULT_MOBILE_DRC_HEAVY);
    }
    // DRC_PRES_MODE_WRAP_ENCODER_TARGET
    if (property_get(PROP_DRC_OVERRIDE_ENC_LEVEL, value, NULL)) {
        unsigned encoderRefLevel = atoi(value);
        ALOGV("AAC decoder using encoder-side DRC reference level of %d instead of %d",
                encoderRefLevel, DRC_DEFAULT_MOBILE_ENC_LEVEL);
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_ENCODER_TARGET, encoderRefLevel);
    } else {
        mDrcWrap.setParam(DRC_PRES_MODE_WRAP_ENCODER_TARGET, DRC_DEFAULT_MOBILE_ENC_LEVEL);
    }
    // AAC_UNIDRC_SET_EFFECT
    int32_t effectType =
            property_get_int32(PROP_DRC_OVERRIDE_EFFECT, DRC_DEFAULT_MOBILE_DRC_EFFECT);
    if (effectType < -1 || effectType > 8) {
        effectType = DRC_DEFAULT_MOBILE_DRC_EFFECT;
    }
    ALOGV("AAC decoder using MPEG-D DRC effect type %d (default=%d)",
            effectType, DRC_DEFAULT_MOBILE_DRC_EFFECT);
    aacDecoder_SetParam(mAACDecoder, AAC_UNIDRC_SET_EFFECT, effectType);

    // By default, the decoder creates a 5.1 channel downmix signal.
    // For seven and eight channel input streams, enable 6.1 and 7.1 channel output
    aacDecoder_SetParam(mAACDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, -1);

    return status;
}

OMX_ERRORTYPE SoftAAC2::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch ((OMX_U32) index) {
        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *formatParams =
                (OMX_AUDIO_PARAM_PORTFORMATTYPE *)params;

            if (!isValidOMXParam(formatParams)) {
                return OMX_ErrorBadParameter;
            }

            if (formatParams->nPortIndex > 1) {
                return OMX_ErrorUndefined;
            }

            if (formatParams->nIndex > 0) {
                return OMX_ErrorNoMore;
            }

            formatParams->eEncoding =
                (formatParams->nPortIndex == 0)
                    ? OMX_AUDIO_CodingAAC : OMX_AUDIO_CodingPCM;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *aacParams =
                (OMX_AUDIO_PARAM_AACPROFILETYPE *)params;

            if (!isValidOMXParam(aacParams)) {
                return OMX_ErrorBadParameter;
            }

            if (aacParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            aacParams->nBitRate = 0;
            aacParams->nAudioBandWidth = 0;
            aacParams->nAACtools = 0;
            aacParams->nAACERtools = 0;
            aacParams->eAACProfile = OMX_AUDIO_AACObjectMain;

            aacParams->eAACStreamFormat =
                mIsADTS
                    ? OMX_AUDIO_AACStreamFormatMP4ADTS
                    : OMX_AUDIO_AACStreamFormatMP4FF;

            aacParams->eChannelMode = OMX_AUDIO_ChannelModeStereo;

            if (!isConfigured()) {
                aacParams->nChannels = 1;
                aacParams->nSampleRate = 44100;
                aacParams->nFrameLength = 0;
            } else {
                aacParams->nChannels = mStreamInfo->numChannels;
                aacParams->nSampleRate = mStreamInfo->sampleRate;
                aacParams->nFrameLength = mStreamInfo->frameSize;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
                (OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            pcmParams->eNumData = OMX_NumericalDataSigned;
            pcmParams->eEndian = OMX_EndianBig;
            pcmParams->bInterleaved = OMX_TRUE;
            pcmParams->nBitPerSample = 16;
            pcmParams->ePCMMode = OMX_AUDIO_PCMModeLinear;
            pcmParams->eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcmParams->eChannelMapping[1] = OMX_AUDIO_ChannelRF;
            pcmParams->eChannelMapping[2] = OMX_AUDIO_ChannelCF;
            pcmParams->eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
            pcmParams->eChannelMapping[4] = OMX_AUDIO_ChannelLS;
            pcmParams->eChannelMapping[5] = OMX_AUDIO_ChannelRS;

            if (!isConfigured()) {
                pcmParams->nChannels = 1;
                pcmParams->nSamplingRate = 44100;
            } else {
                pcmParams->nChannels = mStreamInfo->numChannels;
                pcmParams->nSamplingRate = mStreamInfo->sampleRate;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioProfileQuerySupported:
        {
            OMX_AUDIO_PARAM_ANDROID_PROFILETYPE *profileParams =
                (OMX_AUDIO_PARAM_ANDROID_PROFILETYPE *)params;

            if (!isValidOMXParam(profileParams)) {
                return OMX_ErrorBadParameter;
            }

            if (profileParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            if (profileParams->nProfileIndex >= NELEM(kSupportedProfiles)) {
                return OMX_ErrorNoMore;
            }

            profileParams->eProfile =
                kSupportedProfiles[profileParams->nProfileIndex];

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftAAC2::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params) {
    switch ((int)index) {
        case OMX_IndexParamStandardComponentRole:
        {
            const OMX_PARAM_COMPONENTROLETYPE *roleParams =
                (const OMX_PARAM_COMPONENTROLETYPE *)params;

            if (!isValidOMXParam(roleParams)) {
                return OMX_ErrorBadParameter;
            }

            if (strncmp((const char *)roleParams->cRole,
                        "audio_decoder.aac",
                        OMX_MAX_STRINGNAME_SIZE - 1)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            const OMX_AUDIO_PARAM_PORTFORMATTYPE *formatParams =
                (const OMX_AUDIO_PARAM_PORTFORMATTYPE *)params;

            if (!isValidOMXParam(formatParams)) {
                return OMX_ErrorBadParameter;
            }

            if (formatParams->nPortIndex > 1) {
                return OMX_ErrorUndefined;
            }

            if ((formatParams->nPortIndex == 0
                        && formatParams->eEncoding != OMX_AUDIO_CodingAAC)
                || (formatParams->nPortIndex == 1
                        && formatParams->eEncoding != OMX_AUDIO_CodingPCM)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioAac:
        {
            const OMX_AUDIO_PARAM_AACPROFILETYPE *aacParams =
                (const OMX_AUDIO_PARAM_AACPROFILETYPE *)params;

            if (!isValidOMXParam(aacParams)) {
                return OMX_ErrorBadParameter;
            }

            if (aacParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            if (aacParams->eAACStreamFormat == OMX_AUDIO_AACStreamFormatMP4FF) {
                mIsADTS = false;
            } else if (aacParams->eAACStreamFormat
                        == OMX_AUDIO_AACStreamFormatMP4ADTS) {
                mIsADTS = true;
            } else {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioAndroidAacDrcPresentation:
        {
            const OMX_AUDIO_PARAM_ANDROID_AACDRCPRESENTATIONTYPE *aacPresParams =
                    (const OMX_AUDIO_PARAM_ANDROID_AACDRCPRESENTATIONTYPE *)params;

            if (!isValidOMXParam(aacPresParams)) {
                return OMX_ErrorBadParameter;
            }

            // for the following parameters of the OMX_AUDIO_PARAM_AACPROFILETYPE structure,
            // a value of -1 implies the parameter is not set by the application:
            //   nMaxOutputChannels     -1 by default 
            //   nDrcCut                uses default platform properties, see initDecoder()
            //   nDrcBoost                idem
            //   nHeavyCompression        idem
            //   nTargetReferenceLevel    idem
            //   nEncodedTargetLevel      idem
            if (aacPresParams->nMaxOutputChannels >= 0) {
                int max;
                if (aacPresParams->nMaxOutputChannels >= 8) { max = 8; }
                else if (aacPresParams->nMaxOutputChannels >= 6) { max = 6; }
                else if (aacPresParams->nMaxOutputChannels >= 2) { max = 2; }
                else {
                    // -1 or 0: disable downmix,  1: mono
                    max = aacPresParams->nMaxOutputChannels;
                }
                ALOGV("set nMaxOutputChannels=%d", max);
                aacDecoder_SetParam(mAACDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, max);
            }
            if (aacPresParams->nDrcEffectType >= -1) {
                ALOGV("set nDrcEffectType=%d", aacPresParams->nDrcEffectType);
                aacDecoder_SetParam(mAACDecoder, AAC_UNIDRC_SET_EFFECT, aacPresParams->nDrcEffectType);
            }
            bool updateDrcWrapper = false;
            if (aacPresParams->nDrcBoost >= 0) {
                ALOGV("set nDrcBoost=%d", aacPresParams->nDrcBoost);
                mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR,
                        aacPresParams->nDrcBoost);
                updateDrcWrapper = true;
            }
            if (aacPresParams->nDrcCut >= 0) {
                ALOGV("set nDrcCut=%d", aacPresParams->nDrcCut);
                mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR, aacPresParams->nDrcCut);
                updateDrcWrapper = true;
            }
            if (aacPresParams->nHeavyCompression >= 0) {
                ALOGV("set nHeavyCompression=%d", aacPresParams->nHeavyCompression);
                mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_HEAVY,
                        aacPresParams->nHeavyCompression);
                updateDrcWrapper = true;
            }
            if (aacPresParams->nTargetReferenceLevel >= 0) {
                ALOGV("set nTargetReferenceLevel=%d", aacPresParams->nTargetReferenceLevel);
                mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_TARGET,
                        aacPresParams->nTargetReferenceLevel);
                updateDrcWrapper = true;
            }
            if (aacPresParams->nEncodedTargetLevel >= 0) {
                ALOGV("set nEncodedTargetLevel=%d", aacPresParams->nEncodedTargetLevel);
                mDrcWrap.setParam(DRC_PRES_MODE_WRAP_ENCODER_TARGET,
                        aacPresParams->nEncodedTargetLevel);
                updateDrcWrapper = true;
            }
            if (aacPresParams->nPCMLimiterEnable >= 0) {
                aacDecoder_SetParam(mAACDecoder, AAC_PCM_LIMITER_ENABLE,
                        (aacPresParams->nPCMLimiterEnable != 0));
            }
            if (updateDrcWrapper) {
                mDrcWrap.update();
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            const OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
                (OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

bool SoftAAC2::isConfigured() const {
    return mInputBufferCount > 0;
}

bool SoftAAC2::outputDelayRingBufferPutSamples(INT_PCM *samples, int32_t numSamples) {
    if (numSamples == 0) {
        return true;
    }
    if (outputDelayRingBufferSpaceLeft() < numSamples) {
        ALOGE("RING BUFFER WOULD OVERFLOW");
        return false;
    }
    if (mOutputDelayRingBufferWritePos + numSamples <= mOutputDelayRingBufferSize
            && (mOutputDelayRingBufferReadPos <= mOutputDelayRingBufferWritePos
                    || mOutputDelayRingBufferReadPos > mOutputDelayRingBufferWritePos + numSamples)) {
        // faster memcopy loop without checks, if the preconditions allow this
        for (int32_t i = 0; i < numSamples; i++) {
            mOutputDelayRingBuffer[mOutputDelayRingBufferWritePos++] = samples[i];
        }

        if (mOutputDelayRingBufferWritePos >= mOutputDelayRingBufferSize) {
            mOutputDelayRingBufferWritePos -= mOutputDelayRingBufferSize;
        }
    } else {
        ALOGV("slow SoftAAC2::outputDelayRingBufferPutSamples()");

        for (int32_t i = 0; i < numSamples; i++) {
            mOutputDelayRingBuffer[mOutputDelayRingBufferWritePos] = samples[i];
            mOutputDelayRingBufferWritePos++;
            if (mOutputDelayRingBufferWritePos >= mOutputDelayRingBufferSize) {
                mOutputDelayRingBufferWritePos -= mOutputDelayRingBufferSize;
            }
        }
    }
    mOutputDelayRingBufferFilled += numSamples;
    return true;
}

int32_t SoftAAC2::outputDelayRingBufferGetSamples(INT_PCM *samples, int32_t numSamples) {

    if (numSamples > mOutputDelayRingBufferFilled) {
        ALOGE("RING BUFFER WOULD UNDERRUN");
        return -1;
    }

    if (mOutputDelayRingBufferReadPos + numSamples <= mOutputDelayRingBufferSize
            && (mOutputDelayRingBufferWritePos < mOutputDelayRingBufferReadPos
                    || mOutputDelayRingBufferWritePos >= mOutputDelayRingBufferReadPos + numSamples)) {
        // faster memcopy loop without checks, if the preconditions allow this
        if (samples != 0) {
            for (int32_t i = 0; i < numSamples; i++) {
                samples[i] = mOutputDelayRingBuffer[mOutputDelayRingBufferReadPos++];
            }
        } else {
            mOutputDelayRingBufferReadPos += numSamples;
        }
        if (mOutputDelayRingBufferReadPos >= mOutputDelayRingBufferSize) {
            mOutputDelayRingBufferReadPos -= mOutputDelayRingBufferSize;
        }
    } else {
        ALOGV("slow SoftAAC2::outputDelayRingBufferGetSamples()");

        for (int32_t i = 0; i < numSamples; i++) {
            if (samples != 0) {
                samples[i] = mOutputDelayRingBuffer[mOutputDelayRingBufferReadPos];
            }
            mOutputDelayRingBufferReadPos++;
            if (mOutputDelayRingBufferReadPos >= mOutputDelayRingBufferSize) {
                mOutputDelayRingBufferReadPos -= mOutputDelayRingBufferSize;
            }
        }
    }
    mOutputDelayRingBufferFilled -= numSamples;
    return numSamples;
}

int32_t SoftAAC2::outputDelayRingBufferSamplesAvailable() {
    return mOutputDelayRingBufferFilled;
}

int32_t SoftAAC2::outputDelayRingBufferSpaceLeft() {
    return mOutputDelayRingBufferSize - outputDelayRingBufferSamplesAvailable();
}


void SoftAAC2::onQueueFilled(OMX_U32 /* portIndex */) {
    if (mSignalledError || mOutputPortSettingsChange != NONE) {
        return;
    }

    UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
    UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
    UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    while ((!inQueue.empty() || mEndOfInput) && !outQueue.empty()) {
        if (!inQueue.empty()) {
            INT_PCM tmpOutBuffer[2048 * MAX_CHANNEL_COUNT];
            BufferInfo *inInfo = *inQueue.begin();
            OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;

            mEndOfInput = (inHeader->nFlags & OMX_BUFFERFLAG_EOS) != 0;

            if (mInputBufferCount == 0 && !(inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG)) {
                ALOGE("first buffer should have OMX_BUFFERFLAG_CODECCONFIG set");
                inHeader->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
            }
            if ((inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG) != 0) {
                BufferInfo *inInfo = *inQueue.begin();
                OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;

                inBuffer[0] = inHeader->pBuffer + inHeader->nOffset;
                inBufferLength[0] = inHeader->nFilledLen;

                AAC_DECODER_ERROR decoderErr =
                    aacDecoder_ConfigRaw(mAACDecoder,
                                         inBuffer,
                                         inBufferLength);

                if (decoderErr != AAC_DEC_OK) {
                    ALOGW("aacDecoder_ConfigRaw decoderErr = 0x%4.4x", decoderErr);
                    mSignalledError = true;
                    notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                    return;
                }

                mInputBufferCount++;
                mOutputBufferCount++; // fake increase of outputBufferCount to keep the counters aligned

                inInfo->mOwnedByUs = false;
                inQueue.erase(inQueue.begin());
                mLastInHeader = NULL;
                inInfo = NULL;
                notifyEmptyBufferDone(inHeader);
                inHeader = NULL;

                // Only send out port settings changed event if both sample rate
                // and numChannels are valid.
                if (mStreamInfo->sampleRate && mStreamInfo->numChannels) {
                    ALOGI("Initially configuring decoder: %d Hz, %d channels",
                        mStreamInfo->sampleRate,
                        mStreamInfo->numChannels);

                    notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
                    mOutputPortSettingsChange = AWAITING_DISABLED;
                }
                return;
            }

            if (inHeader->nFilledLen == 0) {
                inInfo->mOwnedByUs = false;
                inQueue.erase(inQueue.begin());
                mLastInHeader = NULL;
                inInfo = NULL;
                notifyEmptyBufferDone(inHeader);
                inHeader = NULL;
                continue;
            }

            if (mIsADTS) {
                size_t adtsHeaderSize = 0;
                // skip 30 bits, aac_frame_length follows.
                // ssssssss ssssiiip ppffffPc ccohCCll llllllll lll?????

                const uint8_t *adtsHeader = inHeader->pBuffer + inHeader->nOffset;

                bool signalError = false;
                if (inHeader->nFilledLen < 7) {
                    ALOGE("Audio data too short to contain even the ADTS header. "
                            "Got %d bytes.", inHeader->nFilledLen);
                    hexdump(adtsHeader, inHeader->nFilledLen);
                    signalError = true;
                } else {
                    bool protectionAbsent = (adtsHeader[1] & 1);

                    unsigned aac_frame_length =
                        ((adtsHeader[3] & 3) << 11)
                        | (adtsHeader[4] << 3)
                        | (adtsHeader[5] >> 5);

                    if (inHeader->nFilledLen < aac_frame_length) {
                        ALOGE("Not enough audio data for the complete frame. "
                                "Got %d bytes, frame size according to the ADTS "
                                "header is %u bytes.",
                                inHeader->nFilledLen, aac_frame_length);
                        hexdump(adtsHeader, inHeader->nFilledLen);
                        signalError = true;
                    } else {
                        adtsHeaderSize = (protectionAbsent ? 7 : 9);
                        if (aac_frame_length < adtsHeaderSize) {
                            signalError = true;
                        } else {
                            inBuffer[0] = (UCHAR *)adtsHeader + adtsHeaderSize;
                            inBufferLength[0] = aac_frame_length - adtsHeaderSize;

                            inHeader->nOffset += adtsHeaderSize;
                            inHeader->nFilledLen -= adtsHeaderSize;
                        }
                    }
                }

                if (signalError) {
                    mSignalledError = true;
                    notify(OMX_EventError, OMX_ErrorStreamCorrupt, ERROR_MALFORMED, NULL);
                    return;
                }

                // insert buffer size and time stamp
                mBufferSizes.add(inBufferLength[0]);
                if (mLastInHeader != inHeader) {
                    mBufferTimestamps.add(inHeader->nTimeStamp);
                    mLastInHeader = inHeader;
                } else {
                    int64_t currentTime = mBufferTimestamps.top();
                    currentTime += mStreamInfo->aacSamplesPerFrame *
                            1000000LL / mStreamInfo->aacSampleRate;
                    mBufferTimestamps.add(currentTime);
                }
            } else {
                inBuffer[0] = inHeader->pBuffer + inHeader->nOffset;
                inBufferLength[0] = inHeader->nFilledLen;
                mLastInHeader = inHeader;
                mBufferTimestamps.add(inHeader->nTimeStamp);
                mBufferSizes.add(inHeader->nFilledLen);
            }

            // Fill and decode
            bytesValid[0] = inBufferLength[0];

            INT prevSampleRate = mStreamInfo->sampleRate;
            INT prevNumChannels = mStreamInfo->numChannels;

            aacDecoder_Fill(mAACDecoder,
                            inBuffer,
                            inBufferLength,
                            bytesValid);

            // run DRC check
            mDrcWrap.submitStreamData(mStreamInfo);
            mDrcWrap.update();

            UINT inBufferUsedLength = inBufferLength[0] - bytesValid[0];
            inHeader->nFilledLen -= inBufferUsedLength;
            inHeader->nOffset += inBufferUsedLength;

            AAC_DECODER_ERROR decoderErr;
            int numLoops = 0;
            do {
                if (outputDelayRingBufferSpaceLeft() <
                        (mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                    ALOGV("skipping decode: not enough space left in ringbuffer");
                    break;
                }

                int numConsumed = mStreamInfo->numTotalBytes;
                decoderErr = aacDecoder_DecodeFrame(mAACDecoder,
                                           tmpOutBuffer,
                                           2048 * MAX_CHANNEL_COUNT,
                                           0 /* flags */);

                numConsumed = mStreamInfo->numTotalBytes - numConsumed;
                numLoops++;

                if (decoderErr == AAC_DEC_NOT_ENOUGH_BITS) {
                    break;
                }
                mDecodedSizes.add(numConsumed);

                if (decoderErr != AAC_DEC_OK) {
                    ALOGW("aacDecoder_DecodeFrame decoderErr = 0x%4.4x", decoderErr);
                }

                if (bytesValid[0] != 0) {
                    ALOGE("bytesValid[0] != 0 should never happen");
                    mSignalledError = true;
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    return;
                }

                size_t numOutBytes =
                    mStreamInfo->frameSize * sizeof(int16_t) * mStreamInfo->numChannels;

                if (decoderErr == AAC_DEC_OK) {
                    if (!outputDelayRingBufferPutSamples(tmpOutBuffer,
                            mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                        mSignalledError = true;
                        notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                        return;
                    }
                } else {
                    ALOGW("AAC decoder returned error 0x%4.4x, substituting silence", decoderErr);

                    memset(tmpOutBuffer, 0, numOutBytes); // TODO: check for overflow

                    if (!outputDelayRingBufferPutSamples(tmpOutBuffer,
                            mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                        mSignalledError = true;
                        notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                        return;
                    }

                    // Discard input buffer.
                    if (inHeader) {
                        inHeader->nFilledLen = 0;
                    }

                    aacDecoder_SetParam(mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);

                    // After an error, replace the last entry in mBufferSizes with the sum of the
                    // last <numLoops> entries from mDecodedSizes to resynchronize the in/out lists.
                    mBufferSizes.pop();
                    int n = 0;
                    for (int i = 0; i < numLoops; i++) {
                        n += mDecodedSizes.itemAt(mDecodedSizes.size() - numLoops + i);
                    }
                    mBufferSizes.add(n);

                    // fall through
                }

                /*
                 * AAC+/eAAC+ streams can be signalled in two ways: either explicitly
                 * or implicitly, according to MPEG4 spec. AAC+/eAAC+ is a dual
                 * rate system and the sampling rate in the final output is actually
                 * doubled compared with the core AAC decoder sampling rate.
                 *
                 * Explicit signalling is done by explicitly defining SBR audio object
                 * type in the bitstream. Implicit signalling is done by embedding
                 * SBR content in AAC extension payload specific to SBR, and hence
                 * requires an AAC decoder to perform pre-checks on actual audio frames.
                 *
                 * Thus, we could not say for sure whether a stream is
                 * AAC+/eAAC+ until the first data frame is decoded.
                 */
                if (!mStreamInfo->sampleRate || !mStreamInfo->numChannels) {
                    if ((mInputBufferCount > 2) && (mOutputBufferCount <= 1)) {
                        ALOGW("Invalid AAC stream");
                        mSignalledError = true;
                        notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                        return;
                    }
                } else if ((mStreamInfo->sampleRate != prevSampleRate) ||
                           (mStreamInfo->numChannels != prevNumChannels)) {
                    ALOGI("Reconfiguring decoder: %d->%d Hz, %d->%d channels",
                          prevSampleRate, mStreamInfo->sampleRate,
                          prevNumChannels, mStreamInfo->numChannels);

                    notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
                    mOutputPortSettingsChange = AWAITING_DISABLED;

                    if (inHeader && inHeader->nFilledLen == 0) {
                        inInfo->mOwnedByUs = false;
                        mInputBufferCount++;
                        inQueue.erase(inQueue.begin());
                        mLastInHeader = NULL;
                        inInfo = NULL;
                        notifyEmptyBufferDone(inHeader);
                        inHeader = NULL;
                    }
                    return;
                }
                if (inHeader && inHeader->nFilledLen == 0) {
                    inInfo->mOwnedByUs = false;
                    mInputBufferCount++;
                    inQueue.erase(inQueue.begin());
                    mLastInHeader = NULL;
                    inInfo = NULL;
                    notifyEmptyBufferDone(inHeader);
                    inHeader = NULL;
                } else {
                    ALOGV("inHeader->nFilledLen = %d", inHeader ? inHeader->nFilledLen : 0);
                }
            } while (decoderErr == AAC_DEC_OK);
        }

        int32_t outputDelay = mStreamInfo->outputDelay * mStreamInfo->numChannels;

        if (!mEndOfInput && mOutputDelayCompensated < outputDelay) {
            // discard outputDelay at the beginning
            int32_t toCompensate = outputDelay - mOutputDelayCompensated;
            int32_t discard = outputDelayRingBufferSamplesAvailable();
            if (discard > toCompensate) {
                discard = toCompensate;
            }
            int32_t discarded = outputDelayRingBufferGetSamples(0, discard);
            mOutputDelayCompensated += discarded;
            continue;
        }

        if (mEndOfInput) {
            while (mOutputDelayCompensated > 0) {
                // a buffer big enough for MAX_CHANNEL_COUNT channels of decoded HE-AAC
                INT_PCM tmpOutBuffer[2048 * MAX_CHANNEL_COUNT];

                // run DRC check
                mDrcWrap.submitStreamData(mStreamInfo);
                mDrcWrap.update();

                AAC_DECODER_ERROR decoderErr =
                    aacDecoder_DecodeFrame(mAACDecoder,
                                           tmpOutBuffer,
                                           2048 * MAX_CHANNEL_COUNT,
                                           AACDEC_FLUSH);
                if (decoderErr != AAC_DEC_OK) {
                    ALOGW("aacDecoder_DecodeFrame decoderErr = 0x%4.4x", decoderErr);
                }

                int32_t tmpOutBufferSamples = mStreamInfo->frameSize * mStreamInfo->numChannels;
                if (tmpOutBufferSamples > mOutputDelayCompensated) {
                    tmpOutBufferSamples = mOutputDelayCompensated;
                }
                outputDelayRingBufferPutSamples(tmpOutBuffer, tmpOutBufferSamples);
                mOutputDelayCompensated -= tmpOutBufferSamples;
            }
        }

        while (!outQueue.empty()
                && outputDelayRingBufferSamplesAvailable()
                        >= mStreamInfo->frameSize * mStreamInfo->numChannels) {
            BufferInfo *outInfo = *outQueue.begin();
            OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

            if (outHeader->nOffset != 0) {
                ALOGE("outHeader->nOffset != 0 is not handled");
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                return;
            }

            INT_PCM *outBuffer =
                    reinterpret_cast<INT_PCM *>(outHeader->pBuffer + outHeader->nOffset);
            int samplesize = mStreamInfo->numChannels * sizeof(int16_t);
            if (outHeader->nOffset
                    + mStreamInfo->frameSize * samplesize
                    > outHeader->nAllocLen) {
                ALOGE("buffer overflow");
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                return;

            }

            int available = outputDelayRingBufferSamplesAvailable();
            int numSamples = outHeader->nAllocLen / sizeof(int16_t);
            if (numSamples > available) {
                numSamples = available;
            }
            int64_t currentTime = 0;
            if (available) {

                int numFrames = numSamples / (mStreamInfo->frameSize * mStreamInfo->numChannels);
                numSamples = numFrames * (mStreamInfo->frameSize * mStreamInfo->numChannels);

                ALOGV("%d samples available (%d), or %d frames",
                        numSamples, available, numFrames);
                int64_t *nextTimeStamp = &mBufferTimestamps.editItemAt(0);
                currentTime = *nextTimeStamp;
                int32_t *currentBufLeft = &mBufferSizes.editItemAt(0);
                for (int i = 0; i < numFrames; i++) {
                    int32_t decodedSize = mDecodedSizes.itemAt(0);
                    mDecodedSizes.removeAt(0);
                    ALOGV("decoded %d of %d", decodedSize, *currentBufLeft);
                    if (*currentBufLeft > decodedSize) {
                        // adjust/interpolate next time stamp
                        *currentBufLeft -= decodedSize;
                        *nextTimeStamp += mStreamInfo->aacSamplesPerFrame *
                                1000000LL / mStreamInfo->aacSampleRate;
                        ALOGV("adjusted nextTimeStamp/size to %lld/%d",
                                (long long) *nextTimeStamp, *currentBufLeft);
                    } else {
                        // move to next timestamp in list
                        if (mBufferTimestamps.size() > 0) {
                            mBufferTimestamps.removeAt(0);
                            nextTimeStamp = &mBufferTimestamps.editItemAt(0);
                            mBufferSizes.removeAt(0);
                            currentBufLeft = &mBufferSizes.editItemAt(0);
                            ALOGV("moved to next time/size: %lld/%d",
                                    (long long) *nextTimeStamp, *currentBufLeft);
                        }
                        // try to limit output buffer size to match input buffers
                        // (e.g when an input buffer contained 4 "sub" frames, output
                        // at most 4 decoded units in the corresponding output buffer)
                        // This is optional. Remove the next three lines to fill the output
                        // buffer with as many units as available.
                        numFrames = i + 1;
                        numSamples = numFrames * mStreamInfo->frameSize * mStreamInfo->numChannels;
                        break;
                    }
                }

                ALOGV("getting %d from ringbuffer", numSamples);
                int32_t ns = outputDelayRingBufferGetSamples(outBuffer, numSamples);
                if (ns != numSamples) {
                    ALOGE("not a complete frame of samples available");
                    mSignalledError = true;
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    return;
                }
            }

            outHeader->nFilledLen = numSamples * sizeof(int16_t);

            if (mEndOfInput && !outQueue.empty() && outputDelayRingBufferSamplesAvailable() == 0) {
                outHeader->nFlags = OMX_BUFFERFLAG_EOS;
                mEndOfOutput = true;
            } else {
                outHeader->nFlags = 0;
            }

            outHeader->nTimeStamp = currentTime;

            mOutputBufferCount++;
            outInfo->mOwnedByUs = false;
            outQueue.erase(outQueue.begin());
            outInfo = NULL;
            ALOGV("out timestamp %lld / %d", outHeader->nTimeStamp, outHeader->nFilledLen);
            notifyFillBufferDone(outHeader);
            outHeader = NULL;
        }

        if (mEndOfInput) {
            int ringBufAvail = outputDelayRingBufferSamplesAvailable();
            if (!outQueue.empty()
                    && ringBufAvail < mStreamInfo->frameSize * mStreamInfo->numChannels) {
                if (!mEndOfOutput) {
                    // send partial or empty block signaling EOS
                    mEndOfOutput = true;
                    BufferInfo *outInfo = *outQueue.begin();
                    OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

                    INT_PCM *outBuffer = reinterpret_cast<INT_PCM *>(outHeader->pBuffer
                            + outHeader->nOffset);
                    int32_t ns = outputDelayRingBufferGetSamples(outBuffer, ringBufAvail);
                    if (ns < 0) {
                        ns = 0;
                    }
                    outHeader->nFilledLen = ns;
                    outHeader->nFlags = OMX_BUFFERFLAG_EOS;

                    outHeader->nTimeStamp = mBufferTimestamps.itemAt(0);
                    mBufferTimestamps.clear();
                    mBufferSizes.clear();
                    mDecodedSizes.clear();

                    mOutputBufferCount++;
                    outInfo->mOwnedByUs = false;
                    outQueue.erase(outQueue.begin());
                    outInfo = NULL;
                    notifyFillBufferDone(outHeader);
                    outHeader = NULL;
                }
                break; // if outQueue not empty but no more output
            }
        }
    }
}

void SoftAAC2::onPortFlushCompleted(OMX_U32 portIndex) {
    if (portIndex == 0) {
        // Make sure that the next buffer output does not still
        // depend on fragments from the last one decoded.
        // drain all existing data
        drainDecoder();
        mBufferTimestamps.clear();
        mBufferSizes.clear();
        mDecodedSizes.clear();
        mLastInHeader = NULL;
        mEndOfInput = false;
    } else {
        int avail;
        while ((avail = outputDelayRingBufferSamplesAvailable()) > 0) {
            if (avail > mStreamInfo->frameSize * mStreamInfo->numChannels) {
                avail = mStreamInfo->frameSize * mStreamInfo->numChannels;
            }
            int32_t ns = outputDelayRingBufferGetSamples(0, avail);
            if (ns != avail) {
                ALOGW("not a complete frame of samples available");
                break;
            }
            mOutputBufferCount++;
        }
        mOutputDelayRingBufferReadPos = mOutputDelayRingBufferWritePos;
        mEndOfOutput = false;
    }
}

void SoftAAC2::drainDecoder() {
    // flush decoder until outputDelay is compensated
    while (mOutputDelayCompensated > 0) {
        // a buffer big enough for MAX_CHANNEL_COUNT channels of decoded HE-AAC
        INT_PCM tmpOutBuffer[2048 * MAX_CHANNEL_COUNT];

        // run DRC check
        mDrcWrap.submitStreamData(mStreamInfo);
        mDrcWrap.update();

        AAC_DECODER_ERROR decoderErr =
            aacDecoder_DecodeFrame(mAACDecoder,
                                   tmpOutBuffer,
                                   2048 * MAX_CHANNEL_COUNT,
                                   AACDEC_FLUSH);
        if (decoderErr != AAC_DEC_OK) {
            ALOGW("aacDecoder_DecodeFrame decoderErr = 0x%4.4x", decoderErr);
        }

        int32_t tmpOutBufferSamples = mStreamInfo->frameSize * mStreamInfo->numChannels;
        if (tmpOutBufferSamples > mOutputDelayCompensated) {
            tmpOutBufferSamples = mOutputDelayCompensated;
        }
        outputDelayRingBufferPutSamples(tmpOutBuffer, tmpOutBufferSamples);

        mOutputDelayCompensated -= tmpOutBufferSamples;
    }
}

void SoftAAC2::onReset() {
    drainDecoder();
    // reset the "configured" state
    mInputBufferCount = 0;
    mOutputBufferCount = 0;
    mOutputDelayCompensated = 0;
    mOutputDelayRingBufferWritePos = 0;
    mOutputDelayRingBufferReadPos = 0;
    mOutputDelayRingBufferFilled = 0;
    mEndOfInput = false;
    mEndOfOutput = false;
    mBufferTimestamps.clear();
    mBufferSizes.clear();
    mDecodedSizes.clear();
    mLastInHeader = NULL;

    // To make the codec behave the same before and after a reset, we need to invalidate the
    // streaminfo struct. This does that:
    mStreamInfo->sampleRate = 0; // TODO: mStreamInfo is read only

    mSignalledError = false;
    mOutputPortSettingsChange = NONE;
}

void SoftAAC2::onPortEnableCompleted(OMX_U32 portIndex, bool enabled) {
    if (portIndex != 1) {
        return;
    }

    switch (mOutputPortSettingsChange) {
        case NONE:
            break;

        case AWAITING_DISABLED:
        {
            CHECK(!enabled);
            mOutputPortSettingsChange = AWAITING_ENABLED;
            break;
        }

        default:
        {
            CHECK_EQ((int)mOutputPortSettingsChange, (int)AWAITING_ENABLED);
            CHECK(enabled);
            mOutputPortSettingsChange = NONE;
            break;
        }
    }
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    return new android::SoftAAC2(name, callbacks, appData, component);
}
