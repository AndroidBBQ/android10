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

#ifdef ANDROID
#define SWAPPYVK_USE_WRAPPER
#endif
#include <swappyVk/SwappyVk.h>

#include <map>
#include <condition_variable>
#include <cstring>
#include <unistd.h>

#include <dlfcn.h>
#include <cstdlib>

#include <inttypes.h>

#ifdef ANDROID
#include <mutex>
#include <pthread.h>
#include <list>
#include <android/looper.h>
#include <android/log.h>
#include "Trace.h"
#include "ChoreographerShim.h"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, "SwappyVk", __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, "SwappyVk", __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, "SwappyVk", __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SwappyVk", __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "SwappyVk", __VA_ARGS__)
#else
#define ATRACE_CALL() ((void)0)
#define ALOGE(...)    ((void)0)
#define ALOGW(...)    ((void)0)
#define ALOGD(...)    ((void)0)
#define ALOGV(...)    ((void)0)
#endif


constexpr uint32_t kThousand = 1000;
constexpr uint32_t kMillion  = 1000000;
constexpr uint32_t kBillion  = 1000000000;
constexpr uint32_t k16_6msec = 16666666;

constexpr uint32_t kTooCloseToVsyncBoundary     = 3000000;
constexpr uint32_t kTooFarAwayFromVsyncBoundary = 7000000;
constexpr uint32_t kNudgeWithinVsyncBoundaries  = 2000000;

// Note: The API functions is at the botton of the file.  Those functions call methods of the
// singleton SwappyVk class.  Those methods call virtual methods of the abstract SwappyVkBase
// class, which is actually implemented by one of the derived/concrete classes:
//
// - SwappyVkGoogleDisplayTiming
// - SwappyVkVulkanFallback
// - SwappyVkAndroidFallback

// Forward declarations:
class SwappyVk;

// AChoreographer is supported from API 24. To allow compilation for minSDK < 24
// and still use AChoreographer for SDK >= 24 we need runtime support to call
// AChoreographer APIs.

using PFN_AChoreographer_getInstance = AChoreographer* (*)();

using PFN_AChoreographer_postFrameCallback = void (*)(AChoreographer* choreographer,
                                                  AChoreographer_frameCallback callback,
                                                  void* data);

using PFN_AChoreographer_postFrameCallbackDelayed = void (*)(AChoreographer* choreographer,
                                                        AChoreographer_frameCallback callback,
                                                        void* data,
                                                        long delayMillis);

/***************************************************************************************************
 *
 * Per-Device abstract base class.
 *
 ***************************************************************************************************/

/**
 * Abstract base class that calls the Vulkan API.
 *
 * It is expected that one concrete class will be instantiated per VkDevice, and that all
 * VkSwapchainKHR's for a given VkDevice will share the same instance.
 *
 * Base class members are used by the derived classes to unify the behavior across implementaitons:
 *  @mThread - Thread used for getting Choreographer events.
 *  @mTreadRunning - Used to signal the tread to exit
 *  @mNextPresentID - unique ID for frame presentation.
 *  @mNextDesiredPresentTime - Holds the time in nanoseconds for the next frame to be presented.
 *  @mNextPresentIDToCheck - Used to determine whether presentation time needs to be adjusted.
 *  @mFrameID - Keeps track of how many Choreographer callbacks received.
 *  @mLastframeTimeNanos - Holds the last frame time reported by Choreographer.
 *  @mSumRefreshTime - Used together with @mSamples to calculate refresh rate based on Choreographer.
 */
class SwappyVkBase
{
public:
    SwappyVkBase(VkPhysicalDevice physicalDevice,
                 VkDevice         device,
                 uint64_t         refreshDur,
                 uint32_t         interval,
                 SwappyVk         &swappyVk,
                 void             *libVulkan) :
            mPhysicalDevice(physicalDevice), mDevice(device), mRefreshDur(refreshDur),
            mInterval(interval), mSwappyVk(swappyVk), mLibVulkan(libVulkan),
            mInitialized(false)
    {
#ifdef ANDROID
        InitVulkan();
#endif
        mpfnGetDeviceProcAddr =
                reinterpret_cast<PFN_vkGetDeviceProcAddr>(
                    dlsym(mLibVulkan, "vkGetDeviceProcAddr"));
        mpfnQueuePresentKHR =
                reinterpret_cast<PFN_vkQueuePresentKHR>(
                    mpfnGetDeviceProcAddr(mDevice, "vkQueuePresentKHR"));

        mLibAndroid = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
        if (mLibAndroid == nullptr) {
            ALOGE("FATAL: cannot open libandroid.so: %s", strerror(errno));
            abort();
        }

        mAChoreographer_getInstance =
                reinterpret_cast<PFN_AChoreographer_getInstance >(
                    dlsym(mLibAndroid, "AChoreographer_getInstance"));

        mAChoreographer_postFrameCallback =
                reinterpret_cast<PFN_AChoreographer_postFrameCallback >(
                        dlsym(mLibAndroid, "AChoreographer_postFrameCallback"));

        mAChoreographer_postFrameCallbackDelayed =
                reinterpret_cast<PFN_AChoreographer_postFrameCallbackDelayed >(
                        dlsym(mLibAndroid, "AChoreographer_postFrameCallbackDelayed"));
        if (!mAChoreographer_getInstance ||
            !mAChoreographer_postFrameCallback ||
            !mAChoreographer_postFrameCallbackDelayed) {
            ALOGE("FATAL: cannot get AChoreographer symbols");
            abort();
        }
    }
    virtual ~SwappyVkBase() {
        if(mLibAndroid)
            dlclose(mLibAndroid);
    }
    virtual bool doGetRefreshCycleDuration(VkSwapchainKHR swapchain,
                                           uint64_t*      pRefreshDuration) = 0;
    void doSetSwapInterval(VkSwapchainKHR swapchain,
                           uint32_t       interval)
    {
        mInterval = interval;
    }
    virtual VkResult doQueuePresent(VkQueue                 queue,
                                    uint32_t                queueFamilyIndex,
                                    const VkPresentInfoKHR* pPresentInfo) = 0;
protected:
    VkPhysicalDevice mPhysicalDevice;
    VkDevice         mDevice;
    uint64_t         mRefreshDur;
    uint32_t         mInterval;
    SwappyVk         &mSwappyVk;
    void             *mLibVulkan;
    bool             mInitialized;
    pthread_t mThread = 0;
    ALooper *mLooper = nullptr;
    bool mTreadRunning = false;
    AChoreographer *mChoreographer = nullptr;
    std::mutex mWaitingMutex;
    std::condition_variable mWaitingCondition;
    uint32_t mNextPresentID = 0;
    uint64_t mNextDesiredPresentTime = 0;
    uint32_t mNextPresentIDToCheck = 2;

    PFN_vkGetDeviceProcAddr mpfnGetDeviceProcAddr = nullptr;
    PFN_vkQueuePresentKHR   mpfnQueuePresentKHR = nullptr;
    PFN_vkGetRefreshCycleDurationGOOGLE mpfnGetRefreshCycleDurationGOOGLE = nullptr;
    PFN_vkGetPastPresentationTimingGOOGLE mpfnGetPastPresentationTimingGOOGLE = nullptr;

    void *mLibAndroid = nullptr;
    PFN_AChoreographer_getInstance mAChoreographer_getInstance = nullptr;
    PFN_AChoreographer_postFrameCallback mAChoreographer_postFrameCallback = nullptr;
    PFN_AChoreographer_postFrameCallbackDelayed mAChoreographer_postFrameCallbackDelayed = nullptr;

    long mFrameID = 0;
    long mTargetFrameID = 0;
    uint64_t mLastframeTimeNanos = 0;
    long mSumRefreshTime = 0;
    long mSamples = 0;
    long mCallbacksBeforeIdle = 0;

    static constexpr int MAX_SAMPLES = 5;
    static constexpr int MAX_CALLBACKS_BEFORE_IDLE = 10;

    void initGoogExtention()
    {
        mpfnGetRefreshCycleDurationGOOGLE =
                reinterpret_cast<PFN_vkGetRefreshCycleDurationGOOGLE>(
                        mpfnGetDeviceProcAddr(mDevice, "vkGetRefreshCycleDurationGOOGLE"));
        mpfnGetPastPresentationTimingGOOGLE =
                reinterpret_cast<PFN_vkGetPastPresentationTimingGOOGLE>(
                        mpfnGetDeviceProcAddr(mDevice, "vkGetPastPresentationTimingGOOGLE"));
    }

    void startChoreographerThread();
    void stopChoreographerThread();
    static void *looperThreadWrapper(void *data);
    void *looperThread();
    static void frameCallback(long frameTimeNanos, void *data);
    void onDisplayRefresh(long frameTimeNanos);
    void calcRefreshRate(uint64_t currentTime);
    void postChoreographerCallback();
};

void SwappyVkBase::startChoreographerThread() {
    std::unique_lock<std::mutex> lock(mWaitingMutex);
    // create a new ALooper thread to get Choreographer events
    mTreadRunning = true;
    pthread_create(&mThread, NULL, looperThreadWrapper, this);
    mWaitingCondition.wait(lock, [&]() { return mChoreographer != nullptr; });
}

void SwappyVkBase::stopChoreographerThread() {
    if (mLooper) {
        ALooper_acquire(mLooper);
        mTreadRunning = false;
        ALooper_wake(mLooper);
        ALooper_release(mLooper);
        pthread_join(mThread, NULL);
    }
}

void *SwappyVkBase::looperThreadWrapper(void *data) {
    SwappyVkBase *me = reinterpret_cast<SwappyVkBase *>(data);
    return me->looperThread();
}

void *SwappyVkBase::looperThread() {
    int outFd, outEvents;
    void *outData;

    mLooper = ALooper_prepare(0);
    if (!mLooper) {
        ALOGE("ALooper_prepare failed");
        return NULL;
    }

    mChoreographer = mAChoreographer_getInstance();
    if (!mChoreographer) {
        ALOGE("AChoreographer_getInstance failed");
        return NULL;
    }
    mWaitingCondition.notify_all();

    while (mTreadRunning) {
        ALooper_pollAll(-1, &outFd, &outEvents, &outData);
    }

    return NULL;
}

void SwappyVkBase::frameCallback(long frameTimeNanos, void *data) {
    SwappyVkBase *me = reinterpret_cast<SwappyVkBase *>(data);
    me->onDisplayRefresh(frameTimeNanos);
}

void SwappyVkBase::onDisplayRefresh(long frameTimeNanos) {
    std::lock_guard<std::mutex> lock(mWaitingMutex);
    struct timespec currTime;
    clock_gettime(CLOCK_MONOTONIC, &currTime);
    uint64_t currentTime =
            ((uint64_t) currTime.tv_sec * kBillion) + (uint64_t) currTime.tv_nsec;

    calcRefreshRate(currentTime);
    mLastframeTimeNanos = currentTime;
    mFrameID++;
    mWaitingCondition.notify_all();

    // queue the next frame callback
    if (mCallbacksBeforeIdle > 0) {
        mCallbacksBeforeIdle--;
        mAChoreographer_postFrameCallbackDelayed(mChoreographer, frameCallback, this, 1);
    }
}

void SwappyVkBase::postChoreographerCallback() {
    if (mCallbacksBeforeIdle == 0) {
        mAChoreographer_postFrameCallbackDelayed(mChoreographer, frameCallback, this, 1);
    }
    mCallbacksBeforeIdle = MAX_CALLBACKS_BEFORE_IDLE;
}

void SwappyVkBase::calcRefreshRate(uint64_t currentTime) {
    long refresh_nano = currentTime - mLastframeTimeNanos;

    if (mRefreshDur != 0 || mLastframeTimeNanos == 0) {
        return;
    }

    mSumRefreshTime += refresh_nano;
    mSamples++;

    if (mSamples == MAX_SAMPLES) {
        mRefreshDur = mSumRefreshTime / mSamples;
    }
}


/***************************************************************************************************
 *
 * Per-Device concrete/derived class for using VK_GOOGLE_display_timing.
 *
 * This class uses the VK_GOOGLE_display_timing in order to present frames at a muliple (the "swap
 * interval") of a fixed refresh-cycle duration (i.e. the time between successive vsync's).
 *
 * In order to reduce complexity, some simplifying assumptions are made:
 *
 * - We assume a fixed refresh-rate (FRR) display that's between 60 Hz and 120 Hz.
 *
 * - While Vulkan allows applications to create and use multiple VkSwapchainKHR's per VkDevice, and
 *   to re-create VkSwapchainKHR's, we assume that the application uses a single VkSwapchainKHR,
 *   and never re-creates it.
 *
 * - The values reported back by the VK_GOOGLE_display_timing extension (which comes from
 *   lower-level Android interfaces) are not precise, and that values can drift over time.  For
 *   example, the refresh-cycle duration for a 60 Hz display should be 16,666,666 nsec; but the
 *   value reported back by the extension won't be precisely this.  Also, the differences betweeen
 *   the times of two successive frames won't be an exact multiple of 16,666,666 nsec.  This can
 *   make it difficult to precisely predict when a future vsync will be (it can appear to drift
 *   overtime).  Therefore, we try to give a desiredPresentTime for each image that is between 3
 *   and 7 msec before vsync.  We look at the actualPresentTime for previously-presented images,
 *   and nudge the future desiredPresentTime back within those 3-7 msec boundaries.
 *
 * - There can be a few frames of latency between when an image is presented and when the
 *   actualPresentTime is available for that image.  Therefore, we initially just pick times based
 *   upon CLOCK_MONOTONIC (which is the time domain for VK_GOOGLE_display_timing).  After we get
 *   past-present times, we nudge the desiredPresentTime, we wait for a few presents before looking
 *   again to see whether we need to nudge again.
 *
 * - If, for some reason, an application can't keep up with its chosen swap interval (e.g. it's
 *   designed for 30FPS on a premium device and is now running on a slow device; or it's running on
 *   a 120Hz display), this algorithm may not be able to make up for this (i.e. smooth rendering at
 *   a targetted frame rate may not be possible with an application that can't render fast enough).
 *
 ***************************************************************************************************/

/**
 * Concrete/derived class that sits on top of VK_GOOGLE_display_timing
 */
class SwappyVkGoogleDisplayTiming : public SwappyVkBase
{
public:
    SwappyVkGoogleDisplayTiming(VkPhysicalDevice physicalDevice,
                                VkDevice         device,
                                SwappyVk         &swappyVk,
                                void             *libVulkan) :
            SwappyVkBase(physicalDevice, device, k16_6msec, 1, swappyVk, libVulkan)
    {
        initGoogExtention();
    }
    virtual bool doGetRefreshCycleDuration(VkSwapchainKHR swapchain,
                                           uint64_t*      pRefreshDuration) override
    {
        VkRefreshCycleDurationGOOGLE refreshCycleDuration;
        VkResult res = mpfnGetRefreshCycleDurationGOOGLE(mDevice, swapchain, &refreshCycleDuration);
        if (res != VK_SUCCESS) {
            // This should never occur, but in case it does, return 16,666,666ns:
            mRefreshDur = k16_6msec;
        } else {
            mRefreshDur = refreshCycleDuration.refreshDuration;
        }

        // TEMP CODE: LOG REFRESH DURATION AND RATE:
        double refreshRate = mRefreshDur;
        refreshRate = 1.0 / (refreshRate / 1000000000.0);

        ALOGD("Returning refresh duration of %" PRIu64 " nsec (approx %f Hz)", mRefreshDur, refreshRate);

        *pRefreshDuration = mRefreshDur;
        return true;
    }
    virtual VkResult doQueuePresent(VkQueue                 queue,
                                    uint32_t                queueFamilyIndex,
                                    const VkPresentInfoKHR* pPresentInfo) override;

private:
    void calculateNextDesiredPresentTime(VkSwapchainKHR swapchain);
    void checkPastPresentTiming(VkSwapchainKHR swapchain);
};

VkResult SwappyVkGoogleDisplayTiming::doQueuePresent(VkQueue                 queue,
                                                     uint32_t                queueFamilyIndex,
                                                     const VkPresentInfoKHR* pPresentInfo)
{
    VkResult ret = VK_SUCCESS;

    calculateNextDesiredPresentTime(pPresentInfo->pSwapchains[0]);

    // Setup the new structures to pass:
    VkPresentTimeGOOGLE *pPresentTimes =
            reinterpret_cast<VkPresentTimeGOOGLE*>(malloc(sizeof(VkPresentTimeGOOGLE) *
                                                          pPresentInfo->swapchainCount));
    for (uint32_t i = 0 ; i < pPresentInfo->swapchainCount ; i++) {
        pPresentTimes[i].presentID = mNextPresentID;
        pPresentTimes[i].desiredPresentTime = mNextDesiredPresentTime;
    }
    mNextPresentID++;

    VkPresentTimesInfoGOOGLE presentTimesInfo = {VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
                                                 pPresentInfo->pNext, pPresentInfo->swapchainCount,
                                                 pPresentTimes};
    VkPresentInfoKHR replacementPresentInfo = {pPresentInfo->sType, &presentTimesInfo,
                                               pPresentInfo->waitSemaphoreCount,
                                               pPresentInfo->pWaitSemaphores,
                                               pPresentInfo->swapchainCount,
                                               pPresentInfo->pSwapchains,
                                               pPresentInfo->pImageIndices, pPresentInfo->pResults};
    ret = mpfnQueuePresentKHR(queue, &replacementPresentInfo);
    free(pPresentTimes);
    return ret;
}

void SwappyVkGoogleDisplayTiming::calculateNextDesiredPresentTime(VkSwapchainKHR swapchain)
{
    struct timespec currTime;
    clock_gettime(CLOCK_MONOTONIC, &currTime);
    uint64_t currentTime =
            ((uint64_t) currTime.tv_sec * kBillion) + (uint64_t) currTime.tv_nsec;


    // Determine the desiredPresentTime:
    if (!mNextDesiredPresentTime) {
        mNextDesiredPresentTime = currentTime + mRefreshDur;
    } else {
        // Look at the timing of past presents, and potentially adjust mNextDesiredPresentTime:
        checkPastPresentTiming(swapchain);
        mNextDesiredPresentTime += mRefreshDur * mInterval;

        // Make sure the calculated time is not before the current time to present
        if (mNextDesiredPresentTime < currentTime) {
            mNextDesiredPresentTime = currentTime + mRefreshDur;
        }
    }
}

void SwappyVkGoogleDisplayTiming::checkPastPresentTiming(VkSwapchainKHR swapchain)
{
    VkResult ret = VK_SUCCESS;

    if (mNextPresentID <= mNextPresentIDToCheck) {
        return;
    }
    // Check the timing of past presents to see if we need to adjust mNextDesiredPresentTime:
    uint32_t pastPresentationTimingCount = 0;
    VkResult err = mpfnGetPastPresentationTimingGOOGLE(mDevice, swapchain,
                                                       &pastPresentationTimingCount, NULL);
    if (!pastPresentationTimingCount) {
        return;
    }
    // TODO: don't allocate memory for the timestamps every time.
    VkPastPresentationTimingGOOGLE *past =
            reinterpret_cast<VkPastPresentationTimingGOOGLE*>(
                    malloc(sizeof(VkPastPresentationTimingGOOGLE) *
                           pastPresentationTimingCount));
    err = mpfnGetPastPresentationTimingGOOGLE(mDevice, swapchain,
                                              &pastPresentationTimingCount, past);
    for (uint32_t i = 0; i < pastPresentationTimingCount; i++) {
        // Note: On Android, actualPresentTime can actually be before desiredPresentTime
        // (which shouldn't be possible.  Therefore, this must be a signed integer.
        int64_t amountEarlyBy =
                (int64_t) past[i].actualPresentTime - (int64_t)past[i].desiredPresentTime;
        if (amountEarlyBy < kTooCloseToVsyncBoundary) {
            // We're getting too close to vsync.  Nudge the next present back
            // towards/in the boundaries, and check back after a few more presents:
            mNextDesiredPresentTime -= kNudgeWithinVsyncBoundaries;
            mNextPresentIDToCheck = mNextPresentID + 7;
            break;
        }
        if (amountEarlyBy > kTooFarAwayFromVsyncBoundary) {
            // We're getting too far away from vsync.  Nudge the next present back
            // towards/in the boundaries, and check back after a few more presents:
            mNextDesiredPresentTime += kNudgeWithinVsyncBoundaries;
            mNextPresentIDToCheck = mNextPresentID + 7;
            break;
        }
    }
    free(past);
}

/**
 * Concrete/derived class that sits on top of VK_GOOGLE_display_timing
 */
class SwappyVkGoogleDisplayTimingAndroid : public SwappyVkGoogleDisplayTiming
{
public:
    SwappyVkGoogleDisplayTimingAndroid(VkPhysicalDevice physicalDevice,
                                VkDevice         device,
                                SwappyVk         &swappyVk,
                                void             *libVulkan) :
            SwappyVkGoogleDisplayTiming(physicalDevice, device, swappyVk,libVulkan) {
        startChoreographerThread();
    }

    ~SwappyVkGoogleDisplayTimingAndroid() {
        stopChoreographerThread();
        destroyVkSyncObjects();
    }

    virtual bool doGetRefreshCycleDuration(VkSwapchainKHR swapchain,
                                          uint64_t*      pRefreshDuration) override {
        bool res = SwappyVkGoogleDisplayTiming::doGetRefreshCycleDuration(swapchain, pRefreshDuration);
        return res;
    }



    virtual VkResult doQueuePresent(VkQueue queue,
                                    uint32_t queueFamilyIndex,
                                    const VkPresentInfoKHR *pPresentInfo) override;

private:
    VkResult initializeVkSyncObjects(VkQueue queue, uint32_t queueFamilyIndex);
    void destroyVkSyncObjects();

    void waitForFenceChoreographer(VkQueue queue);

    struct VkSync {
        VkFence fence;
        VkSemaphore semaphore;
        VkCommandBuffer command;
        VkEvent event;
    };

    std::map<VkQueue, std::list<VkSync>> mFreeSync;
    std::map<VkQueue, std::list<VkSync>> mPendingSync;
    std::map<VkQueue, VkCommandPool> mCommandPool;

    static constexpr int MAX_PENDING_FENCES = 1;
};

VkResult SwappyVkGoogleDisplayTimingAndroid::initializeVkSyncObjects(VkQueue   queue,
                                                                     uint32_t  queueFamilyIndex)
{
    if (mCommandPool.find(queue) != mCommandPool.end()) {
        return VK_SUCCESS;
    }

    VkSync sync;

    const VkCommandPoolCreateInfo cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = queueFamilyIndex,
            .flags = 0,
    };

    VkResult res = vkCreateCommandPool(mDevice, &cmd_pool_info, NULL, &mCommandPool[queue]);
    if (res) {
        ALOGE("vkCreateCommandPool failed %d", res);
        return res;
    }
    const VkCommandBufferAllocateInfo present_cmd_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = mCommandPool[queue],
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };

    for(int i = 0; i < MAX_PENDING_FENCES; i++) {
        VkFenceCreateInfo fence_ci =
                {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = 0};

        res = vkCreateFence(mDevice, &fence_ci, NULL, &sync.fence);
        if (res) {
            ALOGE("failed to create fence: %d", res);
            return res;
        }

        VkSemaphoreCreateInfo semaphore_ci =
                {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = NULL, .flags = 0};

        res = vkCreateSemaphore(mDevice, &semaphore_ci, NULL, &sync.semaphore);
        if (res) {
            ALOGE("failed to create semaphore: %d", res);
            return res;
        }


        res = vkAllocateCommandBuffers(mDevice, &present_cmd_info, &sync.command);
        if (res) {
            ALOGE("vkAllocateCommandBuffers failed %d", res);
            return res;
        }

        const VkCommandBufferBeginInfo cmd_buf_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = NULL,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = NULL,
        };

        res = vkBeginCommandBuffer(sync.command, &cmd_buf_info);
        if (res) {
            ALOGE("vkAllocateCommandBuffers failed %d", res);
            return res;
        }

        VkEventCreateInfo event_info = {
                .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
        };

        res = vkCreateEvent(mDevice, &event_info, NULL, &sync.event);
        if (res) {
            ALOGE("vkCreateEvent failed %d", res);
            return res;
        }

        vkCmdSetEvent(sync.command, sync.event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

        res = vkEndCommandBuffer(sync.command);
        if (res) {
            ALOGE("vkCreateEvent failed %d", res);
            return res;
        }

        mFreeSync[queue].push_back(sync);
    }

    return VK_SUCCESS;
}

void SwappyVkGoogleDisplayTimingAndroid::destroyVkSyncObjects() {
    for (auto it = mPendingSync.begin(); it != mPendingSync.end(); it++) {
        while (mPendingSync[it->first].size() > 0) {
            VkSync sync = mPendingSync[it->first].front();
            mPendingSync[it->first].pop_front();
            vkWaitForFences(mDevice, 1, &sync.fence, VK_TRUE, UINT64_MAX);
            vkResetFences(mDevice, 1, &sync.fence);
            mFreeSync[it->first].push_back(sync);
        }

        while (mFreeSync[it->first].size() > 0) {
            VkSync sync = mFreeSync[it->first].front();
            mFreeSync[it->first].pop_front();
            vkFreeCommandBuffers(mDevice, mCommandPool[it->first], 1, &sync.command);
            vkDestroyEvent(mDevice, sync.event, NULL);
            vkDestroySemaphore(mDevice, sync.semaphore, NULL);
            vkDestroyFence(mDevice, sync.fence, NULL);
        }

        vkDestroyCommandPool(mDevice, mCommandPool[it->first], NULL);
    }
}

void SwappyVkGoogleDisplayTimingAndroid::waitForFenceChoreographer(VkQueue queue)
{
    std::unique_lock<std::mutex> lock(mWaitingMutex);
    VkSync sync = mPendingSync[queue].front();
    mPendingSync[queue].pop_front();
    mWaitingCondition.wait(lock, [&]() {
        if (vkWaitForFences(mDevice, 1, &sync.fence, VK_TRUE, 0) == VK_TIMEOUT) {
            postChoreographerCallback();

            // adjust the target frame here as we are waiting additional frame for the fence
            mTargetFrameID++;
            return false;
        }
        return true;
    });

    vkResetFences(mDevice, 1, &sync.fence);
    mFreeSync[queue].push_back(sync);
}

VkResult SwappyVkGoogleDisplayTimingAndroid::doQueuePresent(VkQueue                 queue,
                                                     uint32_t                queueFamilyIndex,
                                                     const VkPresentInfoKHR* pPresentInfo)
{
    VkResult ret = initializeVkSyncObjects(queue, queueFamilyIndex);
    if (ret) {
        return ret;
    }

    {
        std::unique_lock<std::mutex> lock(mWaitingMutex);
        mWaitingCondition.wait(lock, [&]() {
            if (mFrameID < mTargetFrameID) {
                postChoreographerCallback();
                return false;
            }
            return true;
        });
    }

    if (mPendingSync[queue].size() >= MAX_PENDING_FENCES) {
        waitForFenceChoreographer(queue);
    }

    // Adjust the presentation time based on the current frameID we are at.
    if(mFrameID < mTargetFrameID) {
        ALOGE("Bad frame ID %ld < target %ld", mFrameID, mTargetFrameID);
        mTargetFrameID = mFrameID;
    }
    mNextDesiredPresentTime += (mFrameID - mTargetFrameID) * mRefreshDur;

    // Setup the new structures to pass:
    VkPresentTimeGOOGLE pPresentTimes[pPresentInfo->swapchainCount];
    for (uint32_t i = 0 ; i < pPresentInfo->swapchainCount ; i++) {
        pPresentTimes[i].presentID = mNextPresentID;
        pPresentTimes[i].desiredPresentTime = mNextDesiredPresentTime;
    }
    mNextPresentID++;

    VkSync sync = mFreeSync[queue].front();
    mFreeSync[queue].pop_front();
    mPendingSync[queue].push_back(sync);

    VkPipelineStageFlags pipe_stage_flags;
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.pWaitDstStageMask = &pipe_stage_flags;
    pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
    submit_info.pWaitSemaphores = pPresentInfo->pWaitSemaphores;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &sync.command;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &sync.semaphore;
    ret = vkQueueSubmit(queue, 1, &submit_info, sync.fence);
    if (ret) {
        ALOGE("Failed to vkQueueSubmit %d", ret);
        return ret;
    }

    VkPresentTimesInfoGOOGLE presentTimesInfo = {VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
                                                 pPresentInfo->pNext, pPresentInfo->swapchainCount,
                                                 pPresentTimes};
    VkPresentInfoKHR replacementPresentInfo = {pPresentInfo->sType, &presentTimesInfo,
                                               1,
                                               &sync.semaphore,
                                               pPresentInfo->swapchainCount,
                                               pPresentInfo->pSwapchains,
                                               pPresentInfo->pImageIndices, pPresentInfo->pResults};
    ret = mpfnQueuePresentKHR(queue, &replacementPresentInfo);

    // next present time is going to be 2 intervals from now, leaving 1 interval for cpu work
    // and 1 interval for gpu work
    mNextDesiredPresentTime = mLastframeTimeNanos + 2 * mRefreshDur * mInterval;
    mTargetFrameID = mFrameID + mInterval;

    return ret;
}

/***************************************************************************************************
 *
 * Per-Device concrete/derived class for the "Android fallback" path (uses
 * Choreographer to try to get presents to occur at the desired time).
 *
 ***************************************************************************************************/

/**
 * Concrete/derived class that sits on top of the Vulkan API
 */
#ifdef ANDROID
class SwappyVkAndroidFallback : public SwappyVkBase
{
public:
    SwappyVkAndroidFallback(VkPhysicalDevice physicalDevice,
                            VkDevice         device,
                            SwappyVk         &swappyVk,
                            void             *libVulkan) :
            SwappyVkBase(physicalDevice, device, 0, 1, swappyVk, libVulkan) {
        startChoreographerThread();
    }

        ~SwappyVkAndroidFallback() {
            stopChoreographerThread();
    }

    virtual bool doGetRefreshCycleDuration(VkSwapchainKHR swapchain,
                                               uint64_t*      pRefreshDuration) override
    {
        std::unique_lock<std::mutex> lock(mWaitingMutex);
        mWaitingCondition.wait(lock, [&]() {
            if (mRefreshDur == 0) {
                postChoreographerCallback();
                return false;
            }
            return true;
        });

        *pRefreshDuration = mRefreshDur;

        double refreshRate = mRefreshDur;
        refreshRate = 1.0 / (refreshRate / 1000000000.0);
        ALOGI("Returning refresh duration of %" PRIu64 " nsec (approx %f Hz)", mRefreshDur, refreshRate);
        return true;
    }

    virtual VkResult doQueuePresent(VkQueue                 queue,
                                    uint32_t                queueFamilyIndex,
                                    const VkPresentInfoKHR* pPresentInfo) override
    {
        {
            std::unique_lock<std::mutex> lock(mWaitingMutex);

            mWaitingCondition.wait(lock, [&]() {
                if (mFrameID < mTargetFrameID) {
                    postChoreographerCallback();
                    return false;
                }
                return true;
            });
        }
        mTargetFrameID = mFrameID + mInterval;
        return mpfnQueuePresentKHR(queue, pPresentInfo);
    }
};
#endif

/***************************************************************************************************
 *
 * Per-Device concrete/derived class for the "Vulkan fallback" path (i.e. no API/OS timing support;
 * just generic Vulkan)
 *
 ***************************************************************************************************/

/**
 * Concrete/derived class that sits on top of the Vulkan API
 */
class SwappyVkVulkanFallback : public SwappyVkBase
{
public:
    SwappyVkVulkanFallback(VkPhysicalDevice physicalDevice,
                            VkDevice         device,
                            SwappyVk         &swappyVk,
                            void             *libVulkan) :
            SwappyVkBase(physicalDevice, device, k16_6msec, 1, swappyVk, libVulkan) {}
    virtual bool doGetRefreshCycleDuration(VkSwapchainKHR swapchain,
                                           uint64_t*      pRefreshDuration) override
    {
        *pRefreshDuration = mRefreshDur;
        return true;
    }
    virtual VkResult doQueuePresent(VkQueue                 queue,
                                    uint32_t                queueFamilyIndex,
                                    const VkPresentInfoKHR* pPresentInfo) override
    {
        return mpfnQueuePresentKHR(queue, pPresentInfo);
    }
};




/***************************************************************************************************
 *
 * Singleton class that provides the high-level implementation of the Swappy entrypoints.
 *
 ***************************************************************************************************/
/**
 * Singleton class that provides the high-level implementation of the Swappy entrypoints.
 *
 * This class determines which low-level implementation to use for each physical
 * device, and then calls that class's do-method for the entrypoint.
 */
class SwappyVk
{
public:
    static SwappyVk& getInstance()
    {
        static SwappyVk instance;
        return instance;
    }
    ~SwappyVk() {
        if(mLibVulkan)
            dlclose(mLibVulkan);
    }

    void swappyVkDetermineDeviceExtensions(VkPhysicalDevice       physicalDevice,
                                           uint32_t               availableExtensionCount,
                                           VkExtensionProperties* pAvailableExtensions,
                                           uint32_t*              pRequiredExtensionCount,
                                           char**                 pRequiredExtensions);
    void SetQueueFamilyIndex(VkDevice   device,
                             VkQueue    queue,
                             uint32_t   queueFamilyIndex);
    bool GetRefreshCycleDuration(VkPhysicalDevice physicalDevice,
                                 VkDevice         device,
                                 VkSwapchainKHR   swapchain,
                                 uint64_t*        pRefreshDuration);
    void SetSwapInterval(VkDevice       device,
                         VkSwapchainKHR swapchain,
                         uint32_t       interval);
    VkResult QueuePresent(VkQueue                 queue,
                          const VkPresentInfoKHR* pPresentInfo);
    void DestroySwapchain(VkDevice                device,
                          VkSwapchainKHR          swapchain);

private:
    std::map<VkPhysicalDevice, bool> doesPhysicalDeviceHaveGoogleDisplayTiming;
    std::map<VkDevice, std::shared_ptr<SwappyVkBase>> perDeviceImplementation;
    std::map<VkSwapchainKHR, std::shared_ptr<SwappyVkBase>> perSwapchainImplementation;

    struct QueueFamilyIndex {
        VkDevice device;
        uint32_t queueFamilyIndex;
    };
    std::map<VkQueue, QueueFamilyIndex> perQueueFamilyIndex;

    void *mLibVulkan     = nullptr;

private:
    SwappyVk() {} // Need to implement this constructor
    SwappyVk(SwappyVk const&); // Don't implement a copy constructor--no copies
    void operator=(SwappyVk const&); // Don't implement--no copies
};

/**
 * Generic/Singleton implementation of swappyVkDetermineDeviceExtensions.
 */
void SwappyVk::swappyVkDetermineDeviceExtensions(
    VkPhysicalDevice       physicalDevice,
    uint32_t               availableExtensionCount,
    VkExtensionProperties* pAvailableExtensions,
    uint32_t*              pRequiredExtensionCount,
    char**                 pRequiredExtensions)
{
    // TODO: Refactor this to be more concise:
    if (!pRequiredExtensions) {
        for (uint32_t i = 0; i < availableExtensionCount; i++) {
            if (!strcmp(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
                        pAvailableExtensions[i].extensionName)) {
                (*pRequiredExtensionCount)++;
            }
        }
    } else {
        doesPhysicalDeviceHaveGoogleDisplayTiming[physicalDevice] = false;
        for (uint32_t i = 0, j = 0; i < availableExtensionCount; i++) {
            if (!strcmp(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
                        pAvailableExtensions[i].extensionName)) {
                if (j < *pRequiredExtensionCount) {
                    strcpy(pRequiredExtensions[j++], VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
                    doesPhysicalDeviceHaveGoogleDisplayTiming[physicalDevice] = true;
                }
            }
        }
    }
}

void SwappyVk::SetQueueFamilyIndex(VkDevice   device,
                                    VkQueue    queue,
                                    uint32_t   queueFamilyIndex)
{
    perQueueFamilyIndex[queue] = {device, queueFamilyIndex};
}


/**
 * Generic/Singleton implementation of swappyVkGetRefreshCycleDuration.
 */
bool SwappyVk::GetRefreshCycleDuration(VkPhysicalDevice physicalDevice,
                                       VkDevice         device,
                                       VkSwapchainKHR   swapchain,
                                       uint64_t*        pRefreshDuration)
{
    auto& pImplementation = perDeviceImplementation[device];
    if (!pImplementation) {
        // We have not seen this device yet.
        if (!mLibVulkan) {
            // This is the first time we've been called--initialize function pointers:
            mLibVulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
            if (!mLibVulkan)
            {
                // If Vulkan doesn't exist, bail-out early:
                return false;
            }
        }

        // First, based on whether VK_GOOGLE_display_timing is available
        // (determined and cached by swappyVkDetermineDeviceExtensions),
        // determine which derived class to use to implement the rest of the API
        if (doesPhysicalDeviceHaveGoogleDisplayTiming[physicalDevice]) {
#ifdef ANDROID
            pImplementation = std::make_shared<SwappyVkGoogleDisplayTimingAndroid>
                    (physicalDevice, device, getInstance(), mLibVulkan);
            ALOGV("SwappyVk initialized for VkDevice %p using VK_GOOGLE_display_timing on Android", device);
#else
            // Instantiate the class that sits on top of VK_GOOGLE_display_timing
            pImplementation = std::make_shared<SwappyVkGoogleDisplayTiming>
                    (physicalDevice, device, getInstance(), mLibVulkan);
            ALOGV("SwappyVk initialized for VkDevice %p using VK_GOOGLE_display_timing", device);
#endif
        } else {
            // Instantiate the class that sits on top of the basic Vulkan APIs
#ifdef ANDROID
            pImplementation = std::make_shared<SwappyVkAndroidFallback>
                    (physicalDevice, device, getInstance(), mLibVulkan);
            ALOGV("SwappyVk initialized for VkDevice %p using Android fallback", device);
#else  // ANDROID
            pImplementation = std::make_shared<SwappyVkVulkanFallback>
                    (physicalDevice, device, getInstance(), mLibVulkan);
            ALOGV("SwappyVk initialized for VkDevice %p using Vulkan-only fallback", device);
#endif // ANDROID
        }

        if (!pImplementation) {
            // This shouldn't happen, but if it does, something is really wrong.
            return false;
        }
    }

    // Cache the per-swapchain pointer to the derived class:
    perSwapchainImplementation[swapchain] = pImplementation;

    // Now, call that derived class to get the refresh duration to return
    return pImplementation->doGetRefreshCycleDuration(swapchain, pRefreshDuration);
}


/**
 * Generic/Singleton implementation of swappyVkSetSwapInterval.
 */
void SwappyVk::SetSwapInterval(VkDevice       device,
                               VkSwapchainKHR swapchain,
                               uint32_t       interval)
{
    auto& pImplementation = perDeviceImplementation[device];
    if (!pImplementation) {
        return;
    }
    pImplementation->doSetSwapInterval(swapchain, interval);
}


/**
 * Generic/Singleton implementation of swappyVkQueuePresent.
 */
VkResult SwappyVk::QueuePresent(VkQueue                 queue,
                                const VkPresentInfoKHR* pPresentInfo)
{
    if (perQueueFamilyIndex.find(queue) == perQueueFamilyIndex.end()) {
        ALOGE("Unknown queue %p. Did you call SwappyVkSetQueueFamilyIndex ?", queue);
        return VK_INCOMPLETE;
    }

    // This command doesn't have a VkDevice.  It should have at least one VkSwapchainKHR's.  For
    // this command, all VkSwapchainKHR's will have the same VkDevice and VkQueue.
    if ((pPresentInfo->swapchainCount == 0) || (!pPresentInfo->pSwapchains)) {
        // This shouldn't happen, but if it does, something is really wrong.
        return VK_ERROR_DEVICE_LOST;
    }
    auto& pImplementation = perSwapchainImplementation[*pPresentInfo->pSwapchains];
    if (pImplementation) {
        return pImplementation->doQueuePresent(queue,
                                               perQueueFamilyIndex[queue].queueFamilyIndex,
                                               pPresentInfo);
    } else {
        // This should only happen if the API was used wrong (e.g. they never
        // called swappyVkGetRefreshCycleDuration).
        // NOTE: Technically, a Vulkan library shouldn't protect a user from
        // themselves, but we'll be friendlier
        return VK_ERROR_DEVICE_LOST;
    }
}

void SwappyVk::DestroySwapchain(VkDevice                device,
                                        VkSwapchainKHR          swapchain) {
    auto it = perQueueFamilyIndex.begin();
    while (it != perQueueFamilyIndex.end()) {
        if (it->second.device == device) {
            it = perQueueFamilyIndex.erase(it);
        } else {
            ++it;
        }
    }

    perDeviceImplementation[device] = nullptr;
    perSwapchainImplementation[swapchain] = nullptr;
}


/***************************************************************************************************
 *
 * API ENTRYPOINTS
 *
 ***************************************************************************************************/

extern "C" {

void SwappyVk_determineDeviceExtensions(
    VkPhysicalDevice       physicalDevice,
    uint32_t               availableExtensionCount,
    VkExtensionProperties* pAvailableExtensions,
    uint32_t*              pRequiredExtensionCount,
    char**                 pRequiredExtensions)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    swappy.swappyVkDetermineDeviceExtensions(physicalDevice,
                                             availableExtensionCount, pAvailableExtensions,
                                             pRequiredExtensionCount, pRequiredExtensions);
}

void SwappyVk_setQueueFamilyIndex(
        VkDevice    device,
        VkQueue     queue,
        uint32_t    queueFamilyIndex)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    swappy.SetQueueFamilyIndex(device, queue, queueFamilyIndex);
}

bool SwappyVk_initAndGetRefreshCycleDuration(
        VkPhysicalDevice physicalDevice,
        VkDevice         device,
        VkSwapchainKHR   swapchain,
        uint64_t*        pRefreshDuration)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    return swappy.GetRefreshCycleDuration(physicalDevice, device, swapchain, pRefreshDuration);
}

void SwappyVk_setSwapInterval(
        VkDevice       device,
        VkSwapchainKHR swapchain,
        uint32_t       interval)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    swappy.SetSwapInterval(device, swapchain, interval);
}

VkResult SwappyVk_queuePresent(
        VkQueue                 queue,
        const VkPresentInfoKHR* pPresentInfo)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    return swappy.QueuePresent(queue, pPresentInfo);
}

void SwappyVk_destroySwapchain(
        VkDevice                device,
        VkSwapchainKHR          swapchain)
{
    TRACE_CALL();
    SwappyVk& swappy = SwappyVk::getInstance();
    swappy.DestroySwapchain(device, swapchain);
}

}  // extern "C"
