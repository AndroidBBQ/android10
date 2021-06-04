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

#define LOG_TAG "HalDeathHandler"
//#define LOG_NDEBUG 0

#include <utils/Log.h>

#include <media/audiohal/hidl/HalDeathHandler.h>

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(HalDeathHandler);

// static
sp<HalDeathHandler> HalDeathHandler::getInstance() {
    return &Singleton<HalDeathHandler>::getInstance();
}

HalDeathHandler::HalDeathHandler() : mSelf(this) {
}

HalDeathHandler::~HalDeathHandler() {
}

void HalDeathHandler::registerAtExitHandler(void* cookie, AtExitHandler handler) {
    std::lock_guard<std::mutex> guard(mHandlersLock);
    mHandlers.insert({cookie, handler});
}

void HalDeathHandler::unregisterAtExitHandler(void* cookie) {
    std::lock_guard<std::mutex> guard(mHandlersLock);
    mHandlers.erase(cookie);
}

void HalDeathHandler::serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) {
    // No matter which of the service objects has died,
    // we need to run all the registered handlers and exit.
    std::lock_guard<std::mutex> guard(mHandlersLock);
    for (const auto& handler : mHandlers) {
        handler.second();
    }
    ALOGE("HAL server crashed, audio server is restarting");
    _exit(1);  // Avoid calling atexit handlers, as this code runs on a thread from RPC threadpool.
}

} // namespace android
