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

#include <inttypes.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "SoftVideoDecoderOMXComponent"
#include <utils/Log.h>

#include <media/stagefright/omx/SoftVideoDecoderOMXComponent.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/hardware/HardwareAPI.h>

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftVideoDecoderOMXComponent::SoftVideoDecoderOMXComponent(
        const char *name,
        const char *componentRole,
        OMX_VIDEO_CODINGTYPE codingType,
        const CodecProfileLevel *profileLevels,
        size_t numProfileLevels,
        int32_t width,
        int32_t height,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
        : SimpleSoftOMXComponent(name, callbacks, appData, component),
        mIsAdaptive(false),
        mAdaptiveMaxWidth(0),
        mAdaptiveMaxHeight(0),
        mWidth(width),
        mHeight(height),
        mCropLeft(0),
        mCropTop(0),
        mCropWidth(width),
        mCropHeight(height),
        mOutputFormat(OMX_COLOR_FormatYUV420Planar),
        mOutputPortSettingsChange(NONE),
        mUpdateColorAspects(false),
        mMinInputBufferSize(384), // arbitrary, using one uncompressed macroblock
        mMinCompressionRatio(1),  // max input size is normally the output size
        mComponentRole(componentRole),
        mCodingType(codingType),
        mProfileLevels(profileLevels),
        mNumProfileLevels(numProfileLevels) {

    // init all the color aspects to be Unspecified.
    memset(&mDefaultColorAspects, 0, sizeof(ColorAspects));
    memset(&mBitstreamColorAspects, 0, sizeof(ColorAspects));
    memset(&mFinalColorAspects, 0, sizeof(ColorAspects));
    memset(&mHdrStaticInfo, 0, sizeof(HDRStaticInfo));
}

void SoftVideoDecoderOMXComponent::initPorts(
        OMX_U32 numInputBuffers,
        OMX_U32 inputBufferSize,
        OMX_U32 numOutputBuffers,
        const char *mimeType,
        OMX_U32 minCompressionRatio) {
    initPorts(numInputBuffers, numInputBuffers, inputBufferSize,
            numOutputBuffers, numOutputBuffers, mimeType, minCompressionRatio);
}

void SoftVideoDecoderOMXComponent::initPorts(
        OMX_U32 numMinInputBuffers,
        OMX_U32 numInputBuffers,
        OMX_U32 inputBufferSize,
        OMX_U32 numMinOutputBuffers,
        OMX_U32 numOutputBuffers,
        const char *mimeType,
        OMX_U32 minCompressionRatio) {
    mMinInputBufferSize = inputBufferSize;
    mMinCompressionRatio = minCompressionRatio;

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    def.nPortIndex = kInputPortIndex;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = numMinInputBuffers;
    def.nBufferCountActual = numInputBuffers;
    def.nBufferSize = inputBufferSize;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainVideo;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 1;

    def.format.video.cMIMEType = const_cast<char *>(mimeType);
    def.format.video.pNativeRender = NULL;
    /* size is initialized in updatePortDefinitions() */
    def.format.video.nBitrate = 0;
    def.format.video.xFramerate = 0;
    def.format.video.bFlagErrorConcealment = OMX_FALSE;
    def.format.video.eCompressionFormat = mCodingType;
    def.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    def.format.video.pNativeWindow = NULL;

    addPort(def);

    def.nPortIndex = kOutputPortIndex;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = numMinOutputBuffers;
    def.nBufferCountActual = numOutputBuffers;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainVideo;
    def.bBuffersContiguous = OMX_FALSE;
    def.nBufferAlignment = 2;

    def.format.video.cMIMEType = const_cast<char *>("video/raw");
    def.format.video.pNativeRender = NULL;
    /* size is initialized in updatePortDefinitions() */
    def.format.video.nBitrate = 0;
    def.format.video.xFramerate = 0;
    def.format.video.bFlagErrorConcealment = OMX_FALSE;
    def.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    def.format.video.pNativeWindow = NULL;

    addPort(def);

    updatePortDefinitions(true /* updateCrop */, true /* updateInputSize */);
}

void SoftVideoDecoderOMXComponent::updatePortDefinitions(bool updateCrop, bool updateInputSize) {
    OMX_PARAM_PORTDEFINITIONTYPE *outDef = &editPortInfo(kOutputPortIndex)->mDef;
    outDef->format.video.nFrameWidth = outputBufferWidth();
    outDef->format.video.nFrameHeight = outputBufferHeight();
    outDef->format.video.eColorFormat = mOutputFormat;
    outDef->format.video.nSliceHeight = outDef->format.video.nFrameHeight;

    int32_t bpp = (mOutputFormat == OMX_COLOR_FormatYUV420Planar16) ? 2 : 1;
    outDef->format.video.nStride = outDef->format.video.nFrameWidth * bpp;
    outDef->nBufferSize =
            (outDef->format.video.nStride * outDef->format.video.nSliceHeight * 3) / 2;

    OMX_PARAM_PORTDEFINITIONTYPE *inDef = &editPortInfo(kInputPortIndex)->mDef;
    inDef->format.video.nFrameWidth = mWidth;
    inDef->format.video.nFrameHeight = mHeight;
    // input port is compressed, hence it has no stride
    inDef->format.video.nStride = 0;
    inDef->format.video.nSliceHeight = 0;

    // when output format changes, input buffer size does not actually change
    if (updateInputSize) {
        inDef->nBufferSize = max(
                outDef->nBufferSize / mMinCompressionRatio,
                max(mMinInputBufferSize, inDef->nBufferSize));
    }

    if (updateCrop) {
        mCropLeft = 0;
        mCropTop = 0;
        mCropWidth = mWidth;
        mCropHeight = mHeight;
    }
}


uint32_t SoftVideoDecoderOMXComponent::outputBufferWidth() {
    return max(mIsAdaptive ? mAdaptiveMaxWidth : 0, mWidth);
}

uint32_t SoftVideoDecoderOMXComponent::outputBufferHeight() {
    return max(mIsAdaptive ? mAdaptiveMaxHeight : 0, mHeight);
}

void SoftVideoDecoderOMXComponent::handlePortSettingsChange(
        bool *portWillReset, uint32_t width, uint32_t height,
        OMX_COLOR_FORMATTYPE outputFormat,
        CropSettingsMode cropSettingsMode, bool fakeStride) {
    *portWillReset = false;
    bool sizeChanged = (width != mWidth || height != mHeight);
    bool formatChanged = (outputFormat != mOutputFormat);
    bool updateCrop = (cropSettingsMode == kCropUnSet);
    bool cropChanged = (cropSettingsMode == kCropChanged);
    bool strideChanged = false;
    if (fakeStride) {
        OMX_PARAM_PORTDEFINITIONTYPE *def = &editPortInfo(kOutputPortIndex)->mDef;
        if (def->format.video.nStride != (OMX_S32)width
                || def->format.video.nSliceHeight != (OMX_U32)height) {
            strideChanged = true;
        }
    }

    if (formatChanged || sizeChanged || cropChanged || strideChanged) {
        if (formatChanged) {
            ALOGD("formatChanged: 0x%08x -> 0x%08x", mOutputFormat, outputFormat);
        }
        mOutputFormat = outputFormat;
        mWidth = width;
        mHeight = height;

        if ((sizeChanged && !mIsAdaptive)
            || width > mAdaptiveMaxWidth
            || height > mAdaptiveMaxHeight
            || formatChanged) {
            if (mIsAdaptive) {
                if (width > mAdaptiveMaxWidth) {
                    mAdaptiveMaxWidth = width;
                }
                if (height > mAdaptiveMaxHeight) {
                    mAdaptiveMaxHeight = height;
                }
            }
            updatePortDefinitions(updateCrop);
            notify(OMX_EventPortSettingsChanged, kOutputPortIndex, 0, NULL);
            mOutputPortSettingsChange = AWAITING_DISABLED;
            *portWillReset = true;
        } else {
            updatePortDefinitions(updateCrop);

            if (fakeStride) {
                // MAJOR HACK that is not pretty, it's just to fool the renderer to read the correct
                // data.
                // Some software decoders (e.g. SoftMPEG4) fill decoded frame directly to output
                // buffer without considering the output buffer stride and slice height. So this is
                // used to signal how the buffer is arranged.  The alternative is to re-arrange the
                // output buffer in SoftMPEG4, but that results in memcopies.
                OMX_PARAM_PORTDEFINITIONTYPE *def = &editPortInfo(kOutputPortIndex)->mDef;
                def->format.video.nStride = mWidth;
                def->format.video.nSliceHeight = mHeight;
            }

            notify(OMX_EventPortSettingsChanged, kOutputPortIndex,
                   OMX_IndexConfigCommonOutputCrop, NULL);
        }
    } else if (mUpdateColorAspects) {
        notify(OMX_EventPortSettingsChanged, kOutputPortIndex,
                kDescribeColorAspectsIndex, NULL);
        mUpdateColorAspects = false;
    }
}

void SoftVideoDecoderOMXComponent::dumpColorAspects(const ColorAspects &colorAspects) {
    ALOGD("dumpColorAspects: (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) ",
            colorAspects.mRange, asString(colorAspects.mRange),
            colorAspects.mPrimaries, asString(colorAspects.mPrimaries),
            colorAspects.mMatrixCoeffs, asString(colorAspects.mMatrixCoeffs),
            colorAspects.mTransfer, asString(colorAspects.mTransfer));
}

bool SoftVideoDecoderOMXComponent::colorAspectsDiffer(
        const ColorAspects &a, const ColorAspects &b) {
    if (a.mRange != b.mRange
        || a.mPrimaries != b.mPrimaries
        || a.mTransfer != b.mTransfer
        || a.mMatrixCoeffs != b.mMatrixCoeffs) {
        return true;
    }
    return false;
}

void SoftVideoDecoderOMXComponent::updateFinalColorAspects(
        const ColorAspects &otherAspects, const ColorAspects &preferredAspects) {
    Mutex::Autolock autoLock(mColorAspectsLock);
    ColorAspects newAspects;
    newAspects.mRange = preferredAspects.mRange != ColorAspects::RangeUnspecified ?
        preferredAspects.mRange : otherAspects.mRange;
    newAspects.mPrimaries = preferredAspects.mPrimaries != ColorAspects::PrimariesUnspecified ?
        preferredAspects.mPrimaries : otherAspects.mPrimaries;
    newAspects.mTransfer = preferredAspects.mTransfer != ColorAspects::TransferUnspecified ?
        preferredAspects.mTransfer : otherAspects.mTransfer;
    newAspects.mMatrixCoeffs = preferredAspects.mMatrixCoeffs != ColorAspects::MatrixUnspecified ?
        preferredAspects.mMatrixCoeffs : otherAspects.mMatrixCoeffs;

    // Check to see if need update mFinalColorAspects.
    if (colorAspectsDiffer(mFinalColorAspects, newAspects)) {
        mFinalColorAspects = newAspects;
        mUpdateColorAspects = true;
    }
}

status_t SoftVideoDecoderOMXComponent::handleColorAspectsChange() {
    int perference = getColorAspectPreference();
    ALOGD("Color Aspects preference: %d ", perference);

    if (perference == kPreferBitstream) {
        updateFinalColorAspects(mDefaultColorAspects, mBitstreamColorAspects);
    } else if (perference == kPreferContainer) {
        updateFinalColorAspects(mBitstreamColorAspects, mDefaultColorAspects);
    } else {
        return OMX_ErrorUnsupportedSetting;
    }
    return OK;
}

void SoftVideoDecoderOMXComponent::copyYV12FrameToOutputBuffer(
        uint8_t *dst, const uint8_t *srcY, const uint8_t *srcU, const uint8_t *srcV,
        size_t srcYStride, size_t srcUStride, size_t srcVStride) {
    OMX_PARAM_PORTDEFINITIONTYPE *outDef = &editPortInfo(kOutputPortIndex)->mDef;
    int32_t bpp = (outDef->format.video.eColorFormat == OMX_COLOR_FormatYUV420Planar16) ? 2 : 1;

    size_t dstYStride = outputBufferWidth() * bpp;
    size_t dstUVStride = dstYStride / 2;
    size_t dstHeight = outputBufferHeight();
    uint8_t *dstStart = dst;

    for (size_t i = 0; i < mHeight; ++i) {
         memcpy(dst, srcY, mWidth * bpp);
         srcY += srcYStride;
         dst += dstYStride;
    }

    dst = dstStart + dstYStride * dstHeight;
    for (size_t i = 0; i < mHeight / 2; ++i) {
         memcpy(dst, srcU, mWidth / 2 * bpp);
         srcU += srcUStride;
         dst += dstUVStride;
    }

    dst = dstStart + (5 * dstYStride * dstHeight) / 4;
    for (size_t i = 0; i < mHeight / 2; ++i) {
         memcpy(dst, srcV, mWidth / 2 * bpp);
         srcV += srcVStride;
         dst += dstUVStride;
    }
}

OMX_ERRORTYPE SoftVideoDecoderOMXComponent::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch (index) {
        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *formatParams =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)params;

            if (!isValidOMXParam(formatParams)) {
                return OMX_ErrorBadParameter;
            }

            if (formatParams->nPortIndex > kMaxPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            if (formatParams->nIndex != 0) {
                return OMX_ErrorNoMore;
            }

            if (formatParams->nPortIndex == kInputPortIndex) {
                formatParams->eCompressionFormat = mCodingType;
                formatParams->eColorFormat = OMX_COLOR_FormatUnused;
                formatParams->xFramerate = 0;
            } else {
                CHECK_EQ(formatParams->nPortIndex, 1u);

                formatParams->eCompressionFormat = OMX_VIDEO_CodingUnused;
                formatParams->eColorFormat = OMX_COLOR_FormatYUV420Planar;
                formatParams->xFramerate = 0;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel =
                  (OMX_VIDEO_PARAM_PROFILELEVELTYPE *) params;

            if (!isValidOMXParam(profileLevel)) {
                return OMX_ErrorBadParameter;
            }

            if (profileLevel->nPortIndex != kInputPortIndex) {
                ALOGE("Invalid port index: %" PRIu32, profileLevel->nPortIndex);
                return OMX_ErrorUnsupportedIndex;
            }

            if (profileLevel->nProfileIndex >= mNumProfileLevels) {
                return OMX_ErrorNoMore;
            }

            profileLevel->eProfile = mProfileLevels[profileLevel->nProfileIndex].mProfile;
            profileLevel->eLevel   = mProfileLevels[profileLevel->nProfileIndex].mLevel;
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftVideoDecoderOMXComponent::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params) {
    // Include extension index OMX_INDEXEXTTYPE.
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamStandardComponentRole:
        {
            const OMX_PARAM_COMPONENTROLETYPE *roleParams =
                (const OMX_PARAM_COMPONENTROLETYPE *)params;

            if (!isValidOMXParam(roleParams)) {
                return OMX_ErrorBadParameter;
            }

            if (strncmp((const char *)roleParams->cRole,
                        mComponentRole,
                        OMX_MAX_STRINGNAME_SIZE - 1)) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *formatParams =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)params;

            if (!isValidOMXParam(formatParams)) {
                return OMX_ErrorBadParameter;
            }

            if (formatParams->nPortIndex > kMaxPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            if (formatParams->nPortIndex == kInputPortIndex) {
                if (formatParams->eCompressionFormat != mCodingType
                        || formatParams->eColorFormat != OMX_COLOR_FormatUnused) {
                    return OMX_ErrorUnsupportedSetting;
                }
            } else {
                if (formatParams->eCompressionFormat != OMX_VIDEO_CodingUnused
                        || formatParams->eColorFormat != OMX_COLOR_FormatYUV420Planar) {
                    return OMX_ErrorUnsupportedSetting;
                }
            }

            return OMX_ErrorNone;
        }

        case kPrepareForAdaptivePlaybackIndex:
        {
            const PrepareForAdaptivePlaybackParams* adaptivePlaybackParams =
                    (const PrepareForAdaptivePlaybackParams *)params;

            if (!isValidOMXParam(adaptivePlaybackParams)) {
                return OMX_ErrorBadParameter;
            }

            mIsAdaptive = adaptivePlaybackParams->bEnable;
            if (mIsAdaptive) {
                mAdaptiveMaxWidth = adaptivePlaybackParams->nMaxFrameWidth;
                mAdaptiveMaxHeight = adaptivePlaybackParams->nMaxFrameHeight;
                mWidth = mAdaptiveMaxWidth;
                mHeight = mAdaptiveMaxHeight;
            } else {
                mAdaptiveMaxWidth = 0;
                mAdaptiveMaxHeight = 0;
            }
            updatePortDefinitions(true /* updateCrop */, true /* updateInputSize */);
            return OMX_ErrorNone;
        }

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *newParams =
                (OMX_PARAM_PORTDEFINITIONTYPE *)params;

            if (!isValidOMXParam(newParams)) {
                return OMX_ErrorBadParameter;
            }

            OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &newParams->format.video;
            OMX_PARAM_PORTDEFINITIONTYPE *def = &editPortInfo(newParams->nPortIndex)->mDef;

            uint32_t oldWidth = def->format.video.nFrameWidth;
            uint32_t oldHeight = def->format.video.nFrameHeight;
            uint32_t newWidth = video_def->nFrameWidth;
            uint32_t newHeight = video_def->nFrameHeight;
            // We need width, height, stride and slice-height to be non-zero and sensible.
            // These values were chosen to prevent integer overflows further down the line, and do
            // not indicate support for 32kx32k video.
            if (newWidth > 32768 || newHeight > 32768
                    || video_def->nStride > 32768 || video_def->nStride < -32768
                    || video_def->nSliceHeight > 32768) {
                ALOGE("b/22885421");
                return OMX_ErrorBadParameter;
            }
            if (newWidth != oldWidth || newHeight != oldHeight) {
                bool outputPort = (newParams->nPortIndex == kOutputPortIndex);
                if (outputPort) {
                    // only update (essentially crop) if size changes
                    mWidth = newWidth;
                    mHeight = newHeight;

                    updatePortDefinitions(true /* updateCrop */, true /* updateInputSize */);
                    // reset buffer size based on frame size
                    newParams->nBufferSize = def->nBufferSize;
                } else {
                    // For input port, we only set nFrameWidth and nFrameHeight. Buffer size
                    // is updated when configuring the output port using the max-frame-size,
                    // though client can still request a larger size.
                    def->format.video.nFrameWidth = newWidth;
                    def->format.video.nFrameHeight = newHeight;
                }
            }
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftVideoDecoderOMXComponent::getConfig(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch ((int)index) {
        case OMX_IndexConfigCommonOutputCrop:
        {
            OMX_CONFIG_RECTTYPE *rectParams = (OMX_CONFIG_RECTTYPE *)params;

            if (!isValidOMXParam(rectParams)) {
                return OMX_ErrorBadParameter;
            }

            if (rectParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorUndefined;
            }

            rectParams->nLeft = mCropLeft;
            rectParams->nTop = mCropTop;
            rectParams->nWidth = mCropWidth;
            rectParams->nHeight = mCropHeight;

            return OMX_ErrorNone;
        }
        case kDescribeColorAspectsIndex:
        {
            if (!supportsDescribeColorAspects()) {
                return OMX_ErrorUnsupportedIndex;
            }

            DescribeColorAspectsParams* colorAspectsParams =
                    (DescribeColorAspectsParams *)params;

            if (!isValidOMXParam(colorAspectsParams)) {
                return OMX_ErrorBadParameter;
            }

            if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadParameter;
            }

            colorAspectsParams->sAspects = mFinalColorAspects;
            if (colorAspectsParams->bRequestingDataSpace || colorAspectsParams->bDataSpaceChanged) {
                return OMX_ErrorUnsupportedSetting;
            }

            return OMX_ErrorNone;
        }

        case kDescribeHdrStaticInfoIndex:
        {
            if (!supportDescribeHdrStaticInfo()) {
                return OMX_ErrorUnsupportedIndex;
            }

            DescribeHDRStaticInfoParams* hdrStaticInfoParams =
                    (DescribeHDRStaticInfoParams *)params;

            if (!isValidOMXParam(hdrStaticInfoParams)) {
                return OMX_ErrorBadParameter;
            }

            if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            hdrStaticInfoParams->sInfo = mHdrStaticInfo;

            return OMX_ErrorNone;
        }

        case kDescribeHdr10PlusInfoIndex:
        {
            if (!supportDescribeHdr10PlusInfo()) {
                return OMX_ErrorUnsupportedIndex;
            }

            if (mHdr10PlusOutputs.size() > 0) {
                auto it = mHdr10PlusOutputs.begin();

                auto info = (*it).get();

                DescribeHDR10PlusInfoParams* outParams =
                        (DescribeHDR10PlusInfoParams *)params;

                outParams->nParamSizeUsed = info->size();

                // If the buffer provided by the client does not have enough
                // storage, return the size only and do not remove the param yet.
                if (outParams->nParamSize >= info->size()) {
                    memcpy(outParams->nValue, info->data(), info->size());
                    mHdr10PlusOutputs.erase(it);
                }
                return OMX_ErrorNone;
            }
            return OMX_ErrorUnderflow;
        }

        default:
            return OMX_ErrorUnsupportedIndex;
    }
}

OMX_ERRORTYPE SoftVideoDecoderOMXComponent::internalSetConfig(
        OMX_INDEXTYPE index, const OMX_PTR params, bool *frameConfig){
    switch ((int)index) {
        case kDescribeColorAspectsIndex:
        {
            if (!supportsDescribeColorAspects()) {
                return OMX_ErrorUnsupportedIndex;
            }
            const DescribeColorAspectsParams* colorAspectsParams =
                    (const DescribeColorAspectsParams *)params;

            if (!isValidOMXParam(colorAspectsParams)) {
                return OMX_ErrorBadParameter;
            }

            if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadParameter;
            }

            // Update color aspects if necessary.
            if (colorAspectsDiffer(colorAspectsParams->sAspects, mDefaultColorAspects)) {
                mDefaultColorAspects = colorAspectsParams->sAspects;
                status_t err = handleColorAspectsChange();
                CHECK(err == OK);
            }
            return OMX_ErrorNone;
        }

        case kDescribeHdrStaticInfoIndex:
        {
            if (!supportDescribeHdrStaticInfo()) {
                return OMX_ErrorUnsupportedIndex;
            }

            const DescribeHDRStaticInfoParams* hdrStaticInfoParams =
                    (DescribeHDRStaticInfoParams *)params;

            if (!isValidOMXParam(hdrStaticInfoParams)) {
                return OMX_ErrorBadParameter;
            }

            if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            mHdrStaticInfo = hdrStaticInfoParams->sInfo;
            updatePortDefinitions(false);

            return OMX_ErrorNone;
        }

        case kDescribeHdr10PlusInfoIndex:
        {
            if (!supportDescribeHdr10PlusInfo()) {
                return OMX_ErrorUnsupportedIndex;
            }

            const DescribeHDR10PlusInfoParams* inParams =
                    (DescribeHDR10PlusInfoParams *)params;

            if (*frameConfig) {
                // This is a request to append to the current frame config set.
                // For now, we only support kDescribeHdr10PlusInfoIndex, which
                // we simply replace with the last set value.
                if (mHdr10PlusInputs.size() > 0) {
                    *(--mHdr10PlusInputs.end()) = ABuffer::CreateAsCopy(
                            inParams->nValue, inParams->nParamSizeUsed);
                } else {
                    ALOGW("Ignoring kDescribeHdr10PlusInfoIndex: append to "
                            "frame config while no frame config is present");
                }
            } else {
                // This is a frame config, setting *frameConfig to true so that
                // the client marks the next queued input frame to apply it.
                *frameConfig = true;
                mHdr10PlusInputs.push_back(ABuffer::CreateAsCopy(
                        inParams->nValue, inParams->nParamSizeUsed));
            }
            return OMX_ErrorNone;
        }

        default:
            return OMX_ErrorUnsupportedIndex;
    }
}

sp<ABuffer> SoftVideoDecoderOMXComponent::dequeueInputFrameConfig() {
    auto it = mHdr10PlusInputs.begin();
    sp<ABuffer> info = *it;
    mHdr10PlusInputs.erase(it);
    return info;
}

void SoftVideoDecoderOMXComponent::queueOutputFrameConfig(const sp<ABuffer> &info) {
    mHdr10PlusOutputs.push_back(info);
    notify(OMX_EventConfigUpdate,
           kOutputPortIndex,
           kDescribeHdr10PlusInfoIndex,
           NULL);
}

OMX_ERRORTYPE SoftVideoDecoderOMXComponent::getExtensionIndex(
        const char *name, OMX_INDEXTYPE *index) {
    if (!strcmp(name, "OMX.google.android.index.prepareForAdaptivePlayback")) {
        *(int32_t*)index = kPrepareForAdaptivePlaybackIndex;
        return OMX_ErrorNone;
    } else if (!strcmp(name, "OMX.google.android.index.describeColorAspects")
                && supportsDescribeColorAspects()) {
        *(int32_t*)index = kDescribeColorAspectsIndex;
        return OMX_ErrorNone;
    } else if (!strcmp(name, "OMX.google.android.index.describeHDRStaticInfo")
            && supportDescribeHdrStaticInfo()) {
        *(int32_t*)index = kDescribeHdrStaticInfoIndex;
        return OMX_ErrorNone;
    } else if (!strcmp(name, "OMX.google.android.index.describeHDR10PlusInfo")
            && supportDescribeHdr10PlusInfo()) {
        *(int32_t*)index = kDescribeHdr10PlusInfoIndex;
        return OMX_ErrorNone;
    }

    return SimpleSoftOMXComponent::getExtensionIndex(name, index);
}

bool SoftVideoDecoderOMXComponent::supportsDescribeColorAspects() {
    return getColorAspectPreference() != kNotSupported;
}

int SoftVideoDecoderOMXComponent::getColorAspectPreference() {
    return kNotSupported;
}

bool SoftVideoDecoderOMXComponent::supportDescribeHdrStaticInfo() {
    return false;
}

bool SoftVideoDecoderOMXComponent::supportDescribeHdr10PlusInfo() {
    return false;
}

void SoftVideoDecoderOMXComponent::onReset() {
    mOutputPortSettingsChange = NONE;
}

void SoftVideoDecoderOMXComponent::onPortEnableCompleted(OMX_U32 portIndex, bool enabled) {
    if (portIndex != kOutputPortIndex) {
        return;
    }

    switch (mOutputPortSettingsChange) {
        case NONE:
            break;

        case AWAITING_DISABLED:
        {
            CHECK(!enabled);
            mOutputPortSettingsChange = AWAITING_ENABLED;
            break;
        }

        default:
        {
            CHECK_EQ((int)mOutputPortSettingsChange, (int)AWAITING_ENABLED);
            CHECK(enabled);
            mOutputPortSettingsChange = NONE;
            break;
        }
    }
}

}  // namespace android
