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


#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#endif

#ifndef RS_COMPATIBILITY_LIB
#include "hardware/gralloc.h"
#endif

namespace android {
namespace renderscript {


class RsdCpuScriptIntrinsicYuvToRGB : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicYuvToRGB() override;
    RsdCpuScriptIntrinsicYuvToRGB(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    ObjectBaseRef<Allocation> alloc;

    static void kernel(const RsExpandKernelDriverInfo *info,
                       uint32_t xstart, uint32_t xend,
                       uint32_t outstep);
};


void RsdCpuScriptIntrinsicYuvToRGB::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 0);
    alloc.set(static_cast<Allocation *>(data));
}




static uchar4 rsYuvToRGBA_uchar4(uchar y, uchar u, uchar v) {
    int16_t Y = ((int16_t)y) - 16;
    int16_t U = ((int16_t)u) - 128;
    int16_t V = ((int16_t)v) - 128;

    short4 p;
    p.x = (Y * 298 + V * 409 + 128) >> 8;
    p.y = (Y * 298 - U * 100 - V * 208 + 128) >> 8;
    p.z = (Y * 298 + U * 516 + 128) >> 8;
    p.w = 255;
    if(p.x < 0) {
        p.x = 0;
    }
    if(p.x > 255) {
        p.x = 255;
    }
    if(p.y < 0) {
        p.y = 0;
    }
    if(p.y > 255) {
        p.y = 255;
    }
    if(p.z < 0) {
        p.z = 0;
    }
    if(p.z > 255) {
        p.z = 255;
    }

    return (uchar4){static_cast<uchar>(p.x), static_cast<uchar>(p.y),
                    static_cast<uchar>(p.z), static_cast<uchar>(p.w)};
}


extern "C" void rsdIntrinsicYuv_K(void *dst, const uchar *Y, const uchar *uv, uint32_t xstart, size_t xend);
extern "C" void rsdIntrinsicYuvR_K(void *dst, const uchar *Y, const uchar *uv, uint32_t xstart, size_t xend);
extern "C" void rsdIntrinsicYuv2_K(void *dst, const uchar *Y, const uchar *u, const uchar *v, size_t xstart, size_t xend);

void RsdCpuScriptIntrinsicYuvToRGB::kernel(const RsExpandKernelDriverInfo *info,
                                           uint32_t xstart, uint32_t xend,
                                           uint32_t outstep) {
    RsdCpuScriptIntrinsicYuvToRGB *cp = (RsdCpuScriptIntrinsicYuvToRGB *)info->usr;
    if (!cp->alloc.get()) {
        ALOGE("YuvToRGB executed without input, skipping");
        return;
    }
    const uchar *pinY = (const uchar *)cp->alloc->mHal.drvState.lod[0].mallocPtr;
    if (pinY == nullptr) {
        ALOGE("YuvToRGB executed without data, skipping");
        return;
    }

    size_t strideY = cp->alloc->mHal.drvState.lod[0].stride;

    // calculate correct stride in legacy case
    if (cp->alloc->mHal.drvState.lod[0].dimY == 0) {
        strideY = info->dim.x;
    }
    const uchar *Y = pinY + (info->current.y * strideY);

    uchar4 *out = (uchar4 *)info->outPtr[0] + xstart;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    size_t cstep = cp->alloc->mHal.drvState.yuv.step;

    const uchar *pinU = (const uchar *)cp->alloc->mHal.drvState.lod[1].mallocPtr;
    const size_t strideU = cp->alloc->mHal.drvState.lod[1].stride;
    const uchar *u = pinU + ((info->current.y >> 1) * strideU);

    const uchar *pinV = (const uchar *)cp->alloc->mHal.drvState.lod[2].mallocPtr;
    const size_t strideV = cp->alloc->mHal.drvState.lod[2].stride;
    const uchar *v = pinV + ((info->current.y >> 1) * strideV);

    //ALOGE("pinY, %p, Y, %p, info->current.y, %d, strideY, %d", pinY, Y, info->current.y, strideY);
    //ALOGE("pinU, %p, U, %p, info->current.y, %d, strideU, %d", pinU, u, info->current.y, strideU);
    //ALOGE("pinV, %p, V, %p, info->current.y, %d, strideV, %d", pinV, v, info->current.y, strideV);
    //ALOGE("dimX, %d, dimY, %d", cp->alloc->mHal.drvState.lod[0].dimX, cp->alloc->mHal.drvState.lod[0].dimY);
    //ALOGE("info->dim.x, %d, info->dim.y, %d", info->dim.x, info->dim.y);

    if (pinU == nullptr) {
        // Legacy yuv support didn't fill in uv
        v = ((uint8_t *)cp->alloc->mHal.drvState.lod[0].mallocPtr) +
            (strideY * info->dim.y) +
            ((info->current.y >> 1) * strideY);
        u = v + 1;
        cstep = 2;
    }

    /* If we start on an odd pixel then deal with it here and bump things along
     * so that subsequent code can carry on with even-odd pairing assumptions.
     */
    if((x1 & 1) && (x2 > x1)) {
        int cx = (x1 >> 1) * cstep;
        *out = rsYuvToRGBA_uchar4(Y[x1], u[cx], v[cx]);
        out++;
        x1++;
    }

#if defined(ARCH_ARM_USE_INTRINSICS)
    if((x2 > x1) && gArchUseSIMD) {
        int32_t len = x2 - x1;
        if (cstep == 1) {
            rsdIntrinsicYuv2_K(info->outPtr[0], Y, u, v, x1, x2);
            x1 += len;
            out += len;
        } else if (cstep == 2) {
            // Check for proper interleave
            intptr_t ipu = (intptr_t)u;
            intptr_t ipv = (intptr_t)v;

            if (ipu == (ipv + 1)) {
                rsdIntrinsicYuv_K(info->outPtr[0], Y, v, x1, x2);
                x1 += len;
                out += len;
            } else if (ipu == (ipv - 1)) {
                rsdIntrinsicYuvR_K(info->outPtr[0], Y, u, x1, x2);
                x1 += len;
                out += len;
            }
        }
    }
#endif

    if(x2 > x1) {
       // ALOGE("y %i  %i  %i", info->current.y, x1, x2);
        while(x1 < x2) {
            int cx = (x1 >> 1) * cstep;
            *out = rsYuvToRGBA_uchar4(Y[x1], u[cx], v[cx]);
            out++;
            x1++;
            *out = rsYuvToRGBA_uchar4(Y[x1], u[cx], v[cx]);
            out++;
            x1++;
        }
    }

}

RsdCpuScriptIntrinsicYuvToRGB::RsdCpuScriptIntrinsicYuvToRGB(
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_YUV_TO_RGB) {

    mRootPtr = &kernel;
}

RsdCpuScriptIntrinsicYuvToRGB::~RsdCpuScriptIntrinsicYuvToRGB() {
}

void RsdCpuScriptIntrinsicYuvToRGB::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 1;
}

void RsdCpuScriptIntrinsicYuvToRGB::invokeFreeChildren() {
    alloc.clear();
}

RsdCpuScriptImpl * rsdIntrinsic_YuvToRGB(RsdCpuReferenceImpl *ctx,
                                         const Script *s, const Element *e) {
    return new RsdCpuScriptIntrinsicYuvToRGB(ctx, s, e);
}

} // namespace renderscript
} // namespace android
