/*
 * Copyright (C) 2014 The Android Open Source Project
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
#define LOG_TAG "SoftOpus"
#include <utils/Log.h>

#include "SoftOpus.h"
#include <OMX_AudioExt.h>
#include <OMX_IndexExt.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

extern "C" {
    #include <opus.h>
    #include <opus_multistream.h>
}

namespace android {

static const int kRate = 48000;

// Opus uses Vorbis channel mapping, and Vorbis channel mapping specifies
// mappings for up to 8 channels. This information is part of the Vorbis I
// Specification:
// http://www.xiph.org/vorbis/doc/Vorbis_I_spec.html
static const int kMaxChannels = 8;

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftOpus::SoftOpus(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SimpleSoftOMXComponent(name, callbacks, appData, component),
      mInputBufferCount(0),
      mDecoder(NULL),
      mHeader(NULL),
      mCodecDelay(0),
      mSeekPreRoll(0),
      mAnchorTimeUs(0),
      mNumFramesOutput(0),
      mHaveEOS(false),
      mOutputPortSettingsChange(NONE) {
    initPorts();
    CHECK_EQ(initDecoder(), (status_t)OK);
}

SoftOpus::~SoftOpus() {
    if (mDecoder != NULL) {
        opus_multistream_decoder_destroy(mDecoder);
        mDecoder = NULL;
    }
    if (mHeader != NULL) {
        delete mHeader;
        mHeader = NULL;
    }
}

void SoftOpus::initPorts() {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    def.nPortIndex = 0;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = 960 * 6;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainAudio;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.audio.cMIMEType =
        const_cast<char *>(MEDIA_MIMETYPE_AUDIO_OPUS);

    def.format.audio.pNativeRender = NULL;
    def.format.audio.bFlagErrorConcealment = OMX_FALSE;
    def.format.audio.eEncoding =
        (OMX_AUDIO_CODINGTYPE)OMX_AUDIO_CodingAndroidOPUS;

    addPort(def);

    def.nPortIndex = 1;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = kNumBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.nBufferSize = kMaxNumSamplesPerBuffer * sizeof(int16_t) * kMaxChannels;
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

status_t SoftOpus::initDecoder() {
    return OK;
}

OMX_ERRORTYPE SoftOpus::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch ((int)index) {
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
                    ? (OMX_AUDIO_CODINGTYPE)OMX_AUDIO_CodingAndroidOPUS :
                       OMX_AUDIO_CodingPCM;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioAndroidOpus:
        {
            OMX_AUDIO_PARAM_ANDROID_OPUSTYPE *opusParams =
                (OMX_AUDIO_PARAM_ANDROID_OPUSTYPE *)params;

            if (!isValidOMXParam(opusParams)) {
                return OMX_ErrorBadParameter;
            }

            if (opusParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            opusParams->nAudioBandWidth = 0;
            opusParams->nSampleRate = kRate;
            opusParams->nBitRate = 0;

            if (!isConfigured()) {
                opusParams->nChannels = 1;
            } else {
                opusParams->nChannels = mHeader->channels;
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
            pcmParams->nSamplingRate = kRate;

            if (!isConfigured()) {
                pcmParams->nChannels = 1;
            } else {
                pcmParams->nChannels = mHeader->channels;
            }

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftOpus::internalSetParameter(
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
                        "audio_decoder.opus",
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
                        && formatParams->eEncoding !=
                           (OMX_AUDIO_CODINGTYPE)OMX_AUDIO_CodingAndroidOPUS)
                || (formatParams->nPortIndex == 1
                        && formatParams->eEncoding != OMX_AUDIO_CodingPCM)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioAndroidOpus:
        {
            const OMX_AUDIO_PARAM_ANDROID_OPUSTYPE *opusParams =
                (const OMX_AUDIO_PARAM_ANDROID_OPUSTYPE *)params;

            if (!isValidOMXParam(opusParams)) {
                return OMX_ErrorBadParameter;
            }

            if (opusParams->nPortIndex != 0) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

bool SoftOpus::isConfigured() const {
    return mInputBufferCount >= 1;
}

static uint16_t ReadLE16(const uint8_t *data, size_t data_size,
                         uint32_t read_offset) {
    if (read_offset + 1 > data_size)
        return 0;
    uint16_t val;
    val = data[read_offset];
    val |= data[read_offset + 1] << 8;
    return val;
}

// Maximum packet size used in Xiph's opusdec.
static const int kMaxOpusOutputPacketSizeSamples = 960 * 6;

// Default audio output channel layout. Used to initialize |stream_map| in
// OpusHeader, and passed to opus_multistream_decoder_create() when the header
// does not contain mapping information. The values are valid only for mono and
// stereo output: Opus streams with more than 2 channels require a stream map.
static const int kMaxChannelsWithDefaultLayout = 2;
static const uint8_t kDefaultOpusChannelLayout[kMaxChannelsWithDefaultLayout] = { 0, 1 };

// Parses Opus Header. Header spec: http://wiki.xiph.org/OggOpus#ID_Header
static bool ParseOpusHeader(const uint8_t *data, size_t data_size,
                            OpusHeader* header) {
    // Size of the Opus header excluding optional mapping information.
    const size_t kOpusHeaderSize = 19;

    // Offset to the channel count byte in the Opus header.
    const size_t kOpusHeaderChannelsOffset = 9;

    // Offset to the pre-skip value in the Opus header.
    const size_t kOpusHeaderSkipSamplesOffset = 10;

    // Offset to the gain value in the Opus header.
    const size_t kOpusHeaderGainOffset = 16;

    // Offset to the channel mapping byte in the Opus header.
    const size_t kOpusHeaderChannelMappingOffset = 18;

    // Opus Header contains a stream map. The mapping values are in the header
    // beyond the always present |kOpusHeaderSize| bytes of data. The mapping
    // data contains stream count, coupling information, and per channel mapping
    // values:
    //   - Byte 0: Number of streams.
    //   - Byte 1: Number coupled.
    //   - Byte 2: Starting at byte 2 are |header->channels| uint8 mapping
    //             values.
    const size_t kOpusHeaderNumStreamsOffset = kOpusHeaderSize;
    const size_t kOpusHeaderNumCoupledOffset = kOpusHeaderNumStreamsOffset + 1;
    const size_t kOpusHeaderStreamMapOffset = kOpusHeaderNumStreamsOffset + 2;

    if (data_size < kOpusHeaderSize) {
        ALOGV("Header size is too small.");
        return false;
    }
    header->channels = *(data + kOpusHeaderChannelsOffset);

    if (header->channels <= 0 || header->channels > kMaxChannels) {
        ALOGV("Invalid Header, wrong channel count: %d", header->channels);
        return false;
    }
    header->skip_samples = ReadLE16(data, data_size,
                                        kOpusHeaderSkipSamplesOffset);
    header->gain_db = static_cast<int16_t>(
                              ReadLE16(data, data_size,
                                       kOpusHeaderGainOffset));
    header->channel_mapping = *(data + kOpusHeaderChannelMappingOffset);
    if (!header->channel_mapping) {
        if (header->channels > kMaxChannelsWithDefaultLayout) {
            ALOGV("Invalid Header, missing stream map.");
            return false;
        }
        header->num_streams = 1;
        header->num_coupled = header->channels > 1;
        header->stream_map[0] = 0;
        header->stream_map[1] = 1;
        return true;
    }
    if (data_size < kOpusHeaderStreamMapOffset + header->channels) {
        ALOGV("Invalid stream map; insufficient data for current channel "
              "count: %d", header->channels);
        return false;
    }
    header->num_streams = *(data + kOpusHeaderNumStreamsOffset);
    header->num_coupled = *(data + kOpusHeaderNumCoupledOffset);
    if (header->num_streams + header->num_coupled != header->channels) {
        ALOGV("Inconsistent channel mapping.");
        return false;
    }
    for (int i = 0; i < header->channels; ++i)
      header->stream_map[i] = *(data + kOpusHeaderStreamMapOffset + i);
    return true;
}

// Convert nanoseconds to number of samples.
static uint64_t ns_to_samples(uint64_t ns, int kRate) {
    return static_cast<double>(ns) * kRate / 1000000000;
}

void SoftOpus::handleEOS() {
    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);
    CHECK(!inQueue.empty() && !outQueue.empty());

    BufferInfo *outInfo = *outQueue.begin();
    OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;
    outHeader->nFilledLen = 0;
    outHeader->nFlags = OMX_BUFFERFLAG_EOS;
    mHaveEOS = true;

    outQueue.erase(outQueue.begin());
    outInfo->mOwnedByUs = false;
    notifyFillBufferDone(outHeader);

    BufferInfo *inInfo = *inQueue.begin();
    OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;
    inQueue.erase(inQueue.begin());
    inInfo->mOwnedByUs = false;
    notifyEmptyBufferDone(inHeader);

    ++mInputBufferCount;
}

void SoftOpus::onQueueFilled(OMX_U32 /* portIndex */) {
    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    if (mOutputPortSettingsChange != NONE) {
        return;
    }

    while (!mHaveEOS && !inQueue.empty() && !outQueue.empty()) {
        BufferInfo *inInfo = *inQueue.begin();
        OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;

        if (mInputBufferCount < 3) {
            const uint8_t *data = inHeader->pBuffer + inHeader->nOffset;
            size_t size = inHeader->nFilledLen;

            if ((inHeader->nFlags & OMX_BUFFERFLAG_EOS) && size == 0) {
                handleEOS();
                return;
            }

            if (size < sizeof(int64_t)) {
                // The 2nd and 3rd input buffer are expected to contain
                //  an int64_t (see below), so make sure we get at least
                //  that much. The first input buffer must contain 19 bytes,
                //  but that is checked already.
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                return;
            }

            if (mInputBufferCount == 0) {
                delete mHeader;
                mHeader = new OpusHeader();
                memset(mHeader, 0, sizeof(*mHeader));
                if (!ParseOpusHeader(data, size, mHeader)) {
                    ALOGV("Parsing Opus Header failed.");
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    return;
                }

                uint8_t channel_mapping[kMaxChannels] = {0};
                if (mHeader->channels <= kMaxChannelsWithDefaultLayout) {
                    memcpy(&channel_mapping,
                           kDefaultOpusChannelLayout,
                           kMaxChannelsWithDefaultLayout);
                } else {
                    memcpy(&channel_mapping,
                           mHeader->stream_map,
                           mHeader->channels);
                }

                int status = OPUS_INVALID_STATE;
                if (mDecoder != NULL) {
                    opus_multistream_decoder_destroy(mDecoder);
                }
                mDecoder = opus_multistream_decoder_create(kRate,
                                                           mHeader->channels,
                                                           mHeader->num_streams,
                                                           mHeader->num_coupled,
                                                           channel_mapping,
                                                           &status);
                if (!mDecoder || status != OPUS_OK) {
                    ALOGV("opus_multistream_decoder_create failed status=%s",
                          opus_strerror(status));
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    return;
                }
                status =
                    opus_multistream_decoder_ctl(mDecoder,
                                                 OPUS_SET_GAIN(mHeader->gain_db));
                if (status != OPUS_OK) {
                    ALOGV("Failed to set OPUS header gain; status=%s",
                          opus_strerror(status));
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                    return;
                }
            } else if (mInputBufferCount == 1) {
                mCodecDelay = ns_to_samples(
                                  *(reinterpret_cast<int64_t*>(inHeader->pBuffer +
                                                               inHeader->nOffset)),
                                  kRate);
                mSamplesToDiscard = mCodecDelay;
            } else {
                mSeekPreRoll = ns_to_samples(
                                   *(reinterpret_cast<int64_t*>(inHeader->pBuffer +
                                                                inHeader->nOffset)),
                                   kRate);
                notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
                mOutputPortSettingsChange = AWAITING_DISABLED;
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

        // Ignore CSD re-submissions.
        if (mInputBufferCount >= 3 && (inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG)) {
            if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                handleEOS();
                return;
            }

            inQueue.erase(inQueue.begin());
            inInfo->mOwnedByUs = false;
            notifyEmptyBufferDone(inHeader);
            continue;
        }

        BufferInfo *outInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

        if ((inHeader->nFlags & OMX_BUFFERFLAG_EOS) && inHeader->nFilledLen == 0) {
            handleEOS();
            return;
        }

        if (inHeader->nOffset == 0) {
            mAnchorTimeUs = inHeader->nTimeStamp;
            mNumFramesOutput = 0;
        }

        // When seeking to zero, |mCodecDelay| samples has to be discarded
        // instead of |mSeekPreRoll| samples (as we would when seeking to any
        // other timestamp).
        if (inHeader->nTimeStamp == 0) {
            mSamplesToDiscard = mCodecDelay;
        }

        const uint8_t *data = inHeader->pBuffer + inHeader->nOffset;
        const uint32_t size = inHeader->nFilledLen;
        size_t frameSize = kMaxOpusOutputPacketSizeSamples;
        if (frameSize > outHeader->nAllocLen / sizeof(int16_t) / mHeader->channels) {
            frameSize = outHeader->nAllocLen / sizeof(int16_t) / mHeader->channels;
            android_errorWriteLog(0x534e4554, "27833616");
        }

        int numFrames = opus_multistream_decode(mDecoder,
                                                data,
                                                size,
                                                (int16_t *)outHeader->pBuffer,
                                                frameSize,
                                                0);
        if (numFrames < 0) {
            ALOGE("opus_multistream_decode returned %d", numFrames);
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            return;
        }

        outHeader->nOffset = 0;
        if (mSamplesToDiscard > 0) {
            if (mSamplesToDiscard > numFrames) {
                mSamplesToDiscard -= numFrames;
                numFrames = 0;
            } else {
                numFrames -= mSamplesToDiscard;
                outHeader->nOffset = mSamplesToDiscard * sizeof(int16_t) *
                                     mHeader->channels;
                mSamplesToDiscard = 0;
            }
        }

        outHeader->nFilledLen = numFrames * sizeof(int16_t) * mHeader->channels;

        outHeader->nTimeStamp = mAnchorTimeUs +
                                (mNumFramesOutput * 1000000LL) /
                                kRate;

        mNumFramesOutput += numFrames;

        if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
            outHeader->nFlags = OMX_BUFFERFLAG_EOS;
            mHaveEOS = true;
        } else {
            outHeader->nFlags = 0;
        }

        inInfo->mOwnedByUs = false;
        inQueue.erase(inQueue.begin());
        notifyEmptyBufferDone(inHeader);
        ++mInputBufferCount;

        outInfo->mOwnedByUs = false;
        outQueue.erase(outQueue.begin());
        notifyFillBufferDone(outHeader);
    }
}

void SoftOpus::onPortFlushCompleted(OMX_U32 portIndex) {
    if (portIndex == 0 && mDecoder != NULL) {
        // Make sure that the next buffer output does not still
        // depend on fragments from the last one decoded.
        mNumFramesOutput = 0;
        opus_multistream_decoder_ctl(mDecoder, OPUS_RESET_STATE);
        mAnchorTimeUs = 0;
        mSamplesToDiscard = mSeekPreRoll;
        mHaveEOS = false;
    }
}

void SoftOpus::onReset() {
    mInputBufferCount = 0;
    mNumFramesOutput = 0;
    if (mDecoder != NULL) {
        opus_multistream_decoder_destroy(mDecoder);
        mDecoder = NULL;
    }
    if (mHeader != NULL) {
        delete mHeader;
        mHeader = NULL;
    }

    mOutputPortSettingsChange = NONE;
    mHaveEOS = false;
}

void SoftOpus::onPortEnableCompleted(OMX_U32 portIndex, bool enabled) {
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
    return new android::SoftOpus(name, callbacks, appData, component);
}
