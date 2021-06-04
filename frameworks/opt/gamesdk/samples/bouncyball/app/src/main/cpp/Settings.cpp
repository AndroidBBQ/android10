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

#include "Settings.h"

#define LOG_TAG "Settings"

#include <memory>

#include "swappy/swappy.h"
#include "Log.h"

namespace samples {

Settings *Settings::getInstance() {
    static auto settings = std::make_unique<Settings>(ConstructorTag{});
    return settings.get();
}

void Settings::addListener(Listener listener) {
    std::lock_guard<std::mutex> lock(mMutex);
    mListeners.emplace_back(std::move(listener));
}

void Settings::setPreference(std::string key, std::string value) {
    if (key == "refresh_period") {
        Swappy_setRefreshPeriod(std::stoll(value));
    } else if (key == "swap_interval") {
        Swappy_setSwapIntervalNS(std::stoi(value) * 1e6);
    } else if (key == "use_affinity") {
        Swappy_setUseAffinity(value == "true");
    } else if (key == "hot_pocket") {
        std::lock_guard<std::mutex> lock(mMutex);
        mHotPocket = (value == "true");
    } else {
        ALOGI("Can't find matching preference for %s", key.c_str());
        return;
    }

    // Notify the listeners without the lock held
    notifyListeners();
}

std::chrono::nanoseconds Settings::getRefreshPeriod() const {
    return std::chrono::nanoseconds(Swappy_getRefreshPeriodNanos());
}

int32_t Settings::getSwapIntervalNS() const {
    return Swappy_getSwapIntervalNS();
}

bool Settings::getUseAffinity() const {
    return Swappy_getUseAffinity();
}

bool Settings::getHotPocket() const {
    return mHotPocket;
}

void Settings::notifyListeners() {
    // Grab a local copy of the listeners
    std::vector<Listener> listeners;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        listeners = mListeners;
    }

    // Call the listeners without the lock held
    for (const auto &listener : listeners) {
        listener();
    }
}

} // namespace samples
