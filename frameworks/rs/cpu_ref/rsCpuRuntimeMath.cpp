/*
 * Copyright (C) 2011-2012 The Android Open Source Project
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

#include "rsCpuCore.h"
#include "rsCpuScript.h"

using android::renderscript::Matrix2x2;
using android::renderscript::Matrix3x3;
using android::renderscript::Matrix4x4;

#define EXPORT_F32_FN_F32(func)                                 \
    float __attribute__((overloadable)) SC_##func(float v) {    \
        return func(v);                                         \
    }

#define EXPORT_F32_FN_F32_F32(func)                                     \
    float __attribute__((overloadable)) SC_##func(float t, float v) {   \
        return func(t, v);                                              \
    }

//////////////////////////////////////////////////////////////////////////////
// Float util
//////////////////////////////////////////////////////////////////////////////

// Handle missing Gingerbread functions like tgammaf.
float SC_tgammaf(float x) {
#ifdef RS_COMPATIBILITY_LIB
    return __builtin_tgamma(x);
#else
    return tgammaf(x);
#endif
}

uint32_t SC_abs_i32(int32_t v) {return abs(v);}

static void SC_MatrixLoadRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->loadRotate(rot, x, y, z);
}
static void SC_MatrixLoadScale(Matrix4x4 *m, float x, float y, float z) {
    m->loadScale(x, y, z);
}
static void SC_MatrixLoadTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->loadTranslate(x, y, z);
}
static void SC_MatrixRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->rotate(rot, x, y, z);
}
static void SC_MatrixScale(Matrix4x4 *m, float x, float y, float z) {
    m->scale(x, y, z);
}
static void SC_MatrixTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->translate(x, y, z);
}

static void SC_MatrixLoadOrtho(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadOrtho(l, r, b, t, n, f);
}
static void SC_MatrixLoadFrustum(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadFrustum(l, r, b, t, n, f);
}
static void SC_MatrixLoadPerspective(Matrix4x4 *m, float fovy, float aspect, float near, float far) {
    m->loadPerspective(fovy, aspect, near, far);
}

static bool SC_MatrixInverse_4x4(Matrix4x4 *m) {
    return m->inverse();
}
static bool SC_MatrixInverseTranspose_4x4(Matrix4x4 *m) {
    return m->inverseTranspose();
}
static void SC_MatrixTranspose_4x4(Matrix4x4 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_3x3(Matrix3x3 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_2x2(Matrix2x2 *m) {
    m->transpose();
}

float SC_randf2(float min, float max) {
    float r = (float)rand();
    r /= RAND_MAX;
    r = r * (max - min) + min;
    return r;
}

EXPORT_F32_FN_F32(acosf)
EXPORT_F32_FN_F32(acoshf)
EXPORT_F32_FN_F32(asinf)
EXPORT_F32_FN_F32(asinhf)
EXPORT_F32_FN_F32(atanf)
EXPORT_F32_FN_F32_F32(atan2f)
EXPORT_F32_FN_F32(atanhf)
EXPORT_F32_FN_F32(cbrtf)
EXPORT_F32_FN_F32(ceilf)
EXPORT_F32_FN_F32_F32(copysignf)
EXPORT_F32_FN_F32(cosf)
EXPORT_F32_FN_F32(coshf)
EXPORT_F32_FN_F32(erfcf)
EXPORT_F32_FN_F32(erff)
EXPORT_F32_FN_F32(expf)
EXPORT_F32_FN_F32(exp2f)
EXPORT_F32_FN_F32(expm1f)
EXPORT_F32_FN_F32_F32(fdimf)
EXPORT_F32_FN_F32(floorf)
float SC_fmaf(float u, float t, float v) {return fmaf(u, t, v);}
EXPORT_F32_FN_F32_F32(fmaxf)
EXPORT_F32_FN_F32_F32(fminf)
EXPORT_F32_FN_F32_F32(fmodf)
float SC_frexpf(float v, int* ptr) {return frexpf(v, ptr);}
EXPORT_F32_FN_F32_F32(hypotf)
int SC_ilogbf(float v) {return ilogbf(v); }
float SC_ldexpf(float v, int i) {return ldexpf(v, i);}
EXPORT_F32_FN_F32(lgammaf)
float SC_lgammaf_r(float v, int* ptr) {return lgammaf_r(v, ptr);}
EXPORT_F32_FN_F32(logf)
EXPORT_F32_FN_F32(log10f)
EXPORT_F32_FN_F32(log1pf)
EXPORT_F32_FN_F32(logbf)
float SC_modff(float v, float* ptr) {return modff(v, ptr);}
EXPORT_F32_FN_F32_F32(nextafterf)
EXPORT_F32_FN_F32_F32(powf)
EXPORT_F32_FN_F32_F32(remainderf)
float SC_remquof(float t, float v, int* ptr) {return remquof(t, v, ptr);}
EXPORT_F32_FN_F32(rintf)
EXPORT_F32_FN_F32(roundf)
EXPORT_F32_FN_F32(sinf)
EXPORT_F32_FN_F32(sinhf)
EXPORT_F32_FN_F32(sqrtf)
EXPORT_F32_FN_F32(tanf)
EXPORT_F32_FN_F32(tanhf)
EXPORT_F32_FN_F32(truncf)
void __attribute__((overloadable)) rsMatrixLoadRotate(rs_matrix4x4 *m,
        float rot, float x, float y, float z) {
    SC_MatrixLoadRotate((Matrix4x4 *) m, rot, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadScale(rs_matrix4x4 *m,
        float x, float y, float z) {
    SC_MatrixLoadScale((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadTranslate(rs_matrix4x4 *m,
        float x, float y, float z) {
    SC_MatrixLoadTranslate((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixRotate(rs_matrix4x4 *m, float rot,
        float x, float y, float z) {
    SC_MatrixRotate((Matrix4x4 *) m, rot, x, y, z);
}
void __attribute__((overloadable)) rsMatrixScale(rs_matrix4x4 *m, float x,
        float y, float z) {
    SC_MatrixScale((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixTranslate(rs_matrix4x4 *m, float x,
        float y, float z) {
    SC_MatrixTranslate((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadOrtho(rs_matrix4x4 *m, float l,
        float r, float b, float t, float n, float f) {
    SC_MatrixLoadOrtho((Matrix4x4 *) m, l, r, b, t, n, f);
}
void __attribute__((overloadable)) rsMatrixLoadFrustum(rs_matrix4x4 *m,
        float l, float r, float b, float t, float n, float f) {
    SC_MatrixLoadFrustum((Matrix4x4 *) m, l, r, b, t, n, f);
}
void __attribute__((overloadable)) rsMatrixLoadPerspective(rs_matrix4x4 *m,
        float fovy, float aspect, float near, float far) {
    SC_MatrixLoadPerspective((Matrix4x4 *) m, fovy, aspect, near, far);
}
bool __attribute__((overloadable)) rsMatrixInverse(rs_matrix4x4 *m) {
    return SC_MatrixInverse_4x4((Matrix4x4 *) m);
}
bool __attribute__((overloadable)) rsMatrixInverseTranspose(rs_matrix4x4 *m) {
    return SC_MatrixInverseTranspose_4x4((Matrix4x4 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix4x4 *m) {
    SC_MatrixTranspose_4x4((Matrix4x4 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix3x3 *m) {
    SC_MatrixTranspose_3x3((Matrix3x3 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix2x2 *m) {
    SC_MatrixTranspose_2x2((Matrix2x2 *) m);
}


