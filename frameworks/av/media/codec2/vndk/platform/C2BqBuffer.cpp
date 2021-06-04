/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "C2BqBuffer"
#include <utils/Log.h>

#include <ui/BufferQueueDefs.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>

#include <types.h>

#include <hidl/HidlSupport.h>

#include <C2AllocatorGralloc.h>
#include <C2BqBufferPriv.h>
#include <C2BlockInternal.h>

#include <list>
#include <map>
#include <mutex>

using ::android::BufferQueueDefs::NUM_BUFFER_SLOTS;
using ::android::C2AllocatorGralloc;
using ::android::C2AndroidMemoryUsage;
using ::android::Fence;
using ::android::GraphicBuffer;
using ::android::sp;
using ::android::status_t;
using ::android::wp;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;

using HBuffer = ::android::hardware::graphics::common::V1_2::HardwareBuffer;
using HStatus = ::android::hardware::graphics::bufferqueue::V2_0::Status;
using ::android::hardware::graphics::bufferqueue::V2_0::utils::b2h;
using ::android::hardware::graphics::bufferqueue::V2_0::utils::h2b;
using ::android::hardware::graphics::bufferqueue::V2_0::utils::HFenceWrapper;

using HGraphicBufferProducer = ::android::hardware::graphics::bufferqueue::V2_0
        ::IGraphicBufferProducer;

struct C2BufferQueueBlockPoolData : public _C2BlockPoolData {

    bool held;
    bool local;
    uint32_t generation;
    uint64_t bqId;
    int32_t bqSlot;
    bool transfer; // local transfer to remote
    bool attach; // attach on remote
    bool display; // display on remote;
    std::weak_ptr<int> owner;
    sp<HGraphicBufferProducer> igbp;
    std::shared_ptr<C2BufferQueueBlockPool::Impl> localPool;
    mutable std::mutex lock;

    virtual type_t getType() const override {
        return TYPE_BUFFERQUEUE;
    }

    // Create a remote BlockPoolData.
    C2BufferQueueBlockPoolData(
            uint32_t generation, uint64_t bqId, int32_t bqSlot,
            const std::shared_ptr<int> &owner,
            const sp<HGraphicBufferProducer>& producer);

    // Create a local BlockPoolData.
    C2BufferQueueBlockPoolData(
            uint32_t generation, uint64_t bqId, int32_t bqSlot,
            const std::shared_ptr<C2BufferQueueBlockPool::Impl>& pool);

    virtual ~C2BufferQueueBlockPoolData() override;

    int migrate(const sp<HGraphicBufferProducer>& producer,
                uint32_t toGeneration, uint64_t toBqId,
                sp<GraphicBuffer> *buffers, uint32_t oldGeneration);
};

bool _C2BlockFactory::GetBufferQueueData(
        const std::shared_ptr<const _C2BlockPoolData>& data,
        uint32_t* generation, uint64_t* bqId, int32_t* bqSlot) {
    if (data && data->getType() == _C2BlockPoolData::TYPE_BUFFERQUEUE) {
        if (generation) {
            const std::shared_ptr<const C2BufferQueueBlockPoolData> poolData =
                    std::static_pointer_cast<const C2BufferQueueBlockPoolData>(data);
            std::scoped_lock<std::mutex> lock(poolData->lock);
            *generation = poolData->generation;
            if (bqId) {
                *bqId = poolData->bqId;
            }
            if (bqSlot) {
                *bqSlot = poolData->bqSlot;
            }
        }
        return true;
    }
    return false;
}

bool _C2BlockFactory::HoldBlockFromBufferQueue(
        const std::shared_ptr<_C2BlockPoolData>& data,
        const std::shared_ptr<int>& owner,
        const sp<HGraphicBufferProducer>& igbp) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    if (!poolData->local) {
        poolData->owner = owner;
        poolData->igbp = igbp;
    }
    if (poolData->held) {
        poolData->held = true;
        return false;
    }
    poolData->held = true;
    return true;
}

bool _C2BlockFactory::BeginTransferBlockToClient(
        const std::shared_ptr<_C2BlockPoolData>& data) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    poolData->transfer = true;
    return true;
}

bool _C2BlockFactory::EndTransferBlockToClient(
        const std::shared_ptr<_C2BlockPoolData>& data,
        bool transfer) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    poolData->transfer = false;
    if (transfer) {
        poolData->held = false;
    }
    return true;
}

bool _C2BlockFactory::BeginAttachBlockToBufferQueue(
        const std::shared_ptr<_C2BlockPoolData>& data) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    if (poolData->local || poolData->display ||
        poolData->attach || !poolData->held) {
        return false;
    }
    if (poolData->bqId == 0) {
        return false;
    }
    poolData->attach = true;
    return true;
}

// if display was tried during attach, buffer should be retired ASAP.
bool _C2BlockFactory::EndAttachBlockToBufferQueue(
        const std::shared_ptr<_C2BlockPoolData>& data,
        const std::shared_ptr<int>& owner,
        const sp<HGraphicBufferProducer>& igbp,
        uint32_t generation,
        uint64_t bqId,
        int32_t bqSlot) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    if (poolData->local || !poolData->attach ) {
        return false;
    }
    if (poolData->display) {
        poolData->attach = false;
        poolData->held = false;
        return false;
    }
    poolData->attach = false;
    poolData->held = true;
    poolData->owner = owner;
    poolData->igbp = igbp;
    poolData->generation = generation;
    poolData->bqId = bqId;
    poolData->bqSlot = bqSlot;
    return true;
}

bool _C2BlockFactory::DisplayBlockToBufferQueue(
        const std::shared_ptr<_C2BlockPoolData>& data) {
    const std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
            std::static_pointer_cast<C2BufferQueueBlockPoolData>(data);
    std::scoped_lock<std::mutex> lock(poolData->lock);
    if (poolData->local || poolData->display || !poolData->held) {
        return false;
    }
    if (poolData->bqId == 0) {
        return false;
    }
    poolData->display = true;
    if (poolData->attach) {
        return false;
    }
    poolData->held = false;
    return true;
}

std::shared_ptr<C2GraphicBlock> _C2BlockFactory::CreateGraphicBlock(
        const C2Handle *handle) {
    // TODO: get proper allocator? and mutex?
    static std::unique_ptr<C2AllocatorGralloc> sAllocator = std::make_unique<C2AllocatorGralloc>(0);

    std::shared_ptr<C2GraphicAllocation> alloc;
    if (C2AllocatorGralloc::isValid(handle)) {
        uint32_t width;
        uint32_t height;
        uint32_t format;
        uint64_t usage;
        uint32_t stride;
        uint32_t generation;
        uint64_t bqId;
        uint32_t bqSlot;
        android::_UnwrapNativeCodec2GrallocMetadata(
                handle, &width, &height, &format, &usage, &stride, &generation, &bqId, &bqSlot);
        c2_status_t err = sAllocator->priorGraphicAllocation(handle, &alloc);
        if (err == C2_OK) {
            std::shared_ptr<C2GraphicBlock> block;
            if (bqId || bqSlot) {
                // BQBBP
                std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
                        std::make_shared<C2BufferQueueBlockPoolData>(generation,
                                                                     bqId,
                                                                     (int32_t)bqSlot,
                                                                     nullptr,
                                                                     nullptr);
                block = _C2BlockFactory::CreateGraphicBlock(alloc, poolData);
            } else {
                block = _C2BlockFactory::CreateGraphicBlock(alloc);
            }
            return block;
        }
    }
    return nullptr;
}

namespace {

int64_t getTimestampNow() {
    int64_t stamp;
    struct timespec ts;
    // TODO: CLOCK_MONOTONIC_COARSE?
    clock_gettime(CLOCK_MONOTONIC, &ts);
    stamp = ts.tv_nsec / 1000;
    stamp += (ts.tv_sec * 1000000LL);
    return stamp;
}

bool getGenerationNumber(const sp<HGraphicBufferProducer> &producer,
                         uint32_t *generation) {
    status_t status{};
    int slot{};
    bool bufferNeedsReallocation{};
    sp<Fence> fence = new Fence();

    using Input = HGraphicBufferProducer::DequeueBufferInput;
    using Output = HGraphicBufferProducer::DequeueBufferOutput;
    Return<void> transResult = producer->dequeueBuffer(
            Input{640, 480, HAL_PIXEL_FORMAT_YCBCR_420_888, 0},
            [&status, &slot, &bufferNeedsReallocation, &fence]
            (HStatus hStatus, int32_t hSlot, Output const& hOutput) {
                slot = static_cast<int>(hSlot);
                if (!h2b(hStatus, &status) || !h2b(hOutput.fence, &fence)) {
                    status = ::android::BAD_VALUE;
                } else {
                    bufferNeedsReallocation =
                            hOutput.bufferNeedsReallocation;
                }
            });
    if (!transResult.isOk() || status != android::OK) {
        return false;
    }
    HFenceWrapper hFenceWrapper{};
    if (!b2h(fence, &hFenceWrapper)) {
        (void)producer->detachBuffer(static_cast<int32_t>(slot)).isOk();
        ALOGE("Invalid fence received from dequeueBuffer.");
        return false;
    }
    sp<GraphicBuffer> slotBuffer = new GraphicBuffer();
    // N.B. This assumes requestBuffer# returns an existing allocation
    // instead of a new allocation.
    transResult = producer->requestBuffer(
            slot,
            [&status, &slotBuffer, &generation](
                    HStatus hStatus,
                    HBuffer const& hBuffer,
                    uint32_t generationNumber){
                if (h2b(hStatus, &status) &&
                        h2b(hBuffer, &slotBuffer) &&
                        slotBuffer) {
                    *generation = generationNumber;
                    slotBuffer->setGenerationNumber(generationNumber);
                } else {
                    status = android::BAD_VALUE;
                }
            });
    if (!transResult.isOk()) {
        return false;
    } else if (status != android::NO_ERROR) {
        (void)producer->detachBuffer(static_cast<int32_t>(slot)).isOk();
        return false;
    }
    (void)producer->detachBuffer(static_cast<int32_t>(slot)).isOk();
    return true;
}

};

class C2BufferQueueBlockPool::Impl
        : public std::enable_shared_from_this<C2BufferQueueBlockPool::Impl> {
private:
    c2_status_t fetchFromIgbp_l(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            C2MemoryUsage usage,
            std::shared_ptr<C2GraphicBlock> *block /* nonnull */) {
        // We have an IGBP now.
        C2AndroidMemoryUsage androidUsage = usage;
        status_t status{};
        int slot{};
        bool bufferNeedsReallocation{};
        sp<Fence> fence = new Fence();
        ALOGV("tries to dequeue buffer");

        { // Call dequeueBuffer().
            using Input = HGraphicBufferProducer::DequeueBufferInput;
            using Output = HGraphicBufferProducer::DequeueBufferOutput;
            Return<void> transResult = mProducer->dequeueBuffer(
                    Input{
                        width,
                        height,
                        format,
                        androidUsage.asGrallocUsage()},
                    [&status, &slot, &bufferNeedsReallocation,
                     &fence](HStatus hStatus,
                             int32_t hSlot,
                             Output const& hOutput) {
                        slot = static_cast<int>(hSlot);
                        if (!h2b(hStatus, &status) ||
                                !h2b(hOutput.fence, &fence)) {
                            status = ::android::BAD_VALUE;
                        } else {
                            bufferNeedsReallocation =
                                    hOutput.bufferNeedsReallocation;
                        }
                    });
            if (!transResult.isOk() || status != android::OK) {
                if (transResult.isOk()) {
                    ++mDqFailure;
                    if (status == android::INVALID_OPERATION ||
                        status == android::TIMED_OUT ||
                        status == android::WOULD_BLOCK) {
                        // Dequeue buffer is blocked temporarily. Retrying is
                        // required.
                        return C2_BLOCKING;
                    }
                }
                ALOGD("cannot dequeue buffer %d", status);
                return C2_BAD_VALUE;
            }
            mDqFailure = 0;
            mLastDqTs = getTimestampNow();
        }
        HFenceWrapper hFenceWrapper{};
        if (!b2h(fence, &hFenceWrapper)) {
            ALOGE("Invalid fence received from dequeueBuffer.");
            return C2_BAD_VALUE;
        }
        ALOGV("dequeued a buffer successfully");
        if (fence) {
            static constexpr int kFenceWaitTimeMs = 10;

            status_t status = fence->wait(kFenceWaitTimeMs);
            if (status == -ETIME) {
                // fence is not signalled yet.
                (void)mProducer->cancelBuffer(slot, hFenceWrapper.getHandle()).isOk();
                return C2_BLOCKING;
            }
            if (status != android::NO_ERROR) {
                ALOGD("buffer fence wait error %d", status);
                (void)mProducer->cancelBuffer(slot, hFenceWrapper.getHandle()).isOk();
                return C2_BAD_VALUE;
            } else if (mRenderCallback) {
                nsecs_t signalTime = fence->getSignalTime();
                if (signalTime >= 0 && signalTime < INT64_MAX) {
                    mRenderCallback(mProducerId, slot, signalTime);
                } else {
                    ALOGV("got fence signal time of %lld", (long long)signalTime);
                }
            }
        }

        sp<GraphicBuffer> &slotBuffer = mBuffers[slot];
        uint32_t outGeneration;
        if (bufferNeedsReallocation || !slotBuffer) {
            if (!slotBuffer) {
                slotBuffer = new GraphicBuffer();
            }
            // N.B. This assumes requestBuffer# returns an existing allocation
            // instead of a new allocation.
            Return<void> transResult = mProducer->requestBuffer(
                    slot,
                    [&status, &slotBuffer, &outGeneration](
                            HStatus hStatus,
                            HBuffer const& hBuffer,
                            uint32_t generationNumber){
                        if (h2b(hStatus, &status) &&
                                h2b(hBuffer, &slotBuffer) &&
                                slotBuffer) {
                            slotBuffer->setGenerationNumber(generationNumber);
                            outGeneration = generationNumber;
                        } else {
                            status = android::BAD_VALUE;
                        }
                    });
            if (!transResult.isOk()) {
                slotBuffer.clear();
                return C2_BAD_VALUE;
            } else if (status != android::NO_ERROR) {
                slotBuffer.clear();
                (void)mProducer->cancelBuffer(slot, hFenceWrapper.getHandle()).isOk();
                return C2_BAD_VALUE;
            }
            if (mGeneration == 0) {
                // getting generation # lazily due to dequeue failure.
                mGeneration = outGeneration;
            }
        }
        if (slotBuffer) {
            ALOGV("buffer wraps %llu %d", (unsigned long long)mProducerId, slot);
            C2Handle *c2Handle = android::WrapNativeCodec2GrallocHandle(
                    slotBuffer->handle,
                    slotBuffer->width,
                    slotBuffer->height,
                    slotBuffer->format,
                    slotBuffer->usage,
                    slotBuffer->stride,
                    slotBuffer->getGenerationNumber(),
                    mProducerId, slot);
            if (c2Handle) {
                std::shared_ptr<C2GraphicAllocation> alloc;
                c2_status_t err = mAllocator->priorGraphicAllocation(c2Handle, &alloc);
                if (err != C2_OK) {
                    return err;
                }
                std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
                        std::make_shared<C2BufferQueueBlockPoolData>(
                                slotBuffer->getGenerationNumber(),
                                mProducerId, slot,
                                shared_from_this());
                mPoolDatas[slot] = poolData;
                *block = _C2BlockFactory::CreateGraphicBlock(alloc, poolData);
                return C2_OK;
            }
            // Block was not created. call requestBuffer# again next time.
            slotBuffer.clear();
            (void)mProducer->cancelBuffer(slot, hFenceWrapper.getHandle()).isOk();
        }
        return C2_BAD_VALUE;
    }

public:
    Impl(const std::shared_ptr<C2Allocator> &allocator)
        : mInit(C2_OK), mProducerId(0), mGeneration(0),
          mDqFailure(0), mLastDqTs(0), mLastDqLogTs(0),
          mAllocator(allocator) {
    }

    ~Impl() {
        bool noInit = false;
        for (int i = 0; i < NUM_BUFFER_SLOTS; ++i) {
            if (!noInit && mProducer) {
                Return<HStatus> transResult =
                        mProducer->detachBuffer(static_cast<int32_t>(i));
                noInit = !transResult.isOk() ||
                         static_cast<HStatus>(transResult) == HStatus::NO_INIT;
            }
            mBuffers[i].clear();
        }
    }

    c2_status_t fetchGraphicBlock(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            C2MemoryUsage usage,
            std::shared_ptr<C2GraphicBlock> *block /* nonnull */) {
        block->reset();
        if (mInit != C2_OK) {
            return mInit;
        }

        static int kMaxIgbpRetryDelayUs = 10000;

        std::unique_lock<std::mutex> lock(mMutex);
        if (mLastDqLogTs == 0) {
            mLastDqLogTs = getTimestampNow();
        } else {
            int64_t now = getTimestampNow();
            if (now >= mLastDqLogTs + 5000000) {
                if (now >= mLastDqTs + 1000000 || mDqFailure > 5) {
                    ALOGW("last successful dequeue was %lld us ago, "
                          "%zu consecutive failures",
                          (long long)(now - mLastDqTs), mDqFailure);
                }
                mLastDqLogTs = now;
            }
        }
        if (mProducerId == 0) {
            std::shared_ptr<C2GraphicAllocation> alloc;
            c2_status_t err = mAllocator->newGraphicAllocation(
                    width, height, format, usage, &alloc);
            if (err != C2_OK) {
                return err;
            }
            std::shared_ptr<C2BufferQueueBlockPoolData> poolData =
                    std::make_shared<C2BufferQueueBlockPoolData>(
                            0, (uint64_t)0, ~0, shared_from_this());
            *block = _C2BlockFactory::CreateGraphicBlock(alloc, poolData);
            ALOGV("allocated a buffer successfully");

            return C2_OK;
        }
        c2_status_t status = fetchFromIgbp_l(width, height, format, usage, block);
        if (status == C2_BLOCKING) {
            lock.unlock();
            // in order not to drain cpu from component's spinning
            ::usleep(kMaxIgbpRetryDelayUs);
        }
        return status;
    }

    void setRenderCallback(const OnRenderCallback &renderCallback) {
        std::scoped_lock<std::mutex> lock(mMutex);
        mRenderCallback = renderCallback;
    }

    void configureProducer(const sp<HGraphicBufferProducer> &producer) {
        uint64_t producerId = 0;
        uint32_t generation = 0;
        bool haveGeneration = false;
        if (producer) {
            Return<uint64_t> transResult = producer->getUniqueId();
            if (!transResult.isOk()) {
                ALOGD("configureProducer -- failed to connect to the producer");
                return;
            }
            producerId = static_cast<uint64_t>(transResult);
            // TODO: provide gneration number from parameter.
            haveGeneration = getGenerationNumber(producer, &generation);
            if (!haveGeneration) {
                ALOGW("get generationNumber failed %llu",
                      (unsigned long long)producerId);
            }
        }
        int migrated = 0;
        {
            sp<GraphicBuffer> buffers[NUM_BUFFER_SLOTS];
            std::weak_ptr<C2BufferQueueBlockPoolData>
                    poolDatas[NUM_BUFFER_SLOTS];
            std::scoped_lock<std::mutex> lock(mMutex);
            bool noInit = false;
            for (int i = 0; i < NUM_BUFFER_SLOTS; ++i) {
                if (!noInit && mProducer) {
                    Return<HStatus> transResult =
                            mProducer->detachBuffer(static_cast<int32_t>(i));
                    noInit = !transResult.isOk() ||
                             static_cast<HStatus>(transResult) == HStatus::NO_INIT;
                }
            }
            int32_t oldGeneration = mGeneration;
            if (producer) {
                mProducer = producer;
                mProducerId = producerId;
                mGeneration = haveGeneration ? generation : 0;
            } else {
                mProducer = nullptr;
                mProducerId = 0;
                mGeneration = 0;
                ALOGW("invalid producer producer(%d), generation(%d)",
                      (bool)producer, haveGeneration);
            }
            if (mProducer && haveGeneration) { // migrate buffers
                for (int i = 0; i < NUM_BUFFER_SLOTS; ++i) {
                    std::shared_ptr<C2BufferQueueBlockPoolData> data =
                            mPoolDatas[i].lock();
                    if (data) {
                        int slot = data->migrate(
                                mProducer, generation,
                                producerId, mBuffers, oldGeneration);
                        if (slot >= 0) {
                            buffers[slot] = mBuffers[i];
                            poolDatas[slot] = data;
                            ++migrated;
                        }
                    }
                }
            }
            for (int i = 0; i < NUM_BUFFER_SLOTS; ++i) {
                mBuffers[i] = buffers[i];
                mPoolDatas[i] = poolDatas[i];
            }
        }
        if (producer && haveGeneration) {
            ALOGD("local generation change %u , "
                  "bqId: %llu migrated buffers # %d",
                  generation, (unsigned long long)producerId, migrated);
        }
    }

private:
    friend struct C2BufferQueueBlockPoolData;

    void cancel(uint32_t generation, uint64_t igbp_id, int32_t igbp_slot) {
        bool cancelled = false;
        {
        std::scoped_lock<std::mutex> lock(mMutex);
        if (generation == mGeneration && igbp_id == mProducerId && mProducer) {
            (void)mProducer->cancelBuffer(igbp_slot, hidl_handle{}).isOk();
            cancelled = true;
        }
        }
    }

    c2_status_t mInit;
    uint64_t mProducerId;
    uint32_t mGeneration;
    OnRenderCallback mRenderCallback;

    size_t mDqFailure;
    int64_t mLastDqTs;
    int64_t mLastDqLogTs;

    const std::shared_ptr<C2Allocator> mAllocator;

    std::mutex mMutex;
    sp<HGraphicBufferProducer> mProducer;
    sp<HGraphicBufferProducer> mSavedProducer;

    sp<GraphicBuffer> mBuffers[NUM_BUFFER_SLOTS];
    std::weak_ptr<C2BufferQueueBlockPoolData> mPoolDatas[NUM_BUFFER_SLOTS];
};

C2BufferQueueBlockPoolData::C2BufferQueueBlockPoolData(
        uint32_t generation, uint64_t bqId, int32_t bqSlot,
        const std::shared_ptr<int>& owner,
        const sp<HGraphicBufferProducer>& producer) :
        held(producer && bqId != 0), local(false),
        generation(generation), bqId(bqId), bqSlot(bqSlot),
        transfer(false), attach(false), display(false),
        owner(owner), igbp(producer),
        localPool() {
}

C2BufferQueueBlockPoolData::C2BufferQueueBlockPoolData(
        uint32_t generation, uint64_t bqId, int32_t bqSlot,
        const std::shared_ptr<C2BufferQueueBlockPool::Impl>& pool) :
        held(true), local(true),
        generation(generation), bqId(bqId), bqSlot(bqSlot),
        transfer(false), attach(false), display(false),
        igbp(pool ? pool->mProducer : nullptr),
        localPool(pool) {
}

C2BufferQueueBlockPoolData::~C2BufferQueueBlockPoolData() {
    if (!held || bqId == 0) {
        return;
    }
    if (local) {
        if (localPool) {
            localPool->cancel(generation, bqId, bqSlot);
        }
    } else if (igbp && !owner.expired()) {
        igbp->cancelBuffer(bqSlot, hidl_handle{}).isOk();
    }
}
int C2BufferQueueBlockPoolData::migrate(
        const sp<HGraphicBufferProducer>& producer,
        uint32_t toGeneration, uint64_t toBqId,
        sp<GraphicBuffer> *buffers, uint32_t oldGeneration) {
    std::scoped_lock<std::mutex> l(lock);
    if (!held || bqId == 0) {
        ALOGV("buffer is not owned");
        return -1;
    }
    if (!local || !localPool) {
        ALOGV("pool is not local");
        return -1;
    }
    if (bqSlot < 0 || bqSlot >= NUM_BUFFER_SLOTS || !buffers[bqSlot]) {
        ALOGV("slot is not in effect");
        return -1;
    }
    if (toGeneration == generation && bqId == toBqId) {
        ALOGV("cannot migrate to same bufferqueue");
        return -1;
    }
    if (oldGeneration != generation) {
        ALOGV("cannot migrate stale buffer");
    }
    if (transfer) {
        // either transferred or detached.
        ALOGV("buffer is in transfer");
        return -1;
    }
    sp<GraphicBuffer> const& graphicBuffer = buffers[bqSlot];
    graphicBuffer->setGenerationNumber(toGeneration);

    HBuffer hBuffer{};
    uint32_t hGenerationNumber{};
    if (!b2h(graphicBuffer, &hBuffer, &hGenerationNumber)) {
        ALOGD("I to O conversion failed");
        return -1;
    }

    bool converted{};
    status_t bStatus{};
    int slot;
    int *outSlot = &slot;
    Return<void> transResult =
            producer->attachBuffer(hBuffer, hGenerationNumber,
                    [&converted, &bStatus, outSlot](
                            HStatus hStatus, int32_t hSlot, bool releaseAll) {
                        converted = h2b(hStatus, &bStatus);
                        *outSlot = static_cast<int>(hSlot);
                        if (converted && releaseAll && bStatus == android::OK) {
                            bStatus = android::INVALID_OPERATION;
                        }
                    });
    if (!transResult.isOk() || !converted || bStatus != android::OK) {
        ALOGD("attach failed %d", static_cast<int>(bStatus));
        return -1;
    }
    ALOGV("local migration from gen %u : %u slot %d : %d",
          generation, toGeneration, bqSlot, slot);
    generation = toGeneration;
    bqId = toBqId;
    bqSlot = slot;
    return slot;
}

C2BufferQueueBlockPool::C2BufferQueueBlockPool(
        const std::shared_ptr<C2Allocator> &allocator, const local_id_t localId)
        : mAllocator(allocator), mLocalId(localId), mImpl(new Impl(allocator)) {}

C2BufferQueueBlockPool::~C2BufferQueueBlockPool() {}

c2_status_t C2BufferQueueBlockPool::fetchGraphicBlock(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        C2MemoryUsage usage,
        std::shared_ptr<C2GraphicBlock> *block /* nonnull */) {
    if (mImpl) {
        return mImpl->fetchGraphicBlock(width, height, format, usage, block);
    }
    return C2_CORRUPTED;
}

void C2BufferQueueBlockPool::configureProducer(const sp<HGraphicBufferProducer> &producer) {
    if (mImpl) {
        mImpl->configureProducer(producer);
    }
}

void C2BufferQueueBlockPool::setRenderCallback(const OnRenderCallback &renderCallback) {
    if (mImpl) {
        mImpl->setRenderCallback(renderCallback);
    }
}
