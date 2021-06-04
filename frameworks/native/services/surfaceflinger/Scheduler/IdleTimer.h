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

#pragma once

#include <chrono>
#include <condition_variable>
#include <thread>

#include <android-base/thread_annotations.h>

namespace android {
namespace scheduler {

/*
 * Class that sets off a timer for a given interval, and fires a callback when the
 * interval expires.
 */
class IdleTimer {
public:
    using Interval = std::chrono::milliseconds;
    using ResetCallback = std::function<void()>;
    using TimeoutCallback = std::function<void()>;

    IdleTimer(const Interval& interval, const ResetCallback& resetCallback,
              const TimeoutCallback& timeoutCallback);
    ~IdleTimer();

    // Initializes and turns on the idle timer.
    void start();
    // Stops the idle timer and any held resources.
    void stop();
    // Resets the wakeup time and fires the reset callback.
    void reset();

private:
    // Enum to track in what state is the timer.
    enum class TimerState {
        // The internal timer thread has been destroyed, and no state is
        // tracked.
        // Possible state transitions: RESET
        STOPPED = 0,
        // An external thread has just reset this timer.
        // If there is a reset callback, then that callback is fired.
        // Possible state transitions: STOPPED, WAITING
        RESET = 1,
        // This timer is waiting for the timeout interval to expire.
        // Possible state transaitions: STOPPED, RESET, IDLE
        WAITING = 2,
        // The timeout interval has expired, so we are sleeping now.
        // Possible state transaitions: STOPPED, RESET
        IDLE = 3
    };

    // Function that loops until the condition for stopping is met.
    void loop();

    // Thread waiting for timer to expire.
    std::thread mThread;

    // Condition used to notify mThread.
    std::condition_variable_any mCondition;

    // Lock used for synchronizing the waiting thread with the application thread.
    std::mutex mMutex;

    // Current timer state
    TimerState mState GUARDED_BY(mMutex) = TimerState::RESET;

    // Interval after which timer expires.
    const Interval mInterval;

    // Callback that happens when timer resets.
    const ResetCallback mResetCallback;

    // Callback that happens when timer expires.
    const TimeoutCallback mTimeoutCallback;
};

} // namespace scheduler
} // namespace android
