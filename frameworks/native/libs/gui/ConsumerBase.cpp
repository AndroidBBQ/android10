/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "ConsumerBase"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <hardware/hardware.h>

#include <cutils/atomic.h>

#include <gui/BufferItem.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ConsumerBase.h>

#include <private/gui/ComposerService.h>

#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Trace.h>

// Macros for including the ConsumerBase name in log messages
#define CB_LOGV(x, ...) ALOGV("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define CB_LOGD(x, ...) ALOGD("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define CB_LOGI(x, ...) ALOGI("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define CB_LOGW(x, ...) ALOGW("[%s] " x, mName.string(), ##__VA_ARGS__)
#define CB_LOGE(x, ...) ALOGE("[%s] " x, mName.string(), ##__VA_ARGS__)

namespace android {

// Get an ID that's unique within this process.
static int32_t createProcessUniqueId() {
    static volatile int32_t globalCounter = 0;
    return android_atomic_inc(&globalCounter);
}

ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue),
        mPrevFinalReleaseFence(Fence::NO_FENCE) {
    // Choose a name using the PID and a process-unique ID.
    mName = String8::format("unnamed-%d-%d", getpid(), createProcessUniqueId());

    // Note that we can't create an sp<...>(this) in a ctor that will not keep a
    // reference once the ctor ends, as that would cause the refcount of 'this'
    // dropping to 0 at the end of the ctor.  Since all we need is a wp<...>
    // that's what we create.
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);
    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

    status_t err = mConsumer->consumerConnect(proxy, controlledByApp);
    if (err != NO_ERROR) {
        CB_LOGE("ConsumerBase: error connecting to BufferQueue: %s (%d)",
                strerror(-err), err);
    } else {
        mConsumer->setConsumerName(mName);
    }
}

ConsumerBase::~ConsumerBase() {
    CB_LOGV("~ConsumerBase");
    Mutex::Autolock lock(mMutex);

    // Verify that abandon() has been called before we get here.  This should
    // be done by ConsumerBase::onLastStrongRef(), but it's possible for a
    // derived class to override that method and not call
    // ConsumerBase::onLastStrongRef().
    LOG_ALWAYS_FATAL_IF(!mAbandoned, "[%s] ~ConsumerBase was called, but the "
        "consumer is not abandoned!", mName.string());
}

void ConsumerBase::onLastStrongRef(const void* id __attribute__((unused))) {
    abandon();
}

void ConsumerBase::freeBufferLocked(int slotIndex) {
    CB_LOGV("freeBufferLocked: slotIndex=%d", slotIndex);
    mSlots[slotIndex].mGraphicBuffer = nullptr;
    mSlots[slotIndex].mFence = Fence::NO_FENCE;
    mSlots[slotIndex].mFrameNumber = 0;
}

void ConsumerBase::onFrameAvailable(const BufferItem& item) {
    CB_LOGV("onFrameAvailable");

    sp<FrameAvailableListener> listener;
    { // scope for the lock
        Mutex::Autolock lock(mFrameAvailableMutex);
        listener = mFrameAvailableListener.promote();
    }

    if (listener != nullptr) {
        CB_LOGV("actually calling onFrameAvailable");
        listener->onFrameAvailable(item);
    }
}

void ConsumerBase::onFrameReplaced(const BufferItem &item) {
    CB_LOGV("onFrameReplaced");

    sp<FrameAvailableListener> listener;
    {
        Mutex::Autolock lock(mFrameAvailableMutex);
        listener = mFrameAvailableListener.promote();
    }

    if (listener != nullptr) {
        CB_LOGV("actually calling onFrameReplaced");
        listener->onFrameReplaced(item);
    }
}

void ConsumerBase::onBuffersReleased() {
    Mutex::Autolock lock(mMutex);

    CB_LOGV("onBuffersReleased");

    if (mAbandoned) {
        // Nothing to do if we're already abandoned.
        return;
    }

    uint64_t mask = 0;
    mConsumer->getReleasedBuffers(&mask);
    for (int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
        if (mask & (1ULL << i)) {
            freeBufferLocked(i);
        }
    }
}

void ConsumerBase::onSidebandStreamChanged() {
}

void ConsumerBase::abandon() {
    CB_LOGV("abandon");
    Mutex::Autolock lock(mMutex);

    if (!mAbandoned) {
        abandonLocked();
        mAbandoned = true;
    }
}

void ConsumerBase::abandonLocked() {
    CB_LOGV("abandonLocked");
    if (mAbandoned) {
        CB_LOGE("abandonLocked: ConsumerBase is abandoned!");
        return;
    }
    for (int i =0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
        freeBufferLocked(i);
    }
    // disconnect from the BufferQueue
    mConsumer->consumerDisconnect();
    mConsumer.clear();
}

bool ConsumerBase::isAbandoned() {
    Mutex::Autolock _l(mMutex);
    return mAbandoned;
}

void ConsumerBase::setName(const String8& name) {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("setName: ConsumerBase is abandoned!");
        return;
    }
    mName = name;
    mConsumer->setConsumerName(name);
}

void ConsumerBase::setFrameAvailableListener(
        const wp<FrameAvailableListener>& listener) {
    CB_LOGV("setFrameAvailableListener");
    Mutex::Autolock lock(mFrameAvailableMutex);
    mFrameAvailableListener = listener;
}

status_t ConsumerBase::detachBuffer(int slot) {
    CB_LOGV("detachBuffer");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        CB_LOGE("detachBuffer: ConsumerBase is abandoned!");
        return NO_INIT;
    }

    status_t result = mConsumer->detachBuffer(slot);
    if (result != NO_ERROR) {
        CB_LOGE("Failed to detach buffer: %d", result);
        return result;
    }

    freeBufferLocked(slot);

    return result;
}

status_t ConsumerBase::setDefaultBufferSize(uint32_t width, uint32_t height) {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("setDefaultBufferSize: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setDefaultBufferSize(width, height);
}

status_t ConsumerBase::setDefaultBufferFormat(PixelFormat defaultFormat) {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("setDefaultBufferFormat: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setDefaultBufferFormat(defaultFormat);
}

status_t ConsumerBase::setDefaultBufferDataSpace(
        android_dataspace defaultDataSpace) {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("setDefaultBufferDataSpace: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setDefaultBufferDataSpace(defaultDataSpace);
}

status_t ConsumerBase::setConsumerUsageBits(uint64_t usage) {
    Mutex::Autolock lock(mMutex);
    if (mAbandoned) {
        CB_LOGE("setConsumerUsageBits: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setConsumerUsageBits(usage);
}

status_t ConsumerBase::setTransformHint(uint32_t hint) {
    Mutex::Autolock lock(mMutex);
    if (mAbandoned) {
        CB_LOGE("setTransformHint: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setTransformHint(hint);
}

status_t ConsumerBase::setMaxAcquiredBufferCount(int maxAcquiredBuffers) {
    Mutex::Autolock lock(mMutex);
    if (mAbandoned) {
        CB_LOGE("setMaxAcquiredBufferCount: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->setMaxAcquiredBufferCount(maxAcquiredBuffers);
}

sp<NativeHandle> ConsumerBase::getSidebandStream() const {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("getSidebandStream: ConsumerBase is abandoned!");
        return nullptr;
    }

    sp<NativeHandle> stream;
    status_t err = mConsumer->getSidebandStream(&stream);
    if (err != NO_ERROR) {
        CB_LOGE("failed to get sideband stream: %d", err);
        return nullptr;
    }

    return stream;
}

status_t ConsumerBase::getOccupancyHistory(bool forceFlush,
        std::vector<OccupancyTracker::Segment>* outHistory) {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("getOccupancyHistory: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    return mConsumer->getOccupancyHistory(forceFlush, outHistory);
}

status_t ConsumerBase::discardFreeBuffers() {
    Mutex::Autolock _l(mMutex);
    if (mAbandoned) {
        CB_LOGE("discardFreeBuffers: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    status_t err = mConsumer->discardFreeBuffers();
    if (err != OK) {
        return err;
    }
    uint64_t mask;
    mConsumer->getReleasedBuffers(&mask);
    for (int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
        if (mask & (1ULL << i)) {
            freeBufferLocked(i);
        }
    }
    return OK;
}

void ConsumerBase::dumpState(String8& result) const {
    dumpState(result, "");
}

void ConsumerBase::dumpState(String8& result, const char* prefix) const {
    Mutex::Autolock _l(mMutex);
    dumpLocked(result, prefix);
}

void ConsumerBase::dumpLocked(String8& result, const char* prefix) const {
    result.appendFormat("%smAbandoned=%d\n", prefix, int(mAbandoned));

    if (!mAbandoned) {
        String8 consumerState;
        mConsumer->dumpState(String8(prefix), &consumerState);
        result.append(consumerState);
    }
}

status_t ConsumerBase::acquireBufferLocked(BufferItem *item,
        nsecs_t presentWhen, uint64_t maxFrameNumber) {
    if (mAbandoned) {
        CB_LOGE("acquireBufferLocked: ConsumerBase is abandoned!");
        return NO_INIT;
    }

    status_t err = mConsumer->acquireBuffer(item, presentWhen, maxFrameNumber);
    if (err != NO_ERROR) {
        return err;
    }

    if (item->mGraphicBuffer != nullptr) {
        if (mSlots[item->mSlot].mGraphicBuffer != nullptr) {
            freeBufferLocked(item->mSlot);
        }
        mSlots[item->mSlot].mGraphicBuffer = item->mGraphicBuffer;
    }

    mSlots[item->mSlot].mFrameNumber = item->mFrameNumber;
    mSlots[item->mSlot].mFence = item->mFence;

    CB_LOGV("acquireBufferLocked: -> slot=%d/%" PRIu64,
            item->mSlot, item->mFrameNumber);

    return OK;
}

status_t ConsumerBase::addReleaseFence(int slot,
        const sp<GraphicBuffer> graphicBuffer, const sp<Fence>& fence) {
    Mutex::Autolock lock(mMutex);
    return addReleaseFenceLocked(slot, graphicBuffer, fence);
}

status_t ConsumerBase::addReleaseFenceLocked(int slot,
        const sp<GraphicBuffer> graphicBuffer, const sp<Fence>& fence) {
    CB_LOGV("addReleaseFenceLocked: slot=%d", slot);

    // If consumer no longer tracks this graphicBuffer, we can safely
    // drop this fence, as it will never be received by the producer.
    if (!stillTracking(slot, graphicBuffer)) {
        return OK;
    }

    if (!mSlots[slot].mFence.get()) {
        mSlots[slot].mFence = fence;
        return OK;
    }

    // Check status of fences first because merging is expensive.
    // Merging an invalid fence with any other fence results in an
    // invalid fence.
    auto currentStatus = mSlots[slot].mFence->getStatus();
    if (currentStatus == Fence::Status::Invalid) {
        CB_LOGE("Existing fence has invalid state");
        return BAD_VALUE;
    }

    auto incomingStatus = fence->getStatus();
    if (incomingStatus == Fence::Status::Invalid) {
        CB_LOGE("New fence has invalid state");
        mSlots[slot].mFence = fence;
        return BAD_VALUE;
    }

    // If both fences are signaled or both are unsignaled, we need to merge
    // them to get an accurate timestamp.
    if (currentStatus == incomingStatus) {
        char fenceName[32] = {};
        snprintf(fenceName, 32, "%.28s:%d", mName.string(), slot);
        sp<Fence> mergedFence = Fence::merge(
                fenceName, mSlots[slot].mFence, fence);
        if (!mergedFence.get()) {
            CB_LOGE("failed to merge release fences");
            // synchronization is broken, the best we can do is hope fences
            // signal in order so the new fence will act like a union
            mSlots[slot].mFence = fence;
            return BAD_VALUE;
        }
        mSlots[slot].mFence = mergedFence;
    } else if (incomingStatus == Fence::Status::Unsignaled) {
        // If one fence has signaled and the other hasn't, the unsignaled
        // fence will approximately correspond with the correct timestamp.
        // There's a small race if both fences signal at about the same time
        // and their statuses are retrieved with unfortunate timing. However,
        // by this point, they will have both signaled and only the timestamp
        // will be slightly off; any dependencies after this point will
        // already have been met.
        mSlots[slot].mFence = fence;
    }
    // else if (currentStatus == Fence::Status::Unsignaled) is a no-op.

    return OK;
}

status_t ConsumerBase::releaseBufferLocked(
        int slot, const sp<GraphicBuffer> graphicBuffer,
        EGLDisplay display, EGLSyncKHR eglFence) {
    if (mAbandoned) {
        CB_LOGE("releaseBufferLocked: ConsumerBase is abandoned!");
        return NO_INIT;
    }
    // If consumer no longer tracks this graphicBuffer (we received a new
    // buffer on the same slot), the buffer producer is definitely no longer
    // tracking it.
    if (!stillTracking(slot, graphicBuffer)) {
        return OK;
    }

    CB_LOGV("releaseBufferLocked: slot=%d/%" PRIu64,
            slot, mSlots[slot].mFrameNumber);
    status_t err = mConsumer->releaseBuffer(slot, mSlots[slot].mFrameNumber,
            display, eglFence, mSlots[slot].mFence);
    if (err == IGraphicBufferConsumer::STALE_BUFFER_SLOT) {
        freeBufferLocked(slot);
    }

    mPrevFinalReleaseFence = mSlots[slot].mFence;
    mSlots[slot].mFence = Fence::NO_FENCE;

    return err;
}

bool ConsumerBase::stillTracking(int slot,
        const sp<GraphicBuffer> graphicBuffer) {
    if (slot < 0 || slot >= BufferQueue::NUM_BUFFER_SLOTS) {
        return false;
    }
    return (mSlots[slot].mGraphicBuffer != nullptr &&
            mSlots[slot].mGraphicBuffer->handle == graphicBuffer->handle);
}

} // namespace android
