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

#ifndef RENDER_SCRIPT_INTERNAL_DEFINES_H
#define RENDER_SCRIPT_INTERNAL_DEFINES_H

#include <stdint.h>
#include <sys/types.h>

#include "rsDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////
//


typedef void (* RsBitmapCallback_t)(void *);


typedef struct {
    uint32_t colorMin;
    uint32_t colorPref;
    uint32_t alphaMin;
    uint32_t alphaPref;
    uint32_t depthMin;
    uint32_t depthPref;
    uint32_t stencilMin;
    uint32_t stencilPref;
    uint32_t samplesMin;
    uint32_t samplesPref;
    float samplesQ;
} RsSurfaceConfig;

enum RsMessageToClientType {
    RS_MESSAGE_TO_CLIENT_NONE = 0,
    RS_MESSAGE_TO_CLIENT_EXCEPTION = 1,
    RS_MESSAGE_TO_CLIENT_RESIZE = 2,
    RS_MESSAGE_TO_CLIENT_ERROR = 3,
    RS_MESSAGE_TO_CLIENT_USER = 4,
    RS_MESSAGE_TO_CLIENT_NEW_BUFFER = 5
};

enum RsTextureTarget {
    RS_TEXTURE_2D,
    RS_TEXTURE_CUBE
};

enum RsDepthFunc {
    RS_DEPTH_FUNC_ALWAYS,
    RS_DEPTH_FUNC_LESS,
    RS_DEPTH_FUNC_LEQUAL,
    RS_DEPTH_FUNC_GREATER,
    RS_DEPTH_FUNC_GEQUAL,
    RS_DEPTH_FUNC_EQUAL,
    RS_DEPTH_FUNC_NOTEQUAL
};

enum RsBlendSrcFunc {
    RS_BLEND_SRC_ZERO,                  // 0
    RS_BLEND_SRC_ONE,                   // 1
    RS_BLEND_SRC_DST_COLOR,             // 2
    RS_BLEND_SRC_ONE_MINUS_DST_COLOR,   // 3
    RS_BLEND_SRC_SRC_ALPHA,             // 4
    RS_BLEND_SRC_ONE_MINUS_SRC_ALPHA,   // 5
    RS_BLEND_SRC_DST_ALPHA,             // 6
    RS_BLEND_SRC_ONE_MINUS_DST_ALPHA,   // 7
    RS_BLEND_SRC_SRC_ALPHA_SATURATE,    // 8
    RS_BLEND_SRC_INVALID = 100,
};

enum RsBlendDstFunc {
    RS_BLEND_DST_ZERO,                  // 0
    RS_BLEND_DST_ONE,                   // 1
    RS_BLEND_DST_SRC_COLOR,             // 2
    RS_BLEND_DST_ONE_MINUS_SRC_COLOR,   // 3
    RS_BLEND_DST_SRC_ALPHA,             // 4
    RS_BLEND_DST_ONE_MINUS_SRC_ALPHA,   // 5
    RS_BLEND_DST_DST_ALPHA,             // 6
    RS_BLEND_DST_ONE_MINUS_DST_ALPHA,   // 7

    RS_BLEND_DST_INVALID = 100,
};

enum RsTexEnvMode {
    RS_TEX_ENV_MODE_NONE,
    RS_TEX_ENV_MODE_REPLACE,
    RS_TEX_ENV_MODE_MODULATE,
    RS_TEX_ENV_MODE_DECAL
};

enum RsProgramParam {
    RS_PROGRAM_PARAM_INPUT,
    RS_PROGRAM_PARAM_OUTPUT,
    RS_PROGRAM_PARAM_CONSTANT,
    RS_PROGRAM_PARAM_TEXTURE_TYPE,
};

enum RsPrimitive {
    RS_PRIMITIVE_POINT,
    RS_PRIMITIVE_LINE,
    RS_PRIMITIVE_LINE_STRIP,
    RS_PRIMITIVE_TRIANGLE,
    RS_PRIMITIVE_TRIANGLE_STRIP,
    RS_PRIMITIVE_TRIANGLE_FAN,

    RS_PRIMITIVE_INVALID = 100,
};

enum RsPathPrimitive {
    RS_PATH_PRIMITIVE_QUADRATIC_BEZIER,
    RS_PATH_PRIMITIVE_CUBIC_BEZIER
};

enum RsAnimationInterpolation {
    RS_ANIMATION_INTERPOLATION_STEP,
    RS_ANIMATION_INTERPOLATION_LINEAR,
    RS_ANIMATION_INTERPOLATION_BEZIER,
    RS_ANIMATION_INTERPOLATION_CARDINAL,
    RS_ANIMATION_INTERPOLATION_HERMITE,
    RS_ANIMATION_INTERPOLATION_BSPLINE
};

enum RsAnimationEdge {
    RS_ANIMATION_EDGE_UNDEFINED,
    RS_ANIMATION_EDGE_CONSTANT,
    RS_ANIMATION_EDGE_GRADIENT,
    RS_ANIMATION_EDGE_CYCLE,
    RS_ANIMATION_EDGE_OSCILLATE,
    RS_ANIMATION_EDGE_CYLE_RELATIVE
};

enum RsA3DClassID {
    RS_A3D_CLASS_ID_UNKNOWN,
    RS_A3D_CLASS_ID_MESH,
    RS_A3D_CLASS_ID_TYPE,
    RS_A3D_CLASS_ID_ELEMENT,
    RS_A3D_CLASS_ID_ALLOCATION,
    RS_A3D_CLASS_ID_PROGRAM_VERTEX,
    RS_A3D_CLASS_ID_PROGRAM_RASTER,
    RS_A3D_CLASS_ID_PROGRAM_FRAGMENT,
    RS_A3D_CLASS_ID_PROGRAM_STORE,
    RS_A3D_CLASS_ID_SAMPLER,
    RS_A3D_CLASS_ID_ANIMATION,
    RS_A3D_CLASS_ID_ADAPTER_1D,
    RS_A3D_CLASS_ID_ADAPTER_2D,
    RS_A3D_CLASS_ID_SCRIPT_C,
    RS_A3D_CLASS_ID_SCRIPT_KERNEL_ID,
    RS_A3D_CLASS_ID_SCRIPT_FIELD_ID,
    RS_A3D_CLASS_ID_SCRIPT_METHOD_ID,
    RS_A3D_CLASS_ID_SCRIPT_GROUP,
    RS_A3D_CLASS_ID_CLOSURE,
    RS_A3D_CLASS_ID_SCRIPT_GROUP2,
    RS_A3D_CLASS_ID_SCRIPT_INVOKE_ID
};

enum RsCullMode {
    RS_CULL_BACK,
    RS_CULL_FRONT,
    RS_CULL_NONE,
    RS_CULL_INVALID = 100,
};

enum RsScriptIntrinsicID {
    RS_SCRIPT_INTRINSIC_ID_UNDEFINED = 0,
    RS_SCRIPT_INTRINSIC_ID_CONVOLVE_3x3 = 1,
    RS_SCRIPT_INTRINSIC_ID_COLOR_MATRIX = 2,
    RS_SCRIPT_INTRINSIC_ID_LUT = 3,
    RS_SCRIPT_INTRINSIC_ID_CONVOLVE_5x5 = 4,
    RS_SCRIPT_INTRINSIC_ID_BLUR = 5,
    RS_SCRIPT_INTRINSIC_ID_YUV_TO_RGB = 6,
    RS_SCRIPT_INTRINSIC_ID_BLEND = 7,
    RS_SCRIPT_INTRINSIC_ID_3DLUT = 8,
    RS_SCRIPT_INTRINSIC_ID_HISTOGRAM = 9,
    // unused 10, 11
    RS_SCRIPT_INTRINSIC_ID_RESIZE = 12,
    RS_SCRIPT_INTRINSIC_ID_BLAS = 13,
    RS_SCRIPT_INTRINSIC_ID_EXTBLAS = 14,
    RS_SCRIPT_INTRINSIC_ID_OEM_START = 0x10000000
};

typedef struct {
    RsA3DClassID classID;
    const char* objectName;
} RsFileIndexEntry;


enum RsThreadPriorities {
    RS_THREAD_PRIORITY_LOW = 15,
    RS_THREAD_PRIORITY_NORMAL_GRAPHICS = -8,
    RS_THREAD_PRIORITY_NORMAL = -1,
    RS_THREAD_PRIORITY_LOW_LATENCY = -4
};


// Structure for rs.spec functions

typedef struct {
    RsElement e;
    uint32_t dimX;
    uint32_t dimY;
    uint32_t dimZ;
    bool mipmaps;
    bool faces;
    uint32_t yuv;
    uint32_t array0;
    uint32_t array1;
    uint32_t array2;
    uint32_t array3;
} RsTypeCreateParams;



#ifdef __cplusplus
};
#endif

#endif // RENDER_SCRIPT_DEFINES_H
