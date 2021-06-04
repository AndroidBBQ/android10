#include <errno.h>

#include <bufferhub/BufferHubService.h>
#include <bufferhub/BufferNode.h>
#include <log/log.h>
#include <ui/GraphicBufferAllocator.h>

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

void BufferNode::initializeMetadata() {
    // Using placement new here to reuse shared memory instead of new allocation
    // Initialize the atomic variables to zero.
    BufferHubDefs::MetadataHeader* metadataHeader = mMetadata.metadataHeader();
    mBufferState = new (&metadataHeader->bufferState) std::atomic<uint32_t>(0);
    mFenceState = new (&metadataHeader->fenceState) std::atomic<uint32_t>(0);
    mActiveClientsBitMask = new (&metadataHeader->activeClientsBitMask) std::atomic<uint32_t>(0);
    // The C++ standard recommends (but does not require) that lock-free atomic operations are
    // also address-free, that is, suitable for communication between processes using shared
    // memory.
    LOG_ALWAYS_FATAL_IF(!std::atomic_is_lock_free(mBufferState) ||
                                !std::atomic_is_lock_free(mFenceState) ||
                                !std::atomic_is_lock_free(mActiveClientsBitMask),
                        "Atomic variables in ashmen are not lock free.");
}

// Allocates a new BufferNode.
BufferNode::BufferNode(uint32_t width, uint32_t height, uint32_t layerCount, uint32_t format,
                       uint64_t usage, size_t userMetadataSize, int id)
      : mId(id) {
    uint32_t outStride = 0;
    // graphicBufferId is not used in GraphicBufferAllocator::allocate
    // TODO(b/112338294) After move to the service folder, stop using the
    // hardcoded service name "bufferhub".
    int ret = GraphicBufferAllocator::get().allocate(width, height, format, layerCount, usage,
                                                     const_cast<const native_handle_t**>(
                                                             &mBufferHandle),
                                                     &outStride,
                                                     /*graphicBufferId=*/0,
                                                     /*requestor=*/"bufferhub");

    if (ret != OK || mBufferHandle == nullptr) {
        ALOGE("%s: Failed to allocate buffer: %s", __FUNCTION__, strerror(-ret));
        return;
    }

    mBufferDesc.width = width;
    mBufferDesc.height = height;
    mBufferDesc.layers = layerCount;
    mBufferDesc.format = format;
    mBufferDesc.usage = usage;
    mBufferDesc.stride = outStride;

    mMetadata = BufferHubMetadata::create(userMetadataSize);
    if (!mMetadata.isValid()) {
        ALOGE("%s: Failed to allocate metadata.", __FUNCTION__);
        return;
    }
    initializeMetadata();
}

BufferNode::~BufferNode() {
    // Free the handle
    if (mBufferHandle != nullptr) {
        status_t ret = GraphicBufferAllocator::get().free(mBufferHandle);
        if (ret != OK) {
            ALOGE("%s: Failed to free handle; Got error: %d", __FUNCTION__, ret);
        }
    }

    // Free the id, if valid
    if (mId >= 0) {
        BufferHubIdGenerator::getInstance().freeId(mId);
    }
}

uint32_t BufferNode::getActiveClientsBitMask() const {
    return mActiveClientsBitMask->load(std::memory_order_acquire);
}

uint32_t BufferNode::addNewActiveClientsBitToMask() {
    uint32_t currentActiveClientsBitMask = getActiveClientsBitMask();
    uint32_t clientStateMask = 0U;
    uint32_t updatedActiveClientsBitMask = 0U;
    do {
        clientStateMask =
                BufferHubDefs::findNextAvailableClientStateMask(currentActiveClientsBitMask);
        if (clientStateMask == 0U) {
            ALOGE("%s: reached the maximum number of channels per buffer node: %d.", __FUNCTION__,
                  BufferHubDefs::kMaxNumberOfClients);
            errno = E2BIG;
            return 0U;
        }
        updatedActiveClientsBitMask = currentActiveClientsBitMask | clientStateMask;
    } while (!(mActiveClientsBitMask->compare_exchange_weak(currentActiveClientsBitMask,
                                                            updatedActiveClientsBitMask,
                                                            std::memory_order_acq_rel,
                                                            std::memory_order_acquire)));
    return clientStateMask;
}

void BufferNode::removeClientsBitFromMask(const uint32_t& value) {
    mActiveClientsBitMask->fetch_and(~value);
}

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android
