/*
 * Copyright (C) 2008-2012 The Android Open Source Project
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

#include "RenderScript.h"
#include "rsCppInternal.h"

using android::RSC::Sampler;
using android::RSC::sp;

Sampler::Sampler(sp<RS> rs, void* id):
    BaseObj(id, rs)
{
    RsSamplerValue mMin = RS_SAMPLER_INVALID;
    RsSamplerValue mMag = RS_SAMPLER_INVALID;
    RsSamplerValue mWrapS = RS_SAMPLER_INVALID;
    RsSamplerValue mWrapT = RS_SAMPLER_INVALID;
    float mAniso = 0.f;
}

Sampler::Sampler(sp<RS> rs, void* id, RsSamplerValue min, RsSamplerValue mag,
                 RsSamplerValue wrapS, RsSamplerValue wrapT, float anisotropy):
    BaseObj(id, rs)
{
    RsSamplerValue mMin = min;
    RsSamplerValue mMag = mag;
    RsSamplerValue mWrapS = wrapS;
    RsSamplerValue mWrapT = wrapT;
    float mAniso = anisotropy;
}

RsSamplerValue Sampler::getMinification() {
    return mMin;
}

RsSamplerValue Sampler::getMagnification() {
    return mMag;
}

RsSamplerValue Sampler::getWrapS() {
    return mWrapS;
}

RsSamplerValue Sampler::getWrapT() {
    return mWrapT;
}

float Sampler::getAnisotropy() {
    return mAniso;
}

sp<Sampler> Sampler::create(const sp<RS>& rs, RsSamplerValue min, RsSamplerValue mag,
                            RsSamplerValue wrapS, RsSamplerValue wrapT, float anisotropy) {
    // We aren't supporting wrapR in C++ API atm, so always pass wrap for that.
    void* id = RS::dispatch->SamplerCreate(rs->getContext(), min, mag, wrapS, wrapT,
                                           RS_SAMPLER_WRAP, anisotropy);
    return new Sampler(rs, id, min, mag, wrapS, wrapT, anisotropy);
}

#define CREATE_SAMPLER(N, MIN, MAG, WRAPS, WRAPT) sp<const Sampler> Sampler::N(const sp<RS> &rs) { \
        if (rs->mSamplers.N == nullptr) {                                \
            rs->mSamplers.N = (create(rs, MIN, MAG, WRAPS, WRAPT, 0.f)); \
        }                                                                \
        return rs->mSamplers.N;                                          \
    }

CREATE_SAMPLER(CLAMP_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_CLAMP, RS_SAMPLER_CLAMP);
CREATE_SAMPLER(CLAMP_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_CLAMP, RS_SAMPLER_CLAMP);
CREATE_SAMPLER(CLAMP_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_CLAMP, RS_SAMPLER_CLAMP);
CREATE_SAMPLER(WRAP_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_WRAP, RS_SAMPLER_WRAP);
CREATE_SAMPLER(WRAP_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_WRAP, RS_SAMPLER_WRAP);
CREATE_SAMPLER(WRAP_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_WRAP, RS_SAMPLER_WRAP);
CREATE_SAMPLER(MIRRORED_REPEAT_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_NEAREST, RS_SAMPLER_MIRRORED_REPEAT, RS_SAMPLER_MIRRORED_REPEAT);
CREATE_SAMPLER(MIRRORED_REPEAT_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_MIRRORED_REPEAT, RS_SAMPLER_MIRRORED_REPEAT);
CREATE_SAMPLER(MIRRORED_REPEAT_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR_MIP_LINEAR, RS_SAMPLER_LINEAR, RS_SAMPLER_MIRRORED_REPEAT, RS_SAMPLER_MIRRORED_REPEAT);
