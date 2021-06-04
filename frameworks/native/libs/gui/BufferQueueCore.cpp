/*
 * Copyright 2014 The Android Open Source Project
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

#define LOG_TAG "BufferQueueCore"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#define EGL_EGLEXT_PROTOTYPES

#if DEBUG_ONLY_CODE
#define VALIDATE_CONSISTENCY() do { validateConsistencyLocked(); } while (0)
#else
#define VALIDATE_CONSISTENCY()
#endif

#include <inttypes.h>

#include <cutils/properties.h>
#include <cutils/atomic.h>

#include <gui/BufferItem.h>
#include <gui/BufferQueueCore.h>
#include <gui/IConsumerListener.h>
#include <gui/IProducerListener.h>
#include <gui/ISurfaceComposer.h>
#include <private/gui/ComposerService.h>

#include <system/window.h>

namespace android {

static String8 getUniqueName() {
    static volatile int32_t counter = 0;
    return String8::format("unnamed-%d-%d", getpid(),
            android_atomic_inc(&counter));
}

static uint64_t getUniqueId() {
    static std::atomic<uint32_t> counter{0};
    static uint64_t id = static_cast<uint64_t>(getpid()) << 32;
    return id | counter++;
}

BufferQueueCore::BufferQueueCore() :
    mMutex(),
    mIsAbandoned(false),
    mConsumerControlledByApp(false),
    mConsumerName(getUniqueName()),
    mConsumerListener(),
    mConsumerUsageBits(0),
    mConsumerIsProtected(false),
    mConnectedApi(NO_CONNECTED_API),
    mLinkedToDeath(),
    mConnectedProducerListener(),
    mBufferReleasedCbEnabled(false),
    mSlots(),
    mQueue(),
    mFreeSlots(),
    mFreeBuffers(),
    mUnusedSlots(),
    mActiveBuffers(),
    mDequeueCondition(),
    mDequeueBufferCannotBlock(false),
    mQueueBufferCanDrop(false),
    mLegacyBufferDrop(true),
    mDefaultBufferFormat(PIXEL_FORMAT_RGBA_8888),
    mDefaultWidth(1),
    mDefaultHeight(1),
    mDefaultBufferDataSpace(HAL_DATASPACE_UNKNOWN),
    mMaxBufferCount(BufferQueueDefs::NUM_BUFFER_SLOTS),
    mMaxAcquiredBufferCount(1),
    mMaxDequeuedBufferCount(1),
    mBufferHasBeenQueued(false),
    mFrameCounter(0),
    mTransformHint(0),
    mIsAllocating(false),
    mIsAllocatingCondition(),
    mAllowAllocation(true),
    mBufferAge(0),
    mGenerationNumber(0),
    mAsyncMode(false),
    mSharedBufferMode(false),
    mAutoRefresh(false),
    mSharedBufferSlot(INVALID_BUFFER_SLOT),
    mSharedBufferCache(Rect::INVALID_RECT, 0, NATIVE_WINDOW_SCALING_MODE_FREEZE,
            HAL_DATASPACE_UNKNOWN),
    mLastQueuedSlot(INVALID_BUFFER_SLOT),
    mUniqueId(getUniqueId())
{
    int numStartingBuffers = getMaxBufferCountLocked();
    for (int s = 0; s < numStartingBuffers; s++) {
        mFreeSlots.insert(s);
    }
    for (int s = numStartingBuffers; s < BufferQueueDefs::NUM_BUFFER_SLOTS;
            s++) {
        mUnusedSlots.push_front(s);
    }
}

BufferQueueCore::~BufferQueueCore() {}

void BufferQueueCore::dumpState(const String8& prefix, String8* outResult) const {
    std::lock_guard<std::mutex> lock(mMutex);

    outResult->appendFormat("%s- BufferQueue ", prefix.string());
    outResult->appendFormat("mMaxAcquiredBufferCount=%d mMaxDequeuedBufferCount=%d\n",
                            mMaxAcquiredBufferCount, mMaxDequeuedBufferCount);
    outResult->appendFormat("%s  mDequeueBufferCannotBlock=%d mAsyncMode=%d\n", prefix.string(),
                            mDequeueBufferCannotBlock, mAsyncMode);
    outResult->appendFormat("%s  mQueueBufferCanDrop=%d mLegacyBufferDrop=%d\n", prefix.string(),
                            mQueueBufferCanDrop, mLegacyBufferDrop);
    outResult->appendFormat("%s  default-size=[%dx%d] default-format=%d ", prefix.string(),
                            mDefaultWidth, mDefaultHeight, mDefaultBufferFormat);
    outResult->appendFormat("transform-hint=%02x frame-counter=%" PRIu64, mTransformHint,
                            mFrameCounter);

    outResult->appendFormat("\n%sFIFO(%zu):\n", prefix.string(), mQueue.size());
    Fifo::const_iterator current(mQueue.begin());
    while (current != mQueue.end()) {
        double timestamp = current->mTimestamp / 1e9;
        outResult->appendFormat("%s  %02d:%p ", prefix.string(), current->mSlot,
                                current->mGraphicBuffer.get());
        outResult->appendFormat("crop=[%d,%d,%d,%d] ", current->mCrop.left, current->mCrop.top,
                                current->mCrop.right, current->mCrop.bottom);
        outResult->appendFormat("xform=0x%02x time=%.4f scale=%s\n", current->mTransform, timestamp,
                                BufferItem::scalingModeName(current->mScalingMode));
        ++current;
    }

    outResult->appendFormat("%sSlots:\n", prefix.string());
    for (int s : mActiveBuffers) {
        const sp<GraphicBuffer>& buffer(mSlots[s].mGraphicBuffer);
        // A dequeued buffer might be null if it's still being allocated
        if (buffer.get()) {
            outResult->appendFormat("%s %s[%02d:%p] ", prefix.string(),
                                    (mSlots[s].mBufferState.isAcquired()) ? ">" : " ", s,
                                    buffer.get());
            outResult->appendFormat("state=%-8s %p frame=%" PRIu64, mSlots[s].mBufferState.string(),
                                    buffer->handle, mSlots[s].mFrameNumber);
            outResult->appendFormat(" [%4ux%4u:%4u,%3X]\n", buffer->width, buffer->height,
                                    buffer->stride, buffer->format);
        } else {
            outResult->appendFormat("%s  [%02d:%p] ", prefix.string(), s, buffer.get());
            outResult->appendFormat("state=%-8s frame=%" PRIu64 "\n",
                                    mSlots[s].mBufferState.string(), mSlots[s].mFrameNumber);
        }
    }
    for (int s : mFreeBuffers) {
        const sp<GraphicBuffer>& buffer(mSlots[s].mGraphicBuffer);
        outResult->appendFormat("%s  [%02d:%p] ", prefix.string(), s, buffer.get());
        outResult->appendFormat("state=%-8s %p frame=%" PRIu64, mSlots[s].mBufferState.string(),
                                buffer->handle, mSlots[s].mFrameNumber);
        outResult->appendFormat(" [%4ux%4u:%4u,%3X]\n", buffer->width, buffer->height,
                                buffer->stride, buffer->format);
    }

    for (int s : mFreeSlots) {
        const sp<GraphicBuffer>& buffer(mSlots[s].mGraphicBuffer);
        outResult->appendFormat("%s  [%02d:%p] state=%-8s\n", prefix.string(), s, buffer.get(),
                                mSlots[s].mBufferState.string());
    }
}

int BufferQueueCore::getMinUndequeuedBufferCountLocked() const {
    // If dequeueBuffer is allowed to error out, we don't have to add an
    // extra buffer.
    if (mAsyncMode || mDequeueBufferCannotBlock) {
        return mMaxAcquiredBufferCount + 1;
    }

    return mMaxAcquiredBufferCount;
}

int BufferQueueCore::getMinMaxBufferCountLocked() const {
    return getMinUndequeuedBufferCountLocked() + 1;
}

int BufferQueueCore::getMaxBufferCountLocked(bool asyncMode,
        bool dequeueBufferCannotBlock, int maxBufferCount) const {
    int maxCount = mMaxAcquiredBufferCount + mMaxDequeuedBufferCount +
            ((asyncMode || dequeueBufferCannotBlock) ? 1 : 0);
    maxCount = std::min(maxBufferCount, maxCount);
    return maxCount;
}

int BufferQueueCore::getMaxBufferCountLocked() const {
    int maxBufferCount = mMaxAcquiredBufferCount + mMaxDequeuedBufferCount +
            ((mAsyncMode || mDequeueBufferCannotBlock) ? 1 : 0);

    // limit maxBufferCount by mMaxBufferCount always
    maxBufferCount = std::min(mMaxBufferCount, maxBufferCount);

    return maxBufferCount;
}

void BufferQueueCore::clearBufferSlotLocked(int slot) {
    BQ_LOGV("clearBufferSlotLocked: slot %d", slot);

    mSlots[slot].mGraphicBuffer.clear();
    mSlots[slot].mBufferState.reset();
    mSlots[slot].mRequestBufferCalled = false;
    mSlots[slot].mFrameNumber = 0;
    mSlots[slot].mAcquireCalled = false;
    mSlots[slot].mNeedsReallocation = true;

    // Destroy fence as BufferQueue now takes ownership
    if (mSlots[slot].mEglFence != EGL_NO_SYNC_KHR) {
        eglDestroySyncKHR(mSlots[slot].mEglDisplay, mSlots[slot].mEglFence);
        mSlots[slot].mEglFence = EGL_NO_SYNC_KHR;
    }
    mSlots[slot].mFence = Fence::NO_FENCE;
    mSlots[slot].mEglDisplay = EGL_NO_DISPLAY;

    if (mLastQueuedSlot == slot) {
        mLastQueuedSlot = INVALID_BUFFER_SLOT;
    }
}

void BufferQueueCore::freeAllBuffersLocked() {
    for (int s : mFreeSlots) {
        clearBufferSlotLocked(s);
    }

    for (int s : mFreeBuffers) {
        mFreeSlots.insert(s);
        clearBufferSlotLocked(s);
    }
    mFreeBuffers.clear();

    for (int s : mActiveBuffers) {
        mFreeSlots.insert(s);
        clearBufferSlotLocked(s);
    }
    mActiveBuffers.clear();

    for (auto& b : mQueue) {
        b.mIsStale = true;

        // We set this to false to force the BufferQueue to resend the buffer
        // handle upon acquire, since if we're here due to a producer
        // disconnect, the consumer will have been told to purge its cache of
        // slot-to-buffer-handle mappings and will not be able to otherwise
        // obtain a valid buffer handle.
        b.mAcquireCalled = false;
    }

    VALIDATE_CONSISTENCY();
}

void BufferQueueCore::discardFreeBuffersLocked() {
    // Notify producer about the discarded buffers.
    if (mConnectedProducerListener != nullptr && mFreeBuffers.size() > 0) {
        std::vector<int32_t> freeBuffers(mFreeBuffers.begin(), mFreeBuffers.end());
        mConnectedProducerListener->onBuffersDiscarded(freeBuffers);
    }

    for (int s : mFreeBuffers) {
        mFreeSlots.insert(s);
        clearBufferSlotLocked(s);
    }
    mFreeBuffers.clear();

    VALIDATE_CONSISTENCY();
}

bool BufferQueueCore::adjustAvailableSlotsLocked(int delta) {
    if (delta >= 0) {
        // If we're going to fail, do so before modifying anything
        if (delta > static_cast<int>(mUnusedSlots.size())) {
            return false;
        }
        while (delta > 0) {
            if (mUnusedSlots.empty()) {
                return false;
            }
            int slot = mUnusedSlots.back();
            mUnusedSlots.pop_back();
            mFreeSlots.insert(slot);
            delta--;
        }
    } else {
        // If we're going to fail, do so before modifying anything
        if (-delta > static_cast<int>(mFreeSlots.size() +
                mFreeBuffers.size())) {
            return false;
        }
        while (delta < 0) {
            if (!mFreeSlots.empty()) {
                auto slot = mFreeSlots.begin();
                clearBufferSlotLocked(*slot);
                mUnusedSlots.push_back(*slot);
                mFreeSlots.erase(slot);
            } else if (!mFreeBuffers.empty()) {
                int slot = mFreeBuffers.back();
                clearBufferSlotLocked(slot);
                mUnusedSlots.push_back(slot);
                mFreeBuffers.pop_back();
            } else {
                return false;
            }
            delta++;
        }
    }
    return true;
}

void BufferQueueCore::waitWhileAllocatingLocked(std::unique_lock<std::mutex>& lock) const {
    ATRACE_CALL();
    while (mIsAllocating) {
        mIsAllocatingCondition.wait(lock);
    }
}

#if DEBUG_ONLY_CODE
void BufferQueueCore::validateConsistencyLocked() const {
    static const useconds_t PAUSE_TIME = 0;
    int allocatedSlots = 0;
    for (int slot = 0; slot < BufferQueueDefs::NUM_BUFFER_SLOTS; ++slot) {
        bool isInFreeSlots = mFreeSlots.count(slot) != 0;
        bool isInFreeBuffers =
                std::find(mFreeBuffers.cbegin(), mFreeBuffers.cend(), slot) !=
                mFreeBuffers.cend();
        bool isInActiveBuffers = mActiveBuffers.count(slot) != 0;
        bool isInUnusedSlots =
                std::find(mUnusedSlots.cbegin(), mUnusedSlots.cend(), slot) !=
                mUnusedSlots.cend();

        if (isInFreeSlots || isInFreeBuffers || isInActiveBuffers) {
            allocatedSlots++;
        }

        if (isInUnusedSlots) {
            if (isInFreeSlots) {
                BQ_LOGE("Slot %d is in mUnusedSlots and in mFreeSlots", slot);
                usleep(PAUSE_TIME);
            }
            if (isInFreeBuffers) {
                BQ_LOGE("Slot %d is in mUnusedSlots and in mFreeBuffers", slot);
                usleep(PAUSE_TIME);
            }
            if (isInActiveBuffers) {
                BQ_LOGE("Slot %d is in mUnusedSlots and in mActiveBuffers",
                        slot);
                usleep(PAUSE_TIME);
            }
            if (!mSlots[slot].mBufferState.isFree()) {
                BQ_LOGE("Slot %d is in mUnusedSlots but is not FREE", slot);
                usleep(PAUSE_TIME);
            }
            if (mSlots[slot].mGraphicBuffer != nullptr) {
                BQ_LOGE("Slot %d is in mUnusedSluts but has an active buffer",
                        slot);
                usleep(PAUSE_TIME);
            }
        } else if (isInFreeSlots) {
            if (isInUnusedSlots) {
                BQ_LOGE("Slot %d is in mFreeSlots and in mUnusedSlots", slot);
                usleep(PAUSE_TIME);
            }
            if (isInFreeBuffers) {
                BQ_LOGE("Slot %d is in mFreeSlots and in mFreeBuffers", slot);
                usleep(PAUSE_TIME);
            }
            if (isInActiveBuffers) {
                BQ_LOGE("Slot %d is in mFreeSlots and in mActiveBuffers", slot);
                usleep(PAUSE_TIME);
            }
            if (!mSlots[slot].mBufferState.isFree()) {
                BQ_LOGE("Slot %d is in mFreeSlots but is not FREE", slot);
                usleep(PAUSE_TIME);
            }
            if (mSlots[slot].mGraphicBuffer != nullptr) {
                BQ_LOGE("Slot %d is in mFreeSlots but has a buffer",
                        slot);
                usleep(PAUSE_TIME);
            }
        } else if (isInFreeBuffers) {
            if (isInUnusedSlots) {
                BQ_LOGE("Slot %d is in mFreeBuffers and in mUnusedSlots", slot);
                usleep(PAUSE_TIME);
            }
            if (isInFreeSlots) {
                BQ_LOGE("Slot %d is in mFreeBuffers and in mFreeSlots", slot);
                usleep(PAUSE_TIME);
            }
            if (isInActiveBuffers) {
                BQ_LOGE("Slot %d is in mFreeBuffers and in mActiveBuffers",
                        slot);
                usleep(PAUSE_TIME);
            }
            if (!mSlots[slot].mBufferState.isFree()) {
                BQ_LOGE("Slot %d is in mFreeBuffers but is not FREE", slot);
                usleep(PAUSE_TIME);
            }
            if (mSlots[slot].mGraphicBuffer == nullptr) {
                BQ_LOGE("Slot %d is in mFreeBuffers but has no buffer", slot);
                usleep(PAUSE_TIME);
            }
        } else if (isInActiveBuffers) {
            if (isInUnusedSlots) {
                BQ_LOGE("Slot %d is in mActiveBuffers and in mUnusedSlots",
                        slot);
                usleep(PAUSE_TIME);
            }
            if (isInFreeSlots) {
                BQ_LOGE("Slot %d is in mActiveBuffers and in mFreeSlots", slot);
                usleep(PAUSE_TIME);
            }
            if (isInFreeBuffers) {
                BQ_LOGE("Slot %d is in mActiveBuffers and in mFreeBuffers",
                        slot);
                usleep(PAUSE_TIME);
            }
            if (mSlots[slot].mBufferState.isFree() &&
                    !mSlots[slot].mBufferState.isShared()) {
                BQ_LOGE("Slot %d is in mActiveBuffers but is FREE", slot);
                usleep(PAUSE_TIME);
            }
            if (mSlots[slot].mGraphicBuffer == nullptr && !mIsAllocating) {
                BQ_LOGE("Slot %d is in mActiveBuffers but has no buffer", slot);
                usleep(PAUSE_TIME);
            }
        } else {
            BQ_LOGE("Slot %d isn't in any of mUnusedSlots, mFreeSlots, "
                    "mFreeBuffers, or mActiveBuffers", slot);
            usleep(PAUSE_TIME);
        }
    }

    if (allocatedSlots != getMaxBufferCountLocked()) {
        BQ_LOGE("Number of allocated slots is incorrect. Allocated = %d, "
                "Should be %d (%zu free slots, %zu free buffers, "
                "%zu activeBuffers, %zu unusedSlots)", allocatedSlots,
                getMaxBufferCountLocked(), mFreeSlots.size(),
                mFreeBuffers.size(), mActiveBuffers.size(),
                mUnusedSlots.size());
    }
}
#endif

} // namespace android
