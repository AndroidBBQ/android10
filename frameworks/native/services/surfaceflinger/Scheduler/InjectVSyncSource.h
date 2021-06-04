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

#include <mutex>

#include "EventThread.h"

namespace android {

/**
 * VSync signals used during SurfaceFlinger trace playback (traces we captured
 * with SurfaceInterceptor).
 */
class InjectVSyncSource final : public VSyncSource {
public:
    ~InjectVSyncSource() override = default;

    void setCallback(VSyncSource::Callback* callback) override {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mCallback = callback;
    }

    void onInjectSyncEvent(nsecs_t when) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        if (mCallback) {
            mCallback->onVSyncEvent(when);
        }
    }

    void setVSyncEnabled(bool) override {}
    void setPhaseOffset(nsecs_t) override {}
    void pauseVsyncCallback(bool) {}

private:
    std::mutex mCallbackMutex;
    VSyncSource::Callback* mCallback GUARDED_BY(mCallbackMutex) = nullptr;
};

} // namespace android