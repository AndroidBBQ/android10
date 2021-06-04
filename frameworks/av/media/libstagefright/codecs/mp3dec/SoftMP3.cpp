/*
 * Copyright (C) 2011 The Android Open Source Project
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
#define LOG_TAG "SoftMP3"
#include <utils/Log.h>

#include "SoftMP3.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

#include "include/pvmp3decoder_api.h"

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftMP3::SoftMP3(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SimpleSoftOMXComponent(name, callbacks, appData, component),
      mConfig(new tPVMP3DecoderExternal),
      mDecoderBuf(NULL),
      mAnchorTimeUs(0),
      mNumFramesOutput(0),
      mNumChannels(2),
      mSamplingRate(44100),
      mSignalledError(false),
      mSawInputEos(false),
      mSignalledOutputEos(false),
      mOutputPortSettingsChange(NONE) {
    initPorts();
    initDecoder();
}

SoftMP3::~SoftMP3() {
    if (mDecoderBuf != NULL) {
        free(mDecoderBuf);
        mDecoderBuf = NULL;
    }

    delete mConfig;
    mConfig = NULL;
}

void SoftMP3::initPorts() {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    def.nPortIndex = 0;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = 8192;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.audio.cMIMEType =
        const_cast<char *>(MEDIA_MIMETYPE_AUDIO_MPEG);

    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingMP3;

    addPort(def);

    def.nPortIndex = 1;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kOutputBufferSize;
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

void SoftMP3::initDecoder() {
    mConfig->equalizerType = flat;
    mConfig->crcEnabled = false;

    uint32_t memRequirements = pvmp3_decoderMemRequirements();
    mDecoderBuf = calloc(1, memRequirements);

    pvmp3_InitDecoder(mConfig, mDecoderBuf);
    mIsFirst = true;
}

void *SoftMP3::memsetSafe(OMX_BUFFERHEADERTYPE *outHeader, int c, size_t len) {
    if (len > outHeader->nAllocLen) {
        ALOGE("memset buffer too small: got %u, expected %zu", outHeader->nAllocLen, len);
        android_errorWriteLog(0x534e4554, "29422022");
        notify(OMX_EventError, OMX_ErrorUndefined, OUTPUT_BUFFER_TOO_SMALL, NULL);
        mSignalledError = true;
        return NULL;
    }
    return memset(outHeader->pBuffer, c, len);
}

OMX_ERRORTYPE SoftMP3::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch (index) {
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
                    ? OMX_AUDIO_CodingMP3 : OMX_AUDIO_CodingPCM;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
                (OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex > 1) {
                return OMX_ErrorUndefined;
            }

            pcmParams->eNumData = OMX_NumericalDataSigned;
            pcmParams->eEndian = OMX_EndianBig;
            pcmParams->bInterleaved = OMX_TRUE;
            pcmParams->nBitPerSample = 16;
            pcmParams->ePCMMode = OMX_AUDIO_PCMModeLinear;
            pcmParams->eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcmParams->eChannelMapping[1] = OMX_AUDIO_ChannelRF;

            pcmParams->nChannels = mNumChannels;
            pcmParams->nSamplingRate = mSamplingRate;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioMp3:
        {
            OMX_AUDIO_PARAM_MP3TYPE *mp3Params =
                (OMX_AUDIO_PARAM_MP3TYPE *)params;

            if (!isValidOMXParam(mp3Params)) {
                return OMX_ErrorBadParameter;
            }

            if (mp3Params->nPortIndex > 1) {
                return OMX_ErrorUndefined;
            }

            mp3Params->nChannels = mNumChannels;
            mp3Params->nBitRate = 0 /* unknown */;
            mp3Params->nSampleRate = mSamplingRate;
            // other fields are encoder-only

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftMP3::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params) {
    switch (index) {
        case OMX_IndexParamStandardComponentRole:
        {
            const OMX_PARAM_COMPONENTROLETYPE *roleParams =
                (const OMX_PARAM_COMPONENTROLETYPE *)params;

            if (!isValidOMXParam(roleParams)) {
                return OMX_ErrorBadParameter;
            }

            if (strncmp((const char *)roleParams->cRole,
                        "audio_decoder.mp3",
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
                        && formatParams->eEncoding != OMX_AUDIO_CodingMP3)
                || (formatParams->nPortIndex == 1
                        && formatParams->eEncoding != OMX_AUDIO_CodingPCM)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            const OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
                (const OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            mNumChannels = pcmParams->nChannels;
            mSamplingRate = pcmParams->nSamplingRate;

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

void SoftMP3::onQueueFilled(OMX_U32 /* portIndex */) {
    if (mSignalledError || mOutputPortSettingsChange != NONE) {
        return;
    }

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    while ((!inQueue.empty() || (mSawInputEos && !mSignalledOutputEos)) && !outQueue.empty()) {
        BufferInfo *inInfo = NULL;
        OMX_BUFFERHEADERTYPE *inHeader = NULL;
        if (!inQueue.empty()) {
            inInfo = *inQueue.begin();
            inHeader = inInfo->mHeader;
        }

        BufferInfo *outInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;
        outHeader->nFlags = 0;

        if (inHeader) {
            if (inHeader->nOffset == 0 && inHeader->nFilledLen) {
                mAnchorTimeUs = inHeader->nTimeStamp;
                mNumFramesOutput = 0;
            }

            if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                mSawInputEos = true;
            }

            mConfig->pInputBuffer =
                inHeader->pBuffer + inHeader->nOffset;

            mConfig->inputBufferCurrentLength = inHeader->nFilledLen;
        } else {
            mConfig->pInputBuffer = NULL;
            mConfig->inputBufferCurrentLength = 0;
        }
        mConfig->inputBufferMaxLength = 0;
        mConfig->inputBufferUsedLength = 0;

        mConfig->outputFrameSize = kOutputBufferSize / sizeof(int16_t);
        if ((int32_t)outHeader->nAllocLen < mConfig->outputFrameSize) {
            ALOGE("input buffer too small: got %u, expected %u",
                outHeader->nAllocLen, mConfig->outputFrameSize);
            android_errorWriteLog(0x534e4554, "27793371");
            notify(OMX_EventError, OMX_ErrorUndefined, OUTPUT_BUFFER_TOO_SMALL, NULL);
            mSignalledError = true;
            return;
        }

        mConfig->pOutputBuffer =
            reinterpret_cast<int16_t *>(outHeader->pBuffer);

        ERROR_CODE decoderErr;
        if ((decoderErr = pvmp3_framedecoder(mConfig, mDecoderBuf))
                != NO_DECODING_ERROR) {
            ALOGV("mp3 decoder returned error %d", decoderErr);

            if (decoderErr != NO_ENOUGH_MAIN_DATA_ERROR
                        && decoderErr != SIDE_INFO_ERROR) {
                ALOGE("mp3 decoder returned error %d", decoderErr);

                notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                mSignalledError = true;
                return;
            }

            if (mConfig->outputFrameSize == 0) {
                mConfig->outputFrameSize = kOutputBufferSize / sizeof(int16_t);
            }

            if (decoderErr == NO_ENOUGH_MAIN_DATA_ERROR && mSawInputEos) {
                if (!mIsFirst) {
                    // pad the end of the stream with 529 samples, since that many samples
                    // were trimmed off the beginning when decoding started
                    outHeader->nOffset = 0;
                    outHeader->nFilledLen = kPVMP3DecoderDelay * mNumChannels * sizeof(int16_t);

                    if (!memsetSafe(outHeader, 0, outHeader->nFilledLen)) {
                        return;
                    }

                }
                outHeader->nFlags = OMX_BUFFERFLAG_EOS;
                mSignalledOutputEos = true;
            } else {
                // This is recoverable, just ignore the current frame and
                // play silence instead.

                // TODO: should we skip silence (and consume input data)
                // if mIsFirst is true as we may not have a valid
                // mConfig->samplingRate and mConfig->num_channels?
                ALOGV_IF(mIsFirst, "insufficient data for first frame, sending silence");
                if (!memsetSafe(outHeader, 0, mConfig->outputFrameSize * sizeof(int16_t))) {
                    return;
                }

                if (inHeader) {
                    mConfig->inputBufferUsedLength = inHeader->nFilledLen;
                }
            }
        } else if (mConfig->samplingRate != mSamplingRate
                || mConfig->num_channels != mNumChannels) {
            mSamplingRate = mConfig->samplingRate;
            mNumChannels = mConfig->num_channels;

            notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
            mOutputPortSettingsChange = AWAITING_DISABLED;
            return;
        }

        if (mIsFirst) {
            mIsFirst = false;
            // The decoder delay is 529 samples, so trim that many samples off
            // the start of the first output buffer. This essentially makes this
            // decoder have zero delay, which the rest of the pipeline assumes.
            outHeader->nOffset =
                kPVMP3DecoderDelay * mNumChannels * sizeof(int16_t);

            outHeader->nFilledLen =
                mConfig->outputFrameSize * sizeof(int16_t) - outHeader->nOffset;
        } else if (!mSignalledOutputEos) {
            outHeader->nOffset = 0;
            outHeader->nFilledLen = mConfig->outputFrameSize * sizeof(int16_t);
        }

        outHeader->nTimeStamp =
            mAnchorTimeUs + (mNumFramesOutput * 1000000LL) / mSamplingRate;

        if (inHeader) {
            CHECK_GE((int32_t)inHeader->nFilledLen, mConfig->inputBufferUsedLength);

            inHeader->nOffset += mConfig->inputBufferUsedLength;
            inHeader->nFilledLen -= mConfig->inputBufferUsedLength;


            if (inHeader->nFilledLen == 0) {
                inInfo->mOwnedByUs = false;
                inQueue.erase(inQueue.begin());
                inInfo = NULL;
                notifyEmptyBufferDone(inHeader);
                inHeader = NULL;
            }
        }

        mNumFramesOutput += mConfig->outputFrameSize / mNumChannels;

        outInfo->mOwnedByUs = false;
        outQueue.erase(outQueue.begin());
        outInfo = NULL;
        notifyFillBufferDone(outHeader);
        outHeader = NULL;
    }
}

void SoftMP3::onPortFlushCompleted(OMX_U32 portIndex) {
    if (portIndex == 0) {
        // Make sure that the next buffer output does not still
        // depend on fragments from the last one decoded.
        pvmp3_InitDecoder(mConfig, mDecoderBuf);
        mIsFirst = true;
        mSignalledError = false;
        mSawInputEos = false;
        mSignalledOutputEos = false;
    }
}

void SoftMP3::onPortEnableCompleted(OMX_U32 portIndex, bool enabled) {
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

void SoftMP3::onReset() {
    pvmp3_InitDecoder(mConfig, mDecoderBuf);
    mIsFirst = true;
    mSignalledError = false;
    mSawInputEos = false;
    mSignalledOutputEos = false;
    mOutputPortSettingsChange = NONE;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    return new android::SoftMP3(name, callbacks, appData, component);
}
