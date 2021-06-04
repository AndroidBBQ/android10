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

#ifndef RS_HAL_H
#define RS_HAL_H

#include <rsInternalDefines.h>

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! Major version number of the driver.  This is used to ensure that
 * !! the driver (e.g., libRSDriver) is compatible with the shell
 * !! (i.e., libRS_internal) responsible for loading the driver.
 * !! There is no notion of backwards compatibility -- the driver and
 * !! the shell must agree on the major version number.
 * !!
 * !! The version number must change whenever there is a semantic change
 * !! to the HAL such as adding or removing an entry point or changing
 * !! the meaning of an entry point.  By convention it is monotonically
 * !! increasing across all branches (e.g., aosp/master and all internal
 * !! branches).
 * !!
 * !! Be very careful when merging or cherry picking between branches!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define RS_HAL_VERSION 200

/**
 * The interface for loading RenderScript drivers
 *
 * The startup sequence is
 *
 * 1: dlopen driver
 * 2: Query driver version with rsdHalQueryVersion() and verify
 *    that the driver (e.g., libRSDriver) is compatible with the shell
 *    (i.e., libRS_internal) responsible for loading the driver
 * 3: Fill in HAL pointer table with calls to rsdHalQueryHAL()
 * 4: Initialize the context with rsdHalInit()
 *
 * If any of these functions return false, the loading of the
 * driver will abort and the reference driver will be used.
 * rsdHalAbort() will be called to clean up any partially
 * allocated state.
 *
 * A driver should return FALSE for any conditions that will
 * prevent the driver from working normally.
 *
 *
 * If these are successful, the driver will be loaded and used
 * normally.  Teardown will use the normal
 * context->mHal.funcs.shutdown() path.  There will be no call
 * to rsdHalAbort().
 *
 *
 */


struct ANativeWindow;

namespace android {
namespace renderscript {

class Context;
class ObjectBase;
class Element;
class Type;
class Allocation;
class Script;
class ScriptKernelID;
class ScriptFieldID;
class ScriptMethodID;
class ScriptC;
class ScriptGroup;
class ScriptGroupBase;
class Path;
class Program;
class ProgramStore;
class ProgramRaster;
class ProgramVertex;
class ProgramFragment;
class Mesh;
class Sampler;
class FBOCache;

/**
 * Define the internal object types.  This ia a mirror of the
 * definition in rs_types.rsh except with the p value typed
 * correctly.
 *
 * p = pointer to internal object implementation
 * unused1, unused2, unused3 = reserved for ABI compatibility
 */

// RS_BASE_OBJ must have the same layout as _RS_OBJECT_DECL defined in
// script_api/rs_object_types.spec.
// TODO(jeanluc) Look at unifying.
#ifndef __LP64__
#define RS_BASE_OBJ(_t_) typedef struct { const _t_* p; } __attribute__((packed, aligned(4)))
#define RS_BASE_NULL_OBJ {0}
#else
#define RS_BASE_OBJ(_t_) typedef struct { const _t_* p; const void* unused1; const void* unused2; const void* unused3; }
#define RS_BASE_NULL_OBJ {0, 0, 0, 0}
#endif

RS_BASE_OBJ(ObjectBase) rs_object_base;
RS_BASE_OBJ(Element) rs_element;
RS_BASE_OBJ(Type) rs_type;
RS_BASE_OBJ(Allocation) rs_allocation;
RS_BASE_OBJ(Sampler) rs_sampler;
RS_BASE_OBJ(Script) rs_script;
RS_BASE_OBJ(ScriptGroup) rs_script_group;

#ifndef __LP64__
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_mesh;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_fragment;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_vertex;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_raster;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_store;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_font;
#endif // __LP64__


typedef void *(*RsHalSymbolLookupFunc)(void *usrptr, char const *symbolName);

/**
 * Script management functions
 */
typedef struct {
    int (*initGraphics)(const Context *);
    void (*shutdownGraphics)(const Context *);
    bool (*setSurface)(const Context *, uint32_t w, uint32_t h, RsNativeWindow);
    void (*swap)(const Context *);

    void (*shutdownDriver)(Context *);
    void (*setPriority)(const Context *, int32_t priority);

    void* (*allocRuntimeMem)(size_t size, uint32_t flags);
    void (*freeRuntimeMem)(void* ptr);

    struct {
        bool (*init)(const Context *rsc, ScriptC *s,
                     char const *resName,
                     char const *cacheDir,
                     uint8_t const *bitcode,
                     size_t bitcodeSize,
                     uint32_t flags);
        bool (*initIntrinsic)(const Context *rsc, Script *s,
                              RsScriptIntrinsicID iid,
                              Element *e);

        void (*invokeFunction)(const Context *rsc, Script *s,
                               uint32_t slot,
                               const void *params,
                               size_t paramLength);
        int (*invokeRoot)(const Context *rsc, Script *s);
        void (*invokeForEach)(const Context *rsc,
                              Script *s,
                              uint32_t slot,
                              const Allocation * ain,
                              Allocation * aout,
                              const void * usr,
                              size_t usrLen,
                              const RsScriptCall *sc);
        void (*invokeReduce)(const Context *rsc, Script *s,
                             uint32_t slot,
                             const Allocation ** ains, size_t inLen,
                             Allocation *aout,
                             const RsScriptCall *sc);
        void (*invokeInit)(const Context *rsc, Script *s);
        void (*invokeFreeChildren)(const Context *rsc, Script *s);

        void (*setGlobalVar)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             void *data,
                             size_t dataLength);
        void (*getGlobalVar)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             void *data,
                             size_t dataLength);
        void (*setGlobalVarWithElemDims)(const Context *rsc, const Script *s,
                                         uint32_t slot,
                                         void *data,
                                         size_t dataLength,
                                         const Element *e,
                                         const uint32_t *dims,
                                         size_t dimLength);
        void (*setGlobalBind)(const Context *rsc, const Script *s,
                              uint32_t slot,
                              Allocation *data);
        void (*setGlobalObj)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             ObjectBase *data);

        void (*destroy)(const Context *rsc, Script *s);
        void (*invokeForEachMulti)(const Context *rsc,
                                   Script *s,
                                   uint32_t slot,
                                   const Allocation ** ains,
                                   size_t inLen,
                                   Allocation * aout,
                                   const void * usr,
                                   size_t usrLen,
                                   const RsScriptCall *sc);
        void (*updateCachedObject)(const Context *rsc, const Script *, rs_script *obj);
    } script;

    struct {
        bool (*init)(const Context *rsc, Allocation *alloc, bool forceZero);
        bool (*initOem)(const Context *rsc, Allocation *alloc, bool forceZero, void *usrPtr);
        bool (*initAdapter)(const Context *rsc, Allocation *alloc);
        void (*destroy)(const Context *rsc, Allocation *alloc);
        uint32_t (*grallocBits)(const Context *rsc, Allocation *alloc);

        void (*resize)(const Context *rsc, const Allocation *alloc, const Type *newType,
                       bool zeroNew);
        void (*syncAll)(const Context *rsc, const Allocation *alloc, RsAllocationUsageType src);
        void (*markDirty)(const Context *rsc, const Allocation *alloc);

        void (*setSurface)(const Context *rsc, Allocation *alloc, ANativeWindow *sur);
        void (*ioSend)(const Context *rsc, Allocation *alloc);

        /**
         * A new gralloc buffer is in use. The pointers and strides in
         * mHal.drvState.lod[0-2] will be updated with the new values.
         *
         * The new gralloc handle is provided in mHal.state.nativeBuffer
         *
         */
        void (*ioReceive)(const Context *rsc, Allocation *alloc);

        void (*data1D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t lod, size_t count,
                       const void *data, size_t sizeBytes);
        void (*data2D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t lod,
                       RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                       const void *data, size_t sizeBytes, size_t stride);
        void (*data3D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                       uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes,
                       size_t stride);

        void (*read1D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t lod, size_t count,
                       void *data, size_t sizeBytes);
        void (*read2D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t lod,
                       RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                       void *data, size_t sizeBytes, size_t stride);
        void (*read3D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                       uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes,
                       size_t stride);

        // Lock and unlock make a 1D region of memory available to the CPU
        // for direct access by pointer.  Once unlock is called control is
        // returned to the SOC driver.
        void * (*lock1D)(const Context *rsc, const Allocation *alloc);
        void (*unlock1D)(const Context *rsc, const Allocation *alloc);

        // Allocation to allocation copies
        void (*allocData1D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstLod, size_t count,
                            const Allocation *srcAlloc, uint32_t srcXoff, uint32_t srcLod);
        void (*allocData2D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstYoff, uint32_t dstLod,
                            RsAllocationCubemapFace dstFace, uint32_t w, uint32_t h,
                            const Allocation *srcAlloc,
                            uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
                            RsAllocationCubemapFace srcFace);
        void (*allocData3D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                            uint32_t dstLod,
                            uint32_t w, uint32_t h, uint32_t d,
                            const Allocation *srcAlloc,
                            uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                            uint32_t srcLod);

        void (*elementData)(const Context *rsc, const Allocation *alloc,
                            uint32_t x, uint32_t y, uint32_t z,
                            const void *data, uint32_t elementOff, size_t sizeBytes);
        void (*elementRead)(const Context *rsc, const Allocation *alloc,
                            uint32_t x, uint32_t y, uint32_t z,
                            void *data, uint32_t elementOff, size_t sizeBytes);

        void (*generateMipmaps)(const Context *rsc, const Allocation *alloc);

        void (*updateCachedObject)(const Context *rsc, const Allocation *alloc, rs_allocation *obj);

        void (*adapterOffset)(const Context *rsc, const Allocation *alloc);

        void (*getPointer)(const Context *rsc, const Allocation *alloc,
                           uint32_t lod, RsAllocationCubemapFace face,
                           uint32_t z, uint32_t array);
#ifdef RS_COMPATIBILITY_LIB
        bool (*initStrided)(const Context *rsc, Allocation *alloc, bool forceZero, size_t requiredAlignment);
#endif
    } allocation;

    struct {
        bool (*init)(const Context *rsc, const ProgramStore *ps);
        void (*setActive)(const Context *rsc, const ProgramStore *ps);
        void (*destroy)(const Context *rsc, const ProgramStore *ps);
    } store;

    struct {
        bool (*init)(const Context *rsc, const ProgramRaster *ps);
        void (*setActive)(const Context *rsc, const ProgramRaster *ps);
        void (*destroy)(const Context *rsc, const ProgramRaster *ps);
    } raster;

    struct {
        bool (*init)(const Context *rsc, const ProgramVertex *pv,
                     const char* shader, size_t shaderLen,
                     const char** textureNames, size_t textureNamesCount,
                     const size_t *textureNamesLength);
        void (*setActive)(const Context *rsc, const ProgramVertex *pv);
        void (*destroy)(const Context *rsc, const ProgramVertex *pv);
    } vertex;

    struct {
        bool (*init)(const Context *rsc, const ProgramFragment *pf,
                     const char* shader, size_t shaderLen,
                     const char** textureNames, size_t textureNamesCount,
                     const size_t *textureNamesLength);
        void (*setActive)(const Context *rsc, const ProgramFragment *pf);
        void (*destroy)(const Context *rsc, const ProgramFragment *pf);
    } fragment;

    struct {
        bool (*init)(const Context *rsc, const Mesh *m);
        void (*draw)(const Context *rsc, const Mesh *m, uint32_t primIndex, uint32_t start, uint32_t len);
        void (*destroy)(const Context *rsc, const Mesh *m);
    } mesh;

    struct {
        bool (*init)(const Context *rsc, const Sampler *m);
        void (*destroy)(const Context *rsc, const Sampler *m);
        void (*updateCachedObject)(const Context *rsc, const Sampler *s, rs_sampler *obj);
    } sampler;

    struct {
        bool (*init)(const Context *rsc, const FBOCache *fb);
        void (*setActive)(const Context *rsc, const FBOCache *fb);
        void (*destroy)(const Context *rsc, const FBOCache *fb);
    } framebuffer;

    struct {
        bool (*init)(const Context *rsc, ScriptGroupBase *sg);
        void (*setInput)(const Context *rsc, const ScriptGroup *sg,
                         const ScriptKernelID *kid, Allocation *);
        void (*setOutput)(const Context *rsc, const ScriptGroup *sg,
                          const ScriptKernelID *kid, Allocation *);
        void (*execute)(const Context *rsc, const ScriptGroupBase *sg);
        void (*destroy)(const Context *rsc, const ScriptGroupBase *sg);
        void (*updateCachedObject)(const Context *rsc, const ScriptGroup *sg, rs_script_group *obj);
    } scriptgroup;

    struct {
        bool (*init)(const Context *rsc, const Type *m);
        void (*destroy)(const Context *rsc, const Type *m);
        void (*updateCachedObject)(const Context *rsc, const Type *s, rs_type *obj);
    } type;

    struct {
        bool (*init)(const Context *rsc, const Element *m);
        void (*destroy)(const Context *rsc, const Element *m);
        void (*updateCachedObject)(const Context *rsc, const Element *s, rs_element *obj);
    } element;

    void (*finish)(const Context *rsc);
} RsdHalFunctions;


enum RsHalInitEnums {
    RS_HAL_CORE_SHUTDOWN                                    = 1,
    RS_HAL_CORE_SET_PRIORITY                                = 2,
    RS_HAL_CORE_ALLOC_RUNTIME_MEM                           = 3,
    RS_HAL_CORE_FREE_RUNTIME_MEM                            = 4,
    RS_HAL_CORE_FINISH                                      = 5,

    RS_HAL_SCRIPT_INIT                                      = 1000,
    RS_HAL_SCRIPT_INIT_INTRINSIC                            = 1001,
    RS_HAL_SCRIPT_INVOKE_FUNCTION                           = 1002,
    RS_HAL_SCRIPT_INVOKE_ROOT                               = 1003,
    RS_HAL_SCRIPT_INVOKE_FOR_EACH                           = 1004,
    RS_HAL_SCRIPT_INVOKE_INIT                               = 1005,
    RS_HAL_SCRIPT_INVOKE_FREE_CHILDREN                      = 1006,
    RS_HAL_SCRIPT_SET_GLOBAL_VAR                            = 1007,
    RS_HAL_SCRIPT_GET_GLOBAL_VAR                            = 1008,
    RS_HAL_SCRIPT_SET_GLOBAL_VAR_WITH_ELEMENT_DIM           = 1009,
    RS_HAL_SCRIPT_SET_GLOBAL_BIND                           = 1010,
    RS_HAL_SCRIPT_SET_GLOBAL_OBJECT                         = 1011,
    RS_HAL_SCRIPT_DESTROY                                   = 1012,
    RS_HAL_SCRIPT_INVOKE_FOR_EACH_MULTI                     = 1013,
    RS_HAL_SCRIPT_UPDATE_CACHED_OBJECT                      = 1014,
    RS_HAL_SCRIPT_INVOKE_REDUCE                             = 1015,

    RS_HAL_ALLOCATION_INIT                                  = 2000,
    RS_HAL_ALLOCATION_INIT_ADAPTER                          = 2001,
    RS_HAL_ALLOCATION_DESTROY                               = 2002,
    RS_HAL_ALLOCATION_GET_GRALLOC_BITS                      = 2003,
    RS_HAL_ALLOCATION_RESIZE                                = 2004,
    RS_HAL_ALLOCATION_SYNC_ALL                              = 2005,
    RS_HAL_ALLOCATION_MARK_DIRTY                            = 2006,
    RS_HAL_ALLOCATION_SET_SURFACE                           = 2007,
    RS_HAL_ALLOCATION_IO_SEND                               = 2008,
    RS_HAL_ALLOCATION_IO_RECEIVE                            = 2009,
    RS_HAL_ALLOCATION_DATA_1D                               = 2010,
    RS_HAL_ALLOCATION_DATA_2D                               = 2011,
    RS_HAL_ALLOCATION_DATA_3D                               = 2012,
    RS_HAL_ALLOCATION_READ_1D                               = 2013,
    RS_HAL_ALLOCATION_READ_2D                               = 2014,
    RS_HAL_ALLOCATION_READ_3D                               = 2015,
    RS_HAL_ALLOCATION_LOCK_1D                               = 2016,
    RS_HAL_ALLOCATION_UNLOCK_1D                             = 2017,
    RS_HAL_ALLOCATION_COPY_1D                               = 2018,
    RS_HAL_ALLOCATION_COPY_2D                               = 2019,
    RS_HAL_ALLOCATION_COPY_3D                               = 2020,
    RS_HAL_ALLOCATION_ELEMENT_DATA                          = 2021,
    RS_HAL_ALLOCATION_ELEMENT_READ                          = 2022,
    RS_HAL_ALLOCATION_GENERATE_MIPMAPS                      = 2023,
    RS_HAL_ALLOCATION_UPDATE_CACHED_OBJECT                  = 2024,
    RS_HAL_ALLOCATION_ADAPTER_OFFSET                        = 2025,
    RS_HAL_ALLOCATION_INIT_OEM                              = 2026,
    RS_HAL_ALLOCATION_GET_POINTER                           = 2027,
#ifdef RS_COMPATIBILITY_LIB
    RS_HAL_ALLOCATION_INIT_STRIDED                          = 2999,
#endif

    RS_HAL_SAMPLER_INIT                                     = 3000,
    RS_HAL_SAMPLER_DESTROY                                  = 3001,
    RS_HAL_SAMPLER_UPDATE_CACHED_OBJECT                     = 3002,

    RS_HAL_TYPE_INIT                                        = 4000,
    RS_HAL_TYPE_DESTROY                                     = 4001,
    RS_HAL_TYPE_UPDATE_CACHED_OBJECT                        = 4002,

    RS_HAL_ELEMENT_INIT                                     = 5000,
    RS_HAL_ELEMENT_DESTROY                                  = 5001,
    RS_HAL_ELEMENT_UPDATE_CACHED_OBJECT                     = 5002,

    RS_HAL_SCRIPT_GROUP_INIT                                = 6000,
    RS_HAL_SCRIPT_GROUP_DESTROY                             = 6001,
    RS_HAL_SCRIPT_GROUP_UPDATE_CACHED_OBJECT                = 6002,
    RS_HAL_SCRIPT_GROUP_SET_INPUT                           = 6003,
    RS_HAL_SCRIPT_GROUP_SET_OUTPUT                          = 6004,
    RS_HAL_SCRIPT_GROUP_EXECUTE                             = 6005,



    RS_HAL_GRAPHICS_INIT                                    = 100001,
    RS_HAL_GRAPHICS_SHUTDOWN                                = 100002,
    RS_HAL_GRAPHICS_SWAP                                    = 100003,
    RS_HAL_GRAPHICS_SET_SURFACE                             = 100004,
    RS_HAL_GRAPHICS_RASTER_INIT                             = 101000,
    RS_HAL_GRAPHICS_RASTER_SET_ACTIVE                       = 101001,
    RS_HAL_GRAPHICS_RASTER_DESTROY                          = 101002,
    RS_HAL_GRAPHICS_VERTEX_INIT                             = 102000,
    RS_HAL_GRAPHICS_VERTEX_SET_ACTIVE                       = 102001,
    RS_HAL_GRAPHICS_VERTEX_DESTROY                          = 102002,
    RS_HAL_GRAPHICS_FRAGMENT_INIT                           = 103000,
    RS_HAL_GRAPHICS_FRAGMENT_SET_ACTIVE                     = 103001,
    RS_HAL_GRAPHICS_FRAGMENT_DESTROY                        = 103002,
    RS_HAL_GRAPHICS_MESH_INIT                               = 104000,
    RS_HAL_GRAPHICS_MESH_DRAW                               = 104001,
    RS_HAL_GRAPHICS_MESH_DESTROY                            = 104002,
    RS_HAL_GRAPHICS_FB_INIT                                 = 105000,
    RS_HAL_GRAPHICS_FB_SET_ACTIVE                           = 105001,
    RS_HAL_GRAPHICS_FB_DESTROY                              = 105002,
    RS_HAL_GRAPHICS_STORE_INIT                              = 106000,
    RS_HAL_GRAPHICS_STORE_SET_ACTIVE                        = 106001,
    RS_HAL_GRAPHICS_STORE_DESTROY                           = 106002,
};

} // namespace renderscript
} // namespace android

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the major version number of the driver.  The major
 * version should be the RS_HAL_VERSION against which the
 * driver was built
 *
 * The Minor version number is vendor specific
 *
 * The caller should ensure that *version_major is the same as
 * RS_HAL_VERSION -- i.e., that the driver (e.g., libRSDriver)
 * is compatible with the shell (i.e., libRS_internal) responsible
 * for loading the driver
 *
 * return: False will abort loading the driver, true indicates
 * success
 */
bool rsdHalQueryVersion(uint32_t *version_major, uint32_t *version_minor);


/**
 * Get an entry point in the driver HAL
 *
 * The driver should set the function pointer to its
 * implementation of the function.  If it does not have an entry
 * for an enum, its should set the function pointer to NULL
 *
 * Returning NULL is expected in cases during development as new
 * entry points are added that a driver may not understand.  If
 * the runtime receives a NULL it will decide if the function is
 * required and will either continue loading or abort as needed.
 *
 *
 * return: False will abort loading the driver, true indicates
 * success
 *
 */
bool rsdHalQueryHal(android::renderscript::RsHalInitEnums entry, void **fnPtr);


/**
 * Called to initialize the context for use with a driver.
 *
 * return: False will abort loading the driver, true indicates
 * success
 */
bool rsdHalInit(RsContext, uint32_t version_major, uint32_t version_minor);

/**
 * Called if one of the loading functions above returns false.
 * This is to clean up any resources allocated during an error
 * condition. If this path is called it means the normal
 * context->mHal.funcs.shutdown() will not be called.
 */
void rsdHalAbort(RsContext);

#ifdef __cplusplus
}
#endif

#endif
