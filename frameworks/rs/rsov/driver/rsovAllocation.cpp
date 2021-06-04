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

#include "rsovAllocation.h"

#include "rsAllocation.h"
#include "rsContext.h"
#include "rsCppUtils.h"
#include "rsElement.h"
#include "rsType.h"
#include "rsovContext.h"
#include "rsovCore.h"

namespace android {
namespace renderscript {
namespace rsov {

namespace {

size_t DeriveYUVLayout(int yuv, Allocation::Hal::DrvState *state) {
  // For the flexible YCbCr format, layout is initialized during call to
  // Allocation::ioReceive.  Return early and avoid clobberring any
  // pre-existing layout.
  if (yuv == HAL_PIXEL_FORMAT_YCbCr_420_888) {
    return 0;
  }

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

  switch (yuv) {
    case HAL_PIXEL_FORMAT_YV12:
      state->lod[2].stride = rsRound(state->lod[0].stride >> 1, 16);
      state->lod[2].mallocPtr = ((uint8_t *)state->lod[0].mallocPtr) +
                                (state->lod[0].stride * state->lod[0].dimY);
      uvSize += state->lod[2].stride * state->lod[2].dimY;

      state->lod[1].stride = state->lod[2].stride;
      state->lod[1].mallocPtr = ((uint8_t *)state->lod[2].mallocPtr) +
                                (state->lod[2].stride * state->lod[2].dimY);
      uvSize += state->lod[1].stride * state->lod[2].dimY;
      break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:  // NV21
      // state->lod[1].dimX = state->lod[0].dimX;
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

// TODO: Dedup this with the same code under frameworks/rs/driver
size_t AllocationBuildPointerTable(const Context *rsc, const Allocation *alloc,
                                   const Type *type, uint8_t *ptr,
                                   size_t requiredAlignment) {
  alloc->mHal.drvState.lod[0].dimX = type->getDimX();
  alloc->mHal.drvState.lod[0].dimY = type->getDimY();
  alloc->mHal.drvState.lod[0].dimZ = type->getDimZ();
  alloc->mHal.drvState.lod[0].mallocPtr = 0;
  // Stride needs to be aligned to a boundary defined by requiredAlignment!
  size_t stride =
      alloc->mHal.drvState.lod[0].dimX * type->getElementSizeBytes();
  alloc->mHal.drvState.lod[0].stride = rsRound(stride, requiredAlignment);
  alloc->mHal.drvState.lodCount = type->getLODCount();
  alloc->mHal.drvState.faceCount = type->getDimFaces();

  size_t offsets[Allocation::MAX_LOD];
  memset(offsets, 0, sizeof(offsets));

  size_t o = alloc->mHal.drvState.lod[0].stride *
             rsMax(alloc->mHal.drvState.lod[0].dimY, 1u) *
             rsMax(alloc->mHal.drvState.lod[0].dimZ, 1u);
  if (alloc->mHal.state.yuv) {
    o += DeriveYUVLayout(alloc->mHal.state.yuv, &alloc->mHal.drvState);

    for (uint32_t ct = 1; ct < alloc->mHal.drvState.lodCount; ct++) {
      offsets[ct] = (size_t)alloc->mHal.drvState.lod[ct].mallocPtr;
    }
  } else if (alloc->mHal.drvState.lodCount > 1) {
    uint32_t tx = alloc->mHal.drvState.lod[0].dimX;
    uint32_t ty = alloc->mHal.drvState.lod[0].dimY;
    uint32_t tz = alloc->mHal.drvState.lod[0].dimZ;
    for (uint32_t lod = 1; lod < alloc->mHal.drvState.lodCount; lod++) {
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
  for (uint32_t lod = 1; lod < alloc->mHal.drvState.lodCount; lod++) {
    alloc->mHal.drvState.lod[lod].mallocPtr = ptr + offsets[lod];
  }

  size_t allocSize = alloc->mHal.drvState.faceOffset;
  if (alloc->mHal.drvState.faceCount) {
    allocSize *= 6;
  }

  return allocSize;
}

size_t AllocationBuildPointerTable(const Context *rsc, const Allocation *alloc,
                                   const Type *type, uint8_t *ptr) {
  return AllocationBuildPointerTable(rsc, alloc, type, ptr,
                                     Allocation::kMinimumRSAlignment);
}

uint8_t *GetOffsetPtr(const Allocation *alloc, uint32_t xoff, uint32_t yoff,
                      uint32_t zoff, uint32_t lod,
                      RsAllocationCubemapFace face) {
  uint8_t *ptr = (uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;
  ptr += face * alloc->mHal.drvState.faceOffset;
  ptr += zoff * alloc->mHal.drvState.lod[lod].dimY *
         alloc->mHal.drvState.lod[lod].stride;
  ptr += yoff * alloc->mHal.drvState.lod[lod].stride;
  ptr += xoff * alloc->mHal.state.elementSizeBytes;
  return ptr;
}

void mip565(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
  uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
  uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

  for (uint32_t y = 0; y < h; y++) {
    uint16_t *oPtr = (uint16_t *)GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
    const uint16_t *i1 =
        (uint16_t *)GetOffsetPtr(alloc, 0, 0, y * 2, lod, face);
    const uint16_t *i2 =
        (uint16_t *)GetOffsetPtr(alloc, 0, 0, y * 2 + 1, lod, face);

    for (uint32_t x = 0; x < w; x++) {
      *oPtr = rsBoxFilter565(i1[0], i1[1], i2[0], i2[1]);
      oPtr++;
      i1 += 2;
      i2 += 2;
    }
  }
}

void mip8888(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
  uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
  uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

  for (uint32_t y = 0; y < h; y++) {
    uint32_t *oPtr = (uint32_t *)GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
    const uint32_t *i1 =
        (uint32_t *)GetOffsetPtr(alloc, 0, y * 2, 0, lod, face);
    const uint32_t *i2 =
        (uint32_t *)GetOffsetPtr(alloc, 0, y * 2 + 1, 0, lod, face);

    for (uint32_t x = 0; x < w; x++) {
      *oPtr = rsBoxFilter8888(i1[0], i1[1], i2[0], i2[1]);
      oPtr++;
      i1 += 2;
      i2 += 2;
    }
  }
}

void mip8(const Allocation *alloc, int lod, RsAllocationCubemapFace face) {
  uint32_t w = alloc->mHal.drvState.lod[lod + 1].dimX;
  uint32_t h = alloc->mHal.drvState.lod[lod + 1].dimY;

  for (uint32_t y = 0; y < h; y++) {
    uint8_t *oPtr = GetOffsetPtr(alloc, 0, y, 0, lod + 1, face);
    const uint8_t *i1 = GetOffsetPtr(alloc, 0, y * 2, 0, lod, face);
    const uint8_t *i2 = GetOffsetPtr(alloc, 0, y * 2 + 1, 0, lod, face);

    for (uint32_t x = 0; x < w; x++) {
      *oPtr = (uint8_t)(((uint32_t)i1[0] + i1[1] + i2[0] + i2[1]) * 0.25f);
      oPtr++;
      i1 += 2;
      i2 += 2;
    }
  }
}

}  // anonymous namespace

RSoVAllocation::RSoVAllocation(RSoVContext *context, const Type *type,
                               size_t bufferSize)
    : mBuffer(new RSoVBuffer(context, bufferSize)),
      mType(type),
      mWidth(type->getDimX()),
      mHeight(type->getDimY()),
      mDepth(type->getDimZ()) {}

RSoVBuffer::RSoVBuffer(RSoVContext *context, size_t size)
    : mRSoV(context), mDevice(context->getDevice()) {
  InitBuffer(size);
}

RSoVBuffer::~RSoVBuffer() {
  vkUnmapMemory(mDevice, mMem);
  vkDestroyBuffer(mDevice, mBuf, nullptr);
  vkFreeMemory(mDevice, mMem, nullptr);
}

void RSoVBuffer::InitBuffer(size_t bufferSize) {
  VkResult res;

  VkBufferCreateInfo buf_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .size = bufferSize,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .flags = 0,
  };
  res = vkCreateBuffer(mDevice, &buf_info, nullptr, &mBuf);
  rsAssert(res == VK_SUCCESS);

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(mDevice, mBuf, &mem_reqs);

  VkMemoryAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .memoryTypeIndex = 0,
      .allocationSize = mem_reqs.size,
  };

  bool pass;
  pass =
      mRSoV->MemoryTypeFromProperties(mem_reqs.memoryTypeBits,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      &allocateInfo.memoryTypeIndex);
  rsAssert(pass);

  // TODO: Make this aligned
  res = vkAllocateMemory(mDevice, &allocateInfo, nullptr, &mMem);
  rsAssert(res == VK_SUCCESS);

  res = vkBindBufferMemory(mDevice, mBuf, mMem, 0);
  rsAssert(res == VK_SUCCESS);

  mBufferInfo.buffer = mBuf;
  mBufferInfo.offset = 0;
  mBufferInfo.range = bufferSize;

  res = vkMapMemory(mDevice, mMem, 0, mem_reqs.size, 0, (void **)&mPtr);
  rsAssert(res == VK_SUCCESS);
}

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::Element;
using android::renderscript::Type;
using android::renderscript::rs_allocation;
using android::renderscript::rsMax;
using namespace android::renderscript::rsov;

bool rsovAllocationInit(const Context *rsc, Allocation *alloc, bool forceZero) {
  RSoVHal *hal = static_cast<RSoVHal *>(rsc->mHal.drv);
  RSoVContext *rsov = hal->mRSoV;
  const Type *type = alloc->getType();

  // Calculate the object size.
  size_t allocSize = AllocationBuildPointerTable(rsc, alloc, type, nullptr);
  RSoVAllocation *rsovAlloc = new RSoVAllocation(rsov, type, allocSize);
  alloc->mHal.drv = rsovAlloc;
  AllocationBuildPointerTable(rsc, alloc, type,
                              (uint8_t *)rsovAlloc->getHostPtr());
  return true;
}

void rsovAllocationDestroy(const Context *rsc, Allocation *alloc) {
  RSoVAllocation *rsovAlloc = static_cast<RSoVAllocation *>(alloc->mHal.drv);
  delete rsovAlloc;
  alloc->mHal.drv = nullptr;
}

void rsovAllocationData1D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t lod, size_t count,
                          const void *data, size_t sizeBytes) {
  const size_t eSize = alloc->mHal.state.type->getElementSizeBytes();
  uint8_t *ptr =
      GetOffsetPtr(alloc, xoff, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
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
}

void rsovAllocationData2D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t yoff, uint32_t lod,
                          RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                          const void *data, size_t sizeBytes, size_t stride) {
  size_t eSize = alloc->mHal.state.elementSizeBytes;
  size_t lineSize = eSize * w;
  if (!stride) {
    stride = lineSize;
  }

  if (alloc->mHal.drvState.lod[0].mallocPtr) {
    const uint8_t *src = static_cast<const uint8_t *>(data);
    uint8_t *dst = GetOffsetPtr(alloc, xoff, yoff, 0, lod, face);

    for (uint32_t line = yoff; line < (yoff + h); line++) {
      if (alloc->mHal.state.hasReferences) {
        alloc->incRefs(src, w);
        alloc->decRefs(dst, w);
      }
      memcpy(dst, src, lineSize);
      src += stride;
      dst += alloc->mHal.drvState.lod[lod].stride;
    }
    // TODO: handle YUV Allocations
    if (alloc->mHal.state.yuv) {
      size_t clineSize = lineSize;
      int lod = 1;
      int maxLod = 2;
      if (alloc->mHal.state.yuv == HAL_PIXEL_FORMAT_YV12) {
        maxLod = 3;
        clineSize >>= 1;
      } else if (alloc->mHal.state.yuv == HAL_PIXEL_FORMAT_YCrCb_420_SP) {
        lod = 2;
        maxLod = 3;
      }

      while (lod < maxLod) {
        uint8_t *dst = GetOffsetPtr(alloc, xoff, yoff, 0, lod, face);

        for (uint32_t line = (yoff >> 1); line < ((yoff + h) >> 1); line++) {
          memcpy(dst, src, clineSize);
          // When copying from an array to an Allocation, the src pointer
          // to the array should just move by the number of bytes copied.
          src += clineSize;
          dst += alloc->mHal.drvState.lod[lod].stride;
        }
        lod++;
      }
    }
  }
}

void rsovAllocationData3D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t yoff, uint32_t zoff,
                          uint32_t lod, uint32_t w, uint32_t h, uint32_t d,
                          const void *data, size_t sizeBytes, size_t stride) {
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
      for (uint32_t line = yoff; line < (yoff + h); line++) {
        if (alloc->mHal.state.hasReferences) {
          alloc->incRefs(src, w);
          alloc->decRefs(dst, w);
        }
        memcpy(dst, src, lineSize);
        src += stride;
        dst += alloc->mHal.drvState.lod[lod].stride;
      }
    }
  }
}

void rsovAllocationRead1D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t lod, size_t count, void *data,
                          size_t sizeBytes) {
  const size_t eSize = alloc->mHal.state.type->getElementSizeBytes();
  const uint8_t *ptr =
      GetOffsetPtr(alloc, xoff, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
  if (data != ptr) {
    // Skip the copy if we are the same allocation. This can arise from
    // our Bitmap optimization, where we share the same storage.
    memcpy(data, ptr, count * eSize);
  }
}

void rsovAllocationRead2D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t yoff, uint32_t lod,
                          RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                          void *data, size_t sizeBytes, size_t stride) {
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

    for (uint32_t line = yoff; line < (yoff + h); line++) {
      memcpy(dst, src, lineSize);
      dst += stride;
      src += alloc->mHal.drvState.lod[lod].stride;
    }
  } else {
    ALOGE("Add code to readback from non-script memory");
  }
}

void rsovAllocationRead3D(const Context *rsc, const Allocation *alloc,
                          uint32_t xoff, uint32_t yoff, uint32_t zoff,
                          uint32_t lod, uint32_t w, uint32_t h, uint32_t d,
                          void *data, size_t sizeBytes, size_t stride) {
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

      for (uint32_t line = yoff; line < (yoff + h); line++) {
        memcpy(dst, src, lineSize);
        dst += stride;
        src += alloc->mHal.drvState.lod[lod].stride;
      }
    }
  }
}

void *rsovAllocationLock1D(const Context *rsc, const Allocation *alloc) {
  return alloc->mHal.drvState.lod[0].mallocPtr;
}

void rsovAllocationUnlock1D(const Context *rsc, const Allocation *alloc) {}

void rsovAllocationData1D_alloc(const Context *rsc, const Allocation *dstAlloc,
                                uint32_t dstXoff, uint32_t dstLod, size_t count,
                                const Allocation *srcAlloc, uint32_t srcXoff,
                                uint32_t srcLod) {}

void rsovAllocationData2D_alloc_script(
    const Context *rsc, const Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstLod, RsAllocationCubemapFace dstFace,
    uint32_t w, uint32_t h, const Allocation *srcAlloc, uint32_t srcXoff,
    uint32_t srcYoff, uint32_t srcLod, RsAllocationCubemapFace srcFace) {
  size_t elementSize = dstAlloc->getType()->getElementSizeBytes();
  for (uint32_t i = 0; i < h; i++) {
    uint8_t *dstPtr =
        GetOffsetPtr(dstAlloc, dstXoff, dstYoff + i, 0, dstLod, dstFace);
    uint8_t *srcPtr =
        GetOffsetPtr(srcAlloc, srcXoff, srcYoff + i, 0, srcLod, srcFace);
    memcpy(dstPtr, srcPtr, w * elementSize);
  }
}

void rsovAllocationData3D_alloc_script(
    const Context *rsc, const Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstZoff, uint32_t dstLod, uint32_t w, uint32_t h,
    uint32_t d, const Allocation *srcAlloc, uint32_t srcXoff, uint32_t srcYoff,
    uint32_t srcZoff, uint32_t srcLod) {
  uint32_t elementSize = dstAlloc->getType()->getElementSizeBytes();
  for (uint32_t j = 0; j < d; j++) {
    for (uint32_t i = 0; i < h; i++) {
      uint8_t *dstPtr =
          GetOffsetPtr(dstAlloc, dstXoff, dstYoff + i, dstZoff + j, dstLod,
                       RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
      uint8_t *srcPtr =
          GetOffsetPtr(srcAlloc, srcXoff, srcYoff + i, srcZoff + j, srcLod,
                       RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
      memcpy(dstPtr, srcPtr, w * elementSize);
    }
  }
}

void rsovAllocationData2D_alloc(
    const Context *rsc, const Allocation *dstAlloc, uint32_t dstXoff,
    uint32_t dstYoff, uint32_t dstLod, RsAllocationCubemapFace dstFace,
    uint32_t w, uint32_t h, const Allocation *srcAlloc, uint32_t srcXoff,
    uint32_t srcYoff, uint32_t srcLod, RsAllocationCubemapFace srcFace) {
  if (!dstAlloc->getIsScript() && !srcAlloc->getIsScript()) {
    rsc->setError(RS_ERROR_FATAL_DRIVER,
                  "Non-script allocation copies not "
                  "yet implemented.");
    return;
  }
  rsovAllocationData2D_alloc_script(rsc, dstAlloc, dstXoff, dstYoff, dstLod,
                                    dstFace, w, h, srcAlloc, srcXoff, srcYoff,
                                    srcLod, srcFace);
}

void rsovAllocationData3D_alloc(const Context *rsc, const Allocation *dstAlloc,
                                uint32_t dstXoff, uint32_t dstYoff,
                                uint32_t dstZoff, uint32_t dstLod, uint32_t w,
                                uint32_t h, uint32_t d,
                                const Allocation *srcAlloc, uint32_t srcXoff,
                                uint32_t srcYoff, uint32_t srcZoff,
                                uint32_t srcLod) {
  if (!dstAlloc->getIsScript() && !srcAlloc->getIsScript()) {
    rsc->setError(RS_ERROR_FATAL_DRIVER,
                  "Non-script allocation copies not "
                  "yet implemented.");
    return;
  }
  rsovAllocationData3D_alloc_script(rsc, dstAlloc, dstXoff, dstYoff, dstZoff,
                                    dstLod, w, h, d, srcAlloc, srcXoff, srcYoff,
                                    srcZoff, srcLod);
}

void rsovAllocationAdapterOffset(const Context *rsc, const Allocation *alloc) {
  // Get a base pointer to the new LOD
  const Allocation *base = alloc->mHal.state.baseAlloc;
  const Type *type = alloc->mHal.state.type;
  if (base == nullptr) {
    return;
  }

  const int lodBias = alloc->mHal.state.originLOD;
  uint32_t lodCount = rsMax(alloc->mHal.drvState.lodCount, (uint32_t)1);
  for (uint32_t lod = 0; lod < lodCount; lod++) {
    alloc->mHal.drvState.lod[lod] = base->mHal.drvState.lod[lod + lodBias];
    alloc->mHal.drvState.lod[lod].mallocPtr = GetOffsetPtr(
        alloc, alloc->mHal.state.originX, alloc->mHal.state.originY,
        alloc->mHal.state.originZ, lodBias,
        (RsAllocationCubemapFace)alloc->mHal.state.originFace);
  }
}

bool rsovAllocationAdapterInit(const Context *rsc, Allocation *alloc) {
// TODO: may need a RSoV Allocation here
#if 0
    DrvAllocation *drv = (DrvAllocation *)calloc(1, sizeof(DrvAllocation));
    if (!drv) {
        return false;
    }
    alloc->mHal.drv = drv;
#endif
  // We need to build an allocation that looks like a subset of the parent
  // allocation
  rsovAllocationAdapterOffset(rsc, alloc);

  return true;
}

void rsovAllocationSyncAll(const Context *rsc, const Allocation *alloc,
                           RsAllocationUsageType src) {
  // TODO: anything to do here?
}

void rsovAllocationMarkDirty(const Context *rsc, const Allocation *alloc) {
  // TODO: anything to do here?
}

void rsovAllocationResize(const Context *rsc, const Allocation *alloc,
                          const Type *newType, bool zeroNew) {
  // TODO: implement this
  // can this be done without copying, if the new size is greater than the
  // original?
}

void rsovAllocationGenerateMipmaps(const Context *rsc,
                                   const Allocation *alloc) {
  if (!alloc->mHal.drvState.lod[0].mallocPtr) {
    return;
  }
  uint32_t numFaces = alloc->getType()->getDimFaces() ? 6 : 1;
  for (uint32_t face = 0; face < numFaces; face++) {
    for (uint32_t lod = 0; lod < (alloc->getType()->getLODCount() - 1); lod++) {
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

uint32_t rsovAllocationGrallocBits(const Context *rsc, Allocation *alloc) {
  return 0;
}

void rsovAllocationUpdateCachedObject(const Context *rsc,
                                      const Allocation *alloc,
                                      rs_allocation *obj) {
  obj->p = alloc;
#ifdef __LP64__
  obj->unused1 = nullptr;
  obj->unused2 = nullptr;
  obj->unused3 = nullptr;
#endif
}

void rsovAllocationSetSurface(const Context *rsc, Allocation *alloc,
                              ANativeWindow *nw) {
  // TODO: implement this
}

void rsovAllocationIoSend(const Context *rsc, Allocation *alloc) {
  // TODO: implement this
}

void rsovAllocationIoReceive(const Context *rsc, Allocation *alloc) {
  // TODO: implement this
}

void rsovAllocationElementData(const Context *rsc, const Allocation *alloc,
                               uint32_t x, uint32_t y, uint32_t z,
                               const void *data, uint32_t cIdx,
                               size_t sizeBytes) {
  uint8_t *ptr =
      GetOffsetPtr(alloc, x, y, z, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);

  const Element *e = alloc->mHal.state.type->getElement()->getField(cIdx);
  ptr += alloc->mHal.state.type->getElement()->getFieldOffsetBytes(cIdx);

  if (alloc->mHal.state.hasReferences) {
    e->incRefs(data);
    e->decRefs(ptr);
  }

  memcpy(ptr, data, sizeBytes);
}

void rsovAllocationElementRead(const Context *rsc, const Allocation *alloc,
                               uint32_t x, uint32_t y, uint32_t z, void *data,
                               uint32_t cIdx, size_t sizeBytes) {
  uint8_t *ptr =
      GetOffsetPtr(alloc, x, y, z, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);

  const Element *e = alloc->mHal.state.type->getElement()->getField(cIdx);
  ptr += alloc->mHal.state.type->getElement()->getFieldOffsetBytes(cIdx);

  memcpy(data, ptr, sizeBytes);
}
