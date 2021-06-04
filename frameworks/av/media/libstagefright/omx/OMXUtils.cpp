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

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXUtils"

#include <string.h>

#include <android-base/macros.h>
#include <media/stagefright/omx/OMXUtils.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/hardware/HardwareAPI.h>
#include <system/graphics-base.h>

namespace android {

status_t StatusFromOMXError(OMX_ERRORTYPE err) {
    switch (err) {
        case OMX_ErrorNone:
            return OK;
        case OMX_ErrorNoMore:
            return NOT_ENOUGH_DATA;
        case OMX_ErrorUnsupportedSetting:
        case OMX_ErrorUnsupportedIndex:
            return ERROR_UNSUPPORTED; // this is a media specific error
        case OMX_ErrorBadParameter:
            return BAD_VALUE;
        case OMX_ErrorInsufficientResources:
            return NO_MEMORY;
        case OMX_ErrorInvalidComponentName:
        case OMX_ErrorComponentNotFound:
            return NAME_NOT_FOUND;
        default:
            return UNKNOWN_ERROR;
    }
}

/**************************************************************************************************/

DescribeColorFormatParams::DescribeColorFormatParams(const DescribeColorFormat2Params &params) {
    InitOMXParams(this);

    eColorFormat = params.eColorFormat;
    nFrameWidth = params.nFrameWidth;
    nFrameHeight = params.nFrameHeight;
    nStride = params.nStride;
    nSliceHeight = params.nSliceHeight;
    bUsingNativeBuffers = params.bUsingNativeBuffers;
    // we don't copy media images as this conversion is only used pre-query
};

void DescribeColorFormat2Params::initFromV1(const DescribeColorFormatParams &params) {
    InitOMXParams(this);

    eColorFormat = params.eColorFormat;
    nFrameWidth = params.nFrameWidth;
    nFrameHeight = params.nFrameHeight;
    nStride = params.nStride;
    nSliceHeight = params.nSliceHeight;
    bUsingNativeBuffers = params.bUsingNativeBuffers;
    sMediaImage.initFromV1(params.sMediaImage);
};

void MediaImage2::initFromV1(const MediaImage &image) {
    memset(this, 0, sizeof(*this));

    if (image.mType != MediaImage::MEDIA_IMAGE_TYPE_YUV) {
        mType = MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
        return;
    }

    for (size_t ix = 0; ix < image.mNumPlanes; ++ix) {
        if (image.mPlane[ix].mHorizSubsampling > INT32_MAX
                || image.mPlane[ix].mVertSubsampling > INT32_MAX) {
            mType = MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
            return;
        }
    }

    mType = (MediaImage2::Type)image.mType;
    mNumPlanes = image.mNumPlanes;
    mWidth = image.mWidth;
    mHeight = image.mHeight;
    mBitDepth = image.mBitDepth;
    mBitDepthAllocated = 8;
    for (size_t ix = 0; ix < image.mNumPlanes; ++ix) {
        mPlane[ix].mOffset = image.mPlane[ix].mOffset;
        mPlane[ix].mColInc = image.mPlane[ix].mColInc;
        mPlane[ix].mRowInc = image.mPlane[ix].mRowInc;
        mPlane[ix].mHorizSubsampling = (int32_t)image.mPlane[ix].mHorizSubsampling;
        mPlane[ix].mVertSubsampling = (int32_t)image.mPlane[ix].mVertSubsampling;
    }
}

/**************************************************************************************************/

const char *GetComponentRole(bool isEncoder, const char *mime) {
    struct MimeToRole {
        const char *mime;
        const char *decoderRole;
        const char *encoderRole;
    };

    static const MimeToRole kMimeToRole[] = {
        { MEDIA_MIMETYPE_AUDIO_MPEG,
            "audio_decoder.mp3", "audio_encoder.mp3" },
        { MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I,
            "audio_decoder.mp1", "audio_encoder.mp1" },
        { MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II,
            "audio_decoder.mp2", "audio_encoder.mp2" },
        { MEDIA_MIMETYPE_AUDIO_AMR_NB,
            "audio_decoder.amrnb", "audio_encoder.amrnb" },
        { MEDIA_MIMETYPE_AUDIO_AMR_WB,
            "audio_decoder.amrwb", "audio_encoder.amrwb" },
        { MEDIA_MIMETYPE_AUDIO_AAC,
            "audio_decoder.aac", "audio_encoder.aac" },
        { MEDIA_MIMETYPE_AUDIO_VORBIS,
            "audio_decoder.vorbis", "audio_encoder.vorbis" },
        { MEDIA_MIMETYPE_AUDIO_OPUS,
            "audio_decoder.opus", "audio_encoder.opus" },
        { MEDIA_MIMETYPE_AUDIO_G711_MLAW,
            "audio_decoder.g711mlaw", "audio_encoder.g711mlaw" },
        { MEDIA_MIMETYPE_AUDIO_G711_ALAW,
            "audio_decoder.g711alaw", "audio_encoder.g711alaw" },
        { MEDIA_MIMETYPE_VIDEO_AVC,
            "video_decoder.avc", "video_encoder.avc" },
        { MEDIA_MIMETYPE_VIDEO_HEVC,
            "video_decoder.hevc", "video_encoder.hevc" },
        { MEDIA_MIMETYPE_VIDEO_MPEG4,
            "video_decoder.mpeg4", "video_encoder.mpeg4" },
        { MEDIA_MIMETYPE_VIDEO_H263,
            "video_decoder.h263", "video_encoder.h263" },
        { MEDIA_MIMETYPE_VIDEO_VP8,
            "video_decoder.vp8", "video_encoder.vp8" },
        { MEDIA_MIMETYPE_VIDEO_VP9,
            "video_decoder.vp9", "video_encoder.vp9" },
        { MEDIA_MIMETYPE_VIDEO_AV1,
            "video_decoder.av1", "video_encoder.av1" },
        { MEDIA_MIMETYPE_AUDIO_RAW,
            "audio_decoder.raw", "audio_encoder.raw" },
        { MEDIA_MIMETYPE_VIDEO_DOLBY_VISION,
            "video_decoder.dolby-vision", "video_encoder.dolby-vision" },
        { MEDIA_MIMETYPE_AUDIO_FLAC,
            "audio_decoder.flac", "audio_encoder.flac" },
        { MEDIA_MIMETYPE_AUDIO_MSGSM,
            "audio_decoder.gsm", "audio_encoder.gsm" },
        { MEDIA_MIMETYPE_VIDEO_MPEG2,
            "video_decoder.mpeg2", "video_encoder.mpeg2" },
        { MEDIA_MIMETYPE_AUDIO_AC3,
            "audio_decoder.ac3", "audio_encoder.ac3" },
        { MEDIA_MIMETYPE_AUDIO_EAC3,
            "audio_decoder.eac3", "audio_encoder.eac3" },
        { MEDIA_MIMETYPE_AUDIO_EAC3_JOC,
            "audio_decoder.eac3_joc", "audio_encoder.eac3_joc" },
        { MEDIA_MIMETYPE_AUDIO_AC4,
            "audio_decoder.ac4", "audio_encoder.ac4" },
        { MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC,
            "image_decoder.heic", "image_encoder.heic" },
    };

    static const size_t kNumMimeToRole =
        sizeof(kMimeToRole) / sizeof(kMimeToRole[0]);

    size_t i;
    for (i = 0; i < kNumMimeToRole; ++i) {
        if (!strcasecmp(mime, kMimeToRole[i].mime)) {
            break;
        }
    }

    if (i == kNumMimeToRole) {
        return NULL;
    }

    return isEncoder ? kMimeToRole[i].encoderRole
                  : kMimeToRole[i].decoderRole;
}

status_t SetComponentRole(const sp<IOMXNode> &omxNode, const char *role) {
    OMX_PARAM_COMPONENTROLETYPE roleParams;
    InitOMXParams(&roleParams);

    strncpy((char *)roleParams.cRole,
            role, OMX_MAX_STRINGNAME_SIZE - 1);

    roleParams.cRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';

    return omxNode->setParameter(
            OMX_IndexParamStandardComponentRole,
            &roleParams, sizeof(roleParams));
}

bool DescribeDefaultColorFormat(DescribeColorFormat2Params &params) {
    MediaImage2 &image = params.sMediaImage;
    memset(&image, 0, sizeof(image));

    image.mType = MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
    image.mNumPlanes = 0;

    const OMX_COLOR_FORMATTYPE fmt = params.eColorFormat;
    image.mWidth = params.nFrameWidth;
    image.mHeight = params.nFrameHeight;

    // only supporting YUV420
    if (fmt != OMX_COLOR_FormatYUV420Planar &&
        fmt != OMX_COLOR_FormatYUV420PackedPlanar &&
        fmt != OMX_COLOR_FormatYUV420SemiPlanar &&
        fmt != OMX_COLOR_FormatYUV420PackedSemiPlanar &&
        fmt != (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YV12) {
        ALOGW("do not know color format 0x%x = %d", fmt, fmt);
        if (fmt == OMX_COLOR_FormatYUV420Planar16) {
            ALOGW("Cannot describe color format OMX_COLOR_FormatYUV420Planar16");
        }
        return false;
    }

    // TEMPORARY FIX for some vendors that advertise sliceHeight as 0
    if (params.nStride != 0 && params.nSliceHeight == 0) {
        ALOGW("using sliceHeight=%u instead of what codec advertised (=0)",
                params.nFrameHeight);
        params.nSliceHeight = params.nFrameHeight;
    }

    // we need stride and slice-height to be non-zero and sensible. These values were chosen to
    // prevent integer overflows further down the line, and do not indicate support for
    // 32kx32k video.
    if (params.nStride == 0 || params.nSliceHeight == 0
            || params.nStride > 32768 || params.nSliceHeight > 32768) {
        ALOGW("cannot describe color format 0x%x = %d with stride=%u and sliceHeight=%u",
                fmt, fmt, params.nStride, params.nSliceHeight);
        return false;
    }

    // set-up YUV format
    image.mType = MediaImage2::MEDIA_IMAGE_TYPE_YUV;
    image.mNumPlanes = 3;
    image.mBitDepth = 8;
    image.mBitDepthAllocated = 8;
    image.mPlane[image.Y].mOffset = 0;
    image.mPlane[image.Y].mColInc = 1;
    image.mPlane[image.Y].mRowInc = params.nStride;
    image.mPlane[image.Y].mHorizSubsampling = 1;
    image.mPlane[image.Y].mVertSubsampling = 1;

    switch ((int)fmt) {
        case HAL_PIXEL_FORMAT_YV12:
            if (params.bUsingNativeBuffers) {
                size_t ystride = align(params.nStride, 16);
                size_t cstride = align(params.nStride / 2, 16);
                image.mPlane[image.Y].mRowInc = ystride;

                image.mPlane[image.V].mOffset = ystride * params.nSliceHeight;
                image.mPlane[image.V].mColInc = 1;
                image.mPlane[image.V].mRowInc = cstride;
                image.mPlane[image.V].mHorizSubsampling = 2;
                image.mPlane[image.V].mVertSubsampling = 2;

                image.mPlane[image.U].mOffset = image.mPlane[image.V].mOffset
                        + (cstride * params.nSliceHeight / 2);
                image.mPlane[image.U].mColInc = 1;
                image.mPlane[image.U].mRowInc = cstride;
                image.mPlane[image.U].mHorizSubsampling = 2;
                image.mPlane[image.U].mVertSubsampling = 2;
                break;
            } else {
                // fall through as YV12 is used for YUV420Planar by some codecs
                FALLTHROUGH_INTENDED;
            }

        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420PackedPlanar:
            image.mPlane[image.U].mOffset = params.nStride * params.nSliceHeight;
            image.mPlane[image.U].mColInc = 1;
            image.mPlane[image.U].mRowInc = params.nStride / 2;
            image.mPlane[image.U].mHorizSubsampling = 2;
            image.mPlane[image.U].mVertSubsampling = 2;

            image.mPlane[image.V].mOffset = image.mPlane[image.U].mOffset
                    + (params.nStride * params.nSliceHeight / 4);
            image.mPlane[image.V].mColInc = 1;
            image.mPlane[image.V].mRowInc = params.nStride / 2;
            image.mPlane[image.V].mHorizSubsampling = 2;
            image.mPlane[image.V].mVertSubsampling = 2;
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
            // FIXME: NV21 for sw-encoder, NV12 for decoder and hw-encoder
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            // NV12
            image.mPlane[image.U].mOffset = params.nStride * params.nSliceHeight;
            image.mPlane[image.U].mColInc = 2;
            image.mPlane[image.U].mRowInc = params.nStride;
            image.mPlane[image.U].mHorizSubsampling = 2;
            image.mPlane[image.U].mVertSubsampling = 2;

            image.mPlane[image.V].mOffset = image.mPlane[image.U].mOffset + 1;
            image.mPlane[image.V].mColInc = 2;
            image.mPlane[image.V].mRowInc = params.nStride;
            image.mPlane[image.V].mHorizSubsampling = 2;
            image.mPlane[image.V].mVertSubsampling = 2;
            break;

        default:
            TRESPASS();
    }
    return true;
}

bool DescribeColorFormat(
        const sp<IOMXNode> &omxNode,
        DescribeColorFormat2Params &describeParams)
{
    OMX_INDEXTYPE describeColorFormatIndex;
    if (omxNode->getExtensionIndex(
            "OMX.google.android.index.describeColorFormat",
            &describeColorFormatIndex) == OK) {
        DescribeColorFormatParams describeParamsV1(describeParams);
        if (omxNode->getParameter(
                describeColorFormatIndex,
                &describeParamsV1, sizeof(describeParamsV1)) == OK) {
            describeParams.initFromV1(describeParamsV1);
            return describeParams.sMediaImage.mType != MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
        }
    } else if (omxNode->getExtensionIndex(
            "OMX.google.android.index.describeColorFormat2", &describeColorFormatIndex) == OK
               && omxNode->getParameter(
                       describeColorFormatIndex, &describeParams, sizeof(describeParams)) == OK) {
        return describeParams.sMediaImage.mType != MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
    }

    return DescribeDefaultColorFormat(describeParams);
}

// static
bool IsFlexibleColorFormat(
         const sp<IOMXNode> &omxNode,
         uint32_t colorFormat, bool usingNativeBuffers, OMX_U32 *flexibleEquivalent) {
    DescribeColorFormat2Params describeParams;
    InitOMXParams(&describeParams);
    describeParams.eColorFormat = (OMX_COLOR_FORMATTYPE)colorFormat;
    // reasonable dummy values
    describeParams.nFrameWidth = 128;
    describeParams.nFrameHeight = 128;
    describeParams.nStride = 128;
    describeParams.nSliceHeight = 128;
    describeParams.bUsingNativeBuffers = (OMX_BOOL)usingNativeBuffers;

    CHECK(flexibleEquivalent != NULL);

    if (!DescribeColorFormat(omxNode, describeParams)) {
        return false;
    }

    const MediaImage2 &img = describeParams.sMediaImage;
    if (img.mType == MediaImage2::MEDIA_IMAGE_TYPE_YUV) {
        if (img.mNumPlanes != 3
                || img.mPlane[img.Y].mHorizSubsampling != 1
                || img.mPlane[img.Y].mVertSubsampling != 1) {
            return false;
        }

        // YUV 420
        if (img.mPlane[img.U].mHorizSubsampling == 2
                && img.mPlane[img.U].mVertSubsampling == 2
                && img.mPlane[img.V].mHorizSubsampling == 2
                && img.mPlane[img.V].mVertSubsampling == 2) {
            // possible flexible YUV420 format
            if (img.mBitDepth <= 8) {
               *flexibleEquivalent = OMX_COLOR_FormatYUV420Flexible;
               return true;
            }
        }
    }
    return false;
}

}  // namespace android

