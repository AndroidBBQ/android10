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

#ifndef RSOV_ALLOCATION_H
#define RSOV_ALLOCATION_H

#include <vulkan/vulkan.h>

#include "rsDefines.h"
#include "rs_hal.h"
#include "system/window.h"

namespace android {
namespace renderscript {

class Allocation;
class Context;
class Type;

namespace rsov {

class RSoVContext;
// Abstraction for a Vulkan Buffer
class RSoVBuffer {
 public:
  RSoVBuffer(RSoVContext *context, size_t bufferSize);
  ~RSoVBuffer();

  const VkDescriptorBufferInfo *getBufferInfo() const { return &mBufferInfo; }
  char *getHostPtr() const { return mPtr; }

 private:
  void InitBuffer(size_t);

  char *mPtr;  // Host pointer to mmapped device memory for the Buffer
  RSoVContext *mRSoV;
  VkDevice mDevice;

  VkDeviceMemory mMem;
  VkBuffer mBuf;
  VkDescriptorBufferInfo mBufferInfo;
};

class RSoVAllocation {
 public:
  RSoVAllocation(RSoVContext *context, const Type *type, size_t bufferSize);
  ~RSoVAllocation() { delete mBuffer; }

  const Type *getType() const { return mType; }
  uint32_t getWidth() const { return mWidth; }
  uint32_t getHeight() const { return mHeight; }
  uint32_t getDepth() const { return mDepth; }
  RSoVBuffer *getBuffer() const { return mBuffer; }
  char *getHostPtr() const { return mBuffer->getHostPtr(); }

 private:
  RSoVBuffer *mBuffer;
  const Type *mType;
  const uint32_t mWidth;
  const uint32_t mHeight;
  const uint32_t mDepth;
};

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

extern bool rsovAllocationInit(const android::renderscript::Context *rsc,
                               android::renderscript::Allocation *alloc,
                               bool forceZero);

extern void rsovAllocationDestroy(const android::renderscript::Context *rsc,
                                  android::renderscript::Allocation *alloc);

extern void rsovAllocationData1D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t lod, size_t count,
                                 const void *data, size_t sizeBytes);

extern void rsovAllocationData2D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t yoff, uint32_t lod,
                                 RsAllocationCubemapFace face, uint32_t w,
                                 uint32_t h, const void *data, size_t sizeBytes,
                                 size_t stride);

extern void rsovAllocationData3D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t yoff, uint32_t zoff,
                                 uint32_t lod, uint32_t w, uint32_t h,
                                 uint32_t d, const void *data, size_t sizeBytes,
                                 size_t stride);

extern void rsovAllocationRead1D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t lod, size_t count,
                                 void *data, size_t sizeBytes);

extern void rsovAllocationRead2D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t yoff, uint32_t lod,
                                 RsAllocationCubemapFace face, uint32_t w,
                                 uint32_t h, void *data, size_t sizeBytes,
                                 size_t stride);

extern void rsovAllocationRead3D(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 uint32_t xoff, uint32_t yoff, uint32_t zoff,
                                 uint32_t lod, uint32_t w, uint32_t h,
                                 uint32_t d, void *data, size_t sizeBytes,
                                 size_t stride);

extern void *rsovAllocationLock1D(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc);

extern void rsovAllocationUnlock1D(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc);

extern void rsovAllocationData1D_alloc(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstLod, size_t count,
    const android::renderscript::Allocation *srcAlloc, uint32_t srcXoff,
    uint32_t srcLod);

extern void rsovAllocationData2D_alloc_script(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstLod, RsAllocationCubemapFace dstFace,
    uint32_t w, uint32_t h, const android::renderscript::Allocation *srcAlloc,
    uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
    RsAllocationCubemapFace srcFace);

extern void rsovAllocationData2D_alloc(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstLod, RsAllocationCubemapFace dstFace,
    uint32_t w, uint32_t h, const android::renderscript::Allocation *srcAlloc,
    uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
    RsAllocationCubemapFace srcFace);

extern void rsovAllocationData3D_alloc_script(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstZoff, uint32_t dstLod, uint32_t w, uint32_t h,
    uint32_t d, const android::renderscript::Allocation *srcAlloc,
    uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff, uint32_t srcLod);

extern void rsovAllocationData3D_alloc(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstZoff, uint32_t dstLod, uint32_t w, uint32_t h,
    uint32_t d, const android::renderscript::Allocation *srcAlloc,
    uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff, uint32_t srcLod);

extern void rsovAllocationAdapterOffset(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc);

extern bool rsovAllocationAdapterInit(const android::renderscript::Context *rsc,
                                      android::renderscript::Allocation *alloc);

extern void rsovAllocationSyncAll(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc, RsAllocationUsageType src);

extern void rsovAllocationMarkDirty(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc);

extern void rsovAllocationResize(const android::renderscript::Context *rsc,
                                 const android::renderscript::Allocation *alloc,
                                 const android::renderscript::Type *newType,
                                 bool zeroNew);

extern void rsovAllocationGenerateMipmaps(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc);

extern uint32_t rsovAllocationGrallocBits(
    const android::renderscript::Context *rsc,
    android::renderscript::Allocation *alloc);

extern void rsovAllocationUpdateCachedObject(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc,
    android::renderscript::rs_allocation *obj);

extern void rsovAllocationSetSurface(const android::renderscript::Context *rsc,
                                     android::renderscript::Allocation *alloc,
                                     ANativeWindow *nw);

extern void rsovAllocationIoSend(const android::renderscript::Context *rsc,
                                 android::renderscript::Allocation *alloc);

extern void rsovAllocationIoReceive(const android::renderscript::Context *rsc,
                                    android::renderscript::Allocation *alloc);

extern void rsovAllocationElementData(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc, uint32_t x, uint32_t y,
    uint32_t z, const void *data, uint32_t cIdx, size_t sizeBytes);

extern void rsovAllocationElementRead(
    const android::renderscript::Context *rsc,
    const android::renderscript::Allocation *alloc, uint32_t x, uint32_t y,
    uint32_t z, void *data, uint32_t cIdx, size_t sizeBytes);

#endif  // RSOV_ALLOCATION_H
