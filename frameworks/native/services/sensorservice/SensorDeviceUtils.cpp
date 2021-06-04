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

#include "SensorDeviceUtils.h"

#include <android/hardware/sensors/1.0/ISensors.h>
#include <utils/Log.h>

#include <chrono>
#include <thread>

using ::android::hardware::Void;
using namespace android::hardware::sensors::V1_0;

namespace android {
namespace SensorDeviceUtils {

HidlServiceRegistrationWaiter::HidlServiceRegistrationWaiter() {
}

void HidlServiceRegistrationWaiter::onFirstRef() {
    // Creating sp<...>(this) in the constructor should be avoided, hence
    // registerForNotifications is called in onFirstRef callback.
    mRegistered = ISensors::registerForNotifications("default", this);
}

Return<void> HidlServiceRegistrationWaiter::onRegistration(
        const hidl_string &fqName, const hidl_string &name, bool preexisting) {
    ALOGV("onRegistration fqName %s, name %s, preexisting %d",
          fqName.c_str(), name.c_str(), preexisting);

    {
        std::lock_guard<std::mutex> lk(mLock);
        mRestartObserved = true;
    }
    mCondition.notify_all();
    return Void();
}

void HidlServiceRegistrationWaiter::reset() {
    std::lock_guard<std::mutex> lk(mLock);
    mRestartObserved = false;
}

bool HidlServiceRegistrationWaiter::wait() {
    constexpr int DEFAULT_WAIT_MS = 100;
    constexpr int TIMEOUT_MS = 1000;

    if (!mRegistered) {
        ALOGW("Cannot register service notification, use default wait(%d ms)", DEFAULT_WAIT_MS);
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_WAIT_MS));
        // not sure if service is actually restarted
        return false;
    }

    std::unique_lock<std::mutex> lk(mLock);
    return mCondition.wait_for(lk, std::chrono::milliseconds(TIMEOUT_MS),
            [this]{return mRestartObserved;});
}

} // namespace SensorDeviceUtils
} // namespace android
