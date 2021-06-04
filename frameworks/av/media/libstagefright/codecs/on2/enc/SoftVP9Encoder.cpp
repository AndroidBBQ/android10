/*
 * Copyright (C) 2016 The Android Open Source Project
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
#define LOG_TAG "SoftVP9Encoder"
#include "SoftVP9Encoder.h"

#include <utils/Log.h>
#include <utils/misc.h>

#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>

namespace android {

static const CodecProfileLevel kVp9ProfileLevels[] = {
    { OMX_VIDEO_VP9Profile0, OMX_VIDEO_VP9Level41 },
};

SoftVP9Encoder::SoftVP9Encoder(
        const char *name, const OMX_CALLBACKTYPE *callbacks, OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SoftVPXEncoder(
            name, callbacks, appData, component, "video_encoder.vp9",
            OMX_VIDEO_CodingVP9, MEDIA_MIMETYPE_VIDEO_VP9, 4,
            kVp9ProfileLevels, NELEM(kVp9ProfileLevels)),
      mLevel(OMX_VIDEO_VP9Level1),
      mTileColumns(0),
      mFrameParallelDecoding(OMX_FALSE) {
}

void SoftVP9Encoder::setCodecSpecificInterface() {
    mCodecInterface = vpx_codec_vp9_cx();
}

void SoftVP9Encoder::setCodecSpecificConfiguration() {
    mCodecConfiguration->g_profile = 0;
}

vpx_codec_err_t SoftVP9Encoder::setCodecSpecificControls() {
    vpx_codec_err_t codecReturn = vpx_codec_control(
            mCodecContext, VP9E_SET_TILE_COLUMNS, mTileColumns);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_TILE_COLUMNS to %d. vpx_codec_control() "
              "returned %d", mTileColumns, codecReturn);
        return codecReturn;
    }
    codecReturn = vpx_codec_control(
            mCodecContext, VP9E_SET_FRAME_PARALLEL_DECODING,
            mFrameParallelDecoding);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_FRAME_PARALLEL_DECODING to %d."
              "vpx_codec_control() returned %d", mFrameParallelDecoding,
              codecReturn);
        return codecReturn;
    }
    codecReturn = vpx_codec_control(mCodecContext, VP9E_SET_ROW_MT, 1);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_ROW_MT to 1. vpx_codec_control() "
              "returned %d", codecReturn);
        return codecReturn;
    }

    // For VP9, we always set CPU_USED to 8 (because the realtime default is 0
    // which is too slow).
    codecReturn = vpx_codec_control(mCodecContext, VP8E_SET_CPUUSED, 8);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP8E_SET_CPUUSED to 8. vpx_codec_control() "
              "returned %d", codecReturn);
        return codecReturn;
    }
    return codecReturn;
}

OMX_ERRORTYPE SoftVP9Encoder::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR param) {
    // can include extension index OMX_INDEXEXTTYPE
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoVp9:
            return internalGetVp9Params(
                    (OMX_VIDEO_PARAM_VP9TYPE *)param);

        default:
            return SoftVPXEncoder::internalGetParameter(index, param);
    }
}

OMX_ERRORTYPE SoftVP9Encoder::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR param) {
    // can include extension index OMX_INDEXEXTTYPE
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoVp9:
            return internalSetVp9Params(
                    (const OMX_VIDEO_PARAM_VP9TYPE *)param);

        default:
            return SoftVPXEncoder::internalSetParameter(index, param);
    }
}

OMX_ERRORTYPE SoftVP9Encoder::internalGetVp9Params(
        OMX_VIDEO_PARAM_VP9TYPE *vp9Params) {
    if (vp9Params->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    vp9Params->eProfile = OMX_VIDEO_VP9Profile0;
    vp9Params->eLevel = mLevel;
    vp9Params->bErrorResilientMode = mErrorResilience;
    vp9Params->nTileColumns = mTileColumns;
    vp9Params->bEnableFrameParallelDecoding = mFrameParallelDecoding;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftVP9Encoder::internalSetVp9Params(
        const OMX_VIDEO_PARAM_VP9TYPE *vp9Params) {
    if (vp9Params->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorUnsupportedIndex;
    }

    if (vp9Params->eProfile != OMX_VIDEO_VP9Profile0) {
        return OMX_ErrorBadParameter;
    }

    if (vp9Params->eLevel == OMX_VIDEO_VP9Level1 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level11 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level2 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level21 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level3 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level31 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level4 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level41 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level5 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level51 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level52 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level6 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level61 ||
        vp9Params->eLevel == OMX_VIDEO_VP9Level62) {
        mLevel = vp9Params->eLevel;
    } else {
        return OMX_ErrorBadParameter;
    }

    mErrorResilience = vp9Params->bErrorResilientMode;
    mTileColumns = vp9Params->nTileColumns;
    mFrameParallelDecoding = vp9Params->bEnableFrameParallelDecoding;
    return OMX_ErrorNone;
}

}  // namespace android
