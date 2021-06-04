#include "rs_core.rsh"
#include "rs_structs.h"

/* Function declarations from libRS */
extern float4 __attribute__((overloadable)) convert_float4(uchar4 c);

/* Implementation of Core Runtime */


/////////////////////////////////////////////////////
// Matrix ops
/////////////////////////////////////////////////////


extern void __attribute__((overloadable))
rsMatrixLoadIdentity(rs_matrix4x4 *m) {
    m->m[0] = 1.f;
    m->m[1] = 0.f;
    m->m[2] = 0.f;
    m->m[3] = 0.f;
    m->m[4] = 0.f;
    m->m[5] = 1.f;
    m->m[6] = 0.f;
    m->m[7] = 0.f;
    m->m[8] = 0.f;
    m->m[9] = 0.f;
    m->m[10] = 1.f;
    m->m[11] = 0.f;
    m->m[12] = 0.f;
    m->m[13] = 0.f;
    m->m[14] = 0.f;
    m->m[15] = 1.f;
}

extern void __attribute__((overloadable))
rsMatrixLoadIdentity(rs_matrix3x3 *m) {
    m->m[0] = 1.f;
    m->m[1] = 0.f;
    m->m[2] = 0.f;
    m->m[3] = 0.f;
    m->m[4] = 1.f;
    m->m[5] = 0.f;
    m->m[6] = 0.f;
    m->m[7] = 0.f;
    m->m[8] = 1.f;
}
extern void __attribute__((overloadable))
rsMatrixLoadIdentity(rs_matrix2x2 *m) {
    m->m[0] = 1.f;
    m->m[1] = 0.f;
    m->m[2] = 0.f;
    m->m[3] = 1.f;
}

extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix4x4 *m, const float *f) {
    m->m[0] = f[0];
    m->m[1] = f[1];
    m->m[2] = f[2];
    m->m[3] = f[3];
    m->m[4] = f[4];
    m->m[5] = f[5];
    m->m[6] = f[6];
    m->m[7] = f[7];
    m->m[8] = f[8];
    m->m[9] = f[9];
    m->m[10] = f[10];
    m->m[11] = f[11];
    m->m[12] = f[12];
    m->m[13] = f[13];
    m->m[14] = f[14];
    m->m[15] = f[15];
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix3x3 *m, const float *f) {
    m->m[0] = f[0];
    m->m[1] = f[1];
    m->m[2] = f[2];
    m->m[3] = f[3];
    m->m[4] = f[4];
    m->m[5] = f[5];
    m->m[6] = f[6];
    m->m[7] = f[7];
    m->m[8] = f[8];
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix2x2 *m, const float *f) {
    m->m[0] = f[0];
    m->m[1] = f[1];
    m->m[2] = f[2];
    m->m[3] = f[3];
}

extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix4x4 *m, const rs_matrix4x4 *s) {
    m->m[0] = s->m[0];
    m->m[1] = s->m[1];
    m->m[2] = s->m[2];
    m->m[3] = s->m[3];
    m->m[4] = s->m[4];
    m->m[5] = s->m[5];
    m->m[6] = s->m[6];
    m->m[7] = s->m[7];
    m->m[8] = s->m[8];
    m->m[9] = s->m[9];
    m->m[10] = s->m[10];
    m->m[11] = s->m[11];
    m->m[12] = s->m[12];
    m->m[13] = s->m[13];
    m->m[14] = s->m[14];
    m->m[15] = s->m[15];
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix4x4 *m, const rs_matrix3x3 *v) {
    m->m[0] = v->m[0];
    m->m[1] = v->m[1];
    m->m[2] = v->m[2];
    m->m[3] = 0.f;
    m->m[4] = v->m[3];
    m->m[5] = v->m[4];
    m->m[6] = v->m[5];
    m->m[7] = 0.f;
    m->m[8] = v->m[6];
    m->m[9] = v->m[7];
    m->m[10] = v->m[8];
    m->m[11] = 0.f;
    m->m[12] = 0.f;
    m->m[13] = 0.f;
    m->m[14] = 0.f;
    m->m[15] = 1.f;
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix4x4 *m, const rs_matrix2x2 *v) {
    m->m[0] = v->m[0];
    m->m[1] = v->m[1];
    m->m[2] = 0.f;
    m->m[3] = 0.f;
    m->m[4] = v->m[2];
    m->m[5] = v->m[3];
    m->m[6] = 0.f;
    m->m[7] = 0.f;
    m->m[8] = 0.f;
    m->m[9] = 0.f;
    m->m[10] = 1.f;
    m->m[11] = 0.f;
    m->m[12] = 0.f;
    m->m[13] = 0.f;
    m->m[14] = 0.f;
    m->m[15] = 1.f;
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix3x3 *m, const rs_matrix3x3 *s) {
    m->m[0] = s->m[0];
    m->m[1] = s->m[1];
    m->m[2] = s->m[2];
    m->m[3] = s->m[3];
    m->m[4] = s->m[4];
    m->m[5] = s->m[5];
    m->m[6] = s->m[6];
    m->m[7] = s->m[7];
    m->m[8] = s->m[8];
}
extern void __attribute__((overloadable))
rsMatrixLoad(rs_matrix2x2 *m, const rs_matrix2x2 *s) {
    m->m[0] = s->m[0];
    m->m[1] = s->m[1];
    m->m[2] = s->m[2];
    m->m[3] = s->m[3];
}


extern void __attribute__((overloadable))
rsMatrixSet(rs_matrix4x4 *m, uint32_t col, uint32_t row, float v) {
    m->m[col * 4 + row] = v;
}

extern float __attribute__((overloadable))
rsMatrixGet(const rs_matrix4x4 *m, uint32_t col, uint32_t row) {
    return m->m[col * 4 + row];
}

extern void __attribute__((overloadable))
rsMatrixSet(rs_matrix3x3 *m, uint32_t col, uint32_t row, float v) {
    m->m[col * 3 + row] = v;
}

extern float __attribute__((overloadable))
rsMatrixGet(const rs_matrix3x3 *m, uint32_t col, uint32_t row) {
    return m->m[col * 3 + row];
}

extern void __attribute__((overloadable))
rsMatrixSet(rs_matrix2x2 *m, uint32_t col, uint32_t row, float v) {
    m->m[col * 2 + row] = v;
}

extern float __attribute__((overloadable))
rsMatrixGet(const rs_matrix2x2 *m, uint32_t col, uint32_t row) {
    return m->m[col * 2 + row];
}

extern float2 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix2x2 *m, float2 in) {
    float2 ret;
    ret.x = (m->m[0] * in.x) + (m->m[2] * in.y);
    ret.y = (m->m[1] * in.x) + (m->m[3] * in.y);
    return ret;
}
extern float2 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix2x2 *m, float2 in) {
    return rsMatrixMultiply((const rs_matrix2x2 *)m, in);
}

extern float4 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix4x4 *m, float4 in) {
    return rsMatrixMultiply((const rs_matrix4x4 *)m, in);
}

extern float4 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix4x4 *m, float3 in) {
    return rsMatrixMultiply((const rs_matrix4x4 *)m, in);
}

extern float4 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix4x4 *m, float2 in) {
    return rsMatrixMultiply((const rs_matrix4x4 *)m, in);
}

extern float3 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix3x3 *m, float3 in) {
    return rsMatrixMultiply((const rs_matrix3x3 *)m, in);
}

extern float3 __attribute__((overloadable))
rsMatrixMultiply(rs_matrix3x3 *m, float2 in) {
    return rsMatrixMultiply((const rs_matrix3x3 *)m, in);
}

extern void __attribute__((overloadable))
rsMatrixLoadMultiply(rs_matrix4x4 *ret, const rs_matrix4x4 *lhs, const rs_matrix4x4 *rhs) {
    // Use a temporary variable to support the case where one of the inputs
    // is also the destination, e.g. rsMatrixLoadMultiply(&left, &left, &right);
    rs_matrix4x4 result;
    for (int i=0 ; i<4 ; i++) {
        float ri0 = 0;
        float ri1 = 0;
        float ri2 = 0;
        float ri3 = 0;
        for (int j=0 ; j<4 ; j++) {
            const float rhs_ij = rsMatrixGet(rhs, i, j);
            ri0 += rsMatrixGet(lhs, j, 0) * rhs_ij;
            ri1 += rsMatrixGet(lhs, j, 1) * rhs_ij;
            ri2 += rsMatrixGet(lhs, j, 2) * rhs_ij;
            ri3 += rsMatrixGet(lhs, j, 3) * rhs_ij;
        }
        rsMatrixSet(&result, i, 0, ri0);
        rsMatrixSet(&result, i, 1, ri1);
        rsMatrixSet(&result, i, 2, ri2);
        rsMatrixSet(&result, i, 3, ri3);
    }
    rsMatrixLoad(ret, &result);
}

extern void __attribute__((overloadable))
rsMatrixMultiply(rs_matrix4x4 *lhs, const rs_matrix4x4 *rhs) {
    rsMatrixLoadMultiply(lhs, lhs, rhs);
}

extern void __attribute__((overloadable))
rsMatrixLoadMultiply(rs_matrix3x3 *ret, const rs_matrix3x3 *lhs, const rs_matrix3x3 *rhs) {
    // Use a temporary variable to support the case where one of the inputs
    // is also the destination, e.g. rsMatrixLoadMultiply(&left, &left, &right);
    rs_matrix3x3 result;
    for (int i=0 ; i<3 ; i++) {
        float ri0 = 0;
        float ri1 = 0;
        float ri2 = 0;
        for (int j=0 ; j<3 ; j++) {
            const float rhs_ij = rsMatrixGet(rhs, i, j);
            ri0 += rsMatrixGet(lhs, j, 0) * rhs_ij;
            ri1 += rsMatrixGet(lhs, j, 1) * rhs_ij;
            ri2 += rsMatrixGet(lhs, j, 2) * rhs_ij;
        }
        rsMatrixSet(&result, i, 0, ri0);
        rsMatrixSet(&result, i, 1, ri1);
        rsMatrixSet(&result, i, 2, ri2);
    }
    rsMatrixLoad(ret, &result);
}

extern void __attribute__((overloadable))
rsMatrixMultiply(rs_matrix3x3 *lhs, const rs_matrix3x3 *rhs) {
    rsMatrixLoadMultiply(lhs, lhs, rhs);
}

extern void __attribute__((overloadable))
rsMatrixLoadMultiply(rs_matrix2x2 *ret, const rs_matrix2x2 *lhs, const rs_matrix2x2 *rhs) {
    // Use a temporary variable to support the case where one of the inputs
    // is also the destination, e.g. rsMatrixLoadMultiply(&left, &left, &right);
    rs_matrix2x2 result;
    for (int i=0 ; i<2 ; i++) {
        float ri0 = 0;
        float ri1 = 0;
        for (int j=0 ; j<2 ; j++) {
            const float rhs_ij = rsMatrixGet(rhs, i, j);
            ri0 += rsMatrixGet(lhs, j, 0) * rhs_ij;
            ri1 += rsMatrixGet(lhs, j, 1) * rhs_ij;
        }
        rsMatrixSet(&result, i, 0, ri0);
        rsMatrixSet(&result, i, 1, ri1);
    }
    rsMatrixLoad(ret, &result);
}

extern void __attribute__((overloadable))
rsMatrixMultiply(rs_matrix2x2 *lhs, const rs_matrix2x2 *rhs) {
    rsMatrixLoadMultiply(lhs, lhs, rhs);
}

extern void __attribute__((overloadable))
    rsExtractFrustumPlanes(const rs_matrix4x4* viewProj, float4* left, float4* right, float4* top,
                           float4* bottom, float4* near, float4* far) {
    // x y z w = a b c d in the plane equation
    left->x = viewProj->m[3] + viewProj->m[0];
    left->y = viewProj->m[7] + viewProj->m[4];
    left->z = viewProj->m[11] + viewProj->m[8];
    left->w = viewProj->m[15] + viewProj->m[12];

    right->x = viewProj->m[3] - viewProj->m[0];
    right->y = viewProj->m[7] - viewProj->m[4];
    right->z = viewProj->m[11] - viewProj->m[8];
    right->w = viewProj->m[15] - viewProj->m[12];

    top->x = viewProj->m[3] - viewProj->m[1];
    top->y = viewProj->m[7] - viewProj->m[5];
    top->z = viewProj->m[11] - viewProj->m[9];
    top->w = viewProj->m[15] - viewProj->m[13];

    bottom->x = viewProj->m[3] + viewProj->m[1];
    bottom->y = viewProj->m[7] + viewProj->m[5];
    bottom->z = viewProj->m[11] + viewProj->m[9];
    bottom->w = viewProj->m[15] + viewProj->m[13];

    near->x = viewProj->m[3] + viewProj->m[2];
    near->y = viewProj->m[7] + viewProj->m[6];
    near->z = viewProj->m[11] + viewProj->m[10];
    near->w = viewProj->m[15] + viewProj->m[14];

    far->x = viewProj->m[3] - viewProj->m[2];
    far->y = viewProj->m[7] - viewProj->m[6];
    far->z = viewProj->m[11] - viewProj->m[10];
    far->w = viewProj->m[15] - viewProj->m[14];

    float len = length(left->xyz);
    *left /= len;
    len = length(right->xyz);
    *right /= len;
    len = length(top->xyz);
    *top /= len;
    len = length(bottom->xyz);
    *bottom /= len;
    len = length(near->xyz);
    *near /= len;
    len = length(far->xyz);
    *far /= len;
}

extern bool __attribute__((overloadable))
    rsIsSphereInFrustum(float4* sphere, float4* left, float4* right, float4* top, float4* bottom,
                        float4* near, float4* far) {
    float distToCenter = dot(left->xyz, sphere->xyz) + left->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(right->xyz, sphere->xyz) + right->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(top->xyz, sphere->xyz) + top->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(bottom->xyz, sphere->xyz) + bottom->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(near->xyz, sphere->xyz) + near->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(far->xyz, sphere->xyz) + far->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    return true;
}
