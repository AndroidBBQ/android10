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
#define LOG_TAG "C2SoftAomDec"
#include <log/log.h>

#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftAomDec.h"

namespace android {

// codecname set and passed in as a compile flag from Android.bp
constexpr char COMPONENT_NAME[] = CODECNAME;

class C2SoftAomDec::IntfImpl : public SimpleInterface<void>::BaseParams {
  public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper>& helper)
        : SimpleInterface<void>::BaseParams(
              helper, COMPONENT_NAME, C2Component::KIND_DECODER,
              C2Component::DOMAIN_VIDEO, MEDIA_MIMETYPE_VIDEO_AV1) {
        noPrivateBuffers();  // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();

        addParameter(DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                         .withConstValue(new C2ComponentAttributesSetting(
                             C2Component::ATTRIB_IS_TEMPORAL))
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

        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_AV1_0, C2Config::LEVEL_AV1_2_1))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                            C2Config::PROFILE_AV1_0,
                            C2Config::PROFILE_AV1_1}),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_AV1_2,
                            C2Config::LEVEL_AV1_2_1,
                            C2Config::LEVEL_AV1_2_2,
                            C2Config::LEVEL_AV1_3,
                            C2Config::LEVEL_AV1_3_1,
                            C2Config::LEVEL_AV1_3_2,
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

        addParameter(DefineParam(mMaxSize, C2_PARAMKEY_MAX_PICTURE_SIZE)
                         .withDefault(new C2StreamMaxPictureSizeTuning::output(
                             0u, 320, 240))
                         .withFields({
                             C2F(mSize, width).inRange(2, 2048, 2),
                             C2F(mSize, height).inRange(2, 2048, 2),
                         })
                         .withSetter(MaxPictureSizeSetter, mSize)
                         .build());

        addParameter(
            DefineParam(mMaxInputSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
                .withDefault(
                    new C2StreamMaxBufferSizeInfo::input(0u, 320 * 240 * 3 / 4))
                .withFields({
                    C2F(mMaxInputSize, value).any(),
                })
                .calculatedAs(MaxInputSizeSetter, mMaxSize)
                .build());

        C2ChromaOffsetStruct locations[1] = {
            C2ChromaOffsetStruct::ITU_YUV_420_0()};
        std::shared_ptr<C2StreamColorInfo::output> defaultColorInfo =
            C2StreamColorInfo::output::AllocShared(1u, 0u, 8u /* bitDepth */,
                                                   C2Color::YUV_420);
        memcpy(defaultColorInfo->m.locations, locations, sizeof(locations));

        defaultColorInfo = C2StreamColorInfo::output::AllocShared(
            {C2ChromaOffsetStruct::ITU_YUV_420_0()}, 0u, 8u /* bitDepth */,
            C2Color::YUV_420);
        helper->addStructDescriptors<C2ChromaOffsetStruct>();

        addParameter(DefineParam(mColorInfo, C2_PARAMKEY_CODED_COLOR_INFO)
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
        addParameter(DefineParam(mPixelFormat, C2_PARAMKEY_PIXEL_FORMAT)
                         .withConstValue(new C2StreamPixelFormatInfo::output(
                             0u, HAL_PIXEL_FORMAT_YCBCR_420_888))
                         .build());
    }

    static C2R SizeSetter(bool mayBlock,
                          const C2P<C2StreamPictureSizeInfo::output>& oldMe,
                          C2P<C2StreamPictureSizeInfo::output>& me) {
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

    static C2R MaxPictureSizeSetter(
        bool mayBlock, C2P<C2StreamMaxPictureSizeTuning::output>& me,
        const C2P<C2StreamPictureSizeInfo::output>& size) {
        (void)mayBlock;
        // TODO: get max width/height from the size's field helpers vs.
        // hardcoding
        me.set().width = c2_min(c2_max(me.v.width, size.v.width), 4096u);
        me.set().height = c2_min(c2_max(me.v.height, size.v.height), 4096u);
        return C2R::Ok();
    }

    static C2R MaxInputSizeSetter(
        bool mayBlock, C2P<C2StreamMaxBufferSizeInfo::input>& me,
        const C2P<C2StreamMaxPictureSizeTuning::output>& maxSize) {
        (void)mayBlock;
        // assume compression ratio of 2
        me.set().value = (((maxSize.v.width + 63) / 64) *
                          ((maxSize.v.height + 63) / 64) * 3072);
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
    std::shared_ptr<C2StreamHdr10PlusInfo::input> mHdr10PlusInfoInput;
    std::shared_ptr<C2StreamHdr10PlusInfo::output> mHdr10PlusInfoOutput;
};

C2SoftAomDec::C2SoftAomDec(const char* name, c2_node_id_t id,
                           const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
          std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mCodecCtx(nullptr){

    GENERATE_FILE_NAMES();
    CREATE_DUMP_FILE(mInFile);
    CREATE_DUMP_FILE(mOutFile);

    gettimeofday(&mTimeStart, nullptr);
    gettimeofday(&mTimeEnd, nullptr);
}

C2SoftAomDec::~C2SoftAomDec() {
    onRelease();
}

c2_status_t C2SoftAomDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

c2_status_t C2SoftAomDec::onStop() {
    mSignalledError = false;
    mSignalledOutputEos = false;
    return C2_OK;
}

void C2SoftAomDec::onReset() {
    (void)onStop();
    c2_status_t err = onFlush_sm();
    if (err != C2_OK) {
        ALOGW("Failed to flush decoder. Try to hard reset decoder.");
        destroyDecoder();
        (void)initDecoder();
    }
}

void C2SoftAomDec::onRelease() {
    destroyDecoder();
}

c2_status_t C2SoftAomDec::onFlush_sm() {
    if (aom_codec_decode(mCodecCtx, nullptr, 0, nullptr)) {
        ALOGE("Failed to flush av1 decoder.");
        return C2_CORRUPTED;
    }

    aom_codec_iter_t iter = nullptr;
    while (aom_codec_get_frame(mCodecCtx, &iter)) {
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

status_t C2SoftAomDec::initDecoder() {
    mSignalledError = false;
    mSignalledOutputEos = false;
    if (!mCodecCtx) {
        mCodecCtx = new aom_codec_ctx_t;
    }

    if (!mCodecCtx) {
        ALOGE("mCodecCtx is null");
        return NO_MEMORY;
    }

    aom_codec_dec_cfg_t cfg;
    memset(&cfg, 0, sizeof(aom_codec_dec_cfg_t));
    cfg.threads = GetCPUCoreCount();
    cfg.allow_lowbitdepth = 1;

    aom_codec_flags_t flags;
    memset(&flags, 0, sizeof(aom_codec_flags_t));

    ALOGV("Using libaom AV1 software decoder.");
    aom_codec_err_t err;
    if ((err = aom_codec_dec_init(mCodecCtx, aom_codec_av1_dx(), &cfg, 0))) {
        ALOGE("av1 decoder failed to initialize. (%d)", err);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t C2SoftAomDec::destroyDecoder() {
    if (mCodecCtx) {
        aom_codec_destroy(mCodecCtx);
        delete mCodecCtx;
        mCodecCtx = nullptr;
    }
    return OK;
}

void fillEmptyWork(const std::unique_ptr<C2Work>& work) {
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

void C2SoftAomDec::finishWork(uint64_t index,
                              const std::unique_ptr<C2Work>& work,
                              const std::shared_ptr<C2GraphicBlock>& block) {
    std::shared_ptr<C2Buffer> buffer =
        createGraphicBuffer(block, C2Rect(mWidth, mHeight));
    auto fillWork = [buffer, index, intf = this->mIntf](
            const std::unique_ptr<C2Work>& work) {
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

void C2SoftAomDec::process(const std::unique_ptr<C2Work>& work,
                           const std::shared_ptr<C2BlockPool>& pool) {
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
        rView =
            work->input.buffers[0]->data().linearBlocks().front().map().get();
        inSize = rView.capacity();
        if (inSize && rView.error()) {
            ALOGE("read view map failed %d", rView.error());
            work->result = C2_CORRUPTED;
            return;
        }
    }

    bool codecConfig =
        ((work->input.flags & C2FrameData::FLAG_CODEC_CONFIG) != 0);
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);

    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d, flags %x",
          inSize, (int)work->input.ordinal.timestamp.peeku(),
          (int)work->input.ordinal.frameIndex.peeku(), work->input.flags);

    if (codecConfig) {
        fillEmptyWork(work);
        return;
    }

    int64_t frameIndex = work->input.ordinal.frameIndex.peekll();
    if (inSize) {
        uint8_t* bitstream = const_cast<uint8_t*>(rView.data() + inOffset);
        int32_t decodeTime = 0;
        int32_t delay = 0;

        DUMP_TO_FILE(mOutFile, bitstream, inSize);
        GETTIME(&mTimeStart, nullptr);
        TIME_DIFF(mTimeEnd, mTimeStart, delay);

        aom_codec_err_t err =
            aom_codec_decode(mCodecCtx, bitstream, inSize, &frameIndex);

        GETTIME(&mTimeEnd, nullptr);
        TIME_DIFF(mTimeStart, mTimeEnd, decodeTime);
        ALOGV("decodeTime=%4d delay=%4d\n", decodeTime, delay);

        if (err != AOM_CODEC_OK) {
            ALOGE("av1 decoder failed to decode frame err: %d", err);
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            mSignalledError = true;
            return;
        }

    } else {
        if (aom_codec_decode(mCodecCtx, nullptr, 0, nullptr)) {
            ALOGE("Failed to flush av1 decoder.");
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            mSignalledError = true;
            return;
        }
    }

    (void)outputBuffer(pool, work);

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
    uint8_t* dstStart = dst;

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
bool C2SoftAomDec::outputBuffer(
        const std::shared_ptr<C2BlockPool> &pool,
        const std::unique_ptr<C2Work> &work)
{
    if (!(work && pool)) return false;

    aom_codec_iter_t iter = nullptr;
    aom_image_t* img = aom_codec_get_frame(mCodecCtx, &iter);

    if (!img) return false;

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
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return false;
        }
    }

    CHECK(img->fmt == AOM_IMG_FMT_I420 || img->fmt == AOM_IMG_FMT_I42016);

    std::shared_ptr<C2GraphicBlock> block;
    uint32_t format = HAL_PIXEL_FORMAT_YV12;
    if (img->fmt == AOM_IMG_FMT_I42016) {
        IntfImpl::Lock lock = mIntf->lock();
        std::shared_ptr<C2StreamColorAspectsTuning::output> defaultColorAspects = mIntf->getDefaultColorAspects_l();

        if (defaultColorAspects->primaries == C2Color::PRIMARIES_BT2020 &&
            defaultColorAspects->matrix == C2Color::MATRIX_BT2020 &&
            defaultColorAspects->transfer == C2Color::TRANSFER_ST2084) {
            format = HAL_PIXEL_FORMAT_RGBA_1010102;
        }
    }
    C2MemoryUsage usage = {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE};

    c2_status_t err = pool->fetchGraphicBlock(align(mWidth, 16), mHeight,
                                              format, usage, &block);

    if (err != C2_OK) {
        ALOGE("fetchGraphicBlock for Output failed with status %d", err);
        work->result = err;
        return false;
    }

    C2GraphicView wView = block->map().get();

    if (wView.error()) {
        ALOGE("graphic view map failed %d", wView.error());
        work->result = C2_CORRUPTED;
        return false;
    }

    ALOGV("provided (%dx%d) required (%dx%d), out frameindex %d",
          block->width(), block->height(), mWidth, mHeight,
          (int)*(int64_t*)img->user_priv);

    uint8_t* dst = const_cast<uint8_t*>(wView.data()[C2PlanarLayout::PLANE_Y]);
    size_t srcYStride = img->stride[AOM_PLANE_Y];
    size_t srcUStride = img->stride[AOM_PLANE_U];
    size_t srcVStride = img->stride[AOM_PLANE_V];
    C2PlanarLayout layout = wView.layout();
    size_t dstYStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
    size_t dstUVStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;

    if (img->fmt == AOM_IMG_FMT_I42016) {
        const uint16_t *srcY = (const uint16_t *)img->planes[AOM_PLANE_Y];
        const uint16_t *srcU = (const uint16_t *)img->planes[AOM_PLANE_U];
        const uint16_t *srcV = (const uint16_t *)img->planes[AOM_PLANE_V];

        if (format == HAL_PIXEL_FORMAT_RGBA_1010102) {
            convertYUV420Planar16ToY410((uint32_t *)dst, srcY, srcU, srcV, srcYStride / 2,
                                    srcUStride / 2, srcVStride / 2,
                                    dstYStride / sizeof(uint32_t),
                                    mWidth, mHeight);
        } else {
            convertYUV420Planar16ToYUV420Planar(dst, srcY, srcU, srcV, srcYStride / 2,
                                    srcUStride / 2, srcVStride / 2,
                                    dstYStride, dstUVStride,
                                    mWidth, mHeight);
        }
    } else {
        const uint8_t *srcY = (const uint8_t *)img->planes[AOM_PLANE_Y];
        const uint8_t *srcU = (const uint8_t *)img->planes[AOM_PLANE_U];
        const uint8_t *srcV = (const uint8_t *)img->planes[AOM_PLANE_V];
        copyOutputBufferToYuvPlanarFrame(
                dst, srcY, srcU, srcV,
                srcYStride, srcUStride, srcVStride,
                dstYStride, dstUVStride,
                mWidth, mHeight);
    }
    finishWork(*(int64_t*)img->user_priv, work, std::move(block));
    block = nullptr;
    return true;
}

c2_status_t C2SoftAomDec::drainInternal(
    uint32_t drainMode, const std::shared_ptr<C2BlockPool>& pool,
    const std::unique_ptr<C2Work>& work) {
    if (drainMode == NO_DRAIN) {
        ALOGW("drain with NO_DRAIN: no-op");
        return C2_OK;
    }
    if (drainMode == DRAIN_CHAIN) {
        ALOGW("DRAIN_CHAIN not supported");
        return C2_OMITTED;
    }

    if (aom_codec_decode(mCodecCtx, nullptr, 0, nullptr)) {
        ALOGE("Failed to flush av1 decoder.");
        return C2_CORRUPTED;
    }

    while ((outputBuffer(pool, work))) {
    }

    if (drainMode == DRAIN_COMPONENT_WITH_EOS && work &&
        work->workletsProcessed == 0u) {
        fillEmptyWork(work);
    }

    return C2_OK;
}

c2_status_t C2SoftAomDec::drain(uint32_t drainMode,
                                const std::shared_ptr<C2BlockPool>& pool) {
    return drainInternal(drainMode, pool, nullptr);
}

class C2SoftAomFactory : public C2ComponentFactory {
   public:
    C2SoftAomFactory()
        : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
              GetCodec2PlatformComponentStore()->getParamReflector())) {}

    virtual c2_status_t createComponent(
        c2_node_id_t id, std::shared_ptr<C2Component>* const component,
        std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftAomDec(COMPONENT_NAME, id,
                             std::make_shared<C2SoftAomDec::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
        c2_node_id_t id, std::shared_ptr<C2ComponentInterface>* const interface,
        std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftAomDec::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftAomDec::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual ~C2SoftAomFactory() override = default;

   private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftAomFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
