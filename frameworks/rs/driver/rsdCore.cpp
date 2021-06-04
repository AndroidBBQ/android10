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

#include "../cpu_ref/rsd_cpu.h"

#include "rsdCore.h"
#include "rsdAllocation.h"
#include "rsdBcc.h"
#include "rsdElement.h"
#include "rsdType.h"
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    #include "rsdGL.h"
    #include "rsdProgramStore.h"
    #include "rsdProgramRaster.h"
    #include "rsdProgramVertex.h"
    #include "rsdProgramFragment.h"
    #include "rsdMesh.h"
    #include "rsdFrameBuffer.h"
#else
    #include <dlfcn.h>
#endif
#include "rsdSampler.h"
#include "rsdScriptGroup.h"

#include <malloc.h>
#include "rsContext.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/syscall.h>
#include <string.h>

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::RsHalInitEnums;
using android::renderscript::RsdCpuReference;
using android::renderscript::Script;

static void Shutdown(Context *rsc);
static void SetPriority(const Context *rsc, int32_t priority);

#ifndef RS_COMPATIBILITY_LIB
    #define NATIVE_FUNC(a) a
#else
    #define NATIVE_FUNC(a) nullptr
#endif

namespace android {
namespace renderscript {

extern "C" bool rsdHalQueryHal(RsHalInitEnums entry, void **fnPtr) {
    switch(entry) {
    case RS_HAL_CORE_SHUTDOWN:
        fnPtr[0] = (void *)Shutdown; break;
    case RS_HAL_CORE_SET_PRIORITY:
        fnPtr[0] = (void *)SetPriority; break;
    case RS_HAL_CORE_ALLOC_RUNTIME_MEM:
        fnPtr[0] = (void *)rsdAllocRuntimeMem; break;
    case RS_HAL_CORE_FREE_RUNTIME_MEM:
        fnPtr[0] = (void *)rsdFreeRuntimeMem; break;
    case RS_HAL_CORE_FINISH:
        fnPtr[0] = (void *)nullptr; break;

    case RS_HAL_SCRIPT_INIT:
        fnPtr[0] = (void *)rsdScriptInit; break;
    case RS_HAL_SCRIPT_INIT_INTRINSIC:
        fnPtr[0] = (void *)rsdInitIntrinsic; break;
    case RS_HAL_SCRIPT_INVOKE_FUNCTION:
        fnPtr[0] = (void *)rsdScriptInvokeFunction; break;
    case RS_HAL_SCRIPT_INVOKE_ROOT:
        fnPtr[0] = (void *)rsdScriptInvokeRoot; break;
    case RS_HAL_SCRIPT_INVOKE_FOR_EACH:
        fnPtr[0] = (void *)rsdScriptInvokeForEach; break;
    case RS_HAL_SCRIPT_INVOKE_INIT:
        fnPtr[0] = (void *)rsdScriptInvokeInit; break;
    case RS_HAL_SCRIPT_INVOKE_FREE_CHILDREN:
        fnPtr[0] = (void *)rsdScriptInvokeFreeChildren; break;
    case RS_HAL_SCRIPT_SET_GLOBAL_VAR:
        fnPtr[0] = (void *)rsdScriptSetGlobalVar; break;
    case RS_HAL_SCRIPT_GET_GLOBAL_VAR:
        fnPtr[0] = (void *)rsdScriptGetGlobalVar; break;
    case RS_HAL_SCRIPT_SET_GLOBAL_VAR_WITH_ELEMENT_DIM:
        fnPtr[0] = (void *)rsdScriptSetGlobalVarWithElemDims; break;
    case RS_HAL_SCRIPT_SET_GLOBAL_BIND:
        fnPtr[0] = (void *)rsdScriptSetGlobalBind; break;
    case RS_HAL_SCRIPT_SET_GLOBAL_OBJECT:
        fnPtr[0] = (void *)rsdScriptSetGlobalObj; break;
    case RS_HAL_SCRIPT_DESTROY:
        fnPtr[0] = (void *)rsdScriptDestroy; break;
    case RS_HAL_SCRIPT_INVOKE_FOR_EACH_MULTI:
        fnPtr[0] = (void *)rsdScriptInvokeForEachMulti; break;
    case RS_HAL_SCRIPT_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)rsdScriptUpdateCachedObject; break;
    case RS_HAL_SCRIPT_INVOKE_REDUCE:
        fnPtr[0] = (void *)rsdScriptInvokeReduce; break;

    case RS_HAL_ALLOCATION_INIT:
        fnPtr[0] = (void *)rsdAllocationInit; break;
    case RS_HAL_ALLOCATION_INIT_OEM:
        fnPtr[0] = (void *)nullptr; break;
    case RS_HAL_ALLOCATION_INIT_ADAPTER:
        fnPtr[0] = (void *)rsdAllocationAdapterInit; break;
    case RS_HAL_ALLOCATION_DESTROY:
        fnPtr[0] = (void *)rsdAllocationDestroy; break;
    case RS_HAL_ALLOCATION_GET_GRALLOC_BITS:
        fnPtr[0] = (void *)rsdAllocationGrallocBits; break;
    case RS_HAL_ALLOCATION_RESIZE:
        fnPtr[0] = (void *)rsdAllocationResize; break;
    case RS_HAL_ALLOCATION_SYNC_ALL:
        fnPtr[0] = (void *)rsdAllocationSyncAll; break;
    case RS_HAL_ALLOCATION_MARK_DIRTY:
        fnPtr[0] = (void *)rsdAllocationMarkDirty; break;
    case RS_HAL_ALLOCATION_SET_SURFACE:
        fnPtr[0] = (void *)NATIVE_FUNC(rsdAllocationSetSurface); break;
    case RS_HAL_ALLOCATION_IO_SEND:
        fnPtr[0] = (void *)NATIVE_FUNC(rsdAllocationIoSend); break;
    case RS_HAL_ALLOCATION_IO_RECEIVE:
        fnPtr[0] = (void *)NATIVE_FUNC(rsdAllocationIoReceive); break;
    case RS_HAL_ALLOCATION_DATA_1D:
        fnPtr[0] = (void *)rsdAllocationData1D; break;
    case RS_HAL_ALLOCATION_DATA_2D:
        fnPtr[0] = (void *)rsdAllocationData2D; break;
    case RS_HAL_ALLOCATION_DATA_3D:
        fnPtr[0] = (void *)rsdAllocationData3D; break;
    case RS_HAL_ALLOCATION_READ_1D:
        fnPtr[0] = (void *)rsdAllocationRead1D; break;
    case RS_HAL_ALLOCATION_READ_2D:
        fnPtr[0] = (void *)rsdAllocationRead2D; break;
    case RS_HAL_ALLOCATION_READ_3D:
        fnPtr[0] = (void *)rsdAllocationRead3D; break;
    case RS_HAL_ALLOCATION_LOCK_1D:
        fnPtr[0] = (void *)rsdAllocationLock1D; break;
    case RS_HAL_ALLOCATION_UNLOCK_1D:
        fnPtr[0] = (void *)rsdAllocationUnlock1D; break;
    case RS_HAL_ALLOCATION_COPY_1D:
        fnPtr[0] = (void *)rsdAllocationData1D_alloc; break;
    case RS_HAL_ALLOCATION_COPY_2D:
        fnPtr[0] = (void *)rsdAllocationData2D_alloc; break;
    case RS_HAL_ALLOCATION_COPY_3D:
        fnPtr[0] = (void *)rsdAllocationData3D_alloc; break;
    case RS_HAL_ALLOCATION_ELEMENT_DATA:
        fnPtr[0] = (void *)rsdAllocationElementData; break;
    case RS_HAL_ALLOCATION_ELEMENT_READ:
        fnPtr[0] = (void *)rsdAllocationElementRead; break;
    case RS_HAL_ALLOCATION_GENERATE_MIPMAPS:
        fnPtr[0] = (void *)rsdAllocationGenerateMipmaps; break;
    case RS_HAL_ALLOCATION_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)rsdAllocationUpdateCachedObject; break;
    case RS_HAL_ALLOCATION_ADAPTER_OFFSET:
        fnPtr[0] = (void *)rsdAllocationAdapterOffset; break;
    case RS_HAL_ALLOCATION_GET_POINTER:
        fnPtr[0] = (void *)nullptr; break;
#ifdef RS_COMPATIBILITY_LIB
    case RS_HAL_ALLOCATION_INIT_STRIDED:
        fnPtr[0] = (void *)rsdAllocationInitStrided; break;
#endif

    case RS_HAL_SAMPLER_INIT:
        fnPtr[0] = (void *)rsdSamplerInit; break;
    case RS_HAL_SAMPLER_DESTROY:
        fnPtr[0] = (void *)rsdSamplerDestroy; break;
    case RS_HAL_SAMPLER_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)rsdSamplerUpdateCachedObject; break;

    case RS_HAL_TYPE_INIT:
        fnPtr[0] = (void *)rsdTypeInit; break;
    case RS_HAL_TYPE_DESTROY:
        fnPtr[0] = (void *)rsdTypeDestroy; break;
    case RS_HAL_TYPE_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)rsdTypeUpdateCachedObject; break;

    case RS_HAL_ELEMENT_INIT:
        fnPtr[0] = (void *)rsdElementInit; break;
    case RS_HAL_ELEMENT_DESTROY:
        fnPtr[0] = (void *)rsdElementDestroy; break;
    case RS_HAL_ELEMENT_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)rsdElementUpdateCachedObject; break;

    case RS_HAL_SCRIPT_GROUP_INIT:
        fnPtr[0] = (void *)rsdScriptGroupInit; break;
    case RS_HAL_SCRIPT_GROUP_DESTROY:
        fnPtr[0] = (void *)rsdScriptGroupDestroy; break;
    case RS_HAL_SCRIPT_GROUP_UPDATE_CACHED_OBJECT:
        fnPtr[0] = (void *)nullptr; break;
    case RS_HAL_SCRIPT_GROUP_SET_INPUT:
        fnPtr[0] = (void *)rsdScriptGroupSetInput; break;
    case RS_HAL_SCRIPT_GROUP_SET_OUTPUT:
        fnPtr[0] = (void *)rsdScriptGroupSetOutput; break;
    case RS_HAL_SCRIPT_GROUP_EXECUTE:
        fnPtr[0] = (void *)rsdScriptGroupExecute; break;



    // Functions below this point are for the legacy graphics api,
    // vendor drivers are NOT expected to implement these.  They will never be called
    // for an external driver.
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    case RS_HAL_GRAPHICS_INIT:
        fnPtr[0] = (void *)rsdGLInit; break;
    case RS_HAL_GRAPHICS_SHUTDOWN:
        fnPtr[0] = (void *)rsdGLShutdown; break;
    case RS_HAL_GRAPHICS_SWAP:
        fnPtr[0] = (void *)rsdGLSwap; break;
    case RS_HAL_GRAPHICS_SET_SURFACE:
        fnPtr[0] = (void *)rsdGLSetSurface; break;
    case RS_HAL_GRAPHICS_RASTER_INIT:
        fnPtr[0] = (void *)rsdProgramRasterInit; break;
    case RS_HAL_GRAPHICS_RASTER_SET_ACTIVE:
        fnPtr[0] = (void *)rsdProgramRasterSetActive; break;
    case RS_HAL_GRAPHICS_RASTER_DESTROY:
        fnPtr[0] = (void *)rsdProgramRasterDestroy; break;
    case RS_HAL_GRAPHICS_VERTEX_INIT:
        fnPtr[0] = (void *)rsdProgramVertexInit; break;
    case RS_HAL_GRAPHICS_VERTEX_SET_ACTIVE:
        fnPtr[0] = (void *)rsdProgramVertexSetActive; break;
    case RS_HAL_GRAPHICS_VERTEX_DESTROY:
        fnPtr[0] = (void *)rsdProgramVertexDestroy; break;
    case RS_HAL_GRAPHICS_FRAGMENT_INIT:
        fnPtr[0] = (void *)rsdProgramFragmentInit; break;
    case RS_HAL_GRAPHICS_FRAGMENT_SET_ACTIVE:
        fnPtr[0] = (void *)rsdProgramFragmentSetActive; break;
    case RS_HAL_GRAPHICS_FRAGMENT_DESTROY:
        fnPtr[0] = (void *)rsdProgramFragmentDestroy; break;
    case RS_HAL_GRAPHICS_MESH_INIT:
        fnPtr[0] = (void *)rsdMeshInit; break;
    case RS_HAL_GRAPHICS_MESH_DRAW:
        fnPtr[0] = (void *)rsdMeshDraw; break;
    case RS_HAL_GRAPHICS_MESH_DESTROY:
        fnPtr[0] = (void *)rsdMeshDestroy; break;
    case RS_HAL_GRAPHICS_FB_INIT:
        fnPtr[0] = (void *)rsdFrameBufferInit; break;
    case RS_HAL_GRAPHICS_FB_SET_ACTIVE:
        fnPtr[0] = (void *)rsdFrameBufferSetActive; break;
    case RS_HAL_GRAPHICS_FB_DESTROY:
        fnPtr[0] = (void *)rsdFrameBufferDestroy; break;
    case RS_HAL_GRAPHICS_STORE_INIT:
        fnPtr[0] = (void *)rsdProgramStoreInit; break;
    case RS_HAL_GRAPHICS_STORE_SET_ACTIVE:
        fnPtr[0] = (void *)rsdProgramStoreSetActive; break;
    case RS_HAL_GRAPHICS_STORE_DESTROY:
        fnPtr[0] = (void *)rsdProgramStoreDestroy; break;
#endif

    default:
        ALOGE("ERROR: unknown RenderScript HAL API query, %i", entry);
        return false;
    }

    return true;
}

extern "C" void rsdHalAbort(RsContext) {

}


extern "C" bool rsdHalQueryVersion(uint32_t *major, uint32_t *minor) {
    *major = RS_HAL_VERSION;
    *minor = 0;
    return true;
}

} // namespace renderscript
} // namespace android

extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name);

static RsdCpuReference::CpuScript * LookupScript(Context *, const Script *s) {
    return (RsdCpuReference::CpuScript *)s->mHal.drv;
}

#ifdef RS_COMPATIBILITY_LIB
typedef void (*sAllocationDestroyFnPtr) (const Context *rsc, Allocation *alloc);
typedef void (*sAllocationIoSendFnPtr) (const Context *rsc, Allocation *alloc);
typedef void (*sAllocationSetSurfaceFnPtr) (const Context *rsc, Allocation *alloc, ANativeWindow *nw);
static sAllocationDestroyFnPtr sAllocationDestroy;
static sAllocationIoSendFnPtr sAllocationIoSend;
static sAllocationSetSurfaceFnPtr sAllocationSetSurface;

static bool loadIOSuppLibSyms() {
    void* handleIO = nullptr;
    handleIO = dlopen("libRSSupportIO.so", RTLD_LAZY | RTLD_LOCAL);
    if (handleIO == nullptr) {
        ALOGE("Couldn't load libRSSupportIO.so");
        return false;
    }
    sAllocationDestroy = (sAllocationDestroyFnPtr)dlsym(handleIO, "rscAllocationDestroy");
    if (sAllocationDestroy==nullptr) {
        ALOGE("Failed to initialize sAllocationDestroy");
        return false;
    }
    sAllocationIoSend = (sAllocationIoSendFnPtr)dlsym(handleIO, "rscAllocationIoSend");
    if (sAllocationIoSend==nullptr) {
        ALOGE("Failed to initialize sAllocationIoSend");
        return false;
    }
    sAllocationSetSurface = (sAllocationSetSurfaceFnPtr)dlsym(handleIO, "rscAllocationSetSurface");
    if (sAllocationSetSurface==nullptr) {
        ALOGE("Failed to initialize sAllocationIoSend");
        return false;
    }
    return true;
}
#endif

extern "C" bool rsdHalInit(RsContext c, uint32_t version_major,
                           uint32_t version_minor) {
    Context *rsc = (Context*) c;
#ifdef RS_COMPATIBILITY_LIB
    if (loadIOSuppLibSyms()) {
        rsc->mHal.funcs.allocation.destroy = sAllocationDestroy;
        rsc->mHal.funcs.allocation.ioSend = sAllocationIoSend;
        rsc->mHal.funcs.allocation.setSurface = sAllocationSetSurface;
    }
#endif

    RsdHal *dc = (RsdHal *)calloc(1, sizeof(RsdHal));
    if (!dc) {
        ALOGE("Calloc for driver hal failed.");
        return false;
    }
    rsc->mHal.drv = dc;

    dc->mCpuRef = RsdCpuReference::create(rsc, version_major, version_minor,
                                          &rsdLookupRuntimeStub, &LookupScript);
    if (!dc->mCpuRef) {
        ALOGE("RsdCpuReference::create for driver hal failed.");
        rsc->mHal.drv = nullptr;
        free(dc);
        return false;
    }

    return true;
}


void SetPriority(const Context *rsc, int32_t priority) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;

    dc->mCpuRef->setPriority(priority);

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (dc->mHasGraphics) {
        rsdGLSetPriority(rsc, priority);
    }
#endif
}

void Shutdown(Context *rsc) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    delete dc->mCpuRef;
    free(dc);
    rsc->mHal.drv = nullptr;
}

void* rsdAllocRuntimeMem(size_t size, uint32_t flags) {
    void* buffer = calloc(size, sizeof(char));
    return buffer;
}

void rsdFreeRuntimeMem(void* ptr) {
    free(ptr);
}
