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

#ifndef ANDROID_HARDWARE_HIDL_HAL_DEATH_HANDLER_H
#define ANDROID_HARDWARE_HIDL_HAL_DEATH_HANDLER_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include <hidl/HidlSupport.h>
#include <utils/Singleton.h>

using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;

namespace android {

class HalDeathHandler : public hidl_death_recipient, private Singleton<HalDeathHandler> {
  public:
    typedef std::function<void()> AtExitHandler;

    // Note that the exit handler gets called using a thread from
    // RPC threadpool, thus it needs to be thread-safe.
    void registerAtExitHandler(void* cookie, AtExitHandler handler);
    void unregisterAtExitHandler(void* cookie);

    // hidl_death_recipient
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who);

    // Used both for (un)registering handlers, and for passing to
    // '(un)linkToDeath'.
    static sp<HalDeathHandler> getInstance();

  private:
    friend class Singleton<HalDeathHandler>;
    typedef std::unordered_map<void*, AtExitHandler> Handlers;

    HalDeathHandler();
    virtual ~HalDeathHandler();

    sp<HalDeathHandler> mSelf;  // Allows the singleton instance to live forever.
    std::mutex mHandlersLock;
    Handlers mHandlers;
};

}  // namespace android

#endif // ANDROID_HARDWARE_HIDL_HAL_DEATH_HANDLER_H
