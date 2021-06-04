/*
 * Copyright 2018 The Android Open Source Project
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
#define LOG_TAG "Codec2-block_helper"
#include <android-base/logging.h>

#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>
#include <codec2/hidl/1.0/ClientBlockHelper.h>
#include <gui/bufferqueue/2.0/B2HGraphicBufferProducer.h>

#include <C2AllocatorGralloc.h>
#include <C2BlockInternal.h>
#include <C2Buffer.h>
#include <C2PlatformSupport.h>

#include <iomanip>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using HGraphicBufferProducer = ::android::hardware::graphics::bufferqueue::
        V2_0::IGraphicBufferProducer;
using B2HGraphicBufferProducer = ::android::hardware::graphics::bufferqueue::
        V2_0::utils::B2HGraphicBufferProducer;

namespace /* unnamed */ {

// Create a GraphicBuffer object from a graphic block.
sp<GraphicBuffer> createGraphicBuffer(const C2ConstGraphicBlock& block) {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint64_t usage;
    uint32_t stride;
    uint32_t generation;
    uint64_t bqId;
    int32_t bqSlot;
    _UnwrapNativeCodec2GrallocMetadata(
            block.handle(), &width, &height, &format, &usage,
            &stride, &generation, &bqId, reinterpret_cast<uint32_t*>(&bqSlot));
    native_handle_t *grallocHandle =
            UnwrapNativeCodec2GrallocHandle(block.handle());
    sp<GraphicBuffer> graphicBuffer =
            new GraphicBuffer(grallocHandle,
                              GraphicBuffer::CLONE_HANDLE,
                              width, height, format,
                              1, usage, stride);
    native_handle_delete(grallocHandle);
    return graphicBuffer;
}

template <typename BlockProcessor>
void forEachBlock(C2FrameData& frameData,
                  BlockProcessor process) {
    for (const std::shared_ptr<C2Buffer>& buffer : frameData.buffers) {
        if (buffer) {
            for (const C2ConstGraphicBlock& block :
                    buffer->data().graphicBlocks()) {
                process(block);
            }
        }
    }
}

template <typename BlockProcessor>
void forEachBlock(const std::list<std::unique_ptr<C2Work>>& workList,
                  BlockProcessor process) {
    for (const std::unique_ptr<C2Work>& work : workList) {
        if (!work) {
            continue;
        }
        for (const std::unique_ptr<C2Worklet>& worklet : work->worklets) {
            if (worklet) {
                forEachBlock(worklet->output, process);
            }
        }
    }
}

sp<HGraphicBufferProducer> getHgbp(const sp<IGraphicBufferProducer>& igbp) {
    sp<HGraphicBufferProducer> hgbp =
            igbp->getHalInterface<HGraphicBufferProducer>();
    return hgbp ? hgbp :
            new B2HGraphicBufferProducer(igbp);
}

status_t attachToBufferQueue(const C2ConstGraphicBlock& block,
                             const sp<IGraphicBufferProducer>& igbp,
                             uint32_t generation,
                             int32_t* bqSlot) {
    if (!igbp) {
        LOG(WARNING) << "attachToBufferQueue -- null producer.";
        return NO_INIT;
    }

    sp<GraphicBuffer> graphicBuffer = createGraphicBuffer(block);
    graphicBuffer->setGenerationNumber(generation);

    LOG(VERBOSE) << "attachToBufferQueue -- attaching buffer:"
            << " block dimension " << block.width() << "x"
                                   << block.height()
            << ", graphicBuffer dimension " << graphicBuffer->getWidth() << "x"
                                           << graphicBuffer->getHeight()
            << std::hex << std::setfill('0')
            << ", format 0x" << std::setw(8) << graphicBuffer->getPixelFormat()
            << ", usage 0x" << std::setw(16) << graphicBuffer->getUsage()
            << std::dec << std::setfill(' ')
            << ", stride " << graphicBuffer->getStride()
            << ", generation " << graphicBuffer->getGenerationNumber();

    status_t result = igbp->attachBuffer(bqSlot, graphicBuffer);
    if (result != OK) {
        LOG(WARNING) << "attachToBufferQueue -- attachBuffer failed: "
                        "status = " << result << ".";
        return result;
    }
    LOG(VERBOSE) << "attachToBufferQueue -- attachBuffer returned slot #"
                 << *bqSlot << ".";
    return OK;
}

bool getBufferQueueAssignment(const C2ConstGraphicBlock& block,
                              uint32_t* generation,
                              uint64_t* bqId,
                              int32_t* bqSlot) {
    return _C2BlockFactory::GetBufferQueueData(
            _C2BlockFactory::GetGraphicBlockPoolData(block),
            generation, bqId, bqSlot);
}
} // unnamed namespace

class OutputBufferQueue::Impl {
    std::mutex mMutex;
    sp<IGraphicBufferProducer> mIgbp;
    uint32_t mGeneration;
    uint64_t mBqId;
    std::shared_ptr<int> mOwner;
    // To migrate existing buffers
    sp<GraphicBuffer> mBuffers[BufferQueueDefs::NUM_BUFFER_SLOTS]; // find a better way
    std::weak_ptr<_C2BlockPoolData>
                    mPoolDatas[BufferQueueDefs::NUM_BUFFER_SLOTS];

public:
    Impl(): mGeneration(0), mBqId(0) {}

    bool configure(const sp<IGraphicBufferProducer>& igbp,
                   uint32_t generation,
                   uint64_t bqId) {
        size_t tryNum = 0;
        size_t success = 0;
        sp<GraphicBuffer> buffers[BufferQueueDefs::NUM_BUFFER_SLOTS];
        std::weak_ptr<_C2BlockPoolData>
                poolDatas[BufferQueueDefs::NUM_BUFFER_SLOTS];
        {
            std::scoped_lock<std::mutex> l(mMutex);
            if (generation == mGeneration) {
                return false;
            }
            mIgbp = igbp;
            mGeneration = generation;
            mBqId = bqId;
            mOwner = std::make_shared<int>(0);
            for (int i = 0; i < BufferQueueDefs::NUM_BUFFER_SLOTS; ++i) {
                if (mBqId == 0 || !mBuffers[i]) {
                    continue;
                }
                std::shared_ptr<_C2BlockPoolData> data = mPoolDatas[i].lock();
                if (!data ||
                    !_C2BlockFactory::BeginAttachBlockToBufferQueue(data)) {
                    continue;
                }
                ++tryNum;
                int bqSlot;
                mBuffers[i]->setGenerationNumber(generation);
                status_t result = igbp->attachBuffer(&bqSlot, mBuffers[i]);
                if (result != OK) {
                    continue;
                }
                bool attach =
                        _C2BlockFactory::EndAttachBlockToBufferQueue(
                                data, mOwner, getHgbp(mIgbp),
                                generation, bqId, bqSlot);
                if (!attach) {
                    igbp->cancelBuffer(bqSlot, Fence::NO_FENCE);
                    continue;
                }
                buffers[bqSlot] = mBuffers[i];
                poolDatas[bqSlot] = data;
                ++success;
            }
            for (int i = 0; i < BufferQueueDefs::NUM_BUFFER_SLOTS; ++i) {
                mBuffers[i] = buffers[i];
                mPoolDatas[i] = poolDatas[i];
            }
        }
        ALOGD("remote graphic buffer migration %zu/%zu", success, tryNum);
        return true;
    }

    bool registerBuffer(const C2ConstGraphicBlock& block) {
        std::shared_ptr<_C2BlockPoolData> data =
                _C2BlockFactory::GetGraphicBlockPoolData(block);
        if (!data) {
            return false;
        }
        std::scoped_lock<std::mutex> l(mMutex);

        if (!mIgbp) {
            return false;
        }

        uint32_t oldGeneration;
        uint64_t oldId;
        int32_t oldSlot;
        // If the block is not bufferqueue-based, do nothing.
        if (!_C2BlockFactory::GetBufferQueueData(
                data, &oldGeneration, &oldId, &oldSlot) || (oldId == 0)) {
            return false;
        }
        // If the block's bqId is the same as the desired bqId, just hold.
        if ((oldId == mBqId) && (oldGeneration == mGeneration)) {
            LOG(VERBOSE) << "holdBufferQueueBlock -- import without attaching:"
                         << " bqId " << oldId
                         << ", bqSlot " << oldSlot
                         << ", generation " << mGeneration
                         << ".";
            _C2BlockFactory::HoldBlockFromBufferQueue(data, mOwner, getHgbp(mIgbp));
            mPoolDatas[oldSlot] = data;
            mBuffers[oldSlot] = createGraphicBuffer(block);
            mBuffers[oldSlot]->setGenerationNumber(mGeneration);
            return true;
        }
        int32_t d = (int32_t) mGeneration - (int32_t) oldGeneration;
        LOG(WARNING) << "receiving stale buffer: generation "
                     << mGeneration << " , diff " << d  << " : slot "
                     << oldSlot;
        return false;
    }

    status_t outputBuffer(
            const C2ConstGraphicBlock& block,
            const BnGraphicBufferProducer::QueueBufferInput& input,
            BnGraphicBufferProducer::QueueBufferOutput* output) {
        uint32_t generation;
        uint64_t bqId;
        int32_t bqSlot;
        bool display = displayBufferQueueBlock(block);
        if (!getBufferQueueAssignment(block, &generation, &bqId, &bqSlot) ||
            bqId == 0) {
            // Block not from bufferqueue -- it must be attached before queuing.

            mMutex.lock();
            sp<IGraphicBufferProducer> outputIgbp = mIgbp;
            uint32_t outputGeneration = mGeneration;
            mMutex.unlock();

            status_t status = attachToBufferQueue(
                    block, outputIgbp, outputGeneration, &bqSlot);
            if (status != OK) {
                LOG(WARNING) << "outputBuffer -- attaching failed.";
                return INVALID_OPERATION;
            }

            status = outputIgbp->queueBuffer(static_cast<int>(bqSlot),
                                         input, output);
            if (status != OK) {
                LOG(ERROR) << "outputBuffer -- queueBuffer() failed "
                           "on non-bufferqueue-based block. "
                           "Error = " << status << ".";
                return status;
            }
            return OK;
        }

        mMutex.lock();
        sp<IGraphicBufferProducer> outputIgbp = mIgbp;
        uint32_t outputGeneration = mGeneration;
        uint64_t outputBqId = mBqId;
        mMutex.unlock();

        if (!outputIgbp) {
            LOG(VERBOSE) << "outputBuffer -- output surface is null.";
            return NO_INIT;
        }

        if (!display) {
            LOG(WARNING) << "outputBuffer -- cannot display "
                         "bufferqueue-based block to the bufferqueue.";
            return UNKNOWN_ERROR;
        }
        if (bqId != outputBqId || generation != outputGeneration) {
            int32_t diff = (int32_t) outputGeneration - (int32_t) generation;
            LOG(WARNING) << "outputBuffer -- buffers from old generation to "
                         << outputGeneration << " , diff: " << diff
                         << " , slot: " << bqSlot;
            return DEAD_OBJECT;
        }

        status_t status = outputIgbp->queueBuffer(static_cast<int>(bqSlot),
                                              input, output);
        if (status != OK) {
            LOG(ERROR) << "outputBuffer -- queueBuffer() failed "
                       "on bufferqueue-based block. "
                       "Error = " << status << ".";
            return status;
        }
        return OK;
    }

    Impl *getPtr() {
        return this;
    }

    ~Impl() {}
};

OutputBufferQueue::OutputBufferQueue(): mImpl(new Impl()) {}

OutputBufferQueue::~OutputBufferQueue() {}

bool OutputBufferQueue::configure(const sp<IGraphicBufferProducer>& igbp,
                                  uint32_t generation,
                                  uint64_t bqId) {
    return mImpl && mImpl->configure(igbp, generation, bqId);
}

status_t OutputBufferQueue::outputBuffer(
    const C2ConstGraphicBlock& block,
    const BnGraphicBufferProducer::QueueBufferInput& input,
    BnGraphicBufferProducer::QueueBufferOutput* output) {
    if (mImpl) {
        return mImpl->outputBuffer(block, input, output);
    }
    return DEAD_OBJECT;
}

void OutputBufferQueue::holdBufferQueueBlocks(
        const std::list<std::unique_ptr<C2Work>>& workList) {
    if (!mImpl) {
        return;
    }
    forEachBlock(workList,
                 std::bind(&OutputBufferQueue::Impl::registerBuffer,
                           mImpl->getPtr(), std::placeholders::_1));
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

