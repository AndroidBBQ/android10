#include "rs_core.rsh"
#include "rs_structs.h"

#include "rsCpuCoreRuntime.h"

extern float __attribute__((overloadable)) rsFrac(float v) {
    int i = (int)floor(v);
    return fmin(v - i, 0x1.fffffep-1f);
}

/* Function declarations from libRS */
extern float4 __attribute__((overloadable)) convert_float4(uchar4 c);

/* Implementation of Core Runtime */

extern float4 rsUnpackColor8888(uchar4 c)
{
    return convert_float4(c) * 0.003921569f;
}


extern float __attribute__((overloadable)) rsClamp(float v, float l, float h) {
    return clamp(v, l, h);
}
extern char __attribute__((overloadable)) rsClamp(char v, char l, char h) {
    return clamp(v, l, h);
}
extern uchar __attribute__((overloadable)) rsClamp(uchar v, uchar l, uchar h) {
    return clamp(v, l, h);
}
extern short __attribute__((overloadable)) rsClamp(short v, short l, short h) {
    return clamp(v, l, h);
}
extern ushort __attribute__((overloadable)) rsClamp(ushort v, ushort l, ushort h) {
    return clamp(v, l, h);
}
extern int __attribute__((overloadable)) rsClamp(int v, int l, int h) {
    return clamp(v, l, h);
}
extern uint __attribute__((overloadable)) rsClamp(uint v, uint l, uint h) {
    return clamp(v, l, h);
}

extern int32_t __attribute__((overloadable)) rsAtomicCas(volatile int32_t *ptr, int32_t expectedValue, int32_t newValue) {
    return __sync_val_compare_and_swap(ptr, expectedValue, newValue);
}

extern uint32_t __attribute__((overloadable)) rsAtomicCas(volatile uint32_t *ptr, uint32_t expectedValue, uint32_t newValue) {
    return __sync_val_compare_and_swap(ptr, expectedValue, newValue);
}

extern int32_t __attribute__((overloadable)) rsAtomicInc(volatile int32_t *ptr) {
    return __sync_fetch_and_add(ptr, 1);
}

extern int32_t __attribute__((overloadable)) rsAtomicInc(volatile uint32_t *ptr) {
    return __sync_fetch_and_add(ptr, 1);
}

extern int32_t __attribute__((overloadable)) rsAtomicDec(volatile int32_t *ptr) {
    return __sync_fetch_and_sub(ptr, 1);
}

extern int32_t __attribute__((overloadable)) rsAtomicDec(volatile uint32_t *ptr) {
    return __sync_fetch_and_sub(ptr, 1);
}

extern int32_t __attribute__((overloadable)) rsAtomicAdd(volatile int32_t *ptr, int32_t value) {
    return __sync_fetch_and_add(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicAdd(volatile uint32_t *ptr, uint32_t value) {
    return __sync_fetch_and_add(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicSub(volatile int32_t *ptr, int32_t value) {
    return __sync_fetch_and_sub(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicSub(volatile uint32_t *ptr, uint32_t value) {
    return __sync_fetch_and_sub(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicAnd(volatile int32_t *ptr, int32_t value) {
    return __sync_fetch_and_and(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicAnd(volatile uint32_t *ptr, uint32_t value) {
    return __sync_fetch_and_and(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicOr(volatile int32_t *ptr, int32_t value) {
    return __sync_fetch_and_or(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicOr(volatile uint32_t *ptr, uint32_t value) {
    return __sync_fetch_and_or(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicXor(volatile int32_t *ptr, int32_t value) {
    return __sync_fetch_and_xor(ptr, value);
}

extern int32_t __attribute__((overloadable)) rsAtomicXor(volatile uint32_t *ptr, uint32_t value) {
    return __sync_fetch_and_xor(ptr, value);
}

extern uint32_t __attribute__((overloadable)) min(uint32_t, uint32_t);
extern int32_t __attribute__((overloadable)) min(int32_t, int32_t);
extern uint32_t __attribute__((overloadable)) max(uint32_t, uint32_t);
extern int32_t __attribute__((overloadable)) max(int32_t, int32_t);

extern uint32_t __attribute__((overloadable)) rsAtomicMin(volatile uint32_t *ptr, uint32_t value) {
    uint32_t prev, status;
    do {
        prev = *ptr;
        uint32_t n = min(value, prev);
        status = __sync_val_compare_and_swap(ptr, prev, n);
    } while (status != prev);
    return prev;
}

extern int32_t __attribute__((overloadable)) rsAtomicMin(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        int32_t n = min(value, prev);
        status = __sync_val_compare_and_swap(ptr, prev, n);
    } while (status != prev);
    return prev;
}

extern uint32_t __attribute__((overloadable)) rsAtomicMax(volatile uint32_t *ptr, uint32_t value) {
    uint32_t prev, status;
    do {
        prev = *ptr;
        uint32_t n = max(value, prev);
        status = __sync_val_compare_and_swap(ptr, prev, n);
    } while (status != prev);
    return prev;
}

extern int32_t __attribute__((overloadable)) rsAtomicMax(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        int32_t n = max(value, prev);
        status = __sync_val_compare_and_swap(ptr, prev, n);
    } while (status != prev);
    return prev;
}



extern int32_t rand();
#define RAND_MAX 0x7fffffff



extern float __attribute__((overloadable)) rsRand(float min, float max);/* {
    float r = (float)rand();
    r /= RAND_MAX;
    r = r * (max - min) + min;
    return r;
}
*/

extern float __attribute__((overloadable)) rsRand(float max) {
    return rsRand(0.f, max);
    //float r = (float)rand();
    //r *= max;
    //r /= RAND_MAX;
    //return r;
}

extern int __attribute__((overloadable)) rsRand(int max) {
    return (int)rsRand((float)max);
}

extern int __attribute__((overloadable)) rsRand(int min, int max) {
    return (int)rsRand((float)min, (float)max);
}

extern uint32_t __attribute__((overloadable)) rsGetArray0(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->current.array[0];
}

extern uint32_t __attribute__((overloadable)) rsGetArray1(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->current.array[1];
}

extern uint32_t __attribute__((overloadable)) rsGetArray2(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->current.array[2];
}

extern uint32_t __attribute__((overloadable)) rsGetArray3(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->current.array[3];
}

extern rs_allocation_cubemap_face __attribute__((overloadable)) rsGetFace(rs_kernel_context ctxt) {
    return (rs_allocation_cubemap_face)(((struct RsExpandKernelDriverInfo *)ctxt)->current.face);
}

extern uint32_t __attribute__((overloadable)) rsGetLod(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->current.lod;
}

extern uint32_t __attribute__((overloadable)) rsGetDimX(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.x;
}

extern uint32_t __attribute__((overloadable)) rsGetDimY(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.y;
}

extern uint32_t __attribute__((overloadable)) rsGetDimZ(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.z;
}

extern uint32_t __attribute__((overloadable)) rsGetDimArray0(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.array[0];
}

extern uint32_t __attribute__((overloadable)) rsGetDimArray1(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.array[1];
}

extern uint32_t __attribute__((overloadable)) rsGetDimArray2(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.array[2];
}

extern uint32_t __attribute__((overloadable)) rsGetDimArray3(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.array[3];
}

extern bool __attribute__((overloadable)) rsGetDimHasFaces(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.face != 0;
}

extern uint32_t __attribute__((overloadable)) rsGetDimLod(rs_kernel_context ctxt) {
    return ((struct RsExpandKernelDriverInfo *)ctxt)->dim.lod;
}

#define PRIM_DEBUG(T)                               \
extern void __attribute__((overloadable)) rsDebug(const char *, const T *);     \
void __attribute__((overloadable)) rsDebug(const char *txt, T val) {            \
    rsDebug(txt, &val);                                                         \
}

PRIM_DEBUG(char2)
PRIM_DEBUG(char3)
PRIM_DEBUG(char4)
PRIM_DEBUG(uchar2)
PRIM_DEBUG(uchar3)
PRIM_DEBUG(uchar4)
PRIM_DEBUG(short2)
PRIM_DEBUG(short3)
PRIM_DEBUG(short4)
PRIM_DEBUG(ushort2)
PRIM_DEBUG(ushort3)
PRIM_DEBUG(ushort4)
PRIM_DEBUG(int2)
PRIM_DEBUG(int3)
PRIM_DEBUG(int4)
PRIM_DEBUG(uint2)
PRIM_DEBUG(uint3)
PRIM_DEBUG(uint4)
PRIM_DEBUG(long2)
PRIM_DEBUG(long3)
PRIM_DEBUG(long4)
PRIM_DEBUG(ulong2)
PRIM_DEBUG(ulong3)
PRIM_DEBUG(ulong4)
PRIM_DEBUG(float2)
PRIM_DEBUG(float3)
PRIM_DEBUG(float4)
PRIM_DEBUG(double2)
PRIM_DEBUG(double3)
PRIM_DEBUG(double4)

#undef PRIM_DEBUG

// Convert the half values to float before handing off to the driver.  This
// eliminates the need in the driver to properly support the half datatype
// (either by adding compiler flags for half or link against compiler_rt).
// Also, pass the bit-equivalent ushort to be printed.
extern void __attribute__((overloadable)) rsDebug(const char *s, float f,
                                                  ushort us);
extern void __attribute__((overloadable)) rsDebug(const char *s, half h) {
    rsDebug(s, (float) h, *(ushort *) &h);
}

extern void __attribute__((overloadable)) rsDebug(const char *s,
                                                  const float2 *f,
                                                  const ushort2 *us);
extern void __attribute__((overloadable)) rsDebug(const char *s, half2 h2) {
    float2 f = convert_float2(h2);
    rsDebug(s, &f, (ushort2 *) &h2);
}

extern void __attribute__((overloadable)) rsDebug(const char *s,
                                                  const float3 *f,
                                                  const ushort3 *us);
extern void __attribute__((overloadable)) rsDebug(const char *s, half3 h3) {
    float3 f = convert_float3(h3);
    rsDebug(s, &f, (ushort3 *) &h3);
}

extern void __attribute__((overloadable)) rsDebug(const char *s,
                                                  const float4 *f,
                                                  const ushort4 *us);
extern void __attribute__((overloadable)) rsDebug(const char *s, half4 h4) {
    float4 f = convert_float4(h4);
    rsDebug(s, &f, (ushort4 *) &h4);
}
