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


#include "rs_core.rsh"

extern short __attribute__((overloadable, always_inline)) rsClamp(short amount, short low, short high);
extern uchar4 __attribute__((overloadable)) convert_uchar4(short4);
extern uchar4 __attribute__((overloadable)) convert_uchar4(float4);
extern float4 __attribute__((overloadable)) convert_float4(uchar4);
extern float __attribute__((overloadable)) sqrt(float);

/*
 * CLAMP
 */
#define _CLAMP(T) \
extern T __attribute__((overloadable)) clamp(T amount, T low, T high) {             \
    return amount < low ? low : (amount > high ? high : amount);                    \
}                                                                                   \
                                                                                    \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T##2 low, T##2 high) { \
    T##2 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T##3 low, T##3 high) { \
    T##3 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    r.z = amount.z < low.z ? low.z : (amount.z > high.z ? high.z : amount.z);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T##4 low, T##4 high) { \
    T##4 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    r.z = amount.z < low.z ? low.z : (amount.z > high.z ? high.z : amount.z);       \
    r.w = amount.w < low.w ? low.w : (amount.w > high.w ? high.w : amount.w);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T low, T high) {       \
    T##2 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T low, T high) {       \
    T##3 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);               \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T low, T high) {       \
    T##4 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);               \
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);               \
    return r;                                                                       \
}

#if !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)
// These functions must be defined here if we are not using the SSE
// implementation, which includes when we are built as part of the
// debug runtime (libclcore_debug.bc) or compiling with debug info.

_CLAMP(float);

#else

extern float __attribute__((overloadable)) clamp(float amount, float low, float high);
extern float2 __attribute__((overloadable)) clamp(float2 amount, float2 low, float2 high);
extern float3 __attribute__((overloadable)) clamp(float3 amount, float3 low, float3 high);
extern float4 __attribute__((overloadable)) clamp(float4 amount, float4 low, float4 high);
extern float2 __attribute__((overloadable)) clamp(float2 amount, float low, float high);
extern float3 __attribute__((overloadable)) clamp(float3 amount, float low, float high);
extern float4 __attribute__((overloadable)) clamp(float4 amount, float low, float high);

#endif // !defined(ARCH_X86_HAVE_SSSE3) || defined(RS_DEBUG_RUNTIME) || defined(RS_G_RUNTIME)

_CLAMP(half);
_CLAMP(double);
_CLAMP(char);
_CLAMP(uchar);
_CLAMP(short);
_CLAMP(ushort);
_CLAMP(int);
_CLAMP(uint);
_CLAMP(long);
_CLAMP(ulong);

#undef _CLAMP

/*
 * FMAX
 */

extern float __attribute__((overloadable)) fmax(float v1, float v2) {
    return v1 > v2 ? v1 : v2;
}

extern float2 __attribute__((overloadable)) fmax(float2 v1, float2 v2) {
    float2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern float3 __attribute__((overloadable)) fmax(float3 v1, float3 v2) {
    float3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern float4 __attribute__((overloadable)) fmax(float4 v1, float4 v2) {
    float4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern float2 __attribute__((overloadable)) fmax(float2 v1, float v2) {
    float2 r;
    r.x = v1.x > v2 ? v1.x : v2;
    r.y = v1.y > v2 ? v1.y : v2;
    return r;
}

extern float3 __attribute__((overloadable)) fmax(float3 v1, float v2) {
    float3 r;
    r.x = v1.x > v2 ? v1.x : v2;
    r.y = v1.y > v2 ? v1.y : v2;
    r.z = v1.z > v2 ? v1.z : v2;
    return r;
}

extern float4 __attribute__((overloadable)) fmax(float4 v1, float v2) {
    float4 r;
    r.x = v1.x > v2 ? v1.x : v2;
    r.y = v1.y > v2 ? v1.y : v2;
    r.z = v1.z > v2 ? v1.z : v2;
    r.w = v1.w > v2 ? v1.w : v2;
    return r;
}

extern float __attribute__((overloadable)) fmin(float v1, float v2) {
    return v1 < v2 ? v1 : v2;
}


/*
 * FMIN
 */
extern float2 __attribute__((overloadable)) fmin(float2 v1, float2 v2) {
    float2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern float3 __attribute__((overloadable)) fmin(float3 v1, float3 v2) {
    float3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern float4 __attribute__((overloadable)) fmin(float4 v1, float4 v2) {
    float4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern float2 __attribute__((overloadable)) fmin(float2 v1, float v2) {
    float2 r;
    r.x = v1.x < v2 ? v1.x : v2;
    r.y = v1.y < v2 ? v1.y : v2;
    return r;
}

extern float3 __attribute__((overloadable)) fmin(float3 v1, float v2) {
    float3 r;
    r.x = v1.x < v2 ? v1.x : v2;
    r.y = v1.y < v2 ? v1.y : v2;
    r.z = v1.z < v2 ? v1.z : v2;
    return r;
}

extern float4 __attribute__((overloadable)) fmin(float4 v1, float v2) {
    float4 r;
    r.x = v1.x < v2 ? v1.x : v2;
    r.y = v1.y < v2 ? v1.y : v2;
    r.z = v1.z < v2 ? v1.z : v2;
    r.w = v1.w < v2 ? v1.w : v2;
    return r;
}


/*
 * MAX
 */

extern char __attribute__((overloadable)) max(char v1, char v2) {
    return v1 > v2 ? v1 : v2;
}

extern char2 __attribute__((overloadable)) max(char2 v1, char2 v2) {
    char2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern char3 __attribute__((overloadable)) max(char3 v1, char3 v2) {
    char3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern char4 __attribute__((overloadable)) max(char4 v1, char4 v2) {
    char4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern short __attribute__((overloadable)) max(short v1, short v2) {
    return v1 > v2 ? v1 : v2;
}

extern short2 __attribute__((overloadable)) max(short2 v1, short2 v2) {
    short2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern short3 __attribute__((overloadable)) max(short3 v1, short3 v2) {
    short3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern short4 __attribute__((overloadable)) max(short4 v1, short4 v2) {
    short4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern int __attribute__((overloadable)) max(int v1, int v2) {
    return v1 > v2 ? v1 : v2;
}

extern int2 __attribute__((overloadable)) max(int2 v1, int2 v2) {
    int2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern int3 __attribute__((overloadable)) max(int3 v1, int3 v2) {
    int3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern int4 __attribute__((overloadable)) max(int4 v1, int4 v2) {
    int4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern uchar __attribute__((overloadable)) max(uchar v1, uchar v2) {
    return v1 > v2 ? v1 : v2;
}

extern uchar2 __attribute__((overloadable)) max(uchar2 v1, uchar2 v2) {
    uchar2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern uchar3 __attribute__((overloadable)) max(uchar3 v1, uchar3 v2) {
    uchar3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern uchar4 __attribute__((overloadable)) max(uchar4 v1, uchar4 v2) {
    uchar4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern ushort __attribute__((overloadable)) max(ushort v1, ushort v2) {
    return v1 > v2 ? v1 : v2;
}

extern ushort2 __attribute__((overloadable)) max(ushort2 v1, ushort2 v2) {
    ushort2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern ushort3 __attribute__((overloadable)) max(ushort3 v1, ushort3 v2) {
    ushort3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern ushort4 __attribute__((overloadable)) max(ushort4 v1, ushort4 v2) {
    ushort4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern uint __attribute__((overloadable)) max(uint v1, uint v2) {
    return v1 > v2 ? v1 : v2;
}

extern uint2 __attribute__((overloadable)) max(uint2 v1, uint2 v2) {
    uint2 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    return r;
}

extern uint3 __attribute__((overloadable)) max(uint3 v1, uint3 v2) {
    uint3 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    return r;
}

extern uint4 __attribute__((overloadable)) max(uint4 v1, uint4 v2) {
    uint4 r;
    r.x = v1.x > v2.x ? v1.x : v2.x;
    r.y = v1.y > v2.y ? v1.y : v2.y;
    r.z = v1.z > v2.z ? v1.z : v2.z;
    r.w = v1.w > v2.w ? v1.w : v2.w;
    return r;
}

extern float __attribute__((overloadable)) max(float v1, float v2) {
    return fmax(v1, v2);
}

extern float2 __attribute__((overloadable)) max(float2 v1, float2 v2) {
    return fmax(v1, v2);
}

extern float2 __attribute__((overloadable)) max(float2 v1, float v2) {
    return fmax(v1, v2);
}

extern float3 __attribute__((overloadable)) max(float3 v1, float3 v2) {
    return fmax(v1, v2);
}

extern float3 __attribute__((overloadable)) max(float3 v1, float v2) {
    return fmax(v1, v2);
}

extern float4 __attribute__((overloadable)) max(float4 v1, float4 v2) {
    return fmax(v1, v2);
}

extern float4 __attribute__((overloadable)) max(float4 v1, float v2) {
    return fmax(v1, v2);
}


/*
 * MIN
 */

extern int8_t __attribute__((overloadable)) min(int8_t v1, int8_t v2) {
    return v1 < v2 ? v1 : v2;
}

extern char2 __attribute__((overloadable)) min(char2 v1, char2 v2) {
    char2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern char3 __attribute__((overloadable)) min(char3 v1, char3 v2) {
    char3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern char4 __attribute__((overloadable)) min(char4 v1, char4 v2) {
    char4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern int16_t __attribute__((overloadable)) min(int16_t v1, int16_t v2) {
    return v1 < v2 ? v1 : v2;
}

extern short2 __attribute__((overloadable)) min(short2 v1, short2 v2) {
    short2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern short3 __attribute__((overloadable)) min(short3 v1, short3 v2) {
    short3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern short4 __attribute__((overloadable)) min(short4 v1, short4 v2) {
    short4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern int32_t __attribute__((overloadable)) min(int32_t v1, int32_t v2) {
    return v1 < v2 ? v1 : v2;
}

extern int2 __attribute__((overloadable)) min(int2 v1, int2 v2) {
    int2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern int3 __attribute__((overloadable)) min(int3 v1, int3 v2) {
    int3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern int4 __attribute__((overloadable)) min(int4 v1, int4 v2) {
    int4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern uchar __attribute__((overloadable)) min(uchar v1, uchar v2) {
    return v1 < v2 ? v1 : v2;
}

extern uchar2 __attribute__((overloadable)) min(uchar2 v1, uchar2 v2) {
    uchar2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern uchar3 __attribute__((overloadable)) min(uchar3 v1, uchar3 v2) {
    uchar3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern uchar4 __attribute__((overloadable)) min(uchar4 v1, uchar4 v2) {
    uchar4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern ushort __attribute__((overloadable)) min(ushort v1, ushort v2) {
    return v1 < v2 ? v1 : v2;
}

extern ushort2 __attribute__((overloadable)) min(ushort2 v1, ushort2 v2) {
    ushort2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern ushort3 __attribute__((overloadable)) min(ushort3 v1, ushort3 v2) {
    ushort3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern ushort4 __attribute__((overloadable)) min(ushort4 v1, ushort4 v2) {
    ushort4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern uint __attribute__((overloadable)) min(uint v1, uint v2) {
    return v1 < v2 ? v1 : v2;
}

extern uint2 __attribute__((overloadable)) min(uint2 v1, uint2 v2) {
    uint2 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    return r;
}

extern uint3 __attribute__((overloadable)) min(uint3 v1, uint3 v2) {
    uint3 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    return r;
}

extern uint4 __attribute__((overloadable)) min(uint4 v1, uint4 v2) {
    uint4 r;
    r.x = v1.x < v2.x ? v1.x : v2.x;
    r.y = v1.y < v2.y ? v1.y : v2.y;
    r.z = v1.z < v2.z ? v1.z : v2.z;
    r.w = v1.w < v2.w ? v1.w : v2.w;
    return r;
}

extern float __attribute__((overloadable)) min(float v1, float v2) {
    return fmin(v1, v2);
}

extern float2 __attribute__((overloadable)) min(float2 v1, float2 v2) {
    return fmin(v1, v2);
}

extern float2 __attribute__((overloadable)) min(float2 v1, float v2) {
    return fmin(v1, v2);
}

extern float3 __attribute__((overloadable)) min(float3 v1, float3 v2) {
    return fmin(v1, v2);
}

extern float3 __attribute__((overloadable)) min(float3 v1, float v2) {
    return fmin(v1, v2);
}

extern float4 __attribute__((overloadable)) min(float4 v1, float4 v2) {
    return fmin(v1, v2);
}

extern float4 __attribute__((overloadable)) min(float4 v1, float v2) {
    return fmin(v1, v2);
}

/*
 * YUV
 */

extern uchar4 __attribute__((overloadable)) rsYuvToRGBA_uchar4(uchar y, uchar u, uchar v) {
    short Y = ((short)y) - 16;
    short U = ((short)u) - 128;
    short V = ((short)v) - 128;

    short4 p;
    p.r = (Y * 298 + V * 409 + 128) >> 8;
    p.g = (Y * 298 - U * 100 - V * 208 + 128) >> 8;
    p.b = (Y * 298 + U * 516 + 128) >> 8;
    p.a = 255;
    p.r = rsClamp(p.r, (short)0, (short)255);
    p.g = rsClamp(p.g, (short)0, (short)255);
    p.b = rsClamp(p.b, (short)0, (short)255);

    return convert_uchar4(p);
}

/*
 * half_RECIP
 */

extern float2 __attribute__((overloadable)) half_recip(float2 v) {
    return ((float2) 1.f) / v;
}

extern float3 __attribute__((overloadable)) half_recip(float3 v) {
    return ((float3) 1.f) / v;
}

extern float4 __attribute__((overloadable)) half_recip(float4 v) {
    return ((float4) 1.f) / v;
}



/*
 * half_rsqrt
 */

extern float __attribute__((overloadable)) half_rsqrt(float v) {
    return 1.f / sqrt(v);
}

extern float2 __attribute__((overloadable)) half_rsqrt(float2 v) {
    float2 r;
    r.x = half_rsqrt(v.x);
    r.y = half_rsqrt(v.y);
    return r;
}

extern float3 __attribute__((overloadable)) half_rsqrt(float3 v) {
    float3 r;
    r.x = half_rsqrt(v.x);
    r.y = half_rsqrt(v.y);
    r.z = half_rsqrt(v.z);
    return r;
}

extern float4 __attribute__((overloadable)) half_rsqrt(float4 v) {
    float4 r;
    r.x = half_rsqrt(v.x);
    r.y = half_rsqrt(v.y);
    r.z = half_rsqrt(v.z);
    r.w = half_rsqrt(v.w);
    return r;
}

/**
 * matrix ops
 */

extern float4 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix4x4 *m, float4 in) {
    float4 ret;
    ret.x = (m->m[0] * in.x) + (m->m[4] * in.y) + (m->m[8] * in.z) + (m->m[12] * in.w);
    ret.y = (m->m[1] * in.x) + (m->m[5] * in.y) + (m->m[9] * in.z) + (m->m[13] * in.w);
    ret.z = (m->m[2] * in.x) + (m->m[6] * in.y) + (m->m[10] * in.z) + (m->m[14] * in.w);
    ret.w = (m->m[3] * in.x) + (m->m[7] * in.y) + (m->m[11] * in.z) + (m->m[15] * in.w);
    return ret;
}

extern float4 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix4x4 *m, float3 in) {
    float4 ret;
    ret.x = (m->m[0] * in.x) + (m->m[4] * in.y) + (m->m[8] * in.z) + m->m[12];
    ret.y = (m->m[1] * in.x) + (m->m[5] * in.y) + (m->m[9] * in.z) + m->m[13];
    ret.z = (m->m[2] * in.x) + (m->m[6] * in.y) + (m->m[10] * in.z) + m->m[14];
    ret.w = (m->m[3] * in.x) + (m->m[7] * in.y) + (m->m[11] * in.z) + m->m[15];
    return ret;
}

extern float4 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix4x4 *m, float2 in) {
    float4 ret;
    ret.x = (m->m[0] * in.x) + (m->m[4] * in.y) + m->m[12];
    ret.y = (m->m[1] * in.x) + (m->m[5] * in.y) + m->m[13];
    ret.z = (m->m[2] * in.x) + (m->m[6] * in.y) + m->m[14];
    ret.w = (m->m[3] * in.x) + (m->m[7] * in.y) + m->m[15];
    return ret;
}

extern float3 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix3x3 *m, float3 in) {
    float3 ret;
    ret.x = (m->m[0] * in.x) + (m->m[3] * in.y) + (m->m[6] * in.z);
    ret.y = (m->m[1] * in.x) + (m->m[4] * in.y) + (m->m[7] * in.z);
    ret.z = (m->m[2] * in.x) + (m->m[5] * in.y) + (m->m[8] * in.z);
    return ret;
}

extern float3 __attribute__((overloadable))
rsMatrixMultiply(const rs_matrix3x3 *m, float2 in) {
    float3 ret;
    ret.x = (m->m[0] * in.x) + (m->m[3] * in.y);
    ret.y = (m->m[1] * in.x) + (m->m[4] * in.y);
    ret.z = (m->m[2] * in.x) + (m->m[5] * in.y);
    return ret;
}

/**
 * Pixel Ops
 */
extern uchar4 __attribute__((overloadable)) rsPackColorTo8888(float r, float g, float b)
{
    uchar4 c;
    c.x = (uchar)clamp((r * 255.f + 0.5f), 0.f, 255.f);
    c.y = (uchar)clamp((g * 255.f + 0.5f), 0.f, 255.f);
    c.z = (uchar)clamp((b * 255.f + 0.5f), 0.f, 255.f);
    c.w = 255;
    return c;
}

extern uchar4 __attribute__((overloadable)) rsPackColorTo8888(float r, float g, float b, float a)
{
    uchar4 c;
    c.x = (uchar)clamp((r * 255.f + 0.5f), 0.f, 255.f);
    c.y = (uchar)clamp((g * 255.f + 0.5f), 0.f, 255.f);
    c.z = (uchar)clamp((b * 255.f + 0.5f), 0.f, 255.f);
    c.w = (uchar)clamp((a * 255.f + 0.5f), 0.f, 255.f);
    return c;
}

extern uchar4 __attribute__((overloadable)) rsPackColorTo8888(float3 color)
{
    color *= 255.f;
    color += 0.5f;
    color = clamp(color, 0.f, 255.f);
    uchar4 c = {color.x, color.y, color.z, 255};
    return c;
}

extern uchar4 __attribute__((overloadable)) rsPackColorTo8888(float4 color)
{
    color *= 255.f;
    color += 0.5f;
    color = clamp(color, 0.f, 255.f);
    uchar4 c = {color.x, color.y, color.z, color.w};
    return c;
}
