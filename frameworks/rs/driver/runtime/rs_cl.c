#include "rs_core.rsh"
#include "rs_f16_util.h"

extern float2 __attribute__((overloadable)) convert_float2(int2 c);
extern float3 __attribute__((overloadable)) convert_float3(int3 c);
extern float4 __attribute__((overloadable)) convert_float4(int4 c);

extern int2 __attribute__((overloadable)) convert_int2(float2 c);
extern int3 __attribute__((overloadable)) convert_int3(float3 c);
extern int4 __attribute__((overloadable)) convert_int4(float4 c);


extern float __attribute__((overloadable)) fmin(float v, float v2);
extern float2 __attribute__((overloadable)) fmin(float2 v, float v2);
extern float3 __attribute__((overloadable)) fmin(float3 v, float v2);
extern float4 __attribute__((overloadable)) fmin(float4 v, float v2);

extern float __attribute__((overloadable)) fmax(float v, float v2);
extern float2 __attribute__((overloadable)) fmax(float2 v, float v2);
extern float3 __attribute__((overloadable)) fmax(float3 v, float v2);
extern float4 __attribute__((overloadable)) fmax(float4 v, float v2);

// Float ops, 6.11.2

#define FN_FUNC_FN(fnc)                                         \
extern float2 __attribute__((overloadable)) fnc(float2 v) { \
    float2 r;                                                   \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    return r;                                                   \
}                                                               \
extern float3 __attribute__((overloadable)) fnc(float3 v) { \
    float3 r;                                                   \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    r.z = fnc(v.z);                                             \
    return r;                                                   \
}                                                               \
extern float4 __attribute__((overloadable)) fnc(float4 v) { \
    float4 r;                                                   \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    r.z = fnc(v.z);                                             \
    r.w = fnc(v.w);                                             \
    return r;                                                   \
}

#define IN_FUNC_FN(fnc)                                         \
extern int2 __attribute__((overloadable)) fnc(float2 v) {   \
    int2 r;                                                     \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    return r;                                                   \
}                                                               \
extern int3 __attribute__((overloadable)) fnc(float3 v) {   \
    int3 r;                                                     \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    r.z = fnc(v.z);                                             \
    return r;                                                   \
}                                                               \
extern int4 __attribute__((overloadable)) fnc(float4 v) {   \
    int4 r;                                                     \
    r.x = fnc(v.x);                                             \
    r.y = fnc(v.y);                                             \
    r.z = fnc(v.z);                                             \
    r.w = fnc(v.w);                                             \
    return r;                                                   \
}

#define FN_FUNC_FN_FN(fnc)                                                  \
extern float2 __attribute__((overloadable)) fnc(float2 v1, float2 v2) { \
    float2 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    return r;                                                               \
}                                                                           \
extern float3 __attribute__((overloadable)) fnc(float3 v1, float3 v2) { \
    float3 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    r.z = fnc(v1.z, v2.z);                                                  \
    return r;                                                               \
}                                                                           \
extern float4 __attribute__((overloadable)) fnc(float4 v1, float4 v2) { \
    float4 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    r.z = fnc(v1.z, v2.z);                                                  \
    r.w = fnc(v1.w, v2.w);                                                  \
    return r;                                                               \
}

#define FN_FUNC_FN_F(fnc)                                                   \
extern float2 __attribute__((overloadable)) fnc(float2 v1, float v2) {  \
    float2 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    return r;                                                               \
}                                                                           \
extern float3 __attribute__((overloadable)) fnc(float3 v1, float v2) {  \
    float3 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    r.z = fnc(v1.z, v2);                                                    \
    return r;                                                               \
}                                                                           \
extern float4 __attribute__((overloadable)) fnc(float4 v1, float v2) {  \
    float4 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    r.z = fnc(v1.z, v2);                                                    \
    r.w = fnc(v1.w, v2);                                                    \
    return r;                                                               \
}

#define FN_FUNC_FN_IN(fnc)                                                  \
extern float2 __attribute__((overloadable)) fnc(float2 v1, int2 v2) {   \
    float2 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    return r;                                                               \
}                                                                           \
extern float3 __attribute__((overloadable)) fnc(float3 v1, int3 v2) {   \
    float3 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    r.z = fnc(v1.z, v2.z);                                                  \
    return r;                                                               \
}                                                                           \
extern float4 __attribute__((overloadable)) fnc(float4 v1, int4 v2) {   \
    float4 r;                                                               \
    r.x = fnc(v1.x, v2.x);                                                  \
    r.y = fnc(v1.y, v2.y);                                                  \
    r.z = fnc(v1.z, v2.z);                                                  \
    r.w = fnc(v1.w, v2.w);                                                  \
    return r;                                                               \
}

#define FN_FUNC_FN_I(fnc)                                                   \
extern float2 __attribute__((overloadable)) fnc(float2 v1, int v2) {    \
    float2 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    return r;                                                               \
}                                                                           \
extern float3 __attribute__((overloadable)) fnc(float3 v1, int v2) {    \
    float3 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    r.z = fnc(v1.z, v2);                                                    \
    return r;                                                               \
}                                                                           \
extern float4 __attribute__((overloadable)) fnc(float4 v1, int v2) {    \
    float4 r;                                                               \
    r.x = fnc(v1.x, v2);                                                    \
    r.y = fnc(v1.y, v2);                                                    \
    r.z = fnc(v1.z, v2);                                                    \
    r.w = fnc(v1.w, v2);                                                    \
    return r;                                                               \
}

#define FN_FUNC_FN_PFN(fnc)                     \
extern float2 __attribute__((overloadable)) \
        fnc(float2 v1, float2 *v2) {            \
    float2 r;                                   \
    float t[2];                                 \
    r.x = fnc(v1.x, &t[0]);                     \
    r.y = fnc(v1.y, &t[1]);                     \
    v2->x = t[0];                               \
    v2->y = t[1];                               \
    return r;                                   \
}                                               \
extern float3 __attribute__((overloadable)) \
        fnc(float3 v1, float3 *v2) {            \
    float3 r;                                   \
    float t[3];                                 \
    r.x = fnc(v1.x, &t[0]);                     \
    r.y = fnc(v1.y, &t[1]);                     \
    r.z = fnc(v1.z, &t[2]);                     \
    v2->x = t[0];                               \
    v2->y = t[1];                               \
    v2->z = t[2];                               \
    return r;                                   \
}                                               \
extern float4 __attribute__((overloadable)) \
        fnc(float4 v1, float4 *v2) {            \
    float4 r;                                   \
    float t[4];                                 \
    r.x = fnc(v1.x, &t[0]);                     \
    r.y = fnc(v1.y, &t[1]);                     \
    r.z = fnc(v1.z, &t[2]);                     \
    r.w = fnc(v1.w, &t[3]);                     \
    v2->x = t[0];                               \
    v2->y = t[1];                               \
    v2->z = t[2];                               \
    v2->w = t[3];                               \
    return r;                                   \
}

#define FN_FUNC_FN_PIN(fnc)                                                 \
extern float2 __attribute__((overloadable)) fnc(float2 v1, int2 *v2) {  \
    float2 r;                                                               \
    int t[2];                                                               \
    r.x = fnc(v1.x, &t[0]);                                                 \
    r.y = fnc(v1.y, &t[1]);                                                 \
    v2->x = t[0];                                                           \
    v2->y = t[1];                                                           \
    return r;                                                               \
}                                                                           \
extern float3 __attribute__((overloadable)) fnc(float3 v1, int3 *v2) {  \
    float3 r;                                                               \
    int t[3];                                                               \
    r.x = fnc(v1.x, &t[0]);                                                 \
    r.y = fnc(v1.y, &t[1]);                                                 \
    r.z = fnc(v1.z, &t[2]);                                                 \
    v2->x = t[0];                                                           \
    v2->y = t[1];                                                           \
    v2->z = t[2];                                                           \
    return r;                                                               \
}                                                                           \
extern float4 __attribute__((overloadable)) fnc(float4 v1, int4 *v2) {  \
    float4 r;                                                               \
    int t[4];                                                               \
    r.x = fnc(v1.x, &t[0]);                                                 \
    r.y = fnc(v1.y, &t[1]);                                                 \
    r.z = fnc(v1.z, &t[2]);                                                 \
    r.w = fnc(v1.w, &t[3]);                                                 \
    v2->x = t[0];                                                           \
    v2->y = t[1];                                                           \
    v2->z = t[2];                                                           \
    v2->w = t[3];                                                           \
    return r;                                                               \
}

#define FN_FUNC_FN_FN_FN(fnc)                   \
extern float2 __attribute__((overloadable)) \
        fnc(float2 v1, float2 v2, float2 v3) {  \
    float2 r;                                   \
    r.x = fnc(v1.x, v2.x, v3.x);                \
    r.y = fnc(v1.y, v2.y, v3.y);                \
    return r;                                   \
}                                               \
extern float3 __attribute__((overloadable)) \
        fnc(float3 v1, float3 v2, float3 v3) {  \
    float3 r;                                   \
    r.x = fnc(v1.x, v2.x, v3.x);                \
    r.y = fnc(v1.y, v2.y, v3.y);                \
    r.z = fnc(v1.z, v2.z, v3.z);                \
    return r;                                   \
}                                               \
extern float4 __attribute__((overloadable)) \
        fnc(float4 v1, float4 v2, float4 v3) {  \
    float4 r;                                   \
    r.x = fnc(v1.x, v2.x, v3.x);                \
    r.y = fnc(v1.y, v2.y, v3.y);                \
    r.z = fnc(v1.z, v2.z, v3.z);                \
    r.w = fnc(v1.w, v2.w, v3.w);                \
    return r;                                   \
}

#define FN_FUNC_FN_FN_PIN(fnc)                  \
extern float2 __attribute__((overloadable)) \
        fnc(float2 v1, float2 v2, int2 *v3) {   \
    float2 r;                                   \
    int t[2];                                   \
    r.x = fnc(v1.x, v2.x, &t[0]);               \
    r.y = fnc(v1.y, v2.y, &t[1]);               \
    v3->x = t[0];                               \
    v3->y = t[1];                               \
    return r;                                   \
}                                               \
extern float3 __attribute__((overloadable)) \
        fnc(float3 v1, float3 v2, int3 *v3) {   \
    float3 r;                                   \
    int t[3];                                   \
    r.x = fnc(v1.x, v2.x, &t[0]);               \
    r.y = fnc(v1.y, v2.y, &t[1]);               \
    r.z = fnc(v1.z, v2.z, &t[2]);               \
    v3->x = t[0];                               \
    v3->y = t[1];                               \
    v3->z = t[2];                               \
    return r;                                   \
}                                               \
extern float4 __attribute__((overloadable)) \
        fnc(float4 v1, float4 v2, int4 *v3) {   \
    float4 r;                                   \
    int t[4];                                   \
    r.x = fnc(v1.x, v2.x, &t[0]);               \
    r.y = fnc(v1.y, v2.y, &t[1]);               \
    r.z = fnc(v1.z, v2.z, &t[2]);               \
    r.w = fnc(v1.w, v2.w, &t[3]);               \
    v3->x = t[0];                               \
    v3->y = t[1];                               \
    v3->z = t[2];                               \
    v3->w = t[3];                               \
    return r;                                   \
}

static const unsigned int iposinf = 0x7f800000;
static const unsigned int ineginf = 0xff800000;

static float posinf() {
    float f = *((float*)&iposinf);
    return f;
}

static unsigned int float_bits(float f) {
    /* TODO(jeanluc) Use this better approach once the Mac(SDK) build issues are fixed.
    // Get the bits while following the strict aliasing rules.
    unsigned int result;
    memcpy(&result, &f, sizeof(f));
    return result;
    */
    return *(unsigned int*)(char*)(&f);
}

static bool isinf(float f) {
    unsigned int i = float_bits(f);
    return (i == iposinf) || (i == ineginf);
}

static bool isnan(float f) {
    unsigned int i = float_bits(f);
    return (((i & 0x7f800000) == 0x7f800000) && (i & 0x007fffff));
}

static bool isposzero(float f) {
    return (float_bits(f) == 0x00000000);
}

static bool isnegzero(float f) {
    return (float_bits(f) == 0x80000000);
}

static bool iszero(float f) {
    return isposzero(f) || isnegzero(f);
}


extern float __attribute__((overloadable)) SC_acosf(float);
float __attribute__((overloadable)) acos(float v) {
    return SC_acosf(v);
}
FN_FUNC_FN(acos)

extern float __attribute__((overloadable)) SC_acoshf(float);
float __attribute__((overloadable)) acosh(float v) {
    return SC_acoshf(v);
}
FN_FUNC_FN(acosh)


extern float __attribute__((overloadable)) acospi(float v) {
    return acos(v) / M_PI;
}
FN_FUNC_FN(acospi)

extern float __attribute__((overloadable)) SC_asinf(float);
float __attribute__((overloadable)) asin(float v) {
    return SC_asinf(v);
}
FN_FUNC_FN(asin)

extern float __attribute__((overloadable)) SC_asinhf(float);
float __attribute__((overloadable)) asinh(float v) {
    return SC_asinhf(v);
}
FN_FUNC_FN(asinh)

extern float __attribute__((overloadable)) asinpi(float v) {
    return asin(v) / M_PI;
}
FN_FUNC_FN(asinpi)

extern float __attribute__((overloadable)) SC_atanf(float);
float __attribute__((overloadable)) atan(float v) {
    return SC_atanf(v);
}
FN_FUNC_FN(atan)

extern float __attribute__((overloadable)) SC_atan2f(float, float);
float __attribute__((overloadable)) atan2(float v1, float v2) {
    return SC_atan2f(v1, v2);
}
FN_FUNC_FN_FN(atan2)

extern float __attribute__((overloadable)) SC_atanhf(float);
float __attribute__((overloadable)) atanh(float v) {
    return SC_atanhf(v);
}
FN_FUNC_FN(atanh)

extern float __attribute__((overloadable)) atanpi(float v) {
    return atan(v) / M_PI;
}
FN_FUNC_FN(atanpi)


extern float __attribute__((overloadable)) atan2pi(float y, float x) {
    return atan2(y, x) / M_PI;
}
FN_FUNC_FN_FN(atan2pi)

extern float __attribute__((overloadable)) SC_cbrtf(float);
float __attribute__((overloadable)) cbrt(float v) {
    return SC_cbrtf(v);
}
FN_FUNC_FN(cbrt)

extern float __attribute__((overloadable)) SC_ceilf(float);
float __attribute__((overloadable)) ceil(float v) {
    return SC_ceilf(v);
}
FN_FUNC_FN(ceil)

extern float __attribute__((overloadable)) SC_copysignf(float, float);
float __attribute__((overloadable)) copysign(float v1, float v2) {
    return SC_copysignf(v1, v2);
}
FN_FUNC_FN_FN(copysign)

extern float __attribute__((overloadable)) SC_cosf(float);
float __attribute__((overloadable)) cos(float v) {
    return SC_cosf(v);
}
FN_FUNC_FN(cos)

extern float __attribute__((overloadable)) SC_coshf(float);
float __attribute__((overloadable)) cosh(float v) {
    return SC_coshf(v);
}
FN_FUNC_FN(cosh)

extern float __attribute__((overloadable)) cospi(float v) {
    return cos(v * M_PI);
}
FN_FUNC_FN(cospi)

extern float __attribute__((overloadable)) SC_erfcf(float);
float __attribute__((overloadable)) erfc(float v) {
    return SC_erfcf(v);
}
FN_FUNC_FN(erfc)

extern float __attribute__((overloadable)) SC_erff(float);
float __attribute__((overloadable)) erf(float v) {
    return SC_erff(v);
}
FN_FUNC_FN(erf)

extern float __attribute__((overloadable)) SC_expf(float);
float __attribute__((overloadable)) exp(float v) {
    return SC_expf(v);
}
FN_FUNC_FN(exp)

extern float __attribute__((overloadable)) SC_exp2f(float);
float __attribute__((overloadable)) exp2(float v) {
    return SC_exp2f(v);
}
FN_FUNC_FN(exp2)

extern float __attribute__((overloadable)) pow(float, float);

extern float __attribute__((overloadable)) exp10(float v) {
    return exp2(v * 3.321928095f);
}
FN_FUNC_FN(exp10)

extern float __attribute__((overloadable)) SC_expm1f(float);
float __attribute__((overloadable)) expm1(float v) {
    return SC_expm1f(v);
}
FN_FUNC_FN(expm1)

extern float __attribute__((overloadable)) fabs(float v) {
    int i = *((int*)(void*)&v) & 0x7fffffff;
    return  *((float*)(void*)&i);
}
FN_FUNC_FN(fabs)

extern float __attribute__((overloadable)) SC_fdimf(float, float);
float __attribute__((overloadable)) fdim(float v1, float v2) {
    return SC_fdimf(v1, v2);
}
FN_FUNC_FN_FN(fdim)

extern float __attribute__((overloadable)) SC_floorf(float);
float __attribute__((overloadable)) floor(float v) {
    return SC_floorf(v);
}
FN_FUNC_FN(floor)

extern float __attribute__((overloadable)) SC_fmaf(float, float, float);
float __attribute__((overloadable)) fma(float v1, float v2, float v3) {
    return SC_fmaf(v1, v2, v3);
}
FN_FUNC_FN_FN_FN(fma)

extern float __attribute__((overloadable)) SC_fminf(float, float);

extern float __attribute__((overloadable)) SC_fmodf(float, float);
float __attribute__((overloadable)) fmod(float v1, float v2) {
    return SC_fmodf(v1, v2);
}
FN_FUNC_FN_FN(fmod)

extern float __attribute__((overloadable)) fract(float v, float *iptr) {
    int i = (int)floor(v);
    if (iptr) {
        iptr[0] = i;
    }
    return fmin(v - i, 0x1.fffffep-1f);
}
FN_FUNC_FN_PFN(fract)

extern float __attribute__((const, overloadable)) fract(float v) {
    float unused;
    return fract(v, &unused);
}
FN_FUNC_FN(fract)

extern float __attribute__((overloadable)) SC_frexpf(float, int *);
float __attribute__((overloadable)) frexp(float v1, int* v2) {
    return SC_frexpf(v1, v2);
}
FN_FUNC_FN_PIN(frexp)

extern float __attribute__((overloadable)) SC_hypotf(float, float);
float __attribute__((overloadable)) hypot(float v1, float v2) {
    return SC_hypotf(v1, v2);
}
FN_FUNC_FN_FN(hypot)

extern int __attribute__((overloadable)) SC_ilogbf(float);
int __attribute__((overloadable)) ilogb(float v) {
    return SC_ilogbf(v);
}
IN_FUNC_FN(ilogb)

extern float __attribute__((overloadable)) SC_ldexpf(float, int);
float __attribute__((overloadable)) ldexp(float v1, int v2) {
    return SC_ldexpf(v1, v2);
}
FN_FUNC_FN_IN(ldexp)
FN_FUNC_FN_I(ldexp)

extern float __attribute__((overloadable)) SC_lgammaf(float);
float __attribute__((overloadable)) lgamma(float v) {
    return SC_lgammaf(v);
}
FN_FUNC_FN(lgamma)
extern float __attribute__((overloadable)) SC_lgammaf_r(float, int*);
float __attribute__((overloadable)) lgamma(float v, int* ptr) {
    return SC_lgammaf_r(v, ptr);
}
FN_FUNC_FN_PIN(lgamma)

extern float __attribute__((overloadable)) SC_logf(float);
float __attribute__((overloadable)) log(float v) {
    return SC_logf(v);
}
FN_FUNC_FN(log)

extern float __attribute__((overloadable)) SC_log10f(float);
float __attribute__((overloadable)) log10(float v) {
    return SC_log10f(v);
}
FN_FUNC_FN(log10)


extern float __attribute__((overloadable)) log2(float v) {
    return log10(v) * 3.321928095f;
}
FN_FUNC_FN(log2)

extern float __attribute__((overloadable)) SC_log1pf(float);
float __attribute__((overloadable)) log1p(float v) {
    return SC_log1pf(v);
}
FN_FUNC_FN(log1p)

extern float __attribute__((overloadable)) SC_logbf(float);
float __attribute__((overloadable)) logb(float v) {
    return SC_logbf(v);
}
FN_FUNC_FN(logb)

extern float __attribute__((overloadable)) mad(float a, float b, float c) {
    return a * b + c;
}
extern float2 __attribute__((overloadable)) mad(float2 a, float2 b, float2 c) {
    return a * b + c;
}
extern float3 __attribute__((overloadable)) mad(float3 a, float3 b, float3 c) {
    return a * b + c;
}
extern float4 __attribute__((overloadable)) mad(float4 a, float4 b, float4 c) {
    return a * b + c;
}

extern float __attribute__((overloadable)) SC_modff(float, float *);
float __attribute__((overloadable)) modf(float v1, float *v2) {
    return SC_modff(v1, v2);
}
FN_FUNC_FN_PFN(modf);

extern float __attribute__((overloadable)) nan(uint v) {
    float f[1];
    uint32_t *ip = (uint32_t *)f;
    *ip = v | 0x7fc00000;
    return f[0];
}

extern float __attribute__((overloadable)) SC_nextafterf(float, float);
float __attribute__((overloadable)) nextafter(float v1, float v2) {
    return SC_nextafterf(v1, v2);
}
FN_FUNC_FN_FN(nextafter)

// This function must be defined here if we're compiling with debug info
// (libclcore_g.bc), because we need a C source to get debug information.
// Otherwise the implementation can be found in IR.
#if defined(RS_G_RUNTIME)
extern float __attribute__((overloadable)) SC_powf(float, float);
float __attribute__((overloadable)) pow(float v1, float v2) {
    return SC_powf(v1, v2);
}
#endif // defined(RS_G_RUNTIME)
FN_FUNC_FN_FN(pow)

extern float __attribute__((overloadable)) pown(float v, int p) {
    /* The mantissa of a float has fewer bits than an int (24 effective vs. 31).
     * For very large ints, we'll lose whether the exponent is even or odd, making
     * the selection of a correct sign incorrect.  We correct this.  Use copysign
     * to handle the negative zero case.
     */
    float sign = (p & 0x1) ? copysign(1.f, v) : 1.f;
    float f = pow(v, (float)p);
    return copysign(f, sign);
}
FN_FUNC_FN_IN(pown)

extern float __attribute__((overloadable)) powr(float v, float p) {
    return pow(v, p);
}
extern float2 __attribute__((overloadable)) powr(float2 v, float2 p) {
    return pow(v, p);
}
extern float3 __attribute__((overloadable)) powr(float3 v, float3 p) {
    return pow(v, p);
}
extern float4 __attribute__((overloadable)) powr(float4 v, float4 p) {
    return pow(v, p);
}

extern float __attribute__((overloadable)) SC_remainderf(float, float);
float __attribute__((overloadable)) remainder(float v1, float v2) {
    return SC_remainderf(v1, v2);
}
FN_FUNC_FN_FN(remainder)

extern float __attribute__((overloadable)) SC_remquof(float, float, int *);
float __attribute__((overloadable)) remquo(float v1, float v2, int *v3) {
    return SC_remquof(v1, v2, v3);
}
FN_FUNC_FN_FN_PIN(remquo)

extern float __attribute__((overloadable)) SC_rintf(float);
float __attribute__((overloadable)) rint(float v) {
    return SC_rintf(v);
}
FN_FUNC_FN(rint)

extern float __attribute__((overloadable)) rootn(float v, int r) {
    if (r == 0) {
        return posinf();
    }

    if (iszero(v)) {
        if (r < 0) {
            if (r & 1) {
                return copysign(posinf(), v);
            } else {
                return posinf();
            }
        } else {
            if (r & 1) {
                return copysign(0.f, v);
            } else {
                return 0.f;
            }
        }
    }

    if (!isinf(v) && !isnan(v) && (v < 0.f)) {
        if (r & 1) {
            return (-1.f * pow(-1.f * v, 1.f / r));
        } else {
            return nan(0);
        }
    }

    return pow(v, 1.f / r);
}
FN_FUNC_FN_IN(rootn);

extern float __attribute__((overloadable)) SC_roundf(float);
float __attribute__((overloadable)) round(float v) {
    return SC_roundf(v);
}
FN_FUNC_FN(round)

extern float __attribute__((overloadable)) SC_randf2(float, float);
float __attribute__((overloadable)) rsRand(float min, float max) {
  return SC_randf2(min, max);
}


extern float __attribute__((overloadable)) rsqrt(float v) {
    return 1.f / sqrt(v);
}

#if !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)
// These functions must be defined here if we are not using the SSE
// implementation, which includes when we are built as part of the
// debug runtime (libclcore_debug.bc) or compiling with debug info.
#if defined(RS_G_RUNTIME)
extern float __attribute__((overloadable)) SC_sqrtf(float);
float __attribute__((overloadable)) sqrt(float v) {
    return SC_sqrtf(v);
}
#endif // defined(RS_G_RUNTIME)

FN_FUNC_FN(sqrt)
#else
extern float2 __attribute__((overloadable)) sqrt(float2);
extern float3 __attribute__((overloadable)) sqrt(float3);
extern float4 __attribute__((overloadable)) sqrt(float4);
#endif // !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)

FN_FUNC_FN(rsqrt)

extern float __attribute__((overloadable)) SC_sinf(float);
float __attribute__((overloadable)) sin(float v) {
    return SC_sinf(v);
}
FN_FUNC_FN(sin)

extern float __attribute__((overloadable)) sincos(float v, float *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
extern float2 __attribute__((overloadable)) sincos(float2 v, float2 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
extern float3 __attribute__((overloadable)) sincos(float3 v, float3 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
extern float4 __attribute__((overloadable)) sincos(float4 v, float4 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}

extern float __attribute__((overloadable)) SC_sinhf(float);
float __attribute__((overloadable)) sinh(float v) {
    return SC_sinhf(v);
}
FN_FUNC_FN(sinh)

extern float __attribute__((overloadable)) sinpi(float v) {
    return sin(v * M_PI);
}
FN_FUNC_FN(sinpi)

extern float __attribute__((overloadable)) SC_tanf(float);
float __attribute__((overloadable)) tan(float v) {
    return SC_tanf(v);
}
FN_FUNC_FN(tan)

extern float __attribute__((overloadable)) SC_tanhf(float);
float __attribute__((overloadable)) tanh(float v) {
    return SC_tanhf(v);
}
FN_FUNC_FN(tanh)

extern float __attribute__((overloadable)) tanpi(float v) {
    return tan(v * M_PI);
}
FN_FUNC_FN(tanpi)


extern float __attribute__((overloadable)) SC_tgammaf(float);
float __attribute__((overloadable)) tgamma(float v) {
    return SC_tgammaf(v);
}
FN_FUNC_FN(tgamma)

extern float __attribute__((overloadable)) SC_truncf(float);
float __attribute__((overloadable)) trunc(float v) {
    return SC_truncf(v);
}
FN_FUNC_FN(trunc)

// Int ops (partial), 6.11.3

#define XN_FUNC_YN(typeout, fnc, typein)                                \
extern typeout __attribute__((overloadable)) fnc(typein);               \
extern typeout##2 __attribute__((overloadable)) fnc(typein##2 v) {  \
    typeout##2 r;                                                       \
    r.x = fnc(v.x);                                                     \
    r.y = fnc(v.y);                                                     \
    return r;                                                           \
}                                                                       \
extern typeout##3 __attribute__((overloadable)) fnc(typein##3 v) {  \
    typeout##3 r;                                                       \
    r.x = fnc(v.x);                                                     \
    r.y = fnc(v.y);                                                     \
    r.z = fnc(v.z);                                                     \
    return r;                                                           \
}                                                                       \
extern typeout##4 __attribute__((overloadable)) fnc(typein##4 v) {  \
    typeout##4 r;                                                       \
    r.x = fnc(v.x);                                                     \
    r.y = fnc(v.y);                                                     \
    r.z = fnc(v.z);                                                     \
    r.w = fnc(v.w);                                                     \
    return r;                                                           \
}


#define UIN_FUNC_IN(fnc)          \
XN_FUNC_YN(uchar, fnc, char)      \
XN_FUNC_YN(ushort, fnc, short)    \
XN_FUNC_YN(uint, fnc, int)

#define IN_FUNC_IN(fnc)           \
XN_FUNC_YN(uchar, fnc, uchar)     \
XN_FUNC_YN(char, fnc, char)       \
XN_FUNC_YN(ushort, fnc, ushort)   \
XN_FUNC_YN(short, fnc, short)     \
XN_FUNC_YN(uint, fnc, uint)       \
XN_FUNC_YN(int, fnc, int)


#define XN_FUNC_XN_XN_BODY(type, fnc, body)         \
extern type __attribute__((overloadable))       \
        fnc(type v1, type v2) {                     \
    return body;                                    \
}                                                   \
extern type##2 __attribute__((overloadable))    \
        fnc(type##2 v1, type##2 v2) {               \
    type##2 r;                                      \
    r.x = fnc(v1.x, v2.x);                          \
    r.y = fnc(v1.y, v2.y);                          \
    return r;                                       \
}                                                   \
extern type##3 __attribute__((overloadable))    \
        fnc(type##3 v1, type##3 v2) {               \
    type##3 r;                                      \
    r.x = fnc(v1.x, v2.x);                          \
    r.y = fnc(v1.y, v2.y);                          \
    r.z = fnc(v1.z, v2.z);                          \
    return r;                                       \
}                                                   \
extern type##4 __attribute__((overloadable))    \
        fnc(type##4 v1, type##4 v2) {               \
    type##4 r;                                      \
    r.x = fnc(v1.x, v2.x);                          \
    r.y = fnc(v1.y, v2.y);                          \
    r.z = fnc(v1.z, v2.z);                          \
    r.w = fnc(v1.w, v2.w);                          \
    return r;                                       \
}

#define IN_FUNC_IN_IN_BODY(fnc, body) \
XN_FUNC_XN_XN_BODY(uchar, fnc, body)  \
XN_FUNC_XN_XN_BODY(char, fnc, body)   \
XN_FUNC_XN_XN_BODY(ushort, fnc, body) \
XN_FUNC_XN_XN_BODY(short, fnc, body)  \
XN_FUNC_XN_XN_BODY(uint, fnc, body)   \
XN_FUNC_XN_XN_BODY(int, fnc, body)    \
XN_FUNC_XN_XN_BODY(float, fnc, body)


/**
 * abs
 */
extern uint32_t __attribute__((overloadable)) abs(int32_t v) {
    if (v < 0)
        return -v;
    return v;
}
extern uint16_t __attribute__((overloadable)) abs(int16_t v) {
    if (v < 0)
        return -v;
    return v;
}
extern uint8_t __attribute__((overloadable)) abs(int8_t v) {
    if (v < 0)
        return -v;
    return v;
}

/**
 * clz
 * __builtin_clz only accepts a 32-bit unsigned int, so every input will be
 * expanded to 32 bits. For our smaller data types, we need to subtract off
 * these unused top bits (that will be always be composed of zeros).
 */
extern uint32_t __attribute__((overloadable)) clz(uint32_t v) {
    return __builtin_clz(v);
}
extern uint16_t __attribute__((overloadable)) clz(uint16_t v) {
    return __builtin_clz(v) - 16;
}
extern uint8_t __attribute__((overloadable)) clz(uint8_t v) {
    return __builtin_clz(v) - 24;
}
extern int32_t __attribute__((overloadable)) clz(int32_t v) {
    return __builtin_clz(v);
}
extern int16_t __attribute__((overloadable)) clz(int16_t v) {
    return __builtin_clz(((uint32_t)v) & 0x0000ffff) - 16;
}
extern int8_t __attribute__((overloadable)) clz(int8_t v) {
    return __builtin_clz(((uint32_t)v) & 0x000000ff) - 24;
}


UIN_FUNC_IN(abs)
IN_FUNC_IN(clz)


// 6.11.4


extern float __attribute__((overloadable)) degrees(float radians) {
    return radians * (180.f / M_PI);
}
extern float2 __attribute__((overloadable)) degrees(float2 radians) {
    return radians * (180.f / M_PI);
}
extern float3 __attribute__((overloadable)) degrees(float3 radians) {
    return radians * (180.f / M_PI);
}
extern float4 __attribute__((overloadable)) degrees(float4 radians) {
    return radians * (180.f / M_PI);
}

extern float __attribute__((overloadable)) mix(float start, float stop, float amount) {
    return start + (stop - start) * amount;
}
extern float2 __attribute__((overloadable)) mix(float2 start, float2 stop, float2 amount) {
    return start + (stop - start) * amount;
}
extern float3 __attribute__((overloadable)) mix(float3 start, float3 stop, float3 amount) {
    return start + (stop - start) * amount;
}
extern float4 __attribute__((overloadable)) mix(float4 start, float4 stop, float4 amount) {
    return start + (stop - start) * amount;
}
extern float2 __attribute__((overloadable)) mix(float2 start, float2 stop, float amount) {
    return start + (stop - start) * amount;
}
extern float3 __attribute__((overloadable)) mix(float3 start, float3 stop, float amount) {
    return start + (stop - start) * amount;
}
extern float4 __attribute__((overloadable)) mix(float4 start, float4 stop, float amount) {
    return start + (stop - start) * amount;
}

extern float __attribute__((overloadable)) radians(float degrees) {
    return degrees * (M_PI / 180.f);
}
extern float2 __attribute__((overloadable)) radians(float2 degrees) {
    return degrees * (M_PI / 180.f);
}
extern float3 __attribute__((overloadable)) radians(float3 degrees) {
    return degrees * (M_PI / 180.f);
}
extern float4 __attribute__((overloadable)) radians(float4 degrees) {
    return degrees * (M_PI / 180.f);
}

extern float __attribute__((overloadable)) step(float edge, float v) {
    return (v < edge) ? 0.f : 1.f;
}
extern float2 __attribute__((overloadable)) step(float2 edge, float2 v) {
    float2 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    return r;
}
extern float3 __attribute__((overloadable)) step(float3 edge, float3 v) {
    float3 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    r.z = (v.z < edge.z) ? 0.f : 1.f;
    return r;
}
extern float4 __attribute__((overloadable)) step(float4 edge, float4 v) {
    float4 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    r.z = (v.z < edge.z) ? 0.f : 1.f;
    r.w = (v.w < edge.w) ? 0.f : 1.f;
    return r;
}
extern float2 __attribute__((overloadable)) step(float2 edge, float v) {
    float2 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    return r;
}
extern float3 __attribute__((overloadable)) step(float3 edge, float v) {
    float3 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    r.z = (v < edge.z) ? 0.f : 1.f;
    return r;
}
extern float4 __attribute__((overloadable)) step(float4 edge, float v) {
    float4 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    r.z = (v < edge.z) ? 0.f : 1.f;
    r.w = (v < edge.w) ? 0.f : 1.f;
    return r;
}
extern float2 __attribute__((overloadable)) step(float edge, float2 v) {
    float2 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    return r;
}
extern float3 __attribute__((overloadable)) step(float edge, float3 v) {
    float3 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    r.z = (v.z < edge) ? 0.f : 1.f;
    return r;
}
extern float4 __attribute__((overloadable)) step(float edge, float4 v) {
    float4 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    r.z = (v.z < edge) ? 0.f : 1.f;
    r.w = (v.w < edge) ? 0.f : 1.f;
    return r;
}

extern float __attribute__((overloadable)) sign(float v) {
    if (v > 0) return 1.f;
    if (v < 0) return -1.f;
    return v;
}
FN_FUNC_FN(sign)


// 6.11.5
extern float3 __attribute__((overloadable)) cross(float3 lhs, float3 rhs) {
    float3 r;
    r.x = lhs.y * rhs.z  - lhs.z * rhs.y;
    r.y = lhs.z * rhs.x  - lhs.x * rhs.z;
    r.z = lhs.x * rhs.y  - lhs.y * rhs.x;
    return r;
}

extern float4 __attribute__((overloadable)) cross(float4 lhs, float4 rhs) {
    float4 r;
    r.x = lhs.y * rhs.z  - lhs.z * rhs.y;
    r.y = lhs.z * rhs.x  - lhs.x * rhs.z;
    r.z = lhs.x * rhs.y  - lhs.y * rhs.x;
    r.w = 0.f;
    return r;
}

#if !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)
// These functions must be defined here if we are not using the SSE
// implementation, which includes when we are built as part of the
// debug runtime (libclcore_debug.bc) or compiling with debug info.

extern float __attribute__((overloadable)) dot(float lhs, float rhs) {
    return lhs * rhs;
}
extern float __attribute__((overloadable)) dot(float2 lhs, float2 rhs) {
    return lhs.x*rhs.x + lhs.y*rhs.y;
}
extern float __attribute__((overloadable)) dot(float3 lhs, float3 rhs) {
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}
extern float __attribute__((overloadable)) dot(float4 lhs, float4 rhs) {
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z + lhs.w*rhs.w;
}

extern float __attribute__((overloadable)) length(float v) {
    return fabs(v);
}
extern float __attribute__((overloadable)) length(float2 v) {
    return sqrt(v.x*v.x + v.y*v.y);
}
extern float __attribute__((overloadable)) length(float3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}
extern float __attribute__((overloadable)) length(float4 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

#else

extern float __attribute__((overloadable)) length(float v);
extern float __attribute__((overloadable)) length(float2 v);
extern float __attribute__((overloadable)) length(float3 v);
extern float __attribute__((overloadable)) length(float4 v);

#endif // !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)

extern float __attribute__((overloadable)) distance(float lhs, float rhs) {
    return length(lhs - rhs);
}
extern float __attribute__((overloadable)) distance(float2 lhs, float2 rhs) {
    return length(lhs - rhs);
}
extern float __attribute__((overloadable)) distance(float3 lhs, float3 rhs) {
    return length(lhs - rhs);
}
extern float __attribute__((overloadable)) distance(float4 lhs, float4 rhs) {
    return length(lhs - rhs);
}

/* For the normalization functions, vectors of length 0 should simply be
 * returned (i.e. all the components of that vector are 0).
 */
extern float __attribute__((overloadable)) normalize(float v) {
    if (v == 0.0f) {
        return 0.0f;
    } else if (v < 0.0f) {
        return -1.0f;
    } else {
        return 1.0f;
    }
}
extern float2 __attribute__((overloadable)) normalize(float2 v) {
    float l = length(v);
    return l == 0.0f ? v : v / l;
}
extern float3 __attribute__((overloadable)) normalize(float3 v) {
    float l = length(v);
    return l == 0.0f ? v : v / l;
}
extern float4 __attribute__((overloadable)) normalize(float4 v) {
    float l = length(v);
    return l == 0.0f ? v : v / l;
}

extern float __attribute__((overloadable)) half_sqrt(float v) {
    return sqrt(v);
}
FN_FUNC_FN(half_sqrt)

extern float __attribute__((overloadable)) fast_length(float v) {
    return fabs(v);
}
extern float __attribute__((overloadable)) fast_length(float2 v) {
    return half_sqrt(v.x*v.x + v.y*v.y);
}
extern float __attribute__((overloadable)) fast_length(float3 v) {
    return half_sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}
extern float __attribute__((overloadable)) fast_length(float4 v) {
    return half_sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

extern float __attribute__((overloadable)) fast_distance(float lhs, float rhs) {
    return fast_length(lhs - rhs);
}
extern float __attribute__((overloadable)) fast_distance(float2 lhs, float2 rhs) {
    return fast_length(lhs - rhs);
}
extern float __attribute__((overloadable)) fast_distance(float3 lhs, float3 rhs) {
    return fast_length(lhs - rhs);
}
extern float __attribute__((overloadable)) fast_distance(float4 lhs, float4 rhs) {
    return fast_length(lhs - rhs);
}

extern float __attribute__((overloadable)) half_rsqrt(float);

/* For the normalization functions, vectors of length 0 should simply be
 * returned (i.e. all the components of that vector are 0).
 */
extern float __attribute__((overloadable)) fast_normalize(float v) {
    if (v == 0.0f) {
        return 0.0f;
    } else if (v < 0.0f) {
        return -1.0f;
    } else {
        return 1.0f;
    }
}
// If the length is 0, then rlength should be NaN.
extern float2 __attribute__((overloadable)) fast_normalize(float2 v) {
    float rlength = half_rsqrt(v.x*v.x + v.y*v.y);
    return (rlength == rlength) ? v * rlength : v;
}
extern float3 __attribute__((overloadable)) fast_normalize(float3 v) {
    float rlength = half_rsqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return (rlength == rlength) ? v * rlength : v;
}
extern float4 __attribute__((overloadable)) fast_normalize(float4 v) {
    float rlength = half_rsqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
    return (rlength == rlength) ? v * rlength : v;
}

extern float __attribute__((overloadable)) half_recip(float v) {
    return 1.f / v;
}

/*
extern float __attribute__((overloadable)) approx_atan(float x) {
    if (x == 0.f)
        return 0.f;
    if (x < 0.f)
        return -1.f * approx_atan(-1.f * x);
    if (x > 1.f)
        return M_PI_2 - approx_atan(approx_recip(x));
    return x * approx_recip(1.f + 0.28f * x*x);
}
FN_FUNC_FN(approx_atan)
*/

typedef union
{
  float fv;
  int32_t iv;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)                 \
do {                                \
  ieee_float_shape_type gf_u;                   \
  gf_u.fv = (d);                     \
  (i) = gf_u.iv;                      \
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)                 \
do {                                \
  ieee_float_shape_type sf_u;                   \
  sf_u.iv = (i);                      \
  (d) = sf_u.fv;                     \
} while (0)



// Valid -125 to 125
extern float __attribute__((overloadable)) native_exp2(float v) {
    int32_t iv = (int)v;
    int32_t x = iv + (iv >> 31); // ~floor(v)
    float r = (v - x);

    float fo;
    SET_FLOAT_WORD(fo, (x + 127) << 23);

    r *= 0.694f; // ~ log(e) / log(2)
    float r2 = r*r;
    float adj = 1.f + r + (r2 * 0.5f) + (r2*r * 0.166666f) + (r2*r2 * 0.0416666f);
    return fo * adj;
}

extern float2 __attribute__((overloadable)) native_exp2(float2 v) {
    int2 iv = convert_int2(v);
    int2 x = iv + (iv >> (int2)31);//floor(v);
    float2 r = (v - convert_float2(x));

    x += 127;

    float2 fo = (float2)(x << (int2)23);

    r *= 0.694f; // ~ log(e) / log(2)
    float2 r2 = r*r;
    float2 adj = 1.f + r + (r2 * 0.5f) + (r2*r * 0.166666f) + (r2*r2 * 0.0416666f);
    return fo * adj;
}

extern float4 __attribute__((overloadable)) native_exp2(float4 v) {
    int4 iv = convert_int4(v);
    int4 x = iv + (iv >> (int4)31);//floor(v);
    float4 r = (v - convert_float4(x));

    x += 127;

    float4 fo = (float4)(x << (int4)23);

    r *= 0.694f; // ~ log(e) / log(2)
    float4 r2 = r*r;
    float4 adj = 1.f + r + (r2 * 0.5f) + (r2*r * 0.166666f) + (r2*r2 * 0.0416666f);
    return fo * adj;
}

extern float3 __attribute__((overloadable)) native_exp2(float3 v) {
    float4 t = 1.f;
    t.xyz = v;
    return native_exp2(t).xyz;
}


extern float __attribute__((overloadable)) native_exp(float v) {
    return native_exp2(v * 1.442695041f);
}
extern float2 __attribute__((overloadable)) native_exp(float2 v) {
    return native_exp2(v * 1.442695041f);
}
extern float3 __attribute__((overloadable)) native_exp(float3 v) {
    return native_exp2(v * 1.442695041f);
}
extern float4 __attribute__((overloadable)) native_exp(float4 v) {
    return native_exp2(v * 1.442695041f);
}

extern float __attribute__((overloadable)) native_exp10(float v) {
    return native_exp2(v * 3.321928095f);
}
extern float2 __attribute__((overloadable)) native_exp10(float2 v) {
    return native_exp2(v * 3.321928095f);
}
extern float3 __attribute__((overloadable)) native_exp10(float3 v) {
    return native_exp2(v * 3.321928095f);
}
extern float4 __attribute__((overloadable)) native_exp10(float4 v) {
    return native_exp2(v * 3.321928095f);
}

extern float __attribute__((overloadable)) native_log2(float v) {
    int32_t ibits;
    GET_FLOAT_WORD(ibits, v);

    int32_t e = (ibits >> 23) & 0xff;

    ibits &= 0x7fffff;
    ibits |= 127 << 23;

    float ir;
    SET_FLOAT_WORD(ir, ibits);
    ir -= 1.5f;
    float ir2 = ir*ir;
    float adj2 = (0.405465108f / 0.693147181f) +
                 ((0.666666667f / 0.693147181f) * ir) -
                 ((0.222222222f / 0.693147181f) * ir2) +
                 ((0.098765432f / 0.693147181f) * ir*ir2) -
                 ((0.049382716f / 0.693147181f) * ir2*ir2) +
                 ((0.026337449f / 0.693147181f) * ir*ir2*ir2) -
                 ((0.014631916f / 0.693147181f) * ir2*ir2*ir2);
    return (float)(e - 127) + adj2;
}
extern float2 __attribute__((overloadable)) native_log2(float2 v) {
    float2 v2 = {native_log2(v.x), native_log2(v.y)};
    return v2;
}
extern float3 __attribute__((overloadable)) native_log2(float3 v) {
    float3 v2 = {native_log2(v.x), native_log2(v.y), native_log2(v.z)};
    return v2;
}
extern float4 __attribute__((overloadable)) native_log2(float4 v) {
    float4 v2 = {native_log2(v.x), native_log2(v.y), native_log2(v.z), native_log2(v.w)};
    return v2;
}

extern float __attribute__((overloadable)) native_log(float v) {
    return native_log2(v) * (1.f / 1.442695041f);
}
extern float2 __attribute__((overloadable)) native_log(float2 v) {
    return native_log2(v) * (1.f / 1.442695041f);
}
extern float3 __attribute__((overloadable)) native_log(float3 v) {
    return native_log2(v) * (1.f / 1.442695041f);
}
extern float4 __attribute__((overloadable)) native_log(float4 v) {
    return native_log2(v) * (1.f / 1.442695041f);
}

extern float __attribute__((overloadable)) native_log10(float v) {
    return native_log2(v) * (1.f / 3.321928095f);
}
extern float2 __attribute__((overloadable)) native_log10(float2 v) {
    return native_log2(v) * (1.f / 3.321928095f);
}
extern float3 __attribute__((overloadable)) native_log10(float3 v) {
    return native_log2(v) * (1.f / 3.321928095f);
}
extern float4 __attribute__((overloadable)) native_log10(float4 v) {
    return native_log2(v) * (1.f / 3.321928095f);
}


extern float __attribute__((overloadable)) native_powr(float v, float y) {
    float v2 = native_log2(v);
    v2 = fmax(v2 * y, -125.f);
    return native_exp2(v2);
}
extern float2 __attribute__((overloadable)) native_powr(float2 v, float2 y) {
    float2 v2 = native_log2(v);
    v2 = fmax(v2 * y, -125.f);
    return native_exp2(v2);
}
extern float3 __attribute__((overloadable)) native_powr(float3 v, float3 y) {
    float3 v2 = native_log2(v);
    v2 = fmax(v2 * y, -125.f);
    return native_exp2(v2);
}
extern float4 __attribute__((overloadable)) native_powr(float4 v, float4 y) {
    float4 v2 = native_log2(v);
    v2 = fmax(v2 * y, -125.f);
    return native_exp2(v2);
}

extern double __attribute__((overloadable)) min(double v1, double v2) {
    return v1 < v2 ? v1 : v2;
}

extern double2 __attribute__((overloadable)) min(double2 v1, double2 v2) {
    double2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern double3 __attribute__((overloadable)) min(double3 v1, double3 v2) {
    double3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern double4 __attribute__((overloadable)) min(double4 v1, double4 v2) {
    double4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern long __attribute__((overloadable)) min(long v1, long v2) {
    return v1 < v2 ? v1 : v2;
}
extern long2 __attribute__((overloadable)) min(long2 v1, long2 v2) {
    long2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}
extern long3 __attribute__((overloadable)) min(long3 v1, long3 v2) {
    long3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}
extern long4 __attribute__((overloadable)) min(long4 v1, long4 v2) {
    long4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern ulong __attribute__((overloadable)) min(ulong v1, ulong v2) {
    return v1 < v2 ? v1 : v2;
}
extern ulong2 __attribute__((overloadable)) min(ulong2 v1, ulong2 v2) {
    ulong2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}
extern ulong3 __attribute__((overloadable)) min(ulong3 v1, ulong3 v2) {
    ulong3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}
extern ulong4 __attribute__((overloadable)) min(ulong4 v1, ulong4 v2) {
    ulong4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern double __attribute__((overloadable)) max(double v1, double v2) {
    return v1 > v2 ? v1 : v2;
}

extern double2 __attribute__((overloadable)) max(double2 v1, double2 v2) {
    double2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern double3 __attribute__((overloadable)) max(double3 v1, double3 v2) {
    double3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern double4 __attribute__((overloadable)) max(double4 v1, double4 v2) {
    double4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern long __attribute__((overloadable)) max(long v1, long v2) {
    return v1 > v2 ? v1 : v2;
}
extern long2 __attribute__((overloadable)) max(long2 v1, long2 v2) {
    long2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}
extern long3 __attribute__((overloadable)) max(long3 v1, long3 v2) {
    long3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}
extern long4 __attribute__((overloadable)) max(long4 v1, long4 v2) {
    long4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern ulong __attribute__((overloadable)) max(ulong v1, ulong v2) {
    return v1 > v2 ? v1 : v2;
}
extern ulong2 __attribute__((overloadable)) max(ulong2 v1, ulong2 v2) {
    ulong2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}
extern ulong3 __attribute__((overloadable)) max(ulong3 v1, ulong3 v2) {
    ulong3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}
extern ulong4 __attribute__((overloadable)) max(ulong4 v1, ulong4 v2) {
    ulong4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

#define THUNK_NATIVE_F(fn) \
    float __attribute__((overloadable)) native_##fn(float v) { return fn(v);} \
    float2 __attribute__((overloadable)) native_##fn(float2 v) { return fn(v);} \
    float3 __attribute__((overloadable)) native_##fn(float3 v) { return fn(v);} \
    float4 __attribute__((overloadable)) native_##fn(float4 v) { return fn(v);}

#define THUNK_NATIVE_F_F(fn) \
    float __attribute__((overloadable)) native_##fn(float v1, float v2) { return fn(v1, v2);} \
    float2 __attribute__((overloadable)) native_##fn(float2 v1, float2 v2) { return fn(v1, v2);} \
    float3 __attribute__((overloadable)) native_##fn(float3 v1, float3 v2) { return fn(v1, v2);} \
    float4 __attribute__((overloadable)) native_##fn(float4 v1, float4 v2) { return fn(v1, v2);}

#define THUNK_NATIVE_F_FP(fn) \
    float __attribute__((overloadable)) native_##fn(float v1, float *v2) { return fn(v1, v2);} \
    float2 __attribute__((overloadable)) native_##fn(float2 v1, float2 *v2) { return fn(v1, v2);} \
    float3 __attribute__((overloadable)) native_##fn(float3 v1, float3 *v2) { return fn(v1, v2);} \
    float4 __attribute__((overloadable)) native_##fn(float4 v1, float4 *v2) { return fn(v1, v2);}

#define THUNK_NATIVE_F_I(fn) \
    float __attribute__((overloadable)) native_##fn(float v1, int v2) { return fn(v1, v2);} \
    float2 __attribute__((overloadable)) native_##fn(float2 v1, int2 v2) { return fn(v1, v2);} \
    float3 __attribute__((overloadable)) native_##fn(float3 v1, int3 v2) { return fn(v1, v2);} \
    float4 __attribute__((overloadable)) native_##fn(float4 v1, int4 v2) { return fn(v1, v2);}

THUNK_NATIVE_F(acos)
THUNK_NATIVE_F(acosh)
THUNK_NATIVE_F(acospi)
THUNK_NATIVE_F(asin)
THUNK_NATIVE_F(asinh)
THUNK_NATIVE_F(asinpi)
THUNK_NATIVE_F(atan)
THUNK_NATIVE_F_F(atan2)
THUNK_NATIVE_F(atanh)
THUNK_NATIVE_F(atanpi)
THUNK_NATIVE_F_F(atan2pi)
THUNK_NATIVE_F(cbrt)
THUNK_NATIVE_F(cos)
THUNK_NATIVE_F(cosh)
THUNK_NATIVE_F(cospi)
THUNK_NATIVE_F(expm1)
THUNK_NATIVE_F_F(hypot)
THUNK_NATIVE_F(log1p)
THUNK_NATIVE_F_I(rootn)
THUNK_NATIVE_F(rsqrt)
THUNK_NATIVE_F(sqrt)
THUNK_NATIVE_F(sin)
THUNK_NATIVE_F_FP(sincos)
THUNK_NATIVE_F(sinh)
THUNK_NATIVE_F(sinpi)
THUNK_NATIVE_F(tan)
THUNK_NATIVE_F(tanh)
THUNK_NATIVE_F(tanpi)

#undef THUNK_NATIVE_F
#undef THUNK_NATIVE_F_F
#undef THUNK_NATIVE_F_I
#undef THUNK_NATIVE_F_FP

float __attribute__((overloadable)) native_normalize(float v) { return fast_normalize(v);}
float2 __attribute__((overloadable)) native_normalize(float2 v) { return fast_normalize(v);}
float3 __attribute__((overloadable)) native_normalize(float3 v) { return fast_normalize(v);}
float4 __attribute__((overloadable)) native_normalize(float4 v) { return fast_normalize(v);}

float __attribute__((overloadable)) native_distance(float v1, float v2) { return fast_distance(v1, v2);}
float __attribute__((overloadable)) native_distance(float2 v1, float2 v2) { return fast_distance(v1, v2);}
float __attribute__((overloadable)) native_distance(float3 v1, float3 v2) { return fast_distance(v1, v2);}
float __attribute__((overloadable)) native_distance(float4 v1, float4 v2) { return fast_distance(v1, v2);}

float __attribute__((overloadable)) native_length(float v) { return fast_length(v);}
float __attribute__((overloadable)) native_length(float2 v) { return fast_length(v);}
float __attribute__((overloadable)) native_length(float3 v) { return fast_length(v);}
float __attribute__((overloadable)) native_length(float4 v) { return fast_length(v);}

float __attribute__((overloadable)) native_divide(float v1, float v2) { return v1 / v2;}
float2 __attribute__((overloadable)) native_divide(float2 v1, float2 v2) { return v1 / v2;}
float3 __attribute__((overloadable)) native_divide(float3 v1, float3 v2) { return v1 / v2;}
float4 __attribute__((overloadable)) native_divide(float4 v1, float4 v2) { return v1 / v2;}

float __attribute__((overloadable)) native_recip(float v) { return 1.f / v;}
float2 __attribute__((overloadable)) native_recip(float2 v) { return ((float2)1.f) / v;}
float3 __attribute__((overloadable)) native_recip(float3 v) { return ((float3)1.f) / v;}
float4 __attribute__((overloadable)) native_recip(float4 v) { return ((float4)1.f) / v;}





#undef FN_FUNC_FN
#undef IN_FUNC_FN
#undef FN_FUNC_FN_FN
#undef FN_FUNC_FN_F
#undef FN_FUNC_FN_IN
#undef FN_FUNC_FN_I
#undef FN_FUNC_FN_PFN
#undef FN_FUNC_FN_PIN
#undef FN_FUNC_FN_FN_FN
#undef FN_FUNC_FN_FN_PIN
#undef XN_FUNC_YN
#undef UIN_FUNC_IN
#undef IN_FUNC_IN
#undef XN_FUNC_XN_XN_BODY
#undef IN_FUNC_IN_IN_BODY

static const unsigned short kHalfPositiveInfinity = 0x7c00;

/* Define f16 functions of the form
 *     HN output = fn(HN input)
 * where HN is scalar or vector half type
 */
#define HN_FUNC_HN(fn)                                                    \
extern half __attribute__((overloadable)) fn(half h) {                    \
    return (half) fn((float) h);                                          \
}                                                                         \
extern half2 __attribute__((overloadable)) fn(half2 v) {                  \
  return convert_half2(fn(convert_float2(v)));                            \
}                                                                         \
extern half3 __attribute__((overloadable)) fn(half3 v) {                  \
  return convert_half3(fn(convert_float3(v)));                            \
}                                                                         \
extern half4 __attribute__((overloadable)) fn(half4 v) {                  \
  return convert_half4(fn(convert_float4(v)));                            \
}

/* Define f16 functions of the form
 *     HN output = fn(HN input1, HN input2)
 * where HN is scalar or vector half type
 */
#define HN_FUNC_HN_HN(fn)                                                 \
extern half __attribute__((overloadable)) fn(half h1, half h2) {          \
    return (half) fn((float) h1, (float) h2);                             \
}                                                                         \
extern half2 __attribute__((overloadable)) fn(half2 v1, half2 v2) {       \
  return convert_half2(fn(convert_float2(v1),                             \
                          convert_float2(v2)));                           \
}                                                                         \
extern half3 __attribute__((overloadable)) fn(half3 v1, half3 v2) {       \
  return convert_half3(fn(convert_float3(v1),                             \
                          convert_float3(v2)));                           \
}                                                                         \
extern half4 __attribute__((overloadable)) fn(half4 v1, half4 v2) {       \
  return convert_half4(fn(convert_float4(v1),                             \
                          convert_float4(v2)));                           \
}

/* Define f16 functions of the form
 *     HN output = fn(HN input1, half input2)
 * where HN is scalar or vector half type
 */
#define HN_FUNC_HN_H(fn)                                                  \
extern half2 __attribute__((overloadable)) fn(half2 v1, half v2) {        \
  return convert_half2(fn(convert_float2(v1), (float) v2));               \
}                                                                         \
extern half3 __attribute__((overloadable)) fn(half3 v1, half v2) {        \
  return convert_half3(fn(convert_float3(v1), (float) v2));               \
}                                                                         \
extern half4 __attribute__((overloadable)) fn(half4 v1, half v2) {        \
  return convert_half4(fn(convert_float4(v1), (float) v2));               \
}

/* Define f16 functions of the form
 *     HN output = fn(HN input1, HN input2, HN input3)
 * where HN is scalar or vector half type
 */
#define HN_FUNC_HN_HN_HN(fn)                                                   \
extern half __attribute__((overloadable)) fn(half h1, half h2, half h3) {      \
    return (half) fn((float) h1, (float) h2, (float) h3);                      \
}                                                                              \
extern half2 __attribute__((overloadable)) fn(half2 v1, half2 v2, half2 v3) {  \
  return convert_half2(fn(convert_float2(v1),                                  \
                          convert_float2(v2),                                  \
                          convert_float2(v3)));                                \
}                                                                              \
extern half3 __attribute__((overloadable)) fn(half3 v1, half3 v2, half3 v3) {  \
  return convert_half3(fn(convert_float3(v1),                                  \
                          convert_float3(v2),                                  \
                          convert_float3(v3)));                                \
}                                                                              \
extern half4 __attribute__((overloadable)) fn(half4 v1, half4 v2, half4 v3) {  \
  return convert_half4(fn(convert_float4(v1),                                  \
                          convert_float4(v2),                                  \
                          convert_float4(v3)));                                \
}

/* Define f16 functions of the form
 *     HN output = fn(HN input1, IN input2)
 * where HN is scalar or vector half type and IN the equivalent integer type
 * of same vector length.
 */
#define HN_FUNC_HN_IN(fn)                                                 \
extern half __attribute__((overloadable)) fn(half h1, int v) {            \
    return (half) fn((float) h1, v);                                      \
}                                                                         \
extern half2 __attribute__((overloadable)) fn(half2 v1, int2 v2) {        \
  return convert_half2(fn(convert_float2(v1), v2));                       \
}                                                                         \
extern half3 __attribute__((overloadable)) fn(half3 v1, int3 v2) {        \
  return convert_half3(fn(convert_float3(v1), v2));                       \
}                                                                         \
extern half4 __attribute__((overloadable)) fn(half4 v1, int4 v2) {        \
  return convert_half4(fn(convert_float4(v1), v2));                       \
}

/* Define f16 functions of the form
 *     half output = fn(HN input1)
 * where HN is a scalar or vector half type.
 */
#define H_FUNC_HN(fn)                                                     \
extern half __attribute__((overloadable)) fn(half h) {                    \
    return (half) fn((float) h);                                          \
}                                                                         \
extern half __attribute__((overloadable)) fn(half2 v) {                   \
  return fn(convert_float2(v));                                           \
}                                                                         \
extern half __attribute__((overloadable)) fn(half3 v) {                   \
  return fn(convert_float3(v));                                           \
}                                                                         \
extern half __attribute__((overloadable)) fn(half4 v) {                   \
  return fn(convert_float4(v));                                           \
}

/* Define f16 functions of the form
 *     half output = fn(HN input1, HN input2)
 * where HN is a scalar or vector half type.
 */
#define H_FUNC_HN_HN(fn)                                                  \
extern half __attribute__((overloadable)) fn(half h1, half h2) {          \
    return (half) fn((float) h1, (float) h2);                             \
}                                                                         \
extern half __attribute__((overloadable)) fn(half2 v1, half2 v2) {        \
  return fn(convert_float2(v1), convert_float2(v2));                      \
}                                                                         \
extern half __attribute__((overloadable)) fn(half3 v1, half3 v2) {        \
  return fn(convert_float3(v1), convert_float3(v2));                      \
}                                                                         \
extern half __attribute__((overloadable)) fn(half4 v1, half4 v2) {        \
  return fn(convert_float4(v1), convert_float4(v2));                      \
}

#define SCALARIZE_HN_FUNC_HN_PHN(fnc)                                 \
extern half2 __attribute__((overloadable)) fnc(half2 v1, half2 *v2) { \
    half2 ret;                                                        \
    half t[2];                                                        \
    ret.x = fnc(v1.x, &t[0]);                                         \
    ret.y = fnc(v1.y, &t[1]);                                         \
    v2->x = t[0];                                                     \
    v2->y = t[1];                                                     \
    return ret;                                                       \
}                                                                     \
extern half3 __attribute__((overloadable)) fnc(half3 v1, half3 *v2) { \
    half3 ret;                                                        \
    half t[3];                                                        \
    ret.x = fnc(v1.x, &t[0]);                                         \
    ret.y = fnc(v1.y, &t[1]);                                         \
    ret.z = fnc(v1.z, &t[2]);                                         \
    v2->x = t[0];                                                     \
    v2->y = t[1];                                                     \
    v2->z = t[2];                                                     \
    return ret;                                                       \
}                                                                     \
extern half4 __attribute__((overloadable)) fnc(half4 v1, half4 *v2) { \
    half4 ret;                                                        \
    half t[4];                                                        \
    ret.x = fnc(v1.x, &t[0]);                                         \
    ret.y = fnc(v1.y, &t[1]);                                         \
    ret.z = fnc(v1.z, &t[2]);                                         \
    ret.w = fnc(v1.w, &t[3]);                                         \
    v2->x = t[0];                                                     \
    v2->y = t[1];                                                     \
    v2->z = t[2];                                                     \
    v2->w = t[3];                                                     \
    return ret;                                                       \
}

/* Define f16 functions of the form
 *     HN output = fn(HN input1, HN input2)
 * where HN is a vector half type.  The functions are defined to call the
 * scalar function of the same name.
 */
#define SCALARIZE_HN_FUNC_HN_HN(fn)                                       \
extern half2 __attribute__((overloadable)) fn(half2 v1, half2 v2) {       \
  half2 ret;                                                              \
  ret.x = fn(v1.x, v2.x);                                                 \
  ret.y = fn(v1.y, v2.y);                                                 \
  return ret;                                                             \
}                                                                         \
extern half3 __attribute__((overloadable)) fn(half3 v1, half3 v2) {       \
  half3 ret;                                                              \
  ret.x = fn(v1.x, v2.x);                                                 \
  ret.y = fn(v1.y, v2.y);                                                 \
  ret.z = fn(v1.z, v2.z);                                                 \
  return ret;                                                             \
}                                                                         \
extern half4 __attribute__((overloadable)) fn(half4 v1, half4 v2) {       \
  half4 ret;                                                              \
  ret.x = fn(v1.x, v2.x);                                                 \
  ret.y = fn(v1.y, v2.y);                                                 \
  ret.z = fn(v1.z, v2.z);                                                 \
  ret.w = fn(v1.w, v2.w);                                                 \
  return ret;                                                             \
}                                                                         \

HN_FUNC_HN(acos);
HN_FUNC_HN(acosh);
HN_FUNC_HN(acospi);
HN_FUNC_HN(asin);
HN_FUNC_HN(asinh);
HN_FUNC_HN(asinpi);
HN_FUNC_HN(atan);
HN_FUNC_HN(atanh);
HN_FUNC_HN(atanpi);
HN_FUNC_HN_HN(atan2);
HN_FUNC_HN_HN(atan2pi);

HN_FUNC_HN(cbrt);
HN_FUNC_HN(ceil);

extern half __attribute__((overloadable)) copysign(half x, half y);
SCALARIZE_HN_FUNC_HN_HN(copysign);

HN_FUNC_HN(cos);
HN_FUNC_HN(cosh);
HN_FUNC_HN(cospi);

extern half3 __attribute__((overloadable)) cross(half3 lhs, half3 rhs) {
    half3 r;
    r.x = lhs.y * rhs.z  - lhs.z * rhs.y;
    r.y = lhs.z * rhs.x  - lhs.x * rhs.z;
    r.z = lhs.x * rhs.y  - lhs.y * rhs.x;
    return r;
}

extern half4 __attribute__((overloadable)) cross(half4 lhs, half4 rhs) {
    half4 r;
    r.x = lhs.y * rhs.z  - lhs.z * rhs.y;
    r.y = lhs.z * rhs.x  - lhs.x * rhs.z;
    r.z = lhs.x * rhs.y  - lhs.y * rhs.x;
    r.w = 0.f;
    return r;
}

HN_FUNC_HN(degrees);
H_FUNC_HN_HN(distance);
H_FUNC_HN_HN(dot);

HN_FUNC_HN(erf);
HN_FUNC_HN(erfc);
HN_FUNC_HN(exp);
HN_FUNC_HN(exp10);
HN_FUNC_HN(exp2);
HN_FUNC_HN(expm1);

HN_FUNC_HN(fabs);
HN_FUNC_HN_HN(fdim);
HN_FUNC_HN(floor);
HN_FUNC_HN_HN_HN(fma);
HN_FUNC_HN_HN(fmax);
HN_FUNC_HN_H(fmax);
HN_FUNC_HN_HN(fmin);
HN_FUNC_HN_H(fmin);
HN_FUNC_HN_HN(fmod);

extern half __attribute__((overloadable)) fract(half v, half *iptr) {
    // maxLessThanOne = 0.99951171875, the largest value < 1.0
    half maxLessThanOne;
    SET_HALF_WORD(maxLessThanOne, 0x3bff);

    int i = (int) floor(v);
    if (iptr) {
        *iptr = i;
    }
    // return v - floor(v), if strictly less than one
    return fmin(v - i, maxLessThanOne);
}

SCALARIZE_HN_FUNC_HN_PHN(fract);

extern half __attribute__((const, overloadable)) fract(half v) {
    half unused;
    return fract(v, &unused);
}

extern half2 __attribute__((const, overloadable)) fract(half2 v) {
    half2 unused;
    return fract(v, &unused);
}

extern half3 __attribute__((const, overloadable)) fract(half3 v) {
    half3 unused;
    return fract(v, &unused);
}

extern half4 __attribute__((const, overloadable)) fract(half4 v) {
    half4 unused;
    return fract(v, &unused);
}

extern half __attribute__((overloadable)) frexp(half x, int *eptr);

extern half2 __attribute__((overloadable)) frexp(half2 v1, int2 *eptr) {
    half2 ret;
    int e[2];
    ret.x = frexp(v1.x, &e[0]);
    ret.y = frexp(v1.y, &e[1]);
    eptr->x = e[0];
    eptr->y = e[1];
    return ret;
}

extern half3 __attribute__((overloadable)) frexp(half3 v1, int3 *eptr) {
    half3 ret;
    int e[3];
    ret.x = frexp(v1.x, &e[0]);
    ret.y = frexp(v1.y, &e[1]);
    ret.z = frexp(v1.z, &e[2]);
    eptr->x = e[0];
    eptr->y = e[1];
    eptr->z = e[2];
    return ret;
}

extern half4 __attribute__((overloadable)) frexp(half4 v1, int4 *eptr) {
    half4 ret;
    int e[4];
    ret.x = frexp(v1.x, &e[0]);
    ret.y = frexp(v1.y, &e[1]);
    ret.z = frexp(v1.z, &e[2]);
    ret.w = frexp(v1.w, &e[3]);
    eptr->x = e[0];
    eptr->y = e[1];
    eptr->z = e[2];
    eptr->w = e[3];
    return ret;
}

HN_FUNC_HN_HN(hypot);

extern int __attribute__((overloadable)) ilogb(half x);

extern int2 __attribute__((overloadable)) ilogb(half2 v) {
    int2 ret;
    ret.x = ilogb(v.x);
    ret.y = ilogb(v.y);
    return ret;
}
extern int3 __attribute__((overloadable)) ilogb(half3 v) {
    int3 ret;
    ret.x = ilogb(v.x);
    ret.y = ilogb(v.y);
    ret.z = ilogb(v.z);
    return ret;
}
extern int4 __attribute__((overloadable)) ilogb(half4 v) {
    int4 ret;
    ret.x = ilogb(v.x);
    ret.y = ilogb(v.y);
    ret.z = ilogb(v.z);
    ret.w = ilogb(v.w);
    return ret;
}

HN_FUNC_HN_IN(ldexp);
extern half2 __attribute__((overloadable)) ldexp(half2 v, int exponent) {
    return convert_half2(ldexp(convert_float2(v), exponent));
}
extern half3 __attribute__((overloadable)) ldexp(half3 v, int exponent) {
    return convert_half3(ldexp(convert_float3(v), exponent));
}
extern half4 __attribute__((overloadable)) ldexp(half4 v, int exponent) {
    return convert_half4(ldexp(convert_float4(v), exponent));
}

H_FUNC_HN(length);
HN_FUNC_HN(lgamma);

extern half __attribute__((overloadable)) lgamma(half h, int *signp) {
    return (half) lgamma((float) h, signp);
}
extern half2 __attribute__((overloadable)) lgamma(half2 v, int2 *signp) {
    return convert_half2(lgamma(convert_float2(v), signp));
}
extern half3 __attribute__((overloadable)) lgamma(half3 v, int3 *signp) {
    return convert_half3(lgamma(convert_float3(v), signp));
}
extern half4 __attribute__((overloadable)) lgamma(half4 v, int4 *signp) {
    return convert_half4(lgamma(convert_float4(v), signp));
}

HN_FUNC_HN(log);
HN_FUNC_HN(log10);
HN_FUNC_HN(log1p);
HN_FUNC_HN(log2);
HN_FUNC_HN(logb);

HN_FUNC_HN_HN_HN(mad);
HN_FUNC_HN_HN(max);
HN_FUNC_HN_H(max); // TODO can this be arch-specific similar to _Z3maxDv2_ff?
HN_FUNC_HN_HN(min);
HN_FUNC_HN_H(min); // TODO can this be arch-specific similar to _Z3minDv2_ff?

extern half __attribute__((overloadable)) mix(half start, half stop, half amount) {
    return start + (stop - start) * amount;
}
extern half2 __attribute__((overloadable)) mix(half2 start, half2 stop, half2 amount) {
    return start + (stop - start) * amount;
}
extern half3 __attribute__((overloadable)) mix(half3 start, half3 stop, half3 amount) {
    return start + (stop - start) * amount;
}
extern half4 __attribute__((overloadable)) mix(half4 start, half4 stop, half4 amount) {
    return start + (stop - start) * amount;
}
extern half2 __attribute__((overloadable)) mix(half2 start, half2 stop, half amount) {
    return start + (stop - start) * amount;
}
extern half3 __attribute__((overloadable)) mix(half3 start, half3 stop, half amount) {
    return start + (stop - start) * amount;
}
extern half4 __attribute__((overloadable)) mix(half4 start, half4 stop, half amount) {
    return start + (stop - start) * amount;
}

extern half __attribute__((overloadable)) modf(half x, half *iptr);
SCALARIZE_HN_FUNC_HN_PHN(modf);

half __attribute__((overloadable)) nan_half() {
  unsigned short nan_short = kHalfPositiveInfinity | 0x0200;
  half nan;
  SET_HALF_WORD(nan, nan_short);
  return nan;
}

HN_FUNC_HN(normalize);

extern half __attribute__((overloadable)) nextafter(half x, half y);
SCALARIZE_HN_FUNC_HN_HN(nextafter);

HN_FUNC_HN_HN(pow);
HN_FUNC_HN_IN(pown);
HN_FUNC_HN_HN(powr);
HN_FUNC_HN(radians);
HN_FUNC_HN_HN(remainder);

extern half __attribute__((overloadable)) remquo(half n, half d, int *quo) {
    return (float) remquo((float) n, (float) d, quo);
}
extern half2 __attribute__((overloadable)) remquo(half2 n, half2 d, int2 *quo) {
    return convert_half2(remquo(convert_float2(d), convert_float2(n), quo));
}
extern half3 __attribute__((overloadable)) remquo(half3 n, half3 d, int3 *quo) {
    return convert_half3(remquo(convert_float3(d), convert_float3(n), quo));
}
extern half4 __attribute__((overloadable)) remquo(half4 n, half4 d, int4 *quo) {
    return convert_half4(remquo(convert_float4(d), convert_float4(n), quo));
}

HN_FUNC_HN(rint);
HN_FUNC_HN_IN(rootn);
HN_FUNC_HN(round);
HN_FUNC_HN(rsqrt);

extern half __attribute__((overloadable)) sign(half h) {
    if (h > 0) return (half) 1.f;
    if (h < 0) return (half) -1.f;
    return h;
}
extern half2 __attribute__((overloadable)) sign(half2 v) {
    half2 ret;
    ret.x = sign(v.x);
    ret.y = sign(v.y);
    return ret;
}
extern half3 __attribute__((overloadable)) sign(half3 v) {
    half3 ret;
    ret.x = sign(v.x);
    ret.y = sign(v.y);
    ret.z = sign(v.z);
    return ret;
}
extern half4 __attribute__((overloadable)) sign(half4 v) {
    half4 ret;
    ret.x = sign(v.x);
    ret.y = sign(v.y);
    ret.z = sign(v.z);
    ret.w = sign(v.w);
    return ret;
}

HN_FUNC_HN(sin);

extern half __attribute__((overloadable)) sincos(half v, half *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
// TODO verify if LLVM eliminates the duplicate convert_float2
extern half2 __attribute__((overloadable)) sincos(half2 v, half2 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
extern half3 __attribute__((overloadable)) sincos(half3 v, half3 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}
extern half4 __attribute__((overloadable)) sincos(half4 v, half4 *cosptr) {
    *cosptr = cos(v);
    return sin(v);
}

HN_FUNC_HN(sinh);
HN_FUNC_HN(sinpi);
HN_FUNC_HN(sqrt);

extern half __attribute__((overloadable)) step(half edge, half v) {
    return (v < edge) ? 0.f : 1.f;
}
extern half2 __attribute__((overloadable)) step(half2 edge, half2 v) {
    half2 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    return r;
}
extern half3 __attribute__((overloadable)) step(half3 edge, half3 v) {
    half3 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    r.z = (v.z < edge.z) ? 0.f : 1.f;
    return r;
}
extern half4 __attribute__((overloadable)) step(half4 edge, half4 v) {
    half4 r;
    r.x = (v.x < edge.x) ? 0.f : 1.f;
    r.y = (v.y < edge.y) ? 0.f : 1.f;
    r.z = (v.z < edge.z) ? 0.f : 1.f;
    r.w = (v.w < edge.w) ? 0.f : 1.f;
    return r;
}
extern half2 __attribute__((overloadable)) step(half2 edge, half v) {
    half2 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    return r;
}
extern half3 __attribute__((overloadable)) step(half3 edge, half v) {
    half3 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    r.z = (v < edge.z) ? 0.f : 1.f;
    return r;
}
extern half4 __attribute__((overloadable)) step(half4 edge, half v) {
    half4 r;
    r.x = (v < edge.x) ? 0.f : 1.f;
    r.y = (v < edge.y) ? 0.f : 1.f;
    r.z = (v < edge.z) ? 0.f : 1.f;
    r.w = (v < edge.w) ? 0.f : 1.f;
    return r;
}
extern half2 __attribute__((overloadable)) step(half edge, half2 v) {
    half2 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    return r;
}
extern half3 __attribute__((overloadable)) step(half edge, half3 v) {
    half3 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    r.z = (v.z < edge) ? 0.f : 1.f;
    return r;
}
extern half4 __attribute__((overloadable)) step(half edge, half4 v) {
    half4 r;
    r.x = (v.x < edge) ? 0.f : 1.f;
    r.y = (v.y < edge) ? 0.f : 1.f;
    r.z = (v.z < edge) ? 0.f : 1.f;
    r.w = (v.w < edge) ? 0.f : 1.f;
    return r;
}

HN_FUNC_HN(tan);
HN_FUNC_HN(tanh);
HN_FUNC_HN(tanpi);
HN_FUNC_HN(tgamma);
HN_FUNC_HN(trunc); // TODO: rethink: needs half-specific implementation?

HN_FUNC_HN(native_acos);
HN_FUNC_HN(native_acosh);
HN_FUNC_HN(native_acospi);
HN_FUNC_HN(native_asin);
HN_FUNC_HN(native_asinh);
HN_FUNC_HN(native_asinpi);
HN_FUNC_HN(native_atan);
HN_FUNC_HN(native_atanh);
HN_FUNC_HN(native_atanpi);
HN_FUNC_HN_HN(native_atan2);
HN_FUNC_HN_HN(native_atan2pi);

HN_FUNC_HN(native_cbrt);
HN_FUNC_HN(native_cos);
HN_FUNC_HN(native_cosh);
HN_FUNC_HN(native_cospi);

H_FUNC_HN_HN(native_distance);
HN_FUNC_HN_HN(native_divide);

HN_FUNC_HN(native_exp);
HN_FUNC_HN(native_exp10);
HN_FUNC_HN(native_exp2);
HN_FUNC_HN(native_expm1);

HN_FUNC_HN_HN(native_hypot);
H_FUNC_HN(native_length);

HN_FUNC_HN(native_log);
HN_FUNC_HN(native_log10);
HN_FUNC_HN(native_log1p);
HN_FUNC_HN(native_log2);

HN_FUNC_HN(native_normalize);

HN_FUNC_HN_HN(native_powr); // TODO are parameter limits different for half?

HN_FUNC_HN(native_recip);
HN_FUNC_HN_IN(native_rootn);
HN_FUNC_HN(native_rsqrt);

HN_FUNC_HN(native_sin);

extern half __attribute__((overloadable)) native_sincos(half v, half *cosptr) {
    return sincos(v, cosptr);
}
extern half2 __attribute__((overloadable)) native_sincos(half2 v, half2 *cosptr) {
    return sincos(v, cosptr);
}
extern half3 __attribute__((overloadable)) native_sincos(half3 v, half3 *cosptr) {
    return sincos(v, cosptr);
}
extern half4 __attribute__((overloadable)) native_sincos(half4 v, half4 *cosptr) {
    return sincos(v, cosptr);
}

HN_FUNC_HN(native_sinh);
HN_FUNC_HN(native_sinpi);
HN_FUNC_HN(native_sqrt);

HN_FUNC_HN(native_tan);
HN_FUNC_HN(native_tanh);
HN_FUNC_HN(native_tanpi);

#undef HN_FUNC_HN
#undef HN_FUNC_HN_HN
#undef HN_FUNC_HN_H
#undef HN_FUNC_HN_HN_HN
#undef HN_FUNC_HN_IN
#undef H_FUNC_HN
#undef H_FUNC_HN_HN
#undef SCALARIZE_HN_FUNC_HN_HN

// exports unavailable mathlib functions to compat lib

#ifdef RS_COMPATIBILITY_LIB

// !!! DANGER !!!
// These functions are potentially missing on older Android versions.
// Work around the issue by supplying our own variants.
// !!! DANGER !!!

// The logbl() implementation is taken from the latest bionic/, since
// double == long double on Android.
extern "C" long double logbl(long double x) { return logb(x); }

// __aeabi_idiv0 is a missing function in libcompiler_rt.so, so we just
// pick the simplest implementation based on the ARM EABI doc.
extern "C" int __aeabi_idiv0(int v) { return v; }

#endif // compatibility lib
