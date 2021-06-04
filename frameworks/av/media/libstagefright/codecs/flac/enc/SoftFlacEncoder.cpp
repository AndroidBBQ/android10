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
#define LOG_TAG "SoftFlacEncoder"
#include <android-base/macros.h>
#include <utils/Log.h>

#include "SoftFlacEncoder.h"
#include <audio_utils/primitives.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

#define FLAC_COMPRESSION_LEVEL_MIN     0
#define FLAC_COMPRESSION_LEVEL_DEFAULT 5
#define FLAC_COMPRESSION_LEVEL_MAX     8

#if LOG_NDEBUG
#define UNUSED_UNLESS_VERBOSE(x) (void)(x)
#else
#define UNUSED_UNLESS_VERBOSE(x)
#endif

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftFlacEncoder::SoftFlacEncoder(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SimpleSoftOMXComponent(name, callbacks, appData, component),
      mSignalledError(false),
      mNumChannels(1),
      mSampleRate(44100),
      mCompressionLevel(FLAC_COMPRESSION_LEVEL_DEFAULT),
      mEncoderWriteData(false),
      mEncoderReturnedEncodedData(false),
      mSawInputEOS(false),
      mSentOutputEOS(false),
      mEncoderReturnedNbBytes(0),
      mInputBufferPcm32(NULL),
      mHeaderOffset(0),
      mHeaderComplete(false),
      mWroteHeader(false)
{
    ALOGV("SoftFlacEncoder::SoftFlacEncoder(name=%s)", name);
    initPorts();

    mFlacStreamEncoder = FLAC__stream_encoder_new();
    if (mFlacStreamEncoder == NULL) {
        ALOGE("SoftFlacEncoder::SoftFlacEncoder(name=%s) error instantiating FLAC encoder", name);
        mSignalledError = true;
    }

    if (!mSignalledError) { // no use allocating input buffer if we had an error above
        // 2x the pcm16 samples can exist with the same size as pcmFloat samples.
        mInputBufferPcm32 = (FLAC__int32*) malloc(
                sizeof(FLAC__int32) * kNumSamplesPerFrame * kMaxChannels * 2);
        if (mInputBufferPcm32 == NULL) {
            ALOGE("SoftFlacEncoder::SoftFlacEncoder(name=%s) error allocating internal input buffer", name);
            mSignalledError = true;
        }
    }
}

SoftFlacEncoder::~SoftFlacEncoder() {
    ALOGV("SoftFlacEncoder::~SoftFlacEncoder()");
    if (mFlacStreamEncoder != NULL) {
        FLAC__stream_encoder_delete(mFlacStreamEncoder);
        mFlacStreamEncoder = NULL;
    }
    free(mInputBufferPcm32);
    mInputBufferPcm32 = NULL;
}

OMX_ERRORTYPE SoftFlacEncoder::initCheck() const {
    if (mSignalledError) {
        if (mFlacStreamEncoder == NULL) {
            ALOGE("initCheck() failed due to NULL encoder");
        } else if (mInputBufferPcm32 == NULL) {
            ALOGE("initCheck() failed due to error allocating internal input buffer");
        }
        return OMX_ErrorUndefined;
    } else {
        return SimpleSoftOMXComponent::initCheck();
    }
}

void SoftFlacEncoder::initPorts() {
    ALOGV("SoftFlacEncoder::initPorts()");

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    // configure input port of the encoder
    def.nPortIndex = 0;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kMaxInputBufferSize;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = sizeof(float);

    def.format.audio.cMIMEType = const_cast<char *>("audio/raw");
    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    addPort(def);

    // configure output port of the encoder
    def.nPortIndex = 1;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kMaxOutputBufferSize;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.audio.cMIMEType = const_cast<char *>(MEDIA_MIMETYPE_AUDIO_FLAC);
    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingFLAC;

    addPort(def);
}

OMX_ERRORTYPE SoftFlacEncoder::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    ALOGV("SoftFlacEncoder::internalGetParameter(index=0x%x)", index);

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
                    ? OMX_AUDIO_CodingPCM : OMX_AUDIO_CodingFLAC;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
                (OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            pcmParams->eNumData = mNumericalData;
            pcmParams->eEndian = OMX_EndianBig;
            pcmParams->bInterleaved = OMX_TRUE;
            pcmParams->nBitPerSample = mBitsPerSample;
            pcmParams->ePCMMode = OMX_AUDIO_PCMModeLinear;
            pcmParams->eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcmParams->eChannelMapping[1] = OMX_AUDIO_ChannelRF;

            pcmParams->nChannels = mNumChannels;
            pcmParams->nSamplingRate = mSampleRate;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioFlac:
        {
            OMX_AUDIO_PARAM_FLACTYPE *flacParams = (OMX_AUDIO_PARAM_FLACTYPE *)params;

            if (!isValidOMXParam(flacParams)) {
                return OMX_ErrorBadParameter;
            }

            if (flacParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            flacParams->nCompressionLevel = mCompressionLevel;
            flacParams->nChannels = mNumChannels;
            flacParams->nSampleRate = mSampleRate;
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftFlacEncoder::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params) {
    switch (index) {
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
                        && formatParams->eEncoding != OMX_AUDIO_CodingPCM)
                || (formatParams->nPortIndex == 1
                        && formatParams->eEncoding != OMX_AUDIO_CodingFLAC)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            ALOGV("SoftFlacEncoder::internalSetParameter(OMX_IndexParamAudioPcm)");
            OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams = (OMX_AUDIO_PARAM_PCMMODETYPE *)params;

            if (!isValidOMXParam(pcmParams)) {
                return OMX_ErrorBadParameter;
            }

            if (pcmParams->nPortIndex != 0) {
                ALOGE("SoftFlacEncoder::internalSetParameter() Error #1");
                return OMX_ErrorUndefined;
            }

            if (pcmParams->nChannels < 1 || pcmParams->nChannels > kMaxChannels) {
                return OMX_ErrorUndefined;
            }

            mNumChannels = pcmParams->nChannels;
            mSampleRate = pcmParams->nSamplingRate;

            if (pcmParams->eNumData == OMX_NumericalDataFloat && pcmParams->nBitPerSample == 32) {
                mNumericalData = OMX_NumericalDataFloat;
                mBitsPerSample = 32;
            } else if (pcmParams->eNumData == OMX_NumericalDataSigned
                     && pcmParams->nBitPerSample == 16) {
                mNumericalData = OMX_NumericalDataSigned;
                mBitsPerSample = 16;
            } else {
                ALOGE("%s: invalid eNumData %d, nBitsPerSample %d",
                        __func__, pcmParams->eNumData, pcmParams->nBitPerSample);
                return OMX_ErrorUndefined;
            }

            ALOGV("will encode %d channels at %dHz", mNumChannels, mSampleRate);

            return configureEncoder();
        }

        case OMX_IndexParamStandardComponentRole:
        {
            ALOGV("SoftFlacEncoder::internalSetParameter(OMX_IndexParamStandardComponentRole)");
            const OMX_PARAM_COMPONENTROLETYPE *roleParams =
                (const OMX_PARAM_COMPONENTROLETYPE *)params;

            if (!isValidOMXParam(roleParams)) {
                return OMX_ErrorBadParameter;
            }

            if (strncmp((const char *)roleParams->cRole,
                    "audio_encoder.flac",
                    OMX_MAX_STRINGNAME_SIZE - 1)) {
                ALOGE("SoftFlacEncoder::internalSetParameter(OMX_IndexParamStandardComponentRole)"
                        "error");
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioFlac:
        {
            // used only for setting the compression level
            OMX_AUDIO_PARAM_FLACTYPE *flacParams = (OMX_AUDIO_PARAM_FLACTYPE *)params;

            if (!isValidOMXParam(flacParams)) {
                return OMX_ErrorBadParameter;
            }

            if (flacParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            mCompressionLevel = flacParams->nCompressionLevel; // range clamping done inside encoder
            return OMX_ErrorNone;
        }

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *defParams =
                (OMX_PARAM_PORTDEFINITIONTYPE *)params;

            if (!isValidOMXParam(defParams)) {
                return OMX_ErrorBadParameter;
            }

            if (defParams->nPortIndex == 0) {
                if (defParams->nBufferSize > kMaxInputBufferSize) {
                    ALOGE("Input buffer size must be at most %d bytes",
                        kMaxInputBufferSize);
                    return OMX_ErrorUnsupportedSetting;
                }
            }

            FALLTHROUGH_INTENDED;
        }

        default:
            ALOGV("SoftFlacEncoder::internalSetParameter(default)");
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

void SoftFlacEncoder::onQueueFilled(OMX_U32 portIndex) {
    UNUSED_UNLESS_VERBOSE(portIndex);
    ALOGV("SoftFlacEncoder::onQueueFilled(portIndex=%d)", portIndex);

    if (mSignalledError) {
        return;
    }

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    const bool inputFloat = mNumericalData == OMX_NumericalDataFloat;
    const size_t sampleSize = inputFloat ? sizeof(float) : sizeof(int16_t);
    const size_t frameSize = sampleSize * mNumChannels;

    FLAC__bool ok = true;

    while ((!inQueue.empty() || mSawInputEOS) && !outQueue.empty() && !mSentOutputEOS) {
        if (!inQueue.empty()) {
            BufferInfo *inInfo = *inQueue.begin();
            OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;

            if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                ALOGV("saw EOS on buffer of size %u", inHeader->nFilledLen);
                mSawInputEOS = true;
            }

            if (inHeader->nFilledLen > kMaxInputBufferSize) {
                ALOGE("input buffer too large (%d).", inHeader->nFilledLen);
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                return;
            }

            assert(mNumChannels != 0);
            mEncoderWriteData = true;
            mEncoderReturnedEncodedData = false;
            mEncoderReturnedNbBytes = 0;
            mCurrentInputTimeStamp = inHeader->nTimeStamp;

            const unsigned nbInputFrames = inHeader->nFilledLen / frameSize;
            const unsigned nbInputSamples = inHeader->nFilledLen / sampleSize;

            if (inputFloat) {
                CHECK_LE(nbInputSamples, kNumSamplesPerFrame * kMaxChannels);
                const float * const pcmFloat = reinterpret_cast<float *>(inHeader->pBuffer);
                 memcpy_to_q8_23_from_float_with_clamp(
                         mInputBufferPcm32, pcmFloat, nbInputSamples);
            } else {
                // note nbInputSamples may be 2x as large for pcm16 data.
                CHECK_LE(nbInputSamples, kNumSamplesPerFrame * kMaxChannels * 2);
                const int16_t * const pcm16 = reinterpret_cast<int16_t *>(inHeader->pBuffer);
                for (unsigned i = 0; i < nbInputSamples; ++i) {
                    mInputBufferPcm32[i] = (FLAC__int32) pcm16[i];
                }
            }
            ALOGV(" about to encode %u samples per channel", nbInputFrames);
            ok = FLAC__stream_encoder_process_interleaved(
                            mFlacStreamEncoder,
                            mInputBufferPcm32,
                            nbInputFrames /*samples per channel*/ );

            inInfo->mOwnedByUs = false;
            inQueue.erase(inQueue.begin());
            inInfo = NULL;
            notifyEmptyBufferDone(inHeader);
            inHeader = NULL;
        }

        BufferInfo *outInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

        if (ok) {
            if (mEncoderReturnedEncodedData && (mEncoderReturnedNbBytes != 0)) {
                ALOGV(" dequeueing buffer on output port after writing data");
                outInfo->mOwnedByUs = false;
                outQueue.erase(outQueue.begin());
                outInfo = NULL;
                notifyFillBufferDone(outHeader);
                outHeader = NULL;
                mEncoderReturnedEncodedData = false;
            } else {
                ALOGV(" encoder process_interleaved returned without data to write");
                if (mSawInputEOS) {
                    ALOGV("finishing encoder");
                    mSentOutputEOS = true;
                    FLAC__stream_encoder_finish(mFlacStreamEncoder);
                    if (mEncoderReturnedEncodedData && (mEncoderReturnedNbBytes != 0)) {
                        ALOGV(" dequeueing residual buffer on output port after writing data");
                        outInfo->mOwnedByUs = false;
                        outQueue.erase(outQueue.begin());
                        outInfo = NULL;
                        outHeader->nFlags = OMX_BUFFERFLAG_EOS;
                        notifyFillBufferDone(outHeader);
                        outHeader = NULL;
                        mEncoderReturnedEncodedData = false;
                    }
                }
            }
        } else {
            ALOGE(" error encountered during encoding");
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            return;
        }

    }
}

FLAC__StreamEncoderWriteStatus SoftFlacEncoder::onEncodedFlacAvailable(
            const FLAC__byte buffer[],
            size_t bytes, unsigned samples,
            unsigned current_frame) {
    UNUSED_UNLESS_VERBOSE(current_frame);
    ALOGV("SoftFlacEncoder::onEncodedFlacAvailable(bytes=%zu, samples=%u, curr_frame=%u)",
            bytes, samples, current_frame);

    if (samples == 0) {
        ALOGV("saving %zu bytes of header", bytes);
        if (mHeaderOffset + bytes > sizeof(mHeader) || mHeaderComplete) {
            ALOGW("header is too big, or header already received");
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
        } else {
            memcpy(mHeader + mHeaderOffset, buffer, bytes);
            mHeaderOffset += bytes;// will contain header size when finished receiving header
            if (buffer[0] & 0x80) {
                mHeaderComplete = true;
            }
        }
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    if ((samples == 0) || !mEncoderWriteData) {
        // called by the encoder because there's header data to save, but it's not the role
        // of this component (unless WRITE_FLAC_HEADER_IN_FIRST_BUFFER is defined)
        ALOGV("ignoring %zu bytes of header data (samples=%d)", bytes, samples);
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    List<BufferInfo *> &outQueue = getPortQueue(1);
    CHECK(!outQueue.empty());
    BufferInfo *outInfo = *outQueue.begin();
    OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

    if (mHeaderComplete && !mWroteHeader) {
        ALOGV(" writing %d bytes of header on output port", mHeaderOffset);
        memcpy(outHeader->pBuffer + outHeader->nOffset + outHeader->nFilledLen,
                mHeader, mHeaderOffset);
        outHeader->nFilledLen += mHeaderOffset;
        mWroteHeader = true;
        outInfo->mOwnedByUs = false;
        outQueue.erase(outQueue.begin());
        outHeader->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
        notifyFillBufferDone(outHeader);
        outInfo = NULL;
        outHeader = NULL;
        // get the next buffer for the rest of the data
        CHECK(!outQueue.empty());
        outInfo = *outQueue.begin();
        outHeader = outInfo->mHeader;
    }

    // write encoded data
    ALOGV(" writing %zu bytes of encoded data on output port", bytes);
    if (bytes > outHeader->nAllocLen - outHeader->nOffset - outHeader->nFilledLen) {
        ALOGE(" not enough space left to write encoded data, dropping %zu bytes", bytes);
        // a fatal error would stop the encoding
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }
    memcpy(outHeader->pBuffer + outHeader->nOffset, buffer, bytes);

    outHeader->nTimeStamp = mCurrentInputTimeStamp;
    outHeader->nOffset = 0;
    outHeader->nFilledLen += bytes;
    outHeader->nFlags = 0;

    mEncoderReturnedEncodedData = true;
    mEncoderReturnedNbBytes += bytes;

    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}


OMX_ERRORTYPE SoftFlacEncoder::configureEncoder() {
    ALOGV("SoftFlacEncoder::configureEncoder() numChannel=%d, sampleRate=%d",
            mNumChannels, mSampleRate);

    if (mSignalledError || (mFlacStreamEncoder == NULL)) {
        ALOGE("can't configure encoder: no encoder or invalid state");
        return OMX_ErrorInvalidState;
    }

    const bool inputFloat = mNumericalData == OMX_NumericalDataFloat;
    const int codecBitsPerSample = inputFloat ? 24 : 16;
    FLAC__bool ok = true;
    ok = ok && FLAC__stream_encoder_set_channels(mFlacStreamEncoder, mNumChannels);
    ok = ok && FLAC__stream_encoder_set_sample_rate(mFlacStreamEncoder, mSampleRate);
    ok = ok && FLAC__stream_encoder_set_bits_per_sample(mFlacStreamEncoder, codecBitsPerSample);
    ok = ok && FLAC__stream_encoder_set_compression_level(mFlacStreamEncoder,
            (unsigned)mCompressionLevel);
    ok = ok && FLAC__stream_encoder_set_verify(mFlacStreamEncoder, false);
    if (!ok) { goto return_result; }

    ok &= FLAC__STREAM_ENCODER_INIT_STATUS_OK ==
            FLAC__stream_encoder_init_stream(mFlacStreamEncoder,
                    flacEncoderWriteCallback    /*write_callback*/,
                    NULL /*seek_callback*/,
                    NULL /*tell_callback*/,
                    NULL /*metadata_callback*/,
                    (void *) this /*client_data*/);

return_result:
    if (ok) {
        ALOGV("encoder successfully configured");
        return OMX_ErrorNone;
    } else {
        ALOGE("unknown error when configuring encoder");
        return OMX_ErrorUndefined;
    }
}


// static
FLAC__StreamEncoderWriteStatus SoftFlacEncoder::flacEncoderWriteCallback(
            const FLAC__StreamEncoder * /* encoder */,
            const FLAC__byte buffer[],
            size_t bytes,
            unsigned samples,
            unsigned current_frame,
            void *client_data) {
    return ((SoftFlacEncoder*) client_data)->onEncodedFlacAvailable(
            buffer, bytes, samples, current_frame);
}

}  // namespace android


android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    return new android::SoftFlacEncoder(name, callbacks, appData, component);
}

