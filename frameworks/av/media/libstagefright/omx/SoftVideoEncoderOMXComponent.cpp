/*
 * Copyright 2014 The Android Open Source Project
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
#define LOG_TAG "SoftVideoEncoderOMXComponent"
#include <utils/Log.h>
#include <utils/misc.h>

#include <media/stagefright/omx/SoftVideoEncoderOMXComponent.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/hardware/HardwareAPI.h>
#include <media/openmax/OMX_IndexExt.h>

#include <ui/Fence.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>

#include <hardware/gralloc.h>
#include <nativebase/nativebase.h>

namespace android {

const static OMX_COLOR_FORMATTYPE kSupportedColorFormats[] = {
    OMX_COLOR_FormatYUV420Planar,
    OMX_COLOR_FormatYUV420SemiPlanar,
    OMX_COLOR_FormatAndroidOpaque
};

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

SoftVideoEncoderOMXComponent::SoftVideoEncoderOMXComponent(
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
      mInputDataIsMeta(false),
      mWidth(width),
      mHeight(height),
      mBitrate(192000),
      mFramerate(30 << 16), // Q16 format
      mColorFormat(OMX_COLOR_FormatYUV420Planar),
      mMinOutputBufferSize(384), // arbitrary, using one uncompressed macroblock
      mMinCompressionRatio(1),   // max output size is normally the input size
      mComponentRole(componentRole),
      mCodingType(codingType),
      mProfileLevels(profileLevels),
      mNumProfileLevels(numProfileLevels) {
}

void SoftVideoEncoderOMXComponent::initPorts(
        OMX_U32 numInputBuffers, OMX_U32 numOutputBuffers, OMX_U32 outputBufferSize,
        const char *mime, OMX_U32 minCompressionRatio) {
    OMX_PARAM_PORTDEFINITIONTYPE def;

    mMinOutputBufferSize = outputBufferSize;
    mMinCompressionRatio = minCompressionRatio;

    InitOMXParams(&def);

    def.nPortIndex = kInputPortIndex;
    def.eDir = OMX_DirInput;
    def.nBufferCountMin = numInputBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainVideo;
    def.bBuffersContiguous = OMX_FALSE;
    def.format.video.pNativeRender = NULL;
    def.format.video.nFrameWidth = mWidth;
    def.format.video.nFrameHeight = mHeight;
    def.format.video.nStride = def.format.video.nFrameWidth;
    def.format.video.nSliceHeight = def.format.video.nFrameHeight;
    def.format.video.nBitrate = 0;
    // frameRate is in Q16 format.
    def.format.video.xFramerate = mFramerate;
    def.format.video.bFlagErrorConcealment = OMX_FALSE;
    def.nBufferAlignment = kInputBufferAlignment;
    def.format.video.cMIMEType = const_cast<char *>("video/raw");
    def.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    def.format.video.eColorFormat = mColorFormat;
    def.format.video.pNativeWindow = NULL;
    // buffersize set in updatePortParams

    addPort(def);

    InitOMXParams(&def);

    def.nPortIndex = kOutputPortIndex;
    def.eDir = OMX_DirOutput;
    def.nBufferCountMin = numOutputBuffers;
    def.nBufferCountActual = def.nBufferCountMin;
    def.bEnabled = OMX_TRUE;
    def.bPopulated = OMX_FALSE;
    def.eDomain = OMX_PortDomainVideo;
    def.bBuffersContiguous = OMX_FALSE;
    def.format.video.pNativeRender = NULL;
    def.format.video.nFrameWidth = mWidth;
    def.format.video.nFrameHeight = mHeight;
    def.format.video.nStride = 0;
    def.format.video.nSliceHeight = 0;
    def.format.video.nBitrate = mBitrate;
    def.format.video.xFramerate = 0 << 16;
    def.format.video.bFlagErrorConcealment = OMX_FALSE;
    def.nBufferAlignment = kOutputBufferAlignment;
    def.format.video.cMIMEType = const_cast<char *>(mime);
    def.format.video.eCompressionFormat = mCodingType;
    def.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    def.format.video.pNativeWindow = NULL;
    // buffersize set in updatePortParams

    addPort(def);

    updatePortParams();
}

void SoftVideoEncoderOMXComponent::updatePortParams() {
    OMX_PARAM_PORTDEFINITIONTYPE *inDef = &editPortInfo(kInputPortIndex)->mDef;
    inDef->format.video.nFrameWidth = mWidth;
    inDef->format.video.nFrameHeight = mHeight;
    inDef->format.video.nStride = inDef->format.video.nFrameWidth;
    inDef->format.video.nSliceHeight = inDef->format.video.nFrameHeight;
    inDef->format.video.xFramerate = mFramerate;
    inDef->format.video.eColorFormat = mColorFormat;
    uint32_t rawBufferSize =
        inDef->format.video.nStride * inDef->format.video.nSliceHeight * 3 / 2;
    if (inDef->format.video.eColorFormat == OMX_COLOR_FormatAndroidOpaque) {
        inDef->nBufferSize = max(sizeof(VideoNativeMetadata), sizeof(VideoGrallocMetadata));
    } else {
        inDef->nBufferSize = rawBufferSize;
    }

    OMX_PARAM_PORTDEFINITIONTYPE *outDef = &editPortInfo(kOutputPortIndex)->mDef;
    outDef->format.video.nFrameWidth = mWidth;
    outDef->format.video.nFrameHeight = mHeight;
    outDef->format.video.nBitrate = mBitrate;

    outDef->nBufferSize = max(mMinOutputBufferSize, rawBufferSize / mMinCompressionRatio);
}

OMX_ERRORTYPE SoftVideoEncoderOMXComponent::internalSetPortParams(
        const OMX_PARAM_PORTDEFINITIONTYPE *port) {

    if (!isValidOMXParam(port)) {
        return OMX_ErrorBadParameter;
    }

    if (port->nPortIndex == kInputPortIndex) {
        mWidth = port->format.video.nFrameWidth;
        mHeight = port->format.video.nFrameHeight;

        // xFramerate comes in Q16 format, in frames per second unit
        mFramerate = port->format.video.xFramerate;

        if (port->format.video.eCompressionFormat != OMX_VIDEO_CodingUnused
                || (port->format.video.eColorFormat != OMX_COLOR_FormatYUV420Planar
                        && port->format.video.eColorFormat != OMX_COLOR_FormatYUV420SemiPlanar
                        && port->format.video.eColorFormat != OMX_COLOR_FormatAndroidOpaque)) {
            return OMX_ErrorUnsupportedSetting;
        }

        mColorFormat = port->format.video.eColorFormat;
    } else if (port->nPortIndex == kOutputPortIndex) {
        if (port->format.video.eCompressionFormat != mCodingType
                || port->format.video.eColorFormat != OMX_COLOR_FormatUnused) {
            return OMX_ErrorUnsupportedSetting;
        }

        mBitrate = port->format.video.nBitrate;
    } else {
        return OMX_ErrorBadPortIndex;
    }

    updatePortParams();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftVideoEncoderOMXComponent::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR param) {
    // can include extension index OMX_INDEXEXTTYPE
    const int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoErrorCorrection:
        {
            return OMX_ErrorNotImplemented;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            const OMX_PARAM_COMPONENTROLETYPE *roleParams =
                (const OMX_PARAM_COMPONENTROLETYPE *)param;

            if (!isValidOMXParam(roleParams)) {
                return OMX_ErrorBadParameter;
            }

            if (strncmp((const char *)roleParams->cRole,
                        mComponentRole,
                        OMX_MAX_STRINGNAME_SIZE - 1)) {
                return OMX_ErrorUnsupportedSetting;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamPortDefinition:
        {
            OMX_ERRORTYPE err = internalSetPortParams((const OMX_PARAM_PORTDEFINITIONTYPE *)param);

            if (err != OMX_ErrorNone) {
                return err;
            }

            return SimpleSoftOMXComponent::internalSetParameter(index, param);
        }

        case OMX_IndexParamVideoPortFormat:
        {
            const OMX_VIDEO_PARAM_PORTFORMATTYPE* format =
                (const OMX_VIDEO_PARAM_PORTFORMATTYPE *)param;

            if (!isValidOMXParam(format)) {
                return OMX_ErrorBadParameter;
            }

            if (format->nPortIndex == kInputPortIndex) {
                if (format->eColorFormat == OMX_COLOR_FormatYUV420Planar ||
                    format->eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar ||
                    format->eColorFormat == OMX_COLOR_FormatAndroidOpaque) {
                    mColorFormat = format->eColorFormat;

                    updatePortParams();
                    return OMX_ErrorNone;
                } else {
                    ALOGE("Unsupported color format %i", format->eColorFormat);
                    return OMX_ErrorUnsupportedSetting;
                }
            } else if (format->nPortIndex == kOutputPortIndex) {
                if (format->eCompressionFormat == mCodingType) {
                    return OMX_ErrorNone;
                } else {
                    return OMX_ErrorUnsupportedSetting;
                }
            } else {
                return OMX_ErrorBadPortIndex;
            }
        }

        case kStoreMetaDataExtensionIndex:
        {
            // storeMetaDataInBuffers
            const StoreMetaDataInBuffersParams *storeParam =
                (const StoreMetaDataInBuffersParams *)param;

            if (!isValidOMXParam(storeParam)) {
                return OMX_ErrorBadParameter;
            }

            if (storeParam->nPortIndex == kOutputPortIndex) {
                return storeParam->bStoreMetaData ? OMX_ErrorUnsupportedSetting : OMX_ErrorNone;
            } else if (storeParam->nPortIndex != kInputPortIndex) {
                return OMX_ErrorBadPortIndex;
            }

            mInputDataIsMeta = (storeParam->bStoreMetaData == OMX_TRUE);
            if (mInputDataIsMeta) {
                mColorFormat = OMX_COLOR_FormatAndroidOpaque;
            } else if (mColorFormat == OMX_COLOR_FormatAndroidOpaque) {
                mColorFormat = OMX_COLOR_FormatYUV420Planar;
            }
            updatePortParams();
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalSetParameter(index, param);
    }
}

OMX_ERRORTYPE SoftVideoEncoderOMXComponent::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR param) {
    switch ((int)index) {
        case OMX_IndexParamVideoErrorCorrection:
        {
            return OMX_ErrorNotImplemented;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *formatParams =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)param;

            if (!isValidOMXParam(formatParams)) {
                return OMX_ErrorBadParameter;
            }

            if (formatParams->nPortIndex == kInputPortIndex) {
                if (formatParams->nIndex >= NELEM(kSupportedColorFormats)) {
                    return OMX_ErrorNoMore;
                }

                // Color formats, in order of preference
                formatParams->eColorFormat = kSupportedColorFormats[formatParams->nIndex];
                formatParams->eCompressionFormat = OMX_VIDEO_CodingUnused;
                formatParams->xFramerate = mFramerate;
                return OMX_ErrorNone;
            } else if (formatParams->nPortIndex == kOutputPortIndex) {
                formatParams->eCompressionFormat = mCodingType;
                formatParams->eColorFormat = OMX_COLOR_FormatUnused;
                formatParams->xFramerate = 0;
                return OMX_ErrorNone;
            } else {
                return OMX_ErrorBadPortIndex;
            }
        }

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel =
                  (OMX_VIDEO_PARAM_PROFILELEVELTYPE *) param;

            if (!isValidOMXParam(profileLevel)) {
                return OMX_ErrorBadParameter;
            }

            if (profileLevel->nPortIndex != kOutputPortIndex) {
                ALOGE("Invalid port index: %u", profileLevel->nPortIndex);
                return OMX_ErrorUnsupportedIndex;
            }

            if (profileLevel->nProfileIndex >= mNumProfileLevels) {
                return OMX_ErrorNoMore;
            }

            profileLevel->eProfile = mProfileLevels[profileLevel->nProfileIndex].mProfile;
            profileLevel->eLevel   = mProfileLevels[profileLevel->nProfileIndex].mLevel;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamConsumerUsageBits:
        {
            OMX_U32 *usageBits = (OMX_U32 *)param;
            *usageBits = GRALLOC_USAGE_SW_READ_OFTEN;
            return OMX_ErrorNone;
        }

        default:
            return SimpleSoftOMXComponent::internalGetParameter(index, param);
    }
}

// static
__attribute__((no_sanitize("integer")))
void SoftVideoEncoderOMXComponent::ConvertFlexYUVToPlanar(
        uint8_t *dst, size_t dstStride, size_t dstVStride,
        struct android_ycbcr *ycbcr, int32_t width, int32_t height) {
    const uint8_t *src = (const uint8_t *)ycbcr->y;
    const uint8_t *srcU = (const uint8_t *)ycbcr->cb;
    const uint8_t *srcV = (const uint8_t *)ycbcr->cr;
    uint8_t *dstU = dst + dstVStride * dstStride;
    uint8_t *dstV = dstU + (dstVStride >> 1) * (dstStride >> 1);

    for (size_t y = height; y > 0; --y) {
        memcpy(dst, src, width);
        dst += dstStride;
        src += ycbcr->ystride;
    }
    if (ycbcr->cstride == ycbcr->ystride >> 1 && ycbcr->chroma_step == 1) {
        // planar
        for (size_t y = height >> 1; y > 0; --y) {
            memcpy(dstU, srcU, width >> 1);
            dstU += dstStride >> 1;
            srcU += ycbcr->cstride;
            memcpy(dstV, srcV, width >> 1);
            dstV += dstStride >> 1;
            srcV += ycbcr->cstride;
        }
    } else {
        // arbitrary
        for (size_t y = height >> 1; y > 0; --y) {
            for (size_t x = width >> 1; x > 0; --x) {
                *dstU++ = *srcU;
                *dstV++ = *srcV;
                srcU += ycbcr->chroma_step;
                srcV += ycbcr->chroma_step;
            }
            dstU += (dstStride >> 1) - (width >> 1);
            dstV += (dstStride >> 1) - (width >> 1);
            srcU += ycbcr->cstride - (width >> 1) * ycbcr->chroma_step;
            srcV += ycbcr->cstride - (width >> 1) * ycbcr->chroma_step;
        }
    }
}

// static
__attribute__((no_sanitize("integer")))
void SoftVideoEncoderOMXComponent::ConvertYUV420SemiPlanarToYUV420Planar(
        const uint8_t *inYVU, uint8_t* outYUV, int32_t width, int32_t height) {
    // TODO: add support for stride
    int32_t outYsize = width * height;
    uint32_t *outY  = (uint32_t *) outYUV;
    uint16_t *outCb = (uint16_t *) (outYUV + outYsize);
    uint16_t *outCr = (uint16_t *) (outYUV + outYsize + (outYsize >> 2));

    /* Y copying */
    memcpy(outY, inYVU, outYsize);

    /* U & V copying */
    // FIXME this only works if width is multiple of 4
    uint32_t *inYVU_4 = (uint32_t *) (inYVU + outYsize);
    for (int32_t i = height >> 1; i > 0; --i) {
        for (int32_t j = width >> 2; j > 0; --j) {
            uint32_t temp = *inYVU_4++;
            uint32_t tempU = temp & 0xFF;
            tempU = tempU | ((temp >> 8) & 0xFF00);

            uint32_t tempV = (temp >> 8) & 0xFF;
            tempV = tempV | ((temp >> 16) & 0xFF00);

            *outCb++ = tempU;
            *outCr++ = tempV;
        }
    }
}

// static
__attribute__((no_sanitize("integer")))
void SoftVideoEncoderOMXComponent::ConvertRGB32ToPlanar(
        uint8_t *dstY, size_t dstStride, size_t dstVStride,
        const uint8_t *src, size_t width, size_t height, size_t srcStride,
        bool bgr) {
    CHECK((width & 1) == 0);
    CHECK((height & 1) == 0);

    uint8_t *dstU = dstY + dstStride * dstVStride;
    uint8_t *dstV = dstU + (dstStride >> 1) * (dstVStride >> 1);

#ifdef SURFACE_IS_BGR32
    bgr = !bgr;
#endif

    const size_t redOffset   = bgr ? 2 : 0;
    const size_t greenOffset = 1;
    const size_t blueOffset  = bgr ? 0 : 2;

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            unsigned red   = src[redOffset];
            unsigned green = src[greenOffset];
            unsigned blue  = src[blueOffset];

            // Using ITU-R BT.601-7 (03/2011)
            //   2.5.1: Ey'  = ( 0.299*R + 0.587*G + 0.114*B)
            //   2.5.2: ECr' = ( 0.701*R - 0.587*G - 0.114*B) / 1.402
            //          ECb' = (-0.299*R - 0.587*G + 0.886*B) / 1.772
            //   2.5.3: Y  = 219 * Ey'  +  16
            //          Cr = 224 * ECr' + 128
            //          Cb = 224 * ECb' + 128

            unsigned luma =
                ((red * 65 + green * 129 + blue * 25 + 128) >> 8) + 16;

            dstY[x] = luma;

            if ((x & 1) == 0 && (y & 1) == 0) {
                unsigned U =
                    ((-red * 38 - green * 74 + blue * 112 + 128) >> 8) + 128;

                unsigned V =
                    ((red * 112 - green * 94 - blue * 18 + 128) >> 8) + 128;

                dstU[x >> 1] = U;
                dstV[x >> 1] = V;
            }
            src += 4;
        }

        if ((y & 1) == 0) {
            dstU += dstStride >> 1;
            dstV += dstStride >> 1;
        }

        src += srcStride - 4 * width;
        dstY += dstStride;
    }
}

const uint8_t *SoftVideoEncoderOMXComponent::extractGraphicBuffer(
        uint8_t *dst, size_t dstSize,
        const uint8_t *src, size_t srcSize,
        size_t width, size_t height) const {
    size_t dstStride = width;
    size_t dstVStride = height;

    MetadataBufferType bufferType = *(MetadataBufferType *)src;
    bool usingANWBuffer = bufferType == kMetadataBufferTypeANWBuffer;
    if (!usingANWBuffer && bufferType != kMetadataBufferTypeGrallocSource) {
        ALOGE("Unsupported metadata type (%d)", bufferType);
        return NULL;
    }

    buffer_handle_t handle;
    int format;
    size_t srcStride;
    size_t srcVStride;
    if (usingANWBuffer) {
        if (srcSize < sizeof(VideoNativeMetadata)) {
            ALOGE("Metadata is too small (%zu vs %zu)", srcSize, sizeof(VideoNativeMetadata));
            return NULL;
        }

        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)src;
        ANativeWindowBuffer *buffer = nativeMeta.pBuffer;
        handle = buffer->handle;
        format = buffer->format;
        srcStride = buffer->stride;
        srcVStride = buffer->height;
        // convert stride from pixels to bytes
        if (format != HAL_PIXEL_FORMAT_YV12 &&
            format != HAL_PIXEL_FORMAT_YCrCb_420_SP &&
            format != HAL_PIXEL_FORMAT_YCbCr_420_888) {
            // TODO do we need to support other formats?
            srcStride *= 4;
        }

        if (nativeMeta.nFenceFd >= 0) {
            sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
            nativeMeta.nFenceFd = -1;
            status_t err = fence->wait(kFenceTimeoutMs);
            if (err != OK) {
                ALOGE("Timed out waiting on input fence");
                return NULL;
            }
        }
    } else {
        // TODO: remove this part.  Check if anyone uses this.

        if (srcSize < sizeof(VideoGrallocMetadata)) {
            ALOGE("Metadata is too small (%zu vs %zu)", srcSize, sizeof(VideoGrallocMetadata));
            return NULL;
        }

        VideoGrallocMetadata &grallocMeta = *(VideoGrallocMetadata *)(src);
        handle = grallocMeta.pHandle;
        // assume HAL_PIXEL_FORMAT_RGBA_8888
        // there is no way to get the src stride without the graphic buffer
        format = HAL_PIXEL_FORMAT_RGBA_8888;
        srcStride = width * 4;
        srcVStride = height;
    }

    size_t neededSize =
        dstStride * dstVStride + (width >> 1)
                + (dstStride >> 1) * ((dstVStride >> 1) + (height >> 1) - 1);
    if (dstSize < neededSize) {
        ALOGE("destination buffer is too small (%zu vs %zu)", dstSize, neededSize);
        return NULL;
    }

    auto& mapper = GraphicBufferMapper::get();

    void *bits = NULL;
    struct android_ycbcr ycbcr;
    status_t res;
    if (format == HAL_PIXEL_FORMAT_YCbCr_420_888) {
        res = mapper.lockYCbCr(
                 handle,
                 GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER,
                 Rect(width, height), &ycbcr);
    } else {
        res = mapper.lock(
                 handle,
                 GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER,
                 Rect(width, height), &bits);
    }
    if (res != OK) {
        ALOGE("Unable to lock image buffer %p for access", handle);
        return NULL;
    }

    switch (format) {
        case HAL_PIXEL_FORMAT_YV12:  // YCrCb / YVU planar
            ycbcr.y = bits;
            ycbcr.cr = (uint8_t *)bits + srcStride * srcVStride;
            ycbcr.cb = (uint8_t *)ycbcr.cr + (srcStride >> 1) * (srcVStride >> 1);
            ycbcr.chroma_step = 1;
            ycbcr.cstride = srcStride >> 1;
            ycbcr.ystride = srcStride;
            ConvertFlexYUVToPlanar(dst, dstStride, dstVStride, &ycbcr, width, height);
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:  // YCrCb / YVU semiplanar, NV21
            ycbcr.y = bits;
            ycbcr.cr = (uint8_t *)bits + srcStride * srcVStride;
            ycbcr.cb = (uint8_t *)ycbcr.cr + 1;
            ycbcr.chroma_step = 2;
            ycbcr.cstride = srcStride;
            ycbcr.ystride = srcStride;
            ConvertFlexYUVToPlanar(dst, dstStride, dstVStride, &ycbcr, width, height);
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:  // YCbCr / YUV planar
            ConvertFlexYUVToPlanar(dst, dstStride, dstVStride, &ycbcr, width, height);
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            ConvertRGB32ToPlanar(
                    dst, dstStride, dstVStride,
                    (const uint8_t *)bits, width, height, srcStride,
                    format == HAL_PIXEL_FORMAT_BGRA_8888);
            break;
        default:
            ALOGE("Unsupported pixel format %#x", format);
            dst = NULL;
            break;
    }

    if (mapper.unlock(handle) != OK) {
        ALOGE("Unable to unlock image buffer %p for access", handle);
    }

    return dst;
}

OMX_ERRORTYPE SoftVideoEncoderOMXComponent::getExtensionIndex(
        const char *name, OMX_INDEXTYPE *index) {
    if (!strcmp(name, "OMX.google.android.index.storeMetaDataInBuffers") ||
        !strcmp(name, "OMX.google.android.index.storeANWBufferInMetadata")) {
        *(int32_t*)index = kStoreMetaDataExtensionIndex;
        return OMX_ErrorNone;
    }
    return SimpleSoftOMXComponent::getExtensionIndex(name, index);
}

OMX_ERRORTYPE SoftVideoEncoderOMXComponent::validateInputBuffer(
        const OMX_BUFFERHEADERTYPE *inputBufferHeader) {
    size_t frameSize = mInputDataIsMeta ?
            max(sizeof(VideoNativeMetadata), sizeof(VideoGrallocMetadata))
            : mWidth * mHeight * 3 / 2;
    if (inputBufferHeader->nFilledLen < frameSize) {
        return OMX_ErrorUndefined;
    } else if (inputBufferHeader->nFilledLen > frameSize) {
        ALOGW("Input buffer contains more data than expected.");
    }
    return OMX_ErrorNone;
}

}  // namespace android
