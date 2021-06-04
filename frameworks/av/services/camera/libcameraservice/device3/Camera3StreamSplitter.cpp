/*
 * Copyright 2014,2016 The Android Open Source Project
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

#include <inttypes.h>

#define LOG_TAG "Camera3StreamSplitter"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <gui/BufferItem.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/BufferQueue.h>
#include <gui/Surface.h>

#include <ui/GraphicBuffer.h>

#include <binder/ProcessState.h>

#include <utils/Trace.h>

#include <cutils/atomic.h>

#include "Camera3StreamSplitter.h"

namespace android {

status_t Camera3StreamSplitter::connect(const std::unordered_map<size_t, sp<Surface>> &surfaces,
        uint64_t consumerUsage, uint64_t producerUsage, size_t halMaxBuffers, uint32_t width,
        uint32_t height, android::PixelFormat format, sp<Surface>* consumer) {
    ATRACE_CALL();
    if (consumer == nullptr) {
        SP_LOGE("%s: consumer pointer is NULL", __FUNCTION__);
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);
    status_t res = OK;

    if (mOutputs.size() > 0 || mConsumer != nullptr) {
        SP_LOGE("%s: already connected", __FUNCTION__);
        return BAD_VALUE;
    }
    if (mBuffers.size() > 0) {
        SP_LOGE("%s: still has %zu pending buffers", __FUNCTION__, mBuffers.size());
        return BAD_VALUE;
    }

    mMaxHalBuffers = halMaxBuffers;
    mConsumerName = getUniqueConsumerName();
    // Add output surfaces. This has to be before creating internal buffer queue
    // in order to get max consumer side buffers.
    for (auto &it : surfaces) {
        if (it.second == nullptr) {
            SP_LOGE("%s: Fatal: surface is NULL", __FUNCTION__);
            return BAD_VALUE;
        }
        res = addOutputLocked(it.first, it.second);
        if (res != OK) {
            SP_LOGE("%s: Failed to add output surface: %s(%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
    }

    // Create BufferQueue for input
    BufferQueue::createBufferQueue(&mProducer, &mConsumer);

    // Allocate 1 extra buffer to handle the case where all buffers are detached
    // from input, and attached to the outputs. In this case, the input queue's
    // dequeueBuffer can still allocate 1 extra buffer before being blocked by
    // the output's attachBuffer().
    mMaxConsumerBuffers++;
    mBufferItemConsumer = new BufferItemConsumer(mConsumer, consumerUsage, mMaxConsumerBuffers);
    if (mBufferItemConsumer == nullptr) {
        return NO_MEMORY;
    }
    mConsumer->setConsumerName(mConsumerName);

    *consumer = new Surface(mProducer);
    if (*consumer == nullptr) {
        return NO_MEMORY;
    }

    res = mProducer->setAsyncMode(true);
    if (res != OK) {
        SP_LOGE("%s: Failed to enable input queue async mode: %s(%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    res = mConsumer->consumerConnect(this, /* controlledByApp */ false);

    mWidth = width;
    mHeight = height;
    mFormat = format;
    mProducerUsage = producerUsage;
    mAcquiredInputBuffers = 0;

    SP_LOGV("%s: connected", __FUNCTION__);
    return res;
}

status_t Camera3StreamSplitter::getOnFrameAvailableResult() {
    ATRACE_CALL();
    return mOnFrameAvailableRes.load();
}

void Camera3StreamSplitter::disconnect() {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);

    for (auto& notifier : mNotifiers) {
        sp<IGraphicBufferProducer> producer = notifier.first;
        sp<OutputListener> listener = notifier.second;
        IInterface::asBinder(producer)->unlinkToDeath(listener);
    }
    mNotifiers.clear();

    for (auto& output : mOutputs) {
        if (output.second != nullptr) {
            output.second->disconnect(NATIVE_WINDOW_API_CAMERA);
        }
    }
    mOutputs.clear();
    mOutputSlots.clear();
    mConsumerBufferCount.clear();

    if (mConsumer.get() != nullptr) {
        mConsumer->consumerDisconnect();
    }

    if (mBuffers.size() > 0) {
        SP_LOGW("%zu buffers still being tracked", mBuffers.size());
        mBuffers.clear();
    }

    mMaxHalBuffers = 0;
    mMaxConsumerBuffers = 0;
    mAcquiredInputBuffers = 0;
    SP_LOGV("%s: Disconnected", __FUNCTION__);
}

Camera3StreamSplitter::Camera3StreamSplitter(bool useHalBufManager) :
        mUseHalBufManager(useHalBufManager) {}

Camera3StreamSplitter::~Camera3StreamSplitter() {
    disconnect();
}

status_t Camera3StreamSplitter::addOutput(size_t surfaceId, const sp<Surface>& outputQueue) {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);
    status_t res = addOutputLocked(surfaceId, outputQueue);

    if (res != OK) {
        SP_LOGE("%s: addOutputLocked failed %d", __FUNCTION__, res);
        return res;
    }

    if (mMaxConsumerBuffers > mAcquiredInputBuffers) {
        res = mConsumer->setMaxAcquiredBufferCount(mMaxConsumerBuffers);
    }

    return res;
}

status_t Camera3StreamSplitter::addOutputLocked(size_t surfaceId, const sp<Surface>& outputQueue) {
    ATRACE_CALL();
    if (outputQueue == nullptr) {
        SP_LOGE("addOutput: outputQueue must not be NULL");
        return BAD_VALUE;
    }

    if (mOutputs[surfaceId] != nullptr) {
        SP_LOGE("%s: surfaceId: %u already taken!", __FUNCTION__, (unsigned) surfaceId);
        return BAD_VALUE;
    }

    status_t res = native_window_set_buffers_dimensions(outputQueue.get(),
            mWidth, mHeight);
    if (res != NO_ERROR) {
        SP_LOGE("addOutput: failed to set buffer dimensions (%d)", res);
        return res;
    }
    res = native_window_set_buffers_format(outputQueue.get(),
            mFormat);
    if (res != OK) {
        ALOGE("%s: Unable to configure stream buffer format %#x for surfaceId %zu",
                __FUNCTION__, mFormat, surfaceId);
        return res;
    }

    sp<IGraphicBufferProducer> gbp = outputQueue->getIGraphicBufferProducer();
    // Connect to the buffer producer
    sp<OutputListener> listener(new OutputListener(this, gbp));
    IInterface::asBinder(gbp)->linkToDeath(listener);
    res = outputQueue->connect(NATIVE_WINDOW_API_CAMERA, listener);
    if (res != NO_ERROR) {
        SP_LOGE("addOutput: failed to connect (%d)", res);
        return res;
    }

    // Query consumer side buffer count, and update overall buffer count
    int maxConsumerBuffers = 0;
    res = static_cast<ANativeWindow*>(outputQueue.get())->query(
            outputQueue.get(),
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &maxConsumerBuffers);
    if (res != OK) {
        SP_LOGE("%s: Unable to query consumer undequeued buffer count"
              " for surface", __FUNCTION__);
        return res;
    }

    SP_LOGV("%s: Consumer wants %d buffers, Producer wants %zu", __FUNCTION__,
            maxConsumerBuffers, mMaxHalBuffers);
    // The output slot count requirement can change depending on the current amount
    // of outputs and incoming buffer consumption rate. To avoid any issues with
    // insufficient slots, set their count to the maximum supported. The output
    // surface buffer allocation is disabled so no real buffers will get allocated.
    size_t totalBufferCount = BufferQueue::NUM_BUFFER_SLOTS;
    res = native_window_set_buffer_count(outputQueue.get(),
            totalBufferCount);
    if (res != OK) {
        SP_LOGE("%s: Unable to set buffer count for surface %p",
                __FUNCTION__, outputQueue.get());
        return res;
    }

    // Set dequeueBuffer/attachBuffer timeout if the consumer is not hw composer or hw texture.
    // We need skip these cases as timeout will disable the non-blocking (async) mode.
    uint64_t usage = 0;
    res = native_window_get_consumer_usage(static_cast<ANativeWindow*>(outputQueue.get()), &usage);
    if (!(usage & (GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_TEXTURE))) {
        nsecs_t timeout = mUseHalBufManager ?
                kHalBufMgrDequeueBufferTimeout : kNormalDequeueBufferTimeout;
        outputQueue->setDequeueTimeout(timeout);
    }

    res = gbp->allowAllocation(false);
    if (res != OK) {
        SP_LOGE("%s: Failed to turn off allocation for outputQueue", __FUNCTION__);
        return res;
    }

    // Add new entry into mOutputs
    mOutputs[surfaceId] = gbp;
    mConsumerBufferCount[surfaceId] = maxConsumerBuffers;
    if (mConsumerBufferCount[surfaceId] > mMaxHalBuffers) {
        SP_LOGW("%s: Consumer buffer count %zu larger than max. Hal buffers: %zu", __FUNCTION__,
                mConsumerBufferCount[surfaceId], mMaxHalBuffers);
    }
    mNotifiers[gbp] = listener;
    mOutputSlots[gbp] = std::make_unique<OutputSlots>(totalBufferCount);

    mMaxConsumerBuffers += maxConsumerBuffers;
    return NO_ERROR;
}

status_t Camera3StreamSplitter::removeOutput(size_t surfaceId) {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);

    status_t res = removeOutputLocked(surfaceId);
    if (res != OK) {
        SP_LOGE("%s: removeOutputLocked failed %d", __FUNCTION__, res);
        return res;
    }

    if (mAcquiredInputBuffers < mMaxConsumerBuffers) {
        res = mConsumer->setMaxAcquiredBufferCount(mMaxConsumerBuffers);
        if (res != OK) {
            SP_LOGE("%s: setMaxAcquiredBufferCount failed %d", __FUNCTION__, res);
            return res;
        }
    }

    return res;
}

status_t Camera3StreamSplitter::removeOutputLocked(size_t surfaceId) {
    if (mOutputs[surfaceId] == nullptr) {
        SP_LOGE("%s: output surface is not present!", __FUNCTION__);
        return BAD_VALUE;
    }

    sp<IGraphicBufferProducer> gbp = mOutputs[surfaceId];
    //Search and decrement the ref. count of any buffers that are
    //still attached to the removed surface.
    std::vector<uint64_t> pendingBufferIds;
    auto& outputSlots = *mOutputSlots[gbp];
    for (size_t i = 0; i < outputSlots.size(); i++) {
        if (outputSlots[i] != nullptr) {
            pendingBufferIds.push_back(outputSlots[i]->getId());
            auto rc = gbp->detachBuffer(i);
            if (rc != NO_ERROR) {
                //Buffers that fail to detach here will be scheduled for detach in the
                //input buffer queue and the rest of the registered outputs instead.
                //This will help ensure that camera stops accessing buffers that still
                //can get referenced by the disconnected output.
                mDetachedBuffers.emplace(outputSlots[i]->getId());
            }
        }
    }
    mOutputs[surfaceId] = nullptr;
    mOutputSlots[gbp] = nullptr;
    for (const auto &id : pendingBufferIds) {
        decrementBufRefCountLocked(id, surfaceId);
    }

    auto res = IInterface::asBinder(gbp)->unlinkToDeath(mNotifiers[gbp]);
    if (res != OK) {
        SP_LOGE("%s: Failed to unlink producer death listener: %d ", __FUNCTION__, res);
        return res;
    }

    res = gbp->disconnect(NATIVE_WINDOW_API_CAMERA);
    if (res != OK) {
        SP_LOGE("%s: Unable disconnect from producer interface: %d ", __FUNCTION__, res);
        return res;
    }

    mNotifiers[gbp] = nullptr;
    mMaxConsumerBuffers -= mConsumerBufferCount[surfaceId];
    mConsumerBufferCount[surfaceId] = 0;

    return res;
}

status_t Camera3StreamSplitter::outputBufferLocked(const sp<IGraphicBufferProducer>& output,
        const BufferItem& bufferItem, size_t surfaceId) {
    ATRACE_CALL();
    status_t res;
    IGraphicBufferProducer::QueueBufferInput queueInput(
            bufferItem.mTimestamp, bufferItem.mIsAutoTimestamp,
            bufferItem.mDataSpace, bufferItem.mCrop,
            static_cast<int32_t>(bufferItem.mScalingMode),
            bufferItem.mTransform, bufferItem.mFence);

    IGraphicBufferProducer::QueueBufferOutput queueOutput;

    uint64_t bufferId = bufferItem.mGraphicBuffer->getId();
    const BufferTracker& tracker = *(mBuffers[bufferId]);
    int slot = getSlotForOutputLocked(output, tracker.getBuffer());

    // In case the output BufferQueue has its own lock, if we hold splitter lock while calling
    // queueBuffer (which will try to acquire the output lock), the output could be holding its
    // own lock calling releaseBuffer (which  will try to acquire the splitter lock), running into
    // circular lock situation.
    mMutex.unlock();
    res = output->queueBuffer(slot, queueInput, &queueOutput);
    mMutex.lock();

    SP_LOGV("%s: Queuing buffer to buffer queue %p slot %d returns %d",
            __FUNCTION__, output.get(), slot, res);
    //During buffer queue 'mMutex' is not held which makes the removal of
    //"output" possible. Check whether this is the case and return.
    if (mOutputSlots[output] == nullptr) {
        return res;
    }
    if (res != OK) {
        if (res != NO_INIT && res != DEAD_OBJECT) {
            SP_LOGE("Queuing buffer to output failed (%d)", res);
        }
        // If we just discovered that this output has been abandoned, note
        // that, increment the release count so that we still release this
        // buffer eventually, and move on to the next output
        onAbandonedLocked();
        decrementBufRefCountLocked(bufferItem.mGraphicBuffer->getId(), surfaceId);
        return res;
    }

    // If the queued buffer replaces a pending buffer in the async
    // queue, no onBufferReleased is called by the buffer queue.
    // Proactively trigger the callback to avoid buffer loss.
    if (queueOutput.bufferReplaced) {
        onBufferReplacedLocked(output, surfaceId);
    }

    return res;
}

String8 Camera3StreamSplitter::getUniqueConsumerName() {
    static volatile int32_t counter = 0;
    return String8::format("Camera3StreamSplitter-%d", android_atomic_inc(&counter));
}

status_t Camera3StreamSplitter::notifyBufferReleased(const sp<GraphicBuffer>& buffer) {
    ATRACE_CALL();

    Mutex::Autolock lock(mMutex);

    uint64_t bufferId = buffer->getId();
    std::unique_ptr<BufferTracker> tracker_ptr = std::move(mBuffers[bufferId]);
    mBuffers.erase(bufferId);

    return OK;
}

status_t Camera3StreamSplitter::attachBufferToOutputs(ANativeWindowBuffer* anb,
        const std::vector<size_t>& surface_ids) {
    ATRACE_CALL();
    status_t res = OK;

    Mutex::Autolock lock(mMutex);

    sp<GraphicBuffer> gb(static_cast<GraphicBuffer*>(anb));
    uint64_t bufferId = gb->getId();

    // Initialize buffer tracker for this input buffer
    auto tracker = std::make_unique<BufferTracker>(gb, surface_ids);

    for (auto& surface_id : surface_ids) {
        sp<IGraphicBufferProducer>& gbp = mOutputs[surface_id];
        if (gbp.get() == nullptr) {
            //Output surface got likely removed by client.
            continue;
        }
        int slot = getSlotForOutputLocked(gbp, gb);
        if (slot != BufferItem::INVALID_BUFFER_SLOT) {
            //Buffer is already attached to this output surface.
            continue;
        }
        //Temporarly Unlock the mutex when trying to attachBuffer to the output
        //queue, because attachBuffer could block in case of a slow consumer. If
        //we block while holding the lock, onFrameAvailable and onBufferReleased
        //will block as well because they need to acquire the same lock.
        mMutex.unlock();
        res = gbp->attachBuffer(&slot, gb);
        mMutex.lock();
        if (res != OK) {
            SP_LOGE("%s: Cannot attachBuffer from GraphicBufferProducer %p: %s (%d)",
                    __FUNCTION__, gbp.get(), strerror(-res), res);
            // TODO: might need to detach/cleanup the already attached buffers before return?
            return res;
        }
        if ((slot < 0) || (slot > BufferQueue::NUM_BUFFER_SLOTS)) {
            SP_LOGE("%s: Slot received %d either bigger than expected maximum %d or negative!",
                    __FUNCTION__, slot, BufferQueue::NUM_BUFFER_SLOTS);
            return BAD_VALUE;
        }
        //During buffer attach 'mMutex' is not held which makes the removal of
        //"gbp" possible. Check whether this is the case and continue.
        if (mOutputSlots[gbp] == nullptr) {
            continue;
        }
        auto& outputSlots = *mOutputSlots[gbp];
        if (static_cast<size_t> (slot + 1) > outputSlots.size()) {
            outputSlots.resize(slot + 1);
        }
        if (outputSlots[slot] != nullptr) {
            // If the buffer is attached to a slot which already contains a buffer,
            // the previous buffer will be removed from the output queue. Decrement
            // the reference count accordingly.
            decrementBufRefCountLocked(outputSlots[slot]->getId(), surface_id);
        }
        SP_LOGV("%s: Attached buffer %p to slot %d on output %p.",__FUNCTION__, gb.get(),
                slot, gbp.get());
        outputSlots[slot] = gb;
    }

    mBuffers[bufferId] = std::move(tracker);

    return res;
}

void Camera3StreamSplitter::onFrameAvailable(const BufferItem& /*item*/) {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);

    // Acquire and detach the buffer from the input
    BufferItem bufferItem;
    status_t res = mConsumer->acquireBuffer(&bufferItem, /* presentWhen */ 0);
    if (res != NO_ERROR) {
        SP_LOGE("%s: Acquiring buffer from input failed (%d)", __FUNCTION__, res);
        mOnFrameAvailableRes.store(res);
        return;
    }

    uint64_t bufferId;
    if (bufferItem.mGraphicBuffer != nullptr) {
        mInputSlots[bufferItem.mSlot] = bufferItem;
    } else if (bufferItem.mAcquireCalled) {
        bufferItem.mGraphicBuffer = mInputSlots[bufferItem.mSlot].mGraphicBuffer;
        mInputSlots[bufferItem.mSlot].mFrameNumber = bufferItem.mFrameNumber;
    } else {
        SP_LOGE("%s: Invalid input graphic buffer!", __FUNCTION__);
        res = BAD_VALUE;
        return;
    }
    bufferId = bufferItem.mGraphicBuffer->getId();

    if (mBuffers.find(bufferId) == mBuffers.end()) {
        SP_LOGE("%s: Acquired buffer doesn't exist in attached buffer map",
                __FUNCTION__);
        mOnFrameAvailableRes.store(INVALID_OPERATION);
        return;
    }

    mAcquiredInputBuffers++;
    SP_LOGV("acquired buffer %" PRId64 " from input at slot %d",
            bufferItem.mGraphicBuffer->getId(), bufferItem.mSlot);

    if (bufferItem.mTransformToDisplayInverse) {
        bufferItem.mTransform |= NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
    }

    // Attach and queue the buffer to each of the outputs
    BufferTracker& tracker = *(mBuffers[bufferId]);

    SP_LOGV("%s: BufferTracker for buffer %" PRId64 ", number of requests %zu",
           __FUNCTION__, bufferItem.mGraphicBuffer->getId(), tracker.requestedSurfaces().size());
    for (const auto id : tracker.requestedSurfaces()) {

        if (mOutputs[id] == nullptr) {
            //Output surface got likely removed by client.
            continue;
        }

        res = outputBufferLocked(mOutputs[id], bufferItem, id);
        if (res != OK) {
            SP_LOGE("%s: outputBufferLocked failed %d", __FUNCTION__, res);
            mOnFrameAvailableRes.store(res);
            // If we fail to send buffer to certain output, keep sending to
            // other outputs.
            continue;
        }
    }

    mOnFrameAvailableRes.store(res);
}

void Camera3StreamSplitter::decrementBufRefCountLocked(uint64_t id, size_t surfaceId) {
    ATRACE_CALL();

    if (mBuffers[id] == nullptr) {
        return;
    }

    size_t referenceCount = mBuffers[id]->decrementReferenceCountLocked(surfaceId);
    if (referenceCount > 0) {
        return;
    }

    // We no longer need to track the buffer now that it is being returned to the
    // input. Note that this should happen before we unlock the mutex and call
    // releaseBuffer, to avoid the case where the same bufferId is acquired in
    // attachBufferToOutputs resulting in a new BufferTracker with same bufferId
    // overwrites the current one.
    std::unique_ptr<BufferTracker> tracker_ptr = std::move(mBuffers[id]);
    mBuffers.erase(id);

    uint64_t bufferId = tracker_ptr->getBuffer()->getId();
    int consumerSlot = -1;
    uint64_t frameNumber;
    auto inputSlot = mInputSlots.begin();
    for (; inputSlot != mInputSlots.end(); inputSlot++) {
        if (inputSlot->second.mGraphicBuffer->getId() == bufferId) {
            consumerSlot = inputSlot->second.mSlot;
            frameNumber = inputSlot->second.mFrameNumber;
            break;
        }
    }
    if (consumerSlot == -1) {
        SP_LOGE("%s: Buffer missing inside input slots!", __FUNCTION__);
        return;
    }

    auto detachBuffer = mDetachedBuffers.find(bufferId);
    bool detach = (detachBuffer != mDetachedBuffers.end());
    if (detach) {
        mDetachedBuffers.erase(detachBuffer);
        mInputSlots.erase(inputSlot);
    }
    // Temporarily unlock mutex to avoid circular lock:
    // 1. This function holds splitter lock, calls releaseBuffer which triggers
    // onBufferReleased in Camera3OutputStream. onBufferReleased waits on the
    // OutputStream lock
    // 2. Camera3SharedOutputStream::getBufferLocked calls
    // attachBufferToOutputs, which holds the stream lock, and waits for the
    // splitter lock.
    sp<IGraphicBufferConsumer> consumer(mConsumer);
    mMutex.unlock();
    int res = NO_ERROR;
    if (consumer != nullptr) {
        if (detach) {
            res = consumer->detachBuffer(consumerSlot);
        } else {
            res = consumer->releaseBuffer(consumerSlot, frameNumber,
                    EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, tracker_ptr->getMergedFence());
        }
    } else {
        SP_LOGE("%s: consumer has become null!", __FUNCTION__);
    }
    mMutex.lock();

    if (res != NO_ERROR) {
        if (detach) {
            SP_LOGE("%s: detachBuffer returns %d", __FUNCTION__, res);
        } else {
            SP_LOGE("%s: releaseBuffer returns %d", __FUNCTION__, res);
        }
    } else {
        if (mAcquiredInputBuffers == 0) {
            ALOGW("%s: Acquired input buffer count already at zero!", __FUNCTION__);
        } else {
            mAcquiredInputBuffers--;
        }
    }
}

void Camera3StreamSplitter::onBufferReleasedByOutput(
        const sp<IGraphicBufferProducer>& from) {
    ATRACE_CALL();
    sp<Fence> fence;

    int slot = BufferItem::INVALID_BUFFER_SLOT;
    auto res = from->dequeueBuffer(&slot, &fence, mWidth, mHeight, mFormat, mProducerUsage,
            nullptr, nullptr);
    Mutex::Autolock lock(mMutex);
    handleOutputDequeueStatusLocked(res, slot);
    if (res != OK) {
        return;
    }

    size_t surfaceId = 0;
    bool found = false;
    for (const auto& it : mOutputs) {
        if (it.second == from) {
            found = true;
            surfaceId = it.first;
            break;
        }
    }
    if (!found) {
        SP_LOGV("%s: output surface not registered anymore!", __FUNCTION__);
        return;
    }

    returnOutputBufferLocked(fence, from, surfaceId, slot);
}

void Camera3StreamSplitter::onBufferReplacedLocked(
        const sp<IGraphicBufferProducer>& from, size_t surfaceId) {
    ATRACE_CALL();
    sp<Fence> fence;

    int slot = BufferItem::INVALID_BUFFER_SLOT;
    auto res = from->dequeueBuffer(&slot, &fence, mWidth, mHeight, mFormat, mProducerUsage,
            nullptr, nullptr);
    handleOutputDequeueStatusLocked(res, slot);
    if (res != OK) {
        return;
    }

    returnOutputBufferLocked(fence, from, surfaceId, slot);
}

void Camera3StreamSplitter::returnOutputBufferLocked(const sp<Fence>& fence,
        const sp<IGraphicBufferProducer>& from, size_t surfaceId, int slot) {
    sp<GraphicBuffer> buffer;

    if (mOutputSlots[from] == nullptr) {
        //Output surface got likely removed by client.
        return;
    }

    auto outputSlots = *mOutputSlots[from];
    buffer = outputSlots[slot];
    BufferTracker& tracker = *(mBuffers[buffer->getId()]);
    // Merge the release fence of the incoming buffer so that the fence we send
    // back to the input includes all of the outputs' fences
    if (fence != nullptr && fence->isValid()) {
        tracker.mergeFence(fence);
    }

    auto detachBuffer = mDetachedBuffers.find(buffer->getId());
    bool detach = (detachBuffer != mDetachedBuffers.end());
    if (detach) {
        auto res = from->detachBuffer(slot);
        if (res == NO_ERROR) {
            outputSlots[slot] = nullptr;
        } else {
            SP_LOGE("%s: detach buffer from output failed (%d)", __FUNCTION__, res);
        }
    }

    // Check to see if this is the last outstanding reference to this buffer
    decrementBufRefCountLocked(buffer->getId(), surfaceId);
}

void Camera3StreamSplitter::handleOutputDequeueStatusLocked(status_t res, int slot) {
    if (res == NO_INIT) {
        // If we just discovered that this output has been abandoned, note that,
        // but we can't do anything else, since buffer is invalid
        onAbandonedLocked();
    } else if (res == IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) {
        SP_LOGE("%s: Producer needs to re-allocate buffer!", __FUNCTION__);
        SP_LOGE("%s: This should not happen with buffer allocation disabled!", __FUNCTION__);
    } else if (res == IGraphicBufferProducer::RELEASE_ALL_BUFFERS) {
        SP_LOGE("%s: All slot->buffer mapping should be released!", __FUNCTION__);
        SP_LOGE("%s: This should not happen with buffer allocation disabled!", __FUNCTION__);
    } else if (res == NO_MEMORY) {
        SP_LOGE("%s: No free buffers", __FUNCTION__);
    } else if (res == WOULD_BLOCK) {
        SP_LOGE("%s: Dequeue call will block", __FUNCTION__);
    } else if (res != OK || (slot == BufferItem::INVALID_BUFFER_SLOT)) {
        SP_LOGE("%s: dequeue buffer from output failed (%d)", __FUNCTION__, res);
    }
}

void Camera3StreamSplitter::onAbandonedLocked() {
    // If this is called from binderDied callback, it means the app process
    // holding the binder has died. CameraService will be notified of the binder
    // death, and camera device will be closed, which in turn calls
    // disconnect().
    //
    // If this is called from onBufferReleasedByOutput or onFrameAvailable, one
    // consumer being abanoned shouldn't impact the other consumer. So we won't
    // stop the buffer flow.
    //
    // In both cases, we don't need to do anything here.
    SP_LOGV("One of my outputs has abandoned me");
}

int Camera3StreamSplitter::getSlotForOutputLocked(const sp<IGraphicBufferProducer>& gbp,
        const sp<GraphicBuffer>& gb) {
    auto& outputSlots = *mOutputSlots[gbp];

    for (size_t i = 0; i < outputSlots.size(); i++) {
        if (outputSlots[i] == gb) {
            return (int)i;
        }
    }

    SP_LOGV("%s: Cannot find slot for gb %p on output %p", __FUNCTION__, gb.get(),
            gbp.get());
    return BufferItem::INVALID_BUFFER_SLOT;
}

Camera3StreamSplitter::OutputListener::OutputListener(
        wp<Camera3StreamSplitter> splitter,
        wp<IGraphicBufferProducer> output)
      : mSplitter(splitter), mOutput(output) {}

void Camera3StreamSplitter::OutputListener::onBufferReleased() {
    ATRACE_CALL();
    sp<Camera3StreamSplitter> splitter = mSplitter.promote();
    sp<IGraphicBufferProducer> output = mOutput.promote();
    if (splitter != nullptr && output != nullptr) {
        splitter->onBufferReleasedByOutput(output);
    }
}

void Camera3StreamSplitter::OutputListener::binderDied(const wp<IBinder>& /* who */) {
    sp<Camera3StreamSplitter> splitter = mSplitter.promote();
    if (splitter != nullptr) {
        Mutex::Autolock lock(splitter->mMutex);
        splitter->onAbandonedLocked();
    }
}

Camera3StreamSplitter::BufferTracker::BufferTracker(
        const sp<GraphicBuffer>& buffer, const std::vector<size_t>& requestedSurfaces)
      : mBuffer(buffer), mMergedFence(Fence::NO_FENCE), mRequestedSurfaces(requestedSurfaces),
        mReferenceCount(requestedSurfaces.size()) {}

void Camera3StreamSplitter::BufferTracker::mergeFence(const sp<Fence>& with) {
    mMergedFence = Fence::merge(String8("Camera3StreamSplitter"), mMergedFence, with);
}

size_t Camera3StreamSplitter::BufferTracker::decrementReferenceCountLocked(size_t surfaceId) {
    const auto& it = std::find(mRequestedSurfaces.begin(), mRequestedSurfaces.end(), surfaceId);
    if (it == mRequestedSurfaces.end()) {
        return mReferenceCount;
    } else {
        mRequestedSurfaces.erase(it);
    }

    if (mReferenceCount > 0)
        --mReferenceCount;
    return mReferenceCount;
}

} // namespace android
