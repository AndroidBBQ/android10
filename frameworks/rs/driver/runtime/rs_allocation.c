#include "rs_core.rsh"
#include "rs_structs.h"

// Opaque Allocation type operations
extern uint32_t __attribute__((overloadable))
    rsAllocationGetDimX(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    return alloc->mHal.drvState.lod[0].dimX;
}

extern uint32_t __attribute__((overloadable))
        rsAllocationGetDimY(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    return alloc->mHal.drvState.lod[0].dimY;
}

extern uint32_t __attribute__((overloadable))
        rsAllocationGetDimZ(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    return alloc->mHal.drvState.lod[0].dimZ;
}

extern uint32_t __attribute__((overloadable))
        rsAllocationGetDimLOD(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    return alloc->mHal.state.hasMipmaps;
}

extern uint32_t __attribute__((overloadable))
        rsAllocationGetDimFaces(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    return alloc->mHal.state.hasFaces;
}


extern rs_element __attribute__((overloadable))
        rsAllocationGetElement(rs_allocation a) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    if (alloc == NULL) {
        rs_element nullElem = RS_NULL_OBJ;
        return nullElem;
    }
    Type_t *type = (Type_t *)alloc->mHal.state.type;
    rs_element returnElem = {
        type->mHal.state.element
#ifdef __LP64__
        , 0, 0, 0
#endif
    };
    rs_element rs_retval = RS_NULL_OBJ;
    rsSetObject(&rs_retval, returnElem);
    return rs_retval;
}

// TODO: this needs to be optimized, obviously
static void local_memcpy(void* dst, const void* src, size_t size) {
    char* dst_c = (char*) dst;
    const char* src_c = (const char*) src;
    for (; size > 0; size--) {
        *dst_c++ = *src_c++;
    }
}

uint8_t*
rsOffset(rs_allocation a, uint32_t sizeOf, uint32_t x, uint32_t y,
         uint32_t z) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    uint8_t *p = (uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t stride = (uint32_t)alloc->mHal.drvState.lod[0].stride;
    const uint32_t dimY = alloc->mHal.drvState.lod[0].dimY;
    uint8_t *dp = &p[(sizeOf * x) + (y * stride) +
                     (z * stride * dimY)];
    return dp;
}

uint8_t*
rsOffsetNs(rs_allocation a, uint32_t x, uint32_t y, uint32_t z) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    uint8_t *p = (uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t stride = alloc->mHal.drvState.lod[0].stride;
    const uint32_t dimY = alloc->mHal.drvState.lod[0].dimY;
    const uint32_t sizeOf = alloc->mHal.state.elementSizeBytes;;
    uint8_t *dp = &p[(sizeOf * x) + (y * stride) +
                     (z * stride * dimY)];
    return dp;
}

#ifdef RS_DEBUG_RUNTIME
#define ELEMENT_AT(T)                                                   \
    extern void __attribute__((overloadable))                           \
        rsSetElementAt_##T(rs_allocation a, const T *val, uint32_t x);  \
    extern void __attribute__((overloadable))                           \
        rsSetElementAt_##T(rs_allocation a, const T *val, uint32_t x, uint32_t y); \
    extern void __attribute__((overloadable))                           \
        rsSetElementAt_##T(rs_allocation a, const T *val, uint32_t x, uint32_t y, uint32_t z); \
    extern void __attribute__((overloadable))                           \
        rsGetElementAt_##T(rs_allocation a, T *val, uint32_t x);  \
    extern void __attribute__((overloadable))                           \
        rsGetElementAt_##T(rs_allocation a, T *val, uint32_t x, uint32_t y); \
    extern void __attribute__((overloadable))                           \
        rsGetElementAt_##T(rs_allocation a, T *val, uint32_t x, uint32_t y, uint32_t z); \
                                                                        \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##T(rs_allocation a, T val, uint32_t x) {            \
        rsSetElementAt_##T(a, &val, x);                                 \
    }                                                                   \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##T(rs_allocation a, T val, uint32_t x, uint32_t y) { \
        rsSetElementAt_##T(a, &val, x, y);                              \
    }                                                                   \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##T(rs_allocation a, T val, uint32_t x, uint32_t y, uint32_t z) { \
        rsSetElementAt_##T(a, &val, x, y, z);                           \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsGetElementAt_##T(rs_allocation a, uint32_t x) {                   \
        T tmp;                                                          \
        rsGetElementAt_##T(a, &tmp, x);                                 \
        return tmp;                                                     \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsGetElementAt_##T(rs_allocation a, uint32_t x, uint32_t y) {       \
        T tmp;                                                          \
        rsGetElementAt_##T(a, &tmp, x, y);                              \
        return tmp;                                                     \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsGetElementAt_##T(rs_allocation a, uint32_t x, uint32_t y, uint32_t z) { \
        T tmp;                                                          \
        rsGetElementAt_##T(a, &tmp, x, y, z);                           \
        return tmp;                                                     \
    }
#else  // NOT RS_DEBUG_RUNTIME

#define SET_ELEMENT_AT_TYPE_IMPL(T, typename)                                    \
    void                                                                \
    rsSetElementAtImpl_##typename(rs_allocation a, typename val, uint32_t x,   \
                                  uint32_t y, uint32_t z);

#define GET_ELEMENT_AT_TYPE_IMPL(T, typename)                                \
    typename                                                            \
    rsGetElementAtImpl_##typename(rs_allocation a, uint32_t x, uint32_t y, \
                                  uint32_t z);

#define SET_ELEMENT_AT_TYPE_DEF(T, typename)                                    \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##typename(rs_allocation a, T val, uint32_t x) {     \
        rsSetElementAtImpl_##typename(a, (typename)val, x, 0, 0);              \
    }                                                                   \
                                                                        \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##typename(rs_allocation a, T val, uint32_t x,       \
                              uint32_t y) {                             \
        rsSetElementAtImpl_##typename(a, (typename)val, x, y, 0);              \
    }                                                                   \
                                                                        \
    extern void __attribute__((overloadable))                           \
    rsSetElementAt_##typename(rs_allocation a, T val, uint32_t x, uint32_t y, \
                              uint32_t z) {                             \
        rsSetElementAtImpl_##typename(a, (typename)val, x, y, z);              \
    }

#define GET_ELEMENT_AT_TYPE_DEF(T, typename)                                \
    extern typename __attribute__((overloadable))                       \
    rsGetElementAt_##typename(rs_allocation a, uint32_t x) {            \
        return (typename)rsGetElementAtImpl_##typename(a, x, 0, 0);     \
    }                                                                   \
                                                                        \
    extern typename __attribute__((overloadable))                       \
    rsGetElementAt_##typename(rs_allocation a, uint32_t x, uint32_t y) { \
        return (typename)rsGetElementAtImpl_##typename(a, x, y, 0);     \
    }                                                                   \
                                                                        \
    extern typename __attribute__((overloadable))                       \
    rsGetElementAt_##typename(rs_allocation a, uint32_t x, uint32_t y,  \
                              uint32_t z) {                             \
        return (typename)rsGetElementAtImpl_##typename(a, x, y, z);     \
    }

#define SET_ELEMENT_AT(T) SET_ELEMENT_AT_TYPE_IMPL(T, T) \
    SET_ELEMENT_AT_TYPE_DEF(T, T)
#define GET_ELEMENT_AT(T) GET_ELEMENT_AT_TYPE_IMPL(T, T) \
    GET_ELEMENT_AT_TYPE_DEF(T, T)

#define ELEMENT_AT(T)                           \
    SET_ELEMENT_AT(T)                           \
    GET_ELEMENT_AT(T)

#endif // RS_DEBUG_RUNTIME

extern const void * __attribute__((overloadable))
        rsGetElementAt(rs_allocation a, uint32_t x) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    return &p[eSize * x];
}

extern const void * __attribute__((overloadable))
        rsGetElementAt(rs_allocation a, uint32_t x, uint32_t y) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    const uint32_t stride = alloc->mHal.drvState.lod[0].stride;
    return &p[(eSize * x) + (y * stride)];
}

extern const void * __attribute__((overloadable))
        rsGetElementAt(rs_allocation a, uint32_t x, uint32_t y, uint32_t z) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    const uint32_t stride = alloc->mHal.drvState.lod[0].stride;
    const uint32_t dimY = alloc->mHal.drvState.lod[0].dimY;
    return &p[(eSize * x) + (y * stride) + (z * stride * dimY)];
}
extern void __attribute__((overloadable))
        rsSetElementAt(rs_allocation a, void* ptr, uint32_t x) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    local_memcpy((void*)&p[eSize * x], ptr, eSize);
}

extern void __attribute__((overloadable))
        rsSetElementAt(rs_allocation a, void* ptr, uint32_t x, uint32_t y) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    const uint32_t stride = alloc->mHal.drvState.lod[0].stride;
    local_memcpy((void*)&p[(eSize * x) + (y * stride)], ptr, eSize);
}

extern void __attribute__((overloadable))
        rsSetElementAt(rs_allocation a, void* ptr, uint32_t x, uint32_t y, uint32_t z) {
    Allocation_t *alloc = (Allocation_t *)a.p;
    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr;
    const uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    const uint32_t stride = alloc->mHal.drvState.lod[0].stride;
    const uint32_t dimY = alloc->mHal.drvState.lod[0].dimY;
    local_memcpy((void*)&p[(eSize * x) + (y * stride) + (z * stride * dimY)], ptr, eSize);
}

ELEMENT_AT(char)
ELEMENT_AT(char2)
ELEMENT_AT(char3)
ELEMENT_AT(char4)
ELEMENT_AT(uchar)
ELEMENT_AT(uchar2)
ELEMENT_AT(uchar3)
ELEMENT_AT(uchar4)
ELEMENT_AT(short)
ELEMENT_AT(short2)
ELEMENT_AT(short3)
ELEMENT_AT(short4)
ELEMENT_AT(ushort)
ELEMENT_AT(ushort2)
ELEMENT_AT(ushort3)
ELEMENT_AT(ushort4)
ELEMENT_AT(int)
ELEMENT_AT(int2)
ELEMENT_AT(int3)
ELEMENT_AT(int4)
ELEMENT_AT(uint)
ELEMENT_AT(uint2)
ELEMENT_AT(uint3)
ELEMENT_AT(uint4)
ELEMENT_AT(long)
ELEMENT_AT(long2)
ELEMENT_AT(long3)
ELEMENT_AT(long4)
ELEMENT_AT(ulong)
ELEMENT_AT(ulong2)
ELEMENT_AT(ulong3)
ELEMENT_AT(ulong4)
ELEMENT_AT(half)
ELEMENT_AT(half2)
ELEMENT_AT(half3)
ELEMENT_AT(half4)
ELEMENT_AT(float)
ELEMENT_AT(float2)
ELEMENT_AT(float3)
ELEMENT_AT(float4)
ELEMENT_AT(double)
ELEMENT_AT(double2)
ELEMENT_AT(double3)
ELEMENT_AT(double4)

typedef unsigned long long ull;
typedef unsigned long long ull2 __attribute__((ext_vector_type(2)));
typedef unsigned long long ull3 __attribute__((ext_vector_type(3)));
typedef unsigned long long ull4 __attribute__((ext_vector_type(4)));

#ifndef RS_DEBUG_RUNTIME
SET_ELEMENT_AT_TYPE_IMPL(ull, ulong)
SET_ELEMENT_AT_TYPE_IMPL(ull2, ulong2)
SET_ELEMENT_AT_TYPE_IMPL(ull3, ulong3)
SET_ELEMENT_AT_TYPE_IMPL(ull4, ulong4)

#undef SET_ELEMENT_AT_TYPE_DEF
#undef GET_ELEMENT_AT_TYPE_DEF
#undef SET_ELEMENT_AT_TYPE_IMPL
#undef GET_ELEMENT_AT_TYPE_IMPL
#undef ELEMENT_AT_TYPE
#endif

#undef ELEMENT_AT


extern uchar __attribute__((overloadable))
        rsGetElementAtYuv_uchar_Y(rs_allocation a, uint32_t x, uint32_t y) {
    return rsGetElementAt_uchar(a, x, y);
}

extern uchar __attribute__((overloadable))
        rsGetElementAtYuv_uchar_U(rs_allocation a, uint32_t x, uint32_t y) {

    Allocation_t *alloc = (Allocation_t *)a.p;

    const size_t cstep = alloc->mHal.drvState.yuv.step;
    const size_t shift = alloc->mHal.drvState.yuv.shift;
    const size_t stride = alloc->mHal.drvState.lod[1].stride;

    const uchar *pin = (const uchar *)alloc->mHal.drvState.lod[1].mallocPtr;

    return pin[((x >> shift) * cstep) + ((y >> shift) * stride)];
}

extern uchar __attribute__((overloadable))
        rsGetElementAtYuv_uchar_V(rs_allocation a, uint32_t x, uint32_t y) {

    Allocation_t *alloc = (Allocation_t *)a.p;

    const size_t cstep = alloc->mHal.drvState.yuv.step;
    const size_t shift = alloc->mHal.drvState.yuv.shift;
    const size_t stride = alloc->mHal.drvState.lod[2].stride;

    const uchar *pin = (const uchar *)alloc->mHal.drvState.lod[2].mallocPtr;

    return pin[((x >> shift) * cstep) + ((y >> shift) * stride)];
}

#define VOP_IMPL(T)                                                          \
    extern void __rsAllocationVStoreXImpl_##T(rs_allocation a, const T val, uint32_t x, uint32_t y, uint32_t z); \
    extern T __rsAllocationVLoadXImpl_##T(rs_allocation a, uint32_t x, uint32_t y, uint32_t z);

#define VOP_DEF(T)                                                      \
    extern void __attribute__((overloadable))                           \
    rsAllocationVStoreX_##T(rs_allocation a, T val, uint32_t x) {       \
        __rsAllocationVStoreXImpl_##T(a, val, x, 0, 0);                 \
    }                                                                   \
    extern void __attribute__((overloadable))                           \
    rsAllocationVStoreX_##T(rs_allocation a, T val, uint32_t x, uint32_t y) { \
        __rsAllocationVStoreXImpl_##T(a, val, x, y, 0);                 \
    }                                                                   \
    extern void __attribute__((overloadable))                           \
    rsAllocationVStoreX_##T(rs_allocation a, T val, uint32_t x, uint32_t y, uint32_t z) { \
        __rsAllocationVStoreXImpl_##T(a, val, x, y, z);                 \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsAllocationVLoadX_##T(rs_allocation a, uint32_t x) {               \
        return __rsAllocationVLoadXImpl_##T(a, x, 0, 0);                \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsAllocationVLoadX_##T(rs_allocation a, uint32_t x, uint32_t y) {   \
        return __rsAllocationVLoadXImpl_##T(a, x, y, 0);                \
    }                                                                   \
    extern T __attribute__((overloadable))                              \
    rsAllocationVLoadX_##T(rs_allocation a, uint32_t x, uint32_t y, uint32_t z) { \
        return __rsAllocationVLoadXImpl_##T(a, x, y, z);                \
    }

#define VOP(T) VOP_IMPL(T) \
    VOP_DEF(T)

VOP(char2)
VOP(char3)
VOP(char4)
VOP(uchar2)
VOP(uchar3)
VOP(uchar4)
VOP(short2)
VOP(short3)
VOP(short4)
VOP(ushort2)
VOP(ushort3)
VOP(ushort4)
VOP(int2)
VOP(int3)
VOP(int4)
VOP(uint2)
VOP(uint3)
VOP(uint4)
VOP(long2)
VOP(long3)
VOP(long4)
VOP(ulong2)
VOP(ulong3)
VOP(ulong4)
VOP(float2)
VOP(float3)
VOP(float4)
VOP(double2)
VOP(double3)
VOP(double4)

#undef VOP_IMPL
#undef VOP_DEF
#undef VOP

static const rs_element kInvalidElement = RS_NULL_OBJ;

extern rs_element __attribute__((overloadable)) rsCreateElement(
        int32_t dt, int32_t dk, bool isNormalized, uint32_t vecSize);

extern rs_type __attribute__((overloadable)) rsCreateType(
    rs_element element, uint32_t dimX, uint32_t dimY, uint32_t dimZ,
    bool mipmaps, bool faces, rs_yuv_format yuv_format);

extern rs_allocation __attribute__((overloadable)) rsCreateAllocation(
        rs_type type, rs_allocation_mipmap_control mipmaps, uint32_t usages,
        void *ptr);

rs_element __attribute__((overloadable)) rsCreateElement(
        rs_data_type data_type) {

    switch (data_type) {
        case RS_TYPE_BOOLEAN:
        case RS_TYPE_FLOAT_16:
        case RS_TYPE_FLOAT_32:
        case RS_TYPE_FLOAT_64:
        case RS_TYPE_SIGNED_8:
        case RS_TYPE_SIGNED_16:
        case RS_TYPE_SIGNED_32:
        case RS_TYPE_SIGNED_64:
        case RS_TYPE_UNSIGNED_8:
        case RS_TYPE_UNSIGNED_16:
        case RS_TYPE_UNSIGNED_32:
        case RS_TYPE_UNSIGNED_64:
        case RS_TYPE_MATRIX_4X4:
        case RS_TYPE_MATRIX_3X3:
        case RS_TYPE_MATRIX_2X2:
        case RS_TYPE_ELEMENT:
        case RS_TYPE_TYPE:
        case RS_TYPE_ALLOCATION:
        case RS_TYPE_SCRIPT:
            return rsCreateElement(data_type, RS_KIND_USER, false, 1);
        default:
            rsDebug("Invalid data_type", data_type);
            return kInvalidElement;
    }
}

rs_element __attribute__((overloadable)) rsCreateVectorElement(
        rs_data_type data_type, uint32_t vector_width) {
    if (vector_width < 2 || vector_width > 4) {
        rsDebug("Invalid vector_width", vector_width);
        return kInvalidElement;
    }
    switch (data_type) {
        case RS_TYPE_BOOLEAN:
        case RS_TYPE_FLOAT_16:
        case RS_TYPE_FLOAT_32:
        case RS_TYPE_FLOAT_64:
        case RS_TYPE_SIGNED_8:
        case RS_TYPE_SIGNED_16:
        case RS_TYPE_SIGNED_32:
        case RS_TYPE_SIGNED_64:
        case RS_TYPE_UNSIGNED_8:
        case RS_TYPE_UNSIGNED_16:
        case RS_TYPE_UNSIGNED_32:
        case RS_TYPE_UNSIGNED_64:
            return rsCreateElement(data_type, RS_KIND_USER, false,
                                   vector_width);
        default:
            rsDebug("Invalid data_type for vector element", data_type);
            return kInvalidElement;
    }
}

rs_element __attribute__((overloadable)) rsCreatePixelElement(
        rs_data_type data_type, rs_data_kind data_kind) {
    if (data_type != RS_TYPE_UNSIGNED_8 &&
        data_type != RS_TYPE_UNSIGNED_16 &&
        data_type != RS_TYPE_UNSIGNED_5_6_5 &&
        data_type != RS_TYPE_UNSIGNED_4_4_4_4 &&
        data_type != RS_TYPE_UNSIGNED_5_5_5_1) {

        rsDebug("Invalid data_type for pixel element", data_type);
        return kInvalidElement;
    }
    if (data_kind != RS_KIND_PIXEL_L &&
        data_kind != RS_KIND_PIXEL_A &&
        data_kind != RS_KIND_PIXEL_LA &&
        data_kind != RS_KIND_PIXEL_RGB &&
        data_kind != RS_KIND_PIXEL_RGBA &&
        data_kind != RS_KIND_PIXEL_DEPTH &&
        data_kind != RS_KIND_PIXEL_YUV) {

        rsDebug("Invalid data_kind for pixel element", data_type);
        return kInvalidElement;
    }
    if (data_type == RS_TYPE_UNSIGNED_5_6_5 && data_kind != RS_KIND_PIXEL_RGB) {
        rsDebug("Bad data_type and data_kind combo", data_type, data_kind);
        return kInvalidElement;
    }
    if (data_type == RS_TYPE_UNSIGNED_5_5_5_1 &&
        data_kind != RS_KIND_PIXEL_RGBA) {

        rsDebug("Bad data_type and data_kind combo", data_type, data_kind);
        return kInvalidElement;
    }
    if (data_type == RS_TYPE_UNSIGNED_4_4_4_4 &&
        data_kind != RS_KIND_PIXEL_RGBA) {

        rsDebug("Bad data_type and data_kind combo", data_type, data_kind);
        return kInvalidElement;
    }
    if (data_type == RS_TYPE_UNSIGNED_16 && data_kind != RS_KIND_PIXEL_DEPTH) {
        rsDebug("Bad data_type and data_kind combo", data_type, data_kind);
        return kInvalidElement;
    }

    int vector_width = 1;
    switch (data_kind) {
        case RS_KIND_PIXEL_LA:
            vector_width = 2;
            break;
        case RS_KIND_PIXEL_RGB:
            vector_width = 3;
            break;
        case RS_KIND_PIXEL_RGBA:
            vector_width = 4;
            break;
        case RS_KIND_PIXEL_DEPTH:
            vector_width = 2;
            break;
        default:
            break;
    }

    return rsCreateElement(data_type, data_kind, true, vector_width);
}

rs_type __attribute__((overloadable)) rsCreateType(rs_element element,
                                                   uint32_t dimX, uint32_t dimY,
                                                   uint32_t dimZ) {
    return rsCreateType(element, dimX, dimY, dimZ, false, false, RS_YUV_NONE);
}

rs_type __attribute__((overloadable)) rsCreateType(rs_element element,
                                                   uint32_t dimX,
                                                   uint32_t dimY) {
    return rsCreateType(element, dimX, dimY, 0, false, false, RS_YUV_NONE);
}

rs_type __attribute__((overloadable)) rsCreateType(rs_element element,
                                                   uint32_t dimX) {
    return rsCreateType(element, dimX, 0, 0, false, false, RS_YUV_NONE);
}

rs_allocation __attribute__((overloadable)) rsCreateAllocation(rs_type type,
                                                               uint32_t usage) {
    return rsCreateAllocation(type, RS_ALLOCATION_MIPMAP_NONE, usage, NULL);
}

rs_allocation __attribute__((overloadable)) rsCreateAllocation(rs_type type) {
    return rsCreateAllocation(type, RS_ALLOCATION_MIPMAP_NONE,
                              RS_ALLOCATION_USAGE_SCRIPT, NULL);
}
