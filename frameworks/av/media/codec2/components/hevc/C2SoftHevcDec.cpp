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
#define LOG_TAG "C2SoftHevcDec"
#include <log/log.h>

#include <media/stagefright/foundation/MediaDefs.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <Codec2Mapper.h>
#include <SimpleC2Interface.h>

#include "C2SoftHevcDec.h"
#include "ihevcd_cxa.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.hevc.decoder";
constexpr uint32_t kDefaultOutputDelay = 8;
constexpr uint32_t kMaxOutputDelay = 16;
}  // namespace

class C2SoftHevcDec::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_DECODER,
                C2Component::DOMAIN_VIDEO,
                MEDIA_MIMETYPE_VIDEO_HEVC) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();

        // TODO: Proper support for reorder depth.
        addParameter(
                DefineParam(mActualOutputDelay, C2_PARAMKEY_OUTPUT_DELAY)
                .withDefault(new C2PortActualDelayTuning::output(kDefaultOutputDelay))
                .withFields({C2F(mActualOutputDelay, value).inRange(0, kMaxOutputDelay)})
                .withSetter(Setter<decltype(*mActualOutputDelay)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::output(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 4096, 2),
                    C2F(mSize, height).inRange(2, 4096, 2),
                })
                .withSetter(SizeSetter)
                .build());

        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::input(0u,
                        C2Config::PROFILE_HEVC_MAIN, C2Config::LEVEL_HEVC_MAIN_5_1))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                            C2Config::PROFILE_HEVC_MAIN,
                            C2Config::PROFILE_HEVC_MAIN_STILL}),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_HEVC_MAIN_1,
                            C2Config::LEVEL_HEVC_MAIN_2, C2Config::LEVEL_HEVC_MAIN_2_1,
                            C2Config::LEVEL_HEVC_MAIN_3, C2Config::LEVEL_HEVC_MAIN_3_1,
                            C2Config::LEVEL_HEVC_MAIN_4, C2Config::LEVEL_HEVC_MAIN_4_1,
                            C2Config::LEVEL_HEVC_MAIN_5, C2Config::LEVEL_HEVC_MAIN_5_1,
                            C2Config::LEVEL_HEVC_MAIN_5_2, C2Config::LEVEL_HEVC_HIGH_4,
                            C2Config::LEVEL_HEVC_HIGH_4_1, C2Config::LEVEL_HEVC_HIGH_5,
                            C2Config::LEVEL_HEVC_HIGH_5_1, C2Config::LEVEL_HEVC_HIGH_5_2
                    })
                })
                .withSetter(ProfileLevelSetter, mSize)
                .build());

        addParameter(
                DefineParam(mMaxSize, C2_PARAMKEY_MAX_PICTURE_SIZE)
                .withDefault(new C2StreamMaxPictureSizeTuning::output(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 4096, 2),
                    C2F(mSize, height).inRange(2, 4096, 2),
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

        defaultColorInfo = C2StreamColorInfo::output::AllocShared(
            {C2ChromaOffsetStruct::ITU_YUV_420_0()}, 0u, 8u /* bitDepth */,
            C2Color::YUV_420);
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

        addParameter(
                DefineParam(mCodedColorAspects, C2_PARAMKEY_VUI_COLOR_ASPECTS)
                .withDefault(new C2StreamColorAspectsInfo::input(
                        0u, C2Color::RANGE_LIMITED, C2Color::PRIMARIES_UNSPECIFIED,
                        C2Color::TRANSFER_UNSPECIFIED, C2Color::MATRIX_UNSPECIFIED))
                .withFields({
                    C2F(mCodedColorAspects, range).inRange(
                                C2Color::RANGE_UNSPECIFIED,     C2Color::RANGE_OTHER),
                    C2F(mCodedColorAspects, primaries).inRange(
                                C2Color::PRIMARIES_UNSPECIFIED, C2Color::PRIMARIES_OTHER),
                    C2F(mCodedColorAspects, transfer).inRange(
                                C2Color::TRANSFER_UNSPECIFIED,  C2Color::TRANSFER_OTHER),
                    C2F(mCodedColorAspects, matrix).inRange(
                                C2Color::MATRIX_UNSPECIFIED,    C2Color::MATRIX_OTHER)
                })
                .withSetter(CodedColorAspectsSetter)
                .build());

        addParameter(
                DefineParam(mColorAspects, C2_PARAMKEY_COLOR_ASPECTS)
                .withDefault(new C2StreamColorAspectsInfo::output(
                        0u, C2Color::RANGE_UNSPECIFIED, C2Color::PRIMARIES_UNSPECIFIED,
                        C2Color::TRANSFER_UNSPECIFIED, C2Color::MATRIX_UNSPECIFIED))
                .withFields({
                    C2F(mColorAspects, range).inRange(
                                C2Color::RANGE_UNSPECIFIED,     C2Color::RANGE_OTHER),
                    C2F(mColorAspects, primaries).inRange(
                                C2Color::PRIMARIES_UNSPECIFIED, C2Color::PRIMARIES_OTHER),
                    C2F(mColorAspects, transfer).inRange(
                                C2Color::TRANSFER_UNSPECIFIED,  C2Color::TRANSFER_OTHER),
                    C2F(mColorAspects, matrix).inRange(
                                C2Color::MATRIX_UNSPECIFIED,    C2Color::MATRIX_OTHER)
                })
                .withSetter(ColorAspectsSetter, mDefaultColorAspects, mCodedColorAspects)
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
        me.set().width = c2_min(c2_max(me.v.width, size.v.width), 4096u);
        me.set().height = c2_min(c2_max(me.v.height, size.v.height), 4096u);
        return C2R::Ok();
    }

    static C2R MaxInputSizeSetter(bool mayBlock, C2P<C2StreamMaxBufferSizeInfo::input> &me,
                                  const C2P<C2StreamMaxPictureSizeTuning::output> &maxSize) {
        (void)mayBlock;
        // assume compression ratio of 2
        me.set().value = (((maxSize.v.width + 63) / 64) * ((maxSize.v.height + 63) / 64) * 3072);
        return C2R::Ok();
    }

    static C2R ProfileLevelSetter(bool mayBlock, C2P<C2StreamProfileLevelInfo::input> &me,
                                  const C2P<C2StreamPictureSizeInfo::output> &size) {
        (void)mayBlock;
        (void)size;
        (void)me;  // TODO: validate
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

    static C2R CodedColorAspectsSetter(bool mayBlock, C2P<C2StreamColorAspectsInfo::input> &me) {
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

    static C2R ColorAspectsSetter(bool mayBlock, C2P<C2StreamColorAspectsInfo::output> &me,
                                  const C2P<C2StreamColorAspectsTuning::output> &def,
                                  const C2P<C2StreamColorAspectsInfo::input> &coded) {
        (void)mayBlock;
        // take default values for all unspecified fields, and coded values for specified ones
        me.set().range = coded.v.range == RANGE_UNSPECIFIED ? def.v.range : coded.v.range;
        me.set().primaries = coded.v.primaries == PRIMARIES_UNSPECIFIED
                ? def.v.primaries : coded.v.primaries;
        me.set().transfer = coded.v.transfer == TRANSFER_UNSPECIFIED
                ? def.v.transfer : coded.v.transfer;
        me.set().matrix = coded.v.matrix == MATRIX_UNSPECIFIED ? def.v.matrix : coded.v.matrix;
        return C2R::Ok();
    }

    std::shared_ptr<C2StreamColorAspectsInfo::output> getColorAspects_l() {
        return mColorAspects;
    }

private:
    std::shared_ptr<C2StreamProfileLevelInfo::input> mProfileLevel;
    std::shared_ptr<C2StreamPictureSizeInfo::output> mSize;
    std::shared_ptr<C2StreamMaxPictureSizeTuning::output> mMaxSize;
    std::shared_ptr<C2StreamMaxBufferSizeInfo::input> mMaxInputSize;
    std::shared_ptr<C2StreamColorInfo::output> mColorInfo;
    std::shared_ptr<C2StreamColorAspectsInfo::input> mCodedColorAspects;
    std::shared_ptr<C2StreamColorAspectsTuning::output> mDefaultColorAspects;
    std::shared_ptr<C2StreamColorAspectsInfo::output> mColorAspects;
    std::shared_ptr<C2StreamPixelFormatInfo::output> mPixelFormat;
};

static size_t getCpuCoreCount() {
    long cpuCoreCount = 1;
#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif
    CHECK(cpuCoreCount >= 1);
    ALOGV("Number of CPU cores: %ld", cpuCoreCount);
    return (size_t)cpuCoreCount;
}

static void *ivd_aligned_malloc(void *ctxt, WORD32 alignment, WORD32 size) {
    (void) ctxt;
    return memalign(alignment, size);
}

static void ivd_aligned_free(void *ctxt, void *mem) {
    (void) ctxt;
    free(mem);
}

C2SoftHevcDec::C2SoftHevcDec(
        const char *name,
        c2_node_id_t id,
        const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
        mIntf(intfImpl),
        mDecHandle(nullptr),
        mOutBufferFlush(nullptr),
        mIvColorformat(IV_YUV_420P),
        mOutputDelay(kDefaultOutputDelay),
        mWidth(320),
        mHeight(240),
        mHeaderDecoded(false),
        mOutIndex(0u) {
}

C2SoftHevcDec::~C2SoftHevcDec() {
    onRelease();
}

c2_status_t C2SoftHevcDec::onInit() {
    status_t err = initDecoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

c2_status_t C2SoftHevcDec::onStop() {
    if (OK != resetDecoder()) return C2_CORRUPTED;
    resetPlugin();
    return C2_OK;
}

void C2SoftHevcDec::onReset() {
    (void) onStop();
}

void C2SoftHevcDec::onRelease() {
    (void) deleteDecoder();
    if (mOutBufferFlush) {
        ivd_aligned_free(nullptr, mOutBufferFlush);
        mOutBufferFlush = nullptr;
    }
    if (mOutBlock) {
        mOutBlock.reset();
    }
}

c2_status_t C2SoftHevcDec::onFlush_sm() {
    if (OK != setFlushMode()) return C2_CORRUPTED;

    uint32_t displayStride = mStride;
    uint32_t displayHeight = mHeight;
    uint32_t bufferSize = displayStride * displayHeight * 3 / 2;
    mOutBufferFlush = (uint8_t *)ivd_aligned_malloc(nullptr, 128, bufferSize);
    if (!mOutBufferFlush) {
        ALOGE("could not allocate tmp output buffer (for flush) of size %u ", bufferSize);
        return C2_NO_MEMORY;
    }

    while (true) {
        ivd_video_decode_ip_t s_decode_ip;
        ivd_video_decode_op_t s_decode_op;

        setDecodeArgs(&s_decode_ip, &s_decode_op, nullptr, nullptr, 0, 0, 0);
        (void) ivdec_api_function(mDecHandle, &s_decode_ip, &s_decode_op);
        if (0 == s_decode_op.u4_output_present) {
            resetPlugin();
            break;
        }
    }

    if (mOutBufferFlush) {
        ivd_aligned_free(nullptr, mOutBufferFlush);
        mOutBufferFlush = nullptr;
    }

    return C2_OK;
}

status_t C2SoftHevcDec::createDecoder() {
    ivdext_create_ip_t s_create_ip;
    ivdext_create_op_t s_create_op;

    s_create_ip.s_ivd_create_ip_t.u4_size = sizeof(ivdext_create_ip_t);
    s_create_ip.s_ivd_create_ip_t.e_cmd = IVD_CMD_CREATE;
    s_create_ip.s_ivd_create_ip_t.u4_share_disp_buf = 0;
    s_create_ip.s_ivd_create_ip_t.e_output_format = mIvColorformat;
    s_create_ip.s_ivd_create_ip_t.pf_aligned_alloc = ivd_aligned_malloc;
    s_create_ip.s_ivd_create_ip_t.pf_aligned_free = ivd_aligned_free;
    s_create_ip.s_ivd_create_ip_t.pv_mem_ctxt = nullptr;
    s_create_op.s_ivd_create_op_t.u4_size = sizeof(ivdext_create_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_create_ip,
                                                     &s_create_op);
    if (status != IV_SUCCESS) {
        ALOGE("error in %s: 0x%x", __func__,
              s_create_op.s_ivd_create_op_t.u4_error_code);
        return UNKNOWN_ERROR;
    }
    mDecHandle = (iv_obj_t*)s_create_op.s_ivd_create_op_t.pv_handle;
    mDecHandle->pv_fxns = (void *)ivdec_api_function;
    mDecHandle->u4_size = sizeof(iv_obj_t);

    return OK;
}

status_t C2SoftHevcDec::setNumCores() {
    ivdext_ctl_set_num_cores_ip_t s_set_num_cores_ip;
    ivdext_ctl_set_num_cores_op_t s_set_num_cores_op;

    s_set_num_cores_ip.u4_size = sizeof(ivdext_ctl_set_num_cores_ip_t);
    s_set_num_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_set_num_cores_ip.e_sub_cmd = IVDEXT_CMD_CTL_SET_NUM_CORES;
    s_set_num_cores_ip.u4_num_cores = mNumCores;
    s_set_num_cores_op.u4_size = sizeof(ivdext_ctl_set_num_cores_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_set_num_cores_ip,
                                                     &s_set_num_cores_op);
    if (IV_SUCCESS != status) {
        ALOGD("error in %s: 0x%x", __func__, s_set_num_cores_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t C2SoftHevcDec::setParams(size_t stride, IVD_VIDEO_DECODE_MODE_T dec_mode) {
    ivd_ctl_set_config_ip_t s_set_dyn_params_ip;
    ivd_ctl_set_config_op_t s_set_dyn_params_op;

    s_set_dyn_params_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_set_dyn_params_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_set_dyn_params_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_set_dyn_params_ip.u4_disp_wd = (UWORD32) stride;
    s_set_dyn_params_ip.e_frm_skip_mode = IVD_SKIP_NONE;
    s_set_dyn_params_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_set_dyn_params_ip.e_vid_dec_mode = dec_mode;
    s_set_dyn_params_op.u4_size = sizeof(ivd_ctl_set_config_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_set_dyn_params_ip,
                                                     &s_set_dyn_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("error in %s: 0x%x", __func__, s_set_dyn_params_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t C2SoftHevcDec::getVersion() {
    ivd_ctl_getversioninfo_ip_t s_get_versioninfo_ip;
    ivd_ctl_getversioninfo_op_t s_get_versioninfo_op;
    UWORD8 au1_buf[512];

    s_get_versioninfo_ip.u4_size = sizeof(ivd_ctl_getversioninfo_ip_t);
    s_get_versioninfo_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_get_versioninfo_ip.e_sub_cmd = IVD_CMD_CTL_GETVERSION;
    s_get_versioninfo_ip.pv_version_buffer = au1_buf;
    s_get_versioninfo_ip.u4_version_buffer_size = sizeof(au1_buf);
    s_get_versioninfo_op.u4_size = sizeof(ivd_ctl_getversioninfo_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_get_versioninfo_ip,
                                                     &s_get_versioninfo_op);
    if (status != IV_SUCCESS) {
        ALOGD("error in %s: 0x%x", __func__,
              s_get_versioninfo_op.u4_error_code);
    } else {
        ALOGV("ittiam decoder version number: %s",
              (char *) s_get_versioninfo_ip.pv_version_buffer);
    }

    return OK;
}

status_t C2SoftHevcDec::initDecoder() {
    if (OK != createDecoder()) return UNKNOWN_ERROR;
    mNumCores = MIN(getCpuCoreCount(), MAX_NUM_CORES);
    mStride = ALIGN64(mWidth);
    mSignalledError = false;
    resetPlugin();
    (void) setNumCores();
    if (OK != setParams(mStride, IVD_DECODE_FRAME)) return UNKNOWN_ERROR;
    (void) getVersion();

    return OK;
}

bool C2SoftHevcDec::setDecodeArgs(ivd_video_decode_ip_t *ps_decode_ip,
                                  ivd_video_decode_op_t *ps_decode_op,
                                  C2ReadView *inBuffer,
                                  C2GraphicView *outBuffer,
                                  size_t inOffset,
                                  size_t inSize,
                                  uint32_t tsMarker) {
    uint32_t displayStride = mStride;
    uint32_t displayHeight = mHeight;
    size_t lumaSize = displayStride * displayHeight;
    size_t chromaSize = lumaSize >> 2;

    ps_decode_ip->u4_size = sizeof(ivd_video_decode_ip_t);
    ps_decode_ip->e_cmd = IVD_CMD_VIDEO_DECODE;
    if (inBuffer) {
        ps_decode_ip->u4_ts = tsMarker;
        ps_decode_ip->pv_stream_buffer = const_cast<uint8_t *>(inBuffer->data() + inOffset);
        ps_decode_ip->u4_num_Bytes = inSize;
    } else {
        ps_decode_ip->u4_ts = 0;
        ps_decode_ip->pv_stream_buffer = nullptr;
        ps_decode_ip->u4_num_Bytes = 0;
    }
    ps_decode_ip->s_out_buffer.u4_min_out_buf_size[0] = lumaSize;
    ps_decode_ip->s_out_buffer.u4_min_out_buf_size[1] = chromaSize;
    ps_decode_ip->s_out_buffer.u4_min_out_buf_size[2] = chromaSize;
    if (outBuffer) {
        if (outBuffer->width() < displayStride || outBuffer->height() < displayHeight) {
            ALOGE("Output buffer too small: provided (%dx%d) required (%ux%u)",
                  outBuffer->width(), outBuffer->height(), displayStride, displayHeight);
            return false;
        }
        ps_decode_ip->s_out_buffer.pu1_bufs[0] = outBuffer->data()[C2PlanarLayout::PLANE_Y];
        ps_decode_ip->s_out_buffer.pu1_bufs[1] = outBuffer->data()[C2PlanarLayout::PLANE_U];
        ps_decode_ip->s_out_buffer.pu1_bufs[2] = outBuffer->data()[C2PlanarLayout::PLANE_V];
    } else {
        ps_decode_ip->s_out_buffer.pu1_bufs[0] = mOutBufferFlush;
        ps_decode_ip->s_out_buffer.pu1_bufs[1] = mOutBufferFlush + lumaSize;
        ps_decode_ip->s_out_buffer.pu1_bufs[2] = mOutBufferFlush + lumaSize + chromaSize;
    }
    ps_decode_ip->s_out_buffer.u4_num_bufs = 3;
    ps_decode_op->u4_size = sizeof(ivd_video_decode_op_t);
    ps_decode_op->u4_output_present = 0;

    return true;
}

bool C2SoftHevcDec::getVuiParams() {
    ivdext_ctl_get_vui_params_ip_t s_get_vui_params_ip;
    ivdext_ctl_get_vui_params_op_t s_get_vui_params_op;

    s_get_vui_params_ip.u4_size = sizeof(ivdext_ctl_get_vui_params_ip_t);
    s_get_vui_params_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_get_vui_params_ip.e_sub_cmd =
            (IVD_CONTROL_API_COMMAND_TYPE_T) IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS;
    s_get_vui_params_op.u4_size = sizeof(ivdext_ctl_get_vui_params_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_get_vui_params_ip,
                                                     &s_get_vui_params_op);
    if (status != IV_SUCCESS) {
        ALOGD("error in %s: 0x%x", __func__, s_get_vui_params_op.u4_error_code);
        return false;
    }

    VuiColorAspects vuiColorAspects;
    vuiColorAspects.primaries = s_get_vui_params_op.u1_colour_primaries;
    vuiColorAspects.transfer = s_get_vui_params_op.u1_transfer_characteristics;
    vuiColorAspects.coeffs = s_get_vui_params_op.u1_matrix_coefficients;
    vuiColorAspects.fullRange = s_get_vui_params_op.u1_video_full_range_flag;

    // convert vui aspects to C2 values if changed
    if (!(vuiColorAspects == mBitstreamColorAspects)) {
        mBitstreamColorAspects = vuiColorAspects;
        ColorAspects sfAspects;
        C2StreamColorAspectsInfo::input codedAspects = { 0u };
        ColorUtils::convertIsoColorAspectsToCodecAspects(
                vuiColorAspects.primaries, vuiColorAspects.transfer, vuiColorAspects.coeffs,
                vuiColorAspects.fullRange, sfAspects);
        if (!C2Mapper::map(sfAspects.mPrimaries, &codedAspects.primaries)) {
            codedAspects.primaries = C2Color::PRIMARIES_UNSPECIFIED;
        }
        if (!C2Mapper::map(sfAspects.mRange, &codedAspects.range)) {
            codedAspects.range = C2Color::RANGE_UNSPECIFIED;
        }
        if (!C2Mapper::map(sfAspects.mMatrixCoeffs, &codedAspects.matrix)) {
            codedAspects.matrix = C2Color::MATRIX_UNSPECIFIED;
        }
        if (!C2Mapper::map(sfAspects.mTransfer, &codedAspects.transfer)) {
            codedAspects.transfer = C2Color::TRANSFER_UNSPECIFIED;
        }
        std::vector<std::unique_ptr<C2SettingResult>> failures;
        (void)mIntf->config({&codedAspects}, C2_MAY_BLOCK, &failures);
    }
    return true;
}

status_t C2SoftHevcDec::setFlushMode() {
    ivd_ctl_flush_ip_t s_set_flush_ip;
    ivd_ctl_flush_op_t s_set_flush_op;

    s_set_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
    s_set_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_set_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
    s_set_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_set_flush_ip,
                                                     &s_set_flush_op);
    if (status != IV_SUCCESS) {
        ALOGE("error in %s: 0x%x", __func__, s_set_flush_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t C2SoftHevcDec::resetDecoder() {
    ivd_ctl_reset_ip_t s_reset_ip;
    ivd_ctl_reset_op_t s_reset_op;

    s_reset_ip.u4_size = sizeof(ivd_ctl_reset_ip_t);
    s_reset_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_reset_ip.e_sub_cmd = IVD_CMD_CTL_RESET;
    s_reset_op.u4_size = sizeof(ivd_ctl_reset_op_t);
    IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                     &s_reset_ip,
                                                     &s_reset_op);
    if (IV_SUCCESS != status) {
        ALOGE("error in %s: 0x%x", __func__, s_reset_op.u4_error_code);
        return UNKNOWN_ERROR;
    }
    mStride = 0;
    (void) setNumCores();
    mSignalledError = false;
    mHeaderDecoded = false;
    return OK;
}

void C2SoftHevcDec::resetPlugin() {
    mSignalledOutputEos = false;
    gettimeofday(&mTimeStart, nullptr);
    gettimeofday(&mTimeEnd, nullptr);
}

status_t C2SoftHevcDec::deleteDecoder() {
    if (mDecHandle) {
        ivdext_delete_ip_t s_delete_ip;
        ivdext_delete_op_t s_delete_op;

        s_delete_ip.s_ivd_delete_ip_t.u4_size = sizeof(ivdext_delete_ip_t);
        s_delete_ip.s_ivd_delete_ip_t.e_cmd = IVD_CMD_DELETE;
        s_delete_op.s_ivd_delete_op_t.u4_size = sizeof(ivdext_delete_op_t);
        IV_API_CALL_STATUS_T status = ivdec_api_function(mDecHandle,
                                                         &s_delete_ip,
                                                         &s_delete_op);
        if (status != IV_SUCCESS) {
            ALOGE("error in %s: 0x%x", __func__,
                  s_delete_op.s_ivd_delete_op_t.u4_error_code);
            return UNKNOWN_ERROR;
        }
        mDecHandle = nullptr;
    }

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

void C2SoftHevcDec::finishWork(uint64_t index, const std::unique_ptr<C2Work> &work) {
    std::shared_ptr<C2Buffer> buffer = createGraphicBuffer(std::move(mOutBlock),
                                                           C2Rect(mWidth, mHeight));
    mOutBlock = nullptr;
    {
        IntfImpl::Lock lock = mIntf->lock();
        buffer->setInfo(mIntf->getColorAspects_l());
    }

    class FillWork {
       public:
        FillWork(uint32_t flags, C2WorkOrdinalStruct ordinal,
                 const std::shared_ptr<C2Buffer>& buffer)
            : mFlags(flags), mOrdinal(ordinal), mBuffer(buffer) {}
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
                  mOrdinal.timestamp.peekll(), mOrdinal.frameIndex.peekll(),
                  mBuffer ? "" : "o");
        }

       private:
        const uint32_t mFlags;
        const C2WorkOrdinalStruct mOrdinal;
        const std::shared_ptr<C2Buffer> mBuffer;
    };

    auto fillWork = [buffer](const std::unique_ptr<C2Work> &work) {
        work->worklets.front()->output.flags = (C2FrameData::flags_t)0;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
    };
    if (work && c2_cntr64_t(index) == work->input.ordinal.frameIndex) {
        bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
        // TODO: Check if cloneAndSend can be avoided by tracking number of frames remaining
        if (eos) {
            if (buffer) {
                mOutIndex = index;
                C2WorkOrdinalStruct outOrdinal = work->input.ordinal;
                cloneAndSend(
                    mOutIndex, work,
                    FillWork(C2FrameData::FLAG_INCOMPLETE, outOrdinal, buffer));
                buffer.reset();
            }
        } else {
            fillWork(work);
        }
    } else {
        finish(index, fillWork);
    }
}

c2_status_t C2SoftHevcDec::ensureDecoderState(const std::shared_ptr<C2BlockPool> &pool) {
    if (!mDecHandle) {
        ALOGE("not supposed to be here, invalid decoder context");
        return C2_CORRUPTED;
    }
    if (mStride != ALIGN64(mWidth)) {
        mStride = ALIGN64(mWidth);
        if (OK != setParams(mStride, IVD_DECODE_FRAME)) return C2_CORRUPTED;
    }
    if (mOutBlock &&
            (mOutBlock->width() != mStride || mOutBlock->height() != mHeight)) {
        mOutBlock.reset();
    }
    if (!mOutBlock) {
        uint32_t format = HAL_PIXEL_FORMAT_YV12;
        C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
        c2_status_t err = pool->fetchGraphicBlock(mStride, mHeight, format, usage, &mOutBlock);
        if (err != C2_OK) {
            ALOGE("fetchGraphicBlock for Output failed with status %d", err);
            return err;
        }
        ALOGV("provided (%dx%d) required (%dx%d)",
              mOutBlock->width(), mOutBlock->height(), mStride, mHeight);
    }

    return C2_OK;
}

// TODO: can overall error checking be improved?
// TODO: allow configuration of color format and usage for graphic buffers instead
//       of hard coding them to HAL_PIXEL_FORMAT_YV12
// TODO: pass coloraspects information to surface
// TODO: test support for dynamic change in resolution
// TODO: verify if the decoder sent back all frames
void C2SoftHevcDec::process(
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
    uint32_t workIndex = work->input.ordinal.frameIndex.peeku() & 0xFFFFFFFF;
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
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    bool hasPicture = false;

    ALOGV("in buffer attr. size %zu timestamp %d frameindex %d, flags %x",
          inSize, (int)work->input.ordinal.timestamp.peeku(),
          (int)work->input.ordinal.frameIndex.peeku(), work->input.flags);
    size_t inPos = 0;
    while (inPos < inSize) {
        if (C2_OK != ensureDecoderState(pool)) {
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        }
        C2GraphicView wView = mOutBlock->map().get();
        if (wView.error()) {
            ALOGE("graphic view map failed %d", wView.error());
            work->result = wView.error();
            return;
        }
        ivd_video_decode_ip_t s_decode_ip;
        ivd_video_decode_op_t s_decode_op;
        if (!setDecodeArgs(&s_decode_ip, &s_decode_op, &rView, &wView,
                           inOffset + inPos, inSize - inPos, workIndex)) {
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        }

        if (false == mHeaderDecoded) {
            /* Decode header and get dimensions */
            setParams(mStride, IVD_DECODE_HEADER);
        }
        WORD32 delay;
        GETTIME(&mTimeStart, nullptr);
        TIME_DIFF(mTimeEnd, mTimeStart, delay);
        (void) ivdec_api_function(mDecHandle, &s_decode_ip, &s_decode_op);
        WORD32 decodeTime;
        GETTIME(&mTimeEnd, nullptr);
        TIME_DIFF(mTimeStart, mTimeEnd, decodeTime);
        ALOGV("decodeTime=%6d delay=%6d numBytes=%6d", decodeTime, delay,
              s_decode_op.u4_num_bytes_consumed);
        if (IVD_MEM_ALLOC_FAILED == (s_decode_op.u4_error_code & IVD_ERROR_MASK)) {
            ALOGE("allocation failure in decoder");
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        } else if (IVD_STREAM_WIDTH_HEIGHT_NOT_SUPPORTED ==
                   (s_decode_op.u4_error_code & IVD_ERROR_MASK)) {
            ALOGE("unsupported resolution : %dx%d", mWidth, mHeight);
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        } else if (IVD_RES_CHANGED == (s_decode_op.u4_error_code & IVD_ERROR_MASK)) {
            ALOGV("resolution changed");
            drainInternal(DRAIN_COMPONENT_NO_EOS, pool, work);
            resetDecoder();
            resetPlugin();
            work->workletsProcessed = 0u;

            /* Decode header and get new dimensions */
            setParams(mStride, IVD_DECODE_HEADER);
            (void) ivdec_api_function(mDecHandle, &s_decode_ip, &s_decode_op);
        } else if (IS_IVD_FATAL_ERROR(s_decode_op.u4_error_code)) {
            ALOGE("Fatal error in decoder 0x%x", s_decode_op.u4_error_code);
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return;
        }
        if (s_decode_op.i4_reorder_depth >= 0 && mOutputDelay != s_decode_op.i4_reorder_depth) {
            mOutputDelay = s_decode_op.i4_reorder_depth;
            ALOGV("New Output delay %d ", mOutputDelay);

            C2PortActualDelayTuning::output outputDelay(mOutputDelay);
            std::vector<std::unique_ptr<C2SettingResult>> failures;
            c2_status_t err =
                mIntf->config({&outputDelay}, C2_MAY_BLOCK, &failures);
            if (err == OK) {
                work->worklets.front()->output.configUpdate.push_back(
                    C2Param::Copy(outputDelay));
            } else {
                ALOGE("Cannot set output delay");
                mSignalledError = true;
                work->workletsProcessed = 1u;
                work->result = C2_CORRUPTED;
                return;
            }
            continue;
        }
        if (0 < s_decode_op.u4_pic_wd && 0 < s_decode_op.u4_pic_ht) {
            if (mHeaderDecoded == false) {
                mHeaderDecoded = true;
                setParams(ALIGN64(s_decode_op.u4_pic_wd), IVD_DECODE_FRAME);
            }
            if (s_decode_op.u4_pic_wd != mWidth ||  s_decode_op.u4_pic_ht != mHeight) {
                mWidth = s_decode_op.u4_pic_wd;
                mHeight = s_decode_op.u4_pic_ht;
                CHECK_EQ(0u, s_decode_op.u4_output_present);

                C2StreamPictureSizeInfo::output size(0u, mWidth, mHeight);
                std::vector<std::unique_ptr<C2SettingResult>> failures;
                c2_status_t err =
                    mIntf->config({&size}, C2_MAY_BLOCK, &failures);
                if (err == OK) {
                    work->worklets.front()->output.configUpdate.push_back(
                        C2Param::Copy(size));
                } else {
                    ALOGE("Cannot set width and height");
                    mSignalledError = true;
                    work->workletsProcessed = 1u;
                    work->result = C2_CORRUPTED;
                    return;
                }
                continue;
            }
        }
        (void) getVuiParams();
        hasPicture |= (1 == s_decode_op.u4_frame_decoded_flag);
        if (s_decode_op.u4_output_present) {
            finishWork(s_decode_op.u4_ts, work);
        }
        if (0 == s_decode_op.u4_num_bytes_consumed) {
            ALOGD("Bytes consumed is zero. Ignoring remaining bytes");
            break;
        }
        inPos += s_decode_op.u4_num_bytes_consumed;
        if (hasPicture && (inSize - inPos)) {
            ALOGD("decoded frame in current access nal, ignoring further trailing bytes %d",
                  (int)inSize - (int)inPos);
            break;
        }
    }

    if (eos) {
        drainInternal(DRAIN_COMPONENT_WITH_EOS, pool, work);
        mSignalledOutputEos = true;
    } else if (!hasPicture) {
        fillEmptyWork(work);
    }
}

c2_status_t C2SoftHevcDec::drainInternal(
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

    if (OK != setFlushMode()) return C2_CORRUPTED;
    while (true) {
        if (C2_OK != ensureDecoderState(pool)) {
            mSignalledError = true;
            work->workletsProcessed = 1u;
            work->result = C2_CORRUPTED;
            return C2_CORRUPTED;
        }
        C2GraphicView wView = mOutBlock->map().get();
        if (wView.error()) {
            ALOGE("graphic view map failed %d", wView.error());
            return C2_CORRUPTED;
        }
        ivd_video_decode_ip_t s_decode_ip;
        ivd_video_decode_op_t s_decode_op;
        if (!setDecodeArgs(&s_decode_ip, &s_decode_op, nullptr, &wView, 0, 0, 0)) {
            mSignalledError = true;
            work->workletsProcessed = 1u;
            return C2_CORRUPTED;
        }
        (void) ivdec_api_function(mDecHandle, &s_decode_ip, &s_decode_op);
        if (s_decode_op.u4_output_present) {
            finishWork(s_decode_op.u4_ts, work);
        } else {
            fillEmptyWork(work);
            break;
        }
    }

    return C2_OK;
}

c2_status_t C2SoftHevcDec::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    return drainInternal(drainMode, pool, nullptr);
}

class C2SoftHevcDecFactory : public C2ComponentFactory {
public:
    C2SoftHevcDecFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
        GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftHevcDec(COMPONENT_NAME,
                                 id,
                                 std::make_shared<C2SoftHevcDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftHevcDec::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftHevcDec::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftHevcDecFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftHevcDecFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
