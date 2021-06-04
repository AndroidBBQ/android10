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
#define LOG_TAG "C2Buffer"
#include <utils/Log.h>

#include <list>
#include <map>
#include <mutex>

#include <C2AllocatorIon.h>
#include <C2AllocatorGralloc.h>
#include <C2BufferPriv.h>
#include <C2BlockInternal.h>
#include <bufferpool/ClientManager.h>

namespace {

using android::C2AllocatorGralloc;
using android::C2AllocatorIon;
using android::hardware::media::bufferpool::BufferPoolData;
using android::hardware::media::bufferpool::V2_0::ResultStatus;
using android::hardware::media::bufferpool::V2_0::implementation::BufferPoolAllocation;
using android::hardware::media::bufferpool::V2_0::implementation::BufferPoolAllocator;
using android::hardware::media::bufferpool::V2_0::implementation::ClientManager;
using android::hardware::media::bufferpool::V2_0::implementation::ConnectionId;
using android::hardware::media::bufferpool::V2_0::implementation::INVALID_CONNECTIONID;

// This anonymous namespace contains the helper classes that allow our implementation to create
// block/buffer objects.
//
// Inherit from the parent, share with the friend.
class ReadViewBuddy : public C2ReadView {
    using C2ReadView::C2ReadView;
    friend class ::C2ConstLinearBlock;
};

class WriteViewBuddy : public C2WriteView {
    using C2WriteView::C2WriteView;
    friend class ::C2LinearBlock;
};

class ConstLinearBlockBuddy : public C2ConstLinearBlock {
    using C2ConstLinearBlock::C2ConstLinearBlock;
    friend class ::C2LinearBlock;
};

class LinearBlockBuddy : public C2LinearBlock {
    using C2LinearBlock::C2LinearBlock;
    friend class ::C2BasicLinearBlockPool;
};

class AcquirableReadViewBuddy : public C2Acquirable<C2ReadView> {
    using C2Acquirable::C2Acquirable;
    friend class ::C2ConstLinearBlock;
};

class AcquirableWriteViewBuddy : public C2Acquirable<C2WriteView> {
    using C2Acquirable::C2Acquirable;
    friend class ::C2LinearBlock;
};

class GraphicViewBuddy : public C2GraphicView {
    using C2GraphicView::C2GraphicView;
    friend class ::C2ConstGraphicBlock;
    friend class ::C2GraphicBlock;
};

class AcquirableConstGraphicViewBuddy : public C2Acquirable<const C2GraphicView> {
    using C2Acquirable::C2Acquirable;
    friend class ::C2ConstGraphicBlock;
};

class AcquirableGraphicViewBuddy : public C2Acquirable<C2GraphicView> {
    using C2Acquirable::C2Acquirable;
    friend class ::C2GraphicBlock;
};

class ConstGraphicBlockBuddy : public C2ConstGraphicBlock {
    using C2ConstGraphicBlock::C2ConstGraphicBlock;
    friend class ::C2GraphicBlock;
};

class GraphicBlockBuddy : public C2GraphicBlock {
    using C2GraphicBlock::C2GraphicBlock;
    friend class ::C2BasicGraphicBlockPool;
};

class BufferDataBuddy : public C2BufferData {
    using C2BufferData::C2BufferData;
    friend class ::C2Buffer;
};

}  // namespace

/* ========================================== 1D BLOCK ========================================= */

/**
 * This class is the base class for all 1D block and view implementations.
 *
 * This is basically just a placeholder for the underlying 1D allocation and the range of the
 * alloted portion to this block. There is also a placeholder for a blockpool data.
 */
class C2_HIDE _C2Block1DImpl : public _C2LinearRangeAspect {
public:
    _C2Block1DImpl(const std::shared_ptr<C2LinearAllocation> &alloc,
            const std::shared_ptr<_C2BlockPoolData> &poolData = nullptr,
            size_t offset = 0, size_t size = ~(size_t)0)
        : _C2LinearRangeAspect(alloc.get(), offset, size),
          mAllocation(alloc),
          mPoolData(poolData) { }

    _C2Block1DImpl(const _C2Block1DImpl &other, size_t offset = 0, size_t size = ~(size_t)0)
        : _C2LinearRangeAspect(&other, offset, size),
          mAllocation(other.mAllocation),
          mPoolData(other.mPoolData) { }

    /** returns pool data  */
    std::shared_ptr<_C2BlockPoolData> poolData() const {
        return mPoolData;
    }

    /** returns native handle */
    const C2Handle *handle() const {
        return mAllocation ? mAllocation->handle() : nullptr;
    }

    /** returns the allocator's ID */
    C2Allocator::id_t getAllocatorId() const {
        // BAD_ID can only happen if this Impl class is initialized for a view - never for a block.
        return mAllocation ? mAllocation->getAllocatorId() : C2Allocator::BAD_ID;
    }

    std::shared_ptr<C2LinearAllocation> getAllocation() const {
        return mAllocation;
    }

private:
    std::shared_ptr<C2LinearAllocation> mAllocation;
    std::shared_ptr<_C2BlockPoolData> mPoolData;
};

/**
 * This class contains the mapped data pointer, and the potential error.
 *
 * range is the mapped range of the underlying allocation (which is part of the allotted
 * range).
 */
class C2_HIDE _C2MappedBlock1DImpl : public _C2Block1DImpl {
public:
    _C2MappedBlock1DImpl(const _C2Block1DImpl &block, uint8_t *data,
                         size_t offset = 0, size_t size = ~(size_t)0)
        : _C2Block1DImpl(block, offset, size), mData(data), mError(C2_OK) { }

    _C2MappedBlock1DImpl(c2_status_t error)
        : _C2Block1DImpl(nullptr), mData(nullptr), mError(error) {
        // CHECK(error != C2_OK);
    }

    const uint8_t *data() const {
        return mData;
    }

    uint8_t *data() {
        return mData;
    }

    c2_status_t error() const {
        return mError;
    }

private:
    uint8_t *mData;
    c2_status_t mError;
};

/**
 * Block implementation.
 */
class C2Block1D::Impl : public _C2Block1DImpl {
    using _C2Block1DImpl::_C2Block1DImpl;
};

const C2Handle *C2Block1D::handle() const {
    return mImpl->handle();
};

C2Allocator::id_t C2Block1D::getAllocatorId() const {
    return mImpl->getAllocatorId();
};

C2Block1D::C2Block1D(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range)
    // always clamp subrange to parent (impl) range for safety
    : _C2LinearRangeAspect(impl.get(), range.offset(), range.size()), mImpl(impl) {
}

/**
 * Read view implementation.
 *
 * range of Impl is the mapped range of the underlying allocation (which is part of the allotted
 * range). range of View is 0 to capacity() (not represented as an actual range). This maps to a
 * subrange of Impl range starting at mImpl->offset() + _mOffset.
 */
class C2ReadView::Impl : public _C2MappedBlock1DImpl {
    using _C2MappedBlock1DImpl::_C2MappedBlock1DImpl;
};

C2ReadView::C2ReadView(std::shared_ptr<Impl> impl, uint32_t offset, uint32_t size)
    : _C2LinearCapacityAspect(C2LinearCapacity(impl->size()).range(offset, size).size()),
      mImpl(impl),
      mOffset(C2LinearCapacity(impl->size()).range(offset, size).offset()) { }

C2ReadView::C2ReadView(c2_status_t error)
    : _C2LinearCapacityAspect(0u), mImpl(std::make_shared<Impl>(error)), mOffset(0u) {
    // CHECK(error != C2_OK);
}

const uint8_t *C2ReadView::data() const {
    return mImpl->error() ? nullptr : mImpl->data() + mOffset;
}

c2_status_t C2ReadView::error() const {
    return mImpl->error();
}

C2ReadView C2ReadView::subView(size_t offset, size_t size) const {
    C2LinearRange subRange(*this, offset, size);
    return C2ReadView(mImpl, mOffset + subRange.offset(), subRange.size());
}

/**
 * Write view implementation.
 */
class C2WriteView::Impl : public _C2MappedBlock1DImpl {
    using _C2MappedBlock1DImpl::_C2MappedBlock1DImpl;
};

C2WriteView::C2WriteView(std::shared_ptr<Impl> impl)
// UGLY: _C2LinearRangeAspect requires a bona-fide object for capacity to prevent spoofing, so
// this is what we have to do.
// TODO: use childRange
    : _C2EditableLinearRangeAspect(std::make_unique<C2LinearCapacity>(impl->size()).get()), mImpl(impl) { }

C2WriteView::C2WriteView(c2_status_t error)
    : _C2EditableLinearRangeAspect(nullptr), mImpl(std::make_shared<Impl>(error)) {}

uint8_t *C2WriteView::base() { return mImpl->data(); }

uint8_t *C2WriteView::data() { return mImpl->data() + offset(); }

c2_status_t C2WriteView::error() const { return mImpl->error(); }

/**
 * Const linear block implementation.
 */
C2ConstLinearBlock::C2ConstLinearBlock(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range, C2Fence fence)
    : C2Block1D(impl, range), mFence(fence) { }

C2Acquirable<C2ReadView> C2ConstLinearBlock::map() const {
    void *base = nullptr;
    uint32_t len = size();
    c2_status_t error = mImpl->getAllocation()->map(
            offset(), len, { C2MemoryUsage::CPU_READ, 0 }, nullptr, &base);
    // TODO: wait on fence
    if (error == C2_OK) {
        std::shared_ptr<ReadViewBuddy::Impl> rvi = std::shared_ptr<ReadViewBuddy::Impl>(
                new ReadViewBuddy::Impl(*mImpl, (uint8_t *)base, offset(), len),
                [base, len](ReadViewBuddy::Impl *i) {
                    (void)i->getAllocation()->unmap(base, len, nullptr);
                    delete i;
        });
        return AcquirableReadViewBuddy(error, C2Fence(), ReadViewBuddy(rvi, 0, len));
    } else {
        return AcquirableReadViewBuddy(error, C2Fence(), ReadViewBuddy(error));
    }
}

C2ConstLinearBlock C2ConstLinearBlock::subBlock(size_t offset_, size_t size_) const {
    C2LinearRange subRange(*mImpl, offset_, size_);
    return C2ConstLinearBlock(mImpl, subRange, mFence);
}

/**
 * Linear block implementation.
 */
C2LinearBlock::C2LinearBlock(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range)
    : C2Block1D(impl, range) { }

C2Acquirable<C2WriteView> C2LinearBlock::map() {
    void *base = nullptr;
    uint32_t len = size();
    c2_status_t error = mImpl->getAllocation()->map(
            offset(), len, { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE }, nullptr, &base);
    // TODO: wait on fence
    if (error == C2_OK) {
        std::shared_ptr<WriteViewBuddy::Impl> rvi = std::shared_ptr<WriteViewBuddy::Impl>(
                new WriteViewBuddy::Impl(*mImpl, (uint8_t *)base, 0, len),
                [base, len](WriteViewBuddy::Impl *i) {
                    (void)i->getAllocation()->unmap(base, len, nullptr);
                    delete i;
        });
        return AcquirableWriteViewBuddy(error, C2Fence(), WriteViewBuddy(rvi));
    } else {
        return AcquirableWriteViewBuddy(error, C2Fence(), WriteViewBuddy(error));
    }
}

C2ConstLinearBlock C2LinearBlock::share(size_t offset_, size_t size_, C2Fence fence) {
    return ConstLinearBlockBuddy(mImpl, C2LinearRange(*this, offset_, size_), fence);
}

C2BasicLinearBlockPool::C2BasicLinearBlockPool(
        const std::shared_ptr<C2Allocator> &allocator)
  : mAllocator(allocator) { }

c2_status_t C2BasicLinearBlockPool::fetchLinearBlock(
        uint32_t capacity,
        C2MemoryUsage usage,
        std::shared_ptr<C2LinearBlock> *block /* nonnull */) {
    block->reset();

    std::shared_ptr<C2LinearAllocation> alloc;
    c2_status_t err = mAllocator->newLinearAllocation(capacity, usage, &alloc);
    if (err != C2_OK) {
        return err;
    }

    *block = _C2BlockFactory::CreateLinearBlock(alloc);

    return C2_OK;
}

struct C2_HIDE C2PooledBlockPoolData : _C2BlockPoolData {

    virtual type_t getType() const override {
        return TYPE_BUFFERPOOL;
    }

    void getBufferPoolData(std::shared_ptr<BufferPoolData> *data) const {
        *data = mData;
    }

    C2PooledBlockPoolData(const std::shared_ptr<BufferPoolData> &data) : mData(data) {}

    virtual ~C2PooledBlockPoolData() override {}

private:
    std::shared_ptr<BufferPoolData> mData;
};

bool _C2BlockFactory::GetBufferPoolData(
        const std::shared_ptr<const _C2BlockPoolData> &data,
        std::shared_ptr<BufferPoolData> *bufferPoolData) {
    if (data && data->getType() == _C2BlockPoolData::TYPE_BUFFERPOOL) {
        const std::shared_ptr<const C2PooledBlockPoolData> poolData =
                std::static_pointer_cast<const C2PooledBlockPoolData>(data);
        poolData->getBufferPoolData(bufferPoolData);
        return true;
    }
    return false;
}

std::shared_ptr<C2LinearBlock> _C2BlockFactory::CreateLinearBlock(
        const std::shared_ptr<C2LinearAllocation> &alloc,
        const std::shared_ptr<_C2BlockPoolData> &data, size_t offset, size_t size) {
    std::shared_ptr<C2Block1D::Impl> impl =
        std::make_shared<C2Block1D::Impl>(alloc, data, offset, size);
    return std::shared_ptr<C2LinearBlock>(new C2LinearBlock(impl, *impl));
}

std::shared_ptr<_C2BlockPoolData> _C2BlockFactory::GetLinearBlockPoolData(
        const C2Block1D &block) {
    if (block.mImpl) {
        return block.mImpl->poolData();
    }
    return nullptr;
}

std::shared_ptr<C2LinearBlock> _C2BlockFactory::CreateLinearBlock(
        const C2Handle *handle) {
    // TODO: get proper allocator? and mutex?
    static std::unique_ptr<C2AllocatorIon> sAllocator = std::make_unique<C2AllocatorIon>(0);

    std::shared_ptr<C2LinearAllocation> alloc;
    if (C2AllocatorIon::isValid(handle)) {
        c2_status_t err = sAllocator->priorLinearAllocation(handle, &alloc);
        if (err == C2_OK) {
            std::shared_ptr<C2LinearBlock> block = _C2BlockFactory::CreateLinearBlock(alloc);
            return block;
        }
    }
    return nullptr;
}

std::shared_ptr<C2LinearBlock> _C2BlockFactory::CreateLinearBlock(
        const C2Handle *cHandle, const std::shared_ptr<BufferPoolData> &data) {
    // TODO: get proper allocator? and mutex?
    static std::unique_ptr<C2AllocatorIon> sAllocator = std::make_unique<C2AllocatorIon>(0);

    std::shared_ptr<C2LinearAllocation> alloc;
    if (C2AllocatorIon::isValid(cHandle)) {
        c2_status_t err = sAllocator->priorLinearAllocation(cHandle, &alloc);
        const std::shared_ptr<C2PooledBlockPoolData> poolData =
                std::make_shared<C2PooledBlockPoolData>(data);
        if (err == C2_OK && poolData) {
            // TODO: config params?
            std::shared_ptr<C2LinearBlock> block =
                    _C2BlockFactory::CreateLinearBlock(alloc, poolData);
            return block;
        }
    }
    return nullptr;
};

/**
 * Wrapped C2Allocator which is injected to buffer pool on behalf of
 * C2BlockPool.
 */
class _C2BufferPoolAllocator : public BufferPoolAllocator {
public:
    _C2BufferPoolAllocator(const std::shared_ptr<C2Allocator> &allocator)
        : mAllocator(allocator) {}

    ~_C2BufferPoolAllocator() override {}

    ResultStatus allocate(const std::vector<uint8_t> &params,
                          std::shared_ptr<BufferPoolAllocation> *alloc,
                          size_t *allocSize) override;

    bool compatible(const std::vector<uint8_t> &newParams,
                    const std::vector<uint8_t> &oldParams) override;

    // Methods for codec2 component (C2BlockPool).
    /**
     * Transforms linear allocation parameters for C2Allocator to parameters
     * for buffer pool.
     *
     * @param capacity      size of linear allocation
     * @param usage         memory usage pattern for linear allocation
     * @param params        allocation parameters for buffer pool
     */
    void getLinearParams(uint32_t capacity, C2MemoryUsage usage,
                         std::vector<uint8_t> *params);

    /**
     * Transforms graphic allocation parameters for C2Allocator to parameters
     * for buffer pool.
     *
     * @param width         width of graphic allocation
     * @param height        height of graphic allocation
     * @param format        color format of graphic allocation
     * @param params        allocation parameter for buffer pool
     */
    void getGraphicParams(uint32_t width, uint32_t height,
                          uint32_t format, C2MemoryUsage usage,
                          std::vector<uint8_t> *params);

    /**
     * Transforms an existing native handle to an C2LinearAllcation.
     * Wrapper to C2Allocator#priorLinearAllocation
     */
    c2_status_t priorLinearAllocation(
            const C2Handle *handle,
            std::shared_ptr<C2LinearAllocation> *c2Allocation);

    /**
     * Transforms an existing native handle to an C2GraphicAllcation.
     * Wrapper to C2Allocator#priorGraphicAllocation
     */
    c2_status_t priorGraphicAllocation(
            const C2Handle *handle,
            std::shared_ptr<C2GraphicAllocation> *c2Allocation);

private:
    static constexpr int kMaxIntParams = 5; // large enough number;

    enum AllocType : uint8_t {
        ALLOC_NONE = 0,

        ALLOC_LINEAR,
        ALLOC_GRAPHIC,
    };

    union AllocParams {
        struct {
            AllocType allocType;
            C2MemoryUsage usage;
            uint32_t params[kMaxIntParams];
        } data;
        uint8_t array[0];

        AllocParams() : data{ALLOC_NONE, {0, 0}, {0}} {}
        AllocParams(C2MemoryUsage usage, uint32_t capacity)
            : data{ALLOC_LINEAR, usage, {[0] = capacity}} {}
        AllocParams(
                C2MemoryUsage usage,
                uint32_t width, uint32_t height, uint32_t format)
                : data{ALLOC_GRAPHIC, usage, {width, height, format}} {}
    };

    const std::shared_ptr<C2Allocator> mAllocator;
};

struct LinearAllocationDtor {
    LinearAllocationDtor(const std::shared_ptr<C2LinearAllocation> &alloc)
        : mAllocation(alloc) {}

    void operator()(BufferPoolAllocation *poolAlloc) { delete poolAlloc; }

    const std::shared_ptr<C2LinearAllocation> mAllocation;
};

struct GraphicAllocationDtor {
    GraphicAllocationDtor(const std::shared_ptr<C2GraphicAllocation> &alloc)
        : mAllocation(alloc) {}

    void operator()(BufferPoolAllocation *poolAlloc) { delete poolAlloc; }

    const std::shared_ptr<C2GraphicAllocation> mAllocation;
};

ResultStatus _C2BufferPoolAllocator::allocate(
        const std::vector<uint8_t>  &params,
        std::shared_ptr<BufferPoolAllocation> *alloc,
        size_t *allocSize) {
    AllocParams c2Params;
    memcpy(&c2Params, params.data(), std::min(sizeof(AllocParams), params.size()));
    c2_status_t status = C2_BAD_VALUE;
    switch(c2Params.data.allocType) {
        case ALLOC_NONE:
            break;
        case ALLOC_LINEAR: {
            std::shared_ptr<C2LinearAllocation> c2Linear;
            status = mAllocator->newLinearAllocation(
                    c2Params.data.params[0], c2Params.data.usage, &c2Linear);
            if (status == C2_OK && c2Linear) {
                BufferPoolAllocation *ptr = new BufferPoolAllocation(c2Linear->handle());
                if (ptr) {
                    *alloc = std::shared_ptr<BufferPoolAllocation>(
                            ptr, LinearAllocationDtor(c2Linear));
                    if (*alloc) {
                        *allocSize = (size_t)c2Params.data.params[0];
                        return ResultStatus::OK;
                    }
                    delete ptr;
                }
                return ResultStatus::NO_MEMORY;
            }
            break;
        }
        case ALLOC_GRAPHIC: {
            std::shared_ptr<C2GraphicAllocation> c2Graphic;
            status = mAllocator->newGraphicAllocation(
                    c2Params.data.params[0],
                    c2Params.data.params[1],
                    c2Params.data.params[2],
                    c2Params.data.usage, &c2Graphic);
            if (status == C2_OK && c2Graphic) {
                BufferPoolAllocation *ptr = new BufferPoolAllocation(c2Graphic->handle());
                if (ptr) {
                    *alloc = std::shared_ptr<BufferPoolAllocation>(
                            ptr, GraphicAllocationDtor(c2Graphic));
                    if (*alloc) {
                        *allocSize = c2Params.data.params[0] * c2Params.data.params[1];
                        return ResultStatus::OK;
                    }
                    delete ptr;
                }
                return ResultStatus::NO_MEMORY;
            }
            break;
        }
        default:
            break;
    }
    return ResultStatus::CRITICAL_ERROR;
}

bool _C2BufferPoolAllocator::compatible(
        const std::vector<uint8_t>  &newParams,
        const std::vector<uint8_t>  &oldParams) {
    AllocParams newAlloc;
    AllocParams oldAlloc;
    memcpy(&newAlloc, newParams.data(), std::min(sizeof(AllocParams), newParams.size()));
    memcpy(&oldAlloc, oldParams.data(), std::min(sizeof(AllocParams), oldParams.size()));

    // TODO: support not exact matching. e.g) newCapacity < oldCapacity
    if (newAlloc.data.allocType == oldAlloc.data.allocType &&
            newAlloc.data.usage.expected == oldAlloc.data.usage.expected) {
        for (int i = 0; i < kMaxIntParams; ++i) {
            if (newAlloc.data.params[i] != oldAlloc.data.params[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void _C2BufferPoolAllocator::getLinearParams(
        uint32_t capacity, C2MemoryUsage usage, std::vector<uint8_t> *params) {
    AllocParams c2Params(usage, capacity);
    params->assign(c2Params.array, c2Params.array + sizeof(AllocParams));
}

void _C2BufferPoolAllocator::getGraphicParams(
        uint32_t width, uint32_t height, uint32_t format, C2MemoryUsage usage,
        std::vector<uint8_t> *params) {
    AllocParams c2Params(usage, width, height, format);
    params->assign(c2Params.array, c2Params.array + sizeof(AllocParams));
}

c2_status_t _C2BufferPoolAllocator::priorLinearAllocation(
        const C2Handle *handle,
        std::shared_ptr<C2LinearAllocation> *c2Allocation) {
    return mAllocator->priorLinearAllocation(handle, c2Allocation);
}

c2_status_t _C2BufferPoolAllocator::priorGraphicAllocation(
        const C2Handle *handle,
        std::shared_ptr<C2GraphicAllocation> *c2Allocation) {
    return mAllocator->priorGraphicAllocation(handle, c2Allocation);
}

class C2PooledBlockPool::Impl {
public:
    Impl(const std::shared_ptr<C2Allocator> &allocator)
            : mInit(C2_OK),
              mBufferPoolManager(ClientManager::getInstance()),
              mAllocator(std::make_shared<_C2BufferPoolAllocator>(allocator)) {
        if (mAllocator && mBufferPoolManager) {
            if (mBufferPoolManager->create(
                    mAllocator, &mConnectionId) == ResultStatus::OK) {
                return;
            }
        }
        mInit = C2_NO_INIT;
    }

    ~Impl() {
        if (mInit == C2_OK) {
            mBufferPoolManager->close(mConnectionId);
        }
    }

    c2_status_t fetchLinearBlock(
            uint32_t capacity, C2MemoryUsage usage,
            std::shared_ptr<C2LinearBlock> *block /* nonnull */) {
        block->reset();
        if (mInit != C2_OK) {
            return mInit;
        }
        std::vector<uint8_t> params;
        mAllocator->getLinearParams(capacity, usage, &params);
        std::shared_ptr<BufferPoolData> bufferPoolData;
        native_handle_t *cHandle = nullptr;
        ResultStatus status = mBufferPoolManager->allocate(
                mConnectionId, params, &cHandle, &bufferPoolData);
        if (status == ResultStatus::OK) {
            std::shared_ptr<C2LinearAllocation> alloc;
            std::shared_ptr<C2PooledBlockPoolData> poolData =
                    std::make_shared<C2PooledBlockPoolData>(bufferPoolData);
            c2_status_t err = mAllocator->priorLinearAllocation(cHandle, &alloc);
            if (err == C2_OK && poolData && alloc) {
                *block = _C2BlockFactory::CreateLinearBlock(alloc, poolData, 0, capacity);
                if (*block) {
                    return C2_OK;
                }
            }
            return C2_NO_MEMORY;
        }
        if (status == ResultStatus::NO_MEMORY) {
            return C2_NO_MEMORY;
        }
        return C2_CORRUPTED;
    }

    c2_status_t fetchGraphicBlock(
            uint32_t width, uint32_t height, uint32_t format,
            C2MemoryUsage usage,
            std::shared_ptr<C2GraphicBlock> *block) {
        block->reset();
        if (mInit != C2_OK) {
            return mInit;
        }
        std::vector<uint8_t> params;
        mAllocator->getGraphicParams(width, height, format, usage, &params);
        std::shared_ptr<BufferPoolData> bufferPoolData;
        native_handle_t *cHandle = nullptr;
        ResultStatus status = mBufferPoolManager->allocate(
                mConnectionId, params, &cHandle, &bufferPoolData);
        if (status == ResultStatus::OK) {
            std::shared_ptr<C2GraphicAllocation> alloc;
            std::shared_ptr<C2PooledBlockPoolData> poolData =
                std::make_shared<C2PooledBlockPoolData>(bufferPoolData);
            c2_status_t err = mAllocator->priorGraphicAllocation(
                    cHandle, &alloc);
            if (err == C2_OK && poolData && alloc) {
                *block = _C2BlockFactory::CreateGraphicBlock(
                        alloc, poolData, C2Rect(width, height));
                if (*block) {
                    return C2_OK;
                }
            }
            return C2_NO_MEMORY;
        }
        if (status == ResultStatus::NO_MEMORY) {
            return C2_NO_MEMORY;
        }
        return C2_CORRUPTED;
    }

    ConnectionId getConnectionId() {
        return mInit != C2_OK ? INVALID_CONNECTIONID : mConnectionId;
    }

private:
    c2_status_t mInit;
    const android::sp<ClientManager> mBufferPoolManager;
    ConnectionId mConnectionId; // locally
    const std::shared_ptr<_C2BufferPoolAllocator> mAllocator;
};

C2PooledBlockPool::C2PooledBlockPool(
        const std::shared_ptr<C2Allocator> &allocator, const local_id_t localId)
        : mAllocator(allocator), mLocalId(localId), mImpl(new Impl(allocator)) {}

C2PooledBlockPool::~C2PooledBlockPool() {
}

c2_status_t C2PooledBlockPool::fetchLinearBlock(
        uint32_t capacity,
        C2MemoryUsage usage,
        std::shared_ptr<C2LinearBlock> *block /* nonnull */) {
    if (mImpl) {
        return mImpl->fetchLinearBlock(capacity, usage, block);
    }
    return C2_CORRUPTED;
}

c2_status_t C2PooledBlockPool::fetchGraphicBlock(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        C2MemoryUsage usage,
        std::shared_ptr<C2GraphicBlock> *block) {
    if (mImpl) {
        return mImpl->fetchGraphicBlock(width, height, format, usage, block);
    }
    return C2_CORRUPTED;
}

int64_t C2PooledBlockPool::getConnectionId() {
    if (mImpl) {
        return mImpl->getConnectionId();
    }
    return 0;
}

/* ========================================== 2D BLOCK ========================================= */

/**
 * Implementation that is shared between all 2D blocks and views.
 *
 * For blocks' Impl's crop is always the allotted crop, even if it is a sub block.
 *
 * For views' Impl's crop is the mapped portion - which for now is always the
 * allotted crop.
 */
class C2_HIDE _C2Block2DImpl : public _C2PlanarSectionAspect {
public:
    /**
     * Impl's crop is always the or part of the allotted crop of the allocation.
     */
    _C2Block2DImpl(const std::shared_ptr<C2GraphicAllocation> &alloc,
            const std::shared_ptr<_C2BlockPoolData> &poolData = nullptr,
            const C2Rect &allottedCrop = C2Rect(~0u, ~0u))
        : _C2PlanarSectionAspect(alloc.get(), allottedCrop),
          mAllocation(alloc),
          mPoolData(poolData) { }

    virtual ~_C2Block2DImpl() = default;

    /** returns pool data  */
    std::shared_ptr<_C2BlockPoolData> poolData() const {
        return mPoolData;
    }

    /** returns native handle */
    const C2Handle *handle() const {
        return mAllocation ? mAllocation->handle() : nullptr;
    }

    /** returns the allocator's ID */
    C2Allocator::id_t getAllocatorId() const {
        // BAD_ID can only happen if this Impl class is initialized for a view - never for a block.
        return mAllocation ? mAllocation->getAllocatorId() : C2Allocator::BAD_ID;
    }

    std::shared_ptr<C2GraphicAllocation> getAllocation() const {
        return mAllocation;
    }

private:
    std::shared_ptr<C2GraphicAllocation> mAllocation;
    std::shared_ptr<_C2BlockPoolData> mPoolData;
};

class C2_HIDE _C2MappingBlock2DImpl
    : public _C2Block2DImpl, public std::enable_shared_from_this<_C2MappingBlock2DImpl> {
public:
    using _C2Block2DImpl::_C2Block2DImpl;

    virtual ~_C2MappingBlock2DImpl() override = default;

    /**
     * This class contains the mapped data pointer, and the potential error.
     */
    struct Mapped {
    private:
        friend class _C2MappingBlock2DImpl;

        Mapped(const std::shared_ptr<_C2Block2DImpl> &impl, bool writable, C2Fence *fence __unused)
            : mImpl(impl), mWritable(writable) {
            memset(mData, 0, sizeof(mData));
            const C2Rect crop = mImpl->crop();
            // gralloc requires mapping the whole region of interest as we cannot
            // map multiple regions
            mError = mImpl->getAllocation()->map(
                    crop,
                    { C2MemoryUsage::CPU_READ, writable ? C2MemoryUsage::CPU_WRITE : 0 },
                    nullptr,
                    &mLayout,
                    mData);
            if (mError != C2_OK) {
                memset(&mLayout, 0, sizeof(mLayout));
                memset(mData, 0, sizeof(mData));
                memset(mOffsetData, 0, sizeof(mData));
            } else {
                // TODO: validate plane layout and
                // adjust data pointers to the crop region's top left corner.
                // fail if it is not on a subsampling boundary
                for (size_t planeIx = 0; planeIx < mLayout.numPlanes; ++planeIx) {
                    const uint32_t colSampling = mLayout.planes[planeIx].colSampling;
                    const uint32_t rowSampling = mLayout.planes[planeIx].rowSampling;
                    if (crop.left % colSampling || crop.right() % colSampling
                            || crop.top % rowSampling || crop.bottom() % rowSampling) {
                        // cannot calculate data pointer
                        mImpl->getAllocation()->unmap(mData, crop, nullptr);
                        memset(&mLayout, 0, sizeof(mLayout));
                        memset(mData, 0, sizeof(mData));
                        memset(mOffsetData, 0, sizeof(mData));
                        mError = C2_BAD_VALUE;
                        return;
                    }
                    mOffsetData[planeIx] =
                        mData[planeIx] + (ssize_t)crop.left * mLayout.planes[planeIx].colInc
                                + (ssize_t)crop.top * mLayout.planes[planeIx].rowInc;
                }
            }
        }

        explicit Mapped(c2_status_t error)
            : mImpl(nullptr), mWritable(false), mError(error) {
            // CHECK(error != C2_OK);
            memset(&mLayout, 0, sizeof(mLayout));
            memset(mData, 0, sizeof(mData));
            memset(mOffsetData, 0, sizeof(mData));
        }

    public:
        ~Mapped() {
            if (mData[0] != nullptr) {
                mImpl->getAllocation()->unmap(mData, mImpl->crop(), nullptr);
            }
        }

        /** returns mapping status */
        c2_status_t error() const { return mError; }

        /** returns data pointer */
        uint8_t *const *data() const { return mOffsetData; }

        /** returns the plane layout */
        C2PlanarLayout layout() const { return mLayout; }

        /** returns whether the mapping is writable */
        bool writable() const { return mWritable; }

    private:
        const std::shared_ptr<_C2Block2DImpl> mImpl;
        bool mWritable;
        c2_status_t mError;
        uint8_t *mData[C2PlanarLayout::MAX_NUM_PLANES];
        uint8_t *mOffsetData[C2PlanarLayout::MAX_NUM_PLANES];
        C2PlanarLayout mLayout;
    };

    /**
     * Maps the allotted region.
     *
     * If already mapped and it is currently in use, returns the existing mapping.
     * If fence is provided, an acquire fence is stored there.
     */
    std::shared_ptr<Mapped> map(bool writable, C2Fence *fence) {
        std::lock_guard<std::mutex> lock(mMappedLock);
        std::shared_ptr<Mapped> existing = mMapped.lock();
        if (!existing) {
            existing = std::shared_ptr<Mapped>(new Mapped(shared_from_this(), writable, fence));
            mMapped = existing;
        } else {
            // if we mapped the region read-only, we cannot remap it read-write
            if (writable && !existing->writable()) {
                existing = std::shared_ptr<Mapped>(new Mapped(C2_CANNOT_DO));
            }
            if (fence != nullptr) {
                *fence = C2Fence();
            }
        }
        return existing;
    }

private:
    std::weak_ptr<Mapped> mMapped;
    std::mutex mMappedLock;
};

class C2_HIDE _C2MappedBlock2DImpl : public _C2Block2DImpl {
public:
    _C2MappedBlock2DImpl(const _C2Block2DImpl &impl,
                         std::shared_ptr<_C2MappingBlock2DImpl::Mapped> mapping)
        : _C2Block2DImpl(impl), mMapping(mapping) {
    }

    virtual ~_C2MappedBlock2DImpl() override = default;

    std::shared_ptr<_C2MappingBlock2DImpl::Mapped> mapping() const { return mMapping; }

private:
    std::shared_ptr<_C2MappingBlock2DImpl::Mapped> mMapping;
};

/**
 * Block implementation.
 */
class C2Block2D::Impl : public _C2MappingBlock2DImpl {
public:
    using _C2MappingBlock2DImpl::_C2MappingBlock2DImpl;
    virtual ~Impl() override = default;
};

const C2Handle *C2Block2D::handle() const {
    return mImpl->handle();
}

C2Allocator::id_t C2Block2D::getAllocatorId() const {
    return mImpl->getAllocatorId();
}

C2Block2D::C2Block2D(std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section)
    // always clamp subsection to parent (impl) crop for safety
    : _C2PlanarSectionAspect(impl.get(), section.crop()), mImpl(impl) {
}

/**
 * Graphic view implementation.
 *
 * range of Impl is the mapped range of the underlying allocation. range of View is the current
 * crop.
 */
class C2GraphicView::Impl : public _C2MappedBlock2DImpl {
public:
    using _C2MappedBlock2DImpl::_C2MappedBlock2DImpl;
    virtual ~Impl() override = default;
};

C2GraphicView::C2GraphicView(std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section)
    : _C2EditablePlanarSectionAspect(impl.get(), section.crop()), mImpl(impl) {
}

const uint8_t *const *C2GraphicView::data() const {
    return mImpl->mapping()->data();
}

uint8_t *const *C2GraphicView::data() {
    return mImpl->mapping()->data();
}

const C2PlanarLayout C2GraphicView::layout() const {
    return mImpl->mapping()->layout();
}

const C2GraphicView C2GraphicView::subView(const C2Rect &rect) const {
    return C2GraphicView(mImpl, C2PlanarSection(*mImpl, rect));
}

C2GraphicView C2GraphicView::subView(const C2Rect &rect) {
    return C2GraphicView(mImpl, C2PlanarSection(*mImpl, rect));
}

c2_status_t C2GraphicView::error() const {
    return mImpl->mapping()->error();
}

/**
 * Const graphic block implementation.
 */
C2ConstGraphicBlock::C2ConstGraphicBlock(
        std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section, C2Fence fence)
    : C2Block2D(impl, section), mFence(fence) { }

C2Acquirable<const C2GraphicView> C2ConstGraphicBlock::map() const {
    C2Fence fence;
    std::shared_ptr<_C2MappingBlock2DImpl::Mapped> mapping =
        mImpl->map(false /* writable */, &fence);
    std::shared_ptr<GraphicViewBuddy::Impl> gvi =
        std::shared_ptr<GraphicViewBuddy::Impl>(new GraphicViewBuddy::Impl(*mImpl, mapping));
    return AcquirableConstGraphicViewBuddy(
            mapping->error(), fence, GraphicViewBuddy(gvi, C2PlanarSection(*mImpl, crop())));
}

C2ConstGraphicBlock C2ConstGraphicBlock::subBlock(const C2Rect &rect) const {
    return C2ConstGraphicBlock(mImpl, C2PlanarSection(*mImpl, crop().intersect(rect)), mFence);
}

/**
 * Graphic block implementation.
 */
C2GraphicBlock::C2GraphicBlock(
    std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section)
    : C2Block2D(impl, section) { }

C2Acquirable<C2GraphicView> C2GraphicBlock::map() {
    C2Fence fence;
    std::shared_ptr<_C2MappingBlock2DImpl::Mapped> mapping =
        mImpl->map(true /* writable */, &fence);
    std::shared_ptr<GraphicViewBuddy::Impl> gvi =
        std::shared_ptr<GraphicViewBuddy::Impl>(new GraphicViewBuddy::Impl(*mImpl, mapping));
    return AcquirableGraphicViewBuddy(
            mapping->error(), fence, GraphicViewBuddy(gvi, C2PlanarSection(*mImpl, crop())));
}

C2ConstGraphicBlock C2GraphicBlock::share(const C2Rect &crop, C2Fence fence) {
    return ConstGraphicBlockBuddy(mImpl, C2PlanarSection(*mImpl, crop), fence);
}

/**
 * Basic block pool implementations.
 */
C2BasicGraphicBlockPool::C2BasicGraphicBlockPool(
        const std::shared_ptr<C2Allocator> &allocator)
  : mAllocator(allocator) {}

c2_status_t C2BasicGraphicBlockPool::fetchGraphicBlock(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        C2MemoryUsage usage,
        std::shared_ptr<C2GraphicBlock> *block /* nonnull */) {
    block->reset();

    std::shared_ptr<C2GraphicAllocation> alloc;
    c2_status_t err = mAllocator->newGraphicAllocation(width, height, format, usage, &alloc);
    if (err != C2_OK) {
        return err;
    }

    *block = _C2BlockFactory::CreateGraphicBlock(alloc);

    return C2_OK;
}

std::shared_ptr<C2GraphicBlock> _C2BlockFactory::CreateGraphicBlock(
        const std::shared_ptr<C2GraphicAllocation> &alloc,
        const std::shared_ptr<_C2BlockPoolData> &data, const C2Rect &allottedCrop) {
    std::shared_ptr<C2Block2D::Impl> impl =
        std::make_shared<C2Block2D::Impl>(alloc, data, allottedCrop);
    return std::shared_ptr<C2GraphicBlock>(new C2GraphicBlock(impl, *impl));
}

std::shared_ptr<_C2BlockPoolData> _C2BlockFactory::GetGraphicBlockPoolData(
        const C2Block2D &block) {
    if (block.mImpl) {
        return block.mImpl->poolData();
    }
    return nullptr;
}

std::shared_ptr<C2GraphicBlock> _C2BlockFactory::CreateGraphicBlock(
        const C2Handle *cHandle,
        const std::shared_ptr<BufferPoolData> &data) {
    // TODO: get proper allocator? and mutex?
    static std::unique_ptr<C2AllocatorGralloc> sAllocator = std::make_unique<C2AllocatorGralloc>(0);

    std::shared_ptr<C2GraphicAllocation> alloc;
    if (C2AllocatorGralloc::isValid(cHandle)) {
        c2_status_t err = sAllocator->priorGraphicAllocation(cHandle, &alloc);
        const std::shared_ptr<C2PooledBlockPoolData> poolData =
                std::make_shared<C2PooledBlockPoolData>(data);
        if (err == C2_OK && poolData) {
            // TODO: config setup?
            std::shared_ptr<C2GraphicBlock> block =
                    _C2BlockFactory::CreateGraphicBlock(alloc, poolData);
            return block;
        }
    }
    return nullptr;
};


/* ========================================== BUFFER ========================================= */

class C2BufferData::Impl {
public:
    explicit Impl(const std::vector<C2ConstLinearBlock> &blocks)
        : mType(blocks.size() == 1 ? LINEAR : LINEAR_CHUNKS),
          mLinearBlocks(blocks) {
    }

    explicit Impl(const std::vector<C2ConstGraphicBlock> &blocks)
        : mType(blocks.size() == 1 ? GRAPHIC : GRAPHIC_CHUNKS),
          mGraphicBlocks(blocks) {
    }

    type_t type() const { return mType; }
    const std::vector<C2ConstLinearBlock> &linearBlocks() const { return mLinearBlocks; }
    const std::vector<C2ConstGraphicBlock> &graphicBlocks() const { return mGraphicBlocks; }

private:
    type_t mType;
    std::vector<C2ConstLinearBlock> mLinearBlocks;
    std::vector<C2ConstGraphicBlock> mGraphicBlocks;
};

C2BufferData::C2BufferData(const std::vector<C2ConstLinearBlock> &blocks) : mImpl(new Impl(blocks)) {}
C2BufferData::C2BufferData(const std::vector<C2ConstGraphicBlock> &blocks) : mImpl(new Impl(blocks)) {}

C2BufferData::type_t C2BufferData::type() const { return mImpl->type(); }

const std::vector<C2ConstLinearBlock> C2BufferData::linearBlocks() const {
    return mImpl->linearBlocks();
}

const std::vector<C2ConstGraphicBlock> C2BufferData::graphicBlocks() const {
    return mImpl->graphicBlocks();
}

class C2Buffer::Impl {
public:
    Impl(C2Buffer *thiz, const std::vector<C2ConstLinearBlock> &blocks)
        : mThis(thiz), mData(blocks) {}
    Impl(C2Buffer *thiz, const std::vector<C2ConstGraphicBlock> &blocks)
        : mThis(thiz), mData(blocks) {}

    ~Impl() {
        for (const auto &pair : mNotify) {
            pair.first(mThis, pair.second);
        }
    }

    const C2BufferData &data() const { return mData; }

    c2_status_t registerOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg) {
        auto it = std::find_if(
                mNotify.begin(), mNotify.end(),
                [onDestroyNotify, arg] (const auto &pair) {
                    return pair.first == onDestroyNotify && pair.second == arg;
                });
        if (it != mNotify.end()) {
            return C2_DUPLICATE;
        }
        mNotify.emplace_back(onDestroyNotify, arg);
        return C2_OK;
    }

    c2_status_t unregisterOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg) {
        auto it = std::find_if(
                mNotify.begin(), mNotify.end(),
                [onDestroyNotify, arg] (const auto &pair) {
                    return pair.first == onDestroyNotify && pair.second == arg;
                });
        if (it == mNotify.end()) {
            return C2_NOT_FOUND;
        }
        mNotify.erase(it);
        return C2_OK;
    }

    std::vector<std::shared_ptr<const C2Info>> info() const {
        std::vector<std::shared_ptr<const C2Info>> result(mInfos.size());
        std::transform(
                mInfos.begin(), mInfos.end(), result.begin(),
                [] (const auto &elem) { return elem.second; });
        return result;
    }

    c2_status_t setInfo(const std::shared_ptr<C2Info> &info) {
        // To "update" you need to erase the existing one if any, and then insert.
        (void) mInfos.erase(info->coreIndex());
        (void) mInfos.insert({ info->coreIndex(), info });
        return C2_OK;
    }

    bool hasInfo(C2Param::Type index) const {
        return mInfos.count(index.coreIndex()) > 0;
    }

    std::shared_ptr<const C2Info> getInfo(C2Param::Type index) const {
        auto it = mInfos.find(index.coreIndex());
        if (it == mInfos.end()) {
            return nullptr;
        }
        return std::const_pointer_cast<const C2Info>(it->second);
    }

    std::shared_ptr<C2Info> removeInfo(C2Param::Type index) {
        auto it = mInfos.find(index.coreIndex());
        if (it == mInfos.end()) {
            return nullptr;
        }
        std::shared_ptr<C2Info> ret = it->second;
        (void) mInfos.erase(it);
        return ret;
    }

private:
    C2Buffer * const mThis;
    BufferDataBuddy mData;
    std::map<C2Param::CoreIndex, std::shared_ptr<C2Info>> mInfos;
    std::list<std::pair<OnDestroyNotify, void *>> mNotify;
};

C2Buffer::C2Buffer(const std::vector<C2ConstLinearBlock> &blocks)
    : mImpl(new Impl(this, blocks)) {}

C2Buffer::C2Buffer(const std::vector<C2ConstGraphicBlock> &blocks)
    : mImpl(new Impl(this, blocks)) {}

const C2BufferData C2Buffer::data() const { return mImpl->data(); }

c2_status_t C2Buffer::registerOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg) {
    return mImpl->registerOnDestroyNotify(onDestroyNotify, arg);
}

c2_status_t C2Buffer::unregisterOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg) {
    return mImpl->unregisterOnDestroyNotify(onDestroyNotify, arg);
}

const std::vector<std::shared_ptr<const C2Info>> C2Buffer::info() const {
    return mImpl->info();
}

c2_status_t C2Buffer::setInfo(const std::shared_ptr<C2Info> &info) {
    return mImpl->setInfo(info);
}

bool C2Buffer::hasInfo(C2Param::Type index) const {
    return mImpl->hasInfo(index);
}

std::shared_ptr<const C2Info> C2Buffer::getInfo(C2Param::Type index) const {
    return mImpl->getInfo(index);
}

std::shared_ptr<C2Info> C2Buffer::removeInfo(C2Param::Type index) {
    return mImpl->removeInfo(index);
}

// static
std::shared_ptr<C2Buffer> C2Buffer::CreateLinearBuffer(const C2ConstLinearBlock &block) {
    return std::shared_ptr<C2Buffer>(new C2Buffer({ block }));
}

// static
std::shared_ptr<C2Buffer> C2Buffer::CreateGraphicBuffer(const C2ConstGraphicBlock &block) {
    return std::shared_ptr<C2Buffer>(new C2Buffer({ block }));
}

