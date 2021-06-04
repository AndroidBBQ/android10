#ifndef ANDROID_RENDERSCRIPT_CLOSURE_H_
#define ANDROID_RENDERSCRIPT_CLOSURE_H_

#include "rsDefines.h"
#include "rsMap.h"
#include "rsObjectBase.h"

namespace android {
namespace renderscript {

class Allocation;
class Context;
class IDBase;
class ObjectBase;
class ScriptFieldID;
class ScriptInvokeID;
class ScriptKernelID;
class Type;

class Closure : public ObjectBase {
 public:
    Closure(Context* context,
            const ScriptKernelID* kernelID,
            Allocation* returnValue,
            const int numValues,
            const ScriptFieldID** fieldIDs,
            const int64_t* values,  // Allocations or primitive (numeric) types
            const int* sizes,  // size for data type. -1 indicates an allocation.
            const Closure** depClosures,
            const ScriptFieldID** depFieldIDs);
    Closure(Context* context,
            const ScriptInvokeID* invokeID,
            const void* params,
            const size_t paramLength,
            const size_t numValues,
            const ScriptFieldID** fieldIDs,
            const int64_t* values,  // Allocations or primitive (numeric) types
            const int* sizes);  // size for data type. -1 indicates an allocation.

    virtual ~Closure();

    virtual void serialize(Context *rsc, OStream *stream) const {}

    virtual RsA3DClassID getClassId() const { return RS_A3D_CLASS_ID_CLOSURE; }

    // Set the value of an argument or a global.
    // The special value -1 for the size indicates the value is an Allocation.
    void setArg(const uint32_t index, const void* value, const int size);
    void setGlobal(const ScriptFieldID* fieldID, const int64_t value,
                   const int size);

    Context* mContext;

    // KernelId or InvokeID
    const ObjectBaseRef<IDBase> mFunctionID;
    // Flag indicating if this closure is for a kernel (true) or invocable
    // function (false)
    const bool mIsKernel;

    // Values referrenced in arguments and globals cannot be futures. They must be
    // either a known value or unbound value.
    // For now, all arguments should be Allocations.
    const void** mArgs;
    size_t mNumArg;

    // A global could be allocation or any primitive data type.
    Map<const ScriptFieldID*, Pair<int64_t, int>> mGlobals;

    Allocation* mReturnValue;

    // All the other closures which this closure depends on for one of its
    // arguments, and the fields which it depends on.
    Map<const Closure*, Map<int, ObjectBaseRef<ScriptFieldID>>*> mArgDeps;

    // All the other closures that this closure depends on for one of its fields,
    // and the fields that it depends on.
    Map<const Closure*, Map<const ScriptFieldID*,
            ObjectBaseRef<ScriptFieldID>>*> mGlobalDeps;

    uint8_t* mParams;
    const size_t mParamLength;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_CLOSURE_H_
