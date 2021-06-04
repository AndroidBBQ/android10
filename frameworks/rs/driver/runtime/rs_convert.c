/*
 * Copyright (C) 2014 The Android Open Source Project
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


#define CVT_FUNC_2(typeout, typein)                             \
extern typeout##2 __attribute__((const, overloadable))   \
    convert_##typeout##2(typein##2 i) {                         \
        return __builtin_convertvector(i, typeout##2);          \
    }                                                           \
extern typeout##3 __attribute__((const, overloadable))   \
    convert_##typeout##3(typein##3 i) {                         \
        return __builtin_convertvector(i, typeout##3);          \
    }                                                           \
extern typeout##4 __attribute__((const, overloadable))   \
    convert_##typeout##4(typein##4 i) {                         \
        return __builtin_convertvector(i, typeout##4);          \
    }
#define CVT_FUNC(type)  CVT_FUNC_2(type, uchar)     \
                        CVT_FUNC_2(type, char)      \
                        CVT_FUNC_2(type, ushort)    \
                        CVT_FUNC_2(type, short)     \
                        CVT_FUNC_2(type, uint)      \
                        CVT_FUNC_2(type, int)       \
                        CVT_FUNC_2(type, ulong)     \
                        CVT_FUNC_2(type, long)      \
                        CVT_FUNC_2(type, half)      \
                        CVT_FUNC_2(type, float)     \
                        CVT_FUNC_2(type, double)

CVT_FUNC(char)
CVT_FUNC(uchar)
CVT_FUNC(short)
CVT_FUNC(ushort)
CVT_FUNC(int)
CVT_FUNC(uint)
CVT_FUNC(long)
CVT_FUNC(ulong)
CVT_FUNC(half)
CVT_FUNC(float)
CVT_FUNC(double)


/*
 * YUV float4 version
 */

static float4 yuv_U_values = {0.f, -0.392f * 0.003921569f, +2.02 * 0.003921569f, 0.f};
static float4 yuv_V_values = {1.603f * 0.003921569f, -0.815f * 0.003921569f, 0.f, 0.f};

extern float4 __attribute__((overloadable)) rsYuvToRGBA_float4(uchar y, uchar u, uchar v) {
    float4 color = (float)y * 0.003921569f;
    float4 fU = ((float)u) - 128.f;
    float4 fV = ((float)v) - 128.f;

    color += fU * yuv_U_values;
    color += fV * yuv_V_values;
    color = clamp(color, 0.f, 1.f);
    return color;
}

