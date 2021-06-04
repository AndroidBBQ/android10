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

#include "Swappy.h"

#define LOG_TAG "Swappy"

#include <cmath>
#include <thread>
#include <cstdlib>
#include <cinttypes>

#include "Settings.h"
#include "Thread.h"
#include "ChoreographerFilter.h"
#include "ChoreographerThread.h"
#include "EGL.h"
#include "FrameStatistics.h"
#include "SystemProperties.h"

// uncomment below line to enable ALOGV messages
//#define SWAPPY_DEBUG

#include "Log.h"
#include "Trace.h"

namespace swappy {

using std::chrono::milliseconds;
using std::chrono::nanoseconds;

std::mutex Swappy::sInstanceMutex;
std::unique_ptr<Swappy> Swappy::sInstance;

// NB These are only needed for C++14
constexpr std::chrono::nanoseconds Swappy::FrameDuration::MAX_DURATION;
constexpr std::chrono::nanoseconds Swappy::FRAME_HYSTERESIS;

void Swappy::init(JNIEnv *env, jobject jactivity) {
    jclass activityClass = env->FindClass("android/app/NativeActivity");
    jclass windowManagerClass = env->FindClass("android/view/WindowManager");
    jclass displayClass = env->FindClass("android/view/Display");

    jmethodID getWindowManager = env->GetMethodID(
            activityClass,
            "getWindowManager",
            "()Landroid/view/WindowManager;");

    jmethodID getDefaultDisplay = env->GetMethodID(
            windowManagerClass,
            "getDefaultDisplay",
            "()Landroid/view/Display;");

    jobject wm = env->CallObjectMethod(jactivity, getWindowManager);
    jobject display = env->CallObjectMethod(wm, getDefaultDisplay);

    jmethodID getRefreshRate = env->GetMethodID(
            displayClass,
            "getRefreshRate",
            "()F");

    const float refreshRateHz = env->CallFloatMethod(display, getRefreshRate);

    jmethodID getAppVsyncOffsetNanos = env->GetMethodID(
            displayClass,
            "getAppVsyncOffsetNanos", "()J");

    // getAppVsyncOffsetNanos was only added in API 21.
    // Return gracefully if this device doesn't support it.
    if (getAppVsyncOffsetNanos == 0 || env->ExceptionOccurred()) {
        env->ExceptionClear();
        return;
    }
    const long appVsyncOffsetNanos = env->CallLongMethod(display, getAppVsyncOffsetNanos);

    jmethodID getPresentationDeadlineNanos = env->GetMethodID(
            displayClass,
            "getPresentationDeadlineNanos",
            "()J");

    const long vsyncPresentationDeadlineNanos = env->CallLongMethod(
            display,
            getPresentationDeadlineNanos);

    const long ONE_MS_IN_NS = 1000000;
    const long ONE_S_IN_NS = ONE_MS_IN_NS * 1000;

    const long vsyncPeriodNanos = static_cast<long>(ONE_S_IN_NS / refreshRateHz);
    const long sfVsyncOffsetNanos =
            vsyncPeriodNanos - (vsyncPresentationDeadlineNanos - ONE_MS_IN_NS);

    using std::chrono::nanoseconds;
    JavaVM *vm;
    env->GetJavaVM(&vm);
    Swappy::init(
            vm,
            nanoseconds(vsyncPeriodNanos),
            nanoseconds(appVsyncOffsetNanos),
            nanoseconds(sfVsyncOffsetNanos));
}

void Swappy::init(JavaVM *vm, nanoseconds refreshPeriod, nanoseconds appOffset, nanoseconds sfOffset) {
    std::lock_guard<std::mutex> lock(sInstanceMutex);
    if (sInstance) {
        ALOGE("Attempted to initialize Swappy twice");
        return;
    }
    sInstance = std::make_unique<Swappy>(vm, refreshPeriod, appOffset, sfOffset, ConstructorTag{});
}

void Swappy::onChoreographer(int64_t frameTimeNanos) {
    TRACE_CALL();

    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in swap");
        return;
    }

    if (!swappy->mUsingExternalChoreographer) {
        swappy->mUsingExternalChoreographer = true;
        swappy->mChoreographerThread =
                ChoreographerThread::createChoreographerThread(
                        ChoreographerThread::Type::App,
                        nullptr,
                        [swappy] { swappy->handleChoreographer(); });
    }

    swappy->mChoreographerThread->postFrameCallbacks();
}

bool Swappy::swap(EGLDisplay display, EGLSurface surface) {
    TRACE_CALL();

    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in swap");
        return EGL_FALSE;
    }

    if (swappy->enabled()) {
        return swappy->swapInternal(display, surface);
    } else {
        return eglSwapBuffers(display, surface) == EGL_TRUE;
    }
}

bool Swappy::swapInternal(EGLDisplay display, EGLSurface surface) {
    if (!mUsingExternalChoreographer) {
        mChoreographerThread->postFrameCallbacks();
    }

    // for non pipeline mode where both cpu and gpu work is done at the same stage
    // wait for next frame will happen after swap
    const bool needToSetPresentationTime = mPipelineMode ?
            waitForNextFrame(display) :
            (mAutoSwapInterval <= mAutoSwapIntervalThreshold);

    mSwapTime = std::chrono::steady_clock::now();

    if (needToSetPresentationTime) {
        bool setPresentationTimeResult = setPresentationTime(display, surface);
        if (!setPresentationTimeResult) {
            return setPresentationTimeResult;
        }
    }

    resetSyncFence(display);

    preSwapBuffersCallbacks();

    bool swapBuffersResult = (eglSwapBuffers(display, surface) == EGL_TRUE);

    postSwapBuffersCallbacks();

    if (updateSwapInterval()) {
        swapIntervalChangedCallbacks();
    }

    updateSwapDuration(std::chrono::steady_clock::now() - mSwapTime);

    if (!mPipelineMode) {
        waitForNextFrame(display);
    }

    startFrame();

    return swapBuffersResult;
}

void Swappy::addTracer(const SwappyTracer *tracer) {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in addTracer");
        return;
    }
    swappy->addTracerCallbacks(*tracer);
}

uint64_t Swappy::getSwapIntervalNS() {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in getSwapIntervalNS");
        return -1;
    }

    std::lock_guard<std::mutex> lock(swappy->mFrameDurationsMutex);
    return swappy->mAutoSwapInterval.load() * swappy->mRefreshPeriod.count();
};

void Swappy::setAutoSwapInterval(bool enabled) {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in setAutoSwapInterval");
        return;
    }

    std::lock_guard<std::mutex> lock(swappy->mFrameDurationsMutex);
    swappy->mAutoSwapIntervalEnabled = enabled;

    // non pipeline mode is not supported when auto mode is disabled
    if (!enabled) {
        swappy->mPipelineMode = true;
    }
}

void Swappy::setAutoPipelineMode(bool enabled) {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in setAutoPipelineMode");
        return;
    }

    std::lock_guard<std::mutex> lock(swappy->mFrameDurationsMutex);
    swappy->mPipelineModeAutoMode = enabled;
    if (!enabled) {
        swappy->mPipelineMode = true;
    }
}

void Swappy::enableStats(bool enabled) {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in enableStats");
            return;
    }

    if (!swappy->enabled()) {
        return;
    }

    if (!swappy->getEgl()->statsSupported()) {
        ALOGI("stats are not suppored on this platform");
        return;
    }

    if (enabled && swappy->mFrameStatistics == nullptr) {
        swappy->mFrameStatistics = std::make_unique<FrameStatistics>(
                swappy->mEgl, swappy->mRefreshPeriod);
        ALOGI("Enabling stats");
    } else {
        swappy->mFrameStatistics = nullptr;
        ALOGI("Disabling stats");
    }
}

void Swappy::recordFrameStart(EGLDisplay display, EGLSurface surface) {
    TRACE_CALL();
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in recordFrameStart");
        return;
    }

    if (swappy->mFrameStatistics)
        swappy->mFrameStatistics->capture(display, surface);
}

void Swappy::getStats(Swappy_Stats *stats) {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in getStats");
        return;
    }

    if (swappy->mFrameStatistics)
        *stats = swappy->mFrameStatistics->getStats();
}

Swappy *Swappy::getInstance() {
    std::lock_guard<std::mutex> lock(sInstanceMutex);
    return sInstance.get();
}

bool Swappy::isEnabled() {
    Swappy *swappy = getInstance();
    if (!swappy) {
        ALOGE("Failed to get Swappy instance in getStats");
        return false;
    }
    return swappy->enabled();
}

void Swappy::destroyInstance() {
    std::lock_guard<std::mutex> lock(sInstanceMutex);
    sInstance.reset();
}

template<typename Tracers, typename Func> void addToTracers(Tracers& tracers, Func func, void *userData) {
    if (func != nullptr) {
        tracers.push_back([func, userData](auto... params) {
            func(userData, params...);
        });
    }
}

void Swappy::addTracerCallbacks(SwappyTracer tracer) {
    addToTracers(mInjectedTracers.preWait, tracer.preWait, tracer.userData);
    addToTracers(mInjectedTracers.postWait, tracer.postWait, tracer.userData);
    addToTracers(mInjectedTracers.preSwapBuffers, tracer.preSwapBuffers, tracer.userData);
    addToTracers(mInjectedTracers.postSwapBuffers, tracer.postSwapBuffers, tracer.userData);
    addToTracers(mInjectedTracers.startFrame, tracer.startFrame, tracer.userData);
    addToTracers(mInjectedTracers.swapIntervalChanged, tracer.swapIntervalChanged, tracer.userData);
}

template<typename T, typename ...Args> void executeTracers(T& tracers, Args... args) {
    for (const auto& tracer : tracers) {
        tracer(std::forward<Args>(args)...);
    }
}

void Swappy::preSwapBuffersCallbacks() {
    executeTracers(mInjectedTracers.preSwapBuffers);
}

void Swappy::postSwapBuffersCallbacks() {
    executeTracers(mInjectedTracers.postSwapBuffers,
                   (long) mPresentationTime.time_since_epoch().count());
}

void Swappy::preWaitCallbacks() {
    executeTracers(mInjectedTracers.preWait);
}

void Swappy::postWaitCallbacks() {
    executeTracers(mInjectedTracers.postWait);
}

void Swappy::startFrameCallbacks() {
    executeTracers(mInjectedTracers.startFrame,
                   mCurrentFrame,
                   (long) mCurrentFrameTimestamp.time_since_epoch().count());
}

void Swappy::swapIntervalChangedCallbacks() {
    executeTracers(mInjectedTracers.swapIntervalChanged);
}

EGL *Swappy::getEgl() {
    static thread_local EGL *egl = nullptr;
    if (!egl) {
        std::lock_guard<std::mutex> lock(mEglMutex);
        egl = mEgl.get();
    }
    return egl;
}

Swappy::Swappy(JavaVM *vm,
               nanoseconds refreshPeriod,
               nanoseconds appOffset,
               nanoseconds sfOffset,
               ConstructorTag /*tag*/)
    : mRefreshPeriod(refreshPeriod),
      mFrameStatistics(nullptr),
      mSfOffset(sfOffset),
      mSwapDuration(std::chrono::nanoseconds(0)),
      mSwapInterval(1),
      mAutoSwapInterval(1)
{
    mDisableSwappy = getSystemPropViaGetAsBool("swappy.disable", false);
    if (!enabled()) {
        ALOGI("Swappy is disabled");
        return;
    }

    std::lock_guard<std::mutex> lock(mEglMutex);
    mEgl = EGL::create(refreshPeriod);
    if (!mEgl) {
        ALOGE("Failed to load EGL functions");
        mDisableSwappy = true;
        return;
    }
    mChoreographerFilter = std::make_unique<ChoreographerFilter>(refreshPeriod,
                                                                 sfOffset - appOffset,
                                                                 [this]() { return wakeClient(); });

    mChoreographerThread = ChoreographerThread::createChoreographerThread(
        ChoreographerThread::Type::Swappy,
        vm,
        [this]{ handleChoreographer(); });
    Settings::getInstance()->addListener([this]() { onSettingsChanged(); });
    mAutoSwapIntervalThreshold = (1e9f / mRefreshPeriod.count()) / 20; // 20FPS
    mFrameDurations.reserve(mFrameDurationSamples);
    ALOGI("Initialized Swappy with refreshPeriod=%lld, appOffset=%lld, sfOffset=%lld" ,
          (long long)refreshPeriod.count(), (long long)appOffset.count(),
          (long long)sfOffset.count());
}

void Swappy::onSettingsChanged() {
    std::lock_guard<std::mutex> lock(mFrameDurationsMutex);
    int32_t newSwapInterval = ::round(float(Settings::getInstance()->getSwapIntervalNS()) /
                               float(mRefreshPeriod.count()));
    if (mSwapInterval != newSwapInterval || mAutoSwapInterval != newSwapInterval) {
        mSwapInterval = newSwapInterval;
        mAutoSwapInterval = mSwapInterval.load();
        mFrameDurations.clear();
        mFrameDurationsSum = {};
    }
}

void Swappy::handleChoreographer() {
    mChoreographerFilter->onChoreographer();
}

std::chrono::nanoseconds Swappy::wakeClient() {
    std::lock_guard<std::mutex> lock(mWaitingMutex);
    ++mCurrentFrame;

    // We're attempting to align with SurfaceFlinger's vsync, but it's always better to be a little
    // late than a little early (since a little early could cause our frame to be picked up
    // prematurely), so we pad by an additional millisecond.
    mCurrentFrameTimestamp = std::chrono::steady_clock::now() + mSwapDuration.load() + 1ms;
    mWaitingCondition.notify_all();
    return mSwapDuration;
}

void Swappy::startFrame() {
    TRACE_CALL();

    int32_t currentFrame;
    std::chrono::steady_clock::time_point currentFrameTimestamp;
    {
        std::unique_lock<std::mutex> lock(mWaitingMutex);
        currentFrame = mCurrentFrame;
        currentFrameTimestamp = mCurrentFrameTimestamp;
    }

    startFrameCallbacks();

    mTargetFrame = currentFrame + mAutoSwapInterval;

    const int intervals = (mPipelineMode) ? 2 : 1;

    // We compute the target time as now
    //   + the time the buffer will be on the GPU and in the queue to the compositor (1 swap period)
    mPresentationTime = currentFrameTimestamp + (mAutoSwapInterval * intervals) * mRefreshPeriod;

    mStartFrameTime = std::chrono::steady_clock::now();
}

void Swappy::waitUntil(int32_t frameNumber) {
    TRACE_CALL();
    std::unique_lock<std::mutex> lock(mWaitingMutex);
    mWaitingCondition.wait(lock, [&]() { return mCurrentFrame >= frameNumber; });
}

void Swappy::waitOneFrame() {
    TRACE_CALL();
    std::unique_lock<std::mutex> lock(mWaitingMutex);
    const int32_t target = mCurrentFrame + 1;
    mWaitingCondition.wait(lock, [&]() { return mCurrentFrame >= target; });
}

void Swappy::addFrameDuration(FrameDuration duration) {
    ALOGV("cpuTime = %.2f", duration.getCpuTime().count() / 1e6f);
    ALOGV("gpuTime = %.2f", duration.getGpuTime().count() / 1e6f);

    std::lock_guard<std::mutex> lock(mFrameDurationsMutex);
    // keep a sliding window of mFrameDurationSamples
    if (mFrameDurations.size() == mFrameDurationSamples) {
        mFrameDurationsSum -= mFrameDurations.front();
        mFrameDurations.erase(mFrameDurations.begin());
    }

    mFrameDurations.push_back(duration);
    mFrameDurationsSum += duration;
}

int32_t Swappy::nanoToSwapInterval(std::chrono::nanoseconds nano) {
    int32_t interval = nano / mRefreshPeriod;

    // round the number based on the nearest
    if (nano.count() - (interval * mRefreshPeriod.count()) > mRefreshPeriod.count() / 2) {
        return interval + 1;
    } else {
        return interval;
    }
}

bool Swappy::waitForNextFrame(EGLDisplay display) {
    preWaitCallbacks();

    int lateFrames = 0;
    bool needToSetPresentationTime;

    std::chrono::nanoseconds cpuTime = std::chrono::steady_clock::now() - mStartFrameTime;
    std::chrono::nanoseconds gpuTime;

    // if we are running slower than the threshold there is no point to sleep, just let the
    // app run as fast as it can
    if (mAutoSwapInterval <= mAutoSwapIntervalThreshold) {
        waitUntil(mTargetFrame);

        // wait for the previous frame to be rendered
        while (!getEgl()->lastFrameIsComplete(display)) {
            gamesdk::ScopedTrace trace("lastFrameIncomplete");
            ALOGV("lastFrameIncomplete");
            lateFrames++;
            waitOneFrame();
        }

        gpuTime = getEgl()->getFencePendingTime();

        mPresentationTime += lateFrames * mRefreshPeriod;
        needToSetPresentationTime = true;

    } else {
        needToSetPresentationTime = false;
        gpuTime = getEgl()->getFencePendingTime();

    }
    addFrameDuration({cpuTime, gpuTime});

    postWaitCallbacks();
    return needToSetPresentationTime;
}

void Swappy::swapSlower(const FrameDuration& averageFrameTime,
                        const std::chrono::nanoseconds& upperBound,
                        const std::chrono::nanoseconds& lowerBound,
                        const int32_t& newSwapInterval) {
    ALOGV("Rendering takes too much time for the given config");

    if (!mPipelineMode && averageFrameTime.getTime(true) <= upperBound) {
        ALOGV("turning on pipelining");
        mPipelineMode = true;
    } else {
        mAutoSwapInterval = newSwapInterval;
        ALOGV("Changing Swap interval to %d", mAutoSwapInterval.load());

        // since we changed the swap interval, we may be able to turn off pipeline mode
        nanoseconds newBound = mRefreshPeriod * mAutoSwapInterval.load();
        newBound -= (FRAME_HYSTERESIS * 2);
        if (mPipelineModeAutoMode && averageFrameTime.getTime(false) < newBound) {
            ALOGV("Turning off pipelining");
            mPipelineMode = false;
        } else {
            ALOGV("Turning on pipelining");
            mPipelineMode = true;
        }
    }
}

void Swappy::swapFaster(const FrameDuration& averageFrameTime,
                        const std::chrono::nanoseconds& upperBound,
                        const std::chrono::nanoseconds& lowerBound,
                        const int32_t& newSwapInterval) {
    ALOGV("Rendering is much shorter for the given config");
    mAutoSwapInterval = newSwapInterval;
    ALOGV("Changing Swap interval to %d", mAutoSwapInterval.load());

    // since we changed the swap interval, we may need to turn on pipeline mode
    nanoseconds newBound = mRefreshPeriod * mAutoSwapInterval.load();
    newBound -= FRAME_HYSTERESIS;
    if (!mPipelineModeAutoMode || averageFrameTime.getTime(false) > newBound) {
        ALOGV("Turning on pipelining");
        mPipelineMode = true;
    } else {
        ALOGV("Turning off pipelining");
        mPipelineMode = false;
    }
}

bool Swappy::updateSwapInterval() {
    std::lock_guard<std::mutex> lock(mFrameDurationsMutex);
    if (!mAutoSwapIntervalEnabled)
        return false;

    if (mFrameDurations.size() < mFrameDurationSamples)
        return false;

    const auto averageFrameTime = mFrameDurationsSum / mFrameDurations.size();
    // define lower and upper bound based on the swap duration
    nanoseconds upperBound = mRefreshPeriod * mAutoSwapInterval.load();
    nanoseconds lowerBound = mRefreshPeriod * (mAutoSwapInterval - 1);

    // to be on the conservative side, lower bounds by FRAME_HYSTERESIS
    upperBound -= FRAME_HYSTERESIS;
    lowerBound -= FRAME_HYSTERESIS;

    // add the hysteresis to one of the bounds to avoid going back and forth when frames
    // are exactly at the edge.
    lowerBound -= FRAME_HYSTERESIS;

    auto div_result = ::div((averageFrameTime.getTime(true) + FRAME_HYSTERESIS).count(),
                               mRefreshPeriod.count());
    auto framesPerRefresh = div_result.quot;
    auto framesPerRefreshRemainder = div_result.rem;

    const int32_t newSwapInterval = framesPerRefresh + (framesPerRefreshRemainder ? 1 : 0);

    ALOGV("mPipelineMode = %d", mPipelineMode);
    ALOGV("Average cpu frame time = %.2f", (averageFrameTime.getCpuTime().count()) / 1e6f);
    ALOGV("Average gpu frame time = %.2f", (averageFrameTime.getGpuTime().count()) / 1e6f);
    ALOGV("upperBound = %.2f", upperBound.count() / 1e6f);
    ALOGV("lowerBound = %.2f", lowerBound.count() / 1e6f);

    bool configChanged = false;
    if (averageFrameTime.getTime(mPipelineMode) > upperBound) {
        swapSlower(averageFrameTime, upperBound, lowerBound, newSwapInterval);
        configChanged = true;
    } else if (mSwapInterval < mAutoSwapInterval &&
                                            (averageFrameTime.getTime(true) < lowerBound)) {
        swapFaster(averageFrameTime, upperBound, lowerBound, newSwapInterval);
        configChanged = true;
    } else if (mPipelineModeAutoMode && mPipelineMode &&
                            averageFrameTime.getTime(false) < upperBound - FRAME_HYSTERESIS) {
        ALOGV("Rendering time fits the current swap interval without pipelining");
        mPipelineMode = false;
        configChanged = true;
    }

    if (configChanged) {
        mFrameDurationsSum = {};
        mFrameDurations.clear();
    }
    return configChanged;
}

void Swappy::resetSyncFence(EGLDisplay display) {
    getEgl()->resetSyncFence(display);
}

bool Swappy::setPresentationTime(EGLDisplay display, EGLSurface surface) {
    TRACE_CALL();

    // if we are too close to the vsync, there is no need to set presentation time
    if ((mPresentationTime - std::chrono::steady_clock::now()) <
            (mRefreshPeriod - mSfOffset)) {
        return EGL_TRUE;
    }

    return getEgl()->setPresentationTime(display, surface, mPresentationTime);
}

void Swappy::updateSwapDuration(std::chrono::nanoseconds duration) {
    // TODO: The exponential smoothing factor here is arbitrary
    mSwapDuration = (mSwapDuration.load() * 4 / 5) + duration / 5;

    // Clamp the swap duration to half the refresh period
    //
    // We do this since the swap duration can be a bit noisy during periods such as app startup,
    // which can cause some stuttering as the smoothing catches up with the actual duration. By
    // clamping, we reduce the maximum error which reduces the calibration time.
    if (mSwapDuration.load() > (mRefreshPeriod / 2)) mSwapDuration = mRefreshPeriod / 2;
}

} // namespace swappy
