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
#define LOG_TAG "libDispatch"
#include <android/log.h>

#include "rsDispatch.h"
#include <dlfcn.h>
#include <limits.h>

#define LOG_ERR(...) __android_log_print(ANDROID_LOG_ERROR, "RS Dispatch", __VA_ARGS__);
#define REDUCE_API_LEVEL 24

bool loadSymbols(void* handle, dispatchTable& dispatchTab, int targetApiLevel) {
#ifdef __LP64__
    // Function to set the native lib path for 64bit compat lib.
    dispatchTab.SetNativeLibDir = (SetNativeLibDirFnPtr)dlsym(handle, "rsaContextSetNativeLibDir");
    if (dispatchTab.SetNativeLibDir == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.SetNativeLibDir");
        return false;
    }
#endif

    dispatchTab.Allocation1DData = (Allocation1DDataFnPtr)dlsym(handle, "rsAllocation1DData");
    dispatchTab.Allocation1DElementData = (Allocation1DElementDataFnPtr)dlsym(handle, "rsAllocation1DElementData");
    dispatchTab.Allocation1DRead = (Allocation1DReadFnPtr)dlsym(handle, "rsAllocation1DRead");
    dispatchTab.Allocation2DData = (Allocation2DDataFnPtr)dlsym(handle, "rsAllocation2DData");
    dispatchTab.Allocation2DRead = (Allocation2DReadFnPtr)dlsym(handle, "rsAllocation2DRead");
    dispatchTab.Allocation3DData = (Allocation3DDataFnPtr)dlsym(handle, "rsAllocation3DData");
    dispatchTab.Allocation3DRead = (Allocation3DReadFnPtr)dlsym(handle, "rsAllocation3DRead");
    dispatchTab.AllocationAdapterCreate = (AllocationAdapterCreateFnPtr)dlsym(handle, "rsAllocationAdapterCreate");
    dispatchTab.AllocationAdapterOffset = (AllocationAdapterOffsetFnPtr)dlsym(handle, "rsAllocationAdapterOffset");
    dispatchTab.AllocationCopy2DRange = (AllocationCopy2DRangeFnPtr)dlsym(handle, "rsAllocationCopy2DRange");
    dispatchTab.AllocationCopy3DRange = (AllocationCopy3DRangeFnPtr)dlsym(handle, "rsAllocationCopy3DRange");
    dispatchTab.AllocationCopyToBitmap = (AllocationCopyToBitmapFnPtr)dlsym(handle, "rsAllocationCopyToBitmap");
    dispatchTab.AllocationCreateFromBitmap = (AllocationCreateFromBitmapFnPtr)dlsym(handle, "rsAllocationCreateFromBitmap");
    dispatchTab.AllocationCreateTyped = (AllocationCreateTypedFnPtr)dlsym(handle, "rsAllocationCreateTyped");
    dispatchTab.AllocationCubeCreateFromBitmap = (AllocationCubeCreateFromBitmapFnPtr)dlsym(handle, "rsAllocationCubeCreateFromBitmap");
    dispatchTab.AllocationElementData = (AllocationElementDataFnPtr)dlsym(handle, "rsAllocationElementData");
    dispatchTab.AllocationElementRead = (AllocationElementReadFnPtr)dlsym(handle, "rsAllocationElementRead");
    dispatchTab.AllocationGenerateMipmaps = (AllocationGenerateMipmapsFnPtr)dlsym(handle, "rsAllocationGenerateMipmaps");
    dispatchTab.AllocationGetPointer = (AllocationGetPointerFnPtr)dlsym(handle, "rsAllocationGetPointer");
    dispatchTab.AllocationGetSurface = (AllocationGetSurfaceFnPtr)dlsym(handle, "rsAllocationGetSurface");
    dispatchTab.AllocationGetType = (AllocationGetTypeFnPtr)dlsym(handle, "rsaAllocationGetType");
    dispatchTab.AllocationIoReceive = (AllocationIoReceiveFnPtr)dlsym(handle, "rsAllocationIoReceive");
    dispatchTab.AllocationIoSend = (AllocationIoSendFnPtr)dlsym(handle, "rsAllocationIoSend");
    dispatchTab.AllocationRead = (AllocationReadFnPtr)dlsym(handle, "rsAllocationRead");
    dispatchTab.AllocationResize1D = (AllocationResize1DFnPtr)dlsym(handle, "rsAllocationResize1D");
    dispatchTab.AllocationSetSurface = (AllocationSetSurfaceFnPtr)dlsym(handle, "rsAllocationSetSurface");
    dispatchTab.AllocationSetupBufferQueue = (AllocationSetupBufferQueueFnPtr)dlsym(handle, "rsAllocationSetupBufferQueue");
    dispatchTab.AllocationShareBufferQueue = (AllocationShareBufferQueueFnPtr)dlsym(handle, "rsAllocationShareBufferQueue");
    dispatchTab.AllocationSyncAll = (AllocationSyncAllFnPtr)dlsym(handle, "rsAllocationSyncAll");
    dispatchTab.AssignName = (AssignNameFnPtr)dlsym(handle, "rsAssignName");
    dispatchTab.ClosureCreate = (ClosureCreateFnPtr)dlsym(handle, "rsClosureCreate");
    dispatchTab.ClosureSetArg = (ClosureSetArgFnPtr)dlsym(handle, "rsClosureSetArg");
    dispatchTab.ClosureSetGlobal = (ClosureSetGlobalFnPtr)dlsym(handle, "rsClosureSetGlobal");
    dispatchTab.ContextCreate = (ContextCreateFnPtr)dlsym(handle, "rsContextCreate");;
    dispatchTab.ContextDeinitToClient = (ContextDeinitToClientFnPtr)dlsym(handle, "rsContextDeinitToClient");
    dispatchTab.ContextDestroy = (ContextDestroyFnPtr)dlsym(handle, "rsContextDestroy");
    dispatchTab.ContextDump = (ContextDumpFnPtr)dlsym(handle, "rsContextDump");
    dispatchTab.ContextFinish = (ContextFinishFnPtr)dlsym(handle, "rsContextFinish");
    dispatchTab.ContextGetMessage = (ContextGetMessageFnPtr)dlsym(handle, "rsContextGetMessage");
    dispatchTab.ContextInitToClient = (ContextInitToClientFnPtr)dlsym(handle, "rsContextInitToClient");
    dispatchTab.ContextPeekMessage = (ContextPeekMessageFnPtr)dlsym(handle, "rsContextPeekMessage");
    dispatchTab.ContextSendMessage = (ContextSendMessageFnPtr)dlsym(handle, "rsContextSendMessage");
    dispatchTab.ContextSetPriority = (ContextSetPriorityFnPtr)dlsym(handle, "rsContextSetPriority");
    dispatchTab.ContextSetCacheDir = (ContextSetCacheDirFnPtr)dlsym(handle, "rsContextSetCacheDir");
    dispatchTab.DeviceCreate = (DeviceCreateFnPtr)dlsym(handle, "rsDeviceCreate");
    dispatchTab.DeviceDestroy = (DeviceDestroyFnPtr)dlsym(handle, "rsDeviceDestroy");
    dispatchTab.DeviceSetConfig = (DeviceSetConfigFnPtr)dlsym(handle, "rsDeviceSetConfig");
    dispatchTab.ElementCreate = (ElementCreateFnPtr)dlsym(handle, "rsElementCreate");
    dispatchTab.ElementCreate2 = (ElementCreate2FnPtr)dlsym(handle, "rsElementCreate2");
    dispatchTab.ElementGetNativeData = (ElementGetNativeDataFnPtr)dlsym(handle, "rsaElementGetNativeData");
    dispatchTab.ElementGetSubElements = (ElementGetSubElementsFnPtr)dlsym(handle, "rsaElementGetSubElements");
    dispatchTab.GetName = (GetNameFnPtr)dlsym(handle, "rsaGetName");;
    dispatchTab.InvokeClosureCreate = (InvokeClosureCreateFnPtr)dlsym(handle, "rsInvokeClosureCreate");
    dispatchTab.ObjDestroy = (ObjDestroyFnPtr)dlsym(handle, "rsObjDestroy");
    dispatchTab.SamplerCreate = (SamplerCreateFnPtr)dlsym(handle, "rsSamplerCreate");
    dispatchTab.ScriptBindAllocation = (ScriptBindAllocationFnPtr)dlsym(handle, "rsScriptBindAllocation");
    dispatchTab.ScriptCCreate = (ScriptCCreateFnPtr)dlsym(handle, "rsScriptCCreate");
    dispatchTab.ScriptFieldIDCreate = (ScriptFieldIDCreateFnPtr)dlsym(handle, "rsScriptFieldIDCreate");
    dispatchTab.ScriptForEach = (ScriptForEachFnPtr)dlsym(handle, "rsScriptForEach");
    dispatchTab.ScriptForEachMulti = (ScriptForEachMultiFnPtr)dlsym(handle, "rsScriptForEachMulti");
    dispatchTab.ScriptGetVarV = (ScriptGetVarVFnPtr)dlsym(handle, "rsScriptGetVarV");
    dispatchTab.ScriptGroup2Create = (ScriptGroup2CreateFnPtr)dlsym(handle, "rsScriptGroup2Create");
    dispatchTab.ScriptGroupCreate = (ScriptGroupCreateFnPtr)dlsym(handle, "rsScriptGroupCreate");
    dispatchTab.ScriptGroupExecute = (ScriptGroupExecuteFnPtr)dlsym(handle, "rsScriptGroupExecute");
    dispatchTab.ScriptGroupSetInput = (ScriptGroupSetInputFnPtr)dlsym(handle, "rsScriptGroupSetInput");
    dispatchTab.ScriptGroupSetOutput = (ScriptGroupSetOutputFnPtr)dlsym(handle, "rsScriptGroupSetOutput");
    dispatchTab.ScriptIntrinsicCreate = (ScriptIntrinsicCreateFnPtr)dlsym(handle, "rsScriptIntrinsicCreate");
    dispatchTab.ScriptInvoke = (ScriptInvokeFnPtr)dlsym(handle, "rsScriptInvoke");
    dispatchTab.ScriptInvokeIDCreate = (ScriptInvokeIDCreateFnPtr)dlsym(handle, "rsScriptInvokeIDCreate");
    dispatchTab.ScriptInvokeV = (ScriptInvokeVFnPtr)dlsym(handle, "rsScriptInvokeV");
    dispatchTab.ScriptKernelIDCreate = (ScriptKernelIDCreateFnPtr)dlsym(handle, "rsScriptKernelIDCreate");
    dispatchTab.ScriptReduce = (ScriptReduceFnPtr)dlsym(handle, "rsScriptReduce");
    dispatchTab.ScriptSetTimeZone = (ScriptSetTimeZoneFnPtr)dlsym(handle, "rsScriptSetTimeZone");
    dispatchTab.ScriptSetVarD = (ScriptSetVarDFnPtr)dlsym(handle, "rsScriptSetVarD");
    dispatchTab.ScriptSetVarF = (ScriptSetVarFFnPtr)dlsym(handle, "rsScriptSetVarF");
    dispatchTab.ScriptSetVarI = (ScriptSetVarIFnPtr)dlsym(handle, "rsScriptSetVarI");
    dispatchTab.ScriptSetVarJ = (ScriptSetVarJFnPtr)dlsym(handle, "rsScriptSetVarJ");
    dispatchTab.ScriptSetVarObj = (ScriptSetVarObjFnPtr)dlsym(handle, "rsScriptSetVarObj");
    dispatchTab.ScriptSetVarV = (ScriptSetVarVFnPtr)dlsym(handle, "rsScriptSetVarV");
    dispatchTab.ScriptSetVarVE = (ScriptSetVarVEFnPtr)dlsym(handle, "rsScriptSetVarVE");
    dispatchTab.TypeCreate = (TypeCreateFnPtr)dlsym(handle, "rsTypeCreate");
    dispatchTab.TypeGetNativeData = (TypeGetNativeDataFnPtr)dlsym(handle, "rsaTypeGetNativeData");

    // Load graphics APIs
    dispatchTab.ContextCreateGL = (ContextCreateGLFnPtr)dlsym(handle, "rsContextCreateGL");
    dispatchTab.ContextSetSurface = (ContextSetSurfaceFnPtr)dlsym(handle, "rsContextSetSurface");
    dispatchTab.ContextPause = (ContextPauseFnPtr)dlsym(handle, "rsContextPause");
    dispatchTab.ContextResume = (ContextResumeFnPtr)dlsym(handle, "rsContextResume");
    dispatchTab.ContextBindProgramStore = (ContextBindProgramStoreFnPtr)dlsym(handle, "rsContextBindProgramStore");
    dispatchTab.ContextBindProgramFragment = (ContextBindProgramFragmentFnPtr)dlsym(handle, "rsContextBindProgramFragment");
    dispatchTab.ContextBindProgramVertex = (ContextBindProgramVertexFnPtr)dlsym(handle, "rsContextBindProgramVertex");
    dispatchTab.ContextBindProgramRaster = (ContextBindProgramRasterFnPtr)dlsym(handle, "rsContextBindProgramRaster");
    dispatchTab.ContextBindFont = (ContextBindFontFnPtr)dlsym(handle, "rsContextBindFont");
    dispatchTab.ContextBindRootScript = (ContextBindRootScriptFnPtr)dlsym(handle, "rsContextBindRootScript");

    dispatchTab.ProgramStoreCreate = (ProgramStoreCreateFnPtr)dlsym(handle, "rsProgramStoreCreate");
    dispatchTab.ProgramRasterCreate = (ProgramRasterCreateFnPtr)dlsym(handle, "rsProgramRasterCreate");
    dispatchTab.ProgramBindConstants = (ProgramBindConstantsFnPtr)dlsym(handle, "rsProgramBindConstants");
    dispatchTab.ProgramBindTexture = (ProgramBindTextureFnPtr)dlsym(handle, "rsProgramBindTexture");
    dispatchTab.ProgramBindSampler = (ProgramBindSamplerFnPtr)dlsym(handle, "rsProgramBindSampler");
    dispatchTab.ProgramFragmentCreate = (ProgramFragmentCreateFnPtr)dlsym(handle, "rsProgramFragmentCreate");
    dispatchTab.ProgramVertexCreate = (ProgramVertexCreateFnPtr)dlsym(handle, "rsProgramVertexCreate");
    dispatchTab.FontCreateFromFile = (FontCreateFromFileFnPtr)dlsym(handle, "rsFontCreateFromFile");
    dispatchTab.FontCreateFromMemory = (FontCreateFromMemoryFnPtr)dlsym(handle, "rsFontCreateFromMemory");
    dispatchTab.MeshCreate = (MeshCreateFnPtr)dlsym(handle, "rsMeshCreate");
    dispatchTab.MeshGetVertexBufferCount = (MeshGetVertexBufferCountFnPtr)dlsym(handle, "rsaMeshGetVertexBufferCount");
    dispatchTab.MeshGetIndexCount = (MeshGetIndexCountFnPtr)dlsym(handle, "rsaMeshGetIndexCount");
    dispatchTab.MeshGetVertices = (MeshGetVerticesFnPtr)dlsym(handle, "rsaMeshGetVertices");
    dispatchTab.MeshGetIndices = (MeshGetIndicesFnPtr)dlsym(handle, "rsaMeshGetIndices");
    dispatchTab.FileA3DGetEntryByIndex = (FileA3DGetEntryByIndexFnPtr)dlsym(handle, "rsaFileA3DGetEntryByIndex");
    dispatchTab.FileA3DGetNumIndexEntries = (FileA3DGetNumIndexEntriesFnPtr)dlsym(handle, "rsaFileA3DGetNumIndexEntries");
    dispatchTab.FileA3DGetIndexEntries = (FileA3DGetIndexEntriesFnPtr)dlsym(handle, "rsaFileA3DGetIndexEntries");
    dispatchTab.FileA3DCreateFromMemory = (FileA3DCreateFromMemoryFnPtr)dlsym(handle, "rsaFileA3DCreateFromMemory");
    dispatchTab.FileA3DCreateFromAsset = (FileA3DCreateFromAssetFnPtr)dlsym(handle, "rsaFileA3DCreateFromAsset");
    dispatchTab.FileA3DCreateFromFile = (FileA3DCreateFromFileFnPtr)dlsym(handle, "rsaFileA3DCreateFromFile");

    // Clear error buffer for later operations.
    dlerror();

    if (dispatchTab.AllocationGetType == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationGetType");
        return false;
    }
    if (dispatchTab.TypeGetNativeData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.TypeGetNativeData");
        return false;
    }
    if (dispatchTab.ElementGetNativeData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ElementGetNativeData");
        return false;
    }
    if (dispatchTab.ElementGetSubElements == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ElementGetSubElements");
        return false;
    }
    if (dispatchTab.DeviceCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.DeviceCreate");
        return false;
    }
    if (dispatchTab.DeviceDestroy == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.DeviceDestroy");
        return false;
    }
    if (dispatchTab.DeviceSetConfig == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.DeviceSetConfig");
        return false;
    }
    if (dispatchTab.ContextCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextCreate");
        return false;
    }
    if (dispatchTab.GetName == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.GetName");
        return false;
    }
    if (dispatchTab.ContextDestroy == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextDestroy");
        return false;
    }
    if (dispatchTab.ContextGetMessage == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextGetMessage");
        return false;
    }
    if (dispatchTab.ContextPeekMessage == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextPeekMessage");
        return false;
    }
    if (dispatchTab.ContextSendMessage == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextSendMessage");
        return false;
    }
    if (dispatchTab.ContextInitToClient == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextInitToClient");
        return false;
    }
    if (dispatchTab.ContextDeinitToClient == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextDeinitToClient");
        return false;
    }
    if (dispatchTab.TypeCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.TypeCreate");
        return false;
    }
    if (dispatchTab.AllocationCreateTyped == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCreateTyped");
        return false;
    }
    if (dispatchTab.AllocationCreateFromBitmap == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCreateFromBitmap");
        return false;
    }
    if (dispatchTab.AllocationCubeCreateFromBitmap == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCubeCreateFromBitmap");
        return false;
    }
    if (dispatchTab.AllocationGetSurface == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationGetSurface");
        return false;
    }
    if (dispatchTab.AllocationSetSurface == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationSetSurface");
        return false;
    }
    if (dispatchTab.ContextFinish == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextFinish");
        return false;
    }
    if (dispatchTab.ContextDump == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextDump");
        return false;
    }
    if (dispatchTab.ContextSetPriority == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ContextSetPriority");
        return false;
    }
    if (dispatchTab.AssignName == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AssignName");
        return false;
    }
    if (dispatchTab.ObjDestroy == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ObjDestroy");
        return false;
    }
    if (dispatchTab.ElementCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ElementCreate");
        return false;
    }
    if (dispatchTab.ElementCreate2 == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ElementCreate2");
        return false;
    }
    if (dispatchTab.AllocationCopyToBitmap == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCopyToBitmap");
        return false;
    }
    if (dispatchTab.Allocation1DData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation1DData");
        return false;
    }
    if (dispatchTab.Allocation1DElementData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation1DElementData");
        return false;
    }
    if (dispatchTab.Allocation2DData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation2DData");
        return false;
    }
    if (dispatchTab.Allocation3DData == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation3DData");
        return false;
    }
    if (dispatchTab.AllocationGenerateMipmaps == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationGenerateMipmaps");
        return false;
    }
    if (dispatchTab.AllocationRead == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationRead");
        return false;
    }
    if (dispatchTab.Allocation1DRead == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation1DRead");
        return false;
    }
    if (dispatchTab.Allocation2DRead == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.Allocation2DRead");
        return false;
    }
    if (dispatchTab.AllocationSyncAll == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationSyncAll");
        return false;
    }
    if (dispatchTab.AllocationResize1D == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationResize1D");
        return false;
    }
    if (dispatchTab.AllocationCopy2DRange == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCopy2DRange");
        return false;
    }
    if (dispatchTab.AllocationCopy3DRange == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationCopy3DRange");
        return false;
    }
    if (dispatchTab.SamplerCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.SamplerCreate");
        return false;
    }
    if (dispatchTab.ScriptBindAllocation == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptBindAllocation");
        return false;
    }
    if (dispatchTab.ScriptSetTimeZone == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetTimeZone");
        return false;
    }
    if (dispatchTab.ScriptInvoke == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptInvoke");
        return false;
    }
    if (dispatchTab.ScriptInvokeV == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptInvokeV");
        return false;
    }
    if (dispatchTab.ScriptForEach == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptForEach");
        return false;
    }
    if (dispatchTab.ScriptSetVarI == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarI");
        return false;
    }
    if (dispatchTab.ScriptSetVarObj == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarObj");
        return false;
    }
    if (dispatchTab.ScriptSetVarJ == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarJ");
        return false;
    }
    if (dispatchTab.ScriptSetVarF == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarF");
        return false;
    }
    if (dispatchTab.ScriptSetVarD == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarD");
        return false;
    }
    if (dispatchTab.ScriptSetVarV == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarV");
        return false;
    }
    if (dispatchTab.ScriptGetVarV == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptGetVarV");
        return false;
    }
    if (dispatchTab.ScriptSetVarVE == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptSetVarVE");
        return false;
    }
    if (dispatchTab.ScriptCCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptCCreate");
        return false;
    }
    if (dispatchTab.ScriptIntrinsicCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptIntrinsicCreate");
        return false;
    }
    if (dispatchTab.ScriptKernelIDCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptKernelIDCreate");
        return false;
    }
    if (dispatchTab.ScriptFieldIDCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptFieldIDCreate");
        return false;
    }
    if (dispatchTab.ScriptGroupCreate == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptGroupCreate");
        return false;
    }
    if (dispatchTab.ScriptGroupSetOutput == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptGroupSetOutput");
        return false;
    }
    if (dispatchTab.ScriptGroupSetInput == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptGroupSetInput");
        return false;
    }
    if (dispatchTab.ScriptGroupExecute == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.ScriptGroupExecute");
        return false;
    }
    if (dispatchTab.AllocationIoSend == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationIoSend");
        return false;
    }
    if (dispatchTab.AllocationIoReceive == nullptr) {
        LOG_ERR("Couldn't initialize dispatchTab.AllocationIoReceive");
        return false;
    }
    // API_21 functions
    if (targetApiLevel >= 21) {
        if (dispatchTab.AllocationGetPointer == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.AllocationGetPointer");
            return false;
        }
    }
    // API_23 functions
    if (targetApiLevel >= 23) {
        // ScriptGroup V2 functions
        if (dispatchTab.ScriptInvokeIDCreate == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ScriptInvokeIDCreate");
            return false;
        }
        if (dispatchTab.ClosureCreate == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ClosureCreate");
            return false;
        }
        if (dispatchTab.InvokeClosureCreate == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.InvokeClosureCreate");
            return false;
        }
        if (dispatchTab.ClosureSetArg == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ClosureSetArg");
            return false;
        }
        if (dispatchTab.ClosureSetGlobal == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ClosureSetGlobal");
            return false;
        }
        if (dispatchTab.ScriptGroup2Create == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ScriptGroup2Create");
            return false;
        }
        if (dispatchTab.AllocationElementData == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.AllocationElementData");
            return false;
        }
        if (dispatchTab.AllocationElementRead == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.AllocationElementRead");
            return false;
        }
        if (dispatchTab.Allocation3DRead == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.Allocation3DRead");
            return false;
        }
        if (dispatchTab.ScriptForEachMulti == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ScriptForEachMulti");
            return false;
        }
    }

    if (targetApiLevel >= REDUCE_API_LEVEL) {
        if (dispatchTab.ScriptReduce == nullptr) {
            LOG_ERR("Couldn't initialize dispatchTab.ScriptReduce");
            return false;
        }
    }

    return true;

}


bool loadIOSuppSyms(void* handleIO, ioSuppDT& ioDispatch){
    ioDispatch.sAllocationSetSurface = (sAllocationSetSurfaceFnPtr)dlsym(handleIO, "AllocationSetSurface");
    if (ioDispatch.sAllocationSetSurface == nullptr) {
        LOG_ERR("Couldn't initialize ioDispatch.sAllocationSetSurface");
        return false;
    }
    return true;
}