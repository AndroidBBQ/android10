/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>

#include <cutils/sched_policy.h>
#include <log/log.h>
#include <system/thread_defs.h>

#include "EventControlThread.h"

namespace android {

EventControlThread::~EventControlThread() = default;

namespace impl {

EventControlThread::EventControlThread(EventControlThread::SetVSyncEnabledFunction function)
      : mSetVSyncEnabled(function) {
    pthread_setname_np(mThread.native_handle(), "EventControlThread");

    pid_t tid = pthread_gettid_np(mThread.native_handle());
    setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_DISPLAY);
    set_sched_policy(tid, SP_FOREGROUND);
}

EventControlThread::~EventControlThread() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mKeepRunning = false;
        mCondition.notify_all();
    }
    mThread.join();
}

void EventControlThread::setVsyncEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVsyncEnabled = enabled;
    mCondition.notify_all();
}

// Unfortunately std::unique_lock gives warnings with -Wthread-safety
void EventControlThread::threadMain() NO_THREAD_SAFETY_ANALYSIS {
    auto keepRunning = true;
    auto currentVsyncEnabled = false;

    while (keepRunning) {
        mSetVSyncEnabled(currentVsyncEnabled);

        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this, currentVsyncEnabled, keepRunning]() NO_THREAD_SAFETY_ANALYSIS {
            return currentVsyncEnabled != mVsyncEnabled || keepRunning != mKeepRunning;
        });
        currentVsyncEnabled = mVsyncEnabled;
        keepRunning = mKeepRunning;
    }
}

} // namespace impl
} // namespace android
