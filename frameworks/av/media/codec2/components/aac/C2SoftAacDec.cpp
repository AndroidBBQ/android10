/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "C2SoftAacDec"
#include <log/log.h>

#include <inttypes.h>
#include <math.h>
#include <numeric>

#include <cutils/properties.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/misc.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftAacDec.h"

#define FILEREAD_MAX_LAYERS 2

#define DRC_DEFAULT_MOBILE_REF_LEVEL -16.0  /* 64*-0.25dB = -16 dB below full scale for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_CUT   1.0 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_BOOST 1.0 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_HEAVY C2Config::DRC_COMPRESSION_HEAVY   /* switch for heavy compression for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_EFFECT 3  /* MPEG-D DRC effect type; 3 => Limited playback range */
#define DRC_DEFAULT_MOBILE_ENC_LEVEL (0.25) /* encoder target level; -1 => the value is unknown, otherwise dB step value (e.g. 64 for -16 dB) */
#define MAX_CHANNEL_COUNT            8  /* maximum number of audio channels that can be decoded */
// names of properties that can be used to override the default DRC settings
#define PROP_DRC_OVERRIDE_REF_LEVEL  "aac_drc_reference_level"
#define PROP_DRC_OVERRIDE_CUT        "aac_drc_cut"
#define PROP_DRC_OVERRIDE_BOOST      "aac_drc_boost"
#define PROP_DRC_OVERRIDE_HEAVY      "aac_drc_heavy"
#define PROP_DRC_OVERRIDE_ENC_LEVEL "aac_drc_enc_target_level"
#define PROP_DRC_OVERRIDE_EFFECT     "ro.aac_drc_effect_type"

namespace android {

constexpr char COMPONENT_NAME[] = "c2.android.aac.decoder";

class C2SoftAacDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_AAC) {
        noPrivateBuffers();
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();

        addParameter(
                DefineParam(mActualOutputDelay, C2_PARAMKEY_OUTPUT_DELAY)
                .withConstValue(new C2PortActualDelayTuning::output(2u))
                .build());

        addParameter(
                DefineParam(mSampleRate, C2_PARAMKEY_SAMPLE_RATE)
                .withDefault(new C2StreamSampleRateInfo::output(0u, 44100))
                .withFields({C2F(mSampleRate, value).oneOf({
                    7350, 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
                })})
                .withSetter(Setter<decltype(*mSampleRate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mChannelCount, C2_PARAMKEY_CHANNEL_COUNT)
                .withDefault(new C2StreamChannelCountInfo::output(0u, 1))
                .withFields({C2F(mChannelCount, value).inRange(1, 8)})
                .withSetter(Setter<decltype(*mChannelCount)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::input(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(8000, 960000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 8192))
                .build());

        addParameter(
                DefineParam(mAacFormat, C2_PARAMKEY_AAC_PACKAGING)
                .withDefault(new C2StreamAacFormatInfo::input(0u, C2Config::AAC_PACKAGING_RAW))
                .withFields({C2F(mAacFormat, value).oneOf({
                    C2Config::AAC_PACKAGING_RAW, C2Config::AAC_PACKAGING_ADTS
                })})
                .withSetter(Setter<decltype(*mAacFormat)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_AAC_LC, C2Config::LEVEL_UNUSED))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                            C2Config::PROFILE_AAC_LC,
                            C2Config::PROFILE_AAC_HE,
                            C2Config::PROFILE_AAC_HE_PS,
                            C2Config::PROFILE_AAC_LD,
                            C2Config::PROFILE_AAC_ELD,
                            C2Config::PROFILE_AAC_ER_SCALABLE,
                            C2Config::PROFILE_AAC_XHE}),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_UNUSED
                    })
                })
                .withSetter(ProfileLevelSetter)
                .build());

        addParameter(
                DefineParam(mDrcCompressMode, C2_PARAMKEY_DRC_COMPRESSION_MODE)
                .withDefault(new C2StreamDrcCompressionModeTuning::input(0u, C2Config::DRC_COMPRESSION_HEAVY))
                .withFields({
                    C2F(mDrcCompressMode, value).oneOf({
                            C2Config::DRC_COMPRESSION_ODM_DEFAULT,
                            C2Config::DRC_COMPRESSION_NONE,
                            C2Config::DRC_COMPRESSION_LIGHT,
                            C2Config::DRC_COMPRESSION_HEAVY})
                })
                .withSetter(Setter<decltype(*mDrcCompressMode)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mDrcTargetRefLevel, C2_PARAMKEY_DRC_TARGET_REFERENCE_LEVEL)
                .withDefault(new C2StreamDrcTargetReferenceLevelTuning::input(0u, DRC_DEFAULT_MOBILE_REF_LEVEL))
                .withFields({C2F(mDrcTargetRefLevel, value).inRange(-31.75, 0.25)})
                .withSetter(Setter<decltype(*mDrcTargetRefLevel)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mDrcEncTargetLevel, C2_PARAMKEY_DRC_ENCODED_TARGET_LEVEL)
                .withDefault(new C2StreamDrcEncodedTargetLevelTuning::input(0u, DRC_DEFAULT_MOBILE_ENC_LEVEL))
                .withFields({C2F(mDrcEncTargetLevel, value).inRange(-31.75, 0.25)})
                .withSetter(Setter<decltype(*mDrcEncTargetLevel)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mDrcBoostFactor, C2_PARAMKEY_DRC_BOOST_FACTOR)
                .withDefault(new C2StreamDrcBoostFactorTuning::input(0u, DRC_DEFAULT_MOBILE_DRC_BOOST))
                .withFields({C2F(mDrcBoostFactor, value).inRange(0, 1.)})
                .withSetter(Setter<decltype(*mDrcBoostFactor)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mDrcAttenuationFactor, C2_PARAMKEY_DRC_ATTENUATION_FACTOR)
                .withDefault(new C2StreamDrcAttenuationFactorTuning::input(0u, DRC_DEFAULT_MOBILE_DRC_CUT))
                .withFields({C2F(mDrcAttenuationFactor, value).inRange(0, 1.)})
                .withSetter(Setter<decltype(*mDrcAttenuationFactor)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mDrcEffectType, C2_PARAMKEY_DRC_EFFECT_TYPE)
                .withDefault(new C2StreamDrcEffectTypeTuning::input(0u, C2Config::DRC_EFFECT_LIMITED_PLAYBACK_RANGE))
                .withFields({
                    C2F(mDrcEffectType, value).oneOf({
                            C2Config::DRC_EFFECT_ODM_DEFAULT,
                            C2Config::DRC_EFFECT_OFF,
                            C2Config::DRC_EFFECT_NONE,
                            C2Config::DRC_EFFECT_LATE_NIGHT,
                            C2Config::DRC_EFFECT_NOISY_ENVIRONMENT,
                            C2Config::DRC_EFFECT_LIMITED_PLAYBACK_RANGE,
                            C2Config::DRC_EFFECT_LOW_PLAYBACK_LEVEL,
                            C2Config::DRC_EFFECT_DIALOG_ENHANCEMENT,
                            C2Config::DRC_EFFECT_GENERAL_COMPRESSION})
                })
                .withSetter(Setter<decltype(*mDrcEffectType)>::StrictValueWithNoDeps)
                .build());
    }

    bool isAdts() const { return mAacFormat->value == C2Config::AAC_PACKAGING_ADTS; }
    static C2R ProfileLevelSetter(bool mayBlock, C2P<C2StreamProfileLevelInfo::input> &me) {
        (void)mayBlock;
        (void)me;  // TODO: validate
        return C2R::Ok();
    }
    int32_t getDrcCompressMode() const { return mDrcCompressMode->value == C2Config::DRC_COMPRESSION_HEAVY ? 1 : 0; }
    int32_t getDrcTargetRefLevel() const { return (mDrcTargetRefLevel->value <= 0 ? -mDrcTargetRefLevel->value * 4. + 0.5 : -1); }
    int32_t getDrcEncTargetLevel() const { return (mDrcEncTargetLevel->value <= 0 ? -mDrcEncTargetLevel->value * 4. + 0.5 : -1); }
    int32_t getDrcBoostFactor() const { return mDrcBoostFactor->value * 127. + 0.5; }
    int32_t getDrcAttenuationFactor() const { return mDrcAttenuationFactor->value * 127. + 0.5; }
    int32_t getDrcEffectType() const { return mDrcEffectType->value; }

private:
    std::shared_ptr<C2StreamSampleRateInfo::output> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::output> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::input> mBitrate;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
    std::shared_ptr<C2StreamAacFormatInfo::input> mAacFormat;
    std::shared_ptr<C2StreamProfileLevelInfo::input> mProfileLevel;
    std::shared_ptr<C2StreamDrcCompressionModeTuning::input> mDrcCompressMode;
    std::shared_ptr<C2StreamDrcTargetReferenceLevelTuning::input> mDrcTargetRefLevel;
    std::shared_ptr<C2StreamDrcEncodedTargetLevelTuning::input> mDrcEncTargetLevel;
    std::shared_ptr<C2StreamDrcBoostFactorTuning::input> mDrcBoostFactor;
    std::shared_ptr<C2StreamDrcAttenuationFactorTuning::input> mDrcAttenuationFactor;
    std::shared_ptr<C2StreamDrcEffectTypeTuning::input> mDrcEffectType;
    // TODO Add : C2StreamAacSbrModeTuning
};

C2SoftAacDec::C2SoftAacDec(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mAACDecoder(nullptr),
      mStreamInfo(nullptr),
      mSignalledError(false),
      mOutputDelayRingBuffer(nullptr) {
}

C2SoftAacDec::~C2SoftAacDec() {
    onRelease();
}

c2_status_t C2SoftAacDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

c2_status_t C2SoftAacDec::onStop() {
    drainDecoder();
    // reset the "configured" state
    mOutputDelayCompensated = 0;
    mOutputDelayRingBufferWritePos = 0;
    mOutputDelayRingBufferReadPos = 0;
    mOutputDelayRingBufferFilled = 0;
    mBuffersInfo.clear();

    // To make the codec behave the same before and after a reset, we need to invalidate the
    // streaminfo struct. This does that:
    mStreamInfo->sampleRate = 0; // TODO: mStreamInfo is read only

    mSignalledError = false;

    return C2_OK;
}

void C2SoftAacDec::onReset() {
    (void)onStop();
}

void C2SoftAacDec::onRelease() {
    if (mAACDecoder) {
        aacDecoder_Close(mAACDecoder);
        mAACDecoder = nullptr;
    }
    if (mOutputDelayRingBuffer) {
        delete[] mOutputDelayRingBuffer;
        mOutputDelayRingBuffer = nullptr;
    }
}

status_t C2SoftAacDec::initDecoder() {
    ALOGV("initDecoder()");
    status_t status = UNKNOWN_ERROR;
    mAACDecoder = aacDecoder_Open(TT_MP4_ADIF, /* num layers */ 1);
    if (mAACDecoder != nullptr) {
        mStreamInfo = aacDecoder_GetStreamInfo(mAACDecoder);
        if (mStreamInfo != nullptr) {
            status = OK;
        }
    }

    mOutputDelayCompensated = 0;
    mOutputDelayRingBufferSize = 2048 * MAX_CHANNEL_COUNT * kNumDelayBlocksMax;
    mOutputDelayRingBuffer = new short[mOutputDelayRingBufferSize];
    mOutputDelayRingBufferWritePos = 0;
    mOutputDelayRingBufferReadPos = 0;
    mOutputDelayRingBufferFilled = 0;

    if (mAACDecoder == nullptr) {
        ALOGE("AAC decoder is null. TODO: Can not call aacDecoder_SetParam in the following code");
    }

    //aacDecoder_SetParam(mAACDecoder, AAC_PCM_LIMITER_ENABLE, 0);

    //init DRC wrapper
    mDrcWrap.setDecoderHandle(mAACDecoder);
    mDrcWrap.submitStreamData(mStreamInfo);

    // for streams that contain metadata, use the mobile profile DRC settings unless overridden by platform properties
    // TODO: change the DRC settings depending on audio output device type (HDMI, loadspeaker, headphone)

    //  DRC_PRES_MODE_WRAP_DESIRED_TARGET
    int32_t targetRefLevel = mIntf->getDrcTargetRefLevel();
    ALOGV("AAC decoder using desired DRC target reference level of %d", targetRefLevel);
    mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_TARGET, (unsigned)targetRefLevel);

    //  DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR

    int32_t attenuationFactor = mIntf->getDrcAttenuationFactor();
    ALOGV("AAC decoder using desired DRC attenuation factor of %d", attenuationFactor);
    mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR, (unsigned)attenuationFactor);

    //  DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR
    int32_t boostFactor = mIntf->getDrcBoostFactor();
    ALOGV("AAC decoder using desired DRC boost factor of %d", boostFactor);
    mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR, (unsigned)boostFactor);

    //  DRC_PRES_MODE_WRAP_DESIRED_HEAVY
    int32_t compressMode = mIntf->getDrcCompressMode();
    ALOGV("AAC decoder using desried DRC heavy compression switch of %d", compressMode);
    mDrcWrap.setParam(DRC_PRES_MODE_WRAP_DESIRED_HEAVY, (unsigned)compressMode);

    // DRC_PRES_MODE_WRAP_ENCODER_TARGET
    int32_t encTargetLevel = mIntf->getDrcEncTargetLevel();
    ALOGV("AAC decoder using encoder-side DRC reference level of %d", encTargetLevel);
    mDrcWrap.setParam(DRC_PRES_MODE_WRAP_ENCODER_TARGET, (unsigned)encTargetLevel);

    // AAC_UNIDRC_SET_EFFECT
    int32_t effectType = mIntf->getDrcEffectType();
    ALOGV("AAC decoder using MPEG-D DRC effect type %d", effectType);
    aacDecoder_SetParam(mAACDecoder, AAC_UNIDRC_SET_EFFECT, effectType);

    // By default, the decoder creates a 5.1 channel downmix signal.
    // For seven and eight channel input streams, enable 6.1 and 7.1 channel output
    aacDecoder_SetParam(mAACDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, -1);

    return status;
}

bool C2SoftAacDec::outputDelayRingBufferPutSamples(INT_PCM *samples, int32_t numSamples) {
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
        ALOGV("slow C2SoftAacDec::outputDelayRingBufferPutSamples()");

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

int32_t C2SoftAacDec::outputDelayRingBufferGetSamples(INT_PCM *samples, int32_t numSamples) {

    if (numSamples > mOutputDelayRingBufferFilled) {
        ALOGE("RING BUFFER WOULD UNDERRUN");
        return -1;
    }

    if (mOutputDelayRingBufferReadPos + numSamples <= mOutputDelayRingBufferSize
            && (mOutputDelayRingBufferWritePos < mOutputDelayRingBufferReadPos
                    || mOutputDelayRingBufferWritePos >= mOutputDelayRingBufferReadPos + numSamples)) {
        // faster memcopy loop without checks, if the preconditions allow this
        if (samples != nullptr) {
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
        ALOGV("slow C2SoftAacDec::outputDelayRingBufferGetSamples()");

        for (int32_t i = 0; i < numSamples; i++) {
            if (samples != nullptr) {
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

int32_t C2SoftAacDec::outputDelayRingBufferSamplesAvailable() {
    return mOutputDelayRingBufferFilled;
}

int32_t C2SoftAacDec::outputDelayRingBufferSpaceLeft() {
    return mOutputDelayRingBufferSize - outputDelayRingBufferSamplesAvailable();
}

void C2SoftAacDec::drainRingBuffer(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool,
        bool eos) {
    while (!mBuffersInfo.empty() && outputDelayRingBufferSamplesAvailable()
            >= mStreamInfo->frameSize * mStreamInfo->numChannels) {
        Info &outInfo = mBuffersInfo.front();
        ALOGV("outInfo.frameIndex = %" PRIu64, outInfo.frameIndex);
        int samplesize __unused = mStreamInfo->numChannels * sizeof(int16_t);

        int available = outputDelayRingBufferSamplesAvailable();
        int numFrames = outInfo.decodedSizes.size();
        int numSamples = numFrames * (mStreamInfo->frameSize * mStreamInfo->numChannels);
        if (available < numSamples) {
            if (eos) {
                numSamples = available;
            } else {
                break;
            }
        }
        ALOGV("%d samples available (%d), or %d frames",
                numSamples, available, numFrames);
        ALOGV("getting %d from ringbuffer", numSamples);

        std::shared_ptr<C2LinearBlock> block;
        std::function<void(const std::unique_ptr<C2Work>&)> fillWork =
            [&block, numSamples, pool, this]()
                    -> std::function<void(const std::unique_ptr<C2Work>&)> {
                auto fillEmptyWork = [](
                        const std::unique_ptr<C2Work> &work, c2_status_t err) {
                    work->result = err;
                    C2FrameData &output = work->worklets.front()->output;
                    output.flags = work->input.flags;
                    output.buffers.clear();
                    output.ordinal = work->input.ordinal;

                    work->workletsProcessed = 1u;
                };

                using namespace std::placeholders;
                if (numSamples == 0) {
                    return std::bind(fillEmptyWork, _1, C2_OK);
                }

                // TODO: error handling, proper usage, etc.
                C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
                c2_status_t err = pool->fetchLinearBlock(
                        numSamples * sizeof(int16_t), usage, &block);
                if (err != C2_OK) {
                    ALOGD("failed to fetch a linear block (%d)", err);
                    return std::bind(fillEmptyWork, _1, C2_NO_MEMORY);
                }
                C2WriteView wView = block->map().get();
                // TODO
                INT_PCM *outBuffer = reinterpret_cast<INT_PCM *>(wView.data());
                int32_t ns = outputDelayRingBufferGetSamples(outBuffer, numSamples);
                if (ns != numSamples) {
                    ALOGE("not a complete frame of samples available");
                    mSignalledError = true;
                    return std::bind(fillEmptyWork, _1, C2_CORRUPTED);
                }
                return [buffer = createLinearBuffer(block)](
                        const std::unique_ptr<C2Work> &work) {
                    work->result = C2_OK;
                    C2FrameData &output = work->worklets.front()->output;
                    output.flags = work->input.flags;
                    output.buffers.clear();
                    output.buffers.push_back(buffer);
                    output.ordinal = work->input.ordinal;
                    work->workletsProcessed = 1u;
                };
            }();

        if (work && work->input.ordinal.frameIndex == c2_cntr64_t(outInfo.frameIndex)) {
            fillWork(work);
        } else {
            finish(outInfo.frameIndex, fillWork);
        }

        ALOGV("out timestamp %" PRIu64 " / %u", outInfo.timestamp, block ? block->capacity() : 0);
        mBuffersInfo.pop_front();
    }
}

void C2SoftAacDec::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.configUpdate.clear();
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError) {
        return;
    }

    UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
    UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
    UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};

    INT_PCM tmpOutBuffer[2048 * MAX_CHANNEL_COUNT];
    C2ReadView view = mDummyReadView;
    size_t offset = 0u;
    size_t size = 0u;
    if (!work->input.buffers.empty()) {
        view = work->input.buffers[0]->data().linearBlocks().front().map().get();
        size = view.capacity();
    }

    bool eos = (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0;
    bool codecConfig = (work->input.flags & C2FrameData::FLAG_CODEC_CONFIG) != 0;

    //TODO
#if 0
    if (mInputBufferCount == 0 && !codecConfig) {
        ALOGW("first buffer should have FLAG_CODEC_CONFIG set");
        codecConfig = true;
    }
#endif
    if (codecConfig && size > 0u) {
        // const_cast because of libAACdec method signature.
        inBuffer[0] = const_cast<UCHAR *>(view.data() + offset);
        inBufferLength[0] = size;

        AAC_DECODER_ERROR decoderErr =
            aacDecoder_ConfigRaw(mAACDecoder,
                                 inBuffer,
                                 inBufferLength);

        if (decoderErr != AAC_DEC_OK) {
            ALOGE("aacDecoder_ConfigRaw decoderErr = 0x%4.4x", decoderErr);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->worklets.front()->output.buffers.clear();
        return;
    }

    Info inInfo;
    inInfo.frameIndex = work->input.ordinal.frameIndex.peeku();
    inInfo.timestamp = work->input.ordinal.timestamp.peeku();
    inInfo.bufferSize = size;
    inInfo.decodedSizes.clear();
    while (size > 0u) {
        ALOGV("size = %zu", size);
        if (mIntf->isAdts()) {
            size_t adtsHeaderSize = 0;
            // skip 30 bits, aac_frame_length follows.
            // ssssssss ssssiiip ppffffPc ccohCCll llllllll lll?????

            const uint8_t *adtsHeader = view.data() + offset;

            bool signalError = false;
            if (size < 7) {
                ALOGE("Audio data too short to contain even the ADTS header. "
                        "Got %zu bytes.", size);
                hexdump(adtsHeader, size);
                signalError = true;
            } else {
                bool protectionAbsent = (adtsHeader[1] & 1);

                unsigned aac_frame_length =
                    ((adtsHeader[3] & 3) << 11)
                    | (adtsHeader[4] << 3)
                    | (adtsHeader[5] >> 5);

                if (size < aac_frame_length) {
                    ALOGE("Not enough audio data for the complete frame. "
                            "Got %zu bytes, frame size according to the ADTS "
                            "header is %u bytes.",
                            size, aac_frame_length);
                    hexdump(adtsHeader, size);
                    signalError = true;
                } else {
                    adtsHeaderSize = (protectionAbsent ? 7 : 9);
                    if (aac_frame_length < adtsHeaderSize) {
                        signalError = true;
                    } else {
                        // const_cast because of libAACdec method signature.
                        inBuffer[0] = const_cast<UCHAR *>(adtsHeader + adtsHeaderSize);
                        inBufferLength[0] = aac_frame_length - adtsHeaderSize;

                        offset += adtsHeaderSize;
                        size -= adtsHeaderSize;
                    }
                }
            }

            if (signalError) {
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
        } else {
            // const_cast because of libAACdec method signature.
            inBuffer[0] = const_cast<UCHAR *>(view.data() + offset);
            inBufferLength[0] = size;
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
        size -= inBufferUsedLength;
        offset += inBufferUsedLength;

        AAC_DECODER_ERROR decoderErr;
        do {
            if (outputDelayRingBufferSpaceLeft() <
                    (mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                ALOGV("skipping decode: not enough space left in ringbuffer");
                // discard buffer
                size = 0;
                break;
            }

            int numConsumed = mStreamInfo->numTotalBytes;
            decoderErr = aacDecoder_DecodeFrame(mAACDecoder,
                                       tmpOutBuffer,
                                       2048 * MAX_CHANNEL_COUNT,
                                       0 /* flags */);

            numConsumed = mStreamInfo->numTotalBytes - numConsumed;

            if (decoderErr == AAC_DEC_NOT_ENOUGH_BITS) {
                break;
            }
            inInfo.decodedSizes.push_back(numConsumed);

            if (decoderErr != AAC_DEC_OK) {
                ALOGW("aacDecoder_DecodeFrame decoderErr = 0x%4.4x", decoderErr);
            }

            if (bytesValid[0] != 0) {
                ALOGE("bytesValid[0] != 0 should never happen");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }

            size_t numOutBytes =
                mStreamInfo->frameSize * sizeof(int16_t) * mStreamInfo->numChannels;

            if (decoderErr == AAC_DEC_OK) {
                if (!outputDelayRingBufferPutSamples(tmpOutBuffer,
                        mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                    mSignalledError = true;
                    work->result = C2_CORRUPTED;
                    return;
                }
            } else {
                ALOGW("AAC decoder returned error 0x%4.4x, substituting silence", decoderErr);

                memset(tmpOutBuffer, 0, numOutBytes); // TODO: check for overflow

                if (!outputDelayRingBufferPutSamples(tmpOutBuffer,
                        mStreamInfo->frameSize * mStreamInfo->numChannels)) {
                    mSignalledError = true;
                    work->result = C2_CORRUPTED;
                    return;
                }

                // Discard input buffer.
                size = 0;

                aacDecoder_SetParam(mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);

                // After an error, replace bufferSize with the sum of the
                // decodedSizes to resynchronize the in/out lists.
                inInfo.decodedSizes.pop_back();
                inInfo.bufferSize = std::accumulate(
                        inInfo.decodedSizes.begin(), inInfo.decodedSizes.end(), 0);

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
                // if ((mInputBufferCount > 2) && (mOutputBufferCount <= 1)) {
                    ALOGD("Invalid AAC stream");
                    // TODO: notify(OMX_EventError, OMX_ErrorUndefined, decoderErr, NULL);
                    // mSignalledError = true;
                // }
            } else if ((mStreamInfo->sampleRate != prevSampleRate) ||
                       (mStreamInfo->numChannels != prevNumChannels)) {
                ALOGI("Reconfiguring decoder: %d->%d Hz, %d->%d channels",
                      prevSampleRate, mStreamInfo->sampleRate,
                      prevNumChannels, mStreamInfo->numChannels);

                C2StreamSampleRateInfo::output sampleRateInfo(0u, mStreamInfo->sampleRate);
                C2StreamChannelCountInfo::output channelCountInfo(0u, mStreamInfo->numChannels);
                std::vector<std::unique_ptr<C2SettingResult>> failures;
                c2_status_t err = mIntf->config(
                        { &sampleRateInfo, &channelCountInfo },
                        C2_MAY_BLOCK,
                        &failures);
                if (err == OK) {
                    // TODO: this does not handle the case where the values are
                    //       altered during config.
                    C2FrameData &output = work->worklets.front()->output;
                    output.configUpdate.push_back(C2Param::Copy(sampleRateInfo));
                    output.configUpdate.push_back(C2Param::Copy(channelCountInfo));
                } else {
                    ALOGE("Config Update failed");
                    mSignalledError = true;
                    work->result = C2_CORRUPTED;
                    return;
                }
            }
            ALOGV("size = %zu", size);
        } while (decoderErr == AAC_DEC_OK);
    }

    int32_t outputDelay = mStreamInfo->outputDelay * mStreamInfo->numChannels;

    mBuffersInfo.push_back(std::move(inInfo));
    work->workletsProcessed = 0u;
    if (!eos && mOutputDelayCompensated < outputDelay) {
        // discard outputDelay at the beginning
        int32_t toCompensate = outputDelay - mOutputDelayCompensated;
        int32_t discard = outputDelayRingBufferSamplesAvailable();
        if (discard > toCompensate) {
            discard = toCompensate;
        }
        int32_t discarded = outputDelayRingBufferGetSamples(nullptr, discard);
        mOutputDelayCompensated += discarded;
        return;
    }

    if (eos) {
        drainInternal(DRAIN_COMPONENT_WITH_EOS, pool, work);
    } else {
        drainRingBuffer(work, pool, false /* not EOS */);
    }
}

c2_status_t C2SoftAacDec::drainInternal(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool,
        const std::unique_ptr<C2Work> &work) {
    if (drainMode == NO_DRAIN) {
        ALOGW("drain with NO_DRAIN: no-op");
        return C2_OK;
    }
    if (drainMode == DRAIN_CHAIN) {
        ALOGW("DRAIN_CHAIN not supported");
        return C2_OMITTED;
    }

    bool eos = (drainMode == DRAIN_COMPONENT_WITH_EOS);

    drainDecoder();
    drainRingBuffer(work, pool, eos);

    if (eos) {
        auto fillEmptyWork = [](const std::unique_ptr<C2Work> &work) {
            work->worklets.front()->output.flags = work->input.flags;
            work->worklets.front()->output.buffers.clear();
            work->worklets.front()->output.ordinal = work->input.ordinal;
            work->workletsProcessed = 1u;
        };
        while (mBuffersInfo.size() > 1u) {
            finish(mBuffersInfo.front().frameIndex, fillEmptyWork);
            mBuffersInfo.pop_front();
        }
        if (work && work->workletsProcessed == 0u) {
            fillEmptyWork(work);
        }
        mBuffersInfo.clear();
    }

    return C2_OK;
}

c2_status_t C2SoftAacDec::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    return drainInternal(drainMode, pool, nullptr);
}

c2_status_t C2SoftAacDec::onFlush_sm() {
    drainDecoder();
    mBuffersInfo.clear();

    int avail;
    while ((avail = outputDelayRingBufferSamplesAvailable()) > 0) {
        if (avail > mStreamInfo->frameSize * mStreamInfo->numChannels) {
            avail = mStreamInfo->frameSize * mStreamInfo->numChannels;
        }
        int32_t ns = outputDelayRingBufferGetSamples(nullptr, avail);
        if (ns != avail) {
            ALOGW("not a complete frame of samples available");
            break;
        }
    }
    mOutputDelayRingBufferReadPos = mOutputDelayRingBufferWritePos;

    return C2_OK;
}

void C2SoftAacDec::drainDecoder() {
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

class C2SoftAacDecFactory : public C2ComponentFactory {
public:
    C2SoftAacDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftAacDec(COMPONENT_NAME,
                              id,
                              std::make_shared<C2SoftAacDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id, std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftAacDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftAacDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftAacDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftAacDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
