/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "C2SoftXaacDec"
#include <log/log.h>

#include <inttypes.h>

#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/foundation/hexdump.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftXaacDec.h"

#define DRC_DEFAULT_MOBILE_REF_LEVEL -16.0   /* 64*-0.25dB = -16 dB below full scale for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_CUT   1.0  /* maximum compression of dynamic range for mobile conf */
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
#define PROP_DRC_OVERRIDE_ENC_LEVEL  "aac_drc_enc_target_level"
#define PROP_DRC_OVERRIDE_EFFECT_TYPE "ro.aac_drc_effect_type"

#define RETURN_IF_FATAL(retval, str)                       \
    if (retval & IA_FATAL_ERROR) {                         \
        ALOGE("Error in %s: Returned: %d", str, retval);   \
        return retval;                                     \
    } else if (retval != IA_NO_ERROR) {                    \
        ALOGW("Warning in %s: Returned: %d", str, retval); \
    }


namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.xaac.decoder";

}  // namespace

class C2SoftXaacDec::IntfImpl : public SimpleInterface<void>::BaseParams {
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
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(
                    C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSampleRate, C2_PARAMKEY_SAMPLE_RATE)
                .withDefault(new C2StreamSampleRateInfo::output(0u, 44100))
                .withFields({C2F(mSampleRate, value).oneOf({
                    7350, 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
                })})
                .withSetter((Setter<decltype(*mSampleRate)>::StrictValueWithNoDeps))
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
    uint32_t getBitrate() const { return mBitrate->value; }
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

C2SoftXaacDec::C2SoftXaacDec(
        const char* name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
        mIntf(intfImpl),
        mXheaacCodecHandle(nullptr),
        mMpegDDrcHandle(nullptr),
        mOutputDrainBuffer(nullptr) {
}

C2SoftXaacDec::~C2SoftXaacDec() {
    onRelease();
}

c2_status_t C2SoftXaacDec::onInit() {
    mOutputFrameLength = 1024;
    mInputBuffer = nullptr;
    mOutputBuffer = nullptr;
    mSampFreq = 0;
    mNumChannels = 0;
    mPcmWdSz = 0;
    mChannelMask = 0;
    mNumOutBytes = 0;
    mCurFrameIndex = 0;
    mCurTimestamp = 0;
    mIsCodecInitialized = false;
    mIsCodecConfigFlushRequired = false;
    mSignalledOutputEos = false;
    mSignalledError = false;
    mOutputDrainBufferWritePos = 0;
    mDRCFlag = 0;
    mMpegDDRCPresent = 0;
    mMemoryVec.clear();
    mDrcMemoryVec.clear();

    IA_ERRORCODE err = initDecoder();
    return err == IA_NO_ERROR ? C2_OK : C2_CORRUPTED;

}

c2_status_t C2SoftXaacDec::onStop() {
    mOutputFrameLength = 1024;
    drainDecoder();
    // reset the "configured" state
    mSampFreq = 0;
    mNumChannels = 0;
    mPcmWdSz = 0;
    mChannelMask = 0;
    mNumOutBytes = 0;
    mCurFrameIndex = 0;
    mCurTimestamp = 0;
    mSignalledOutputEos = false;
    mSignalledError = false;
    mOutputDrainBufferWritePos = 0;
    mDRCFlag = 0;
    mMpegDDRCPresent = 0;

    return C2_OK;
}

void C2SoftXaacDec::onReset() {
    (void)onStop();
}

void C2SoftXaacDec::onRelease() {
    IA_ERRORCODE errCode = deInitXAACDecoder();
    if (IA_NO_ERROR != errCode) ALOGE("deInitXAACDecoder() failed %d", errCode);

    errCode = deInitMPEGDDDrc();
    if (IA_NO_ERROR != errCode) ALOGE("deInitMPEGDDDrc() failed %d", errCode);

    if (mOutputDrainBuffer) {
        delete[] mOutputDrainBuffer;
        mOutputDrainBuffer = nullptr;
    }
}

IA_ERRORCODE C2SoftXaacDec::initDecoder() {
    ALOGV("initDecoder()");
    IA_ERRORCODE err_code = IA_NO_ERROR;

    err_code = initXAACDecoder();
    if (err_code != IA_NO_ERROR) {
        ALOGE("initXAACDecoder Failed");
        /* Call deInit to free any allocated memory */
        deInitXAACDecoder();
        return IA_FATAL_ERROR;
    }

    if (!mOutputDrainBuffer) {
        mOutputDrainBuffer = new (std::nothrow) char[kOutputDrainBufferSize];
        if (!mOutputDrainBuffer) return IA_FATAL_ERROR;
    }

    err_code = initXAACDrc();
    RETURN_IF_FATAL(err_code,  "initXAACDrc");


    return IA_NO_ERROR;
}

static void fillEmptyWork(const std::unique_ptr<C2Work>& work) {
    uint32_t flags = 0;
    if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
        flags |= C2FrameData::FLAG_END_OF_STREAM;
        ALOGV("signalling eos");
    }
    work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = work->input.ordinal;
    work->workletsProcessed = 1u;
}

void C2SoftXaacDec::finishWork(const std::unique_ptr<C2Work>& work,
                            const std::shared_ptr<C2BlockPool>& pool) {
    ALOGV("mCurFrameIndex = %" PRIu64, mCurFrameIndex);

    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE};
    // TODO: error handling, proper usage, etc.
    c2_status_t err =
        pool->fetchLinearBlock(mOutputDrainBufferWritePos, usage, &block);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock failed : err = %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }
    C2WriteView wView = block->map().get();
    int16_t* outBuffer = reinterpret_cast<int16_t*>(wView.data());
    memcpy(outBuffer, mOutputDrainBuffer, mOutputDrainBufferWritePos);
    mOutputDrainBufferWritePos = 0;

    auto fillWork = [buffer = createLinearBuffer(block)](
        const std::unique_ptr<C2Work>& work) {
        uint32_t flags = 0;
        if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
            flags |= C2FrameData::FLAG_END_OF_STREAM;
            ALOGV("signalling eos");
        }
        work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
    };
    if (work && work->input.ordinal.frameIndex == c2_cntr64_t(mCurFrameIndex)) {
        fillWork(work);
    } else {
        finish(mCurFrameIndex, fillWork);
    }

    ALOGV("out timestamp %" PRIu64 " / %u", mCurTimestamp, block->capacity());
}

void C2SoftXaacDec::process(const std::unique_ptr<C2Work>& work,
                         const std::shared_ptr<C2BlockPool>& pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.configUpdate.clear();
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledOutputEos) {
        work->result = C2_BAD_VALUE;
        return;
    }
    uint8_t* inBuffer = nullptr;
    uint32_t inBufferLength = 0;
    C2ReadView view = mDummyReadView;
    size_t offset = 0u;
    size_t size = 0u;
    if (!work->input.buffers.empty()) {
        view = work->input.buffers[0]->data().linearBlocks().front().map().get();
        size = view.capacity();
    }
    if (size && view.error()) {
        ALOGE("read view map failed %d", view.error());
        work->result = view.error();
        return;
    }

    bool eos = (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0;
    bool codecConfig =
        (work->input.flags & C2FrameData::FLAG_CODEC_CONFIG) != 0;
    if (codecConfig) {
        if (size == 0u) {
            ALOGE("empty codec config");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        // const_cast because of libAACdec method signature.
        inBuffer = const_cast<uint8_t*>(view.data() + offset);
        inBufferLength = size;

        /* GA header configuration sent to Decoder! */
        IA_ERRORCODE err_code = configXAACDecoder(inBuffer, inBufferLength);
        if (IA_NO_ERROR != err_code) {
            ALOGE("configXAACDecoder err_code = %d", err_code);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->worklets.front()->output.buffers.clear();
        return;
    }

    mCurFrameIndex = work->input.ordinal.frameIndex.peeku();
    mCurTimestamp = work->input.ordinal.timestamp.peeku();
    mOutputDrainBufferWritePos = 0;
    char* tempOutputDrainBuffer = mOutputDrainBuffer;
    while (size > 0u) {
        if ((kOutputDrainBufferSize * sizeof(int16_t) -
             mOutputDrainBufferWritePos) <
            (mOutputFrameLength * sizeof(int16_t) * mNumChannels)) {
            ALOGV("skipping decode: not enough space left in DrainBuffer");
            break;
        }

        ALOGV("inAttribute size = %zu", size);
        if (mIntf->isAdts()) {
            ALOGV("ADTS");
            size_t adtsHeaderSize = 0;
            // skip 30 bits, aac_frame_length follows.
            // ssssssss ssssiiip ppffffPc ccohCCll llllllll lll?????

            const uint8_t* adtsHeader = view.data() + offset;
            bool signalError = false;
            if (size < 7) {
                ALOGE("Audio data too short to contain even the ADTS header. "
                      "Got %zu bytes.", size);
                hexdump(adtsHeader, size);
                signalError = true;
            } else {
                bool protectionAbsent = (adtsHeader[1] & 1);
                unsigned aac_frame_length = ((adtsHeader[3] & 3) << 11) |
                                            (adtsHeader[4] << 3) |
                                            (adtsHeader[5] >> 5);

                if (size < aac_frame_length) {
                    ALOGE("Not enough audio data for the complete frame. "
                          "Got %zu bytes, frame size according to the ADTS "
                          "header is %u bytes.", size, aac_frame_length);
                    hexdump(adtsHeader, size);
                    signalError = true;
                } else {
                    adtsHeaderSize = (protectionAbsent ? 7 : 9);
                    if (aac_frame_length < adtsHeaderSize) {
                        signalError = true;
                    } else {
                        // const_cast because of libAACdec method signature.
                        inBuffer =
                            const_cast<uint8_t*>(adtsHeader + adtsHeaderSize);
                        inBufferLength = aac_frame_length - adtsHeaderSize;

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
            ALOGV("Non ADTS");
            // const_cast because of libAACdec method signature.
            inBuffer = const_cast<uint8_t*>(view.data() + offset);
            inBufferLength = size;
        }

        signed int prevSampleRate = mSampFreq;
        signed int prevNumChannels = mNumChannels;

        /* XAAC decoder expects first frame to be fed via configXAACDecoder API
         * which should initialize the codec. Once this state is reached, call the
         * decodeXAACStream API with same frame to decode! */
        if (!mIsCodecInitialized) {
            IA_ERRORCODE err_code = configXAACDecoder(inBuffer, inBufferLength);
            if (IA_NO_ERROR != err_code) {
                ALOGE("configXAACDecoder Failed 2 err_code = %d", err_code);
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }

            if ((mSampFreq != prevSampleRate) ||
                (mNumChannels != prevNumChannels)) {
                ALOGI("Reconfiguring decoder: %d->%d Hz, %d->%d channels",
                      prevSampleRate, mSampFreq, prevNumChannels, mNumChannels);

                C2StreamSampleRateInfo::output sampleRateInfo(0u, mSampFreq);
                C2StreamChannelCountInfo::output channelCountInfo(0u, mNumChannels);
                std::vector<std::unique_ptr<C2SettingResult>> failures;
                c2_status_t err = mIntf->config(
                        { &sampleRateInfo, &channelCountInfo },
                        C2_MAY_BLOCK,
                        &failures);
                if (err == OK) {
                    work->worklets.front()->output.configUpdate.push_back(
                        C2Param::Copy(sampleRateInfo));
                    work->worklets.front()->output.configUpdate.push_back(
                        C2Param::Copy(channelCountInfo));
                } else {
                    ALOGE("Config Update failed");
                    mSignalledError = true;
                    work->result = C2_CORRUPTED;
                    return;
                }
            }
        }

        signed int bytesConsumed = 0;
        IA_ERRORCODE errorCode = IA_NO_ERROR;
        if (mIsCodecInitialized) {
            mIsCodecConfigFlushRequired = true;
            errorCode = decodeXAACStream(inBuffer, inBufferLength,
                                         &bytesConsumed, &mNumOutBytes);
        } else if (!mIsCodecConfigFlushRequired) {
            ALOGW("Assumption that first frame after header initializes decoder Failed!");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        size -= bytesConsumed;
        offset += bytesConsumed;

        if (inBufferLength != (uint32_t)bytesConsumed)
            ALOGW("All data not consumed");

        /* In case of error, decoder would have given out empty buffer */
        if ((IA_NO_ERROR != errorCode) && (0 == mNumOutBytes) && mIsCodecInitialized)
            mNumOutBytes = mOutputFrameLength * (mPcmWdSz / 8) * mNumChannels;

        if (!bytesConsumed) {
            ALOGW("bytesConsumed = 0 should never happen");
        }

        if ((uint32_t)mNumOutBytes >
            mOutputFrameLength * sizeof(int16_t) * mNumChannels) {
            ALOGE("mNumOutBytes > mOutputFrameLength * sizeof(int16_t) * mNumChannels, should never happen");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }

        if (IA_NO_ERROR != errorCode) {
            // TODO: check for overflow, ASAN
            memset(mOutputBuffer, 0, mNumOutBytes);

            // Discard input buffer.
            size = 0;

            // fall through
        }
        memcpy(tempOutputDrainBuffer, mOutputBuffer, mNumOutBytes);
        tempOutputDrainBuffer += mNumOutBytes;
        mOutputDrainBufferWritePos += mNumOutBytes;
    }

    if (mOutputDrainBufferWritePos) {
        finishWork(work, pool);
    } else {
        fillEmptyWork(work);
    }
    if (eos) mSignalledOutputEos = true;
}

c2_status_t C2SoftXaacDec::drain(uint32_t drainMode,
                              const std::shared_ptr<C2BlockPool>& pool) {
    (void)pool;
    if (drainMode == NO_DRAIN) {
        ALOGW("drain with NO_DRAIN: no-op");
        return C2_OK;
    }
    if (drainMode == DRAIN_CHAIN) {
        ALOGW("DRAIN_CHAIN not supported");
        return C2_OMITTED;
    }

    return C2_OK;
}

IA_ERRORCODE C2SoftXaacDec::configflushDecode() {
    IA_ERRORCODE err_code;
    uint32_t ui_init_done;
    uint32_t inBufferLength = 8203;

    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_FLUSH_MEM,
                                nullptr);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_FLUSH_MEM");

    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_INPUT_BYTES,
                                0,
                                &inBufferLength);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_SET_INPUT_BYTES");

    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_FLUSH_MEM,
                                nullptr);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_FLUSH_MEM");

    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_INIT_DONE_QUERY,
                                &ui_init_done);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_INIT_DONE_QUERY");

    if (ui_init_done) {
        err_code = getXAACStreamInfo();
        RETURN_IF_FATAL(err_code, "getXAACStreamInfo");
        ALOGV("Found Codec with below config---\nsampFreq %d\nnumChannels %d\npcmWdSz %d\nchannelMask %d\noutputFrameLength %d",
               mSampFreq, mNumChannels, mPcmWdSz, mChannelMask, mOutputFrameLength);
        mIsCodecInitialized = true;
    }
    return IA_NO_ERROR;
}

c2_status_t C2SoftXaacDec::onFlush_sm() {
    if (mIsCodecInitialized) {
        IA_ERRORCODE err_code = configflushDecode();
        if (err_code != IA_NO_ERROR) {
            ALOGE("Error in configflushDecode: Error %d", err_code);
        }
    }
    drainDecoder();
    mSignalledOutputEos = false;
    mSignalledError = false;

    return C2_OK;
}

IA_ERRORCODE C2SoftXaacDec::drainDecoder() {
    /* Output delay compensation logic should sit here. */
    /* Nothing to be done as XAAC decoder does not introduce output buffer delay */

    return 0;
}

IA_ERRORCODE C2SoftXaacDec::initXAACDecoder() {
    /* First part                                        */
    /* Error Handler Init                                */
    /* Get Library Name, Library Version and API Version */
    /* Initialize API structure + Default config set     */
    /* Set config params from user                       */
    /* Initialize memory tables                          */
    /* Get memory information and allocate memory        */

    mInputBufferSize = 0;
    mInputBuffer = nullptr;
    mOutputBuffer = nullptr;
    /* Process struct initing end */

    /* ******************************************************************/
    /* Initialize API structure and set config params to default        */
    /* ******************************************************************/
    /* API size */
    uint32_t pui_api_size;
    /* Get the API size */
    IA_ERRORCODE err_code = ixheaacd_dec_api(nullptr,
                                             IA_API_CMD_GET_API_SIZE,
                                             0,
                                             &pui_api_size);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_API_SIZE");

    /* Allocate memory for API */
    mXheaacCodecHandle = memalign(4, pui_api_size);
    if (!mXheaacCodecHandle) {
        ALOGE("malloc for pui_api_size + 4 >> %d Failed", pui_api_size + 4);
        return IA_FATAL_ERROR;
    }
    mMemoryVec.push(mXheaacCodecHandle);

    /* Set the config params to default values */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS,
                                nullptr);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS");

    /* Get the API size */
    err_code = ia_drc_dec_api(nullptr, IA_API_CMD_GET_API_SIZE, 0, &pui_api_size);

    RETURN_IF_FATAL(err_code, "IA_API_CMD_GET_API_SIZE");

    /* Allocate memory for API */
    mMpegDDrcHandle = memalign(4, pui_api_size);
    if (!mMpegDDrcHandle) {
        ALOGE("malloc for pui_api_size + 4 >> %d Failed", pui_api_size + 4);
        return IA_FATAL_ERROR;
    }
    mMemoryVec.push(mMpegDDrcHandle);

    /* Set the config params to default values */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                              IA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS, nullptr);

    RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS");

    /* ******************************************************************/
    /* Set config parameters                                            */
    /* ******************************************************************/
    uint32_t ui_mp4_flag = 1;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_ISMP4,
                                &ui_mp4_flag);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_ISMP4");

    /* ******************************************************************/
    /* Initialize Memory info tables                                    */
    /* ******************************************************************/
    uint32_t ui_proc_mem_tabs_size;
    pVOID pv_alloc_ptr;
    /* Get memory info tables size */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_MEMTABS_SIZE,
                                0,
                                &ui_proc_mem_tabs_size);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_MEMTABS_SIZE");

    pv_alloc_ptr = memalign(4, ui_proc_mem_tabs_size);
    if (!pv_alloc_ptr) {
        ALOGE("Malloc for size (ui_proc_mem_tabs_size + 4) = %d failed!", ui_proc_mem_tabs_size + 4);
        return IA_FATAL_ERROR;
    }
    mMemoryVec.push(pv_alloc_ptr);

    /* Set pointer for process memory tables    */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_MEMTABS_PTR,
                                0,
                                pv_alloc_ptr);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_SET_MEMTABS_PTR");

    /* initialize the API, post config, fill memory tables  */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_INIT_API_POST_CONFIG_PARAMS,
                                nullptr);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_INIT_API_POST_CONFIG_PARAMS");

    /* ******************************************************************/
    /* Allocate Memory with info from library                           */
    /* ******************************************************************/
    /* There are four different types of memories, that needs to be allocated */
    /* persistent,scratch,input and output */
    for (int i = 0; i < 4; i++) {
        int ui_size = 0, ui_alignment = 0, ui_type = 0;

        /* Get memory size */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_GET_MEM_INFO_SIZE,
                                    i,
                                    &ui_size);
        RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_MEM_INFO_SIZE");

        /* Get memory alignment */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_GET_MEM_INFO_ALIGNMENT,
                                    i,
                                    &ui_alignment);
        RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_MEM_INFO_ALIGNMENT");

        /* Get memory type */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_GET_MEM_INFO_TYPE,
                                    i,
                                    &ui_type);
        RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_MEM_INFO_TYPE");

        pv_alloc_ptr = memalign(ui_alignment, ui_size);
        if (!pv_alloc_ptr) {
            ALOGE("Malloc for size (ui_size + ui_alignment) = %d failed!",
                   ui_size + ui_alignment);
            return IA_FATAL_ERROR;
        }
        mMemoryVec.push(pv_alloc_ptr);

        /* Set the buffer pointer */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_SET_MEM_PTR,
                                    i,
                                    pv_alloc_ptr);
        RETURN_IF_FATAL(err_code,  "IA_API_CMD_SET_MEM_PTR");
        if (ui_type == IA_MEMTYPE_INPUT) {
            mInputBuffer = (pWORD8)pv_alloc_ptr;
            mInputBufferSize = ui_size;
        }
        if (ui_type == IA_MEMTYPE_OUTPUT)
            mOutputBuffer = (pWORD8)pv_alloc_ptr;
    }
    /* End first part */

    return IA_NO_ERROR;
}

status_t C2SoftXaacDec::initXAACDrc() {
    IA_ERRORCODE err_code = IA_NO_ERROR;
    unsigned int ui_drc_val;
    //  DRC_PRES_MODE_WRAP_DESIRED_TARGET
    int32_t targetRefLevel = mIntf->getDrcTargetRefLevel();
    ALOGV("AAC decoder using desired DRC target reference level of %d", targetRefLevel);
    ui_drc_val = (unsigned int)targetRefLevel;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LEVEL,
                                &ui_drc_val);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LEVEL");

    /* Use ui_drc_val from PROP_DRC_OVERRIDE_REF_LEVEL or DRC_DEFAULT_MOBILE_REF_LEVEL
     * for IA_ENHAACPLUS_DEC_DRC_TARGET_LOUDNESS too */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_DRC_TARGET_LOUDNESS, &ui_drc_val);

    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_DRC_TARGET_LOUDNESS");

    int32_t attenuationFactor = mIntf->getDrcAttenuationFactor();
    ALOGV("AAC decoder using desired DRC attenuation factor of %d", attenuationFactor);
    ui_drc_val = (unsigned int)attenuationFactor;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_CUT,
                                &ui_drc_val);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_CUT");

    //  DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR
    int32_t boostFactor = mIntf->getDrcBoostFactor();
    ALOGV("AAC decoder using desired DRC boost factor of %d", boostFactor);
    ui_drc_val = (unsigned int)boostFactor;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_BOOST,
                                &ui_drc_val);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_BOOST");

    //  DRC_PRES_MODE_WRAP_DESIRED_HEAVY
    int32_t compressMode = mIntf->getDrcCompressMode();
    ALOGV("AAC decoder using desried DRC heavy compression switch of %d", compressMode);
    ui_drc_val = (unsigned int)compressMode;

    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_HEAVY_COMP,
                                &ui_drc_val);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_HEAVY_COMP");

    // AAC_UNIDRC_SET_EFFECT
    int32_t effectType = mIntf->getDrcEffectType();
    ALOGV("AAC decoder using MPEG-D DRC effect type %d", effectType);
    ui_drc_val = (unsigned int)effectType;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_DRC_EFFECT_TYPE, &ui_drc_val);

    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_DRC_EFFECT_TYPE");

    return IA_NO_ERROR;
}

IA_ERRORCODE C2SoftXaacDec::deInitXAACDecoder() {
    ALOGV("deInitXAACDecoder");

    /* Error code */
    IA_ERRORCODE err_code = IA_NO_ERROR;

    if (mXheaacCodecHandle) {
        /* Tell that the input is over in this buffer */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_INPUT_OVER,
                                    0,
                                    nullptr);
    }

    /* Irrespective of error returned in IA_API_CMD_INPUT_OVER, free allocated memory */
    for (void* buf : mMemoryVec) {
        if (buf) free(buf);
    }
    mMemoryVec.clear();
    mXheaacCodecHandle = nullptr;

    return err_code;
}

IA_ERRORCODE C2SoftXaacDec::deInitMPEGDDDrc() {
    ALOGV("deInitMPEGDDDrc");

    for (void* buf : mDrcMemoryVec) {
        if (buf) free(buf);
    }
    mDrcMemoryVec.clear();
    return IA_NO_ERROR;
}

IA_ERRORCODE C2SoftXaacDec::configXAACDecoder(uint8_t* inBuffer, uint32_t inBufferLength) {
    if (mInputBufferSize < inBufferLength) {
        ALOGE("Cannot config AAC, input buffer size %d < inBufferLength %d", mInputBufferSize, inBufferLength);
        return false;
    }
    /* Copy the buffer passed by Android plugin to codec input buffer */
    memcpy(mInputBuffer, inBuffer, inBufferLength);

    /* Set number of bytes to be processed */
    IA_ERRORCODE err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                             IA_API_CMD_SET_INPUT_BYTES,
                                             0,
                                             &inBufferLength);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_SET_INPUT_BYTES");

    if (mIsCodecConfigFlushRequired) {
        /* If codec is already initialized, then GA header is passed again */
        /* Need to call the Flush API instead of INIT_PROCESS */
        mIsCodecInitialized = false; /* Codec needs to be Reinitialized after flush */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_INIT,
                                    IA_CMD_TYPE_GA_HDR,
                                    nullptr);
        RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_GA_HDR");
    } else {
        /* Initialize the process */
        err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                    IA_API_CMD_INIT,
                                    IA_CMD_TYPE_INIT_PROCESS,
                                    nullptr);
        RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_INIT_PROCESS");
    }

    uint32_t ui_init_done;
    /* Checking for end of initialization */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_INIT,
                                IA_CMD_TYPE_INIT_DONE_QUERY,
                                &ui_init_done);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_INIT_DONE_QUERY");

    /* How much buffer is used in input buffers */
    int32_t i_bytes_consumed;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CURIDX_INPUT_BUF,
                                0,
                                &i_bytes_consumed);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_CURIDX_INPUT_BUF");

    if (ui_init_done) {
        err_code = getXAACStreamInfo();
        RETURN_IF_FATAL(err_code, "getXAACStreamInfo");
        ALOGI("Found Codec with below config---\nsampFreq %d\nnumChannels %d\npcmWdSz %d\nchannelMask %d\noutputFrameLength %d",
               mSampFreq, mNumChannels, mPcmWdSz, mChannelMask, mOutputFrameLength);
        mIsCodecInitialized = true;

        err_code = configMPEGDDrc();
        RETURN_IF_FATAL(err_code, "configMPEGDDrc");
    }

    return IA_NO_ERROR;
}
IA_ERRORCODE C2SoftXaacDec::initMPEGDDDrc() {
    IA_ERRORCODE err_code = IA_NO_ERROR;

    for (int i = 0; i < (WORD32)2; i++) {
        WORD32 ui_size, ui_alignment, ui_type;
        pVOID pv_alloc_ptr;

        /* Get memory size */
        err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_GET_MEM_INFO_SIZE, i, &ui_size);

        RETURN_IF_FATAL(err_code, "IA_API_CMD_GET_MEM_INFO_SIZE");

        /* Get memory alignment */
        err_code =
            ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_GET_MEM_INFO_ALIGNMENT, i, &ui_alignment);

        RETURN_IF_FATAL(err_code, "IA_API_CMD_GET_MEM_INFO_ALIGNMENT");

        /* Get memory type */
        err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_GET_MEM_INFO_TYPE, i, &ui_type);
        RETURN_IF_FATAL(err_code, "IA_API_CMD_GET_MEM_INFO_TYPE");

        pv_alloc_ptr = memalign(4, ui_size);
        if (pv_alloc_ptr == nullptr) {
            ALOGE(" Cannot create requested memory  %d", ui_size);
            return IA_FATAL_ERROR;
        }
        mDrcMemoryVec.push(pv_alloc_ptr);

        /* Set the buffer pointer */
        err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_MEM_PTR, i, pv_alloc_ptr);

        RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_MEM_PTR");
    }

    WORD32 ui_size;
    ui_size = 8192 * 2;

    mDrcInBuf = (int8_t*)memalign(4, ui_size);
    if (mDrcInBuf == nullptr) {
        ALOGE(" Cannot create requested memory  %d", ui_size);
        return IA_FATAL_ERROR;
    }
    mDrcMemoryVec.push(mDrcInBuf);

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_MEM_PTR, 2, mDrcInBuf);
    RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_MEM_PTR");

    mDrcOutBuf = (int8_t*)memalign(4, ui_size);
    if (mDrcOutBuf == nullptr) {
        ALOGE(" Cannot create requested memory  %d", ui_size);
        return IA_FATAL_ERROR;
    }
    mDrcMemoryVec.push(mDrcOutBuf);

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_MEM_PTR, 3, mDrcOutBuf);
    RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_MEM_PTR");

    return IA_NO_ERROR;
}
int C2SoftXaacDec::configMPEGDDrc() {
    IA_ERRORCODE err_code = IA_NO_ERROR;
    int i_effect_type;
    int i_loud_norm;
    int i_target_loudness;
    unsigned int i_sbr_mode;
    uint32_t ui_proc_mem_tabs_size = 0;
    pVOID pv_alloc_ptr = NULL;

    /* Sampling Frequency */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_PARAM_SAMP_FREQ, &mSampFreq);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_SAMP_FREQ");
    /* Total Number of Channels */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_PARAM_NUM_CHANNELS, &mNumChannels);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_NUM_CHANNELS");

    /* PCM word size  */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_PARAM_PCM_WDSZ, &mPcmWdSz);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_PCM_WDSZ");

    /*Set Effect Type*/

    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_EFFECT_TYPE, &i_effect_type);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_EFFECT_TYPE");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_DRC_EFFECT_TYPE, &i_effect_type);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_EFFECT_TYPE");

    /*Set target loudness */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LOUDNESS,
                                &i_target_loudness);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LOUDNESS");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_DRC_TARGET_LOUDNESS, &i_target_loudness);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_TARGET_LOUDNESS");

    /*Set loud_norm_flag*/
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_LOUD_NORM, &i_loud_norm);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_LOUD_NORM");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_DRC_LOUD_NORM, &i_loud_norm);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_LOUD_NORM");

    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_SBR_MODE, &i_sbr_mode);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_SBR_MODE");

    /* Get memory info tables size */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_GET_MEMTABS_SIZE, 0,
                              &ui_proc_mem_tabs_size);
    RETURN_IF_FATAL(err_code, "IA_API_CMD_GET_MEMTABS_SIZE");

    pv_alloc_ptr = memalign(4, ui_proc_mem_tabs_size);
    if (pv_alloc_ptr == NULL) {
        ALOGE(" Cannot create requested memory  %d", ui_proc_mem_tabs_size);
        return IA_FATAL_ERROR;
    }
    memset(pv_alloc_ptr, 0, ui_proc_mem_tabs_size);
    mMemoryVec.push(pv_alloc_ptr);

    /* Set pointer for process memory tables */
    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_MEMTABS_PTR, 0,
                              pv_alloc_ptr);
    RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_MEMTABS_PTR");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                              IA_CMD_TYPE_INIT_API_POST_CONFIG_PARAMS, nullptr);

    RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_API_POST_CONFIG_PARAMS");

    /* Free any memory that is allocated for MPEG D Drc so far */
    deInitMPEGDDDrc();

    err_code = initMPEGDDDrc();
    if (err_code != IA_NO_ERROR) {
        ALOGE("initMPEGDDDrc failed with error %d", err_code);
        deInitMPEGDDDrc();
        return err_code;
    }

    /* DRC buffers
        buf[0] - contains extension element pay load loudness related
        buf[1] - contains extension element pay load*/
    {
        VOID* p_array[2][16];
        WORD32 ii;
        WORD32 buf_sizes[2][16];
        WORD32 num_elements;
        WORD32 num_config_ext;
        WORD32 bit_str_fmt = 1;

        WORD32 uo_num_chan;

        memset(buf_sizes, 0, 32 * sizeof(WORD32));

        err_code =
            ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                             IA_ENHAACPLUS_DEC_CONFIG_EXT_ELE_BUF_SIZES, &buf_sizes[0][0]);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_EXT_ELE_BUF_SIZES");

        err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                    IA_ENHAACPLUS_DEC_CONFIG_EXT_ELE_PTR, &p_array);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_EXT_ELE_PTR");

        err_code =
            ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT, IA_CMD_TYPE_INIT_SET_BUFF_PTR, nullptr);
        RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_SET_BUFF_PTR");

        err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                    IA_ENHAACPLUS_DEC_CONFIG_NUM_ELE, &num_elements);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_NUM_ELE");

        err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                    IA_ENHAACPLUS_DEC_CONFIG_NUM_CONFIG_EXT, &num_config_ext);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_NUM_CONFIG_EXT");

        for (ii = 0; ii < num_config_ext; ii++) {
            /*copy loudness bitstream*/
            if (buf_sizes[0][ii] > 0) {
                memcpy(mDrcInBuf, p_array[0][ii], buf_sizes[0][ii]);

                /*Set bitstream_split_format */
                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                          IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT, &bit_str_fmt);
                RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

                /* Set number of bytes to be processed */
                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_INPUT_BYTES_IL_BS, 0,
                                          &buf_sizes[0][ii]);
                RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_INPUT_BYTES_IL_BS");

                /* Execute process */
                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                                          IA_CMD_TYPE_INIT_CPY_IL_BSF_BUFF, nullptr);
                RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_CPY_IL_BSF_BUFF");

                mDRCFlag = 1;
            }
        }

        for (ii = 0; ii < num_elements; ii++) {
            /*copy config bitstream*/
            if (buf_sizes[1][ii] > 0) {
                memcpy(mDrcInBuf, p_array[1][ii], buf_sizes[1][ii]);
                /* Set number of bytes to be processed */

                /*Set bitstream_split_format */
                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                          IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT, &bit_str_fmt);
                RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_INPUT_BYTES_IC_BS, 0,
                                          &buf_sizes[1][ii]);
                RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_INPUT_BYTES_IC_BS");

                /* Execute process */
                err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                                          IA_CMD_TYPE_INIT_CPY_IC_BSF_BUFF, nullptr);

                RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_CPY_IC_BSF_BUFF");

                mDRCFlag = 1;
            }
        }

        if (mDRCFlag == 1) {
            mMpegDDRCPresent = 1;
        } else {
            mMpegDDRCPresent = 0;
        }

        /*Read interface buffer config file bitstream*/
        if (mMpegDDRCPresent == 1) {
            WORD32 interface_is_present = 1;

            if (i_sbr_mode != 0) {
                if (i_sbr_mode == 1) {
                    mOutputFrameLength = 2048;
                } else if (i_sbr_mode == 3) {
                    mOutputFrameLength = 4096;
                } else {
                    mOutputFrameLength = 1024;
                }
            } else {
                mOutputFrameLength = 4096;
            }

            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                      IA_DRC_DEC_CONFIG_PARAM_FRAME_SIZE, (WORD32 *)&mOutputFrameLength);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_FRAME_SIZE");

            err_code =
                ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                               IA_DRC_DEC_CONFIG_PARAM_INT_PRESENT, &interface_is_present);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_INT_PRESENT");

            /* Execute process */
            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                                      IA_CMD_TYPE_INIT_CPY_IN_BSF_BUFF, nullptr);
            RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_CPY_IN_BSF_BUFF");

            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                                      IA_CMD_TYPE_INIT_PROCESS, nullptr);
            RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_INIT_PROCESS");

            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                      IA_DRC_DEC_CONFIG_PARAM_NUM_CHANNELS, &uo_num_chan);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_NUM_CHANNELS");
        }
    }

    return err_code;
}

IA_ERRORCODE C2SoftXaacDec::decodeXAACStream(uint8_t* inBuffer,
                                 uint32_t inBufferLength,
                                 int32_t* bytesConsumed,
                                 int32_t* outBytes) {
    if (mInputBufferSize < inBufferLength) {
        ALOGE("Cannot config AAC, input buffer size %d < inBufferLength %d", mInputBufferSize, inBufferLength);
        return -1;
    }
    /* Copy the buffer passed by Android plugin to codec input buffer */
    memcpy(mInputBuffer, inBuffer, inBufferLength);

    /* Set number of bytes to be processed */
    IA_ERRORCODE err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                             IA_API_CMD_SET_INPUT_BYTES,
                                             0,
                                             &inBufferLength);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_SET_INPUT_BYTES");

    /* Execute process */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_EXECUTE,
                                IA_CMD_TYPE_DO_EXECUTE,
                                nullptr);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_DO_EXECUTE");

    /* Checking for end of processing */
    uint32_t ui_exec_done;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_EXECUTE,
                                IA_CMD_TYPE_DONE_QUERY,
                                &ui_exec_done);
    RETURN_IF_FATAL(err_code,  "IA_CMD_TYPE_DONE_QUERY");

    if (ui_exec_done != 1) {
        VOID* p_array;        // ITTIAM:buffer to handle gain payload
        WORD32 buf_size = 0;  // ITTIAM:gain payload length
        WORD32 bit_str_fmt = 1;
        WORD32 gain_stream_flag = 1;

        err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                    IA_ENHAACPLUS_DEC_CONFIG_GAIN_PAYLOAD_LEN, &buf_size);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_GAIN_PAYLOAD_LEN");

        err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                    IA_ENHAACPLUS_DEC_CONFIG_GAIN_PAYLOAD_BUF, &p_array);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_GAIN_PAYLOAD_BUF");

        if (buf_size > 0) {
            /*Set bitstream_split_format */
            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                      IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT, &bit_str_fmt);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

            memcpy(mDrcInBuf, p_array, buf_size);
            /* Set number of bytes to be processed */
            err_code =
                ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_INPUT_BYTES_BS, 0, &buf_size);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                      IA_DRC_DEC_CONFIG_GAIN_STREAM_FLAG, &gain_stream_flag);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

            /* Execute process */
            err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_INIT,
                                      IA_CMD_TYPE_INIT_CPY_BSF_BUFF, nullptr);
            RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_PARAM_BITS_FORMAT");

            mMpegDDRCPresent = 1;
        }
    }

    /* How much buffer is used in input buffers */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CURIDX_INPUT_BUF,
                                0,
                                bytesConsumed);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_CURIDX_INPUT_BUF");

    /* Get the output bytes */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_OUTPUT_BYTES,
                                0,
                                outBytes);
    RETURN_IF_FATAL(err_code,  "IA_API_CMD_GET_OUTPUT_BYTES");

    if (mMpegDDRCPresent == 1) {
        memcpy(mDrcInBuf, mOutputBuffer, *outBytes);
        err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_INPUT_BYTES, 0, outBytes);
        RETURN_IF_FATAL(err_code, "IA_API_CMD_SET_INPUT_BYTES");

        err_code =
            ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_EXECUTE, IA_CMD_TYPE_DO_EXECUTE, nullptr);
        RETURN_IF_FATAL(err_code, "IA_CMD_TYPE_DO_EXECUTE");

        memcpy(mOutputBuffer, mDrcOutBuf, *outBytes);
    }
    return IA_NO_ERROR;
}

IA_ERRORCODE C2SoftXaacDec::getXAACStreamInfo() {
    IA_ERRORCODE err_code = IA_NO_ERROR;

    /* Sampling frequency */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_SAMP_FREQ,
                                &mSampFreq);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_SAMP_FREQ");

    /* Total Number of Channels */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_NUM_CHANNELS,
                                &mNumChannels);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_NUM_CHANNELS");
    if (mNumChannels > MAX_CHANNEL_COUNT) {
        ALOGE(" No of channels are more than max channels\n");
        return IA_FATAL_ERROR;
    }

    /* PCM word size */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_PCM_WDSZ,
                                &mPcmWdSz);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_PCM_WDSZ");
    if ((mPcmWdSz / 8) != 2) {
        ALOGE(" No of channels are more than max channels\n");
        return IA_FATAL_ERROR;
    }

    /* channel mask to tell the arrangement of channels in bit stream */
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_CHANNEL_MASK,
                                &mChannelMask);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_CHANNEL_MASK");

    /* Channel mode to tell MONO/STEREO/DUAL-MONO/NONE_OF_THESE */
    uint32_t ui_channel_mode;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_CHANNEL_MODE,
                                &ui_channel_mode);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_CHANNEL_MODE");
    if (ui_channel_mode == 0)
        ALOGV("Channel Mode: MONO_OR_PS\n");
    else if (ui_channel_mode == 1)
        ALOGV("Channel Mode: STEREO\n");
    else if (ui_channel_mode == 2)
        ALOGV("Channel Mode: DUAL-MONO\n");
    else
        ALOGV("Channel Mode: NONE_OF_THESE or MULTICHANNEL\n");

    /* Channel mode to tell SBR PRESENT/NOT_PRESENT */
    uint32_t ui_sbr_mode;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle,
                                IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_SBR_MODE,
                                &ui_sbr_mode);
    RETURN_IF_FATAL(err_code,  "IA_ENHAACPLUS_DEC_CONFIG_PARAM_SBR_MODE");
    if (ui_sbr_mode == 0)
        ALOGV("SBR Mode: NOT_PRESENT\n");
    else if (ui_sbr_mode == 1)
        ALOGV("SBR Mode: PRESENT\n");
    else
        ALOGV("SBR Mode: ILLEGAL\n");

    /* mOutputFrameLength = 1024 * (1 + SBR_MODE) for AAC */
    /* For USAC it could be 1024 * 3 , support to query  */
    /* not yet added in codec                            */
    mOutputFrameLength = 1024 * (1 + ui_sbr_mode);
    ALOGI("mOutputFrameLength %d ui_sbr_mode %d", mOutputFrameLength, ui_sbr_mode);

    return IA_NO_ERROR;
}

IA_ERRORCODE C2SoftXaacDec::setXAACDRCInfo(int32_t drcCut, int32_t drcBoost,
                                           int32_t drcRefLevel,
                                           int32_t drcHeavyCompression,
                                           int32_t drEffectType) {
    IA_ERRORCODE err_code = IA_NO_ERROR;

    int32_t ui_drc_enable = 1;
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_ENABLE,
                                &ui_drc_enable);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_ENABLE");
    if (drcCut != -1) {
        err_code =
            ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                             IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_CUT, &drcCut);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_CUT");
    }

    if (drcBoost != -1) {
        err_code = ixheaacd_dec_api(
            mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
            IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_BOOST, &drcBoost);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_BOOST");
    }

    if (drcRefLevel != -1) {
        err_code = ixheaacd_dec_api(
            mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
            IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LEVEL, &drcRefLevel);
        RETURN_IF_FATAL(err_code,
                        "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LEVEL");
    }

    if (drcRefLevel != -1) {
        err_code = ixheaacd_dec_api(
            mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
            IA_ENHAACPLUS_DEC_DRC_TARGET_LOUDNESS, &drcRefLevel);
        RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_DRC_TARGET_LOUDNESS");
    }

    if (drcHeavyCompression != -1) {
        err_code =
            ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                             IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_HEAVY_COMP,
                             &drcHeavyCompression);
        RETURN_IF_FATAL(err_code,
                        "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_HEAVY_COMP");
    }

    err_code =
        ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_SET_CONFIG_PARAM,
                         IA_ENHAACPLUS_DEC_DRC_EFFECT_TYPE, &drEffectType);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_DRC_EFFECT_TYPE");

    int32_t i_effect_type, i_target_loudness, i_loud_norm;
    /*Set Effect Type*/
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_EFFECT_TYPE,
                                &i_effect_type);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_EFFECT_TYPE");

    err_code =
        ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                       IA_DRC_DEC_CONFIG_DRC_EFFECT_TYPE, &i_effect_type);

    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_EFFECT_TYPE");

    /*Set target loudness */
    err_code = ixheaacd_dec_api(
        mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
        IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LOUDNESS, &i_target_loudness);
    RETURN_IF_FATAL(err_code,
                    "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_TARGET_LOUDNESS");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_DRC_TARGET_LOUDNESS,
                              &i_target_loudness);
    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_TARGET_LOUDNESS");

    /*Set loud_norm_flag*/
    err_code = ixheaacd_dec_api(mXheaacCodecHandle, IA_API_CMD_GET_CONFIG_PARAM,
                                IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_LOUD_NORM,
                                &i_loud_norm);
    RETURN_IF_FATAL(err_code, "IA_ENHAACPLUS_DEC_CONFIG_PARAM_DRC_LOUD_NORM");

    err_code = ia_drc_dec_api(mMpegDDrcHandle, IA_API_CMD_SET_CONFIG_PARAM,
                              IA_DRC_DEC_CONFIG_DRC_LOUD_NORM, &i_loud_norm);

    RETURN_IF_FATAL(err_code, "IA_DRC_DEC_CONFIG_DRC_LOUD_NORM");

    return IA_NO_ERROR;
}

class C2SoftXaacDecFactory : public C2ComponentFactory {
public:
    C2SoftXaacDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftXaacDec(COMPONENT_NAME,
                               id,
                               std::make_shared<C2SoftXaacDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftXaacDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftXaacDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftXaacDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftXaacDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
