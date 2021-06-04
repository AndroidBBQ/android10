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
#define LOG_TAG "C2SoftVpxDec"
#include <log/log.h>

#include <algorithm>

#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftVpxDec.h"

namespace android {

#ifdef VP9
constexpr char COMPONENT_NAME[] = "c2.android.vp9.decoder";
#else
constexpr char COMPONENT_NAME[] = "c2.android.vp8.decoder";
#endif

class C2SoftVpxDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_VIDEO,
#ifdef VP9
                MEDIA_MIMETYPE_VIDEO_VP9
#else
                MEDIA_MIMETYPE_VIDEO_VP8
#endif
                ) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();

        // TODO: output latency and reordering

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::output(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 2048, 2),
                    C2F(mSize, height).inRange(2, 2048, 2),
                })
                .withSetter(SizeSetter)
                .build());

#ifdef VP9
        // TODO: Add C2Config::PROFILE_VP9_2HDR ??
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_VP9_0, C2Config::LEVEL_VP9_5))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                            C2Config::PROFILE_VP9_0,
                            C2Config::PROFILE_VP9_2}),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_VP9_1,
                            C2Config::LEVEL_VP9_1_1,
                            C2Config::LEVEL_VP9_2,
                            C2Config::LEVEL_VP9_2_1,
                            C2Config::LEVEL_VP9_3,
                            C2Config::LEVEL_VP9_3_1,
                            C2Config::LEVEL_VP9_4,
                            C2Config::LEVEL_VP9_4_1,
                            C2Config::LEVEL_VP9_5,
                    })
                })
                .withSetter(ProfileLevelSetter, mSize)
                .build());

        mHdr10PlusInfoInput = C2StreamHdr10PlusInfo::input::AllocShared(0);
        addParameter(
                DefineParam(mHdr10PlusInfoInput, C2_PARAMKEY_INPUT_HDR10_PLUS_INFO)
                .withDefault(mHdr10PlusInfoInput)
                .withFields({
                    C2F(mHdr10PlusInfoInput, m.value).any(),
                })
                .withSetter(Hdr10PlusInfoInputSetter)
                .build());

        mHdr10PlusInfoOutput = C2StreamHdr10PlusInfo::output::AllocShared(0);
        addParameter(
                DefineParam(mHdr10PlusInfoOutput, C2_PARAMKEY_OUTPUT_HDR10_PLUS_INFO)
                .withDefault(mHdr10PlusInfoOutput)
                .withFields({
                    C2F(mHdr10PlusInfoOutput, m.value).any(),
                })
                .withSetter(Hdr10PlusInfoOutputSetter)
                .build());

#if 0
        // sample BT.2020 static info
        mHdrStaticInfo = std::make_shared<C2StreamHdrStaticInfo::output>();
        mHdrStaticInfo->mastering = {
            .red   = { .x = 0.708,  .y = 0.292 },
            .green = { .x = 0.170,  .y = 0.797 },
            .blue  = { .x = 0.131,  .y = 0.046 },
            .white = { .x = 0.3127, .y = 0.3290 },
            .maxLuminance = 1000,
            .minLuminance = 0.1,
        };
        mHdrStaticInfo->maxCll = 1000;
        mHdrStaticInfo->maxFall = 120;

        mHdrStaticInfo->maxLuminance = 0; // disable static info

        helper->addStructDescriptors<C2MasteringDisplayColorVolumeStruct, C2ColorXyStruct>();
        addParameter(
                DefineParam(mHdrStaticInfo, C2_PARAMKEY_HDR_STATIC_INFO)
                .withDefault(mHdrStaticInfo)
                .withFields({
                    C2F(mHdrStaticInfo, mastering.red.x).inRange(0, 1),
                    // TODO
                })
                .withSetter(HdrStaticInfoSetter)
                .build());
#endif
#else
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withConstValue(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_UNUSED, C2Config::LEVEL_UNUSED))
                .build());
#endif

        addParameter(
                DefineParam(mMaxSize, C2_PARAMKEY_MAX_PICTURE_SIZE)
                .withDefault(new C2StreamMaxPictureSizeTuning::output(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 2048, 2),
                    C2F(mSize, height).inRange(2, 2048, 2),
                })
                .withSetter(MaxPictureSizeSetter, mSize)
                .build());

        addParameter(
                DefineParam(mMaxInputSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withDefault(new C2StreamMaxBufferSizeInfo::input(0u, 320 * 240 * 3 / 4))
                .withFields({
                    C2F(mMaxInputSize, value).any(),
                })
                .calculatedAs(MaxInputSizeSetter, mMaxSize)
                .build());

        C2ChromaOffsetStruct locations[1] = { C2ChromaOffsetStruct::ITU_YUV_420_0() };
        std::shared_ptr<C2StreamColorInfo::output> defaultColorInfo =
            C2StreamColorInfo::output::AllocShared(
                    1u, 0u, 8u /* bitDepth */, C2Color::YUV_420);
        memcpy(defaultColorInfo->m.locations, locations, sizeof(locations));

        defaultColorInfo =
            C2StreamColorInfo::output::AllocShared(
                    { C2ChromaOffsetStruct::ITU_YUV_420_0() },
                    0u, 8u /* bitDepth */, C2Color::YUV_420);
        helper->addStructDescriptors<C2ChromaOffsetStruct>();

        addParameter(
                DefineParam(mColorInfo, C2_PARAMKEY_CODED_COLOR_INFO)
                .withConstValue(defaultColorInfo)
                .build());

        addParameter(
                DefineParam(mDefaultColorAspects, C2_PARAMKEY_DEFAULT_COLOR_ASPECTS)
                .withDefault(new C2StreamColorAspectsTuning::output(
                        0u, C2Color::RANGE_UNSPECIFIED, C2Color::PRIMARIES_UNSPECIFIED,
                        C2Color::TRANSFER_UNSPECIFIED, C2Color::MATRIX_UNSPECIFIED))
                .withFields({
                    C2F(mDefaultColorAspects, range).inRange(
                                C2Color::RANGE_UNSPECIFIED,     C2Color::RANGE_OTHER),
                    C2F(mDefaultColorAspects, primaries).inRange(
                                C2Color::PRIMARIES_UNSPECIFIED, C2Color::PRIMARIES_OTHER),
                    C2F(mDefaultColorAspects, transfer).inRange(
                                C2Color::TRANSFER_UNSPECIFIED,  C2Color::TRANSFER_OTHER),
                    C2F(mDefaultColorAspects, matrix).inRange(
                                C2Color::MATRIX_UNSPECIFIED,    C2Color::MATRIX_OTHER)
                })
                .withSetter(DefaultColorAspectsSetter)
                .build());

        // TODO: support more formats?
        addParameter(
                DefineParam(mPixelFormat, C2_PARAMKEY_PIXEL_FORMAT)
                .withConstValue(new C2StreamPixelFormatInfo::output(
                                     0u, HAL_PIXEL_FORMAT_YCBCR_420_888))
                .build());
    }

    static C2R SizeSetter(bool mayBlock, const C2P<C2StreamPictureSizeInfo::output> &oldMe,
                          C2P<C2StreamPictureSizeInfo::output> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (!me.F(me.v.width).supportsAtAll(me.v.width)) {
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.width)));
            me.set().width = oldMe.v.width;
        }
        if (!me.F(me.v.height).supportsAtAll(me.v.height)) {
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.height)));
            me.set().height = oldMe.v.height;
        }
        return res;
    }

    static C2R MaxPictureSizeSetter(bool mayBlock, C2P<C2StreamMaxPictureSizeTuning::output> &me,
                                    const C2P<C2StreamPictureSizeInfo::output> &size) {
        (void)mayBlock;
        // TODO: get max width/height from the size's field helpers vs. hardcoding
        me.set().width = c2_min(c2_max(me.v.width, size.v.width), 2048u);
        me.set().height = c2_min(c2_max(me.v.height, size.v.height), 2048u);
        return C2R::Ok();
    }

    static C2R MaxInputSizeSetter(bool mayBlock, C2P<C2StreamMaxBufferSizeInfo::input> &me,
                                  const C2P<C2StreamMaxPictureSizeTuning::output> &maxSize) {
        (void)mayBlock;
        // assume compression ratio of 2
        me.set().value = (((maxSize.v.width + 63) / 64) * ((maxSize.v.height + 63) / 64) * 3072);
        return C2R::Ok();
    }

    static C2R DefaultColorAspectsSetter(bool mayBlock, C2P<C2StreamColorAspectsTuning::output> &me) {
        (void)mayBlock;
        if (me.v.range > C2Color::RANGE_OTHER) {
            me.set().range = C2Color::RANGE_OTHER;
        }
        if (me.v.primaries > C2Color::PRIMARIES_OTHER) {
            me.set().primaries = C2Color::PRIMARIES_OTHER;
        }
        if (me.v.transfer > C2Color::TRANSFER_OTHER) {
            me.set().transfer = C2Color::TRANSFER_OTHER;
        }
        if (me.v.matrix > C2Color::MATRIX_OTHER) {
            me.set().matrix = C2Color::MATRIX_OTHER;
        }
        return C2R::Ok();
    }

    static C2R ProfileLevelSetter(bool mayBlock, C2P<C2StreamProfileLevelInfo::input> &me,
                                  const C2P<C2StreamPictureSizeInfo::output> &size) {
        (void)mayBlock;
        (void)size;
        (void)me;  // TODO: validate
        return C2R::Ok();
    }
    std::shared_ptr<C2StreamColorAspectsTuning::output> getDefaultColorAspects_l() {
        return mDefaultColorAspects;
    }

    static C2R Hdr10PlusInfoInputSetter(bool mayBlock, C2P<C2StreamHdr10PlusInfo::input> &me) {
        (void)mayBlock;
        (void)me;  // TODO: validate
        return C2R::Ok();
    }

    static C2R Hdr10PlusInfoOutputSetter(bool mayBlock, C2P<C2StreamHdr10PlusInfo::output> &me) {
        (void)mayBlock;
        (void)me;  // TODO: validate
        return C2R::Ok();
    }

private:
    std::shared_ptr<C2StreamProfileLevelInfo::input> mProfileLevel;
    std::shared_ptr<C2StreamPictureSizeInfo::output> mSize;
    std::shared_ptr<C2StreamMaxPictureSizeTuning::output> mMaxSize;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mMaxInputSize;
    std::shared_ptr<C2StreamColorInfo::output> mColorInfo;
    std::shared_ptr<C2StreamPixelFormatInfo::output> mPixelFormat;
    std::shared_ptr<C2StreamColorAspectsTuning::output> mDefaultColorAspects;
#ifdef VP9
#if 0
    std::shared_ptr<C2StreamHdrStaticInfo::output> mHdrStaticInfo;
#endif
    std::shared_ptr<C2StreamHdr10PlusInfo::input> mHdr10PlusInfoInput;
    std::shared_ptr<C2StreamHdr10PlusInfo::output> mHdr10PlusInfoOutput;
#endif
};

C2SoftVpxDec::ConverterThread::ConverterThread(
        const std::shared_ptr<Mutexed<ConversionQueue>> &queue)
    : Thread(false), mQueue(queue) {}

bool C2SoftVpxDec::ConverterThread::threadLoop() {
    Mutexed<ConversionQueue>::Locked queue(*mQueue);
    if (queue->entries.empty()) {
        queue.waitForCondition(queue->cond);
        if (queue->entries.empty()) {
            return true;
        }
    }
    std::function<void()> convert = queue->entries.front();
    queue->entries.pop_front();
    if (!queue->entries.empty()) {
        queue->cond.signal();
    }
    queue.unlock();

    convert();

    queue.lock();
    if (--queue->numPending == 0u) {
        queue->cond.broadcast();
    }
    return true;
}

C2SoftVpxDec::C2SoftVpxDec(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mCodecCtx(nullptr),
      mCoreCount(1),
      mQueue(new Mutexed<ConversionQueue>) {
}

C2SoftVpxDec::~C2SoftVpxDec() {
    onRelease();
}

c2_status_t C2SoftVpxDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

c2_status_t C2SoftVpxDec::onStop() {
    mSignalledError = false;
    mSignalledOutputEos = false;

    return C2_OK;
}

void C2SoftVpxDec::onReset() {
    (void)onStop();
    c2_status_t err = onFlush_sm();
    if (err != C2_OK)
    {
        ALOGW("Failed to flush decoder. Try to hard reset decoder");
        destroyDecoder();
        (void)initDecoder();
    }
}

void C2SoftVpxDec::onRelease() {
    destroyDecoder();
}

c2_status_t C2SoftVpxDec::onFlush_sm() {
    if (mFrameParallelMode) {
        // Flush decoder by passing nullptr data ptr and 0 size.
        // Ideally, this should never fail.
        if (vpx_codec_decode(mCodecCtx, nullptr, 0, nullptr, 0)) {
            ALOGE("Failed to flush on2 decoder.");
            return C2_CORRUPTED;
        }
    }

    // Drop all the decoded frames in decoder.
    vpx_codec_iter_t iter = nullptr;
    while (vpx_codec_get_frame(mCodecCtx, &iter)) {
    }

    mSignalledError = false;
    mSignalledOutputEos = false;
    return C2_OK;
}

static int GetCPUCoreCount() {
    int cpuCoreCount = 1;
#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif
    CHECK(cpuCoreCount >= 1);
    ALOGV("Number of CPU cores: %d", cpuCoreCount);
    return cpuCoreCount;
}

status_t C2SoftVpxDec::initDecoder() {
#ifdef VP9
    mMode = MODE_VP9;
#else
    mMode = MODE_VP8;
#endif

    mWidth = 320;
    mHeight = 240;
    mFrameParallelMode = false;
    mSignalledOutputEos = false;
    mSignalledError = false;

    if (!mCodecCtx) {
        mCodecCtx = new vpx_codec_ctx_t;
    }
    if (!mCodecCtx) {
        ALOGE("mCodecCtx is null");
        return NO_MEMORY;
    }

    vpx_codec_dec_cfg_t cfg;
    memset(&cfg, 0, sizeof(vpx_codec_dec_cfg_t));
    cfg.threads = mCoreCount = GetCPUCoreCount();

    vpx_codec_flags_t flags;
    memset(&flags, 0, sizeof(vpx_codec_flags_t));
    if (mFrameParallelMode) flags |= VPX_CODEC_USE_FRAME_THREADING;

    vpx_codec_err_t vpx_err;
    if ((vpx_err = vpx_codec_dec_init(
                 mCodecCtx, mMode == MODE_VP8 ? &vpx_codec_vp8_dx_algo : &vpx_codec_vp9_dx_algo,
                 &cfg, flags))) {
        ALOGE("on2 decoder failed to initialize. (%d)", vpx_err);
        return UNKNOWN_ERROR;
    }

    if (mMode == MODE_VP9) {
        using namespace std::string_literals;
        for (int i = 0; i < mCoreCount; ++i) {
            sp<ConverterThread> thread(new ConverterThread(mQueue));
            mConverterThreads.push_back(thread);
            if (thread->run(("vp9conv #"s + std::to_string(i)).c_str(),
                            ANDROID_PRIORITY_AUDIO) != OK) {
                return UNKNOWN_ERROR;
            }
        }
    }

    return OK;
}

status_t C2SoftVpxDec::destroyDecoder() {
    if  (mCodecCtx) {
        vpx_codec_destroy(mCodecCtx);
        delete mCodecCtx;
        mCodecCtx = nullptr;
    }
    bool running = true;
    for (const sp<ConverterThread> &thread : mConverterThreads) {
        thread->requestExit();
    }
    while (running) {
        mQueue->lock()->cond.broadcast();
        running = false;
        for (const sp<ConverterThread> &thread : mConverterThreads) {
            if (thread->isRunning()) {
                running = true;
                break;
            }
        }
    }
    mConverterThreads.clear();

    return OK;
}

void fillEmptyWork(const std::unique_ptr<C2Work> &work) {
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

void C2SoftVpxDec::finishWork(uint64_t index, const std::unique_ptr<C2Work> &work,
                           const std::shared_ptr<C2GraphicBlock> &block) {
    std::shared_ptr<C2Buffer> buffer = createGraphicBuffer(block,
                                                           C2Rect(mWidth, mHeight));
    auto fillWork = [buffer, index, intf = this->mIntf](
            const std::unique_ptr<C2Work> &work) {
        uint32_t flags = 0;
        if ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) &&
                (c2_cntr64_t(index) == work->input.ordinal.frameIndex)) {
            flags |= C2FrameData::FLAG_END_OF_STREAM;
            ALOGV("signalling eos");
        }
        work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;

        for (const std::unique_ptr<C2Param> &param: work->input.configUpdate) {
            if (param) {
                C2StreamHdr10PlusInfo::input *hdr10PlusInfo =
                        C2StreamHdr10PlusInfo::input::From(param.get());

                if (hdr10PlusInfo != nullptr) {
                    std::vector<std::unique_ptr<C2SettingResult>> failures;
                    std::unique_ptr<C2Param> outParam = C2Param::CopyAsStream(
                            *param.get(), true /*output*/, param->stream());
                    c2_status_t err = intf->config(
                            { outParam.get() }, C2_MAY_BLOCK, &failures);
                    if (err == C2_OK) {
                        work->worklets.front()->output.configUpdate.push_back(
                                C2Param::Copy(*outParam.get()));
                    } else {
                        ALOGE("finishWork: Config update size failed");
                    }
                    break;
                }
            }
        }
    };
    if (work && c2_cntr64_t(index) == work->input.ordinal.frameIndex) {
        fillWork(work);
    } else {
        finish(index, fillWork);
    }
}

void C2SoftVpxDec::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 0u;
    work->worklets.front()->output.configUpdate.clear();
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledOutputEos) {
        work->result = C2_BAD_VALUE;
        return;
    }

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

    bool codecConfig = ((work->input.flags & C2FrameData::FLAG_CODEC_CONFIG) !=0);
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);

    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d, flags %x",
          inSize, (int)work->input.ordinal.timestamp.peeku(),
          (int)work->input.ordinal.frameIndex.peeku(), work->input.flags);

    // Software VP9 Decoder does not need the Codec Specific Data (CSD)
    // (specified in http://www.webmproject.org/vp9/profiles/). Ignore it if
    // it was passed.
    if (codecConfig) {
        // Ignore CSD buffer for VP9.
        if (mMode == MODE_VP9) {
            fillEmptyWork(work);
            return;
        } else {
            // Tolerate the CSD buffer for VP8. This is a workaround
            // for b/28689536. continue
            ALOGW("WARNING: Got CSD buffer for VP8. Continue");
        }
    }

    if (inSize) {
        uint8_t *bitstream = const_cast<uint8_t *>(rView.data() + inOffset);
        vpx_codec_err_t err = vpx_codec_decode(
                mCodecCtx, bitstream, inSize, &work->input.ordinal.frameIndex, 0);
        if (err != VPX_CODEC_OK) {
            ALOGE("on2 decoder failed to decode frame. err: %d", err);
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        }
    }

    status_t err = outputBuffer(pool, work);
    if (err == NOT_ENOUGH_DATA) {
        if (inSize > 0) {
            ALOGV("Maybe non-display frame at %lld.",
                  work->input.ordinal.frameIndex.peekll());
            // send the work back with empty buffer.
            inSize = 0;
        }
    } else if (err != OK) {
        ALOGD("Error while getting the output frame out");
        // work->result would be already filled; do fillEmptyWork() below to
        // send the work back.
        inSize = 0;
    }

    if (eos) {
        drainInternal(DRAIN_COMPONENT_WITH_EOS, pool, work);
        mSignalledOutputEos = true;
    } else if (!inSize) {
        fillEmptyWork(work);
    }
}

static void copyOutputBufferToYuvPlanarFrame(
        uint8_t *dst, const uint8_t *srcY, const uint8_t *srcU, const uint8_t *srcV,
        size_t srcYStride, size_t srcUStride, size_t srcVStride,
        size_t dstYStride, size_t dstUVStride,
        uint32_t width, uint32_t height) {
    uint8_t *dstStart = dst;

    for (size_t i = 0; i < height; ++i) {
         memcpy(dst, srcY, width);
         srcY += srcYStride;
         dst += dstYStride;
    }

    dst = dstStart + dstYStride * height;
    for (size_t i = 0; i < height / 2; ++i) {
         memcpy(dst, srcV, width / 2);
         srcV += srcVStride;
         dst += dstUVStride;
    }

    dst = dstStart + (dstYStride * height) + (dstUVStride * height / 2);
    for (size_t i = 0; i < height / 2; ++i) {
         memcpy(dst, srcU, width / 2);
         srcU += srcUStride;
         dst += dstUVStride;
    }
}

static void convertYUV420Planar16ToY410(uint32_t *dst,
        const uint16_t *srcY, const uint16_t *srcU, const uint16_t *srcV,
        size_t srcYStride, size_t srcUStride, size_t srcVStride,
        size_t dstStride, size_t width, size_t height) {

    // Converting two lines at a time, slightly faster
    for (size_t y = 0; y < height; y += 2) {
        uint32_t *dstTop = (uint32_t *) dst;
        uint32_t *dstBot = (uint32_t *) (dst + dstStride);
        uint16_t *ySrcTop = (uint16_t*) srcY;
        uint16_t *ySrcBot = (uint16_t*) (srcY + srcYStride);
        uint16_t *uSrc = (uint16_t*) srcU;
        uint16_t *vSrc = (uint16_t*) srcV;

        uint32_t u01, v01, y01, y23, y45, y67, uv0, uv1;
        size_t x = 0;
        for (; x < width - 3; x += 4) {

            u01 = *((uint32_t*)uSrc); uSrc += 2;
            v01 = *((uint32_t*)vSrc); vSrc += 2;

            y01 = *((uint32_t*)ySrcTop); ySrcTop += 2;
            y23 = *((uint32_t*)ySrcTop); ySrcTop += 2;
            y45 = *((uint32_t*)ySrcBot); ySrcBot += 2;
            y67 = *((uint32_t*)ySrcBot); ySrcBot += 2;

            uv0 = (u01 & 0x3FF) | ((v01 & 0x3FF) << 20);
            uv1 = (u01 >> 16) | ((v01 >> 16) << 20);

            *dstTop++ = 3 << 30 | ((y01 & 0x3FF) << 10) | uv0;
            *dstTop++ = 3 << 30 | ((y01 >> 16) << 10) | uv0;
            *dstTop++ = 3 << 30 | ((y23 & 0x3FF) << 10) | uv1;
            *dstTop++ = 3 << 30 | ((y23 >> 16) << 10) | uv1;

            *dstBot++ = 3 << 30 | ((y45 & 0x3FF) << 10) | uv0;
            *dstBot++ = 3 << 30 | ((y45 >> 16) << 10) | uv0;
            *dstBot++ = 3 << 30 | ((y67 & 0x3FF) << 10) | uv1;
            *dstBot++ = 3 << 30 | ((y67 >> 16) << 10) | uv1;
        }

        // There should be at most 2 more pixels to process. Note that we don't
        // need to consider odd case as the buffer is always aligned to even.
        if (x < width) {
            u01 = *uSrc;
            v01 = *vSrc;
            y01 = *((uint32_t*)ySrcTop);
            y45 = *((uint32_t*)ySrcBot);
            uv0 = (u01 & 0x3FF) | ((v01 & 0x3FF) << 20);
            *dstTop++ = ((y01 & 0x3FF) << 10) | uv0;
            *dstTop++ = ((y01 >> 16) << 10) | uv0;
            *dstBot++ = ((y45 & 0x3FF) << 10) | uv0;
            *dstBot++ = ((y45 >> 16) << 10) | uv0;
        }

        srcY += srcYStride * 2;
        srcU += srcUStride;
        srcV += srcVStride;
        dst += dstStride * 2;
    }

    return;
}

static void convertYUV420Planar16ToYUV420Planar(uint8_t *dst,
        const uint16_t *srcY, const uint16_t *srcU, const uint16_t *srcV,
        size_t srcYStride, size_t srcUStride, size_t srcVStride,
        size_t dstYStride, size_t dstUVStride, size_t width, size_t height) {

    uint8_t *dstY = (uint8_t *)dst;
    size_t dstYSize = dstYStride * height;
    size_t dstUVSize = dstUVStride * height / 2;
    uint8_t *dstV = dstY + dstYSize;
    uint8_t *dstU = dstV + dstUVSize;

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            dstY[x] = (uint8_t)(srcY[x] >> 2);
        }

        srcY += srcYStride;
        dstY += dstYStride;
    }

    for (size_t y = 0; y < (height + 1) / 2; ++y) {
        for (size_t x = 0; x < (width + 1) / 2; ++x) {
            dstU[x] = (uint8_t)(srcU[x] >> 2);
            dstV[x] = (uint8_t)(srcV[x] >> 2);
        }

        srcU += srcUStride;
        srcV += srcVStride;
        dstU += dstUVStride;
        dstV += dstUVStride;
    }
    return;
}
status_t C2SoftVpxDec::outputBuffer(
        const std::shared_ptr<C2BlockPool> &pool,
        const std::unique_ptr<C2Work> &work)
{
    if (!(work && pool)) return BAD_VALUE;

    vpx_codec_iter_t iter = nullptr;
    vpx_image_t *img = vpx_codec_get_frame(mCodecCtx, &iter);

    if (!img) return NOT_ENOUGH_DATA;

    if (img->d_w != mWidth || img->d_h != mHeight) {
        mWidth = img->d_w;
        mHeight = img->d_h;

        C2StreamPictureSizeInfo::output size(0u, mWidth, mHeight);
        std::vector<std::unique_ptr<C2SettingResult>> failures;
        c2_status_t err = mIntf->config({&size}, C2_MAY_BLOCK, &failures);
        if (err == C2_OK) {
            work->worklets.front()->output.configUpdate.push_back(
                C2Param::Copy(size));
        } else {
            ALOGE("Config update size failed");
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return UNKNOWN_ERROR;
        }

    }
    CHECK(img->fmt == VPX_IMG_FMT_I420 || img->fmt == VPX_IMG_FMT_I42016);

    std::shared_ptr<C2GraphicBlock> block;
    uint32_t format = HAL_PIXEL_FORMAT_YV12;
    if (img->fmt == VPX_IMG_FMT_I42016) {
        IntfImpl::Lock lock = mIntf->lock();
        std::shared_ptr<C2StreamColorAspectsTuning::output> defaultColorAspects = mIntf->getDefaultColorAspects_l();

        if (defaultColorAspects->primaries == C2Color::PRIMARIES_BT2020 &&
            defaultColorAspects->matrix == C2Color::MATRIX_BT2020 &&
            defaultColorAspects->transfer == C2Color::TRANSFER_ST2084) {
            format = HAL_PIXEL_FORMAT_RGBA_1010102;
        }
    }
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchGraphicBlock(align(mWidth, 16), mHeight, format, usage, &block);
    if (err != C2_OK) {
        ALOGE("fetchGraphicBlock for Output failed with status %d", err);
        work->result = err;
        return UNKNOWN_ERROR;
    }

    C2GraphicView wView = block->map().get();
    if (wView.error()) {
        ALOGE("graphic view map failed %d", wView.error());
        work->result = C2_CORRUPTED;
        return UNKNOWN_ERROR;
    }

    ALOGV("provided (%dx%d) required (%dx%d), out frameindex %lld",
           block->width(), block->height(), mWidth, mHeight,
           ((c2_cntr64_t *)img->user_priv)->peekll());

    uint8_t *dst = const_cast<uint8_t *>(wView.data()[C2PlanarLayout::PLANE_Y]);
    size_t srcYStride = img->stride[VPX_PLANE_Y];
    size_t srcUStride = img->stride[VPX_PLANE_U];
    size_t srcVStride = img->stride[VPX_PLANE_V];
    C2PlanarLayout layout = wView.layout();
    size_t dstYStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
    size_t dstUVStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;

    if (img->fmt == VPX_IMG_FMT_I42016) {
        const uint16_t *srcY = (const uint16_t *)img->planes[VPX_PLANE_Y];
        const uint16_t *srcU = (const uint16_t *)img->planes[VPX_PLANE_U];
        const uint16_t *srcV = (const uint16_t *)img->planes[VPX_PLANE_V];

        if (format == HAL_PIXEL_FORMAT_RGBA_1010102) {
            Mutexed<ConversionQueue>::Locked queue(*mQueue);
            size_t i = 0;
            constexpr size_t kHeight = 64;
            for (; i < mHeight; i += kHeight) {
                queue->entries.push_back(
                        [dst, srcY, srcU, srcV,
                         srcYStride, srcUStride, srcVStride, dstYStride,
                         width = mWidth, height = std::min(mHeight - i, kHeight)] {
                            convertYUV420Planar16ToY410(
                                    (uint32_t *)dst, srcY, srcU, srcV, srcYStride / 2,
                                    srcUStride / 2, srcVStride / 2, dstYStride / sizeof(uint32_t),
                                    width, height);
                        });
                srcY += srcYStride / 2 * kHeight;
                srcU += srcUStride / 2 * (kHeight / 2);
                srcV += srcVStride / 2 * (kHeight / 2);
                dst += dstYStride * kHeight;
            }
            CHECK_EQ(0u, queue->numPending);
            queue->numPending = queue->entries.size();
            while (queue->numPending > 0) {
                queue->cond.signal();
                queue.waitForCondition(queue->cond);
            }
        } else {
            convertYUV420Planar16ToYUV420Planar(dst, srcY, srcU, srcV, srcYStride / 2,
                                                srcUStride / 2, srcVStride / 2,
                                                dstYStride, dstUVStride,
                                                mWidth, mHeight);
        }
    } else {
        const uint8_t *srcY = (const uint8_t *)img->planes[VPX_PLANE_Y];
        const uint8_t *srcU = (const uint8_t *)img->planes[VPX_PLANE_U];
        const uint8_t *srcV = (const uint8_t *)img->planes[VPX_PLANE_V];
        copyOutputBufferToYuvPlanarFrame(
                dst, srcY, srcU, srcV,
                srcYStride, srcUStride, srcVStride,
                dstYStride, dstUVStride,
                mWidth, mHeight);
    }
    finishWork(((c2_cntr64_t *)img->user_priv)->peekull(), work, std::move(block));
    return OK;
}

c2_status_t C2SoftVpxDec::drainInternal(
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

    while (outputBuffer(pool, work) == OK) {
    }

    if (drainMode == DRAIN_COMPONENT_WITH_EOS &&
            work && work->workletsProcessed == 0u) {
        fillEmptyWork(work);
    }

    return C2_OK;
}
c2_status_t C2SoftVpxDec::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    return drainInternal(drainMode, pool, nullptr);
}

class C2SoftVpxFactory : public C2ComponentFactory {
public:
    C2SoftVpxFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
        GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftVpxDec(COMPONENT_NAME, id,
                          std::make_shared<C2SoftVpxDec::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftVpxDec::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftVpxDec::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual ~C2SoftVpxFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftVpxFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
