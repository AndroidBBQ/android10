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

//#define LOG_NDEBUG 0
#define LOG_TAG "C2AllocatorGralloc"
#include <utils/Log.h>

#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>
#include <android/hardware/graphics/allocator/3.0/IAllocator.h>
#include <android/hardware/graphics/mapper/3.0/IMapper.h>
#include <cutils/native_handle.h>
#include <hardware/gralloc.h>

#include <C2AllocatorGralloc.h>
#include <C2Buffer.h>
#include <C2PlatformSupport.h>

namespace android {

namespace /* unnamed */ {
    enum : uint64_t {
        /**
         * Usage mask that is passed through from gralloc to Codec 2.0 usage.
         */
        PASSTHROUGH_USAGE_MASK =
            ~(GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_PROTECTED)
    };

    // verify that passthrough mask is within the platform mask
    static_assert((~C2MemoryUsage::PLATFORM_MASK & PASSTHROUGH_USAGE_MASK) == 0, "");
} // unnamed

C2MemoryUsage C2AndroidMemoryUsage::FromGrallocUsage(uint64_t usage) {
    // gralloc does not support WRITE_PROTECTED
    return C2MemoryUsage(
            ((usage & GRALLOC_USAGE_SW_READ_MASK) ? C2MemoryUsage::CPU_READ : 0) |
            ((usage & GRALLOC_USAGE_SW_WRITE_MASK) ? C2MemoryUsage::CPU_WRITE : 0) |
            ((usage & GRALLOC_USAGE_PROTECTED) ? C2MemoryUsage::READ_PROTECTED : 0) |
            (usage & PASSTHROUGH_USAGE_MASK));
}

uint64_t C2AndroidMemoryUsage::asGrallocUsage() const {
    // gralloc does not support WRITE_PROTECTED
    return (((expected & C2MemoryUsage::CPU_READ) ? GRALLOC_USAGE_SW_READ_OFTEN : 0) |
            ((expected & C2MemoryUsage::CPU_WRITE) ? GRALLOC_USAGE_SW_WRITE_OFTEN : 0) |
            ((expected & C2MemoryUsage::READ_PROTECTED) ? GRALLOC_USAGE_PROTECTED : 0) |
            (expected & PASSTHROUGH_USAGE_MASK));
}

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_vec;
using ::android::hardware::graphics::common::V1_0::BufferUsage;
using PixelFormat2 = ::android::hardware::graphics::common::V1_0::PixelFormat;
using PixelFormat3 = ::android::hardware::graphics::common::V1_2::PixelFormat;

using IAllocator2 = ::android::hardware::graphics::allocator::V2_0::IAllocator;
using BufferDescriptor2 = ::android::hardware::graphics::mapper::V2_0::BufferDescriptor;
using Error2 = ::android::hardware::graphics::mapper::V2_0::Error;
using IMapper2 = ::android::hardware::graphics::mapper::V2_0::IMapper;

using IAllocator3 = ::android::hardware::graphics::allocator::V3_0::IAllocator;
using BufferDescriptor3 = ::android::hardware::graphics::mapper::V3_0::BufferDescriptor;
using Error3 = ::android::hardware::graphics::mapper::V3_0::Error;
using IMapper3 = ::android::hardware::graphics::mapper::V3_0::IMapper;

namespace /* unnamed */ {

struct BufferDescriptorInfo2 {
    IMapper2::BufferDescriptorInfo mapperInfo;
    uint32_t stride;
};

struct BufferDescriptorInfo3 {
    IMapper3::BufferDescriptorInfo mapperInfo;
    uint32_t stride;
};

/* ===================================== GRALLOC ALLOCATION ==================================== */
c2_status_t maperr2error(Error2 maperr) {
    switch (maperr) {
        case Error2::NONE:           return C2_OK;
        case Error2::BAD_DESCRIPTOR: return C2_BAD_VALUE;
        case Error2::BAD_BUFFER:     return C2_BAD_VALUE;
        case Error2::BAD_VALUE:      return C2_BAD_VALUE;
        case Error2::NO_RESOURCES:   return C2_NO_MEMORY;
        case Error2::UNSUPPORTED:    return C2_CANNOT_DO;
    }
    return C2_CORRUPTED;
}

c2_status_t maperr2error(Error3 maperr) {
    switch (maperr) {
        case Error3::NONE:           return C2_OK;
        case Error3::BAD_DESCRIPTOR: return C2_BAD_VALUE;
        case Error3::BAD_BUFFER:     return C2_BAD_VALUE;
        case Error3::BAD_VALUE:      return C2_BAD_VALUE;
        case Error3::NO_RESOURCES:   return C2_NO_MEMORY;
        case Error3::UNSUPPORTED:    return C2_CANNOT_DO;
    }
    return C2_CORRUPTED;
}

bool native_handle_is_invalid(const native_handle_t *const handle) {
    // perform basic validation of a native handle
    if (handle == nullptr) {
        // null handle is considered valid
        return false;
    }
    return ((size_t)handle->version != sizeof(native_handle_t) ||
            handle->numFds < 0 ||
            handle->numInts < 0 ||
            // for sanity assume handles must occupy less memory than INT_MAX bytes
            handle->numFds > int((INT_MAX - handle->version) / sizeof(int)) - handle->numInts);
}

class C2HandleGralloc : public C2Handle {
private:
    struct ExtraData {
        uint32_t width;
        uint32_t height;
        uint32_t format;
        uint32_t usage_lo;
        uint32_t usage_hi;
        uint32_t stride;
        uint32_t generation;
        uint32_t igbp_id_lo;
        uint32_t igbp_id_hi;
        uint32_t igbp_slot;
        uint32_t magic;
    };

    enum {
        NUM_INTS = sizeof(ExtraData) / sizeof(int),
    };
    const static uint32_t MAGIC = '\xc2gr\x00';

    static
    const ExtraData* getExtraData(const C2Handle *const handle) {
        if (handle == nullptr
                || native_handle_is_invalid(handle)
                || handle->numInts < NUM_INTS) {
            return nullptr;
        }
        return reinterpret_cast<const ExtraData*>(
                &handle->data[handle->numFds + handle->numInts - NUM_INTS]);
    }

    static
    ExtraData *getExtraData(C2Handle *const handle) {
        return const_cast<ExtraData *>(getExtraData(const_cast<const C2Handle *const>(handle)));
    }

public:
    void getIgbpData(uint32_t *generation, uint64_t *igbp_id, uint32_t *igbp_slot) const {
        const ExtraData *ed = getExtraData(this);
        *generation = ed->generation;
        *igbp_id = unsigned(ed->igbp_id_lo) | uint64_t(unsigned(ed->igbp_id_hi)) << 32;
        *igbp_slot = ed->igbp_slot;
    }

    static bool isValid(const C2Handle *const o) {
        if (o == nullptr) { // null handle is always valid
            return true;
        }
        const ExtraData *xd = getExtraData(o);
        // we cannot validate width/height/format/usage without accessing gralloc driver
        return xd != nullptr && xd->magic == MAGIC;
    }

    static C2HandleGralloc* WrapAndMoveNativeHandle(
            const native_handle_t *const handle,
            uint32_t width, uint32_t height, uint32_t format, uint64_t usage,
            uint32_t stride, uint32_t generation, uint64_t igbp_id = 0, uint32_t igbp_slot = 0) {
        //CHECK(handle != nullptr);
        if (native_handle_is_invalid(handle) ||
            handle->numInts > int((INT_MAX - handle->version) / sizeof(int)) - NUM_INTS - handle->numFds) {
            return nullptr;
        }
        ExtraData xd = {
            width, height, format, uint32_t(usage & 0xFFFFFFFF), uint32_t(usage >> 32),
            stride, generation, uint32_t(igbp_id & 0xFFFFFFFF), uint32_t(igbp_id >> 32),
            igbp_slot, MAGIC
        };
        native_handle_t *res = native_handle_create(handle->numFds, handle->numInts + NUM_INTS);
        if (res != nullptr) {
            memcpy(&res->data, &handle->data, sizeof(int) * (handle->numFds + handle->numInts));
            *getExtraData(res) = xd;
        }
        return reinterpret_cast<C2HandleGralloc *>(res);
    }

    static C2HandleGralloc* WrapNativeHandle(
            const native_handle_t *const handle,
            uint32_t width, uint32_t height, uint32_t format, uint64_t usage,
            uint32_t stride, uint32_t generation, uint64_t igbp_id = 0, uint32_t igbp_slot = 0) {
        if (handle == nullptr) {
            return nullptr;
        }
        native_handle_t *clone = native_handle_clone(handle);
        if (clone == nullptr) {
            return nullptr;
        }
        C2HandleGralloc *res = WrapAndMoveNativeHandle(
                clone, width, height, format, usage, stride, generation, igbp_id, igbp_slot);
        if (res == nullptr) {
            native_handle_close(clone);
        }
        native_handle_delete(clone);
        return res;
    }

    static bool MigrateNativeHandle(
            native_handle_t *handle,
            uint32_t generation, uint64_t igbp_id, uint32_t igbp_slot) {
        if (handle == nullptr || !isValid(handle)) {
            return false;
        }
        ExtraData *ed = getExtraData(handle);
        if (!ed) return false;
        ed->generation = generation;
        ed->igbp_id_lo = uint32_t(igbp_id & 0xFFFFFFFF);
        ed->igbp_id_hi = uint32_t(igbp_id >> 32);
        ed->igbp_slot = igbp_slot;
        return true;
    }


    static native_handle_t* UnwrapNativeHandle(
            const C2Handle *const handle) {
        const ExtraData *xd = getExtraData(handle);
        if (xd == nullptr || xd->magic != MAGIC) {
            return nullptr;
        }
        native_handle_t *res = native_handle_create(handle->numFds, handle->numInts - NUM_INTS);
        if (res != nullptr) {
            memcpy(&res->data, &handle->data, sizeof(int) * (res->numFds + res->numInts));
        }
        return res;
    }

    static const C2HandleGralloc* Import(
            const C2Handle *const handle,
            uint32_t *width, uint32_t *height, uint32_t *format,
            uint64_t *usage, uint32_t *stride,
            uint32_t *generation, uint64_t *igbp_id, uint32_t *igbp_slot) {
        const ExtraData *xd = getExtraData(handle);
        if (xd == nullptr) {
            return nullptr;
        }
        *width = xd->width;
        *height = xd->height;
        *format = xd->format;
        *usage = xd->usage_lo | (uint64_t(xd->usage_hi) << 32);
        *stride = xd->stride;
        *generation = xd->generation;
        *igbp_id = xd->igbp_id_lo | (uint64_t(xd->igbp_id_hi) << 32);
        *igbp_slot = xd->igbp_slot;
        return reinterpret_cast<const C2HandleGralloc *>(handle);
    }
};

} // unnamed namespace

native_handle_t *UnwrapNativeCodec2GrallocHandle(const C2Handle *const handle) {
    return C2HandleGralloc::UnwrapNativeHandle(handle);
}

C2Handle *WrapNativeCodec2GrallocHandle(
        const native_handle_t *const handle,
        uint32_t width, uint32_t height, uint32_t format, uint64_t usage, uint32_t stride,
        uint32_t generation, uint64_t igbp_id, uint32_t igbp_slot) {
    return C2HandleGralloc::WrapNativeHandle(handle, width, height, format, usage, stride,
                                             generation, igbp_id, igbp_slot);
}

bool MigrateNativeCodec2GrallocHandle(
        native_handle_t *handle,
        uint32_t generation, uint64_t igbp_id, uint32_t igbp_slot) {
    return C2HandleGralloc::MigrateNativeHandle(handle, generation, igbp_id, igbp_slot);
}


class C2AllocationGralloc : public C2GraphicAllocation {
public:
    virtual ~C2AllocationGralloc() override;

    virtual c2_status_t map(
            C2Rect rect, C2MemoryUsage usage, C2Fence *fence,
            C2PlanarLayout *layout /* nonnull */, uint8_t **addr /* nonnull */) override;
    virtual c2_status_t unmap(
            uint8_t **addr /* nonnull */, C2Rect rect, C2Fence *fence /* nullable */) override;
    virtual C2Allocator::id_t getAllocatorId() const override { return mAllocatorId; }
    virtual const C2Handle *handle() const override { return mLockedHandle ? : mHandle; }
    virtual bool equals(const std::shared_ptr<const C2GraphicAllocation> &other) const override;

    // internal methods
    // |handle| will be moved.
    C2AllocationGralloc(
              const BufferDescriptorInfo2 &info,
              const sp<IMapper2> &mapper,
              hidl_handle &hidlHandle,
              const C2HandleGralloc *const handle,
              C2Allocator::id_t allocatorId);
    C2AllocationGralloc(
              const BufferDescriptorInfo3 &info,
              const sp<IMapper3> &mapper,
              hidl_handle &hidlHandle,
              const C2HandleGralloc *const handle,
              C2Allocator::id_t allocatorId);
    int dup() const;
    c2_status_t status() const;

private:
    const BufferDescriptorInfo2 mInfo2{};
    const sp<IMapper2> mMapper2{nullptr};
    const BufferDescriptorInfo3 mInfo3{};
    const sp<IMapper3> mMapper3{nullptr};
    const hidl_handle mHidlHandle;
    const C2HandleGralloc *mHandle;
    buffer_handle_t mBuffer;
    const C2HandleGralloc *mLockedHandle;
    bool mLocked;
    C2Allocator::id_t mAllocatorId;
    std::mutex mMappedLock;
};

C2AllocationGralloc::C2AllocationGralloc(
          const BufferDescriptorInfo2 &info,
          const sp<IMapper2> &mapper,
          hidl_handle &hidlHandle,
          const C2HandleGralloc *const handle,
          C2Allocator::id_t allocatorId)
    : C2GraphicAllocation(info.mapperInfo.width, info.mapperInfo.height),
      mInfo2(info),
      mMapper2(mapper),
      mHidlHandle(std::move(hidlHandle)),
      mHandle(handle),
      mBuffer(nullptr),
      mLockedHandle(nullptr),
      mLocked(false),
      mAllocatorId(allocatorId) {
}

C2AllocationGralloc::C2AllocationGralloc(
          const BufferDescriptorInfo3 &info,
          const sp<IMapper3> &mapper,
          hidl_handle &hidlHandle,
          const C2HandleGralloc *const handle,
          C2Allocator::id_t allocatorId)
    : C2GraphicAllocation(info.mapperInfo.width, info.mapperInfo.height),
      mInfo3(info),
      mMapper3(mapper),
      mHidlHandle(std::move(hidlHandle)),
      mHandle(handle),
      mBuffer(nullptr),
      mLockedHandle(nullptr),
      mLocked(false),
      mAllocatorId(allocatorId) {
}

C2AllocationGralloc::~C2AllocationGralloc() {
    if (mBuffer && mLocked) {
        // implementation ignores addresss and rect
        uint8_t* addr[C2PlanarLayout::MAX_NUM_PLANES] = {};
        unmap(addr, C2Rect(), nullptr);
    }
    if (mBuffer) {
        if (mMapper2) {
            if (!mMapper2->freeBuffer(const_cast<native_handle_t *>(
                    mBuffer)).isOk()) {
                ALOGE("failed transaction: freeBuffer");
            }
        } else {
            if (!mMapper3->freeBuffer(const_cast<native_handle_t *>(
                    mBuffer)).isOk()) {
                ALOGE("failed transaction: freeBuffer");
            }
        }
    }
    if (mHandle) {
        native_handle_delete(
                const_cast<native_handle_t *>(reinterpret_cast<const native_handle_t *>(mHandle)));
    }
    if (mLockedHandle) {
        native_handle_delete(
                const_cast<native_handle_t *>(
                        reinterpret_cast<const native_handle_t *>(mLockedHandle)));
    }
}

c2_status_t C2AllocationGralloc::map(
        C2Rect rect, C2MemoryUsage usage, C2Fence *fence,
        C2PlanarLayout *layout /* nonnull */, uint8_t **addr /* nonnull */) {
    uint64_t grallocUsage = static_cast<C2AndroidMemoryUsage>(usage).asGrallocUsage();
    ALOGV("mapping buffer with usage %#llx => %#llx",
          (long long)usage.expected, (long long)grallocUsage);

    // TODO
    (void) fence;

    std::lock_guard<std::mutex> lock(mMappedLock);
    if (mBuffer && mLocked) {
        ALOGD("already mapped");
        return C2_DUPLICATE;
    }
    if (!layout || !addr) {
        ALOGD("wrong param");
        return C2_BAD_VALUE;
    }

    c2_status_t err = C2_OK;
    if (!mBuffer) {
        if (mMapper2) {
            if (!mMapper2->importBuffer(
                    mHidlHandle, [&err, this](const auto &maperr, const auto &buffer) {
                        err = maperr2error(maperr);
                        if (err == C2_OK) {
                            mBuffer = static_cast<buffer_handle_t>(buffer);
                        }
                    }).isOk()) {
                ALOGE("failed transaction: importBuffer");
                return C2_CORRUPTED;
            }
        } else {
            if (!mMapper3->importBuffer(
                    mHidlHandle, [&err, this](const auto &maperr, const auto &buffer) {
                        err = maperr2error(maperr);
                        if (err == C2_OK) {
                            mBuffer = static_cast<buffer_handle_t>(buffer);
                        }
                    }).isOk()) {
                ALOGE("failed transaction: importBuffer (@3.0)");
                return C2_CORRUPTED;
            }
        }
        if (err != C2_OK) {
            ALOGD("importBuffer failed: %d", err);
            return err;
        }
        if (mBuffer == nullptr) {
            ALOGD("importBuffer returned null buffer");
            return C2_CORRUPTED;
        }
        uint32_t generation = 0;
        uint64_t igbp_id = 0;
        uint32_t igbp_slot = 0;
        if (mHandle) {
            mHandle->getIgbpData(&generation, &igbp_id, &igbp_slot);
        }
        if (mMapper2) {
            mLockedHandle = C2HandleGralloc::WrapAndMoveNativeHandle(
                    mBuffer, mInfo2.mapperInfo.width, mInfo2.mapperInfo.height,
                    (uint32_t)mInfo2.mapperInfo.format, mInfo2.mapperInfo.usage,
                    mInfo2.stride, generation, igbp_id, igbp_slot);
        } else {
            mLockedHandle = C2HandleGralloc::WrapAndMoveNativeHandle(
                    mBuffer, mInfo3.mapperInfo.width, mInfo3.mapperInfo.height,
                    (uint32_t)mInfo3.mapperInfo.format, mInfo3.mapperInfo.usage,
                    mInfo3.stride, generation, igbp_id, igbp_slot);
        }
    }

    PixelFormat3 format = mMapper2 ?
            PixelFormat3(mInfo2.mapperInfo.format) :
            PixelFormat3(mInfo3.mapperInfo.format);
    switch (format) {
        case PixelFormat3::RGBA_1010102: {
            // TRICKY: this is used for media as YUV444 in the case when it is queued directly to a
            // Surface. In all other cases it is RGBA. We don't know which case it is here, so
            // default to YUV for now.
            void *pointer = nullptr;
            if (mMapper2) {
                if (!mMapper2->lock(
                        const_cast<native_handle_t *>(mBuffer),
                        grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &pointer](const auto &maperr, const auto &mapPointer) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                pointer = mapPointer;
                            }
                        }).isOk()) {
                    ALOGE("failed transaction: lock(RGBA_1010102)");
                    return C2_CORRUPTED;
                }
            } else {
                if (!mMapper3->lock(
                        const_cast<native_handle_t *>(mBuffer),
                        grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &pointer](const auto &maperr, const auto &mapPointer,
                                         int32_t bytesPerPixel, int32_t bytesPerStride) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                pointer = mapPointer;
                            }
                            (void)bytesPerPixel;
                            (void)bytesPerStride;
                        }).isOk()) {
                    ALOGE("failed transaction: lock(RGBA_1010102) (@3.0)");
                    return C2_CORRUPTED;
                }
            }
            if (err != C2_OK) {
                ALOGD("lock failed: %d", err);
                return err;
            }
            // treat as 32-bit values
            addr[C2PlanarLayout::PLANE_Y] = (uint8_t *)pointer;
            addr[C2PlanarLayout::PLANE_U] = (uint8_t *)pointer;
            addr[C2PlanarLayout::PLANE_V] = (uint8_t *)pointer;
            addr[C2PlanarLayout::PLANE_A] = (uint8_t *)pointer;
            layout->type = C2PlanarLayout::TYPE_YUVA;
            layout->numPlanes = 4;
            layout->rootPlanes = 1;
            int32_t stride = mMapper2 ?
                    int32_t(mInfo2.stride) :
                    int32_t(mInfo3.stride);
            layout->planes[C2PlanarLayout::PLANE_Y] = {
                C2PlaneInfo::CHANNEL_Y,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                32,                             // allocatedDepth
                10,                             // bitDepth
                10,                             // rightShift
                C2PlaneInfo::LITTLE_END,        // endianness
                C2PlanarLayout::PLANE_Y,        // rootIx
                0,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_U] = {
                C2PlaneInfo::CHANNEL_CB,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                32,                             // allocatedDepth
                10,                             // bitDepth
                0,                              // rightShift
                C2PlaneInfo::LITTLE_END,        // endianness
                C2PlanarLayout::PLANE_Y,        // rootIx
                0,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_V] = {
                C2PlaneInfo::CHANNEL_CR,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                32,                             // allocatedDepth
                10,                             // bitDepth
                20,                             // rightShift
                C2PlaneInfo::LITTLE_END,        // endianness
                C2PlanarLayout::PLANE_Y,        // rootIx
                0,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_A] = {
                C2PlaneInfo::CHANNEL_A,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                32,                             // allocatedDepth
                2,                              // bitDepth
                30,                             // rightShift
                C2PlaneInfo::LITTLE_END,        // endianness
                C2PlanarLayout::PLANE_Y,        // rootIx
                0,                              // offset
            };
            break;
        }

        case PixelFormat3::RGBA_8888:
            // TODO: alpha channel
            // fall-through
        case PixelFormat3::RGBX_8888: {
            void *pointer = nullptr;
            if (mMapper2) {
                if (!mMapper2->lock(
                        const_cast<native_handle_t *>(mBuffer),
                        grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &pointer](const auto &maperr, const auto &mapPointer) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                pointer = mapPointer;
                            }
                        }).isOk()) {
                    ALOGE("failed transaction: lock(RGBA_8888)");
                    return C2_CORRUPTED;
                }
            } else {
                if (!mMapper3->lock(
                        const_cast<native_handle_t *>(mBuffer),
                        grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &pointer](const auto &maperr, const auto &mapPointer,
                                         int32_t bytesPerPixel, int32_t bytesPerStride) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                pointer = mapPointer;
                            }
                            (void)bytesPerPixel;
                            (void)bytesPerStride;
                        }).isOk()) {
                    ALOGE("failed transaction: lock(RGBA_8888) (@3.0)");
                    return C2_CORRUPTED;
                }
            }
            if (err != C2_OK) {
                ALOGD("lock failed: %d", err);
                return err;
            }
            addr[C2PlanarLayout::PLANE_R] = (uint8_t *)pointer;
            addr[C2PlanarLayout::PLANE_G] = (uint8_t *)pointer + 1;
            addr[C2PlanarLayout::PLANE_B] = (uint8_t *)pointer + 2;
            layout->type = C2PlanarLayout::TYPE_RGB;
            layout->numPlanes = 3;
            layout->rootPlanes = 1;
            int32_t stride = mMapper2 ?
                    int32_t(mInfo2.stride) :
                    int32_t(mInfo3.stride);
            layout->planes[C2PlanarLayout::PLANE_R] = {
                C2PlaneInfo::CHANNEL_R,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                8,                              // allocatedDepth
                8,                              // bitDepth
                0,                              // rightShift
                C2PlaneInfo::NATIVE,            // endianness
                C2PlanarLayout::PLANE_R,        // rootIx
                0,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_G] = {
                C2PlaneInfo::CHANNEL_G,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                8,                              // allocatedDepth
                8,                              // bitDepth
                0,                              // rightShift
                C2PlaneInfo::NATIVE,            // endianness
                C2PlanarLayout::PLANE_R,        // rootIx
                1,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_B] = {
                C2PlaneInfo::CHANNEL_B,         // channel
                4,                              // colInc
                4 * stride,                     // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                8,                              // allocatedDepth
                8,                              // bitDepth
                0,                              // rightShift
                C2PlaneInfo::NATIVE,            // endianness
                C2PlanarLayout::PLANE_R,        // rootIx
                2,                              // offset
            };
            break;
        }

        case PixelFormat3::YCBCR_420_888:
            // fall-through
        case PixelFormat3::YV12:
            // fall-through
        default: {
            struct YCbCrLayout {
                void* y;
                void* cb;
                void* cr;
                uint32_t yStride;
                uint32_t cStride;
                uint32_t chromaStep;
            };
            YCbCrLayout ycbcrLayout;
            if (mMapper2) {
                if (!mMapper2->lockYCbCr(
                        const_cast<native_handle_t *>(mBuffer), grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &ycbcrLayout](const auto &maperr, const auto &mapLayout) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                ycbcrLayout = YCbCrLayout{
                                        mapLayout.y,
                                        mapLayout.cb,
                                        mapLayout.cr,
                                        mapLayout.yStride,
                                        mapLayout.cStride,
                                        mapLayout.chromaStep};
                            }
                        }).isOk()) {
                    ALOGE("failed transaction: lockYCbCr");
                    return C2_CORRUPTED;
                }
            } else {
                if (!mMapper3->lockYCbCr(
                        const_cast<native_handle_t *>(mBuffer), grallocUsage,
                        { (int32_t)rect.left, (int32_t)rect.top,
                          (int32_t)rect.width, (int32_t)rect.height },
                        // TODO: fence
                        hidl_handle(),
                        [&err, &ycbcrLayout](const auto &maperr, const auto &mapLayout) {
                            err = maperr2error(maperr);
                            if (err == C2_OK) {
                                ycbcrLayout = YCbCrLayout{
                                        mapLayout.y,
                                        mapLayout.cb,
                                        mapLayout.cr,
                                        mapLayout.yStride,
                                        mapLayout.cStride,
                                        mapLayout.chromaStep};
                            }
                        }).isOk()) {
                    ALOGE("failed transaction: lockYCbCr (@3.0)");
                    return C2_CORRUPTED;
                }
            }
            if (err != C2_OK) {
                ALOGD("lockYCbCr failed: %d", err);
                return err;
            }
            addr[C2PlanarLayout::PLANE_Y] = (uint8_t *)ycbcrLayout.y;
            addr[C2PlanarLayout::PLANE_U] = (uint8_t *)ycbcrLayout.cb;
            addr[C2PlanarLayout::PLANE_V] = (uint8_t *)ycbcrLayout.cr;
            layout->type = C2PlanarLayout::TYPE_YUV;
            layout->numPlanes = 3;
            layout->rootPlanes = 3;
            layout->planes[C2PlanarLayout::PLANE_Y] = {
                C2PlaneInfo::CHANNEL_Y,         // channel
                1,                              // colInc
                (int32_t)ycbcrLayout.yStride,   // rowInc
                1,                              // mColSampling
                1,                              // mRowSampling
                8,                              // allocatedDepth
                8,                              // bitDepth
                0,                              // rightShift
                C2PlaneInfo::NATIVE,            // endianness
                C2PlanarLayout::PLANE_Y,        // rootIx
                0,                              // offset
            };
            layout->planes[C2PlanarLayout::PLANE_U] = {
                C2PlaneInfo::CHANNEL_CB,          // channel
                (int32_t)ycbcrLayout.chromaStep,  // colInc
                (int32_t)ycbcrLayout.cStride,     // rowInc
                2,                                // mColSampling
                2,                                // mRowSampling
                8,                                // allocatedDepth
                8,                                // bitDepth
                0,                                // rightShift
                C2PlaneInfo::NATIVE,              // endianness
                C2PlanarLayout::PLANE_U,          // rootIx
                0,                                // offset
            };
            layout->planes[C2PlanarLayout::PLANE_V] = {
                C2PlaneInfo::CHANNEL_CR,          // channel
                (int32_t)ycbcrLayout.chromaStep,  // colInc
                (int32_t)ycbcrLayout.cStride,     // rowInc
                2,                                // mColSampling
                2,                                // mRowSampling
                8,                                // allocatedDepth
                8,                                // bitDepth
                0,                                // rightShift
                C2PlaneInfo::NATIVE,              // endianness
                C2PlanarLayout::PLANE_V,          // rootIx
                0,                                // offset
            };
            // handle interleaved formats
            intptr_t uvOffset = addr[C2PlanarLayout::PLANE_V] - addr[C2PlanarLayout::PLANE_U];
            if (uvOffset > 0 && uvOffset < (intptr_t)ycbcrLayout.chromaStep) {
                layout->rootPlanes = 2;
                layout->planes[C2PlanarLayout::PLANE_V].rootIx = C2PlanarLayout::PLANE_U;
                layout->planes[C2PlanarLayout::PLANE_V].offset = uvOffset;
            } else if (uvOffset < 0 && uvOffset > -(intptr_t)ycbcrLayout.chromaStep) {
                layout->rootPlanes = 2;
                layout->planes[C2PlanarLayout::PLANE_U].rootIx = C2PlanarLayout::PLANE_V;
                layout->planes[C2PlanarLayout::PLANE_U].offset = -uvOffset;
            }
            break;
        }
    }
    mLocked = true;

    return C2_OK;
}

c2_status_t C2AllocationGralloc::unmap(
        uint8_t **addr, C2Rect rect, C2Fence *fence /* nullable */) {
    // TODO: check addr and size, use fence
    (void)addr;
    (void)rect;

    std::lock_guard<std::mutex> lock(mMappedLock);
    c2_status_t err = C2_OK;
    if (mMapper2) {
        if (!mMapper2->unlock(
                const_cast<native_handle_t *>(mBuffer),
                [&err, &fence](const auto &maperr, const auto &releaseFence) {
                    // TODO
                    (void) fence;
                    (void) releaseFence;
                    err = maperr2error(maperr);
                    if (err == C2_OK) {
                        // TODO: fence
                    }
                }).isOk()) {
            ALOGE("failed transaction: unlock");
            return C2_CORRUPTED;
        }
    } else {
        if (!mMapper3->unlock(
                const_cast<native_handle_t *>(mBuffer),
                [&err, &fence](const auto &maperr, const auto &releaseFence) {
                    // TODO
                    (void) fence;
                    (void) releaseFence;
                    err = maperr2error(maperr);
                    if (err == C2_OK) {
                        // TODO: fence
                    }
                }).isOk()) {
            ALOGE("failed transaction: unlock (@3.0)");
            return C2_CORRUPTED;
        }
    }
    if (err == C2_OK) {
        mLocked = false;
    }
    return err;
}

bool C2AllocationGralloc::equals(const std::shared_ptr<const C2GraphicAllocation> &other) const {
    return other && other->handle() == handle();
}

/* ===================================== GRALLOC ALLOCATOR ==================================== */
class C2AllocatorGralloc::Impl {
public:
    Impl(id_t id, bool bufferQueue);

    id_t getId() const {
        return mTraits->id;
    }

    C2String getName() const {
        return mTraits->name;
    }

    std::shared_ptr<const C2Allocator::Traits> getTraits() const {
        return mTraits;
    }

    c2_status_t newGraphicAllocation(
            uint32_t width, uint32_t height, uint32_t format, const C2MemoryUsage &usage,
            std::shared_ptr<C2GraphicAllocation> *allocation);

    c2_status_t priorGraphicAllocation(
            const C2Handle *handle,
            std::shared_ptr<C2GraphicAllocation> *allocation);

    c2_status_t status() const { return mInit; }

private:
    std::shared_ptr<C2Allocator::Traits> mTraits;
    c2_status_t mInit;
    sp<IAllocator2> mAllocator2;
    sp<IMapper2> mMapper2;
    sp<IAllocator3> mAllocator3;
    sp<IMapper3> mMapper3;
    const bool mBufferQueue;
};

void _UnwrapNativeCodec2GrallocMetadata(
        const C2Handle *const handle,
        uint32_t *width, uint32_t *height, uint32_t *format,uint64_t *usage, uint32_t *stride,
        uint32_t *generation, uint64_t *igbp_id, uint32_t *igbp_slot) {
    (void)C2HandleGralloc::Import(handle, width, height, format, usage, stride,
                                  generation, igbp_id, igbp_slot);
}

C2AllocatorGralloc::Impl::Impl(id_t id, bool bufferQueue)
    : mInit(C2_OK), mBufferQueue(bufferQueue) {
    // TODO: get this from allocator
    C2MemoryUsage minUsage = { 0, 0 }, maxUsage = { ~(uint64_t)0, ~(uint64_t)0 };
    Traits traits = { "android.allocator.gralloc", id, C2Allocator::GRAPHIC, minUsage, maxUsage };
    mTraits = std::make_shared<C2Allocator::Traits>(traits);

    // gralloc allocator is a singleton, so all objects share a global service
    mAllocator3 = IAllocator3::getService();
    mMapper3 = IMapper3::getService();
    if (!mAllocator3 || !mMapper3) {
        mAllocator3 = nullptr;
        mMapper3 = nullptr;
        mAllocator2 = IAllocator2::getService();
        mMapper2 = IMapper2::getService();
        if (!mAllocator2 || !mMapper2) {
            mAllocator2 = nullptr;
            mMapper2 = nullptr;
            mInit = C2_CORRUPTED;
        }
    }
}

c2_status_t C2AllocatorGralloc::Impl::newGraphicAllocation(
        uint32_t width, uint32_t height, uint32_t format, const C2MemoryUsage &usage,
        std::shared_ptr<C2GraphicAllocation> *allocation) {
    uint64_t grallocUsage = static_cast<C2AndroidMemoryUsage>(usage).asGrallocUsage();
    ALOGV("allocating buffer with usage %#llx => %#llx",
          (long long)usage.expected, (long long)grallocUsage);

    c2_status_t err = C2_OK;
    hidl_handle buffer{};

    if (mMapper2) {
        BufferDescriptorInfo2 info = {
            {
                width,
                height,
                1u,  // layerCount
                PixelFormat2(format),
                grallocUsage,
            },
            0u,  // stride placeholder
        };
        BufferDescriptor2 desc;
        if (!mMapper2->createDescriptor(
                info.mapperInfo, [&err, &desc](const auto &maperr, const auto &descriptor) {
                    err = maperr2error(maperr);
                    if (err == C2_OK) {
                        desc = descriptor;
                    }
                }).isOk()) {
            ALOGE("failed transaction: createDescriptor");
            return C2_CORRUPTED;
        }
        if (err != C2_OK) {
            return err;
        }

        // IAllocator shares IMapper error codes.
        if (!mAllocator2->allocate(
                desc,
                1u,
                [&err, &buffer, &info](const auto &maperr, const auto &stride, auto &buffers) {
                    err = maperr2error(maperr);
                    if (err != C2_OK) {
                        return;
                    }
                    if (buffers.size() != 1u) {
                        err = C2_CORRUPTED;
                        return;
                    }
                    info.stride = stride;
                    buffer = buffers[0];
                }).isOk()) {
            ALOGE("failed transaction: allocate");
            return C2_CORRUPTED;
        }
        if (err != C2_OK) {
            return err;
        }
        allocation->reset(new C2AllocationGralloc(
                info, mMapper2, buffer,
                C2HandleGralloc::WrapAndMoveNativeHandle(
                        buffer.getNativeHandle(),
                        width, height,
                        format, grallocUsage, info.stride,
                        0, 0, mBufferQueue ? ~0 : 0),
                mTraits->id));
        return C2_OK;
    } else {
        BufferDescriptorInfo3 info = {
            {
                width,
                height,
                1u,  // layerCount
                PixelFormat3(format),
                grallocUsage,
            },
            0u,  // stride placeholder
        };
        BufferDescriptor3 desc;
        if (!mMapper3->createDescriptor(
                info.mapperInfo, [&err, &desc](const auto &maperr, const auto &descriptor) {
                    err = maperr2error(maperr);
                    if (err == C2_OK) {
                        desc = descriptor;
                    }
                }).isOk()) {
            ALOGE("failed transaction: createDescriptor");
            return C2_CORRUPTED;
        }
        if (err != C2_OK) {
            return err;
        }

        // IAllocator shares IMapper error codes.
        if (!mAllocator3->allocate(
                desc,
                1u,
                [&err, &buffer, &info](const auto &maperr, const auto &stride, auto &buffers) {
                    err = maperr2error(maperr);
                    if (err != C2_OK) {
                        return;
                    }
                    if (buffers.size() != 1u) {
                        err = C2_CORRUPTED;
                        return;
                    }
                    info.stride = stride;
                    buffer = buffers[0];
                }).isOk()) {
            ALOGE("failed transaction: allocate");
            return C2_CORRUPTED;
        }
        if (err != C2_OK) {
            return err;
        }
        allocation->reset(new C2AllocationGralloc(
                info, mMapper3, buffer,
                C2HandleGralloc::WrapAndMoveNativeHandle(
                        buffer.getNativeHandle(),
                        width, height,
                        format, grallocUsage, info.stride,
                        0, 0, mBufferQueue ? ~0 : 0),
                mTraits->id));
        return C2_OK;
    }
}

c2_status_t C2AllocatorGralloc::Impl::priorGraphicAllocation(
        const C2Handle *handle,
        std::shared_ptr<C2GraphicAllocation> *allocation) {
    if (mMapper2) {
        BufferDescriptorInfo2 info;
        info.mapperInfo.layerCount = 1u;
        uint32_t generation;
        uint64_t igbp_id;
        uint32_t igbp_slot;
        const C2HandleGralloc *grallocHandle = C2HandleGralloc::Import(
                handle,
                &info.mapperInfo.width, &info.mapperInfo.height,
                (uint32_t *)&info.mapperInfo.format,
                (uint64_t *)&info.mapperInfo.usage,
                &info.stride,
                &generation, &igbp_id, &igbp_slot);
        if (grallocHandle == nullptr) {
            return C2_BAD_VALUE;
        }

        hidl_handle hidlHandle;
        hidlHandle.setTo(C2HandleGralloc::UnwrapNativeHandle(grallocHandle), true);

        allocation->reset(new C2AllocationGralloc(
                info, mMapper2, hidlHandle, grallocHandle, mTraits->id));
        return C2_OK;
    } else {
        BufferDescriptorInfo3 info;
        info.mapperInfo.layerCount = 1u;
        uint32_t generation;
        uint64_t igbp_id;
        uint32_t igbp_slot;
        const C2HandleGralloc *grallocHandle = C2HandleGralloc::Import(
                handle,
                &info.mapperInfo.width, &info.mapperInfo.height,
                (uint32_t *)&info.mapperInfo.format,
                (uint64_t *)&info.mapperInfo.usage,
                &info.stride,
                &generation, &igbp_id, &igbp_slot);
        if (grallocHandle == nullptr) {
            return C2_BAD_VALUE;
        }

        hidl_handle hidlHandle;
        hidlHandle.setTo(C2HandleGralloc::UnwrapNativeHandle(grallocHandle), true);

        allocation->reset(new C2AllocationGralloc(
                info, mMapper3, hidlHandle, grallocHandle, mTraits->id));
        return C2_OK;
    }
}

C2AllocatorGralloc::C2AllocatorGralloc(id_t id, bool bufferQueue)
        : mImpl(new Impl(id, bufferQueue)) {}

C2AllocatorGralloc::~C2AllocatorGralloc() { delete mImpl; }

C2Allocator::id_t C2AllocatorGralloc::getId() const {
    return mImpl->getId();
}

C2String C2AllocatorGralloc::getName() const {
    return mImpl->getName();
}

std::shared_ptr<const C2Allocator::Traits> C2AllocatorGralloc::getTraits() const {
    return mImpl->getTraits();
}

c2_status_t C2AllocatorGralloc::newGraphicAllocation(
        uint32_t width, uint32_t height, uint32_t format, C2MemoryUsage usage,
        std::shared_ptr<C2GraphicAllocation> *allocation) {
    return mImpl->newGraphicAllocation(width, height, format, usage, allocation);
}

c2_status_t C2AllocatorGralloc::priorGraphicAllocation(
        const C2Handle *handle,
        std::shared_ptr<C2GraphicAllocation> *allocation) {
    return mImpl->priorGraphicAllocation(handle, allocation);
}

c2_status_t C2AllocatorGralloc::status() const {
    return mImpl->status();
}

bool C2AllocatorGralloc::isValid(const C2Handle* const o) {
    return C2HandleGralloc::isValid(o);
}

} // namespace android
