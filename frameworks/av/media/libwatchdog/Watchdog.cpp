/*
 * Copyright (C) 2020 The Android Open Source Project
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

#define LOG_TAG "Watchdog"

#include <watchdog/Watchdog.h>

#include <android-base/logging.h>
#include <android-base/threads.h>
#include <signal.h>
#include <time.h>
#include <cstring>
#include <utils/Log.h>

namespace android {

Watchdog::Watchdog(::std::chrono::steady_clock::duration timeout) {
    // Create the timer.
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD_ID;
    sev.sigev_notify_thread_id = base::GetThreadId();
    sev.sigev_signo = SIGABRT;
    sev.sigev_value.sival_ptr = &mTimerId;
    int err = timer_create(CLOCK_MONOTONIC, &sev, &mTimerId);
    if (err != 0) {
        PLOG(FATAL) << "Failed to create timer";
    }

    // Start the timer.
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout);
    LOG_ALWAYS_FATAL_IF(timeout.count() <= 0, "Duration must be positive");
    spec.it_value.tv_sec = ns.count() / 1000000000;
    spec.it_value.tv_nsec = ns.count() % 1000000000;
    err = timer_settime(mTimerId, 0, &spec, nullptr);
    if (err != 0) {
        PLOG(FATAL) << "Failed to start timer";
    }
}

Watchdog::~Watchdog() {
    // Delete the timer.
    int err = timer_delete(mTimerId);
    if (err != 0) {
        PLOG(FATAL) << "Failed to delete timer";
    }
}

}  // namespace android
