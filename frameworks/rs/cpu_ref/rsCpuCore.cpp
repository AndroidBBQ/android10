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

#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsCpuScriptGroup.h"
#include "rsCpuScriptGroup2.h"

#include <malloc.h>
#include "rsContext.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define REDUCE_ALOGV(mtls, level, ...) do { if ((mtls)->logReduce >= (level)) ALOGV(__VA_ARGS__); } while(0)

static pthread_key_t gThreadTLSKey = 0;
static uint32_t gThreadTLSKeyCount = 0;
static pthread_mutex_t gInitMutex = PTHREAD_MUTEX_INITIALIZER;

namespace android {
namespace renderscript {

bool gArchUseSIMD = false;

RsdCpuReference::~RsdCpuReference() {
}

RsdCpuReference * RsdCpuReference::create(Context *rsc, uint32_t version_major,
        uint32_t version_minor, sym_lookup_t lfn, script_lookup_t slfn
        , RSSelectRTCallback pSelectRTCallback,
        const char *pBccPluginName
        ) {

    RsdCpuReferenceImpl *cpu = new RsdCpuReferenceImpl(rsc);
    if (!cpu) {
        return nullptr;
    }
    if (!cpu->init(version_major, version_minor, lfn, slfn)) {
        delete cpu;
        return nullptr;
    }

    cpu->setSelectRTCallback(pSelectRTCallback);
    if (pBccPluginName) {
        cpu->setBccPluginName(pBccPluginName);
    }

    return cpu;
}


Context * RsdCpuReference::getTlsContext() {
    ScriptTLSStruct * tls = (ScriptTLSStruct *)pthread_getspecific(gThreadTLSKey);
    return tls->mContext;
}

const Script * RsdCpuReference::getTlsScript() {
    ScriptTLSStruct * tls = (ScriptTLSStruct *)pthread_getspecific(gThreadTLSKey);
    return tls->mScript;
}

pthread_key_t RsdCpuReference::getThreadTLSKey(){ return gThreadTLSKey; }

////////////////////////////////////////////////////////////
///

RsdCpuReferenceImpl::RsdCpuReferenceImpl(Context *rsc) {
    mRSC = rsc;

    version_major = 0;
    version_minor = 0;
    mInKernel = false;
    memset(&mWorkers, 0, sizeof(mWorkers));
    memset(&mTlsStruct, 0, sizeof(mTlsStruct));
    mExit = false;
    mSelectRTCallback = nullptr;
    mEmbedGlobalInfo = true;
    mEmbedGlobalInfoSkipConstant = true;
}


void * RsdCpuReferenceImpl::helperThreadProc(void *vrsc) {
    RsdCpuReferenceImpl *dc = (RsdCpuReferenceImpl *)vrsc;

    uint32_t idx = __sync_fetch_and_add(&dc->mWorkers.mLaunchCount, 1);

    //ALOGV("RS helperThread starting %p idx=%i", dc, idx);

    dc->mWorkers.mLaunchSignals[idx].init();
    dc->mWorkers.mNativeThreadId[idx] = gettid();

    memset(&dc->mTlsStruct, 0, sizeof(dc->mTlsStruct));
    int status = pthread_setspecific(gThreadTLSKey, &dc->mTlsStruct);
    if (status) {
        ALOGE("pthread_setspecific %i", status);
    }

#if 0
    typedef struct {uint64_t bits[1024 / 64]; } cpu_set_t;
    cpu_set_t cpuset;
    memset(&cpuset, 0, sizeof(cpuset));
    cpuset.bits[idx / 64] |= 1ULL << (idx % 64);
    int ret = syscall(241, rsc->mWorkers.mNativeThreadId[idx],
              sizeof(cpuset), &cpuset);
    ALOGE("SETAFFINITY ret = %i %s", ret, EGLUtils::strerror(ret));
#endif

    while (!dc->mExit) {
        dc->mWorkers.mLaunchSignals[idx].wait();
        if (dc->mWorkers.mLaunchCallback) {
           // idx +1 is used because the calling thread is always worker 0.
           dc->mWorkers.mLaunchCallback(dc->mWorkers.mLaunchData, idx+1);
        }
        __sync_fetch_and_sub(&dc->mWorkers.mRunningCount, 1);
        dc->mWorkers.mCompleteSignal.set();
    }

    //ALOGV("RS helperThread exited %p idx=%i", dc, idx);
    return nullptr;
}

// Launch a kernel.
// The callback function is called to execute the kernel.
void RsdCpuReferenceImpl::launchThreads(WorkerCallback_t cbk, void *data) {
    mWorkers.mLaunchData = data;
    mWorkers.mLaunchCallback = cbk;

    // fast path for very small launches
    MTLaunchStructCommon *mtls = (MTLaunchStructCommon *)data;
    if (mtls && mtls->dimPtr->y <= 1 && mtls->end.x <= mtls->start.x + mtls->mSliceSize) {
        if (mWorkers.mLaunchCallback) {
            mWorkers.mLaunchCallback(mWorkers.mLaunchData, 0);
        }
        return;
    }

    mWorkers.mRunningCount = mWorkers.mCount;
    __sync_synchronize();

    for (uint32_t ct = 0; ct < mWorkers.mCount; ct++) {
        mWorkers.mLaunchSignals[ct].set();
    }

    // We use the calling thread as one of the workers so we can start without
    // the delay of the thread wakeup.
    if (mWorkers.mLaunchCallback) {
        mWorkers.mLaunchCallback(mWorkers.mLaunchData, 0);
    }

    while (__sync_fetch_and_or(&mWorkers.mRunningCount, 0) != 0) {
        mWorkers.mCompleteSignal.wait();
    }
}


void RsdCpuReferenceImpl::lockMutex() {
    pthread_mutex_lock(&gInitMutex);
}

void RsdCpuReferenceImpl::unlockMutex() {
    pthread_mutex_unlock(&gInitMutex);
}

// Determine if the CPU we're running on supports SIMD instructions.
static void GetCpuInfo() {
    // Read the CPU flags from /proc/cpuinfo.
    FILE *cpuinfo = fopen("/proc/cpuinfo", "re");

    if (!cpuinfo) {
        return;
    }

    char cpuinfostr[4096];
    // fgets() ends with newline or EOF, need to check the whole
    // "cpuinfo" file to make sure we can use SIMD or not.
    while (fgets(cpuinfostr, sizeof(cpuinfostr), cpuinfo)) {
#if defined(ARCH_ARM_HAVE_VFP) || defined(ARCH_ARM_USE_INTRINSICS)
        gArchUseSIMD = strstr(cpuinfostr, " neon") || strstr(cpuinfostr, " asimd");
#elif defined(ARCH_X86_HAVE_SSSE3)
        gArchUseSIMD = strstr(cpuinfostr, " ssse3");
#endif
        if (gArchUseSIMD) {
            break;
        }
    }
    fclose(cpuinfo);
}

bool RsdCpuReferenceImpl::init(uint32_t version_major, uint32_t version_minor,
                               sym_lookup_t lfn, script_lookup_t slfn) {
    mSymLookupFn = lfn;
    mScriptLookupFn = slfn;

    lockMutex();
    if (!gThreadTLSKeyCount) {
        int status = pthread_key_create(&gThreadTLSKey, nullptr);
        if (status) {
            ALOGE("Failed to init thread tls key.");
            unlockMutex();
            return false;
        }
    }
    gThreadTLSKeyCount++;
    unlockMutex();

    mTlsStruct.mContext = mRSC;
    mTlsStruct.mScript = nullptr;
    int status = pthread_setspecific(gThreadTLSKey, &mTlsStruct);
    if (status) {
        ALOGE("pthread_setspecific %i", status);
    }

    mPageSize = sysconf(_SC_PAGE_SIZE);
    // ALOGV("page size = %ld", mPageSize);

    GetCpuInfo();

    int cpu = sysconf(_SC_NPROCESSORS_CONF);
    if(mRSC->props.mDebugMaxThreads) {
        cpu = mRSC->props.mDebugMaxThreads;
    }
    if (cpu < 2) {
        mWorkers.mCount = 0;
        return true;
    }

    // Subtract one from the cpu count because we also use the command thread as a worker.
    mWorkers.mCount = (uint32_t)(cpu - 1);

    if (mRSC->props.mLogScripts) {
      ALOGV("%p Launching thread(s), CPUs %i", mRSC, mWorkers.mCount + 1);
    }

    mWorkers.mThreadId = (pthread_t *) calloc(mWorkers.mCount, sizeof(pthread_t));
    mWorkers.mNativeThreadId = (pid_t *) calloc(mWorkers.mCount, sizeof(pid_t));
    mWorkers.mLaunchSignals = new Signal[mWorkers.mCount];
    mWorkers.mLaunchCallback = nullptr;

    mWorkers.mCompleteSignal.init();

    mWorkers.mRunningCount = mWorkers.mCount;
    mWorkers.mLaunchCount = 0;
    __sync_synchronize();

    pthread_attr_t threadAttr;
    status = pthread_attr_init(&threadAttr);
    if (status) {
        ALOGE("Failed to init thread attribute.");
        return false;
    }

    for (uint32_t ct=0; ct < mWorkers.mCount; ct++) {
        status = pthread_create(&mWorkers.mThreadId[ct], &threadAttr, helperThreadProc, this);
        if (status) {
            mWorkers.mCount = ct;
            ALOGE("Created fewer than expected number of RS threads.");
            break;
        }
    }
    while (__sync_fetch_and_or(&mWorkers.mRunningCount, 0) != 0) {
        usleep(100);
    }

    pthread_attr_destroy(&threadAttr);
    return true;
}


void RsdCpuReferenceImpl::setPriority(int32_t priority) {
    for (uint32_t ct=0; ct < mWorkers.mCount; ct++) {
        setpriority(PRIO_PROCESS, mWorkers.mNativeThreadId[ct], priority);
    }
}

RsdCpuReferenceImpl::~RsdCpuReferenceImpl() {
    mExit = true;
    mWorkers.mLaunchData = nullptr;
    mWorkers.mLaunchCallback = nullptr;
    mWorkers.mRunningCount = mWorkers.mCount;
    __sync_synchronize();
    for (uint32_t ct = 0; ct < mWorkers.mCount; ct++) {
        mWorkers.mLaunchSignals[ct].set();
    }
    void *res;
    for (uint32_t ct = 0; ct < mWorkers.mCount; ct++) {
        pthread_join(mWorkers.mThreadId[ct], &res);
    }
    // b/23109602
    // TODO: Refactor the implementation with threadpool to
    // fix the race condition in the destuctor.
    // rsAssert(__sync_fetch_and_or(&mWorkers.mRunningCount, 0) == 0);
    free(mWorkers.mThreadId);
    free(mWorkers.mNativeThreadId);
    delete[] mWorkers.mLaunchSignals;

    // Global structure cleanup.
    lockMutex();
    --gThreadTLSKeyCount;
    if (!gThreadTLSKeyCount) {
        pthread_key_delete(gThreadTLSKey);
    }
    unlockMutex();

}

// Set up the appropriate input and output pointers to the kernel driver info structure.
// Inputs:
//   mtls - The MTLaunchStruct holding information about the kernel launch
//   fep - The forEach parameters (driver info structure)
//   x, y, z, lod, face, a1, a2, a3, a4 - The start offsets into each dimension
static inline void FepPtrSetup(const MTLaunchStructForEach *mtls, RsExpandKernelDriverInfo *fep,
                               uint32_t x, uint32_t y,
                               uint32_t z = 0, uint32_t lod = 0,
                               RsAllocationCubemapFace face = RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X,
                               uint32_t a1 = 0, uint32_t a2 = 0, uint32_t a3 = 0, uint32_t a4 = 0) {
    // When rsForEach passes a null input allocation (as opposed to no input),
    // fep->inLen can be 1 with mtls->ains[0] being null.
    // This should only happen on old style kernels.
    for (uint32_t i = 0; i < fep->inLen; i++) {
        if (mtls->ains[i] == nullptr) {
            rsAssert(fep->inLen == 1);
            continue;
        }
        fep->inPtr[i] = (const uint8_t *)mtls->ains[i]->getPointerUnchecked(x, y, z, lod, face, a1, a2, a3, a4);
    }
    if (mtls->aout[0] != nullptr) {
        fep->outPtr[0] = (uint8_t *)mtls->aout[0]->getPointerUnchecked(x, y, z, lod, face, a1, a2, a3, a4);
    }
}

// Set up the appropriate input and output pointers to the kernel driver info structure.
// Inputs:
//   mtls - The MTLaunchStruct holding information about the kernel launch
//   redp - The reduce parameters (driver info structure)
//   x, y, z - The start offsets into each dimension
static inline void RedpPtrSetup(const MTLaunchStructReduce *mtls, RsExpandKernelDriverInfo *redp,
                                uint32_t x, uint32_t y, uint32_t z) {
    for (uint32_t i = 0; i < redp->inLen; i++) {
        redp->inPtr[i] = (const uint8_t *)mtls->ains[i]->getPointerUnchecked(x, y, z);
    }
}

static uint32_t sliceInt(uint32_t *p, uint32_t val, uint32_t start, uint32_t end) {
    if (start >= end) {
        *p = start;
        return val;
    }

    uint32_t div = end - start;

    uint32_t n = val / div;
    *p = (val - (n * div)) + start;
    return n;
}

static bool SelectOuterSlice(const MTLaunchStructCommon *mtls, RsExpandKernelDriverInfo* info, uint32_t sliceNum) {
    uint32_t r = sliceNum;
    r = sliceInt(&info->current.z, r, mtls->start.z, mtls->end.z);
    r = sliceInt(&info->current.lod, r, mtls->start.lod, mtls->end.lod);
    r = sliceInt(&info->current.face, r, mtls->start.face, mtls->end.face);
    r = sliceInt(&info->current.array[0], r, mtls->start.array[0], mtls->end.array[0]);
    r = sliceInt(&info->current.array[1], r, mtls->start.array[1], mtls->end.array[1]);
    r = sliceInt(&info->current.array[2], r, mtls->start.array[2], mtls->end.array[2]);
    r = sliceInt(&info->current.array[3], r, mtls->start.array[3], mtls->end.array[3]);
    return r == 0;
}

static bool SelectZSlice(const MTLaunchStructCommon *mtls, RsExpandKernelDriverInfo* info, uint32_t sliceNum) {
    return sliceInt(&info->current.z, sliceNum, mtls->start.z, mtls->end.z) == 0;
}

static void walk_general_foreach(void *usr, uint32_t idx) {
    MTLaunchStructForEach *mtls = (MTLaunchStructForEach *)usr;
    RsExpandKernelDriverInfo fep = mtls->fep;
    fep.lid = idx;
    ForEachFunc_t fn = mtls->kernel;

    while(1) {
        uint32_t slice = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);

        if (!SelectOuterSlice(mtls, &fep, slice)) {
            return;
        }

        for (fep.current.y = mtls->start.y; fep.current.y < mtls->end.y;
             fep.current.y++) {

            FepPtrSetup(mtls, &fep, mtls->start.x,
                        fep.current.y, fep.current.z, fep.current.lod,
                        (RsAllocationCubemapFace)fep.current.face,
                        fep.current.array[0], fep.current.array[1],
                        fep.current.array[2], fep.current.array[3]);

            fn(&fep, mtls->start.x, mtls->end.x, mtls->fep.outStride[0]);
        }
    }
}

static void walk_2d_foreach(void *usr, uint32_t idx) {
    MTLaunchStructForEach *mtls = (MTLaunchStructForEach *)usr;
    RsExpandKernelDriverInfo fep = mtls->fep;
    fep.lid = idx;
    ForEachFunc_t fn = mtls->kernel;

    while (1) {
        uint32_t slice  = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);
        uint32_t yStart = mtls->start.y + slice * mtls->mSliceSize;
        uint32_t yEnd   = yStart + mtls->mSliceSize;

        yEnd = rsMin(yEnd, mtls->end.y);

        if (yEnd <= yStart) {
            return;
        }

        for (fep.current.y = yStart; fep.current.y < yEnd; fep.current.y++) {
            FepPtrSetup(mtls, &fep, mtls->start.x, fep.current.y);

            fn(&fep, mtls->start.x, mtls->end.x, fep.outStride[0]);
        }
    }
}

static void walk_1d_foreach(void *usr, uint32_t idx) {
    MTLaunchStructForEach *mtls = (MTLaunchStructForEach *)usr;
    RsExpandKernelDriverInfo fep = mtls->fep;
    fep.lid = idx;
    ForEachFunc_t fn = mtls->kernel;

    while (1) {
        uint32_t slice  = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);
        uint32_t xStart = mtls->start.x + slice * mtls->mSliceSize;
        uint32_t xEnd   = xStart + mtls->mSliceSize;

        xEnd = rsMin(xEnd, mtls->end.x);

        if (xEnd <= xStart) {
            return;
        }

        FepPtrSetup(mtls, &fep, xStart, 0);

        fn(&fep, xStart, xEnd, fep.outStride[0]);
    }
}

// The function format_bytes() is an auxiliary function to assist in logging.
//
// Bytes are read from an input (inBuf) and written (as pairs of hex digits)
// to an output (outBuf).
//
// Output format:
// - starts with ": "
// - each input byte is translated to a pair of hex digits
// - bytes are separated by "." except that every fourth separator is "|"
// - if the input is sufficiently long, the output is truncated and terminated with "..."
//
// Arguments:
// - outBuf  -- Pointer to buffer of type "FormatBuf" into which output is written
// - inBuf   -- Pointer to bytes which are to be formatted into outBuf
// - inBytes -- Number of bytes in inBuf
//
// Constant:
// - kFormatInBytesMax -- Only min(kFormatInBytesMax, inBytes) bytes will be read
//                        from inBuf
//
// Return value:
// - pointer (const char *) to output (which is part of outBuf)
//
static const int kFormatInBytesMax = 16;
// ": " + 2 digits per byte + 1 separator between bytes + "..." + null
typedef char FormatBuf[2 + kFormatInBytesMax*2 + (kFormatInBytesMax - 1) + 3 + 1];
static const char *format_bytes(FormatBuf *outBuf, const uint8_t *inBuf, const int inBytes) {
  strlcpy(*outBuf, ": ", sizeof(*outBuf));
  int pos = 2;
  const int lim = std::min(kFormatInBytesMax, inBytes);
  for (int i = 0; i < lim; ++i) {
    if (i) {
      sprintf(*outBuf + pos, (i % 4 ? "." : "|"));
      ++pos;
    }
    sprintf(*outBuf + pos, "%02x", inBuf[i]);
    pos += 2;
  }
  if (kFormatInBytesMax < inBytes)
    strlcpy(*outBuf + pos, "...", sizeof(FormatBuf) - pos);
  return *outBuf;
}

static void reduce_get_accumulator(uint8_t *&accumPtr, MTLaunchStructReduce *mtls,
                                   const char *walkerName, uint32_t threadIdx) {
  rsAssert(!accumPtr);

  uint32_t accumIdx = (uint32_t)__sync_fetch_and_add(&mtls->accumCount, 1);
  if (mtls->outFunc) {
    accumPtr = mtls->accumAlloc + mtls->accumStride * accumIdx;
  } else {
    if (accumIdx == 0) {
      accumPtr = mtls->redp.outPtr[0];
    } else {
      accumPtr = mtls->accumAlloc + mtls->accumStride * (accumIdx - 1);
    }
  }
  REDUCE_ALOGV(mtls, 2, "%s(%p): idx = %u got accumCount %u and accumPtr %p",
               walkerName, mtls->accumFunc, threadIdx, accumIdx, accumPtr);
  // initialize accumulator
  if (mtls->initFunc) {
    mtls->initFunc(accumPtr);
  } else {
    memset(accumPtr, 0, mtls->accumSize);
  }
}

static void walk_1d_reduce(void *usr, uint32_t idx) {
  MTLaunchStructReduce *mtls = (MTLaunchStructReduce *)usr;
  RsExpandKernelDriverInfo redp = mtls->redp;

  // find accumulator
  uint8_t *&accumPtr = mtls->accumPtr[idx];
  if (!accumPtr) {
    reduce_get_accumulator(accumPtr, mtls, __func__, idx);
  }

  // accumulate
  const ReduceAccumulatorFunc_t fn = mtls->accumFunc;
  while (1) {
    uint32_t slice  = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);
    uint32_t xStart = mtls->start.x + slice * mtls->mSliceSize;
    uint32_t xEnd   = xStart + mtls->mSliceSize;

    xEnd = rsMin(xEnd, mtls->end.x);

    if (xEnd <= xStart) {
      return;
    }

    RedpPtrSetup(mtls, &redp, xStart, 0, 0);
    fn(&redp, xStart, xEnd, accumPtr);

    // Emit log line after slice has been run, so that we can include
    // the results of the run on that line.
    FormatBuf fmt;
    if (mtls->logReduce >= 3) {
      format_bytes(&fmt, accumPtr, mtls->accumSize);
    } else {
      fmt[0] = 0;
    }
    REDUCE_ALOGV(mtls, 2, "walk_1d_reduce(%p): idx = %u, x in [%u, %u)%s",
                 mtls->accumFunc, idx, xStart, xEnd, fmt);
  }
}

static void walk_2d_reduce(void *usr, uint32_t idx) {
  MTLaunchStructReduce *mtls = (MTLaunchStructReduce *)usr;
  RsExpandKernelDriverInfo redp = mtls->redp;

  // find accumulator
  uint8_t *&accumPtr = mtls->accumPtr[idx];
  if (!accumPtr) {
    reduce_get_accumulator(accumPtr, mtls, __func__, idx);
  }

  // accumulate
  const ReduceAccumulatorFunc_t fn = mtls->accumFunc;
  while (1) {
    uint32_t slice  = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);
    uint32_t yStart = mtls->start.y + slice * mtls->mSliceSize;
    uint32_t yEnd   = yStart + mtls->mSliceSize;

    yEnd = rsMin(yEnd, mtls->end.y);

    if (yEnd <= yStart) {
      return;
    }

    for (redp.current.y = yStart; redp.current.y < yEnd; redp.current.y++) {
      RedpPtrSetup(mtls, &redp, mtls->start.x, redp.current.y, 0);
      fn(&redp, mtls->start.x, mtls->end.x, accumPtr);
    }

    FormatBuf fmt;
    if (mtls->logReduce >= 3) {
      format_bytes(&fmt, accumPtr, mtls->accumSize);
    } else {
      fmt[0] = 0;
    }
    REDUCE_ALOGV(mtls, 2, "walk_2d_reduce(%p): idx = %u, y in [%u, %u)%s",
                 mtls->accumFunc, idx, yStart, yEnd, fmt);
  }
}

static void walk_3d_reduce(void *usr, uint32_t idx) {
  MTLaunchStructReduce *mtls = (MTLaunchStructReduce *)usr;
  RsExpandKernelDriverInfo redp = mtls->redp;

  // find accumulator
  uint8_t *&accumPtr = mtls->accumPtr[idx];
  if (!accumPtr) {
    reduce_get_accumulator(accumPtr, mtls, __func__, idx);
  }

  // accumulate
  const ReduceAccumulatorFunc_t fn = mtls->accumFunc;
  while (1) {
    uint32_t slice  = (uint32_t)__sync_fetch_and_add(&mtls->mSliceNum, 1);

    if (!SelectZSlice(mtls, &redp, slice)) {
      return;
    }

    for (redp.current.y = mtls->start.y; redp.current.y < mtls->end.y; redp.current.y++) {
      RedpPtrSetup(mtls, &redp, mtls->start.x, redp.current.y, redp.current.z);
      fn(&redp, mtls->start.x, mtls->end.x, accumPtr);
    }

    FormatBuf fmt;
    if (mtls->logReduce >= 3) {
      format_bytes(&fmt, accumPtr, mtls->accumSize);
    } else {
      fmt[0] = 0;
    }
    REDUCE_ALOGV(mtls, 2, "walk_3d_reduce(%p): idx = %u, z = %u%s",
                 mtls->accumFunc, idx, redp.current.z, fmt);
  }
}

// Launch a general reduce-style kernel.
// Inputs:
//   ains[0..inLen-1]: Array of allocations that contain the inputs
//   aout:             The allocation that will hold the output
//   mtls:             Holds launch parameters
void RsdCpuReferenceImpl::launchReduce(const Allocation ** ains,
                                       uint32_t inLen,
                                       Allocation * aout,
                                       MTLaunchStructReduce *mtls) {
  mtls->logReduce = mRSC->props.mLogReduce;
  if ((mWorkers.mCount >= 1) && mtls->isThreadable && !mInKernel) {
    launchReduceParallel(ains, inLen, aout, mtls);
  } else {
    launchReduceSerial(ains, inLen, aout, mtls);
  }
}

// Launch a general reduce-style kernel, single-threaded.
// Inputs:
//   ains[0..inLen-1]: Array of allocations that contain the inputs
//   aout:             The allocation that will hold the output
//   mtls:             Holds launch parameters
void RsdCpuReferenceImpl::launchReduceSerial(const Allocation ** ains,
                                             uint32_t inLen,
                                             Allocation * aout,
                                             MTLaunchStructReduce *mtls) {
  REDUCE_ALOGV(mtls, 1, "launchReduceSerial(%p): %u x %u x %u", mtls->accumFunc,
               mtls->redp.dim.x, mtls->redp.dim.y, mtls->redp.dim.z);

  // In the presence of outconverter, we allocate temporary memory for
  // the accumulator.
  //
  // In the absence of outconverter, we use the output allocation as the
  // accumulator.
  uint8_t *const accumPtr = (mtls->outFunc
                             ? static_cast<uint8_t *>(malloc(mtls->accumSize))
                             : mtls->redp.outPtr[0]);

  // initialize
  if (mtls->initFunc) {
    mtls->initFunc(accumPtr);
  } else {
    memset(accumPtr, 0, mtls->accumSize);
  }

  // accumulate
  const ReduceAccumulatorFunc_t fn = mtls->accumFunc;
  uint32_t slice = 0;
  while (SelectOuterSlice(mtls, &mtls->redp, slice++)) {
    for (mtls->redp.current.y = mtls->start.y;
         mtls->redp.current.y < mtls->end.y;
         mtls->redp.current.y++) {
      RedpPtrSetup(mtls, &mtls->redp, mtls->start.x, mtls->redp.current.y, mtls->redp.current.z);
      fn(&mtls->redp, mtls->start.x, mtls->end.x, accumPtr);
    }
  }

  // outconvert
  if (mtls->outFunc) {
    mtls->outFunc(mtls->redp.outPtr[0], accumPtr);
    free(accumPtr);
  }
}

// Launch a general reduce-style kernel, multi-threaded.
// Inputs:
//   ains[0..inLen-1]: Array of allocations that contain the inputs
//   aout:             The allocation that will hold the output
//   mtls:             Holds launch parameters
void RsdCpuReferenceImpl::launchReduceParallel(const Allocation ** ains,
                                               uint32_t inLen,
                                               Allocation * aout,
                                               MTLaunchStructReduce *mtls) {
  // For now, we don't know how to go parallel in the absence of a combiner.
  if (!mtls->combFunc) {
    launchReduceSerial(ains, inLen, aout, mtls);
    return;
  }

  // Number of threads = "main thread" + number of other (worker) threads
  const uint32_t numThreads = mWorkers.mCount + 1;

  // In the absence of outconverter, we use the output allocation as
  // an accumulator, and therefore need to allocate one fewer accumulator.
  const uint32_t numAllocAccum = numThreads - (mtls->outFunc == nullptr);

  // If mDebugReduceSplitAccum, then we want each accumulator to start
  // on a page boundary.  (TODO: Would some unit smaller than a page
  // be sufficient to avoid false sharing?)
  if (mRSC->props.mDebugReduceSplitAccum) {
    // Round up accumulator size to an integral number of pages
    mtls->accumStride =
        (unsigned(mtls->accumSize) + unsigned(mPageSize)-1) &
        ~(unsigned(mPageSize)-1);
    // Each accumulator gets its own page.  Alternatively, if we just
    // wanted to make sure no two accumulators are on the same page,
    // we could instead do
    //   allocSize = mtls->accumStride * (numAllocation - 1) + mtls->accumSize
    const size_t allocSize = mtls->accumStride * numAllocAccum;
    mtls->accumAlloc = static_cast<uint8_t *>(memalign(mPageSize, allocSize));
  } else {
    mtls->accumStride = mtls->accumSize;
    mtls->accumAlloc = static_cast<uint8_t *>(malloc(mtls->accumStride * numAllocAccum));
  }

  const size_t accumPtrArrayBytes = sizeof(uint8_t *) * numThreads;
  mtls->accumPtr = static_cast<uint8_t **>(malloc(accumPtrArrayBytes));
  memset(mtls->accumPtr, 0, accumPtrArrayBytes);

  mtls->accumCount = 0;

  rsAssert(!mInKernel);
  mInKernel = true;
  REDUCE_ALOGV(mtls, 1, "launchReduceParallel(%p): %u x %u x %u, %u threads, accumAlloc = %p",
               mtls->accumFunc,
               mtls->redp.dim.x, mtls->redp.dim.y, mtls->redp.dim.z,
               numThreads, mtls->accumAlloc);
  if (mtls->redp.dim.z > 1) {
    mtls->mSliceSize = 1;
    launchThreads(walk_3d_reduce, mtls);
  } else if (mtls->redp.dim.y > 1) {
    mtls->mSliceSize = rsMax(1U, mtls->redp.dim.y / (numThreads * 4));
    launchThreads(walk_2d_reduce, mtls);
  } else {
    mtls->mSliceSize = rsMax(1U, mtls->redp.dim.x / (numThreads * 4));
    launchThreads(walk_1d_reduce, mtls);
  }
  mInKernel = false;

  // Combine accumulators and identify final accumulator
  uint8_t *finalAccumPtr = (mtls->outFunc ? nullptr : mtls->redp.outPtr[0]);
  //   Loop over accumulators, combining into finalAccumPtr.  If finalAccumPtr
  //   is null, then the first accumulator I find becomes finalAccumPtr.
  for (unsigned idx = 0; idx < mtls->accumCount; ++idx) {
    uint8_t *const thisAccumPtr = mtls->accumPtr[idx];
    if (finalAccumPtr) {
      if (finalAccumPtr != thisAccumPtr) {
        if (mtls->combFunc) {
          if (mtls->logReduce >= 3) {
            FormatBuf fmt;
            REDUCE_ALOGV(mtls, 3, "launchReduceParallel(%p): accumulating into%s",
                         mtls->accumFunc,
                         format_bytes(&fmt, finalAccumPtr, mtls->accumSize));
            REDUCE_ALOGV(mtls, 3, "launchReduceParallel(%p):    accumulator[%d]%s",
                         mtls->accumFunc, idx,
                         format_bytes(&fmt, thisAccumPtr, mtls->accumSize));
          }
          mtls->combFunc(finalAccumPtr, thisAccumPtr);
        } else {
          rsAssert(!"expected combiner");
        }
      }
    } else {
      finalAccumPtr = thisAccumPtr;
    }
  }
  rsAssert(finalAccumPtr != nullptr);
  if (mtls->logReduce >= 3) {
    FormatBuf fmt;
    REDUCE_ALOGV(mtls, 3, "launchReduceParallel(%p): final accumulator%s",
                 mtls->accumFunc, format_bytes(&fmt, finalAccumPtr, mtls->accumSize));
  }

  // Outconvert
  if (mtls->outFunc) {
    mtls->outFunc(mtls->redp.outPtr[0], finalAccumPtr);
    if (mtls->logReduce >= 3) {
      FormatBuf fmt;
      REDUCE_ALOGV(mtls, 3, "launchReduceParallel(%p): final outconverted result%s",
                   mtls->accumFunc,
                   format_bytes(&fmt, mtls->redp.outPtr[0], mtls->redp.outStride[0]));
    }
  }

  // Clean up
  free(mtls->accumPtr);
  free(mtls->accumAlloc);
}


void RsdCpuReferenceImpl::launchForEach(const Allocation ** ains,
                                        uint32_t inLen,
                                        Allocation* aout,
                                        const RsScriptCall* sc,
                                        MTLaunchStructForEach* mtls) {

    //android::StopWatch kernel_time("kernel time");

    bool outerDims = (mtls->start.z != mtls->end.z) ||
                     (mtls->start.face != mtls->end.face) ||
                     (mtls->start.lod != mtls->end.lod) ||
                     (mtls->start.array[0] != mtls->end.array[0]) ||
                     (mtls->start.array[1] != mtls->end.array[1]) ||
                     (mtls->start.array[2] != mtls->end.array[2]) ||
                     (mtls->start.array[3] != mtls->end.array[3]);

    if ((mWorkers.mCount >= 1) && mtls->isThreadable && !mInKernel) {
        const size_t targetByteChunk = 16 * 1024;
        mInKernel = true;  // NOTE: The guard immediately above ensures this was !mInKernel

        if (outerDims) {
            // No fancy logic for chunk size
            mtls->mSliceSize = 1;
            launchThreads(walk_general_foreach, mtls);
        } else if (mtls->fep.dim.y > 1) {
            uint32_t s1 = mtls->fep.dim.y / ((mWorkers.mCount + 1) * 4);
            uint32_t s2 = 0;

            // This chooses our slice size to rate limit atomic ops to
            // one per 16k bytes of reads/writes.
            if ((mtls->aout[0] != nullptr) && mtls->aout[0]->mHal.drvState.lod[0].stride) {
                s2 = targetByteChunk / mtls->aout[0]->mHal.drvState.lod[0].stride;
            } else if (mtls->ains[0]) {
                s2 = targetByteChunk / mtls->ains[0]->mHal.drvState.lod[0].stride;
            } else {
                // Launch option only case
                // Use s1 based only on the dimensions
                s2 = s1;
            }
            mtls->mSliceSize = rsMin(s1, s2);

            if(mtls->mSliceSize < 1) {
                mtls->mSliceSize = 1;
            }

            launchThreads(walk_2d_foreach, mtls);
        } else {
            uint32_t s1 = mtls->fep.dim.x / ((mWorkers.mCount + 1) * 4);
            uint32_t s2 = 0;

            // This chooses our slice size to rate limit atomic ops to
            // one per 16k bytes of reads/writes.
            if ((mtls->aout[0] != nullptr) && mtls->aout[0]->getType()->getElementSizeBytes()) {
                s2 = targetByteChunk / mtls->aout[0]->getType()->getElementSizeBytes();
            } else if (mtls->ains[0]) {
                s2 = targetByteChunk / mtls->ains[0]->getType()->getElementSizeBytes();
            } else {
                // Launch option only case
                // Use s1 based only on the dimensions
                s2 = s1;
            }
            mtls->mSliceSize = rsMin(s1, s2);

            if (mtls->mSliceSize < 1) {
                mtls->mSliceSize = 1;
            }

            launchThreads(walk_1d_foreach, mtls);
        }
        mInKernel = false;

    } else {
        ForEachFunc_t fn = mtls->kernel;
        uint32_t slice = 0;


        while(SelectOuterSlice(mtls, &mtls->fep, slice++)) {
            for (mtls->fep.current.y = mtls->start.y;
                 mtls->fep.current.y < mtls->end.y;
                 mtls->fep.current.y++) {

                FepPtrSetup(mtls, &mtls->fep, mtls->start.x,
                            mtls->fep.current.y, mtls->fep.current.z, mtls->fep.current.lod,
                            (RsAllocationCubemapFace) mtls->fep.current.face,
                            mtls->fep.current.array[0], mtls->fep.current.array[1],
                            mtls->fep.current.array[2], mtls->fep.current.array[3]);

                fn(&mtls->fep, mtls->start.x, mtls->end.x, mtls->fep.outStride[0]);
            }
        }
    }
}

RsdCpuScriptImpl * RsdCpuReferenceImpl::setTLS(RsdCpuScriptImpl *sc) {
    //ALOGE("setTls %p", sc);
    ScriptTLSStruct * tls = (ScriptTLSStruct *)pthread_getspecific(gThreadTLSKey);
    rsAssert(tls);
    RsdCpuScriptImpl *old = tls->mImpl;
    tls->mImpl = sc;
    tls->mContext = mRSC;
    if (sc) {
        tls->mScript = sc->getScript();
    } else {
        tls->mScript = nullptr;
    }
    return old;
}

const RsdCpuReference::CpuSymbol * RsdCpuReferenceImpl::symLookup(const char *name) {
    return mSymLookupFn(mRSC, name);
}


RsdCpuReference::CpuScript * RsdCpuReferenceImpl::createScript(const ScriptC *s,
                                    char const *resName, char const *cacheDir,
                                    uint8_t const *bitcode, size_t bitcodeSize,
                                    uint32_t flags) {

    RsdCpuScriptImpl *i = new RsdCpuScriptImpl(this, s);
    if (!i->init(resName, cacheDir, bitcode, bitcodeSize, flags
        , getBccPluginName()
        )) {
        delete i;
        return nullptr;
    }
    return i;
}

extern RsdCpuScriptImpl * rsdIntrinsic_3DLUT(RsdCpuReferenceImpl *ctx,
                                             const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Convolve3x3(RsdCpuReferenceImpl *ctx,
                                                   const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_ColorMatrix(RsdCpuReferenceImpl *ctx,
                                                   const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_LUT(RsdCpuReferenceImpl *ctx,
                                           const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Convolve5x5(RsdCpuReferenceImpl *ctx,
                                                   const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Blur(RsdCpuReferenceImpl *ctx,
                                            const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_YuvToRGB(RsdCpuReferenceImpl *ctx,
                                                const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Blend(RsdCpuReferenceImpl *ctx,
                                             const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Histogram(RsdCpuReferenceImpl *ctx,
                                                 const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_Resize(RsdCpuReferenceImpl *ctx,
                                              const Script *s, const Element *e);
extern RsdCpuScriptImpl * rsdIntrinsic_BLAS(RsdCpuReferenceImpl *ctx,
                                              const Script *s, const Element *e);

RsdCpuReference::CpuScript * RsdCpuReferenceImpl::createIntrinsic(const Script *s,
                                    RsScriptIntrinsicID iid, Element *e) {

    RsdCpuScriptImpl *i = nullptr;
    switch (iid) {
    case RS_SCRIPT_INTRINSIC_ID_3DLUT:
        i = rsdIntrinsic_3DLUT(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_CONVOLVE_3x3:
        i = rsdIntrinsic_Convolve3x3(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_COLOR_MATRIX:
        i = rsdIntrinsic_ColorMatrix(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_LUT:
        i = rsdIntrinsic_LUT(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_CONVOLVE_5x5:
        i = rsdIntrinsic_Convolve5x5(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_BLUR:
        i = rsdIntrinsic_Blur(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_YUV_TO_RGB:
        i = rsdIntrinsic_YuvToRGB(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_BLEND:
        i = rsdIntrinsic_Blend(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_HISTOGRAM:
        i = rsdIntrinsic_Histogram(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_RESIZE:
        i = rsdIntrinsic_Resize(this, s, e);
        break;
    case RS_SCRIPT_INTRINSIC_ID_BLAS:
        i = rsdIntrinsic_BLAS(this, s, e);
        break;

    default:
        rsAssert(0);
    }

    return i;
}

void* RsdCpuReferenceImpl::createScriptGroup(const ScriptGroupBase *sg) {
  switch (sg->getApiVersion()) {
    case ScriptGroupBase::SG_V1: {
      CpuScriptGroupImpl *sgi = new CpuScriptGroupImpl(this, sg);
      if (!sgi->init()) {
        delete sgi;
        return nullptr;
      }
      return sgi;
    }
    case ScriptGroupBase::SG_V2: {
      return new CpuScriptGroup2Impl(this, sg);
    }
  }
  return nullptr;
}

} // namespace renderscript
} // namespace android
