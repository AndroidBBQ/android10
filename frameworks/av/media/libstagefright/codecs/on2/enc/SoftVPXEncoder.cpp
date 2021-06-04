/*
 * Copyright (C) 2013 The Android Open Source Project
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

// #define LOG_NDEBUG 0
#define LOG_TAG "SoftVPXEncoder"
#include "SoftVPXEncoder.h"

#include "SoftVP8Encoder.h"
#include "SoftVP9Encoder.h"

#include <android-base/macros.h>
#include <utils/Log.h>
#include <utils/misc.h>

#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

#ifndef INT32_MAX
#define INT32_MAX   2147483647
#endif

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    // OMX IL 1.1.2
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 1;
    params->nVersion.s.nRevision = 2;
    params->nVersion.s.nStep = 0;
}

static int GetCPUCoreCount() {
    int cpuCoreCount = 1;
#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif
    CHECK_GE(cpuCoreCount, 1);
    return cpuCoreCount;
}

SoftVPXEncoder::SoftVPXEncoder(const char *name,
                               const OMX_CALLBACKTYPE *callbacks,
                               OMX_PTR appData,
                               OMX_COMPONENTTYPE **component,
                               const char* role,
                               OMX_VIDEO_CODINGTYPE codingType,
                               const char* mimeType,
                               int32_t minCompressionRatio,
                               const CodecProfileLevel *profileLevels,
                               size_t numProfileLevels)
    : SoftVideoEncoderOMXComponent(
            name, role, codingType, profileLevels, numProfileLevels,
            176 /* width */, 144 /* height */,
            callbacks, appData, component),
      mCodecContext(NULL),
      mCodecConfiguration(NULL),
      mCodecInterface(NULL),
      mBitrateUpdated(false),
      mBitrateControlMode(VPX_VBR),
      mErrorResilience(OMX_FALSE),
      mKeyFrameInterval(0),
      mMinQuantizer(0),
      mMaxQuantizer(0),
      mTemporalLayers(0),
      mTemporalPatternType(OMX_VIDEO_VPXTemporalLayerPatternNone),
      mTemporalPatternLength(0),
      mTemporalPatternIdx(0),
      mLastTimestamp(0x7FFFFFFFFFFFFFFFLL),
      mConversionBuffer(NULL),
      mKeyFrameRequested(false) {
    memset(mTemporalLayerBitrateRatio, 0, sizeof(mTemporalLayerBitrateRatio));
    mTemporalLayerBitrateRatio[0] = 100;

    const size_t kMinOutputBufferSize = 1024 * 1024; // arbitrary

    initPorts(
            kNumBuffers, kNumBuffers, kMinOutputBufferSize,
            mimeType, minCompressionRatio);
}

SoftVPXEncoder::~SoftVPXEncoder() {
    releaseEncoder();
}

status_t SoftVPXEncoder::initEncoder() {
    vpx_codec_err_t codec_return;
    status_t result = UNKNOWN_ERROR;

    setCodecSpecificInterface();
    if (mCodecInterface == NULL) {
        goto CLEAN_UP;
    }
    ALOGD("VPx: initEncoder. BRMode: %u. TSLayers: %zu. KF: %u. QP: %u - %u",
          (uint32_t)mBitrateControlMode, mTemporalLayers, mKeyFrameInterval,
          mMinQuantizer, mMaxQuantizer);

    mCodecConfiguration = new vpx_codec_enc_cfg_t;
    codec_return = vpx_codec_enc_config_default(mCodecInterface,
                                                mCodecConfiguration,
                                                0);

    if (codec_return != VPX_CODEC_OK) {
        ALOGE("Error populating default configuration for vpx encoder.");
        goto CLEAN_UP;
    }

    mCodecConfiguration->g_w = mWidth;
    mCodecConfiguration->g_h = mHeight;
    mCodecConfiguration->g_threads = GetCPUCoreCount();
    mCodecConfiguration->g_error_resilient = mErrorResilience;

    // OMX timebase unit is microsecond
    // g_timebase is in seconds (i.e. 1/1000000 seconds)
    mCodecConfiguration->g_timebase.num = 1;
    mCodecConfiguration->g_timebase.den = 1000000;
    // rc_target_bitrate is in kbps, mBitrate in bps
    mCodecConfiguration->rc_target_bitrate = (mBitrate + 500) / 1000;
    mCodecConfiguration->rc_end_usage = mBitrateControlMode;
    // Disable frame drop - not allowed in MediaCodec now.
    mCodecConfiguration->rc_dropframe_thresh = 0;
    // Disable lagged encoding.
    mCodecConfiguration->g_lag_in_frames = 0;
    if (mBitrateControlMode == VPX_CBR) {
        // Disable spatial resizing.
        mCodecConfiguration->rc_resize_allowed = 0;
        // Single-pass mode.
        mCodecConfiguration->g_pass = VPX_RC_ONE_PASS;
        // Maximum amount of bits that can be subtracted from the target
        // bitrate - expressed as percentage of the target bitrate.
        mCodecConfiguration->rc_undershoot_pct = 100;
        // Maximum amount of bits that can be added to the target
        // bitrate - expressed as percentage of the target bitrate.
        mCodecConfiguration->rc_overshoot_pct = 15;
        // Initial value of the buffer level in ms.
        mCodecConfiguration->rc_buf_initial_sz = 500;
        // Amount of data that the encoder should try to maintain in ms.
        mCodecConfiguration->rc_buf_optimal_sz = 600;
        // The amount of data that may be buffered by the decoding
        // application in ms.
        mCodecConfiguration->rc_buf_sz = 1000;
        // Enable error resilience - needed for packet loss.
        mCodecConfiguration->g_error_resilient = 1;
        // Maximum key frame interval - for CBR boost to 3000
        mCodecConfiguration->kf_max_dist = 3000;
        // Encoder determines optimal key frame placement automatically.
        mCodecConfiguration->kf_mode = VPX_KF_AUTO;
    }

    // Frames temporal pattern - for now WebRTC like pattern is only supported.
    switch (mTemporalLayers) {
        case 0:
        {
            mTemporalPatternLength = 0;
            break;
        }
        case 1:
        {
            mCodecConfiguration->ts_number_layers = 1;
            mCodecConfiguration->ts_rate_decimator[0] = 1;
            mCodecConfiguration->ts_periodicity = 1;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mTemporalPattern[0] = kTemporalUpdateLastRefAll;
            mTemporalPatternLength = 1;
            break;
        }
        case 2:
        {
            mCodecConfiguration->ts_number_layers = 2;
            mCodecConfiguration->ts_rate_decimator[0] = 2;
            mCodecConfiguration->ts_rate_decimator[1] = 1;
            mCodecConfiguration->ts_periodicity = 2;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mCodecConfiguration->ts_layer_id[1] = 1;
            mTemporalPattern[0] = kTemporalUpdateLastAndGoldenRefAltRef;
            mTemporalPattern[1] = kTemporalUpdateGoldenWithoutDependencyRefAltRef;
            mTemporalPattern[2] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[3] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[4] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[5] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[6] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[7] = kTemporalUpdateNone;
            mTemporalPatternLength = 8;
            break;
        }
        case 3:
        {
            mCodecConfiguration->ts_number_layers = 3;
            mCodecConfiguration->ts_rate_decimator[0] = 4;
            mCodecConfiguration->ts_rate_decimator[1] = 2;
            mCodecConfiguration->ts_rate_decimator[2] = 1;
            mCodecConfiguration->ts_periodicity = 4;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mCodecConfiguration->ts_layer_id[1] = 2;
            mCodecConfiguration->ts_layer_id[2] = 1;
            mCodecConfiguration->ts_layer_id[3] = 2;
            mTemporalPattern[0] = kTemporalUpdateLastAndGoldenRefAltRef;
            mTemporalPattern[1] = kTemporalUpdateNoneNoRefGoldenRefAltRef;
            mTemporalPattern[2] = kTemporalUpdateGoldenWithoutDependencyRefAltRef;
            mTemporalPattern[3] = kTemporalUpdateNone;
            mTemporalPattern[4] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[5] = kTemporalUpdateNone;
            mTemporalPattern[6] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[7] = kTemporalUpdateNone;
            mTemporalPatternLength = 8;
            break;
        }
        default:
        {
            ALOGE("Wrong number of temporal layers %zu", mTemporalLayers);
            goto CLEAN_UP;
        }
    }
    // Set bitrate values for each layer
    for (size_t i = 0; i < mCodecConfiguration->ts_number_layers; i++) {
        mCodecConfiguration->ts_target_bitrate[i] =
            mCodecConfiguration->rc_target_bitrate *
            mTemporalLayerBitrateRatio[i] / 100;
    }
    if (mKeyFrameInterval > 0) {
        mCodecConfiguration->kf_max_dist = mKeyFrameInterval;
        mCodecConfiguration->kf_min_dist = mKeyFrameInterval;
        mCodecConfiguration->kf_mode = VPX_KF_AUTO;
    }
    if (mMinQuantizer > 0) {
        mCodecConfiguration->rc_min_quantizer = mMinQuantizer;
    }
    if (mMaxQuantizer > 0) {
        mCodecConfiguration->rc_max_quantizer = mMaxQuantizer;
    }
    setCodecSpecificConfiguration();
    mCodecContext = new vpx_codec_ctx_t;
    codec_return = vpx_codec_enc_init(mCodecContext,
                                      mCodecInterface,
                                      mCodecConfiguration,
                                      0);  // flags

    if (codec_return != VPX_CODEC_OK) {
        ALOGE("Error initializing vpx encoder");
        goto CLEAN_UP;
    }

    // Extra CBR settings
    if (mBitrateControlMode == VPX_CBR) {
        codec_return = vpx_codec_control(mCodecContext,
                                         VP8E_SET_STATIC_THRESHOLD,
                                         1);
        if (codec_return == VPX_CODEC_OK) {
            uint32_t rc_max_intra_target =
                mCodecConfiguration->rc_buf_optimal_sz * (mFramerate >> 17) / 10;
            // Don't go below 3 times per frame bandwidth.
            if (rc_max_intra_target < 300) {
                rc_max_intra_target = 300;
            }
            codec_return = vpx_codec_control(mCodecContext,
                                             VP8E_SET_MAX_INTRA_BITRATE_PCT,
                                             rc_max_intra_target);
        }
        if (codec_return == VPX_CODEC_OK) {
            codec_return = vpx_codec_control(mCodecContext,
                                             VP8E_SET_CPUUSED,
                                             -8);
        }
        if (codec_return != VPX_CODEC_OK) {
            ALOGE("Error setting cbr parameters for vpx encoder.");
            goto CLEAN_UP;
        }
    }

    codec_return = setCodecSpecificControls();

    if (codec_return != VPX_CODEC_OK) {
        // The codec specific method would have logged the error.
        goto CLEAN_UP;
    }

    if (mColorFormat != OMX_COLOR_FormatYUV420Planar || mInputDataIsMeta) {
        free(mConversionBuffer);
        mConversionBuffer = NULL;
        if (((uint64_t)mWidth * mHeight) > ((uint64_t)INT32_MAX / 3)) {
            ALOGE("b/25812794, Buffer size is too big, width=%d, height=%d.", mWidth, mHeight);
            goto CLEAN_UP;
        }
        mConversionBuffer = (uint8_t *)malloc(mWidth * mHeight * 3 / 2);
        if (mConversionBuffer == NULL) {
            ALOGE("Allocating conversion buffer failed.");
            goto CLEAN_UP;
        }
    }
    return OK;

CLEAN_UP:
    releaseEncoder();
    return result;
}

status_t SoftVPXEncoder::releaseEncoder() {
    if (mCodecContext != NULL) {
        vpx_codec_destroy(mCodecContext);
        delete mCodecContext;
        mCodecContext = NULL;
    }

    if (mCodecConfiguration != NULL) {
        delete mCodecConfiguration;
        mCodecConfiguration = NULL;
    }

    if (mConversionBuffer != NULL) {
        free(mConversionBuffer);
        mConversionBuffer = NULL;
    }

    // this one is not allocated by us
    mCodecInterface = NULL;

    return OK;
}

OMX_ERRORTYPE SoftVPXEncoder::internalGetParameter(OMX_INDEXTYPE index,
                                                   OMX_PTR param) {
    // can include extension index OMX_INDEXEXTTYPE
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoBitrate: {
            OMX_VIDEO_PARAM_BITRATETYPE *bitrate =
                (OMX_VIDEO_PARAM_BITRATETYPE *)param;

            if (!isValidOMXParam(bitrate)) {
                return OMX_ErrorBadParameter;
            }

            if (bitrate->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorUnsupportedIndex;
            }

            bitrate->nTargetBitrate = mBitrate;

            if (mBitrateControlMode == VPX_VBR) {
                bitrate->eControlRate = OMX_Video_ControlRateVariable;
            } else if (mBitrateControlMode == VPX_CBR) {
                bitrate->eControlRate = OMX_Video_ControlRateConstant;
            } else {
                return OMX_ErrorUnsupportedSetting;
            }
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoAndroidVp8Encoder:
            return internalGetAndroidVpxParams(
                (OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE *)param);

        default:
            return SoftVideoEncoderOMXComponent::internalGetParameter(index, param);
    }
}

OMX_ERRORTYPE SoftVPXEncoder::internalSetParameter(OMX_INDEXTYPE index,
                                                   const OMX_PTR param) {
    // can include extension index OMX_INDEXEXTTYPE
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoBitrate: {
            const OMX_VIDEO_PARAM_BITRATETYPE *bitRate =
                (const OMX_VIDEO_PARAM_BITRATETYPE*) param;

            if (!isValidOMXParam(bitRate)) {
                return OMX_ErrorBadParameter;
            }

            return internalSetBitrateParams(bitRate);
        }

        case OMX_IndexParamVideoAndroidVp8Encoder:
            return internalSetAndroidVpxParams(
                (const OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE *)param);

        default:
            return SoftVideoEncoderOMXComponent::internalSetParameter(index, param);
    }
}

OMX_ERRORTYPE SoftVPXEncoder::internalSetConfig(
        OMX_INDEXTYPE index, const OMX_PTR _params, bool *frameConfig) {
    switch (index) {
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

            mKeyFrameRequested = params->IntraRefreshVOP;
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
                mBitrateUpdated = true;
            }
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetConfig(index, _params, frameConfig);
    }
}

OMX_ERRORTYPE SoftVPXEncoder::internalGetBitrateParams(
        OMX_VIDEO_PARAM_BITRATETYPE* bitrate) {
    if (bitrate->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    bitrate->nTargetBitrate = mBitrate;

    if (mBitrateControlMode == VPX_VBR) {
        bitrate->eControlRate = OMX_Video_ControlRateVariable;
    } else if (mBitrateControlMode == VPX_CBR) {
        bitrate->eControlRate = OMX_Video_ControlRateConstant;
    } else {
        return OMX_ErrorUnsupportedSetting;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftVPXEncoder::internalSetBitrateParams(
        const OMX_VIDEO_PARAM_BITRATETYPE* bitrate) {
    if (bitrate->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    mBitrate = bitrate->nTargetBitrate;

    if (bitrate->eControlRate == OMX_Video_ControlRateVariable) {
        mBitrateControlMode = VPX_VBR;
    } else if (bitrate->eControlRate == OMX_Video_ControlRateConstant) {
        mBitrateControlMode = VPX_CBR;
    } else {
        return OMX_ErrorUnsupportedSetting;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftVPXEncoder::internalGetAndroidVpxParams(
        OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE *vpxAndroidParams) {
    if (vpxAndroidParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    vpxAndroidParams->nKeyFrameInterval = mKeyFrameInterval;
    vpxAndroidParams->eTemporalPattern = mTemporalPatternType;
    vpxAndroidParams->nTemporalLayerCount = mTemporalLayers;
    vpxAndroidParams->nMinQuantizer = mMinQuantizer;
    vpxAndroidParams->nMaxQuantizer = mMaxQuantizer;
    memcpy(vpxAndroidParams->nTemporalLayerBitrateRatio,
           mTemporalLayerBitrateRatio, sizeof(mTemporalLayerBitrateRatio));
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftVPXEncoder::internalSetAndroidVpxParams(
        const OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE *vpxAndroidParams) {
    if (vpxAndroidParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }
    if (vpxAndroidParams->eTemporalPattern != OMX_VIDEO_VPXTemporalLayerPatternNone &&
            vpxAndroidParams->eTemporalPattern != OMX_VIDEO_VPXTemporalLayerPatternWebRTC) {
        return OMX_ErrorBadParameter;
    }
    if (vpxAndroidParams->nTemporalLayerCount > OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS) {
        return OMX_ErrorBadParameter;
    }
    if (vpxAndroidParams->nMinQuantizer > vpxAndroidParams->nMaxQuantizer) {
        return OMX_ErrorBadParameter;
    }

    mTemporalPatternType = vpxAndroidParams->eTemporalPattern;
    if (vpxAndroidParams->eTemporalPattern == OMX_VIDEO_VPXTemporalLayerPatternWebRTC) {
        mTemporalLayers = vpxAndroidParams->nTemporalLayerCount;
    } else if (vpxAndroidParams->eTemporalPattern == OMX_VIDEO_VPXTemporalLayerPatternNone) {
        mTemporalLayers = 0;
    }
    // Check the bitrate distribution between layers is in increasing order
    if (mTemporalLayers > 1) {
        for (size_t i = 0; i < mTemporalLayers - 1; i++) {
            if (vpxAndroidParams->nTemporalLayerBitrateRatio[i + 1] <=
                    vpxAndroidParams->nTemporalLayerBitrateRatio[i]) {
                ALOGE("Wrong bitrate ratio - should be in increasing order.");
                return OMX_ErrorBadParameter;
            }
        }
    }
    mKeyFrameInterval = vpxAndroidParams->nKeyFrameInterval;
    mMinQuantizer = vpxAndroidParams->nMinQuantizer;
    mMaxQuantizer = vpxAndroidParams->nMaxQuantizer;
    memcpy(mTemporalLayerBitrateRatio, vpxAndroidParams->nTemporalLayerBitrateRatio,
            sizeof(mTemporalLayerBitrateRatio));
    ALOGD("VPx: internalSetAndroidVpxParams. BRMode: %u. TS: %zu. KF: %u."
            " QP: %u - %u BR0: %u. BR1: %u. BR2: %u",
            (uint32_t)mBitrateControlMode, mTemporalLayers, mKeyFrameInterval,
            mMinQuantizer, mMaxQuantizer, mTemporalLayerBitrateRatio[0],
            mTemporalLayerBitrateRatio[1], mTemporalLayerBitrateRatio[2]);
    return OMX_ErrorNone;
}

vpx_enc_frame_flags_t SoftVPXEncoder::getEncodeFlags() {
    vpx_enc_frame_flags_t flags = 0;
    if (mTemporalPatternLength > 0) {
      int patternIdx = mTemporalPatternIdx % mTemporalPatternLength;
      mTemporalPatternIdx++;
      switch (mTemporalPattern[patternIdx]) {
          case kTemporalUpdateLast:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_REF_ARF;
              break;
          case kTemporalUpdateGoldenWithoutDependency:
              flags |= VP8_EFLAG_NO_REF_GF;
              FALLTHROUGH_INTENDED;
          case kTemporalUpdateGolden:
              flags |= VP8_EFLAG_NO_REF_ARF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateAltrefWithoutDependency:
              flags |= VP8_EFLAG_NO_REF_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              FALLTHROUGH_INTENDED;
          case kTemporalUpdateAltref:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateNoneNoRefAltref:
              flags |= VP8_EFLAG_NO_REF_ARF;
              FALLTHROUGH_INTENDED;
          case kTemporalUpdateNone:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              flags |= VP8_EFLAG_NO_UPD_ENTROPY;
              break;
          case kTemporalUpdateNoneNoRefGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              flags |= VP8_EFLAG_NO_UPD_ENTROPY;
              break;
          case kTemporalUpdateGoldenWithoutDependencyRefAltRef:
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateLastRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              break;
          case kTemporalUpdateGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateLastAndGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              break;
          case kTemporalUpdateLastRefAll:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_GF;
              break;
      }
    }
    return flags;
}

void SoftVPXEncoder::onQueueFilled(OMX_U32 /* portIndex */) {
    // Initialize encoder if not already
    if (mCodecContext == NULL) {
        if (OK != initEncoder()) {
            ALOGE("Failed to initialize encoder");
            notify(OMX_EventError,
                   OMX_ErrorUndefined,
                   0,  // Extra notification data
                   NULL);  // Notification data pointer
            return;
        }
    }

    vpx_codec_err_t codec_return;
    List<BufferInfo *> &inputBufferInfoQueue = getPortQueue(kInputPortIndex);
    List<BufferInfo *> &outputBufferInfoQueue = getPortQueue(kOutputPortIndex);

    while (!inputBufferInfoQueue.empty() && !outputBufferInfoQueue.empty()) {
        BufferInfo *inputBufferInfo = *inputBufferInfoQueue.begin();
        OMX_BUFFERHEADERTYPE *inputBufferHeader = inputBufferInfo->mHeader;

        BufferInfo *outputBufferInfo = *outputBufferInfoQueue.begin();
        OMX_BUFFERHEADERTYPE *outputBufferHeader = outputBufferInfo->mHeader;

        if ((inputBufferHeader->nFlags & OMX_BUFFERFLAG_EOS) &&
                inputBufferHeader->nFilledLen == 0) {
            inputBufferInfoQueue.erase(inputBufferInfoQueue.begin());
            inputBufferInfo->mOwnedByUs = false;
            notifyEmptyBufferDone(inputBufferHeader);

            outputBufferHeader->nFilledLen = 0;
            outputBufferHeader->nFlags = OMX_BUFFERFLAG_EOS;

            outputBufferInfoQueue.erase(outputBufferInfoQueue.begin());
            outputBufferInfo->mOwnedByUs = false;
            notifyFillBufferDone(outputBufferHeader);
            return;
        }

        OMX_ERRORTYPE error = validateInputBuffer(inputBufferHeader);
        if (error != OMX_ErrorNone) {
            ALOGE("b/27569635");
            android_errorWriteLog(0x534e4554, "27569635");
            notify(OMX_EventError, error, 0, 0);
            return;
        }
        const uint8_t *source =
            inputBufferHeader->pBuffer + inputBufferHeader->nOffset;

        size_t frameSize = mWidth * mHeight * 3 / 2;
        if (mInputDataIsMeta) {
            source = extractGraphicBuffer(
                    mConversionBuffer, frameSize,
                    source, inputBufferHeader->nFilledLen,
                    mWidth, mHeight);
            if (source == NULL) {
                ALOGE("Unable to extract gralloc buffer in metadata mode");
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                return;
            }
        } else {
            if (mColorFormat == OMX_COLOR_FormatYUV420SemiPlanar) {
                ConvertYUV420SemiPlanarToYUV420Planar(
                        source, mConversionBuffer, mWidth, mHeight);

                source = mConversionBuffer;
            }
        }
        vpx_image_t raw_frame;
        vpx_img_wrap(&raw_frame, VPX_IMG_FMT_I420, mWidth, mHeight,
                     kInputBufferAlignment, (uint8_t *)source);

        vpx_enc_frame_flags_t flags = getEncodeFlags();
        if (mKeyFrameRequested) {
            flags |= VPX_EFLAG_FORCE_KF;
            mKeyFrameRequested = false;
        }

        if (mBitrateUpdated) {
            mCodecConfiguration->rc_target_bitrate = mBitrate/1000;
            vpx_codec_err_t res = vpx_codec_enc_config_set(mCodecContext,
                                                           mCodecConfiguration);
            if (res != VPX_CODEC_OK) {
                ALOGE("vpx encoder failed to update bitrate: %s",
                      vpx_codec_err_to_string(res));
                notify(OMX_EventError,
                       OMX_ErrorUndefined,
                       0, // Extra notification data
                       NULL); // Notification data pointer
            }
            mBitrateUpdated = false;
        }

        uint32_t frameDuration;
        if (inputBufferHeader->nTimeStamp > mLastTimestamp) {
            frameDuration = (uint32_t)(inputBufferHeader->nTimeStamp - mLastTimestamp);
        } else {
            // Use default of 30 fps in case of 0 frame rate.
            uint32_t framerate = mFramerate ?: (30 << 16);
            frameDuration = (uint32_t)(((uint64_t)1000000 << 16) / framerate);
        }
        mLastTimestamp = inputBufferHeader->nTimeStamp;
        codec_return = vpx_codec_encode(
                mCodecContext,
                &raw_frame,
                inputBufferHeader->nTimeStamp,  // in timebase units
                frameDuration,  // frame duration in timebase units
                flags,  // frame flags
                VPX_DL_REALTIME);  // encoding deadline
        if (codec_return != VPX_CODEC_OK) {
            ALOGE("vpx encoder failed to encode frame");
            notify(OMX_EventError,
                   OMX_ErrorUndefined,
                   0,  // Extra notification data
                   NULL);  // Notification data pointer
            return;
        }

        vpx_codec_iter_t encoded_packet_iterator = NULL;
        const vpx_codec_cx_pkt_t* encoded_packet;

        while ((encoded_packet = vpx_codec_get_cx_data(
                        mCodecContext, &encoded_packet_iterator))) {
            if (encoded_packet->kind == VPX_CODEC_CX_FRAME_PKT) {
                outputBufferHeader->nTimeStamp = encoded_packet->data.frame.pts;
                outputBufferHeader->nFlags = 0;
                if (encoded_packet->data.frame.flags & VPX_FRAME_IS_KEY)
                    outputBufferHeader->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                outputBufferHeader->nOffset = 0;
                outputBufferHeader->nFilledLen = encoded_packet->data.frame.sz;
                if (outputBufferHeader->nFilledLen > outputBufferHeader->nAllocLen) {
                    android_errorWriteLog(0x534e4554, "27569635");
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                    return;
                }
                memcpy(outputBufferHeader->pBuffer,
                       encoded_packet->data.frame.buf,
                       encoded_packet->data.frame.sz);
                outputBufferInfo->mOwnedByUs = false;
                outputBufferInfoQueue.erase(outputBufferInfoQueue.begin());
                if (inputBufferHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                    outputBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                }
                notifyFillBufferDone(outputBufferHeader);
            }
        }

        inputBufferInfo->mOwnedByUs = false;
        inputBufferInfoQueue.erase(inputBufferInfoQueue.begin());
        notifyEmptyBufferDone(inputBufferHeader);
    }
}

void SoftVPXEncoder::onReset() {
    releaseEncoder();
    mLastTimestamp = 0x7FFFFFFFFFFFFFFFLL;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
  if (!strcmp(name, "OMX.google.vp8.encoder")) {
      return new android::SoftVP8Encoder(name, callbacks, appData, component);
  } else if (!strcmp(name, "OMX.google.vp9.encoder")) {
      return new android::SoftVP9Encoder(name, callbacks, appData, component);
  } else {
      CHECK(!"Unknown component");
  }
  return NULL;
}
