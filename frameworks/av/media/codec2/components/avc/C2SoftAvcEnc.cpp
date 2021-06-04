/*
 * Copyright 2018 The Android Open Source Project
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
#define LOG_TAG "C2SoftAvcEnc"
#include <log/log.h>
#include <utils/misc.h>

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

#include "C2SoftAvcEnc.h"
#include "ih264e.h"
#include "ih264e_error.h"

namespace android {

namespace {

constexpr char COMPONENT_NAME[] = "c2.android.avc.encoder";

void ParseGop(
        const C2StreamGopTuning::output &gop,
        uint32_t *syncInterval, uint32_t *iInterval, uint32_t *maxBframes) {
    uint32_t syncInt = 1;
    uint32_t iInt = 1;
    for (size_t i = 0; i < gop.flexCount(); ++i) {
        const C2GopLayerStruct &layer = gop.m.values[i];
        if (layer.count == UINT32_MAX) {
            syncInt = 0;
        } else if (syncInt <= UINT32_MAX / (layer.count + 1)) {
            syncInt *= (layer.count + 1);
        }
        if ((layer.type_ & I_FRAME) == 0) {
            if (layer.count == UINT32_MAX) {
                iInt = 0;
            } else if (iInt <= UINT32_MAX / (layer.count + 1)) {
                iInt *= (layer.count + 1);
            }
        }
        if (layer.type_ == C2Config::picture_type_t(P_FRAME | B_FRAME) && maxBframes) {
            *maxBframes = layer.count;
        }
    }
    if (syncInterval) {
        *syncInterval = syncInt;
    }
    if (iInterval) {
        *iInterval = iInt;
    }
}

}  // namespace

class C2SoftAvcEnc::IntfImpl : public SimpleInterface<void>::BaseParams {
public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_ENCODER,
                C2Component::DOMAIN_VIDEO,
                MEDIA_MIMETYPE_VIDEO_AVC) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noTimeStretch();
        setDerivedInstance(this);

        addParameter(
                DefineParam(mUsage, C2_PARAMKEY_INPUT_STREAM_USAGE)
                .withConstValue(new C2StreamUsageTuning::input(
                        0u, (uint64_t)C2MemoryUsage::CPU_READ))
                .build());

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(
                    C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::input(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 2560, 2),
                    C2F(mSize, height).inRange(2, 2560, 2),
                })
                .withSetter(SizeSetter)
                .build());

        addParameter(
                DefineParam(mGop, C2_PARAMKEY_GOP)
                .withDefault(C2StreamGopTuning::output::AllocShared(
                        0 /* flexCount */, 0u /* stream */))
                .withFields({C2F(mGop, m.values[0].type_).any(),
                             C2F(mGop, m.values[0].count).any()})
                .withSetter(GopSetter)
                .build());

        addParameter(
                DefineParam(mActualInputDelay, C2_PARAMKEY_INPUT_DELAY)
                .withDefault(new C2PortActualDelayTuning::input(DEFAULT_B_FRAMES))
                .withFields({C2F(mActualInputDelay, value).inRange(0, MAX_B_FRAMES)})
                .calculatedAs(InputDelaySetter, mGop)
                .build());

        addParameter(
                DefineParam(mFrameRate, C2_PARAMKEY_FRAME_RATE)
                .withDefault(new C2StreamFrameRateInfo::output(0u, 30.))
                // TODO: More restriction?
                .withFields({C2F(mFrameRate, value).greaterThan(0.)})
                .withSetter(Setter<decltype(*mFrameRate)>::StrictValueWithNoDeps)
                .build());

        addParameter(
                DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(4096, 12000000)})
                .withSetter(BitrateSetter)
                .build());

        addParameter(
                DefineParam(mIntraRefresh, C2_PARAMKEY_INTRA_REFRESH)
                .withDefault(new C2StreamIntraRefreshTuning::output(
                        0u, C2Config::INTRA_REFRESH_DISABLED, 0.))
                .withFields({
                    C2F(mIntraRefresh, mode).oneOf({
                        C2Config::INTRA_REFRESH_DISABLED, C2Config::INTRA_REFRESH_ARBITRARY }),
                    C2F(mIntraRefresh, period).any()
                })
                .withSetter(IntraRefreshSetter)
                .build());

        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::output(
                        0u, PROFILE_AVC_CONSTRAINED_BASELINE, LEVEL_AVC_4_1))
                .withFields({
                    C2F(mProfileLevel, profile).oneOf({
                        PROFILE_AVC_BASELINE,
                        PROFILE_AVC_CONSTRAINED_BASELINE,
                        PROFILE_AVC_MAIN,
                    }),
                    C2F(mProfileLevel, level).oneOf({
                        LEVEL_AVC_1,
                        LEVEL_AVC_1B,
                        LEVEL_AVC_1_1,
                        LEVEL_AVC_1_2,
                        LEVEL_AVC_1_3,
                        LEVEL_AVC_2,
                        LEVEL_AVC_2_1,
                        LEVEL_AVC_2_2,
                        LEVEL_AVC_3,
                        LEVEL_AVC_3_1,
                        LEVEL_AVC_3_2,
                        LEVEL_AVC_4,
                        LEVEL_AVC_4_1,
                        LEVEL_AVC_4_2,
                        LEVEL_AVC_5,
                    }),
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
                .withDefault(new C2StreamSyncFrameIntervalTuning::output(0u, 1000000))
                .withFields({C2F(mSyncFramePeriod, value).any()})
                .withSetter(Setter<decltype(*mSyncFramePeriod)>::StrictValueWithNoDeps)
                .build());
    }

    static C2R InputDelaySetter(
            bool mayBlock,
            C2P<C2PortActualDelayTuning::input> &me,
            const C2P<C2StreamGopTuning::output> &gop) {
        (void)mayBlock;
        uint32_t maxBframes = 0;
        ParseGop(gop.v, nullptr, nullptr, &maxBframes);
        me.set().value = maxBframes;
        return C2R::Ok();
    }

    static C2R BitrateSetter(bool mayBlock, C2P<C2StreamBitrateInfo::output> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (me.v.value <= 4096) {
            me.set().value = 4096;
        }
        return res;
    }

    static C2R SizeSetter(bool mayBlock, const C2P<C2StreamPictureSizeInfo::input> &oldMe,
                          C2P<C2StreamPictureSizeInfo::input> &me) {
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
            me.set().profile = PROFILE_AVC_CONSTRAINED_BASELINE;
        }

        struct LevelLimits {
            C2Config::level_t level;
            float mbsPerSec;
            uint64_t mbs;
            uint32_t bitrate;
        };
        constexpr LevelLimits kLimits[] = {
            { LEVEL_AVC_1,     1485,    99,     64000 },
            // Decoder does not properly handle level 1b.
            // { LEVEL_AVC_1B,    1485,   99,   128000 },
            { LEVEL_AVC_1_1,   3000,   396,    192000 },
            { LEVEL_AVC_1_2,   6000,   396,    384000 },
            { LEVEL_AVC_1_3,  11880,   396,    768000 },
            { LEVEL_AVC_2,    11880,   396,   2000000 },
            { LEVEL_AVC_2_1,  19800,   792,   4000000 },
            { LEVEL_AVC_2_2,  20250,  1620,   4000000 },
            { LEVEL_AVC_3,    40500,  1620,  10000000 },
            { LEVEL_AVC_3_1, 108000,  3600,  14000000 },
            { LEVEL_AVC_3_2, 216000,  5120,  20000000 },
            { LEVEL_AVC_4,   245760,  8192,  20000000 },
            { LEVEL_AVC_4_1, 245760,  8192,  50000000 },
            { LEVEL_AVC_4_2, 522240,  8704,  50000000 },
            { LEVEL_AVC_5,   589824, 22080, 135000000 },
        };

        uint64_t mbs = uint64_t((size.v.width + 15) / 16) * ((size.v.height + 15) / 16);
        float mbsPerSec = float(mbs) * frameRate.v.value;

        // Check if the supplied level meets the MB / bitrate requirements. If
        // not, update the level with the lowest level meeting the requirements.

        bool found = false;
        // By default needsUpdate = false in case the supplied level does meet
        // the requirements. For Level 1b, we want to update the level anyway,
        // so we set it to true in that case.
        bool needsUpdate = (me.v.level == LEVEL_AVC_1B);
        for (const LevelLimits &limit : kLimits) {
            if (mbs <= limit.mbs && mbsPerSec <= limit.mbsPerSec &&
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
            me.set().level = LEVEL_AVC_5;
        }

        return C2R::Ok();
    }

    static C2R IntraRefreshSetter(bool mayBlock, C2P<C2StreamIntraRefreshTuning::output> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (me.v.period < 1) {
            me.set().mode = C2Config::INTRA_REFRESH_DISABLED;
            me.set().period = 0;
        } else {
            // only support arbitrary mode (cyclic in our case)
            me.set().mode = C2Config::INTRA_REFRESH_ARBITRARY;
        }
        return res;
    }

    static C2R GopSetter(bool mayBlock, C2P<C2StreamGopTuning::output> &me) {
        (void)mayBlock;
        for (size_t i = 0; i < me.v.flexCount(); ++i) {
            const C2GopLayerStruct &layer = me.v.m.values[0];
            if (layer.type_ == C2Config::picture_type_t(P_FRAME | B_FRAME)
                    && layer.count > MAX_B_FRAMES) {
                me.set().m.values[i].count = MAX_B_FRAMES;
            }
        }
        return C2R::Ok();
    }

    IV_PROFILE_T getProfile_l() const {
        switch (mProfileLevel->profile) {
        case PROFILE_AVC_CONSTRAINED_BASELINE:  [[fallthrough]];
        case PROFILE_AVC_BASELINE: return IV_PROFILE_BASE;
        case PROFILE_AVC_MAIN:     return IV_PROFILE_MAIN;
        default:
            ALOGD("Unrecognized profile: %x", mProfileLevel->profile);
            return IV_PROFILE_DEFAULT;
        }
    }

    UWORD32 getLevel_l() const {
        struct Level {
            C2Config::level_t c2Level;
            UWORD32 avcLevel;
        };
        constexpr Level levels[] = {
            { LEVEL_AVC_1,   10 },
            { LEVEL_AVC_1B,   9 },
            { LEVEL_AVC_1_1, 11 },
            { LEVEL_AVC_1_2, 12 },
            { LEVEL_AVC_1_3, 13 },
            { LEVEL_AVC_2,   20 },
            { LEVEL_AVC_2_1, 21 },
            { LEVEL_AVC_2_2, 22 },
            { LEVEL_AVC_3,   30 },
            { LEVEL_AVC_3_1, 31 },
            { LEVEL_AVC_3_2, 32 },
            { LEVEL_AVC_4,   40 },
            { LEVEL_AVC_4_1, 41 },
            { LEVEL_AVC_4_2, 42 },
            { LEVEL_AVC_5,   50 },
        };
        for (const Level &level : levels) {
            if (mProfileLevel->level == level.c2Level) {
                return level.avcLevel;
            }
        }
        ALOGD("Unrecognized level: %x", mProfileLevel->level);
        return 41;
    }

    uint32_t getSyncFramePeriod_l() const {
        if (mSyncFramePeriod->value < 0 || mSyncFramePeriod->value == INT64_MAX) {
            return 0;
        }
        double period = mSyncFramePeriod->value / 1e6 * mFrameRate->value;
        return (uint32_t)c2_max(c2_min(period + 0.5, double(UINT32_MAX)), 1.);
    }

    // unsafe getters
    std::shared_ptr<C2StreamPictureSizeInfo::input> getSize_l() const { return mSize; }
    std::shared_ptr<C2StreamIntraRefreshTuning::output> getIntraRefresh_l() const { return mIntraRefresh; }
    std::shared_ptr<C2StreamFrameRateInfo::output> getFrameRate_l() const { return mFrameRate; }
    std::shared_ptr<C2StreamBitrateInfo::output> getBitrate_l() const { return mBitrate; }
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> getRequestSync_l() const { return mRequestSync; }
    std::shared_ptr<C2StreamGopTuning::output> getGop_l() const { return mGop; }

private:
    std::shared_ptr<C2StreamUsageTuning::input> mUsage;
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> mRequestSync;
    std::shared_ptr<C2StreamIntraRefreshTuning::output> mIntraRefresh;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamProfileLevelInfo::output> mProfileLevel;
    std::shared_ptr<C2StreamSyncFrameIntervalTuning::output> mSyncFramePeriod;
    std::shared_ptr<C2StreamGopTuning::output> mGop;
};

#define ive_api_function  ih264e_api_function

namespace {

// From external/libavc/encoder/ih264e_bitstream.h
constexpr uint32_t MIN_STREAM_SIZE = 0x800;

static size_t GetCPUCoreCount() {
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

}  // namespace

C2SoftAvcEnc::C2SoftAvcEnc(
        const char *name, c2_node_id_t id, const std::shared_ptr<IntfImpl> &intfImpl)
    : SimpleC2Component(std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mIvVideoColorFormat(IV_YUV_420P),
      mAVCEncProfile(IV_PROFILE_BASE),
      mAVCEncLevel(41),
      mStarted(false),
      mSawInputEOS(false),
      mSignalledError(false),
      mCodecCtx(nullptr),
      mOutBlock(nullptr),
      // TODO: output buffer size
      mOutBufferSize(524288) {

    // If dump is enabled, then open create an empty file
    GENERATE_FILE_NAMES();
    CREATE_DUMP_FILE(mInFile);
    CREATE_DUMP_FILE(mOutFile);

    initEncParams();
}

C2SoftAvcEnc::~C2SoftAvcEnc() {
    onRelease();
}

c2_status_t C2SoftAvcEnc::onInit() {
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::onStop() {
    return C2_OK;
}

void C2SoftAvcEnc::onReset() {
    // TODO: use IVE_CMD_CTL_RESET?
    releaseEncoder();
    if (mOutBlock) {
        mOutBlock.reset();
    }
    initEncParams();
}

void C2SoftAvcEnc::onRelease() {
    releaseEncoder();
    if (mOutBlock) {
        mOutBlock.reset();
    }
}

c2_status_t C2SoftAvcEnc::onFlush_sm() {
    // TODO: use IVE_CMD_CTL_FLUSH?
    return C2_OK;
}

void  C2SoftAvcEnc::initEncParams() {
    mCodecCtx = nullptr;
    mMemRecords = nullptr;
    mNumMemRecords = DEFAULT_MEM_REC_CNT;
    mHeaderGenerated = 0;
    mNumCores = GetCPUCoreCount();
    mArch = DEFAULT_ARCH;
    mSliceMode = DEFAULT_SLICE_MODE;
    mSliceParam = DEFAULT_SLICE_PARAM;
    mHalfPelEnable = DEFAULT_HPEL;
    mIInterval = DEFAULT_I_INTERVAL;
    mIDRInterval = DEFAULT_IDR_INTERVAL;
    mDisableDeblkLevel = DEFAULT_DISABLE_DEBLK_LEVEL;
    mEnableFastSad = DEFAULT_ENABLE_FAST_SAD;
    mEnableAltRef = DEFAULT_ENABLE_ALT_REF;
    mEncSpeed = DEFAULT_ENC_SPEED;
    mIntra4x4 = DEFAULT_INTRA4x4;
    mConstrainedIntraFlag = DEFAULT_CONSTRAINED_INTRA;
    mPSNREnable = DEFAULT_PSNR_ENABLE;
    mReconEnable = DEFAULT_RECON_ENABLE;
    mEntropyMode = DEFAULT_ENTROPY_MODE;
    mBframes = DEFAULT_B_FRAMES;

    gettimeofday(&mTimeStart, nullptr);
    gettimeofday(&mTimeEnd, nullptr);
}

c2_status_t C2SoftAvcEnc::setDimensions() {
    ive_ctl_set_dimensions_ip_t s_dimensions_ip;
    ive_ctl_set_dimensions_op_t s_dimensions_op;
    IV_STATUS_T status;

    s_dimensions_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_dimensions_ip.e_sub_cmd = IVE_CMD_CTL_SET_DIMENSIONS;
    s_dimensions_ip.u4_ht = mSize->height;
    s_dimensions_ip.u4_wd = mSize->width;

    s_dimensions_ip.u4_timestamp_high = -1;
    s_dimensions_ip.u4_timestamp_low = -1;

    s_dimensions_ip.u4_size = sizeof(ive_ctl_set_dimensions_ip_t);
    s_dimensions_op.u4_size = sizeof(ive_ctl_set_dimensions_op_t);

    status = ive_api_function(mCodecCtx, &s_dimensions_ip, &s_dimensions_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set frame dimensions = 0x%x\n",
                s_dimensions_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setNumCores() {
    IV_STATUS_T status;
    ive_ctl_set_num_cores_ip_t s_num_cores_ip;
    ive_ctl_set_num_cores_op_t s_num_cores_op;
    s_num_cores_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_num_cores_ip.e_sub_cmd = IVE_CMD_CTL_SET_NUM_CORES;
    s_num_cores_ip.u4_num_cores = MIN(mNumCores, CODEC_MAX_CORES);
    s_num_cores_ip.u4_timestamp_high = -1;
    s_num_cores_ip.u4_timestamp_low = -1;
    s_num_cores_ip.u4_size = sizeof(ive_ctl_set_num_cores_ip_t);

    s_num_cores_op.u4_size = sizeof(ive_ctl_set_num_cores_op_t);

    status = ive_api_function(
            mCodecCtx, (void *) &s_num_cores_ip, (void *) &s_num_cores_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set processor params = 0x%x\n",
                s_num_cores_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setFrameRate() {
    ive_ctl_set_frame_rate_ip_t s_frame_rate_ip;
    ive_ctl_set_frame_rate_op_t s_frame_rate_op;
    IV_STATUS_T status;

    s_frame_rate_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_frame_rate_ip.e_sub_cmd = IVE_CMD_CTL_SET_FRAMERATE;

    s_frame_rate_ip.u4_src_frame_rate = mFrameRate->value + 0.5;
    s_frame_rate_ip.u4_tgt_frame_rate = mFrameRate->value + 0.5;

    s_frame_rate_ip.u4_timestamp_high = -1;
    s_frame_rate_ip.u4_timestamp_low = -1;

    s_frame_rate_ip.u4_size = sizeof(ive_ctl_set_frame_rate_ip_t);
    s_frame_rate_op.u4_size = sizeof(ive_ctl_set_frame_rate_op_t);

    status = ive_api_function(mCodecCtx, &s_frame_rate_ip, &s_frame_rate_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set frame rate = 0x%x\n",
                s_frame_rate_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setIpeParams() {
    ive_ctl_set_ipe_params_ip_t s_ipe_params_ip;
    ive_ctl_set_ipe_params_op_t s_ipe_params_op;
    IV_STATUS_T status;

    s_ipe_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_ipe_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_IPE_PARAMS;

    s_ipe_params_ip.u4_enable_intra_4x4 = mIntra4x4;
    s_ipe_params_ip.u4_enc_speed_preset = mEncSpeed;
    s_ipe_params_ip.u4_constrained_intra_pred = mConstrainedIntraFlag;

    s_ipe_params_ip.u4_timestamp_high = -1;
    s_ipe_params_ip.u4_timestamp_low = -1;

    s_ipe_params_ip.u4_size = sizeof(ive_ctl_set_ipe_params_ip_t);
    s_ipe_params_op.u4_size = sizeof(ive_ctl_set_ipe_params_op_t);

    status = ive_api_function(mCodecCtx, &s_ipe_params_ip, &s_ipe_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set ipe params = 0x%x\n",
                s_ipe_params_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setBitRate() {
    ive_ctl_set_bitrate_ip_t s_bitrate_ip;
    ive_ctl_set_bitrate_op_t s_bitrate_op;
    IV_STATUS_T status;

    s_bitrate_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_bitrate_ip.e_sub_cmd = IVE_CMD_CTL_SET_BITRATE;

    s_bitrate_ip.u4_target_bitrate = mBitrate->value;

    s_bitrate_ip.u4_timestamp_high = -1;
    s_bitrate_ip.u4_timestamp_low = -1;

    s_bitrate_ip.u4_size = sizeof(ive_ctl_set_bitrate_ip_t);
    s_bitrate_op.u4_size = sizeof(ive_ctl_set_bitrate_op_t);

    status = ive_api_function(mCodecCtx, &s_bitrate_ip, &s_bitrate_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set bit rate = 0x%x\n", s_bitrate_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setFrameType(IV_PICTURE_CODING_TYPE_T e_frame_type) {
    ive_ctl_set_frame_type_ip_t s_frame_type_ip;
    ive_ctl_set_frame_type_op_t s_frame_type_op;
    IV_STATUS_T status;
    s_frame_type_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_frame_type_ip.e_sub_cmd = IVE_CMD_CTL_SET_FRAMETYPE;

    s_frame_type_ip.e_frame_type = e_frame_type;

    s_frame_type_ip.u4_timestamp_high = -1;
    s_frame_type_ip.u4_timestamp_low = -1;

    s_frame_type_ip.u4_size = sizeof(ive_ctl_set_frame_type_ip_t);
    s_frame_type_op.u4_size = sizeof(ive_ctl_set_frame_type_op_t);

    status = ive_api_function(mCodecCtx, &s_frame_type_ip, &s_frame_type_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set frame type = 0x%x\n",
                s_frame_type_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setQp() {
    ive_ctl_set_qp_ip_t s_qp_ip;
    ive_ctl_set_qp_op_t s_qp_op;
    IV_STATUS_T status;

    s_qp_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_qp_ip.e_sub_cmd = IVE_CMD_CTL_SET_QP;

    s_qp_ip.u4_i_qp = DEFAULT_I_QP;
    s_qp_ip.u4_i_qp_max = DEFAULT_QP_MAX;
    s_qp_ip.u4_i_qp_min = DEFAULT_QP_MIN;

    s_qp_ip.u4_p_qp = DEFAULT_P_QP;
    s_qp_ip.u4_p_qp_max = DEFAULT_QP_MAX;
    s_qp_ip.u4_p_qp_min = DEFAULT_QP_MIN;

    s_qp_ip.u4_b_qp = DEFAULT_P_QP;
    s_qp_ip.u4_b_qp_max = DEFAULT_QP_MAX;
    s_qp_ip.u4_b_qp_min = DEFAULT_QP_MIN;

    s_qp_ip.u4_timestamp_high = -1;
    s_qp_ip.u4_timestamp_low = -1;

    s_qp_ip.u4_size = sizeof(ive_ctl_set_qp_ip_t);
    s_qp_op.u4_size = sizeof(ive_ctl_set_qp_op_t);

    status = ive_api_function(mCodecCtx, &s_qp_ip, &s_qp_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set qp 0x%x\n", s_qp_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setEncMode(IVE_ENC_MODE_T e_enc_mode) {
    IV_STATUS_T status;
    ive_ctl_set_enc_mode_ip_t s_enc_mode_ip;
    ive_ctl_set_enc_mode_op_t s_enc_mode_op;

    s_enc_mode_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_enc_mode_ip.e_sub_cmd = IVE_CMD_CTL_SET_ENC_MODE;

    s_enc_mode_ip.e_enc_mode = e_enc_mode;

    s_enc_mode_ip.u4_timestamp_high = -1;
    s_enc_mode_ip.u4_timestamp_low = -1;

    s_enc_mode_ip.u4_size = sizeof(ive_ctl_set_enc_mode_ip_t);
    s_enc_mode_op.u4_size = sizeof(ive_ctl_set_enc_mode_op_t);

    status = ive_api_function(mCodecCtx, &s_enc_mode_ip, &s_enc_mode_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set in header encode mode = 0x%x\n",
                s_enc_mode_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setVbvParams() {
    ive_ctl_set_vbv_params_ip_t s_vbv_ip;
    ive_ctl_set_vbv_params_op_t s_vbv_op;
    IV_STATUS_T status;

    s_vbv_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_vbv_ip.e_sub_cmd = IVE_CMD_CTL_SET_VBV_PARAMS;

    s_vbv_ip.u4_vbv_buf_size = 0;
    s_vbv_ip.u4_vbv_buffer_delay = 1000;

    s_vbv_ip.u4_timestamp_high = -1;
    s_vbv_ip.u4_timestamp_low = -1;

    s_vbv_ip.u4_size = sizeof(ive_ctl_set_vbv_params_ip_t);
    s_vbv_op.u4_size = sizeof(ive_ctl_set_vbv_params_op_t);

    status = ive_api_function(mCodecCtx, &s_vbv_ip, &s_vbv_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set VBV params = 0x%x\n", s_vbv_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setAirParams() {
    ive_ctl_set_air_params_ip_t s_air_ip;
    ive_ctl_set_air_params_op_t s_air_op;
    IV_STATUS_T status;

    s_air_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_air_ip.e_sub_cmd = IVE_CMD_CTL_SET_AIR_PARAMS;

    s_air_ip.e_air_mode =
        (mIntraRefresh->mode == C2Config::INTRA_REFRESH_DISABLED || mIntraRefresh->period < 1)
            ? IVE_AIR_MODE_NONE : IVE_AIR_MODE_CYCLIC;
    s_air_ip.u4_air_refresh_period = mIntraRefresh->period;

    s_air_ip.u4_timestamp_high = -1;
    s_air_ip.u4_timestamp_low = -1;

    s_air_ip.u4_size = sizeof(ive_ctl_set_air_params_ip_t);
    s_air_op.u4_size = sizeof(ive_ctl_set_air_params_op_t);

    status = ive_api_function(mCodecCtx, &s_air_ip, &s_air_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set air params = 0x%x\n", s_air_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setMeParams() {
    IV_STATUS_T status;
    ive_ctl_set_me_params_ip_t s_me_params_ip;
    ive_ctl_set_me_params_op_t s_me_params_op;

    s_me_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_me_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_ME_PARAMS;

    s_me_params_ip.u4_enable_fast_sad = mEnableFastSad;
    s_me_params_ip.u4_enable_alt_ref = mEnableAltRef;

    s_me_params_ip.u4_enable_hpel = mHalfPelEnable;
    s_me_params_ip.u4_enable_qpel = DEFAULT_QPEL;
    s_me_params_ip.u4_me_speed_preset = DEFAULT_ME_SPEED;
    s_me_params_ip.u4_srch_rng_x = DEFAULT_SRCH_RNG_X;
    s_me_params_ip.u4_srch_rng_y = DEFAULT_SRCH_RNG_Y;

    s_me_params_ip.u4_timestamp_high = -1;
    s_me_params_ip.u4_timestamp_low = -1;

    s_me_params_ip.u4_size = sizeof(ive_ctl_set_me_params_ip_t);
    s_me_params_op.u4_size = sizeof(ive_ctl_set_me_params_op_t);

    status = ive_api_function(mCodecCtx, &s_me_params_ip, &s_me_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set me params = 0x%x\n", s_me_params_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setGopParams() {
    IV_STATUS_T status;
    ive_ctl_set_gop_params_ip_t s_gop_params_ip;
    ive_ctl_set_gop_params_op_t s_gop_params_op;

    s_gop_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_gop_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_GOP_PARAMS;

    s_gop_params_ip.u4_i_frm_interval = mIInterval;
    s_gop_params_ip.u4_idr_frm_interval = mIDRInterval;

    s_gop_params_ip.u4_timestamp_high = -1;
    s_gop_params_ip.u4_timestamp_low = -1;

    s_gop_params_ip.u4_size = sizeof(ive_ctl_set_gop_params_ip_t);
    s_gop_params_op.u4_size = sizeof(ive_ctl_set_gop_params_op_t);

    status = ive_api_function(mCodecCtx, &s_gop_params_ip, &s_gop_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set GOP params = 0x%x\n",
                s_gop_params_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setProfileParams() {
    IntfImpl::Lock lock = mIntf->lock();

    IV_STATUS_T status;
    ive_ctl_set_profile_params_ip_t s_profile_params_ip;
    ive_ctl_set_profile_params_op_t s_profile_params_op;

    s_profile_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_profile_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_PROFILE_PARAMS;

    s_profile_params_ip.e_profile = mIntf->getProfile_l();
    if (s_profile_params_ip.e_profile == IV_PROFILE_BASE) {
        s_profile_params_ip.u4_entropy_coding_mode = 0;
    } else {
        s_profile_params_ip.u4_entropy_coding_mode = 1;
    }
    s_profile_params_ip.u4_timestamp_high = -1;
    s_profile_params_ip.u4_timestamp_low = -1;

    s_profile_params_ip.u4_size = sizeof(ive_ctl_set_profile_params_ip_t);
    s_profile_params_op.u4_size = sizeof(ive_ctl_set_profile_params_op_t);
    lock.unlock();

    status = ive_api_function(mCodecCtx, &s_profile_params_ip, &s_profile_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set profile params = 0x%x\n",
                s_profile_params_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setDeblockParams() {
    IV_STATUS_T status;
    ive_ctl_set_deblock_params_ip_t s_deblock_params_ip;
    ive_ctl_set_deblock_params_op_t s_deblock_params_op;

    s_deblock_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_deblock_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_DEBLOCK_PARAMS;

    s_deblock_params_ip.u4_disable_deblock_level = mDisableDeblkLevel;

    s_deblock_params_ip.u4_timestamp_high = -1;
    s_deblock_params_ip.u4_timestamp_low = -1;

    s_deblock_params_ip.u4_size = sizeof(ive_ctl_set_deblock_params_ip_t);
    s_deblock_params_op.u4_size = sizeof(ive_ctl_set_deblock_params_op_t);

    status = ive_api_function(mCodecCtx, &s_deblock_params_ip, &s_deblock_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to enable/disable deblock params = 0x%x\n",
                s_deblock_params_op.u4_error_code);
        return C2_CORRUPTED;
    }
    return C2_OK;
}

void C2SoftAvcEnc::logVersion() {
    ive_ctl_getversioninfo_ip_t s_ctl_ip;
    ive_ctl_getversioninfo_op_t s_ctl_op;
    UWORD8 au1_buf[512];
    IV_STATUS_T status;

    s_ctl_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVE_CMD_CTL_GETVERSION;
    s_ctl_ip.u4_size = sizeof(ive_ctl_getversioninfo_ip_t);
    s_ctl_op.u4_size = sizeof(ive_ctl_getversioninfo_op_t);
    s_ctl_ip.pu1_version = au1_buf;
    s_ctl_ip.u4_version_bufsize = sizeof(au1_buf);

    status = ive_api_function(mCodecCtx, (void *) &s_ctl_ip, (void *) &s_ctl_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in getting version: 0x%x", s_ctl_op.u4_error_code);
    } else {
        ALOGV("Ittiam encoder version: %s", (char *)s_ctl_ip.pu1_version);
    }
    return;
}

c2_status_t C2SoftAvcEnc::initEncoder() {
    IV_STATUS_T status;
    WORD32 level;

    CHECK(!mStarted);

    c2_status_t errType = C2_OK;

    std::shared_ptr<C2StreamGopTuning::output> gop;
    {
        IntfImpl::Lock lock = mIntf->lock();
        mSize = mIntf->getSize_l();
        mBitrate = mIntf->getBitrate_l();
        mFrameRate = mIntf->getFrameRate_l();
        mIntraRefresh = mIntf->getIntraRefresh_l();
        mAVCEncLevel = mIntf->getLevel_l();
        mIInterval = mIntf->getSyncFramePeriod_l();
        mIDRInterval = mIntf->getSyncFramePeriod_l();
        gop = mIntf->getGop_l();
    }
    if (gop && gop->flexCount() > 0) {
        uint32_t syncInterval = 1;
        uint32_t iInterval = 1;
        uint32_t maxBframes = 0;
        ParseGop(*gop, &syncInterval, &iInterval, &maxBframes);
        if (syncInterval > 0) {
            ALOGD("Updating IDR interval from GOP: old %u new %u", mIDRInterval, syncInterval);
            mIDRInterval = syncInterval;
        }
        if (iInterval > 0) {
            ALOGD("Updating I interval from GOP: old %u new %u", mIInterval, iInterval);
            mIInterval = iInterval;
        }
        if (mBframes != maxBframes) {
            ALOGD("Updating max B frames from GOP: old %u new %u", mBframes, maxBframes);
            mBframes = maxBframes;
        }
    }
    uint32_t width = mSize->width;
    uint32_t height = mSize->height;

    mStride = width;

    // TODO
    mIvVideoColorFormat = IV_YUV_420P;

    ALOGD("Params width %d height %d level %d colorFormat %d bframes %d", width,
            height, mAVCEncLevel, mIvVideoColorFormat, mBframes);

    /* Getting Number of MemRecords */
    {
        iv_num_mem_rec_ip_t s_num_mem_rec_ip;
        iv_num_mem_rec_op_t s_num_mem_rec_op;

        s_num_mem_rec_ip.u4_size = sizeof(iv_num_mem_rec_ip_t);
        s_num_mem_rec_op.u4_size = sizeof(iv_num_mem_rec_op_t);

        s_num_mem_rec_ip.e_cmd = IV_CMD_GET_NUM_MEM_REC;

        status = ive_api_function(nullptr, &s_num_mem_rec_ip, &s_num_mem_rec_op);

        if (status != IV_SUCCESS) {
            ALOGE("Get number of memory records failed = 0x%x\n",
                    s_num_mem_rec_op.u4_error_code);
            return C2_CORRUPTED;
        }

        mNumMemRecords = s_num_mem_rec_op.u4_num_mem_rec;
    }

    /* Allocate array to hold memory records */
    if (mNumMemRecords > SIZE_MAX / sizeof(iv_mem_rec_t)) {
        ALOGE("requested memory size is too big.");
        return C2_CORRUPTED;
    }
    mMemRecords = (iv_mem_rec_t *)malloc(mNumMemRecords * sizeof(iv_mem_rec_t));
    if (nullptr == mMemRecords) {
        ALOGE("Unable to allocate memory for hold memory records: Size %zu",
                mNumMemRecords * sizeof(iv_mem_rec_t));
        mSignalledError = true;
        return C2_CORRUPTED;
    }

    {
        iv_mem_rec_t *ps_mem_rec;
        ps_mem_rec = mMemRecords;
        for (size_t i = 0; i < mNumMemRecords; i++) {
            ps_mem_rec->u4_size = sizeof(iv_mem_rec_t);
            ps_mem_rec->pv_base = nullptr;
            ps_mem_rec->u4_mem_size = 0;
            ps_mem_rec->u4_mem_alignment = 0;
            ps_mem_rec->e_mem_type = IV_NA_MEM_TYPE;

            ps_mem_rec++;
        }
    }

    /* Getting MemRecords Attributes */
    {
        iv_fill_mem_rec_ip_t s_fill_mem_rec_ip;
        iv_fill_mem_rec_op_t s_fill_mem_rec_op;

        s_fill_mem_rec_ip.u4_size = sizeof(iv_fill_mem_rec_ip_t);
        s_fill_mem_rec_op.u4_size = sizeof(iv_fill_mem_rec_op_t);

        s_fill_mem_rec_ip.e_cmd = IV_CMD_FILL_NUM_MEM_REC;
        s_fill_mem_rec_ip.ps_mem_rec = mMemRecords;
        s_fill_mem_rec_ip.u4_num_mem_rec = mNumMemRecords;
        s_fill_mem_rec_ip.u4_max_wd = width;
        s_fill_mem_rec_ip.u4_max_ht = height;
        s_fill_mem_rec_ip.u4_max_level = mAVCEncLevel;
        s_fill_mem_rec_ip.e_color_format = DEFAULT_INP_COLOR_FORMAT;
        s_fill_mem_rec_ip.u4_max_ref_cnt = DEFAULT_MAX_REF_FRM;
        s_fill_mem_rec_ip.u4_max_reorder_cnt = DEFAULT_MAX_REORDER_FRM;
        s_fill_mem_rec_ip.u4_max_srch_rng_x = DEFAULT_MAX_SRCH_RANGE_X;
        s_fill_mem_rec_ip.u4_max_srch_rng_y = DEFAULT_MAX_SRCH_RANGE_Y;

        status = ive_api_function(nullptr, &s_fill_mem_rec_ip, &s_fill_mem_rec_op);

        if (status != IV_SUCCESS) {
            ALOGE("Fill memory records failed = 0x%x\n",
                    s_fill_mem_rec_op.u4_error_code);
            return C2_CORRUPTED;
        }
    }

    /* Allocating Memory for Mem Records */
    {
        WORD32 total_size;
        iv_mem_rec_t *ps_mem_rec;
        total_size = 0;
        ps_mem_rec = mMemRecords;

        for (size_t i = 0; i < mNumMemRecords; i++) {
            ps_mem_rec->pv_base = ive_aligned_malloc(
                    ps_mem_rec->u4_mem_alignment, ps_mem_rec->u4_mem_size);
            if (ps_mem_rec->pv_base == nullptr) {
                ALOGE("Allocation failure for mem record id %zu size %u\n", i,
                        ps_mem_rec->u4_mem_size);
                return C2_CORRUPTED;

            }
            total_size += ps_mem_rec->u4_mem_size;

            ps_mem_rec++;
        }
    }

    /* Codec Instance Creation */
    {
        ive_init_ip_t s_init_ip;
        ive_init_op_t s_init_op;

        mCodecCtx = (iv_obj_t *)mMemRecords[0].pv_base;
        mCodecCtx->u4_size = sizeof(iv_obj_t);
        mCodecCtx->pv_fxns = (void *)ive_api_function;

        s_init_ip.u4_size = sizeof(ive_init_ip_t);
        s_init_op.u4_size = sizeof(ive_init_op_t);

        s_init_ip.e_cmd = IV_CMD_INIT;
        s_init_ip.u4_num_mem_rec = mNumMemRecords;
        s_init_ip.ps_mem_rec = mMemRecords;
        s_init_ip.u4_max_wd = width;
        s_init_ip.u4_max_ht = height;
        s_init_ip.u4_max_ref_cnt = DEFAULT_MAX_REF_FRM;
        s_init_ip.u4_max_reorder_cnt = DEFAULT_MAX_REORDER_FRM;
        s_init_ip.u4_max_level = mAVCEncLevel;
        s_init_ip.e_inp_color_fmt = mIvVideoColorFormat;

        if (mReconEnable || mPSNREnable) {
            s_init_ip.u4_enable_recon = 1;
        } else {
            s_init_ip.u4_enable_recon = 0;
        }
        s_init_ip.e_recon_color_fmt = DEFAULT_RECON_COLOR_FORMAT;
        s_init_ip.e_rc_mode = DEFAULT_RC_MODE;
        s_init_ip.u4_max_framerate = DEFAULT_MAX_FRAMERATE;
        s_init_ip.u4_max_bitrate = DEFAULT_MAX_BITRATE;
        s_init_ip.u4_num_bframes = mBframes;
        s_init_ip.e_content_type = IV_PROGRESSIVE;
        s_init_ip.u4_max_srch_rng_x = DEFAULT_MAX_SRCH_RANGE_X;
        s_init_ip.u4_max_srch_rng_y = DEFAULT_MAX_SRCH_RANGE_Y;
        s_init_ip.e_slice_mode = mSliceMode;
        s_init_ip.u4_slice_param = mSliceParam;
        s_init_ip.e_arch = mArch;
        s_init_ip.e_soc = DEFAULT_SOC;

        status = ive_api_function(mCodecCtx, &s_init_ip, &s_init_op);

        if (status != IV_SUCCESS) {
            ALOGE("Init encoder failed = 0x%x\n", s_init_op.u4_error_code);
            return C2_CORRUPTED;
        }
    }

    /* Get Codec Version */
    logVersion();

    /* set processor details */
    setNumCores();

    /* Video control Set Frame dimensions */
    setDimensions();

    /* Video control Set Frame rates */
    setFrameRate();

    /* Video control Set IPE Params */
    setIpeParams();

    /* Video control Set Bitrate */
    setBitRate();

    /* Video control Set QP */
    setQp();

    /* Video control Set AIR params */
    setAirParams();

    /* Video control Set VBV params */
    setVbvParams();

    /* Video control Set Motion estimation params */
    setMeParams();

    /* Video control Set GOP params */
    setGopParams();

    /* Video control Set Deblock params */
    setDeblockParams();

    /* Video control Set Profile params */
    setProfileParams();

    /* Video control Set in Encode header mode */
    setEncMode(IVE_ENC_MODE_HEADER);

    ALOGV("init_codec successfull");

    mSpsPpsHeaderReceived = false;
    mStarted = true;

    return C2_OK;
}

c2_status_t C2SoftAvcEnc::releaseEncoder() {
    IV_STATUS_T status = IV_SUCCESS;
    iv_retrieve_mem_rec_ip_t s_retrieve_mem_ip;
    iv_retrieve_mem_rec_op_t s_retrieve_mem_op;
    iv_mem_rec_t *ps_mem_rec;

    if (!mStarted) {
        return C2_OK;
    }

    s_retrieve_mem_ip.u4_size = sizeof(iv_retrieve_mem_rec_ip_t);
    s_retrieve_mem_op.u4_size = sizeof(iv_retrieve_mem_rec_op_t);
    s_retrieve_mem_ip.e_cmd = IV_CMD_RETRIEVE_MEMREC;
    s_retrieve_mem_ip.ps_mem_rec = mMemRecords;

    status = ive_api_function(mCodecCtx, &s_retrieve_mem_ip, &s_retrieve_mem_op);

    if (status != IV_SUCCESS) {
        ALOGE("Unable to retrieve memory records = 0x%x\n",
                s_retrieve_mem_op.u4_error_code);
        return C2_CORRUPTED;
    }

    /* Free memory records */
    ps_mem_rec = mMemRecords;
    for (size_t i = 0; i < s_retrieve_mem_op.u4_num_mem_rec_filled; i++) {
        if (ps_mem_rec) ive_aligned_free(ps_mem_rec->pv_base);
        else {
            ALOGE("memory record is null.");
            return C2_CORRUPTED;
        }
        ps_mem_rec++;
    }

    if (mMemRecords) free(mMemRecords);

    // clear other pointers into the space being free()d
    mCodecCtx = nullptr;

    mStarted = false;

    return C2_OK;
}

c2_status_t C2SoftAvcEnc::setEncodeArgs(
        ive_video_encode_ip_t *ps_encode_ip,
        ive_video_encode_op_t *ps_encode_op,
        const C2GraphicView *const input,
        uint8_t *base,
        uint32_t capacity,
        uint64_t workIndex) {
    iv_raw_buf_t *ps_inp_raw_buf;
    memset(ps_encode_ip, 0, sizeof(*ps_encode_ip));
    memset(ps_encode_op, 0, sizeof(*ps_encode_op));

    ps_inp_raw_buf = &ps_encode_ip->s_inp_buf;
    ps_encode_ip->s_out_buf.pv_buf = base;
    ps_encode_ip->s_out_buf.u4_bytes = 0;
    ps_encode_ip->s_out_buf.u4_bufsize = capacity;
    ps_encode_ip->u4_size = sizeof(ive_video_encode_ip_t);
    ps_encode_op->u4_size = sizeof(ive_video_encode_op_t);

    ps_encode_ip->e_cmd = IVE_CMD_VIDEO_ENCODE;
    ps_encode_ip->pv_bufs = nullptr;
    ps_encode_ip->pv_mb_info = nullptr;
    ps_encode_ip->pv_pic_info = nullptr;
    ps_encode_ip->u4_mb_info_type = 0;
    ps_encode_ip->u4_pic_info_type = 0;
    ps_encode_ip->u4_is_last = 0;
    ps_encode_ip->u4_timestamp_high = workIndex >> 32;
    ps_encode_ip->u4_timestamp_low = workIndex & 0xFFFFFFFF;
    ps_encode_op->s_out_buf.pv_buf = nullptr;

    /* Initialize color formats */
    memset(ps_inp_raw_buf, 0, sizeof(iv_raw_buf_t));
    ps_inp_raw_buf->u4_size = sizeof(iv_raw_buf_t);
    ps_inp_raw_buf->e_color_fmt = mIvVideoColorFormat;
    if (input == nullptr) {
        if (mSawInputEOS) {
            ps_encode_ip->u4_is_last = 1;
        }
        return C2_OK;
    }

    if (input->width() < mSize->width ||
        input->height() < mSize->height) {
        /* Expect width height to be configured */
        ALOGW("unexpected Capacity Aspect %d(%d) x %d(%d)", input->width(),
              mSize->width, input->height(), mSize->height);
        return C2_BAD_VALUE;
    }
    ALOGV("width = %d, height = %d", input->width(), input->height());
    const C2PlanarLayout &layout = input->layout();
    uint8_t *yPlane = const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_Y]);
    uint8_t *uPlane = const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_U]);
    uint8_t *vPlane = const_cast<uint8_t *>(input->data()[C2PlanarLayout::PLANE_V]);
    int32_t yStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
    int32_t uStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;
    int32_t vStride = layout.planes[C2PlanarLayout::PLANE_V].rowInc;

    uint32_t width = mSize->width;
    uint32_t height = mSize->height;
    // width and height are always even (as block size is 16x16)
    CHECK_EQ((width & 1u), 0u);
    CHECK_EQ((height & 1u), 0u);
    size_t yPlaneSize = width * height;

    switch (layout.type) {
        case C2PlanarLayout::TYPE_RGB:
            [[fallthrough]];
        case C2PlanarLayout::TYPE_RGBA: {
            ALOGV("yPlaneSize = %zu", yPlaneSize);
            MemoryBlock conversionBuffer = mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(), conversionBuffer);
            yPlane = conversionBuffer.data();
            uPlane = yPlane + yPlaneSize;
            vPlane = uPlane + yPlaneSize / 4;
            yStride = width;
            uStride = vStride = yStride / 2;
            ConvertRGBToPlanarYUV(yPlane, yStride, height, conversionBuffer.size(), *input);
            break;
        }
        case C2PlanarLayout::TYPE_YUV: {
            if (!IsYUV420(*input)) {
                ALOGE("input is not YUV420");
                return C2_BAD_VALUE;
            }

            if (layout.planes[layout.PLANE_Y].colInc == 1
                    && layout.planes[layout.PLANE_U].colInc == 1
                    && layout.planes[layout.PLANE_V].colInc == 1
                    && uStride == vStride
                    && yStride == 2 * vStride) {
                // I420 compatible - already set up above
                break;
            }

            // copy to I420
            yStride = width;
            uStride = vStride = yStride / 2;
            MemoryBlock conversionBuffer = mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(), conversionBuffer);
            MediaImage2 img = CreateYUV420PlanarMediaImage2(width, height, yStride, height);
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
        case IV_YUV_420P:
        {
            // input buffer is supposed to be const but Ittiam API wants bare pointer.
            ps_inp_raw_buf->apv_bufs[0] = yPlane;
            ps_inp_raw_buf->apv_bufs[1] = uPlane;
            ps_inp_raw_buf->apv_bufs[2] = vPlane;

            ps_inp_raw_buf->au4_wd[0] = input->width();
            ps_inp_raw_buf->au4_wd[1] = input->width() / 2;
            ps_inp_raw_buf->au4_wd[2] = input->width() / 2;

            ps_inp_raw_buf->au4_ht[0] = input->height();
            ps_inp_raw_buf->au4_ht[1] = input->height() / 2;
            ps_inp_raw_buf->au4_ht[2] = input->height() / 2;

            ps_inp_raw_buf->au4_strd[0] = yStride;
            ps_inp_raw_buf->au4_strd[1] = uStride;
            ps_inp_raw_buf->au4_strd[2] = vStride;
            break;
        }

        case IV_YUV_422ILE:
        {
            // TODO
            // ps_inp_raw_buf->apv_bufs[0] = pu1_buf;
            // ps_inp_raw_buf->au4_wd[0] = mWidth * 2;
            // ps_inp_raw_buf->au4_ht[0] = mHeight;
            // ps_inp_raw_buf->au4_strd[0] = mStride * 2;
            break;
        }

        case IV_YUV_420SP_UV:
        case IV_YUV_420SP_VU:
        default:
        {
            ps_inp_raw_buf->apv_bufs[0] = yPlane;
            ps_inp_raw_buf->apv_bufs[1] = uPlane;

            ps_inp_raw_buf->au4_wd[0] = input->width();
            ps_inp_raw_buf->au4_wd[1] = input->width();

            ps_inp_raw_buf->au4_ht[0] = input->height();
            ps_inp_raw_buf->au4_ht[1] = input->height() / 2;

            ps_inp_raw_buf->au4_strd[0] = yStride;
            ps_inp_raw_buf->au4_strd[1] = uStride;
            break;
        }
    }
    return C2_OK;
}

void C2SoftAvcEnc::finishWork(uint64_t workIndex, const std::unique_ptr<C2Work> &work,
                              ive_video_encode_op_t *ps_encode_op) {
    std::shared_ptr<C2Buffer> buffer =
            createLinearBuffer(mOutBlock, 0, ps_encode_op->s_out_buf.u4_bytes);
    if (IV_IDR_FRAME == ps_encode_op->u4_encoded_frame_type) {
        ALOGV("IDR frame produced");
        buffer->setInfo(std::make_shared<C2StreamPictureTypeMaskInfo::output>(
                0u /* stream id */, C2Config::SYNC_FRAME));
    }
    mOutBlock = nullptr;

    auto fillWork = [buffer](const std::unique_ptr<C2Work> &work) {
        work->worklets.front()->output.flags = (C2FrameData::flags_t)0;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.buffers.push_back(buffer);
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
    };
    if (work && c2_cntr64_t(workIndex) == work->input.ordinal.frameIndex) {
        fillWork(work);
        if (mSawInputEOS) {
            work->worklets.front()->output.flags = C2FrameData::FLAG_END_OF_STREAM;
        }
    } else {
        finish(workIndex, fillWork);
    }
}

void C2SoftAvcEnc::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 0u;
    work->worklets.front()->output.flags = work->input.flags;

    IV_STATUS_T status;
    WORD32 timeDelay = 0;
    WORD32 timeTaken = 0;
    uint64_t workIndex = work->input.ordinal.frameIndex.peekull();

    // Initialize encoder if not already initialized
    if (mCodecCtx == nullptr) {
        if (C2_OK != initEncoder()) {
            ALOGE("Failed to initialize encoder");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
    }
    if (mSignalledError) {
        return;
    }
    // while (!mSawOutputEOS && !outQueue.empty()) {
    c2_status_t error;
    ive_video_encode_ip_t s_encode_ip;
    ive_video_encode_op_t s_encode_op;
    memset(&s_encode_op, 0, sizeof(s_encode_op));

    if (!mSpsPpsHeaderReceived) {
        constexpr uint32_t kHeaderLength = MIN_STREAM_SIZE;
        uint8_t header[kHeaderLength];
        error = setEncodeArgs(
                &s_encode_ip, &s_encode_op, nullptr, header, kHeaderLength, workIndex);
        if (error != C2_OK) {
            ALOGE("setEncodeArgs failed: %d", error);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
        status = ive_api_function(mCodecCtx, &s_encode_ip, &s_encode_op);

        if (IV_SUCCESS != status) {
            ALOGE("Encode header failed = 0x%x\n",
                    s_encode_op.u4_error_code);
            work->workletsProcessed = 1u;
            return;
        } else {
            ALOGV("Bytes Generated in header %d\n",
                    s_encode_op.s_out_buf.u4_bytes);
        }

        mSpsPpsHeaderReceived = true;

        std::unique_ptr<C2StreamInitDataInfo::output> csd =
            C2StreamInitDataInfo::output::AllocUnique(s_encode_op.s_out_buf.u4_bytes, 0u);
        if (!csd) {
            ALOGE("CSD allocation failed");
            mSignalledError = true;
            work->result = C2_NO_MEMORY;
            work->workletsProcessed = 1u;
            return;
        }
        memcpy(csd->m.value, header, s_encode_op.s_out_buf.u4_bytes);
        work->worklets.front()->output.configUpdate.push_back(std::move(csd));

        DUMP_TO_FILE(
                mOutFile, csd->m.value, csd->flexCount());
        if (work->input.buffers.empty()) {
            work->workletsProcessed = 1u;
            return;
        }
    }

    // handle dynamic config parameters
    {
        IntfImpl::Lock lock = mIntf->lock();
        std::shared_ptr<C2StreamIntraRefreshTuning::output> intraRefresh = mIntf->getIntraRefresh_l();
        std::shared_ptr<C2StreamBitrateInfo::output> bitrate = mIntf->getBitrate_l();
        std::shared_ptr<C2StreamRequestSyncFrameTuning::output> requestSync = mIntf->getRequestSync_l();
        lock.unlock();

        if (bitrate != mBitrate) {
            mBitrate = bitrate;
            setBitRate();
        }

        if (intraRefresh != mIntraRefresh) {
            mIntraRefresh = intraRefresh;
            setAirParams();
        }

        if (requestSync != mRequestSync) {
            // we can handle IDR immediately
            if (requestSync->value) {
                // unset request
                C2StreamRequestSyncFrameTuning::output clearSync(0u, C2_FALSE);
                std::vector<std::unique_ptr<C2SettingResult>> failures;
                mIntf->config({ &clearSync }, C2_MAY_BLOCK, &failures);
                ALOGV("Got sync request");
                setFrameType(IV_IDR_FRAME);
            }
            mRequestSync = requestSync;
        }
    }

    if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
        mSawInputEOS = true;
    }

    /* In normal mode, store inputBufferInfo and this will be returned
       when encoder consumes this input */
    // if (!mInputDataIsMeta && (inputBufferInfo != NULL)) {
    //     for (size_t i = 0; i < MAX_INPUT_BUFFER_HEADERS; i++) {
    //         if (NULL == mInputBufferInfo[i]) {
    //             mInputBufferInfo[i] = inputBufferInfo;
    //             break;
    //         }
    //     }
    // }
    std::shared_ptr<const C2GraphicView> view;
    std::shared_ptr<C2Buffer> inputBuffer;
    if (!work->input.buffers.empty()) {
        inputBuffer = work->input.buffers[0];
        view = std::make_shared<const C2GraphicView>(
                inputBuffer->data().graphicBlocks().front().map().get());
        if (view->error() != C2_OK) {
            ALOGE("graphic view map err = %d", view->error());
            work->workletsProcessed = 1u;
            return;
        }
    }

    do {
        if (mSawInputEOS && work->input.buffers.empty()) break;
        if (!mOutBlock) {
            C2MemoryUsage usage = {C2MemoryUsage::CPU_READ,
                                   C2MemoryUsage::CPU_WRITE};
            // TODO: error handling, proper usage, etc.
            c2_status_t err =
                pool->fetchLinearBlock(mOutBufferSize, usage, &mOutBlock);
            if (err != C2_OK) {
                ALOGE("fetch linear block err = %d", err);
                work->result = err;
                work->workletsProcessed = 1u;
                return;
            }
        }
        C2WriteView wView = mOutBlock->map().get();
        if (wView.error() != C2_OK) {
            ALOGE("write view map err = %d", wView.error());
            work->result = wView.error();
            work->workletsProcessed = 1u;
            return;
        }

        error = setEncodeArgs(
                &s_encode_ip, &s_encode_op, view.get(), wView.base(), wView.capacity(), workIndex);
        if (error != C2_OK) {
            ALOGE("setEncodeArgs failed : %d", error);
            mSignalledError = true;
            work->result = error;
            work->workletsProcessed = 1u;
            return;
        }

        // DUMP_TO_FILE(
        //         mInFile, s_encode_ip.s_inp_buf.apv_bufs[0],
        //         (mHeight * mStride * 3 / 2));

        GETTIME(&mTimeStart, nullptr);
        /* Compute time elapsed between end of previous decode()
         * to start of current decode() */
        TIME_DIFF(mTimeEnd, mTimeStart, timeDelay);
        status = ive_api_function(mCodecCtx, &s_encode_ip, &s_encode_op);

        if (IV_SUCCESS != status) {
            if ((s_encode_op.u4_error_code & 0xFF) == IH264E_BITSTREAM_BUFFER_OVERFLOW) {
                // TODO: use IVE_CMD_CTL_GETBUFINFO for proper max input size?
                mOutBufferSize *= 2;
                mOutBlock.reset();
                continue;
            }
            ALOGE("Encode Frame failed = 0x%x\n",
                    s_encode_op.u4_error_code);
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
    } while (IV_SUCCESS != status);

    // Hold input buffer reference
    if (inputBuffer) {
        mBuffers[s_encode_ip.s_inp_buf.apv_bufs[0]] = inputBuffer;
    }

    GETTIME(&mTimeEnd, nullptr);
    /* Compute time taken for decode() */
    TIME_DIFF(mTimeStart, mTimeEnd, timeTaken);

    ALOGV("timeTaken=%6d delay=%6d numBytes=%6d", timeTaken, timeDelay,
            s_encode_op.s_out_buf.u4_bytes);

    void *freed = s_encode_op.s_inp_buf.apv_bufs[0];
    /* If encoder frees up an input buffer, mark it as free */
    if (freed != nullptr) {
        if (mBuffers.count(freed) == 0u) {
            ALOGD("buffer not tracked");
        } else {
            // Release input buffer reference
            mBuffers.erase(freed);
            mConversionBuffersInUse.erase(freed);
        }
    }

    if (s_encode_op.output_present) {
        if (!s_encode_op.s_out_buf.u4_bytes) {
            ALOGE("Error: Output present but bytes generated is zero");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return;
        }
        uint64_t workId = ((uint64_t)s_encode_op.u4_timestamp_high << 32) |
                      s_encode_op.u4_timestamp_low;
        finishWork(workId, work, &s_encode_op);
    }
    if (mSawInputEOS) {
        drainInternal(DRAIN_COMPONENT_WITH_EOS, pool, work);
    }
}

c2_status_t C2SoftAvcEnc::drainInternal(
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
        if (!mOutBlock) {
            C2MemoryUsage usage = {C2MemoryUsage::CPU_READ,
                                   C2MemoryUsage::CPU_WRITE};
            // TODO: error handling, proper usage, etc.
            c2_status_t err =
                pool->fetchLinearBlock(mOutBufferSize, usage, &mOutBlock);
            if (err != C2_OK) {
                ALOGE("fetch linear block err = %d", err);
                work->result = err;
                work->workletsProcessed = 1u;
                return err;
            }
        }
        C2WriteView wView = mOutBlock->map().get();
        if (wView.error()) {
            ALOGE("graphic view map failed %d", wView.error());
            return C2_CORRUPTED;
        }
        ive_video_encode_ip_t s_encode_ip;
        ive_video_encode_op_t s_encode_op;
        if (C2_OK != setEncodeArgs(&s_encode_ip, &s_encode_op, nullptr,
                                   wView.base(), wView.capacity(), 0)) {
            ALOGE("setEncodeArgs failed for drainInternal");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            work->workletsProcessed = 1u;
            return C2_CORRUPTED;
        }
        (void)ive_api_function(mCodecCtx, &s_encode_ip, &s_encode_op);

        void *freed = s_encode_op.s_inp_buf.apv_bufs[0];
        /* If encoder frees up an input buffer, mark it as free */
        if (freed != nullptr) {
            if (mBuffers.count(freed) == 0u) {
                ALOGD("buffer not tracked");
            } else {
                // Release input buffer reference
                mBuffers.erase(freed);
                mConversionBuffersInUse.erase(freed);
            }
        }

        if (s_encode_op.output_present) {
            uint64_t workId = ((uint64_t)s_encode_op.u4_timestamp_high << 32) |
                          s_encode_op.u4_timestamp_low;
            finishWork(workId, work, &s_encode_op);
        } else {
            if (work->workletsProcessed != 1u) {
                work->worklets.front()->output.flags = work->input.flags;
                work->worklets.front()->output.ordinal = work->input.ordinal;
                work->worklets.front()->output.buffers.clear();
                work->workletsProcessed = 1u;
            }
            break;
        }
    }

    return C2_OK;
}

c2_status_t C2SoftAvcEnc::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    return drainInternal(drainMode, pool, nullptr);
}

class C2SoftAvcEncFactory : public C2ComponentFactory {
public:
    C2SoftAvcEncFactory() : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
        GetCodec2PlatformComponentStore()->getParamReflector())) {
    }

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
                new C2SoftAvcEnc(COMPONENT_NAME,
                                 id,
                                 std::make_shared<C2SoftAvcEnc::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
                new SimpleInterface<C2SoftAvcEnc::IntfImpl>(
                        COMPONENT_NAME, id, std::make_shared<C2SoftAvcEnc::IntfImpl>(mHelper)),
                deleter);
        return C2_OK;
    }

    virtual ~C2SoftAvcEncFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftAvcEncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
