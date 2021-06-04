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
#include "rsElement.h"
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"
#include "rsRuntime.h"
#include "rsType.h"

#include "rsdCore.h"
#include "rsdBcc.h"

#include "rsdAllocation.h"
#include "rsdShaderCache.h"
#include "rsdVertexArray.h"

#include <time.h>

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
using android::renderscript::Font;
#endif

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::Element;
using android::renderscript::RsdCpuReference;
using android::renderscript::Mesh;
using android::renderscript::ObjectBase;
using android::renderscript::ObjectBaseRef;
using android::renderscript::ProgramFragment;
using android::renderscript::ProgramRaster;
using android::renderscript::ProgramStore;
using android::renderscript::ProgramVertex;
using android::renderscript::Sampler;
using android::renderscript::Script;
using android::renderscript::Type;
using android::renderscript::rs_object_base;

typedef __fp16 half;
typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef unsigned char uchar2 __attribute__((ext_vector_type(2)));
typedef unsigned char uchar3 __attribute__((ext_vector_type(3)));
typedef unsigned char uchar4 __attribute__((ext_vector_type(4)));
typedef int16_t short2 __attribute__((ext_vector_type(2)));
typedef int16_t short3 __attribute__((ext_vector_type(3)));
typedef int16_t short4 __attribute__((ext_vector_type(4)));
typedef uint16_t ushort2 __attribute__((ext_vector_type(2)));
typedef uint16_t ushort3 __attribute__((ext_vector_type(3)));
typedef uint16_t ushort4 __attribute__((ext_vector_type(4)));
typedef int32_t int2 __attribute__((ext_vector_type(2)));
typedef int32_t int3 __attribute__((ext_vector_type(3)));
typedef int32_t int4 __attribute__((ext_vector_type(4)));
typedef uint32_t uint2 __attribute__((ext_vector_type(2)));
typedef uint32_t uint3 __attribute__((ext_vector_type(3)));
typedef uint32_t uint4 __attribute__((ext_vector_type(4)));
typedef int64_t long2 __attribute__((ext_vector_type(2)));
typedef int64_t long3 __attribute__((ext_vector_type(3)));
typedef int64_t long4 __attribute__((ext_vector_type(4)));
typedef uint64_t ulong2 __attribute__((ext_vector_type(2)));
typedef uint64_t ulong3 __attribute__((ext_vector_type(3)));
typedef uint64_t ulong4 __attribute__((ext_vector_type(4)));

typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

// Add NOLINT to suppress wrong warnings from clang-tidy.
#ifndef __LP64__
#define OPAQUETYPE(t) \
    typedef struct { const int* const p; } __attribute__((packed, aligned(4))) t; /*NOLINT*/
#else
#define OPAQUETYPE(t) \
    typedef struct { const void* p; const void* unused1; const void* unused2; const void* unused3; } t; /*NOLINT*/
#endif

OPAQUETYPE(rs_element)
OPAQUETYPE(rs_type)
OPAQUETYPE(rs_allocation)
OPAQUETYPE(rs_sampler)
OPAQUETYPE(rs_script)
OPAQUETYPE(rs_script_call)

OPAQUETYPE(rs_program_fragment);
OPAQUETYPE(rs_program_store);
OPAQUETYPE(rs_program_vertex);
OPAQUETYPE(rs_program_raster);
OPAQUETYPE(rs_mesh);
OPAQUETYPE(rs_font);

#undef OPAQUETYPE

typedef enum {
    // Empty to avoid conflicting definitions with RsAllocationCubemapFace
} rs_allocation_cubemap_face;

typedef enum {
    // Empty to avoid conflicting definitions with RsYuvFormat
} rs_yuv_format;

typedef enum {
    // Empty to avoid conflicting definitions with RsAllocationMipmapControl
} rs_allocation_mipmap_control;

typedef struct { unsigned int val; } rs_allocation_usage_type;

typedef struct {
    int tm_sec;     ///< seconds
    int tm_min;     ///< minutes
    int tm_hour;    ///< hours
    int tm_mday;    ///< day of the month
    int tm_mon;     ///< month
    int tm_year;    ///< year
    int tm_wday;    ///< day of the week
    int tm_yday;    ///< day of the year
    int tm_isdst;   ///< daylight savings time
} rs_tm;

// Some RS functions are not threadsafe but can be called from an invoke
// function.  Instead of summarily marking scripts that call these functions as
// not-threadable we detect calls to them in the driver and sends a fatal error
// message.
static bool failIfInKernel(Context *rsc, const char *funcName) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    RsdCpuReference *impl = (RsdCpuReference *) dc->mCpuRef;

    if (impl->getInKernel()) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Error: Call to unsupported function %s "
                         "in kernel", funcName);
        rsc->setError(RS_ERROR_FATAL_DRIVER, buf);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Allocation routines
//////////////////////////////////////////////////////////////////////////////
#if defined(__i386__) || (defined(__mips__) && __mips==32)
// i386 and MIPS32 have different struct return passing to ARM; emulate with a pointer
const Allocation * rsGetAllocation(const void *ptr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Script *sc = RsdCpuReference::getTlsScript();
    Allocation* alloc = rsdScriptGetAllocationForPointer(rsc, sc, ptr);
    android::renderscript::rs_allocation obj = {0};
    alloc->callUpdateCacheObject(rsc, &obj);
    return (Allocation *)obj.p;
}
#else
const android::renderscript::rs_allocation rsGetAllocation(const void *ptr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Script *sc = RsdCpuReference::getTlsScript();
    Allocation* alloc = rsdScriptGetAllocationForPointer(rsc, sc, ptr);

#ifndef __LP64__ // ARMv7
    android::renderscript::rs_allocation obj = {0};
#else // AArch64/x86_64/MIPS64
    android::renderscript::rs_allocation obj = {0, 0, 0, 0};
#endif
    alloc->callUpdateCacheObject(rsc, &obj);
    return obj;
}
#endif

void __attribute__((overloadable)) rsAllocationIoSend(::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    if (failIfInKernel(rsc, "rsAllocationIoSend"))
        return;
    rsrAllocationIoSend(rsc, (Allocation *)a.p);
}

void __attribute__((overloadable)) rsAllocationIoReceive(::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    if (failIfInKernel(rsc, "rsAllocationIoReceive"))
        return;
    rsrAllocationIoReceive(rsc, (Allocation *)a.p);
}

void __attribute__((overloadable)) rsAllocationCopy1DRange(
        ::rs_allocation dstAlloc,
        uint32_t dstOff, uint32_t dstMip, uint32_t count,
        ::rs_allocation srcAlloc,
        uint32_t srcOff, uint32_t srcMip) {
    Context *rsc = RsdCpuReference::getTlsContext();
    if (failIfInKernel(rsc, "rsAllocationCopy1DRange"))
        return;
    rsrAllocationCopy1DRange(rsc, (Allocation *)dstAlloc.p, dstOff, dstMip,
                             count, (Allocation *)srcAlloc.p, srcOff, srcMip);
}

void __attribute__((overloadable)) rsAllocationCopy2DRange(
        ::rs_allocation dstAlloc,
        uint32_t dstXoff, uint32_t dstYoff,
        uint32_t dstMip, rs_allocation_cubemap_face dstFace,
        uint32_t width, uint32_t height,
        ::rs_allocation srcAlloc,
        uint32_t srcXoff, uint32_t srcYoff,
        uint32_t srcMip, rs_allocation_cubemap_face srcFace) {
    Context *rsc = RsdCpuReference::getTlsContext();
    if (failIfInKernel(rsc, "rsAllocationCopy2DRange"))
        return;
    rsrAllocationCopy2DRange(rsc, (Allocation *)dstAlloc.p,
                             dstXoff, dstYoff, dstMip, dstFace,
                             width, height, (Allocation *)srcAlloc.p,
                             srcXoff, srcYoff, srcMip, srcFace);
}

static android::renderscript::rs_element CreateElement(RsDataType dt,
                                                       RsDataKind dk,
                                                       bool isNormalized,
                                                       uint32_t vecSize) {
    Context *rsc = RsdCpuReference::getTlsContext();

    // No need for validation here.  The rsCreateElement overload below is not
    // exposed to the Script.  The Element-creation APIs call this function in a
    // consistent manner and rsComponent.cpp asserts on any inconsistency.
    Element *element = (Element *) rsrElementCreate(rsc, dt, dk, isNormalized,
                                                    vecSize);
    android::renderscript::rs_element obj = {};
    if (element == nullptr)
        return obj;
    element->callUpdateCacheObject(rsc, &obj);

    // Any new rsObject created from inside a script should have the usrRefCount
    // initialized to 0 and the sysRefCount initialized to 1.
    element->incSysRef();
    element->decUserRef();

    return obj;
}

static android::renderscript::rs_type CreateType(RsElement element,
                                                 uint32_t dimX, uint32_t dimY,
                                                 uint32_t dimZ, bool mipmaps,
                                                 bool faces,
                                                 uint32_t yuv_format) {

    Context *rsc = RsdCpuReference::getTlsContext();
    android::renderscript::rs_type obj = {};

    if (element == nullptr) {
        ALOGE("rs_type creation error: Invalid element");
        return obj;
    }

    // validate yuv_format
    RsYuvFormat yuv = (RsYuvFormat) yuv_format;
    if (yuv != RS_YUV_NONE &&
        yuv != RS_YUV_YV12 &&
        yuv != RS_YUV_NV21 &&
        yuv != RS_YUV_420_888) {

        ALOGE("rs_type creation error: Invalid yuv_format %d\n", yuv_format);
        return obj;
    }

    // validate consistency of shape parameters
    if (dimZ > 0) {
        if (dimX < 1 || dimY < 1) {
            ALOGE("rs_type creation error: Both X and Y dimension required "
                  "when Z is present.");
            return obj;
        }
        if (mipmaps) {
            ALOGE("rs_type creation error: mipmap control requires 2D types");
            return obj;
        }
        if (faces) {
            ALOGE("rs_type creation error: Cube maps require 2D types");
            return obj;
        }
    }
    if (dimY > 0 && dimX < 1) {
        ALOGE("rs_type creation error: X dimension required when Y is "
              "present.");
        return obj;
    }
    if (mipmaps && dimY < 1) {
        ALOGE("rs_type creation error: mipmap control require 2D Types.");
        return obj;
    }
    if (faces && dimY < 1) {
        ALOGE("rs_type creation error: Cube maps require 2D Types.");
        return obj;
    }
    if (yuv_format != RS_YUV_NONE) {
        if (dimZ != 0 || dimY == 0 || faces || mipmaps) {
            ALOGE("rs_type creation error: YUV only supports basic 2D.");
            return obj;
        }
    }

    Type *type = (Type *) rsrTypeCreate(rsc, element, dimX, dimY, dimZ, mipmaps,
                                        faces, yuv_format);
    if (type == nullptr)
        return obj;
    type->callUpdateCacheObject(rsc, &obj);

    // Any new rsObject created from inside a script should have the usrRefCount
    // initialized to 0 and the sysRefCount initialized to 1.
    type->incSysRef();
    type->decUserRef();

    return obj;
}

static android::renderscript::rs_allocation CreateAllocation(
        RsType type, RsAllocationMipmapControl mipmaps, uint32_t usages,
        void *ptr) {

    Context *rsc = RsdCpuReference::getTlsContext();
    android::renderscript::rs_allocation obj = {};

    if (type == nullptr) {
        ALOGE("rs_allocation creation error: Invalid type");
        return obj;
    }

    uint32_t validUsages = RS_ALLOCATION_USAGE_SCRIPT | \
                           RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE;
    if (usages & ~validUsages) {
        ALOGE("rs_allocation creation error: Invalid usage flag");
        return obj;
    }

    Allocation *alloc = (Allocation *) rsrAllocationCreateTyped(rsc, type,
                                                                mipmaps, usages,
                                                                (uintptr_t) ptr);
    if (alloc == nullptr)
        return obj;
    alloc->callUpdateCacheObject(rsc, &obj);

    // Any new rsObject created from inside a script should have the usrRefCount
    // initialized to 0 and the sysRefCount initialized to 1.
    alloc->incSysRef();
    alloc->decUserRef();

    return obj;
}

// Define rsCreateElement, rsCreateType and rsCreateAllocation entry points
// differently for 32-bit x86 and Mips.  The definitions for ARM32 and all
// 64-bit architectures is further below.
#if defined(__i386__) || (defined(__mips__) && __mips==32)

// The calling convention for the driver on 32-bit x86 and Mips returns
// rs_element etc. as a stack-return parameter.  The Script uses ARM32 calling
// conventions that return the structs in a register.  To match this convention,
// emulate the return value using a pointer.
Element *rsCreateElement(int32_t dt, int32_t dk, bool isNormalized,
                         uint32_t vecSize) {

    android::renderscript::rs_element obj = CreateElement((RsDataType) dt,
                                                          (RsDataKind) dk,
                                                          isNormalized,
                                                          vecSize);
    return (Element *) obj.p;
}

Type *rsCreateType(::rs_element element, uint32_t dimX, uint32_t dimY,
                   uint32_t dimZ, bool mipmaps, bool faces,
                   rs_yuv_format yuv_format) {
    android::renderscript::rs_type obj = CreateType((RsElement) element.p, dimX,
                                                    dimY, dimZ, mipmaps, faces,
                                                    (RsYuvFormat) yuv_format);
    return (Type *) obj.p;
}

Allocation *rsCreateAllocation(::rs_type type,
                               rs_allocation_mipmap_control mipmaps,
                               uint32_t usages, void *ptr) {

    android::renderscript::rs_allocation obj;
    obj = CreateAllocation((RsType) type.p, (RsAllocationMipmapControl) mipmaps,
                           usages, ptr);
    return (Allocation *) obj.p;
}

#else
android::renderscript::rs_element rsCreateElement(int32_t dt, int32_t dk,
                                                  bool isNormalized,
                                                  uint32_t vecSize) {

    return CreateElement((RsDataType) dt, (RsDataKind) dk, isNormalized,
                         vecSize);
}

android::renderscript::rs_type rsCreateType(::rs_element element, uint32_t dimX,
                                            uint32_t dimY, uint32_t dimZ,
                                            bool mipmaps, bool faces,
                                            rs_yuv_format yuv_format) {
    return CreateType((RsElement) element.p, dimX, dimY, dimZ, mipmaps, faces,
                      yuv_format);
}

android::renderscript::rs_allocation rsCreateAllocation(
        ::rs_type type, rs_allocation_mipmap_control mipmaps, uint32_t usages,
        void *ptr) {

    return CreateAllocation((RsType) type.p,
                            (RsAllocationMipmapControl) mipmaps,
                            usages, ptr);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Object routines
//////////////////////////////////////////////////////////////////////////////
// Add NOLINT to suppress wrong warnings from clang-tidy.
#define IS_CLEAR_SET_OBJ(t) \
    bool rsIsObject(t src) { \
        return src.p != nullptr; \
    } \
    void __attribute__((overloadable)) rsClearObject(t *dst) { /*NOLINT*/ \
        rsrClearObject(reinterpret_cast<rs_object_base *>(dst)); \
    } \
    void __attribute__((overloadable)) rsSetObject(t *dst, t src) { /*NOLINT*/ \
        Context *rsc = RsdCpuReference::getTlsContext(); \
        rsrSetObject(rsc, reinterpret_cast<rs_object_base *>(dst), (ObjectBase*)src.p); \
    }

IS_CLEAR_SET_OBJ(::rs_element)
IS_CLEAR_SET_OBJ(::rs_type)
IS_CLEAR_SET_OBJ(::rs_allocation)
IS_CLEAR_SET_OBJ(::rs_sampler)
IS_CLEAR_SET_OBJ(::rs_script)

IS_CLEAR_SET_OBJ(::rs_mesh)
IS_CLEAR_SET_OBJ(::rs_program_fragment)
IS_CLEAR_SET_OBJ(::rs_program_vertex)
IS_CLEAR_SET_OBJ(::rs_program_raster)
IS_CLEAR_SET_OBJ(::rs_program_store)
IS_CLEAR_SET_OBJ(::rs_font)

#undef IS_CLEAR_SET_OBJ

//////////////////////////////////////////////////////////////////////////////
// Element routines
//////////////////////////////////////////////////////////////////////////////
static void * ElementAt(Allocation *a, RsDataType dt, uint32_t vecSize,
                        uint32_t x, uint32_t y, uint32_t z) {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Type *t = a->getType();
    const Element *e = t->getElement();

    char buf[256];
    if (x && (x >= t->getLODDimX(0))) {
        snprintf(buf, sizeof(buf), "Out range ElementAt X %i of %i", x, t->getLODDimX(0));
        rsc->setError(RS_ERROR_FATAL_DEBUG, buf);
        return nullptr;
    }

    if (y && (y >= t->getLODDimY(0))) {
        snprintf(buf, sizeof(buf), "Out range ElementAt Y %i of %i", y, t->getLODDimY(0));
        rsc->setError(RS_ERROR_FATAL_DEBUG, buf);
        return nullptr;
    }

    if (z && (z >= t->getLODDimZ(0))) {
        snprintf(buf, sizeof(buf), "Out range ElementAt Z %i of %i", z, t->getLODDimZ(0));
        rsc->setError(RS_ERROR_FATAL_DEBUG, buf);
        return nullptr;
    }

    if (vecSize > 0) {
        if (vecSize != e->getVectorSize()) {
            snprintf(buf, sizeof(buf), "Vector size mismatch for ElementAt %i of %i", vecSize, e->getVectorSize());
            rsc->setError(RS_ERROR_FATAL_DEBUG, buf);
            return nullptr;
        }

        if (dt != e->getType()) {
            snprintf(buf, sizeof(buf), "Data type mismatch for ElementAt %i of %i", dt, e->getType());
            rsc->setError(RS_ERROR_FATAL_DEBUG, buf);
            return nullptr;
        }
    }

    uint8_t *p = (uint8_t *)a->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = e->getSizeBytes();
    const uint32_t stride = a->mHal.drvState.lod[0].stride;
    const uint32_t dimY = a->mHal.drvState.lod[0].dimY;
    return &p[(x * eSize) + (y * stride) + (z * stride * dimY)];
}

void rsSetElementAt(::rs_allocation a, const void *ptr, uint32_t x, uint32_t y, uint32_t z) {
    const Type *t = const_cast<Allocation*>((Allocation*)a.p)->getType();
    const Element *e = t->getElement();
    void *tmp = ElementAt((Allocation *)a.p, RS_TYPE_UNSIGNED_8, 0, x, y, z);
    if (tmp != nullptr)
        memcpy(tmp, ptr, e->getSizeBytes());
}

void rsSetElementAt(::rs_allocation a, const void *ptr, uint32_t x, uint32_t y) {
    rsSetElementAt(a, ptr, x, y, 0);
}

void rsSetElementAt(::rs_allocation a, const void *ptr, uint32_t x) {
    rsSetElementAt(a, ptr, x, 0, 0);
}

const void *rsGetElementAt(::rs_allocation a, uint32_t x, uint32_t y, uint32_t z) {
    return ElementAt((Allocation *)a.p, RS_TYPE_UNSIGNED_8, 0, x, y, z);
}

const void *rsGetElementAt(::rs_allocation a, uint32_t x, uint32_t y) {
    return rsGetElementAt(a, x, y ,0);
}

const void *rsGetElementAt(::rs_allocation a, uint32_t x) {
    return rsGetElementAt(a, x, 0, 0);
}

// Add NOLINT to suppress wrong warnings from clang-tidy.
#define ELEMENT_AT(T, DT, VS) \
    void rsSetElementAt_##T(::rs_allocation a, const T *val, uint32_t x, uint32_t y, uint32_t z) { \
        void *r = ElementAt((Allocation *)a.p, DT, VS, x, y, z); \
        if (r != nullptr) ((T *)r)[0] = *val; \
        else ALOGE("Error from %s", __PRETTY_FUNCTION__); \
    } \
    void rsSetElementAt_##T(::rs_allocation a, const T *val, uint32_t x, uint32_t y) { \
        rsSetElementAt_##T(a, val, x, y, 0); \
    } \
    void rsSetElementAt_##T(::rs_allocation a, const T *val, uint32_t x) { \
        rsSetElementAt_##T(a, val, x, 0, 0); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, T *val, uint32_t x, uint32_t y, uint32_t z) { /*NOLINT*/ \
        void *r = ElementAt((Allocation *)a.p, DT, VS, x, y, z); \
        if (r != nullptr) *val = ((T *)r)[0]; \
        else ALOGE("Error from %s", __PRETTY_FUNCTION__); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, T *val, uint32_t x, uint32_t y) { /*NOLINT*/ \
        rsGetElementAt_##T(a, val, x, y, 0); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, T *val, uint32_t x) { /*NOLINT*/ \
        rsGetElementAt_##T(a, val, x, 0, 0); \
    }

ELEMENT_AT(char, RS_TYPE_SIGNED_8, 1)
ELEMENT_AT(char2, RS_TYPE_SIGNED_8, 2)
ELEMENT_AT(char3, RS_TYPE_SIGNED_8, 3)
ELEMENT_AT(char4, RS_TYPE_SIGNED_8, 4)
ELEMENT_AT(uchar, RS_TYPE_UNSIGNED_8, 1)
ELEMENT_AT(uchar2, RS_TYPE_UNSIGNED_8, 2)
ELEMENT_AT(uchar3, RS_TYPE_UNSIGNED_8, 3)
ELEMENT_AT(uchar4, RS_TYPE_UNSIGNED_8, 4)
ELEMENT_AT(short, RS_TYPE_SIGNED_16, 1)
ELEMENT_AT(short2, RS_TYPE_SIGNED_16, 2)
ELEMENT_AT(short3, RS_TYPE_SIGNED_16, 3)
ELEMENT_AT(short4, RS_TYPE_SIGNED_16, 4)
ELEMENT_AT(ushort, RS_TYPE_UNSIGNED_16, 1)
ELEMENT_AT(ushort2, RS_TYPE_UNSIGNED_16, 2)
ELEMENT_AT(ushort3, RS_TYPE_UNSIGNED_16, 3)
ELEMENT_AT(ushort4, RS_TYPE_UNSIGNED_16, 4)
ELEMENT_AT(int, RS_TYPE_SIGNED_32, 1)
ELEMENT_AT(int2, RS_TYPE_SIGNED_32, 2)
ELEMENT_AT(int3, RS_TYPE_SIGNED_32, 3)
ELEMENT_AT(int4, RS_TYPE_SIGNED_32, 4)
ELEMENT_AT(uint, RS_TYPE_UNSIGNED_32, 1)
ELEMENT_AT(uint2, RS_TYPE_UNSIGNED_32, 2)
ELEMENT_AT(uint3, RS_TYPE_UNSIGNED_32, 3)
ELEMENT_AT(uint4, RS_TYPE_UNSIGNED_32, 4)
#ifdef __LP64__
ELEMENT_AT(long, RS_TYPE_SIGNED_64, 1)
#else
/* the long versions need special treatment; the long * argument has to be
 * kept so the signatures match, but the actual accesses have to be done in
 * int64_t * to be consistent with the script ABI.
 */
void rsSetElementAt_long(::rs_allocation a, const long *val, uint32_t x, uint32_t y, uint32_t z) {
    void *r = ElementAt((Allocation *)a.p, RS_TYPE_SIGNED_64, 1, x, y, z);
    if (r != nullptr) ((int64_t *)r)[0] = *((int64_t *)val);
    else ALOGE("Error from %s", __PRETTY_FUNCTION__);
}
void rsSetElementAt_long(::rs_allocation a, const long *val, uint32_t x, uint32_t y) {
    rsSetElementAt_long(a, val, x, y, 0);
}
void rsSetElementAt_long(::rs_allocation a, const long *val, uint32_t x) {
    rsSetElementAt_long(a, val, x, 0, 0);
}
void rsGetElementAt_long(::rs_allocation a, long *val, uint32_t x, uint32_t y, uint32_t z) { /*NOLINT*/
    void *r = ElementAt((Allocation *)a.p, RS_TYPE_SIGNED_64, 1, x, y, z);
    if (r != nullptr) *((int64_t*)val) = ((int64_t *)r)[0];
    else ALOGE("Error from %s", __PRETTY_FUNCTION__);
}
void rsGetElementAt_long(::rs_allocation a, long *val, uint32_t x, uint32_t y) { /*NOLINT*/
    rsGetElementAt_long(a, val, x, y, 0);
}
void rsGetElementAt_long(::rs_allocation a, long *val, uint32_t x) { /*NOLINT*/
    rsGetElementAt_long(a, val, x, 0, 0);
}
#endif
ELEMENT_AT(long2, RS_TYPE_SIGNED_64, 2)
ELEMENT_AT(long3, RS_TYPE_SIGNED_64, 3)
ELEMENT_AT(long4, RS_TYPE_SIGNED_64, 4)
ELEMENT_AT(ulong, RS_TYPE_UNSIGNED_64, 1)
ELEMENT_AT(ulong2, RS_TYPE_UNSIGNED_64, 2)
ELEMENT_AT(ulong3, RS_TYPE_UNSIGNED_64, 3)
ELEMENT_AT(ulong4, RS_TYPE_UNSIGNED_64, 4)
ELEMENT_AT(half, RS_TYPE_FLOAT_16, 1)
ELEMENT_AT(half2, RS_TYPE_FLOAT_16, 2)
ELEMENT_AT(half3, RS_TYPE_FLOAT_16, 3)
ELEMENT_AT(half4, RS_TYPE_FLOAT_16, 4)
ELEMENT_AT(float, RS_TYPE_FLOAT_32, 1)
ELEMENT_AT(float2, RS_TYPE_FLOAT_32, 2)
ELEMENT_AT(float3, RS_TYPE_FLOAT_32, 3)
ELEMENT_AT(float4, RS_TYPE_FLOAT_32, 4)
ELEMENT_AT(double, RS_TYPE_FLOAT_64, 1)
ELEMENT_AT(double2, RS_TYPE_FLOAT_64, 2)
ELEMENT_AT(double3, RS_TYPE_FLOAT_64, 3)
ELEMENT_AT(double4, RS_TYPE_FLOAT_64, 4)

#undef ELEMENT_AT

#ifndef __LP64__
/*
 * We miss some symbols for rs{Get,Set}Element_long,ulong variants because 64
 * bit integer values are 'long' in RS-land but might be 'long long' in the
 * driver.  Define native_long* and native_ulong* types to be vectors of
 * 'long' as seen by the driver and define overloaded versions of
 * rsSetElementAt_* and rsGetElementAt_*.  This should get us the correct
 * mangled names in the driver.
 */

typedef long native_long2 __attribute__((ext_vector_type(2)));
typedef long native_long3 __attribute__((ext_vector_type(3)));
typedef long native_long4 __attribute__((ext_vector_type(4)));
typedef unsigned long native_ulong2 __attribute__((ext_vector_type(2)));
typedef unsigned long native_ulong3 __attribute__((ext_vector_type(3)));
typedef unsigned long native_ulong4 __attribute__((ext_vector_type(4)));

// Add NOLINT to suppress wrong warnings from clang-tidy.
#define ELEMENT_AT_OVERLOADS(T, U) \
    void rsSetElementAt_##T(::rs_allocation a, const U *val, uint32_t x, uint32_t y, uint32_t z) { \
        rsSetElementAt_##T(a, (T *) val, x, y, z); \
    } \
    void rsSetElementAt_##T(::rs_allocation a, const U *val, uint32_t x, uint32_t y) { \
        rsSetElementAt_##T(a, (T *) val, x, y, 0); \
    } \
    void rsSetElementAt_##T(::rs_allocation a, const U *val, uint32_t x) { \
        rsSetElementAt_##T(a, (T *) val, x, 0, 0); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, U *val, uint32_t x, uint32_t y, uint32_t z) { /*NOLINT*/ \
        rsGetElementAt_##T(a, (T *) val, x, y, z); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, U *val, uint32_t x, uint32_t y) { /*NOLINT*/ \
        rsGetElementAt_##T(a, (T *) val, x, y, 0); \
    } \
    void rsGetElementAt_##T(::rs_allocation a, U *val, uint32_t x) { /*NOLINT*/ \
        rsGetElementAt_##T(a, (T *) val, x, 0, 0); \
    } \

ELEMENT_AT_OVERLOADS(long2, native_long2)
ELEMENT_AT_OVERLOADS(long3, native_long3)
ELEMENT_AT_OVERLOADS(long4, native_long4)
ELEMENT_AT_OVERLOADS(ulong, unsigned long)
ELEMENT_AT_OVERLOADS(ulong2, native_ulong2)
ELEMENT_AT_OVERLOADS(ulong3, native_ulong3)
ELEMENT_AT_OVERLOADS(ulong4, native_ulong4)

// We also need variants of rs{Get,Set}ElementAt_long that take 'long long *' as
// we might have this overloaded variant in old APKs.
ELEMENT_AT_OVERLOADS(long, long long)

#undef ELEMENT_AT_OVERLOADS
#endif

//////////////////////////////////////////////////////////////////////////////
// ForEach routines
//////////////////////////////////////////////////////////////////////////////
void rsForEachInternal(int slot,
                       rs_script_call *options,
                       int hasOutput,
                       int numInputs,
                       ::rs_allocation* allocs) {
    Context *rsc = RsdCpuReference::getTlsContext();
    Script *s = const_cast<Script*>(RsdCpuReference::getTlsScript());
    if (numInputs > RS_KERNEL_MAX_ARGUMENTS) {
        rsc->setError(RS_ERROR_BAD_SCRIPT,
                      "rsForEachInternal: too many inputs to a kernel.");
        return;
    }
    Allocation* inputs[RS_KERNEL_MAX_ARGUMENTS];
    for (int i = 0; i < numInputs; i++) {
        inputs[i] = (Allocation*)allocs[i].p;
        CHECK_OBJ(inputs[i]);
        inputs[i]->incSysRef();
    }
    Allocation* out = nullptr;
    if (hasOutput) {
        out = (Allocation*)allocs[numInputs].p;
        CHECK_OBJ(out);
        out->incSysRef();
    }
    rsrForEach(rsc, s, slot, numInputs, numInputs > 0 ? inputs : nullptr, out,
               nullptr, 0, (RsScriptCall*)options);
    for (int i = 0; i < numInputs; i++) {
        inputs[i]->decSysRef();
    }
    if (hasOutput) {
        out->decSysRef();
    }
}

void __attribute__((overloadable)) rsForEach(::rs_script script,
                                             ::rs_allocation in,
                                             ::rs_allocation out,
                                             const void *usr,
                                             const rs_script_call *call) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, (Script *)script.p, 0, 1, (Allocation **)&in.p,
               (Allocation *)out.p, usr, 0, (RsScriptCall *)call);
}

void __attribute__((overloadable)) rsForEach(::rs_script script,
                                             ::rs_allocation in,
                                             ::rs_allocation out,
                                             const void *usr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, (Script *)script.p, 0, 1, (Allocation **)&in.p, (Allocation *)out.p,
               usr, 0, nullptr);
}

void __attribute__((overloadable)) rsForEach(::rs_script script,
                                             ::rs_allocation in,
                                             ::rs_allocation out) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, (Script *)script.p, 0, 1, (Allocation **)&in.p, (Allocation *)out.p,
               nullptr, 0, nullptr);
}

// These functions are only supported in 32-bit.
#ifndef __LP64__
void __attribute__((overloadable)) rsForEach(::rs_script script,
                                             ::rs_allocation in,
                                             ::rs_allocation out,
                                             const void *usr,
                                             uint32_t usrLen) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, (Script *)script.p, 0, 1, (Allocation **)&in.p, (Allocation *)out.p,
               usr, usrLen, nullptr);
}

void __attribute__((overloadable)) rsForEach(::rs_script script,
                                             ::rs_allocation in,
                                             ::rs_allocation out,
                                             const void *usr,
                                             uint32_t usrLen,
                                             const rs_script_call *call) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, (Script *)script.p, 0, 1, (Allocation **)&in.p, (Allocation *)out.p,
               usr, usrLen, (RsScriptCall *)call);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Message routines
//////////////////////////////////////////////////////////////////////////////
uint32_t rsSendToClient(int cmdID) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClient(rsc, cmdID, (const void *)nullptr, 0);
}

uint32_t rsSendToClient(int cmdID, const void *data, uint32_t len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClient(rsc, cmdID, data, len);
}

uint32_t rsSendToClientBlocking(int cmdID) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClientBlocking(rsc, cmdID, (const void *)nullptr, 0);
}

uint32_t rsSendToClientBlocking(int cmdID, const void *data, uint32_t len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClientBlocking(rsc, cmdID, data, len);
}

//////////////////////////////////////////////////////////////////////////////
// Time routines
//////////////////////////////////////////////////////////////////////////////

// time_t is int in 32-bit RenderScript.  time_t is long in bionic.  rsTime and
// rsLocaltime are set to explicitly take 'const int *' so we generate the
// correct mangled names.
#ifndef __LP64__
int rsTime(int *timer) {
#else
time_t rsTime(time_t * timer) {
#endif
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrTime(rsc, (time_t *)timer);
}

#ifndef __LP64__
rs_tm* rsLocaltime(rs_tm* local, const int *timer) {
#else
rs_tm* rsLocaltime(rs_tm* local, const time_t *timer) {
#endif
    Context *rsc = RsdCpuReference::getTlsContext();
    return (rs_tm*)rsrLocalTime(rsc, (tm*)local, (time_t *)timer);
}

int64_t rsUptimeMillis() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrUptimeMillis(rsc);
}

int64_t rsUptimeNanos() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrUptimeNanos(rsc);
}

float rsGetDt() {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Script *sc = RsdCpuReference::getTlsScript();
    return rsrGetDt(rsc, sc);
}

//////////////////////////////////////////////////////////////////////////////
// Graphics routines
//////////////////////////////////////////////////////////////////////////////
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
static void SC_DrawQuadTexCoords(float x1, float y1, float z1, float u1, float v1,
                                 float x2, float y2, float z2, float u2, float v2,
                                 float x3, float y3, float z3, float u3, float v3,
                                 float x4, float y4, float z4, float u4, float v4) {
    Context *rsc = RsdCpuReference::getTlsContext();

    if (!rsc->setupCheck()) {
        return;
    }

    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    if (!dc->gl.shaderCache->setup(rsc)) {
        return;
    }

    //ALOGE("Quad");
    //ALOGE("%4.2f, %4.2f, %4.2f", x1, y1, z1);
    //ALOGE("%4.2f, %4.2f, %4.2f", x2, y2, z2);
    //ALOGE("%4.2f, %4.2f, %4.2f", x3, y3, z3);
    //ALOGE("%4.2f, %4.2f, %4.2f", x4, y4, z4);

    float vtx[] = {x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4};
    const float tex[] = {u1,v1, u2,v2, u3,v3, u4,v4};

    RsdVertexArray::Attrib attribs[2];
    attribs[0].set(GL_FLOAT, 3, 12, false, (size_t)vtx, "ATTRIB_position");
    attribs[1].set(GL_FLOAT, 2, 8, false, (size_t)tex, "ATTRIB_texture0");

    RsdVertexArray va(attribs, 2);
    va.setup(rsc);

    RSD_CALL_GL(glDrawArrays, GL_TRIANGLE_FAN, 0, 4);
}

static void SC_DrawQuad(float x1, float y1, float z1,
                        float x2, float y2, float z2,
                        float x3, float y3, float z3,
                        float x4, float y4, float z4) {
    SC_DrawQuadTexCoords(x1, y1, z1, 0, 1,
                         x2, y2, z2, 1, 1,
                         x3, y3, z3, 1, 0,
                         x4, y4, z4, 0, 0);
}

static void SC_DrawSpriteScreenspace(float x, float y, float z, float w, float h) {
    Context *rsc = RsdCpuReference::getTlsContext();

    ObjectBaseRef<const ProgramVertex> tmp(rsc->getProgramVertex());
    rsc->setProgramVertex(rsc->getDefaultProgramVertex());
    //rsc->setupCheck();

    //GLint crop[4] = {0, h, w, -h};

    float sh = rsc->getHeight();

    SC_DrawQuad(x,   sh - y,     z,
                x+w, sh - y,     z,
                x+w, sh - (y+h), z,
                x,   sh - (y+h), z);
    rsc->setProgramVertex((ProgramVertex *)tmp.get());
}

void rsAllocationMarkDirty(::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, (Allocation *)a.p, RS_ALLOCATION_USAGE_SCRIPT);
}

void rsgAllocationSyncAll(::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, (Allocation *)a.p, RS_ALLOCATION_USAGE_SCRIPT);
}

void rsgAllocationSyncAll(::rs_allocation a,
                          unsigned int usage) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, (Allocation *)a.p, (RsAllocationUsageType)usage);
}


void rsgAllocationSyncAll(::rs_allocation a,
                          rs_allocation_usage_type source) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, (Allocation *)a.p, (RsAllocationUsageType)source.val);
}

void rsgBindProgramFragment(::rs_program_fragment pf) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramFragment(rsc, (ProgramFragment *)pf.p);
}

void rsgBindProgramStore(::rs_program_store ps) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramStore(rsc, (ProgramStore *)ps.p);
}

void rsgBindProgramVertex(::rs_program_vertex pv) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramVertex(rsc, (ProgramVertex *)pv.p);
}

void rsgBindProgramRaster(::rs_program_raster pr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramRaster(rsc, (ProgramRaster *)pr.p);
}

void rsgBindSampler(::rs_program_fragment pf,
                    uint32_t slot, ::rs_sampler s) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindSampler(rsc, (ProgramFragment *)pf.p, slot, (Sampler *)s.p);
}

void rsgBindTexture(::rs_program_fragment pf,
                    uint32_t slot, ::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindTexture(rsc, (ProgramFragment *)pf.p, slot, (Allocation *)a.p);
}

void rsgBindConstant(::rs_program_fragment pf,
                     uint32_t slot, ::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindConstant(rsc, (ProgramFragment *)pf.p, slot, (Allocation *)a.p);
}

void rsgBindConstant(::rs_program_vertex pv,
                     uint32_t slot, ::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindConstant(rsc, (ProgramVertex *)pv.p, slot, (Allocation *)a.p);
}

void rsgProgramVertexLoadProjectionMatrix(const rs_matrix4x4 *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadProjectionMatrix(rsc, (const rsc_Matrix *)m);
}

void rsgProgramVertexLoadModelMatrix(const rs_matrix4x4 *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadModelMatrix(rsc, (const rsc_Matrix *)m);
}

void rsgProgramVertexLoadTextureMatrix(const rs_matrix4x4 *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadTextureMatrix(rsc, (const rsc_Matrix *)m);
}

void rsgProgramVertexGetProjectionMatrix(rs_matrix4x4 *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpGetProjectionMatrix(rsc, (rsc_Matrix *)m);
}

void rsgProgramFragmentConstantColor(::rs_program_fragment pf,
                                     float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPfConstantColor(rsc, (ProgramFragment *)pf.p, r, g, b, a);
}

uint32_t rsgGetWidth(void) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrGetWidth(rsc);
}

uint32_t rsgGetHeight(void) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrGetHeight(rsc);
}

void rsgDrawRect(float x1, float y1, float x2, float y2, float z) {
    SC_DrawQuad(x1, y2, z,
                x2, y2, z,
                x2, y1, z,
                x1, y1, z);
}

void rsgDrawQuad(float x1, float y1, float z1,
                 float x2, float y2, float z2,
                 float x3, float y3, float z3,
                 float x4, float y4, float z4) {
    SC_DrawQuad(x1, y1, z1,
                x2, y2, z2,
                x3, y3, z3,
                x4, y4, z4);
}

void rsgDrawQuadTexCoords(float x1, float y1, float z1, float u1, float v1,
                          float x2, float y2, float z2, float u2, float v2,
                          float x3, float y3, float z3, float u3, float v3,
                          float x4, float y4, float z4, float u4, float v4) {
    SC_DrawQuadTexCoords(x1, y1, z1, u1, v1,
                         x2, y2, z2, u2, v2,
                         x3, y3, z3, u3, v3,
                         x4, y4, z4, u4, v4);
}

void rsgDrawSpriteScreenspace(float x, float y, float z, float w, float h) {
    SC_DrawSpriteScreenspace(x, y, z, w, h);
}

void rsgDrawMesh(::rs_mesh ism) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMesh(rsc, (Mesh *)ism.p);
}

void rsgDrawMesh(::rs_mesh ism, uint primitiveIndex) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMeshPrimitive(rsc, (Mesh *)ism.p, primitiveIndex);
}

void rsgDrawMesh(::rs_mesh ism, uint primitiveIndex, uint start, uint len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMeshPrimitiveRange(rsc, (Mesh *)ism.p, primitiveIndex, start, len);
}

void  rsgMeshComputeBoundingBox(::rs_mesh mesh,
                                float *minX, float *minY, float *minZ,
                                float *maxX, float *maxY, float *maxZ) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeshComputeBoundingBox(rsc, (Mesh *)mesh.p, minX, minY, minZ, maxX, maxY, maxZ);
}

void rsgClearColor(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPrepareClear(rsc);
    rsdGLClearColor(rsc, r, g, b, a);
}

void rsgClearDepth(float value) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPrepareClear(rsc);
    rsdGLClearDepth(rsc, value);
}

void rsgDrawText(const char *text, int x, int y) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawText(rsc, text, x, y);
}

void rsgDrawText(::rs_allocation a, int x, int y) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawTextAlloc(rsc, (Allocation *)a.p, x, y);
}

void rsgMeasureText(const char *text, int *left, int *right,
                    int *top, int *bottom) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeasureText(rsc, text, left, right, top, bottom);
}

void rsgMeasureText(::rs_allocation a, int *left, int *right,
                    int *top, int *bottom) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeasureTextAlloc(rsc, (Allocation *)a.p, left, right, top, bottom);
}

void rsgBindFont(::rs_font font) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFont(rsc, (Font *)font.p);
}

void rsgFontColor(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrFontColor(rsc, r, g, b, a);
}

void rsgBindColorTarget(::rs_allocation a, uint slot) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFrameBufferObjectColorTarget(rsc, (Allocation *)a.p, slot);
}

void rsgBindDepthTarget(::rs_allocation a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFrameBufferObjectDepthTarget(rsc, (Allocation *)a.p);
}

void rsgClearColorTarget(uint slot) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectColorTarget(rsc, slot);
}

void rsgClearDepthTarget(void) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectDepthTarget(rsc);
}

void rsgClearAllRenderTargets(void) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectTargets(rsc);
}

void color(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrColor(rsc, r, g, b, a);
}

void rsgFinish(void) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsdGLFinish(rsc);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Debug routines
//////////////////////////////////////////////////////////////////////////////
void rsDebug(const char *s, float f) {
    ALOGD("%s %f, 0x%08x", s, f, *((int *) (&f)));
}

void rsDebug(const char *s, float f1, float f2) {
    ALOGD("%s {%f, %f}", s, f1, f2);
}

void rsDebug(const char *s, float f1, float f2, float f3) {
    ALOGD("%s {%f, %f, %f}", s, f1, f2, f3);
}

void rsDebug(const char *s, float f1, float f2, float f3, float f4) {
    ALOGD("%s {%f, %f, %f, %f}", s, f1, f2, f3, f4);
}

void rsDebug(const char *s, const float2 *f2) {
    float2 f = *f2;
    ALOGD("%s {%f, %f}", s, f.x, f.y);
}

void rsDebug(const char *s, const float3 *f3) {
    float3 f = *f3;
    ALOGD("%s {%f, %f, %f}", s, f.x, f.y, f.z);
}

void rsDebug(const char *s, const float4 *f4) {
    float4 f = *f4;
    ALOGD("%s {%f, %f, %f, %f}", s, f.x, f.y, f.z, f.w);
}

// Accept a half value converted to float.  This eliminates the need in the
// driver to properly support the half datatype (either by adding compiler flags
// for half or link against compiler_rt).
void rsDebug(const char *s, float f, ushort us) {
    ALOGD("%s {%f} {0x%hx}", s, f, us);
}

void rsDebug(const char *s, const float2 *f2, const ushort2 *us2) {
    float2 f = *f2;
    ushort2 us = *us2;
    ALOGD("%s {%f %f} {0x%hx 0x%hx}", s, f.x, f.y, us.x, us.y);
}

void rsDebug(const char *s, const float3 *f3, const ushort3 *us3) {
    float3 f = *f3;
    ushort3 us = *us3;
    ALOGD("%s {%f %f %f} {0x%hx 0x%hx 0x%hx}", s, f.x, f.y, f.z, us.x, us.y,
          us.z);
}

void rsDebug(const char *s, const float4 *f4, const ushort4 *us4) {
    float4 f = *f4;
    ushort4 us = *us4;
    ALOGD("%s {%f %f %f %f} {0x%hx 0x%hx 0x%hx 0x%hx}", s, f.x, f.y, f.z, f.w,
          us.x, us.y, us.z, us.w);
}

void rsDebug(const char *s, double d) {
    ALOGD("%s %f, 0x%08llx", s, d, *((long long *) (&d)));
}

void rsDebug(const char *s, const double2 *d2) {
    double2 d = *d2;
    ALOGD("%s {%f, %f}", s, d.x, d.y);
}

void rsDebug(const char *s, const double3 *d3) {
    double3 d = *d3;
    ALOGD("%s {%f, %f, %f}", s, d.x, d.y, d.z);
}

void rsDebug(const char *s, const double4 *d4) {
    double4 d = *d4;
    ALOGD("%s {%f, %f, %f, %f}", s, d.x, d.y, d.z, d.w);
}

void rsDebug(const char *s, const rs_matrix4x4 *m) {
    float *f = (float *)m;
    ALOGD("%s {%f, %f, %f, %f", s, f[0], f[4], f[8], f[12]);
    ALOGD("%s  %f, %f, %f, %f", s, f[1], f[5], f[9], f[13]);
    ALOGD("%s  %f, %f, %f, %f", s, f[2], f[6], f[10], f[14]);
    ALOGD("%s  %f, %f, %f, %f}", s, f[3], f[7], f[11], f[15]);
}

void rsDebug(const char *s, const rs_matrix3x3 *m) {
    float *f = (float *)m;
    ALOGD("%s {%f, %f, %f", s, f[0], f[3], f[6]);
    ALOGD("%s  %f, %f, %f", s, f[1], f[4], f[7]);
    ALOGD("%s  %f, %f, %f}",s, f[2], f[5], f[8]);
}

void rsDebug(const char *s, const rs_matrix2x2 *m) {
    float *f = (float *)m;
    ALOGD("%s {%f, %f", s, f[0], f[2]);
    ALOGD("%s  %f, %f}",s, f[1], f[3]);
}

void rsDebug(const char *s, char c) {
    ALOGD("%s %hhd  0x%hhx", s, c, (unsigned char)c);
}

void rsDebug(const char *s, const char2 *c2) {
    char2 c = *c2;
    ALOGD("%s {%hhd, %hhd}  0x%hhx 0x%hhx", s, c.x, c.y, (unsigned char)c.x, (unsigned char)c.y);
}

void rsDebug(const char *s, const char3 *c3) {
    char3 c = *c3;
    ALOGD("%s {%hhd, %hhd, %hhd}  0x%hhx 0x%hhx 0x%hhx", s, c.x, c.y, c.z, (unsigned char)c.x, (unsigned char)c.y, (unsigned char)c.z);
}

void rsDebug(const char *s, const char4 *c4) {
    char4 c = *c4;
    ALOGD("%s {%hhd, %hhd, %hhd, %hhd}  0x%hhx 0x%hhx 0x%hhx 0x%hhx", s, c.x, c.y, c.z, c.w, (unsigned char)c.x, (unsigned char)c.y, (unsigned char)c.z, (unsigned char)c.w);
}

void rsDebug(const char *s, unsigned char c) {
    ALOGD("%s %hhu  0x%hhx", s, c, c);
}

void rsDebug(const char *s, const uchar2 *c2) {
    uchar2 c = *c2;
    ALOGD("%s {%hhu, %hhu}  0x%hhx 0x%hhx", s, c.x, c.y, c.x, c.y);
}

void rsDebug(const char *s, const uchar3 *c3) {
    uchar3 c = *c3;
    ALOGD("%s {%hhu, %hhu, %hhu}  0x%hhx 0x%hhx 0x%hhx", s, c.x, c.y, c.z, c.x, c.y, c.z);
}

void rsDebug(const char *s, const uchar4 *c4) {
    uchar4 c = *c4;
    ALOGD("%s {%hhu, %hhu, %hhu, %hhu}  0x%hhx 0x%hhx 0x%hhx 0x%hhx", s, c.x, c.y, c.z, c.w, c.x, c.y, c.z, c.w);
}

void rsDebug(const char *s, int16_t c) {
    ALOGD("%s %hd  0x%hx", s, c, c);
}

void rsDebug(const char *s, const short2 *c2) {
    short2 c = *c2;
    ALOGD("%s {%hd, %hd}  0x%hx 0x%hx", s, c.x, c.y, c.x, c.y);
}

void rsDebug(const char *s, const short3 *c3) {
    short3 c = *c3;
    ALOGD("%s {%hd, %hd, %hd}  0x%hx 0x%hx 0x%hx", s, c.x, c.y, c.z, c.x, c.y, c.z);
}

void rsDebug(const char *s, const short4 *c4) {
    short4 c = *c4;
    ALOGD("%s {%hd, %hd, %hd, %hd}  0x%hx 0x%hx 0x%hx 0x%hx", s, c.x, c.y, c.z, c.w, c.x, c.y, c.z, c.w);
}

void rsDebug(const char *s, uint16_t c) {
    ALOGD("%s %hu  0x%hx", s, c, c);
}

void rsDebug(const char *s, const ushort2 *c2) {
    ushort2 c = *c2;
    ALOGD("%s {%hu, %hu}  0x%hx 0x%hx", s, c.x, c.y, c.x, c.y);
}

void rsDebug(const char *s, const ushort3 *c3) {
    ushort3 c = *c3;
    ALOGD("%s {%hu, %hu, %hu}  0x%hx 0x%hx 0x%hx", s, c.x, c.y, c.z, c.x, c.y, c.z);
}

void rsDebug(const char *s, const ushort4 *c4) {
    ushort4 c = *c4;
    ALOGD("%s {%hu, %hu, %hu, %hu}  0x%hx 0x%hx 0x%hx 0x%hx", s, c.x, c.y, c.z, c.w, c.x, c.y, c.z, c.w);
}

void rsDebug(const char *s, int i) {
    ALOGD("%s %d  0x%x", s, i, i);
}

void rsDebug(const char *s, const int2 *i2) {
    int2 i = *i2;
    ALOGD("%s {%d, %d}  0x%x 0x%x", s, i.x, i.y, i.x, i.y);
}

void rsDebug(const char *s, const int3 *i3) {
    int3 i = *i3;
    ALOGD("%s {%d, %d, %d}  0x%x 0x%x 0x%x", s, i.x, i.y, i.z, i.x, i.y, i.z);
}

void rsDebug(const char *s, const int4 *i4) {
    int4 i = *i4;
    ALOGD("%s {%d, %d, %d, %d}  0x%x 0x%x 0x%x 0x%x", s, i.x, i.y, i.z, i.w, i.x, i.y, i.z, i.w);
}

void rsDebug(const char *s, unsigned int i) {
    ALOGD("%s %u  0x%x", s, i, i);
}

void rsDebug(const char *s, const uint2 *i2) {
    uint2 i = *i2;
    ALOGD("%s {%u, %u}  0x%x 0x%x", s, i.x, i.y, i.x, i.y);
}

void rsDebug(const char *s, const uint3 *i3) {
    uint3 i = *i3;
    ALOGD("%s {%u, %u, %u}  0x%x 0x%x 0x%x", s, i.x, i.y, i.z, i.x, i.y, i.z);
}

void rsDebug(const char *s, const uint4 *i4) {
    uint4 i = *i4;
    ALOGD("%s {%u, %u, %u, %u}  0x%x 0x%x 0x%x 0x%x", s, i.x, i.y, i.z, i.w, i.x, i.y, i.z, i.w);
}

template <typename T>
static inline long long LL(const T &x) {
    return static_cast<long long>(x);
}

template <typename T>
static inline unsigned long long LLu(const T &x) {
    return static_cast<unsigned long long>(x);
}

void rsDebug(const char *s, long l) {
    ALOGD("%s %lld  0x%llx", s, LL(l), LL(l));
}

void rsDebug(const char *s, long long ll) {
    ALOGD("%s %lld  0x%llx", s, LL(ll), LL(ll));
}

void rsDebug(const char *s, const long2 *c) {
    long2 ll = *c;
    ALOGD("%s {%lld, %lld}  0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.x), LL(ll.y));
}

void rsDebug(const char *s, const long3 *c) {
    long3 ll = *c;
    ALOGD("%s {%lld, %lld, %lld}  0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.x), LL(ll.y), LL(ll.z));
}

void rsDebug(const char *s, const long4 *c) {
    long4 ll = *c;
    ALOGD("%s {%lld, %lld, %lld, %lld}  0x%llx 0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w), LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w));
}

void rsDebug(const char *s, unsigned long l) {
    unsigned long long ll = l;
    ALOGD("%s %llu  0x%llx", s, ll, ll);
}

void rsDebug(const char *s, unsigned long long ll) {
    ALOGD("%s %llu  0x%llx", s, ll, ll);
}

void rsDebug(const char *s, const ulong2 *c) {
    ulong2 ll = *c;
    ALOGD("%s {%llu, %llu}  0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.x), LLu(ll.y));
}

void rsDebug(const char *s, const ulong3 *c) {
    ulong3 ll = *c;
    ALOGD("%s {%llu, %llu, %llu}  0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.x), LLu(ll.y), LLu(ll.z));
}

void rsDebug(const char *s, const ulong4 *c) {
    ulong4 ll = *c;
    ALOGD("%s {%llu, %llu, %llu, %llu}  0x%llx 0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w), LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w));
}

// FIXME: We need to export these function signatures for the compatibility
// library. The C++ name mangling that LLVM uses for ext_vector_type requires
// different versions for "long" vs. "long long". Note that the called
// functions are still using the appropriate 64-bit sizes.

#ifndef __LP64__
typedef long l2 __attribute__((ext_vector_type(2)));
typedef long l3 __attribute__((ext_vector_type(3)));
typedef long l4 __attribute__((ext_vector_type(4)));
typedef unsigned long ul2 __attribute__((ext_vector_type(2)));
typedef unsigned long ul3 __attribute__((ext_vector_type(3)));
typedef unsigned long ul4 __attribute__((ext_vector_type(4)));

void rsDebug(const char *s, const l2 *c) {
    long2 ll = *(const long2 *)c;
    ALOGD("%s {%lld, %lld}  0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.x), LL(ll.y));
}

void rsDebug(const char *s, const l3 *c) {
    long3 ll = *(const long3 *)c;
    ALOGD("%s {%lld, %lld, %lld}  0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.x), LL(ll.y), LL(ll.z));
}

void rsDebug(const char *s, const l4 *c) {
    long4 ll = *(const long4 *)c;
    ALOGD("%s {%lld, %lld, %lld, %lld}  0x%llx 0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w), LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w));
}

void rsDebug(const char *s, const ul2 *c) {
    ulong2 ll = *(const ulong2 *)c;
    ALOGD("%s {%llu, %llu}  0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.x), LLu(ll.y));
}

void rsDebug(const char *s, const ul3 *c) {
    ulong3 ll = *(const ulong3 *)c;
    ALOGD("%s {%llu, %llu, %llu}  0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.x), LLu(ll.y), LLu(ll.z));
}

void rsDebug(const char *s, const ul4 *c) {
    ulong4 ll = *(const ulong4 *)c;
    ALOGD("%s {%llu, %llu, %llu, %llu}  0x%llx 0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w), LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w));
}
#endif

void rsDebug(const char *s, const long2 ll) {
    ALOGD("%s {%lld, %lld}  0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.x), LL(ll.y));
}

void rsDebug(const char *s, const long3 ll) {
    ALOGD("%s {%lld, %lld, %lld}  0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.x), LL(ll.y), LL(ll.z));
}

void rsDebug(const char *s, const long4 ll) {
    ALOGD("%s {%lld, %lld, %lld, %lld}  0x%llx 0x%llx 0x%llx 0x%llx", s, LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w), LL(ll.x), LL(ll.y), LL(ll.z), LL(ll.w));
}

void rsDebug(const char *s, const ulong2 ll) {
    ALOGD("%s {%llu, %llu}  0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.x), LLu(ll.y));
}

void rsDebug(const char *s, const ulong3 ll) {
    ALOGD("%s {%llu, %llu, %llu}  0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.x), LLu(ll.y), LLu(ll.z));
}

void rsDebug(const char *s, const ulong4 ll) {
    ALOGD("%s {%llu, %llu, %llu, %llu}  0x%llx 0x%llx 0x%llx 0x%llx", s, LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w), LLu(ll.x), LLu(ll.y), LLu(ll.z), LLu(ll.w));
}

void rsDebug(const char *s, const void *p) {
    ALOGD("%s %p", s, p);
}

extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name) {
// TODO: remove
    return nullptr;
}
