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

#define LOG_TAG "RenderScript HIDL Adaptation"

#include "cpp/rsDispatch.h"
#include "rsHidlAdaptation.h"
#include "rsFallbackAdaptation.h"

#include <cutils/properties.h>
#include <log/log.h>

using ::android::hardware::renderscript::V1_0::IDevice;
using ::android::hardware::renderscript::V1_0::IContext;
using ::android::hardware::renderscript::V1_0::Allocation;
using ::android::hardware::renderscript::V1_0::AllocationCubemapFace;
using ::android::hardware::renderscript::V1_0::AllocationMipmapControl;
using ::android::hardware::renderscript::V1_0::AllocationUsageType;
using ::android::hardware::renderscript::V1_0::Closure;
using ::android::hardware::renderscript::V1_0::ContextType;
using ::android::hardware::renderscript::V1_0::DataKind;
using ::android::hardware::renderscript::V1_0::DataType;
using ::android::hardware::renderscript::V1_0::ForEachStrategy;
using ::android::hardware::renderscript::V1_0::MessageToClientType;
using ::android::hardware::renderscript::V1_0::SamplerValue;
using ::android::hardware::renderscript::V1_0::ScriptCall;
using ::android::hardware::renderscript::V1_0::ScriptFieldID;
using ::android::hardware::renderscript::V1_0::ScriptKernelID;
using ::android::hardware::renderscript::V1_0::ScriptIntrinsicID;
using ::android::hardware::renderscript::V1_0::ThreadPriorities;
using ::android::hardware::renderscript::V1_0::Type;
using ::android::hardware::renderscript::V1_0::YuvFormat;
using ::android::hardware::renderscript::V1_0::Size;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::details::hidl_pointer;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

dispatchTable RsHidlAdaptation::mEntryFuncs;
sp<IDevice> RsHidlAdaptation::mHidl;
std::set<sp<IContext> > RsHidlAdaptation::mContexts;
std::mutex RsHidlAdaptation::mContextsMutex;

static uint32_t getProp(const char *str) {
    char buf[PROPERTY_VALUE_MAX];
    property_get(str, buf, "0");
    return atoi(buf);
}

RsHidlAdaptation::RsHidlAdaptation()
{
    InitializeHalDeviceContext();
}

RsHidlAdaptation& RsHidlAdaptation::GetInstance()
{
    // This function-local-static guarantees the instance is a singleton. The
    // constructor of RsHidlAdaptation will only be called when GetInstance is
    // called for the first time.
    static RsHidlAdaptation instance;
    return instance;
}

const dispatchTable* RsHidlAdaptation::GetEntryFuncs()
{
    return &mEntryFuncs;
}


void RsHidlAdaptation::LoadDispatchForHidl() {
    mEntryFuncs.Allocation1DData = Allocation1DData;
    mEntryFuncs.Allocation1DElementData = Allocation1DElementData;
    mEntryFuncs.Allocation1DRead = Allocation1DRead;
    mEntryFuncs.Allocation2DData = Allocation2DData;
    mEntryFuncs.Allocation2DRead = Allocation2DRead;
    mEntryFuncs.Allocation3DData = Allocation3DData;
    mEntryFuncs.Allocation3DRead = Allocation3DRead;
    mEntryFuncs.AllocationAdapterCreate = AllocationAdapterCreate;
    mEntryFuncs.AllocationAdapterOffset = AllocationAdapterOffset;
    mEntryFuncs.AllocationCopy2DRange = AllocationCopy2DRange;
    mEntryFuncs.AllocationCopy3DRange = AllocationCopy3DRange;
    mEntryFuncs.AllocationCopyToBitmap = AllocationCopyToBitmap;
    mEntryFuncs.AllocationCreateFromBitmap = AllocationCreateFromBitmap;
    mEntryFuncs.AllocationCreateTyped = AllocationCreateTyped;
    mEntryFuncs.AllocationCubeCreateFromBitmap = AllocationCubeCreateFromBitmap;
    mEntryFuncs.AllocationElementData = AllocationElementData;
    mEntryFuncs.AllocationElementRead = AllocationElementRead;
    mEntryFuncs.AllocationGenerateMipmaps = AllocationGenerateMipmaps;
    mEntryFuncs.AllocationGetPointer = AllocationGetPointer;
    mEntryFuncs.AllocationGetSurface = AllocationGetSurface;
    mEntryFuncs.AllocationGetType = AllocationGetType;
    mEntryFuncs.AllocationIoReceive = AllocationIoReceive;
    mEntryFuncs.AllocationIoSend = AllocationIoSend;
    mEntryFuncs.AllocationRead = AllocationRead;
    mEntryFuncs.AllocationResize1D = AllocationResize1D;
    mEntryFuncs.AllocationSetSurface = AllocationSetSurface;
    mEntryFuncs.AllocationSetupBufferQueue = AllocationSetupBufferQueue;
    mEntryFuncs.AllocationShareBufferQueue = AllocationShareBufferQueue;
    mEntryFuncs.AllocationSyncAll = AllocationSyncAll;
    mEntryFuncs.AssignName = AssignName;
    mEntryFuncs.ClosureCreate = ClosureCreate;
    mEntryFuncs.ClosureSetArg = ClosureSetArg;
    mEntryFuncs.ClosureSetGlobal = ClosureSetGlobal;
    mEntryFuncs.ContextCreate = ContextCreate;
    mEntryFuncs.ContextDeinitToClient = ContextDeinitToClient;
    mEntryFuncs.ContextDestroy = ContextDestroy;
    mEntryFuncs.ContextDump = ContextDump;
    mEntryFuncs.ContextFinish = ContextFinish;
    mEntryFuncs.ContextGetMessage = ContextGetMessage;
    mEntryFuncs.ContextInitToClient = ContextInitToClient;
    mEntryFuncs.ContextPeekMessage = ContextPeekMessage;
    mEntryFuncs.ContextSendMessage = ContextSendMessage;
    mEntryFuncs.ContextSetCacheDir = ContextSetCacheDir;
    mEntryFuncs.ContextSetPriority = ContextSetPriority;
    mEntryFuncs.ElementCreate = ElementCreate;
    mEntryFuncs.ElementCreate2 = ElementCreate2;
    mEntryFuncs.ElementGetNativeData = ElementGetNativeData;
    mEntryFuncs.ElementGetSubElements = ElementGetSubElements;
    mEntryFuncs.GetName = GetName;
    mEntryFuncs.InvokeClosureCreate = InvokeClosureCreate;
    mEntryFuncs.ObjDestroy = ObjDestroy;
    mEntryFuncs.SamplerCreate = SamplerCreate;
    mEntryFuncs.ScriptBindAllocation = ScriptBindAllocation;
    mEntryFuncs.ScriptCCreate = ScriptCCreate;
    mEntryFuncs.ScriptFieldIDCreate = ScriptFieldIDCreate;
    mEntryFuncs.ScriptForEach = ScriptForEach;
    mEntryFuncs.ScriptForEachMulti = ScriptForEachMulti;
    mEntryFuncs.ScriptGetVarV = ScriptGetVarV;
    mEntryFuncs.ScriptGroup2Create = ScriptGroup2Create;
    mEntryFuncs.ScriptGroupCreate = ScriptGroupCreate;
    mEntryFuncs.ScriptGroupExecute = ScriptGroupExecute;
    mEntryFuncs.ScriptGroupSetInput = ScriptGroupSetInput;
    mEntryFuncs.ScriptGroupSetOutput = ScriptGroupSetOutput;
    mEntryFuncs.ScriptIntrinsicCreate = ScriptIntrinsicCreate;
    mEntryFuncs.ScriptInvoke = ScriptInvoke;
    mEntryFuncs.ScriptInvokeIDCreate = ScriptInvokeIDCreate;
    mEntryFuncs.ScriptInvokeV = ScriptInvokeV;
    mEntryFuncs.ScriptKernelIDCreate = ScriptKernelIDCreate;
    mEntryFuncs.ScriptReduce = ScriptReduce;
    mEntryFuncs.ScriptSetTimeZone = ScriptSetTimeZone;
    mEntryFuncs.ScriptSetVarD = ScriptSetVarD;
    mEntryFuncs.ScriptSetVarF = ScriptSetVarF;
    mEntryFuncs.ScriptSetVarI = ScriptSetVarI;
    mEntryFuncs.ScriptSetVarJ = ScriptSetVarJ;
    mEntryFuncs.ScriptSetVarObj = ScriptSetVarObj;
    mEntryFuncs.ScriptSetVarV = ScriptSetVarV;
    mEntryFuncs.ScriptSetVarVE = ScriptSetVarVE;
    mEntryFuncs.TypeCreate = TypeCreate;
    mEntryFuncs.TypeGetNativeData = TypeGetNativeData;
}

void RsHidlAdaptation::InitializeHalDeviceContext()
{
    ALOGD("IRenderScriptDevice::getService()");
    if (getProp("debug.rs.rsov") == 0 &&
        getProp("debug.rs.default-CPU-driver") == 0) {
        // get HIDL service and register callback
        mHidl = IDevice::getService();
    }
    ALOGD("IRenderScriptDevice::getService() returned %p", mHidl.get());

    if (mHidl == NULL) {
        ALOGD("Using Fallback Path.");
        RsFallbackAdaptation& fallbackInstance = RsFallbackAdaptation::GetInstance();
        mEntryFuncs = *(fallbackInstance.GetEntryFuncs());
   } else {
        ALOGD("HIDL successfully loaded.");
        LoadDispatchForHidl();
    }
}

IContext *RsHidlAdaptation::GetIContextHandle(RsContext context) {
    return (IContext *)context;
}

RsContext RsHidlAdaptation::ContextCreate(RsDevice vdev, uint32_t version, uint32_t sdkVersion, RsContextType ct, uint32_t flags)
{
    // Get the handle to IContext interface object and save it to the map.
    sp<IContext> context = mHidl->contextCreate(sdkVersion, (ContextType)ct, flags);
    RsContext ret = (RsContext)(uintptr_t)context.get();

    std::unique_lock<std::mutex> lock(mContextsMutex);
    mContexts.insert(context);
    return ret;
}

void RsHidlAdaptation::ContextDestroy (RsContext context)
{
    // Destroy the driver context and remove IContext handle from the map.
    GetIContextHandle(context)->contextDestroy();

    std::unique_lock<std::mutex> lock(mContextsMutex);
    mContexts.erase((IContext*)context);
}

const void* RsHidlAdaptation::AllocationGetType(RsContext context, RsAllocation allocation)
{
    // TODO: Replace this idiom "(uint64_t)(uintptr_t)" with a rs_to_hal function?
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;
    uint64_t typeRet = GetIContextHandle(context)->allocationGetType(_allocation);
    return (void *)typeRet;
}


void RsHidlAdaptation::TypeGetNativeData(RsContext context, RsType type, uintptr_t *typedata, uint32_t typeDataSize)
{
    uint64_t _type = (uint64_t)(uintptr_t)type;

    GetIContextHandle(context)->typeGetNativeMetadata(_type,
                               [typedata, typeDataSize] (const hidl_vec<uint64_t> &retTypeData){
                                   for (uint32_t i=0; i<typeDataSize; i++) {
                                       typedata[i] = (uintptr_t)retTypeData[i];
                                   }
                               });
}


void RsHidlAdaptation::ElementGetNativeData(RsContext context, RsElement element, uint32_t *elemData, uint32_t elemDataSize)
{
    uint64_t _element = (uint64_t)(uintptr_t)element;

    GetIContextHandle(context)->elementGetNativeMetadata(_element,
                                [elemData, elemDataSize] (const hidl_vec<uint32_t> &retElemData){
                                    for (uint32_t i=0; i<elemDataSize; i++) {
                                        elemData[i] = retElemData[i];
                                    }
                                });
}

void RsHidlAdaptation::ElementGetSubElements(RsContext context, RsElement element, uintptr_t *ids, const char **names, size_t *arraySizes, uint32_t dataSize)
{
    uint64_t _element = (uint64_t)(uintptr_t)element;
    uint64_t _ids = (uint64_t)(uintptr_t)ids;
    uint64_t _names = (uint64_t)(uintptr_t)names;
    uint64_t _arraySizes = (uint64_t)(uintptr_t)arraySizes;

    GetIContextHandle(context)->elementGetSubElements(_element, dataSize,
                                 [ids, names, arraySizes, dataSize] (const hidl_vec<uint64_t> &retIds, const hidl_vec<hidl_string> &retNames, const hidl_vec<Size> &retArraySizes){
                                     for (uint32_t i=0; i<dataSize; i++) {
                                         ids[i] = static_cast<uintptr_t>(retIds[i]);
                                         names[i] = static_cast<const char *>(retNames[i].c_str());
                                         arraySizes[i] = static_cast<size_t>(retArraySizes[i]);
                                     }
                                 });
}


void RsHidlAdaptation::GetName(RsContext context, void * obj, const char **name) {
    uint64_t _obj = (uint64_t)(uintptr_t)obj;

    GetIContextHandle(context)->getName(_obj, [name](hidl_string ret_name) {*name = ret_name.c_str();});
}

RsClosure RsHidlAdaptation::ClosureCreate(RsContext context, RsScriptKernelID kernelID,
                                          RsAllocation returnValue,
                                          RsScriptFieldID* fieldIDs, size_t fieldIDs_length,
                                          int64_t* values, size_t values_length,
                                          int* sizes, size_t sizes_length,
                                          RsClosure* depClosures, size_t depClosures_length,
                                          RsScriptFieldID* depFieldIDs,
                                          size_t depFieldIDs_length)
{
    uint64_t _kernelID = (uint64_t)(uintptr_t)kernelID;
    uint64_t _returnValue = (uint64_t)(uintptr_t)returnValue;

    std::vector<ScriptFieldID> _fieldIDs(fieldIDs_length);
    std::vector<int64_t> _values(values_length);
    std::vector<int32_t> _sizes(sizes_length);
    std::vector<Closure> _depClosures(depClosures_length);
    std::vector<ScriptFieldID> _depFieldIDs(depFieldIDs_length);

    // TODO: Replace this idiom with a rs_to_hal function?
    for (size_t i = 0; i < fieldIDs_length; i++) {
        _fieldIDs[i] = (ScriptFieldID)(uintptr_t)fieldIDs[i];
    }
    for (size_t i = 0; i < values_length; i++) {
        _values[i] = (int64_t)values[i];
    }
    for (size_t i = 0; i < sizes_length; i++) {
        _sizes[i] = (int32_t)sizes[i];
    }
    for (size_t i = 0; i < depClosures_length; i++) {
        _depClosures[i] = (Closure)(uintptr_t)depClosures[i];
    }
    for (size_t i = 0; i < depFieldIDs_length; i++) {
        _depFieldIDs[i] = (ScriptFieldID)(uintptr_t)depFieldIDs[i];
    }

    uint64_t closure = GetIContextHandle(context)->closureCreate(_kernelID, _returnValue,
                                                            _fieldIDs, _values, _sizes,
                                                            _depClosures, _depFieldIDs);
    return (RsClosure) closure;

}

RsClosure RsHidlAdaptation::InvokeClosureCreate(RsContext context, RsScriptInvokeID invokeID,
                                                const void* params, const size_t paramLength,
                                                const RsScriptFieldID* fieldIDs, const size_t fieldIDs_length,
                                                const int64_t* values, const size_t values_length,
                                                const int* sizes, const size_t sizes_length)
{
    uint64_t _invokeID = (uint64_t)(uintptr_t)invokeID;

    hidl_vec<uint8_t> _params;
    _params.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(params)), paramLength);

    std::vector<ScriptFieldID> _fieldIDs(fieldIDs_length);
    std::vector<int64_t> _values(values_length);
    std::vector<int32_t> _sizes(sizes_length);

    for (size_t i = 0; i < fieldIDs_length; i++) {
        _fieldIDs[i] = (ScriptFieldID)(uintptr_t)fieldIDs[i];
    }
    for (size_t i = 0; i < values_length; i++) {
        _values[i] = (int64_t)values[i];
    }
    for (size_t i = 0; i < sizes_length; i++) {
        _sizes[i] = (int32_t)sizes[i];
    }

    uint64_t closure = GetIContextHandle(context)->invokeClosureCreate(_invokeID,
                                                                  _params, _fieldIDs,
                                                                  _values, _sizes);
    return (RsClosure) closure;
}

void RsHidlAdaptation::ClosureSetArg(RsContext context, RsClosure closure, uint32_t index,
                                     uintptr_t value, int size)
{
    uint64_t _closure = (uint64_t)(uintptr_t)closure;
    void * _value = (void *)value;
    GetIContextHandle(context)->closureSetArg(_closure, index, _value, size);
}
void RsHidlAdaptation::ClosureSetGlobal(RsContext context, RsClosure closure,
                                        RsScriptFieldID fieldID, int64_t value,
                                        int size)
{
    uint64_t _closure = (uint64_t)(uintptr_t)closure;
    uint64_t _fieldID = (uint64_t)(uintptr_t)fieldID;
    GetIContextHandle(context)->closureSetGlobal(_closure, _fieldID, value, size);
}

RsMessageToClientType RsHidlAdaptation::ContextGetMessage (RsContext context, void * data, size_t data_length,
                                                           size_t * receiveLen, size_t receiveLen_length,
                                                           uint32_t * subID, size_t subID_length)
{
    RsMessageToClientType msgType;
    GetIContextHandle(context)->contextGetMessage(data, data_length,
        [&msgType, receiveLen](MessageToClientType retMessageType, uint64_t retReceiveLen) {
            msgType = (RsMessageToClientType) retMessageType;
            *receiveLen = retReceiveLen;
        });
    return msgType;
}

RsMessageToClientType RsHidlAdaptation::ContextPeekMessage (RsContext context,
                                                            size_t * receiveLen, size_t receiveLen_length,
                                                            uint32_t * subID, size_t subID_length)
{
    RsMessageToClientType msgType;
    GetIContextHandle(context)->contextPeekMessage(
        [&msgType, receiveLen, subID](MessageToClientType retMessageType, uint64_t retReceiveLen, uint32_t retSubID) {
            msgType = (RsMessageToClientType) retMessageType;
            *receiveLen = retReceiveLen;
            *subID = retSubID;
        });

    return msgType;

}

void RsHidlAdaptation::ContextSendMessage (RsContext context, uint32_t id, const uint8_t *data, size_t len)
{
    hidl_vec<uint8_t> _data;
    _data.setToExternal(const_cast<uint8_t *>(data), len);
    GetIContextHandle(context)->contextSendMessage(id, _data);
}

void RsHidlAdaptation::ContextInitToClient (RsContext context)
{
    GetIContextHandle(context)->contextInitToClient();
}

void RsHidlAdaptation::ContextDeinitToClient (RsContext context)
{
    GetIContextHandle(context)->contextDeinitToClient();
}


RsType RsHidlAdaptation::TypeCreate (RsContext context, RsElement element, uint32_t dimX,
                                     uint32_t dimY, uint32_t dimZ, bool mipmaps,
                                     bool faces, uint32_t yuv)
{
    uint64_t _element = (uint64_t)(uintptr_t)element;

    uint64_t type = GetIContextHandle(context)->typeCreate(_element, dimX, dimY, dimZ, mipmaps, faces,
                                                      (YuvFormat) yuv);
    return (RsType) type;
}

RsAllocation RsHidlAdaptation::AllocationCreateTyped (RsContext context, RsType type,
                                                      RsAllocationMipmapControl mipmaps,
                                                      uint32_t usages, uintptr_t ptr)
{
    uint64_t _type = (uint64_t)(uintptr_t)type;
    void * _ptr = (void *)ptr;

    uint64_t allocation = GetIContextHandle(context)->allocationCreateTyped(_type,
        (AllocationMipmapControl)mipmaps, usages, _ptr);
    return (RsAllocation) allocation;
}


RsAllocation RsHidlAdaptation::AllocationCreateFromBitmap (RsContext context, RsType type,
                                                           RsAllocationMipmapControl mipmaps,
                                                           const void *data, size_t sizeBytes, uint32_t usages)
{
    uint64_t _type = (uint64_t)(uintptr_t)type;

    hidl_vec<uint8_t> _bitmap;
    _bitmap.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    uint64_t allocation = GetIContextHandle(context)->allocationCreateFromBitmap(_type,
        (AllocationMipmapControl)mipmaps, _bitmap, usages);
    return (RsAllocation) allocation;
}

RsAllocation RsHidlAdaptation::AllocationCubeCreateFromBitmap(RsContext context, RsType type,
                                                              RsAllocationMipmapControl mipmaps,
                                                              const void *data, size_t sizeBytes, uint32_t usages)
{
    uint64_t _type = (uint64_t)(uintptr_t)type;

    hidl_vec<uint8_t> _bitmap;
    _bitmap.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    uint64_t allocation = GetIContextHandle(context)->allocationCubeCreateFromBitmap(_type, (AllocationMipmapControl)mipmaps, _bitmap, usages);
    return (RsAllocation) allocation;
}

RsNativeWindow RsHidlAdaptation::AllocationGetSurface (RsContext context, RsAllocation allocation)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    uint64_t window = GetIContextHandle(context)->allocationGetNativeWindow(_allocation);
    return (RsNativeWindow) window;
}
void RsHidlAdaptation::AllocationSetSurface (RsContext context, RsAllocation allocation, RsNativeWindow window)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;
    uint64_t _window = (uint64_t)(uintptr_t)window;

    GetIContextHandle(context)->allocationSetNativeWindow(_allocation, _window);
}

void RsHidlAdaptation::ContextFinish (RsContext context)
{
    GetIContextHandle(context)->contextFinish();
}

void RsHidlAdaptation::ContextDump (RsContext context, int32_t bits)
{
    GetIContextHandle(context)->contextLog();
}

void RsHidlAdaptation::ContextSetPriority (RsContext context, int32_t priority)
{
    GetIContextHandle(context)->contextSetPriority((ThreadPriorities)priority);
}

void RsHidlAdaptation::ContextSetCacheDir (RsContext context, const char *cacheDir, size_t cacheDir_length)
{
    GetIContextHandle(context)->contextSetCacheDir(hidl_string(cacheDir));
}

void RsHidlAdaptation::AssignName (RsContext context, RsObjectBase obj, const char* name, size_t size)
{
    uint64_t _obj = (uint64_t)(uintptr_t)obj;

    GetIContextHandle(context)->assignName(_obj, hidl_string(name));
}

void RsHidlAdaptation::ObjDestroy (RsContext context, RsAsyncVoidPtr obj)
{
    uint64_t _obj = (uint64_t)(uintptr_t)obj;

    GetIContextHandle(context)->objDestroy(_obj);
}


RsElement RsHidlAdaptation::ElementCreate (RsContext context,
                                           RsDataType dt,
                                           RsDataKind dk,
                                           bool norm,
                                           uint32_t vecSize)
{
    uint64_t element = GetIContextHandle(context)->elementCreate((DataType) dt,
                                                            (DataKind) dk,
                                                            norm,
                                                            vecSize);
    return (RsElement) element;
}

RsElement RsHidlAdaptation::ElementCreate2 (RsContext context,
                                            const RsElement * ein,
                                            size_t ein_length,
                                            const char ** names,
                                            size_t nameLengths_length,
                                            const size_t * nameLengths,
                                            const uint32_t * arraySizes,
                                            size_t arraySizes_length)
{
    std::vector<uint64_t> _ein(ein_length);
    std::vector<hidl_string> _names(nameLengths_length);
    std::vector<Size> _arraySizes(arraySizes_length);

    for (size_t i = 0; i < ein_length; i++) {
        _ein[i] = (uint64_t)((uintptr_t)ein[i]);
    }
    for (size_t i = 0; i < ein_length; i++) {
        _names[i] = hidl_string(names[i]);
    }
    for (size_t i = 0; i < arraySizes_length; i++) {
        _arraySizes[i] = (Size)arraySizes[i];
    }

    uint64_t element = GetIContextHandle(context)->elementComplexCreate(_ein, _names, _arraySizes);
    return (RsElement) element;
}

void RsHidlAdaptation::AllocationCopyToBitmap (RsContext context, RsAllocation allocation, void *data, size_t sizeBytes)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationCopyToBitmap(_allocation, data, sizeBytes);
}

void RsHidlAdaptation::Allocation1DData (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t lod,
                                         uint32_t count, const void *data, size_t sizeBytes)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    GetIContextHandle(context)->allocation1DWrite(_allocation, xoff, lod, count, _data);
}
void RsHidlAdaptation::Allocation1DElementData (RsContext context, RsAllocation allocation, uint32_t xoff,
                                                uint32_t lod, const void *data, size_t sizeBytes, size_t eoff)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    GetIContextHandle(context)->allocationElementWrite(_allocation, xoff, 0, 0, lod, _data, eoff);
}

void RsHidlAdaptation::AllocationElementData (RsContext context, RsAllocation allocation, uint32_t x, uint32_t y, uint32_t z,
                                              uint32_t lod, const void *data, size_t sizeBytes, size_t eoff)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    GetIContextHandle(context)->allocationElementWrite(_allocation, x, y, z, lod, _data, eoff);
}

void RsHidlAdaptation::Allocation2DData (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t yoff,
                                         uint32_t lod, RsAllocationCubemapFace face,
                                         uint32_t w, uint32_t h, const void *data, size_t sizeBytes, size_t stride)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    GetIContextHandle(context)->allocation2DWrite(_allocation, xoff, yoff, lod, (AllocationCubemapFace)face, w, h, _data, stride);
}

void RsHidlAdaptation::Allocation3DData (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                                         uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes, size_t stride)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), sizeBytes);

    GetIContextHandle(context)->allocation3DWrite(_allocation, xoff, yoff, zoff, lod, w, h, d, _data, stride);
}

void RsHidlAdaptation::AllocationGenerateMipmaps (RsContext context, RsAllocation allocation)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationGenerateMipmaps(_allocation);
}

void RsHidlAdaptation::AllocationRead (RsContext context, RsAllocation allocation, void *data, size_t sizeBytes)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationRead(_allocation, data, sizeBytes);
}

void RsHidlAdaptation::Allocation1DRead (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t lod,
                                         uint32_t count, void *data, size_t sizeBytes)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocation1DRead(_allocation, xoff, lod, count, data, sizeBytes);
}

void RsHidlAdaptation::AllocationElementRead (RsContext context, RsAllocation allocation, uint32_t x, uint32_t y, uint32_t z,
                                              uint32_t lod, void *data, size_t sizeBytes, size_t eoff)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationElementRead(_allocation, x, y, z, lod, data, sizeBytes, eoff);
}

void RsHidlAdaptation::Allocation2DRead (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t yoff,
                                         uint32_t lod, RsAllocationCubemapFace face,
                                         uint32_t w, uint32_t h, void *data, size_t sizeBytes, size_t stride)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocation2DRead(_allocation, xoff, yoff, lod, (AllocationCubemapFace)face, w, h, data, sizeBytes, stride);
}

void RsHidlAdaptation::Allocation3DRead (RsContext context, RsAllocation allocation, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                                         uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes, size_t stride)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocation3DRead(_allocation, xoff, yoff, zoff, lod, w, h, d, data, sizeBytes, stride);
}

void RsHidlAdaptation::AllocationSyncAll (RsContext context, RsAllocation allocation, RsAllocationUsageType usage)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationSyncAll(_allocation,
        (AllocationUsageType) usage);
}

void RsHidlAdaptation::AllocationResize1D (RsContext context, RsAllocation allocation, uint32_t dimX)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationResize1D(_allocation, dimX);
}

void RsHidlAdaptation::AllocationCopy2DRange (RsContext context,
                                              RsAllocation dstAlloc,
                                              uint32_t dstXoff, uint32_t dstYoff,
                                              uint32_t dstMip, uint32_t dstFace,
                                              uint32_t width, uint32_t height,
                                              RsAllocation srcAlloc,
                                              uint32_t srcXoff, uint32_t srcYoff,
                                              uint32_t srcMip, uint32_t srcFace)
{
    uint64_t _dstAlloc = (uint64_t)(uintptr_t)dstAlloc;
    uint64_t _srcAlloc = (uint64_t)(uintptr_t)srcAlloc;

    GetIContextHandle(context)->allocationCopy2DRange(_dstAlloc, dstXoff, dstYoff, dstMip, (AllocationCubemapFace)dstFace, width, height,
                                 _srcAlloc, srcXoff, srcYoff, srcMip, (AllocationCubemapFace)srcFace);
}
void RsHidlAdaptation::AllocationCopy3DRange (RsContext context,
                                              RsAllocation dstAlloc,
                                              uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                                              uint32_t dstMip,
                                              uint32_t width, uint32_t height, uint32_t depth,
                                              RsAllocation srcAlloc,
                                              uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                                              uint32_t srcMip)
{
    uint64_t _dstAlloc = (uint64_t)(uintptr_t)dstAlloc;
    uint64_t _srcAlloc = (uint64_t)(uintptr_t)srcAlloc;

    GetIContextHandle(context)->allocationCopy3DRange(_dstAlloc, dstXoff, dstYoff, dstZoff, dstMip, width, height, depth,
                                                 _srcAlloc, srcXoff, srcYoff, srcZoff, srcMip);
}

RsSampler RsHidlAdaptation::SamplerCreate (RsContext context,
                                           RsSamplerValue magFilter,
                                           RsSamplerValue minFilter,
                                           RsSamplerValue wrapS,
                                           RsSamplerValue wrapT,
                                           RsSamplerValue wrapR,
                                           float aniso)
{
    uint64_t sampler = GetIContextHandle(context)->samplerCreate((SamplerValue) magFilter,
                                                            (SamplerValue)minFilter,
                                                            (SamplerValue)wrapS,
                                                            (SamplerValue)wrapT,
                                                            (SamplerValue)wrapR,
                                                            aniso);
    return (RsSampler) sampler;
}

void RsHidlAdaptation::ScriptBindAllocation (RsContext context, RsScript script, RsAllocation allocation, uint32_t slot)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->scriptBindAllocation(_script, _allocation, slot);
}

void RsHidlAdaptation::ScriptSetTimeZone (RsContext context, RsScript script, const char* timezone, size_t size)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptSetTimeZone(_script, hidl_string(timezone));
}

void RsHidlAdaptation::ScriptInvoke (RsContext context, RsScript script, uint32_t slot)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptInvoke(_script, slot);
}

void RsHidlAdaptation::ScriptInvokeV (RsContext context, RsScript script, uint32_t slot, const void *data, size_t len)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;
    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), len);

    GetIContextHandle(context)->scriptInvokeV(_script, slot, _data);
}

void RsHidlAdaptation::ScriptForEach (RsContext context, RsScript script, uint32_t slot,
                                      RsAllocation vain, RsAllocation vaout,
                                      const void *params, size_t paramLen,
                                      const RsScriptCall *sc, size_t scLen)
{
    RsAllocation * vains = nullptr;
    size_t inLen = 0;
    if (vain) {
        vains = &vain;
        inLen = 1;
    }
    ScriptForEachMulti(context, script, slot, vains, inLen, vaout, params, paramLen, sc, scLen);
}

void RsHidlAdaptation::ScriptForEachMulti (RsContext context, RsScript script, uint32_t slot,
                                           RsAllocation *vains, size_t inLen,
                                           RsAllocation vaout, const void *params,
                                           size_t paramLen, const RsScriptCall *sc,
                                           size_t scLen)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    std::vector<Allocation> _vains(inLen);
    for (size_t i = 0; i < inLen; i++) {
        _vains[i] = (Allocation)(uintptr_t)vains[i];
    }

    uint64_t _vaout = (uint64_t)(uintptr_t)vaout;

    hidl_vec<uint8_t> _params;
    _params.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(params)), paramLen);

    ScriptCall * _scPtr = nullptr;
    ScriptCall _sc;
    if (sc) {
        _sc.strategy    = static_cast<ForEachStrategy>(sc->strategy);
        _sc.xStart      = sc->xStart;
        _sc.xEnd        = sc->xEnd;
        _sc.yStart      = sc->yStart;
        _sc.yEnd        = sc->yEnd;
        _sc.zStart      = sc->zStart;
        _sc.zEnd        = sc->zEnd;
        _sc.arrayStart  = sc->arrayStart;
        _sc.arrayEnd    = sc->arrayEnd;
        _sc.array2Start = sc->array2Start;
        _sc.array2End   = sc->arrayEnd;
        _sc.array3Start = sc->array3Start;
        _sc.array3End   = sc->arrayEnd;
        _sc.array4Start = sc->array4Start;
        _sc.array4End   = sc->arrayEnd;

        _scPtr = &_sc;
    }
    GetIContextHandle(context)->scriptForEach(_script, slot, _vains, _vaout, _params, _scPtr);
}

void RsHidlAdaptation::ScriptReduce (RsContext context, RsScript script, uint32_t slot,
                                     RsAllocation *vains, size_t inLen, RsAllocation vaout,
                                     const RsScriptCall *sc, size_t scLen)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    std::vector<Allocation> _vains(inLen);
    for (size_t i = 0; i < inLen; i++) {
        _vains[i] = (Allocation)(uintptr_t)vains[i];
    }

    uint64_t _vaout = (uint64_t)(uintptr_t)vaout;

    ScriptCall * _scPtr = nullptr;
    ScriptCall _sc;
    if (sc) {
        _sc.strategy    = static_cast<ForEachStrategy>(sc->strategy);
        _sc.xStart      = sc->xStart;
        _sc.xEnd        = sc->xEnd;
        _sc.yStart      = sc->yStart;
        _sc.yEnd        = sc->yEnd;
        _sc.zStart      = sc->zStart;
        _sc.zEnd        = sc->zEnd;
        _sc.arrayStart  = sc->arrayStart;
        _sc.arrayEnd    = sc->arrayEnd;
        _sc.array2Start = sc->array2Start;
        _sc.array2End   = sc->arrayEnd;
        _sc.array3Start = sc->array3Start;
        _sc.array3End   = sc->arrayEnd;
        _sc.array4Start = sc->array4Start;
        _sc.array4End   = sc->arrayEnd;

        _scPtr = &_sc;
    }
    GetIContextHandle(context)->scriptReduce(_script, slot, _vains, _vaout, _scPtr);
}

void RsHidlAdaptation::ScriptSetVarI (RsContext context, RsScript script, uint32_t slot, int value)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptSetVarI(_script, slot, value);
}

void RsHidlAdaptation::ScriptSetVarObj (RsContext context, RsScript script, uint32_t slot, RsObjectBase obj)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;
    uint64_t _obj = (uint64_t)(uintptr_t)obj;

    GetIContextHandle(context)->scriptSetVarObj(_script, slot, _obj);
}

void RsHidlAdaptation::ScriptSetVarJ (RsContext context, RsScript script, uint32_t slot, int64_t value)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptSetVarJ(_script, slot, value);
}

void RsHidlAdaptation::ScriptSetVarF (RsContext context, RsScript script, uint32_t slot, float value)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptSetVarF(_script, slot, value);
}

void RsHidlAdaptation::ScriptSetVarD (RsContext context, RsScript script, uint32_t slot, double value)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptSetVarD(_script, slot, value);
}

void RsHidlAdaptation::ScriptSetVarV (RsContext context, RsScript script, uint32_t slot, const void* data, size_t len)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;
    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), len);

    GetIContextHandle(context)->scriptSetVarV(_script, slot, _data);
}

void RsHidlAdaptation::ScriptGetVarV (RsContext context, RsScript script, uint32_t slot, void* data, size_t len)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    GetIContextHandle(context)->scriptGetVarV(_script, slot, len,
                         [data, len] (const hidl_vec<uint8_t> &retData) {
                             memcpy(data, retData.data(), len);
                         });
}

void RsHidlAdaptation::ScriptSetVarVE (RsContext context, RsScript script, uint32_t slot,
                                       const void *data, size_t len, RsElement ve,
                                       const uint32_t *dims, size_t dimLen)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    hidl_vec<uint8_t> _data;
    _data.setToExternal(reinterpret_cast<uint8_t *>(const_cast<void *>(data)), len);

    uint64_t _ve = (uint64_t)(uintptr_t)ve;

    hidl_vec<uint32_t> _dims;
    _dims.setToExternal(const_cast<uint32_t *>(dims), dimLen / sizeof(uint32_t));

    GetIContextHandle(context)->scriptSetVarVE(_script, slot, _data, _ve, _dims);
}

RsScript RsHidlAdaptation::ScriptCCreate (RsContext context,
                                          const char *resName, size_t resName_length,
                                          const char *cacheDir, size_t cacheDir_length,
                                          const char *text, size_t text_length)
{
    hidl_vec<uint8_t> _text;
    _text.setToExternal(reinterpret_cast<uint8_t *>(const_cast<char *>(text)), text_length);
    uint64_t scriptc = GetIContextHandle(context)->scriptCCreate(hidl_string(resName), hidl_string(cacheDir), _text);
    return (RsScript) scriptc;
}

RsScript RsHidlAdaptation::ScriptIntrinsicCreate (RsContext context, uint32_t id, RsElement element)
{
    uint64_t _element = (uint64_t)(uintptr_t)element;

    uint64_t intrinsic = GetIContextHandle(context)->scriptIntrinsicCreate((ScriptIntrinsicID)id, _element);
    return (RsScript) intrinsic;
}

RsScriptKernelID RsHidlAdaptation::ScriptKernelIDCreate (RsContext context, RsScript script, int slot, int sig)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    uint64_t kernelID = GetIContextHandle(context)->scriptKernelIDCreate(_script, slot, sig);
    return (RsScriptKernelID) kernelID;
}

RsScriptInvokeID RsHidlAdaptation::ScriptInvokeIDCreate (RsContext context, RsScript script, int slot)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    uint64_t invokeID = GetIContextHandle(context)->scriptInvokeIDCreate(_script, slot);
    return (RsScriptInvokeID) invokeID;
}

RsScriptFieldID RsHidlAdaptation::ScriptFieldIDCreate (RsContext context, RsScript script, int slot)
{
    uint64_t _script = (uint64_t)(uintptr_t)script;

    uint64_t fieldID = GetIContextHandle(context)->scriptFieldIDCreate(_script, slot);
    return (RsScriptFieldID) fieldID;
}

RsScriptGroup RsHidlAdaptation::ScriptGroupCreate (RsContext context, RsScriptKernelID * kernels, size_t kernelsSize,
                                                   RsScriptKernelID * src, size_t srcSize,
                                                   RsScriptKernelID * dstK, size_t dstKSize,
                                                   RsScriptFieldID * dstF, size_t dstFSize,
                                                   const RsType * type, size_t typeSize)
{
    std::vector<ScriptKernelID> _kernels(kernelsSize / sizeof(RsScriptKernelID));
    std::vector<ScriptKernelID> _src(srcSize / sizeof(RsScriptKernelID));
    std::vector<ScriptKernelID> _dstK(dstKSize / sizeof(RsScriptKernelID));
    std::vector<ScriptFieldID> _dstF(dstFSize / sizeof(RsScriptFieldID));
    std::vector<Type> _type(typeSize / sizeof(RsType));

    for (size_t i = 0; i < _kernels.size(); i++) {
        _kernels[i] = (ScriptKernelID)(uintptr_t)kernels[i];
    }
    for (size_t i = 0; i < _src.size(); i++) {
        _src[i] = (ScriptKernelID)(uintptr_t)src[i];
    }
    for (size_t i = 0; i < _dstK.size(); i++) {
        _dstK[i] = (ScriptKernelID)(uintptr_t)dstK[i];
    }
    for (size_t i = 0; i < _dstF.size(); i++) {
        _dstF[i] = (ScriptFieldID)(uintptr_t)dstF[i];
    }
    for (size_t i = 0; i < _type.size(); i++) {
        _type[i] = (Type)(uintptr_t)type[i];
    }

    uint64_t scriptGroup = GetIContextHandle(context)->scriptGroupCreate(_kernels, _src, _dstK, _dstF, _type);
    return (RsScriptGroup) scriptGroup;
}

RsScriptGroup2 RsHidlAdaptation::ScriptGroup2Create(RsContext context, const char* name, size_t nameLength,
                                                    const char* cacheDir, size_t cacheDirLength,
                                                    RsClosure* closures, size_t numClosures)
{
    std::vector<Closure> _closures(numClosures);
    for (size_t i = 0; i < numClosures; i++) {
        _closures[i] = (Closure)(uintptr_t)closures[i];
    }

    uint64_t scriptGroup2 = GetIContextHandle(context)->scriptGroup2Create(hidl_string(name), hidl_string(cacheDir), _closures);
    return (RsScriptGroup2) scriptGroup2;
}

void RsHidlAdaptation::ScriptGroupSetOutput (RsContext context, RsScriptGroup sg, RsScriptKernelID kid, RsAllocation alloc)
{
    uint64_t _sg = (uint64_t)(uintptr_t)sg;
    uint64_t _kid = (uint64_t)(uintptr_t)kid;
    uint64_t _alloc = (uint64_t)(uintptr_t)alloc;

    GetIContextHandle(context)->scriptGroupSetOutput(_sg, _kid, _alloc);
}

void RsHidlAdaptation::ScriptGroupSetInput (RsContext context, RsScriptGroup sg, RsScriptKernelID kid, RsAllocation alloc)
{
    uint64_t _sg = (uint64_t)(uintptr_t)sg;
    uint64_t _kid = (uint64_t)(uintptr_t)kid;
    uint64_t _alloc = (uint64_t)(uintptr_t)alloc;

    GetIContextHandle(context)->scriptGroupSetInput(_sg, _kid, _alloc);
}

void RsHidlAdaptation::ScriptGroupExecute (RsContext context, RsScriptGroup sg)
{
    uint64_t _sg = (uint64_t)(uintptr_t)sg;

    GetIContextHandle(context)->scriptGroupExecute(_sg);
}

void RsHidlAdaptation::AllocationIoSend (RsContext context, RsAllocation allocation)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationIoSend(_allocation);
}
int64_t RsHidlAdaptation::AllocationIoReceive (RsContext context, RsAllocation allocation)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationIoReceive(_allocation);
    // Fix me.
    return 0;
}

void * RsHidlAdaptation::AllocationGetPointer (RsContext context, RsAllocation allocation,
                                               uint32_t lod, RsAllocationCubemapFace face,
                                               uint32_t z, uint32_t array, size_t *stride, size_t strideLen)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    void* ptr;
    GetIContextHandle(context)->allocationGetPointer(_allocation, lod,
                                                (AllocationCubemapFace)face, z,
                                                [&ptr, stride] (void* retPtr, uint64_t retStride) {
                                                    ptr = retPtr;
                                                    if (retStride > 0) {
                                                        *stride = retStride;
                                                    }
                                                });
    return ptr;
}

void RsHidlAdaptation::AllocationSetupBufferQueue (RsContext context, RsAllocation allocation, uint32_t numAlloc)
{
    uint64_t _allocation = (uint64_t)(uintptr_t)allocation;

    GetIContextHandle(context)->allocationSetupBufferQueue(_allocation, numAlloc);
}

void RsHidlAdaptation::AllocationShareBufferQueue(RsContext context, RsAllocation valloc1, RsAllocation valloc2)
{
    uint64_t _valloc1 = (uint64_t)(uintptr_t)valloc1;
    uint64_t _valloc2 = (uint64_t)(uintptr_t)valloc2;

    GetIContextHandle(context)->allocationShareBufferQueue(_valloc1, _valloc2);
}

RsAllocation RsHidlAdaptation::AllocationAdapterCreate (RsContext context, RsType vtype, RsAllocation baseAlloc)
{
    uint64_t _vtype = (uint64_t)(uintptr_t)vtype;
    uint64_t _baseAlloc = (uint64_t)(uintptr_t)baseAlloc;

    uint64_t allocationAdapter = GetIContextHandle(context)->allocationAdapterCreate(_vtype, _baseAlloc);
    return (RsAllocation) allocationAdapter;
}

void RsHidlAdaptation::AllocationAdapterOffset (RsContext context, RsAllocation alloc, const uint32_t * offsets, size_t offsets_length)
{
    uint64_t _alloc = (uint64_t)(uintptr_t)alloc;

    hidl_vec<uint32_t> _offsets;
    _offsets.setToExternal(const_cast<uint32_t *>(offsets), offsets_length / sizeof(uint32_t));

    GetIContextHandle(context)->allocationAdapterOffset(_alloc, _offsets);
}
