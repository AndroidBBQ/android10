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
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"
#include "rsMesh.h"
#include "rsgApiStructs.h"

#include "driver/rsdVertexArray.h"
#include "driver/rsdShaderCache.h"
#include "driver/rsdCore.h"

#define GL_GLEXT_PROTOTYPES

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <time.h>

namespace android {
namespace renderscript {

//////////////////////////////////////////////////////////////////////////////
// Context
//////////////////////////////////////////////////////////////////////////////

void rsrBindTexture(Context *rsc, ProgramFragment *pf, uint32_t slot, Allocation *a) {
    CHECK_OBJ_OR_NULL(a);
    CHECK_OBJ(pf);
    pf->bindTexture(rsc, slot, a);
}

void rsrBindConstant(Context *rsc, ProgramFragment *pf, uint32_t slot, Allocation *a) {
    CHECK_OBJ_OR_NULL(a);
    CHECK_OBJ(pf);
    pf->bindAllocation(rsc, a, slot);
}

void rsrBindConstant(Context *rsc, ProgramVertex *pv, uint32_t slot, Allocation *a) {
    CHECK_OBJ_OR_NULL(a);
    CHECK_OBJ(pv);
    pv->bindAllocation(rsc, a, slot);
}

void rsrBindSampler(Context *rsc, ProgramFragment *pf, uint32_t slot, Sampler *s) {
    CHECK_OBJ_OR_NULL(vs);
    CHECK_OBJ(vpf);
    pf->bindSampler(rsc, slot, s);
}

void rsrBindProgramStore(Context *rsc, ProgramStore *ps) {
    CHECK_OBJ_OR_NULL(ps);
    rsc->setProgramStore(ps);
}

void rsrBindProgramFragment(Context *rsc, ProgramFragment *pf) {
    CHECK_OBJ_OR_NULL(pf);
    rsc->setProgramFragment(pf);
}

void rsrBindProgramVertex(Context *rsc, ProgramVertex *pv) {
    CHECK_OBJ_OR_NULL(pv);
    rsc->setProgramVertex(pv);
}

void rsrBindProgramRaster(Context *rsc, ProgramRaster *pr) {
    CHECK_OBJ_OR_NULL(pr);
    rsc->setProgramRaster(pr);
}

void rsrBindFrameBufferObjectColorTarget(Context *rsc, Allocation *a, uint32_t slot) {
    CHECK_OBJ(va);
    rsc->mFBOCache.bindColorTarget(rsc, a, slot);
    rsc->mStateVertex.updateSize(rsc);
}

void rsrBindFrameBufferObjectDepthTarget(Context *rsc, Allocation *a) {
    CHECK_OBJ(va);
    rsc->mFBOCache.bindDepthTarget(rsc, a);
    rsc->mStateVertex.updateSize(rsc);
}

void rsrClearFrameBufferObjectColorTarget(Context *rsc, uint32_t slot) {
    rsc->mFBOCache.bindColorTarget(rsc, nullptr, slot);
    rsc->mStateVertex.updateSize(rsc);
}

void rsrClearFrameBufferObjectDepthTarget(Context *rsc) {
    rsc->mFBOCache.bindDepthTarget(rsc, nullptr);
    rsc->mStateVertex.updateSize(rsc);
}

void rsrClearFrameBufferObjectTargets(Context *rsc) {
    rsc->mFBOCache.resetAll(rsc);
    rsc->mStateVertex.updateSize(rsc);
}

//////////////////////////////////////////////////////////////////////////////
// VP
//////////////////////////////////////////////////////////////////////////////

void rsrVpLoadProjectionMatrix(Context *rsc, const rsc_Matrix *m) {
    rsc->getProgramVertex()->setProjectionMatrix(rsc, m);
}

void rsrVpLoadModelMatrix(Context *rsc, const rsc_Matrix *m) {
    rsc->getProgramVertex()->setModelviewMatrix(rsc, m);
}

void rsrVpLoadTextureMatrix(Context *rsc, const rsc_Matrix *m) {
    rsc->getProgramVertex()->setTextureMatrix(rsc, m);
}

void rsrPfConstantColor(Context *rsc, ProgramFragment *pf,
                        float r, float g, float b, float a) {
    CHECK_OBJ(pf);
    pf->setConstantColor(rsc, r, g, b, a);
}

void rsrVpGetProjectionMatrix(Context *rsc, rsc_Matrix *m) {
    rsc->getProgramVertex()->getProjectionMatrix(rsc, m);
}

//////////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////////


void rsrDrawPath(Context *rsc, Path *sm) {
    // Never shipped in a working state
}

void rsrDrawMesh(Context *rsc, Mesh *sm) {
    CHECK_OBJ(sm);
    if (!rsc->setupCheck()) {
        return;
    }
    sm->render(rsc);
}

void rsrDrawMeshPrimitive(Context *rsc, Mesh *sm, uint32_t primIndex) {
    CHECK_OBJ(sm);
    if (!rsc->setupCheck()) {
        return;
    }
    sm->renderPrimitive(rsc, primIndex);
}

void rsrDrawMeshPrimitiveRange(Context *rsc, Mesh *sm, uint32_t primIndex,
                               uint32_t start, uint32_t len) {
    CHECK_OBJ(sm);
    if (!rsc->setupCheck()) {
        return;
    }
    sm->renderPrimitiveRange(rsc, primIndex, start, len);
}

void rsrMeshComputeBoundingBox(Context *rsc, Mesh *sm,
                               float *minX, float *minY, float *minZ,
                               float *maxX, float *maxY, float *maxZ) {
    CHECK_OBJ(sm);
    sm->computeBBox(rsc);
    *minX = sm->mBBoxMin[0];
    *minY = sm->mBBoxMin[1];
    *minZ = sm->mBBoxMin[2];
    *maxX = sm->mBBoxMax[0];
    *maxY = sm->mBBoxMax[1];
    *maxZ = sm->mBBoxMax[2];
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////


void rsrColor(Context *rsc, float r, float g, float b, float a) {
    ProgramFragment *pf = rsc->getProgramFragment();
    pf->setConstantColor(rsc, r, g, b, a);
}

void rsrPrepareClear(Context *rsc) {
    rsc->mFBOCache.setup(rsc);
    rsc->setupProgramStore();
}

uint32_t rsrGetWidth(Context *rsc) {
    return rsc->getWidth();
}

uint32_t rsrGetHeight(Context *rsc) {
    return rsc->getHeight();
}

void rsrDrawTextAlloc(Context *rsc, Allocation *a, int x, int y) {
    const char *text = (const char *)rsc->mHal.funcs.allocation.lock1D(rsc, a);
    size_t allocSize = a->getType()->getPackedSizeBytes();
    rsc->mStateFont.renderText(text, allocSize, x, y);
    rsc->mHal.funcs.allocation.unlock1D(rsc, a);
}

void rsrDrawText(Context *rsc, const char *text, int x, int y) {
    size_t textLen = strlen(text);
    rsc->mStateFont.renderText(text, textLen, x, y);
}

static void SetMetrics(Font::Rect *metrics,
                       int32_t *left, int32_t *right, int32_t *top, int32_t *bottom) {
    if (left) {
        *left = metrics->left;
    }
    if (right) {
        *right = metrics->right;
    }
    if (top) {
        *top = metrics->top;
    }
    if (bottom) {
        *bottom = metrics->bottom;
    }
}

void rsrMeasureTextAlloc(Context *rsc, Allocation *a,
                         int32_t *left, int32_t *right, int32_t *top, int32_t *bottom) {
    CHECK_OBJ(a);
    const char *text = (const char *)rsc->mHal.funcs.allocation.lock1D(rsc, a);
    size_t textLen = a->getType()->getPackedSizeBytes();
    Font::Rect metrics;
    rsc->mStateFont.measureText(text, textLen, &metrics);
    SetMetrics(&metrics, left, right, top, bottom);
    rsc->mHal.funcs.allocation.unlock1D(rsc, a);
}

void rsrMeasureText(Context *rsc, const char *text,
                    int32_t *left, int32_t *right, int32_t *top, int32_t *bottom) {
    size_t textLen = strlen(text);
    Font::Rect metrics;
    rsc->mStateFont.measureText(text, textLen, &metrics);
    SetMetrics(&metrics, left, right, top, bottom);
}

void rsrBindFont(Context *rsc, Font *font) {
    CHECK_OBJ(font);
    rsi_ContextBindFont(rsc, font);
}

void rsrFontColor(Context *rsc, float r, float g, float b, float a) {
    rsc->mStateFont.setFontColor(r, g, b, a);
}

} // namespace renderscript
} // namespace android
