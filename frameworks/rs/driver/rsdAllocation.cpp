/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "rsdAllocation.h"
#include "rsdCore.h"

#include <android/native_window.h>

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#else
#include "rsdFrameBufferObj.h"
#include <vndk/window.h>

#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES/glext.h>
#endif

#include <malloc.h> // for memalign()
#include <unistd.h> // for close()

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::Element;
using android::renderscript::Type;
using android::renderscript::rs_allocation;
using android::renderscript::rsBoxFilter565;
using android::renderscript::rsBoxFilter8888;
using android::renderscript::rsMax;
using android::renderscript::rsRound;

#ifndef RS_COMPATIBILITY_LIB
const static GLenum gFaceOrder[] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

GLenum rsdTypeToGLType(RsDataType t) {
    switch (t) {
    case RS_TYPE_UNSIGNED_5_6_5:    return GL_UNSIGNED_SHORT_5_6_5;
    case RS_TYPE_UNSIGNED_5_5_5_1:  return GL_UNSIGNED_SHORT_5_5_5_1;
    case RS_TYPE_UNSIGNED_4_4_4_4:  return GL_UNSIGNED_SHORT_4_4_4_4;

    //case RS_TYPE_FLOAT_16:      return GL_HALF_FLOAT;
    case RS_TYPE_FLOAT_32:      return GL_FLOAT;
    case RS_TYPE_UNSIGNED_8:    return GL_UNSIGNED_BYTE;
    case RS_TYPE_UNSIGNED_16:   return GL_UNSIGNED_SHORT;
    case RS_TYPE_SIGNED_8:      return GL_BYTE;
    case RS_TYPE_SIGNED_16:     return GL_SHORT;
    default:    break;
    }
    return 0;
}

GLenum rsdKindToGLFormat(RsDataKind k) {
    switch (k) {
    case RS_KIND_PIXEL_L: return GL_LUMINANCE;
    case RS_KIND_PIXEL_A: return GL_ALPHA;
    case RS_KIND_PIXEL_LA: return GL_LUMINANCE_ALPHA;
    case RS_KIND_PIXEL_RGB: return GL_RGB;
    case RS_KIND_PIXEL_RGBA: return GL_RGBA;
    case RS_KIND_PIXEL_DEPTH: return GL_DEPTH_COMPONENT16;
    default: break;
    }
    return 0;
}
#endif

uint8_t *GetOffsetPtr(const android::renderscript::Allocation *alloc,
                      uint32_t xoff, uint32_t yoff, uint32_t zoff,
                      uint32_t lod, RsAllocationCubemapFace face) {
    uint8_t *ptr = (uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;
    ptr += face * alloc->mHal.drvState.faceOffset;
    ptr += zoff * alloc->mHal.drvState.lod[lod].dimY * alloc->mHal.drvState.lod[lod].stride;
    ptr += yoff * alloc->mHal.drvState.lod[lod].stride;
    ptr += xoff * alloc->mHal.state.elementSizeBytes;
    return ptr;
}


static void Update2DTexture(const Context *rsc, const Allocation *alloc, const void *ptr,
                            uint32_t xoff, uint32_t yoff, uint32_t lod,
                            RsAllocationCubemapFace face, uint32_t w, uint32_t h) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    rsAssert(drv->textureID);
    RSD_CALL_GL(glBindTexture, drv->glTarget, drv->textureID);
    RSD_CALL_GL(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    GLenum t = GL_TEXTURE_2D;
    if (alloc->mHal.state.hasFaces) {
        t = gFaceOrder[face];
    }
    RSD_CALL_GL(glTexSubImage2D, t, lod, xoff, yoff, w, h, drv->glFormat, drv->glType, ptr);
#endif
}


#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
static void Upload2DTexture(const Context *rsc, const Allocation *alloc, bool isFirstUpload) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    RSD_CALL_GL(glBindTexture, drv->glTarget, drv->textureID);
    RSD_CALL_GL(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);

    uint32_t faceCount = 1;
    if (alloc->mHal.state.hasFaces) {
        faceCount = 6;
    }

    rsdGLCheckError(rsc, "Upload2DTexture 1 ");
    for (uint32_t face = 0; face < faceCount; face ++) {
        for (uint32_t lod = 0; lod < alloc->mHal.state.type->getLODCount(); lod++) {
            const uint8_t *p = GetOffsetPtr(alloc, 0, 0, 0, lod, (RsAllocationCubemapFace)face);

            GLenum t = GL_TEXTURE_2D;
            if (alloc->mHal.state.hasFaces) {
                t = gFaceOrder[face];
            }

            if (isFirstUpload) {
                RSD_CALL_GL(glTexImage2D, t, lod, drv->glFormat,
                             alloc->mHal.state.type->getLODDimX(lod),
                             alloc->mHal.state.type->getLODDimY(lod),
                             0, drv->glFormat, drv->glType, p);
            } else {
                RSD_CALL_GL(glTexSubImage2D, t, lod, 0, 0,
                                alloc->mHal.state.type->getLODDimX(lod),
                                alloc->mHal.state.type->getLODDimY(lod),
                                drv->glFormat, drv->glType, p);
            }
        }
    }

    if (alloc->mHal.state.mipmapControl == RS_ALLOCATION_MIPMAP_ON_SYNC_TO_TEXTURE) {
        RSD_CALL_GL(glGenerateMipmap, drv->glTarget);
    }
    rsdGLCheckError(rsc, "Upload2DTexture");
}
#endif

static void UploadToTexture(const Context *rsc, const Allocation *alloc) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_INPUT) {
        if (!drv->textureID) {
            RSD_CALL_GL(glGenTextures, 1, &drv->textureID);
        }
        return;
    }

    if (!drv->glType || !drv->glFormat) {
        return;
    }

    if (!alloc->mHal.drvState.lod[0].mallocPtr) {
        return;
    }

    bool isFirstUpload = false;

    if (!drv->textureID) {
        RSD_CALL_GL(glGenTextures, 1, &drv->textureID);
        isFirstUpload = true;
    }

    Upload2DTexture(rsc, alloc, isFirstUpload);

    if (!(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT)) {
        if (alloc->mHal.drvState.lod[0].mallocPtr) {
            free(alloc->mHal.drvState.lod[0].mallocPtr);
            alloc->mHal.drvState.lod[0].mallocPtr = nullptr;
        }
    }
    rsdGLCheckError(rsc, "UploadToTexture");
#endif
}

static void AllocateRenderTarget(const Context *rsc, const Allocation *alloc) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    if (!drv->glFormat) {
        return;
    }

    if (!drv->renderTargetID) {
        RSD_CALL_GL(glGenRenderbuffers, 1, &drv->renderTargetID);

        if (!drv->renderTargetID) {
            // This should generally not happen
            ALOGE("allocateRenderTarget failed to gen mRenderTargetID");
            rsc->dumpDebug();
            return;
        }
        RSD_CALL_GL(glBindRenderbuffer, GL_RENDERBUFFER, drv->renderTargetID);
        RSD_CALL_GL(glRenderbufferStorage, GL_RENDERBUFFER, drv->glFormat,
                    alloc->mHal.drvState.lod[0].dimX, alloc->mHal.drvState.lod[0].dimY);
    }
    rsdGLCheckError(rsc, "AllocateRenderTarget");
#endif
}

static void UploadToBufferObject(const Context *rsc, const Allocation *alloc) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    rsAssert(!alloc->mHal.state.type->getDimY());
    rsAssert(!alloc->mHal.state.type->getDimZ());

    //alloc->mHal.state.usageFlags |= RS_ALLOCATION_USAGE_GRAPHICS_VERTEX;

    if (!drv->bufferID) {
        RSD_CALL_GL(glGenBuffers, 1, &drv->bufferID);
    }
    if (!drv->bufferID) {
        ALOGE("Upload to buffer object failed");
        drv->uploadDeferred = true;
        return;
    }
    RSD_CALL_GL(glBindBuffer, drv->glTarget, drv->bufferID);
    RSD_CALL_GL(glBufferData, drv->glTarget,
                alloc->mHal.state.type->getPackedSizeBytes(),
                alloc->mHal.drvState.lod[0].mallocPtr, GL_DYNAMIC_DRAW);
    RSD_CALL_GL(glBindBuffer, drv->glTarget, 0);
    rsdGLCheckError(rsc, "UploadToBufferObject");
#endif
}


static size_t DeriveYUVLayout(int yuv, Allocation::Hal::DrvState *state) {
#ifndef RS_COMPATIBILITY_LIB
    // For the flexible YCbCr format, layout is initialized during call to
    // Allocation::ioReceive.  Return early and avoid clobberring any
    // pre-existing layout.
    if (yuv == RS_YUV_420_888) {
        return 0;
    }
#endif

    // YUV only supports basic 2d
    // so we can stash the plane pointers in the mipmap levels.
    size_t uvSize = 0;
    state->lod[1].dimX = state->lod[0].dimX / 2;
    state->lod[1].dimY = state->lod[0].dimY / 2;
    state->lod[2].dimX = state->lod[0].dimX / 2;
    state->lod[2].dimY = state->lod[0].dimY / 2;
    state->yuv.shift = 1;
    state->yuv.step = 1;
    state->lodCount = 3;

    switch(yuv) {
    case RS_YUV_YV12:
        state->lod[2].stride = rsRound(state->lod[0].stride >> 1, 16);
        state->lod[2].mallocPtr = ((uint8_t *)state->lod[0].mallocPtr) +
                (state->lod[0].stride * state->lod[0].dimY);
        uvSize += state->lod[2].stride * state->lod[2].dimY;

        state->lod[1].stride = state->lod[2].stride;
        state->lod[1].mallocPtr = ((uint8_t *)state->lod[2].mallocPtr) +
                (state->lod[2].stride * state->lod[2].dimY);
        uvSize += state->lod[1].stride * state->lod[2].dimY;
        break;
    case RS_YUV_NV21:
        //state->lod[1].dimX = state->lod[0].dimX;
        state->lod[1].stride = state->lod[0].stride;
        state->lod[2].stride = state->lod[0].stride;
        state->lod[2].mallocPtr = ((uint8_t *)state->lod[0].mallocPtr) +
                (state->lod[0].stride * state->lod[0].dimY);
        state->lod[1].mallocPtr = ((uint8_t *)state->lod[2].mallocPtr) + 1;
        uvSize += state->lod[1].stride * state->lod[1].dimY;
        state->yuv.step = 2;
        break;
    default:
        rsAssert(0);
    }
    return uvSize;
}

static size_t AllocationBuildPointerTable(const Context *rsc, const Allocation *alloc,
                                          const Type *type, uint8_t *ptr, size_t requiredAlignment) {
    alloc->mHal.drvState.lod[0].dimX = type->getDimX();
    alloc->mHal.drvState.lod[0].dimY = type->getDimY();
    alloc->mHal.drvState.lod[0].dimZ = type->getDimZ();
    alloc->mHal.drvState.lod[0].mallocPtr = 0;
    // Stride needs to be aligned to a boundary defined by requiredAlignment!
    size_t stride = alloc->mHal.drvState.lod[0].dimX * type->getElementSizeBytes();
    alloc->mHal.drvState.lod[0].stride = rsRound(stride, requiredAlignment);
    alloc->mHal.drvState.lodCount = type->getLODCount();
    alloc->mHal.drvState.faceCount = type->getDimFaces();

    size_t offsets[Allocation::MAX_LOD];
    memset(offsets, 0, sizeof(offsets));

    size_t o = alloc->mHal.drvState.lod[0].stride * rsMax(alloc->mHal.drvState.lod[0].dimY, 1u) *
            rsMax(alloc->mHal.drvState.lod[0].dimZ, 1u);
    if (alloc->mHal.state.yuv) {
        o += DeriveYUVLayout(alloc->mHal.state.yuv, &alloc->mHal.drvState);

        for (uint32_t ct = 1; ct < alloc->mHal.drvState.lodCount; ct++) {
            offsets[ct] = (size_t)alloc->mHal.drvState.lod[ct].mallocPtr;
        }
    } else if(alloc->mHal.drvState.lodCount > 1) {
        uint32_t tx = alloc->mHal.drvState.lod[0].dimX;
        uint32_t ty = alloc->mHal.drvState.lod[0].dimY;
        uint32_t tz = alloc->mHal.drvState.lod[0].dimZ;
        for (uint32_t lod=1; lod < alloc->mHal.drvState.lodCount; lod++) {
            alloc->mHal.drvState.lod[lod].dimX = tx;
            alloc->mHal.drvState.lod[lod].dimY = ty;
            alloc->mHal.drvState.lod[lod].dimZ = tz;
            alloc->mHal.drvState.lod[lod].stride =
                    rsRound(tx * type->getElementSizeBytes(), requiredAlignment);
            offsets[lod] = o;
            o += alloc->mHal.drvState.lod[lod].stride * rsMax(ty, 1u) * rsMax(tz, 1u);
            if (tx > 1) tx >>= 1;
            if (ty > 1) ty >>= 1;
            if (tz > 1) tz >>= 1;
        }
    }

    alloc->mHal.drvState.faceOffset = o;

    alloc->mHal.drvState.lod[0].mallocPtr = ptr;
    for (uint32_t lod=1; lod < alloc->mHal.drvState.lodCount; lod++) {
        alloc->mHal.drvState.lod[lod].mallocPtr = ptr + offsets[lod];
    }

    size_t allocSize = alloc->mHal.drvState.faceOffset;
    if(alloc->mHal.drvState.faceCount) {
        allocSize *= 6;
    }

    return allocSize;
}

static size_t AllocationBuildPointerTable(const Context *rsc, const Allocation *alloc,
                                          const Type *type, uint8_t *ptr) {
    return AllocationBuildPointerTable(rsc, alloc, type, ptr, Allocation::kMinimumRSAlignment);
}

static uint8_t* allocAlignedMemory(size_t allocSize, bool forceZero, size_t requiredAlignment) {
    // We align all allocations to a boundary defined by requiredAlignment.
    uint8_t* ptr = (uint8_t *)memalign(requiredAlignment, allocSize);
    if (!ptr) {
        return nullptr;
    }
    if (forceZero) {
        memset(ptr, 0, allocSize);
    }
    return ptr;
}

bool rsdAllocationInitStrided(const Context *rsc, Allocation *alloc, bool forceZero, size_t requiredAlignment) {
    DrvAllocation *drv = (DrvAllocation *)calloc(1, sizeof(DrvAllocation));
    if (!drv) {
        return false;
    }
    alloc->mHal.drv = drv;

    // Check if requiredAlignment is power of 2, also requiredAlignment should be larger or equal than kMinimumRSAlignment.
    if ((requiredAlignment & (requiredAlignment-1)) != 0 || requiredAlignment < Allocation::kMinimumRSAlignment) {
        ALOGE("requiredAlignment must be power of 2");
        return false;
    }
    // Calculate the object size.
    size_t allocSize = AllocationBuildPointerTable(rsc, alloc, alloc->getType(), nullptr, requiredAlignment);

    uint8_t * ptr = nullptr;
    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_OUTPUT) {

    } else if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_INPUT) {
        // Allocation is allocated when the surface is created
        // in getSurface
#ifdef RS_COMPATIBILITY_LIB
    } else if (alloc->mHal.state.usageFlags == (RS_ALLOCATION_USAGE_INCREMENTAL_SUPPORT | RS_ALLOCATION_USAGE_SHARED)) {
        if (alloc->mHal.state.userProvidedPtr == nullptr) {
            ALOGE("User-backed buffer pointer cannot be null");
            return false;
        }
        if (alloc->getType()->getDimLOD() || alloc->getType()->getDimFaces()) {
            ALOGE("User-allocated buffers must not have multiple faces or LODs");
            return false;
        }

        drv->useUserProvidedPtr = true;
        ptr = (uint8_t*)alloc->mHal.state.userProvidedPtr;
#endif
    } else if (alloc->mHal.state.userProvidedPtr != nullptr) {
        // user-provided allocation
        // limitations: no faces, no LOD, USAGE_SCRIPT or SCRIPT+TEXTURE only
        if (!(alloc->mHal.state.usageFlags == (RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED) ||
              alloc->mHal.state.usageFlags == (RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE))) {
            ALOGE("Can't use user-allocated buffers if usage is not USAGE_SCRIPT | USAGE_SHARED or USAGE_SCRIPT | USAGE_SHARED | USAGE_GRAPHICS_TEXTURE");
            return false;
        }
        if (alloc->getType()->getDimLOD() || alloc->getType()->getDimFaces()) {
            ALOGE("User-allocated buffers must not have multiple faces or LODs");
            return false;
        }

        // rows must be aligned based on requiredAlignment.
        // validate that here, otherwise fall back to not use the user-backed allocation
        if (((alloc->getType()->getDimX() * alloc->getType()->getElement()->getSizeBytes()) % requiredAlignment) != 0) {
            ALOGV("User-backed allocation failed stride requirement, falling back to separate allocation");
            drv->useUserProvidedPtr = false;

            ptr = allocAlignedMemory(allocSize, forceZero, requiredAlignment);
            if (!ptr) {
                alloc->mHal.drv = nullptr;
                free(drv);
                return false;
            }

        } else {
            drv->useUserProvidedPtr = true;
            ptr = (uint8_t*)alloc->mHal.state.userProvidedPtr;
        }
    } else {
        ptr = allocAlignedMemory(allocSize, forceZero, requiredAlignment);
        if (!ptr) {
            alloc->mHal.drv = nullptr;
            free(drv);
            return false;
        }
    }
    // Build the pointer tables
    size_t verifySize = AllocationBuildPointerTable(rsc, alloc, alloc->getType(), ptr, requiredAlignment);
    if(allocSize != verifySize) {
        rsAssert(!"Size mismatch");
    }

    drv->glTarget = GL_NONE;
    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE) {
        if (alloc->mHal.state.hasFaces) {
            drv->glTarget = GL_TEXTURE_CUBE_MAP;
        } else {
            drv->glTarget = GL_TEXTURE_2D;
        }
    } else {
        if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_VERTEX) {
            drv->glTarget = GL_ARRAY_BUFFER;
        }
    }

#ifndef RS_COMPATIBILITY_LIB
    drv->glType = rsdTypeToGLType(alloc->mHal.state.type->getElement()->getComponent().getType());
    drv->glFormat = rsdKindToGLFormat(alloc->mHal.state.type->getElement()->getComponent().getKind());
#else
    drv->glType = 0;
    drv->glFormat = 0;
#endif

    if (alloc->mHal.state.usageFlags & ~RS_ALLOCATION_USAGE_SCRIPT) {
        drv->uploadDeferred = true;
    }

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    drv->readBackFBO = nullptr;
#endif

    // fill out the initial state of the buffer if we couldn't use the user-provided ptr and USAGE_SHARED was accepted
    if ((alloc->mHal.state.userProvidedPtr != 0) && (drv->useUserProvidedPtr == false)) {
        rsdAllocationData2D(rsc, alloc, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X, alloc->getType()->getDimX(), alloc->getType()->getDimY(), alloc->mHal.state.userProvidedPtr, allocSize, 0);
    }


#ifdef RS_FIND_OFFSETS
    ALOGE("pointer for allocation: %p", alloc);
    ALOGE("pointer for allocation.drv: %p", &alloc->mHal.drv);
#endif


    return true;
}

bool rsdAllocationInit(const Context *rsc, Allocation *alloc, bool forceZero) {
    return rsdAllocationInitStrided(rsc, alloc, forceZero, Allocation::kMinimumRSAlignment);
}

void rsdAllocationAdapterOffset(const Context *rsc, const Allocation *alloc) {
    //ALOGE("rsdAllocationAdapterOffset");

    // Get a base pointer to the new LOD
    const Allocation *base = alloc->mHal.state.baseAlloc;
    const Type *type = alloc->mHal.state.type;
    if (base == nullptr) {
        return;
    }

    //ALOGE("rsdAllocationAdapterOffset  %p  %p", ptrA, ptrB);
    //ALOGE("rsdAllocationAdapterOffset  lodCount %i", alloc->mHal.drvState.lodCount);

    const int lodBias = alloc->mHal.state.originLOD;
    uint32_t lodCount = rsMax(alloc->mHal.drvState.lodCount, (uint32_t)1);
    for (uint32_t lod=0; lod < lodCount; lod++) {
        alloc->mHal.drvState.lod[lod] = base->mHal.drvState.lod[lod + lodBias];
        alloc->mHal.drvState.lod[lod].mallocPtr = GetOffsetPtr(alloc,
                      alloc->mHal.state.originX, alloc->mHal.state.originY, alloc->mHal.state.originZ,
                      lodBias, (RsAllocationCubemapFace)alloc->mHal.state.originFace);
    }
}

bool rsdAllocationAdapterInit(const Context *rsc, Allocation *alloc) {
    DrvAllocation *drv = (DrvAllocation *)calloc(1, sizeof(DrvAllocation));
    if (!drv) {
        return false;
    }
    alloc->mHal.drv = drv;

    // We need to build an allocation that looks like a subset of the parent allocation
    rsdAllocationAdapterOffset(rsc, alloc);

    return true;
}

void rsdAllocationDestroy(const Context *rsc, Allocation *alloc) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    if (alloc->mHal.state.baseAlloc == nullptr) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        if (drv->bufferID) {
            // Causes a SW crash....
            //ALOGV(" mBufferID %i", mBufferID);
            //glDeleteBuffers(1, &mBufferID);
            //mBufferID = 0;
        }
        if (drv->textureID) {
            RSD_CALL_GL(glDeleteTextures, 1, &drv->textureID);
            drv->textureID = 0;
        }
        if (drv->renderTargetID) {
            RSD_CALL_GL(glDeleteRenderbuffers, 1, &drv->renderTargetID);
            drv->renderTargetID = 0;
        }
#endif

        if (alloc->mHal.drvState.lod[0].mallocPtr) {
            // don't free user-allocated ptrs or IO_OUTPUT buffers
            if (!(drv->useUserProvidedPtr) &&
                !(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_INPUT) &&
                !(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_OUTPUT)) {
                    free(alloc->mHal.drvState.lod[0].mallocPtr);
            }
            alloc->mHal.drvState.lod[0].mallocPtr = nullptr;
        }

#ifndef RS_COMPATIBILITY_LIB
#ifndef RS_VENDOR_LIB
        if (drv->readBackFBO != nullptr) {
            delete drv->readBackFBO;
            drv->readBackFBO = nullptr;
        }
#endif
        if ((alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_OUTPUT) &&
            (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT)) {
            ANativeWindow *nw = drv->wndSurface;
            if (nw) {
                //If we have an attached surface, need to release it.
                AHardwareBuffer* ahwb = ANativeWindowBuffer_getHardwareBuffer(drv->wndBuffer);
                int fenceID = -1;
                AHardwareBuffer_unlock(ahwb, &fenceID);
                ANativeWindow_cancelBuffer(nw, drv->wndBuffer, fenceID);
                ANativeWindow_release(nw);
                drv->wndSurface = nullptr;
                drv->wndBuffer = nullptr;
            }
        }
#endif
    }

    free(drv);
    alloc->mHal.drv = nullptr;
}

void rsdAllocationResize(const Context *rsc, const Allocation *alloc,
                         const Type *newType, bool zeroNew) {
    const uint32_t oldDimX = alloc->mHal.drvState.lod[0].dimX;
    const uint32_t dimX = newType->getDimX();

    // can't resize Allocations with user-allocated buffers
    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SHARED) {
        ALOGE("Resize cannot be called on a USAGE_SHARED allocation");
        return;
    }
    void * oldPtr = alloc->mHal.drvState.lod[0].mallocPtr;
    // Calculate the object size
    size_t s = AllocationBuildPointerTable(rsc, alloc, newType, nullptr);
    uint8_t *ptr = (uint8_t *)realloc(oldPtr, s);
    // Build the relative pointer tables.
    size_t verifySize = AllocationBuildPointerTable(rsc, alloc, newType, ptr);
    if(s != verifySize) {
        rsAssert(!"Size mismatch");
    }


    if (dimX > oldDimX) {
        size_t stride = alloc->mHal.state.elementSizeBytes;
        memset(((uint8_t *)alloc->mHal.drvState.lod[0].mallocPtr) + stride * oldDimX,
                 0, stride * (dimX - oldDimX));
    }
}

static void rsdAllocationSyncFromFBO(const Context *rsc, const Allocation *alloc) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (!alloc->getIsScript()) {
        return; // nothing to sync
    }

    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    RsdFrameBufferObj *lastFbo = dc->gl.currentFrameBuffer;

    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;
    if (!drv->textureID && !drv->renderTargetID) {
        return; // nothing was rendered here yet, so nothing to sync
    }
    if (drv->readBackFBO == nullptr) {
        drv->readBackFBO = new RsdFrameBufferObj();
        drv->readBackFBO->setColorTarget(drv, 0);
        drv->readBackFBO->setDimensions(alloc->getType()->getDimX(),
                                        alloc->getType()->getDimY());
    }

    // Bind the framebuffer object so we can read back from it
    drv->readBackFBO->setActive(rsc);

    // Do the readback
    RSD_CALL_GL(glReadPixels, 0, 0, alloc->mHal.drvState.lod[0].dimX,
                alloc->mHal.drvState.lod[0].dimY,
                drv->glFormat, drv->glType, alloc->mHal.drvState.lod[0].mallocPtr);

    // Revert framebuffer to its original
    lastFbo->setActive(rsc);
#endif
}


void rsdAllocationSyncAll(const Context *rsc, const Allocation *alloc,
                         RsAllocationUsageType src) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    if (src == RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET) {
        if(!alloc->getIsRenderTarget()) {
            rsc->setError(RS_ERROR_FATAL_DRIVER,
                          "Attempting to sync allocation from render target, "
                          "for non-render target allocation");
        } else if (alloc->getType()->getElement()->getKind() != RS_KIND_PIXEL_RGBA) {
            rsc->setError(RS_ERROR_FATAL_DRIVER, "Cannot only sync from RGBA"
                                                 "render target");
        } else {
            rsdAllocationSyncFromFBO(rsc, alloc);
        }
        return;
    }

    rsAssert(src == RS_ALLOCATION_USAGE_SCRIPT || src == RS_ALLOCATION_USAGE_SHARED);

    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE) {
        UploadToTexture(rsc, alloc);
    } else {
        if ((alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET) &&
            !(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_IO_OUTPUT)) {
            AllocateRenderTarget(rsc, alloc);
        }
    }
    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_VERTEX) {
        UploadToBufferObject(rsc, alloc);
    }

    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SHARED) {

        if (src == RS_ALLOCATION_USAGE_SHARED) {
            // just a memory fence for the CPU driver
            // vendor drivers probably want to flush any dirty cachelines for
            // this particular Allocation
            __sync_synchronize();
        }
    }

    drv->uploadDeferred = false;
}

void rsdAllocationMarkDirty(const Context *rsc, const Allocation *alloc) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;
    drv->uploadDeferred = true;
}

#ifndef RS_COMPATIBILITY_LIB
static bool IoGetBuffer(const Context *rsc, Allocation *alloc, ANativeWindow *nw) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;
    // Must lock the whole surface
    int fenceID = -1;
    int r = ANativeWindow_dequeueBuffer(nw, &drv->wndBuffer, &fenceID);
    if (r) {
        rsc->setError(RS_ERROR_DRIVER, "Error dequeueing IO output buffer.");
        close(fenceID);
        return false;
    }

    void *dst = nullptr;
    AHardwareBuffer* ahwb = ANativeWindowBuffer_getHardwareBuffer(drv->wndBuffer);
    r = AHardwareBuffer_lock(ahwb, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN,
                             fenceID, NULL, &dst);
    if (r) {
        rsc->setError(RS_ERROR_DRIVER, "Error Locking IO output buffer.");
        return false;
    }
    alloc->mHal.drvState.lod[0].mallocPtr = dst;
    alloc->mHal.drvState.lod[0].stride = drv->wndBuffer->stride * alloc->mHal.state.elementSizeBytes;
    rsAssert((alloc->mHal.drvState.lod[0].stride & 0xf) == 0);

    return true;
}
#endif

void rsdAllocationSetSurface(const Context *rsc, Allocation *alloc, ANativeWindow *nw) {
#ifndef RS_COMPATIBILITY_LIB
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    // Cleanup old surface if there is one.
    if (drv->wndSurface) {
        ANativeWindow *old = drv->wndSurface;
        AHardwareBuffer* ahwb = ANativeWindowBuffer_getHardwareBuffer(drv->wndBuffer);
        int fenceID = -1;
        int32_t r = AHardwareBuffer_unlock(ahwb, &fenceID);
        if (r) {
            rsc->setError(RS_ERROR_DRIVER, "Error unlocking output buffer.");
            close(fenceID);
            return;
        }
        r = ANativeWindow_cancelBuffer(old, drv->wndBuffer, fenceID);
        if (r) {
            rsc->setError(RS_ERROR_DRIVER, "Error canceling output buffer.");
            return;
        }
        ANativeWindow_release(old);
        drv->wndSurface = nullptr;
        drv->wndBuffer = nullptr;
    }

    if (nw) {
        int32_t r = ANativeWindow_setBuffersGeometry(nw, alloc->mHal.drvState.lod[0].dimX,
                                                 alloc->mHal.drvState.lod[0].dimY,
                                                 WINDOW_FORMAT_RGBA_8888);
        if (r) {
            rsc->setError(RS_ERROR_DRIVER, "Error setting IO output buffer geometry.");
            return;
        }

        if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT) {
            r = ANativeWindow_setUsage(nw,
                    AHARDWAREBUFFER_USAGE_CPU_READ_RARELY | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN);
            if (r) {
                rsc->setError(RS_ERROR_DRIVER, "Error setting IO output buffer usage.");
                return;
            }
        }

        IoGetBuffer(rsc, alloc, nw);
        drv->wndSurface = nw;
    }

    return;
#endif
}

void rsdAllocationIoSend(const Context *rsc, Allocation *alloc) {
#ifndef RS_COMPATIBILITY_LIB
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;
    ANativeWindow *nw = drv->wndSurface;
#ifndef RS_VENDOR_LIB
    if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET) {
        RsdHal *dc = (RsdHal *)rsc->mHal.drv;
        RSD_CALL_GL(eglSwapBuffers, dc->gl.egl.display, dc->gl.egl.surface);
        return;
    }
#endif
    if (nw) {
        if (alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT) {
            AHardwareBuffer* ahwb = ANativeWindowBuffer_getHardwareBuffer(drv->wndBuffer);
            int fenceID = -1;
            int32_t r = AHardwareBuffer_unlock(ahwb, &fenceID);
            if (r) {
                rsc->setError(RS_ERROR_DRIVER, "Error unlock output buffer.");
                close(fenceID);
                return;
            }
            r = ANativeWindow_queueBuffer(nw, drv->wndBuffer, fenceID);
            if (r) {
                rsc->setError(RS_ERROR_DRIVER, "Error sending IO output buffer.");
                return;
            }
            drv->wndBuffer = nullptr;
            IoGetBuffer(rsc, alloc, nw);
        }
    } else {
        rsc->setError(RS_ERROR_DRIVER, "Sent IO buffer with no attached surface.");
        return;
    }
#endif
}

void rsdAllocationIoReceive(const Context *rsc, Allocation *alloc) {
    if (alloc->mHal.state.yuv) {
        DeriveYUVLayout(alloc->mHal.state.yuv, &alloc->mHal.drvState);
    }
}


void rsdAllocationData1D(const Context *rsc, const Allocation *alloc,
                         uint32_t xoff, uint32_t lod, size_t count,
                         const void *data, size_t sizeBytes) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    const size_t eSize = alloc->mHal.state.type->getElementSizeBytes();
    uint8_t * ptr = GetOffsetPtr(alloc, xoff, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
    size_t size = count * eSize;
    if (ptr != data) {
        // Skip the copy if we are the same allocation. This can arise from
        // our Bitmap optimization, where we share the same storage.
        if (alloc->mHal.state.hasReferences) {
            alloc->incRefs(data, count);
            alloc->decRefs(ptr, count);
        }
        memcpy(ptr, data, size);
    }
    drv->uploadDeferred = true;
}

void rsdAllocationData2D(const Context *rsc, const Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                         uint32_t w, uint32_t h, const void *data, size_t sizeBytes, size_t stride) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    size_t eSize = alloc->mHal.state.elementSizeBytes;
    size_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    }

    if (alloc->mHal.drvState.lod[0].mallocPtr) {
        const uint8_t *src = static_cast<const uint8_t *>(data);
        uint8_t *dst = GetOffsetPtr(alloc, xoff, yoff, 0, lod, face);
        if (dst == src) {
            // Skip the copy if we are the same allocation. This can arise from
            // our Bitmap optimization, where we share the same storage.
            drv->uploadDeferred = true;
            return;
        }

        for (uint32_t line=yoff; line < (yoff+h); line++) {
            if (alloc->mHal.state.hasReferences) {
                alloc->incRefs(src, w);
                alloc->decRefs(dst, w);
            }
            memcpy(dst, src, lineSize);
            src += stride;
            dst += alloc->mHal.drvState.lod[lod].stride;
        }
        if (alloc->mHal.state.yuv) {
            size_t clineSize = lineSize;
            int lod = 1;
            int maxLod = 2;
            if (alloc->mHal.state.yuv == RS_YUV_YV12) {
                maxLod = 3;
                clineSize >>= 1;
            } else if (alloc->mHal.state.yuv == RS_YUV_NV21) {
                lod = 2;
                maxLod = 3;
            }

            while (lod < maxLod) {
                uint8_t *dst = GetOffsetPtr(alloc, xoff, yoff, 0, lod, face);

                for (uint32_t line=(yoff >> 1); line < ((yoff+h)>>1); line++) {
                    memcpy(dst, src, clineSize);
                    // When copying from an array to an Allocation, the src pointer
                    // to the array should just move by the number of bytes copied.
                    src += clineSize;
                    dst += alloc->mHal.drvState.lod[lod].stride;
                }
                lod++;
            }

        }
        drv->uploadDeferred = true;
    } else {
        Update2DTexture(rsc, alloc, data, xoff, yoff, lod, face, w, h);
    }
}

void rsdAllocationData3D(const Context *rsc, const Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t zoff,
                         uint32_t lod,
                         uint32_t w, uint32_t h, uint32_t d, const void *data,
                         size_t sizeBytes, size_t stride) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    uint32_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    }

    if (alloc->mHal.drvState.lod[0].mallocPtr) {
        const uint8_t *src = static_cast<const uint8_t *>(data);
        for (uint32_t z = zoff; z < (d + zoff); z++) {
            uint8_t *dst = GetOffsetPtr(alloc, xoff, yoff, z, lod,
                                        RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
            if (dst == src) {
                // Skip the copy if we are the same allocation. This can arise from
                // our Bitmap optimization, where we share the same storage.
                drv->uploadDeferred = true;
                return;
            }

            for (uint32_t line=yoff; line < (yoff+h); line++) {
                if (alloc->mHal.state.hasReferences) {
                    alloc->incRefs(src, w);
                    alloc->decRefs(dst, w);
                }
                memcpy(dst, src, lineSize);
                src += stride;
                dst += alloc->mHal.drvState.lod[lod].stride;
            }
        }
        drv->uploadDeferred = true;
    }
}

void rsdAllocationRead1D(const Context *rsc, const Allocation *alloc,
                         uint32_t xoff, uint32_t lod, size_t count,
                         void *data, size_t sizeBytes) {
    const size_t eSize = alloc->mHal.state.type->getElementSizeBytes();
    const uint8_t * ptr = GetOffsetPtr(alloc, xoff, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
    if (data != ptr) {
        // Skip the copy if we are the same allocation. This can arise from
        // our Bitmap optimization, where we share the same storage.
        memcpy(data, ptr, count * eSize);
    }
}

void rsdAllocationRead2D(const Context *rsc, const Allocation *alloc,
                                uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                                uint32_t w, uint32_t h, void *data, size_t sizeBytes, size_t stride) {
    size_t eSize = alloc->mHal.state.elementSizeBytes;
    size_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    }

    if (alloc->mHal.drvState.lod[0].mallocPtr) {
        uint8_t *dst = static_cast<uint8_t *>(data);
        const uint8_t *src = GetOffsetPtr(alloc, xoff, yoff, 0, lod, face);
        if (dst == src) {
            // Skip the copy if we are the same allocation. This can arise from
            // our Bitmap optimization, where we share the same storage.
            return;
        }

        for (uint32_t line=yoff; line < (yoff+h); line++) {
            memcpy(dst, src, lineSize);
            dst += stride;
            src += alloc->mHal.drvState.lod[lod].stride;
        }
    } else {
        ALOGE("Add code to readback from non-script memory");
    }
}


void rsdAllocationRead3D(const Context *rsc, const Allocation *alloc,
                         uint32_t xoff, uint32_t yoff, uint32_t zoff,
                         uint32_t lod,
                         uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes, size_t stride) {
    uint32_t eSize = alloc->mHal.state.elementSizeBytes;
    uint32_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    }

    if (alloc->mHal.drvState.lod[0].mallocPtr) {
        uint8_t *dst = static_cast<uint8_t *>(data);
        for (uint32_t z = zoff; z < (d + zoff); z++) {
            const uint8_t *src = GetOffsetPtr(alloc, xoff, yoff, z, lod,
                                              RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
            if (dst == src) {
                // Skip the copy if we are the same allocation. This can arise from
                // our Bitmap optimization, where we share the same storage.
                return;
            }

            for (uint32_t line=yoff; line < (yoff+h); line++) {
                memcpy(dst, src, lineSize);
                dst += stride;
                src += alloc->mHal.drvState.lod[lod].stride;
            }
        }
    }
}

void * rsdAllocationLock1D(const android::renderscript::Context *rsc,
                          const android::renderscript::Allocation *alloc) {
    return alloc->mHal.drvState.lod[0].mallocPtr;
}

void rsdAllocationUnlock1D(const android::renderscript::Context *rsc,
                          const android::renderscript::Allocation *alloc) {

}

void rsdAllocationData1D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstLod, size_t count,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcLod) {
}


void rsdAllocationData2D_alloc_script(const android::renderscript::Context *rsc,
                                      const android::renderscript::Allocation *dstAlloc,
                                      uint32_t dstXoff, uint32_t dstYoff, uint32_t dstLod,
                                      RsAllocationCubemapFace dstFace, uint32_t w, uint32_t h,
                                      const android::renderscript::Allocation *srcAlloc,
                                      uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
                                      RsAllocationCubemapFace srcFace) {
    size_t elementSize = dstAlloc->getType()->getElementSizeBytes();
    for (uint32_t i = 0; i < h; i ++) {
        uint8_t *dstPtr = GetOffsetPtr(dstAlloc, dstXoff, dstYoff + i, 0, dstLod, dstFace);
        uint8_t *srcPtr = GetOffsetPtr(srcAlloc, srcXoff, srcYoff + i, 0, srcLod, srcFace);
        memcpy(dstPtr, srcPtr, w * elementSize);

        //ALOGE("COPIED dstXoff(%u), dstYoff(%u), dstLod(%u), dstFace(%u), w(%u), h(%u), srcXoff(%u), srcYoff(%u), srcLod(%u), srcFace(%u)",
        //     dstXoff, dstYoff, dstLod, dstFace, w, h, srcXoff, srcYoff, srcLod, srcFace);
    }
}

void rsdAllocationData3D_alloc_script(const android::renderscript::Context *rsc,
                                      const android::renderscript::Allocation *dstAlloc,
                                      uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff, uint32_t dstLod,
                                      uint32_t w, uint32_t h, uint32_t d,
                                      const android::renderscript::Allocation *srcAlloc,
                                      uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff, uint32_t srcLod) {
    uint32_t elementSize = dstAlloc->getType()->getElementSizeBytes();
    for (uint32_t j = 0; j < d; j++) {
        for (uint32_t i = 0; i < h; i ++) {
            uint8_t *dstPtr = GetOffsetPtr(dstAlloc, dstXoff, dstYoff + i, dstZoff + j,
                                           dstLod, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
            uint8_t *srcPtr = GetOffsetPtr(srcAlloc, srcXoff, srcYoff + i, srcZoff + j,
                                           srcLod, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
            memcpy(dstPtr, srcPtr, w * elementSize);

            //ALOGE("COPIED dstXoff(%u), dstYoff(%u), dstLod(%u), dstFace(%u), w(%u), h(%u), srcXoff(%u), srcYoff(%u), srcLod(%u), srcFace(%u)",
            //     dstXoff, dstYoff, dstLod, dstFace, w, h, srcXoff, srcYoff, srcLod, srcFace);
        }
    }
}

void rsdAllocationData2D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff, uint32_t dstLod,
                               RsAllocationCubemapFace dstFace, uint32_t w, uint32_t h,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
                               RsAllocationCubemapFace srcFace) {
    if (!dstAlloc->getIsScript() && !srcAlloc->getIsScript()) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Non-script allocation copies not "
                                             "yet implemented.");
        return;
    }
    rsdAllocationData2D_alloc_script(rsc, dstAlloc, dstXoff, dstYoff,
                                     dstLod, dstFace, w, h, srcAlloc,
                                     srcXoff, srcYoff, srcLod, srcFace);
}

void rsdAllocationData3D_alloc(const android::renderscript::Context *rsc,
                               const android::renderscript::Allocation *dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                               uint32_t dstLod,
                               uint32_t w, uint32_t h, uint32_t d,
                               const android::renderscript::Allocation *srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                               uint32_t srcLod) {
    if (!dstAlloc->getIsScript() && !srcAlloc->getIsScript()) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Non-script allocation copies not "
                                             "yet implemented.");
        return;
    }
    rsdAllocationData3D_alloc_script(rsc, dstAlloc, dstXoff, dstYoff, dstZoff,
                                     dstLod, w, h, d, srcAlloc,
                                     srcXoff, srcYoff, srcZoff, srcLod);
}

void rsdAllocationElementData(const Context *rsc, const Allocation *alloc,
                              uint32_t x, uint32_t y, uint32_t z,
                              const void *data, uint32_t cIdx, size_t sizeBytes) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    uint8_t * ptr = GetOffsetPtr(alloc, x, y, z, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);

    const Element * e = alloc->mHal.state.type->getElement()->getField(cIdx);
    ptr += alloc->mHal.state.type->getElement()->getFieldOffsetBytes(cIdx);

    if (alloc->mHal.state.hasReferences) {
        e->incRefs(data);
        e->decRefs(ptr);
    }

    memcpy(ptr, data, sizeBytes);
    drv->uploadDeferred = true;
}

void rsdAllocationElementRead(const Context *rsc, const Allocation *alloc,
                              uint32_t x, uint32_t y, uint32_t z,
                              void *data, uint32_t cIdx, size_t sizeBytes) {
    DrvAllocation *drv = (DrvAllocation *)alloc->mHal.drv;

    uint8_t * ptr = GetOffsetPtr(alloc, x, y, z, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);

    const Element * e = alloc->mHal.state.type->getElement()->getField(cIdx);
    ptr += alloc->mHal.state.type->getElement()->getFieldOffsetBytes(cIdx);

    memcpy(data, ptr, sizeBytes);
}

static void mip565(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
    uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
    uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

    for (uint32_t y=0; y < h; y++) {
        uint16_t *oPtr = (uint16_t *)GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
        const uint16_t *i1 = (uint16_t *)GetOffsetPtr(alloc, 0, 0, y*2, lod, face);
        const uint16_t *i2 = (uint16_t *)GetOffsetPtr(alloc, 0, 0, y*2+1, lod, face);

        for (uint32_t x=0; x < w; x++) {
            *oPtr = rsBoxFilter565(i1[0], i1[1], i2[0], i2[1]);
            oPtr ++;
            i1 += 2;
            i2 += 2;
        }
    }
}

static void mip8888(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
    uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
    uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

    for (uint32_t y=0; y < h; y++) {
        uint32_t *oPtr = (uint32_t *)GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
        const uint32_t *i1 = (uint32_t *)GetOffsetPtr(alloc, 0, y*2, 0, lod, face);
        const uint32_t *i2 = (uint32_t *)GetOffsetPtr(alloc, 0, y*2+1, 0, lod, face);

        for (uint32_t x=0; x < w; x++) {
            *oPtr = rsBoxFilter8888(i1[0], i1[1], i2[0], i2[1]);
            oPtr ++;
            i1 += 2;
            i2 += 2;
        }
    }
}

static void mip8(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
    uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
    uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

    for (uint32_t y=0; y < h; y++) {
        uint8_t *oPtr = GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
        const uint8_t *i1 = GetOffsetPtr(alloc, 0, y*2, 0, lod, face);
        const uint8_t *i2 = GetOffsetPtr(alloc, 0, y*2+1, 0, lod, face);

        for (uint32_t x=0; x < w; x++) {
            *oPtr = (uint8_t)(((uint32_t)i1[0] + i1[1] + i2[0] + i2[1]) * 0.25f);
            oPtr ++;
            i1 += 2;
            i2 += 2;
        }
    }
}

void rsdAllocationGenerateMipmaps(const Context *rsc, const Allocation *alloc) {
    if(!alloc->mHal.drvState.lod[0].mallocPtr) {
        return;
    }
    uint32_t numFaces = alloc->getType()->getDimFaces() ? 6 : 1;
    for (uint32_t face = 0; face < numFaces; face ++) {
        for (uint32_t lod=0; lod < (alloc->getType()->getLODCount() -1); lod++) {
            switch (alloc->getType()->getElement()->getSizeBits()) {
            case 32:
                mip8888(alloc, lod, (RsAllocationCubemapFace)face);
                break;
            case 16:
                mip565(alloc, lod, (RsAllocationCubemapFace)face);
                break;
            case 8:
                mip8(alloc, lod, (RsAllocationCubemapFace)face);
                break;
            }
        }
    }
}

uint32_t rsdAllocationGrallocBits(const android::renderscript::Context *rsc,
                                  android::renderscript::Allocation *alloc)
{
    return 0;
}

void rsdAllocationUpdateCachedObject(const Context *rsc,
                                     const Allocation *alloc,
                                     rs_allocation *obj)
{
    obj->p = alloc;
#ifdef __LP64__
    obj->unused1 = nullptr;
    obj->unused2 = nullptr;
    obj->unused3 = nullptr;
#endif
}
