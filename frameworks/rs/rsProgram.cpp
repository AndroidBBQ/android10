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

#include "rsContext.h"
#include "rsProgram.h"

#include <inttypes.h>

namespace android {
namespace renderscript {

Program::Program(Context *rsc, const char * shaderText, size_t shaderLength,
                 const uintptr_t * params, size_t paramLength)
    : ProgramBase(rsc) {

    initMemberVars();
    for (uint32_t ct=0; ct < paramLength; ct+=2) {
        if (params[ct] == RS_PROGRAM_PARAM_INPUT) {
            mHal.state.inputElementsCount++;
        }
        if (params[ct] == RS_PROGRAM_PARAM_CONSTANT) {
            mHal.state.constantsCount++;
        }
        if (params[ct] == RS_PROGRAM_PARAM_TEXTURE_TYPE) {
            mHal.state.texturesCount++;
        }
    }

    mTextures = new ObjectBaseRef<Allocation>[mHal.state.texturesCount];
    mSamplers = new ObjectBaseRef<Sampler>[mHal.state.texturesCount];
    mInputElements = new ObjectBaseRef<Element>[mHal.state.inputElementsCount];
    mConstantTypes = new ObjectBaseRef<Type>[mHal.state.constantsCount];
    mConstants = new ObjectBaseRef<Allocation>[mHal.state.constantsCount];

    mHal.state.textures = new Allocation*[mHal.state.texturesCount];
    mHal.state.samplers = new Sampler*[mHal.state.texturesCount];
    mHal.state.textureTargets = new RsTextureTarget[mHal.state.texturesCount];
    mHal.state.inputElements = new Element*[mHal.state.inputElementsCount];
    mHal.state.constantTypes = new Type*[mHal.state.constantsCount];
    mHal.state.constants = new Allocation*[mHal.state.constantsCount];

    // Will initialize everything
    freeChildren();

    uint32_t input = 0;
    uint32_t constant = 0;
    uint32_t texture = 0;
    for (uint32_t ct=0; ct < paramLength; ct+=2) {
        if (params[ct] == RS_PROGRAM_PARAM_INPUT) {
            mInputElements[input].set(reinterpret_cast<Element *>(params[ct+1]));
            mHal.state.inputElements[input++] = reinterpret_cast<Element *>(params[ct+1]);
        }
        if (params[ct] == RS_PROGRAM_PARAM_CONSTANT) {
            mConstantTypes[constant].set(reinterpret_cast<Type *>(params[ct+1]));
            mHal.state.constantTypes[constant++] = reinterpret_cast<Type *>(params[ct+1]);
        }
        if (params[ct] == RS_PROGRAM_PARAM_TEXTURE_TYPE) {
            mHal.state.textureTargets[texture++] = (RsTextureTarget)params[ct+1];
        }
    }
    mIsInternal = false;
    uint32_t internalTokenLen = strlen(RS_SHADER_INTERNAL);
    if (shaderLength > internalTokenLen &&
       strncmp(RS_SHADER_INTERNAL, shaderText, internalTokenLen) == 0) {
        mIsInternal = true;
        shaderText += internalTokenLen;
        shaderLength -= internalTokenLen;
    }

    mUserShader = rsuCopyString(shaderText, shaderLength);
    mUserShaderLen = shaderLength;
}

Program::~Program() {
    freeChildren();

    delete[] mTextures;
    delete[] mSamplers;
    delete[] mInputElements;
    delete[] mConstantTypes;
    delete[] mConstants;

    delete[] mHal.state.textures;
    delete[] mHal.state.samplers;
    delete[] mHal.state.textureTargets;
    delete[] mHal.state.inputElements;
    delete[] mHal.state.constantTypes;
    delete[] mHal.state.constants;
    mHal.state.inputElementsCount = 0;
    mHal.state.constantsCount = 0;
    mHal.state.texturesCount = 0;

    if (mUserShader != nullptr) {
        delete[] mUserShader;
        mUserShader = nullptr;
    }
    mUserShaderLen = 0;
}

bool Program::freeChildren() {
    for (uint32_t ct=0; ct < mHal.state.constantsCount; ct++) {
        bindAllocation(nullptr, nullptr, ct);
    }

    for (uint32_t ct=0; ct < mHal.state.texturesCount; ct++) {
        bindTexture(nullptr, ct, nullptr);
        bindSampler(nullptr, ct, nullptr);
    }
    return false;
}

void Program::initMemberVars() {
    mDirty = true;

    mHal.drv = nullptr;
    mHal.state.textures = nullptr;
    mHal.state.samplers = nullptr;
    mHal.state.textureTargets = nullptr;
    mHal.state.inputElements = nullptr;
    mHal.state.constantTypes = nullptr;
    mHal.state.constants = nullptr;

    mHal.state.inputElementsCount = 0;
    mHal.state.constantsCount = 0;
    mHal.state.texturesCount = 0;

    mTextures = nullptr;
    mSamplers = nullptr;
    mInputElements = nullptr;
    mConstantTypes = nullptr;
    mConstants = nullptr;

    mIsInternal = false;

    mUserShader = nullptr;
    mUserShaderLen = 0;
}

void Program::bindAllocation(Context *rsc, Allocation *alloc, uint32_t slot) {
    if (alloc != nullptr) {
        if (slot >= mHal.state.constantsCount) {
            ALOGE("Attempt to bind alloc at slot %u, on shader id %" PRIuPTR ", but const count is %u",
                 slot, (uintptr_t)this, mHal.state.constantsCount);
            rsc->setError(RS_ERROR_BAD_SHADER, "Cannot bind allocation");
            return;
        }
        if (alloc->getType() != mConstantTypes[slot].get()) {
            ALOGE("Attempt to bind alloc at slot %u, on shader id %" PRIuPTR ", but types mismatch",
                 slot, (uintptr_t)this);
            rsc->setError(RS_ERROR_BAD_SHADER, "Cannot bind allocation");
            return;
        }
    }
    if (mConstants[slot].get() == alloc) {
        return;
    }
    if (mConstants[slot].get()) {
        mConstants[slot]->removeProgramToDirty(this);
    }
    mConstants[slot].set(alloc);
    mHal.state.constants[slot] = alloc;
    if (alloc) {
        alloc->addProgramToDirty(this);
    }
    mDirty = true;
}

void Program::bindTexture(Context *rsc, uint32_t slot, Allocation *a) {
    if (slot >= mHal.state.texturesCount) {
        ALOGE("Attempt to bind texture to slot %u but tex count is %u", slot, mHal.state.texturesCount);
        rsc->setError(RS_ERROR_BAD_SHADER, "Cannot bind texture");
        return;
    }

    if (a && a->getType()->getDimFaces() && mHal.state.textureTargets[slot] != RS_TEXTURE_CUBE) {
        ALOGE("Attempt to bind cubemap to slot %u but 2d texture needed", slot);
        rsc->setError(RS_ERROR_BAD_SHADER, "Cannot bind cubemap to 2d texture slot");
        return;
    }

    mTextures[slot].set(a);
    mHal.state.textures[slot] = a;

    mDirty = true;
}

void Program::bindSampler(Context *rsc, uint32_t slot, Sampler *s) {
    if (slot >= mHal.state.texturesCount) {
        ALOGE("Attempt to bind sampler to slot %u but tex count is %u", slot, mHal.state.texturesCount);
        rsc->setError(RS_ERROR_BAD_SHADER, "Cannot bind sampler");
        return;
    }

    mSamplers[slot].set(s);
    mHal.state.samplers[slot] = s;
    mDirty = true;
}

void rsi_ProgramBindConstants(Context *rsc, RsProgram vp, uint32_t slot, RsAllocation constants) {
    Program *p = static_cast<Program *>(vp);
    p->bindAllocation(rsc, static_cast<Allocation *>(constants), slot);
}

void rsi_ProgramBindTexture(Context *rsc, RsProgram vpf, uint32_t slot, RsAllocation a) {
    Program *p = static_cast<Program *>(vpf);
    p->bindTexture(rsc, slot, static_cast<Allocation *>(a));
}

void rsi_ProgramBindSampler(Context *rsc, RsProgram vpf, uint32_t slot, RsSampler s) {
    Program *p = static_cast<Program *>(vpf);
    p->bindSampler(rsc, slot, static_cast<Sampler *>(s));
}

} // namespace renderscript
} // namespace android
