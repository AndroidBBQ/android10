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
#define LOG_TAG "C2SoftHevcEnc"
#include <log/log.h>

#include <media/hardware/VideoAPI.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/AUtils.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <Codec2BufferUtils.h>
#include <SimpleC2Interface.h>
#include <util/C2InterfaceHelper.h>

#include "ihevc_typedefs.h"
#include "itt_video_api.h"
#include "ihevce_api.h"
#include "ihevce_plugin.h"
#include "C2SoftHevcEnc.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.hevc.encoder";

} // namepsace

class C2SoftHevcEnc::IntfImpl : public SimpleInterface<void>::BaseParams {
  public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_ENCODER,
                C2Component::DOMAIN_VIDEO,
                MEDIA_MIMETYPE_VIDEO_HEVC) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noTimeStretch();
        setDerivedInstance(this);

        addParameter(
                DefineParam(mActualInputDelay, C2_PARAMKEY_INPUT_DELAY)
                .withDefault(new C2PortActualDelayTuning::input(
                    DEFAULT_B_FRAMES + DEFAULT_RC_LOOKAHEAD))
                .withFields({C2F(mActualInputDelay, value).inRange(
                    0, MAX_B_FRAMES + MAX_RC_LOOKAHEAD)})
                .withSetter(
                    Setter<decltype(*mActualInputDelay)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(
                    C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mUsage, C2_PARAMKEY_INPUT_STREAM_USAGE)
                .withConstValue(new C2StreamUsageTuning::input(
                        0u, (uint64_t)C2MemoryUsage::CPU_READ))
                .build());

        // matches size limits in codec library
        addParameter(
            DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::input(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 1920, 2),
                    C2F(mSize, height).inRange(2, 1088, 2),
                })
                .withSetter(SizeSetter)
                .build());

        addParameter(
            DefineParam(mFrameRate, C2_PARAMKEY_FRAME_RATE)
                .withDefault(new C2StreamFrameRateInfo::output(0u, 30.))
                .withFields({C2F(mFrameRate, value).greaterThan(0.)})
                .withSetter(
                    Setter<decltype(*mFrameRate)>::StrictValueWithNoDeps)
                .build());

        // matches limits in codec library
        addParameter(
            DefineParam(mBitrateMode, C2_PARAMKEY_BITRATE_MODE)
                .withDefault(new C2StreamBitrateModeTuning::output(
                        0u, C2Config::BITRATE_VARIABLE))
                .withFields({
                    C2F(mBitrateMode, value).oneOf({
                        C2Config::BITRATE_CONST,
                        C2Config::BITRATE_VARIABLE,
                        C2Config::BITRATE_IGNORE})
                })
                .withSetter(
                    Setter<decltype(*mBitrateMode)>::StrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(4096, 12000000)})
                .withSetter(BitrateSetter)
                .build());

        // matches levels allowed within codec library
        addParameter(
                DefineParam(mComplexity, C2_PARAMKEY_COMPLEXITY)
                .withDefault(new C2StreamComplexityTuning::output(0u, 0))
                .withFields({C2F(mComplexity, value).inRange(0, 10)})
                .withSetter(Setter<decltype(*mComplexity)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mQuality, C2_PARAMKEY_QUALITY)
                .withDefault(new C2StreamQualityTuning::output(0u, 80))
                .withFields({C2F(mQuality, value).inRange(0, 100)})
                .withSetter(Setter<decltype(*mQuality)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::output(
                    0u, PROFILE_HEVC_MAIN, LEVEL_HEVC_MAIN_1))
                .withFields({
                    C2F(mProfileLevel, profile)
                        .oneOf({C2Config::PROFILE_HEVC_MAIN,
                                C2Config::PROFILE_HEVC_MAIN_STILL}),
                    C2F(mProfileLevel, level)
                        .oneOf({LEVEL_HEVC_MAIN_1, LEVEL_HEVC_MAIN_2,
                                LEVEL_HEVC_MAIN_2_1, LEVEL_HEVC_MAIN_3,
                                LEVEL_HEVC_MAIN_3_1, LEVEL_HEVC_MAIN_4,
                                LEVEL_HEVC_MAIN_4_1, LEVEL_HEVC_MAIN_5,
                                LEVEL_HEVC_MAIN_5_1, LEVEL_HEVC_MAIN_5_2}),
                })
                .withSetter(ProfileLevelSetter, mSize, mFrameRate, mBitrate)
                .build());

        addParameter(
                DefineParam(mRequestSync, C2_PARAMKEY_REQUEST_SYNC_FRAME)
                .withDefault(new C2StreamRequestSyncFrameTuning::output(0u, C2_FALSE))
                .withFields({C2F(mRequestSync, value).oneOf({ C2_FALSE, C2_TRUE }) })
                .withSetter(Setter<decltype(*mRequestSync)>::NonStrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mSyncFramePeriod, C2_PARAMKEY_SYNC_FRAME_INTERVAL)
                .withDefault(
                    new C2StreamSyncFrameIntervalTuning::output(0u, 1000000))
                .withFields({C2F(mSyncFramePeriod, value).any()})
                .withSetter(
                    Setter<decltype(*mSyncFramePeriod)>::StrictValueWithNoDeps)
                .build());
    }

    static C2R BitrateSetter(bool mayBlock,
                             C2P<C2StreamBitrateInfo::output>& me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (me.v.value < 4096) {
            me.set().value = 4096;
        }
        return res;
    }

    static C2R SizeSetter(bool mayBlock,
                          const C2P<C2StreamPictureSizeInfo::input>& oldMe,
                          C2P<C2StreamPictureSizeInfo::input>& me) {
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

    static C2R ProfileLevelSetter(
            bool mayBlock,
            C2P<C2StreamProfileLevelInfo::output> &me,
            const C2P<C2StreamPictureSizeInfo::input> &size,
            const C2P<C2StreamFrameRateInfo::output> &frameRate,
            const C2P<C2StreamBitrateInfo::output> &bitrate) {
        (void)mayBlock;
        if (!me.F(me.v.profile).supportsAtAll(me.v.profile)) {
            me.set().profile = PROFILE_HEVC_MAIN;
        }

        struct LevelLimits {
            C2Config::level_t level;
            uint64_t samplesPerSec;
            uint64_t samples;
            uint32_t bitrate;
        };

        constexpr LevelLimits kLimits[] = {
            { LEVEL_HEVC_MAIN_1,       552960,    36864,    128000 },
            { LEVEL_HEVC_MAIN_2,      3686400,   122880,   1500000 },
            { LEVEL_HEVC_MAIN_2_1,    7372800,   245760,   3000000 },
            { LEVEL_HEVC_MAIN_3,     16588800,   552960,   6000000 },
            { LEVEL_HEVC_MAIN_3_1,   33177600,   983040,  10000000 },
            { LEVEL_HEVC_MAIN_4,     66846720,  2228224,  12000000 },
            { LEVEL_HEVC_MAIN_4_1,  133693440,  2228224,  20000000 },
            { LEVEL_HEVC_MAIN_5,    267386880,  8912896,  25000000 },
            { LEVEL_HEVC_MAIN_5_1,  534773760,  8912896,  40000000 },
            { LEVEL_HEVC_MAIN_5_2, 1069547520,  8912896,  60000000 },
            { LEVEL_HEVC_MAIN_6,   1069547520, 35651584,  60000000 },
            { LEVEL_HEVC_MAIN_6_1, 2139095040, 35651584, 120000000 },
            { LEVEL_HEVC_MAIN_6_2, 4278190080, 35651584, 240000000 },
        };

        uint64_t samples = size.v.width * size.v.height;
        uint64_t samplesPerSec = samples * frameRate.v.value;

        // Check if the supplied level meets the MB / bitrate requirements. If
        // not, update the level with the lowest level meeting the requirements.

        bool found = false;
        // By default needsUpdate = false in case the supplied level does meet
        // the requirements.
        bool needsUpdate = false;
        for (const LevelLimits &limit : kLimits) {
            if (samples <= limit.samples && samplesPerSec <= limit.samplesPerSec &&
                    bitrate.v.value <= limit.bitrate) {
                // This is the lowest level that meets the requirements, and if
                // we haven't seen the supplied level yet, that means we don't
                // need the update.
                if (needsUpdate) {
                    ALOGD("Given level %x does not cover current configuration: "
                          "adjusting to %x", me.v.level, limit.level);
                    me.set().level = limit.level;
                }
                found = true;
                break;
            }
            if (me.v.level == limit.level) {
                // We break out of the loop when the lowest feasible level is
                // found. The fact that we're here means that our level doesn't
                // meet the requirement and needs to be updated.
                needsUpdate = true;
            }
        }
        if (!found) {
            // We set to the highest supported level.
            me.set().level = LEVEL_HEVC_MAIN_5_2;
        }
        return C2R::Ok();
    }

    UWORD32 getProfile_l() const {
        switch (mProfileLevel->profile) {
        case PROFILE_HEVC_MAIN:  [[fallthrough]];
        case PROFILE_HEVC_MAIN_STILL: return 1;
        default:
            ALOGD("Unrecognized profile: %x", mProfileLevel->profile);
            return 1;
        }
    }

    UWORD32 getLevel_l() const {
        struct Level {
            C2Config::level_t c2Level;
            UWORD32 hevcLevel;
        };
        constexpr Level levels[] = {
            { LEVEL_HEVC_MAIN_1,    30 },
            { LEVEL_HEVC_MAIN_2,    60 },
            { LEVEL_HEVC_MAIN_2_1,  63 },
            { LEVEL_HEVC_MAIN_3,    90 },
            { LEVEL_HEVC_MAIN_3_1,  93 },
            { LEVEL_HEVC_MAIN_4,   120 },
            { LEVEL_HEVC_MAIN_4_1, 123 },
            { LEVEL_HEVC_MAIN_5,   150 },
            { LEVEL_HEVC_MAIN_5_1, 153 },
            { LEVEL_HEVC_MAIN_5_2, 156 },
            { LEVEL_HEVC_MAIN_6,   180 },
            { LEVEL_HEVC_MAIN_6_1, 183 },
            { LEVEL_HEVC_MAIN_6_2, 186 },
        };
        for (const Level &level : levels) {
            if (mProfileLevel->level == level.c2Level) {
                return level.hevcLevel;
            }
        }
        ALOGD("Unrecognized level: %x", mProfileLevel->level);
        return 156;
    }
    uint32_t getSyncFramePeriod_l() const {
        if (mSyncFramePeriod->value < 0 ||
            mSyncFramePeriod->value == INT64_MAX) {
            return 0;
        }
        double period = mSyncFramePeriod->value / 1e6 * mFrameRate->value;
        return (uint32_t)c2_max(c2_min(period + 0.5, double(UINT32_MAX)), 1.);
    }

    std::shared_ptr<C2StreamPictureSizeInfo::input> getSize_l() const {
        return mSize;
    }
    std::shared_ptr<C2StreamFrameRateInfo::output> getFrameRate_l() const {
        return mFrameRate;
    }
    std::shared_ptr<C2StreamBitrateModeTuning::output> getBitrateMode_l() const {
        return mBitrateMode;
    }
    std::shared_ptr<C2StreamBitrateInfo::output> getBitrate_l() const {
        return mBitrate;
    }
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> getRequestSync_l() const {
        return mRequestSync;
    }
    std::shared_ptr<C2StreamComplexityTuning::output> getComplexity_l() const {
        return mComplexity;
    }
    std::shared_ptr<C2StreamQualityTuning::output> getQuality_l() const {
        return mQuality;
    }

   private:
    std::shared_ptr<C2StreamUsageTuning::input> mUsage;
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> mRequestSync;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamBitrateModeTuning::output> mBitrateMode;
    std::shared_ptr<C2StreamComplexityTuning::output> mComplexity;
    std::shared_ptr<C2StreamQualityTuning::output> mQuality;
    std::shared_ptr<C2StreamProfileLevelInfo::output> mProfileLevel;
    std::shared_ptr<C2StreamSyncFrameIntervalTuning::output> mSyncFramePeriod;
};

static size_t GetCPUCoreCount() {
    long cpuCoreCount = 0;

#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif

    if (cpuCoreCount < 1)
        cpuCoreCount = 1;
    return (size_t)cpuCoreCount;
}

C2SoftHevcEnc::C2SoftHevcEnc(const char* name, c2_node_id_t id,
                             const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
          std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mIvVideoColorFormat(IV_YUV_420P),
      mHevcEncProfile(1),
      mHevcEncLevel(30),
      mStarted(false),
      mSpsPpsHeaderReceived(false),
      mSignalledEos(false),
      mSignalledError(false),
      mCodecCtx(nullptr) {
    // If dump is enabled, then create an empty file
    GENERATE_FILE_NAMES();
    CREATE_DUMP_FILE(mInFile);
    CREATE_DUMP_FILE(mOutFile);

    gettimeofday(&mTimeStart, nullptr);
    gettimeofday(&mTimeEnd, nullptr);
}

C2SoftHevcEnc::~C2SoftHevcEnc() {
    onRelease();
}

c2_status_t C2SoftHevcEnc::onInit() {
    return C2_OK;
}

c2_status_t C2SoftHevcEnc::onStop() {
    return C2_OK;
}

void C2SoftHevcEnc::onReset() {
    releaseEncoder();
}

void C2SoftHevcEnc::onRelease() {
    releaseEncoder();
}

c2_status_t C2SoftHevcEnc::onFlush_sm() {
    return C2_OK;
}

static void fillEmptyWork(const std::unique_ptr<C2Work>& work) {
    uint32_t flags = 0;
    if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
        flags |= C2FrameData::FLAG_END_OF_STREAM;
        ALOGV("Signalling EOS");
    }
    work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = work->input.ordinal;
    work->workletsProcessed = 1u;
}

static int getQpFromQuality(int quality) {
    int qp;
#define MIN_QP 4
#define MAX_QP 50
    /* Quality: 100 -> Qp : MIN_QP
     * Quality: 0 -> Qp : MAX_QP
     * Qp = ((MIN_QP - MAX_QP) * quality / 100) + MAX_QP;
     */
    qp = ((MIN_QP - MAX_QP) * quality / 100) + MAX_QP;
    qp = std::min(qp, MAX_QP);
    qp = std::max(qp, MIN_QP);
    return qp;
}
c2_status_t C2SoftHevcEnc::initEncParams() {
    mCodecCtx = nullptr;
    mNumCores = std::min(GetCPUCoreCount(), (size_t) CODEC_MAX_CORES);
    memset(&mEncParams, 0, sizeof(ihevce_static_cfg_params_t));

    // default configuration
    IHEVCE_PLUGIN_STATUS_T err = ihevce_set_def_params(&mEncParams);
    if (IHEVCE_EOK != err) {
        ALOGE("HEVC default init failed : 0x%x", err);
        return C2_CORRUPTED;
    }

    // update configuration
    mEncParams.s_src_prms.i4_width = mSize->width;
    mEncParams.s_src_prms.i4_height = mSize->height;
    mEncParams.s_src_prms.i4_frm_rate_denom = 1000;
    mEncParams.s_src_prms.i4_frm_rate_num =
        mFrameRate->value * mEncParams.s_src_prms.i4_frm_rate_denom;
    mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P5;
    mEncParams.s_tgt_lyr_prms.as_tgt_params[0].ai4_tgt_bitrate[0] =
        mBitrate->value;
    mEncParams.s_tgt_lyr_prms.as_tgt_params[0].ai4_peak_bitrate[0] =
        mBitrate->value << 1;
    mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_codec_level = mHevcEncLevel;
    mEncParams.s_coding_tools_prms.i4_max_i_open_gop_period = mIDRInterval;
    mEncParams.s_coding_tools_prms.i4_max_cra_open_gop_period = mIDRInterval;
    mIvVideoColorFormat = IV_YUV_420P;
    mEncParams.s_multi_thrd_prms.i4_max_num_cores = mNumCores;
    mEncParams.s_out_strm_prms.i4_codec_profile = mHevcEncProfile;
    mEncParams.s_lap_prms.i4_rc_look_ahead_pics = DEFAULT_RC_LOOKAHEAD;
    mEncParams.s_coding_tools_prms.i4_max_temporal_layers = DEFAULT_B_FRAMES;

    switch (mBitrateMode->value) {
        case C2Config::BITRATE_IGNORE:
            mEncParams.s_config_prms.i4_rate_control_mode = 3;
            mEncParams.s_tgt_lyr_prms.as_tgt_params[0].ai4_frame_qp[0] =
                getQpFromQuality(mQuality->value);
            break;
        case C2Config::BITRATE_CONST:
            mEncParams.s_config_prms.i4_rate_control_mode = 5;
            break;
        case C2Config::BITRATE_VARIABLE:
            [[fallthrough]];
        default:
            mEncParams.s_config_prms.i4_rate_control_mode = 2;
            break;
        break;
    }

    if (mComplexity->value == 10) {
        mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P0;
    } else if (mComplexity->value >= 8) {
        mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P2;
    } else if (mComplexity->value >= 7) {
        mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P3;
    } else if (mComplexity->value >= 5) {
        mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P4;
    } else {
        mEncParams.s_tgt_lyr_prms.as_tgt_params[0].i4_quality_preset = IHEVCE_QUALITY_P5;
    }

    return C2_OK;
}

c2_status_t C2SoftHevcEnc::releaseEncoder() {
    mSpsPpsHeaderReceived = false;
    mSignalledEos = false;
    mSignalledError = false;
    mStarted = false;

    if (mCodecCtx) {
        IHEVCE_PLUGIN_STATUS_T err = ihevce_close(mCodecCtx);
        if (IHEVCE_EOK != err) return C2_CORRUPTED;
        mCodecCtx = nullptr;
    }
    return C2_OK;
}

c2_status_t C2SoftHevcEnc::drain(uint32_t drainMode,
                                 const std::shared_ptr<C2BlockPool>& pool) {
    return drainInternal(drainMode, pool, nullptr);
}

c2_status_t C2SoftHevcEnc::initEncoder() {
    CHECK(!mCodecCtx);
    {
        IntfImpl::Lock lock = mIntf->lock();
        mSize = mIntf->getSize_l();
        mBitrateMode = mIntf->getBitrateMode_l();
        mBitrate = mIntf->getBitrate_l();
        mFrameRate = mIntf->getFrameRate_l();
        mHevcEncProfile = mIntf->getProfile_l();
        mHevcEncLevel = mIntf->getLevel_l();
        mIDRInterval = mIntf->getSyncFramePeriod_l();
        mComplexity = mIntf->getComplexity_l();
        mQuality = mIntf->getQuality_l();
    }

    c2_status_t status = initEncParams();

    if (C2_OK != status) {
        ALOGE("Failed to initialize encoder params : 0x%x", status);
        mSignalledError = true;
        return status;
    }

    IHEVCE_PLUGIN_STATUS_T err = IHEVCE_EOK;
    err = ihevce_init(&mEncParams, &mCodecCtx);
    if (IHEVCE_EOK != err) {
        ALOGE("HEVC encoder init failed : 0x%x", err);
        return C2_CORRUPTED;
    }

    mStarted = true;
    return C2_OK;
}

c2_status_t C2SoftHevcEnc::setEncodeArgs(ihevce_inp_buf_t* ps_encode_ip,
                                         const C2GraphicView* const input,
                                         uint64_t workIndex) {
    ihevce_static_cfg_params_t* params = &mEncParams;
    memset(ps_encode_ip, 0, sizeof(*ps_encode_ip));

    if (!input) {
        return C2_OK;
    }

    if (input->width() < mSize->width ||
        input->height() < mSize->height) {
        /* Expect width height to be configured */
        ALOGW("unexpected Capacity Aspect %d(%d) x %d(%d)", input->width(),
              mSize->width, input->height(), mSize->height);
        return C2_BAD_VALUE;
    }

    const C2PlanarLayout& layout = input->layout();
    uint8_t* yPlane =
        const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_Y]);
    uint8_t* uPlane =
        const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_U]);
    uint8_t* vPlane =
        const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_V]);
    int32_t yStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
    int32_t uStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;
    int32_t vStride = layout.planes[C2PlanarLayout::PLANE_V].rowInc;

    const uint32_t width = mSize->width;
    const uint32_t height = mSize->height;

    // width and height must be even
    if (width & 1u || height & 1u) {
        ALOGW("height(%u) and width(%u) must both be even", height, width);
        return C2_BAD_VALUE;
    }

    size_t yPlaneSize = width * height;

    switch (layout.type) {
        case C2PlanarLayout::TYPE_RGB:
            [[fallthrough]];
        case C2PlanarLayout::TYPE_RGBA: {
            MemoryBlock conversionBuffer =
                mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(),
                                            conversionBuffer);
            yPlane = conversionBuffer.data();
            uPlane = yPlane + yPlaneSize;
            vPlane = uPlane + yPlaneSize / 4;
            yStride = width;
            uStride = vStride = yStride / 2;
            ConvertRGBToPlanarYUV(yPlane, yStride, height,
                                  conversionBuffer.size(), *input);
            break;
        }
        case C2PlanarLayout::TYPE_YUV: {
            if (!IsYUV420(*input)) {
                ALOGE("input is not YUV420");
                return C2_BAD_VALUE;
            }

            if (layout.planes[layout.PLANE_Y].colInc == 1 &&
                layout.planes[layout.PLANE_U].colInc == 1 &&
                layout.planes[layout.PLANE_V].colInc == 1 &&
                uStride == vStride && yStride == 2 * vStride) {
                // I420 compatible - already set up above
                break;
            }

            // copy to I420
            yStride = width;
            uStride = vStride = yStride / 2;
            MemoryBlock conversionBuffer =
                mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(),
                                            conversionBuffer);
            MediaImage2 img =
                CreateYUV420PlanarMediaImage2(width, height, yStride, height);
            status_t err = ImageCopy(conversionBuffer.data(), &img, *input);
            if (err != OK) {
                ALOGE("Buffer conversion failed: %d", err);
                return C2_BAD_VALUE;
            }
            yPlane = conversionBuffer.data();
            uPlane = yPlane + yPlaneSize;
            vPlane = uPlane + yPlaneSize / 4;
            break;
        }

        case C2PlanarLayout::TYPE_YUVA:
            ALOGE("YUVA plane type is not supported");
            return C2_BAD_VALUE;

        default:
            ALOGE("Unrecognized plane type: %d", layout.type);
            return C2_BAD_VALUE;
    }

    switch (mIvVideoColorFormat) {
        case IV_YUV_420P: {
            // input buffer is supposed to be const but Ittiam API wants bare
            // pointer.
            ps_encode_ip->apv_inp_planes[0] = yPlane;
            ps_encode_ip->apv_inp_planes[1] = uPlane;
            ps_encode_ip->apv_inp_planes[2] = vPlane;

            ps_encode_ip->ai4_inp_strd[0] = yStride;
            ps_encode_ip->ai4_inp_strd[1] = uStride;
            ps_encode_ip->ai4_inp_strd[2] = vStride;

            ps_encode_ip->ai4_inp_size[0] = yStride * height;
            ps_encode_ip->ai4_inp_size[1] = uStride * height >> 1;
            ps_encode_ip->ai4_inp_size[2] = vStride * height >> 1;
            break;
        }

        case IV_YUV_422ILE: {
            // TODO
            break;
        }

        case IV_YUV_420SP_UV:
        case IV_YUV_420SP_VU:
        default: {
            ps_encode_ip->apv_inp_planes[0] = yPlane;
            ps_encode_ip->apv_inp_planes[1] = uPlane;
            ps_encode_ip->apv_inp_planes[2] = nullptr;

            ps_encode_ip->ai4_inp_strd[0] = yStride;
            ps_encode_ip->ai4_inp_strd[1] = uStride;
            ps_encode_ip->ai4_inp_strd[2] = 0;

            ps_encode_ip->ai4_inp_size[0] = yStride * height;
            ps_encode_ip->ai4_inp_size[1] = uStride * height >> 1;
            ps_encode_ip->ai4_inp_size[2] = 0;
            break;
        }
    }

    ps_encode_ip->i4_curr_bitrate =
        params->s_tgt_lyr_prms.as_tgt_params[0].ai4_tgt_bitrate[0];
    ps_encode_ip->i4_curr_peak_bitrate =
        params->s_tgt_lyr_prms.as_tgt_params[0].ai4_peak_bitrate[0];
    ps_encode_ip->i4_curr_rate_factor = params->s_config_prms.i4_rate_factor;
    ps_encode_ip->u8_pts = workIndex;
    return C2_OK;
}

void C2SoftHevcEnc::finishWork(uint64_t index,
                               const std::unique_ptr<C2Work>& work,
                               const std::shared_ptr<C2BlockPool>& pool,
                               ihevce_out_buf_t* ps_encode_op) {
    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE};
    c2_status_t status =
        pool->fetchLinearBlock(ps_encode_op->i4_bytes_generated, usage, &block);
    if (C2_OK != status) {
        ALOGE("fetchLinearBlock for Output failed with status 0x%x", status);
        mSignalledError = true;
        work->result = status;
        work->workletsProcessed = 1u;
        return;
    }
    C2WriteView wView = block->map().get();
    if (C2_OK != wView.error()) {
        ALOGE("write view map failed with status 0x%x", wView.error());
        mSignalledError = true;
        work->result = wView.error();
        work->workletsProcessed = 1u;
        return;
    }
    memcpy(wView.data(), ps_encode_op->pu1_output_buf,
           ps_encode_op->i4_bytes_generated);

    std::shared_ptr<C2Buffer> buffer =
        createLinearBuffer(block, 0, ps_encode_op->i4_bytes_generated);

    DUMP_TO_FILE(mOutFile, ps_encode_op->pu1_output_buf,
                 ps_encode_op->i4_bytes_generated);

    if (ps_encode_op->i4_is_key_frame) {
        ALOGV("IDR frame produced");
        buffer->setInfo(std::make_shared<C2StreamPictureTypeMaskInfo::output>(
            0u /* stream id */, C2Config::SYNC_FRAME));
    }

    auto fillWork = [buffer](const std::unique_ptr<C2Work>& work) {
        work->worklets.front()->output.flags = (C2FrameData::flags_t)0;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
    };
    if (work && c2_cntr64_t(index) == work->input.ordinal.frameIndex) {
        fillWork(work);
        if (mSignalledEos) {
            work->worklets.front()->output.flags =
                C2FrameData::FLAG_END_OF_STREAM;
        }
    } else {
        finish(index, fillWork);
    }
}

c2_status_t C2SoftHevcEnc::drainInternal(
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

    while (true) {
        ihevce_out_buf_t s_encode_op{};
        memset(&s_encode_op, 0, sizeof(s_encode_op));

        ihevce_encode(mCodecCtx, nullptr, &s_encode_op);
        if (s_encode_op.i4_bytes_generated) {
            finishWork(s_encode_op.u8_pts, work, pool, &s_encode_op);
        } else {
            if (work->workletsProcessed != 1u) fillEmptyWork(work);
            break;
        }
    }
    return C2_OK;
}

void C2SoftHevcEnc::process(const std::unique_ptr<C2Work>& work,
                            const std::shared_ptr<C2BlockPool>& pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 0u;
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledEos) {
        work->result = C2_BAD_VALUE;
        ALOGD("Signalled Error / Signalled Eos");
        return;
    }
    c2_status_t status = C2_OK;

    // Initialize encoder if not already initialized
    if (!mStarted) {
        status = initEncoder();
        if (C2_OK != status) {
            ALOGE("Failed to initialize encoder : 0x%x", status);
            mSignalledError = true;
            work->result = status;
            work->workletsProcessed = 1u;
            return;
        }
    }

    std::shared_ptr<const C2GraphicView> view;
    std::shared_ptr<C2Buffer> inputBuffer = nullptr;
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    if (eos) mSignalledEos = true;

    if (!work->input.buffers.empty()) {
        inputBuffer = work->input.buffers[0];
        view = std::make_shared<const C2GraphicView>(
            inputBuffer->data().graphicBlocks().front().map().get());
        if (view->error() != C2_OK) {
            ALOGE("graphic view map err = %d", view->error());
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
    }
    IHEVCE_PLUGIN_STATUS_T err = IHEVCE_EOK;

    if (!mSpsPpsHeaderReceived) {
        ihevce_out_buf_t s_header_op{};
        err = ihevce_encode_header(mCodecCtx, &s_header_op);
        if (err == IHEVCE_EOK && s_header_op.i4_bytes_generated) {
            std::unique_ptr<C2StreamInitDataInfo::output> csd =
                C2StreamInitDataInfo::output::AllocUnique(
                    s_header_op.i4_bytes_generated, 0u);
            if (!csd) {
                ALOGE("CSD allocation failed");
                mSignalledError = true;
                work->result = C2_NO_MEMORY;
                work->workletsProcessed = 1u;
                return;
            }
            memcpy(csd->m.value, s_header_op.pu1_output_buf,
                   s_header_op.i4_bytes_generated);
            DUMP_TO_FILE(mOutFile, csd->m.value, csd->flexCount());
            work->worklets.front()->output.configUpdate.push_back(
                std::move(csd));
            mSpsPpsHeaderReceived = true;
        }
        if (!inputBuffer) {
            work->workletsProcessed = 1u;
            return;
        }
    }

    // handle dynamic config parameters
    {
        IntfImpl::Lock lock = mIntf->lock();
        std::shared_ptr<C2StreamBitrateInfo::output> bitrate = mIntf->getBitrate_l();
        lock.unlock();

        if (bitrate != mBitrate) {
            mBitrate = bitrate;
            mEncParams.s_tgt_lyr_prms.as_tgt_params[0].ai4_tgt_bitrate[0] =
                mBitrate->value;
            mEncParams.s_tgt_lyr_prms.as_tgt_params[0].ai4_peak_bitrate[0] =
                mBitrate->value << 1;
        }
    }

    ihevce_inp_buf_t s_encode_ip{};
    ihevce_out_buf_t s_encode_op{};
    uint64_t workIndex = work->input.ordinal.frameIndex.peekull();

    status = setEncodeArgs(&s_encode_ip, view.get(), workIndex);
    if (C2_OK != status) {
        ALOGE("setEncodeArgs failed : 0x%x", status);
        mSignalledError = true;
        work->result = status;
        work->workletsProcessed = 1u;
        return;
    }

    uint64_t timeDelay = 0;
    uint64_t timeTaken = 0;
    memset(&s_encode_op, 0, sizeof(s_encode_op));
    GETTIME(&mTimeStart, nullptr);
    TIME_DIFF(mTimeEnd, mTimeStart, timeDelay);

    if (inputBuffer) {
        err = ihevce_encode(mCodecCtx, &s_encode_ip, &s_encode_op);
        if (IHEVCE_EOK != err) {
            ALOGE("Encode Frame failed : 0x%x", err);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
    } else if (!eos) {
        fillEmptyWork(work);
    }

    GETTIME(&mTimeEnd, nullptr);
    /* Compute time taken for decode() */
    TIME_DIFF(mTimeStart, mTimeEnd, timeTaken);

    ALOGV("timeTaken=%6d delay=%6d numBytes=%6d", (int)timeTaken,
          (int)timeDelay, s_encode_op.i4_bytes_generated);

    if (s_encode_op.i4_bytes_generated) {
        finishWork(s_encode_op.u8_pts, work, pool, &s_encode_op);
    }

    if (eos) {
        drainInternal(DRAIN_COMPONENT_WITH_EOS, pool, work);
    }
}

class C2SoftHevcEncFactory : public C2ComponentFactory {
   public:
    C2SoftHevcEncFactory()
        : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
              GetCodec2PlatformComponentStore()->getParamReflector())) {}

    c2_status_t createComponent(
        c2_node_id_t id,
        std::shared_ptr<C2Component>* const component,
        std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftHevcEnc(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftHevcEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    c2_status_t createInterface(
        c2_node_id_t id,
        std::shared_ptr<C2ComponentInterface>* const interface,
        std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftHevcEnc::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftHevcEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    ~C2SoftHevcEncFactory() override = default;

   private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftHevcEncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
