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


class RsdCpuScriptIntrinsicConvolve3x3 : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicConvolve3x3() override;
    RsdCpuScriptIntrinsicConvolve3x3(RsdCpuReferenceImpl *ctx, const Script *s, const Element *);

protected:
    float mFp[16];
    int16_t mIp[16];
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

void RsdCpuScriptIntrinsicConvolve3x3::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 1);
    mAlloc.set(static_cast<Allocation *>(data));
}

void RsdCpuScriptIntrinsicConvolve3x3::setGlobalVar(uint32_t slot, const void *data,
                                                    size_t dataLength) {
    rsAssert(slot == 0);
    memcpy (&mFp, data, dataLength);
    for(int ct=0; ct < 9; ct++) {
        if (mFp[ct] >= 0) {
            mIp[ct] = (int16_t)(mFp[ct] * 256.f + 0.5f);
        } else {
            mIp[ct] = (int16_t)(mFp[ct] * 256.f - 0.5f);
        }
    }
}

extern "C" void rsdIntrinsicConvolve3x3_K(void *dst, const void *y0, const void *y1,
                                          const void *y2, const int16_t *coef, uint32_t count);


static void ConvolveOneU4(const RsExpandKernelDriverInfo *info, uint32_t x, uchar4 *out,
                          const uchar4 *py0, const uchar4 *py1, const uchar4 *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);

    float4 px = convert_float4(py0[x1]) * coeff[0] +
                convert_float4(py0[x]) * coeff[1] +
                convert_float4(py0[x2]) * coeff[2] +
                convert_float4(py1[x1]) * coeff[3] +
                convert_float4(py1[x]) * coeff[4] +
                convert_float4(py1[x2]) * coeff[5] +
                convert_float4(py2[x1]) * coeff[6] +
                convert_float4(py2[x]) * coeff[7] +
                convert_float4(py2[x2]) * coeff[8];

    px = clamp(px + 0.5f, 0.f, 255.f);
    uchar4 o = {(uchar)px.x, (uchar)px.y, (uchar)px.z, (uchar)px.w};
    *out = o;
}

static void ConvolveOneU2(const RsExpandKernelDriverInfo *info, uint32_t x, uchar2 *out,
                          const uchar2 *py0, const uchar2 *py1, const uchar2 *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);

    float2 px = convert_float2(py0[x1]) * coeff[0] +
                convert_float2(py0[x]) * coeff[1] +
                convert_float2(py0[x2]) * coeff[2] +
                convert_float2(py1[x1]) * coeff[3] +
                convert_float2(py1[x]) * coeff[4] +
                convert_float2(py1[x2]) * coeff[5] +
                convert_float2(py2[x1]) * coeff[6] +
                convert_float2(py2[x]) * coeff[7] +
                convert_float2(py2[x2]) * coeff[8];

    px = clamp(px + 0.5f, 0.f, 255.f);
    *out = convert_uchar2(px);
}

static void ConvolveOneU1(const RsExpandKernelDriverInfo *info, uint32_t x, uchar *out,
                          const uchar *py0, const uchar *py1, const uchar *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);

    float px = ((float)py0[x1]) * coeff[0] +
               ((float)py0[x]) * coeff[1] +
               ((float)py0[x2]) * coeff[2] +
               ((float)py1[x1]) * coeff[3] +
               ((float)py1[x]) * coeff[4] +
               ((float)py1[x2]) * coeff[5] +
               ((float)py2[x1]) * coeff[6] +
               ((float)py2[x]) * coeff[7] +
               ((float)py2[x2]) * coeff[8];
    *out = clamp(px + 0.5f, 0.f, 255.f);
}

static void ConvolveOneF4(const RsExpandKernelDriverInfo *info, uint32_t x, float4 *out,
                          const float4 *py0, const float4 *py1, const float4 *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);
    *out = (py0[x1] * coeff[0]) + (py0[x] * coeff[1]) + (py0[x2] * coeff[2]) +
           (py1[x1] * coeff[3]) + (py1[x] * coeff[4]) + (py1[x2] * coeff[5]) +
           (py2[x1] * coeff[6]) + (py2[x] * coeff[7]) + (py2[x2] * coeff[8]);
}

static void ConvolveOneF2(const RsExpandKernelDriverInfo *info, uint32_t x, float2 *out,
                          const float2 *py0, const float2 *py1, const float2 *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);
    *out = (py0[x1] * coeff[0]) + (py0[x] * coeff[1]) + (py0[x2] * coeff[2]) +
           (py1[x1] * coeff[3]) + (py1[x] * coeff[4]) + (py1[x2] * coeff[5]) +
           (py2[x1] * coeff[6]) + (py2[x] * coeff[7]) + (py2[x2] * coeff[8]);
}

static void ConvolveOneF1(const RsExpandKernelDriverInfo *info, uint32_t x, float *out,
                          const float *py0, const float *py1, const float *py2,
                          const float* coeff) {

    uint32_t x1 = rsMax((int32_t)x-1, 0);
    uint32_t x2 = rsMin((int32_t)x+1, (int32_t)info->dim.x-1);
    *out = (py0[x1] * coeff[0]) + (py0[x] * coeff[1]) + (py0[x2] * coeff[2]) +
           (py1[x1] * coeff[3]) + (py1[x] * coeff[4]) + (py1[x2] * coeff[5]) +
           (py2[x1] * coeff[6]) + (py2[x] * coeff[7]) + (py2[x2] * coeff[8]);
}

void RsdCpuScriptIntrinsicConvolve3x3::kernelU4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const uchar4 *py0 = (const uchar4 *)(pin + stride * y2);
    const uchar4 *py1 = (const uchar4 *)(pin + stride * info->current.y);
    const uchar4 *py2 = (const uchar4 *)(pin + stride * y1);

    uchar4 *out = (uchar4 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneU4(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if defined(ARCH_ARM_USE_INTRINSICS) || defined(ARCH_X86_HAVE_SSSE3)
        if (gArchUseSIMD) {
            int32_t len = (x2 - x1 - 1) >> 1;
            if(len > 0) {
                rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
                x1 += len << 1;
                out += len << 1;
            }
        }
#endif

        while(x1 != x2) {
            ConvolveOneU4(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}

void RsdCpuScriptIntrinsicConvolve3x3::kernelU2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const uchar2 *py0 = (const uchar2 *)(pin + stride * y2);
    const uchar2 *py1 = (const uchar2 *)(pin + stride * info->current.y);
    const uchar2 *py2 = (const uchar2 *)(pin + stride * y1);

    uchar2 *out = (uchar2 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneU2(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if 0//defined(ARCH_ARM_HAVE_NEON)
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
            x1 += len << 1;
            out += len << 1;
        }
#endif

        while(x1 != x2) {
            ConvolveOneU2(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}

void RsdCpuScriptIntrinsicConvolve3x3::kernelU1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const uchar *py0 = (const uchar *)(pin + stride * y2);
    const uchar *py1 = (const uchar *)(pin + stride * info->current.y);
    const uchar *py2 = (const uchar *)(pin + stride * y1);

    uchar *out = (uchar *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneU1(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if 0//defined(ARCH_ARM_HAVE_NEON)
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
            x1 += len << 1;
            out += len << 1;
        }
#endif

        while(x1 != x2) {
            ConvolveOneU1(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}

void RsdCpuScriptIntrinsicConvolve3x3::kernelF4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const float4 *py0 = (const float4 *)(pin + stride * y2);
    const float4 *py1 = (const float4 *)(pin + stride * info->current.y);
    const float4 *py2 = (const float4 *)(pin + stride * y1);

    float4 *out = (float4 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneF4(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if 0//defined(ARCH_ARM_HAVE_NEON)
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
            x1 += len << 1;
            out += len << 1;
        }
#endif

        while(x1 != x2) {
            ConvolveOneF4(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}

void RsdCpuScriptIntrinsicConvolve3x3::kernelF2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const float2 *py0 = (const float2 *)(pin + stride * y2);
    const float2 *py1 = (const float2 *)(pin + stride * info->current.y);
    const float2 *py2 = (const float2 *)(pin + stride * y1);

    float2 *out = (float2 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneF2(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if 0//defined(ARCH_ARM_HAVE_NEON)
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
            x1 += len << 1;
            out += len << 1;
        }
#endif

        while(x1 != x2) {
            ConvolveOneF2(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}
void RsdCpuScriptIntrinsicConvolve3x3::kernelF1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {
    RsdCpuScriptIntrinsicConvolve3x3 *cp = (RsdCpuScriptIntrinsicConvolve3x3 *)info->usr;

    if (!cp->mAlloc.get()) {
        ALOGE("Convolve3x3 executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uint32_t y1 = rsMin((int32_t)info->current.y + 1, (int32_t)(info->dim.y-1));
    uint32_t y2 = rsMax((int32_t)info->current.y - 1, 0);
    const float *py0 = (const float *)(pin + stride * y2);
    const float *py1 = (const float *)(pin + stride * info->current.y);
    const float *py2 = (const float *)(pin + stride * y1);

    float *out = (float *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOneF1(info, 0, out, py0, py1, py2, cp->mFp);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
#if 0//defined(ARCH_ARM_HAVE_NEON)
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            rsdIntrinsicConvolve3x3_K(out, &py0[x1-1], &py1[x1-1], &py2[x1-1], cp->mIp, len);
            x1 += len << 1;
            out += len << 1;
        }
#endif

        while(x1 != x2) {
            ConvolveOneF1(info, x1, out, py0, py1, py2, cp->mFp);
            out++;
            x1++;
        }
    }
}

RsdCpuScriptIntrinsicConvolve3x3::RsdCpuScriptIntrinsicConvolve3x3(
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_3x3) {

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
    for(int ct=0; ct < 9; ct++) {
        mFp[ct] = 1.f / 9.f;
        mIp[ct] = (int16_t)(mFp[ct] * 256.f + 0.5f);
    }
}

RsdCpuScriptIntrinsicConvolve3x3::~RsdCpuScriptIntrinsicConvolve3x3() {
}

void RsdCpuScriptIntrinsicConvolve3x3::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 2;
}

void RsdCpuScriptIntrinsicConvolve3x3::invokeFreeChildren() {
    mAlloc.clear();
}

RsdCpuScriptImpl * rsdIntrinsic_Convolve3x3(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicConvolve3x3(ctx, s, e);
}

} // namespace renderscript
} // namespace android
