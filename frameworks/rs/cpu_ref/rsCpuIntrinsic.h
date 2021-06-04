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

#ifndef RSD_CPU_SCRIPT_INTRINSIC_H
#define RSD_CPU_SCRIPT_INTRINSIC_H

#include "rsCpuScript.h"


namespace android {
namespace renderscript {


struct MTLaunchStructForEachBlas : public MTLaunchStructCommon {
    // Driver info structure
    RsExpandKernelDriverInfo fep;

    // Tile size info for M, and N dimensions.
    uint32_t tileSizeM;
    uint32_t numTileM;
    uint32_t tileSizeN;
    uint32_t numTileN;

    const Allocation *ains[RS_KERNEL_INPUT_LIMIT];
    const RsBlasCall *sc;
};


class RsdCpuScriptIntrinsic : public RsdCpuScriptImpl {
public:
    void populateScript(Script *) override = 0;

    void invokeFunction(uint32_t slot, const void * params, size_t paramLength) override;
    int invokeRoot() override;

    void invokeForEach(uint32_t slot,
                       const Allocation ** ain,
                       uint32_t inLen,
                       Allocation * aout,
                       const void * usr,
                       uint32_t usrLen,
                       const RsScriptCall *sc) override;

    void forEachKernelSetup(uint32_t slot, MTLaunchStructForEach * mtls) override;
    void invokeInit() override;
    void invokeFreeChildren() override;

    void preLaunch(uint32_t slot, const Allocation ** ains,
                   uint32_t inLen, Allocation * aout, const void * usr,
                   uint32_t usrLen, const RsScriptCall * sc) override;
    void postLaunch(uint32_t slot, const Allocation ** ains,
                    uint32_t inLen, Allocation * aout,
                    const void * usr, uint32_t usrLen,
                    const RsScriptCall * sc) override;

    void setGlobalVar(uint32_t slot, const void * data, size_t dataLength) override;
    void setGlobalVarWithElemDims(uint32_t slot, const void * data,
                                  size_t dataLength, const Element * e,
                                  const uint32_t * dims,
                                  size_t dimLength) override;
    void setGlobalBind(uint32_t slot, Allocation *data) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    ~RsdCpuScriptIntrinsic() override;
    RsdCpuScriptIntrinsic(RsdCpuReferenceImpl * ctx, const Script * s,
                          const Element * e, RsScriptIntrinsicID iid);

protected:
    RsScriptIntrinsicID mID;
    ForEachFunc_t mRootPtr;
    ObjectBaseRef<const Element> mElement;

};



} // namespace renderscript
} // namespace android

#endif
