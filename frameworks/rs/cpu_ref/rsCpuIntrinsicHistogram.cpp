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

namespace android {
namespace renderscript {


class RsdCpuScriptIntrinsicHistogram : public RsdCpuScriptIntrinsic {
public:
    void populateScript(Script *) override;
    void invokeFreeChildren() override;

    void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsicHistogram() override;
    RsdCpuScriptIntrinsicHistogram(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    void preLaunch(uint32_t slot, const Allocation ** ains, uint32_t inLen,
                   Allocation * aout, const void * usr,
                   uint32_t usrLen, const RsScriptCall *sc);
    void postLaunch(uint32_t slot, const Allocation ** ains, uint32_t inLen,
                    Allocation * aout, const void * usr,
                    uint32_t usrLen, const RsScriptCall *sc);


    float mDot[4];
    int mDotI[4];
    int *mSums;
    ObjectBaseRef<Allocation> mAllocOut;

    static void kernelP1U4(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1U3(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1U2(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1U1(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);

    static void kernelP1L4(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1L3(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1L2(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);
    static void kernelP1L1(const RsExpandKernelDriverInfo *info,
                           uint32_t xstart, uint32_t xend,
                           uint32_t outstep);

};

void RsdCpuScriptIntrinsicHistogram::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot == 1);
    mAllocOut.set(static_cast<Allocation *>(data));
}

void RsdCpuScriptIntrinsicHistogram::setGlobalVar(uint32_t slot, const void *data, size_t dataLength) {
    rsAssert(slot == 0);
    rsAssert(dataLength == 16);
    memcpy(mDot, data, 16);
    mDotI[0] = (int)((mDot[0] * 256.f) + 0.5f);
    mDotI[1] = (int)((mDot[1] * 256.f) + 0.5f);
    mDotI[2] = (int)((mDot[2] * 256.f) + 0.5f);
    mDotI[3] = (int)((mDot[3] * 256.f) + 0.5f);
}



void
RsdCpuScriptIntrinsicHistogram::preLaunch(uint32_t slot,
                                          const Allocation ** ains,
                                          uint32_t inLen, Allocation * aout,
                                          const void * usr, uint32_t usrLen,
                                          const RsScriptCall *sc) {

    const uint32_t threads = mCtx->getThreadCount();
    uint32_t vSize = mAllocOut->getType()->getElement()->getVectorSize();

    switch (slot) {
    case 0:
        switch(vSize) {
        case 1:
            mRootPtr = &kernelP1U1;
            break;
        case 2:
            mRootPtr = &kernelP1U2;
            break;
        case 3:
            mRootPtr = &kernelP1U3;
            vSize = 4;
            break;
        case 4:
            mRootPtr = &kernelP1U4;
            break;
        }
        break;
    case 1:
        switch(ains[0]->getType()->getElement()->getVectorSize()) {
        case 1:
            mRootPtr = &kernelP1L1;
            break;
        case 2:
            mRootPtr = &kernelP1L2;
            break;
        case 3:
            mRootPtr = &kernelP1L3;
            break;
        case 4:
            mRootPtr = &kernelP1L4;
            break;
        }
        break;
    }
    memset(mSums, 0, 256 * sizeof(int32_t) * threads * vSize);
}

void
RsdCpuScriptIntrinsicHistogram::postLaunch(uint32_t slot,
                                           const Allocation ** ains,
                                           uint32_t inLen,  Allocation * aout,
                                           const void * usr, uint32_t usrLen,
                                           const RsScriptCall *sc) {

    unsigned int *o = (unsigned int *)mAllocOut->mHal.drvState.lod[0].mallocPtr;
    uint32_t threads = mCtx->getThreadCount();
    uint32_t vSize = mAllocOut->getType()->getElement()->getVectorSize();

    if (vSize == 3) vSize = 4;

    for (uint32_t ct=0; ct < (256 * vSize); ct++) {
        o[ct] = mSums[ct];
        for (uint32_t t=1; t < threads; t++) {
            o[ct] += mSums[ct + (256 * vSize * t)];
        }
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1U4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * 4 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        sums[(in[0] << 2)    ] ++;
        sums[(in[1] << 2) + 1] ++;
        sums[(in[2] << 2) + 2] ++;
        sums[(in[3] << 2) + 3] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1U3(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * 4 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        sums[(in[0] << 2)    ] ++;
        sums[(in[1] << 2) + 1] ++;
        sums[(in[2] << 2) + 2] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1U2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * 2 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        sums[(in[0] << 1)    ] ++;
        sums[(in[1] << 1) + 1] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1L4(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        int t = (cp->mDotI[0] * in[0]) +
                (cp->mDotI[1] * in[1]) +
                (cp->mDotI[2] * in[2]) +
                (cp->mDotI[3] * in[3]);
        sums[(t + 0x7f) >> 8] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1L3(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        int t = (cp->mDotI[0] * in[0]) +
                (cp->mDotI[1] * in[1]) +
                (cp->mDotI[2] * in[2]);
        sums[(t + 0x7f) >> 8] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1L2(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        int t = (cp->mDotI[0] * in[0]) +
                (cp->mDotI[1] * in[1]);
        sums[(t + 0x7f) >> 8] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1L1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        int t = (cp->mDotI[0] * in[0]);
        sums[(t + 0x7f) >> 8] ++;
        in += info->inStride[0];
    }
}

void RsdCpuScriptIntrinsicHistogram::kernelP1U1(const RsExpandKernelDriverInfo *info,
                                                uint32_t xstart, uint32_t xend,
                                                uint32_t outstep) {

    RsdCpuScriptIntrinsicHistogram *cp = (RsdCpuScriptIntrinsicHistogram *)info->usr;
    uchar *in = (uchar *)info->inPtr[0];
    int * sums = &cp->mSums[256 * info->lid];

    for (uint32_t x = xstart; x < xend; x++) {
        sums[in[0]] ++;
        in += info->inStride[0];
    }
}


RsdCpuScriptIntrinsicHistogram::RsdCpuScriptIntrinsicHistogram(RsdCpuReferenceImpl *ctx,
                                                     const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_HISTOGRAM) {

    mRootPtr = nullptr;
    mSums = new int[256 * 4 * mCtx->getThreadCount()];
    mDot[0] = 0.299f;
    mDot[1] = 0.587f;
    mDot[2] = 0.114f;
    mDot[3] = 0;
    mDotI[0] = (int)((mDot[0] * 256.f) + 0.5f);
    mDotI[1] = (int)((mDot[1] * 256.f) + 0.5f);
    mDotI[2] = (int)((mDot[2] * 256.f) + 0.5f);
    mDotI[3] = (int)((mDot[3] * 256.f) + 0.5f);
}

RsdCpuScriptIntrinsicHistogram::~RsdCpuScriptIntrinsicHistogram() {
    if (mSums) {
        delete []mSums;
    }
}

void RsdCpuScriptIntrinsicHistogram::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 2;
}

void RsdCpuScriptIntrinsicHistogram::invokeFreeChildren() {
}

RsdCpuScriptImpl * rsdIntrinsic_Histogram(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicHistogram(ctx, s, e);
}

} // namespace renderscript
} // namespace android
