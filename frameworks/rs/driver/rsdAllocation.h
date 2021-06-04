/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef RSD_ALLOCATION_H
#define RSD_ALLOCATION_H

#include <rs_hal.h>
#include <rsRuntime.h>
#include <rsAllocation.h>

#include "../cpu_ref/rsd_cpu.h"

#include <GLES/gl.h>
#include <GLES2/gl2.h>


class RsdFrameBufferObj;
struct ANativeWindow;
struct ANativeWindowBuffer;
struct ANativeWindow_Buffer;

struct DrvAllocation {
    // Is this a legal structure to be used as a texture source.
    // Initially this will require 1D or 2D and color data
    uint32_t textureID;

    // Is this a legal structure to be used as a vertex source.
    // Initially this will require 1D and x(yzw).  Additional per element data
    // is allowed.
    uint32_t bufferID;

    // Is this a legal structure to be used as an FBO render target
    uint32_t renderTargetID;

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    GLenum glTarget;
    GLenum glType;
    GLenum glFormat;
#else
    int glTarget;
    int glType;
    int glFormat;
#endif

#if !defined(RS_COMPATIBILITY_LIB)
    ANativeWindowBuffer *wndBuffer;
#else
    ANativeWindow_Buffer *wndBuffer;
#endif

    bool useUserProvidedPtr;
    bool uploadDeferred;

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    RsdFrameBufferObj * readBackFBO;
#endif
    ANativeWindow *wnd;
    ANativeWindow *wndSurface;
};

#ifndef RS_COMPATIBILITY_LIB
GLenum rsdTypeToGLType(RsDataType t);
GLenum rsdKindToGLFormat(RsDataKind k);
#endif


uint32_t rsdAllocationGrallocBits(const android::renderscript::Context *rsc,
                                  android::renderscript::Allocation *alloc);
bool rsdAllocationInit(const android::renderscript::Context *rsc,
                       android::renderscript::Allocation *alloc,
                       bool forceZero);
#ifdef RS_COMPATIBILITY_LIB
bool rsdAllocationInitStrided(const android::renderscript::Context *rsc,
                              android::renderscript::Allocation *alloc,
                              bool forceZero, size_t requiredAlignment);
#endif
bool rsdAllocationAdapterInit(const android::renderscript::Context *rsc,
                              android::renderscript::Allocation *alloc);
void rsdAllocationDestroy(const android::renderscript::Context *rsc,
                          android::renderscript::Allocation *alloc);

void rsdAllocationResize(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         const android::renderscript::Type *newType, bool zeroNew);
void rsdAllocationSyncAll(const android::renderscript::Context *rsc,
                          const android::renderscript::Allocation *alloc,
                          RsAllocationUsageType src);
void rsdAllocationMarkDirty(const android::renderscript::Context *rsc,
                            const android::renderscript::Allocation *alloc);
void rsdAllocationSetSurface(const android::renderscript::Context *rsc,
                            android::renderscript::Allocation *alloc, ANativeWindow *nw);
void rsdAllocationIoSend(const android::renderscript::Context *rsc,
                         android::renderscript::Allocation *alloc);
void rsdAllocationIoReceive(const android::renderscript::Context *rsc,
                            android::renderscript::Allocation *alloc);

void rsdAllocationData1D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t lod, size_t count,
                         const void *data, size_t sizeBytes);
void rsdAllocationData2D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                         uint32_t w, uint32_t h,
                         const void *data, size_t sizeBytes, size_t stride);
void rsdAllocationData3D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                         uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes,
                         size_t stride);

void rsdAllocationRead1D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t lod, size_t count,
                         void *data, size_t sizeBytes);
void rsdAllocationRead2D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                         uint32_t w, uint32_t h,
                         void *data, size_t sizeBytes, size_t stride);
void rsdAllocationRead3D(const android::renderscript::Context *rsc,
                         const android::renderscript::Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                         uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes,
                         size_t stride);

void * rsdAllocationLock1D(const android::renderscript::Context *rsc,
                          const android::renderscript::Allocation *alloc);
void rsdAllocationUnlock1D(const android::renderscript::Context *rsc,
                          const android::renderscript::Allocation *alloc);


void rsdAllocationData1D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstLod, size_t count,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcLod);
void rsdAllocationData2D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff, uint32_t dstLod,
                               RsAllocationCubemapFace dstFace, uint32_t w, uint32_t h,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
                               RsAllocationCubemapFace srcFace);
void rsdAllocationData3D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                               uint32_t dstLod,
                               uint32_t w, uint32_t h, uint32_t d,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                               uint32_t srcLod);

void rsdAllocationElementData(const android::renderscript::Context *rsc,
                              const android::renderscript::Allocation *alloc,
                              uint32_t x, uint32_t y, uint32_t z,
                              const void *data, uint32_t elementOff, size_t sizeBytes);

void rsdAllocationElementRead(const android::renderscript::Context *rsc,
                              const android::renderscript::Allocation *alloc,
                              uint32_t x, uint32_t y, uint32_t z,
                              void *data, uint32_t elementOff, size_t sizeBytes);

void rsdAllocationGenerateMipmaps(const android::renderscript::Context *rsc,
                                  const android::renderscript::Allocation *alloc);

void rsdAllocationUpdateCachedObject(const android::renderscript::Context *rsc,
                                     const android::renderscript::Allocation *alloc,
                                     android::renderscript::rs_allocation *obj);

void rsdAllocationAdapterOffset(const android::renderscript::Context *rsc,
                                const android::renderscript::Allocation *alloc);


#endif
