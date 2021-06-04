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
#define LOG_TAG "C2SoftFlacEnc"
#include <log/log.h>

#include <audio_utils/primitives.h>
#include <media/stagefright/foundation/MediaDefs.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftFlacEnc.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.flac.encoder";

}  // namespace

class C2SoftFlacEnc::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_ENCODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_FLAC) {
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
                .withDefault(new C2StreamSampleRateInfo::input(0u, 44100))
                .withFields({C2F(mSampleRate, value).inRange(1, 655350)})
                .withSetter((Setter<decltype(*mSampleRate)>::StrictValueWithNoDeps))
                .build());
        addParameter(
                DefineParam(mChannelCount, C2_PARAMKEY_CHANNEL_COUNT)
                .withDefault(new C2StreamChannelCountInfo::input(0u, 1))
                .withFields({C2F(mChannelCount, value).inRange(1, 2)})
                .withSetter(Setter<decltype(*mChannelCount)>::StrictValueWithNoDeps)
                .build());
        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 768000))
                .withFields({C2F(mBitrate, value).inRange(1, 21000000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());
        addParameter(
                DefineParam(mComplexity, C2_PARAMKEY_COMPLEXITY)
                .withDefault(new C2StreamComplexityTuning::output(0u,
                    FLAC_COMPRESSION_LEVEL_DEFAULT))
                .withFields({C2F(mComplexity, value).inRange(
                    FLAC_COMPRESSION_LEVEL_MIN, FLAC_COMPRESSION_LEVEL_MAX)})
                .withSetter(Setter<decltype(*mComplexity)>::NonStrictValueWithNoDeps)
                .build());
        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 4608))
                .build());

        addParameter(
                DefineParam(mPcmEncodingInfo, C2_PARAMKEY_PCM_ENCODING)
                .withDefault(new C2StreamPcmEncodingInfo::input(0u, C2Config::PCM_16))
                .withFields({C2F(mPcmEncodingInfo, value).oneOf({
                     C2Config::PCM_16,
                     // C2Config::PCM_8,
                     C2Config::PCM_FLOAT})
                })
                .withSetter((Setter<decltype(*mPcmEncodingInfo)>::StrictValueWithNoDeps))
                .build());
    }

    uint32_t getSampleRate() const { return mSampleRate->value; }
    uint32_t getChannelCount() const { return mChannelCount->value; }
    uint32_t getBitrate() const { return mBitrate->value; }
    uint32_t getComplexity() const { return mComplexity->value; }
    int32_t getPcmEncodingInfo() const { return mPcmEncodingInfo->value; }

private:
    std::shared_ptr<C2StreamSampleRateInfo::input> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::input> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamComplexityTuning::output> mComplexity;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
    std::shared_ptr<C2StreamPcmEncodingInfo::input> mPcmEncodingInfo;
};

C2SoftFlacEnc::C2SoftFlacEnc(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mFlacStreamEncoder(nullptr),
      mInputBufferPcm32(nullptr) {
}

C2SoftFlacEnc::~C2SoftFlacEnc() {
    onRelease();
}

c2_status_t C2SoftFlacEnc::onInit() {
    mFlacStreamEncoder = FLAC__stream_encoder_new();
    if (!mFlacStreamEncoder) return C2_CORRUPTED;

    mInputBufferPcm32 = (FLAC__int32*) malloc(
            kInBlockSize * kMaxNumChannels * sizeof(FLAC__int32));
    if (!mInputBufferPcm32) return C2_NO_MEMORY;

    mSignalledError = false;
    mSignalledOutputEos = false;
    mIsFirstFrame = true;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0u;
    mEncoderWriteData = false;
    mEncoderReturnedNbBytes = 0;
    mHeaderOffset = 0;
    mWroteHeader = false;

    status_t err = configureEncoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

void C2SoftFlacEnc::onRelease() {
    if (mFlacStreamEncoder) {
        FLAC__stream_encoder_delete(mFlacStreamEncoder);
        mFlacStreamEncoder = nullptr;
    }

    if (mInputBufferPcm32) {
        free(mInputBufferPcm32);
        mInputBufferPcm32 = nullptr;
    }
}

void C2SoftFlacEnc::onReset() {
    (void) onStop();
}

c2_status_t C2SoftFlacEnc::onStop() {
    mSignalledError = false;
    mSignalledOutputEos = false;
    mIsFirstFrame = true;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0u;
    mEncoderWriteData = false;
    mEncoderReturnedNbBytes = 0;
    mHeaderOffset = 0;
    mWroteHeader = false;

    c2_status_t status = drain(DRAIN_COMPONENT_NO_EOS, nullptr);
    if (C2_OK != status) return status;

    status_t err = configureEncoder();
    if (err != OK) mSignalledError = true;
    return C2_OK;
}

c2_status_t C2SoftFlacEnc::onFlush_sm() {
    return onStop();
}

static void fillEmptyWork(const std::unique_ptr<C2Work> &work) {
    work->worklets.front()->output.flags = work->input.flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = work->input.ordinal;
}

void C2SoftFlacEnc::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledOutputEos) {
        work->result = C2_BAD_VALUE;
        return;
    }

    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    C2ReadView rView = mDummyReadView;
    size_t inOffset = 0u;
    size_t inSize = 0u;
    if (!work->input.buffers.empty()) {
        rView = work->input.buffers[0]->data().linearBlocks().front().map().get();
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
    if (mIsFirstFrame && inSize) {
        mAnchorTimeStamp = work->input.ordinal.timestamp.peekull();
        mIsFirstFrame = false;
    }

    if (!mWroteHeader) {
        std::unique_ptr<C2StreamInitDataInfo::output> csd =
            C2StreamInitDataInfo::output::AllocUnique(mHeaderOffset, 0u);
        if (!csd) {
            ALOGE("CSD allocation failed");
            mSignalledError = true;
            work->result = C2_NO_MEMORY;
            return;
        }
        memcpy(csd->m.value, mHeader, mHeaderOffset);
        ALOGV("put csd, %d bytes", mHeaderOffset);

        work->worklets.front()->output.configUpdate.push_back(std::move(csd));
        mWroteHeader = true;
    }

    const uint32_t sampleRate = mIntf->getSampleRate();
    const uint32_t channelCount = mIntf->getChannelCount();
    const bool inputFloat = mIntf->getPcmEncodingInfo() == C2Config::PCM_FLOAT;
    const unsigned sampleSize = inputFloat ? sizeof(float) : sizeof(int16_t);
    const unsigned frameSize = channelCount * sampleSize;
    const uint64_t outTimeStamp = mProcessedSamples * 1000000ll / sampleRate;

    size_t outCapacity = inSize;
    outCapacity += mBlockSize * frameSize;

    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchLinearBlock(outCapacity, usage, &mOutputBlock);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }
    C2WriteView wView = mOutputBlock->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        work->result = C2_CORRUPTED;
        return;
    }

    mEncoderWriteData = true;
    mEncoderReturnedNbBytes = 0;
    size_t inPos = 0;
    while (inPos < inSize) {
        const uint8_t *inPtr = rView.data() + inOffset;
        const size_t processSize = MIN(kInBlockSize * frameSize, (inSize - inPos));
        const unsigned nbInputFrames = processSize / frameSize;
        const unsigned nbInputSamples = processSize / sampleSize;

        ALOGV("about to encode %zu bytes", processSize);
        if (inputFloat) {
            const float * const pcmFloat = reinterpret_cast<const float *>(inPtr + inPos);
            memcpy_to_q8_23_from_float_with_clamp(mInputBufferPcm32, pcmFloat, nbInputSamples);
        } else {
            const int16_t * const pcm16 = reinterpret_cast<const int16_t *>(inPtr + inPos);
            for (unsigned i = 0; i < nbInputSamples; i++) {
                mInputBufferPcm32[i] = (FLAC__int32) pcm16[i];
            }
        }

        FLAC__bool ok = FLAC__stream_encoder_process_interleaved(
                mFlacStreamEncoder, mInputBufferPcm32, nbInputFrames);
        if (!ok) {
            ALOGE("error encountered during encoding");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            mOutputBlock.reset();
            return;
        }
        inPos += processSize;
    }
    if (eos && (C2_OK != drain(DRAIN_COMPONENT_WITH_EOS, pool))) {
        ALOGE("error encountered during encoding");
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        mOutputBlock.reset();
        return;
    }
    fillEmptyWork(work);
    if (mEncoderReturnedNbBytes != 0) {
        std::shared_ptr<C2Buffer> buffer = createLinearBuffer(std::move(mOutputBlock), 0, mEncoderReturnedNbBytes);
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal.timestamp = mAnchorTimeStamp + outTimeStamp;
    } else {
        ALOGV("encoder process_interleaved returned without data to write");
    }
    mOutputBlock = nullptr;
    if (eos) {
        mSignalledOutputEos = true;
        ALOGV("signalled EOS");
    }
    mEncoderWriteData = false;
    mEncoderReturnedNbBytes = 0;
}

FLAC__StreamEncoderWriteStatus C2SoftFlacEnc::onEncodedFlacAvailable(
        const FLAC__byte buffer[], size_t bytes, unsigned samples,
        unsigned current_frame) {
    (void) current_frame;
    ALOGV("%s (bytes=%zu, samples=%u, curr_frame=%u)", __func__, bytes, samples,
          current_frame);

    if (samples == 0) {
        ALOGI("saving %zu bytes of header", bytes);
        memcpy(mHeader + mHeaderOffset, buffer, bytes);
        mHeaderOffset += bytes;// will contain header size when finished receiving header
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    if ((samples == 0) || !mEncoderWriteData) {
        // called by the encoder because there's header data to save, but it's not the role
        // of this component (unless WRITE_FLAC_HEADER_IN_FIRST_BUFFER is defined)
        ALOGV("ignoring %zu bytes of header data (samples=%d)", bytes, samples);
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    // write encoded data
    C2WriteView wView = mOutputBlock->map().get();
    uint8_t* outData = wView.data();
    ALOGV("writing %zu bytes of encoded data on output", bytes);
    // increment mProcessedSamples to maintain audio synchronization during
    // play back
    mProcessedSamples += samples;
    if (bytes + mEncoderReturnedNbBytes > mOutputBlock->capacity()) {
        ALOGE("not enough space left to write encoded data, dropping %zu bytes", bytes);
        // a fatal error would stop the encoding
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }
    memcpy(outData + mEncoderReturnedNbBytes, buffer, bytes);
    mEncoderReturnedNbBytes += bytes;
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}


status_t C2SoftFlacEnc::configureEncoder() {
    ALOGV("%s numChannel=%d, sampleRate=%d", __func__, mIntf->getChannelCount(), mIntf->getSampleRate());

    if (mSignalledError || !mFlacStreamEncoder) {
        ALOGE("can't configure encoder: no encoder or invalid state");
        return UNKNOWN_ERROR;
    }

    const bool inputFloat = mIntf->getPcmEncodingInfo() == C2Config::PCM_FLOAT;
    const int bitsPerSample = inputFloat ? 24 : 16;
    FLAC__bool ok = true;
    ok = ok && FLAC__stream_encoder_set_channels(mFlacStreamEncoder, mIntf->getChannelCount());
    ok = ok && FLAC__stream_encoder_set_sample_rate(mFlacStreamEncoder, mIntf->getSampleRate());
    ok = ok && FLAC__stream_encoder_set_bits_per_sample(mFlacStreamEncoder, bitsPerSample);
    ok = ok && FLAC__stream_encoder_set_compression_level(mFlacStreamEncoder,
                    mIntf->getComplexity());
    ok = ok && FLAC__stream_encoder_set_verify(mFlacStreamEncoder, false);
    if (!ok) {
        ALOGE("unknown error when configuring encoder");
        return UNKNOWN_ERROR;
    }

    ok &= FLAC__STREAM_ENCODER_INIT_STATUS_OK ==
            FLAC__stream_encoder_init_stream(mFlacStreamEncoder,
                    flacEncoderWriteCallback    /*write_callback*/,
                    nullptr /*seek_callback*/,
                    nullptr /*tell_callback*/,
                    nullptr /*metadata_callback*/,
                    (void *) this /*client_data*/);

    if (!ok) {
        ALOGE("unknown error when configuring encoder");
        return UNKNOWN_ERROR;
    }

    mBlockSize = FLAC__stream_encoder_get_blocksize(mFlacStreamEncoder);

    ALOGV("encoder successfully configured");
    return OK;
}

FLAC__StreamEncoderWriteStatus C2SoftFlacEnc::flacEncoderWriteCallback(
            const FLAC__StreamEncoder *,
            const FLAC__byte buffer[],
            size_t bytes,
            unsigned samples,
            unsigned current_frame,
            void *client_data) {
    return ((C2SoftFlacEnc*) client_data)->onEncodedFlacAvailable(
            buffer, bytes, samples, current_frame);
}

c2_status_t C2SoftFlacEnc::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    (void) pool;
    switch (drainMode) {
        case NO_DRAIN:
            ALOGW("drain with NO_DRAIN: no-op");
            return C2_OK;
        case DRAIN_CHAIN:
            ALOGW("DRAIN_CHAIN not supported");
            return C2_OMITTED;
        case DRAIN_COMPONENT_WITH_EOS:
            // TODO: This flag is not being sent back to the client
            // because there are no items in PendingWork queue as all the
            // inputs are being sent back with emptywork or valid encoded data
            // mSignalledOutputEos = true;
        case DRAIN_COMPONENT_NO_EOS:
            break;
        default:
            return C2_BAD_VALUE;
    }
    FLAC__bool ok = FLAC__stream_encoder_finish(mFlacStreamEncoder);
    if (!ok) return C2_CORRUPTED;
    mIsFirstFrame = true;
    mAnchorTimeStamp = 0ull;
    mProcessedSamples = 0u;

    return C2_OK;
}

class C2SoftFlacEncFactory : public C2ComponentFactory {
public:
    C2SoftFlacEncFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftFlacEnc(COMPONENT_NAME,
                                  id,
                                  std::make_shared<C2SoftFlacEnc::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftFlacEnc::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftFlacEnc::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftFlacEncFactory() override = default;
private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftFlacEncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
