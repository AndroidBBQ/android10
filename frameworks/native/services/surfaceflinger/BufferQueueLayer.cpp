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

#undef LOG_TAG
#define LOG_TAG "BufferQueueLayer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <compositionengine/Display.h>
#include <compositionengine/Layer.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <gui/BufferQueueConsumer.h>
#include <system/window.h>

#include "BufferQueueLayer.h"
#include "LayerRejecter.h"
#include "SurfaceInterceptor.h"

#include "TimeStats/TimeStats.h"

namespace android {

BufferQueueLayer::BufferQueueLayer(const LayerCreationArgs& args) : BufferLayer(args) {}

BufferQueueLayer::~BufferQueueLayer() {
    mConsumer->abandon();
}

// -----------------------------------------------------------------------
// Interface implementation for Layer
// -----------------------------------------------------------------------

void BufferQueueLayer::onLayerDisplayed(const sp<Fence>& releaseFence) {
    mConsumer->setReleaseFence(releaseFence);
}

void BufferQueueLayer::setTransformHint(uint32_t orientation) const {
    mConsumer->setTransformHint(orientation);
}

std::vector<OccupancyTracker::Segment> BufferQueueLayer::getOccupancyHistory(bool forceFlush) {
    std::vector<OccupancyTracker::Segment> history;
    status_t result = mConsumer->getOccupancyHistory(forceFlush, &history);
    if (result != NO_ERROR) {
        ALOGW("[%s] Failed to obtain occupancy history (%d)", mName.string(), result);
        return {};
    }
    return history;
}

bool BufferQueueLayer::getTransformToDisplayInverse() const {
    return mConsumer->getTransformToDisplayInverse();
}

void BufferQueueLayer::releasePendingBuffer(nsecs_t dequeueReadyTime) {
    if (!mConsumer->releasePendingBuffer()) {
        return;
    }

    auto releaseFenceTime = std::make_shared<FenceTime>(mConsumer->getPrevFinalReleaseFence());
    mReleaseTimeline.updateSignalTimes();
    mReleaseTimeline.push(releaseFenceTime);

    Mutex::Autolock lock(mFrameEventHistoryMutex);
    if (mPreviousFrameNumber != 0) {
        mFrameEventHistory.addRelease(mPreviousFrameNumber, dequeueReadyTime,
                                      std::move(releaseFenceTime));
    }
}

void BufferQueueLayer::setDefaultBufferSize(uint32_t w, uint32_t h) {
    mConsumer->setDefaultBufferSize(w, h);
}

int32_t BufferQueueLayer::getQueuedFrameCount() const {
    return mQueuedFrames;
}

bool BufferQueueLayer::shouldPresentNow(nsecs_t expectedPresentTime) const {
    if (getSidebandStreamChanged() || getAutoRefresh()) {
        return true;
    }

    if (!hasFrameUpdate()) {
        return false;
    }

    Mutex::Autolock lock(mQueueItemLock);

    const int64_t addedTime = mQueueItems[0].mTimestamp;

    // Ignore timestamps more than a second in the future
    const bool isPlausible = addedTime < (expectedPresentTime + s2ns(1));
    ALOGW_IF(!isPlausible,
             "[%s] Timestamp %" PRId64 " seems implausible "
             "relative to expectedPresent %" PRId64,
             mName.string(), addedTime, expectedPresentTime);

    const bool isDue = addedTime < expectedPresentTime;
    return isDue || !isPlausible;
}

// -----------------------------------------------------------------------
// Interface implementation for BufferLayer
// -----------------------------------------------------------------------

bool BufferQueueLayer::fenceHasSignaled() const {
    if (latchUnsignaledBuffers()) {
        return true;
    }

    if (!hasFrameUpdate()) {
        return true;
    }

    Mutex::Autolock lock(mQueueItemLock);
    if (mQueueItems[0].mIsDroppable) {
        // Even though this buffer's fence may not have signaled yet, it could
        // be replaced by another buffer before it has a chance to, which means
        // that it's possible to get into a situation where a buffer is never
        // able to be latched. To avoid this, grab this buffer anyway.
        return true;
    }
    return mQueueItems[0].mFenceTime->getSignalTime() != Fence::SIGNAL_TIME_PENDING;
}

bool BufferQueueLayer::framePresentTimeIsCurrent() const {
    if (!hasFrameUpdate() || isRemovedFromCurrentState()) {
        return true;
    }

    Mutex::Autolock lock(mQueueItemLock);
    return mQueueItems[0].mTimestamp <= mFlinger->getExpectedPresentTime();
}

nsecs_t BufferQueueLayer::getDesiredPresentTime() {
    return mConsumer->getTimestamp();
}

std::shared_ptr<FenceTime> BufferQueueLayer::getCurrentFenceTime() const {
    return mConsumer->getCurrentFenceTime();
}

void BufferQueueLayer::getDrawingTransformMatrix(float *matrix) {
    return mConsumer->getTransformMatrix(matrix);
}

// NOTE: SurfaceFlinger's definitions of "Current" and "Drawing" do not neatly map to BufferQueue's
// These functions get the fields for the frame that is currently in SurfaceFlinger's Drawing state
// so the functions start with "getDrawing". The data is retrieved from the BufferQueueConsumer's
// current buffer so the consumer functions start with "getCurrent".
//
// This results in the rather confusing functions below.
uint32_t BufferQueueLayer::getDrawingTransform() const {
    return mConsumer->getCurrentTransform();
}

ui::Dataspace BufferQueueLayer::getDrawingDataSpace() const {
    return mConsumer->getCurrentDataSpace();
}

Rect BufferQueueLayer::getDrawingCrop() const {
    return mConsumer->getCurrentCrop();
}

uint32_t BufferQueueLayer::getDrawingScalingMode() const {
    return mConsumer->getCurrentScalingMode();
}

Region BufferQueueLayer::getDrawingSurfaceDamage() const {
    return mConsumer->getSurfaceDamage();
}

const HdrMetadata& BufferQueueLayer::getDrawingHdrMetadata() const {
    return mConsumer->getCurrentHdrMetadata();
}

int BufferQueueLayer::getDrawingApi() const {
    return mConsumer->getCurrentApi();
}

PixelFormat BufferQueueLayer::getPixelFormat() const {
    return mFormat;
}

uint64_t BufferQueueLayer::getFrameNumber() const {
    Mutex::Autolock lock(mQueueItemLock);
    uint64_t frameNumber = mQueueItems[0].mFrameNumber;

    // The head of the queue will be dropped if there are signaled and timely frames behind it
    nsecs_t expectedPresentTime = mFlinger->getExpectedPresentTime();

    if (isRemovedFromCurrentState()) {
        expectedPresentTime = 0;
    }

    for (int i = 1; i < mQueueItems.size(); i++) {
        const bool fenceSignaled =
                mQueueItems[i].mFenceTime->getSignalTime() != Fence::SIGNAL_TIME_PENDING;
        if (!fenceSignaled) {
            break;
        }

        // We don't drop frames without explicit timestamps
        if (mQueueItems[i].mIsAutoTimestamp) {
            break;
        }

        const nsecs_t desiredPresent = mQueueItems[i].mTimestamp;
        if (desiredPresent < expectedPresentTime - BufferQueueConsumer::MAX_REASONABLE_NSEC ||
            desiredPresent > expectedPresentTime) {
            break;
        }

        frameNumber = mQueueItems[i].mFrameNumber;
    }

    return frameNumber;
}

bool BufferQueueLayer::getAutoRefresh() const {
    return mAutoRefresh;
}

bool BufferQueueLayer::getSidebandStreamChanged() const {
    return mSidebandStreamChanged;
}

bool BufferQueueLayer::latchSidebandStream(bool& recomputeVisibleRegions) {
    bool sidebandStreamChanged = true;
    if (mSidebandStreamChanged.compare_exchange_strong(sidebandStreamChanged, false)) {
        // mSidebandStreamChanged was changed to false
        auto& layerCompositionState = getCompositionLayer()->editState().frontEnd;
        layerCompositionState.sidebandStream = mConsumer->getSidebandStream();
        if (layerCompositionState.sidebandStream != nullptr) {
            setTransactionFlags(eTransactionNeeded);
            mFlinger->setTransactionFlags(eTraversalNeeded);
        }
        recomputeVisibleRegions = true;

        return true;
    }
    return false;
}

bool BufferQueueLayer::hasFrameUpdate() const {
    return mQueuedFrames > 0;
}

void BufferQueueLayer::setFilteringEnabled(bool enabled) {
    return mConsumer->setFilteringEnabled(enabled);
}

status_t BufferQueueLayer::bindTextureImage() {
    return mConsumer->bindTextureImage();
}

status_t BufferQueueLayer::updateTexImage(bool& recomputeVisibleRegions, nsecs_t latchTime) {
    // This boolean is used to make sure that SurfaceFlinger's shadow copy
    // of the buffer queue isn't modified when the buffer queue is returning
    // BufferItem's that weren't actually queued. This can happen in shared
    // buffer mode.
    bool queuedBuffer = false;
    const int32_t layerID = getSequence();
    LayerRejecter r(mDrawingState, getCurrentState(), recomputeVisibleRegions,
                    getProducerStickyTransform() != 0, mName.string(), mOverrideScalingMode,
                    getTransformToDisplayInverse(), mFreezeGeometryUpdates);

    nsecs_t expectedPresentTime = mFlinger->getExpectedPresentTime();

    if (isRemovedFromCurrentState()) {
        expectedPresentTime = 0;
    }

    // updateTexImage() below might drop the some buffers at the head of the queue if there is a
    // buffer behind them which is timely to be presented. However this buffer may not be signaled
    // yet. The code below makes sure that this wouldn't happen by setting maxFrameNumber to the
    // last buffer that was signaled.
    uint64_t lastSignaledFrameNumber = mLastFrameNumberReceived;
    {
        Mutex::Autolock lock(mQueueItemLock);
        for (int i = 0; i < mQueueItems.size(); i++) {
            bool fenceSignaled =
                    mQueueItems[i].mFenceTime->getSignalTime() != Fence::SIGNAL_TIME_PENDING;
            if (!fenceSignaled) {
                break;
            }
            lastSignaledFrameNumber = mQueueItems[i].mFrameNumber;
        }
    }
    const uint64_t maxFrameNumberToAcquire =
            std::min(mLastFrameNumberReceived.load(), lastSignaledFrameNumber);

    status_t updateResult = mConsumer->updateTexImage(&r, expectedPresentTime, &mAutoRefresh,
                                                      &queuedBuffer, maxFrameNumberToAcquire);
    if (updateResult == BufferQueue::PRESENT_LATER) {
        // Producer doesn't want buffer to be displayed yet.  Signal a
        // layer update so we check again at the next opportunity.
        mFlinger->signalLayerUpdate();
        return BAD_VALUE;
    } else if (updateResult == BufferLayerConsumer::BUFFER_REJECTED) {
        // If the buffer has been rejected, remove it from the shadow queue
        // and return early
        if (queuedBuffer) {
            Mutex::Autolock lock(mQueueItemLock);
            mConsumer->mergeSurfaceDamage(mQueueItems[0].mSurfaceDamage);
            mFlinger->mTimeStats->removeTimeRecord(layerID, mQueueItems[0].mFrameNumber);
            mQueueItems.removeAt(0);
            mQueuedFrames--;
        }
        return BAD_VALUE;
    } else if (updateResult != NO_ERROR || mUpdateTexImageFailed) {
        // This can occur if something goes wrong when trying to create the
        // EGLImage for this buffer. If this happens, the buffer has already
        // been released, so we need to clean up the queue and bug out
        // early.
        if (queuedBuffer) {
            Mutex::Autolock lock(mQueueItemLock);
            mQueueItems.clear();
            mQueuedFrames = 0;
            mFlinger->mTimeStats->onDestroy(layerID);
        }

        // Once we have hit this state, the shadow queue may no longer
        // correctly reflect the incoming BufferQueue's contents, so even if
        // updateTexImage starts working, the only safe course of action is
        // to continue to ignore updates.
        mUpdateTexImageFailed = true;

        return BAD_VALUE;
    }

    if (queuedBuffer) {
        // Autolock scope
        auto currentFrameNumber = mConsumer->getFrameNumber();

        Mutex::Autolock lock(mQueueItemLock);

        // Remove any stale buffers that have been dropped during
        // updateTexImage
        while (mQueueItems[0].mFrameNumber != currentFrameNumber) {
            mConsumer->mergeSurfaceDamage(mQueueItems[0].mSurfaceDamage);
            mFlinger->mTimeStats->removeTimeRecord(layerID, mQueueItems[0].mFrameNumber);
            mQueueItems.removeAt(0);
            mQueuedFrames--;
        }

        mFlinger->mTimeStats->setAcquireFence(layerID, currentFrameNumber,
                                              mQueueItems[0].mFenceTime);
        mFlinger->mTimeStats->setLatchTime(layerID, currentFrameNumber, latchTime);

        mQueueItems.removeAt(0);
    }

    // Decrement the queued-frames count.  Signal another event if we
    // have more frames pending.
    if ((queuedBuffer && mQueuedFrames.fetch_sub(1) > 1) || mAutoRefresh) {
        mFlinger->signalLayerUpdate();
    }

    return NO_ERROR;
}

status_t BufferQueueLayer::updateActiveBuffer() {
    // update the active buffer
    mActiveBuffer = mConsumer->getCurrentBuffer(&mActiveBufferSlot, &mActiveBufferFence);
    auto& layerCompositionState = getCompositionLayer()->editState().frontEnd;
    layerCompositionState.buffer = mActiveBuffer;
    layerCompositionState.bufferSlot = mActiveBufferSlot;

    if (mActiveBuffer == nullptr) {
        // this can only happen if the very first buffer was rejected.
        return BAD_VALUE;
    }
    return NO_ERROR;
}

status_t BufferQueueLayer::updateFrameNumber(nsecs_t latchTime) {
    mPreviousFrameNumber = mCurrentFrameNumber;
    mCurrentFrameNumber = mConsumer->getFrameNumber();

    {
        Mutex::Autolock lock(mFrameEventHistoryMutex);
        mFrameEventHistory.addLatch(mCurrentFrameNumber, latchTime);
    }
    return NO_ERROR;
}

void BufferQueueLayer::setHwcLayerBuffer(const sp<const DisplayDevice>& display) {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    LOG_FATAL_IF(!outputLayer->getState.hwc);
    auto& hwcLayer = (*outputLayer->getState().hwc).hwcLayer;

    uint32_t hwcSlot = 0;
    sp<GraphicBuffer> hwcBuffer;

    // INVALID_BUFFER_SLOT is used to identify BufferStateLayers.  Default to 0
    // for BufferQueueLayers
    int slot = (mActiveBufferSlot == BufferQueue::INVALID_BUFFER_SLOT) ? 0 : mActiveBufferSlot;
    (*outputLayer->editState().hwc)
            .hwcBufferCache.getHwcBuffer(slot, mActiveBuffer, &hwcSlot, &hwcBuffer);

    auto acquireFence = mConsumer->getCurrentFence();
    auto error = hwcLayer->setBuffer(hwcSlot, hwcBuffer, acquireFence);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set buffer %p: %s (%d)", mName.string(), mActiveBuffer->handle,
              to_string(error).c_str(), static_cast<int32_t>(error));
    }

    auto& layerCompositionState = getCompositionLayer()->editState().frontEnd;
    layerCompositionState.bufferSlot = mActiveBufferSlot;
    layerCompositionState.buffer = mActiveBuffer;
    layerCompositionState.acquireFence = acquireFence;
}

// -----------------------------------------------------------------------
// Interface implementation for BufferLayerConsumer::ContentsChangedListener
// -----------------------------------------------------------------------

void BufferQueueLayer::fakeVsync() {
    mRefreshPending = false;
    bool ignored = false;
    latchBuffer(ignored, systemTime());
    usleep(16000);
    releasePendingBuffer(systemTime());
}

void BufferQueueLayer::onFrameAvailable(const BufferItem& item) {
    ATRACE_CALL();
    // Add this buffer from our internal queue tracker
    { // Autolock scope
        if (mFlinger->mUseSmart90ForVideo) {
            const nsecs_t presentTime = item.mIsAutoTimestamp ? 0 : item.mTimestamp;
            mFlinger->mScheduler->addLayerPresentTimeAndHDR(mSchedulerLayerHandle, presentTime,
                                                            item.mHdrMetadata.validTypes != 0);
        }

        Mutex::Autolock lock(mQueueItemLock);
        // Reset the frame number tracker when we receive the first buffer after
        // a frame number reset
        if (item.mFrameNumber == 1) {
            mLastFrameNumberReceived = 0;
        }

        // Ensure that callbacks are handled in order
        while (item.mFrameNumber != mLastFrameNumberReceived + 1) {
            status_t result = mQueueItemCondition.waitRelative(mQueueItemLock, ms2ns(500));
            if (result != NO_ERROR) {
                ALOGE("[%s] Timed out waiting on callback", mName.string());
                break;
            }
        }

        mQueueItems.push_back(item);
        mQueuedFrames++;

        // Wake up any pending callbacks
        mLastFrameNumberReceived = item.mFrameNumber;
        mQueueItemCondition.broadcast();
    }

    mFlinger->mInterceptor->saveBufferUpdate(this, item.mGraphicBuffer->getWidth(),
                                             item.mGraphicBuffer->getHeight(), item.mFrameNumber);

    // If this layer is orphaned, then we run a fake vsync pulse so that
    // dequeueBuffer doesn't block indefinitely.
    if (isRemovedFromCurrentState()) {
        fakeVsync();
    } else {
        mFlinger->signalLayerUpdate();
    }
    mConsumer->onBufferAvailable(item);
}

void BufferQueueLayer::onFrameReplaced(const BufferItem& item) {
    ATRACE_CALL();
    { // Autolock scope
        Mutex::Autolock lock(mQueueItemLock);

        // Ensure that callbacks are handled in order
        while (item.mFrameNumber != mLastFrameNumberReceived + 1) {
            status_t result = mQueueItemCondition.waitRelative(mQueueItemLock, ms2ns(500));
            if (result != NO_ERROR) {
                ALOGE("[%s] Timed out waiting on callback", mName.string());
                break;
            }
        }

        if (!hasFrameUpdate()) {
            ALOGE("Can't replace a frame on an empty queue");
            return;
        }
        mQueueItems.editItemAt(mQueueItems.size() - 1) = item;

        // Wake up any pending callbacks
        mLastFrameNumberReceived = item.mFrameNumber;
        mQueueItemCondition.broadcast();
    }
    mConsumer->onBufferAvailable(item);
}

void BufferQueueLayer::onSidebandStreamChanged() {
    bool sidebandStreamChanged = false;
    if (mSidebandStreamChanged.compare_exchange_strong(sidebandStreamChanged, true)) {
        // mSidebandStreamChanged was changed to true
        mFlinger->signalLayerUpdate();
    }
}

// -----------------------------------------------------------------------

void BufferQueueLayer::onFirstRef() {
    BufferLayer::onFirstRef();

    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer, true);
    mProducer = new MonitoredProducer(producer, mFlinger, this);
    {
        // Grab the SF state lock during this since it's the only safe way to access RenderEngine
        Mutex::Autolock lock(mFlinger->mStateLock);
        mConsumer =
                new BufferLayerConsumer(consumer, mFlinger->getRenderEngine(), mTextureName, this);
    }
    mConsumer->setConsumerUsageBits(getEffectiveUsage(0));
    mConsumer->setContentsChangedListener(this);
    mConsumer->setName(mName);

    // BufferQueueCore::mMaxDequeuedBufferCount is default to 1
    if (!mFlinger->isLayerTripleBufferingDisabled()) {
        mProducer->setMaxDequeuedBufferCount(2);
    }

    if (const auto display = mFlinger->getDefaultDisplayDevice()) {
        updateTransformHint(display);
    }
}

status_t BufferQueueLayer::setDefaultBufferProperties(uint32_t w, uint32_t h, PixelFormat format) {
    uint32_t const maxSurfaceDims =
          std::min(mFlinger->getMaxTextureSize(), mFlinger->getMaxViewportDims());

    // never allow a surface larger than what our underlying GL implementation
    // can handle.
    if ((uint32_t(w) > maxSurfaceDims) || (uint32_t(h) > maxSurfaceDims)) {
        ALOGE("dimensions too large %u x %u", uint32_t(w), uint32_t(h));
        return BAD_VALUE;
    }

    mFormat = format;

    setDefaultBufferSize(w, h);
    mConsumer->setDefaultBufferFormat(format);
    mConsumer->setConsumerUsageBits(getEffectiveUsage(0));

    return NO_ERROR;
}

sp<IGraphicBufferProducer> BufferQueueLayer::getProducer() const {
    return mProducer;
}

uint32_t BufferQueueLayer::getProducerStickyTransform() const {
    int producerStickyTransform = 0;
    int ret = mProducer->query(NATIVE_WINDOW_STICKY_TRANSFORM, &producerStickyTransform);
    if (ret != OK) {
        ALOGW("%s: Error %s (%d) while querying window sticky transform.", __FUNCTION__,
              strerror(-ret), ret);
        return 0;
    }
    return static_cast<uint32_t>(producerStickyTransform);
}

} // namespace android
