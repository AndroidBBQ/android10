/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "SoftAVCEnc"
#include <utils/Log.h>
#include <utils/misc.h>

#include "OMX_Video.h"

#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <OMX_IndexExt.h>
#include <OMX_VideoExt.h>

#include "ih264_typedefs.h"
#include "iv2.h"
#include "ive2.h"
#include "ih264e.h"
#include "SoftAVCEnc.h"

namespace android {

    #define ive_api_function ih264e_api_function

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

struct LevelConversion {
    OMX_VIDEO_AVCLEVELTYPE omxLevel;
    WORD32 avcLevel;
};

static LevelConversion ConversionTable[] = {
    { OMX_VIDEO_AVCLevel1,  10 },
    { OMX_VIDEO_AVCLevel1b, 9  },
    { OMX_VIDEO_AVCLevel11, 11 },
    { OMX_VIDEO_AVCLevel12, 12 },
    { OMX_VIDEO_AVCLevel13, 13 },
    { OMX_VIDEO_AVCLevel2,  20 },
    { OMX_VIDEO_AVCLevel21, 21 },
    { OMX_VIDEO_AVCLevel22, 22 },
    { OMX_VIDEO_AVCLevel3,  30 },
    { OMX_VIDEO_AVCLevel31, 31 },
    { OMX_VIDEO_AVCLevel32, 32 },
    { OMX_VIDEO_AVCLevel4,  40 },
    { OMX_VIDEO_AVCLevel41, 41 },
    { OMX_VIDEO_AVCLevel42, 42 },
    { OMX_VIDEO_AVCLevel5,  50 },
    { OMX_VIDEO_AVCLevel51, 51 },
};

static const CodecProfileLevel kProfileLevels[] = {
    { OMX_VIDEO_AVCProfileConstrainedBaseline, OMX_VIDEO_AVCLevel41 },

    { OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41 },

    { OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41 },
};

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

static status_t ConvertOmxAvcLevelToAvcSpecLevel(
        OMX_VIDEO_AVCLEVELTYPE omxLevel, WORD32 *avcLevel) {
    for (size_t i = 0; i < NELEM(ConversionTable); ++i) {
        if (omxLevel == ConversionTable[i].omxLevel) {
            *avcLevel = ConversionTable[i].avcLevel;
            return OK;
        }
    }

    ALOGE("ConvertOmxAvcLevelToAvcSpecLevel: %d level not supported",
            (int32_t)omxLevel);

    return BAD_VALUE;
}

static status_t ConvertAvcSpecLevelToOmxAvcLevel(
        WORD32 avcLevel, OMX_VIDEO_AVCLEVELTYPE *omxLevel) {
    for (size_t i = 0; i < NELEM(ConversionTable); ++i) {
        if (avcLevel == ConversionTable[i].avcLevel) {
            *omxLevel = ConversionTable[i].omxLevel;
            return OK;
        }
    }

    ALOGE("ConvertAvcSpecLevelToOmxAvcLevel: %d level not supported",
            (int32_t)avcLevel);

    return BAD_VALUE;
}


SoftAVC::SoftAVC(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SoftVideoEncoderOMXComponent(
            name, "video_encoder.avc", OMX_VIDEO_CodingAVC,
            kProfileLevels, NELEM(kProfileLevels),
            176 /* width */, 144 /* height */,
            callbacks, appData, component),
      mUpdateFlag(0),
      mIvVideoColorFormat(IV_YUV_420P),
      mAVCEncProfile(IV_PROFILE_BASE),
      mAVCEncLevel(41),
      mStarted(false),
      mSawInputEOS(false),
      mSawOutputEOS(false),
      mSignalledError(false),
      mCodecCtx(NULL) {

    initPorts(kNumBuffers, kNumBuffers, ((mWidth * mHeight * 3) >> 1),
            MEDIA_MIMETYPE_VIDEO_AVC, 2);

    // If dump is enabled, then open create an empty file
    GENERATE_FILE_NAMES();
    CREATE_DUMP_FILE(mInFile);
    CREATE_DUMP_FILE(mOutFile);
    memset(mConversionBuffers, 0, sizeof(mConversionBuffers));
    memset(mInputBufferInfo, 0, sizeof(mInputBufferInfo));

    initEncParams();

}

SoftAVC::~SoftAVC() {
    releaseEncoder();
    List<BufferInfo *> &outQueue = getPortQueue(1);
    List<BufferInfo *> &inQueue = getPortQueue(0);
    CHECK(outQueue.empty());
    CHECK(inQueue.empty());
}

void  SoftAVC::initEncParams() {
    mCodecCtx = NULL;
    mMemRecords = NULL;
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
    mAIRMode = DEFAULT_AIR;
    mAIRRefreshPeriod = DEFAULT_AIR_REFRESH_PERIOD;
    mPSNREnable = DEFAULT_PSNR_ENABLE;
    mReconEnable = DEFAULT_RECON_ENABLE;
    mEntropyMode = DEFAULT_ENTROPY_MODE;
    mBframes = DEFAULT_B_FRAMES;

    gettimeofday(&mTimeStart, NULL);
    gettimeofday(&mTimeEnd, NULL);

}


OMX_ERRORTYPE SoftAVC::setDimensions() {
    ive_ctl_set_dimensions_ip_t s_dimensions_ip;
    ive_ctl_set_dimensions_op_t s_dimensions_op;
    IV_STATUS_T status;

    s_dimensions_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_dimensions_ip.e_sub_cmd = IVE_CMD_CTL_SET_DIMENSIONS;
    s_dimensions_ip.u4_ht = mHeight;
    s_dimensions_ip.u4_wd = mWidth;

    s_dimensions_ip.u4_timestamp_high = -1;
    s_dimensions_ip.u4_timestamp_low = -1;

    s_dimensions_ip.u4_size = sizeof(ive_ctl_set_dimensions_ip_t);
    s_dimensions_op.u4_size = sizeof(ive_ctl_set_dimensions_op_t);

    status = ive_api_function(mCodecCtx, &s_dimensions_ip, &s_dimensions_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set frame dimensions = 0x%x\n",
                s_dimensions_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setNumCores() {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setFrameRate() {
    ive_ctl_set_frame_rate_ip_t s_frame_rate_ip;
    ive_ctl_set_frame_rate_op_t s_frame_rate_op;
    IV_STATUS_T status;

    s_frame_rate_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_frame_rate_ip.e_sub_cmd = IVE_CMD_CTL_SET_FRAMERATE;

    s_frame_rate_ip.u4_src_frame_rate = mFramerate >> 16;
    s_frame_rate_ip.u4_tgt_frame_rate = mFramerate >> 16;

    s_frame_rate_ip.u4_timestamp_high = -1;
    s_frame_rate_ip.u4_timestamp_low = -1;

    s_frame_rate_ip.u4_size = sizeof(ive_ctl_set_frame_rate_ip_t);
    s_frame_rate_op.u4_size = sizeof(ive_ctl_set_frame_rate_op_t);

    status = ive_api_function(mCodecCtx, &s_frame_rate_ip, &s_frame_rate_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set frame rate = 0x%x\n",
                s_frame_rate_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setIpeParams() {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setBitRate() {
    ive_ctl_set_bitrate_ip_t s_bitrate_ip;
    ive_ctl_set_bitrate_op_t s_bitrate_op;
    IV_STATUS_T status;

    s_bitrate_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_bitrate_ip.e_sub_cmd = IVE_CMD_CTL_SET_BITRATE;

    s_bitrate_ip.u4_target_bitrate = mBitrate;

    s_bitrate_ip.u4_timestamp_high = -1;
    s_bitrate_ip.u4_timestamp_low = -1;

    s_bitrate_ip.u4_size = sizeof(ive_ctl_set_bitrate_ip_t);
    s_bitrate_op.u4_size = sizeof(ive_ctl_set_bitrate_op_t);

    status = ive_api_function(mCodecCtx, &s_bitrate_ip, &s_bitrate_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set bit rate = 0x%x\n", s_bitrate_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setFrameType(IV_PICTURE_CODING_TYPE_T e_frame_type) {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setQp() {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setEncMode(IVE_ENC_MODE_T e_enc_mode) {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setVbvParams() {
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
        ALOGE("Unable to set VBC params = 0x%x\n", s_vbv_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setAirParams() {
    ive_ctl_set_air_params_ip_t s_air_ip;
    ive_ctl_set_air_params_op_t s_air_op;
    IV_STATUS_T status;

    s_air_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_air_ip.e_sub_cmd = IVE_CMD_CTL_SET_AIR_PARAMS;

    s_air_ip.e_air_mode = mAIRMode;
    s_air_ip.u4_air_refresh_period = mAIRRefreshPeriod;

    s_air_ip.u4_timestamp_high = -1;
    s_air_ip.u4_timestamp_low = -1;

    s_air_ip.u4_size = sizeof(ive_ctl_set_air_params_ip_t);
    s_air_op.u4_size = sizeof(ive_ctl_set_air_params_op_t);

    status = ive_api_function(mCodecCtx, &s_air_ip, &s_air_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set air params = 0x%x\n", s_air_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setMeParams() {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setGopParams() {
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
        ALOGE("Unable to set ME params = 0x%x\n",
                s_gop_params_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setProfileParams() {
    IV_STATUS_T status;
    ive_ctl_set_profile_params_ip_t s_profile_params_ip;
    ive_ctl_set_profile_params_op_t s_profile_params_op;

    s_profile_params_ip.e_cmd = IVE_CMD_VIDEO_CTL;
    s_profile_params_ip.e_sub_cmd = IVE_CMD_CTL_SET_PROFILE_PARAMS;

    s_profile_params_ip.e_profile = DEFAULT_EPROFILE;
    s_profile_params_ip.u4_entropy_coding_mode = mEntropyMode;
    s_profile_params_ip.u4_timestamp_high = -1;
    s_profile_params_ip.u4_timestamp_low = -1;

    s_profile_params_ip.u4_size = sizeof(ive_ctl_set_profile_params_ip_t);
    s_profile_params_op.u4_size = sizeof(ive_ctl_set_profile_params_op_t);

    status = ive_api_function(mCodecCtx, &s_profile_params_ip, &s_profile_params_op);
    if (status != IV_SUCCESS) {
        ALOGE("Unable to set profile params = 0x%x\n",
                s_profile_params_op.u4_error_code);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setDeblockParams() {
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
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

void SoftAVC::logVersion() {
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

OMX_ERRORTYPE SoftAVC::initEncoder() {
    IV_STATUS_T status;
    WORD32 level;
    uint32_t displaySizeY;

    CHECK(!mStarted);

    OMX_ERRORTYPE errType = OMX_ErrorNone;

    displaySizeY = mWidth * mHeight;
    if (displaySizeY > (1920 * 1088)) {
        level = 50;
    } else if (displaySizeY > (1280 * 720)) {
        level = 40;
    } else if (displaySizeY > (720 * 576)) {
        level = 31;
    } else if (displaySizeY > (624 * 320)) {
        level = 30;
    } else if (displaySizeY > (352 * 288)) {
        level = 21;
    } else if (displaySizeY > (176 * 144)) {
        level = 20;
    } else {
        level = 10;
    }
    mAVCEncLevel = MAX(level, mAVCEncLevel);

    mStride = mWidth;

    if (mInputDataIsMeta) {
        for (size_t i = 0; i < MAX_CONVERSION_BUFFERS; i++) {
            if (mConversionBuffers[i] != NULL) {
                free(mConversionBuffers[i]);
                mConversionBuffers[i] = 0;
            }

            if (((uint64_t)mStride * mHeight) > ((uint64_t)INT32_MAX / 3)) {
                ALOGE("Buffer size is too big.");
                return OMX_ErrorUndefined;
            }
            mConversionBuffers[i] = (uint8_t *)malloc(mStride * mHeight * 3 / 2);

            if (mConversionBuffers[i] == NULL) {
                ALOGE("Allocating conversion buffer failed.");
                return OMX_ErrorUndefined;
            }

            mConversionBuffersFree[i] = 1;
        }
    }

    switch (mColorFormat) {
        case OMX_COLOR_FormatYUV420SemiPlanar:
            mIvVideoColorFormat = IV_YUV_420SP_UV;
            ALOGV("colorFormat YUV_420SP");
            break;
        default:
        case OMX_COLOR_FormatYUV420Planar:
            mIvVideoColorFormat = IV_YUV_420P;
            ALOGV("colorFormat YUV_420P");
            break;
    }

    ALOGD("Params width %d height %d level %d colorFormat %d", mWidth,
            mHeight, mAVCEncLevel, mIvVideoColorFormat);

    /* Getting Number of MemRecords */
    {
        iv_num_mem_rec_ip_t s_num_mem_rec_ip;
        iv_num_mem_rec_op_t s_num_mem_rec_op;

        s_num_mem_rec_ip.u4_size = sizeof(iv_num_mem_rec_ip_t);
        s_num_mem_rec_op.u4_size = sizeof(iv_num_mem_rec_op_t);

        s_num_mem_rec_ip.e_cmd = IV_CMD_GET_NUM_MEM_REC;

        status = ive_api_function(0, &s_num_mem_rec_ip, &s_num_mem_rec_op);

        if (status != IV_SUCCESS) {
            ALOGE("Get number of memory records failed = 0x%x\n",
                    s_num_mem_rec_op.u4_error_code);
            return OMX_ErrorUndefined;
        }

        mNumMemRecords = s_num_mem_rec_op.u4_num_mem_rec;
    }

    /* Allocate array to hold memory records */
    if (mNumMemRecords > SIZE_MAX / sizeof(iv_mem_rec_t)) {
        ALOGE("requested memory size is too big.");
        return OMX_ErrorUndefined;
    }
    mMemRecords = (iv_mem_rec_t *)malloc(mNumMemRecords * sizeof(iv_mem_rec_t));
    if (NULL == mMemRecords) {
        ALOGE("Unable to allocate memory for hold memory records: Size %zu",
                mNumMemRecords * sizeof(iv_mem_rec_t));
        mSignalledError = true;
        notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
        return OMX_ErrorUndefined;
    }

    {
        iv_mem_rec_t *ps_mem_rec;
        ps_mem_rec = mMemRecords;
        for (size_t i = 0; i < mNumMemRecords; i++) {
            ps_mem_rec->u4_size = sizeof(iv_mem_rec_t);
            ps_mem_rec->pv_base = NULL;
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
        s_fill_mem_rec_ip.u4_max_wd = mWidth;
        s_fill_mem_rec_ip.u4_max_ht = mHeight;
        s_fill_mem_rec_ip.u4_max_level = mAVCEncLevel;
        s_fill_mem_rec_ip.e_color_format = DEFAULT_INP_COLOR_FORMAT;
        s_fill_mem_rec_ip.u4_max_ref_cnt = DEFAULT_MAX_REF_FRM;
        s_fill_mem_rec_ip.u4_max_reorder_cnt = DEFAULT_MAX_REORDER_FRM;
        s_fill_mem_rec_ip.u4_max_srch_rng_x = DEFAULT_MAX_SRCH_RANGE_X;
        s_fill_mem_rec_ip.u4_max_srch_rng_y = DEFAULT_MAX_SRCH_RANGE_Y;

        status = ive_api_function(0, &s_fill_mem_rec_ip, &s_fill_mem_rec_op);

        if (status != IV_SUCCESS) {
            ALOGE("Fill memory records failed = 0x%x\n",
                    s_fill_mem_rec_op.u4_error_code);
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
            return OMX_ErrorUndefined;
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
            if (ps_mem_rec->pv_base == NULL) {
                ALOGE("Allocation failure for mem record id %zu size %u\n", i,
                        ps_mem_rec->u4_mem_size);
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                return OMX_ErrorUndefined;

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
        s_init_ip.u4_max_wd = mWidth;
        s_init_ip.u4_max_ht = mHeight;
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
            ALOGE("Init memory records failed = 0x%x\n",
                    s_init_op.u4_error_code);
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0 /* arg2 */, NULL /* data */);
            return OMX_ErrorUndefined;
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

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::releaseEncoder() {
    IV_STATUS_T status = IV_SUCCESS;
    iv_retrieve_mem_rec_ip_t s_retrieve_mem_ip;
    iv_retrieve_mem_rec_op_t s_retrieve_mem_op;
    iv_mem_rec_t *ps_mem_rec;

    if (!mStarted) {
        return OMX_ErrorNone;
    }

    s_retrieve_mem_ip.u4_size = sizeof(iv_retrieve_mem_rec_ip_t);
    s_retrieve_mem_op.u4_size = sizeof(iv_retrieve_mem_rec_op_t);
    s_retrieve_mem_ip.e_cmd = IV_CMD_RETRIEVE_MEMREC;
    s_retrieve_mem_ip.ps_mem_rec = mMemRecords;

    status = ive_api_function(mCodecCtx, &s_retrieve_mem_ip, &s_retrieve_mem_op);

    if (status != IV_SUCCESS) {
        ALOGE("Unable to retrieve memory records = 0x%x\n",
                s_retrieve_mem_op.u4_error_code);
        return OMX_ErrorUndefined;
    }

    /* Free memory records */
    ps_mem_rec = mMemRecords;
    for (size_t i = 0; i < s_retrieve_mem_op.u4_num_mem_rec_filled; i++) {
        ive_aligned_free(ps_mem_rec->pv_base);
        ps_mem_rec++;
    }

    free(mMemRecords);

    for (size_t i = 0; i < MAX_CONVERSION_BUFFERS; i++) {
        if (mConversionBuffers[i]) {
            free(mConversionBuffers[i]);
            mConversionBuffers[i] = NULL;
        }
    }

    // clear other pointers into the space being free()d
    mCodecCtx = NULL;

    mStarted = false;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::internalGetParameter(OMX_INDEXTYPE index, OMX_PTR params) {
    switch (index) {
        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *bitRate =
                (OMX_VIDEO_PARAM_BITRATETYPE *)params;

            if (!isValidOMXParam(bitRate)) {
                return OMX_ErrorBadParameter;
            }

            if (bitRate->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            bitRate->eControlRate = OMX_Video_ControlRateVariable;
            bitRate->nTargetBitrate = mBitrate;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *avcParams = (OMX_VIDEO_PARAM_AVCTYPE *)params;

            if (!isValidOMXParam(avcParams)) {
                return OMX_ErrorBadParameter;
            }

            if (avcParams->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            OMX_VIDEO_AVCLEVELTYPE omxLevel = OMX_VIDEO_AVCLevel41;
            if (OMX_ErrorNone
                    != ConvertAvcSpecLevelToOmxAvcLevel(mAVCEncLevel, &omxLevel)) {
                return OMX_ErrorUndefined;
            }

            // TODO: maintain profile
            avcParams->eProfile = (OMX_VIDEO_AVCPROFILETYPE)OMX_VIDEO_AVCProfileConstrainedBaseline;
            avcParams->eLevel = omxLevel;
            avcParams->nRefFrames = 1;
            avcParams->bUseHadamard = OMX_TRUE;
            avcParams->nAllowedPictureTypes = (OMX_VIDEO_PictureTypeI
                    | OMX_VIDEO_PictureTypeP | OMX_VIDEO_PictureTypeB);
            avcParams->nRefIdx10ActiveMinus1 = 0;
            avcParams->nRefIdx11ActiveMinus1 = 0;
            avcParams->bWeightedPPrediction = OMX_FALSE;
            avcParams->bconstIpred = OMX_FALSE;
            avcParams->bDirect8x8Inference = OMX_FALSE;
            avcParams->bDirectSpatialTemporal = OMX_FALSE;
            avcParams->nCabacInitIdc = 0;
            return OMX_ErrorNone;
        }

        default:
            return SoftVideoEncoderOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftAVC::internalSetParameter(OMX_INDEXTYPE index, const OMX_PTR params) {
    int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *bitRate =
                (OMX_VIDEO_PARAM_BITRATETYPE *)params;

            if (!isValidOMXParam(bitRate)) {
                return OMX_ErrorBadParameter;
            }

            return internalSetBitrateParams(bitRate);
        }

        case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *avcType = (OMX_VIDEO_PARAM_AVCTYPE *)params;

            if (!isValidOMXParam(avcType)) {
                return OMX_ErrorBadParameter;
            }

            if (avcType->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            mEntropyMode = 0;

            if (OMX_TRUE == avcType->bEntropyCodingCABAC)
                mEntropyMode = 1;

            if ((avcType->nAllowedPictureTypes & OMX_VIDEO_PictureTypeB) &&
                    avcType->nPFrames) {
                mBframes = avcType->nBFrames;
            }

            mIInterval = (avcType->nPFrames + 1) * (avcType->nBFrames + 1);
            mConstrainedIntraFlag = avcType->bconstIpred;

            if (OMX_VIDEO_AVCLoopFilterDisable == avcType->eLoopFilterMode)
                mDisableDeblkLevel = 4;

            if (avcType->nRefFrames != 1
                    || avcType->bUseHadamard != OMX_TRUE
                    || avcType->nRefIdx10ActiveMinus1 != 0
                    || avcType->nRefIdx11ActiveMinus1 != 0
                    || avcType->bWeightedPPrediction != OMX_FALSE
                    || avcType->bDirect8x8Inference != OMX_FALSE
                    || avcType->bDirectSpatialTemporal != OMX_FALSE
                    || avcType->nCabacInitIdc != 0) {
                // OMX does not allow a way to signal what values are wrong, so it's
                // best for components to just do best effort in supporting these values
                ALOGV("ignoring unsupported settings");
            }

            if (OK != ConvertOmxAvcLevelToAvcSpecLevel(avcType->eLevel, &mAVCEncLevel)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        default:
            return SoftVideoEncoderOMXComponent::internalSetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftAVC::getConfig(
        OMX_INDEXTYPE index, OMX_PTR _params) {
    switch ((int)index) {
        case OMX_IndexConfigAndroidIntraRefresh:
        {
            OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *intraRefreshParams =
                (OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)_params;

            if (!isValidOMXParam(intraRefreshParams)) {
                return OMX_ErrorBadParameter;
            }

            if (intraRefreshParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorUndefined;
            }

            intraRefreshParams->nRefreshPeriod =
                    (mAIRMode == IVE_AIR_MODE_NONE) ? 0 : mAIRRefreshPeriod;
            return OMX_ErrorNone;
        }

        default:
            return SoftVideoEncoderOMXComponent::getConfig(index, _params);
    }
}

OMX_ERRORTYPE SoftAVC::internalSetConfig(
        OMX_INDEXTYPE index, const OMX_PTR _params, bool *frameConfig) {
    switch ((int)index) {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            OMX_CONFIG_INTRAREFRESHVOPTYPE *params =
                (OMX_CONFIG_INTRAREFRESHVOPTYPE *)_params;

            if (!isValidOMXParam(params)) {
                return OMX_ErrorBadParameter;
            }

            if (params->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            if (params->IntraRefreshVOP) {
                mUpdateFlag |= kRequestKeyFrame;
            }
            return OMX_ErrorNone;
        }

        case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE *params =
                (OMX_VIDEO_CONFIG_BITRATETYPE *)_params;

            if (!isValidOMXParam(params)) {
                return OMX_ErrorBadParameter;
            }

            if (params->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            if (mBitrate != params->nEncodeBitrate) {
                mBitrate = params->nEncodeBitrate;
                mUpdateFlag |= kUpdateBitrate;
            }
            return OMX_ErrorNone;
        }

        case OMX_IndexConfigAndroidIntraRefresh:
        {
            const OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *intraRefreshParams =
                (const OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)_params;

            if (!isValidOMXParam(intraRefreshParams)) {
                return OMX_ErrorBadParameter;
            }

            if (intraRefreshParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorUndefined;
            }

            if (intraRefreshParams->nRefreshPeriod == 0) {
                mAIRMode = IVE_AIR_MODE_NONE;
                mAIRRefreshPeriod = 0;
            } else if (intraRefreshParams->nRefreshPeriod > 0) {
                mAIRMode = IVE_AIR_MODE_CYCLIC;
                mAIRRefreshPeriod = intraRefreshParams->nRefreshPeriod;
            }
            mUpdateFlag |= kUpdateAIRMode;
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetConfig(index, _params, frameConfig);
    }
}

OMX_ERRORTYPE SoftAVC::internalSetBitrateParams(
        const OMX_VIDEO_PARAM_BITRATETYPE *bitrate) {
    if (bitrate->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    mBitrate = bitrate->nTargetBitrate;
    mUpdateFlag |= kUpdateBitrate;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftAVC::setEncodeArgs(
        ive_video_encode_ip_t *ps_encode_ip,
        ive_video_encode_op_t *ps_encode_op,
        OMX_BUFFERHEADERTYPE *inputBufferHeader,
        OMX_BUFFERHEADERTYPE *outputBufferHeader) {
    iv_raw_buf_t *ps_inp_raw_buf;
    const uint8_t *source;
    UWORD8 *pu1_buf;

    ps_inp_raw_buf = &ps_encode_ip->s_inp_buf;
    ps_encode_ip->s_out_buf.pv_buf = outputBufferHeader->pBuffer;
    ps_encode_ip->s_out_buf.u4_bytes = 0;
    ps_encode_ip->s_out_buf.u4_bufsize = outputBufferHeader->nAllocLen;
    ps_encode_ip->u4_size = sizeof(ive_video_encode_ip_t);
    ps_encode_op->u4_size = sizeof(ive_video_encode_op_t);

    ps_encode_ip->e_cmd = IVE_CMD_VIDEO_ENCODE;
    ps_encode_ip->pv_bufs = NULL;
    ps_encode_ip->pv_mb_info = NULL;
    ps_encode_ip->pv_pic_info = NULL;
    ps_encode_ip->u4_mb_info_type = 0;
    ps_encode_ip->u4_pic_info_type = 0;
    ps_encode_op->s_out_buf.pv_buf = NULL;

    /* Initialize color formats */
    ps_inp_raw_buf->e_color_fmt = mIvVideoColorFormat;
    source = NULL;
    if ((inputBufferHeader != NULL) && inputBufferHeader->nFilledLen) {
        OMX_ERRORTYPE error = validateInputBuffer(inputBufferHeader);
        if (error != OMX_ErrorNone) {
            ALOGE("b/69065651");
            android_errorWriteLog(0x534e4554, "69065651");
            return error;
        }
        source = inputBufferHeader->pBuffer + inputBufferHeader->nOffset;

        if (mInputDataIsMeta) {
            uint8_t *conversionBuffer = NULL;
            for (size_t i = 0; i < MAX_CONVERSION_BUFFERS; i++) {
                if (mConversionBuffersFree[i]) {
                    mConversionBuffersFree[i] = 0;
                    conversionBuffer = mConversionBuffers[i];
                    break;
                }
            }

            if (NULL == conversionBuffer) {
                ALOGE("No free buffers to hold conversion data");
                return OMX_ErrorUndefined;
            }

            source = extractGraphicBuffer(
                    conversionBuffer, (mWidth * mHeight * 3 / 2), source,
                    inputBufferHeader->nFilledLen, mWidth, mHeight);

            if (source == NULL) {
                ALOGE("Error in extractGraphicBuffer");
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                return OMX_ErrorUndefined;
            }
        }
        ps_encode_ip->u4_is_last = 0;
        ps_encode_ip->u4_timestamp_high = (inputBufferHeader->nTimeStamp) >> 32;
        ps_encode_ip->u4_timestamp_low = (inputBufferHeader->nTimeStamp) & 0xFFFFFFFF;
    }
    else {
        if (mSawInputEOS){
            ps_encode_ip->u4_is_last = 1;
        }
        memset(ps_inp_raw_buf, 0, sizeof(iv_raw_buf_t));
        ps_inp_raw_buf->e_color_fmt = mIvVideoColorFormat;
        ps_inp_raw_buf->u4_size = sizeof(iv_raw_buf_t);
        return OMX_ErrorNone;
    }

    pu1_buf = (UWORD8 *)source;
    switch (mIvVideoColorFormat) {
        case IV_YUV_420P:
        {
            ps_inp_raw_buf->apv_bufs[0] = pu1_buf;
            pu1_buf += (mStride) * mHeight;
            ps_inp_raw_buf->apv_bufs[1] = pu1_buf;
            pu1_buf += (mStride / 2) * mHeight / 2;
            ps_inp_raw_buf->apv_bufs[2] = pu1_buf;

            ps_inp_raw_buf->au4_wd[0] = mWidth;
            ps_inp_raw_buf->au4_wd[1] = mWidth / 2;
            ps_inp_raw_buf->au4_wd[2] = mWidth / 2;

            ps_inp_raw_buf->au4_ht[0] = mHeight;
            ps_inp_raw_buf->au4_ht[1] = mHeight / 2;
            ps_inp_raw_buf->au4_ht[2] = mHeight / 2;

            ps_inp_raw_buf->au4_strd[0] = mStride;
            ps_inp_raw_buf->au4_strd[1] = (mStride / 2);
            ps_inp_raw_buf->au4_strd[2] = (mStride / 2);
            break;
        }

        case IV_YUV_422ILE:
        {
            ps_inp_raw_buf->apv_bufs[0] = pu1_buf;
            ps_inp_raw_buf->au4_wd[0] = mWidth * 2;
            ps_inp_raw_buf->au4_ht[0] = mHeight;
            ps_inp_raw_buf->au4_strd[0] = mStride * 2;
            break;
        }

        case IV_YUV_420SP_UV:
        case IV_YUV_420SP_VU:
        default:
        {
            ps_inp_raw_buf->apv_bufs[0] = pu1_buf;
            pu1_buf += (mStride) * mHeight;
            ps_inp_raw_buf->apv_bufs[1] = pu1_buf;

            ps_inp_raw_buf->au4_wd[0] = mWidth;
            ps_inp_raw_buf->au4_wd[1] = mWidth;

            ps_inp_raw_buf->au4_ht[0] = mHeight;
            ps_inp_raw_buf->au4_ht[1] = mHeight / 2;

            ps_inp_raw_buf->au4_strd[0] = mStride;
            ps_inp_raw_buf->au4_strd[1] = mStride;
            break;
        }
    }
    return OMX_ErrorNone;
}

void SoftAVC::onQueueFilled(OMX_U32 portIndex) {
    IV_STATUS_T status;
    WORD32 timeDelay, timeTaken;

    UNUSED(portIndex);

    // Initialize encoder if not already initialized
    if (mCodecCtx == NULL) {
        if (OMX_ErrorNone != initEncoder()) {
            ALOGE("Failed to initialize encoder");
            notify(OMX_EventError, OMX_ErrorUndefined, 0 /* arg2 */, NULL /* data */);
            return;
        }
    }
    if (mSignalledError) {
        return;
    }

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    while (!mSawOutputEOS && !outQueue.empty()) {

        OMX_ERRORTYPE error;
        ive_video_encode_ip_t s_encode_ip;
        ive_video_encode_op_t s_encode_op;
        BufferInfo *outputBufferInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outputBufferHeader = outputBufferInfo->mHeader;

        BufferInfo *inputBufferInfo;
        OMX_BUFFERHEADERTYPE *inputBufferHeader;

        if (mSawInputEOS) {
            inputBufferHeader = NULL;
            inputBufferInfo = NULL;
        } else if (!inQueue.empty()) {
            inputBufferInfo = *inQueue.begin();
            inputBufferHeader = inputBufferInfo->mHeader;
        } else {
            return;
        }

        outputBufferHeader->nTimeStamp = 0;
        outputBufferHeader->nFlags = 0;
        outputBufferHeader->nOffset = 0;
        outputBufferHeader->nFilledLen = 0;
        outputBufferHeader->nOffset = 0;

        if (inputBufferHeader != NULL) {
            outputBufferHeader->nFlags = inputBufferHeader->nFlags;
        }

        uint8_t *outPtr = (uint8_t *)outputBufferHeader->pBuffer;

        if (!mSpsPpsHeaderReceived) {
            error = setEncodeArgs(&s_encode_ip, &s_encode_op, NULL, outputBufferHeader);
            if (error != OMX_ErrorNone) {
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                return;
            }
            status = ive_api_function(mCodecCtx, &s_encode_ip, &s_encode_op);

            if (IV_SUCCESS != status) {
                ALOGE("Encode Frame failed = 0x%x\n",
                        s_encode_op.u4_error_code);
            } else {
                ALOGV("Bytes Generated in header %d\n",
                        s_encode_op.s_out_buf.u4_bytes);
            }

            mSpsPpsHeaderReceived = true;

            outputBufferHeader->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
            outputBufferHeader->nFilledLen = s_encode_op.s_out_buf.u4_bytes;
            if (inputBufferHeader != NULL) {
                outputBufferHeader->nTimeStamp = inputBufferHeader->nTimeStamp;
            }

            outQueue.erase(outQueue.begin());
            outputBufferInfo->mOwnedByUs = false;

            DUMP_TO_FILE(
                    mOutFile, outputBufferHeader->pBuffer,
                    outputBufferHeader->nFilledLen);
            notifyFillBufferDone(outputBufferHeader);

            setEncMode(IVE_ENC_MODE_PICTURE);
            return;
        }

        if (mUpdateFlag) {
            if (mUpdateFlag & kUpdateBitrate) {
                setBitRate();
            }
            if (mUpdateFlag & kRequestKeyFrame) {
                setFrameType(IV_IDR_FRAME);
            }
            if (mUpdateFlag & kUpdateAIRMode) {
                setAirParams();
                notify(OMX_EventPortSettingsChanged, kOutputPortIndex,
                        OMX_IndexConfigAndroidIntraRefresh, NULL);
            }
            mUpdateFlag = 0;
        }

        if ((inputBufferHeader != NULL)
                && (inputBufferHeader->nFlags & OMX_BUFFERFLAG_EOS)) {
            mSawInputEOS = true;
        }

        /* In normal mode, store inputBufferInfo and this will be returned
           when encoder consumes this input */
        if (!mInputDataIsMeta && (inputBufferInfo != NULL)) {
            for (size_t i = 0; i < MAX_INPUT_BUFFER_HEADERS; i++) {
                if (NULL == mInputBufferInfo[i]) {
                    mInputBufferInfo[i] = inputBufferInfo;
                    break;
                }
            }
        }
        error = setEncodeArgs(
                &s_encode_ip, &s_encode_op, inputBufferHeader, outputBufferHeader);

        if (error != OMX_ErrorNone) {
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
            return;
        }

        DUMP_TO_FILE(
                mInFile, s_encode_ip.s_inp_buf.apv_bufs[0],
                (mHeight * mStride * 3 / 2));

        GETTIME(&mTimeStart, NULL);
        /* Compute time elapsed between end of previous decode()
         * to start of current decode() */
        TIME_DIFF(mTimeEnd, mTimeStart, timeDelay);
        status = ive_api_function(mCodecCtx, &s_encode_ip, &s_encode_op);

        if (IV_SUCCESS != status) {
            ALOGE("Encode Frame failed = 0x%x\n",
                    s_encode_op.u4_error_code);
            mSignalledError = true;
            notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
            return;
        }

        GETTIME(&mTimeEnd, NULL);
        /* Compute time taken for decode() */
        TIME_DIFF(mTimeStart, mTimeEnd, timeTaken);

        ALOGV("timeTaken=%6d delay=%6d numBytes=%6d", timeTaken, timeDelay,
                s_encode_op.s_out_buf.u4_bytes);

        /* In encoder frees up an input buffer, mark it as free */
        if (s_encode_op.s_inp_buf.apv_bufs[0] != NULL) {
            if (mInputDataIsMeta) {
                for (size_t i = 0; i < MAX_CONVERSION_BUFFERS; i++) {
                    if (mConversionBuffers[i] == s_encode_op.s_inp_buf.apv_bufs[0]) {
                        mConversionBuffersFree[i] = 1;
                        break;
                    }
                }
            } else {
                /* In normal mode, call EBD on inBuffeHeader that is freed by the codec */
                for (size_t i = 0; i < MAX_INPUT_BUFFER_HEADERS; i++) {
                    uint8_t *buf = NULL;
                    OMX_BUFFERHEADERTYPE *bufHdr = NULL;
                    if (mInputBufferInfo[i] != NULL) {
                        bufHdr = mInputBufferInfo[i]->mHeader;
                        buf = bufHdr->pBuffer + bufHdr->nOffset;
                    }
                    if (s_encode_op.s_inp_buf.apv_bufs[0] == buf) {
                        mInputBufferInfo[i]->mOwnedByUs = false;
                        notifyEmptyBufferDone(bufHdr);
                        mInputBufferInfo[i] = NULL;
                        break;
                    }
                }
            }
        }

        outputBufferHeader->nFilledLen = s_encode_op.s_out_buf.u4_bytes;

        if (IV_IDR_FRAME == s_encode_op.u4_encoded_frame_type) {
            outputBufferHeader->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
        }

        if (inputBufferHeader != NULL) {
            inQueue.erase(inQueue.begin());

            /* If in meta data, call EBD on input */
            /* In case of normal mode, EBD will be done once encoder
            releases the input buffer */
            if (mInputDataIsMeta) {
                inputBufferInfo->mOwnedByUs = false;
                notifyEmptyBufferDone(inputBufferHeader);
            }
        }

        if (s_encode_op.u4_is_last) {
            outputBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
            mSawOutputEOS = true;
        } else {
            outputBufferHeader->nFlags &= ~OMX_BUFFERFLAG_EOS;
        }

        if (outputBufferHeader->nFilledLen || s_encode_op.u4_is_last) {
            outputBufferHeader->nTimeStamp = s_encode_op.u4_timestamp_high;
            outputBufferHeader->nTimeStamp <<= 32;
            outputBufferHeader->nTimeStamp |= s_encode_op.u4_timestamp_low;
            outputBufferInfo->mOwnedByUs = false;
            outQueue.erase(outQueue.begin());
            DUMP_TO_FILE(mOutFile, outputBufferHeader->pBuffer,
                    outputBufferHeader->nFilledLen);
            notifyFillBufferDone(outputBufferHeader);
        }

        if (s_encode_op.u4_is_last == 1) {
            return;
        }
    }
    return;
}

void SoftAVC::onReset() {
    SoftVideoEncoderOMXComponent::onReset();

    if (releaseEncoder() != OMX_ErrorNone) {
        ALOGW("releaseEncoder failed");
    }
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    return new android::SoftAVC(name, callbacks, appData, component);
}
