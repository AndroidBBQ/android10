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
#define LOG_TAG "C2SoftOpusDec"
#include <log/log.h>

#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/foundation/OpusHeader.h>
#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>
#include "C2SoftOpusDec.h"

extern "C" {
    #include <opus.h>
    #include <opus_multistream.h>
}

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.opus.decoder";

}  // namespace

class C2SoftOpusDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
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
                .withDefault(new C2StreamSampleRateInfo::output(0u, 48000))
                .withFields({C2F(mSampleRate, value).equalTo(48000)})
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
                .withDefault(new C2StreamBitrateInfo::input(0u, 6000))
                .withFields({C2F(mBitrate, value).inRange(6000, 510000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 960 * 6))
                .build());
    }

private:
    std::shared_ptr<C2StreamSampleRateInfo::output> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::output> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::input> mBitrate;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
};

C2SoftOpusDec::C2SoftOpusDec(const char *name, c2_node_id_t id,
                       const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
        std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mDecoder(nullptr) {
}

C2SoftOpusDec::~C2SoftOpusDec() {
    onRelease();
}

c2_status_t C2SoftOpusDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_NO_MEMORY;
}

c2_status_t C2SoftOpusDec::onStop() {
    if (mDecoder) {
        opus_multistream_decoder_destroy(mDecoder);
        mDecoder = nullptr;
    }
    memset(&mHeader, 0, sizeof(mHeader));
    mCodecDelay = 0;
    mSeekPreRoll = 0;
    mSamplesToDiscard = 0;
    mInputBufferCount = 0;
    mSignalledError = false;
    mSignalledOutputEos = false;

    return C2_OK;
}

void C2SoftOpusDec::onReset() {
    (void)onStop();
}

void C2SoftOpusDec::onRelease() {
    if (mDecoder) {
        opus_multistream_decoder_destroy(mDecoder);
        mDecoder = nullptr;
    }
}

status_t C2SoftOpusDec::initDecoder() {
    memset(&mHeader, 0, sizeof(mHeader));
    mCodecDelay = 0;
    mSeekPreRoll = 0;
    mSamplesToDiscard = 0;
    mInputBufferCount = 0;
    mSignalledError = false;
    mSignalledOutputEos = false;

    return OK;
}

c2_status_t C2SoftOpusDec::onFlush_sm() {
    if (mDecoder) {
        opus_multistream_decoder_ctl(mDecoder, OPUS_RESET_STATE);
        mSamplesToDiscard = mSeekPreRoll;
        mSignalledOutputEos = false;
    }
    return C2_OK;
}

c2_status_t C2SoftOpusDec::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    (void) pool;
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

static void fillEmptyWork(const std::unique_ptr<C2Work> &work) {
    work->worklets.front()->output.flags = work->input.flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = work->input.ordinal;
    work->workletsProcessed = 1u;
}

static const int kRate = 48000;

// Opus uses Vorbis channel mapping, and Vorbis channel mapping specifies
// mappings for up to 8 channels. This information is part of the Vorbis I
// Specification:
// http://www.xiph.org/vorbis/doc/Vorbis_I_spec.html
static const int kMaxChannels = 8;

// Maximum packet size used in Xiph's opusdec.
static const int kMaxOpusOutputPacketSizeSamples = 960 * 6;

// Default audio output channel layout. Used to initialize |stream_map| in
// OpusHeader, and passed to opus_multistream_decoder_create() when the header
// does not contain mapping information. The values are valid only for mono and
// stereo output: Opus streams with more than 2 channels require a stream map.
static const int kMaxChannelsWithDefaultLayout = 2;
static const uint8_t kDefaultOpusChannelLayout[kMaxChannelsWithDefaultLayout] = { 0, 1 };


// Convert nanoseconds to number of samples.
static uint64_t ns_to_samples(uint64_t ns, int rate) {
    return static_cast<double>(ns) * rate / 1000000000;
}

void C2SoftOpusDec::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.configUpdate.clear();
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledOutputEos) {
        work->result = C2_BAD_VALUE;
        return;
    }

    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    size_t inOffset = 0u;
    size_t inSize = 0u;
    C2ReadView rView = mDummyReadView;
    if (!work->input.buffers.empty()) {
        rView = work->input.buffers[0]->data().linearBlocks().front().map().get();
        inSize = rView.capacity();
        if (inSize && rView.error()) {
            ALOGE("read view map failed %d", rView.error());
            work->result = C2_CORRUPTED;
            return;
        }
    }
    if (inSize == 0) {
        fillEmptyWork(work);
        if (eos) {
            mSignalledOutputEos = true;
            ALOGV("signalled EOS");
        }
        return;
    }

    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d", inSize,
          (int)work->input.ordinal.timestamp.peeku(), (int)work->input.ordinal.frameIndex.peeku());
    const uint8_t *data = rView.data() + inOffset;
    if (mInputBufferCount < 3) {
        if (mInputBufferCount == 0) {
            size_t opusHeadSize = 0;
            size_t codecDelayBufSize = 0;
            size_t seekPreRollBufSize = 0;
            void *opusHeadBuf = NULL;
            void *codecDelayBuf = NULL;
            void *seekPreRollBuf = NULL;

            if (!GetOpusHeaderBuffers(data, inSize, &opusHeadBuf,
                                     &opusHeadSize, &codecDelayBuf,
                                     &codecDelayBufSize, &seekPreRollBuf,
                                     &seekPreRollBufSize)) {
                ALOGE("%s encountered error in GetOpusHeaderBuffers", __func__);
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }

            if (!ParseOpusHeader((uint8_t *)opusHeadBuf, opusHeadSize, &mHeader)) {
                ALOGE("%s Encountered error while Parsing Opus Header.", __func__);
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            uint8_t channel_mapping[kMaxChannels] = {0};
            if (mHeader.channels <= kMaxChannelsWithDefaultLayout) {
                memcpy(&channel_mapping,
                       kDefaultOpusChannelLayout,
                       kMaxChannelsWithDefaultLayout);
            } else {
                memcpy(&channel_mapping,
                       mHeader.stream_map,
                       mHeader.channels);
            }
            int status = OPUS_INVALID_STATE;
            mDecoder = opus_multistream_decoder_create(kRate,
                                                       mHeader.channels,
                                                       mHeader.num_streams,
                                                       mHeader.num_coupled,
                                                       channel_mapping,
                                                       &status);
            if (!mDecoder || status != OPUS_OK) {
                ALOGE("opus_multistream_decoder_create failed status = %s",
                      opus_strerror(status));
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            status = opus_multistream_decoder_ctl(mDecoder,
                                                  OPUS_SET_GAIN(mHeader.gain_db));
            if (status != OPUS_OK) {
                ALOGE("Failed to set OPUS header gain; status = %s",
                      opus_strerror(status));
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }

            if (codecDelayBuf && codecDelayBufSize == sizeof(uint64_t)) {
                uint64_t value;
                memcpy(&value, codecDelayBuf, sizeof(uint64_t));
                mCodecDelay = ns_to_samples(value, kRate);
                mSamplesToDiscard = mCodecDelay;
                ++mInputBufferCount;
            }
            if (seekPreRollBuf && seekPreRollBufSize == sizeof(uint64_t)) {
                uint64_t value;
                memcpy(&value, seekPreRollBuf, sizeof(uint64_t));
                mSeekPreRoll = ns_to_samples(value, kRate);
                ++mInputBufferCount;
            }
        } else {
            if (inSize < 8) {
                ALOGE("Input sample size is too small.");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            int64_t samples = ns_to_samples( *(reinterpret_cast<int64_t*>
                              (const_cast<uint8_t *> (data))), kRate);
            if (mInputBufferCount == 1) {
                mCodecDelay = samples;
                mSamplesToDiscard = mCodecDelay;
            }
            else {
                mSeekPreRoll = samples;
            }
        }

        ++mInputBufferCount;
        if (mInputBufferCount == 3) {
            ALOGI("Configuring decoder: %d Hz, %d channels",
                   kRate, mHeader.channels);
            C2StreamSampleRateInfo::output sampleRateInfo(0u, kRate);
            C2StreamChannelCountInfo::output channelCountInfo(0u, mHeader.channels);
            std::vector<std::unique_ptr<C2SettingResult>> failures;
            c2_status_t err = mIntf->config(
                    { &sampleRateInfo, &channelCountInfo },
                    C2_MAY_BLOCK,
                    &failures);
            if (err == OK) {
                work->worklets.front()->output.configUpdate.push_back(C2Param::Copy(sampleRateInfo));
                work->worklets.front()->output.configUpdate.push_back(C2Param::Copy(channelCountInfo));
            } else {
                ALOGE("Config Update failed");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
        }
        fillEmptyWork(work);
        if (eos) {
            mSignalledOutputEos = true;
            ALOGV("signalled EOS");
        }
        return;
    }

    // Ignore CSD re-submissions.
    if ((work->input.flags & C2FrameData::FLAG_CODEC_CONFIG)) {
        fillEmptyWork(work);
        return;
    }

    // When seeking to zero, |mCodecDelay| samples has to be discarded
    // instead of |mSeekPreRoll| samples (as we would when seeking to any
    // other timestamp).
    if (work->input.ordinal.timestamp.peeku() == 0) mSamplesToDiscard = mCodecDelay;

    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchLinearBlock(
                          kMaxNumSamplesPerBuffer * kMaxChannels * sizeof(int16_t),
                          usage, &block);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }
    C2WriteView wView = block->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        work->result = C2_CORRUPTED;
        return;
    }

    int numSamples = opus_multistream_decode(mDecoder,
                                             data,
                                             inSize,
                                             reinterpret_cast<int16_t *> (wView.data()),
                                             kMaxOpusOutputPacketSizeSamples,
                                             0);
    if (numSamples < 0) {
        ALOGE("opus_multistream_decode returned numSamples %d", numSamples);
        numSamples = 0;
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }

    int outOffset = 0;
    if (mSamplesToDiscard > 0) {
        if (mSamplesToDiscard > numSamples) {
            mSamplesToDiscard -= numSamples;
            numSamples = 0;
        } else {
            numSamples -= mSamplesToDiscard;
            outOffset = mSamplesToDiscard * sizeof(int16_t) * mHeader.channels;
            mSamplesToDiscard = 0;
        }
    }

    if (numSamples) {
        int outSize = numSamples * sizeof(int16_t) * mHeader.channels;
        ALOGV("out buffer attr. offset %d size %d ", outOffset, outSize);

        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(createLinearBuffer(block, outOffset, outSize));
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
    } else {
        fillEmptyWork(work);
        block.reset();
    }
    if (eos) {
        mSignalledOutputEos = true;
        ALOGV("signalled EOS");
    }
}

class C2SoftOpusDecFactory : public C2ComponentFactory {
public:
    C2SoftOpusDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftOpusDec(COMPONENT_NAME,
                               id,
                               std::make_shared<C2SoftOpusDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftOpusDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftOpusDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftOpusDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftOpusDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
