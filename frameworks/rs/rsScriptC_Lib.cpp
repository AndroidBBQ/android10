/*
 * Copyright (C) 2009-2012 The Android Open Source Project
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

#include "rsContext.h"
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"
#include "rsgApiStructs.h"

#include <time.h>
#include <sstream>


namespace android {
namespace renderscript {


//////////////////////////////////////////////////////////////////////////////
// Math routines
//////////////////////////////////////////////////////////////////////////////

#if 0
static float SC_sinf_fast(float x) {
    const float A =   1.0f / (2.0f * M_PI);
    const float B = -16.0f;
    const float C =   8.0f;

    // scale angle for easy argument reduction
    x *= A;

    if (fabsf(x) >= 0.5f) {
        // argument reduction
        x = x - ceilf(x + 0.5f) + 1.0f;
    }

    const float y = B * x * fabsf(x) + C * x;
    return 0.2215f * (y * fabsf(y) - y) + y;
}

static float SC_cosf_fast(float x) {
    x += float(M_PI / 2);

    const float A =   1.0f / (2.0f * M_PI);
    const float B = -16.0f;
    const float C =   8.0f;

    // scale angle for easy argument reduction
    x *= A;

    if (fabsf(x) >= 0.5f) {
        // argument reduction
        x = x - ceilf(x + 0.5f) + 1.0f;
    }

    const float y = B * x * fabsf(x) + C * x;
    return 0.2215f * (y * fabsf(y) - y) + y;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Time routines
//////////////////////////////////////////////////////////////////////////////

time_t rsrTime(Context *rsc, time_t *timer) {
    return time(timer);
}

tm* rsrLocalTime(Context *rsc, tm *local, time_t *timer) {
    if (!local) {
      return nullptr;
    }

    // The native localtime function is not thread-safe, so we
    // have to apply locking for proper behavior in RenderScript.
    pthread_mutex_lock(&rsc->gLibMutex);
    tm *tmp = localtime(timer);
    memcpy(local, tmp, sizeof(int)*9);
    pthread_mutex_unlock(&rsc->gLibMutex);
    return local;
}

int64_t rsrUptimeMillis(Context *rsc) {
    return nanoseconds_to_milliseconds(systemTime(SYSTEM_TIME_MONOTONIC));
}

int64_t rsrUptimeNanos(Context *rsc) {
    return systemTime(SYSTEM_TIME_MONOTONIC);
}

float rsrGetDt(Context *rsc, const Script *sc) {
    int64_t l = sc->mEnviroment.mLastDtTime;
    sc->mEnviroment.mLastDtTime = systemTime(SYSTEM_TIME_MONOTONIC);
    return ((float)(sc->mEnviroment.mLastDtTime - l)) / 1.0e9;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

static void SetObjectRef(const Context *rsc, const ObjectBase *dst, const ObjectBase *src) {
    //ALOGE("setObjectRef  %p,%p  %p", rsc, dst, src);
    if (src) {
        CHECK_OBJ(src);
        src->incSysRef();
    }
    if (dst) {
        CHECK_OBJ(dst);
        dst->decSysRef();
    }
}

// Legacy, remove when drivers are updated
void rsrClearObject(const Context *rsc, void *dst) {
    ObjectBase **odst = (ObjectBase **)dst;
    if (ObjectBase::gDebugReferences) {
        ALOGE("rsrClearObject  %p,%p", odst, *odst);
    }
    if (odst[0]) {
        CHECK_OBJ(odst[0]);
        odst[0]->decSysRef();
    }
    *odst = nullptr;
}

void rsrClearObject(rs_object_base *dst) {
    if (ObjectBase::gDebugReferences) {
        ALOGE("rsrClearObject  %p,%p", dst, dst->p);
    }
    if (dst->p) {
        CHECK_OBJ(dst->p);
        dst->p->decSysRef();
    }
    dst->p = nullptr;
}

// Legacy, remove when drivers are updated
void rsrClearObject(const Context *rsc, rs_object_base *dst) {
    rsrClearObject(dst);
}

// Legacy, remove when drivers are updated
void rsrSetObject(const Context *rsc, void *dst, ObjectBase *src) {
    if (src == nullptr) {
        rsrClearObject(rsc, dst);
        return;
    }

    ObjectBase **odst = (ObjectBase **)dst;
    if (ObjectBase::gDebugReferences) {
        ALOGE("rsrSetObject (base) %p,%p  %p", dst, *odst, src);
    }
    SetObjectRef(rsc, odst[0], src);
    src->callUpdateCacheObject(rsc, dst);
}

void rsrSetObject(const Context *rsc, rs_object_base *dst, const ObjectBase *src) {
    if (src == nullptr) {
        rsrClearObject(rsc, dst);
        return;
    }

    ObjectBase **odst = (ObjectBase **)dst;
    if (ObjectBase::gDebugReferences) {
        ALOGE("rsrSetObject (base) %p,%p  %p", dst, *odst, src);
    }
    SetObjectRef(rsc, odst[0], src);
    src->callUpdateCacheObject(rsc, dst);
}

// Legacy, remove when drivers are updated
bool rsrIsObject(const Context *, ObjectBase* src) {
    ObjectBase **osrc = (ObjectBase **)src;
    return osrc != nullptr;
}

bool rsrIsObject(const Context *rsc, rs_object_base o) {
    return o.p != nullptr;
}



uint32_t rsrToClient(Context *rsc, int cmdID, const void *data, int len) {
    //ALOGE("SC_toClient %i %i %i", cmdID, len);
    return rsc->sendMessageToClient(data, RS_MESSAGE_TO_CLIENT_USER, cmdID, len, false);
}

uint32_t rsrToClientBlocking(Context *rsc, int cmdID, const void *data, int len) {
    //ALOGE("SC_toClientBlocking %i %i", cmdID, len);
    return rsc->sendMessageToClient(data, RS_MESSAGE_TO_CLIENT_USER, cmdID, len, true);
}

// Keep these two routines (using non-const void pointers) so that we can
// still use existing GPU drivers.
uint32_t rsrToClient(Context *rsc, int cmdID, void *data, int len) {
    return rsrToClient(rsc, cmdID, (const void *)data, len);
}

uint32_t rsrToClientBlocking(Context *rsc, int cmdID, void *data, int len) {
    return rsrToClientBlocking(rsc, cmdID, (const void *)data, len);
}

void rsrAllocationIoSend(Context *rsc, Allocation *src) {
    src->ioSend(rsc);
}

void rsrAllocationIoReceive(Context *rsc, Allocation *src) {
    src->ioReceive(rsc);
}

void rsrForEach(Context *rsc,
                Script *target,
                uint32_t slot,
                uint32_t numInputs,
                Allocation **in, Allocation *out,
                const void *usr, uint32_t usrBytes,
                const RsScriptCall *call) {
    target->runForEach(rsc, slot, (const Allocation**)in, numInputs, out, usr, usrBytes, call);
}

void rsrAllocationSyncAll(Context *rsc, Allocation *a, RsAllocationUsageType usage) {
    a->syncAll(rsc, usage);
}

// Helper for validateCopyArgs() - initialize the error message; only called on
// infrequently executed paths
static void initializeErrorMsg(std::stringstream &ss, int expectDim, bool isSrc) {
    ss << (expectDim == 1 ? "rsAllocationCopy1DRange" : "rsAllocationCopy2DRange") << ": ";
    ss << (isSrc? "source" : "destination") << " ";
}

// We are doing the check even in a non-debug context, which is permissible because in that case
// a failed bound check results in unspecified behavior.
static bool validateCopyArgs(Context *rsc, bool isSrc, uint32_t expectDim,
                             const Allocation *alloc, uint32_t xoff, uint32_t yoff,
                             uint32_t lod, uint32_t w, uint32_t h) {
    std::stringstream ss;

    if (lod >= alloc->mHal.drvState.lodCount) {
        initializeErrorMsg(ss, expectDim, isSrc);
        ss << "Mip level out of range: ";
        ss << lod << " >= " << alloc->mHal.drvState.lodCount;
        rsc->setError(RS_ERROR_FATAL_DEBUG, ss.str().c_str());
        return false;
    }

    const uint32_t allocDimX = alloc->mHal.drvState.lod[lod].dimX;

    // Check both in case xoff + w overflows
    if (xoff >= allocDimX || (xoff + w) > allocDimX) {
        initializeErrorMsg(ss, expectDim, isSrc);
        ss << "X range: ";
        ss << "[" << xoff << ", " << xoff + w << ") outside ";
        ss << "[0, " << allocDimX << ")";
        rsc->setError(RS_ERROR_FATAL_DEBUG, ss.str().c_str());
        return false;
    }

    const uint32_t allocDimY = alloc->mHal.drvState.lod[lod].dimY;

    if (expectDim > 1) {
        if (allocDimY == 0) {  // Copy2D was given an allocation of 1D
            initializeErrorMsg(ss, expectDim, isSrc);
            ss << "dimensionality invalid: expected 2D; given 1D rs_allocation";
            rsc->setError(RS_ERROR_FATAL_DEBUG, ss.str().c_str());
            return false;
        }
        // Check both in case yoff + h overflows
        if (yoff >= allocDimY || (yoff + h) > allocDimY) {
            initializeErrorMsg(ss, expectDim, isSrc);
            ss << "Y range: ";
            ss << "[" << yoff << ", " << yoff + h << ") outside ";
            ss << "[0, " << allocDimY << ")";
            rsc->setError(RS_ERROR_FATAL_DEBUG, ss.str().c_str());
            return false;
        }
    } else {
        if (allocDimY != 0) {  // Copy1D was given an allocation of 2D
            initializeErrorMsg(ss, expectDim, isSrc);
            ss << "dimensionality invalid: expected 1D; given 2D rs_allocation";
            rsc->setError(RS_ERROR_FATAL_DEBUG, ss.str().c_str());
            return false;
        }
    }

    return true;
}

void rsrAllocationCopy1DRange(Context *rsc, Allocation *dstAlloc,
                              uint32_t dstOff,
                              uint32_t dstMip,
                              uint32_t count,
                              Allocation *srcAlloc,
                              uint32_t srcOff, uint32_t srcMip) {
    if (!validateCopyArgs(rsc, false, 1, dstAlloc, dstOff, 0, dstMip, count, 1) ||
        !validateCopyArgs(rsc, true, 1, srcAlloc, srcOff, 0, srcMip, count, 1)) {
        return;
    }
    rsi_AllocationCopy2DRange(rsc, dstAlloc, dstOff, 0,
                              dstMip, 0, count, 1,
                              srcAlloc, srcOff, 0, srcMip, 0);
}

void rsrAllocationCopy2DRange(Context *rsc, Allocation *dstAlloc,
                              uint32_t dstXoff, uint32_t dstYoff,
                              uint32_t dstMip, uint32_t dstFace,
                              uint32_t width, uint32_t height,
                              Allocation *srcAlloc,
                              uint32_t srcXoff, uint32_t srcYoff,
                              uint32_t srcMip, uint32_t srcFace) {
    if (!validateCopyArgs(rsc, false, 2, dstAlloc, dstXoff, dstYoff, dstMip, width, height) ||
        !validateCopyArgs(rsc, true, 2, srcAlloc, srcXoff, srcYoff, srcMip, width, height)) {
        return;
    }

    rsi_AllocationCopy2DRange(rsc, dstAlloc, dstXoff, dstYoff,
                              dstMip, dstFace, width, height,
                              srcAlloc, srcXoff, srcYoff, srcMip, srcFace);
}

RsElement rsrElementCreate(Context *rsc, RsDataType dt, RsDataKind dk,
                           bool norm, uint32_t vecSize) {
    return rsi_ElementCreate(rsc, dt, dk, norm, vecSize);
}

RsType rsrTypeCreate(Context *rsc, const RsElement element, uint32_t dimX,
                     uint32_t dimY, uint32_t dimZ, bool mipmaps, bool faces,
                     uint32_t yuv) {
    return rsi_TypeCreate(rsc, element, dimX, dimY, dimZ, mipmaps, faces, yuv);
}

RsAllocation rsrAllocationCreateTyped(Context *rsc, const RsType type,
                                      RsAllocationMipmapControl mipmaps,
                                      uint32_t usages, uintptr_t ptr) {
    return rsi_AllocationCreateTyped(rsc, type, mipmaps, usages, ptr);
}

} // namespace renderscript
} // namespace android
