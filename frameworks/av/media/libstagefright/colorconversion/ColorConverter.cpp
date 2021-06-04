/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "ColorConverter"
#include <android-base/macros.h>
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/ColorConverter.h>
#include <media/stagefright/MediaErrors.h>

#include "libyuv/convert_from.h"
#include "libyuv/convert_argb.h"
#include "libyuv/planar_functions.h"
#include "libyuv/video_common.h"
#include <functional>
#include <sys/time.h>

#define USE_LIBYUV
#define PERF_PROFILING 0


#if defined(__aarch64__) || defined(__ARM_NEON__)
#define USE_NEON_Y410 1
#else
#define USE_NEON_Y410 0
#endif

#if USE_NEON_Y410
#include <arm_neon.h>
#endif

namespace android {

static bool isRGB(OMX_COLOR_FORMATTYPE colorFormat) {
    return colorFormat == OMX_COLOR_Format16bitRGB565
            || colorFormat == OMX_COLOR_Format32BitRGBA8888
            || colorFormat == OMX_COLOR_Format32bitBGRA8888;
}

bool ColorConverter::ColorSpace::isBt709() {
    return (mStandard == ColorUtils::kColorStandardBT709);
}


bool ColorConverter::ColorSpace::isJpeg() {
    return ((mStandard == ColorUtils::kColorStandardBT601_625)
            || (mStandard == ColorUtils::kColorStandardBT601_525))
            && (mRange == ColorUtils::kColorRangeFull);
}

ColorConverter::ColorConverter(
        OMX_COLOR_FORMATTYPE from, OMX_COLOR_FORMATTYPE to)
    : mSrcFormat(from),
      mDstFormat(to),
      mSrcColorSpace({0, 0, 0}),
      mClip(NULL) {
}

ColorConverter::~ColorConverter() {
    delete[] mClip;
    mClip = NULL;
}

bool ColorConverter::isValid() const {
    switch (mSrcFormat) {
        case OMX_COLOR_FormatYUV420Planar16:
            if (mDstFormat == OMX_COLOR_FormatYUV444Y410) {
                return true;
            }
            FALLTHROUGH_INTENDED;
        case OMX_COLOR_FormatYUV420Planar:
            return mDstFormat == OMX_COLOR_Format16bitRGB565
                    || mDstFormat == OMX_COLOR_Format32BitRGBA8888
                    || mDstFormat == OMX_COLOR_Format32bitBGRA8888;

        case OMX_COLOR_FormatCbYCrY:
        case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
        case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
            return mDstFormat == OMX_COLOR_Format16bitRGB565;

        case OMX_COLOR_FormatYUV420SemiPlanar:
#ifdef USE_LIBYUV
            return mDstFormat == OMX_COLOR_Format16bitRGB565
                    || mDstFormat == OMX_COLOR_Format32BitRGBA8888;
#else
            return mDstFormat == OMX_COLOR_Format16bitRGB565;
#endif

        default:
            return false;
    }
}

bool ColorConverter::isDstRGB() const {
    return isRGB(mDstFormat);
}

void ColorConverter::setSrcColorSpace(
        uint32_t standard, uint32_t range, uint32_t transfer) {
    if (isRGB(mSrcFormat)) {
        ALOGW("Can't set color space on RGB source");
        return;
    }
    mSrcColorSpace.mStandard = standard;
    mSrcColorSpace.mRange = range;
    mSrcColorSpace.mTransfer = transfer;
}

/*
 * If stride is non-zero, client's stride will be used. For planar
 * or semi-planar YUV formats, stride must be even numbers.
 * If stride is zero, it will be calculated based on width and bpp
 * of the format, assuming no padding on the right edge.
 */
ColorConverter::BitmapParams::BitmapParams(
        void *bits,
        size_t width, size_t height, size_t stride,
        size_t cropLeft, size_t cropTop,
        size_t cropRight, size_t cropBottom,
        OMX_COLOR_FORMATTYPE colorFromat)
    : mBits(bits),
      mColorFormat(colorFromat),
      mWidth(width),
      mHeight(height),
      mCropLeft(cropLeft),
      mCropTop(cropTop),
      mCropRight(cropRight),
      mCropBottom(cropBottom) {
    switch(mColorFormat) {
    case OMX_COLOR_Format16bitRGB565:
    case OMX_COLOR_FormatYUV420Planar16:
    case OMX_COLOR_FormatCbYCrY:
        mBpp = 2;
        mStride = 2 * mWidth;
        break;

    case OMX_COLOR_Format32bitBGRA8888:
    case OMX_COLOR_Format32BitRGBA8888:
    case OMX_COLOR_FormatYUV444Y410:
        mBpp = 4;
        mStride = 4 * mWidth;
        break;

    case OMX_COLOR_FormatYUV420Planar:
    case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
    case OMX_COLOR_FormatYUV420SemiPlanar:
    case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
        mBpp = 1;
        mStride = mWidth;
        break;

    default:
        ALOGE("Unsupported color format %d", mColorFormat);
        mBpp = 1;
        mStride = mWidth;
        break;
    }
    // use client's stride if it's specified.
    if (stride != 0) {
        mStride = stride;
    }
}

size_t ColorConverter::BitmapParams::cropWidth() const {
    return mCropRight - mCropLeft + 1;
}

size_t ColorConverter::BitmapParams::cropHeight() const {
    return mCropBottom - mCropTop + 1;
}

status_t ColorConverter::convert(
        const void *srcBits,
        size_t srcWidth, size_t srcHeight, size_t srcStride,
        size_t srcCropLeft, size_t srcCropTop,
        size_t srcCropRight, size_t srcCropBottom,
        void *dstBits,
        size_t dstWidth, size_t dstHeight, size_t dstStride,
        size_t dstCropLeft, size_t dstCropTop,
        size_t dstCropRight, size_t dstCropBottom) {
    BitmapParams src(
            const_cast<void *>(srcBits),
            srcWidth, srcHeight, srcStride,
            srcCropLeft, srcCropTop, srcCropRight, srcCropBottom, mSrcFormat);

    BitmapParams dst(
            dstBits,
            dstWidth, dstHeight, dstStride,
            dstCropLeft, dstCropTop, dstCropRight, dstCropBottom, mDstFormat);

    if (!((src.mCropLeft & 1) == 0
        && src.cropWidth() == dst.cropWidth()
        && src.cropHeight() == dst.cropHeight())) {
        return ERROR_UNSUPPORTED;
    }

    status_t err;

    switch (mSrcFormat) {
        case OMX_COLOR_FormatYUV420Planar:
#ifdef USE_LIBYUV
            err = convertYUV420PlanarUseLibYUV(src, dst);
#else
            err = convertYUV420Planar(src, dst);
#endif
            break;

        case OMX_COLOR_FormatYUV420Planar16:
        {
#if PERF_PROFILING
            int64_t startTimeUs = ALooper::GetNowUs();
#endif
            err = convertYUV420Planar16(src, dst);
#if PERF_PROFILING
            int64_t endTimeUs = ALooper::GetNowUs();
            ALOGD("convertYUV420Planar16 took %lld us", (long long) (endTimeUs - startTimeUs));
#endif
            break;
        }

        case OMX_COLOR_FormatCbYCrY:
            err = convertCbYCrY(src, dst);
            break;

        case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
            err = convertQCOMYUV420SemiPlanar(src, dst);
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
#ifdef USE_LIBYUV
            err = convertYUV420SemiPlanarUseLibYUV(src, dst);
#else
            err = convertYUV420SemiPlanar(src, dst);
#endif
            break;

        case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
            err = convertTIYUV420PackedSemiPlanar(src, dst);
            break;

        default:
        {
            CHECK(!"Should not be here. Unknown color conversion.");
            break;
        }
    }

    return err;
}

status_t ColorConverter::convertCbYCrY(
        const BitmapParams &src, const BitmapParams &dst) {
    // XXX Untested

    uint8_t *kAdjustedClip = initClip();

    uint16_t *dst_ptr = (uint16_t *)dst.mBits
        + dst.mCropTop * dst.mWidth + dst.mCropLeft;

    const uint8_t *src_ptr = (const uint8_t *)src.mBits
        + (src.mCropTop * dst.mWidth + src.mCropLeft) * 2;

    for (size_t y = 0; y < src.cropHeight(); ++y) {
        for (size_t x = 0; x < src.cropWidth(); x += 2) {
            signed y1 = (signed)src_ptr[2 * x + 1] - 16;
            signed y2 = (signed)src_ptr[2 * x + 3] - 16;
            signed u = (signed)src_ptr[2 * x] - 128;
            signed v = (signed)src_ptr[2 * x + 2] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            uint32_t rgb1 =
                ((kAdjustedClip[r1] >> 3) << 11)
                | ((kAdjustedClip[g1] >> 2) << 5)
                | (kAdjustedClip[b1] >> 3);

            uint32_t rgb2 =
                ((kAdjustedClip[r2] >> 3) << 11)
                | ((kAdjustedClip[g2] >> 2) << 5)
                | (kAdjustedClip[b2] >> 3);

            if (x + 1 < src.cropWidth()) {
                *(uint32_t *)(&dst_ptr[x]) = (rgb2 << 16) | rgb1;
            } else {
                dst_ptr[x] = rgb1;
            }
        }

        src_ptr += src.mWidth * 2;
        dst_ptr += dst.mWidth;
    }

    return OK;
}

#define DECLARE_YUV2RGBFUNC(func, rgb) int (*func)(     \
        const uint8*, int, const uint8*, int,           \
        const uint8*, int, uint8*, int, int, int)       \
        = mSrcColorSpace.isBt709() ? libyuv::H420To##rgb \
        : mSrcColorSpace.isJpeg() ? libyuv::J420To##rgb  \
        : libyuv::I420To##rgb

status_t ColorConverter::convertYUV420PlanarUseLibYUV(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *dst_ptr = (uint8_t *)dst.mBits
        + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mStride + src.mCropLeft;

    const uint8_t *src_u =
        (const uint8_t *)src.mBits + src.mStride * src.mHeight
        + (src.mCropTop / 2) * (src.mStride / 2) + (src.mCropLeft / 2);

    const uint8_t *src_v =
        src_u + (src.mStride / 2) * (src.mHeight / 2);

    switch (mDstFormat) {
    case OMX_COLOR_Format16bitRGB565:
    {
        DECLARE_YUV2RGBFUNC(func, RGB565);
        (*func)(src_y, src.mStride, src_u, src.mStride / 2, src_v, src.mStride / 2,
                (uint8 *)dst_ptr, dst.mStride, src.cropWidth(), src.cropHeight());
        break;
    }

    case OMX_COLOR_Format32BitRGBA8888:
    {
        DECLARE_YUV2RGBFUNC(func, ABGR);
        (*func)(src_y, src.mStride, src_u, src.mStride / 2, src_v, src.mStride / 2,
                (uint8 *)dst_ptr, dst.mStride, src.cropWidth(), src.cropHeight());
        break;
    }

    case OMX_COLOR_Format32bitBGRA8888:
    {
        DECLARE_YUV2RGBFUNC(func, ARGB);
        (*func)(src_y, src.mStride, src_u, src.mStride / 2, src_v, src.mStride / 2,
                (uint8 *)dst_ptr, dst.mStride, src.cropWidth(), src.cropHeight());
        break;
    }

    default:
        return ERROR_UNSUPPORTED;
    }

    return OK;
}

status_t ColorConverter::convertYUV420SemiPlanarUseLibYUV(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *dst_ptr = (uint8_t *)dst.mBits
        + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mStride + src.mCropLeft;

    const uint8_t *src_u =
        (const uint8_t *)src.mBits + src.mStride * src.mHeight
        + (src.mCropTop / 2) * src.mStride + src.mCropLeft;

    switch (mDstFormat) {
    case OMX_COLOR_Format16bitRGB565:
        libyuv::NV12ToRGB565(src_y, src.mStride, src_u, src.mStride, (uint8 *)dst_ptr,
                dst.mStride, src.cropWidth(), src.cropHeight());
        break;

    case OMX_COLOR_Format32bitBGRA8888:
        libyuv::NV12ToARGB(src_y, src.mStride, src_u, src.mStride, (uint8 *)dst_ptr,
                dst.mStride, src.cropWidth(), src.cropHeight());
        break;

    case OMX_COLOR_Format32BitRGBA8888:
        libyuv::NV12ToABGR(src_y, src.mStride, src_u, src.mStride, (uint8 *)dst_ptr,
                dst.mStride, src.cropWidth(), src.cropHeight());
        break;

    default:
        return ERROR_UNSUPPORTED;
   }

   return OK;
}

std::function<void (void *, void *, void *, size_t,
                    signed *, signed *, signed *, signed *)>
getReadFromSrc(OMX_COLOR_FORMATTYPE srcFormat) {
    switch(srcFormat) {
    case OMX_COLOR_FormatYUV420Planar:
        return [](void *src_y, void *src_u, void *src_v, size_t x,
                  signed *y1, signed *y2, signed *u, signed *v) {
            *y1 = ((uint8_t*)src_y)[x] - 16;
            *y2 = ((uint8_t*)src_y)[x + 1] - 16;
            *u = ((uint8_t*)src_u)[x / 2] - 128;
            *v = ((uint8_t*)src_v)[x / 2] - 128;
        };
    case OMX_COLOR_FormatYUV420Planar16:
        return [](void *src_y, void *src_u, void *src_v, size_t x,
                signed *y1, signed *y2, signed *u, signed *v) {
            *y1 = (signed)(((uint16_t*)src_y)[x] >> 2) - 16;
            *y2 = (signed)(((uint16_t*)src_y)[x + 1] >> 2) - 16;
            *u = (signed)(((uint16_t*)src_u)[x / 2] >> 2) - 128;
            *v = (signed)(((uint16_t*)src_v)[x / 2] >> 2) - 128;
        };
    default:
        TRESPASS();
    }
    return nullptr;
}

std::function<void (void *, bool, signed, signed, signed, signed, signed, signed)>
getWriteToDst(OMX_COLOR_FORMATTYPE dstFormat, uint8_t *kAdjustedClip) {
    switch (dstFormat) {
    case OMX_COLOR_Format16bitRGB565:
    {
        return [kAdjustedClip](void *dst_ptr, bool uncropped,
                               signed r1, signed g1, signed b1,
                               signed r2, signed g2, signed b2) {
            uint32_t rgb1 =
                ((kAdjustedClip[r1] >> 3) << 11)
                | ((kAdjustedClip[g1] >> 2) << 5)
                | (kAdjustedClip[b1] >> 3);

            if (uncropped) {
                uint32_t rgb2 =
                    ((kAdjustedClip[r2] >> 3) << 11)
                    | ((kAdjustedClip[g2] >> 2) << 5)
                    | (kAdjustedClip[b2] >> 3);

                *(uint32_t *)dst_ptr = (rgb2 << 16) | rgb1;
            } else {
                *(uint16_t *)dst_ptr = rgb1;
            }
        };
    }
    case OMX_COLOR_Format32BitRGBA8888:
    {
        return [kAdjustedClip](void *dst_ptr, bool uncropped,
                               signed r1, signed g1, signed b1,
                               signed r2, signed g2, signed b2) {
            ((uint32_t *)dst_ptr)[0] =
                    (kAdjustedClip[r1])
                    | (kAdjustedClip[g1] << 8)
                    | (kAdjustedClip[b1] << 16)
                    | (0xFF << 24);

            if (uncropped) {
                ((uint32_t *)dst_ptr)[1] =
                        (kAdjustedClip[r2])
                        | (kAdjustedClip[g2] << 8)
                        | (kAdjustedClip[b2] << 16)
                        | (0xFF << 24);
            }
        };
    }
    case OMX_COLOR_Format32bitBGRA8888:
    {
        return [kAdjustedClip](void *dst_ptr, bool uncropped,
                               signed r1, signed g1, signed b1,
                               signed r2, signed g2, signed b2) {
            ((uint32_t *)dst_ptr)[0] =
                    (kAdjustedClip[b1])
                    | (kAdjustedClip[g1] << 8)
                    | (kAdjustedClip[r1] << 16)
                    | (0xFF << 24);

            if (uncropped) {
                ((uint32_t *)dst_ptr)[1] =
                        (kAdjustedClip[b2])
                        | (kAdjustedClip[g2] << 8)
                        | (kAdjustedClip[r2] << 16)
                        | (0xFF << 24);
            }
        };
    }
    default:
        TRESPASS();
    }
    return nullptr;
}

status_t ColorConverter::convertYUV420Planar(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *kAdjustedClip = initClip();

    auto readFromSrc = getReadFromSrc(mSrcFormat);
    auto writeToDst = getWriteToDst(mDstFormat, kAdjustedClip);

    uint8_t *dst_ptr = (uint8_t *)dst.mBits
            + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp;

    uint8_t *src_y = (uint8_t *)src.mBits
            + src.mCropTop * src.mStride + src.mCropLeft * src.mBpp;

    uint8_t *src_u = (uint8_t *)src.mBits + src.mStride * src.mHeight
            + (src.mCropTop / 2) * (src.mStride / 2) + src.mCropLeft / 2 * src.mBpp;

    uint8_t *src_v = src_u + (src.mStride / 2) * (src.mHeight / 2);

    for (size_t y = 0; y < src.cropHeight(); ++y) {
        for (size_t x = 0; x < src.cropWidth(); x += 2) {
            // B = 1.164 * (Y - 16) + 2.018 * (U - 128)
            // G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128)
            // R = 1.164 * (Y - 16) + 1.596 * (V - 128)

            // B = 298/256 * (Y - 16) + 517/256 * (U - 128)
            // G = .................. - 208/256 * (V - 128) - 100/256 * (U - 128)
            // R = .................. + 409/256 * (V - 128)

            // min_B = (298 * (- 16) + 517 * (- 128)) / 256 = -277
            // min_G = (298 * (- 16) - 208 * (255 - 128) - 100 * (255 - 128)) / 256 = -172
            // min_R = (298 * (- 16) + 409 * (- 128)) / 256 = -223

            // max_B = (298 * (255 - 16) + 517 * (255 - 128)) / 256 = 534
            // max_G = (298 * (255 - 16) - 208 * (- 128) - 100 * (- 128)) / 256 = 432
            // max_R = (298 * (255 - 16) + 409 * (255 - 128)) / 256 = 481

            // clip range -278 .. 535

            signed y1, y2, u, v;
            readFromSrc(src_y, src_u, src_v, x, &y1, &y2, &u, &v);

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            bool uncropped = x + 1 < src.cropWidth();
            writeToDst(dst_ptr + x * dst.mBpp, uncropped, r1, g1, b1, r2, g2, b2);
        }

        src_y += src.mStride;

        if (y & 1) {
            src_u += src.mStride / 2;
            src_v += src.mStride / 2;
        }

        dst_ptr += dst.mStride;
    }

    return OK;
}

status_t ColorConverter::convertYUV420Planar16(
        const BitmapParams &src, const BitmapParams &dst) {
    if (mDstFormat == OMX_COLOR_FormatYUV444Y410) {
        return convertYUV420Planar16ToY410(src, dst);
    }

    return convertYUV420Planar(src, dst);
}

/*
 * Pack 10-bit YUV into RGBA_1010102.
 *
 * Media sends 10-bit YUV in a RGBA_1010102 format buffer. SF will handle
 * the conversion to RGB using RenderEngine fallback.
 *
 * We do not perform a YUV->RGB conversion here, however the conversion with
 * BT2020 to Full range is below for reference:
 *
 *   B = 1.168  *(Y - 64) + 2.148  *(U - 512)
 *   G = 1.168  *(Y - 64) - 0.652  *(V - 512) - 0.188  *(U - 512)
 *   R = 1.168  *(Y - 64) + 1.683  *(V - 512)
 *
 *   B = 1196/1024  *(Y - 64) + 2200/1024  *(U - 512)
 *   G = .................... -  668/1024  *(V - 512) - 192/1024  *(U - 512)
 *   R = .................... + 1723/1024  *(V - 512)
 *
 *   min_B = (1196  *(- 64) + 2200  *(- 512)) / 1024 = -1175
 *   min_G = (1196  *(- 64) - 668  *(1023 - 512) - 192  *(1023 - 512)) / 1024 = -504
 *   min_R = (1196  *(- 64) + 1723  *(- 512)) / 1024 = -937
 *
 *   max_B = (1196  *(1023 - 64) + 2200  *(1023 - 512)) / 1024 = 2218
 *   max_G = (1196  *(1023 - 64) - 668  *(- 512) - 192  *(- 512)) / 1024 = 1551
 *   max_R = (1196  *(1023 - 64) + 1723  *(1023 - 512)) / 1024 = 1980
 *
 *   clip range -1175 .. 2218
 *
 */

#if !USE_NEON_Y410

status_t ColorConverter::convertYUV420Planar16ToY410(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *dst_ptr = (uint8_t *)dst.mBits
        + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mStride + src.mCropLeft * src.mBpp;

    const uint8_t *src_u =
        (const uint8_t *)src.mBits + src.mStride * src.mHeight
        + (src.mCropTop / 2) * (src.mStride / 2) + (src.mCropLeft / 2) * src.mBpp;

    const uint8_t *src_v =
        src_u + (src.mStride / 2) * (src.mHeight / 2);

    // Converting two lines at a time, slightly faster
    for (size_t y = 0; y < src.cropHeight(); y += 2) {
        uint32_t *dst_top = (uint32_t *) dst_ptr;
        uint32_t *dst_bot = (uint32_t *) (dst_ptr + dst.mStride);
        uint16_t *ptr_ytop = (uint16_t*) src_y;
        uint16_t *ptr_ybot = (uint16_t*) (src_y + src.mStride);
        uint16_t *ptr_u = (uint16_t*) src_u;
        uint16_t *ptr_v = (uint16_t*) src_v;

        uint32_t u01, v01, y01, y23, y45, y67, uv0, uv1;
        size_t x = 0;
        for (; x < src.cropWidth() - 3; x += 4) {
            u01 = *((uint32_t*)ptr_u); ptr_u += 2;
            v01 = *((uint32_t*)ptr_v); ptr_v += 2;

            y01 = *((uint32_t*)ptr_ytop); ptr_ytop += 2;
            y23 = *((uint32_t*)ptr_ytop); ptr_ytop += 2;
            y45 = *((uint32_t*)ptr_ybot); ptr_ybot += 2;
            y67 = *((uint32_t*)ptr_ybot); ptr_ybot += 2;

            uv0 = (u01 & 0x3FF) | ((v01 & 0x3FF) << 20);
            uv1 = (u01 >> 16) | ((v01 >> 16) << 20);

            *dst_top++ = ((y01 & 0x3FF) << 10) | uv0;
            *dst_top++ = ((y01 >> 16) << 10) | uv0;
            *dst_top++ = ((y23 & 0x3FF) << 10) | uv1;
            *dst_top++ = ((y23 >> 16) << 10) | uv1;

            *dst_bot++ = ((y45 & 0x3FF) << 10) | uv0;
            *dst_bot++ = ((y45 >> 16) << 10) | uv0;
            *dst_bot++ = ((y67 & 0x3FF) << 10) | uv1;
            *dst_bot++ = ((y67 >> 16) << 10) | uv1;
        }

        // There should be at most 2 more pixels to process. Note that we don't
        // need to consider odd case as the buffer is always aligned to even.
        if (x < src.cropWidth()) {
            u01 = *ptr_u;
            v01 = *ptr_v;
            y01 = *((uint32_t*)ptr_ytop);
            y45 = *((uint32_t*)ptr_ybot);
            uv0 = (u01 & 0x3FF) | ((v01 & 0x3FF) << 20);
            *dst_top++ = ((y01 & 0x3FF) << 10) | uv0;
            *dst_top++ = ((y01 >> 16) << 10) | uv0;
            *dst_bot++ = ((y45 & 0x3FF) << 10) | uv0;
            *dst_bot++ = ((y45 >> 16) << 10) | uv0;
        }

        src_y += src.mStride * 2;
        src_u += src.mStride / 2;
        src_v += src.mStride / 2;
        dst_ptr += dst.mStride * 2;
    }

    return OK;
}

#else

status_t ColorConverter::convertYUV420Planar16ToY410(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *out = (uint8_t *)dst.mBits
        + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mStride + src.mCropLeft * src.mBpp;

    const uint8_t *src_u =
        (const uint8_t *)src.mBits + src.mStride * src.mHeight
        + (src.mCropTop / 2) * (src.mStride / 2) + (src.mCropLeft / 2) * src.mBpp;

    const uint8_t *src_v =
        src_u + (src.mStride / 2) * (src.mHeight / 2);

    for (size_t y = 0; y < src.cropHeight(); y++) {
        uint16_t *ptr_y = (uint16_t*) src_y;
        uint16_t *ptr_u = (uint16_t*) src_u;
        uint16_t *ptr_v = (uint16_t*) src_v;
        uint32_t *ptr_out = (uint32_t *) out;

        // Process 16-pixel at a time.
        uint32_t *ptr_limit = ptr_out + (src.cropWidth() & ~15);
        while (ptr_out < ptr_limit) {
            uint16x4_t u0123 = vld1_u16(ptr_u); ptr_u += 4;
            uint16x4_t u4567 = vld1_u16(ptr_u); ptr_u += 4;
            uint16x4_t v0123 = vld1_u16(ptr_v); ptr_v += 4;
            uint16x4_t v4567 = vld1_u16(ptr_v); ptr_v += 4;
            uint16x4_t y0123 = vld1_u16(ptr_y); ptr_y += 4;
            uint16x4_t y4567 = vld1_u16(ptr_y); ptr_y += 4;
            uint16x4_t y89ab = vld1_u16(ptr_y); ptr_y += 4;
            uint16x4_t ycdef = vld1_u16(ptr_y); ptr_y += 4;

            uint32x2_t uvtempl;
            uint32x4_t uvtempq;

            uvtempq = vaddw_u16(vshll_n_u16(v0123, 20), u0123);

            uvtempl = vget_low_u32(uvtempq);
            uint32x4_t uv0011 = vreinterpretq_u32_u64(
                    vaddw_u32(vshll_n_u32(uvtempl, 32), uvtempl));

            uvtempl = vget_high_u32(uvtempq);
            uint32x4_t uv2233 = vreinterpretq_u32_u64(
                    vaddw_u32(vshll_n_u32(uvtempl, 32), uvtempl));

            uvtempq = vaddw_u16(vshll_n_u16(v4567, 20), u4567);

            uvtempl = vget_low_u32(uvtempq);
            uint32x4_t uv4455 = vreinterpretq_u32_u64(
                    vaddw_u32(vshll_n_u32(uvtempl, 32), uvtempl));

            uvtempl = vget_high_u32(uvtempq);
            uint32x4_t uv6677 = vreinterpretq_u32_u64(
                    vaddw_u32(vshll_n_u32(uvtempl, 32), uvtempl));

            uint32x4_t dsttemp;

            dsttemp = vorrq_u32(uv0011, vshll_n_u16(y0123, 10));
            vst1q_u32(ptr_out, dsttemp); ptr_out += 4;

            dsttemp = vorrq_u32(uv2233, vshll_n_u16(y4567, 10));
            vst1q_u32(ptr_out, dsttemp); ptr_out += 4;

            dsttemp = vorrq_u32(uv4455, vshll_n_u16(y89ab, 10));
            vst1q_u32(ptr_out, dsttemp); ptr_out += 4;

            dsttemp = vorrq_u32(uv6677, vshll_n_u16(ycdef, 10));
            vst1q_u32(ptr_out, dsttemp); ptr_out += 4;
        }

        src_y += src.mStride;
        if (y & 1) {
            src_u += src.mStride / 2;
            src_v += src.mStride / 2;
        }
        out += dst.mStride;
    }

    // Process the left-overs out-of-loop, 2-pixel at a time. Note that we don't
    // need to consider odd case as the buffer is always aligned to even.
    if (src.cropWidth() & 15) {
        size_t xstart = (src.cropWidth() & ~15);

        uint8_t *out = (uint8_t *)dst.mBits + dst.mCropTop * dst.mStride
                + (dst.mCropLeft + xstart) * dst.mBpp;

        const uint8_t *src_y = (const uint8_t *)src.mBits + src.mCropTop * src.mStride
                + (src.mCropLeft + xstart) * src.mBpp;

        const uint8_t *src_u = (const uint8_t *)src.mBits + src.mStride * src.mHeight
            + (src.mCropTop / 2) * (src.mStride / 2)
            + ((src.mCropLeft + xstart) / 2) * src.mBpp;

        const uint8_t *src_v = src_u + (src.mStride / 2) * (src.mHeight / 2);

        for (size_t y = 0; y < src.cropHeight(); y++) {
            uint16_t *ptr_y = (uint16_t*) src_y;
            uint16_t *ptr_u = (uint16_t*) src_u;
            uint16_t *ptr_v = (uint16_t*) src_v;
            uint32_t *ptr_out = (uint32_t *) out;
            for (size_t x = xstart; x < src.cropWidth(); x += 2) {
                uint16_t u = *ptr_u++;
                uint16_t v = *ptr_v++;
                uint32_t y01 = *((uint32_t*)ptr_y); ptr_y += 2;
                uint32_t uv = u | (((uint32_t)v) << 20);
                *ptr_out++ = ((y01 & 0x3FF) << 10) | uv;
                *ptr_out++ = ((y01 >> 16) << 10) | uv;
            }
            src_y += src.mStride;
            if (y & 1) {
                src_u += src.mStride / 2;
                src_v += src.mStride / 2;
            }
            out += dst.mStride;
        }
    }

    return OK;
}

#endif // USE_NEON_Y410

status_t ColorConverter::convertQCOMYUV420SemiPlanar(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *kAdjustedClip = initClip();

    uint16_t *dst_ptr = (uint16_t *)dst.mBits
        + dst.mCropTop * dst.mWidth + dst.mCropLeft;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mWidth + src.mCropLeft;

    const uint8_t *src_u =
        (const uint8_t *)src_y + src.mWidth * src.mHeight
        + src.mCropTop * src.mWidth + src.mCropLeft;

    for (size_t y = 0; y < src.cropHeight(); ++y) {
        for (size_t x = 0; x < src.cropWidth(); x += 2) {
            signed y1 = (signed)src_y[x] - 16;
            signed y2 = (signed)src_y[x + 1] - 16;

            signed u = (signed)src_u[x & ~1] - 128;
            signed v = (signed)src_u[(x & ~1) + 1] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            uint32_t rgb1 =
                ((kAdjustedClip[b1] >> 3) << 11)
                | ((kAdjustedClip[g1] >> 2) << 5)
                | (kAdjustedClip[r1] >> 3);

            uint32_t rgb2 =
                ((kAdjustedClip[b2] >> 3) << 11)
                | ((kAdjustedClip[g2] >> 2) << 5)
                | (kAdjustedClip[r2] >> 3);

            if (x + 1 < src.cropWidth()) {
                *(uint32_t *)(&dst_ptr[x]) = (rgb2 << 16) | rgb1;
            } else {
                dst_ptr[x] = rgb1;
            }
        }

        src_y += src.mWidth;

        if (y & 1) {
            src_u += src.mWidth;
        }

        dst_ptr += dst.mWidth;
    }

    return OK;
}

status_t ColorConverter::convertYUV420SemiPlanar(
        const BitmapParams &src, const BitmapParams &dst) {
    // XXX Untested

    uint8_t *kAdjustedClip = initClip();

    uint16_t *dst_ptr = (uint16_t *)((uint8_t *)
            dst.mBits + dst.mCropTop * dst.mStride + dst.mCropLeft * dst.mBpp);

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mStride + src.mCropLeft;

    const uint8_t *src_u =
        (const uint8_t *)src.mBits + src.mHeight * src.mStride +
        (src.mCropTop / 2) * src.mStride + src.mCropLeft;

    for (size_t y = 0; y < src.cropHeight(); ++y) {
        for (size_t x = 0; x < src.cropWidth(); x += 2) {
            signed y1 = (signed)src_y[x] - 16;
            signed y2 = (signed)src_y[x + 1] - 16;

            signed v = (signed)src_u[x & ~1] - 128;
            signed u = (signed)src_u[(x & ~1) + 1] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            uint32_t rgb1 =
                ((kAdjustedClip[b1] >> 3) << 11)
                | ((kAdjustedClip[g1] >> 2) << 5)
                | (kAdjustedClip[r1] >> 3);

            uint32_t rgb2 =
                ((kAdjustedClip[b2] >> 3) << 11)
                | ((kAdjustedClip[g2] >> 2) << 5)
                | (kAdjustedClip[r2] >> 3);

            if (x + 1 < src.cropWidth()) {
                *(uint32_t *)(&dst_ptr[x]) = (rgb2 << 16) | rgb1;
            } else {
                dst_ptr[x] = rgb1;
            }
        }

        src_y += src.mStride;

        if (y & 1) {
            src_u += src.mStride;
        }

        dst_ptr = (uint16_t*)((uint8_t*)dst_ptr + dst.mStride);
    }

    return OK;
}

status_t ColorConverter::convertTIYUV420PackedSemiPlanar(
        const BitmapParams &src, const BitmapParams &dst) {
    uint8_t *kAdjustedClip = initClip();

    uint16_t *dst_ptr = (uint16_t *)dst.mBits
        + dst.mCropTop * dst.mWidth + dst.mCropLeft;

    const uint8_t *src_y =
        (const uint8_t *)src.mBits + src.mCropTop * src.mWidth + src.mCropLeft;

    const uint8_t *src_u =
        (const uint8_t *)src_y + src.mWidth * (src.mHeight - src.mCropTop / 2);

    for (size_t y = 0; y < src.cropHeight(); ++y) {
        for (size_t x = 0; x < src.cropWidth(); x += 2) {
            signed y1 = (signed)src_y[x] - 16;
            signed y2 = (signed)src_y[x + 1] - 16;

            signed u = (signed)src_u[x & ~1] - 128;
            signed v = (signed)src_u[(x & ~1) + 1] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            uint32_t rgb1 =
                ((kAdjustedClip[r1] >> 3) << 11)
                | ((kAdjustedClip[g1] >> 2) << 5)
                | (kAdjustedClip[b1] >> 3);

            uint32_t rgb2 =
                ((kAdjustedClip[r2] >> 3) << 11)
                | ((kAdjustedClip[g2] >> 2) << 5)
                | (kAdjustedClip[b2] >> 3);

            if (x + 1 < src.cropWidth()) {
                *(uint32_t *)(&dst_ptr[x]) = (rgb2 << 16) | rgb1;
            } else {
                dst_ptr[x] = rgb1;
            }
        }

        src_y += src.mWidth;

        if (y & 1) {
            src_u += src.mWidth;
        }

        dst_ptr += dst.mWidth;
    }

    return OK;
}

uint8_t *ColorConverter::initClip() {
    static const signed kClipMin = -278;
    static const signed kClipMax = 535;

    if (mClip == NULL) {
        mClip = new uint8_t[kClipMax - kClipMin + 1];

        for (signed i = kClipMin; i <= kClipMax; ++i) {
            mClip[i - kClipMin] = (i < 0) ? 0 : (i > 255) ? 255 : (uint8_t)i;
        }
    }

    return &mClip[-kClipMin];
}

}  // namespace android
