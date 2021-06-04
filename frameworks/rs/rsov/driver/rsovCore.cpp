/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "rsovCore.h"

#include <malloc.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "cpu_ref/rsd_cpu.h"
#include "rsContext.h"
#include "rsovAllocation.h"
#include "rsovContext.h"
#include "rsovElement.h"
#include "rsovSampler.h"
#include "rsovScript.h"
#include "rsovScriptGroup.h"
#include "rsovType.h"

namespace android {
namespace renderscript {

namespace {

void SetPriority(const Context *rsc, int32_t priority) {
  RSoVHal *dc = (RSoVHal *)rsc->mHal.drv;

  dc->mCpuRef->setPriority(priority);
}

void Shutdown(Context *rsc) {
  RSoVHal *dc = (RSoVHal *)rsc->mHal.drv;
  delete dc->mCpuRef;
  free(dc);
  rsc->mHal.drv = nullptr;
}

void *AllocRuntimeMem(size_t size, uint32_t flags) {
    void* buffer = calloc(size, sizeof(char));
    return buffer;
}

void FreeRuntimeMem(void* ptr) {
    free(ptr);
}

const RsdCpuReference::CpuSymbol *lookupRuntimeStubs(
    Context *pContext, char const *name) {
  return nullptr;
}

}  // anonymous namespace

namespace rsov {

namespace {

RsdCpuReference::CpuScript *lookupCpuScript(Context *rsc, const Script *s) {
  if (RSoVScript::isScriptCpuBacked(s)) {
    return reinterpret_cast<RsdCpuReference::CpuScript *>(s->mHal.drv);
  }

  RSoVScript *rsovScript = reinterpret_cast<RSoVScript *>(s->mHal.drv);
  return rsovScript->getCpuScript();
}

}  // anonymous namespace

}  // namespace rsov


extern "C" bool rsdHalQueryHal(RsHalInitEnums entry, void **fnPtr) {
  switch (entry) {
    case RS_HAL_ALLOCATION_INIT:
      fnPtr[0] = (void *)rsovAllocationInit;
      break;
    case RS_HAL_ALLOCATION_INIT_OEM:
      fnPtr[0] = (void *)nullptr;
      break;
    case RS_HAL_ALLOCATION_INIT_ADAPTER:
      fnPtr[0] = (void *)rsovAllocationAdapterInit;
      break;
    case RS_HAL_ALLOCATION_DESTROY:
      fnPtr[0] = (void *)rsovAllocationDestroy;
      break;
    case RS_HAL_ALLOCATION_GET_GRALLOC_BITS:
      fnPtr[0] = (void *)rsovAllocationGrallocBits;
      break;
    case RS_HAL_ALLOCATION_DATA_1D:
      fnPtr[0] = (void *)rsovAllocationData1D;
      break;
    case RS_HAL_ALLOCATION_DATA_2D:
      fnPtr[0] = (void *)rsovAllocationData2D;
      break;
    case RS_HAL_ALLOCATION_DATA_3D:
      fnPtr[0] = (void *)rsovAllocationData3D;
      break;
    case RS_HAL_ALLOCATION_READ_1D:
      fnPtr[0] = (void *)rsovAllocationRead1D;
      break;
    case RS_HAL_ALLOCATION_READ_2D:
      fnPtr[0] = (void *)rsovAllocationRead2D;
      break;
    case RS_HAL_ALLOCATION_READ_3D:
      fnPtr[0] = (void *)rsovAllocationRead3D;
      break;
    case RS_HAL_ALLOCATION_LOCK_1D:
      fnPtr[0] = (void *)rsovAllocationLock1D;
      break;
    case RS_HAL_ALLOCATION_UNLOCK_1D:
      fnPtr[0] = (void *)rsovAllocationUnlock1D;
      break;
    case RS_HAL_ALLOCATION_COPY_1D:
      fnPtr[0] = (void *)rsovAllocationData1D_alloc;
      break;
    case RS_HAL_ALLOCATION_COPY_2D:
      fnPtr[0] = (void *)rsovAllocationData2D_alloc;
      break;
    case RS_HAL_ALLOCATION_COPY_3D:
      fnPtr[0] = (void *)rsovAllocationData3D_alloc;
      break;
    case RS_HAL_ALLOCATION_ADAPTER_OFFSET:
      fnPtr[0] = (void *)rsovAllocationAdapterOffset;
      break;
    case RS_HAL_ALLOCATION_RESIZE:
      fnPtr[0] = (void *)rsovAllocationResize;
      break;
    case RS_HAL_ALLOCATION_SYNC_ALL:
      fnPtr[0] = (void *)rsovAllocationSyncAll;
      break;
    case RS_HAL_ALLOCATION_MARK_DIRTY:
      fnPtr[0] = (void *)rsovAllocationMarkDirty;
      break;
    case RS_HAL_ALLOCATION_GENERATE_MIPMAPS:
      fnPtr[0] = (void *)rsovAllocationGenerateMipmaps;
      break;
    case RS_HAL_ALLOCATION_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)rsovAllocationUpdateCachedObject;
      break;
    case RS_HAL_ALLOCATION_GET_POINTER:
      fnPtr[0] = (void *)nullptr;
      break;
    case RS_HAL_ALLOCATION_SET_SURFACE:
      fnPtr[0] = (void *)rsovAllocationSetSurface;
      break;
    case RS_HAL_ALLOCATION_IO_SEND:
      fnPtr[0] = (void *)rsovAllocationIoSend;
      break;
    case RS_HAL_ALLOCATION_IO_RECEIVE:
      fnPtr[0] = (void *)rsovAllocationIoReceive;
      break;
    case RS_HAL_ALLOCATION_ELEMENT_DATA:
      fnPtr[0] = (void *)rsovAllocationElementData;
      break;
    case RS_HAL_ALLOCATION_ELEMENT_READ:
      fnPtr[0] = (void *)rsovAllocationElementRead;
      break;

    case RS_HAL_CORE_SHUTDOWN:
      fnPtr[0] = (void *)Shutdown;
      break;
    case RS_HAL_CORE_SET_PRIORITY:
      fnPtr[0] = (void *)SetPriority;
      break;
    case RS_HAL_CORE_ALLOC_RUNTIME_MEM:
      fnPtr[0] = (void *)AllocRuntimeMem;
      break;
    case RS_HAL_CORE_FREE_RUNTIME_MEM:
      fnPtr[0] = (void *)FreeRuntimeMem;
      break;
    case RS_HAL_CORE_FINISH:
      fnPtr[0] = (void *)nullptr;
      break;

    case RS_HAL_SCRIPT_INIT:
      fnPtr[0] = (void *)rsovScriptInit;
      break;
    case RS_HAL_SCRIPT_INIT_INTRINSIC:
      fnPtr[0] = (void *)rsovInitIntrinsic;
      break;
    case RS_HAL_SCRIPT_INVOKE_FUNCTION:
      fnPtr[0] = (void *)rsovScriptInvokeFunction;
      break;
    case RS_HAL_SCRIPT_INVOKE_ROOT:
      fnPtr[0] = (void *)rsovScriptInvokeRoot;
      break;
    case RS_HAL_SCRIPT_INVOKE_FOR_EACH:
      fnPtr[0] = (void *)rsovScriptInvokeForEach;
      break;
    case RS_HAL_SCRIPT_INVOKE_INIT:
      fnPtr[0] = (void *)rsovScriptInvokeInit;
      break;
    case RS_HAL_SCRIPT_INVOKE_FREE_CHILDREN:
      fnPtr[0] = (void *)rsovScriptInvokeFreeChildren;
      break;
    case RS_HAL_SCRIPT_DESTROY:
      fnPtr[0] = (void *)rsovScriptDestroy;
      break;
    case RS_HAL_SCRIPT_SET_GLOBAL_VAR:
      fnPtr[0] = (void *)rsovScriptSetGlobalVar;
      break;
    case RS_HAL_SCRIPT_GET_GLOBAL_VAR:
      fnPtr[0] = (void *)rsovScriptGetGlobalVar;
      break;
    case RS_HAL_SCRIPT_SET_GLOBAL_VAR_WITH_ELEMENT_DIM:
      fnPtr[0] = (void *)rsovScriptSetGlobalVarWithElemDims;
      break;
    case RS_HAL_SCRIPT_SET_GLOBAL_BIND:
      fnPtr[0] = (void *)rsovScriptSetGlobalBind;
      break;
    case RS_HAL_SCRIPT_SET_GLOBAL_OBJECT:
      fnPtr[0] = (void *)rsovScriptSetGlobalObj;
      break;
    case RS_HAL_SCRIPT_INVOKE_FOR_EACH_MULTI:
      fnPtr[0] = (void *)rsovScriptInvokeForEachMulti;
      break;
    case RS_HAL_SCRIPT_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)rsovScriptUpdateCachedObject;
      break;
    case RS_HAL_SCRIPT_INVOKE_REDUCE:
      fnPtr[0] = (void *)rsovScriptInvokeReduce;
      break;

    case RS_HAL_SAMPLER_INIT:
      fnPtr[0] = (void *)rsovSamplerInit;
      break;
    case RS_HAL_SAMPLER_DESTROY:
      fnPtr[0] = (void *)rsovSamplerDestroy;
      break;
    case RS_HAL_SAMPLER_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)rsovSamplerUpdateCachedObject;
      break;

    case RS_HAL_TYPE_INIT:
      fnPtr[0] = (void *)rsovTypeInit;
      break;
    case RS_HAL_TYPE_DESTROY:
      fnPtr[0] = (void *)rsovTypeDestroy;
      break;
    case RS_HAL_TYPE_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)rsovTypeUpdateCachedObject;
      break;

    case RS_HAL_ELEMENT_INIT:
      fnPtr[0] = (void *)rsovElementInit;
      break;
    case RS_HAL_ELEMENT_DESTROY:
      fnPtr[0] = (void *)rsovElementDestroy;
      break;
    case RS_HAL_ELEMENT_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)rsovElementUpdateCachedObject;
      break;

    case RS_HAL_SCRIPT_GROUP_INIT:
      fnPtr[0] = (void *)rsovScriptGroupInit;
      break;
    case RS_HAL_SCRIPT_GROUP_DESTROY:
      fnPtr[0] = (void *)rsovScriptGroupDestroy;
      break;
    case RS_HAL_SCRIPT_GROUP_UPDATE_CACHED_OBJECT:
      fnPtr[0] = (void *)nullptr;
      break;
    case RS_HAL_SCRIPT_GROUP_SET_INPUT:
      fnPtr[0] = (void *)rsovScriptGroupSetInput;
      break;
    case RS_HAL_SCRIPT_GROUP_SET_OUTPUT:
      fnPtr[0] = (void *)rsovScriptGroupSetOutput;
      break;
    case RS_HAL_SCRIPT_GROUP_EXECUTE:
      fnPtr[0] = (void *)rsovScriptGroupExecute;
      break;

    // Ignore entries for the legacy graphics api,

    default:
      ALOGE("ERROR: unknown RenderScript HAL API query, %i", entry);
      return false;
  }

  return true;
}

extern "C" void rsdHalAbort(RsContext) {}

extern "C" bool rsdHalQueryVersion(uint32_t *major, uint32_t *minor) {
  *major = RS_HAL_VERSION;
  *minor = 0;
  return true;
}

extern "C" bool rsdHalInit(RsContext c, uint32_t version_major,
                           uint32_t version_minor) {
  Context *rsc = (Context *)c;

  std::unique_ptr<RSoVHal> hal(new RSoVHal());
  if (!hal) {
    ALOGE("Failed creating RSoV driver hal.");
    return false;
  }

  std::unique_ptr<rsov::RSoVContext> rsov(rsov::RSoVContext::create());
  if (!rsov) {
    ALOGE("RSoVContext::create for driver hal failed.");
    return false;
  }

  std::unique_ptr<RsdCpuReference> cpuref(RsdCpuReference::create(rsc, version_major, version_minor,
                                                                  &lookupRuntimeStubs,
                                                                  &rsov::lookupCpuScript));
  if (!cpuref) {
    ALOGE("RsdCpuReference::create for driver hal failed.");
    return false;
  }

  hal->mRSoV = rsov.release();
  hal->mCpuRef = cpuref.release();
  rsc->mHal.drv = hal.release();

  return true;
}

}  // namespace renderscript
}  // namespace android
