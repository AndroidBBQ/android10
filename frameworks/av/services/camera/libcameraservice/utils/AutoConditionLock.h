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


#ifndef ANDROID_SERVICE_UTILS_SCOPED_CONDITION_H
#define ANDROID_SERVICE_UTILS_SCOPED_CONDITION_H

#include <utils/Timers.h>
#include <utils/Condition.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>

#include <memory>

namespace android {

/**
 * WaitableMutexWrapper can be used with AutoConditionLock to construct scoped locks for the
 * wrapped Mutex with timeouts for lock acquisition.
 */
class WaitableMutexWrapper {
    friend class AutoConditionLock;
public:
    /**
     * Construct the ConditionManger with the given Mutex.
     */
    explicit WaitableMutexWrapper(Mutex* mutex);

    virtual ~WaitableMutexWrapper();
private:
    Mutex* mMutex;
    bool mState;
    Condition mCondition;
};

/**
 * AutoConditionLock is a scoped lock similar to Mutex::Autolock, but allows timeouts to be
 * specified for lock acquisition.
 *
 * AutoConditionLock is used with a WaitableMutexWrapper to lock/unlock the WaitableMutexWrapper's
 * wrapped Mutex, and wait/set/signal the WaitableMutexWrapper's wrapped condition. To use this,
 * call AutoConditionLock::waitAndAcquire to get an instance.  This will:
 *      - Lock the given WaitableMutexWrapper's mutex.
 *      - Wait for the WaitableMutexWrapper's condition to become false, or timeout.
 *      - Set the WaitableMutexWrapper's condition to true.
 *
 * When the AutoConditionLock goes out of scope and is destroyed, this will:
 *      - Set the WaitableMutexWrapper's condition to false.
 *      - Signal threads waiting on this condition to wakeup.
 *      - Release WaitableMutexWrapper's mutex.
 */
class AutoConditionLock final {
public:
    AutoConditionLock() = delete;
    AutoConditionLock(const AutoConditionLock& other) = delete;
    AutoConditionLock & operator=(const AutoConditionLock&) = delete;

    ~AutoConditionLock();

    /**
     * Make a new AutoConditionLock from a given WaitableMutexWrapper, waiting up to waitTime
     * nanoseconds to acquire the WaitableMutexWrapper's wrapped lock.
     *
     * Return an empty unique_ptr if this fails, or a timeout occurs.
     */
    static std::unique_ptr<AutoConditionLock> waitAndAcquire(
            const std::shared_ptr<WaitableMutexWrapper>& manager, nsecs_t waitTime);

    /**
     * Make a new AutoConditionLock from a given WaitableMutexWrapper, waiting indefinitely to
     * acquire the WaitableMutexWrapper's wrapped lock.
     *
     * Return an empty unique_ptr if this fails.
     */
    static std::unique_ptr<AutoConditionLock> waitAndAcquire(
            const std::shared_ptr<WaitableMutexWrapper>& manager);
private:
    explicit AutoConditionLock(const std::shared_ptr<WaitableMutexWrapper>& manager);

    std::shared_ptr<WaitableMutexWrapper> mManager;
    Mutex::Autolock mAutoLock;
    bool mAcquired;
};

}; // namespace android

#endif // ANDROID_SERVICE_UTILS_SCOPED_CONDITION_H
