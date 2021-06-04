/*
 * Copyright (C) 2012 The Android Open Source Project
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


class RsdCpuScriptIntrinsicConvolve5x5 : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicConvolve5x5() override;
    RsdCpuScriptIntrinsicConvolve5x5(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    float mFp[28];
    int16_t mIp[28];
    ObjectBaseRef<Allocation> alloc;


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

void RsdCpuScriptIntrinsicConvolve5x5::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 1);
    alloc.set(static_cast<Allocation *>(data));
}

void RsdCpuScriptIntrinsicConvolve5x5::setGlobalVar(uint32_t slot,
                                                    const void *data, size_t dataLength) {
    rsAssert(slot == 0);
    memcpy (&mFp, data, dataLength);
    for(int ct=0; ct < 25; ct++) {
        if (mFp[ct] >= 0) {
            mIp[ct] = (int16_t)(mFp[ct] * 256.f + 0.5f);
        } else {
            mIp[ct] = (int16_t)(mFp[ct] * 256.f - 0.5f);
        }
    }
}


static void OneU4(const RsExpandKernelDriverInfo *info, uint32_t x, uchar4 *out,
                  const uchar4 *py0, const uchar4 *py1, const uchar4 *py2, const uchar4 *py3, const uchar4 *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float4 px = convert_float4(py0[x0]) * coeff[0] +
                convert_float4(py0[x1]) * coeff[1] +
                convert_float4(py0[x2]) * coeff[2] +
                convert_float4(py0[x3]) * coeff[3] +
                convert_float4(py0[x4]) * coeff[4] +

                convert_float4(py1[x0]) * coeff[5] +
                convert_float4(py1[x1]) * coeff[6] +
                convert_float4(py1[x2]) * coeff[7] +
                convert_float4(py1[x3]) * coeff[8] +
                convert_float4(py1[x4]) * coeff[9] +

                convert_float4(py2[x0]) * coeff[10] +
                convert_float4(py2[x1]) * coeff[11] +
                convert_float4(py2[x2]) * coeff[12] +
                convert_float4(py2[x3]) * coeff[13] +
                convert_float4(py2[x4]) * coeff[14] +

                convert_float4(py3[x0]) * coeff[15] +
                convert_float4(py3[x1]) * coeff[16] +
                convert_float4(py3[x2]) * coeff[17] +
                convert_float4(py3[x3]) * coeff[18] +
                convert_float4(py3[x4]) * coeff[19] +

                convert_float4(py4[x0]) * coeff[20] +
                convert_float4(py4[x1]) * coeff[21] +
                convert_float4(py4[x2]) * coeff[22] +
                convert_float4(py4[x3]) * coeff[23] +
                convert_float4(py4[x4]) * coeff[24];
    px = clamp(px + 0.5f, 0.f, 255.f);
    *out = convert_uchar4(px);
}

static void OneU2(const RsExpandKernelDriverInfo *info, uint32_t x, uchar2 *out,
                  const uchar2 *py0, const uchar2 *py1, const uchar2 *py2, const uchar2 *py3, const uchar2 *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float2 px = convert_float2(py0[x0]) * coeff[0] +
                convert_float2(py0[x1]) * coeff[1] +
                convert_float2(py0[x2]) * coeff[2] +
                convert_float2(py0[x3]) * coeff[3] +
                convert_float2(py0[x4]) * coeff[4] +

                convert_float2(py1[x0]) * coeff[5] +
                convert_float2(py1[x1]) * coeff[6] +
                convert_float2(py1[x2]) * coeff[7] +
                convert_float2(py1[x3]) * coeff[8] +
                convert_float2(py1[x4]) * coeff[9] +

                convert_float2(py2[x0]) * coeff[10] +
                convert_float2(py2[x1]) * coeff[11] +
                convert_float2(py2[x2]) * coeff[12] +
                convert_float2(py2[x3]) * coeff[13] +
                convert_float2(py2[x4]) * coeff[14] +

                convert_float2(py3[x0]) * coeff[15] +
                convert_float2(py3[x1]) * coeff[16] +
                convert_float2(py3[x2]) * coeff[17] +
                convert_float2(py3[x3]) * coeff[18] +
                convert_float2(py3[x4]) * coeff[19] +

                convert_float2(py4[x0]) * coeff[20] +
                convert_float2(py4[x1]) * coeff[21] +
                convert_float2(py4[x2]) * coeff[22] +
                convert_float2(py4[x3]) * coeff[23] +
                convert_float2(py4[x4]) * coeff[24];
    px = clamp(px + 0.5f, 0.f, 255.f);
    *out = convert_uchar2(px);
}

static void OneU1(const RsExpandKernelDriverInfo *info, uint32_t x, uchar *out,
                  const uchar *py0, const uchar *py1, const uchar *py2, const uchar *py3, const uchar *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float px = (float)(py0[x0]) * coeff[0] +
               (float)(py0[x1]) * coeff[1] +
               (float)(py0[x2]) * coeff[2] +
               (float)(py0[x3]) * coeff[3] +
               (float)(py0[x4]) * coeff[4] +

               (float)(py1[x0]) * coeff[5] +
               (float)(py1[x1]) * coeff[6] +
               (float)(py1[x2]) * coeff[7] +
               (float)(py1[x3]) * coeff[8] +
               (float)(py1[x4]) * coeff[9] +

               (float)(py2[x0]) * coeff[10] +
               (float)(py2[x1]) * coeff[11] +
               (float)(py2[x2]) * coeff[12] +
               (float)(py2[x3]) * coeff[13] +
               (float)(py2[x4]) * coeff[14] +

               (float)(py3[x0]) * coeff[15] +
               (float)(py3[x1]) * coeff[16] +
               (float)(py3[x2]) * coeff[17] +
               (float)(py3[x3]) * coeff[18] +
               (float)(py3[x4]) * coeff[19] +

               (float)(py4[x0]) * coeff[20] +
               (float)(py4[x1]) * coeff[21] +
               (float)(py4[x2]) * coeff[22] +
               (float)(py4[x3]) * coeff[23] +
               (float)(py4[x4]) * coeff[24];
    px = clamp(px + 0.5f, 0.f, 255.f);
    *out = px;
}

static void OneF4(const RsExpandKernelDriverInfo *info, uint32_t x, float4 *out,
                  const float4 *py0, const float4 *py1, const float4 *py2, const float4 *py3, const float4 *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float4 px = py0[x0] * coeff[0] +
                py0[x1] * coeff[1] +
                py0[x2] * coeff[2] +
                py0[x3] * coeff[3] +
                py0[x4] * coeff[4] +

                py1[x0] * coeff[5] +
                py1[x1] * coeff[6] +
                py1[x2] * coeff[7] +
                py1[x3] * coeff[8] +
                py1[x4] * coeff[9] +

                py2[x0] * coeff[10] +
                py2[x1] * coeff[11] +
                py2[x2] * coeff[12] +
                py2[x3] * coeff[13] +
                py2[x4] * coeff[14] +

                py3[x0] * coeff[15] +
                py3[x1] * coeff[16] +
                py3[x2] * coeff[17] +
                py3[x3] * coeff[18] +
                py3[x4] * coeff[19] +

                py4[x0] * coeff[20] +
                py4[x1] * coeff[21] +
                py4[x2] * coeff[22] +
                py4[x3] * coeff[23] +
                py4[x4] * coeff[24];
    *out = px;
}

static void OneF2(const RsExpandKernelDriverInfo *info, uint32_t x, float2 *out,
                  const float2 *py0, const float2 *py1, const float2 *py2, const float2 *py3, const float2 *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float2 px = py0[x0] * coeff[0] +
                py0[x1] * coeff[1] +
                py0[x2] * coeff[2] +
                py0[x3] * coeff[3] +
                py0[x4] * coeff[4] +

                py1[x0] * coeff[5] +
                py1[x1] * coeff[6] +
                py1[x2] * coeff[7] +
                py1[x3] * coeff[8] +
                py1[x4] * coeff[9] +

                py2[x0] * coeff[10] +
                py2[x1] * coeff[11] +
                py2[x2] * coeff[12] +
                py2[x3] * coeff[13] +
                py2[x4] * coeff[14] +

                py3[x0] * coeff[15] +
                py3[x1] * coeff[16] +
                py3[x2] * coeff[17] +
                py3[x3] * coeff[18] +
                py3[x4] * coeff[19] +

                py4[x0] * coeff[20] +
                py4[x1] * coeff[21] +
                py4[x2] * coeff[22] +
                py4[x3] * coeff[23] +
                py4[x4] * coeff[24];
    *out = px;
}

static void OneF1(const RsExpandKernelDriverInfo *info, uint32_t x, float *out,
                  const float *py0, const float *py1, const float *py2, const float *py3, const float *py4,
                  const float* coeff) {

    uint32_t x0 = rsMax((int32_t)x-2, 0);
    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = x;
    uint32_t x3 = rsMin((int32_t)x+1, (int32_t)(info->dim.x-1));
    uint32_t x4 = rsMin((int32_t)x+2, (int32_t)(info->dim.x-1));

    float px = py0[x0] * coeff[0] +
               py0[x1] * coeff[1] +
               py0[x2] * coeff[2] +
               py0[x3] * coeff[3] +
               py0[x4] * coeff[4] +

               py1[x0] * coeff[5] +
               py1[x1] * coeff[6] +
               py1[x2] * coeff[7] +
               py1[x3] * coeff[8] +
               py1[x4] * coeff[9] +

               py2[x0] * coeff[10] +
               py2[x1] * coeff[11] +
               py2[x2] * coeff[12] +
               py2[x3] * coeff[13] +
               py2[x4] * coeff[14] +

               py3[x0] * coeff[15] +
               py3[x1] * coeff[16] +
               py3[x2] * coeff[17] +
               py3[x3] * coeff[18] +
               py3[x4] * coeff[19] +

               py4[x0] * coeff[20] +
               py4[x1] * coeff[21] +
               py4[x2] * coeff[22] +
               py4[x3] * coeff[23] +
               py4[x4] * coeff[24];
    *out = px;
}


extern "C" void rsdIntrinsicConvolve5x5_K(void *dst, const void *y0, const void *y1,
                                          const void *y2, const void *y3, const void *y4,
                                          const int16_t *coef, uint32_t count);

void RsdCpuScriptIntrinsicConvolve5x5::kernelU4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const uchar4 *py0 = (const uchar4 *)(pin + stride * y0);
    const uchar4 *py1 = (const uchar4 *)(pin + stride * y1);
    const uchar4 *py2 = (const uchar4 *)(pin + stride * y2);
    const uchar4 *py3 = (const uchar4 *)(pin + stride * y3);
    const uchar4 *py4 = (const uchar4 *)(pin + stride * y4);

    uchar4 *out = (uchar4 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneU4(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
#if defined(ARCH_X86_HAVE_SSSE3)
    // for x86 SIMD, require minimum of 7 elements (4 for SIMD,
    // 3 for end boundary where x may hit the end boundary)
    if (gArchUseSIMD &&((x1 + 6) < x2)) {
        // subtract 3 for end boundary
        uint32_t len = (x2 - x1 - 3) >> 2;
        rsdIntrinsicConvolve5x5_K(out, py0 + x1 - 2, py1 + x1 - 2, py2 + x1 - 2, py3 + x1 - 2, py4 + x1 - 2, cp->mIp, len);
        out += len << 2;
        x1 += len << 2;
    }
#endif

#if defined(ARCH_ARM_USE_INTRINSICS)
    if(gArchUseSIMD && ((x1 + 3) < x2)) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0 + x1 - 2, py1 + x1 - 2, py2 + x1 - 2, py3 + x1 - 2, py4 + x1 - 2, cp->mIp, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneU4(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicConvolve5x5::kernelU2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const uchar2 *py0 = (const uchar2 *)(pin + stride * y0);
    const uchar2 *py1 = (const uchar2 *)(pin + stride * y1);
    const uchar2 *py2 = (const uchar2 *)(pin + stride * y2);
    const uchar2 *py3 = (const uchar2 *)(pin + stride * y3);
    const uchar2 *py4 = (const uchar2 *)(pin + stride * y4);

    uchar2 *out = (uchar2 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneU2(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }

#if 0//defined(ARCH_ARM_HAVE_NEON)
    if((x1 + 3) < x2) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0, py1, py2, py3, py4, cp->ip, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneU2(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicConvolve5x5::kernelU1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const uchar *py0 = (const uchar *)(pin + stride * y0);
    const uchar *py1 = (const uchar *)(pin + stride * y1);
    const uchar *py2 = (const uchar *)(pin + stride * y2);
    const uchar *py3 = (const uchar *)(pin + stride * y3);
    const uchar *py4 = (const uchar *)(pin + stride * y4);

    uchar *out = (uchar *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneU1(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }

#if 0//defined(ARCH_ARM_HAVE_NEON)
    if((x1 + 3) < x2) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0, py1, py2, py3, py4, cp->ip, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneU1(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicConvolve5x5::kernelF4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const float4 *py0 = (const float4 *)(pin + stride * y0);
    const float4 *py1 = (const float4 *)(pin + stride * y1);
    const float4 *py2 = (const float4 *)(pin + stride * y2);
    const float4 *py3 = (const float4 *)(pin + stride * y3);
    const float4 *py4 = (const float4 *)(pin + stride * y4);

    float4 *out = (float4 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneF4(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }

#if 0//defined(ARCH_ARM_HAVE_NEON)
    if((x1 + 3) < x2) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0, py1, py2, py3, py4, cp->ip, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneF4(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicConvolve5x5::kernelF2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const float2 *py0 = (const float2 *)(pin + stride * y0);
    const float2 *py1 = (const float2 *)(pin + stride * y1);
    const float2 *py2 = (const float2 *)(pin + stride * y2);
    const float2 *py3 = (const float2 *)(pin + stride * y3);
    const float2 *py4 = (const float2 *)(pin + stride * y4);

    float2 *out = (float2 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneF2(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }

#if 0//defined(ARCH_ARM_HAVE_NEON)
    if((x1 + 3) < x2) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0, py1, py2, py3, py4, cp->ip, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneF2(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicConvolve5x5::kernelF1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve5x5 *cp = (RsdCpuScriptIntrinsicConvolve5x5 *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("Convolve5x5 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->alloc->mHal.drvState.lod[0].stride;

    uint32_t y0 = rsMax((int32_t)info->current.y-2, 0);
    uint32_t y1 = rsMax((int32_t)info->current.y-1, 0);
    uint32_t y2 = info->current.y;
    uint32_t y3 = rsMin((int32_t)info->current.y+1, (int32_t)(info->dim.y-1));
    uint32_t y4 = rsMin((int32_t)info->current.y+2, (int32_t)(info->dim.y-1));

    const float *py0 = (const float *)(pin + stride * y0);
    const float *py1 = (const float *)(pin + stride * y1);
    const float *py2 = (const float *)(pin + stride * y2);
    const float *py3 = (const float *)(pin + stride * y3);
    const float *py4 = (const float *)(pin + stride * y4);

    float *out = (float *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    while((x1 < x2) && (x1 < 2)) {
        OneF1(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }

#if 0//defined(ARCH_ARM_HAVE_NEON)
    if((x1 + 3) < x2) {
        uint32_t len = (x2 - x1 - 3) >> 1;
        rsdIntrinsicConvolve5x5_K(out, py0, py1, py2, py3, py4, cp->ip, len);
        out += len << 1;
        x1 += len << 1;
    }
#endif

    while(x1 < x2) {
        OneF1(info, x1, out, py0, py1, py2, py3, py4, cp->mFp);
        out++;
        x1++;
    }
}

RsdCpuScriptIntrinsicConvolve5x5::RsdCpuScriptIntrinsicConvolve5x5(
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_5x5) {

    if (e->getType() == RS_TYPE_FLOAT_32) {
        switch(e->getVectorSize()) {
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
    } else {
        switch(e->getVectorSize()) {
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
    }
    for(int ct=0; ct < 25; ct++) {
        mFp[ct] = 1.f / 25.f;
        mIp[ct] = (int16_t)(mFp[ct] * 256.f);
    }
}

RsdCpuScriptIntrinsicConvolve5x5::~RsdCpuScriptIntrinsicConvolve5x5() {
}

void RsdCpuScriptIntrinsicConvolve5x5::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 2;
}

void RsdCpuScriptIntrinsicConvolve5x5::invokeFreeChildren() {
    alloc.clear();
}

RsdCpuScriptImpl * rsdIntrinsic_Convolve5x5(RsdCpuReferenceImpl *ctx,
                                            const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicConvolve5x5(ctx, s, e);
}

} // namespace renderscript
} // namespace android
