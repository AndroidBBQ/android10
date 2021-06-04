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
#include "rsProgramVertex.h"
#include "rsMatrix4x4.h"

using android::renderscript::ProgramVertex;
using android::renderscript::ProgramVertexState;

ProgramVertex::ProgramVertex(Context *rsc, const char * shaderText, size_t shaderLength,
                             const char** textureNames, size_t textureNamesCount, const size_t *textureNamesLength,

                             const uintptr_t * params, size_t paramLength)
    : Program(rsc, shaderText, shaderLength, params, paramLength) {
    mRSC->mHal.funcs.vertex.init(mRSC, this, mUserShader, mUserShaderLen,
                                 textureNames, textureNamesCount, textureNamesLength);
}

ProgramVertex::~ProgramVertex() {
    mRSC->mHal.funcs.vertex.destroy(mRSC, this);
}

void ProgramVertex::setup(Context *rsc, ProgramVertexState *state) {
    if ((state->mLast.get() == this) && !mDirty) {
        return;
    }

    if (!isUserProgram()) {
        if (mHal.state.constants[0] == nullptr) {
            rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                          "Unable to set fixed function emulation matrices because allocation is missing");
            return;
        }
        float *f = static_cast<float *>(rsc->mHal.funcs.allocation.lock1D(
                rsc, mHal.state.constants[0]));
        Matrix4x4 mvp;
        mvp.load(&f[RS_PROGRAM_VERTEX_PROJECTION_OFFSET]);
        Matrix4x4 t;
        t.load(&f[RS_PROGRAM_VERTEX_MODELVIEW_OFFSET]);
        mvp.multiply(&t);
        for (uint32_t i = 0; i < 16; i ++) {
            f[RS_PROGRAM_VERTEX_MVP_OFFSET + i] = mvp.m[i];
        }
        rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
    }

    state->mLast.set(this);

    rsc->mHal.funcs.vertex.setActive(rsc, this);
}

void ProgramVertex::setProjectionMatrix(Context *rsc, const rsc_Matrix *m) const {
    if (isUserProgram()) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Attempting to set fixed function emulation matrix projection on user program");
        return;
    }
    if (mHal.state.constants[0] == nullptr) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Unable to set fixed function emulation matrix projection because allocation is missing");
        return;
    }
    float *f = static_cast<float *>(rsc->mHal.funcs.allocation.lock1D(
                rsc, mHal.state.constants[0]));
    memcpy(&f[RS_PROGRAM_VERTEX_PROJECTION_OFFSET], m, sizeof(rsc_Matrix));
    mDirty = true;
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramVertex::setModelviewMatrix(Context *rsc, const rsc_Matrix *m) const {
    if (isUserProgram()) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Attempting to set fixed function emulation matrix modelview on user program");
        return;
    }
    if (mHal.state.constants[0] == nullptr) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Unable to set fixed function emulation matrix modelview because allocation is missing");
        return;
    }
    float *f = static_cast<float *>(rsc->mHal.funcs.allocation.lock1D(
                rsc, mHal.state.constants[0]));
    memcpy(&f[RS_PROGRAM_VERTEX_MODELVIEW_OFFSET], m, sizeof(rsc_Matrix));
    mDirty = true;
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramVertex::setTextureMatrix(Context *rsc, const rsc_Matrix *m) const {
    if (isUserProgram()) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Attempting to set fixed function emulation matrix texture on user program");
        return;
    }
    if (mHal.state.constants[0] == nullptr) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Unable to set fixed function emulation matrix texture because allocation is missing");
        return;
    }
    float *f = static_cast<float *>(rsc->mHal.funcs.allocation.lock1D(
            rsc, mHal.state.constants[0]));
    memcpy(&f[RS_PROGRAM_VERTEX_TEXTURE_OFFSET], m, sizeof(rsc_Matrix));
    mDirty = true;
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramVertex::getProjectionMatrix(Context *rsc, rsc_Matrix *m) const {
    if (isUserProgram()) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Attempting to get fixed function emulation matrix projection on user program");
        return;
    }
    if (mHal.state.constants[0] == nullptr) {
        rsc->setError(RS_ERROR_FATAL_UNKNOWN,
                      "Unable to get fixed function emulation matrix projection because allocation is missing");
        return;
    }
    float *f = static_cast<float *>(
            rsc->mHal.funcs.allocation.lock1D(rsc, mHal.state.constants[0]));
    memcpy(m, &f[RS_PROGRAM_VERTEX_PROJECTION_OFFSET], sizeof(rsc_Matrix));
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramVertex::transformToScreen(Context *rsc, float *v4out, const float *v3in) const {
    if (isUserProgram()) {
        return;
    }
    float *f = static_cast<float *>(
            rsc->mHal.funcs.allocation.lock1D(rsc, mHal.state.constants[0]));
    Matrix4x4 mvp;
    mvp.loadMultiply((Matrix4x4 *)&f[RS_PROGRAM_VERTEX_MODELVIEW_OFFSET],
                     (Matrix4x4 *)&f[RS_PROGRAM_VERTEX_PROJECTION_OFFSET]);
    mvp.vectorMultiply(v4out, v3in);
    rsc->mHal.funcs.allocation.unlock1D(rsc, mHal.state.constants[0]);
}

void ProgramVertex::serialize(Context *rsc, OStream *stream) const {
}

ProgramVertex *ProgramVertex::createFromStream(Context *rsc, IStream *stream) {
    return nullptr;
}


///////////////////////////////////////////////////////////////////////

ProgramVertexState::ProgramVertexState() {
}

ProgramVertexState::~ProgramVertexState() {
}

void ProgramVertexState::init(Context *rsc) {
    ObjectBaseRef<const Element> matrixElem = Element::createRef(rsc, RS_TYPE_MATRIX_4X4,
                                                                 RS_KIND_USER, false, 1);
    ObjectBaseRef<const Element> f2Elem = Element::createRef(rsc, RS_TYPE_FLOAT_32,
                                                             RS_KIND_USER, false, 2);
    ObjectBaseRef<const Element> f3Elem = Element::createRef(rsc, RS_TYPE_FLOAT_32,
                                                             RS_KIND_USER, false, 3);
    ObjectBaseRef<const Element> f4Elem = Element::createRef(rsc, RS_TYPE_FLOAT_32,
                                                             RS_KIND_USER, false, 4);

    const char *ebn1[] = { "MV", "P", "TexMatrix", "MVP" };
    const Element *ebe1[] = {matrixElem.get(), matrixElem.get(),
            matrixElem.get(), matrixElem.get()};
    ObjectBaseRef<const Element> constInput = Element::create(rsc, 4, ebe1, ebn1);

    const char *ebn2[] = { "position", "color", "normal", "texture0" };
    const Element *ebe2[] = {f4Elem.get(), f4Elem.get(), f3Elem.get(), f2Elem.get()};
    ObjectBaseRef<const Element> attrElem = Element::create(rsc, 4, ebe2, ebn2);

    ObjectBaseRef<Type> inputType = Type::getTypeRef(rsc, constInput.get(), 1);

    const char *shaderString =
            RS_SHADER_INTERNAL
            "varying vec4 varColor;\n"
            "varying vec2 varTex0;\n"
            "void main() {\n"
            "  gl_Position = UNI_MVP * ATTRIB_position;\n"
            "  gl_PointSize = 1.0;\n"
            "  varColor = ATTRIB_color;\n"
            "  varTex0 = ATTRIB_texture0;\n"
            "}\n";

    uintptr_t tmp[4];
    tmp[0] = RS_PROGRAM_PARAM_CONSTANT;
    tmp[1] = (uintptr_t)inputType.get();
    tmp[2] = RS_PROGRAM_PARAM_INPUT;
    tmp[3] = (uintptr_t)attrElem.get();

    ProgramVertex *pv = new ProgramVertex(rsc, shaderString, strlen(shaderString),
                                          nullptr, 0, nullptr, tmp, 4);
    Allocation *alloc = Allocation::createAllocation(rsc, inputType.get(),
                              RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS);
    pv->bindAllocation(rsc, alloc, 0);

    mDefaultAlloc.set(alloc);
    mDefault.set(pv);

    updateSize(rsc);
}

void ProgramVertexState::updateSize(Context *rsc) {
    float *f = static_cast<float *>(rsc->mHal.funcs.allocation.lock1D(rsc, mDefaultAlloc.get()));

    float surfaceWidth = (float)rsc->getCurrentSurfaceWidth();
    float surfaceHeight = (float)rsc->getCurrentSurfaceHeight();

    Matrix4x4 m;
    m.loadOrtho(0, surfaceWidth, surfaceHeight, 0, -1, 1);
    memcpy(&f[RS_PROGRAM_VERTEX_PROJECTION_OFFSET], m.m, sizeof(m));
    memcpy(&f[RS_PROGRAM_VERTEX_MVP_OFFSET], m.m, sizeof(m));

    m.loadIdentity();
    memcpy(&f[RS_PROGRAM_VERTEX_MODELVIEW_OFFSET], m.m, sizeof(m));
    memcpy(&f[RS_PROGRAM_VERTEX_TEXTURE_OFFSET], m.m, sizeof(m));
    rsc->mHal.funcs.allocation.unlock1D(rsc, mDefaultAlloc.get());
}

void ProgramVertexState::deinit(Context *rsc) {
    mDefaultAlloc.clear();
    mDefault.clear();
    mLast.clear();
}


namespace android {
namespace renderscript {

RsProgramVertex rsi_ProgramVertexCreate(Context *rsc, const char * shaderText, size_t shaderLength,
                                        const char** textureNames, size_t textureNamesCount,
                                        const size_t *textureNamesLength,
                                        const uintptr_t * params, size_t paramLength) {
    ProgramVertex *pv = new ProgramVertex(rsc, shaderText, shaderLength,
                                          textureNames, textureNamesCount, textureNamesLength,
                                          params, paramLength);
    pv->incUserRef();
    return pv;
}

} // namespace renderscript
} // namespace android
