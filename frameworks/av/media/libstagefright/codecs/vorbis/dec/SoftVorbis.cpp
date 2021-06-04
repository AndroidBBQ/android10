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
#define LOG_TAG "SoftVorbis"
#include <utils/Log.h>

#include "SoftVorbis.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

static int kDefaultChannelCount = 1;
static int kDefaultSamplingRate = 48000;

extern "C" {
    #include <Tremolo/codec_internal.h>

    int _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb);
    int _vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb);
    int _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb);
}

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftVorbis::SoftVorbis(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SimpleSoftOMXComponent(name, callbacks, appData, component),
      mInputBufferCount(0),
      mState(NULL),
      mVi(NULL),
      mAnchorTimeUs(0),
      mNumFramesOutput(0),
      mNumFramesLeftOnPage(-1),
      mSawInputEos(false),
      mSignalledOutputEos(false),
      mSignalledError(false),
      mOutputPortSettingsChange(NONE) {
    initPorts();
    CHECK_EQ(initDecoder(), (status_t)OK);
}

SoftVorbis::~SoftVorbis() {
    if (mState != NULL) {
        vorbis_dsp_clear(mState);
        delete mState;
        mState = NULL;
    }

    if (mVi != NULL) {
        vorbis_info_clear(mVi);
        delete mVi;
        mVi = NULL;
    }
}

void SoftVorbis::initPorts() {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    def.nPortIndex = 0;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kMaxNumSamplesPerBuffer * sizeof(int16_t);
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.audio.cMIMEType =
        const_cast<char *>(MEDIA_MIMETYPE_AUDIO_VORBIS);

    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding = OMX_AUDIO_CodingVORBIS;

    addPort(def);

    def.nPortIndex = 1;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kMaxNumSamplesPerBuffer * sizeof(int16_t);
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

status_t SoftVorbis::initDecoder() {
    return OK;
}

OMX_ERRORTYPE SoftVorbis::internalGetParameter(
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
                    ? OMX_AUDIO_CodingVORBIS : OMX_AUDIO_CodingPCM;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioVorbis:
        {
            OMX_AUDIO_PARAM_VORBISTYPE *vorbisParams =
                (OMX_AUDIO_PARAM_VORBISTYPE *)params;

            if (!isValidOMXParam(vorbisParams)) {
                return OMX_ErrorBadParameter;
            }

            if (vorbisParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            vorbisParams->nBitRate = 0;
            vorbisParams->nMinBitRate = 0;
            vorbisParams->nMaxBitRate = 0;
            vorbisParams->nAudioBandWidth = 0;
            vorbisParams->nQuality = 3;
            vorbisParams->bManaged = OMX_FALSE;
            vorbisParams->bDownmix = OMX_FALSE;

            if (!isConfigured()) {
                vorbisParams->nChannels = kDefaultChannelCount;
                vorbisParams->nSampleRate = kDefaultSamplingRate;
            } else {
                vorbisParams->nChannels = mVi->channels;
                vorbisParams->nSampleRate = mVi->rate;
                vorbisParams->nBitRate = mVi->bitrate_nominal;
                vorbisParams->nMinBitRate = mVi->bitrate_lower;
                vorbisParams->nMaxBitRate = mVi->bitrate_upper;
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

            if (!isConfigured()) {
                pcmParams->nChannels = kDefaultChannelCount;
                pcmParams->nSamplingRate = kDefaultSamplingRate;
            } else {
                pcmParams->nChannels = mVi->channels;
                pcmParams->nSamplingRate = mVi->rate;
            }

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftVorbis::internalSetParameter(
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
                        "audio_decoder.vorbis",
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
                        && formatParams->eEncoding != OMX_AUDIO_CodingVORBIS)
                || (formatParams->nPortIndex == 1
                        && formatParams->eEncoding != OMX_AUDIO_CodingPCM)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioVorbis:
        {
            const OMX_AUDIO_PARAM_VORBISTYPE *vorbisParams =
                (const OMX_AUDIO_PARAM_VORBISTYPE *)params;

            if (!isValidOMXParam(vorbisParams)) {
                return OMX_ErrorBadParameter;
            }

            if (vorbisParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

bool SoftVorbis::isConfigured() const {
    return (mState != NULL && mVi != NULL);
}

static void makeBitReader(
        const void *data, size_t size,
        ogg_buffer *buf, ogg_reference *ref, oggpack_buffer *bits) {
    buf->data = (uint8_t *)data;
    buf->size = size;
    buf->refcount = 1;
    buf->ptr.owner = NULL;

    ref->buffer = buf;
    ref->begin = 0;
    ref->length = size;
    ref->next = NULL;

    oggpack_readinit(bits, ref);
}

void SoftVorbis::handleEOS() {
    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    CHECK(!inQueue.empty() && !outQueue.empty());

    mSawInputEos = true;

    BufferInfo *outInfo = *outQueue.begin();
    OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;
    outHeader->nFilledLen = 0;
    outHeader->nFlags = OMX_BUFFERFLAG_EOS;

    outQueue.erase(outQueue.begin());
    outInfo->mOwnedByUs = false;
    notifyFillBufferDone(outHeader);
    mSignalledOutputEos = true;

    BufferInfo *inInfo = *inQueue.begin();
    OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;
    inQueue.erase(inQueue.begin());
    inInfo->mOwnedByUs = false;
    notifyEmptyBufferDone(inHeader);
    ++mInputBufferCount;
}

void SoftVorbis::onQueueFilled(OMX_U32 /* portIndex */) {
    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    if (mSignalledError || mOutputPortSettingsChange != NONE) {
        return;
    }

    while (!mSignalledOutputEos && (!inQueue.empty() || mSawInputEos) && !outQueue.empty()) {
        BufferInfo *inInfo = NULL;
        OMX_BUFFERHEADERTYPE *inHeader = NULL;
        if (!inQueue.empty()) {
            inInfo = *inQueue.begin();
            inHeader = inInfo->mHeader;
        }

        BufferInfo *outInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

        int32_t numPageSamples = 0;

        if (inHeader) {
            // Assume the very first 2 buffers are always codec config (in this case mState is NULL)
            // After flush, handle CSD
            if (mInputBufferCount < 2 &&
                    (mState == NULL || (inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG))) {
                const uint8_t *data = inHeader->pBuffer + inHeader->nOffset;
                size_t size = inHeader->nFilledLen;

                if ((inHeader->nFlags & OMX_BUFFERFLAG_EOS) && size == 0) {
                    handleEOS();
                    return;
                }

                if (size < 7) {
                    ALOGE("Too small input buffer: %zu bytes", size);
                    android_errorWriteLog(0x534e4554, "27833616");
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    mSignalledError = true;
                    return;
                }

                ogg_buffer buf;
                ogg_reference ref;
                oggpack_buffer bits;

                makeBitReader((const uint8_t *)data + 7, size - 7, &buf, &ref, &bits);

                // Assume very first frame is identification header - or reset identification
                // header after flush, but allow only specifying setup header after flush if
                // identification header was already set up.
                if (mInputBufferCount == 0 &&
                        (mVi == NULL || data[0] == 1 /* identification header */)) {
                    // remove any prior state
                    if (mVi != NULL) {
                        // also clear mState as it may refer to the old mVi
                        if (mState != NULL) {
                            vorbis_dsp_clear(mState);
                            delete mState;
                            mState = NULL;
                        }
                        vorbis_info_clear(mVi);
                        delete mVi;
                        mVi = NULL;
                    }

                    CHECK(mVi == NULL);
                    mVi = new vorbis_info;
                    vorbis_info_init(mVi);

                    int ret = _vorbis_unpack_info(mVi, &bits);
                    if (ret != 0) {
                        notify(OMX_EventError, OMX_ErrorUndefined, ret, NULL);
                        mSignalledError = true;
                        return;
                    }
                } else {
                    // remove any prior state
                    if (mState != NULL) {
                        vorbis_dsp_clear(mState);
                        delete mState;
                        mState = NULL;
                    }

                    int ret = _vorbis_unpack_books(mVi, &bits);
                    if (ret != 0 || mState != NULL) {
                        notify(OMX_EventError, OMX_ErrorUndefined, ret, NULL);
                        mSignalledError = true;
                        return;
                    }

                    CHECK(mState == NULL);
                    mState = new vorbis_dsp_state;
                    CHECK_EQ(0, vorbis_dsp_init(mState, mVi));

                    if (mVi->rate != kDefaultSamplingRate ||
                            mVi->channels != kDefaultChannelCount) {
                        ALOGV("vorbis: rate/channels changed: %ld/%d", mVi->rate, mVi->channels);
                        notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
                        mOutputPortSettingsChange = AWAITING_DISABLED;
                    }
                    mInputBufferCount = 1;
                }

                if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                    handleEOS();
                    return;
                }

                inQueue.erase(inQueue.begin());
                inInfo->mOwnedByUs = false;
                notifyEmptyBufferDone(inHeader);
                ++mInputBufferCount;

                continue;
            }

            if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                mSawInputEos = true;
            }

            if (inHeader->nFilledLen || !mSawInputEos) {
                if (inHeader->nFilledLen < sizeof(numPageSamples)) {
                    notify(OMX_EventError, OMX_ErrorBadParameter, 0, NULL);
                    mSignalledError = true;
                    ALOGE("onQueueFilled, input header has nFilledLen %u, expected %zu",
                            inHeader->nFilledLen, sizeof(numPageSamples));
                    return;
                }
                memcpy(&numPageSamples,
                       inHeader->pBuffer + inHeader->nOffset + inHeader->nFilledLen - 4,
                       sizeof(numPageSamples));

                if (inHeader->nOffset == 0) {
                    mAnchorTimeUs = inHeader->nTimeStamp;
                    mNumFramesOutput = 0;
                }

                inHeader->nFilledLen -= sizeof(numPageSamples);;
            }
        }

        if (numPageSamples >= 0) {
            mNumFramesLeftOnPage = numPageSamples;
        }

        ogg_buffer buf;
        buf.data = inHeader ? inHeader->pBuffer + inHeader->nOffset : NULL;
        buf.size = inHeader ? inHeader->nFilledLen : 0;
        buf.refcount = 1;
        buf.ptr.owner = NULL;

        ogg_reference ref;
        ref.buffer = &buf;
        ref.begin = 0;
        ref.length = buf.size;
        ref.next = NULL;

        ogg_packet pack;
        pack.packet = &ref;
        pack.bytes = ref.length;
        pack.b_o_s = 0;
        pack.e_o_s = 0;
        pack.granulepos = 0;
        pack.packetno = 0;

        int numFrames = 0;

        outHeader->nFlags = 0;

        if (mState == nullptr || mVi == nullptr) {
            notify(OMX_EventError, OMX_ErrorStreamCorrupt, 0, NULL);
            mSignalledError = true;
            ALOGE("onQueueFilled, input does not have CSD");
            return;
        }

        int err = vorbis_dsp_synthesis(mState, &pack, 1);
        if (err != 0) {
            // FIXME temporary workaround for log spam
#if !defined(__arm__) && !defined(__aarch64__)
            ALOGV("vorbis_dsp_synthesis returned %d", err);
#else
            ALOGW("vorbis_dsp_synthesis returned %d", err);
#endif
        } else {
            size_t numSamplesPerBuffer = kMaxNumSamplesPerBuffer;
            if (numSamplesPerBuffer > outHeader->nAllocLen / sizeof(int16_t)) {
                numSamplesPerBuffer = outHeader->nAllocLen / sizeof(int16_t);
                android_errorWriteLog(0x534e4554, "27833616");
            }
            numFrames = vorbis_dsp_pcmout(
                    mState, (int16_t *)outHeader->pBuffer,
                    (numSamplesPerBuffer / mVi->channels));

            if (numFrames < 0) {
                ALOGE("vorbis_dsp_pcmout returned %d", numFrames);
                numFrames = 0;
            }
        }

        if (mNumFramesLeftOnPage >= 0) {
            if (numFrames > mNumFramesLeftOnPage) {
                ALOGV("discarding %d frames at end of page",
                     numFrames - mNumFramesLeftOnPage);
                numFrames = mNumFramesLeftOnPage;
                if (mSawInputEos) {
                    outHeader->nFlags = OMX_BUFFERFLAG_EOS;
                    mSignalledOutputEos = true;
                }
            }
            mNumFramesLeftOnPage -= numFrames;
        }

        outHeader->nFilledLen = numFrames * sizeof(int16_t) * mVi->channels;
        outHeader->nOffset = 0;

        outHeader->nTimeStamp =
            mAnchorTimeUs
                + (mNumFramesOutput * 1000000LL) / mVi->rate;

        mNumFramesOutput += numFrames;

        if (inHeader) {
            inInfo->mOwnedByUs = false;
            inQueue.erase(inQueue.begin());
            notifyEmptyBufferDone(inHeader);
            ++mInputBufferCount;
        }

        outInfo->mOwnedByUs = false;
        outQueue.erase(outQueue.begin());
        notifyFillBufferDone(outHeader);
    }
}

void SoftVorbis::onPortFlushCompleted(OMX_U32 portIndex) {
    if (portIndex == 0 && mState != NULL) {
        // Make sure that the next buffer output does not still
        // depend on fragments from the last one decoded.

        mInputBufferCount = 0;
        mNumFramesOutput = 0;
        mSawInputEos = false;
        mSignalledOutputEos = false;
        mNumFramesLeftOnPage = -1;
        vorbis_dsp_restart(mState);
    }
}

void SoftVorbis::onReset() {
    mInputBufferCount = 0;
    mNumFramesOutput = 0;
    if (mState != NULL) {
        vorbis_dsp_clear(mState);
        delete mState;
        mState = NULL;
    }

    if (mVi != NULL) {
        vorbis_info_clear(mVi);
        delete mVi;
        mVi = NULL;
    }

    mSawInputEos = false;
    mSignalledOutputEos = false;
    mSignalledError = false;
    mOutputPortSettingsChange = NONE;
}

void SoftVorbis::onPortEnableCompleted(OMX_U32 portIndex, bool enabled) {
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
    return new android::SoftVorbis(name, callbacks, appData, component);
}
