#include "rsClosure.h"

#include "cpu_ref/rsCpuCore.h"
#include "rsContext.h" // XXX: necessary to avoid compiler error on rsScript.h below
#include "rsScript.h"
#include "rsType.h"

namespace android {
namespace renderscript {

RsClosure rsi_ClosureCreate(Context* context, RsScriptKernelID kernelID,
                            RsAllocation returnValue,
                            RsScriptFieldID* fieldIDs, size_t fieldIDs_length,
                            const int64_t* values, size_t values_length,
                            const int* sizes, size_t sizes_length,
                            RsClosure* depClosures, size_t depClosures_length,
                            RsScriptFieldID* depFieldIDs,
                            size_t depFieldIDs_length) {
    rsAssert(fieldIDs_length == values_length && values_length == sizes_length &&
             sizes_length == depClosures_length &&
             depClosures_length == depFieldIDs_length);

    Closure* c = new Closure(
        context, (const ScriptKernelID*)kernelID, (Allocation*)returnValue,
        fieldIDs_length, (const ScriptFieldID**)fieldIDs, values,
        sizes, (const Closure**)depClosures,
        (const ScriptFieldID**)depFieldIDs);
    c->incUserRef();
    return static_cast<RsClosure>(c);
}

RsClosure rsi_InvokeClosureCreate(Context* context, RsScriptInvokeID invokeID,
                                  const void* params, const size_t paramLength,
                                  const RsScriptFieldID* fieldIDs, const size_t fieldIDs_length,
                                  const int64_t* values, const size_t values_length,
                                  const int* sizes, const size_t sizes_length) {
    rsAssert(fieldIDs_length == values_length && values_length == sizes_length);
    Closure* c = new Closure(
        context, (const ScriptInvokeID*)invokeID, params, paramLength,
        fieldIDs_length, (const ScriptFieldID**)fieldIDs, values,
        sizes);
    c->incUserRef();
    return static_cast<RsClosure>(c);
}

void rsi_ClosureSetArg(Context* rsc, RsClosure closure, uint32_t index,
                       uintptr_t value, int size) {
    ((Closure*)closure)->setArg(index, (const void*)value, size);
}

void rsi_ClosureSetGlobal(Context* rsc, RsClosure closure,
                          RsScriptFieldID fieldID, int64_t value,
                          int size) {
    ((Closure*)closure)->setGlobal((const ScriptFieldID*)fieldID,
                                   value, size);
}

Closure::Closure(Context* context,
                 const ScriptKernelID* kernelID,
                 Allocation* returnValue,
                 const int numValues,
                 const ScriptFieldID** fieldIDs,
                 const int64_t* values,
                 const int* sizes,
                 const Closure** depClosures,
                 const ScriptFieldID** depFieldIDs) :
    ObjectBase(context), mContext(context), mFunctionID((IDBase*)kernelID),
    mIsKernel(true), mReturnValue(returnValue), mParams(nullptr),
    mParamLength(0) {
    size_t i;

    for (i = 0; i < (size_t)numValues && fieldIDs[i] == nullptr; i++);

    mNumArg = i;
    mArgs = new const void*[mNumArg];
    for (size_t j = 0; j < mNumArg; j++) {
        mArgs[j] = (const void*)values[j];
    }

    for (; i < (size_t)numValues; i++) {
        rsAssert(fieldIDs[i] != nullptr);
        mGlobals[fieldIDs[i]] = make_pair(values[i], sizes[i]);
    }

    for (i = 0; i < mNumArg; i++) {
        const Closure* dep = depClosures[i];
        if (dep != nullptr) {
            auto mapping = mArgDeps[dep];
            if (mapping == nullptr) {
                mapping = new Map<int, ObjectBaseRef<ScriptFieldID>>();
                mArgDeps[dep] = mapping;
            }
            (*mapping)[i].set(const_cast<ScriptFieldID*>(depFieldIDs[i]));
        }
    }

    for (; i < (size_t)numValues; i++) {
        const Closure* dep = depClosures[i];
        if (dep != nullptr) {
            auto mapping = mGlobalDeps[dep];
            if (mapping == nullptr) {
                mapping = new Map<const ScriptFieldID*,
                                  ObjectBaseRef<ScriptFieldID>>();
                mGlobalDeps[dep] = mapping;
            }
            fieldIDs[i]->incSysRef();
            (*mapping)[fieldIDs[i]].set(const_cast<ScriptFieldID*>(depFieldIDs[i]));
        }
    }
}

Closure::Closure(Context* context, const ScriptInvokeID* invokeID,
                 const void* params, const size_t paramLength,
                 const size_t numValues, const ScriptFieldID** fieldIDs,
                 const int64_t* values, const int* sizes) :
    ObjectBase(context), mContext(context), mFunctionID((IDBase*)invokeID), mIsKernel(false),
    mArgs(nullptr), mNumArg(0),
    mReturnValue(nullptr), mParamLength(paramLength) {
    mParams = new uint8_t[mParamLength];
    memcpy(mParams, params, mParamLength);
    for (size_t i = 0; i < numValues; i++) {
        mGlobals[fieldIDs[i]] = make_pair(values[i], sizes[i]);
    }
}

Closure::~Closure() {
    for (const auto& p : mArgDeps) {
        delete p.second;
    }

    for (const auto& p : mGlobalDeps) {
        auto map = p.second;
        for (const auto& p1 : *map) {
            p1.first->decSysRef();
        }
        delete p.second;
    }

    delete[] mArgs;
    delete[] mParams;
}

void Closure::setArg(const uint32_t index, const void* value, const int size) {
    mArgs[index] = value;
}

void Closure::setGlobal(const ScriptFieldID* fieldID, const int64_t value,
                        const int size) {
    mGlobals[fieldID] = make_pair(value, size);
}

}  // namespace renderscript
}  // namespace android
