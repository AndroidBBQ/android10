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

//#define LOG_NDEBUG 0
#define LOG_TAG "VideoFrameScheduler2"
#include <utils/Log.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Mutex.h>
#include <utils/Thread.h>
#include <utils/Trace.h>

#include <algorithm>
#include <jni.h>
#include <math.h>

#include <android/choreographer.h>
#include <android/looper.h>
#include <media/stagefright/VideoFrameScheduler2.h>
#include <mediaplayer2/JavaVMHelper.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>

namespace android {

static void getVsyncOffset(nsecs_t* appVsyncOffsetPtr, nsecs_t* sfVsyncOffsetPtr);

/* ======================================================================= */
/*                               VsyncTracker                              */
/* ======================================================================= */

class VsyncTracker : public RefBase{
public:
    VsyncTracker();
    ~VsyncTracker() {}
    nsecs_t getVsyncPeriod();
    nsecs_t getVsyncTime(nsecs_t periodOffset);
    void addSample(nsecs_t timestamp);

private:
    static const int kMaxSamples = 32;
    static const int kMinSamplesForUpdate = 6;
    int mNumSamples;
    int mFirstSample;
    nsecs_t mReferenceTime;
    nsecs_t mPhase;
    nsecs_t mPeriod;
    nsecs_t mTimestampSamples[kMaxSamples];
    Mutex mLock;

    void updateModelLocked();
};

VsyncTracker::VsyncTracker()
    : mNumSamples(0),
      mFirstSample(0),
      mReferenceTime(0),
      mPhase(0),
      mPeriod(0) {
    for (int i = 0; i < kMaxSamples; i++) {
        mTimestampSamples[i] = 0;
    }
}

nsecs_t VsyncTracker::getVsyncPeriod() {
    Mutex::Autolock dataLock(mLock);
    return mPeriod;
}

nsecs_t VsyncTracker::getVsyncTime(nsecs_t periodOffset) {
    Mutex::Autolock dataLock(mLock);
    const nsecs_t now = systemTime();
    nsecs_t phase = mReferenceTime + mPhase;

    // result = (((now - phase) / mPeriod) + periodOffset + 1) * mPeriod + phase
    // prevent overflow
    nsecs_t result = (now - phase) / mPeriod;
    if (result > LONG_LONG_MAX - periodOffset - 1) {
        return LONG_LONG_MAX;
    } else {
        result += periodOffset + 1;
    }
    if (result > LONG_LONG_MAX / mPeriod) {
        return LONG_LONG_MAX;
    } else {
        result *= mPeriod;
    }
    if (result > LONG_LONG_MAX - phase) {
        return LONG_LONG_MAX;
    } else {
        result += phase;
    }

    return result;
}

void VsyncTracker::addSample(nsecs_t timestamp) {
    Mutex::Autolock dataLock(mLock);
    if (mNumSamples == 0) {
        mPhase = 0;
        mReferenceTime = timestamp;
    }
    int idx = (mFirstSample + mNumSamples) % kMaxSamples;
    mTimestampSamples[idx] = timestamp;
    if (mNumSamples < kMaxSamples) {
        mNumSamples++;
    } else {
        mFirstSample = (mFirstSample + 1) % kMaxSamples;
    }
    updateModelLocked();
}

void VsyncTracker::updateModelLocked() {
    if (mNumSamples < kMinSamplesForUpdate) {
        return;
    }
    nsecs_t durationSum = 0;
    nsecs_t minDuration = LONG_MAX;
    nsecs_t maxDuration = 0;

    for (int i = 1; i < mNumSamples; i++) {
        int idx = (mFirstSample + i) % kMaxSamples;
        int prev = (idx + kMaxSamples - 1) % kMaxSamples;
        long duration = mTimestampSamples[idx] - mTimestampSamples[prev];
        durationSum += duration;
        if (minDuration > duration) { minDuration = duration; }
        if (maxDuration < duration) { maxDuration = duration; }
    }

    durationSum -= (minDuration + maxDuration);
    mPeriod = durationSum / (mNumSamples - 3);

    double sampleAvgX = 0.0;
    double sampleAvgY = 0.0;
    double scale = 2.0 * M_PI / (double) mPeriod;

    for (int i = 1; i < mNumSamples; i++) {
        int idx = (mFirstSample + i) % kMaxSamples;
        long sample = mTimestampSamples[idx] - mReferenceTime;
        double samplePhase = (double) (sample % mPeriod) * scale;
        sampleAvgX += cos(samplePhase);
        sampleAvgY += sin(samplePhase);
    }

    sampleAvgX /= (double) mNumSamples - 1.0;
    sampleAvgY /= (double) mNumSamples - 1.0;
    mPhase = (long) (atan2(sampleAvgY, sampleAvgX) / scale);
}

static void frameCallback(int64_t frameTimeNanos, void* data) {
    if (data == NULL) {
        return;
    }
    sp<VsyncTracker> vsyncTracker(static_cast<VsyncTracker*>(data));
    vsyncTracker->addSample(frameTimeNanos);
    AChoreographer_postFrameCallback64(AChoreographer_getInstance(),
            frameCallback, static_cast<void*>(vsyncTracker.get()));
}

/* ======================================================================= */
/*                                   JNI                                   */
/* ======================================================================= */

static void getVsyncOffset(nsecs_t* appVsyncOffsetPtr, nsecs_t* sfVsyncOffsetPtr) {
    static const nsecs_t kOneMillisecInNanosec = 1000000;
    static const nsecs_t kOneSecInNanosec = kOneMillisecInNanosec * 1000;

    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jclass jDisplayManagerGlobalCls = env->FindClass(
            "android/hardware/display/DisplayManagerGlobal");
    jclass jDisplayCls = env->FindClass("android/view/Display");

    jmethodID jGetInstance = env->GetStaticMethodID(jDisplayManagerGlobalCls,
            "getInstance", "()Landroid/hardware/display/DisplayManagerGlobal;");
    jobject javaDisplayManagerGlobalObj = env->CallStaticObjectMethod(
            jDisplayManagerGlobalCls, jGetInstance);

    jfieldID jDEFAULT_DISPLAY = env->GetStaticFieldID(jDisplayCls, "DEFAULT_DISPLAY", "I");
    jint DEFAULT_DISPLAY = env->GetStaticIntField(jDisplayCls, jDEFAULT_DISPLAY);

    jmethodID jgetRealDisplay = env->GetMethodID(jDisplayManagerGlobalCls,
            "getRealDisplay", "(I)Landroid/view/Display;");
    jobject javaDisplayObj = env->CallObjectMethod(
            javaDisplayManagerGlobalObj, jgetRealDisplay, DEFAULT_DISPLAY);

    jmethodID jGetRefreshRate = env->GetMethodID(jDisplayCls, "getRefreshRate", "()F");
    jfloat javaRefreshRate = env->CallFloatMethod(javaDisplayObj, jGetRefreshRate);
    nsecs_t vsyncPeriod = (nsecs_t) (kOneSecInNanosec / (float) javaRefreshRate);

    jmethodID jGetAppVsyncOffsetNanos = env->GetMethodID(
            jDisplayCls, "getAppVsyncOffsetNanos", "()J");
    jlong javaAppVsyncOffset = env->CallLongMethod(javaDisplayObj, jGetAppVsyncOffsetNanos);
    *appVsyncOffsetPtr = (nsecs_t) javaAppVsyncOffset;

    jmethodID jGetPresentationDeadlineNanos = env->GetMethodID(
            jDisplayCls, "getPresentationDeadlineNanos", "()J");
    jlong javaPresentationDeadline = env->CallLongMethod(
            javaDisplayObj, jGetPresentationDeadlineNanos);

    *sfVsyncOffsetPtr = vsyncPeriod - ((nsecs_t) javaPresentationDeadline - kOneMillisecInNanosec);
}

/* ======================================================================= */
/*                          Choreographer Thread                           */
/* ======================================================================= */

struct ChoreographerThread : public Thread {
    ChoreographerThread(bool canCallJava);
    status_t init(void* data);
    virtual status_t readyToRun() override;
    virtual bool threadLoop() override;

protected:
    virtual ~ChoreographerThread() {}

private:
    DISALLOW_EVIL_CONSTRUCTORS(ChoreographerThread);
    void* mData;
};

ChoreographerThread::ChoreographerThread(bool canCallJava) : Thread(canCallJava) {
}

status_t ChoreographerThread::init(void* data) {
    if (data == NULL) {
        return NO_INIT;
    }
    mData = data;
    return OK;
}

status_t ChoreographerThread::readyToRun() {
    ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (AChoreographer_getInstance() == NULL) {
        return NO_INIT;
    }
    AChoreographer_postFrameCallback64(AChoreographer_getInstance(), frameCallback, mData);
    return OK;
}

bool ChoreographerThread::threadLoop() {
    ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
    return true;
}

/* ======================================================================= */
/*                             Frame Scheduler                             */
/* ======================================================================= */

VideoFrameScheduler2::VideoFrameScheduler2() : VideoFrameSchedulerBase() {

    getVsyncOffset(&mAppVsyncOffset, &mSfVsyncOffset);

    Mutex::Autolock threadLock(mLock);
    mChoreographerThread = new ChoreographerThread(true);

    mVsyncTracker = new VsyncTracker();
    if (mChoreographerThread->init(static_cast<void*>(mVsyncTracker.get())) != OK) {
        mChoreographerThread.clear();
    }
    if (mChoreographerThread != NULL && mChoreographerThread->run("Choreographer") != OK) {
        mChoreographerThread.clear();
    }
}

void VideoFrameScheduler2::updateVsync() {
    mVsyncTime = 0;
    mVsyncPeriod = 0;

    if (mVsyncTracker != NULL) {
        mVsyncPeriod = mVsyncTracker->getVsyncPeriod();
        mVsyncTime = mVsyncTracker->getVsyncTime(mSfVsyncOffset - mAppVsyncOffset);
    }
    mVsyncRefreshAt = systemTime(SYSTEM_TIME_MONOTONIC) + kVsyncRefreshPeriod;
}

void VideoFrameScheduler2::release() {
    // Do not change order
    {
        Mutex::Autolock threadLock(mLock);
        mChoreographerThread->requestExitAndWait();
        mChoreographerThread.clear();
    }

    mVsyncTracker.clear();
}

VideoFrameScheduler2::~VideoFrameScheduler2() {
    release();
}

} // namespace android
