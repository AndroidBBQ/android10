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

#ifndef ANDROID_RSDISPATCH_H
#define ANDROID_RSDISPATCH_H

#include "rsInternalDefines.h"

typedef void (*SetNativeLibDirFnPtr)(RsContext con, const char *nativeLibDir, size_t length);
typedef const void* (*AllocationGetTypeFnPtr)(RsContext con, RsAllocation va);
typedef void (*TypeGetNativeDataFnPtr)(RsContext, RsType, uintptr_t *typeData, uint32_t typeDataSize);
typedef void (*ElementGetNativeDataFnPtr)(RsContext, RsElement, uint32_t *elemData, uint32_t elemDataSize);
typedef void (*ElementGetSubElementsFnPtr)(RsContext, RsElement, uintptr_t *ids, const char **names, size_t *arraySizes, uint32_t dataSize);
typedef RsDevice (*DeviceCreateFnPtr) ();
typedef void (*DeviceDestroyFnPtr) (RsDevice dev);
typedef void (*DeviceSetConfigFnPtr) (RsDevice dev, RsDeviceParam p, int32_t value);
typedef RsContext (*ContextCreateFnPtr)(RsDevice vdev, uint32_t version, uint32_t sdkVersion, RsContextType ct, uint32_t flags);
typedef RsContext (*ContextCreateVendorFnPtr)(RsDevice vdev, uint32_t version, uint32_t sdkVersion, RsContextType ct, uint32_t flags, const char* vendorDriverName);
typedef void (*GetNameFnPtr)(RsContext, void * obj, const char **name);
typedef RsClosure (*ClosureCreateFnPtr)(RsContext, RsScriptKernelID, RsAllocation, RsScriptFieldID*, size_t, int64_t*, size_t, int*, size_t, RsClosure*, size_t, RsScriptFieldID*, size_t);
typedef RsClosure (*InvokeClosureCreateFnPtr)(RsContext, RsScriptInvokeID, const void*, const size_t, const RsScriptFieldID*, const size_t, const int64_t*, const size_t, const int*, const size_t);
typedef void (*ClosureSetArgFnPtr)(RsContext, RsClosure, uint32_t, uintptr_t, int);
typedef void (*ClosureSetGlobalFnPtr)(RsContext, RsClosure, RsScriptFieldID, int64_t, int);
typedef void (*ContextDestroyFnPtr) (RsContext);
typedef RsMessageToClientType (*ContextGetMessageFnPtr) (RsContext, void*, size_t, size_t*, size_t, uint32_t*, size_t);
typedef RsMessageToClientType (*ContextPeekMessageFnPtr) (RsContext, size_t*, size_t, uint32_t*, size_t);
typedef void (*ContextSendMessageFnPtr) (RsContext, uint32_t, const uint8_t*, size_t);
typedef void (*ContextInitToClientFnPtr) (RsContext);
typedef void (*ContextDeinitToClientFnPtr) (RsContext);
typedef void (*ContextSetCacheDirFnPtr) (RsContext rsc, const char *cacheDir, size_t cacheDir_length);
typedef RsType (*TypeCreateFnPtr) (RsContext, RsElement, uint32_t, uint32_t, uint32_t, bool, bool, uint32_t);
typedef RsAllocation (*AllocationCreateTypedFnPtr) (RsContext, RsType, RsAllocationMipmapControl, uint32_t, uintptr_t);
typedef RsAllocation (*AllocationCreateStridedFnPtr) (RsContext, RsType, RsAllocationMipmapControl, uint32_t, uintptr_t, size_t);
typedef RsAllocation (*AllocationCreateFromBitmapFnPtr) (RsContext, RsType, RsAllocationMipmapControl, const void*, size_t, uint32_t);
typedef RsAllocation (*AllocationCubeCreateFromBitmapFnPtr) (RsContext, RsType, RsAllocationMipmapControl, const void*, size_t, uint32_t);
typedef RsNativeWindow (*AllocationGetSurfaceFnPtr) (RsContext, RsAllocation);
typedef void (*AllocationSetSurfaceFnPtr) (RsContext, RsAllocation, RsNativeWindow);
typedef void (*ContextFinishFnPtr) (RsContext);
typedef void (*ContextDumpFnPtr) (RsContext, int32_t);
typedef void (*ContextSetPriorityFnPtr) (RsContext, int32_t);
typedef void (*AssignNameFnPtr) (RsContext, RsObjectBase, const char*, size_t);
typedef void (*ObjDestroyFnPtr) (RsContext, RsAsyncVoidPtr);
typedef RsElement (*ElementCreateFnPtr) (RsContext, RsDataType, RsDataKind, bool, uint32_t);
typedef RsElement (*ElementCreate2FnPtr) (RsContext, const RsElement*, size_t, const char**, size_t, const size_t*, const uint32_t*, size_t);
typedef void (*AllocationCopyToBitmapFnPtr) (RsContext, RsAllocation, void*, size_t);
typedef void (*Allocation1DDataFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, const void*, size_t);
typedef void (*Allocation1DElementDataFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, const void*, size_t, size_t);
typedef void (*AllocationElementDataFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, const void*, size_t, size_t);
typedef void (*Allocation2DDataFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, RsAllocationCubemapFace, uint32_t, uint32_t, const void*, size_t, size_t);
typedef void (*Allocation3DDataFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void*, size_t, size_t);
typedef void (*AllocationGenerateMipmapsFnPtr) (RsContext, RsAllocation);
typedef void (*AllocationReadFnPtr) (RsContext, RsAllocation, void*, size_t);
typedef void (*Allocation1DReadFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, void*, size_t);
typedef void (*AllocationElementReadFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, void*, size_t, size_t);
typedef void (*Allocation2DReadFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, RsAllocationCubemapFace, uint32_t, uint32_t, void*, size_t, size_t);
typedef void (*Allocation3DReadFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*, size_t, size_t);
typedef void (*AllocationSyncAllFnPtr) (RsContext, RsAllocation, RsAllocationUsageType);
typedef void (*AllocationResize1DFnPtr) (RsContext, RsAllocation, uint32_t);
typedef void (*AllocationCopy2DRangeFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t);
typedef void (*AllocationCopy3DRangeFnPtr) (RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t);
typedef void (*AllocationSetupBufferQueueFnPtr) (RsContext context, RsAllocation valloc, uint32_t numAlloc);
typedef void (*AllocationShareBufferQueueFnPtr) (RsContext context, RsAllocation valloc1, RsAllocation valloc2);
typedef RsSampler (*SamplerCreateFnPtr) (RsContext, RsSamplerValue, RsSamplerValue, RsSamplerValue, RsSamplerValue, RsSamplerValue, float);
typedef void (*ScriptBindAllocationFnPtr) (RsContext, RsScript, RsAllocation, uint32_t);
typedef void (*ScriptSetTimeZoneFnPtr) (RsContext, RsScript, const char*, size_t);
typedef void (*ScriptInvokeFnPtr) (RsContext, RsScript, uint32_t);
typedef void (*ScriptInvokeVFnPtr) (RsContext, RsScript, uint32_t, const void*, size_t);
typedef void (*ScriptForEachFnPtr) (RsContext, RsScript, uint32_t, RsAllocation, RsAllocation, const void*, size_t, const RsScriptCall*, size_t);
typedef void (*ScriptForEachMultiFnPtr) (RsContext, RsScript, uint32_t, RsAllocation*, size_t, RsAllocation, const void*, size_t, const RsScriptCall*, size_t);
typedef void (*ScriptReduceFnPtr) (RsContext, RsScript, uint32_t, RsAllocation*, size_t, RsAllocation, const RsScriptCall*, size_t);
typedef void (*ScriptSetVarIFnPtr) (RsContext, RsScript, uint32_t, int);
typedef void (*ScriptSetVarObjFnPtr) (RsContext, RsScript, uint32_t, RsObjectBase);
typedef void (*ScriptSetVarJFnPtr) (RsContext, RsScript, uint32_t, int64_t);
typedef void (*ScriptSetVarFFnPtr) (RsContext, RsScript, uint32_t, float);
typedef void (*ScriptSetVarDFnPtr) (RsContext, RsScript, uint32_t, double);
typedef void (*ScriptSetVarVFnPtr) (RsContext, RsScript, uint32_t, const void*, size_t);
typedef void (*ScriptGetVarVFnPtr) (RsContext, RsScript, uint32_t, void*, size_t);
typedef void (*ScriptSetVarVEFnPtr) (RsContext, RsScript, uint32_t, const void*, size_t, RsElement, const uint32_t*, size_t);
typedef RsScript (*ScriptCCreateFnPtr) (RsContext, const char*, size_t, const char*, size_t, const char*, size_t);
typedef RsScript (*ScriptIntrinsicCreateFnPtr) (RsContext, uint32_t id, RsElement);
typedef RsScriptKernelID (*ScriptKernelIDCreateFnPtr) (RsContext, RsScript, int, int);
typedef RsScriptInvokeID (*ScriptInvokeIDCreateFnPtr) (RsContext, RsScript, int);
typedef RsScriptFieldID (*ScriptFieldIDCreateFnPtr) (RsContext, RsScript, int);
typedef RsScriptGroup (*ScriptGroupCreateFnPtr) (RsContext, RsScriptKernelID*, size_t, RsScriptKernelID*, size_t, RsScriptKernelID*, size_t, RsScriptFieldID*, size_t, const RsType*, size_t);
typedef RsScriptGroup2 (*ScriptGroup2CreateFnPtr)(RsContext, const char*, size_t, const char*, size_t, RsClosure*, size_t);
typedef void (*ScriptGroupSetOutputFnPtr) (RsContext, RsScriptGroup, RsScriptKernelID, RsAllocation);
typedef void (*ScriptGroupSetInputFnPtr) (RsContext, RsScriptGroup, RsScriptKernelID, RsAllocation);
typedef void (*ScriptGroupExecuteFnPtr) (RsContext, RsScriptGroup);
typedef void (*AllocationIoSendFnPtr) (RsContext, RsAllocation);
typedef int64_t (*AllocationIoReceiveFnPtr) (RsContext, RsAllocation);
typedef void * (*AllocationGetPointerFnPtr) (RsContext, RsAllocation, uint32_t lod, RsAllocationCubemapFace face, uint32_t z, uint32_t array, size_t *stride, size_t stride_len);
typedef RsAllocation (*AllocationAdapterCreateFnPtr) (RsContext rsc, RsType vtype, RsAllocation baseAlloc);
typedef void (*AllocationAdapterOffsetFnPtr) (RsContext rsc, RsAllocation alloc, const uint32_t * offsets, size_t offsets_length);

// Graphics APIs
typedef RsContext (*ContextCreateGLFnPtr) (RsDevice vdev, uint32_t version, uint32_t sdkVersion, RsSurfaceConfig sc, uint32_t dpi);
typedef RsProgramStore (*ProgramStoreCreateFnPtr) (RsContext rsc, bool colorMaskR, bool colorMaskG, bool colorMaskB, bool colorMaskA, bool depthMask, bool ditherEnable, RsBlendSrcFunc srcFunc, RsBlendDstFunc destFunc, RsDepthFunc depthFunc);
typedef RsProgramRaster (*ProgramRasterCreateFnPtr) (RsContext rsc, bool pointSprite, RsCullMode cull);
typedef void (*ProgramBindConstantsFnPtr) (RsContext rsc, RsProgram vp, uint32_t slot, RsAllocation constants);
typedef void (*ProgramBindTextureFnPtr) (RsContext rsc, RsProgramFragment pf, uint32_t slot, RsAllocation a);
typedef void (*ProgramBindSamplerFnPtr) (RsContext rsc, RsProgramFragment pf, uint32_t slot, RsSampler s);
typedef RsProgramFragment (*ProgramFragmentCreateFnPtr) (RsContext rsc, const char * shaderText, size_t shaderText_length, const char ** textureNames, size_t textureNames_length_length, const size_t * textureNames_length, const uintptr_t * params, size_t params_length);
typedef RsProgramVertex (*ProgramVertexCreateFnPtr) (RsContext rsc, const char * shaderText, size_t shaderText_length, const char ** textureNames, size_t textureNames_length_length, const size_t * textureNames_length, const uintptr_t * params, size_t params_length);
typedef RsFont (*FontCreateFromFileFnPtr) (RsContext rsc, const char * name, size_t name_length, float fontSize, uint32_t dpi);
typedef RsFont (*FontCreateFromMemoryFnPtr) (RsContext rsc, const char * name, size_t name_length, float fontSize, uint32_t dpi, const void * data, size_t data_length);
typedef RsMesh (*MeshCreateFnPtr) (RsContext rsc, RsAllocation * vtx, size_t vtx_length, RsAllocation * idx, size_t idx_length, uint32_t * primType, size_t primType_length);
typedef void (*ContextBindProgramStoreFnPtr) (RsContext rsc, RsProgramStore pgm);
typedef void (*ContextBindProgramFragmentFnPtr) (RsContext rsc, RsProgramFragment pgm);
typedef void (*ContextBindProgramVertexFnPtr) (RsContext rsc, RsProgramVertex pgm);
typedef void (*ContextBindProgramRasterFnPtr) (RsContext rsc, RsProgramRaster pgm);
typedef void (*ContextBindFontFnPtr) (RsContext rsc, RsFont pgm);
typedef void (*ContextSetSurfaceFnPtr) (RsContext rsc, uint32_t width, uint32_t height, RsNativeWindow sur);
typedef void (*ContextBindRootScriptFnPtr) (RsContext rsc, RsScript sampler);
typedef void (*ContextPauseFnPtr) (RsContext rsc);
typedef void (*ContextResumeFnPtr) (RsContext rsc);
typedef void (*MeshGetVertexBufferCountFnPtr) (RsContext con, RsMesh mv, int32_t *numVtx);
typedef void (*MeshGetIndexCountFnPtr) (RsContext con, RsMesh mv, int32_t *numIdx);
typedef void (*MeshGetVerticesFnPtr) (RsContext con, RsMesh mv, RsAllocation *vtxData, uint32_t vtxDataCount);
typedef void (*MeshGetIndicesFnPtr) (RsContext con, RsMesh mv, RsAllocation *va, uint32_t *primType, uint32_t idxDataCount);
typedef RsObjectBase (*FileA3DGetEntryByIndexFnPtr) (RsContext con, uint32_t index, RsFile file);
typedef void (*FileA3DGetNumIndexEntriesFnPtr) (RsContext con, int32_t *numEntries, RsFile file);
typedef void (*FileA3DGetIndexEntriesFnPtr) (RsContext con, RsFileIndexEntry *fileEntries, uint32_t numEntries, RsFile file);
typedef RsFile (*FileA3DCreateFromMemoryFnPtr) (RsContext con, const void *data, uint32_t len);
typedef RsFile (*FileA3DCreateFromAssetFnPtr) (RsContext con, void *_asset);
typedef RsFile (*FileA3DCreateFromFileFnPtr) (RsContext con, const char *path);


struct dispatchTable {
    SetNativeLibDirFnPtr SetNativeLibDir;

    // Inserted by hand based on rs.spec and rs.h
    Allocation1DDataFnPtr Allocation1DData;
    Allocation1DElementDataFnPtr Allocation1DElementData;
    Allocation1DReadFnPtr Allocation1DRead;
    Allocation2DDataFnPtr Allocation2DData;
    Allocation2DReadFnPtr Allocation2DRead;
    Allocation3DDataFnPtr Allocation3DData;
    Allocation3DReadFnPtr Allocation3DRead;
    AllocationAdapterCreateFnPtr AllocationAdapterCreate;
    AllocationAdapterOffsetFnPtr AllocationAdapterOffset;
    AllocationCopy2DRangeFnPtr AllocationCopy2DRange;
    AllocationCopy3DRangeFnPtr AllocationCopy3DRange;
    AllocationCopyToBitmapFnPtr AllocationCopyToBitmap;
    AllocationCreateFromBitmapFnPtr AllocationCreateFromBitmap;
    AllocationCreateStridedFnPtr AllocationCreateStrided;
    AllocationCreateTypedFnPtr AllocationCreateTyped;
    AllocationCubeCreateFromBitmapFnPtr AllocationCubeCreateFromBitmap;
    AllocationElementDataFnPtr AllocationElementData;
    AllocationElementReadFnPtr AllocationElementRead;
    AllocationGenerateMipmapsFnPtr AllocationGenerateMipmaps;
    AllocationGetPointerFnPtr AllocationGetPointer;
    AllocationGetSurfaceFnPtr AllocationGetSurface;
    AllocationGetTypeFnPtr AllocationGetType;
    AllocationIoReceiveFnPtr AllocationIoReceive;
    AllocationIoSendFnPtr AllocationIoSend;
    AllocationReadFnPtr AllocationRead;
    AllocationResize1DFnPtr AllocationResize1D;
    AllocationSetSurfaceFnPtr AllocationSetSurface;
    AllocationSyncAllFnPtr AllocationSyncAll;
    AllocationSetupBufferQueueFnPtr AllocationSetupBufferQueue;
    AllocationShareBufferQueueFnPtr AllocationShareBufferQueue;
    AssignNameFnPtr AssignName;
    ClosureCreateFnPtr ClosureCreate;
    ClosureSetArgFnPtr ClosureSetArg;
    ClosureSetGlobalFnPtr ClosureSetGlobal;
    ContextCreateFnPtr ContextCreate;
    ContextCreateVendorFnPtr ContextCreateVendor;
    ContextDeinitToClientFnPtr ContextDeinitToClient;
    ContextDestroyFnPtr ContextDestroy;
    ContextDumpFnPtr ContextDump;
    ContextFinishFnPtr ContextFinish;
    ContextGetMessageFnPtr ContextGetMessage;
    ContextInitToClientFnPtr ContextInitToClient;
    ContextPeekMessageFnPtr ContextPeekMessage;
    ContextSendMessageFnPtr ContextSendMessage;
    ContextSetPriorityFnPtr ContextSetPriority;
    ContextSetCacheDirFnPtr ContextSetCacheDir;
    DeviceCreateFnPtr DeviceCreate;
    DeviceDestroyFnPtr DeviceDestroy;
    DeviceSetConfigFnPtr DeviceSetConfig;
    ElementCreate2FnPtr ElementCreate2;
    ElementCreateFnPtr ElementCreate;
    ElementGetNativeDataFnPtr ElementGetNativeData;
    ElementGetSubElementsFnPtr ElementGetSubElements;
    GetNameFnPtr GetName;
    InvokeClosureCreateFnPtr InvokeClosureCreate;
    ObjDestroyFnPtr ObjDestroy;
    SamplerCreateFnPtr SamplerCreate;
    ScriptBindAllocationFnPtr ScriptBindAllocation;
    ScriptCCreateFnPtr ScriptCCreate;
    ScriptFieldIDCreateFnPtr ScriptFieldIDCreate;
    ScriptForEachFnPtr ScriptForEach;
    ScriptForEachMultiFnPtr ScriptForEachMulti;
    ScriptGetVarVFnPtr ScriptGetVarV;
    ScriptGroup2CreateFnPtr ScriptGroup2Create;
    ScriptGroupCreateFnPtr ScriptGroupCreate;
    ScriptGroupExecuteFnPtr ScriptGroupExecute;
    ScriptGroupSetInputFnPtr ScriptGroupSetInput;
    ScriptGroupSetOutputFnPtr ScriptGroupSetOutput;
    ScriptIntrinsicCreateFnPtr ScriptIntrinsicCreate;
    ScriptInvokeFnPtr ScriptInvoke;
    ScriptInvokeIDCreateFnPtr ScriptInvokeIDCreate;
    ScriptInvokeVFnPtr ScriptInvokeV;
    ScriptKernelIDCreateFnPtr ScriptKernelIDCreate;
    ScriptReduceFnPtr ScriptReduce;
    ScriptSetTimeZoneFnPtr ScriptSetTimeZone;
    ScriptSetVarDFnPtr ScriptSetVarD;
    ScriptSetVarFFnPtr ScriptSetVarF;
    ScriptSetVarIFnPtr ScriptSetVarI;
    ScriptSetVarJFnPtr ScriptSetVarJ;
    ScriptSetVarObjFnPtr ScriptSetVarObj;
    ScriptSetVarVEFnPtr ScriptSetVarVE;
    ScriptSetVarVFnPtr ScriptSetVarV;
    TypeCreateFnPtr TypeCreate;
    TypeGetNativeDataFnPtr TypeGetNativeData;

    // Graphics API entries
    ContextCreateGLFnPtr ContextCreateGL;
    ContextPauseFnPtr ContextPause;
    ContextResumeFnPtr ContextResume;
    ContextBindProgramStoreFnPtr ContextBindProgramStore;
    ContextBindProgramFragmentFnPtr ContextBindProgramFragment;
    ContextBindProgramVertexFnPtr ContextBindProgramVertex;
    ContextBindProgramRasterFnPtr ContextBindProgramRaster;
    ContextBindFontFnPtr ContextBindFont;
    ContextSetSurfaceFnPtr ContextSetSurface;
    ContextBindRootScriptFnPtr ContextBindRootScript;
    ProgramStoreCreateFnPtr ProgramStoreCreate;
    ProgramRasterCreateFnPtr ProgramRasterCreate;
    ProgramBindConstantsFnPtr ProgramBindConstants;
    ProgramBindTextureFnPtr ProgramBindTexture;
    ProgramBindSamplerFnPtr ProgramBindSampler;
    ProgramFragmentCreateFnPtr ProgramFragmentCreate;
    ProgramVertexCreateFnPtr ProgramVertexCreate;
    FontCreateFromFileFnPtr FontCreateFromFile;
    FontCreateFromMemoryFnPtr FontCreateFromMemory;
    MeshCreateFnPtr MeshCreate;
    MeshGetVertexBufferCountFnPtr MeshGetVertexBufferCount;
    MeshGetIndexCountFnPtr MeshGetIndexCount;
    MeshGetVerticesFnPtr MeshGetVertices;
    MeshGetIndicesFnPtr MeshGetIndices;
    FileA3DGetEntryByIndexFnPtr FileA3DGetEntryByIndex;
    FileA3DGetNumIndexEntriesFnPtr FileA3DGetNumIndexEntries;
    FileA3DGetIndexEntriesFnPtr FileA3DGetIndexEntries;
    FileA3DCreateFromMemoryFnPtr FileA3DCreateFromMemory;
    FileA3DCreateFromAssetFnPtr FileA3DCreateFromAsset;
    FileA3DCreateFromFileFnPtr FileA3DCreateFromFile;
};

bool loadSymbols(void* handle, dispatchTable& dispatchTab, int device_api = 0);

#ifdef RS_COMPATIBILITY_LIB
#include "jni.h"

// USAGE_IO for RS Support lib
typedef void (*sAllocationSetSurfaceFnPtr) (JNIEnv *, jobject, RsContext, RsAllocation, RsNativeWindow, dispatchTable);
struct ioSuppDT {
    // USAGE_IO_OUTPUT
    sAllocationSetSurfaceFnPtr sAllocationSetSurface;
};

bool loadIOSuppSyms(void* handleIO, ioSuppDT& ioDispatch);
#endif // RS_COMPATIBILITY_LIB

#endif
