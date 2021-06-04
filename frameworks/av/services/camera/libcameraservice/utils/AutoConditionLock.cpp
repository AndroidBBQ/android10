/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "AutoConditionLock.h"

namespace android {

WaitableMutexWrapper::WaitableMutexWrapper(Mutex* mutex) : mMutex{mutex}, mState{false} {}

WaitableMutexWrapper::~WaitableMutexWrapper() {}

// Locks manager-owned mutex
AutoConditionLock::AutoConditionLock(const std::shared_ptr<WaitableMutexWrapper>& manager) :
        mManager{manager}, mAutoLock{manager->mMutex}, mAcquired(false) {}

// Unlocks manager-owned mutex
AutoConditionLock::~AutoConditionLock() {
    // Unset the condition and wake everyone up before releasing lock
    if (mAcquired) {
        mManager->mState = false;
        mManager->mCondition.broadcast();
    }
}

std::unique_ptr<AutoConditionLock> AutoConditionLock::waitAndAcquire(
        const std::shared_ptr<WaitableMutexWrapper>& manager, nsecs_t waitTime) {

    if (manager == nullptr || manager->mMutex == nullptr) {
        // Bad input, return null
        return std::unique_ptr<AutoConditionLock>{nullptr};
    }

    // Acquire scoped lock
    std::unique_ptr<AutoConditionLock> scopedLock(new AutoConditionLock(manager));

    // Figure out what time in the future we should hit the timeout
    nsecs_t failTime = systemTime(SYSTEM_TIME_MONOTONIC) + waitTime;

    // Wait until we timeout, or success
    while(manager->mState) {
        status_t ret = manager->mCondition.waitRelative(*(manager->mMutex), waitTime);
        if (ret != NO_ERROR) {
            // Timed out or whatever, return null
            return std::unique_ptr<AutoConditionLock>{nullptr};
        }
        waitTime = failTime - systemTime(SYSTEM_TIME_MONOTONIC);
    }

    // Set the condition and return
    manager->mState = true;
    scopedLock->mAcquired = true;
    return scopedLock;
}

std::unique_ptr<AutoConditionLock> AutoConditionLock::waitAndAcquire(
        const std::shared_ptr<WaitableMutexWrapper>& manager) {

    if (manager == nullptr || manager->mMutex == nullptr) {
        // Bad input, return null
        return std::unique_ptr<AutoConditionLock>{nullptr};
    }

    // Acquire scoped lock
    std::unique_ptr<AutoConditionLock> scopedLock(new AutoConditionLock(manager));

    // Wait until we timeout, or success
    while(manager->mState) {
        status_t ret = manager->mCondition.wait(*(manager->mMutex));
        if (ret != NO_ERROR) {
            // Timed out or whatever, return null
            return std::unique_ptr<AutoConditionLock>{nullptr};
        }
    }

    // Set the condition and return
    manager->mState = true;
    scopedLock->mAcquired = true;
    return scopedLock;
}

}; // namespace android
