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



typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef uchar uchar2 __attribute__((ext_vector_type(2)));
typedef uchar uchar3 __attribute__((ext_vector_type(3)));
typedef uchar uchar4 __attribute__((ext_vector_type(4)));
typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));
typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef long long2 __attribute__((ext_vector_type(2)));
typedef long long3 __attribute__((ext_vector_type(3)));
typedef long long4 __attribute__((ext_vector_type(4)));

enum IntrinsicEnums {
    INTRINSIC_UNDEFINED,
    INTRINSIC_CONVOLVE_3x3,
    INTRINXIC_COLORMATRIX

};

#define CVT_FUNC_2(typeout, typein)                             \
static inline typeout##2 __attribute__((const, overloadable))   \
    convert_##typeout##2(typein##2 i) {                         \
        return __builtin_convertvector(i, typeout##2);          \
    }                                                           \
static inline typeout##3 __attribute__((const, overloadable))   \
    convert_##typeout##3(typein##3 i) {                         \
        return __builtin_convertvector(i, typeout##3);          \
    }                                                           \
static inline typeout##4 __attribute__((const, overloadable))   \
    convert_##typeout##4(typein##4 i) {                         \
        return __builtin_convertvector(i, typeout##4);          \
    }
#define CVT_FUNC(type)  CVT_FUNC_2(type, uchar)     \
                        CVT_FUNC_2(type, char)      \
                        CVT_FUNC_2(type, ushort)    \
                        CVT_FUNC_2(type, short)     \
                        CVT_FUNC_2(type, uint)      \
                        CVT_FUNC_2(type, int)       \
                        CVT_FUNC_2(type, float)

CVT_FUNC(char)
CVT_FUNC(uchar)
CVT_FUNC(short)
CVT_FUNC(ushort)
CVT_FUNC(int)
CVT_FUNC(uint)
CVT_FUNC(float)


static inline int4 clamp(int4 amount, int low, int high) {
    int4 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);
    return r;
}

static inline float4 clamp(float4 amount, float low, float high) {
    float4 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);
    return r;
}

static inline int2 clamp(int2 amount, int low, int high) {
    int2 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    return r;
}

static inline float2 clamp(float2 amount, float low, float high) {
    float2 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    return r;
}

static inline int clamp(int amount, int low, int high) {
    return amount < low ? low : (amount > high ? high : amount);
}

static inline float clamp(float amount, float low, float high) {
    return amount < low ? low : (amount > high ? high : amount);
}

