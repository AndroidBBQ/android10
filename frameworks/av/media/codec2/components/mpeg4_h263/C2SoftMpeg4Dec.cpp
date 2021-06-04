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
#ifdef MPEG4
  #define LOG_TAG "C2SoftMpeg4Dec"
#else
  #define LOG_TAG "C2SoftH263Dec"
#endif
#include <log/log.h>

#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>

#include "C2SoftMpeg4Dec.h"
#include "mp4dec_api.h"

namespace android {

#ifdef MPEG4
constexpr char COMPONENT_NAME[] = "c2.android.mpeg4.decoder";
#else
constexpr char COMPONENT_NAME[] = "c2.android.h263.decoder";
#endif

class C2SoftMpeg4Dec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_VIDEO,
#ifdef MPEG4
                MEDIA_MIMETYPE_VIDEO_MPEG4
#else
                MEDIA_MIMETYPE_VIDEO_H263
#endif
                ) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();

        // TODO: Proper support for reorder depth.
        addParameter(
                DefineParam(mActualOutputDelay, C2_PARAMKEY_OUTPUT_DELAY)
                .withConstValue(new C2PortActualDelayTuning::output(1u))
                .build());

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::output(0u, 176, 144))
                .withFields({
#ifdef MPEG4
                    C2F(mSize, width).inRange(2, 1920, 2),
                    C2F(mSize, height).inRange(2, 1088, 2),
#else
                    C2F(mSize, width).inRange(2, 352, 2),
                    C2F(mSize, height).inRange(2, 288, 2),
#endif
                })
                .withSetter(SizeSetter)
                .build());

#ifdef MPEG4
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_MP4V_SIMPLE, C2Config::LEVEL_MP4V_3))
                .withFields({
                    C2F(mProfileLevel, profile).equalTo(
                            C2Config::PROFILE_MP4V_SIMPLE),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_MP4V_0,
                            C2Config::LEVEL_MP4V_0B,
                            C2Config::LEVEL_MP4V_1,
                            C2Config::LEVEL_MP4V_2,
                            C2Config::LEVEL_MP4V_3,
                            C2Config::LEVEL_MP4V_3B,
                            C2Config::LEVEL_MP4V_4,
                            C2Config::LEVEL_MP4V_4A,
                            C2Config::LEVEL_MP4V_5,
                            C2Config::LEVEL_MP4V_6})
                })
                .withSetter(ProfileLevelSetter, mSize)
                .build());
#else
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_H263_BASELINE, C2Config::LEVEL_H263_30))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                            C2Config::PROFILE_H263_BASELINE,
                            C2Config::PROFILE_H263_ISWV2}),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_H263_10,
                            C2Config::LEVEL_H263_20,
                            C2Config::LEVEL_H263_30,
                            C2Config::LEVEL_H263_40,
                            C2Config::LEVEL_H263_45})
                })
                .withSetter(ProfileLevelSetter, mSize)
                .build());
#endif

        addParameter(
                DefineParam(mMaxSize, C2_PARAMKEY_MAX_PICTURE_SIZE)
#ifdef MPEG4
                .withDefault(new C2StreamMaxPictureSizeTuning::output(0u, 1920, 1088))
#else
                .withDefault(new C2StreamMaxPictureSizeTuning::output(0u, 352, 288))
#endif
                .withFields({
#ifdef MPEG4
                    C2F(mSize, width).inRange(2, 1920, 2),
                    C2F(mSize, height).inRange(2, 1088, 2),
#else
                    C2F(mSize, width).inRange(2, 352, 2),
                    C2F(mSize, height).inRange(2, 288, 2),
#endif
                })
                .withSetter(MaxPictureSizeSetter, mSize)
                .build());

        addParameter(
                DefineParam(mMaxInputSize, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE)
#ifdef MPEG4
                .withDefault(new C2StreamMaxBufferSizeInfo::input(0u, 1920 * 1088 * 3 / 2))
#else
                .withDefault(new C2StreamMaxBufferSizeInfo::input(0u, 352 * 288 * 3 / 2))
#endif
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
#ifdef MPEG4
        me.set().width = c2_min(c2_max(me.v.width, size.v.width), 1920u);
        me.set().height = c2_min(c2_max(me.v.height, size.v.height), 1088u);
#else
        me.set().width = c2_min(c2_max(me.v.width, size.v.width), 352u);
        me.set().height = c2_min(c2_max(me.v.height, size.v.height), 288u);
#endif
        return C2R::Ok();
    }

    static C2R MaxInputSizeSetter(bool mayBlock, C2P<C2StreamMaxBufferSizeInfo::input> &me,
                                  const C2P<C2StreamMaxPictureSizeTuning::output> &maxSize) {
        (void)mayBlock;
        // assume compression ratio of 1
        me.set().value = (((maxSize.v.width + 15) / 16) * ((maxSize.v.height + 15) / 16) * 384);
        return C2R::Ok();
    }

    static C2R ProfileLevelSetter(bool mayBlock, C2P<C2StreamProfileLevelInfo::input> &me,
                                  const C2P<C2StreamPictureSizeInfo::output> &size) {
        (void)mayBlock;
        (void)size;
        (void)me;  // TODO: validate
        return C2R::Ok();
    }

    uint32_t getMaxWidth() const { return mMaxSize->width; }
    uint32_t getMaxHeight() const { return mMaxSize->height; }

private:
    std::shared_ptr<C2StreamProfileLevelInfo::input> mProfileLevel;
    std::shared_ptr<C2StreamPictureSizeInfo::output> mSize;
    std::shared_ptr<C2StreamMaxPictureSizeTuning::output> mMaxSize;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mMaxInputSize;
    std::shared_ptr<C2StreamColorInfo::output> mColorInfo;
    std::shared_ptr<C2StreamPixelFormatInfo::output> mPixelFormat;
};

C2SoftMpeg4Dec::C2SoftMpeg4Dec(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mDecHandle(nullptr),
      mOutputBuffer{},
      mInitialized(false) {
}

C2SoftMpeg4Dec::~C2SoftMpeg4Dec() {
    onRelease();
}

c2_status_t C2SoftMpeg4Dec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

c2_status_t C2SoftMpeg4Dec::onStop() {
    if (mInitialized) {
        if (mDecHandle) {
            PVCleanUpVideoDecoder(mDecHandle);
        }
        mInitialized = false;
    }
    for (int32_t i = 0; i < kNumOutputBuffers; ++i) {
        if (mOutputBuffer[i]) {
            free(mOutputBuffer[i]);
            mOutputBuffer[i] = nullptr;
        }
    }
    mNumSamplesOutput = 0;
    mFramesConfigured = false;
    mSignalledOutputEos = false;
    mSignalledError = false;

    return C2_OK;
}

void C2SoftMpeg4Dec::onReset() {
    (void)onStop();
    (void)onInit();
}

void C2SoftMpeg4Dec::onRelease() {
    if (mInitialized) {
        if (mDecHandle) {
            PVCleanUpVideoDecoder(mDecHandle);
            delete mDecHandle;
            mDecHandle = nullptr;
        }
        mInitialized = false;
    }
    if (mOutBlock) {
        mOutBlock.reset();
    }
    for (int32_t i = 0; i < kNumOutputBuffers; ++i) {
        if (mOutputBuffer[i]) {
            free(mOutputBuffer[i]);
            mOutputBuffer[i] = nullptr;
        }
    }
}

c2_status_t C2SoftMpeg4Dec::onFlush_sm() {
    if (mInitialized) {
        if (PV_TRUE != PVResetVideoDecoder(mDecHandle)) {
            return C2_CORRUPTED;
        }
    }
    mSignalledOutputEos = false;
    mSignalledError = false;
    return C2_OK;
}

status_t C2SoftMpeg4Dec::initDecoder() {
#ifdef MPEG4
    mIsMpeg4 = true;
#else
    mIsMpeg4 = false;
#endif
    if (!mDecHandle) {
        mDecHandle = new tagvideoDecControls;
    }
    if (!mDecHandle) {
        ALOGE("mDecHandle is null");
        return NO_MEMORY;
    }
    memset(mDecHandle, 0, sizeof(tagvideoDecControls));

    /* TODO: bring these values to 352 and 288. It cannot be done as of now
     * because, h263 doesn't seem to allow port reconfiguration. In OMX, the
     * problem of larger width and height than default width and height is
     * overcome by adaptivePlayBack() api call. This call gets width and height
     * information from extractor. Such a thing is not possible here.
     * So we are configuring to larger values.*/
    mWidth = 1408;
    mHeight = 1152;
    mNumSamplesOutput = 0;
    mInitialized = false;
    mFramesConfigured = false;
    mSignalledOutputEos = false;
    mSignalledError = false;

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

void C2SoftMpeg4Dec::finishWork(uint64_t index, const std::unique_ptr<C2Work> &work) {
    std::shared_ptr<C2Buffer> buffer = createGraphicBuffer(std::move(mOutBlock),
                                                           C2Rect(mWidth, mHeight));
    mOutBlock = nullptr;
    auto fillWork = [buffer, index](const std::unique_ptr<C2Work> &work) {
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
    };
    if (work && c2_cntr64_t(index) == work->input.ordinal.frameIndex) {
        fillWork(work);
    } else {
        finish(index, fillWork);
    }
}

c2_status_t C2SoftMpeg4Dec::ensureDecoderState(const std::shared_ptr<C2BlockPool> &pool) {
    if (!mDecHandle) {
        ALOGE("not supposed to be here, invalid decoder context");
        return C2_CORRUPTED;
    }

    mOutputBufferSize = align(mIntf->getMaxWidth(), 16) * align(mIntf->getMaxHeight(), 16) * 3 / 2;
    for (int32_t i = 0; i < kNumOutputBuffers; ++i) {
        if (!mOutputBuffer[i]) {
            mOutputBuffer[i] = (uint8_t *)malloc(mOutputBufferSize);
            if (!mOutputBuffer[i]) {
                return C2_NO_MEMORY;
            }
        }
    }
    if (mOutBlock &&
            (mOutBlock->width() != align(mWidth, 16) || mOutBlock->height() != mHeight)) {
        mOutBlock.reset();
    }
    if (!mOutBlock) {
        uint32_t format = HAL_PIXEL_FORMAT_YV12;
        C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
        c2_status_t err = pool->fetchGraphicBlock(align(mWidth, 16), mHeight, format, usage, &mOutBlock);
        if (err != C2_OK) {
            ALOGE("fetchGraphicBlock for Output failed with status %d", err);
            return err;
        }
        ALOGV("provided (%dx%d) required (%dx%d)",
              mOutBlock->width(), mOutBlock->height(), mWidth, mHeight);
    }
    return C2_OK;
}

bool C2SoftMpeg4Dec::handleResChange(const std::unique_ptr<C2Work> &work) {
    uint32_t disp_width, disp_height;
    PVGetVideoDimensions(mDecHandle, (int32 *)&disp_width, (int32 *)&disp_height);

    uint32_t buf_width, buf_height;
    PVGetBufferDimensions(mDecHandle, (int32 *)&buf_width, (int32 *)&buf_height);

    CHECK_LE(disp_width, buf_width);
    CHECK_LE(disp_height, buf_height);

    ALOGV("display size (%dx%d), buffer size (%dx%d)",
           disp_width, disp_height, buf_width, buf_height);

    bool resChanged = false;
    if (disp_width != mWidth || disp_height != mHeight) {
        mWidth = disp_width;
        mHeight = disp_height;
        resChanged = true;
        for (int32_t i = 0; i < kNumOutputBuffers; ++i) {
            if (mOutputBuffer[i]) {
                free(mOutputBuffer[i]);
                mOutputBuffer[i] = nullptr;
            }
        }

        if (!mIsMpeg4) {
            PVCleanUpVideoDecoder(mDecHandle);

            uint8_t *vol_data[1]{};
            int32_t vol_size = 0;

            if (!PVInitVideoDecoder(
                    mDecHandle, vol_data, &vol_size, 1, mIntf->getMaxWidth(), mIntf->getMaxHeight(), H263_MODE)) {
                ALOGE("Error in PVInitVideoDecoder H263_MODE while resChanged was set to true");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return true;
            }
        }
        mFramesConfigured = false;
    }
    return resChanged;
}

/* TODO: can remove temporary copy after library supports writing to display
 * buffer Y, U and V plane pointers using stride info. */
static void copyOutputBufferToYuvPlanarFrame(
        uint8_t *dst, uint8_t *src,
        size_t dstYStride, size_t dstUVStride,
        size_t srcYStride, uint32_t width,
        uint32_t height) {
    size_t srcUVStride = srcYStride / 2;
    uint8_t *srcStart = src;
    uint8_t *dstStart = dst;
    size_t vStride = align(height, 16);
    for (size_t i = 0; i < height; ++i) {
         memcpy(dst, src, width);
         src += srcYStride;
         dst += dstYStride;
    }
    /* U buffer */
    src = srcStart + vStride * srcYStride;
    dst = dstStart + (dstYStride * height) + (dstUVStride * height / 2);
    for (size_t i = 0; i < height / 2; ++i) {
         memcpy(dst, src, width / 2);
         src += srcUVStride;
         dst += dstUVStride;
    }
    /* V buffer */
    src = srcStart + vStride * srcYStride * 5 / 4;
    dst = dstStart + (dstYStride * height);
    for (size_t i = 0; i < height / 2; ++i) {
         memcpy(dst, src, width / 2);
         src += srcUVStride;
         dst += dstUVStride;
    }
}

void C2SoftMpeg4Dec::process(
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

    size_t inOffset = 0u;
    size_t inSize = 0u;
    uint32_t workIndex = work->input.ordinal.frameIndex.peeku() & 0xFFFFFFFF;
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
    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d, flags %x",
          inSize, (int)work->input.ordinal.timestamp.peeku(),
          (int)work->input.ordinal.frameIndex.peeku(), work->input.flags);

    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    if (inSize == 0) {
        fillEmptyWork(work);
        if (eos) {
            mSignalledOutputEos = true;
        }
        return;
    }

    uint8_t *bitstream = const_cast<uint8_t *>(rView.data() + inOffset);
    uint32_t *start_code = (uint32_t *)bitstream;
    bool volHeader = *start_code == 0xB0010000;
    if (volHeader) {
        PVCleanUpVideoDecoder(mDecHandle);
        mInitialized = false;
    }

    if (!mInitialized) {
        uint8_t *vol_data[1]{};
        int32_t vol_size = 0;

        bool codecConfig = (work->input.flags & C2FrameData::FLAG_CODEC_CONFIG) != 0;
        if (codecConfig || volHeader) {
            vol_data[0] = bitstream;
            vol_size = inSize;
        }
        MP4DecodingMode mode = (mIsMpeg4) ? MPEG4_MODE : H263_MODE;
        if (!PVInitVideoDecoder(
                mDecHandle, vol_data, &vol_size, 1,
                mIntf->getMaxWidth(), mIntf->getMaxHeight(), mode)) {
            ALOGE("PVInitVideoDecoder failed. Unsupported content?");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        mInitialized = true;
        MP4DecodingMode actualMode = PVGetDecBitstreamMode(mDecHandle);
        if (mode != actualMode) {
            ALOGE("Decoded mode not same as actual mode of the decoder");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }

        PVSetPostProcType(mDecHandle, 0);
        if (handleResChange(work)) {
            ALOGI("Setting width and height");
            C2StreamPictureSizeInfo::output size(0u, mWidth, mHeight);
            std::vector<std::unique_ptr<C2SettingResult>> failures;
            c2_status_t err = mIntf->config({&size}, C2_MAY_BLOCK, &failures);
            if (err == OK) {
                work->worklets.front()->output.configUpdate.push_back(
                    C2Param::Copy(size));
            } else {
                ALOGE("Config update size failed");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
        }
        if (codecConfig) {
            fillEmptyWork(work);
            return;
        }
    }

    size_t inPos = 0;
    while (inPos < inSize) {
        c2_status_t err = ensureDecoderState(pool);
        if (C2_OK != err) {
            mSignalledError = true;
            work->result = err;
            return;
        }
        C2GraphicView wView = mOutBlock->map().get();
        if (wView.error()) {
            ALOGE("graphic view map failed %d", wView.error());
            work->result = C2_CORRUPTED;
            return;
        }

        uint32_t yFrameSize = sizeof(uint8) * mDecHandle->size;
        if (mOutputBufferSize < yFrameSize * 3 / 2){
            ALOGE("Too small output buffer: %zu bytes", mOutputBufferSize);
            mSignalledError = true;
            work->result = C2_NO_MEMORY;
            return;
        }

        if (!mFramesConfigured) {
            PVSetReferenceYUV(mDecHandle,mOutputBuffer[1]);
            mFramesConfigured = true;
        }

        // Need to check if header contains new info, e.g., width/height, etc.
        VopHeaderInfo header_info;
        uint32_t useExtTimestamp = (inPos == 0);
        int32_t tmpInSize = (int32_t)inSize;
        uint8_t *bitstreamTmp = bitstream;
        uint32_t timestamp = workIndex;
        if (PVDecodeVopHeader(
                    mDecHandle, &bitstreamTmp, &timestamp, &tmpInSize,
                    &header_info, &useExtTimestamp,
                    mOutputBuffer[mNumSamplesOutput & 1]) != PV_TRUE) {
            ALOGE("failed to decode vop header.");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }

        // H263 doesn't have VOL header, the frame size information is in short header, i.e. the
        // decoder may detect size change after PVDecodeVopHeader.
        bool resChange = handleResChange(work);
        if (mIsMpeg4 && resChange) {
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        } else if (resChange) {
            ALOGI("Setting width and height");
            C2StreamPictureSizeInfo::output size(0u, mWidth, mHeight);
            std::vector<std::unique_ptr<C2SettingResult>> failures;
            c2_status_t err = mIntf->config({&size}, C2_MAY_BLOCK, &failures);
            if (err == OK) {
                work->worklets.front()->output.configUpdate.push_back(C2Param::Copy(size));
            } else {
                ALOGE("Config update size failed");
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
            continue;
        }

        if (PVDecodeVopBody(mDecHandle, &tmpInSize) != PV_TRUE) {
            ALOGE("failed to decode video frame.");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }
        if (handleResChange(work)) {
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        }

        uint8_t *outputBufferY = wView.data()[C2PlanarLayout::PLANE_Y];
        C2PlanarLayout layout = wView.layout();
        size_t dstYStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
        size_t dstUVStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;
        (void)copyOutputBufferToYuvPlanarFrame(
                outputBufferY,
                mOutputBuffer[mNumSamplesOutput & 1],
                dstYStride, dstUVStride,
                align(mWidth, 16), mWidth, mHeight);

        inPos += inSize - (size_t)tmpInSize;
        finishWork(workIndex, work);
        ++mNumSamplesOutput;
        if (inSize - inPos != 0) {
            ALOGD("decoded frame, ignoring further trailing bytes %d",
                  (int)inSize - (int)inPos);
            break;
        }
    }
}

c2_status_t C2SoftMpeg4Dec::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
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

class C2SoftMpeg4DecFactory : public C2ComponentFactory {
public:
    C2SoftMpeg4DecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
        GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftMpeg4Dec(COMPONENT_NAME,
                                   id,
                                   std::make_shared<C2SoftMpeg4Dec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftMpeg4Dec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftMpeg4Dec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftMpeg4DecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftMpeg4DecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
