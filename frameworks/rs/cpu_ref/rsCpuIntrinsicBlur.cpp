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

class RsdCpuScriptIntrinsicBlur : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicBlur() override;
    RsdCpuScriptIntrinsicBlur(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    // The size of the kernel radius is limited to 25 in ScriptIntrinsicBlur.java.
    // So, the max kernel size is 51 (= 2 * 25 + 1).
    // Considering SSSE3 case, which requires the size is multiple of 4,
    // at least 52 words are necessary. Values outside of the kernel should be 0.
    float mFp[104];
    uint16_t mIp[104];
    void **mScratch;
    size_t *mScratchSize;
    float mRadius;
    int mIradius;
    ObjectBaseRef<Allocation> mAlloc;

    static void kernelU4(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    static void kernelU1(const RsExpandKernelDriverInfo *info,
                         uint32_t xstart, uint32_t xend,
                         uint32_t outstep);
    void ComputeGaussianWeights();
};


void RsdCpuScriptIntrinsicBlur::ComputeGaussianWeights() {
    memset(mFp, 0, sizeof(mFp));
    memset(mIp, 0, sizeof(mIp));

    // Compute gaussian weights for the blur
    // e is the euler's number
    // TODO Define these constants only once
    float e = 2.718281828459045f;
    float pi = 3.1415926535897932f;
    // g(x) = (1 / (sqrt(2 * pi) * sigma)) * e ^ (-x^2 / (2 * sigma^2))
    // x is of the form [-radius .. 0 .. radius]
    // and sigma varies with the radius.
    // Based on some experimental radius values and sigmas,
    // we approximately fit sigma = f(radius) as
    // sigma = radius * 0.4  + 0.6
    // The larger the radius gets, the more our gaussian blur
    // will resemble a box blur since with large sigma
    // the gaussian curve begins to lose its shape
    float sigma = 0.4f * mRadius + 0.6f;

    // Now compute the coefficients. We will store some redundant values to save
    // some math during the blur calculations precompute some values
    float coeff1 = 1.0f / (sqrtf(2.0f * pi) * sigma);
    float coeff2 = - 1.0f / (2.0f * sigma * sigma);

    float normalizeFactor = 0.0f;
    float floatR = 0.0f;
    int r;
    mIradius = (float)ceil(mRadius) + 0.5f;
    for (r = -mIradius; r <= mIradius; r ++) {
        floatR = (float)r;
        mFp[r + mIradius] = coeff1 * powf(e, floatR * floatR * coeff2);
        normalizeFactor += mFp[r + mIradius];
    }

    // Now we need to normalize the weights because all our coefficients need to add up to one
    normalizeFactor = 1.0f / normalizeFactor;
    for (r = -mIradius; r <= mIradius; r ++) {
        mFp[r + mIradius] *= normalizeFactor;
        mIp[r + mIradius] = (uint16_t)(mFp[r + mIradius] * 65536.0f + 0.5f);
    }
}

void RsdCpuScriptIntrinsicBlur::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 1);
    mAlloc.set(static_cast<Allocation *>(data));
}

void RsdCpuScriptIntrinsicBlur::setGlobalVar(uint32_t slot, const void *data, size_t dataLength) {
    rsAssert(slot == 0);
    mRadius = ((const float *)data)[0];
    ComputeGaussianWeights();
}



static void OneVU4(const RsExpandKernelDriverInfo *info, float4 *out, int32_t x, int32_t y,
                   const uchar *ptrIn, int iStride, const float* gPtr, int iradius) {

    const uchar *pi = ptrIn + x*4;

    float4 blurredPixel = 0;
    for (int r = -iradius; r <= iradius; r ++) {
        int validY = rsMax((y + r), 0);
        validY = rsMin(validY, (int)(info->dim.y- 1));
        const uchar4 *pvy = (const uchar4 *)&pi[validY * iStride];
        float4 pf = convert_float4(pvy[0]);
        blurredPixel += pf * gPtr[0];
        gPtr++;
    }

    out[0] = blurredPixel;
}

static void OneVU1(const RsExpandKernelDriverInfo *info, float *out, int32_t x, int32_t y,
                   const uchar *ptrIn, int iStride, const float* gPtr, int iradius) {

    const uchar *pi = ptrIn + x;

    float blurredPixel = 0;
    for (int r = -iradius; r <= iradius; r ++) {
        int validY = rsMax((y + r), 0);
        validY = rsMin(validY, (int)(info->dim.y - 1));
        float pf = (float)pi[validY * iStride];
        blurredPixel += pf * gPtr[0];
        gPtr++;
    }

    out[0] = blurredPixel;
}

} // namespace renderscript
} // namespace android


extern "C" void rsdIntrinsicBlurU1_K(uchar *out, uchar const *in, size_t w, size_t h,
                 size_t p, size_t x, size_t y, size_t count, size_t r, uint16_t const *tab);
extern "C" void rsdIntrinsicBlurU4_K(uchar4 *out, uchar4 const *in, size_t w, size_t h,
                 size_t p, size_t x, size_t y, size_t count, size_t r, uint16_t const *tab);

#if defined(ARCH_X86_HAVE_SSSE3)
extern void rsdIntrinsicBlurVFU4_K(void *dst, const void *pin, int stride, const void *gptr, int rct, int x1, int ct);
extern void rsdIntrinsicBlurHFU4_K(void *dst, const void *pin, const void *gptr, int rct, int x1, int ct);
extern void rsdIntrinsicBlurHFU1_K(void *dst, const void *pin, const void *gptr, int rct, int x1, int ct);
#endif

using android::renderscript::gArchUseSIMD;

static void OneVFU4(float4 *out,
                    const uchar *ptrIn, int iStride, const float* gPtr, int ct,
                    int x1, int x2) {
    out += x1;
#if defined(ARCH_X86_HAVE_SSSE3)
    if (gArchUseSIMD) {
        int t = (x2 - x1);
        t &= ~1;
        if (t) {
            rsdIntrinsicBlurVFU4_K(out, ptrIn, iStride, gPtr, ct, x1, x1 + t);
        }
        x1 += t;
        out += t;
        ptrIn += t << 2;
    }
#endif
    while(x2 > x1) {
        const uchar *pi = ptrIn;
        float4 blurredPixel = 0;
        const float* gp = gPtr;

        for (int r = 0; r < ct; r++) {
            float4 pf = convert_float4(((const uchar4 *)pi)[0]);
            blurredPixel += pf * gp[0];
            pi += iStride;
            gp++;
        }
        out->xyzw = blurredPixel;
        x1++;
        out++;
        ptrIn+=4;
    }
}

static void OneVFU1(float *out,
                    const uchar *ptrIn, int iStride, const float* gPtr, int ct, int x1, int x2) {

    int len = x2 - x1;
    out += x1;

    while((x2 > x1) && (((uintptr_t)ptrIn) & 0x3)) {
        const uchar *pi = ptrIn;
        float blurredPixel = 0;
        const float* gp = gPtr;

        for (int r = 0; r < ct; r++) {
            float pf = (float)pi[0];
            blurredPixel += pf * gp[0];
            pi += iStride;
            gp++;
        }
        out[0] = blurredPixel;
        x1++;
        out++;
        ptrIn++;
        len--;
    }
#if defined(ARCH_X86_HAVE_SSSE3)
    if (gArchUseSIMD && (x2 > x1)) {
        int t = (x2 - x1) >> 2;
        t &= ~1;
        if (t) {
            rsdIntrinsicBlurVFU4_K(out, ptrIn, iStride, gPtr, ct, 0, t );
            len -= t << 2;
            ptrIn += t << 2;
            out += t << 2;
        }
    }
#endif
    while(len > 0) {
        const uchar *pi = ptrIn;
        float blurredPixel = 0;
        const float* gp = gPtr;

        for (int r = 0; r < ct; r++) {
            float pf = (float)pi[0];
            blurredPixel += pf * gp[0];
            pi += iStride;
            gp++;
        }
        out[0] = blurredPixel;
        len--;
        out++;
        ptrIn++;
    }
}

using android::renderscript::rsMin;
using android::renderscript::rsMax;

static void OneHU4(const RsExpandKernelDriverInfo *info, uchar4 *out, int32_t x,
                   const float4 *ptrIn, const float* gPtr, int iradius) {

    float4 blurredPixel = 0;
    for (int r = -iradius; r <= iradius; r ++) {
        int validX = rsMax((x + r), 0);
        validX = rsMin(validX, (int)(info->dim.x - 1));
        float4 pf = ptrIn[validX];
        blurredPixel += pf * gPtr[0];
        gPtr++;
    }

    out->xyzw = convert_uchar4(blurredPixel);
}

static void OneHU1(const RsExpandKernelDriverInfo *info, uchar *out, int32_t x,
                   const float *ptrIn, const float* gPtr, int iradius) {

    float blurredPixel = 0;
    for (int r = -iradius; r <= iradius; r ++) {
        int validX = rsMax((x + r), 0);
        validX = rsMin(validX, (int)(info->dim.x - 1));
        float pf = ptrIn[validX];
        blurredPixel += pf * gPtr[0];
        gPtr++;
    }

    out[0] = (uchar)blurredPixel;
}


namespace android {
namespace renderscript {

void RsdCpuScriptIntrinsicBlur::kernelU4(const RsExpandKernelDriverInfo *info,
                                         uint32_t xstart, uint32_t xend,
                                         uint32_t outstep) {

    float4 stackbuf[2048];
    float4 *buf = &stackbuf[0];
    RsdCpuScriptIntrinsicBlur *cp = (RsdCpuScriptIntrinsicBlur *)info->usr;
    if (!cp->mAlloc.get()) {
        ALOGE("Blur executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uchar4 *out = (uchar4 *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

#if defined(ARCH_ARM_USE_INTRINSICS)
    if (gArchUseSIMD && info->dim.x >= 4) {
      rsdIntrinsicBlurU4_K(out, (uchar4 const *)(pin + stride * info->current.y),
                 info->dim.x, info->dim.y,
                 stride, x1, info->current.y, x2 - x1, cp->mIradius, cp->mIp + cp->mIradius);
        return;
    }
#endif

    if (info->dim.x > 2048) {
        if ((info->dim.x > cp->mScratchSize[info->lid]) || !cp->mScratch[info->lid]) {
            // Pad the side of the allocation by one unit to allow alignment later
            cp->mScratch[info->lid] = realloc(cp->mScratch[info->lid], (info->dim.x + 1) * 16);
            cp->mScratchSize[info->lid] = info->dim.x;
        }
        // realloc only aligns to 8 bytes so we manually align to 16.
        buf = (float4 *) ((((intptr_t)cp->mScratch[info->lid]) + 15) & ~0xf);
    }
    float4 *fout = (float4 *)buf;
    int y = info->current.y;
    if ((y > cp->mIradius) && (y < ((int)info->dim.y - cp->mIradius))) {
        const uchar *pi = pin + (y - cp->mIradius) * stride;
        OneVFU4(fout, pi, stride, cp->mFp, cp->mIradius * 2 + 1, 0, info->dim.x);
    } else {
        x1 = 0;
        while(info->dim.x > x1) {
            OneVU4(info, fout, x1, y, pin, stride, cp->mFp, cp->mIradius);
            fout++;
            x1++;
        }
    }

    x1 = xstart;
    while ((x1 < (uint32_t)cp->mIradius) && (x1 < x2)) {
        OneHU4(info, out, x1, buf, cp->mFp, cp->mIradius);
        out++;
        x1++;
    }
#if defined(ARCH_X86_HAVE_SSSE3)
    if (gArchUseSIMD) {
        if ((x1 + cp->mIradius) < x2) {
            rsdIntrinsicBlurHFU4_K(out, buf - cp->mIradius, cp->mFp,
                                   cp->mIradius * 2 + 1, x1, x2 - cp->mIradius);
            out += (x2 - cp->mIradius) - x1;
            x1 = x2 - cp->mIradius;
        }
    }
#endif
    while(x2 > x1) {
        OneHU4(info, out, x1, buf, cp->mFp, cp->mIradius);
        out++;
        x1++;
    }
}

void RsdCpuScriptIntrinsicBlur::kernelU1(const RsExpandKernelDriverInfo *info,
                                         uint32_t xstart, uint32_t xend,
                                         uint32_t outstep) {
    float buf[4 * 2048];
    RsdCpuScriptIntrinsicBlur *cp = (RsdCpuScriptIntrinsicBlur *)info->usr;
    if (!cp->mAlloc.get()) {
        ALOGE("Blur executed without input, skipping");
        return;
    }
    const uchar *pin = (const uchar *)cp->mAlloc->mHal.drvState.lod[0].mallocPtr;
    const size_t stride = cp->mAlloc->mHal.drvState.lod[0].stride;

    uchar *out = (uchar *)info->outPtr[0];
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

#if defined(ARCH_ARM_USE_INTRINSICS)
    if (gArchUseSIMD && info->dim.x >= 16) {
        // The specialisation for r<=8 has an awkward prefill case, which is
        // fiddly to resolve, where starting close to the right edge can cause
        // a read beyond the end of input.  So avoid that case here.
        if (cp->mIradius > 8 || (info->dim.x - rsMax(0, (int32_t)x1 - 8)) >= 16) {
            rsdIntrinsicBlurU1_K(out, pin + stride * info->current.y, info->dim.x, info->dim.y,
                     stride, x1, info->current.y, x2 - x1, cp->mIradius, cp->mIp + cp->mIradius);
            return;
        }
    }
#endif

    float *fout = (float *)buf;
    int y = info->current.y;
    if ((y > cp->mIradius) && (y < ((int)info->dim.y - cp->mIradius -1))) {
        const uchar *pi = pin + (y - cp->mIradius) * stride;
        OneVFU1(fout, pi, stride, cp->mFp, cp->mIradius * 2 + 1, 0, info->dim.x);
    } else {
        x1 = 0;
        while(info->dim.x > x1) {
            OneVU1(info, fout, x1, y, pin, stride, cp->mFp, cp->mIradius);
            fout++;
            x1++;
        }
    }

    x1 = xstart;
    while ((x1 < x2) &&
           ((x1 < (uint32_t)cp->mIradius) || (((uintptr_t)out) & 0x3))) {
        OneHU1(info, out, x1, buf, cp->mFp, cp->mIradius);
        out++;
        x1++;
    }
#if defined(ARCH_X86_HAVE_SSSE3)
    if (gArchUseSIMD) {
        if ((x1 + cp->mIradius) < x2) {
            uint32_t len = x2 - (x1 + cp->mIradius);
            len &= ~3;

            // rsdIntrinsicBlurHFU1_K() processes each four float values in |buf| at once, so it
            // nees to ensure four more values can be accessed in order to avoid accessing
            // uninitialized buffer.
            if (len > 4) {
                len -= 4;
                rsdIntrinsicBlurHFU1_K(out, ((float *)buf) - cp->mIradius, cp->mFp,
                                       cp->mIradius * 2 + 1, x1, x1 + len);
                out += len;
                x1 += len;
            }
        }
    }
#endif
    while(x2 > x1) {
        OneHU1(info, out, x1, buf, cp->mFp, cp->mIradius);
        out++;
        x1++;
    }
}

RsdCpuScriptIntrinsicBlur::RsdCpuScriptIntrinsicBlur(RsdCpuReferenceImpl *ctx,
                                                     const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_BLUR) {

    mRootPtr = nullptr;
    if (e->getType() == RS_TYPE_UNSIGNED_8) {
        switch (e->getVectorSize()) {
        case 1:
            mRootPtr = &kernelU1;
            break;
        case 4:
            mRootPtr = &kernelU4;
            break;
        }
    }
    rsAssert(mRootPtr);
    mRadius = 5;

    mScratch = new void *[mCtx->getThreadCount()];
    mScratchSize = new size_t[mCtx->getThreadCount()];
    memset(mScratch, 0, sizeof(void *) * mCtx->getThreadCount());
    memset(mScratchSize, 0, sizeof(size_t) * mCtx->getThreadCount());

    ComputeGaussianWeights();
}

RsdCpuScriptIntrinsicBlur::~RsdCpuScriptIntrinsicBlur() {
    uint32_t threads = mCtx->getThreadCount();
    if (mScratch) {
        for (size_t i = 0; i < threads; i++) {
            if (mScratch[i]) {
                free(mScratch[i]);
            }
        }
        delete []mScratch;
    }
    if (mScratchSize) {
        delete []mScratchSize;
    }
}

void RsdCpuScriptIntrinsicBlur::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 2;
}

void RsdCpuScriptIntrinsicBlur::invokeFreeChildren() {
    mAlloc.clear();
}

RsdCpuScriptImpl * rsdIntrinsic_Blur(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicBlur(ctx, s, e);
}

} // namespace renderscript
} // namespace android
