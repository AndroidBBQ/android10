/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsScriptGroup.h"
#include "rsCpuScriptGroup.h"

#include <vector>

namespace android {
namespace renderscript {

CpuScriptGroupImpl::CpuScriptGroupImpl(RsdCpuReferenceImpl *ctx, const ScriptGroupBase *sg) {
    mCtx = ctx;
    mSG = (ScriptGroup*)sg;
}

CpuScriptGroupImpl::~CpuScriptGroupImpl() {

}

bool CpuScriptGroupImpl::init() {
    return true;
}

void CpuScriptGroupImpl::setInput(const ScriptKernelID *kid, Allocation *a) {
}

void CpuScriptGroupImpl::setOutput(const ScriptKernelID *kid, Allocation *a) {
}


typedef void (*ScriptGroupRootFunc_t)(const RsExpandKernelDriverInfo *kinfo,
                                      uint32_t xstart, uint32_t xend,
                                      uint32_t outstep);

void CpuScriptGroupImpl::scriptGroupRoot(const RsExpandKernelDriverInfo *kinfo,
                                         uint32_t xstart, uint32_t xend,
                                         uint32_t outstep) {


    const ScriptList *sl             = (const ScriptList *)kinfo->usr;
    RsExpandKernelDriverInfo *mkinfo = const_cast<RsExpandKernelDriverInfo *>(kinfo);

    const uint32_t oldInStride = mkinfo->inStride[0];

    for (size_t ct = 0; ct < sl->count; ct++) {
        ScriptGroupRootFunc_t func;
        func          = (ScriptGroupRootFunc_t)sl->fnPtrs[ct];
        mkinfo->usr   = sl->usrPtrs[ct];

        if (sl->ins[ct]) {
            rsAssert(kinfo->inLen == 1);

            mkinfo->inPtr[0] = (const uint8_t *)sl->ins[ct]->mHal.drvState.lod[0].mallocPtr;

            mkinfo->inStride[0] = sl->ins[ct]->mHal.state.elementSizeBytes;

            if (sl->inExts[ct]) {
                mkinfo->inPtr[0] =
                  (mkinfo->inPtr[0] +
                   sl->ins[ct]->mHal.drvState.lod[0].stride * kinfo->current.y);

            } else if (sl->ins[ct]->mHal.drvState.lod[0].dimY > kinfo->lid) {
                mkinfo->inPtr[0] =
                  (mkinfo->inPtr[0] +
                   sl->ins[ct]->mHal.drvState.lod[0].stride * kinfo->lid);
            }

        } else {
            rsAssert(kinfo->inLen == 0);

            mkinfo->inPtr[0]     = nullptr;
            mkinfo->inStride[0]  = 0;
        }

        uint32_t ostep;
        if (sl->outs[ct]) {
            mkinfo->outPtr[0] =
              (uint8_t *)sl->outs[ct]->mHal.drvState.lod[0].mallocPtr;

            ostep = sl->outs[ct]->mHal.state.elementSizeBytes;

            if (sl->outExts[ct]) {
                mkinfo->outPtr[0] =
                  mkinfo->outPtr[0] +
                  sl->outs[ct]->mHal.drvState.lod[0].stride * kinfo->current.y;

            } else if (sl->outs[ct]->mHal.drvState.lod[0].dimY > kinfo->lid) {
                mkinfo->outPtr[0] =
                  mkinfo->outPtr[0] +
                  sl->outs[ct]->mHal.drvState.lod[0].stride * kinfo->lid;
            }
        } else {
            mkinfo->outPtr[0] = nullptr;
            ostep             = 0;
        }

        //ALOGE("kernel %i %p,%p  %p,%p", ct, mp->ptrIn, mp->in, mp->ptrOut, mp->out);
        func(kinfo, xstart, xend, ostep);
    }
    //ALOGE("script group root");

    mkinfo->inStride[0] = oldInStride;
    mkinfo->usr         = sl;
}



void CpuScriptGroupImpl::execute() {
    std::vector<Allocation *> ins;
    std::vector<uint8_t> inExts;
    std::vector<Allocation *> outs;
    std::vector<uint8_t> outExts;
    std::vector<const ScriptKernelID *> kernels;
    bool fieldDep = false;

    for (size_t ct=0; ct < mSG->mNodes.size(); ct++) {
        ScriptGroup::Node *n = mSG->mNodes[ct];
        Script *s = n->mKernels[0]->mScript;
        if (s->hasObjectSlots()) {
            // Disable the ScriptGroup optimization if we have global RS
            // objects that might interfere between kernels.
            fieldDep = true;
        }

        //ALOGE("node %i, order %i, in %i out %i", (int)ct, n->mOrder, (int)n->mInputs.size(), (int)n->mOutputs.size());

        for (size_t ct2=0; ct2 < n->mInputs.size(); ct2++) {
            if (n->mInputs[ct2]->mDstField.get() && n->mInputs[ct2]->mDstField->mScript) {
                //ALOGE("field %p %zu", n->mInputs[ct2]->mDstField->mScript, n->mInputs[ct2]->mDstField->mSlot);
                s->setVarObj(n->mInputs[ct2]->mDstField->mSlot, n->mInputs[ct2]->mAlloc.get());
            }
        }

        for (size_t ct2=0; ct2 < n->mKernels.size(); ct2++) {
            const ScriptKernelID *k = n->mKernels[ct2];
            Allocation *ain = nullptr;
            Allocation *aout = nullptr;
            bool inExt = false;
            bool outExt = false;

            for (size_t ct3=0; ct3 < n->mInputs.size(); ct3++) {
                if (n->mInputs[ct3]->mDstKernel.get() == k) {
                    ain = n->mInputs[ct3]->mAlloc.get();
                    break;
                }
            }
            if (ain == nullptr) {
                for (size_t ct3=0; ct3 < mSG->mInputs.size(); ct3++) {
                    if (mSG->mInputs[ct3]->mKernel == k) {
                        ain = mSG->mInputs[ct3]->mAlloc.get();
                        inExt = true;
                        break;
                    }
                }
            }

            for (size_t ct3=0; ct3 < n->mOutputs.size(); ct3++) {
                if (n->mOutputs[ct3]->mSource.get() == k) {
                    aout = n->mOutputs[ct3]->mAlloc.get();
                    if(n->mOutputs[ct3]->mDstField.get() != nullptr) {
                        fieldDep = true;
                    }
                    break;
                }
            }
            if (aout == nullptr) {
                for (size_t ct3=0; ct3 < mSG->mOutputs.size(); ct3++) {
                    if (mSG->mOutputs[ct3]->mKernel == k) {
                        aout = mSG->mOutputs[ct3]->mAlloc.get();
                        outExt = true;
                        break;
                    }
                }
            }

            rsAssert((k->mHasKernelOutput == (aout != nullptr)) &&
                     (k->mHasKernelInput == (ain != nullptr)));

            ins.push_back(ain);
            inExts.push_back(inExt);
            outs.push_back(aout);
            outExts.push_back(outExt);
            kernels.push_back(k);
        }

    }

    MTLaunchStructForEach mtls;

    if (fieldDep) {
        for (size_t ct=0; ct < ins.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);
            uint32_t slot = kernels[ct]->mSlot;

            uint32_t inLen;
            const Allocation **ains;

            if (ins[ct] == nullptr) {
                inLen = 0;
                ains  = nullptr;

            } else {
                inLen = 1;
                ains  = const_cast<const Allocation**>(&ins[ct]);
            }

            bool launchOK = si->forEachMtlsSetup(ains, inLen, outs[ct], nullptr, 0, nullptr, &mtls);

            si->forEachKernelSetup(slot, &mtls);
            si->preLaunch(slot, ains, inLen, outs[ct], mtls.fep.usr,
                          mtls.fep.usrLen, nullptr);

            if (launchOK) {
                mCtx->launchForEach(ains, inLen, outs[ct], nullptr, &mtls);
            }

            si->postLaunch(slot, ains, inLen, outs[ct], nullptr, 0, nullptr);
        }
    } else {
        ScriptList sl;
        sl.ins = ins.data();
        sl.outs = outs.data();
        sl.kernels = kernels.data();
        sl.count = kernels.size();

        uint32_t inLen;
        const Allocation **ains;

        if (ins[0] == nullptr) {
            inLen = 0;
            ains  = nullptr;

        } else {
            inLen = 1;
            ains  = const_cast<const Allocation**>(&ins[0]);
        }

        std::vector<const void *> usrPtrs;
        std::vector<const void *> fnPtrs;
        std::vector<uint32_t> sigs;
        for (size_t ct=0; ct < kernels.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);

            si->forEachKernelSetup(kernels[ct]->mSlot, &mtls);
            fnPtrs.push_back((void *)mtls.kernel);
            usrPtrs.push_back(mtls.fep.usr);
            sigs.push_back(mtls.fep.usrLen);
            si->preLaunch(kernels[ct]->mSlot, ains, inLen, outs[ct],
                          mtls.fep.usr, mtls.fep.usrLen, nullptr);
        }
        sl.sigs = sigs.data();
        sl.usrPtrs = usrPtrs.data();
        sl.fnPtrs = fnPtrs.data();
        sl.inExts = inExts.data();
        sl.outExts = outExts.data();

        Script *s = kernels[0]->mScript;
        RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);

        if (si->forEachMtlsSetup(ains, inLen, outs[0], nullptr, 0, nullptr, &mtls)) {

            mtls.script = nullptr;
            mtls.kernel = &scriptGroupRoot;
            mtls.fep.usr = &sl;

            mCtx->launchForEach(ains, inLen, outs[0], nullptr, &mtls);
        }

        for (size_t ct=0; ct < kernels.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);
            si->postLaunch(kernels[ct]->mSlot, ains, inLen, outs[ct], nullptr, 0,
                           nullptr);
        }
    }
}

} // namespace renderscript
} // namespace android
