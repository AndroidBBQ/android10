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
#define LOG_TAG "C2SoftMp3Dec"
#include <log/log.h>

#include <numeric>

#include <media/stagefright/foundation/MediaDefs.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftMp3Dec.h"
#include "pvmp3decoder_api.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.mp3.decoder";

}  // namespace

class C2SoftMP3::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_MPEG) {
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
                .withDefault(new C2StreamSampleRateInfo::output(0u, 44100))
                .withFields({C2F(mSampleRate, value).oneOf({8000, 11025, 12000, 16000,
                    22050, 24000, 32000, 44100, 48000})})
                .withSetter((Setter<decltype(*mSampleRate)>::StrictValueWithNoDeps))
                .build());

        addParameter(
                DefineParam(mChannelCount, C2_PARAMKEY_CHANNEL_COUNT)
                .withDefault(new C2StreamChannelCountInfo::output(0u, 2))
                .withFields({C2F(mChannelCount, value).inRange(1, 2)})
                .withSetter(Setter<decltype(*mChannelCount)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::input(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(8000, 320000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 8192))
                .build());
    }

private:
    std::shared_ptr<C2StreamSampleRateInfo::output> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::output> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::input> mBitrate;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
};

C2SoftMP3::C2SoftMP3(const char *name, c2_node_id_t id,
                     const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mConfig(nullptr),
      mDecoderBuf(nullptr) {
}

C2SoftMP3::~C2SoftMP3() {
    onRelease();
}

c2_status_t C2SoftMP3::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_NO_MEMORY;
}

c2_status_t C2SoftMP3::onStop() {
    // Make sure that the next buffer output does not still
    // depend on fragments from the last one decoded.
    pvmp3_InitDecoder(mConfig, mDecoderBuf);
    mSignalledError = false;
    mIsFirst = true;
    mSignalledOutputEos = false;
    mAnchorTimeStamp = 0;
    mProcessedSamples = 0;

    return C2_OK;
}

void C2SoftMP3::onReset() {
    (void)onStop();
}

void C2SoftMP3::onRelease() {
    mGaplessBytes = false;
    if (mDecoderBuf) {
        free(mDecoderBuf);
        mDecoderBuf = nullptr;
    }

    if (mConfig) {
        delete mConfig;
        mConfig = nullptr;
    }
}

status_t C2SoftMP3::initDecoder() {
    mConfig = new tPVMP3DecoderExternal{};
    if (!mConfig) return NO_MEMORY;
    mConfig->equalizerType = flat;
    mConfig->crcEnabled = false;

    size_t memRequirements = pvmp3_decoderMemRequirements();
    mDecoderBuf = malloc(memRequirements);
    if (!mDecoderBuf) return NO_MEMORY;

    pvmp3_InitDecoder(mConfig, mDecoderBuf);

    mIsFirst = true;
    mGaplessBytes = false;
    mSignalledError = false;
    mSignalledOutputEos = false;
    mAnchorTimeStamp = 0;
    mProcessedSamples = 0;

    return OK;
}

/* The below code is borrowed from ./test/mp3reader.cpp */
static bool parseMp3Header(uint32_t header, size_t *frame_size,
                           uint32_t *out_sampling_rate = nullptr,
                           uint32_t *out_channels = nullptr,
                           uint32_t *out_bitrate = nullptr,
                           uint32_t *out_num_samples = nullptr) {
    *frame_size = 0;
    if (out_sampling_rate) *out_sampling_rate = 0;
    if (out_channels) *out_channels = 0;
    if (out_bitrate) *out_bitrate = 0;
    if (out_num_samples) *out_num_samples = 1152;

    if ((header & 0xffe00000) != 0xffe00000) return false;

    unsigned version = (header >> 19) & 3;
    if (version == 0x01) return false;

    unsigned layer = (header >> 17) & 3;
    if (layer == 0x00) return false;

    unsigned bitrate_index = (header >> 12) & 0x0f;
    if (bitrate_index == 0 || bitrate_index == 0x0f) return false;

    unsigned sampling_rate_index = (header >> 10) & 3;
    if (sampling_rate_index == 3) return false;

    static const int kSamplingRateV1[] = { 44100, 48000, 32000 };
    int sampling_rate = kSamplingRateV1[sampling_rate_index];
    if (version == 2 /* V2 */) {
        sampling_rate /= 2;
    } else if (version == 0 /* V2.5 */) {
        sampling_rate /= 4;
    }

    unsigned padding = (header >> 9) & 1;

    if (layer == 3) { // layer I
        static const int kBitrateV1[] =
        {
            32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448
        };
        static const int kBitrateV2[] =
        {
            32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256
        };

        int bitrate = (version == 3 /* V1 */) ? kBitrateV1[bitrate_index - 1] :
                kBitrateV2[bitrate_index - 1];

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }
        *frame_size = (12000 * bitrate / sampling_rate + padding) * 4;
        if (out_num_samples) {
            *out_num_samples = 384;
        }
    } else { // layer II or III
        static const int kBitrateV1L2[] =
        {
            32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384
        };

        static const int kBitrateV1L3[] =
        {
            32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
        };

        static const int kBitrateV2[] =
        {
            8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160
        };

        int bitrate;
        if (version == 3 /* V1 */) {
            bitrate = (layer == 2 /* L2 */) ? kBitrateV1L2[bitrate_index - 1] :
                    kBitrateV1L3[bitrate_index - 1];

            if (out_num_samples) {
                *out_num_samples = 1152;
            }
        } else { // V2 (or 2.5)
            bitrate = kBitrateV2[bitrate_index - 1];
            if (out_num_samples) {
                *out_num_samples = (layer == 1 /* L3 */) ? 576 : 1152;
            }
        }

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        if (version == 3 /* V1 */) {
            *frame_size = 144000 * bitrate / sampling_rate + padding;
        } else { // V2 or V2.5
            size_t tmp = (layer == 1 /* L3 */) ? 72000 : 144000;
            *frame_size = tmp * bitrate / sampling_rate + padding;
        }
    }

    if (out_sampling_rate) {
        *out_sampling_rate = sampling_rate;
    }

    if (out_channels) {
        int channel_mode = (header >> 6) & 3;

        *out_channels = (channel_mode == 3) ? 1 : 2;
    }

    return true;
}

static uint32_t U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static status_t calculateOutSize(uint8 *header, size_t inSize,
                                 std::vector<size_t> *decodedSizes) {
    uint32_t channels;
    uint32_t numSamples;
    size_t frameSize;
    size_t totalInSize = 0;

    while (totalInSize + 4 < inSize) {
        if (!parseMp3Header(U32_AT(header + totalInSize), &frameSize,
                            nullptr, &channels, nullptr, &numSamples)) {
            ALOGE("Error in parse mp3 header during outSize estimation");
            return UNKNOWN_ERROR;
        }
        totalInSize += frameSize;
        decodedSizes->push_back(numSamples * channels * sizeof(int16_t));
    }

    if (decodedSizes->empty()) return UNKNOWN_ERROR;

    return OK;
}

c2_status_t C2SoftMP3::onFlush_sm() {
    return onStop();
}

c2_status_t C2SoftMP3::drain(
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

// TODO: Can overall error checking be improved? As in the check for validity of
//       work, pool ptr, work->input.buffers.size() == 1, ...
// TODO: Blind removal of 529 samples from the output may not work. Because
//       mpeg layer 1 frame size is 384 samples per frame. This should introduce
//       negative values and can cause SEG faults. Soft omx mp3 plugin can have
//       this problem (CHECK!)
void C2SoftMP3::process(
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
    size_t inSize = 0u;
    C2ReadView rView = mDummyReadView;
    if (!work->input.buffers.empty()) {
        rView = work->input.buffers[0]->data().linearBlocks().front().map().get();
        inSize = rView.capacity();
        if (inSize && rView.error()) {
            ALOGE("read view map failed %d", rView.error());
            work->result = rView.error();
            return;
        }
    }

    if (inSize == 0 && (!mGaplessBytes || !eos)) {
        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.ordinal = work->input.ordinal;
        return;
    }
    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d", inSize,
          (int)work->input.ordinal.timestamp.peeku(), (int)work->input.ordinal.frameIndex.peeku());

    int32_t numChannels = mConfig->num_channels;
    size_t calOutSize;
    std::vector<size_t> decodedSizes;
    if (inSize && OK != calculateOutSize(const_cast<uint8 *>(rView.data()),
                                         inSize, &decodedSizes)) {
        work->result = C2_CORRUPTED;
        return;
    }
    calOutSize = std::accumulate(decodedSizes.begin(), decodedSizes.end(), 0);
    if (eos) {
        calOutSize += kPVMP3DecoderDelay * numChannels * sizeof(int16_t);
    }

    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchLinearBlock(calOutSize, usage, &block);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }
    C2WriteView wView = block->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        work->result = wView.error();
        return;
    }

    int outSize = 0;
    int outOffset = 0;
    auto it = decodedSizes.begin();
    size_t inPos = 0;
    int32_t samplingRate = mConfig->samplingRate;
    while (inPos < inSize) {
        if (it == decodedSizes.end()) {
            ALOGE("unexpected trailing bytes, ignoring them");
            break;
        }

        mConfig->pInputBuffer = const_cast<uint8 *>(rView.data() + inPos);
        mConfig->inputBufferCurrentLength = (inSize - inPos);
        mConfig->inputBufferMaxLength = 0;
        mConfig->inputBufferUsedLength = 0;
        mConfig->outputFrameSize = (calOutSize - outSize);
        mConfig->pOutputBuffer = reinterpret_cast<int16_t *> (wView.data() + outSize);

        ERROR_CODE decoderErr;
        if ((decoderErr = pvmp3_framedecoder(mConfig, mDecoderBuf))
                != NO_DECODING_ERROR) {
            ALOGE("mp3 decoder returned error %d", decoderErr);
            if (decoderErr != NO_ENOUGH_MAIN_DATA_ERROR
                    && decoderErr != SIDE_INFO_ERROR) {
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }

            // This is recoverable, just ignore the current frame and
            // play silence instead.
            ALOGV("ignoring error and sending silence");
            if (mConfig->outputFrameSize == 0) {
                mConfig->outputFrameSize = *it / sizeof(int16_t);
            }
            memset(mConfig->pOutputBuffer, 0, mConfig->outputFrameSize * sizeof(int16_t));
        } else if (mConfig->samplingRate != samplingRate
                || mConfig->num_channels != numChannels) {
            ALOGI("Reconfiguring decoder: %d->%d Hz, %d->%d channels",
                   samplingRate, mConfig->samplingRate,
                   numChannels, mConfig->num_channels);
            samplingRate = mConfig->samplingRate;
            numChannels = mConfig->num_channels;

            C2StreamSampleRateInfo::output sampleRateInfo(0u, samplingRate);
            C2StreamChannelCountInfo::output channelCountInfo(0u, numChannels);
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
        if (*it != mConfig->outputFrameSize * sizeof(int16_t)) {
            ALOGE("panic, parsed size does not match decoded size");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        outSize += mConfig->outputFrameSize * sizeof(int16_t);
        inPos += mConfig->inputBufferUsedLength;
        it++;
    }
    if (mIsFirst) {
        mIsFirst = false;
        mGaplessBytes = true;
        // The decoder delay is 529 samples, so trim that many samples off
        // the start of the first output buffer. This essentially makes this
        // decoder have zero delay, which the rest of the pipeline assumes.
        outOffset = kPVMP3DecoderDelay * numChannels * sizeof(int16_t);
        mAnchorTimeStamp = work->input.ordinal.timestamp.peekull();
    }
    if (eos) {
        if (calOutSize >=
            outSize + kPVMP3DecoderDelay * numChannels * sizeof(int16_t)) {
            if (!memset(reinterpret_cast<int16_t*>(wView.data() + outSize), 0,
                        kPVMP3DecoderDelay * numChannels * sizeof(int16_t))) {
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
             }
            ALOGV("Adding 529 samples at end");
            mGaplessBytes = false;
            outSize += kPVMP3DecoderDelay * numChannels * sizeof(int16_t);
        }
    }

    uint64_t outTimeStamp = mProcessedSamples * 1000000ll / samplingRate;
    mProcessedSamples += ((outSize - outOffset) / (numChannels * sizeof(int16_t)));
    ALOGV("out buffer attr. offset %d size %d timestamp %u", outOffset, outSize - outOffset,
          (uint32_t)(mAnchorTimeStamp + outTimeStamp));
    decodedSizes.clear();
    work->worklets.front()->output.flags = work->input.flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.buffers.push_back(
            createLinearBuffer(block, outOffset, outSize - outOffset));
    work->worklets.front()->output.ordinal = work->input.ordinal;
    work->worklets.front()->output.ordinal.timestamp = mAnchorTimeStamp + outTimeStamp;
    if (eos) {
        mSignalledOutputEos = true;
        ALOGV("signalled EOS");
    }
}

class C2SoftMp3DecFactory : public C2ComponentFactory {
public:
    C2SoftMp3DecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
              new C2SoftMP3(COMPONENT_NAME,
                            id,
                            std::make_shared<C2SoftMP3::IntfImpl>(mHelper)),
              deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
              new SimpleInterface<C2SoftMP3::IntfImpl>(
                      COMPONENT_NAME, id, std::make_shared<C2SoftMP3::IntfImpl>(mHelper)),
              deleter);
        return C2_OK;
    }

    virtual ~C2SoftMp3DecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftMp3DecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
