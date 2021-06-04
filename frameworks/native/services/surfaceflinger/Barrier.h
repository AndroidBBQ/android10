/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_BARRIER_H
#define ANDROID_BARRIER_H

#include <stdint.h>
#include <condition_variable>
#include <mutex>

namespace android {

class Barrier
{
public:
    // Release any threads waiting at the Barrier.
    // Provides release semantics: preceding loads and stores will be visible
    // to other threads before they wake up.
    void open() {
        std::lock_guard<std::mutex> lock(mMutex);
        mIsOpen = true;
        mCondition.notify_all();
    }

    // Reset the Barrier, so wait() will block until open() has been called.
    void close() {
        std::lock_guard<std::mutex> lock(mMutex);
        mIsOpen = false;
    }

    // Wait until the Barrier is OPEN.
    // Provides acquire semantics: no subsequent loads or stores will occur
    // until wait() returns.
    void wait() const {
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this]() NO_THREAD_SAFETY_ANALYSIS { return mIsOpen; });
    }
private:
    mutable std::mutex mMutex;
    mutable std::condition_variable mCondition;
    int mIsOpen GUARDED_BY(mMutex){false};
};

}; // namespace android

#endif // ANDROID_BARRIER_H
