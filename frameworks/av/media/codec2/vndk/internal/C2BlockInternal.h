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

#ifndef ANDROID_STAGEFRIGHT_C2BLOCK_INTERNAL_H_
#define ANDROID_STAGEFRIGHT_C2BLOCK_INTERNAL_H_

#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>

#include <C2Buffer.h>

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {

struct BufferPoolData;

}
}
}
}

/**
 * Stores informations from C2BlockPool implementations which are required by C2Block.
 */
struct C2_HIDE _C2BlockPoolData {
    enum type_t : int {
        TYPE_BUFFERPOOL = 0,
        TYPE_BUFFERQUEUE,
    };

    virtual type_t getType() const = 0;

protected:
    _C2BlockPoolData() = default;

    virtual ~_C2BlockPoolData() = default;
};

struct C2BufferQueueBlockPoolData;

/**
 * Internal only interface for creating blocks by block pool/buffer passing implementations.
 *
 * \todo this must be hidden
 */
struct _C2BlockFactory {
    /**
     * Create a linear block from an allocation for an allotted range.
     *
     * \param alloc parent allocation
     * \param data  blockpool data
     * \param offset allotted range offset
     * \param size  allotted size
     *
     * \return shared pointer to the linear block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2LinearBlock> CreateLinearBlock(
            const std::shared_ptr<C2LinearAllocation> &alloc,
            const std::shared_ptr<_C2BlockPoolData> &data = nullptr,
            size_t offset = 0,
            size_t size = ~(size_t)0);

    /**
     * Create a graphic block from an allocation for an allotted section.
     *
     * \param alloc parent allocation
     * \param data  blockpool data
     * \param crop  allotted crop region
     *
     * \return shared pointer to the graphic block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2GraphicBlock> CreateGraphicBlock(
            const std::shared_ptr<C2GraphicAllocation> &alloc,
            const std::shared_ptr<_C2BlockPoolData> &data = nullptr,
            const C2Rect &allottedCrop = C2Rect(~0u, ~0u));

    /**
     * Return a block pool data from 1D block.
     *
     * \param shared pointer to the 1D block which is already created.
     */
    static
    std::shared_ptr<_C2BlockPoolData> GetLinearBlockPoolData(
            const C2Block1D& block);

    /**
     * Return a block pool data from 2D block.
     *
     * \param shared pointer to the 2D block which is already created.
     */
    static
    std::shared_ptr<_C2BlockPoolData> GetGraphicBlockPoolData(
            const C2Block2D& block);

    /**
     * Create a linear block from the received native handle.
     *
     * \param handle    native handle to a linear block
     *
     * \return shared pointer to the linear block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2LinearBlock> CreateLinearBlock(
            const C2Handle *handle);

    /**
     * Create a graphic block from the received native handle.
     *
     * \param handle    native handle to a graphic block
     *
     * \return shared pointer to the graphic block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2GraphicBlock> CreateGraphicBlock(
            const C2Handle *handle);

    /**
     * Create a linear block from the received bufferpool data.
     *
     * \param data  bufferpool data to a linear block
     *
     * \return shared pointer to the linear block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2LinearBlock> CreateLinearBlock(
            const C2Handle *handle,
            const std::shared_ptr<android::hardware::media::bufferpool::BufferPoolData> &data);

    /**
     * Create a graphic block from the received bufferpool data.
     *
     * \param data  bufferpool data to a graphic block
     *
     * \return shared pointer to the graphic block. nullptr if there was not enough memory to
     *         create this block.
     */
    static
    std::shared_ptr<C2GraphicBlock> CreateGraphicBlock(
            const C2Handle *handle,
            const std::shared_ptr<android::hardware::media::bufferpool::BufferPoolData> &data);

    /**
     * Get bufferpool data from the blockpool data.
     *
     * \param poolData          blockpool data
     * \param bufferPoolData    pointer to bufferpool data where the bufferpool
     *                          data is stored.
     *
     * \return {\code true} when there is valid bufferpool data, {\code false} otherwise.
     */
    static
    bool GetBufferPoolData(
            const std::shared_ptr<const _C2BlockPoolData> &poolData,
            std::shared_ptr<android::hardware::media::bufferpool::BufferPoolData> *bufferPoolData);

    /*
     * Life Cycle Management of BufferQueue-Based Blocks
     * =================================================
     *
     * A block that is created by a bufferqueue-based blockpool requires some
     * special treatment when it is destroyed. In particular, if the block
     * corresponds to a held (dequeued/attached) GraphicBuffer in a slot of a
     * bufferqueue, its destruction should trigger a call to
     * IGraphicBufferProducer::cancelBuffer(). On the other hand, if the
     * GraphicBuffer is not held, i.e., if it has been queued or detached,
     * cancelBuffer() should not be called upon the destruction of the block.
     *
     * _C2BlockPoolData created by a bufferqueue-based blockpool includes two
     * main pieces of information:
     *   - "held" status: Whether cancelBuffer() should be called upon
     *     destruction of the block.
     *   - bufferqueue assignment: The quadruple (igbp, generation, bqId,
     *     bqSlot), where igbp is the IGraphicBufferProducer instance of the
     *     bufferqueue, generation is the latest generation number, of the
     *     bufferqueue, bqId is the globally unique id of the bufferqueue, and
     *     bqSlot is the slot in the bufferqueue.
     *
     * igbp is the instance of IGraphicBufferProducer on which cancelBuffer()
     * will be called if "held" status is true when the block is destroyed.
     * (bqSlot is an input to cancelBuffer().) However, only generation, bqId
     * and bqSlot are retained when a block is transferred from one process to
     * another. It is the responsibility of both the sending and receiving
     * processes to maintain consistency of "held" status and igbp. Below are
     * functions provided for this purpose:
     *
     *   - GetBufferQueueData(): Returns generation, bqId and bqSlot.
     *   - HoldBlockFromBufferQueue(): Sets "held" status to true.
     *   - BeginTransferBlockToClient()/EndTransferBlockToClient():
     *     Clear "held" status to false if transfer was successful,
     *     otherwise "held" status remains true.
     *   - BeginAttachBlockToBufferQueue()/EndAttachBlockToBufferQueue():
     *     The will keep "held" status true if attach was eligible.
     *     Otherwise, "held" status is cleared to false. In that case,
     *     ownership of buffer should be transferred to bufferqueue.
     *   - DisplayBlockToBufferQueue()
     *     This will clear "held" status to false.
     *
     * All these functions operate on _C2BlockPoolData, which can be obtained by
     * calling GetGraphicBlockPoolData().
     *
     * Maintaining Consistency with IGraphicBufferProducer Operations
     * ==============================================================
     *
     * dequeueBuffer()
     *   - This function is called by the blockpool. It should not be called
     *     manually. The blockpool will automatically generate the correct
     *     information for _C2BlockPoolData, with "held" status set to true.
     *
     * queueBuffer()
     *   - Before queueBuffer() is called, DisplayBlockToBufferQueue() should be
     *     called to test eligibility. If it's not eligible, do not call
     *     queueBuffer().
     *
     * attachBuffer() - remote migration only.
     *   - Local migration on blockpool side will be done automatically by
     *     blockpool.
     *   - Before attachBuffer(), BeginAttachBlockToBufferQueue() should be called
     *     to test eligiblity.
     *   - After attachBuffer() is called, EndAttachBlockToBufferQueue() should
     *     be called. This will set "held" status to true. If it returned
     *     false, cancelBuffer() should be called.
     *
     * detachBuffer() - no-op.
     */

    /**
     * Get bufferqueue data from the blockpool data.
     *
     * Calling this function with \p generation set to nullptr will return
     * whether the block comes from a bufferqueue-based blockpool, but will not
     * fill in the values for \p generation, \p bqId or \p bqSlot.
     *
     * \param[in]  poolData   blockpool data.
     * \param[out] generation Generation number attached to the buffer.
     * \param[out] bqId       Id of the bufferqueue owning the buffer (block).
     * \param[out] bqSlot     Slot number of the buffer.
     *
     * \return \c true when there is valid bufferqueue data;
     *         \c false otherwise.
     */
    static
    bool GetBufferQueueData(
            const std::shared_ptr<const _C2BlockPoolData>& poolData,
            uint32_t* generation = nullptr,
            uint64_t* bqId = nullptr,
            int32_t* bqSlot = nullptr);

    /**
     * Hold a block from the designated bufferqueue. This causes the destruction
     * of the block to trigger a call to cancelBuffer().
     *
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     * \param owner    block owner from client bufferqueue manager.
     *                 If this is expired, the block is not owned by client
     *                 anymore.
     * \param igbp     \c IGraphicBufferProducer instance to be assigned to the
     *                 block. This is not needed when the block is local.
     *
     * \return The previous held status.
     */
    static
    bool HoldBlockFromBufferQueue(
            const std::shared_ptr<_C2BlockPoolData>& poolData,
            const std::shared_ptr<int>& owner,
            const ::android::sp<::android::hardware::graphics::bufferqueue::
                                V2_0::IGraphicBufferProducer>& igbp = nullptr);

    /**
     * Prepare a block to be transferred to other process. This blocks
     * bufferqueue migration from happening. The block should be in held.
     *
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     *
     * \return true if transfer is eligible, false otherwise.
     */
    static
    bool BeginTransferBlockToClient(
            const std::shared_ptr<_C2BlockPoolData>& poolData);

    /**
     * Called after transferring the specified block is finished. Make sure
     * that BeginTransferBlockToClient() was called before this call.
     *
     * This will unblock bufferqueue migration. If transfer result was
     * successful, this causes the destruction of the block not to trigger a
     * call to cancelBuffer().
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     *
     * \return true if transfer began before, false otherwise.
     */
    static
    bool EndTransferBlockToClient(
            const std::shared_ptr<_C2BlockPoolData>& poolData,
            bool transferred);

    /**
     * Prepare a block to be migrated to another bufferqueue. This blocks
     * rendering until migration has been finished.  The block should be in
     * held.
     *
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     *
     * \return true if migration is eligible, false otherwise.
     */
    static
    bool BeginAttachBlockToBufferQueue(
            const std::shared_ptr<_C2BlockPoolData>& poolData);

    /**
     * Called after migration of the specified block is finished. Make sure
     * that BeginAttachBlockToBufferQueue() was called before this call.
     *
     * This will unblock rendering. if redering is tried during migration,
     * this returns false. In that case, cancelBuffer() should be called.
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     *
     * \return true if migration is eligible, false otherwise.
     */
    static
    bool EndAttachBlockToBufferQueue(
            const std::shared_ptr<_C2BlockPoolData>& poolData,
            const std::shared_ptr<int>& owner,
            const ::android::sp<::android::hardware::graphics::bufferqueue::
                                V2_0::IGraphicBufferProducer>& igbp,
            uint32_t generation,
            uint64_t bqId,
            int32_t bqSlot);

    /**
     * Indicates a block to be rendered very soon.
     *
     * This function assumes that \p poolData comes from a bufferqueue-based
     * block. It does not check if that is the case.
     *
     * \param poolData blockpool data associated to the block.
     *
     * \return true if migration is eligible, false otherwise.
     */
    static
    bool DisplayBlockToBufferQueue(
            const std::shared_ptr<_C2BlockPoolData>& poolData);
};

#endif // ANDROID_STAGEFRIGHT_C2BLOCK_INTERNAL_H_

