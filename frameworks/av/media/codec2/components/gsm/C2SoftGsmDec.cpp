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
#define LOG_TAG "C2SoftGsmDec"
#include <log/log.h>

#include <media/stagefright/foundation/MediaDefs.h>

#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftGsmDec.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.gsm.decoder";

}  // namespace

class C2SoftGsmDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_AUDIO,
                MEDIA_MIMETYPE_AUDIO_MSGSM) {
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
                .withDefault(new C2StreamSampleRateInfo::output(0u, 8000))
                .withFields({C2F(mSampleRate, value).equalTo(8000)})
                .withSetter((Setter<decltype(*mSampleRate)>::StrictValueWithNoDeps))
                .build());

        addParameter(
                DefineParam(mChannelCount, C2_PARAMKEY_CHANNEL_COUNT)
                .withDefault(new C2StreamChannelCountInfo::output(0u, 1))
                .withFields({C2F(mChannelCount, value).equalTo(1)})
                .withSetter(Setter<decltype(*mChannelCount)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::input(0u, 13200))
                .withFields({C2F(mBitrate, value).equalTo(13200)})
                .withSetter(Setter<decltype(*mBitrate)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mInputMaxBufSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withConstValue(new C2StreamMaxBufferSizeInfo::input(0u, 1024 / MSGSM_IN_FRM_SZ * MSGSM_IN_FRM_SZ))
                .build());
    }

   private:
    std::shared_ptr<C2StreamSampleRateInfo::output> mSampleRate;
    std::shared_ptr<C2StreamChannelCountInfo::output> mChannelCount;
    std::shared_ptr<C2StreamBitrateInfo::input> mBitrate;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mInputMaxBufSize;
};

C2SoftGsmDec::C2SoftGsmDec(const char *name, c2_node_id_t id,
                     const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
        std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mGsm(nullptr) {
}

C2SoftGsmDec::~C2SoftGsmDec() {
    onRelease();
}

c2_status_t C2SoftGsmDec::onInit() {
    if (!mGsm) mGsm = gsm_create();
    if (!mGsm) return C2_NO_MEMORY;
    int msopt = 1;
    (void)gsm_option(mGsm, GSM_OPT_WAV49, &msopt);
    mSignalledError = false;
    mSignalledEos = false;
    return C2_OK;
}

c2_status_t C2SoftGsmDec::onStop() {
    if (mGsm) {
        gsm_destroy(mGsm);
        mGsm = nullptr;
    }
    if (!mGsm) mGsm = gsm_create();
    if (!mGsm) return C2_NO_MEMORY;
    int msopt = 1;
    (void)gsm_option(mGsm, GSM_OPT_WAV49, &msopt);
    mSignalledError = false;
    mSignalledEos = false;
    return C2_OK;
}

void C2SoftGsmDec::onReset() {
    (void)onStop();
}

void C2SoftGsmDec::onRelease() {
    if (mGsm) {
        gsm_destroy(mGsm);
        mGsm = nullptr;
    }
}

c2_status_t C2SoftGsmDec::onFlush_sm() {
    return onStop();
}

static size_t decodeGSM(gsm handle, int16_t *out, size_t outCapacity,
                        uint8_t *in, size_t inSize) {
    size_t outSize = 0;

    if (inSize % MSGSM_IN_FRM_SZ == 0
            && (inSize / MSGSM_IN_FRM_SZ * MSGSM_OUT_FRM_SZ * sizeof(*out)
                    <= outCapacity)) {
        while (inSize > 0) {
            gsm_decode(handle, in, out);
            in += FRGSM_IN_FRM_SZ;
            inSize -= FRGSM_IN_FRM_SZ;
            out += FRGSM_OUT_FRM_SZ;
            outSize += FRGSM_OUT_FRM_SZ;

            gsm_decode(handle, in, out);
            in += FRGSM_IN_FRM_SZ_MINUS_1;
            inSize -= FRGSM_IN_FRM_SZ_MINUS_1;
            out += FRGSM_OUT_FRM_SZ;
            outSize += FRGSM_OUT_FRM_SZ;
        }
    }

    return outSize * sizeof(int16_t);
}

void C2SoftGsmDec::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledEos) {
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
            work->result = rView.error();
            return;
        }
    }

    if (inSize == 0) {
        work->worklets.front()->output.flags = work->input.flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.ordinal = work->input.ordinal;
        if (eos) {
            mSignalledEos = true;
            ALOGV("signalled EOS");
        }
        return;
    }
    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d", inSize,
          (int)work->input.ordinal.timestamp.peeku(), (int)work->input.ordinal.frameIndex.peeku());

    size_t outCapacity = (inSize / MSGSM_IN_FRM_SZ ) * MSGSM_OUT_FRM_SZ * sizeof(int16_t);
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

    int16_t *output = reinterpret_cast<int16_t *>(wView.data());
    uint8_t *input = const_cast<uint8_t *>(rView.data() + inOffset);
    size_t outSize = decodeGSM(mGsm, output, outCapacity, input, inSize);
    if (!outSize) {
        ALOGE("encountered improper insize or outsize");
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }
    ALOGV("out buffer attr. size %zu", outSize);
    work->worklets.front()->output.flags = work->input.flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.buffers.push_back(createLinearBuffer(block, 0, outSize));
    work->worklets.front()->output.ordinal = work->input.ordinal;
    if (eos) {
        mSignalledEos = true;
        ALOGV("signalled EOS");
    }
}

c2_status_t C2SoftGsmDec::drain(
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

class C2SoftGSMDecFactory : public C2ComponentFactory {
public:
    C2SoftGSMDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
            GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftGsmDec(COMPONENT_NAME,
                              id,
                              std::make_shared<C2SoftGsmDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftGsmDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftGsmDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftGSMDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftGSMDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
