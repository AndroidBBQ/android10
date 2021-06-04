/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "CallbackProtector.h"
#include "sllog.h"

#include <media/stagefright/foundation/ADebug.h>

//--------------------------------------------------------------------------------------------------
namespace android {


CallbackProtector::CallbackProtector() : RefBase(),
        mSafeToEnterCb(true),
        mCbCount(0)
#ifdef USE_DEBUG
        , mCallbackThread((pthread_t) NULL),
        mCallbackTid(0),
        mRequesterThread((pthread_t) NULL),
        mRequesterTid(0)
#endif
{
}


CallbackProtector::~CallbackProtector() {
    Mutex::Autolock _l(mLock);
    if (mCbCount) {
        SL_LOGE("Callback protector detected an active callback after destroy");
    }

}


// static
bool CallbackProtector::enterCbIfOk(const sp<CallbackProtector> &protector) {
    if (protector != 0) {
        return protector->enterCb();
    } else {
        SL_LOGE("Callback protector is missing");
        return false;
    }
}


bool CallbackProtector::enterCb() {
    Mutex::Autolock _l(mLock);
    if (mSafeToEnterCb) {
        mCbCount++;
#ifdef USE_DEBUG
        if (mCbCount > 1) {
            SL_LOGV("Callback protector allowed multiple or nested callback entry: %u", mCbCount);
        } else {
            mCallbackThread = pthread_self();
            mCallbackTid = gettid();
        }
#endif
    } else {
#ifdef USE_DEBUG
        SL_LOGV("Callback protector denied callback entry by thread %p tid %d during destroy"
                " requested by thread %p tid %d",
                (void *) pthread_self(), gettid(),
                (void *) mRequesterThread, mRequesterTid);
#else
        SL_LOGV("Callback protector denied callback entry during destroy");
#endif
    }
    return mSafeToEnterCb;
}


void CallbackProtector::exitCb() {
    Mutex::Autolock _l(mLock);

    CHECK(mCbCount > 0);
    mCbCount--;

    if (mCbCount == 0) {
        if (!mSafeToEnterCb) {
#ifdef USE_DEBUG
            SL_LOGV("Callback protector detected return from callback by thread %p tid %d during"
                    " destroy requested by thread %p tid %d",
                    (void *) mCallbackThread, mCallbackTid,
                    (void *) mRequesterThread, mRequesterTid);
#else
            SL_LOGV("Callback protector detected return from callback during destroy");
#endif
            mCbExitedCondition.broadcast();
        }
#ifdef USE_DEBUG
        mCallbackThread = (pthread_t) NULL;
        mCallbackTid = 0;
#endif
    }
}


void CallbackProtector::requestCbExitAndWait() {
    Mutex::Autolock _l(mLock);
    mSafeToEnterCb = false;
#ifdef USE_DEBUG
    mRequesterThread = pthread_self();
    mRequesterTid = gettid();
#endif
    while (mCbCount) {
#ifdef USE_DEBUG
        SL_LOGV("Callback protector detected in-progress callback by thread %p tid %d during"
                " blocking destroy requested by thread %p tid %d",
                (void *) mCallbackThread, mCallbackTid,
                (void *) pthread_self(), gettid());
#else
        SL_LOGV("Callback protector detected in-progress callback during blocking destroy");
#endif
        mCbExitedCondition.wait(mLock);
    }
}


void CallbackProtector::requestCbExit() {
    Mutex::Autolock _l(mLock);
    mSafeToEnterCb = false;
#ifdef USE_DEBUG
    mRequesterThread = pthread_self();
    mRequesterTid = gettid();
#endif
    if (mCbCount) {
#ifdef USE_DEBUG
        SL_LOGV("Callback protector detected in-progress callback by thread %p tid %d during"
                " non-blocking destroy requested by thread %p tid %d",
                (void *) mCallbackThread, mCallbackTid,
                (void *) pthread_self(), gettid());
#else
        SL_LOGV("Callback protector detected in-progress callback during non-blocking destroy");
#endif
    }
}

} // namespace android
