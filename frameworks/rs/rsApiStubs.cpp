/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "rsApiStubs.h"
#include "rsHidlAdaptation.h"
#include "rsFallbackAdaptation.h"
#include "cpp/rsDispatch.h"

#include <log/log.h>
#include <dlfcn.h>

#include <mutex>
#include <map>

#undef LOG_TAG
#define LOG_TAG "RenderScript"

// TODO: Figure out how to use different declared types for the two interfaces
//       to avoid the confusion. Currently, RsContext is used as the API type for
//       both the client interface and the dispatch table interface, but at the
//       client interface it's really RsContextWrapper* instead.
// TODO: Figure out how to better design class hierarchy for all these Contexts.
// RsContextWrapper wraps the RsContext and corresponding dispatchTable pointer.
// The wrapper object is created during ContextCreate, and the address of the
// object is returned to Java / C++, instead of the RsContext handle.
// The wrapper object is destroyed during ContextDestroy to release the memory.
struct RsContextWrapper {
    RsContext context;
    const dispatchTable* dispatch;
};

#define RS_DISPATCH(opaqueWrapper, func, ...) \
    [&]() { \
      const RsContextWrapper *wrapper = reinterpret_cast<RsContextWrapper *>(opaqueWrapper); \
      RsContext context = wrapper->context; \
      return wrapper->dispatch->func(context, ##__VA_ARGS__); \
    }()


// contextMap maps RsContext to the corresponding RsContextWrapper pointer.
static std::map<RsContext, RsContextWrapper* > contextMap;

// contextMapMutex is used to protect concurrent access of the contextMap.
// std::mutex is safe for pthreads on Android. Since other threading model
// supported on Android are built on top of pthread, std::mutex is safe for them.
static std::mutex contextMapMutex;

// globalObjAlive is a global flag indicating whether the global objects,
// contextMap & contextMapMutex, are still alive.
// For the protected functions during application teardown, this
// flag will be checked before accessing the global objects.
static bool globalObjAlive;

// GlobalObjGuard manipulates the globalObjAlive flag during construction and
// destruction. If the guard object is destroyed, globalObjAlive will be set
// to false, which will make the protected functions NO-OP.
// https://goto.google.com/rs-static-destructor
class GlobalObjGuard {
  public:
    GlobalObjGuard() {
        globalObjAlive = true;
    }

    ~GlobalObjGuard() {
        globalObjAlive = false;
    }
};
static GlobalObjGuard guard;

// API to find high-level context (RsContextWrapper) given a low level context.
// This API is only intended to be used by RenderScript debugger.
extern "C" RsContext rsDebugGetHighLevelContext(RsContext context) {
    std::unique_lock<std::mutex> lock(contextMapMutex);
    return contextMap.at(context);
}

// Device
// These API stubs are kept here to maintain backward compatibility,
// but they are not actually doing anything.

extern "C" RsDevice rsDeviceCreate()
{
    return (void *) 1;
}
extern "C" void rsDeviceDestroy(RsDevice dev)
{
}
extern "C" void rsDeviceSetConfig(RsDevice dev, RsDeviceParam p, int32_t value)
{
}

/*
 * This global will be found by the debugger and will have its value flipped.
 * It's independent of the Context class to allow the debugger to do the above
 * without knowing the type makeup. This allows the debugger to be attached at
 * an earlier stage.
 */
extern "C" int gDebuggerPresent = 0;

namespace{
// Check if the calling process is a vendor process or not.
static bool isVendorProcess() {
    char path[PATH_MAX];
    ssize_t path_len = readlink("/proc/self/exe", path, sizeof(path));
    if (path_len == -1) {
        return false;
    }
    // Vendor process will return "/vendor/*"
    static const char vendor_path[] = "/vendor/";
    return !strncmp(path, vendor_path, sizeof(vendor_path)-1);
}

typedef const char* (*QueryCacheDirFnPtr)();
// Dynamically load the queryCacheDir function pointer, so that for vendor
// processes, libandroid_runtime.so will not be loaded.
static QueryCacheDirFnPtr loadQueryCacheFnPtr() {
    QueryCacheDirFnPtr queryCacheDir = nullptr;
    void* handle = dlopen("libRSCacheDir.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle ||
        !(queryCacheDir = (QueryCacheDirFnPtr)dlsym(handle, "rsQueryCacheDir"))) {
        ALOGW("Not able to query cache dir: %s", dlerror());
    }
    return queryCacheDir;
}
} // anonymous namespace

// Context
extern "C" RsContext rsContextCreate(RsDevice vdev, uint32_t version, uint32_t sdkVersion,
                                     RsContextType ct, uint32_t flags)
{
    if (!globalObjAlive) {
        ALOGE("rsContextCreate is not allowed during process teardown.");
        return nullptr;
    }

    RsContext context;
    RsContextWrapper *ctxWrapper;

    if (flags & RS_CONTEXT_LOW_LATENCY) {
        // Use CPU path for LOW_LATENCY context.
        RsFallbackAdaptation& instance = RsFallbackAdaptation::GetInstance();
        context = instance.GetEntryFuncs()->ContextCreate(vdev, version, sdkVersion, ct, flags);
        ctxWrapper = new RsContextWrapper{context, instance.GetEntryFuncs()};
    } else {
        RsHidlAdaptation& instance = RsHidlAdaptation::GetInstance();
        context = instance.GetEntryFuncs()->ContextCreate(vdev, version, sdkVersion, ct, flags);
        ctxWrapper = new RsContextWrapper{context, instance.GetEntryFuncs()};

        // Use function local static variables to ensure thread safety.
        static QueryCacheDirFnPtr queryCacheDir = isVendorProcess() ? nullptr : loadQueryCacheFnPtr();
        if (queryCacheDir) {
            static std::string defaultCacheDir = std::string(queryCacheDir());
            if (defaultCacheDir.size() > 0) {
                ALOGD("Setting cache dir: %s", defaultCacheDir.c_str());
                rsContextSetCacheDir(ctxWrapper,
                                 defaultCacheDir.c_str(),
                                 defaultCacheDir.size());
            }
        }
    }

    // Wait for debugger to attach if RS_CONTEXT_WAIT_FOR_ATTACH flag set.
    if (flags & RS_CONTEXT_WAIT_FOR_ATTACH) {
        while (!gDebuggerPresent) {
            sleep(0);
        }
    }

    // Lock contextMap when adding new entries.
    std::unique_lock<std::mutex> lock(contextMapMutex);
    contextMap.insert(std::make_pair(context, ctxWrapper));

    return (RsContext) ctxWrapper;
}

extern "C" void rsContextDestroy (RsContext ctxWrapper)
{
    if (!globalObjAlive) {
        return;
    }

    RS_DISPATCH(ctxWrapper, ContextDestroy);

    // Lock contextMap when deleting an existing entry.
    std::unique_lock<std::mutex> lock(contextMapMutex);
    contextMap.erase(reinterpret_cast< RsContextWrapper* >(ctxWrapper)->context);

    delete (RsContextWrapper *)ctxWrapper;
}

extern "C" void rsContextFinish (RsContext ctxWrapper)
{
    RS_DISPATCH(ctxWrapper, ContextFinish);
}

extern "C" void rsContextDump (RsContext ctxWrapper, int32_t bits)
{
    RS_DISPATCH(ctxWrapper, ContextDump, bits);
}

extern "C" void rsContextSetPriority (RsContext ctxWrapper, int32_t priority)
{
    RS_DISPATCH(ctxWrapper, ContextSetPriority, priority);
}

extern "C" void rsContextDestroyWorker (RsContext ctxWrapper)
{
}

extern "C" RsMessageToClientType rsContextGetMessage (RsContext ctxWrapper, void * data, size_t data_length,
                                                      size_t * receiveLen, size_t receiveLen_length,
                                                      uint32_t * usrID, size_t usrID_length)
{
    return RS_DISPATCH(ctxWrapper, ContextGetMessage, data, data_length,
                                       receiveLen, receiveLen_length,
                                       usrID, usrID_length);
}

extern "C" RsMessageToClientType rsContextPeekMessage (RsContext ctxWrapper,
                                                       size_t * receiveLen, size_t receiveLen_length,
                                                       uint32_t * usrID, size_t usrID_length)
{
    return RS_DISPATCH(ctxWrapper, ContextPeekMessage,
                       receiveLen, receiveLen_length,
                       usrID, usrID_length);
}

extern "C" void rsContextSendMessage (RsContext ctxWrapper, uint32_t id, const uint8_t * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, ContextSendMessage, id, data, data_length);
}

extern "C" void rsContextInitToClient (RsContext ctxWrapper)
{
    RS_DISPATCH(ctxWrapper, ContextInitToClient);
}

extern "C" void rsContextDeinitToClient (RsContext ctxWrapper)
{
    RS_DISPATCH(ctxWrapper, ContextDeinitToClient);
}

extern "C" void rsContextSetCacheDir (RsContext ctxWrapper, const char * cacheDir, size_t cacheDir_length)
{
    RS_DISPATCH(ctxWrapper, ContextSetCacheDir, cacheDir, cacheDir_length);
}

extern "C" void rsaContextSetNativeLibDir(RsContext ctxWrapper, char *libDir, size_t length)
{
}

// BaseObject

extern "C" void rsAssignName (RsContext ctxWrapper, RsObjectBase obj, const char * name, size_t name_length)
{
    RS_DISPATCH(ctxWrapper, AssignName, obj, name, name_length);
}

extern "C" void rsaGetName(RsContext ctxWrapper, void * obj, const char **name)
{
    RS_DISPATCH(ctxWrapper, GetName, obj, name);
}

extern "C" void rsObjDestroy (RsContext ctxWrapper, RsAsyncVoidPtr objPtr)
{
    RS_DISPATCH(ctxWrapper, ObjDestroy, objPtr);
}

// Element

extern "C" RsElement rsElementCreate (RsContext ctxWrapper, RsDataType mType, RsDataKind mKind,
                                      bool mNormalized, uint32_t mVectorSize)
{
    return RS_DISPATCH(ctxWrapper, ElementCreate, mType, mKind, mNormalized, mVectorSize);
}

extern "C" RsElement rsElementCreate2 (RsContext ctxWrapper, const RsElement * elements, size_t elements_length,
                                       const char ** names, size_t names_length_length, const size_t * names_length,
                                       const uint32_t * arraySize, size_t arraySize_length)
{
    return RS_DISPATCH(ctxWrapper, ElementCreate2,
                       elements, elements_length,
                       names, names_length_length, names_length,
                       arraySize, arraySize_length);
}

extern "C" void rsaElementGetNativeData(RsContext ctxWrapper, RsElement elem, uint32_t *elemData, uint32_t elemDataSize)
{
    RS_DISPATCH(ctxWrapper, ElementGetNativeData, elem, elemData, elemDataSize);
}

extern "C" void rsaElementGetSubElements(RsContext ctxWrapper, RsElement elem, uintptr_t *ids, const char **names,
                                         size_t *arraySizes, uint32_t dataSize)
{
    RS_DISPATCH(ctxWrapper, ElementGetSubElements, elem, ids, names, arraySizes, dataSize);
}

// Type

extern "C" RsType rsTypeCreate (RsContext ctxWrapper, RsElement e, uint32_t dimX, uint32_t dimY, uint32_t dimZ,
                                bool mipmaps, bool faces, uint32_t yuv)
{
    return RS_DISPATCH(ctxWrapper, TypeCreate, e, dimX, dimY, dimZ, mipmaps, faces, yuv);
}

extern "C" RsType rsTypeCreate2 (RsContext ctxWrapper, const RsTypeCreateParams * dat, size_t dat_length)
{
    return nullptr;
}

extern "C" void rsaTypeGetNativeData(RsContext ctxWrapper, RsType type, uintptr_t *typeData, uint32_t typeDataSize)
{
    RS_DISPATCH(ctxWrapper, TypeGetNativeData, type, typeData, typeDataSize);
}


// Allocation

extern "C" RsAllocation rsAllocationCreateTyped (RsContext ctxWrapper, RsType vtype, RsAllocationMipmapControl mipmaps,
                                                 uint32_t usages, uintptr_t ptr)
{
    return RS_DISPATCH(ctxWrapper, AllocationCreateTyped, vtype, mipmaps, usages, ptr);
}

extern "C" RsAllocation rsAllocationCreateFromBitmap (RsContext ctxWrapper, RsType vtype, RsAllocationMipmapControl mipmaps,
                                                      const void * data, size_t data_length, uint32_t usages)
{
    return RS_DISPATCH(ctxWrapper, AllocationCreateFromBitmap, vtype, mipmaps, data, data_length, usages);
}

extern "C" RsAllocation rsAllocationCubeCreateFromBitmap (RsContext ctxWrapper, RsType vtype, RsAllocationMipmapControl mipmaps,
                                                          const void * data, size_t data_length, uint32_t usages)
{
    return RS_DISPATCH(ctxWrapper, AllocationCubeCreateFromBitmap, vtype, mipmaps, data, data_length, usages);
}

extern "C" RsAllocation rsAllocationAdapterCreate (RsContext ctxWrapper, RsType vtype, RsAllocation baseAlloc)
{
    return RS_DISPATCH(ctxWrapper, AllocationAdapterCreate, vtype, baseAlloc);
}

extern "C" const void * rsaAllocationGetType(RsContext ctxWrapper, RsAllocation va)
{
    return RS_DISPATCH(ctxWrapper, AllocationGetType, va);
}

extern "C" RsNativeWindow rsAllocationGetSurface (RsContext ctxWrapper, RsAllocation alloc)
{
    return RS_DISPATCH(ctxWrapper, AllocationGetSurface, alloc);
}

extern "C" void rsAllocationSetupBufferQueue (RsContext ctxWrapper, RsAllocation alloc, uint32_t numAlloc)
{
    RS_DISPATCH(ctxWrapper, AllocationSetupBufferQueue, alloc, numAlloc);
}

extern "C" void rsAllocationShareBufferQueue (RsContext ctxWrapper, RsAllocation alloc1, RsAllocation alloc2)
{
    RS_DISPATCH(ctxWrapper, AllocationShareBufferQueue, alloc1, alloc2);
}

extern "C" void rsAllocationSetSurface (RsContext ctxWrapper, RsAllocation alloc, RsNativeWindow sur)
{
    RS_DISPATCH(ctxWrapper, AllocationSetSurface, alloc, sur);
}

extern "C" void rsAllocationAdapterOffset (RsContext ctxWrapper, RsAllocation alloc,
                                           const uint32_t * offsets, size_t offsets_length)
{
    RS_DISPATCH(ctxWrapper, AllocationAdapterOffset, alloc, offsets, offsets_length);
}

extern "C" void rsAllocationCopyToBitmap (RsContext ctxWrapper, RsAllocation alloc, void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, AllocationCopyToBitmap, alloc, data, data_length);
}

extern "C" void * rsAllocationGetPointer (RsContext ctxWrapper, RsAllocation va, uint32_t lod, RsAllocationCubemapFace face,
                                          uint32_t z, uint32_t array, size_t * stride, size_t stride_length)
{
    return RS_DISPATCH(ctxWrapper, AllocationGetPointer, va, lod, face, z, array, stride, stride_length);
}

extern "C" void rsAllocation1DData (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t lod, uint32_t count,
                                    const void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, Allocation1DData, va, xoff, lod, count, data, data_length);
}

extern "C" void rsAllocation1DElementData (RsContext ctxWrapper, RsAllocation va, uint32_t x, uint32_t lod,
                                           const void * data, size_t data_length, size_t comp_offset)
{
    RS_DISPATCH(ctxWrapper, Allocation1DElementData, va, x, lod, data, data_length, comp_offset);
}

extern "C" void rsAllocationElementData (RsContext ctxWrapper, RsAllocation va, uint32_t x, uint32_t y, uint32_t z,
                                         uint32_t lod, const void * data, size_t data_length, size_t comp_offset)
{
    RS_DISPATCH(ctxWrapper, AllocationElementData, va, x, y, z, lod, data, data_length, comp_offset);
}

extern "C" void rsAllocation2DData (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t lod,
                                    RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                                    const void * data, size_t data_length, size_t stride)
{
    RS_DISPATCH(ctxWrapper, Allocation2DData, va, xoff, yoff, lod, face, w, h, data, data_length, stride);
}

extern "C" void rsAllocation3DData (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t zoff,
                                    uint32_t lod, uint32_t w, uint32_t h, uint32_t d,
                                    const void * data, size_t data_length, size_t stride)
{
    RS_DISPATCH(ctxWrapper, Allocation3DData, va, xoff, yoff, zoff, lod, w, h, d, data, data_length, stride);
}

extern "C" void rsAllocationGenerateMipmaps (RsContext ctxWrapper, RsAllocation va)
{
    RS_DISPATCH(ctxWrapper, AllocationGenerateMipmaps, va);
}

extern "C" void rsAllocationRead (RsContext ctxWrapper, RsAllocation va, void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, AllocationRead, va, data, data_length);
}

extern "C" void rsAllocation1DRead (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t lod, uint32_t count,
                                    void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, Allocation1DRead, va, xoff, lod, count, data, data_length);
}

extern "C" void rsAllocationElementRead (RsContext ctxWrapper, RsAllocation va, uint32_t x, uint32_t y, uint32_t z,
                                         uint32_t lod, void * data, size_t data_length, size_t comp_offset)
{
    RS_DISPATCH(ctxWrapper, AllocationElementRead, va, x, y, z, lod, data, data_length, comp_offset);
}

extern "C" void rsAllocation2DRead (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t lod,
                                    RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                                    void * data, size_t data_length, size_t stride)
{
    RS_DISPATCH(ctxWrapper, Allocation2DRead, va, xoff, yoff, lod, face, w, h, data, data_length, stride);
}

extern "C" void rsAllocation3DRead (RsContext ctxWrapper, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t zoff,
                                    uint32_t lod, uint32_t w, uint32_t h, uint32_t d,
                                    void * data, size_t data_length, size_t stride)
{
    RS_DISPATCH(ctxWrapper, Allocation3DRead, va, xoff, yoff, zoff, lod, w, h, d, data, data_length, stride);
}

extern "C" void rsAllocationSyncAll (RsContext ctxWrapper, RsAllocation va, RsAllocationUsageType src)
{
    RS_DISPATCH(ctxWrapper, AllocationSyncAll, va, src);
}

extern "C" void rsAllocationResize1D (RsContext ctxWrapper, RsAllocation va, uint32_t dimX)
{
    RS_DISPATCH(ctxWrapper, AllocationResize1D, va, dimX);
}

extern "C" void rsAllocationCopy2DRange (RsContext ctxWrapper,
                                         RsAllocation dest,
                                         uint32_t destXoff, uint32_t destYoff,
                                         uint32_t destMip, uint32_t destFace,
                                         uint32_t width, uint32_t height,
                                         RsAllocation src,
                                         uint32_t srcXoff, uint32_t srcYoff,
                                         uint32_t srcMip, uint32_t srcFace)
{
    RS_DISPATCH(ctxWrapper, AllocationCopy2DRange, dest, destXoff, destYoff, destMip, destFace,
                                    width, height, src, srcXoff, srcYoff, srcMip, srcFace);
}

extern "C" void rsAllocationCopy3DRange (RsContext ctxWrapper,
                                         RsAllocation dest,
                                         uint32_t destXoff, uint32_t destYoff, uint32_t destZoff,
                                         uint32_t destMip,
                                         uint32_t width, uint32_t height, uint32_t depth,
                                         RsAllocation src,
                                         uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                                         uint32_t srcMip)
{
    RS_DISPATCH(ctxWrapper, AllocationCopy3DRange,
                dest, destXoff, destYoff, destZoff, destMip,
                width, height, depth, src, srcXoff, srcYoff, srcZoff, srcMip);
}

extern "C" void rsAllocationIoSend (RsContext ctxWrapper, RsAllocation alloc)
{
    RS_DISPATCH(ctxWrapper, AllocationIoSend, alloc);
}

extern "C" int64_t rsAllocationIoReceive (RsContext ctxWrapper, RsAllocation alloc)
{
    return RS_DISPATCH(ctxWrapper, AllocationIoReceive, alloc);
}

// ScriptGroup

extern "C" void rsScriptGroupExecute (RsContext ctxWrapper, RsScriptGroup group)
{
    RS_DISPATCH(ctxWrapper, ScriptGroupExecute, group);
}

extern "C" RsScriptGroup2 rsScriptGroup2Create (RsContext ctxWrapper, const char * name, size_t name_length,
                                                const char * cacheDir, size_t cacheDir_length,
                                                RsClosure * closures, size_t closures_length)
{
    return RS_DISPATCH(ctxWrapper, ScriptGroup2Create,
                       name, name_length,
                       cacheDir, cacheDir_length,
                       closures, closures_length);
}

extern "C" RsClosure rsClosureCreate (RsContext ctxWrapper, RsScriptKernelID kernelID, RsAllocation returnValue,
                                      RsScriptFieldID * fieldIDs, size_t fieldIDs_length,
                                      const int64_t * values, size_t values_length,
                                      const int * sizes, size_t sizes_length,
                                      RsClosure * depClosures, size_t depClosures_length,
                                      RsScriptFieldID * depFieldIDs, size_t depFieldIDs_length)
{
    return RS_DISPATCH(ctxWrapper, ClosureCreate, kernelID, returnValue, fieldIDs, fieldIDs_length,
                                   const_cast<int64_t *>(values), values_length,
                                   const_cast<int *>(sizes), sizes_length,
                                   depClosures, depClosures_length, depFieldIDs, depFieldIDs_length);
}

extern "C" RsClosure rsInvokeClosureCreate (RsContext ctxWrapper, RsScriptInvokeID invokeID,
                                            const void * params, size_t params_length,
                                            const RsScriptFieldID * fieldIDs, size_t fieldIDs_length,
                                            const int64_t * values, size_t values_length,
                                            const int * sizes, size_t sizes_length)
{
    return RS_DISPATCH(ctxWrapper, InvokeClosureCreate, invokeID,
                       params, params_length,
                       fieldIDs, fieldIDs_length,
                       values, values_length,
                       sizes, sizes_length);
}

extern "C" void rsClosureSetArg (RsContext ctxWrapper, RsClosure closureID, uint32_t index,
                                 uintptr_t value, int valueSize)
{
    RS_DISPATCH(ctxWrapper, ClosureSetArg, closureID, index, value, valueSize);
}

extern "C" void rsClosureSetGlobal (RsContext ctxWrapper, RsClosure closureID, RsScriptFieldID fieldID,
                                    int64_t value, int valueSize)
{
    RS_DISPATCH(ctxWrapper, ClosureSetGlobal, closureID, fieldID, value, valueSize);
}

extern "C" RsScriptKernelID rsScriptKernelIDCreate (RsContext ctxWrapper, RsScript sid, int slot, int sig)
{
    return RS_DISPATCH(ctxWrapper, ScriptKernelIDCreate, sid, slot, sig);
}

extern "C" RsScriptFieldID rsScriptFieldIDCreate (RsContext ctxWrapper, RsScript sid, int slot)
{
    return RS_DISPATCH(ctxWrapper, ScriptFieldIDCreate, sid, slot);
}

extern "C" RsScriptGroup rsScriptGroupCreate (RsContext ctxWrapper, RsScriptKernelID * kernels, size_t kernels_length,
                                              RsScriptKernelID * src, size_t src_length,
                                              RsScriptKernelID * dstK, size_t dstK_length,
                                              RsScriptFieldID * dstF, size_t dstF_length,
                                              const RsType * type, size_t type_length)
{
    return RS_DISPATCH(ctxWrapper, ScriptGroupCreate,
                       kernels, kernels_length,
                       src, src_length, dstK, dstK_length,
                       dstF, dstF_length, type, type_length);
}

extern "C" void rsScriptGroupSetOutput (RsContext ctxWrapper, RsScriptGroup group,
                                        RsScriptKernelID kernel, RsAllocation alloc)
{
    RS_DISPATCH(ctxWrapper, ScriptGroupSetOutput, group, kernel, alloc);
}

extern "C" void rsScriptGroupSetInput (RsContext ctxWrapper, RsScriptGroup group,
                                       RsScriptKernelID kernel, RsAllocation alloc)
{
    RS_DISPATCH(ctxWrapper, ScriptGroupSetInput, group, kernel, alloc);
}


// Sampler
extern "C" RsSampler rsSamplerCreate (RsContext ctxWrapper, RsSamplerValue magFilter, RsSamplerValue minFilter,
                                      RsSamplerValue wrapS, RsSamplerValue wrapT, RsSamplerValue wrapR,
                                      float mAniso)
{
    return RS_DISPATCH(ctxWrapper, SamplerCreate, magFilter, minFilter, wrapS, wrapT, wrapR, mAniso);
}

// Script

extern "C" RsScript rsScriptCCreate (RsContext ctxWrapper, const char * resName, size_t resName_length,
                                     const char * cacheDir, size_t cacheDir_length,
                                     const char * text, size_t text_length)
{
    return RS_DISPATCH(ctxWrapper, ScriptCCreate, resName, resName_length, cacheDir, cacheDir_length, text, text_length);
}

extern "C" RsScript rsScriptIntrinsicCreate (RsContext ctxWrapper, uint32_t id, RsElement eid)
{
    return RS_DISPATCH(ctxWrapper, ScriptIntrinsicCreate, id, eid);
}

extern "C" void rsScriptBindAllocation (RsContext ctxWrapper, RsScript vtm, RsAllocation va, uint32_t slot)
{
    RS_DISPATCH(ctxWrapper, ScriptBindAllocation, vtm, va, slot);
}

extern "C" void rsScriptSetTimeZone (RsContext ctxWrapper, RsScript s, const char * timeZone, size_t timeZone_length)
{
    RS_DISPATCH(ctxWrapper, ScriptSetTimeZone, s, timeZone, timeZone_length);
}

extern "C" RsScriptInvokeID rsScriptInvokeIDCreate (RsContext ctxWrapper, RsScript s, uint32_t slot)
{
    return RS_DISPATCH(ctxWrapper, ScriptInvokeIDCreate, s, slot);
}

extern "C" void rsScriptInvoke (RsContext ctxWrapper, RsScript s, uint32_t slot)
{
    RS_DISPATCH(ctxWrapper, ScriptInvoke, s, slot);
}

extern "C" void rsScriptInvokeV (RsContext ctxWrapper, RsScript s, uint32_t slot, const void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, ScriptInvokeV, s, slot, data, data_length);
}

extern "C" void rsScriptForEach (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                 RsAllocation ain, RsAllocation aout,
                                 const void * usr, size_t usr_length,
                                 const RsScriptCall * sc, size_t sc_length)
{
    RS_DISPATCH(ctxWrapper, ScriptForEach, s, slot, ain, aout, usr, usr_length, sc, sc_length);
}

extern "C" void rsScriptForEachMulti (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                      RsAllocation * ains, size_t ains_length, RsAllocation aout,
                                      const void * usr, size_t usr_length,
                                      const RsScriptCall * sc, size_t sc_length)
{
    RS_DISPATCH(ctxWrapper, ScriptForEachMulti, s, slot, ains, ains_length, aout, usr, usr_length, sc, sc_length);
}

extern "C" void rsScriptReduce (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                RsAllocation * ains, size_t ains_length, RsAllocation aout,
                                const RsScriptCall * sc, size_t sc_length)
{
    RS_DISPATCH(ctxWrapper, ScriptReduce, s, slot, ains, ains_length, aout, sc, sc_length);
}

extern "C" void rsScriptSetVarI (RsContext ctxWrapper, RsScript s, uint32_t slot, int value)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarI, s, slot, value);
}

extern "C" void rsScriptSetVarObj (RsContext ctxWrapper, RsScript s, uint32_t slot, RsObjectBase value)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarObj, s, slot, value);
}

extern "C" void rsScriptSetVarJ (RsContext ctxWrapper, RsScript s, uint32_t slot, int64_t value)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarJ, s, slot, value);
}

extern "C" void rsScriptSetVarF (RsContext ctxWrapper, RsScript s, uint32_t slot, float value)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarF, s, slot, value);
}

extern "C" void rsScriptSetVarD (RsContext ctxWrapper, RsScript s, uint32_t slot, double value)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarD, s, slot, value);
}

extern "C" void rsScriptSetVarV (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                 const void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarV, s, slot, data, data_length);
}

extern "C" void rsScriptGetVarV (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                 void * data, size_t data_length)
{
    RS_DISPATCH(ctxWrapper, ScriptGetVarV, s, slot, data, data_length);
}

extern "C" void rsScriptSetVarVE (RsContext ctxWrapper, RsScript s, uint32_t slot,
                                  const void * data, size_t data_length,
                                  RsElement e, const uint32_t * dims, size_t dims_length)
{
    RS_DISPATCH(ctxWrapper, ScriptSetVarVE, s, slot, data, data_length, e, dims, dims_length);
}


// Graphics
/* The following API are deprecated. */

extern "C" RsContext rsContextCreateGL(RsDevice vdev, uint32_t version, uint32_t sdkVersion,
                                       RsSurfaceConfig sc, uint32_t dpi)
{
    if (!globalObjAlive) {
        ALOGE("rsContextCreateGL is not allowed during process teardown.");
        return nullptr;
    }

    RsFallbackAdaptation& instance = RsFallbackAdaptation::GetInstance();
    RsContext context = instance.GetEntryFuncs()->ContextCreateGL(vdev, version, sdkVersion, sc, dpi);

    RsContextWrapper *ctxWrapper = new RsContextWrapper{context, instance.GetEntryFuncs()};

    // Lock contextMap when adding new entries.
    std::unique_lock<std::mutex> lock(contextMapMutex);
    contextMap.insert(std::make_pair(context, ctxWrapper));

    return (RsContext) ctxWrapper;
}

extern "C" void rsContextBindProgramStore (RsContext ctxWrapper, RsProgramStore pgm)
{
    RS_DISPATCH(ctxWrapper, ContextBindProgramStore, pgm);
}

extern "C" void rsContextBindProgramFragment (RsContext ctxWrapper, RsProgramFragment pgm)
{
    RS_DISPATCH(ctxWrapper, ContextBindProgramFragment, pgm);
}

extern "C" void rsContextBindProgramVertex (RsContext ctxWrapper, RsProgramVertex pgm)
{
    RS_DISPATCH(ctxWrapper, ContextBindProgramVertex, pgm);
}

extern "C" void rsContextBindProgramRaster (RsContext ctxWrapper, RsProgramRaster pgm)
{
    RS_DISPATCH(ctxWrapper, ContextBindProgramRaster, pgm);
}

extern "C" void rsContextBindFont (RsContext ctxWrapper, RsFont pgm)
{
    RS_DISPATCH(ctxWrapper, ContextBindFont, pgm);
}

extern "C" void rsContextSetSurface (RsContext ctxWrapper, uint32_t width, uint32_t height,
                                     RsNativeWindow sur)
{
    RS_DISPATCH(ctxWrapper, ContextSetSurface, width, height, sur);
}

extern "C" void rsContextBindRootScript (RsContext ctxWrapper, RsScript sampler)
{
    RS_DISPATCH(ctxWrapper, ContextBindRootScript, sampler);
}

extern "C" void rsContextPause (RsContext ctxWrapper)
{
    RS_DISPATCH(ctxWrapper, ContextPause);
}

extern "C" void rsContextResume (RsContext ctxWrapper)
{
    RS_DISPATCH(ctxWrapper, ContextResume);
}

extern "C" RsProgramStore rsProgramStoreCreate (RsContext ctxWrapper,
                                                bool colorMaskR, bool colorMaskG,
                                                bool colorMaskB, bool colorMaskA,
                                                bool depthMask, bool ditherEnable,
                                                RsBlendSrcFunc srcFunc,
                                                RsBlendDstFunc destFunc,
                                                RsDepthFunc depthFunc)
{
    return RS_DISPATCH(ctxWrapper, ProgramStoreCreate,
                       colorMaskR, colorMaskG, colorMaskB, colorMaskA,
                       depthMask, ditherEnable, srcFunc, destFunc, depthFunc);
}

extern "C" RsProgramRaster rsProgramRasterCreate (RsContext ctxWrapper, bool pointSprite, RsCullMode cull)
{
    return RS_DISPATCH(ctxWrapper, ProgramRasterCreate, pointSprite, cull);
}

extern "C" RsProgramFragment rsProgramFragmentCreate (RsContext ctxWrapper,
                                                      const char * shaderText, size_t shaderText_length,
                                                      const char ** textureNames, size_t textureNames_length_length,
                                                      const size_t * textureNames_length,
                                                      const uintptr_t * params, size_t params_length)
{
    return RS_DISPATCH(ctxWrapper, ProgramFragmentCreate,
                       shaderText, shaderText_length,
                       textureNames, textureNames_length_length, textureNames_length,
                       params, params_length);
}

extern "C" RsProgramVertex rsProgramVertexCreate (RsContext ctxWrapper,
                                                  const char * shaderText, size_t shaderText_length,
                                                  const char ** textureNames, size_t textureNames_length_length,
                                                  const size_t * textureNames_length,
                                                  const uintptr_t * params, size_t params_length)
{
    return RS_DISPATCH(ctxWrapper, ProgramVertexCreate,
                       shaderText, shaderText_length,
                       textureNames, textureNames_length_length, textureNames_length,
                       params, params_length);
}

extern "C" RsFont rsFontCreateFromFile (RsContext ctxWrapper, const char * name, size_t name_length,
                                        float fontSize, uint32_t dpi)
{
    return RS_DISPATCH(ctxWrapper, FontCreateFromFile, name, name_length, fontSize, dpi);
}

extern "C" RsFont rsFontCreateFromMemory (RsContext ctxWrapper, const char * name, size_t name_length,
                                          float fontSize, uint32_t dpi,
                                          const void * data, size_t data_length)
{
    return RS_DISPATCH(ctxWrapper, FontCreateFromMemory, name, name_length, fontSize, dpi, data, data_length);
}

extern "C" RsMesh rsMeshCreate (RsContext ctxWrapper, RsAllocation * vtx, size_t vtx_length,
                                RsAllocation * idx, size_t idx_length,
                                uint32_t * primType, size_t primType_length)
{
    return RS_DISPATCH(ctxWrapper, MeshCreate, vtx, vtx_length, idx, idx_length, primType, primType_length);
}

extern "C" void rsProgramBindConstants (RsContext ctxWrapper, RsProgram vp, uint32_t slot, RsAllocation constants)
{
    RS_DISPATCH(ctxWrapper, ProgramBindConstants, vp, slot, constants);
}

extern "C" void rsProgramBindTexture (RsContext ctxWrapper, RsProgramFragment pf, uint32_t slot, RsAllocation a)
{
    RS_DISPATCH(ctxWrapper, ProgramBindTexture, pf, slot,a);
}

extern "C" void rsProgramBindSampler (RsContext ctxWrapper, RsProgramFragment pf, uint32_t slot, RsSampler s)
{
    RS_DISPATCH(ctxWrapper, ProgramBindSampler, pf, slot, s);
}

extern "C" RsObjectBase rsaFileA3DGetEntryByIndex(RsContext ctxWrapper, uint32_t index, RsFile file)
{
    return RS_DISPATCH(ctxWrapper, FileA3DGetEntryByIndex, index, file);
}

extern "C" RsFile rsaFileA3DCreateFromMemory(RsContext ctxWrapper, const void *data, uint32_t len)
{
    return RS_DISPATCH(ctxWrapper, FileA3DCreateFromMemory, data, len);
}

extern "C" RsFile rsaFileA3DCreateFromAsset(RsContext ctxWrapper, void *_asset)
{
    return RS_DISPATCH(ctxWrapper, FileA3DCreateFromAsset, _asset);
}

extern "C" RsFile rsaFileA3DCreateFromFile(RsContext ctxWrapper, const char *path)
{
    return RS_DISPATCH(ctxWrapper, FileA3DCreateFromFile, path);
}

extern "C"  void rsaFileA3DGetNumIndexEntries(RsContext ctxWrapper, int32_t *numEntries, RsFile file)
{
    RS_DISPATCH(ctxWrapper, FileA3DGetNumIndexEntries, numEntries, file);
}

extern "C" void rsaFileA3DGetIndexEntries(RsContext ctxWrapper, RsFileIndexEntry *fileEntries, uint32_t numEntries, RsFile file)
{
    RS_DISPATCH(ctxWrapper, FileA3DGetIndexEntries, fileEntries, numEntries, file);
}

extern "C" void rsaMeshGetVertexBufferCount(RsContext ctxWrapper, RsMesh mv, int32_t *numVtx)
{
    RS_DISPATCH(ctxWrapper, MeshGetVertexBufferCount, mv, numVtx);
}

extern "C" void rsaMeshGetIndexCount(RsContext ctxWrapper, RsMesh mv, int32_t *numIdx)
{
    RS_DISPATCH(ctxWrapper, MeshGetIndexCount, mv, numIdx);
}

extern "C" void rsaMeshGetVertices(RsContext ctxWrapper, RsMesh mv, RsAllocation *vtxData, uint32_t vtxDataCount)
{
    RS_DISPATCH(ctxWrapper, MeshGetVertices, mv, vtxData, vtxDataCount);
}

extern "C" void rsaMeshGetIndices(RsContext ctxWrapper, RsMesh mv, RsAllocation *va, uint32_t *primType, uint32_t idxDataCount)
{
    RS_DISPATCH(ctxWrapper, MeshGetIndices, mv, va, primType, idxDataCount);
}
