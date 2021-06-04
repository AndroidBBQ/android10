/*
 * Copyright (C) 2019 The Android Open Source Project
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
#define LOG_TAG "C2SoftOpusEnc"
#include <utils/Log.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/foundation/OpusHeader.h>
#include "C2SoftOpusEnc.h"

extern "C" {
    #include <opus.h>
    #include <opus_multistream.h>
}

#define DEFAULT_FRAME_DURATION_MS 20
namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.opus.encoder";

}  // namespace

static const int kMaxNumChannelsSupported = 2;

class C2SoftOpusEnc::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_ENCODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_OPUS) {
        noPrivateBuffers();
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();
        setDerivedInstance(this);

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(
                    C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSampleRate, C2_PARAMKEY_SAMPLE_RATE)
                .withDefault(new C2StreamSampleRateInfo::input(0u, 48000))
                .withFields({C2F(mSampleRate, value).oneOf({
                    8000, 12000, 16000, 24000, 48000})})
                .withSetter((Setter<decltype(*mSampleRate)>::StrictValueWithNoDeps))
                .build());

        addParameter(
                DefineParam(mChannelCount, C2_PARAMKEY_CHANNEL_COUNT)
                .withDefault(new C2StreamChannelCountInfo::input(0u, 1))
                .withFields({C2F(mChannelCount, value).inRange(1, kMaxNumChannelsSupported)})
                .withSetter((Setter<decltype(*mChannelCount)>::StrictValueWithNoDeps))
                .build());

        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 128000))
                .withFields({C2F(mBitrate, value).inRange(500, 512000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mComplexity, C2_PARAMKEY_COMPLEXITY)
                .withDefault(new C2StreamComplexityTuning::output(0u, 10))
                .withFields({C2F(mComplexity, value).inRange(1, 10)})
                .withSetter(Setter<decltype(*mComplexity)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 3840))
                .build());
    }

    uint32_t getSampleRate() const { return mSampleRate->value; }
    uint32_t getChannelCount() const { return mChannelCount->value; }
    uint32_t getBitrate() const { return mBitrate->value; }
    uint32_t getComplexity() const { return mComplexity->value; }

private:
    std::shared_ptr<C2StreamSampleRateInfo::input> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::input> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamComplexityTuning::output> mComplexity;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
};

C2SoftOpusEnc::C2SoftOpusEnc(const char* name, c2_node_id_t id,
                               const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
          std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mOutputBlock(nullptr),
      mEncoder(nullptr),
      mInputBufferPcm16(nullptr),
      mOutIndex(0u) {
}

C2SoftOpusEnc::~C2SoftOpusEnc() {
    onRelease();
}

c2_status_t C2SoftOpusEnc::onInit() {
    return initEncoder();
}

c2_status_t C2SoftOpusEnc::configureEncoder() {
    static const unsigned char mono_mapping[256] = {0};
    static const unsigned char stereo_mapping[256] = {0, 1};
    mSampleRate = mIntf->getSampleRate();
    mChannelCount = mIntf->getChannelCount();
    uint32_t bitrate = mIntf->getBitrate();
    int complexity = mIntf->getComplexity();
    mNumSamplesPerFrame = mSampleRate / (1000 / mFrameDurationMs);
    mNumPcmBytesPerInputFrame =
        mChannelCount * mNumSamplesPerFrame * sizeof(int16_t);
    int err = C2_OK;

    const unsigned char* mapping;
    if (mChannelCount == 1) {
        mapping = mono_mapping;
    } else if (mChannelCount == 2) {
        mapping = stereo_mapping;
    } else {
        ALOGE("Number of channels (%d) is not supported", mChannelCount);
        return C2_BAD_VALUE;
    }

    if (mEncoder != nullptr) {
        opus_multistream_encoder_destroy(mEncoder);
    }

    mEncoder = opus_multistream_encoder_create(mSampleRate, mChannelCount,
        1, mChannelCount - 1, mapping, OPUS_APPLICATION_AUDIO, &err);
    if (err) {
        ALOGE("Could not create libopus encoder. Error code: %i", err);
        return C2_CORRUPTED;
    }

    // Complexity
    if (opus_multistream_encoder_ctl(
            mEncoder, OPUS_SET_COMPLEXITY(complexity)) != OPUS_OK) {
        ALOGE("failed to set complexity");
        return C2_BAD_VALUE;
    }

    // DTX
    if (opus_multistream_encoder_ctl(mEncoder, OPUS_SET_DTX(0) != OPUS_OK)) {
        ALOGE("failed to set dtx");
        return C2_BAD_VALUE;
    }

    // Application
    if (opus_multistream_encoder_ctl(mEncoder,
            OPUS_SET_APPLICATION(OPUS_APPLICATION_AUDIO)) != OPUS_OK) {
        ALOGE("failed to set application");
        return C2_BAD_VALUE;
    }

    // Signal type
    if (opus_multistream_encoder_ctl(mEncoder, OPUS_SET_SIGNAL(OPUS_AUTO)) !=
        OPUS_OK) {
        ALOGE("failed to set signal");
        return C2_BAD_VALUE;
    }

    // Constrained VBR
    if (opus_multistream_encoder_ctl(mEncoder, OPUS_SET_VBR(1) != OPUS_OK)) {
        ALOGE("failed to set vbr type");
        return C2_BAD_VALUE;
    }
    if (opus_multistream_encoder_ctl(mEncoder, OPUS_SET_VBR_CONSTRAINT(1) !=
            OPUS_OK)) {
        ALOGE("failed to set vbr constraint");
        return C2_BAD_VALUE;
    }

    // Bitrate
    if (opus_multistream_encoder_ctl(mEncoder, OPUS_SET_BITRATE(bitrate)) !=
            OPUS_OK) {
        ALOGE("failed to set bitrate");
        return C2_BAD_VALUE;
    }

    // Set seek preroll to 80 ms
    mSeekPreRoll = 80000000;
    return C2_OK;
}

c2_status_t C2SoftOpusEnc::initEncoder() {
    mSignalledEos = false;
    mSignalledError = false;
    mHeaderGenerated = false;
    mIsFirstFrame = true;
    mEncoderFlushed = false;
    mBufferAvailable = false;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0;
    mFilledLen = 0;
    mFrameDurationMs = DEFAULT_FRAME_DURATION_MS;
    if (!mInputBufferPcm16) {
        mInputBufferPcm16 =
            (int16_t*)malloc(kFrameSize * kMaxNumChannels * sizeof(int16_t));
    }
    if (!mInputBufferPcm16) return C2_NO_MEMORY;

    /* Default Configurations */
    c2_status_t status = configureEncoder();
    return status;
}

c2_status_t C2SoftOpusEnc::onStop() {
    mSignalledEos = false;
    mSignalledError = false;
    mIsFirstFrame = true;
    mEncoderFlushed = false;
    mBufferAvailable = false;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0u;
    mFilledLen = 0;
    if (mEncoder) {
        int status = opus_multistream_encoder_ctl(mEncoder, OPUS_RESET_STATE);
        if (status != OPUS_OK) {
            ALOGE("OPUS_RESET_STATE failed status = %s", opus_strerror(status));
            mSignalledError = true;
            return C2_CORRUPTED;
        }
    }
    if (mOutputBlock) mOutputBlock.reset();
    mOutputBlock = nullptr;

    return C2_OK;
}

void C2SoftOpusEnc::onReset() {
    (void)onStop();
}

void C2SoftOpusEnc::onRelease() {
    (void)onStop();
    if (mInputBufferPcm16) {
        free(mInputBufferPcm16);
        mInputBufferPcm16 = nullptr;
    }
    if (mEncoder) {
        opus_multistream_encoder_destroy(mEncoder);
        mEncoder = nullptr;
    }
}

c2_status_t C2SoftOpusEnc::onFlush_sm() {
    return onStop();
}

// Drain the encoder to get last frames (if any)
int C2SoftOpusEnc::drainEncoder(uint8_t* outPtr) {
    memset((uint8_t *)mInputBufferPcm16 + mFilledLen, 0,
        (mNumPcmBytesPerInputFrame - mFilledLen));
    int encodedBytes = opus_multistream_encode(
        mEncoder, mInputBufferPcm16, mNumSamplesPerFrame, outPtr, kMaxPayload);
    if (encodedBytes > mOutputBlock->capacity()) {
        ALOGE("not enough space left to write encoded data, dropping %d bytes",
              mBytesEncoded);
        // a fatal error would stop the encoding
        return -1;
    }
    ALOGV("encoded %i Opus bytes from %zu PCM bytes", encodedBytes,
          mNumPcmBytesPerInputFrame);
    mEncoderFlushed = true;
    mFilledLen = 0;
    return encodedBytes;
}

void C2SoftOpusEnc::process(const std::unique_ptr<C2Work>& work,
                            const std::shared_ptr<C2BlockPool>& pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledEos) {
        work->result = C2_BAD_VALUE;
        return;
    }

    bool eos = (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0;
    C2ReadView rView = mDummyReadView;
    size_t inOffset = 0u;
    size_t inSize = 0u;
    c2_status_t err = C2_OK;
    if (!work->input.buffers.empty()) {
        rView =
            work->input.buffers[0]->data().linearBlocks().front().map().get();
        inSize = rView.capacity();
        if (inSize && rView.error()) {
            ALOGE("read view map failed %d", rView.error());
            work->result = C2_CORRUPTED;
            return;
        }
    }

    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d, flags %x",
          inSize, (int)work->input.ordinal.timestamp.peeku(),
          (int)work->input.ordinal.frameIndex.peeku(), work->input.flags);

    if (!mEncoder) {
        if (initEncoder() != C2_OK) {
            ALOGE("initEncoder failed with status %d", err);
            work->result = err;
            mSignalledError = true;
            return;
        }
    }
    if (mIsFirstFrame && inSize > 0) {
        mAnchorTimeStamp = work->input.ordinal.timestamp.peekull();
        mIsFirstFrame = false;
    }

    C2MemoryUsage usage = {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE};
    err = pool->fetchLinearBlock(kMaxPayload, usage, &mOutputBlock);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }

    C2WriteView wView = mOutputBlock->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        work->result = C2_CORRUPTED;
        mOutputBlock.reset();
        return;
    }

    size_t inPos = 0;
    size_t processSize = 0;
    mBytesEncoded = 0;
    uint64_t outTimeStamp = 0u;
    std::shared_ptr<C2Buffer> buffer;
    uint64_t inputIndex = work->input.ordinal.frameIndex.peeku();
    const uint8_t* inPtr = rView.data() + inOffset;

    class FillWork {
    public:
        FillWork(uint32_t flags, C2WorkOrdinalStruct ordinal,
                 const std::shared_ptr<C2Buffer> &buffer)
            : mFlags(flags), mOrdinal(ordinal), mBuffer(buffer) {
        }
        ~FillWork() = default;

        void operator()(const std::unique_ptr<C2Work>& work) {
            work->worklets.front()->output.flags = (C2FrameData::flags_t)mFlags;
            work->worklets.front()->output.buffers.clear();
            work->worklets.front()->output.ordinal = mOrdinal;
            work->workletsProcessed = 1u;
            work->result = C2_OK;
            if (mBuffer) {
                work->worklets.front()->output.buffers.push_back(mBuffer);
            }
            ALOGV("timestamp = %lld, index = %lld, w/%s buffer",
                  mOrdinal.timestamp.peekll(),
                  mOrdinal.frameIndex.peekll(),
                  mBuffer ? "" : "o");
        }

    private:
        const uint32_t mFlags;
        const C2WorkOrdinalStruct mOrdinal;
        const std::shared_ptr<C2Buffer> mBuffer;
    };

    C2WorkOrdinalStruct outOrdinal = work->input.ordinal;

    if (!mHeaderGenerated) {
        uint8_t header[AOPUS_UNIFIED_CSD_MAXSIZE];
        memset(header, 0, sizeof(header));

        // Get codecDelay
        int32_t lookahead;
        if (opus_multistream_encoder_ctl(mEncoder, OPUS_GET_LOOKAHEAD(&lookahead)) !=
                OPUS_OK) {
            ALOGE("failed to get lookahead");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        mCodecDelay = lookahead * 1000000000ll / mSampleRate;

        OpusHeader opusHeader;
        memset(&opusHeader, 0, sizeof(opusHeader));
        opusHeader.channels = mChannelCount;
        opusHeader.num_streams = mChannelCount;
        opusHeader.num_coupled = 0;
        opusHeader.channel_mapping = ((mChannelCount > 8) ? 255 : (mChannelCount > 2));
        opusHeader.gain_db = 0;
        opusHeader.skip_samples = lookahead;
        int headerLen = WriteOpusHeaders(opusHeader, mSampleRate, header,
            sizeof(header), mCodecDelay, mSeekPreRoll);

        std::unique_ptr<C2StreamInitDataInfo::output> csd =
            C2StreamInitDataInfo::output::AllocUnique(headerLen, 0u);
        if (!csd) {
            ALOGE("CSD allocation failed");
            mSignalledError = true;
            work->result = C2_NO_MEMORY;
            return;
        }
        ALOGV("put csd, %d bytes", headerLen);
        memcpy(csd->m.value, header, headerLen);
        work->worklets.front()->output.configUpdate.push_back(std::move(csd));
        mHeaderGenerated = true;
    }

    /*
     * For buffer size which is not a multiple of mNumPcmBytesPerInputFrame, we will
     * accumulate the input and keep it. Once the input is filled with expected number
     * of bytes, we will send it to encoder. mFilledLen manages the bytes of input yet
     * to be processed. The next call will fill mNumPcmBytesPerInputFrame - mFilledLen
     * bytes to input and send it to the encoder.
     */
    while (inPos < inSize) {
        const uint8_t* pcmBytes = inPtr + inPos;
        int filledSamples = mFilledLen / sizeof(int16_t);
        if ((inPos + (mNumPcmBytesPerInputFrame - mFilledLen)) <= inSize) {
            processSize = mNumPcmBytesPerInputFrame - mFilledLen;
            mBufferAvailable = true;
        } else {
            processSize = inSize - inPos;
            mBufferAvailable = false;
            if (eos) {
                memset(mInputBufferPcm16 + filledSamples, 0,
                       (mNumPcmBytesPerInputFrame - mFilledLen));
                mBufferAvailable = true;
            }
        }
        const unsigned nInputSamples = processSize / sizeof(int16_t);

        for (unsigned i = 0; i < nInputSamples; i++) {
            int32_t data = pcmBytes[2 * i + 1] << 8 | pcmBytes[2 * i];
            data = ((data & 0xFFFF) ^ 0x8000) - 0x8000;
            mInputBufferPcm16[i + filledSamples] = data;
        }
        inPos += processSize;
        mFilledLen += processSize;
        if (!mBufferAvailable) break;
        uint8_t* outPtr = wView.data() + mBytesEncoded;
        int encodedBytes =
            opus_multistream_encode(mEncoder, mInputBufferPcm16,
                                    mNumSamplesPerFrame, outPtr, kMaxPayload - mBytesEncoded);
        ALOGV("encoded %i Opus bytes from %zu PCM bytes", encodedBytes,
              processSize);

        if (encodedBytes < 0 || encodedBytes > (kMaxPayload - mBytesEncoded)) {
            ALOGE("opus_encode failed, encodedBytes : %d", encodedBytes);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        if (buffer) {
            outOrdinal.frameIndex = mOutIndex++;
            outOrdinal.timestamp = mAnchorTimeStamp + outTimeStamp;
            cloneAndSend(
                inputIndex, work,
                FillWork(C2FrameData::FLAG_INCOMPLETE, outOrdinal, buffer));
            buffer.reset();
        }
        if (encodedBytes > 0) {
            buffer =
                createLinearBuffer(mOutputBlock, mBytesEncoded, encodedBytes);
        }
        mBytesEncoded += encodedBytes;
        mProcessedSamples += (filledSamples + nInputSamples);
        outTimeStamp =
            mProcessedSamples * 1000000ll / mChannelCount / mSampleRate;
        if ((processSize + mFilledLen) < mNumPcmBytesPerInputFrame)
            mEncoderFlushed = true;
        mFilledLen = 0;
    }

    uint32_t flags = 0;
    if (eos) {
        ALOGV("signalled eos");
        mSignalledEos = true;
        if (!mEncoderFlushed) {
            if (buffer) {
                outOrdinal.frameIndex = mOutIndex++;
                outOrdinal.timestamp = mAnchorTimeStamp + outTimeStamp;
                cloneAndSend(
                    inputIndex, work,
                    FillWork(C2FrameData::FLAG_INCOMPLETE, outOrdinal, buffer));
                buffer.reset();
            }
            // drain the encoder for last buffer
            drainInternal(pool, work);
        }
        flags = C2FrameData::FLAG_END_OF_STREAM;
    }
    if (buffer) {
        outOrdinal.frameIndex = mOutIndex++;
        outOrdinal.timestamp = mAnchorTimeStamp + outTimeStamp;
        FillWork((C2FrameData::flags_t)(flags), outOrdinal, buffer)(work);
        buffer.reset();
    }
    mOutputBlock = nullptr;
}

c2_status_t C2SoftOpusEnc::drainInternal(
        const std::shared_ptr<C2BlockPool>& pool,
        const std::unique_ptr<C2Work>& work) {
    mBytesEncoded = 0;
    std::shared_ptr<C2Buffer> buffer = nullptr;
    C2WorkOrdinalStruct outOrdinal = work->input.ordinal;
    bool eos = (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0;

    C2MemoryUsage usage = {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE};
    c2_status_t err = pool->fetchLinearBlock(kMaxPayload, usage, &mOutputBlock);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        return C2_NO_MEMORY;
    }

    C2WriteView wView = mOutputBlock->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        mOutputBlock.reset();
        return C2_CORRUPTED;
    }

    int encBytes = drainEncoder(wView.data());
    if (encBytes > 0) mBytesEncoded += encBytes;
    if (mBytesEncoded > 0) {
        buffer = createLinearBuffer(mOutputBlock, 0, mBytesEncoded);
        mOutputBlock.reset();
    }
    mProcessedSamples += (mNumPcmBytesPerInputFrame / sizeof(int16_t));
    uint64_t outTimeStamp =
        mProcessedSamples * 1000000ll / mChannelCount / mSampleRate;
    outOrdinal.frameIndex = mOutIndex++;
    outOrdinal.timestamp = mAnchorTimeStamp + outTimeStamp;
    work->worklets.front()->output.flags =
        (C2FrameData::flags_t)(eos ? C2FrameData::FLAG_END_OF_STREAM : 0);
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = outOrdinal;
    work->workletsProcessed = 1u;
    work->result = C2_OK;
    if (buffer) {
        work->worklets.front()->output.buffers.push_back(buffer);
    }
    mOutputBlock = nullptr;
    return C2_OK;
}

c2_status_t C2SoftOpusEnc::drain(uint32_t drainMode,
                                 const std::shared_ptr<C2BlockPool>& pool) {
    if (drainMode == NO_DRAIN) {
        ALOGW("drain with NO_DRAIN: no-op");
        return C2_OK;
    }
    if (drainMode == DRAIN_CHAIN) {
        ALOGW("DRAIN_CHAIN not supported");
        return C2_OMITTED;
    }
    mIsFirstFrame = true;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0u;
    return drainInternal(pool, nullptr);
}

class C2SoftOpusEncFactory : public C2ComponentFactory {
public:
    C2SoftOpusEncFactory()
        : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
              GetCodec2PlatformComponentStore()->getParamReflector())) {}

    virtual c2_status_t createComponent(
        c2_node_id_t id, std::shared_ptr<C2Component>* const component,
        std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftOpusEnc(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftOpusEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
        c2_node_id_t id, std::shared_ptr<C2ComponentInterface>* const interface,
        std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftOpusEnc::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftOpusEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual ~C2SoftOpusEncFactory() override = default;
private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftOpusEncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
