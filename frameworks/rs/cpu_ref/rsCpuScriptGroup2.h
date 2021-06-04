#ifndef CPU_REF_CPUSCRIPTGROUP2IMPL_H_
#define CPU_REF_CPUSCRIPTGROUP2IMPL_H_

#include "rsd_cpu.h"
#include "rsList.h"

struct RsExpandKernelDriverInfo;

namespace android {
namespace renderscript {

class Closure;
class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;
class ScriptExecutable;
class ScriptGroup2;

typedef void (*ExpandFuncTy)(const RsExpandKernelDriverInfo*, uint32_t, uint32_t,
                             uint32_t);
typedef void (*InvokeFuncTy)(const void*, uint32_t);

class CPUClosure {
public:
    CPUClosure(const Closure* closure, RsdCpuScriptImpl* si, ExpandFuncTy func) :
        mClosure(closure), mSi(si), mFunc(func) {}

    CPUClosure(const Closure* closure, RsdCpuScriptImpl* si) :
        mClosure(closure), mSi(si), mFunc(nullptr) {}

    // It's important to do forwarding here than inheritance for unbound value
    // binding to work.
    const Closure* mClosure;
    RsdCpuScriptImpl* mSi;
    const ExpandFuncTy mFunc;
};

class CpuScriptGroup2Impl;

class Batch {
public:
    Batch(CpuScriptGroup2Impl* group, const char* name);
    ~Batch();

    // Returns true if closure depends on any closure in this batch for a global
    // variable
    bool conflict(CPUClosure* closure) const;

    void resolveFuncPtr(void* sharedObj);
    void setGlobalsForBatch();
    void run();

    size_t size() const { return mClosures.size(); }

    CpuScriptGroup2Impl* mGroup;
    List<CPUClosure*> mClosures;
    char* mName;
    void* mFunc;
};

class CpuScriptGroup2Impl : public RsdCpuReference::CpuScriptGroup2 {
public:
    CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl, const ScriptGroupBase* group);
    ~CpuScriptGroup2Impl() override;

    bool init();
    void execute() override;

    RsdCpuReferenceImpl* getCpuRefImpl() const { return mCpuRefImpl; }
    ScriptExecutable* getExecutable() const { return mExecutable; }

    void compile(const char* cacheDir);

private:
    RsdCpuReferenceImpl* mCpuRefImpl;
    const ScriptGroup2* mGroup;
    List<Batch*> mBatches;
    ScriptExecutable* mExecutable;
    void* mScriptObj;
};

}  // namespace renderscript
}  // namespace android

#endif  // CPU_REF_CPUSCRIPTGROUP2IMPL_H_
