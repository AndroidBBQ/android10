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

#ifndef RSD_CPU_CORE_H
#define RSD_CPU_CORE_H

#include "rsd_cpu.h"
#include "rsSignal.h"
#include "rsContext.h"
#include "rsCppUtils.h"
#include "rsElement.h"
#include "rsScriptC.h"
#include "rsCpuCoreRuntime.h"

#include <string>

namespace android {
namespace renderscript {

// Whether the CPU we're running on supports SIMD instructions
extern bool gArchUseSIMD;

// Function types found in RenderScript code
typedef void (*ReduceAccumulatorFunc_t)(const RsExpandKernelDriverInfo *info, uint32_t x1, uint32_t x2, uint8_t *accum);
typedef void (*ReduceCombinerFunc_t)(uint8_t *accum, const uint8_t *other);
typedef void (*ReduceInitializerFunc_t)(uint8_t *accum);
typedef void (*ReduceOutConverterFunc_t)(uint8_t *out, const uint8_t *accum);
typedef void (*ForEachFunc_t)(const RsExpandKernelDriverInfo *info, uint32_t x1, uint32_t x2, uint32_t outStride);
typedef void (*InvokeFunc_t)(void *params);
typedef void (*InitOrDtorFunc_t)(void);
typedef int  (*RootFunc_t)(void);

struct ReduceDescription {
    ReduceAccumulatorFunc_t  accumFunc;  // expanded accumulator function
    ReduceInitializerFunc_t  initFunc;   // user initializer function
    ReduceCombinerFunc_t     combFunc;   // user combiner function
    ReduceOutConverterFunc_t outFunc;    // user outconverter function
    size_t                   accumSize;  // accumulator datum size, in bytes
};

// Internal driver callback used to execute a kernel
typedef void (*WorkerCallback_t)(void *usr, uint32_t idx);

class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;

struct ScriptTLSStruct {
    android::renderscript::Context * mContext;
    const android::renderscript::Script * mScript;
    RsdCpuScriptImpl *mImpl;
};

// MTLaunchStruct passes information about a multithreaded kernel launch.
struct MTLaunchStructCommon {
    RsdCpuReferenceImpl *rs;
    RsdCpuScriptImpl *script;

    uint32_t mSliceSize;
    volatile int mSliceNum;
    bool isThreadable;

    // Boundary information about the launch
    RsLaunchDimensions start;
    RsLaunchDimensions end;
    // Points to MTLaunchStructForEach::fep::dim or
    // MTLaunchStructReduce::redp::dim.
    RsLaunchDimensions *dimPtr;
};

struct MTLaunchStructForEach : public MTLaunchStructCommon {
    // Driver info structure
    RsExpandKernelDriverInfo fep;

    ForEachFunc_t kernel;
    const Allocation *ains[RS_KERNEL_INPUT_LIMIT];
    Allocation *aout[RS_KERNEL_INPUT_LIMIT];
};

struct MTLaunchStructReduce : public MTLaunchStructCommon {
    // Driver info structure
    RsExpandKernelDriverInfo redp;

    const Allocation *ains[RS_KERNEL_INPUT_LIMIT];

    ReduceAccumulatorFunc_t accumFunc;
    ReduceInitializerFunc_t initFunc;
    ReduceCombinerFunc_t combFunc;
    ReduceOutConverterFunc_t outFunc;

    size_t accumSize;  // accumulator datum size in bytes

    size_t accumStride;  // stride between accumulators in accumAlloc (below)

    // These fields are used for managing accumulator data items in a
    // multithreaded execution.
    //
    // Let the number of threads be N.
    // Let Outc be true iff there is an outconverter.
    //
    // accumAlloc is a pointer to a single allocation of (N - !Outc)
    // accumulators.  (If there is no outconverter, then the output
    // allocation acts as an accumulator.)  It is created at kernel
    // launch time.  Within that allocation, the distance between the
    // start of adjacent accumulators is accumStride bytes -- this
    // might be the same as accumSize, or it might be larger, if we
    // are attempting to avoid false sharing.
    //
    // accumCount is an atomic counter of how many accumulators have
    // been grabbed by threads.  It is initialized to zero at kernel
    // launch time.  See accumPtr for further description.
    //
    // accumPtr is pointer to an array of N pointers to accumulators.
    // The array is created at kernel launch time, and each element is
    // initialized to nullptr.  When a particular thread goes to work,
    // that thread obtains its accumulator from its entry in this
    // array.  If the entry is nullptr, that thread needs to obtain an
    // accumulator, and initialize its entry in the array accordingly.
    // It does so via atomic access (fetch-and-add) to accumCount.
    // - If Outc, then the fetched value is used as an index into
    //   accumAlloc.
    // - If !Outc, then
    //   - If the fetched value is zero, then this thread gets the
    //     output allocation for its accumulator.
    //   - If the fetched value is nonzero, then (fetched value - 1)
    //     is used as an index into accumAlloc.
    uint8_t *accumAlloc;
    uint8_t **accumPtr;
    uint32_t accumCount;

    // Logging control
    uint32_t logReduce;
};

class RsdCpuReferenceImpl : public RsdCpuReference {
public:
    ~RsdCpuReferenceImpl() override;
    explicit RsdCpuReferenceImpl(Context *);

    void lockMutex();
    void unlockMutex();

    bool init(uint32_t version_major, uint32_t version_minor, sym_lookup_t, script_lookup_t);
    void setPriority(int32_t priority) override;
    virtual void launchThreads(WorkerCallback_t cbk, void *data);
    static void * helperThreadProc(void *vrsc);
    RsdCpuScriptImpl * setTLS(RsdCpuScriptImpl *sc);

    Context * getContext() {return mRSC;}
    uint32_t getThreadCount() const {
        return mWorkers.mCount + 1;
    }

    // Launch foreach kernel
    void launchForEach(const Allocation **ains, uint32_t inLen, Allocation *aout,
                       const RsScriptCall *sc, MTLaunchStructForEach *mtls);

    // Launch a general reduce kernel
    void launchReduce(const Allocation ** ains, uint32_t inLen, Allocation *aout,
                      MTLaunchStructReduce *mtls);

    CpuScript * createScript(const ScriptC *s, char const *resName, char const *cacheDir,
                             uint8_t const *bitcode, size_t bitcodeSize, uint32_t flags) override;
    CpuScript * createIntrinsic(const Script *s, RsScriptIntrinsicID iid, Element *e) override;
    void* createScriptGroup(const ScriptGroupBase *sg) override;

    const RsdCpuReference::CpuSymbol *symLookup(const char *);

    RsdCpuReference::CpuScript *lookupScript(const Script *s) {
        return mScriptLookupFn(mRSC, s);
    }

    void setSelectRTCallback(RSSelectRTCallback pSelectRTCallback) {
        mSelectRTCallback = pSelectRTCallback;
    }
    RSSelectRTCallback getSelectRTCallback() {
        return mSelectRTCallback;
    }

    virtual void setBccPluginName(const char *name) {
        mBccPluginName.assign(name);
    }
    virtual const char *getBccPluginName() const {
        return mBccPluginName.c_str();
    }
    bool getInKernel() override { return mInKernel; }

    // Set to true if we should embed global variable information in the code.
    void setEmbedGlobalInfo(bool v) override {
        mEmbedGlobalInfo = v;
    }

    // Returns true if we should embed global variable information in the code.
    bool getEmbedGlobalInfo() const override {
        return mEmbedGlobalInfo;
    }

    // Set to true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    void setEmbedGlobalInfoSkipConstant(bool v) override {
        mEmbedGlobalInfoSkipConstant = v;
    }

    // Returns true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    bool getEmbedGlobalInfoSkipConstant() const override {
        return mEmbedGlobalInfoSkipConstant;
    }

protected:
    Context *mRSC;
    uint32_t version_major;
    uint32_t version_minor;
    //bool mHasGraphics;
    bool mInKernel;  // Is a parallel kernel execution underway?

    struct Workers {
        volatile int mRunningCount;
        volatile int mLaunchCount;
        uint32_t mCount;
        pthread_t *mThreadId;
        pid_t *mNativeThreadId;
        Signal mCompleteSignal;
        Signal *mLaunchSignals;
        WorkerCallback_t mLaunchCallback;
        void *mLaunchData;
    };
    Workers mWorkers;
    bool mExit;
    sym_lookup_t mSymLookupFn;
    script_lookup_t mScriptLookupFn;

    ScriptTLSStruct mTlsStruct;

    RSSelectRTCallback mSelectRTCallback;
    std::string mBccPluginName;

    // Specifies whether we should embed global variable information in the
    // code via special RS variables that can be examined later by the driver.
    // Defaults to true.
    bool mEmbedGlobalInfo;

    // Specifies whether we should skip constant (immutable) global variables
    // when potentially embedding information about globals.
    // Defaults to true.
    bool mEmbedGlobalInfoSkipConstant;

    long mPageSize;

    // Launch a general reduce kernel
    void launchReduceSerial(const Allocation ** ains, uint32_t inLen, Allocation *aout,
                            MTLaunchStructReduce *mtls);
    void launchReduceParallel(const Allocation ** ains, uint32_t inLen, Allocation *aout,
                              MTLaunchStructReduce *mtls);
};


} // namespace renderscript
} // namespace android

#endif
