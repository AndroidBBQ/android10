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
#define LOG_TAG "C2SoftVorbisDec"
#include <log/log.h>

#include <media/stagefright/foundation/MediaDefs.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftVorbisDec.h"

extern "C" {
    #include <Tremolo/codec_internal.h>

    int _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb);
    int _vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb);
    int _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb);
}

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.vorbis.decoder";

}  // namespace

class C2SoftVorbisDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_VORBIS) {
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
                .withFields({C2F(mSampleRate, value).inRange(8000, 96000)})
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
                .withFields({C2F(mBitrate, value).inRange(32000, 500000)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 8192 * 2 * sizeof(int16_t)))
                .build());
    }

private:
    std::shared_ptr<C2StreamSampleRateInfo::output> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::output> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::input> mBitrate;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
};

C2SoftVorbisDec::C2SoftVorbisDec(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mState(nullptr),
      mVi(nullptr) {
}

C2SoftVorbisDec::~C2SoftVorbisDec() {
    onRelease();
}

c2_status_t C2SoftVorbisDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_NO_MEMORY;
}

c2_status_t C2SoftVorbisDec::onStop() {
    if (mState) {
        vorbis_dsp_clear(mState);
        delete mState;
        mState = nullptr;
    }

    if (mVi) {
        vorbis_info_clear(mVi);
        delete mVi;
        mVi = nullptr;
    }
    mNumFramesLeftOnPage = -1;
    mSignalledOutputEos = false;
    mSignalledError = false;

    return (initDecoder() == OK ? C2_OK : C2_CORRUPTED);
}

void C2SoftVorbisDec::onReset() {
    (void)onStop();
}

void C2SoftVorbisDec::onRelease() {
    if (mState) {
        vorbis_dsp_clear(mState);
        delete mState;
        mState = nullptr;
    }

    if (mVi) {
        vorbis_info_clear(mVi);
        delete mVi;
        mVi = nullptr;
    }
}

status_t C2SoftVorbisDec::initDecoder() {
    mVi = new vorbis_info{};
    if (!mVi) return NO_MEMORY;
    vorbis_info_clear(mVi);

    mState = new vorbis_dsp_state{};
    if (!mState) return NO_MEMORY;
    vorbis_dsp_clear(mState);

    mNumFramesLeftOnPage = -1;
    mSignalledError = false;
    mSignalledOutputEos = false;
    mInfoUnpacked = false;
    mBooksUnpacked = false;
    return OK;
}

c2_status_t C2SoftVorbisDec::onFlush_sm() {
    mNumFramesLeftOnPage = -1;
    mSignalledOutputEos = false;
    if (mState) vorbis_dsp_restart(mState);

    return C2_OK;
}

c2_status_t C2SoftVorbisDec::drain(
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

static void makeBitReader(
        const void *data, size_t size,
        ogg_buffer *buf, ogg_reference *ref, oggpack_buffer *bits) {
    buf->data = (uint8_t *)data;
    buf->size = size;
    buf->refcount = 1;
    buf->ptr.owner = nullptr;

    ref->buffer = buf;
    ref->begin = 0;
    ref->length = size;
    ref->next = nullptr;

    oggpack_readinit(bits, ref);
}

// (CHECK!) multiframe is tricky. decode call doesnt return the number of bytes
// consumed by the component. Also it is unclear why numPageFrames is being
// tagged at the end of input buffers for new pages. Refer lines 297-300 in
// SimpleDecodingSource.cpp
void C2SoftVorbisDec::process(
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
            work->result = rView.error();
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
    int32_t numChannels  = mVi->channels;
    int32_t samplingRate = mVi->rate;
    /* Decode vorbis headers only once */
    if (inSize > 7 && !memcmp(&data[1], "vorbis", 6) && (!mInfoUnpacked || !mBooksUnpacked)) {
        if ((data[0] != 1) && (data[0] != 5)) {
            ALOGE("unexpected type received %d", data[0]);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }

        ogg_buffer buf;
        ogg_reference ref;
        oggpack_buffer bits;

        // skip 7 <type + "vorbis"> bytes
        makeBitReader((const uint8_t *)data + 7, inSize - 7, &buf, &ref, &bits);
        if (data[0] == 1) {
            vorbis_info_init(mVi);
            if (0 != _vorbis_unpack_info(mVi, &bits)) {
                ALOGE("Encountered error while unpacking info");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            if (mVi->rate != samplingRate ||
                    mVi->channels != numChannels) {
                ALOGV("vorbis: rate/channels changed: %ld/%d", mVi->rate, mVi->channels);
                samplingRate = mVi->rate;
                numChannels = mVi->channels;

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
            mInfoUnpacked = true;
        } else {
            if (!mInfoUnpacked) {
                ALOGE("Data with type:5 sent before sending type:1");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            if (0 != _vorbis_unpack_books(mVi, &bits)) {
                ALOGE("Encountered error while unpacking books");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            if (0 != vorbis_dsp_init(mState, mVi)) {
                ALOGE("Encountered error while dsp init");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            mBooksUnpacked = true;
        }
        fillEmptyWork(work);
        if (eos) {
            mSignalledOutputEos = true;
            ALOGV("signalled EOS");
        }
        return;
    }

    if (!mInfoUnpacked || !mBooksUnpacked) {
        ALOGE("Missing CODEC_CONFIG data mInfoUnpacked: %d mBooksUnpack %d", mInfoUnpacked, mBooksUnpacked);
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }

    int32_t numPageFrames = 0;
    if (inSize < sizeof(numPageFrames)) {
        ALOGE("input header has size %zu, expected %zu", inSize, sizeof(numPageFrames));
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }
    memcpy(&numPageFrames, data + inSize - sizeof(numPageFrames), sizeof(numPageFrames));
    inSize -= sizeof(numPageFrames);
    if (numPageFrames >= 0) {
        mNumFramesLeftOnPage = numPageFrames;
    }

    ogg_buffer buf;
    buf.data = const_cast<unsigned char*>(data);
    buf.size = inSize;
    buf.refcount = 1;
    buf.ptr.owner = nullptr;

    ogg_reference ref;
    ref.buffer = &buf;
    ref.begin = 0;
    ref.length = buf.size;
    ref.next = nullptr;

    ogg_packet pack;
    pack.packet = &ref;
    pack.bytes = ref.length;
    pack.b_o_s = 0;
    pack.e_o_s = 0;
    pack.granulepos = 0;
    pack.packetno = 0;

    size_t maxSamplesInBuffer = kMaxNumSamplesPerChannel * mVi->channels;
    size_t outCapacity =  maxSamplesInBuffer * sizeof(int16_t);
    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchLinearBlock(outCapacity, usage, &block);
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

    int numFrames = 0;
    int ret = vorbis_dsp_synthesis(mState, &pack, 1);
    if (0 != ret) {
        ALOGD("vorbis_dsp_synthesis returned %d; ignored", ret);
    } else {
        numFrames = vorbis_dsp_pcmout(
                mState,  reinterpret_cast<int16_t *> (wView.data()),
                kMaxNumSamplesPerChannel);
        if (numFrames < 0) {
            ALOGD("vorbis_dsp_pcmout returned %d", numFrames);
            numFrames = 0;
        }
    }

    if (mNumFramesLeftOnPage >= 0) {
        if (numFrames > mNumFramesLeftOnPage) {
            ALOGV("discarding %d frames at end of page", numFrames - mNumFramesLeftOnPage);
            numFrames = mNumFramesLeftOnPage;
        }
        mNumFramesLeftOnPage -= numFrames;
    }

    if (numFrames) {
        int outSize = numFrames * sizeof(int16_t) * mVi->channels;

        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(createLinearBuffer(block, 0, outSize));
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

class C2SoftVorbisDecFactory : public C2ComponentFactory {
public:
    C2SoftVorbisDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftVorbisDec(COMPONENT_NAME,
                              id,
                              std::make_shared<C2SoftVorbisDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftVorbisDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftVorbisDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftVorbisDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftVorbisDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
