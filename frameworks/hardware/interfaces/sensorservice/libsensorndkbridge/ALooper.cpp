/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "ALooper.h"

#include "ASensorEventQueue.h"

#define LOG_TAG "libsensorndkbridge"
#include <android/looper.h>
#include <android-base/logging.h>

using android::Mutex;

ALooper::ALooper()
    : mAwoken(false) {
}

void ALooper::signalSensorEvents(ASensorEventQueue *queue) {
    Mutex::Autolock autoLock(mLock);
    mReadyQueues.insert(queue);
    mCondition.signal();
}

void ALooper::wake() {
    Mutex::Autolock autoLock(mLock);
    mAwoken = true;
    mCondition.signal();
}

int ALooper::pollOnce(
        int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    if (outFd) { *outFd = 0; }
    if (outEvents) { *outEvents = 0; }
    if (outData) { *outData = NULL; }

    int64_t waitUntilNs;
    if (timeoutMillis < 0) {
        waitUntilNs = -1;
    } else {
        waitUntilNs = systemTime(SYSTEM_TIME_MONOTONIC) + timeoutMillis * 1000000LL;
    }

    Mutex::Autolock autoLock(mLock);
    int64_t nowNs;
    while ((timeoutMillis < 0
                || (nowNs = systemTime(SYSTEM_TIME_MONOTONIC)) < waitUntilNs)
            && mReadyQueues.empty()
            && !mAwoken) {
        if (timeoutMillis < 0) {
            mCondition.wait(mLock);
        } else {
            mCondition.waitRelative(mLock, waitUntilNs - nowNs);
        }
    }

    int result = ALOOPER_POLL_TIMEOUT;

    if (!mReadyQueues.empty()) {
        result = ALOOPER_POLL_CALLBACK;

        for (auto queue : mReadyQueues) {
            queue->dispatchCallback();
        }

        mReadyQueues.clear();
    } else if (mAwoken) {
        result = ALOOPER_POLL_WAKE;
        mAwoken = false;
    }

    LOG(VERBOSE) << "pollOnce returning " << result;

    return result;
}

void ALooper::invalidateSensorQueue(ASensorEventQueue *queue) {
    Mutex::Autolock autoLock(mLock);
    mReadyQueues.erase(queue);
}

