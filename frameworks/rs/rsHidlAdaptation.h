/*
 * Copyright(C) 2017 The Android Open Source Project
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

#ifndef ANDROID_RS_HIDL_ADAPTATION_H
#define ANDROID_RS_HIDL_ADAPTATION_H

#include <android/hardware/renderscript/1.0/IDevice.h>
#include <set>
#include <mutex>

struct dispatchTable;

using ::android::hardware::renderscript::V1_0::IContext;

/*
 * RsHidlAdaptation is a singleton class.
 *
 * RsHidlAdaptation is used for the following purposes:
 * 1. Load the dispatch table with HIDL implementation or CPU fallback.
 * 2. Convert input paramters to HIDL types, when using HIDL path.
 * 3. Convert output parameters from HIDL types to RS types, when using HIDL path.
 *
 * Access of static member objects is protected by global teardown flag.
 * https://goto.google.com/rs-static-destructor
 *
 */
class RsHidlAdaptation
{
public:
    static RsHidlAdaptation& GetInstance();
    static const dispatchTable* GetEntryFuncs();

private:
    RsHidlAdaptation();
    void InitializeHalDeviceContext();
    void LoadDispatchForHidl();
    static IContext *GetIContextHandle(RsContext con);

    // HAL entries
    static RsContext ContextCreate(RsDevice vdev, uint32_t version, uint32_t sdkVersion, RsContextType ct, uint32_t flags);
    static void ContextDestroy(RsContext);
    static void ContextFinish(RsContext);
    static void ContextDump(RsContext, int32_t);
    static void ContextSetCacheDir(RsContext rsc, const char *cacheDir, size_t cacheDir_length);
    static void ContextSetPriority(RsContext, int32_t);
    static RsMessageToClientType ContextGetMessage(RsContext, void*, size_t, size_t*, size_t, uint32_t*, size_t);
    static RsMessageToClientType ContextPeekMessage(RsContext, size_t*, size_t, uint32_t*, size_t);
    static void ContextSendMessage(RsContext, uint32_t, const uint8_t*, size_t);
    static void ContextInitToClient(RsContext);
    static void ContextDeinitToClient(RsContext);

    static void AssignName(RsContext, RsObjectBase, const char*, size_t);
    static void GetName(RsContext, void * obj, const char **name);
    static void ObjDestroy(RsContext, RsAsyncVoidPtr);

    static RsElement ElementCreate(RsContext, RsDataType, RsDataKind, bool, uint32_t);
    static RsElement ElementCreate2(RsContext, const RsElement*, size_t, const char**, size_t, const size_t*, const uint32_t*, size_t);
    static void ElementGetNativeData(RsContext, RsElement, uint32_t *elemData, uint32_t elemDataSize);
    static void ElementGetSubElements(RsContext, RsElement, uintptr_t *ids, const char **names, size_t *arraySizes, uint32_t dataSize);

    static RsType TypeCreate(RsContext, RsElement, uint32_t, uint32_t, uint32_t, bool, bool, uint32_t);
    static void TypeGetNativeData(RsContext, RsType, uintptr_t *typeData, uint32_t typeDataSize);

    static RsAllocation AllocationCreateTyped(RsContext, RsType, RsAllocationMipmapControl, uint32_t, uintptr_t);
    static RsAllocation AllocationCreateFromBitmap(RsContext, RsType, RsAllocationMipmapControl, const void*, size_t, uint32_t);
    static RsAllocation AllocationCubeCreateFromBitmap(RsContext, RsType, RsAllocationMipmapControl, const void*, size_t, uint32_t);
    static const void* AllocationGetType(RsContext con, RsAllocation va);
    static RsNativeWindow AllocationGetSurface(RsContext, RsAllocation);
    static void AllocationSetSurface(RsContext, RsAllocation, RsNativeWindow);
    static void AllocationCopyToBitmap(RsContext, RsAllocation, void*, size_t);
    static void Allocation1DData(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, const void*, size_t);
    static void Allocation1DElementData(RsContext, RsAllocation, uint32_t, uint32_t, const void*, size_t, size_t);
    static void AllocationElementData(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, const void*, size_t, size_t);
    static void Allocation2DData(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, RsAllocationCubemapFace, uint32_t, uint32_t, const void*, size_t, size_t);
    static void Allocation3DData(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void*, size_t, size_t);
    static void AllocationGenerateMipmaps(RsContext, RsAllocation);
    static void AllocationRead(RsContext, RsAllocation, void*, size_t);
    static void Allocation1DRead(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, void*, size_t);
    static void AllocationElementRead(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, void*, size_t, size_t);
    static void Allocation2DRead(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, RsAllocationCubemapFace, uint32_t, uint32_t, void*, size_t, size_t);
    static void Allocation3DRead(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*, size_t, size_t);
    static void AllocationSyncAll(RsContext, RsAllocation, RsAllocationUsageType);
    static void AllocationResize1D(RsContext, RsAllocation, uint32_t);
    static void AllocationCopy2DRange(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t);
    static void AllocationCopy3DRange(RsContext, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, RsAllocation, uint32_t, uint32_t, uint32_t, uint32_t);
    static void AllocationIoSend(RsContext, RsAllocation);
    static int64_t AllocationIoReceive(RsContext, RsAllocation);
    static void *AllocationGetPointer(RsContext, RsAllocation, uint32_t lod, RsAllocationCubemapFace face, uint32_t z, uint32_t array, size_t *stride, size_t stride_len);
    static void AllocationSetupBufferQueue(RsContext context, RsAllocation valloc, uint32_t numAlloc);
    static void AllocationShareBufferQueue(RsContext context, RsAllocation valloc1, RsAllocation valloc2);
    static RsAllocation AllocationAdapterCreate(RsContext rsc, RsType vtype, RsAllocation baseAlloc);
    static void AllocationAdapterOffset(RsContext rsc, RsAllocation alloc, const uint32_t * offsets, size_t offsets_length);

    static RsScript ScriptCCreate(RsContext, const char*, size_t, const char*, size_t, const char*, size_t);
    static RsScript ScriptIntrinsicCreate(RsContext, uint32_t id, RsElement);
    static void ScriptBindAllocation(RsContext, RsScript, RsAllocation, uint32_t);
    static void ScriptSetTimeZone(RsContext, RsScript, const char*, size_t);
    static void ScriptInvoke(RsContext, RsScript, uint32_t);
    static void ScriptInvokeV(RsContext, RsScript, uint32_t, const void*, size_t);
    static void ScriptForEach(RsContext, RsScript, uint32_t, RsAllocation, RsAllocation, const void*, size_t, const RsScriptCall*, size_t);
    static void ScriptForEachMulti(RsContext, RsScript, uint32_t, RsAllocation*, size_t, RsAllocation, const void*, size_t, const RsScriptCall*, size_t);
    static void ScriptReduce(RsContext, RsScript, uint32_t, RsAllocation*, size_t, RsAllocation, const RsScriptCall*, size_t);
    static void ScriptSetVarI(RsContext, RsScript, uint32_t, int);
    static void ScriptSetVarObj(RsContext, RsScript, uint32_t, RsObjectBase);
    static void ScriptSetVarJ(RsContext, RsScript, uint32_t, int64_t);
    static void ScriptSetVarF(RsContext, RsScript, uint32_t, float);
    static void ScriptSetVarD(RsContext, RsScript, uint32_t, double);
    static void ScriptSetVarV(RsContext, RsScript, uint32_t, const void*, size_t);
    static void ScriptGetVarV(RsContext, RsScript, uint32_t, void*, size_t);
    static void ScriptSetVarVE(RsContext, RsScript, uint32_t, const void*, size_t, RsElement, const uint32_t*, size_t);

    static RsScriptGroup ScriptGroupCreate(RsContext, RsScriptKernelID*, size_t, RsScriptKernelID*, size_t, RsScriptKernelID*, size_t, RsScriptFieldID*, size_t, const RsType*, size_t);
    static RsScriptGroup2 ScriptGroup2Create(RsContext, const char*, size_t, const char*, size_t, RsClosure*, size_t);
    static RsScriptKernelID ScriptKernelIDCreate(RsContext, RsScript, int, int);
    static RsScriptInvokeID ScriptInvokeIDCreate(RsContext, RsScript, int);
    static RsScriptFieldID ScriptFieldIDCreate(RsContext, RsScript, int);
    static void ScriptGroupSetOutput(RsContext, RsScriptGroup, RsScriptKernelID, RsAllocation);
    static void ScriptGroupSetInput(RsContext, RsScriptGroup, RsScriptKernelID, RsAllocation);
    static void ScriptGroupExecute(RsContext, RsScriptGroup);
    static RsClosure ClosureCreate(RsContext context, RsScriptKernelID kernelID, RsAllocation returnValue, RsScriptFieldID* fieldIDs, size_t fieldIDs_length, int64_t* values, size_t values_length, int* sizes, size_t sizes_length, RsClosure* depClosures, size_t depClosures_length, RsScriptFieldID* depFieldIDs, size_t depFieldIDs_length);
    static RsClosure InvokeClosureCreate(RsContext, RsScriptInvokeID, const void*, const size_t, const RsScriptFieldID*, const size_t, const int64_t*, const size_t, const int*, const size_t);
    static void ClosureSetArg(RsContext, RsClosure, uint32_t, uintptr_t, int);
    static void ClosureSetGlobal(RsContext, RsClosure, RsScriptFieldID, int64_t, int);

    static RsSampler SamplerCreate(RsContext, RsSamplerValue, RsSamplerValue, RsSamplerValue, RsSamplerValue, RsSamplerValue, float);

    // Dispatch table entries for HIDL. The dispatch table is initialized by the
    // constructor of this singleton class, to avoid concurrency issues.
    static dispatchTable mEntryFuncs;

    // HIDL instance. This is a singleton HIDL instance, providing a handle for
    // RenderScript HIDL implementation (driver). The handle is created by the
    // constructor of this singleton class.
    static android::sp<android::hardware::renderscript::V1_0::IDevice> mHidl;

    // mContexts keeps Treble returned sp<IContext>s alive.
    static std::set<android::sp<IContext> > mContexts;

    // mMutex is used to protect concurrent modification to the mContexts set.
    // std::mutex is safe for pthreads on Android. Since other threading model
    // supported on Android are built on top of pthread, std::mutex is safe for them.
    static std::mutex mContextsMutex;
};

#endif // ANDROID_RS_HIDL_ADAPTATION_H
