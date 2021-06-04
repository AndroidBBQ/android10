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

#ifndef ANDROID_WATCHDOG_H
#define ANDROID_WATCHDOG_H

#include <chrono>
#include <time.h>

namespace android {

/*
 * An RAII-style object, which would crash the process if a timeout expires
 * before the object is destroyed.
 * The calling thread would be sent a SIGABORT, which would typically result in
 * a stack trace.
 *
 * Sample usage:
 * {
 *     Watchdog watchdog(std::chrono::milliseconds(10));
 *     DoSomething();
 * }
 * // If we got here, the function completed in time.
 */
class Watchdog final {
public:
    Watchdog(std::chrono::steady_clock::duration timeout);
    ~Watchdog();

private:
    timer_t mTimerId;
};

}  // namespace android

#endif  // ANDROID_WATCHDOG_H
