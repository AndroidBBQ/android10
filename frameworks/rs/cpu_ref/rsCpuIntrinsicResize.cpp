/*
 * Copyright (C) 2014 The Android Open Source Project
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


#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"

namespace android {
namespace renderscript {


class RsdCpuScriptIntrinsicResize : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicResize() override;
    RsdCpuScriptIntrinsicResize(RsdCpuReferenceImpl *ctx, const Script *s, const Element *);

    void preLaunch(uint32_t slot, const Allocation ** ains,
                   uint32_t inLen, Allocation * aout, const void * usr,
                   uint32_t usrLen, const RsScriptCall *sc) override;

    float scaleX;
    float scaleY;

protected:
    ObjectBaseRef<const Allocation> mAlloc;
    ObjectBaseRef<const Element> mElement;

    static void kernelU1(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelU2(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelU4(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelF1(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelF2(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelF4(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
};

void RsdCpuScriptIntrinsicResize::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 0);
    mAlloc.set(static_cast<Allocation *>(data));
}

static float4 cubicInterpolate(float4 p0,float4 p1,float4 p2,float4 p3, float x) {
    return p1 + 0.5f * x * (p2 - p0 + x * (2.f * p0 - 5.f * p1 + 4.f * p2 - p3
            + x * (3.f * (p1 - p2) + p3 - p0)));
}

static float2 cubicInterpolate(float2 p0,float2 p1,float2 p2,float2 p3, float x) {
    return p1 + 0.5f * x * (p2 - p0 + x * (2.f * p0 - 5.f * p1 + 4.f * p2 - p3
            + x * (3.f * (p1 - p2) + p3 - p0)));
}

static float cubicInterpolate(float p0,float p1,float p2,float p3 , float x) {
    return p1 + 0.5f * x * (p2 - p0 + x * (2.f * p0 - 5.f * p1 + 4.f * p2 - p3
            + x * (3.f * (p1 - p2) + p3 - p0)));
}

static uchar4 OneBiCubic(const uchar4 *yp0, const uchar4 *yp1, const uchar4 *yp2, const uchar4 *yp3,
                         float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float4 p0  = cubicInterpolate(convert_float4(yp0[xs0]),
                                  convert_float4(yp0[xs1]),
                                  convert_float4(yp0[xs2]),
                                  convert_float4(yp0[xs3]), xf);

    float4 p1  = cubicInterpolate(convert_float4(yp1[xs0]),
                                  convert_float4(yp1[xs1]),
                                  convert_float4(yp1[xs2]),
                                  convert_float4(yp1[xs3]), xf);

    float4 p2  = cubicInterpolate(convert_float4(yp2[xs0]),
                                  convert_float4(yp2[xs1]),
                                  convert_float4(yp2[xs2]),
                                  convert_float4(yp2[xs3]), xf);

    float4 p3  = cubicInterpolate(convert_float4(yp3[xs0]),
                                  convert_float4(yp3[xs1]),
                                  convert_float4(yp3[xs2]),
                                  convert_float4(yp3[xs3]), xf);

    float4 p  = cubicInterpolate(p0, p1, p2, p3, yf);
    p = clamp(p + 0.5f, 0.f, 255.f);
    return convert_uchar4(p);
}

static uchar2 OneBiCubic(const uchar2 *yp0, const uchar2 *yp1, const uchar2 *yp2, const uchar2 *yp3,
                         float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float2 p0  = cubicInterpolate(convert_float2(yp0[xs0]),
                                  convert_float2(yp0[xs1]),
                                  convert_float2(yp0[xs2]),
                                  convert_float2(yp0[xs3]), xf);

    float2 p1  = cubicInterpolate(convert_float2(yp1[xs0]),
                                  convert_float2(yp1[xs1]),
                                  convert_float2(yp1[xs2]),
                                  convert_float2(yp1[xs3]), xf);

    float2 p2  = cubicInterpolate(convert_float2(yp2[xs0]),
                                  convert_float2(yp2[xs1]),
                                  convert_float2(yp2[xs2]),
                                  convert_float2(yp2[xs3]), xf);

    float2 p3  = cubicInterpolate(convert_float2(yp3[xs0]),
                                  convert_float2(yp3[xs1]),
                                  convert_float2(yp3[xs2]),
                                  convert_float2(yp3[xs3]), xf);

    float2 p  = cubicInterpolate(p0, p1, p2, p3, yf);
    p = clamp(p + 0.5f, 0.f, 255.f);
    return convert_uchar2(p);
}

static uchar OneBiCubic(const uchar *yp0, const uchar *yp1, const uchar *yp2, const uchar *yp3,
                        float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float p0  = cubicInterpolate((float)yp0[xs0], (float)yp0[xs1],
                                 (float)yp0[xs2], (float)yp0[xs3], xf);
    float p1  = cubicInterpolate((float)yp1[xs0], (float)yp1[xs1],
                                 (float)yp1[xs2], (float)yp1[xs3], xf);
    float p2  = cubicInterpolate((float)yp2[xs0], (float)yp2[xs1],
                                 (float)yp2[xs2], (float)yp2[xs3], xf);
    float p3  = cubicInterpolate((float)yp3[xs0], (float)yp3[xs1],
                                 (float)yp3[xs2], (float)yp3[xs3], xf);

    float p  = cubicInterpolate(p0, p1, p2, p3, yf);
    p = clamp(p + 0.5f, 0.f, 255.f);
    return (uchar)p;
}

extern "C" uint64_t rsdIntrinsicResize_oscctl_K(uint32_t xinc);

extern "C" void rsdIntrinsicResizeB4_K(
            uchar4 *dst,
            size_t count,
            uint32_t xf,
            uint32_t xinc,
            uchar4 const *srcn,
            uchar4 const *src0,
            uchar4 const *src1,
            uchar4 const *src2,
            size_t xclip,
            size_t avail,
            uint64_t osc_ctl,
            int32_t const *yr);

extern "C" void rsdIntrinsicResizeB2_K(
            uchar2 *dst,
            size_t count,
            uint32_t xf,
            uint32_t xinc,
            uchar2 const *srcn,
            uchar2 const *src0,
            uchar2 const *src1,
            uchar2 const *src2,
            size_t xclip,
            size_t avail,
            uint64_t osc_ctl,
            int32_t const *yr);

extern "C" void rsdIntrinsicResizeB1_K(
            uchar *dst,
            size_t count,
            uint32_t xf,
            uint32_t xinc,
            uchar const *srcn,
            uchar const *src0,
            uchar const *src1,
            uchar const *src2,
            size_t xclip,
            size_t avail,
            uint64_t osc_ctl,
            int32_t const *yr);

#if defined(ARCH_ARM_USE_INTRINSICS)
static void mkYCoeff(int32_t *yr, float yf) {
    int32_t yf1 = rint(yf * 0x10000);
    int32_t yf2 = rint(yf * yf * 0x10000);
    int32_t yf3 = rint(yf * yf * yf * 0x10000);

    yr[0] = -(2 * yf2 - yf3 - yf1) >> 1;
    yr[1] = (3 * yf3 - 5 * yf2 + 0x20000) >> 1;
    yr[2] = (-3 * yf3 + 4 * yf2 + yf1) >> 1;
    yr[3] = -(yf3 - yf2) >> 1;
}
#endif

static float4 OneBiCubic(const float4 *yp0, const float4 *yp1, const float4 *yp2, const float4 *yp3,
                         float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float4 p0  = cubicInterpolate(yp0[xs0], yp0[xs1],
                                  yp0[xs2], yp0[xs3], xf);
    float4 p1  = cubicInterpolate(yp1[xs0], yp1[xs1],
                                  yp1[xs2], yp1[xs3], xf);
    float4 p2  = cubicInterpolate(yp2[xs0], yp2[xs1],
                                  yp2[xs2], yp2[xs3], xf);
    float4 p3  = cubicInterpolate(yp3[xs0], yp3[xs1],
                                  yp3[xs2], yp3[xs3], xf);

    float4 p  = cubicInterpolate(p0, p1, p2, p3, yf);
    return p;
}

static float2 OneBiCubic(const float2 *yp0, const float2 *yp1, const float2 *yp2, const float2 *yp3,
                         float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float2 p0  = cubicInterpolate(yp0[xs0], yp0[xs1],
                                  yp0[xs2], yp0[xs3], xf);
    float2 p1  = cubicInterpolate(yp1[xs0], yp1[xs1],
                                  yp1[xs2], yp1[xs3], xf);
    float2 p2  = cubicInterpolate(yp2[xs0], yp2[xs1],
                                  yp2[xs2], yp2[xs3], xf);
    float2 p3  = cubicInterpolate(yp3[xs0], yp3[xs1],
                                  yp3[xs2], yp3[xs3], xf);

    float2 p  = cubicInterpolate(p0, p1, p2, p3, yf);
    return p;
}

static float OneBiCubic(const float *yp0, const float *yp1, const float *yp2, const float *yp3,
                        float xf, float yf, int width) {
    int startx = (int) floor(xf - 1);
    xf = xf - floor(xf);
    int maxx = width - 1;
    int xs0 = rsMax(0, startx + 0);
    int xs1 = rsMax(0, startx + 1);
    int xs2 = rsMin(maxx, startx + 2);
    int xs3 = rsMin(maxx, startx + 3);

    float p0  = cubicInterpolate(yp0[xs0], yp0[xs1],
                                 yp0[xs2], yp0[xs3], xf);
    float p1  = cubicInterpolate(yp1[xs0], yp1[xs1],
                                 yp1[xs2], yp1[xs3], xf);
    float p2  = cubicInterpolate(yp2[xs0], yp2[xs1],
                                 yp2[xs2], yp2[xs3], xf);
    float p3  = cubicInterpolate(yp3[xs0], yp3[xs1],
                                 yp3[xs2], yp3[xs3], xf);

    float p  = cubicInterpolate(p0, p1, p2, p3, yf);
    return p;
}

void RsdCpuScriptIntrinsicResize::kernelU4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const uchar4 *yp0 = (const uchar4 *)(pin + stride * ys0);
    const uchar4 *yp1 = (const uchar4 *)(pin + stride * ys1);
    const uchar4 *yp2 = (const uchar4 *)(pin + stride * ys2);
    const uchar4 *yp3 = (const uchar4 *)(pin + stride * ys3);

    uchar4 *out = ((uchar4 *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

#if defined(ARCH_ARM_USE_INTRINSICS)
    if (gArchUseSIMD && x2 > x1 && cp->scaleX < 4.0f) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        long xf16 = rint(xf * 0x10000);
        uint32_t xinc16 = rint(cp->scaleX * 0x10000);

        int xoff = (xf16 >> 16) - 1;
        int xclip = rsMax(0, xoff) - xoff;
        int len = x2 - x1;

        int32_t yr[4];
        uint64_t osc_ctl = rsdIntrinsicResize_oscctl_K(xinc16);
        mkYCoeff(yr, yf);

        xoff += xclip;

        rsdIntrinsicResizeB4_K(
                out, len,
                xf16 & 0xffff, xinc16,
                yp0 + xoff, yp1 + xoff, yp2 + xoff, yp3 + xoff,
                xclip, srcWidth - xoff + xclip,
                osc_ctl, yr);
        out += len;
        x1 += len;
    }
#endif

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicResize::kernelU2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const uchar2 *yp0 = (const uchar2 *)(pin + stride * ys0);
    const uchar2 *yp1 = (const uchar2 *)(pin + stride * ys1);
    const uchar2 *yp2 = (const uchar2 *)(pin + stride * ys2);
    const uchar2 *yp3 = (const uchar2 *)(pin + stride * ys3);

    uchar2 *out = ((uchar2 *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

#if defined(ARCH_ARM_USE_INTRINSICS)
    if (gArchUseSIMD && x2 > x1 && cp->scaleX < 4.0f) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        long xf16 = rint(xf * 0x10000);
        uint32_t xinc16 = rint(cp->scaleX * 0x10000);

        int xoff = (xf16 >> 16) - 1;
        int xclip = rsMax(0, xoff) - xoff;
        int len = x2 - x1;

        int32_t yr[4];
        uint64_t osc_ctl = rsdIntrinsicResize_oscctl_K(xinc16);
        mkYCoeff(yr, yf);

        xoff += xclip;

        rsdIntrinsicResizeB2_K(
                out, len,
                xf16 & 0xffff, xinc16,
                yp0 + xoff, yp1 + xoff, yp2 + xoff, yp3 + xoff,
                xclip, srcWidth - xoff + xclip,
                osc_ctl, yr);
        out += len;
        x1 += len;
    }
#endif

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicResize::kernelU1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const uchar *yp0 = pin + stride * ys0;
    const uchar *yp1 = pin + stride * ys1;
    const uchar *yp2 = pin + stride * ys2;
    const uchar *yp3 = pin + stride * ys3;

    uchar *out = ((uchar *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

#if defined(ARCH_ARM_USE_INTRINSICS)
    if (gArchUseSIMD && x2 > x1 && cp->scaleX < 4.0f) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        long xf16 = rint(xf * 0x10000);
        uint32_t xinc16 = rint(cp->scaleX * 0x10000);

        int xoff = (xf16 >> 16) - 1;
        int xclip = rsMax(0, xoff) - xoff;
        int len = x2 - x1;

        int32_t yr[4];
        uint64_t osc_ctl = rsdIntrinsicResize_oscctl_K(xinc16);
        mkYCoeff(yr, yf);

        xoff += xclip;

        rsdIntrinsicResizeB1_K(
                out, len,
                xf16 & 0xffff, xinc16,
                yp0 + xoff, yp1 + xoff, yp2 + xoff, yp3 + xoff,
                xclip, srcWidth - xoff + xclip,
                osc_ctl, yr);
        out += len;
        x1 += len;
    }
#endif

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicResize::kernelF4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const float4 *yp0 = (const float4 *)(pin + stride * ys0);
    const float4 *yp1 = (const float4 *)(pin + stride * ys1);
    const float4 *yp2 = (const float4 *)(pin + stride * ys2);
    const float4 *yp3 = (const float4 *)(pin + stride * ys3);

    float4 *out = ((float4 *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicResize::kernelF2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const float2 *yp0 = (const float2 *)(pin + stride * ys0);
    const float2 *yp1 = (const float2 *)(pin + stride * ys1);
    const float2 *yp2 = (const float2 *)(pin + stride * ys2);
    const float2 *yp3 = (const float2 *)(pin + stride * ys3);

    float2 *out = ((float2 *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicResize::kernelF1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicResize *cp = (RsdCpuScriptIntrinsicResize *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const int srcHeight = cp->mAlloc->mHal.drvState.lod[0].dimY;
    const int srcWidth = cp->mAlloc->mHal.drvState.lod[0].dimX;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    float yf = (info->current.y + 0.5f) * cp->scaleY - 0.5f;
    int starty = (int) floor(yf - 1);
    yf = yf - floor(yf);
    int maxy = srcHeight - 1;
    int ys0 = rsMax(0, starty + 0);
    int ys1 = rsMax(0, starty + 1);
    int ys2 = rsMin(maxy, starty + 2);
    int ys3 = rsMin(maxy, starty + 3);

    const float *yp0 = (const float *)(pin + stride * ys0);
    const float *yp1 = (const float *)(pin + stride * ys1);
    const float *yp2 = (const float *)(pin + stride * ys2);
    const float *yp3 = (const float *)(pin + stride * ys3);

    float *out = ((float *)info->outPtr[0]) + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while(x1 < x2) {
        float xf = (x1 + 0.5f) * cp->scaleX - 0.5f;
        *out = OneBiCubic(yp0, yp1, yp2, yp3, xf, yf, srcWidth);
        out++;
        x1++;
    }
}

RsdCpuScriptIntrinsicResize::RsdCpuScriptIntrinsicResize (
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_RESIZE) {

}

RsdCpuScriptIntrinsicResize::~RsdCpuScriptIntrinsicResize() {
}

void RsdCpuScriptIntrinsicResize::preLaunch(uint32_t slot,
                                            const Allocation ** ains,
                                            uint32_t inLen, Allocation * aout,
                                            const void * usr, uint32_t usrLen,
                                            const RsScriptCall *sc)
{
    if (!mAlloc.get()) {
        ALOGE("Resize executed without input, skipping");
        return;
    }
    const uint32_t srcHeight = mAlloc->mHal.drvState.lod[0].dimY;
    const uint32_t srcWidth = mAlloc->mHal.drvState.lod[0].dimX;

    //check the data type to determine F or U.
    if (mAlloc->getType()->getElement()->getType() == RS_TYPE_UNSIGNED_8) {
        switch(mAlloc->getType()->getElement()->getVectorSize()) {
        case 1:
            mRootPtr = &kernelU1;
            break;
        case 2:
            mRootPtr = &kernelU2;
            break;
        case 3:
        case 4:
            mRootPtr = &kernelU4;
            break;
        }
    } else {
        switch(mAlloc->getType()->getElement()->getVectorSize()) {
        case 1:
            mRootPtr = &kernelF1;
            break;
        case 2:
            mRootPtr = &kernelF2;
            break;
        case 3:
        case 4:
            mRootPtr = &kernelF4;
            break;
        }
    }

    scaleX = (float)srcWidth / aout->mHal.drvState.lod[0].dimX;
    scaleY = (float)srcHeight / aout->mHal.drvState.lod[0].dimY;

}

void RsdCpuScriptIntrinsicResize::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 1;
}

void RsdCpuScriptIntrinsicResize::invokeFreeChildren() {
    mAlloc.clear();
}

RsdCpuScriptImpl * rsdIntrinsic_Resize(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicResize(ctx, s, e);
}

} // namespace renderscript
} // namespace android
