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

#define LOG_TAG "Surface"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include <gui/Surface.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include <inttypes.h>

#include <android/native_window.h>

#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/NativeHandle.h>

#include <ui/DisplayStatInfo.h>
#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>
#include <ui/HdrCapabilities.h>
#include <ui/Region.h>

#include <gui/BufferItem.h>
#include <gui/IProducerListener.h>

#include <gui/ISurfaceComposer.h>
#include <private/gui/ComposerService.h>

namespace android {

using ui::ColorMode;
using ui::Dataspace;

Surface::Surface(const sp<IGraphicBufferProducer>& bufferProducer, bool controlledByApp)
      : mGraphicBufferProducer(bufferProducer),
        mCrop(Rect::EMPTY_RECT),
        mBufferAge(0),
        mGenerationNumber(0),
        mSharedBufferMode(false),
        mAutoRefresh(false),
        mSharedBufferSlot(BufferItem::INVALID_BUFFER_SLOT),
        mSharedBufferHasBeenQueued(false),
        mQueriedSupportedTimestamps(false),
        mFrameTimestampsSupportsPresent(false),
        mEnableFrameTimestamps(false),
        mFrameEventHistory(std::make_unique<ProducerFrameEventHistory>()) {
    // Initialize the ANativeWindow function pointers.
    ANativeWindow::setSwapInterval  = hook_setSwapInterval;
    ANativeWindow::dequeueBuffer    = hook_dequeueBuffer;
    ANativeWindow::cancelBuffer     = hook_cancelBuffer;
    ANativeWindow::queueBuffer      = hook_queueBuffer;
    ANativeWindow::query            = hook_query;
    ANativeWindow::perform          = hook_perform;

    ANativeWindow::dequeueBuffer_DEPRECATED = hook_dequeueBuffer_DEPRECATED;
    ANativeWindow::cancelBuffer_DEPRECATED  = hook_cancelBuffer_DEPRECATED;
    ANativeWindow::lockBuffer_DEPRECATED    = hook_lockBuffer_DEPRECATED;
    ANativeWindow::queueBuffer_DEPRECATED   = hook_queueBuffer_DEPRECATED;

    const_cast<int&>(ANativeWindow::minSwapInterval) = 0;
    const_cast<int&>(ANativeWindow::maxSwapInterval) = 1;

    mReqWidth = 0;
    mReqHeight = 0;
    mReqFormat = 0;
    mReqUsage = 0;
    mTimestamp = NATIVE_WINDOW_TIMESTAMP_AUTO;
    mDataSpace = Dataspace::UNKNOWN;
    mScalingMode = NATIVE_WINDOW_SCALING_MODE_FREEZE;
    mTransform = 0;
    mStickyTransform = 0;
    mDefaultWidth = 0;
    mDefaultHeight = 0;
    mUserWidth = 0;
    mUserHeight = 0;
    mTransformHint = 0;
    mConsumerRunningBehind = false;
    mConnectedToCpu = false;
    mProducerControlledByApp = controlledByApp;
    mSwapIntervalZero = false;
}

Surface::~Surface() {
    if (mConnectedToCpu) {
        Surface::disconnect(NATIVE_WINDOW_API_CPU);
    }
}

sp<ISurfaceComposer> Surface::composerService() const {
    return ComposerService::getComposerService();
}

nsecs_t Surface::now() const {
    return systemTime();
}

sp<IGraphicBufferProducer> Surface::getIGraphicBufferProducer() const {
    return mGraphicBufferProducer;
}

void Surface::setSidebandStream(const sp<NativeHandle>& stream) {
    mGraphicBufferProducer->setSidebandStream(stream);
}

void Surface::allocateBuffers() {
    uint32_t reqWidth = mReqWidth ? mReqWidth : mUserWidth;
    uint32_t reqHeight = mReqHeight ? mReqHeight : mUserHeight;
    mGraphicBufferProducer->allocateBuffers(reqWidth, reqHeight,
            mReqFormat, mReqUsage);
}

status_t Surface::setGenerationNumber(uint32_t generation) {
    status_t result = mGraphicBufferProducer->setGenerationNumber(generation);
    if (result == NO_ERROR) {
        mGenerationNumber = generation;
    }
    return result;
}

uint64_t Surface::getNextFrameNumber() const {
    Mutex::Autolock lock(mMutex);
    return mNextFrameNumber;
}

String8 Surface::getConsumerName() const {
    return mGraphicBufferProducer->getConsumerName();
}

status_t Surface::setDequeueTimeout(nsecs_t timeout) {
    return mGraphicBufferProducer->setDequeueTimeout(timeout);
}

status_t Surface::getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer,
        sp<Fence>* outFence, float outTransformMatrix[16]) {
    return mGraphicBufferProducer->getLastQueuedBuffer(outBuffer, outFence,
            outTransformMatrix);
}

status_t Surface::getDisplayRefreshCycleDuration(nsecs_t* outRefreshDuration) {
    ATRACE_CALL();

    DisplayStatInfo stats;
    status_t result = composerService()->getDisplayStats(nullptr, &stats);
    if (result != NO_ERROR) {
        return result;
    }

    *outRefreshDuration = stats.vsyncPeriod;

    return NO_ERROR;
}

void Surface::enableFrameTimestamps(bool enable) {
    Mutex::Autolock lock(mMutex);
    // If going from disabled to enabled, get the initial values for
    // compositor and display timing.
    if (!mEnableFrameTimestamps && enable) {
        FrameEventHistoryDelta delta;
        mGraphicBufferProducer->getFrameTimestamps(&delta);
        mFrameEventHistory->applyDelta(delta);
    }
    mEnableFrameTimestamps = enable;
}

status_t Surface::getCompositorTiming(
        nsecs_t* compositeDeadline, nsecs_t* compositeInterval,
        nsecs_t* compositeToPresentLatency) {
    Mutex::Autolock lock(mMutex);
    if (!mEnableFrameTimestamps) {
        return INVALID_OPERATION;
    }

    if (compositeDeadline != nullptr) {
        *compositeDeadline =
                mFrameEventHistory->getNextCompositeDeadline(now());
    }
    if (compositeInterval != nullptr) {
        *compositeInterval = mFrameEventHistory->getCompositeInterval();
    }
    if (compositeToPresentLatency != nullptr) {
        *compositeToPresentLatency =
                mFrameEventHistory->getCompositeToPresentLatency();
    }
    return NO_ERROR;
}

static bool checkConsumerForUpdates(
        const FrameEvents* e, const uint64_t lastFrameNumber,
        const nsecs_t* outLatchTime,
        const nsecs_t* outFirstRefreshStartTime,
        const nsecs_t* outLastRefreshStartTime,
        const nsecs_t* outGpuCompositionDoneTime,
        const nsecs_t* outDisplayPresentTime,
        const nsecs_t* outDequeueReadyTime,
        const nsecs_t* outReleaseTime) {
    bool checkForLatch = (outLatchTime != nullptr) && !e->hasLatchInfo();
    bool checkForFirstRefreshStart = (outFirstRefreshStartTime != nullptr) &&
            !e->hasFirstRefreshStartInfo();
    bool checkForGpuCompositionDone = (outGpuCompositionDoneTime != nullptr) &&
            !e->hasGpuCompositionDoneInfo();
    bool checkForDisplayPresent = (outDisplayPresentTime != nullptr) &&
            !e->hasDisplayPresentInfo();

    // LastRefreshStart, DequeueReady, and Release are never available for the
    // last frame.
    bool checkForLastRefreshStart = (outLastRefreshStartTime != nullptr) &&
            !e->hasLastRefreshStartInfo() &&
            (e->frameNumber != lastFrameNumber);
    bool checkForDequeueReady = (outDequeueReadyTime != nullptr) &&
            !e->hasDequeueReadyInfo() && (e->frameNumber != lastFrameNumber);
    bool checkForRelease = (outReleaseTime != nullptr) &&
            !e->hasReleaseInfo() && (e->frameNumber != lastFrameNumber);

    // RequestedPresent and Acquire info are always available producer-side.
    return checkForLatch || checkForFirstRefreshStart ||
            checkForLastRefreshStart || checkForGpuCompositionDone ||
            checkForDisplayPresent || checkForDequeueReady || checkForRelease;
}

static void getFrameTimestamp(nsecs_t *dst, const nsecs_t& src) {
    if (dst != nullptr) {
        // We always get valid timestamps for these eventually.
        *dst = (src == FrameEvents::TIMESTAMP_PENDING) ?
                NATIVE_WINDOW_TIMESTAMP_PENDING : src;
    }
}

static void getFrameTimestampFence(nsecs_t *dst,
        const std::shared_ptr<FenceTime>& src, bool fenceShouldBeKnown) {
    if (dst != nullptr) {
        if (!fenceShouldBeKnown) {
            *dst = NATIVE_WINDOW_TIMESTAMP_PENDING;
            return;
        }

        nsecs_t signalTime = src->getSignalTime();
        *dst = (signalTime == Fence::SIGNAL_TIME_PENDING) ?
                    NATIVE_WINDOW_TIMESTAMP_PENDING :
                (signalTime == Fence::SIGNAL_TIME_INVALID) ?
                    NATIVE_WINDOW_TIMESTAMP_INVALID :
                signalTime;
    }
}

status_t Surface::getFrameTimestamps(uint64_t frameNumber,
        nsecs_t* outRequestedPresentTime, nsecs_t* outAcquireTime,
        nsecs_t* outLatchTime, nsecs_t* outFirstRefreshStartTime,
        nsecs_t* outLastRefreshStartTime, nsecs_t* outGpuCompositionDoneTime,
        nsecs_t* outDisplayPresentTime, nsecs_t* outDequeueReadyTime,
        nsecs_t* outReleaseTime) {
    ATRACE_CALL();

    Mutex::Autolock lock(mMutex);

    if (!mEnableFrameTimestamps) {
        return INVALID_OPERATION;
    }

    // Verify the requested timestamps are supported.
    querySupportedTimestampsLocked();
    if (outDisplayPresentTime != nullptr && !mFrameTimestampsSupportsPresent) {
        return BAD_VALUE;
    }

    FrameEvents* events = mFrameEventHistory->getFrame(frameNumber);
    if (events == nullptr) {
        // If the entry isn't available in the producer, it's definitely not
        // available in the consumer.
        return NAME_NOT_FOUND;
    }

    // Update our cache of events if the requested events are not available.
    if (checkConsumerForUpdates(events, mLastFrameNumber,
            outLatchTime, outFirstRefreshStartTime, outLastRefreshStartTime,
            outGpuCompositionDoneTime, outDisplayPresentTime,
            outDequeueReadyTime, outReleaseTime)) {
        FrameEventHistoryDelta delta;
        mGraphicBufferProducer->getFrameTimestamps(&delta);
        mFrameEventHistory->applyDelta(delta);
        events = mFrameEventHistory->getFrame(frameNumber);
    }

    if (events == nullptr) {
        // The entry was available before the update, but was overwritten
        // after the update. Make sure not to send the wrong frame's data.
        return NAME_NOT_FOUND;
    }

    getFrameTimestamp(outRequestedPresentTime, events->requestedPresentTime);
    getFrameTimestamp(outLatchTime, events->latchTime);
    getFrameTimestamp(outFirstRefreshStartTime, events->firstRefreshStartTime);
    getFrameTimestamp(outLastRefreshStartTime, events->lastRefreshStartTime);
    getFrameTimestamp(outDequeueReadyTime, events->dequeueReadyTime);

    getFrameTimestampFence(outAcquireTime, events->acquireFence,
            events->hasAcquireInfo());
    getFrameTimestampFence(outGpuCompositionDoneTime,
            events->gpuCompositionDoneFence,
            events->hasGpuCompositionDoneInfo());
    getFrameTimestampFence(outDisplayPresentTime, events->displayPresentFence,
            events->hasDisplayPresentInfo());
    getFrameTimestampFence(outReleaseTime, events->releaseFence,
            events->hasReleaseInfo());

    return NO_ERROR;
}

status_t Surface::getWideColorSupport(bool* supported) {
    ATRACE_CALL();

    const sp<IBinder> display = composerService()->getInternalDisplayToken();
    if (display == nullptr) {
        return NAME_NOT_FOUND;
    }

    *supported = false;
    status_t error = composerService()->isWideColorDisplay(display, supported);
    return error;
}

status_t Surface::getHdrSupport(bool* supported) {
    ATRACE_CALL();

    const sp<IBinder> display = composerService()->getInternalDisplayToken();
    if (display == nullptr) {
        return NAME_NOT_FOUND;
    }

    HdrCapabilities hdrCapabilities;
    status_t err =
        composerService()->getHdrCapabilities(display, &hdrCapabilities);

    if (err)
        return err;

    *supported = !hdrCapabilities.getSupportedHdrTypes().empty();

    return NO_ERROR;
}

int Surface::hook_setSwapInterval(ANativeWindow* window, int interval) {
    Surface* c = getSelf(window);
    return c->setSwapInterval(interval);
}

int Surface::hook_dequeueBuffer(ANativeWindow* window,
        ANativeWindowBuffer** buffer, int* fenceFd) {
    Surface* c = getSelf(window);
    return c->dequeueBuffer(buffer, fenceFd);
}

int Surface::hook_cancelBuffer(ANativeWindow* window,
        ANativeWindowBuffer* buffer, int fenceFd) {
    Surface* c = getSelf(window);
    return c->cancelBuffer(buffer, fenceFd);
}

int Surface::hook_queueBuffer(ANativeWindow* window,
        ANativeWindowBuffer* buffer, int fenceFd) {
    Surface* c = getSelf(window);
    return c->queueBuffer(buffer, fenceFd);
}

int Surface::hook_dequeueBuffer_DEPRECATED(ANativeWindow* window,
        ANativeWindowBuffer** buffer) {
    Surface* c = getSelf(window);
    ANativeWindowBuffer* buf;
    int fenceFd = -1;
    int result = c->dequeueBuffer(&buf, &fenceFd);
    if (result != OK) {
        return result;
    }
    sp<Fence> fence(new Fence(fenceFd));
    int waitResult = fence->waitForever("dequeueBuffer_DEPRECATED");
    if (waitResult != OK) {
        ALOGE("dequeueBuffer_DEPRECATED: Fence::wait returned an error: %d",
                waitResult);
        c->cancelBuffer(buf, -1);
        return waitResult;
    }
    *buffer = buf;
    return result;
}

int Surface::hook_cancelBuffer_DEPRECATED(ANativeWindow* window,
        ANativeWindowBuffer* buffer) {
    Surface* c = getSelf(window);
    return c->cancelBuffer(buffer, -1);
}

int Surface::hook_lockBuffer_DEPRECATED(ANativeWindow* window,
        ANativeWindowBuffer* buffer) {
    Surface* c = getSelf(window);
    return c->lockBuffer_DEPRECATED(buffer);
}

int Surface::hook_queueBuffer_DEPRECATED(ANativeWindow* window,
        ANativeWindowBuffer* buffer) {
    Surface* c = getSelf(window);
    return c->queueBuffer(buffer, -1);
}

int Surface::hook_query(const ANativeWindow* window,
                                int what, int* value) {
    const Surface* c = getSelf(window);
    return c->query(what, value);
}

int Surface::hook_perform(ANativeWindow* window, int operation, ...) {
    va_list args;
    va_start(args, operation);
    Surface* c = getSelf(window);
    int result = c->perform(operation, args);
    va_end(args);
    return result;
}

int Surface::setSwapInterval(int interval) {
    ATRACE_CALL();
    // EGL specification states:
    //  interval is silently clamped to minimum and maximum implementation
    //  dependent values before being stored.

    if (interval < minSwapInterval)
        interval = minSwapInterval;

    if (interval > maxSwapInterval)
        interval = maxSwapInterval;

    const bool wasSwapIntervalZero = mSwapIntervalZero;
    mSwapIntervalZero = (interval == 0);

    if (mSwapIntervalZero != wasSwapIntervalZero) {
        mGraphicBufferProducer->setAsyncMode(mSwapIntervalZero);
    }

    return NO_ERROR;
}

class FenceMonitor {
public:
    explicit FenceMonitor(const char* name) : mName(name), mFencesQueued(0), mFencesSignaled(0) {
        std::thread thread(&FenceMonitor::loop, this);
        pthread_setname_np(thread.native_handle(), mName);
        thread.detach();
    }

    void queueFence(const sp<Fence>& fence) {
        char message[64];

        std::lock_guard<std::mutex> lock(mMutex);
        if (fence->getSignalTime() != Fence::SIGNAL_TIME_PENDING) {
            snprintf(message, sizeof(message), "%s fence %u has signaled", mName, mFencesQueued);
            ATRACE_NAME(message);
            // Need an increment on both to make the trace number correct.
            mFencesQueued++;
            mFencesSignaled++;
            return;
        }
        snprintf(message, sizeof(message), "Trace %s fence %u", mName, mFencesQueued);
        ATRACE_NAME(message);

        mQueue.push_back(fence);
        mCondition.notify_one();
        mFencesQueued++;
        ATRACE_INT(mName, int32_t(mQueue.size()));
    }

private:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    void loop() {
        while (true) {
            threadLoop();
        }
    }
#pragma clang diagnostic pop

    void threadLoop() {
        sp<Fence> fence;
        uint32_t fenceNum;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            while (mQueue.empty()) {
                mCondition.wait(lock);
            }
            fence = mQueue[0];
            fenceNum = mFencesSignaled;
        }
        {
            char message[64];
            snprintf(message, sizeof(message), "waiting for %s %u", mName, fenceNum);
            ATRACE_NAME(message);

            status_t result = fence->waitForever(message);
            if (result != OK) {
                ALOGE("Error waiting for fence: %d", result);
            }
        }
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.pop_front();
            mFencesSignaled++;
            ATRACE_INT(mName, int32_t(mQueue.size()));
        }
    }

    const char* mName;
    uint32_t mFencesQueued;
    uint32_t mFencesSignaled;
    std::deque<sp<Fence>> mQueue;
    std::condition_variable mCondition;
    std::mutex mMutex;
};

int Surface::dequeueBuffer(android_native_buffer_t** buffer, int* fenceFd) {
    ATRACE_CALL();
    ALOGV("Surface::dequeueBuffer");

    uint32_t reqWidth;
    uint32_t reqHeight;
    PixelFormat reqFormat;
    uint64_t reqUsage;
    bool enableFrameTimestamps;

    {
        Mutex::Autolock lock(mMutex);
        if (mReportRemovedBuffers) {
            mRemovedBuffers.clear();
        }

        reqWidth = mReqWidth ? mReqWidth : mUserWidth;
        reqHeight = mReqHeight ? mReqHeight : mUserHeight;

        reqFormat = mReqFormat;
        reqUsage = mReqUsage;

        enableFrameTimestamps = mEnableFrameTimestamps;

        if (mSharedBufferMode && mAutoRefresh && mSharedBufferSlot !=
                BufferItem::INVALID_BUFFER_SLOT) {
            sp<GraphicBuffer>& gbuf(mSlots[mSharedBufferSlot].buffer);
            if (gbuf != nullptr) {
                *buffer = gbuf.get();
                *fenceFd = -1;
                return OK;
            }
        }
    } // Drop the lock so that we can still touch the Surface while blocking in IGBP::dequeueBuffer

    int buf = -1;
    sp<Fence> fence;
    nsecs_t startTime = systemTime();

    FrameEventHistoryDelta frameTimestamps;
    status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, reqWidth, reqHeight,
                                                            reqFormat, reqUsage, &mBufferAge,
                                                            enableFrameTimestamps ? &frameTimestamps
                                                                                  : nullptr);
    mLastDequeueDuration = systemTime() - startTime;

    if (result < 0) {
        ALOGV("dequeueBuffer: IGraphicBufferProducer::dequeueBuffer"
                "(%d, %d, %d, %#" PRIx64 ") failed: %d",
                reqWidth, reqHeight, reqFormat, reqUsage, result);
        return result;
    }

    if (buf < 0 || buf >= NUM_BUFFER_SLOTS) {
        ALOGE("dequeueBuffer: IGraphicBufferProducer returned invalid slot number %d", buf);
        android_errorWriteLog(0x534e4554, "36991414"); // SafetyNet logging
        return FAILED_TRANSACTION;
    }

    Mutex::Autolock lock(mMutex);

    // Write this while holding the mutex
    mLastDequeueStartTime = startTime;

    sp<GraphicBuffer>& gbuf(mSlots[buf].buffer);

    // this should never happen
    ALOGE_IF(fence == nullptr, "Surface::dequeueBuffer: received null Fence! buf=%d", buf);

    if (CC_UNLIKELY(atrace_is_tag_enabled(ATRACE_TAG_GRAPHICS))) {
        static FenceMonitor hwcReleaseThread("HWC release");
        hwcReleaseThread.queueFence(fence);
    }

    if (result & IGraphicBufferProducer::RELEASE_ALL_BUFFERS) {
        freeAllBuffers();
    }

    if (enableFrameTimestamps) {
         mFrameEventHistory->applyDelta(frameTimestamps);
    }

    if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == nullptr) {
        if (mReportRemovedBuffers && (gbuf != nullptr)) {
            mRemovedBuffers.push_back(gbuf);
        }
        result = mGraphicBufferProducer->requestBuffer(buf, &gbuf);
        if (result != NO_ERROR) {
            ALOGE("dequeueBuffer: IGraphicBufferProducer::requestBuffer failed: %d", result);
            mGraphicBufferProducer->cancelBuffer(buf, fence);
            return result;
        }
    }

    if (fence->isValid()) {
        *fenceFd = fence->dup();
        if (*fenceFd == -1) {
            ALOGE("dequeueBuffer: error duping fence: %d", errno);
            // dup() should never fail; something is badly wrong. Soldier on
            // and hope for the best; the worst that should happen is some
            // visible corruption that lasts until the next frame.
        }
    } else {
        *fenceFd = -1;
    }

    *buffer = gbuf.get();

    if (mSharedBufferMode && mAutoRefresh) {
        mSharedBufferSlot = buf;
        mSharedBufferHasBeenQueued = false;
    } else if (mSharedBufferSlot == buf) {
        mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
        mSharedBufferHasBeenQueued = false;
    }

    return OK;
}

int Surface::cancelBuffer(android_native_buffer_t* buffer,
        int fenceFd) {
    ATRACE_CALL();
    ALOGV("Surface::cancelBuffer");
    Mutex::Autolock lock(mMutex);
    int i = getSlotFromBufferLocked(buffer);
    if (i < 0) {
        if (fenceFd >= 0) {
            close(fenceFd);
        }
        return i;
    }
    if (mSharedBufferSlot == i && mSharedBufferHasBeenQueued) {
        if (fenceFd >= 0) {
            close(fenceFd);
        }
        return OK;
    }
    sp<Fence> fence(fenceFd >= 0 ? new Fence(fenceFd) : Fence::NO_FENCE);
    mGraphicBufferProducer->cancelBuffer(i, fence);

    if (mSharedBufferMode && mAutoRefresh && mSharedBufferSlot == i) {
        mSharedBufferHasBeenQueued = true;
    }

    return OK;
}

int Surface::getSlotFromBufferLocked(
        android_native_buffer_t* buffer) const {
    for (int i = 0; i < NUM_BUFFER_SLOTS; i++) {
        if (mSlots[i].buffer != nullptr &&
                mSlots[i].buffer->handle == buffer->handle) {
            return i;
        }
    }
    ALOGE("getSlotFromBufferLocked: unknown buffer: %p", buffer->handle);
    return BAD_VALUE;
}

int Surface::lockBuffer_DEPRECATED(android_native_buffer_t* buffer __attribute__((unused))) {
    ALOGV("Surface::lockBuffer");
    Mutex::Autolock lock(mMutex);
    return OK;
}

int Surface::queueBuffer(android_native_buffer_t* buffer, int fenceFd) {
    ATRACE_CALL();
    ALOGV("Surface::queueBuffer");
    Mutex::Autolock lock(mMutex);
    int64_t timestamp;
    bool isAutoTimestamp = false;

    if (mTimestamp == NATIVE_WINDOW_TIMESTAMP_AUTO) {
        timestamp = systemTime(SYSTEM_TIME_MONOTONIC);
        isAutoTimestamp = true;
        ALOGV("Surface::queueBuffer making up timestamp: %.2f ms",
            timestamp / 1000000.0);
    } else {
        timestamp = mTimestamp;
    }
    int i = getSlotFromBufferLocked(buffer);
    if (i < 0) {
        if (fenceFd >= 0) {
            close(fenceFd);
        }
        return i;
    }
    if (mSharedBufferSlot == i && mSharedBufferHasBeenQueued) {
        if (fenceFd >= 0) {
            close(fenceFd);
        }
        return OK;
    }


    // Make sure the crop rectangle is entirely inside the buffer.
    Rect crop(Rect::EMPTY_RECT);
    mCrop.intersect(Rect(buffer->width, buffer->height), &crop);

    sp<Fence> fence(fenceFd >= 0 ? new Fence(fenceFd) : Fence::NO_FENCE);
    IGraphicBufferProducer::QueueBufferOutput output;
    IGraphicBufferProducer::QueueBufferInput input(timestamp, isAutoTimestamp,
            static_cast<android_dataspace>(mDataSpace), crop, mScalingMode,
            mTransform ^ mStickyTransform, fence, mStickyTransform,
            mEnableFrameTimestamps);

    // we should send HDR metadata as needed if this becomes a bottleneck
    input.setHdrMetadata(mHdrMetadata);

    if (mConnectedToCpu || mDirtyRegion.bounds() == Rect::INVALID_RECT) {
        input.setSurfaceDamage(Region::INVALID_REGION);
    } else {
        // Here we do two things:
        // 1) The surface damage was specified using the OpenGL ES convention of
        //    the origin being in the bottom-left corner. Here we flip to the
        //    convention that the rest of the system uses (top-left corner) by
        //    subtracting all top/bottom coordinates from the buffer height.
        // 2) If the buffer is coming in rotated (for example, because the EGL
        //    implementation is reacting to the transform hint coming back from
        //    SurfaceFlinger), the surface damage needs to be rotated the
        //    opposite direction, since it was generated assuming an unrotated
        //    buffer (the app doesn't know that the EGL implementation is
        //    reacting to the transform hint behind its back). The
        //    transformations in the switch statement below apply those
        //    complementary rotations (e.g., if 90 degrees, rotate 270 degrees).

        int width = buffer->width;
        int height = buffer->height;
        bool rotated90 = (mTransform ^ mStickyTransform) &
                NATIVE_WINDOW_TRANSFORM_ROT_90;
        if (rotated90) {
            std::swap(width, height);
        }

        Region flippedRegion;
        for (auto rect : mDirtyRegion) {
            int left = rect.left;
            int right = rect.right;
            int top = height - rect.bottom; // Flip from OpenGL convention
            int bottom = height - rect.top; // Flip from OpenGL convention
            switch (mTransform ^ mStickyTransform) {
                case NATIVE_WINDOW_TRANSFORM_ROT_90: {
                    // Rotate 270 degrees
                    Rect flippedRect{top, width - right, bottom, width - left};
                    flippedRegion.orSelf(flippedRect);
                    break;
                }
                case NATIVE_WINDOW_TRANSFORM_ROT_180: {
                    // Rotate 180 degrees
                    Rect flippedRect{width - right, height - bottom,
                            width - left, height - top};
                    flippedRegion.orSelf(flippedRect);
                    break;
                }
                case NATIVE_WINDOW_TRANSFORM_ROT_270: {
                    // Rotate 90 degrees
                    Rect flippedRect{height - bottom, left,
                            height - top, right};
                    flippedRegion.orSelf(flippedRect);
                    break;
                }
                default: {
                    Rect flippedRect{left, top, right, bottom};
                    flippedRegion.orSelf(flippedRect);
                    break;
                }
            }
        }

        input.setSurfaceDamage(flippedRegion);
    }

    nsecs_t now = systemTime();
    status_t err = mGraphicBufferProducer->queueBuffer(i, input, &output);
    mLastQueueDuration = systemTime() - now;
    if (err != OK)  {
        ALOGE("queueBuffer: error queuing buffer to SurfaceTexture, %d", err);
    }

    if (mEnableFrameTimestamps) {
        mFrameEventHistory->applyDelta(output.frameTimestamps);
        // Update timestamps with the local acquire fence.
        // The consumer doesn't send it back to prevent us from having two
        // file descriptors of the same fence.
        mFrameEventHistory->updateAcquireFence(mNextFrameNumber,
                std::make_shared<FenceTime>(fence));

        // Cache timestamps of signaled fences so we can close their file
        // descriptors.
        mFrameEventHistory->updateSignalTimes();
    }

    mLastFrameNumber = mNextFrameNumber;

    mDefaultWidth = output.width;
    mDefaultHeight = output.height;
    mNextFrameNumber = output.nextFrameNumber;

    // Ignore transform hint if sticky transform is set or transform to display inverse flag is
    // set.
    if (mStickyTransform == 0 && !transformToDisplayInverse()) {
        mTransformHint = output.transformHint;
    }

    mConsumerRunningBehind = (output.numPendingBuffers >= 2);

    if (!mConnectedToCpu) {
        // Clear surface damage back to full-buffer
        mDirtyRegion = Region::INVALID_REGION;
    }

    if (mSharedBufferMode && mAutoRefresh && mSharedBufferSlot == i) {
        mSharedBufferHasBeenQueued = true;
    }

    mQueueBufferCondition.broadcast();

    if (CC_UNLIKELY(atrace_is_tag_enabled(ATRACE_TAG_GRAPHICS))) {
        static FenceMonitor gpuCompletionThread("GPU completion");
        gpuCompletionThread.queueFence(fence);
    }

    return err;
}

void Surface::querySupportedTimestampsLocked() const {
    // mMutex must be locked when calling this method.

    if (mQueriedSupportedTimestamps) {
        return;
    }
    mQueriedSupportedTimestamps = true;

    std::vector<FrameEvent> supportedFrameTimestamps;
    status_t err = composerService()->getSupportedFrameTimestamps(
            &supportedFrameTimestamps);

    if (err != NO_ERROR) {
        return;
    }

    for (auto sft : supportedFrameTimestamps) {
        if (sft == FrameEvent::DISPLAY_PRESENT) {
            mFrameTimestampsSupportsPresent = true;
        }
    }
}

int Surface::query(int what, int* value) const {
    ATRACE_CALL();
    ALOGV("Surface::query");
    { // scope for the lock
        Mutex::Autolock lock(mMutex);
        switch (what) {
            case NATIVE_WINDOW_FORMAT:
                if (mReqFormat) {
                    *value = static_cast<int>(mReqFormat);
                    return NO_ERROR;
                }
                break;
            case NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER: {
                if (composerService()->authenticateSurfaceTexture(
                        mGraphicBufferProducer)) {
                    *value = 1;
                } else {
                    *value = 0;
                }
                return NO_ERROR;
            }
            case NATIVE_WINDOW_CONCRETE_TYPE:
                *value = NATIVE_WINDOW_SURFACE;
                return NO_ERROR;
            case NATIVE_WINDOW_DEFAULT_WIDTH:
                *value = static_cast<int>(
                        mUserWidth ? mUserWidth : mDefaultWidth);
                return NO_ERROR;
            case NATIVE_WINDOW_DEFAULT_HEIGHT:
                *value = static_cast<int>(
                        mUserHeight ? mUserHeight : mDefaultHeight);
                return NO_ERROR;
            case NATIVE_WINDOW_TRANSFORM_HINT:
                *value = static_cast<int>(mTransformHint);
                return NO_ERROR;
            case NATIVE_WINDOW_CONSUMER_RUNNING_BEHIND: {
                status_t err = NO_ERROR;
                if (!mConsumerRunningBehind) {
                    *value = 0;
                } else {
                    err = mGraphicBufferProducer->query(what, value);
                    if (err == NO_ERROR) {
                        mConsumerRunningBehind = *value;
                    }
                }
                return err;
            }
            case NATIVE_WINDOW_BUFFER_AGE: {
                if (mBufferAge > INT32_MAX) {
                    *value = 0;
                } else {
                    *value = static_cast<int32_t>(mBufferAge);
                }
                return NO_ERROR;
            }
            case NATIVE_WINDOW_LAST_DEQUEUE_DURATION: {
                int64_t durationUs = mLastDequeueDuration / 1000;
                *value = durationUs > std::numeric_limits<int>::max() ?
                        std::numeric_limits<int>::max() :
                        static_cast<int>(durationUs);
                return NO_ERROR;
            }
            case NATIVE_WINDOW_LAST_QUEUE_DURATION: {
                int64_t durationUs = mLastQueueDuration / 1000;
                *value = durationUs > std::numeric_limits<int>::max() ?
                        std::numeric_limits<int>::max() :
                        static_cast<int>(durationUs);
                return NO_ERROR;
            }
            case NATIVE_WINDOW_FRAME_TIMESTAMPS_SUPPORTS_PRESENT: {
                querySupportedTimestampsLocked();
                *value = mFrameTimestampsSupportsPresent ? 1 : 0;
                return NO_ERROR;
            }
            case NATIVE_WINDOW_IS_VALID: {
                *value = mGraphicBufferProducer != nullptr ? 1 : 0;
                return NO_ERROR;
            }
            case NATIVE_WINDOW_DATASPACE: {
                *value = static_cast<int>(mDataSpace);
                return NO_ERROR;
            }
        }
    }
    return mGraphicBufferProducer->query(what, value);
}

int Surface::perform(int operation, va_list args)
{
    int res = NO_ERROR;
    switch (operation) {
    case NATIVE_WINDOW_CONNECT:
        // deprecated. must return NO_ERROR.
        break;
    case NATIVE_WINDOW_DISCONNECT:
        // deprecated. must return NO_ERROR.
        break;
    case NATIVE_WINDOW_SET_USAGE:
        res = dispatchSetUsage(args);
        break;
    case NATIVE_WINDOW_SET_CROP:
        res = dispatchSetCrop(args);
        break;
    case NATIVE_WINDOW_SET_BUFFER_COUNT:
        res = dispatchSetBufferCount(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_GEOMETRY:
        res = dispatchSetBuffersGeometry(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_TRANSFORM:
        res = dispatchSetBuffersTransform(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_STICKY_TRANSFORM:
        res = dispatchSetBuffersStickyTransform(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP:
        res = dispatchSetBuffersTimestamp(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_DIMENSIONS:
        res = dispatchSetBuffersDimensions(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_USER_DIMENSIONS:
        res = dispatchSetBuffersUserDimensions(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_FORMAT:
        res = dispatchSetBuffersFormat(args);
        break;
    case NATIVE_WINDOW_LOCK:
        res = dispatchLock(args);
        break;
    case NATIVE_WINDOW_UNLOCK_AND_POST:
        res = dispatchUnlockAndPost(args);
        break;
    case NATIVE_WINDOW_SET_SCALING_MODE:
        res = dispatchSetScalingMode(args);
        break;
    case NATIVE_WINDOW_API_CONNECT:
        res = dispatchConnect(args);
        break;
    case NATIVE_WINDOW_API_DISCONNECT:
        res = dispatchDisconnect(args);
        break;
    case NATIVE_WINDOW_SET_SIDEBAND_STREAM:
        res = dispatchSetSidebandStream(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_DATASPACE:
        res = dispatchSetBuffersDataSpace(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_SMPTE2086_METADATA:
        res = dispatchSetBuffersSmpte2086Metadata(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_CTA861_3_METADATA:
        res = dispatchSetBuffersCta8613Metadata(args);
        break;
    case NATIVE_WINDOW_SET_BUFFERS_HDR10_PLUS_METADATA:
        res = dispatchSetBuffersHdr10PlusMetadata(args);
        break;
    case NATIVE_WINDOW_SET_SURFACE_DAMAGE:
        res = dispatchSetSurfaceDamage(args);
        break;
    case NATIVE_WINDOW_SET_SHARED_BUFFER_MODE:
        res = dispatchSetSharedBufferMode(args);
        break;
    case NATIVE_WINDOW_SET_AUTO_REFRESH:
        res = dispatchSetAutoRefresh(args);
        break;
    case NATIVE_WINDOW_GET_REFRESH_CYCLE_DURATION:
        res = dispatchGetDisplayRefreshCycleDuration(args);
        break;
    case NATIVE_WINDOW_GET_NEXT_FRAME_ID:
        res = dispatchGetNextFrameId(args);
        break;
    case NATIVE_WINDOW_ENABLE_FRAME_TIMESTAMPS:
        res = dispatchEnableFrameTimestamps(args);
        break;
    case NATIVE_WINDOW_GET_COMPOSITOR_TIMING:
        res = dispatchGetCompositorTiming(args);
        break;
    case NATIVE_WINDOW_GET_FRAME_TIMESTAMPS:
        res = dispatchGetFrameTimestamps(args);
        break;
    case NATIVE_WINDOW_GET_WIDE_COLOR_SUPPORT:
        res = dispatchGetWideColorSupport(args);
        break;
    case NATIVE_WINDOW_GET_HDR_SUPPORT:
        res = dispatchGetHdrSupport(args);
        break;
    case NATIVE_WINDOW_SET_USAGE64:
        res = dispatchSetUsage64(args);
        break;
    case NATIVE_WINDOW_GET_CONSUMER_USAGE64:
        res = dispatchGetConsumerUsage64(args);
        break;
    default:
        res = NAME_NOT_FOUND;
        break;
    }
    return res;
}

int Surface::dispatchConnect(va_list args) {
    int api = va_arg(args, int);
    return connect(api);
}

int Surface::dispatchDisconnect(va_list args) {
    int api = va_arg(args, int);
    return disconnect(api);
}

int Surface::dispatchSetUsage(va_list args) {
    uint64_t usage = va_arg(args, uint32_t);
    return setUsage(usage);
}

int Surface::dispatchSetUsage64(va_list args) {
    uint64_t usage = va_arg(args, uint64_t);
    return setUsage(usage);
}

int Surface::dispatchSetCrop(va_list args) {
    android_native_rect_t const* rect = va_arg(args, android_native_rect_t*);
    return setCrop(reinterpret_cast<Rect const*>(rect));
}

int Surface::dispatchSetBufferCount(va_list args) {
    size_t bufferCount = va_arg(args, size_t);
    return setBufferCount(static_cast<int32_t>(bufferCount));
}

int Surface::dispatchSetBuffersGeometry(va_list args) {
    uint32_t width = va_arg(args, uint32_t);
    uint32_t height = va_arg(args, uint32_t);
    PixelFormat format = va_arg(args, PixelFormat);
    int err = setBuffersDimensions(width, height);
    if (err != 0) {
        return err;
    }
    return setBuffersFormat(format);
}

int Surface::dispatchSetBuffersDimensions(va_list args) {
    uint32_t width = va_arg(args, uint32_t);
    uint32_t height = va_arg(args, uint32_t);
    return setBuffersDimensions(width, height);
}

int Surface::dispatchSetBuffersUserDimensions(va_list args) {
    uint32_t width = va_arg(args, uint32_t);
    uint32_t height = va_arg(args, uint32_t);
    return setBuffersUserDimensions(width, height);
}

int Surface::dispatchSetBuffersFormat(va_list args) {
    PixelFormat format = va_arg(args, PixelFormat);
    return setBuffersFormat(format);
}

int Surface::dispatchSetScalingMode(va_list args) {
    int mode = va_arg(args, int);
    return setScalingMode(mode);
}

int Surface::dispatchSetBuffersTransform(va_list args) {
    uint32_t transform = va_arg(args, uint32_t);
    return setBuffersTransform(transform);
}

int Surface::dispatchSetBuffersStickyTransform(va_list args) {
    uint32_t transform = va_arg(args, uint32_t);
    return setBuffersStickyTransform(transform);
}

int Surface::dispatchSetBuffersTimestamp(va_list args) {
    int64_t timestamp = va_arg(args, int64_t);
    return setBuffersTimestamp(timestamp);
}

int Surface::dispatchLock(va_list args) {
    ANativeWindow_Buffer* outBuffer = va_arg(args, ANativeWindow_Buffer*);
    ARect* inOutDirtyBounds = va_arg(args, ARect*);
    return lock(outBuffer, inOutDirtyBounds);
}

int Surface::dispatchUnlockAndPost(va_list args __attribute__((unused))) {
    return unlockAndPost();
}

int Surface::dispatchSetSidebandStream(va_list args) {
    native_handle_t* sH = va_arg(args, native_handle_t*);
    sp<NativeHandle> sidebandHandle = NativeHandle::create(sH, false);
    setSidebandStream(sidebandHandle);
    return OK;
}

int Surface::dispatchSetBuffersDataSpace(va_list args) {
    Dataspace dataspace = static_cast<Dataspace>(va_arg(args, int));
    return setBuffersDataSpace(dataspace);
}

int Surface::dispatchSetBuffersSmpte2086Metadata(va_list args) {
    const android_smpte2086_metadata* metadata =
        va_arg(args, const android_smpte2086_metadata*);
    return setBuffersSmpte2086Metadata(metadata);
}

int Surface::dispatchSetBuffersCta8613Metadata(va_list args) {
    const android_cta861_3_metadata* metadata =
        va_arg(args, const android_cta861_3_metadata*);
    return setBuffersCta8613Metadata(metadata);
}

int Surface::dispatchSetBuffersHdr10PlusMetadata(va_list args) {
    const size_t size = va_arg(args, size_t);
    const uint8_t* metadata = va_arg(args, const uint8_t*);
    return setBuffersHdr10PlusMetadata(size, metadata);
}

int Surface::dispatchSetSurfaceDamage(va_list args) {
    android_native_rect_t* rects = va_arg(args, android_native_rect_t*);
    size_t numRects = va_arg(args, size_t);
    setSurfaceDamage(rects, numRects);
    return NO_ERROR;
}

int Surface::dispatchSetSharedBufferMode(va_list args) {
    bool sharedBufferMode = va_arg(args, int);
    return setSharedBufferMode(sharedBufferMode);
}

int Surface::dispatchSetAutoRefresh(va_list args) {
    bool autoRefresh = va_arg(args, int);
    return setAutoRefresh(autoRefresh);
}

int Surface::dispatchGetDisplayRefreshCycleDuration(va_list args) {
    nsecs_t* outRefreshDuration = va_arg(args, int64_t*);
    return getDisplayRefreshCycleDuration(outRefreshDuration);
}

int Surface::dispatchGetNextFrameId(va_list args) {
    uint64_t* nextFrameId = va_arg(args, uint64_t*);
    *nextFrameId = getNextFrameNumber();
    return NO_ERROR;
}

int Surface::dispatchEnableFrameTimestamps(va_list args) {
    bool enable = va_arg(args, int);
    enableFrameTimestamps(enable);
    return NO_ERROR;
}

int Surface::dispatchGetCompositorTiming(va_list args) {
    nsecs_t* compositeDeadline = va_arg(args, int64_t*);
    nsecs_t* compositeInterval = va_arg(args, int64_t*);
    nsecs_t* compositeToPresentLatency = va_arg(args, int64_t*);
    return getCompositorTiming(compositeDeadline, compositeInterval,
            compositeToPresentLatency);
}

int Surface::dispatchGetFrameTimestamps(va_list args) {
    uint64_t frameId = va_arg(args, uint64_t);
    nsecs_t* outRequestedPresentTime = va_arg(args, int64_t*);
    nsecs_t* outAcquireTime = va_arg(args, int64_t*);
    nsecs_t* outLatchTime = va_arg(args, int64_t*);
    nsecs_t* outFirstRefreshStartTime = va_arg(args, int64_t*);
    nsecs_t* outLastRefreshStartTime = va_arg(args, int64_t*);
    nsecs_t* outGpuCompositionDoneTime = va_arg(args, int64_t*);
    nsecs_t* outDisplayPresentTime = va_arg(args, int64_t*);
    nsecs_t* outDequeueReadyTime = va_arg(args, int64_t*);
    nsecs_t* outReleaseTime = va_arg(args, int64_t*);
    return getFrameTimestamps(frameId,
            outRequestedPresentTime, outAcquireTime, outLatchTime,
            outFirstRefreshStartTime, outLastRefreshStartTime,
            outGpuCompositionDoneTime, outDisplayPresentTime,
            outDequeueReadyTime, outReleaseTime);
}

int Surface::dispatchGetWideColorSupport(va_list args) {
    bool* outSupport = va_arg(args, bool*);
    return getWideColorSupport(outSupport);
}

int Surface::dispatchGetHdrSupport(va_list args) {
    bool* outSupport = va_arg(args, bool*);
    return getHdrSupport(outSupport);
}

int Surface::dispatchGetConsumerUsage64(va_list args) {
    uint64_t* usage = va_arg(args, uint64_t*);
    return getConsumerUsage(usage);
}

bool Surface::transformToDisplayInverse() {
    return (mTransform & NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY) ==
            NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
}

int Surface::connect(int api) {
    static sp<IProducerListener> listener = new DummyProducerListener();
    return connect(api, listener);
}

int Surface::connect(int api, const sp<IProducerListener>& listener) {
    return connect(api, listener, false);
}

int Surface::connect(
        int api, bool reportBufferRemoval, const sp<SurfaceListener>& sListener) {
    if (sListener != nullptr) {
        mListenerProxy = new ProducerListenerProxy(this, sListener);
    }
    return connect(api, mListenerProxy, reportBufferRemoval);
}

int Surface::connect(
        int api, const sp<IProducerListener>& listener, bool reportBufferRemoval) {
    ATRACE_CALL();
    ALOGV("Surface::connect");
    Mutex::Autolock lock(mMutex);
    IGraphicBufferProducer::QueueBufferOutput output;
    mReportRemovedBuffers = reportBufferRemoval;
    int err = mGraphicBufferProducer->connect(listener, api, mProducerControlledByApp, &output);
    if (err == NO_ERROR) {
        mDefaultWidth = output.width;
        mDefaultHeight = output.height;
        mNextFrameNumber = output.nextFrameNumber;

        // Ignore transform hint if sticky transform is set or transform to display inverse flag is
        // set. Transform hint should be ignored if the client is expected to always submit buffers
        // in the same orientation.
        if (mStickyTransform == 0 && !transformToDisplayInverse()) {
            mTransformHint = output.transformHint;
        }

        mConsumerRunningBehind = (output.numPendingBuffers >= 2);
    }
    if (!err && api == NATIVE_WINDOW_API_CPU) {
        mConnectedToCpu = true;
        // Clear the dirty region in case we're switching from a non-CPU API
        mDirtyRegion.clear();
    } else if (!err) {
        // Initialize the dirty region for tracking surface damage
        mDirtyRegion = Region::INVALID_REGION;
    }

    return err;
}


int Surface::disconnect(int api, IGraphicBufferProducer::DisconnectMode mode) {
    ATRACE_CALL();
    ALOGV("Surface::disconnect");
    Mutex::Autolock lock(mMutex);
    mRemovedBuffers.clear();
    mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
    mSharedBufferHasBeenQueued = false;
    freeAllBuffers();
    int err = mGraphicBufferProducer->disconnect(api, mode);
    if (!err) {
        mReqFormat = 0;
        mReqWidth = 0;
        mReqHeight = 0;
        mReqUsage = 0;
        mCrop.clear();
        mScalingMode = NATIVE_WINDOW_SCALING_MODE_FREEZE;
        mTransform = 0;
        mStickyTransform = 0;

        if (api == NATIVE_WINDOW_API_CPU) {
            mConnectedToCpu = false;
        }
    }
    return err;
}

int Surface::detachNextBuffer(sp<GraphicBuffer>* outBuffer,
        sp<Fence>* outFence) {
    ATRACE_CALL();
    ALOGV("Surface::detachNextBuffer");

    if (outBuffer == nullptr || outFence == nullptr) {
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);
    if (mReportRemovedBuffers) {
        mRemovedBuffers.clear();
    }

    sp<GraphicBuffer> buffer(nullptr);
    sp<Fence> fence(nullptr);
    status_t result = mGraphicBufferProducer->detachNextBuffer(
            &buffer, &fence);
    if (result != NO_ERROR) {
        return result;
    }

    *outBuffer = buffer;
    if (fence != nullptr && fence->isValid()) {
        *outFence = fence;
    } else {
        *outFence = Fence::NO_FENCE;
    }

    for (int i = 0; i < NUM_BUFFER_SLOTS; i++) {
        if (mSlots[i].buffer != nullptr &&
                mSlots[i].buffer->getId() == buffer->getId()) {
            if (mReportRemovedBuffers) {
                mRemovedBuffers.push_back(mSlots[i].buffer);
            }
            mSlots[i].buffer = nullptr;
        }
    }

    return NO_ERROR;
}

int Surface::attachBuffer(ANativeWindowBuffer* buffer)
{
    ATRACE_CALL();
    ALOGV("Surface::attachBuffer");

    Mutex::Autolock lock(mMutex);
    if (mReportRemovedBuffers) {
        mRemovedBuffers.clear();
    }

    sp<GraphicBuffer> graphicBuffer(static_cast<GraphicBuffer*>(buffer));
    uint32_t priorGeneration = graphicBuffer->mGenerationNumber;
    graphicBuffer->mGenerationNumber = mGenerationNumber;
    int32_t attachedSlot = -1;
    status_t result = mGraphicBufferProducer->attachBuffer(&attachedSlot, graphicBuffer);
    if (result != NO_ERROR) {
        ALOGE("attachBuffer: IGraphicBufferProducer call failed (%d)", result);
        graphicBuffer->mGenerationNumber = priorGeneration;
        return result;
    }
    if (mReportRemovedBuffers && (mSlots[attachedSlot].buffer != nullptr)) {
        mRemovedBuffers.push_back(mSlots[attachedSlot].buffer);
    }
    mSlots[attachedSlot].buffer = graphicBuffer;

    return NO_ERROR;
}

int Surface::setUsage(uint64_t reqUsage)
{
    ALOGV("Surface::setUsage");
    Mutex::Autolock lock(mMutex);
    if (reqUsage != mReqUsage) {
        mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
    }
    mReqUsage = reqUsage;
    return OK;
}

int Surface::setCrop(Rect const* rect)
{
    ATRACE_CALL();

    Rect realRect(Rect::EMPTY_RECT);
    if (rect == nullptr || rect->isEmpty()) {
        realRect.clear();
    } else {
        realRect = *rect;
    }

    ALOGV("Surface::setCrop rect=[%d %d %d %d]",
            realRect.left, realRect.top, realRect.right, realRect.bottom);

    Mutex::Autolock lock(mMutex);
    mCrop = realRect;
    return NO_ERROR;
}

int Surface::setBufferCount(int bufferCount)
{
    ATRACE_CALL();
    ALOGV("Surface::setBufferCount");
    Mutex::Autolock lock(mMutex);

    status_t err = NO_ERROR;
    if (bufferCount == 0) {
        err = mGraphicBufferProducer->setMaxDequeuedBufferCount(1);
    } else {
        int minUndequeuedBuffers = 0;
        err = mGraphicBufferProducer->query(
                NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBuffers);
        if (err == NO_ERROR) {
            err = mGraphicBufferProducer->setMaxDequeuedBufferCount(
                    bufferCount - minUndequeuedBuffers);
        }
    }

    ALOGE_IF(err, "IGraphicBufferProducer::setBufferCount(%d) returned %s",
             bufferCount, strerror(-err));

    return err;
}

int Surface::setMaxDequeuedBufferCount(int maxDequeuedBuffers) {
    ATRACE_CALL();
    ALOGV("Surface::setMaxDequeuedBufferCount");
    Mutex::Autolock lock(mMutex);

    status_t err = mGraphicBufferProducer->setMaxDequeuedBufferCount(
            maxDequeuedBuffers);
    ALOGE_IF(err, "IGraphicBufferProducer::setMaxDequeuedBufferCount(%d) "
            "returned %s", maxDequeuedBuffers, strerror(-err));

    return err;
}

int Surface::setAsyncMode(bool async) {
    ATRACE_CALL();
    ALOGV("Surface::setAsyncMode");
    Mutex::Autolock lock(mMutex);

    status_t err = mGraphicBufferProducer->setAsyncMode(async);
    ALOGE_IF(err, "IGraphicBufferProducer::setAsyncMode(%d) returned %s",
            async, strerror(-err));

    return err;
}

int Surface::setSharedBufferMode(bool sharedBufferMode) {
    ATRACE_CALL();
    ALOGV("Surface::setSharedBufferMode (%d)", sharedBufferMode);
    Mutex::Autolock lock(mMutex);

    status_t err = mGraphicBufferProducer->setSharedBufferMode(
            sharedBufferMode);
    if (err == NO_ERROR) {
        mSharedBufferMode = sharedBufferMode;
    }
    ALOGE_IF(err, "IGraphicBufferProducer::setSharedBufferMode(%d) returned"
            "%s", sharedBufferMode, strerror(-err));

    return err;
}

int Surface::setAutoRefresh(bool autoRefresh) {
    ATRACE_CALL();
    ALOGV("Surface::setAutoRefresh (%d)", autoRefresh);
    Mutex::Autolock lock(mMutex);

    status_t err = mGraphicBufferProducer->setAutoRefresh(autoRefresh);
    if (err == NO_ERROR) {
        mAutoRefresh = autoRefresh;
    }
    ALOGE_IF(err, "IGraphicBufferProducer::setAutoRefresh(%d) returned %s",
            autoRefresh, strerror(-err));
    return err;
}

int Surface::setBuffersDimensions(uint32_t width, uint32_t height)
{
    ATRACE_CALL();
    ALOGV("Surface::setBuffersDimensions");

    if ((width && !height) || (!width && height))
        return BAD_VALUE;

    Mutex::Autolock lock(mMutex);
    if (width != mReqWidth || height != mReqHeight) {
        mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
    }
    mReqWidth = width;
    mReqHeight = height;
    return NO_ERROR;
}

int Surface::setBuffersUserDimensions(uint32_t width, uint32_t height)
{
    ATRACE_CALL();
    ALOGV("Surface::setBuffersUserDimensions");

    if ((width && !height) || (!width && height))
        return BAD_VALUE;

    Mutex::Autolock lock(mMutex);
    if (width != mUserWidth || height != mUserHeight) {
        mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
    }
    mUserWidth = width;
    mUserHeight = height;
    return NO_ERROR;
}

int Surface::setBuffersFormat(PixelFormat format)
{
    ALOGV("Surface::setBuffersFormat");

    Mutex::Autolock lock(mMutex);
    if (format != mReqFormat) {
        mSharedBufferSlot = BufferItem::INVALID_BUFFER_SLOT;
    }
    mReqFormat = format;
    return NO_ERROR;
}

int Surface::setScalingMode(int mode)
{
    ATRACE_CALL();
    ALOGV("Surface::setScalingMode(%d)", mode);

    switch (mode) {
        case NATIVE_WINDOW_SCALING_MODE_FREEZE:
        case NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW:
        case NATIVE_WINDOW_SCALING_MODE_SCALE_CROP:
        case NATIVE_WINDOW_SCALING_MODE_NO_SCALE_CROP:
            break;
        default:
            ALOGE("unknown scaling mode: %d", mode);
            return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);
    mScalingMode = mode;
    return NO_ERROR;
}

int Surface::setBuffersTransform(uint32_t transform)
{
    ATRACE_CALL();
    ALOGV("Surface::setBuffersTransform");
    Mutex::Autolock lock(mMutex);
    // Ensure NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY is sticky. If the client sets the flag, do not
    // override it until the surface is disconnected. This is a temporary workaround for camera
    // until they switch to using Buffer State Layers. Currently if client sets the buffer transform
    // it may be overriden by the buffer producer when the producer sets the buffer transform.
    if (transformToDisplayInverse()) {
        transform |= NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
    }
    mTransform = transform;
    return NO_ERROR;
}

int Surface::setBuffersStickyTransform(uint32_t transform)
{
    ATRACE_CALL();
    ALOGV("Surface::setBuffersStickyTransform");
    Mutex::Autolock lock(mMutex);
    mStickyTransform = transform;
    return NO_ERROR;
}

int Surface::setBuffersTimestamp(int64_t timestamp)
{
    ALOGV("Surface::setBuffersTimestamp");
    Mutex::Autolock lock(mMutex);
    mTimestamp = timestamp;
    return NO_ERROR;
}

int Surface::setBuffersDataSpace(Dataspace dataSpace)
{
    ALOGV("Surface::setBuffersDataSpace");
    Mutex::Autolock lock(mMutex);
    mDataSpace = dataSpace;
    return NO_ERROR;
}

int Surface::setBuffersSmpte2086Metadata(const android_smpte2086_metadata* metadata) {
    ALOGV("Surface::setBuffersSmpte2086Metadata");
    Mutex::Autolock lock(mMutex);
    if (metadata) {
        mHdrMetadata.smpte2086 = *metadata;
        mHdrMetadata.validTypes |= HdrMetadata::SMPTE2086;
    } else {
        mHdrMetadata.validTypes &= ~HdrMetadata::SMPTE2086;
    }
    return NO_ERROR;
}

int Surface::setBuffersCta8613Metadata(const android_cta861_3_metadata* metadata) {
    ALOGV("Surface::setBuffersCta8613Metadata");
    Mutex::Autolock lock(mMutex);
    if (metadata) {
        mHdrMetadata.cta8613 = *metadata;
        mHdrMetadata.validTypes |= HdrMetadata::CTA861_3;
    } else {
        mHdrMetadata.validTypes &= ~HdrMetadata::CTA861_3;
    }
    return NO_ERROR;
}

int Surface::setBuffersHdr10PlusMetadata(const size_t size, const uint8_t* metadata) {
    ALOGV("Surface::setBuffersBlobMetadata");
    Mutex::Autolock lock(mMutex);
    if (size > 0) {
        mHdrMetadata.hdr10plus.assign(metadata, metadata + size);
        mHdrMetadata.validTypes |= HdrMetadata::HDR10PLUS;
    } else {
        mHdrMetadata.validTypes &= ~HdrMetadata::HDR10PLUS;
        mHdrMetadata.hdr10plus.clear();
    }
    return NO_ERROR;
}

Dataspace Surface::getBuffersDataSpace() {
    ALOGV("Surface::getBuffersDataSpace");
    Mutex::Autolock lock(mMutex);
    return mDataSpace;
}

void Surface::freeAllBuffers() {
    for (int i = 0; i < NUM_BUFFER_SLOTS; i++) {
        mSlots[i].buffer = nullptr;
    }
}

status_t Surface::getAndFlushBuffersFromSlots(const std::vector<int32_t>& slots,
        std::vector<sp<GraphicBuffer>>* outBuffers) {
    ALOGV("Surface::getAndFlushBuffersFromSlots");
    for (int32_t i : slots) {
        if (i < 0 || i >= NUM_BUFFER_SLOTS) {
            ALOGE("%s: Invalid slotIndex: %d", __FUNCTION__, i);
            return BAD_VALUE;
        }
    }

    Mutex::Autolock lock(mMutex);
    for (int32_t i : slots) {
        if (mSlots[i].buffer == nullptr) {
            ALOGW("%s: Discarded slot %d doesn't contain buffer!", __FUNCTION__, i);
            continue;
        }
        outBuffers->push_back(mSlots[i].buffer);
        mSlots[i].buffer = nullptr;
    }
    return OK;
}

void Surface::setSurfaceDamage(android_native_rect_t* rects, size_t numRects) {
    ATRACE_CALL();
    ALOGV("Surface::setSurfaceDamage");
    Mutex::Autolock lock(mMutex);

    if (mConnectedToCpu || numRects == 0) {
        mDirtyRegion = Region::INVALID_REGION;
        return;
    }

    mDirtyRegion.clear();
    for (size_t r = 0; r < numRects; ++r) {
        // We intentionally flip top and bottom here, since because they're
        // specified with a bottom-left origin, top > bottom, which fails
        // validation in the Region class. We will fix this up when we flip to a
        // top-left origin in queueBuffer.
        Rect rect(rects[r].left, rects[r].bottom, rects[r].right, rects[r].top);
        mDirtyRegion.orSelf(rect);
    }
}

// ----------------------------------------------------------------------
// the lock/unlock APIs must be used from the same thread

static status_t copyBlt(
        const sp<GraphicBuffer>& dst,
        const sp<GraphicBuffer>& src,
        const Region& reg,
        int *dstFenceFd)
{
    if (dst->getId() == src->getId())
        return OK;

    // src and dst with, height and format must be identical. no verification
    // is done here.
    status_t err;
    uint8_t* src_bits = nullptr;
    err = src->lock(GRALLOC_USAGE_SW_READ_OFTEN, reg.bounds(),
            reinterpret_cast<void**>(&src_bits));
    ALOGE_IF(err, "error locking src buffer %s", strerror(-err));

    uint8_t* dst_bits = nullptr;
    err = dst->lockAsync(GRALLOC_USAGE_SW_WRITE_OFTEN, reg.bounds(),
            reinterpret_cast<void**>(&dst_bits), *dstFenceFd);
    ALOGE_IF(err, "error locking dst buffer %s", strerror(-err));
    *dstFenceFd = -1;

    Region::const_iterator head(reg.begin());
    Region::const_iterator tail(reg.end());
    if (head != tail && src_bits && dst_bits) {
        const size_t bpp = bytesPerPixel(src->format);
        const size_t dbpr = static_cast<uint32_t>(dst->stride) * bpp;
        const size_t sbpr = static_cast<uint32_t>(src->stride) * bpp;

        while (head != tail) {
            const Rect& r(*head++);
            int32_t h = r.height();
            if (h <= 0) continue;
            size_t size = static_cast<uint32_t>(r.width()) * bpp;
            uint8_t const * s = src_bits +
                    static_cast<uint32_t>(r.left + src->stride * r.top) * bpp;
            uint8_t       * d = dst_bits +
                    static_cast<uint32_t>(r.left + dst->stride * r.top) * bpp;
            if (dbpr==sbpr && size==sbpr) {
                size *= static_cast<size_t>(h);
                h = 1;
            }
            do {
                memcpy(d, s, size);
                d += dbpr;
                s += sbpr;
            } while (--h > 0);
        }
    }

    if (src_bits)
        src->unlock();

    if (dst_bits)
        dst->unlockAsync(dstFenceFd);

    return err;
}

// ----------------------------------------------------------------------------

status_t Surface::lock(
        ANativeWindow_Buffer* outBuffer, ARect* inOutDirtyBounds)
{
    if (mLockedBuffer != nullptr) {
        ALOGE("Surface::lock failed, already locked");
        return INVALID_OPERATION;
    }

    if (!mConnectedToCpu) {
        int err = Surface::connect(NATIVE_WINDOW_API_CPU);
        if (err) {
            return err;
        }
        // we're intending to do software rendering from this point
        setUsage(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
    }

    ANativeWindowBuffer* out;
    int fenceFd = -1;
    status_t err = dequeueBuffer(&out, &fenceFd);
    ALOGE_IF(err, "dequeueBuffer failed (%s)", strerror(-err));
    if (err == NO_ERROR) {
        sp<GraphicBuffer> backBuffer(GraphicBuffer::getSelf(out));
        const Rect bounds(backBuffer->width, backBuffer->height);

        Region newDirtyRegion;
        if (inOutDirtyBounds) {
            newDirtyRegion.set(static_cast<Rect const&>(*inOutDirtyBounds));
            newDirtyRegion.andSelf(bounds);
        } else {
            newDirtyRegion.set(bounds);
        }

        // figure out if we can copy the frontbuffer back
        const sp<GraphicBuffer>& frontBuffer(mPostedBuffer);
        const bool canCopyBack = (frontBuffer != nullptr &&
                backBuffer->width  == frontBuffer->width &&
                backBuffer->height == frontBuffer->height &&
                backBuffer->format == frontBuffer->format);

        if (canCopyBack) {
            // copy the area that is invalid and not repainted this round
            const Region copyback(mDirtyRegion.subtract(newDirtyRegion));
            if (!copyback.isEmpty()) {
                copyBlt(backBuffer, frontBuffer, copyback, &fenceFd);
            }
        } else {
            // if we can't copy-back anything, modify the user's dirty
            // region to make sure they redraw the whole buffer
            newDirtyRegion.set(bounds);
            mDirtyRegion.clear();
            Mutex::Autolock lock(mMutex);
            for (size_t i=0 ; i<NUM_BUFFER_SLOTS ; i++) {
                mSlots[i].dirtyRegion.clear();
            }
        }


        { // scope for the lock
            Mutex::Autolock lock(mMutex);
            int backBufferSlot(getSlotFromBufferLocked(backBuffer.get()));
            if (backBufferSlot >= 0) {
                Region& dirtyRegion(mSlots[backBufferSlot].dirtyRegion);
                mDirtyRegion.subtract(dirtyRegion);
                dirtyRegion = newDirtyRegion;
            }
        }

        mDirtyRegion.orSelf(newDirtyRegion);
        if (inOutDirtyBounds) {
            *inOutDirtyBounds = newDirtyRegion.getBounds();
        }

        void* vaddr;
        status_t res = backBuffer->lockAsync(
                GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
                newDirtyRegion.bounds(), &vaddr, fenceFd);

        ALOGW_IF(res, "failed locking buffer (handle = %p)",
                backBuffer->handle);

        if (res != 0) {
            err = INVALID_OPERATION;
        } else {
            mLockedBuffer = backBuffer;
            outBuffer->width  = backBuffer->width;
            outBuffer->height = backBuffer->height;
            outBuffer->stride = backBuffer->stride;
            outBuffer->format = backBuffer->format;
            outBuffer->bits   = vaddr;
        }
    }
    return err;
}

status_t Surface::unlockAndPost()
{
    if (mLockedBuffer == nullptr) {
        ALOGE("Surface::unlockAndPost failed, no locked buffer");
        return INVALID_OPERATION;
    }

    int fd = -1;
    status_t err = mLockedBuffer->unlockAsync(&fd);
    ALOGE_IF(err, "failed unlocking buffer (%p)", mLockedBuffer->handle);

    err = queueBuffer(mLockedBuffer.get(), fd);
    ALOGE_IF(err, "queueBuffer (handle=%p) failed (%s)",
            mLockedBuffer->handle, strerror(-err));

    mPostedBuffer = mLockedBuffer;
    mLockedBuffer = nullptr;
    return err;
}

bool Surface::waitForNextFrame(uint64_t lastFrame, nsecs_t timeout) {
    Mutex::Autolock lock(mMutex);
    if (mNextFrameNumber > lastFrame) {
      return true;
    }
    return mQueueBufferCondition.waitRelative(mMutex, timeout) == OK;
}

status_t Surface::getUniqueId(uint64_t* outId) const {
    Mutex::Autolock lock(mMutex);
    return mGraphicBufferProducer->getUniqueId(outId);
}

int Surface::getConsumerUsage(uint64_t* outUsage) const {
    Mutex::Autolock lock(mMutex);
    return mGraphicBufferProducer->getConsumerUsage(outUsage);
}

nsecs_t Surface::getLastDequeueStartTime() const {
    Mutex::Autolock lock(mMutex);
    return mLastDequeueStartTime;
}

status_t Surface::getAndFlushRemovedBuffers(std::vector<sp<GraphicBuffer>>* out) {
    if (out == nullptr) {
        ALOGE("%s: out must not be null!", __FUNCTION__);
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);
    *out = mRemovedBuffers;
    mRemovedBuffers.clear();
    return OK;
}

status_t Surface::attachAndQueueBufferWithDataspace(Surface* surface, sp<GraphicBuffer> buffer,
                                                    Dataspace dataspace) {
    if (buffer == nullptr) {
        return BAD_VALUE;
    }
    int err = static_cast<ANativeWindow*>(surface)->perform(surface, NATIVE_WINDOW_API_CONNECT,
                                                            NATIVE_WINDOW_API_CPU);
    if (err != OK) {
        return err;
    }
    ui::Dataspace tmpDataspace = surface->getBuffersDataSpace();
    err = surface->setBuffersDataSpace(dataspace);
    if (err != OK) {
        return err;
    }
    err = surface->attachBuffer(buffer->getNativeBuffer());
    if (err != OK) {
        return err;
    }
    err = static_cast<ANativeWindow*>(surface)->queueBuffer(surface, buffer->getNativeBuffer(), -1);
    if (err != OK) {
        return err;
    }
    err = surface->setBuffersDataSpace(tmpDataspace);
    if (err != OK) {
        return err;
    }
    err = surface->disconnect(NATIVE_WINDOW_API_CPU);
    return err;
}

void Surface::ProducerListenerProxy::onBuffersDiscarded(const std::vector<int32_t>& slots) {
    ATRACE_CALL();
    sp<Surface> parent = mParent.promote();
    if (parent == nullptr) {
        return;
    }

    std::vector<sp<GraphicBuffer>> discardedBufs;
    status_t res = parent->getAndFlushBuffersFromSlots(slots, &discardedBufs);
    if (res != OK) {
        ALOGE("%s: Failed to get buffers from slots: %s(%d)", __FUNCTION__,
                strerror(-res), res);
        return;
    }

    mSurfaceListener->onBuffersDiscarded(discardedBufs);
}

}; // namespace android
