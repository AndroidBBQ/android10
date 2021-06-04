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

#include "rsContext.h"
#include "rsProgramFragment.h"

#include <inttypes.h>

namespace android {
namespace renderscript {

ProgramFragment::ProgramFragment(Context *rsc, const char * shaderText, size_t shaderLength,
                                 const char** textureNames, size_t textureNamesCount, const size_t *textureNamesLength,
                                 const uintptr_t * params, size_t paramLength)
    : Program(rsc, shaderText, shaderLength, params, paramLength) {
    mConstantColor[0] = 1.f;
    mConstantColor[1] = 1.f;
    mConstantColor[2] = 1.f;
    mConstantColor[3] = 1.f;

    mRSC->mHal.funcs.fragment.init(mRSC, this, mUserShader, mUserShaderLen,
                                   textureNames, textureNamesCount, textureNamesLength);
}

ProgramFragment::~ProgramFragment() {
    mRSC->mHal.funcs.fragment.destroy(mRSC, this);
}

void ProgramFragment::setConstantColor(Context *rsc, float r, float g, float b, float a) {
    if (isUserProgram()) {
        ALOGE("Attempting to set fixed function emulation color on user program");
        rsc->setError(RS_ERROR_BAD_SHADER, "Cannot  set fixed function emulation color on user program");
        return;
    }
    if (mHal.state.constants[0] == nullptr) {
        ALOGE("Unable to set fixed function emulation color because allocation is missing");
        rsc->setError(RS_ERROR_BAD_SHADER, "Unable to set fixed function emulation color because allocation is missing");
        return;
    }
    mConstantColor[0] = r;
    mConstantColor[1] = g;
    mConstantColor[2] = b;
    mConstantColor[3] = a;
    void *p = rsc->mHal.funcs.allocation.lock1D(rsc, mHal.state.constants[0]);
    memcpy(p, mConstantColor, 4*sizeof(float));
    mDirty = true;
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramFragment::setup(Context *rsc, ProgramFragmentState *state) {
    if ((state->mLast.get() == this) && !mDirty) {
        return;
    }
    state->mLast.set(this);

    for (uint32_t ct=0; ct < mHal.state.texturesCount; ct++) {
        if (!mHal.state.textures[ct]) {
            ALOGE("No texture bound for shader id %" PRIuPTR ", texture unit %u", (uintptr_t)this, ct);
            rsc->setError(RS_ERROR_BAD_SHADER, "No texture bound");
            continue;
        }
    }

    rsc->mHal.funcs.fragment.setActive(rsc, this);
}

void ProgramFragment::serialize(Context *rsc, OStream *stream) const {
}

ProgramFragment *ProgramFragment::createFromStream(Context *rsc, IStream *stream) {
    return nullptr;
}

ProgramFragmentState::ProgramFragmentState() {
    mPF = nullptr;
}

ProgramFragmentState::~ProgramFragmentState() {
    ObjectBase::checkDelete(mPF);
    mPF = nullptr;
}

void ProgramFragmentState::init(Context *rsc) {
    const char *shaderString =
            RS_SHADER_INTERNAL
            "varying lowp vec4 varColor;\n"
            "varying vec2 varTex0;\n"
            "void main() {\n"
            "  lowp vec4 col = UNI_Color;\n"
            "  gl_FragColor = col;\n"
            "}\n";

    ObjectBaseRef<const Element> colorElem = Element::createRef(rsc, RS_TYPE_FLOAT_32, RS_KIND_USER, false, 4);

    const char *enames[] = { "Color" };
    const Element *eins[] = {colorElem.get()};
    ObjectBaseRef<const Element> constInput = Element::create(rsc, 1, eins, enames);

    ObjectBaseRef<Type> inputType = Type::getTypeRef(rsc, constInput.get(), 1);

    uintptr_t tmp[2];
    tmp[0] = RS_PROGRAM_PARAM_CONSTANT;
    tmp[1] = (uintptr_t)inputType.get();

    Allocation *constAlloc = Allocation::createAllocation(rsc, inputType.get(),
                              RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS);
    ProgramFragment *pf = new ProgramFragment(rsc, shaderString, strlen(shaderString),
                                              nullptr, 0, nullptr, tmp, 2);
    pf->bindAllocation(rsc, constAlloc, 0);
    pf->setConstantColor(rsc, 1.0f, 1.0f, 1.0f, 1.0f);

    mDefault.set(pf);
}

void ProgramFragmentState::deinit(Context *rsc) {
    mDefault.clear();
    mLast.clear();
}

RsProgramFragment rsi_ProgramFragmentCreate(Context *rsc, const char * shaderText,
                                            size_t shaderLength,
                                            const char** textureNames,
                                            size_t textureNamesCount,
                                            const size_t *textureNamesLength,
                                            const uintptr_t * params, size_t paramLength) {
    ProgramFragment *pf = new ProgramFragment(rsc, shaderText, shaderLength,
                                              textureNames, textureNamesCount, textureNamesLength,
                                              params, paramLength);
    pf->incUserRef();
    //ALOGE("rsi_ProgramFragmentCreate %p", pf);
    return pf;
}

} // namespace renderscript
} // namespace android
